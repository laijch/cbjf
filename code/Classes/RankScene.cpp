#include "RankScene.h"
#include "WelcomeScene.h"
#include "Global.h"
#include "json/rapidjson.h"
#include "json/document.h"
#include "json/writer.h"
#include "json/stringbuffer.h"

using std::string;
using std::vector;
using namespace rapidjson;
USING_NS_CC;

cocos2d::Scene* RankScene::createScene() {
	// 'scene' is an autorelease object
	auto scene = Scene::create();

	// 'layer' is an autorelease object
	auto layer = RankScene::create();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

bool RankScene::init() {
	if (!Layer::init())
	{
		return false;
	}

	size = Director::getInstance()->getVisibleSize();

	addRankBoard();
	getRank();

	return true;
}

void RankScene::addRankBoard() {
	auto rank_board = Sprite::create("Background.jpg");
	rank_board->setScale(Director::getInstance()->getContentScaleFactor());
	rank_board->setPosition(Vec2(size.width / 2, size.height / 2));
	this->addChild(rank_board, 0);

	rank_field = TextField::create("", "Arial", 30);
	rank_field->setPosition(Size(size.width / 2, size.height / 2 + 100));
	rank_field->setColor(Color3B::BLACK);
	this->addChild(rank_field, 2);

	// exit button
	auto exit_button = MenuItemImage::create(
		"exit.png",
		"exit.png",
		CC_CALLBACK_1(RankScene::exitScene, this));

	exit_button->setPosition(Vec2(100, exit_button->getContentSize().height));

	auto menu = Menu::create(exit_button, NULL);
	menu->setPosition(Vec2::ZERO);

	addChild(menu, 4);
}

void RankScene::getRank() {
	HttpRequest* request = new HttpRequest();
	vector<string> headers;
	// set content-type and cookie
	headers.push_back("Content-Type: application/x-www-form-urlencoded; charset=UTF-8");
	headers.push_back("Cookies: GAMESESSIONID=" + Global::game_session_id);
	request->setHeaders(headers);
	request->setUrl(("http://localhost:8080/rank?top=10&&rand=" + Global::getRandomStr()).c_str());
	request->setRequestType(HttpRequest::Type::GET);
	request->setResponseCallback(CC_CALLBACK_2(RankScene::onGetRankResponse, this));

	request->setTag("GET rank");
	HttpClient::getInstance()->send(request);
	request->release();
}

void RankScene::onGetRankResponse(HttpClient * sender, HttpResponse *response) {
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
		}
		else {
			setRankBoard(d["info"].GetString());
		}
	}
}

void RankScene::setRankBoard(string rankStr) {
	if (rankStr.empty()) return;

	for (unsigned i = 1; i < rankStr.size(); ++i) {
		if (rankStr[i] == '|') {
			rankStr[i] = '\n';
		}
	}

	rank_field->setString(rankStr.substr(1));
}

void RankScene::exitScene(cocos2d::Ref* pSender) {
	Director::getInstance()->replaceScene(TransitionSlideInL::create(1.0, WelcomeScene::createScene()));
}
