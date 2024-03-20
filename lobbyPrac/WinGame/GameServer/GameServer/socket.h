#pragma once

#include <thread>
#include "../../mylib.h"

typedef struct client_socket {
	SOCKET socket;
	CharacterPosition c_position;
};


class WS_SERVER
{
private:
	WSADATA wsa;
	SOCKET lsock;
	SOCKADDR_IN saddr;
	std::thread t1;
	client_socket s_csock;
	
	
	

public:
	WS_SERVER() { initiate(); }
	~WS_SERVER();
	void stop_server();
	void initiate();
	void Client_accept();

	std::list<client_socket> player_list;

	
};

