#pragma once

#include "../../mylib.h"


class WS_CLIENT
{
	WSADATA wsa;
	SOCKET sock;
	
	
public:
	std::list<std::string> chatlist;
	//std::list<CharacterPosition> other_player_list;
	CharacterPosition* cps;
	int other_playercnt=0;

	WS_CLIENT();
	~WS_CLIENT();
	void initiate();
	SOCKET getSocket() { return sock; }
	void disconnect();

	void sendPlayer(CharacterPosition cp);
	void sendMessage(char* buf, int bufsize);

	void recieverThread();

	void displayChatlist();
};

