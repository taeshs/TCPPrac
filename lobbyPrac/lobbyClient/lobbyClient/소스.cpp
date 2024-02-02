#include <iostream>
#include <WinSock2.h>
#include <thread>
#include "../../mylib/myheader.h"

#pragma comment(lib,"ws2_32.lib")

void Roomthread(SOCKET sock);
void LobbyRecvThread(SOCKET sock);
void RoomRecvThread(SOCKET sock);

void Roomthread(SOCKET sock) {
	std::cout << "Room entered." << std::endl;
	char buf[128];
	MYCMD cmd;
	std::thread th_RoomRecv(RoomRecvThread, sock);
	th_RoomRecv.detach();
	while (true) {
		std::cout << "room thread sender" << std::endl;
		
		// 방 안.
		std::cin >> buf;
		if (strcmp(buf,"LEAVE") == 0) {
			cmd.nCode = CMDCODE::CMD_LEAVEROOM;
			send(sock, (char*)&cmd, sizeof(cmd), 0);
			std::thread t1(LobbyRecvThread, sock);
			t1.detach();
			return;
			// 종료및 기존 쓰레드 되돌아가기
		}
		else {
			cmd.nCode = CMDCODE::CMD_ROOMCHAT;
			send(sock, (char*)&cmd, sizeof(cmd), 0);
		}
		send(sock, buf, sizeof(buf), 0);
		ZeroMemory(buf, sizeof(buf));
	}
}

void RoomRecvThread(SOCKET sock) {
	MYCMD cmd;
	char buf[128] = {};
	while (recv(sock, (char*)&cmd, sizeof(cmd), 0) > 0) {
		std::cout << "room thread recver" << std::endl;
		if (cmd.nCode == CMDCODE::CMD_LEAVEROOM) {
			break;
		}
		recv(sock, buf, sizeof(buf), 0);
		if (cmd.nCode == CMDCODE::CMD_ROOMCHAT) {
			std::cout << "from another client in room : ";
			std::cout << buf << std::endl;
			ZeroMemory(buf, sizeof(buf));
		}
	}
}

void LobbyRecvThread(SOCKET sock) {
	MYCMD cmd;
	char buf[128] = {};
	while(recv(sock, (char*)&cmd, sizeof(cmd), 0) > 0) {
		std::cout << "lobby thread reciever" << std::endl;
		if (cmd.nCode == CMDCODE::CMD_ENTERROOM) {
			break;
		}
		recv(sock, buf, sizeof(buf), 0);
		switch (cmd.nCode) {
		case CMDCODE::CMD_CHAT:
			std::cout << "from another client : ";
			break;
		case CMDCODE::CMD_ECHO:
			std::cout << "ECHO from SERVER : ";
			break;
		}
		std::cout << buf << std::endl;
		ZeroMemory(buf, sizeof(buf));
	}
}

int main() {
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { // 0 OR Errorcodes
		std::cout << "socket start error" << std::endl;
		return 0;
	}

	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET) {
		std::cout << "socket init error" << std::endl;
		return 0;
	}
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8888);
	addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	
	if (connect(sock, (SOCKADDR*)&addr, sizeof(addr))) { // 0 OR SOCKET_ERROR ( WSAGetLastError() )
		std::cout << "socket connection error" << std::endl;
		return 0;
	}
	std::cout << "server connected." << std::endl;

	char buf[128] = {};

	std::thread t1(LobbyRecvThread, sock);
	t1.detach();

	MYCMD cmd;
	while(true) {
		puts("lobby thread sender");
		std::cin >> buf;
		if (strcmp(buf, "EXIT") == 0) {
			break;
		}
		else if (strcmp(buf, "ECHO") == 0) {
			std::cout << "ECHO 할 문자 입력 : ";
			std::cin >> buf;
			cmd.nCode = CMDCODE::CMD_ECHO;
			send(sock, (char*)&cmd, sizeof(cmd), 0);
			send(sock, buf, sizeof(buf), 0);
		}
		else if (strcmp(buf, "ENTER") == 0) {
			std::cout << "방에 진입합니다..." << std::endl;
			cmd.nCode = CMDCODE::CMD_ENTERROOM;
			send(sock, (char*)&cmd, sizeof(cmd), 0);
			std::thread room_thread(Roomthread, sock);
			room_thread.join();
		}
		else {
			std::cout << "chat 보냄" << std::endl;
			cmd.nCode = CMDCODE::CMD_CHAT;
			send(sock, (char*)&cmd, sizeof(cmd), 0);
			send(sock, buf, sizeof(buf), 0);
		}
		memset(buf, 0, sizeof(buf));

		//std::cout << buf << std::endl;
	}
	shutdown(sock, SD_BOTH);
	closesocket(sock);


	WSACleanup();
}