#pragma once

#include "cocos2d.h"
#include "SimpleAudioEngine.h"
#include "network/HttpClient.h"
using namespace cocos2d::network;

USING_NS_CC;

enum STATE_T {
	PERFECT, GOOD, MISS
};

class GameScene : public cocos2d::Layer
{
public:
	void setPhysicsWorld(PhysicsWorld * world);
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init(PhysicsWorld* world);

	static GameScene* create(PhysicsWorld* world);

    // implement the "static create()" method manually
    //CREATE_FUNC(GameScene);

private:
	cocos2d::ProgressTimer* pT;
	int EnergyBarPercentage;

	PhysicsWorld* my_world;
	Sprite* fly_out;
	Sprite* one_half;
	Sprite* another_half;
	bool flag;

	Sprite *bonus_text;
	Sprite *board;

	cocos2d::Size visibleSize;
	cocos2d::Vec2 origin;

	cocos2d::Label* time;
	int total_time;
	cocos2d::Label *score_board;
	int score;
	//int time_stamp;

	int energy_bar;

	void preloadMusic();
	void playBgm();

	void addEdge();

	cocos2d::Vec2 position[6];
	double scale_width;
	double scale_height;

	void addNote(int n);
	void flyOut();
	Sprite* createHalf(int mode, Vec2 pos);
	void update(float f);
	void updateScore(float f);
	void updateNotes(float f);
	void stopAc();

	void addBackground();
	void addWidgets();
	void generateNotes();

	void addContactListener();
	bool onConcactBegan(PhysicsContact& contact);

	void CatchFly();
	void addKeyboardListener();
	void onKeyPressed(EventKeyboard::KeyCode code, Event* event);
	void onKeyReleased(EventKeyboard::KeyCode code, Event* event);
	// check the distance of note and track_n;
	void checkMeet(int trackNum);

	void gameOver();
	void restart(cocos2d::Ref* pSender);
	void exitScene(cocos2d::Ref* pSender);

	// update score and show message
	void setBonus(STATE_T t);

	// upload score
	void sumitGrade();
	void onSubmitResponse(HttpClient * sender, HttpResponse *response);

	std::vector<Sprite*> tracks;
	std::vector<Sprite*> keys;
	std::queue<Sprite*> notes;
	int note_counter;
	std::string note_list;
};

