#include <iostream>
#include <WinSock2.h>
#include <thread>
#include <list>
#include "../../mylib/myheader.h"

#pragma comment(lib,"ws2_32.lib")

// Multithread echo -> Multithread chatting

// 접속한 클라이언트들 저장할 list 
std::list<SOCKET> tlist;

CRITICAL_SECTION cs;

void MessageSenderAll(char* buf, int size, int sockid) {
	std::list<SOCKET>::iterator it;
	MYCMD cmd;
	cmd.nCode = CMD_CHAT;
	EnterCriticalSection(&cs);
	for (it = tlist.begin(); it != tlist.end(); it++) {
		if (*it != sockid) {
			send(*it, (char*)&cmd, sizeof(cmd), 0);
			send(*it, buf, size, 0); // 본인한텐 안보내게
		}
		//std::cout << "id" << *it << std::endl;
	}
	LeaveCriticalSection(&cs);
}

 
void threadFunc(SOCKET sock) {
	char buf[128];
	std::cout << "new client connected." << std::endl;
	MYCMD cmd;

	while (recv(sock, (char*)&cmd, sizeof(cmd), 0) > 0) {
		switch (cmd.nCode) {
		case CMD_ECHO:
			recv(sock, buf, sizeof(buf), 0);
			std::cout <<"ECHO : "<< buf << std::endl;
			send(sock, (char*)&cmd, sizeof(cmd), 0);
			send(sock, buf, sizeof(buf), 0);
			break;
		case CMD_CHAT:
			recv(sock, buf, sizeof(buf), 0);
			std::cout << "CHAT : " << buf << std::endl;
			MessageSenderAll(buf, sizeof(buf), sock);
			break;
		}
		memset(buf, 0, sizeof(buf));
	}
	std::cout << "client disconnected." << std::endl;

	closesocket(sock);
}

int main() {
	InitializeCriticalSection(&cs);
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
	
	while ((csock = accept(lsock, (SOCKADDR*)&caddr, &csize))) {
		std::thread t1(
			threadFunc, csock
		);
		std::cout << "id " << csock << " connected" << std::endl;
		EnterCriticalSection(&cs);
		tlist.push_back(csock);
		LeaveCriticalSection(&cs);
		t1.detach();
	}	

	
	closesocket(lsock);

	WSACleanup();
}