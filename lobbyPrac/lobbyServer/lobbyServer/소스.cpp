#include <iostream>
#include <WinSock2.h>
#include <thread>
#include <list>
#include <string>
#include <conio.h>
#include "../../mylib/myheader.h"

#pragma comment(lib,"ws2_32.lib")

// Multithread echo -> Multithread chatting

// ������ Ŭ���̾�Ʈ�� ������ list 
std::list<SOCKET> g_cli_list;

CRITICAL_SECTION g_c_cs;

void g_Lock() {
	EnterCriticalSection(&g_c_cs);
}

void g_Unlock() {
	LeaveCriticalSection(&g_c_cs);
}



class Room {
private:
	SOCKET players[3];
	int num_player;
	std::string name;
	CRITICAL_SECTION m_room_cs;

	void lock() {
		puts("room lock��");
		EnterCriticalSection(&m_room_cs);
	}

	void unlock() {
		puts("room unlock��");
		LeaveCriticalSection(&m_room_cs);
	}

public:
	Room(std::string n) : name{ n }, players{}, num_player{ 0 } {
		std::cout << "created new room." << std::endl;
		InitializeCriticalSection(&m_room_cs); 
	}

	FR enter_room(SOCKET player) {
		lock();
		if (num_player == MAX_ROOM_PLAYER) {
			unlock();
			return FR::FAIL;
		}
		else {
			for (int i = 0; i < MAX_ROOM_PLAYER; i++) {
				if (players[i] == NULL) {
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

	FR leave(SOCKET player) {
		lock();
		for (int i = 0; i < MAX_ROOM_PLAYER; i++) {
			if (players[i] == player) {
				players[i] = NULL;
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

	void display() {
		lock();
		std::cout << "Room name : " << name << " / nums of player : " << num_player << std::endl;;
		for (int i = 0; i < MAX_ROOM_PLAYER; i++) {
			if (players[i] == NULL) {
				std::cout << "EMPTY" << std::endl;
			}
			else {
				std::cout << "player '" << players[i] << "'" << std::endl;
			}
		}
		unlock();
	}

	FR isin(SOCKET player) {
		lock();
		for (int i = 0; i < MAX_ROOM_PLAYER; i++) {
			if (player == players[i]) {
				unlock();
				return FR::SUCCESS;
			}
		}
		unlock();
		return FR::FAIL;
		
	}

	void chatToRoom(SOCKET player, char* buf, int buf_size) {
		lock();
		MYCMD cmd;
		for (int i = 0; i < MAX_ROOM_PLAYER; i++) {
			//player != players[i] && 
			if (players[i] != NULL) {
				cmd.nCode = CMDCODE::CMD_ROOMCHAT;
				std::cout << "send to " << players[i] << " :" << buf << std::endl;
				// cmd send
				send(players[i], (char*)&cmd, sizeof(cmd), 0);
				// buf send
				send(players[i], buf, buf_size, 0);
			}
		}
		unlock();
	}
};

class Rooms_manager {
private:
	int room_no;
	std::list<Room*> room_list;
	CRITICAL_SECTION m_room_list_cs;

	void lock() {
		puts("room list lock��");
		EnterCriticalSection(&m_room_list_cs);
	}

	void unlock() {
		puts("room list unlock��");
		LeaveCriticalSection(&m_room_list_cs);
	}

public:
	Rooms_manager(){
		room_no = 0;
		InitializeCriticalSection(&m_room_list_cs);
	}
	~Rooms_manager() {
		std::list<Room*>::iterator it;
		for (it = room_list.begin(); it != room_list.end(); it++) {
			delete *it;
		}
	}

	FR enter_room(SOCKET player) {
		lock();
		if (room_list.empty()) {
			std::string roomname("Room no. ");
			Room* room = new Room( ( roomname + std::to_string( room_no++ ) ) );
			room->enter_room(player);
			room_list.push_back(room);
			std::cout << "player '" << player << "'successfully joined." << std::endl;
			unlock();
			return FR::SUCCESS;
		}
		else {
			std::list<Room*>::iterator it;
			FR success = FR::F_ERROR;
			for (it = room_list.begin(); it != room_list.end(); it++) {
				if ( (success = (*it)->enter_room(player)) == FR::SUCCESS) {
					std::cout << "player '" << player << "'successfully joined." << std::endl;
					unlock();
					return FR::SUCCESS;
				}
			}
			if (success == FR::FAIL) {
				std::string roomname("Room no. ");
				Room* room = new Room((roomname + std::to_string(room_no++)));
				room->enter_room(player);
				room_list.push_back(room);
				std::cout << "player '" << player << "'successfully joined." << std::endl;
				unlock();
				return FR::SUCCESS;
			}
		}
		unlock();
		return FR::F_ERROR;
	}

	FR leave(SOCKET player) {
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

	void display() {
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

	void chatToRoom(SOCKET player, char* buf, int buf_size) {
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
};


// ������ ��û & ���� ����ü


void MessageSenderAll(char* buf, int size, int sockid) {
	std::list<SOCKET>::iterator it;
	MYCMD cmd;
	cmd.nCode = CMDCODE::CMD_CHAT;
	EnterCriticalSection(&g_c_cs);
	for (it = g_cli_list.begin(); it != g_cli_list.end(); it++) {
		if (*it != sockid) {
			send(*it, (char*)&cmd, sizeof(cmd), 0);
			send(*it, buf, size, 0); // �������� �Ⱥ�����
		}
		//std::cout << "id" << *it << std::endl;
	}
	LeaveCriticalSection(&g_c_cs);
}

Rooms_manager g_rm;

void RoomManagerThread() {
	std::cout << "press any key to see all Room, or q to exit." << std::endl;

	while (true) {
		char c = _getch();
		if (c == 'q' || c == 'Q') {
			break;
		}
		
		g_rm.display();
	}
}

// �� ���.
void RoomThread(SOCKET sock) {
	std::cout << "client '" << sock << "' joined." << std::endl;
	char buf[128];
	MYCMD cmd;
	cmd.nCode = CMDCODE::CMD_ENTERROOM;
	send(sock, (char*)&cmd, sizeof(cmd), 0);

	g_rm.enter_room(sock);
	while(recv(sock, (char*)&cmd, sizeof(cmd), 0) > 0) {
		std::cout << "room thread" << std::endl;
		if (cmd.nCode == CMDCODE::CMD_LEAVEROOM) {
			g_Lock();
			g_cli_list.push_back(sock);
			g_Unlock();
			g_rm.leave(sock);
			
			return;
		}
		// �Լ��� ������. �� ���� ä��
		if (cmd.nCode != CMDCODE::CMD_ROOMCHAT) {
			puts("error");
		}
		recv(sock, buf, sizeof(buf), 0);
		// �� ��. leave room �ϱ�
		g_rm.chatToRoom(sock, buf, sizeof(buf));
		std::cout << "room chat from cli : " << buf << std::endl;
		ZeroMemory(buf, sizeof(buf));
	}
}

 
void threadFunc(SOCKET sock) {
	char buf[128];
	std::cout << "new client connected." << std::endl;
	MYCMD cmd;

	while (recv(sock, (char*)&cmd, sizeof(cmd), 0) > 0) {
		std::cout << "lobby thread" << std::endl;
		switch (cmd.nCode) {
		case CMDCODE::CMD_ECHO:
			recv(sock, buf, sizeof(buf), 0);
			std::cout << "ECHO : " << buf << std::endl;
			send(sock, (char*)&cmd, sizeof(cmd), 0);
			send(sock, buf, sizeof(buf), 0);
			break;
		case CMDCODE::CMD_CHAT:
			recv(sock, buf, sizeof(buf), 0);
			std::cout << "CHAT : " << buf << std::endl;
			MessageSenderAll(buf, sizeof(buf), sock);
			break;
		case CMDCODE::CMD_ENTERROOM: {
			std::cout << "ENTERING ROOM.." << std::endl;
			g_Lock();
			g_cli_list.remove(sock);
			g_Unlock();
			std::thread t1(RoomThread, sock);
			t1.join();
			puts("comeback");
			break;
		}
		default:
			puts("something gone wrong....");
		}
		memset(buf, 0, sizeof(buf));
		cmd.nCode = CMDCODE::CMD_NULL;
	}
	std::cout << "client disconnected." << std::endl;

	closesocket(sock);
}

int main() {
	InitializeCriticalSection(&g_c_cs);
	WSADATA wsa = {0};
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { // 0 OR Errorcodes
		std::cout << "socket start error" << std::endl;
		return 0;
	}

	SOCKET lsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (lsock == INVALID_SOCKET) {
		std::cout << "socket init error" << std::endl;
		return 0;
	}

	SOCKADDR_IN addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8888);
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	if (bind(lsock, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) { // 0 OR SOCKET_ERROR ( WSAGetLastError() )
		std::cout << "socket bind error" << std::endl;
		return 0;
	}

	if(listen(lsock, SOMAXCONN)){ // 0 OR SOCKET_ERROR ( WSAGetLastError() )
		std::cout << "socket listen error" << std::endl;
		return 0;
	}

	SOCKADDR_IN caddr = { 0 };
	int csize = sizeof(caddr);
	SOCKET csock;

	std::thread th_rm(RoomManagerThread);
	
	while ((csock = accept(lsock, (SOCKADDR*)&caddr, &csize))) {
		std::thread t1(
			threadFunc, csock
		);
		std::cout << "id " << csock << " connected" << std::endl;
		EnterCriticalSection(&g_c_cs);
		g_cli_list.push_back(csock);
		LeaveCriticalSection(&g_c_cs);
		t1.detach();
	}	

	th_rm.join();
	
	closesocket(lsock);

	WSACleanup();
}

// ���� �����忡�� Ŭ���̾�Ʈ ���Ʈ ���� �����带 ����. ���� ������ ��ü Ŭ���̾�Ʈ ����Ʈ�� ����.
// ���� �����忡�� ���� �� ���� ��� ���� ��