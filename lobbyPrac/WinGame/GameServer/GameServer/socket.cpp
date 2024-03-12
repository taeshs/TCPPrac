#include "socket.h"

void WS_SERVER::initiate() {
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	SOCKET lsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = ntohs(8888);
	saddr.sin_addr.S_un.S_addr = ntohl(INADDR_ANY);

	bind(lsock, (SOCKADDR*)&saddr, sizeof(saddr));

	SOCKET csock;
	SOCKADDR_IN caddr;
	int caddr_size = sizeof(caddr);
}

void WS_SERVER::Client_accept() {
	while (csock = accept(lsock, (SOCKADDR*)&caddr, &caddr_size)) {
		std::thread t1([]() {});
		t1.detach();
	}
}