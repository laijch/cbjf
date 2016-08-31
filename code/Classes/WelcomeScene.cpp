#include "WelcomeScene.h"
#include "HelpScene.h"
#include "RankScene.h"
#include "GameScene.h"
#include "Global.h"
#include "SimpleAudioEngine.h"

using namespace rapidjson;
using namespace CocosDenshion;
USING_NS_CC;

Scene* WelcomeScene::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = WelcomeScene::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool WelcomeScene::init()
{
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }

	SimpleAudioEngine::getInstance()->stopAllEffects();
	SimpleAudioEngine::getInstance()->pauseBackgroundMusic();

	Size size = Director::getInstance()->getVisibleSize();
	visibleHeight = size.height;
	visibleWidth = size.width;
 
	addBackground();

	// 添加各个场景的入口
	addEntrance();

	text_field = TextField::create("Player Name", "fonts/arial.ttf", 30);
	text_field->setPosition(Size(visibleWidth / 2, visibleHeight / 4));
	text_field->setTextColor(cocos2d::Color4B::BLACK);
	this->addChild(text_field, 2);

	// load data from local store
	Global::loadStatus();
	login(Global::game_username);
	
    return true;
}

void WelcomeScene::addBackground()
{
	auto bg_sprite = Sprite::create("fengmian.jpeg");
	bg_sprite->setPosition(visibleWidth / 2, visibleHeight / 2);
	bg_sprite->setScale(visibleWidth / bg_sprite->getContentSize().width, visibleHeight / bg_sprite->getContentSize().height);
	this->addChild(bg_sprite, 0);

	auto logo_sprite = Sprite::create("logo.png");
	logo_sprite->setPosition(visibleWidth / 4, visibleHeight / 2);
	this->addChild(logo_sprite, 0);
}

void WelcomeScene::addEntrance()
{
	play_button = Button::create();
	play_button->setTitleText("Play");
	play_button->setTitleFontSize(30);
	play_button->setColor(cocos2d::Color3B::BLACK);
	play_button->setPosition(Size(visibleWidth / 16 * 13, visibleHeight / 8 * 5));
	play_button->addTouchEventListener(CC_CALLBACK_2(WelcomeScene::play, this));
	this->addChild(play_button, 2);

	exit_button = Button::create();
	exit_button->setTitleText("Exit");
	exit_button->setTitleFontSize(30);
	exit_button->setColor(cocos2d::Color3B::BLACK);
	exit_button->setPosition(Size(visibleWidth / 16 * 13, visibleHeight / 16 * 3));
	exit_button->addTouchEventListener(CC_CALLBACK_2(WelcomeScene::exit, this));
	this->addChild(exit_button, 2);


	help_button = Button::create();
	help_button->setTitleText("Help");
	help_button->setTitleFontSize(30);
	help_button->setColor(cocos2d::Color3B::BLACK);
	help_button->setPosition(Size(visibleWidth / 16 * 11, visibleHeight / 8 * 3));
	help_button->addTouchEventListener(CC_CALLBACK_2(WelcomeScene::help, this));
	this->addChild(help_button, 2);

	rank_button = Button::create();
	rank_button->setTitleText("Rank");
	rank_button->setTitleFontSize(30);
	rank_button->setColor(cocos2d::Color3B::BLACK);
	rank_button->setPosition(Size(visibleWidth / 16 * 15, visibleHeight / 8 * 3));
	rank_button->addTouchEventListener(CC_CALLBACK_2(WelcomeScene::rank, this));
	this->addChild(rank_button, 2);
}

// 跳转至游戏画面
void WelcomeScene::play(cocos2d::Ref * ref, Widget::TouchEventType touchType)
{
	if (touchType == Widget::TouchEventType::ENDED)
	{
		login(text_field->getString());
	}
}

// 结束游戏，关闭窗口
void WelcomeScene::exit(cocos2d::Ref * ref, Widget::TouchEventType touchType)
{
	if (touchType == Widget::TouchEventType::ENDED)
	{
		Global::saveStatus(NULL, "");
		Director::getInstance()->end();
	}
}

// 游戏帮助页面，显示游戏玩法
void WelcomeScene::help(cocos2d::Ref * ref, Widget::TouchEventType touchType)
{
	if (touchType == Widget::TouchEventType::ENDED)
	{
		Director::getInstance()->replaceScene(TransitionSlideInR::create(1.0, HelpScene::createScene()));
	}
}

// 排行榜，不需要登录，也能查看的
void WelcomeScene::rank(cocos2d::Ref * ref, Widget::TouchEventType touchType)
{
	if (touchType == Widget::TouchEventType::ENDED)
	{
		Director::getInstance()->replaceScene(TransitionSlideInR::create(1.0, RankScene::createScene()));
	}
}

void WelcomeScene::login(string username)
{
	if (!username.empty()) {
		HttpRequest* request = new HttpRequest();
		vector<string> headers;
		// set content-type and cookie
		headers.push_back("Content-Type: application/x-www-form-urlencoded; charset=UTF-8");
		request->setHeaders(headers);
		request->setUrl("http://localhost:8080/login");
		request->setRequestType(HttpRequest::Type::POST);
		request->setResponseCallback(CC_CALLBACK_2(WelcomeScene::onLoginResponse, this));

		// write the post data
		string postDataStr = "username=" + username;
		request->setRequestData(postDataStr.c_str(), postDataStr.size());
		request->setTag("login");
		HttpClient::getInstance()->send(request);
		request->release();
	}
}

void WelcomeScene::onLoginResponse(HttpClient * sender, HttpResponse *response)
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
		if (result) {
			Global::saveStatus(response->getResponseHeader(), text_field->getString());
			Director::getInstance()->replaceScene(TransitionSlideInT::create(1.0, GameScene::createScene()));
		}
		else {
			CCLOG("Failed to login: %s\n", d["info"].GetString());
		}
	}
}