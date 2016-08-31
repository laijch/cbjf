#include "GameScene.h"
#include "WelcomeScene.h"
#include "Global.h"
#include "json/rapidjson.h"
#include "json/document.h"
#include "json/writer.h"
#include "json/stringbuffer.h"

USING_NS_CC;
using namespace std;
using namespace CocosDenshion;
using namespace rapidjson;

void GameScene::setPhysicsWorld(PhysicsWorld* world) { my_world = world; }

cocos2d::Scene* GameScene::createScene() {
	auto scene = Scene::createWithPhysics();
	//scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
	//scene->getPhysicsWorld()->setGravity(Point(0, 0));


	auto layer = GameScene::create(scene->getPhysicsWorld());
	scene->addChild(layer);
	return scene;
}

GameScene * GameScene::create(PhysicsWorld * world) {
	GameScene* pRet = new(std::nothrow) GameScene();
	if (pRet && pRet->init(world)) {
		pRet->autorelease();
		return pRet;
	}
	delete pRet;
	pRet = NULL;
	return NULL;
}

bool GameScene::init(PhysicsWorld* world) {
    if (!Layer::init())
    {
        return false;
    }
	visibleSize = Director::getInstance()->getVisibleSize();
	origin = Director::getInstance()->getVisibleOrigin();
	energy_bar = 0;
	flag = false;

	this->setPhysicsWorld(world);
	total_time = 120;
	EnergyBarPercentage = 0;
	note_counter = 0;
	schedule(schedule_selector(GameScene::update), 1.0f, kRepeatForever, 0);
	schedule(schedule_selector(GameScene::updateScore), 0.1f, kRepeatForever, 0);
	schedule(schedule_selector(GameScene::updateNotes), 0.5f, kRepeatForever, 0);

	// listener
	addKeyboardListener();
	addContactListener();

	// ui
	addBackground();
	addEdge();
	addWidgets();

	// music
	preloadMusic();
	playBgm();

	generateNotes();

    return true;
}

// note list, one note per 0.5 second
// 120 second, 240 notes in total
void GameScene::generateNotes() {
	note_list = std::string()
		+ "66662666666646666266"
		+ "03031414252560125436"
		+ "01234565432106234150"
		+ "60303141425256543210"
		+ "25254141303066001122"
		+ "55443362103456543210"
		+ "55443362103456543210"
		+ "03031414252560125436"
		+ "01234565432106234150"
		+ "60303141425256543210"
		+ "25254141303066001122"
		+ "55443362103456543210";
}

void GameScene::addKeyboardListener() {
	auto keboardListener = EventListenerKeyboard::create();
	keboardListener->onKeyPressed = CC_CALLBACK_2(GameScene::onKeyPressed, this);
	keboardListener->onKeyReleased = CC_CALLBACK_2(GameScene::onKeyReleased, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(keboardListener, this);
}

void GameScene::onKeyPressed(EventKeyboard::KeyCode code, Event* event) {
	switch (code)
	{
	case cocos2d::EventKeyboard::KeyCode::KEY_SPACE:
		CatchFly();
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_A:
		checkMeet(0);
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_S:
		checkMeet(1);
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_D:
		checkMeet(2);
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_J:
		checkMeet(3);
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_K:
		checkMeet(4);
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_L:
		checkMeet(5);
		break;
	default:
		break;
	}
}
void GameScene::onKeyReleased(EventKeyboard::KeyCode code, Event* event) {

}

// check if catching notes and set bonus
void GameScene::checkMeet(int trackNum)
{
	ParticleFireworks* ps = ParticleFireworks::create();
	ps->setPosition(tracks[trackNum]->getPosition());
	ps->setDuration(0.1f);
	addChild(ps);
	if (notes.size() != 0) {
		// calculate the distance of key and note
		float distance = tracks[trackNum]->getPosition().getDistance(notes.front()->getPosition());
		if (distance >= 0 && distance < notes.front()->getContentSize().height/2) {
			notes.front()->removeFromParent();
			notes.pop();
			setBonus(PERFECT);
		}
		else if (distance < notes.front()->getContentSize().height) {
			setBonus(GOOD);
			notes.front()->removeFromParent();
			notes.pop();
		}
	}
}

// break fly out into 2 pieces
Sprite* GameScene::createHalf(int mode, Vec2 pos) {
	Sprite* result;
	if (mode == 1) {
		result = Sprite::create("GameScene/flyOut1.PNG");
	}
	else {
		result = Sprite::create("GameScene/flyOut1.PNG");
	}
	result->setPhysicsBody(PhysicsBody::createCircle(result->getContentSize().height / 2));
	result->setAnchorPoint(Vec2(0.5, 0.5));
	result->setPosition(pos);
	result->setScale(0.5);
	result->getPhysicsBody()->setCategoryBitmask(0x02);
	result->getPhysicsBody()->setCollisionBitmask(0x00);
	result->getPhysicsBody()->setContactTestBitmask(0x02);
	result->getPhysicsBody()->setVelocity(Vec2(fly_out->getPositionX() + mode * 500, -500));
	result->getPhysicsBody()->setAngularVelocity(CCRANDOM_0_1() * 5);
	addChild(result);
	return result;
}

void GameScene::CatchFly() {
	if (flag && fly_out->getPositionX() < visibleSize.width) {
		if (EnergyBarPercentage > 20) EnergyBarPercentage = 30;
		else  EnergyBarPercentage += 10;
		pT->setPercentage(EnergyBarPercentage);

		auto position = fly_out->getPosition();
		createHalf(1, position);
		createHalf(-1, position);
		fly_out->removeFromParentAndCleanup(true);
	}
	flag = false;
}

void GameScene::addContactListener() {
	auto ContactListener = EventListenerPhysicsContact::create();
	ContactListener->onContactBegin = CC_CALLBACK_1(GameScene::onConcactBegan, this);
	_eventDispatcher->addEventListenerWithFixedPriority(ContactListener, 1);
}

bool GameScene::onConcactBegan(PhysicsContact& contact) {
	auto nodeA = contact.getShapeA()->getBody();
	auto nodeB = contact.getShapeB()->getBody();
	auto spA = nodeA->getNode();
	auto spB = nodeB->getNode();
	if (spA && spB) {
		// when the note contack with the key/clearBar, remove and pop the note
		if (nodeA->getCategoryBitmask() == 5 && notes.size() != 0) {
			spA->removeFromParent();
			notes.pop();
			// when spB is the key, set Miss
			if (nodeB->getTag() == 30) setBonus(MISS);
			auto ps = ParticleGalaxy::create();
			ps->setPosition(nodeA->getPosition());
			ps->setDuration(0.1f);
			this->addChild(ps);
		} else if (nodeB->getCategoryBitmask() == 5 && notes.size() != 0) {
			spB->removeFromParent();
			notes.pop();
			if (nodeA->getTag() == 30) setBonus(MISS);
			auto ps = ParticleGalaxy::create();
			ps->setPosition(nodeA->getPosition());
			ps->setDuration(0.1f);
			this->addChild(ps);
		} else if (nodeA->getCategoryBitmask() == 2 && nodeB->getTag() == 60) {
			spA->removeFromParent();
		} else if (nodeB->getCategoryBitmask() == 2 && nodeA->getTag() == 60) {
			spB->removeFromParent();
		}

		if (nodeB->getTag() == 50 || nodeA->getTag() == 50) {
			flag = false;
		}
		return true;
	}
	return false;
}

void GameScene::addEdge() {
	auto edgeSp = Sprite::create();
	auto boundBody = PhysicsBody::createEdgeBox(visibleSize);
	boundBody->setDynamic(false);
	boundBody->setTag(60);
	edgeSp->setPosition(Point(visibleSize.width / 2, visibleSize.height / 2));
	edgeSp->setScale(Director::getInstance()->getContentScaleFactor());
	edgeSp->setPhysicsBody(boundBody);
	edgeSp->getPhysicsBody()->setCategoryBitmask(0x03);
	edgeSp->getPhysicsBody()->setCollisionBitmask(0x00);
	edgeSp->getPhysicsBody()->setContactTestBitmask(0x03);
	this->addChild(edgeSp);
}

void GameScene::addBackground()
{
	// add background
	auto background = Sprite::create("GameScene/Background.jpg");
	background->setScale(Director::getInstance()->getContentScaleFactor());
	background->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
	this->addChild(background, 0);

	// add 6 tracks
	for (unsigned i = 0; i < 6; ++i) {
		position[i] = Vec2((i + 2) * visibleSize.width / 8, visibleSize.height / 9);
		char trackPath[30];
		sprintf(trackPath, "GameScene/originTrack%d.png", i % 2 + 1);
		auto track = Sprite::create(trackPath);

		if (i == 0) {
			scale_width = (visibleSize.width / 10) / track->getContentSize().width;
			scale_height = 2 * visibleSize.height / track->getContentSize().height;
		}
		track->setScale(scale_width, scale_height);
		track->setPosition(position[i]);
		tracks.push_back(track);
		this->addChild(track, 1);
	}

	// add 6 keys
	for (unsigned i = 0; i < 6; ++i) {
		char keyPath[20];
		sprintf(keyPath, "GameScene/key%d.png", i % 2 + 1);
		auto key = Sprite::create(keyPath);
		key->setAnchorPoint(Vec2(0.5, 0.5));
		key->setPhysicsBody(PhysicsBody::createBox(Size(key->getContentSize().width*scale_width, key->getContentSize().height*scale_width / 15)));
		// set bitmask
		auto keyBody = key->getPhysicsBody();
		keyBody->setCategoryBitmask(0x03);
		keyBody->setCollisionBitmask(0x00);
		keyBody->setContactTestBitmask(0x03);
		keyBody->setTag(30);
		keyBody->setGravityEnable(false);
		key->setScale(scale_width);
		key->setPosition(position[i]);
		keys.push_back(key);
		this->addChild(key, 1);
		// reset the position for the position of the notes
		position[i] = Vec2((i + 2) * visibleSize.width / 8, visibleSize.height - key->getContentSize().height/ 2);
	}
}

void GameScene::addWidgets() {
	TTFConfig ttfConfig;
	ttfConfig.fontFilePath = "fonts/arial.ttf";
	ttfConfig.fontSize = 40;

	// timer
	time = Label::createWithTTF(ttfConfig, "120");
	time->setColor(cocos2d::Color3B::YELLOW);
	time->setPosition(Vec2(visibleSize.width / 8, visibleSize.height - visibleSize.height / 8));

	// score counter
	score_board = Label::createWithTTF(ttfConfig, "Score: 0");
	score_board->setColor(cocos2d::Color3B::YELLOW);
	score_board->setPosition(Vec2(score_board->getContentSize().width,
		visibleSize.height - visibleSize.height / 4));

	// energy bar
	Sprite* sp0 = Sprite::create("GameScene/energy.PNG", CC_RECT_PIXELS_TO_POINTS(Rect(0, 0, 42, 363)));
	Sprite* sp = Sprite::create("GameScene/sp.PNG", CC_RECT_PIXELS_TO_POINTS(Rect(0, 0, 20, 10)));
	pT = ProgressTimer::create(sp);
	pT->setScaleY(120);
	pT->setAnchorPoint(Vec2(0, 0));
	pT->setType(ProgressTimerType::BAR);
	pT->setBarChangeRate(Point(0, 1));
	pT->setMidpoint(Point(0, 0));
	pT->setPercentage(EnergyBarPercentage);
	pT->setPosition(Vec2(origin.x + 110, origin.y + 100));
	sp0->setAnchorPoint(Vec2(0, 0));
	sp0->setPosition(Vec2(origin.x + 100, origin.y + 100));

	ActionInterval *jump_to = JumpBy::create(1, Vec2(0, 0), 20, 1);
	ActionInterval *forever = RepeatForever::create(jump_to);
	time->runAction(forever);

	// exit button
	auto exit_button = MenuItemImage::create(
		"exit.png",
		"exit.png",
		CC_CALLBACK_1(GameScene::exitScene, this));

	exit_button->setPosition(Vec2(origin.x + 100, exit_button->getContentSize().height));

	auto menu = Menu::create(exit_button, NULL);
	menu->setPosition(Vec2::ZERO);

	// add to stage
	addChild(menu, 4);
	addChild(time, 3);
	addChild(score_board, 3);
	addChild(pT, 4);
	addChild(sp0, 3);
}

// update timer
// invoke stopAc() gameOver() in the end
void GameScene::update(float f) {
	if (total_time > 1) {
		if (total_time % 8 == 0) {
			if (random() % 10 > 5)
				flyOut();
		}

		total_time--;
		std::stringstream s;
		std::string updateTime;
		s << total_time;
		s >> updateTime;
		time->setString(updateTime);
	}
	else {
		time->setString("Over");
		stopAc();
		gameOver();
	}
}

void GameScene::updateScore(float f) {
	std::stringstream s;
	std::string new_score;
	s << score;
	s >> new_score;
	score_board->setString("Score: " + new_score);

	if (EnergyBarPercentage >= 30) {
		auto clearBar = Sprite::create("GameScene/bar.PNG");
		clearBar->setPosition(Vec2(visibleSize.width / 1.8, visibleSize.height / 9));
		clearBar->setPhysicsBody(PhysicsBody::createBox(clearBar->getContentSize()));
		clearBar->getPhysicsBody()->setGravityEnable(false);
		clearBar->getPhysicsBody()->setCategoryBitmask(0x03);
		clearBar->getPhysicsBody()->setCollisionBitmask(0x00);
		clearBar->getPhysicsBody()->setContactTestBitmask(0x03);
		addChild(clearBar, 3);
		score += 1000;
		clearBar->runAction(MoveTo::create(1, Vec2(visibleSize.width / 1.8, visibleSize.height + 500)));
		EnergyBarPercentage = 0;
		pT->setPercentage(EnergyBarPercentage);
	}
}

// add notes according to note list
void GameScene::updateNotes(float f)
{
	addNote(note_list[note_counter++] - '0');
}

void GameScene::stopAc() {
	unschedule(schedule_selector(GameScene::update));
	unschedule(schedule_selector(GameScene::updateNotes));
	unschedule(schedule_selector(GameScene::updateScore));
	SimpleAudioEngine::getInstance()->stopAllEffects();
	SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}

void GameScene::flyOut() {
	fly_out = Sprite::create("GameScene/flyOut.PNG");
	fly_out->setAnchorPoint(Vec2(0.5, 0.5));
	auto physic_body =  PhysicsBody::createCircle(fly_out->getContentSize().height / 2);
	physic_body->setGravityEnable(false);
	fly_out->setPhysicsBody(physic_body);
	fly_out->setScale(0.5);
	fly_out->setPosition(Vec2(visibleSize.width / 8, visibleSize.height / 2));
	fly_out->setTag(50);
	fly_out->getPhysicsBody()->setCategoryBitmask(0x02);
	fly_out->getPhysicsBody()->setCollisionBitmask(0x00);
	fly_out->getPhysicsBody()->setContactTestBitmask(0x02);
	addChild(fly_out);
	//fly_out->runAction(MoveTo::create(3, position - Vec2(0, 8 * visibleSize.height / 9)));
	fly_out->runAction(MoveTo::create(2, Vec2(visibleSize.width + fly_out->getContentSize().width, visibleSize.height / 2)));
	flag = true;
}


void GameScene::preloadMusic()
{
	auto audio = SimpleAudioEngine::getInstance();
	audio->preloadBackgroundMusic("music/yeshi.mp3");
}

void GameScene::playBgm()
{
	SimpleAudioEngine::getInstance()->playBackgroundMusic("music/yeshi.mp3", true);
}

// add the note
void GameScene::addNote(int n)
{
	if (n < 0 || n > 5) return;
	char notePath[20];
	sprintf(notePath, "GameScene/note%d.png", n % 2 + 1);
	auto note = Sprite::create(notePath);
	note->setPhysicsBody(PhysicsBody::createCircle(note->getContentSize().height / 5));
	note->setAnchorPoint(Vec2(0.5, 0.5));
	note->setScale(scale_width);
	note->setPosition(position[n]);
	// set bitmask
	auto noteBody = note->getPhysicsBody();
	noteBody->setCategoryBitmask(0x05);
	noteBody->setCollisionBitmask(0x00);
	noteBody->setContactTestBitmask(0x05);
	noteBody->setGravityEnable(false);

	notes.push(note);
	this->addChild(note, 1);

	note->runAction(MoveTo::create(0.8f, position[n] - Vec2(0, 8 * visibleSize.height / 9)));
}

// set bonus text when the player hit the note
void GameScene::setBonus(STATE_T t)
{
	if (bonus_text) {
		bonus_text->removeFromParent();
	}
	int bonus;
	switch (t)
	{
	case PERFECT:
		bonus_text = Sprite::create("GameScene/perfect.png");
		bonus = 300;
		break;
	case GOOD:
		bonus = 100;
		bonus_text = Sprite::create("GameScene/good.png");
		break;
	case MISS:
		bonus = 0;
		bonus_text = Sprite::create("GameScene/miss.png");
		break;
	default:
		bonus = 0;
		bonus_text = Sprite::create("GameScene/miss.png");
		break;
	}

	score += bonus;
	if (bonus > 0) EnergyBarPercentage += 1;
	pT->setPercentage(EnergyBarPercentage);
	bonus_text->setPosition(visibleSize.width / 2, visibleSize.height / 2);
	addChild(bonus_text, 2);
}

void GameScene::gameOver() {
	// remove all event listener before restart
	_eventDispatcher->removeAllEventListeners();
	// upload score
	sumitGrade();
	board = Sprite::create("board.png");
	board->setPosition(visibleSize.width / 2, visibleSize.height / 2);
	Size board_size = board->getContentSize();

	auto info = Label::createWithTTF("Game Over", "fonts/arial.ttf", 40);
	info->setColor(Color3B::BLACK);
	info->setPosition(board_size.width / 2, board_size.height / 4 * 3);

	// add buttons
	auto restart_button = MenuItemImage::create(
		"restart.png",
		"restart.png",
		CC_CALLBACK_1(GameScene::restart, this));

	restart_button->setPosition(Vec2(board_size.width / 4,
		board_size.height / 4 + 50));

	auto exit_button = MenuItemImage::create(
		"exit_big.png",
		"exit_big.png",
		CC_CALLBACK_1(GameScene::exitScene, this));

	exit_button->setPosition(Vec2(visibleSize.width / 4 + 200,
		board_size.height / 4 + 50));

	// create menu, it's an autorelease object
	auto menu = Menu::create(restart_button, exit_button, NULL);
	menu->setPosition(Vec2::ZERO);

	board->addChild(menu, 1);
	board->addChild(info, 1);

	addChild(board, 4);
}

void GameScene::restart(cocos2d::Ref* pSender) {
	stopAc();
	Director::getInstance()->replaceScene(TransitionProgressOutIn::create(1.0, GameScene::createScene()));
}

void GameScene::exitScene(cocos2d::Ref* pSender) {
	stopAc();
	Director::getInstance()->replaceScene(TransitionSlideInL::create(1.0, WelcomeScene::createScene()));
}

void GameScene::sumitGrade()
{
	HttpRequest* request = new HttpRequest();
	vector<string> headers;
	// set content-type and cookie
	headers.push_back("Content-Type: application/x-www-form-urlencoded; charset=UTF-8");
	headers.push_back("Cookies: GAMESESSIONID=" + Global::game_session_id);
	log(Global::game_session_id.c_str());
	request->setHeaders(headers);
	request->setUrl("http://localhost:8080/submit");
	request->setRequestType(HttpRequest::Type::POST);
	request->setResponseCallback(CC_CALLBACK_2(GameScene::onSubmitResponse, this));

	std::stringstream ss;
	std::string score_str;
	ss << score;
	ss >> score_str;
	// write the post data
	string postDataStr = "score=" + score_str;
	request->setRequestData(postDataStr.c_str(), postDataStr.size());
	request->setTag("submit");
	HttpClient::getInstance()->send(request);
	request->release();
}

void GameScene::onSubmitResponse(HttpClient * sender, HttpResponse *response)
{

	if (!response) return;

	if (!response->isSucceed()) {
		log("response failed");
		log("error buffer: %s", response->getErrorBuffer());
		return;
	}

	string res = Global::toString(response->getResponseData());
	Document d;
	d.Parse<0>(res.c_str());

	if (d.HasParseError()) {
		CCLOG("GetParseError %s\n", d.GetParseError());
	}

	if (d.IsObject() && d.HasMember("result") && d.HasMember("info")) {
		bool result = d["result"].GetBool();
		if (!result) {
			CCLOG("Failed to login: %s\n", d["info"].GetString());
			CCLOG("Please submit again");
		}
	}
}

