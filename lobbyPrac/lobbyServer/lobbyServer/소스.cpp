#include <iostream>
#include <Winsock2.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
	WSADATA wsa = {0};
	WSAStartup(MAKEWORD(2, 2), &wsa);

	SOCKET lsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8888);
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	bind(lsock, (SOCKADDR*)&addr, sizeof(addr));

	listen(lsock, SOMAXCONN);

	SOCKADDR_IN caddr = { 0 };
	int csize = sizeof(caddr);
	SOCKET csock = accept(lsock, (SOCKADDR*)&caddr, &csize);

	char buf[128] = "hi from server";
	send(csock, buf, sizeof(buf), 0);

	memset(buf, 0, sizeof(buf));

	recv(csock, buf, sizeof(buf), 0);

	std::cout << buf << std::endl;

	closesocket(csock);
	closesocket(lsock);

	WSACleanup();
}