#ifndef __WELCOME_SCENE_H__
#define __WELCOME_SCENE_H__

#include "ui/CocosGUI.h"
#include "network/HttpClient.h"
#include <string>
#include <vector>
using std::vector;
using std::string;
using namespace cocos2d::ui;
using namespace cocos2d::network;
#include "cocos2d.h"

class WelcomeScene : public cocos2d::Layer
{
public:
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();

    // implement the "static create()" method manually
    CREATE_FUNC(WelcomeScene);

private:
	float visibleHeight;
	float visibleWidth;
	Button *exit_button;
	Button *play_button;
	Button *help_button;
	Button *rank_button;
	TextField * text_field;

	void addBackground();
	void addEntrance();
	void login(string username);
	void onLoginResponse(HttpClient *, HttpResponse *);
	void play(cocos2d::Ref *ref, Widget::TouchEventType touchType);
	void exit(cocos2d::Ref *ref, Widget::TouchEventType touchType);
	void help(cocos2d::Ref *ref, Widget::TouchEventType touchType);
	void rank(cocos2d::Ref *ref, Widget::TouchEventType touchType);
};

#endif // __WELCOME_SCENE_H__
