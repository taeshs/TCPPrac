#include "Rooms.h"


//void GameThread() {
//	//GameServer();
//}
// Room class 
Room::Room(std::string n) : name{ n }, players{}, num_player{ 0 } {
	std::cout << "created new room." << std::endl;
	InitializeCriticalSection(&m_room_cs);

	/*std::thread th_gamethread(GameThread);
	th_gamethread.detach();*/
}

void Room::lock() {
	puts("room lockば");
	EnterCriticalSection(&m_room_cs);
}

void Room::unlock() {
	puts("room unlockに");
	LeaveCriticalSection(&m_room_cs);
}

FR Room::enter_room(ClientSock player) {
	lock();
	if (num_player == MAX_ROOM_PLAYER) {
		unlock();
		return FR::FAIL;
	}
	else {
		for (int i = 0; i < MAX_ROOM_PLAYER; i++) {
			if (players[i].socket == NULL) {
				players[i] = player;
				num_player++;
				unlock();
				return FR::SUCCESS;
			}
		}
	}
	unlock();
	return FR::F_ERROR;
}

FR Room::leave(ClientSock player) {
	lock();
	for (int i = 0; i < MAX_ROOM_PLAYER; i++) {
		if (players[i].socket == player.socket) {
			players[i].socket = NULL;
			num_player--;
			if (num_player == 0) {
				unlock();
				return FR::SUCCESS_EMPTY;
			}
			unlock();
			return FR::SUCCESS;
		}
	}
	unlock();
	return FR::FAIL;
}

void Room::display() {
	lock();
	std::cout << "Room name : " << name << " / nums of player : " << num_player << std::endl;;
	for (int i = 0; i < MAX_ROOM_PLAYER; i++) {
		if (players[i].socket == NULL) {
			std::cout << "EMPTY" << std::endl;
		}
		else {
			std::cout << "player '" << players[i].socket << "'" << ", " << players[i].player_name << std::endl;
		}
	}
	unlock();
}

FR Room::isin(ClientSock player) {
	lock();
	for (int i = 0; i < MAX_ROOM_PLAYER; i++) {
		if (player.socket == players[i].socket) {
			unlock();
			return FR::SUCCESS;
		}
	}
	unlock();
	return FR::FAIL;

}

FR Room::isin(SOCKET player) {
	lock();
	for (int i = 0; i < MAX_ROOM_PLAYER; i++) {
		if (player == players[i].socket) {
			unlock();
			return FR::SUCCESS;
		}
	}
	unlock();
	return FR::FAIL;

}

void Room::chatToRoom(ClientSock player, char* buf, int buf_size) {
	lock();
	MYCMD cmd;
	for (int i = 0; i < MAX_ROOM_PLAYER; i++) {
		//player != players[i] && 
		if (players[i].socket != NULL && players[i].socket != player.socket) {
			cmd.nCode = CMDCODE::CMD_ROOMCHAT;
			std::cout << "send to " << players[i].socket << "(" << players[i].player_name << ") :" << buf << std::endl;
			// cmd send
			send(players[i].socket, (char*)&cmd, sizeof(cmd), 0);
			// buf send
			send(players[i].socket, buf, buf_size, 0);
		}
	}
	unlock();
}

int Room::GetClientNo() {
	return no++;
}

bool Room::isGameReady() {
	if (no == 2) {
		return true;
	}
	else {
		return false;
	}
}

void Room::setGameState(int code) {
	g_gameState = code;
}

int Room::getGameState() {
	return g_gameState;
}

void Room::resetRoom() {
	g_gameState = -1;
	no = 0;
	g_connection[0] = NOT_CONN;
	g_connection[1] = NOT_CONN;
	//memset(Buffer,0,BUFSIZE * 2);
	//memset(Player, 0, sizeof(Player_Socket) * 2);
	//memset(aArr, 0, sizeof(Bullet_Alive_Arr) * 2);
	//memset(Bullets, 0, sizeof(Bullet_Arr) * 2);
	puts("reset room complete");
}


// Room class 

// Rooms_manager class
Rooms_manager::Rooms_manager() {
	room_no = 0;
	InitializeCriticalSection(&m_room_list_cs);
}
Rooms_manager::~Rooms_manager() {
	std::list<Room*>::iterator it;
	for (it = room_list.begin(); it != room_list.end(); it++) {
		delete* it;
	}
}

void Rooms_manager::lock() {
	puts("room list lockぬ");
	EnterCriticalSection(&m_room_list_cs);
}

void Rooms_manager::unlock() {
	puts("room list unlockで");
	LeaveCriticalSection(&m_room_list_cs);
}

FR Rooms_manager::enter_room(ClientSock player) {
	lock();
	if (room_list.empty()) {
		std::string roomname("Room no. ");
		Room* room = new Room((roomname + std::to_string(room_no++)));
		room->enter_room(player);
		room_list.push_back(room);
		std::cout << "player '" << player.socket << "(" << player.player_name << ")'successfully joined." << std::endl;
		unlock();
		return FR::SUCCESS;
	}
	else {
		std::list<Room*>::iterator it;
		FR success = FR::F_ERROR;
		for (it = room_list.begin(); it != room_list.end(); it++) {
			if ((success = (*it)->enter_room(player)) == FR::SUCCESS) {
				std::cout << "player '" << player.socket << "(" << player.player_name << ")'successfully joined." << std::endl;
				unlock();
				return FR::SUCCESS;
			}
		}
		if (success == FR::FAIL) {
			std::string roomname("Room no. ");
			Room* room = new Room((roomname + std::to_string(room_no++)));
			room->enter_room(player);
			room_list.push_back(room);
			std::cout << "player '" << player.socket << "(" << player.player_name << ")'successfully joined." << std::endl;
			unlock();
			return FR::SUCCESS;
		}
	}
	unlock();
	return FR::F_ERROR;
}

FR Rooms_manager::leave(ClientSock player) {
	lock();
	std::list<Room*>::iterator it;
	Room* temp;
	FR success = FR::F_ERROR;
	for (it = room_list.begin(); it != room_list.end(); it++) {
		success = (*it)->leave(player);
		if (success == FR::SUCCESS) {
			unlock();
			return FR::SUCCESS;
		}
		if (success == FR::SUCCESS_EMPTY) {
			temp = *it;
			room_list.erase(it);
			delete temp;
			unlock();
			return FR::SUCCESS_EMPTY;
		}
	}
	if (success == FR::FAIL) {
		unlock();
		return FR::FAIL;
	}
	return success;
}

void Rooms_manager::display() {
	std::list<Room*>::iterator it;
	lock();
	if (room_list.empty()) {
		std::cout << "empty." << std::endl;
		unlock();
		return;
	}
	std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
	for (it = room_list.begin(); it != room_list.end(); it++) {
		(*it)->display();
	}
	std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
	unlock();
}

void Rooms_manager::chatToRoom(ClientSock player, char* buf, int buf_size) {
	std::list<Room*>::iterator it;
	lock();
	for (it = room_list.begin(); it != room_list.end(); it++) {
		if ((*it)->isin(player) == FR::SUCCESS) {
			(*it)->chatToRoom(player, buf, buf_size);
			puts("chat to room");
			unlock();
			return;
		}
	}
	unlock();
}

Room* Rooms_manager::GetMyRoom(SOCKET socket) {
	std::list<Room*>::iterator it;
	lock();
	for (it = room_list.begin(); it != room_list.end(); it++) {
		if ((*it)->isin(socket) == FR::SUCCESS) {
			puts("get room successed");
			unlock();
			return (*it);
		}
	}
	puts("get room failed");
	unlock();
	return nullptr;
}
// Rooms_manager class