#include "Global.h"
#include "network/HttpClient.h"
#include <regex>

#define database cocos2d::UserDefault::getInstance()

using std::regex;
using std::match_results;
using std::regex_match;
using std::cmatch;
using namespace cocos2d::network;

string Global::game_session_id = "";
string Global::game_username = "";
long Global::score = 0;

Global::Global()
{
}


Global::~Global()
{
}

string Global::toString(vector<char> *buffer) {
    string rst;
	if (buffer) {
		for (char ch : *buffer) {
			rst.push_back(ch);
		}
	}
    return rst;
}

string Global::getSessionIdFromHeader(string head) {
    regex nlp("\\r\\n");
    string header = regex_replace(head, nlp, " ");
    regex pattern(".*GAMESESSIONID=(.*) Content-Type.*");
    //match_results<std::string::const_iterator> result;
    cmatch result;
    bool valid = regex_match(header.c_str(), result, pattern);

    if (!valid) {
        return "";
    }
    return result[1];
}

void Global::saveStatus(vector<char>* buffer, string username)
{
	game_session_id = getSessionIdFromHeader(toString(buffer));
	game_username = username;
	
	// save to local
	database->setStringForKey("username", game_username);
}

void Global::loadStatus()
{
	game_username = database->getStringForKey("username");
}

string Global::getRandomStr()
{
	std::stringstream ss;
	int randNum = rand();
	ss << randNum;
	std::string res;
	ss >> res;
	return res;
}
