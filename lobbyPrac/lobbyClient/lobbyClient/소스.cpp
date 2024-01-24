#include <iostream>
#include <WinSock2.h>
#include <thread>
#include "../../mylib/myheader.h"

#pragma comment(lib,"ws2_32.lib")

void threadFunc(SOCKET sock) {
	MYCMD cmd;
	char buf[128] = {};
	while(recv(sock, (char*)&cmd, sizeof(cmd), 0) > 0) {
		recv(sock, buf, sizeof(buf), 0);
		switch (cmd.nCode) {
		case CMD_CHAT:
			std::cout << "from another client : ";
			break;
		case CMD_ECHO:
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

	std::thread t1(threadFunc, sock);

	MYCMD cmd;
	while(true) {
		std::cin >> buf;
		if (strcmp(buf, "EXIT") == 0) {
			break;
		}
		else if (strcmp(buf, "ECHO") == 0) {
			std::cout << "ECHO 할 문자 입력 : ";
			std::cin >> buf;
			cmd.nCode = CMD_ECHO;
			send(sock, (char*)&cmd, sizeof(cmd), 0);
			send(sock, buf, sizeof(buf), 0);
			memset(buf, 0, sizeof(buf));
		}
		else {
			cmd.nCode = CMD_CHAT;
			send(sock, (char*)&cmd, sizeof(cmd), 0);
			send(sock, buf, sizeof(buf), 0);
			memset(buf, 0, sizeof(buf));
		}
		//std::cout << buf << std::endl;
	}
	shutdown(sock, SD_BOTH);
	closesocket(sock);
	t1.join();

	WSACleanup();
}