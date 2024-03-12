#pragma once
#include <WinSock2.h>
#include <thread>
#pragma comment(lib, "ws2_32.lib")

class WS_SERVER
{
private:
	WSADATA wsa;
	SOCKET lsock;
	SOCKADDR_IN saddr;
	SOCKET csock;
	SOCKADDR_IN caddr;
	int caddr_size = sizeof(caddr);

public:
	WS_SERVER() { initiate(); }
	~WS_SERVER();
	void initiate();
	void Client_accept();
};



