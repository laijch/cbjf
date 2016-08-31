#pragma once

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "network/HttpClient.h"

using namespace cocos2d::ui;
using namespace cocos2d::network;

class RankScene : public cocos2d::Layer
{
public:
	// there's no 'id' in cpp, so we recommend returning the class instance pointer
	static cocos2d::Scene* createScene();

	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();

	// implement the "static create()" method manually
	CREATE_FUNC(RankScene);

private:
	TextField * rank_field;
	cocos2d::Size size;

	void addRankBoard();
	
	void getRank();

	void onGetRankResponse(HttpClient * sender, HttpResponse *response);

	void setRankBoard(std::string info);

	void exitScene(cocos2d::Ref* pSender);
};
