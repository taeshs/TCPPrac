#pragma once

#include "../../../mylib/myheader.h"
#include "NGP/ngp.h"

typedef enum connection_status {
	NOT_CONN = 0,
	CONN_FAIL = -1,
	CONN_OKAY = 1,
}CONN_STATUS;

class Room {
private:
	ClientSock players [MAX_ROOM_PLAYER];
	int num_player;
	std::string name;
	CRITICAL_SECTION m_room_cs;

	int g_gameState = -1;
	int no = 0;
	

public:
	void lock();
	void unlock();
	
	Room(std::string n);
	FR enter_room(ClientSock player);
	FR leave(ClientSock player);
	void display();
	FR isin(ClientSock player);
	FR isin(SOCKET player);
	void chatToRoom(ClientSock player, char* buf, int buf_size);
	int GetClientNo();
	bool isGameReady();
	void setGameState(int);
	int getGameState();
	void resetRoom();

	int g_connection[2] = { NOT_CONN,NOT_CONN };
	char Buffer[2][BUFSIZE];
	Player_Socket Player[2];
	Bullet_Arr Bullets[2];
	Bullet_Alive_Arr aArr[2];
};

class Rooms_manager {
private:
	int room_no;
	std::list<Room*> room_list;
	CRITICAL_SECTION m_room_list_cs;

	void lock();
	void unlock();
	

public:
	Rooms_manager();
	~Rooms_manager();

	FR enter_room(ClientSock player);

	FR leave(ClientSock player);

	void display();

	void chatToRoom(ClientSock player, char* buf, int buf_size);
	Room* GetMyRoom(SOCKET socket);
};