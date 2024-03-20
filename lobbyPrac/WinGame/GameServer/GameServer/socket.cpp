#include "socket.h"


void ThreadFunc(WS_SERVER* inst, client_socket csock);
void custom_remove(SOCKET sock, WS_SERVER* inst);

void WS_SERVER::initiate() {
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	lsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN saddr = { 0 };
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(8887);
	saddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	int option = TRUE;               //네이글 알고리즘 on/off
	setsockopt(lsock,             //해당 소켓
		IPPROTO_TCP,          //소켓의 레벨
		TCP_NODELAY,          //설정 옵션
		(const char*)&option, // 옵션 포인터
		sizeof(option));

	if (bind(lsock, (SOCKADDR*)&saddr, sizeof(saddr)) == SOCKET_ERROR) {
		exit(0);
	}
	std::cout << "bind" << std::endl;

	


	if (listen(lsock, SOMAXCONN) == SOCKET_ERROR) {
		exit(0);
	}
	std::cout << "listen" << std::endl;
}

void WS_SERVER::Client_accept() {
	SOCKADDR_IN caddr = { 0 };
	int caddr_size = sizeof(caddr);
	SOCKET csock = 0;

	CharacterPosition temp; 
	temp.x = 100; temp.y = 100;
	
	std::cout << "accept 대기" << std::endl;
	while ((csock = accept(lsock, (SOCKADDR*)&caddr, &caddr_size)) != INVALID_SOCKET) {
		std::cout << "accepted" << std::endl;
		s_csock.socket = csock;
		s_csock.c_position = temp;
		player_list.push_back(s_csock);
		std::cout << "클라이언트 " << csock << "입장." << std::endl;
		t1 = std::thread(&ThreadFunc, this, s_csock);
		t1.detach();
		
	}
}


WS_SERVER::~WS_SERVER() {
	stop_server();
}

void WS_SERVER::stop_server() {
	closesocket(lsock);
	WSACleanup();
}

void ThreadFunc(WS_SERVER* inst, client_socket csock) {
	PacketInfo info;
	int i = 0;
	int asd = 0; // 새로 들어왔는지
	while (recv(csock.socket, (char*)&info, sizeof(PacketInfo), 0) > 0) {
		//puts("somethin come");
		switch (info.type) {
		case TYPE_MESSAGE:
		{
			//puts("its message");
			std::list<client_socket>::iterator it;
			Packet_Message message;
			recv(csock.socket, (char*)&message, sizeof(Packet_Message), 0);

			for (it = inst->player_list.begin(); it != inst->player_list.end(); it++) {
				if ((*it).socket != csock.socket) {
					info.size = sizeof(Packet_Message);
					info.type = TYPE_MESSAGE;
					puts(message.buf);
					std::cout << "from" << csock.socket << "sendto" << (*it).socket << std::endl;
					send((*it).socket, (char*)&info, sizeof(PacketInfo), 0);
					send((*it).socket, (char*)&message, sizeof(Packet_Message), 0);
				}
			}
			// 클라목록에 본인제외 메세지 뿌리기.
		}
		break;
		case TYPE_SEND_CHARACTER_POSITION:
		{
			std::list<client_socket>::iterator it;
			asd = 0;
			CharacterPosition cp;
			recv(csock.socket, (char*)&cp, sizeof(CharacterPosition), 0);
			for (it = inst->player_list.begin(); it != inst->player_list.end();) {
				if ((*it).socket == csock.socket) {
					(*it).c_position = cp;
					asd = 1;
					break;
				}
				else {
					it++;
				}
			}
			if (asd == 0) {
				csock.c_position = cp;
				inst->player_list.push_back(csock);
			}

			CharacterPosition* cps=0;
			
			CharacterPosition tempc;
			if (inst->player_list.size() > 1) {
				
				cps = new CharacterPosition[inst->player_list.size()];
				i = 0;
				for (it = inst->player_list.begin(); it != inst->player_list.end(); it++) {
					if ((*it).socket 
						!=
						csock.socket) {
						cps[i] = (*it).c_position;
						i++;

					}
				}
				info.size = sizeof(CharacterPosition) * i;
				info.type = TYPE_RECV_CHARACTER_POSITIONS;
				info.player_cnt = i;
				
			}
			else {
				info.size = 0;
				info.type = TYPE_RECV_CHARACTER_POSITIONS;
				info.player_cnt = 0;
			}
			
			
			send(csock.socket, (char*)&info, sizeof(PacketInfo), 0);				// cmd
			if (inst->player_list.size() > 1) {
				
				send(csock.socket, (char*)cps, (sizeof(CharacterPosition) * i), 0);		// data
				delete[] cps;
			}

		}
		break;
		case TYPE_HEARTBEAT:
		{
			//send(csock.socket, (char*)&info, sizeof(PacketInfo), 0);
		}
		break;
		}
	}
	custom_remove(csock.socket, inst);
	closesocket(csock.socket);
}

void custom_remove(SOCKET sock, WS_SERVER* inst) {
	std::list<client_socket>::iterator it;
	for (it = inst->player_list.begin(); it != inst->player_list.end();) {
		if ((*it).socket == sock) {
			inst->player_list.erase(it);
			return;
		}
		else {
			it++;
		}
	}
}

// TLQKF 다시 하든가 다 뺴던가 개같네