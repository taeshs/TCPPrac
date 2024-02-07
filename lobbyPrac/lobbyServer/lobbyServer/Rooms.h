#pragma once

#include "../../mylib/myheader.h"

class Room {
private:
	SOCKET players[3];
	int num_player;
	std::string name;
	CRITICAL_SECTION m_room_cs;
	void lock();
	void unlock();

	

public:
	Room(std::string n);
	FR enter_room(SOCKET player);
	FR leave(SOCKET player);
	void display();
	FR isin(SOCKET player);
	void chatToRoom(SOCKET player, char* buf, int buf_size);
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

	FR enter_room(SOCKET player);

	FR leave(SOCKET player);

	void display();

	void chatToRoom(SOCKET player, char* buf, int buf_size);
};