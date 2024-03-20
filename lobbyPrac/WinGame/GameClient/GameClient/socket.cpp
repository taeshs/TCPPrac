#include "socket.h"

WS_CLIENT::WS_CLIENT() {
	//initiate();
}

WS_CLIENT::~WS_CLIENT() {
	disconnect();
}

void WS_CLIENT::initiate() {
	WSAStartup(MAKEWORD(2, 2), &wsa);
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8887);
	addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	int option = TRUE;               //네이글 알고리즘 on/off
	setsockopt(sock,             //해당 소켓
		IPPROTO_TCP,          //소켓의 레벨
		TCP_NODELAY,          //설정 옵션
		(const char*)&option, // 옵션 포인터
		sizeof(option));

	connect(sock, (SOCKADDR*)&addr, sizeof(addr));
	puts("init");
}

void WS_CLIENT::sendPlayer(CharacterPosition cp) {
	PacketInfo info;
	info.size = sizeof(CharacterPosition);
	info.type = TYPE_SEND_CHARACTER_POSITION;
	
	send(sock, (char*)&info, sizeof(PacketInfo), 0);

	send(sock, (char*)&cp, sizeof(CharacterPosition), 0);
	//puts("player send.");
}

void WS_CLIENT::sendMessage(char* buf, int bufsize) {
	PacketInfo info;
	info.size = sizeof(Packet_Message);
	info.type = TYPE_MESSAGE;

	Packet_Message message;
	strcpy(message.buf, buf);

	send(sock, (char*)&info, sizeof(PacketInfo), 0);

	send(sock, (char*)&message, sizeof(Packet_Message), 0);
	//puts("message send.");
}

void WS_CLIENT::recieverThread() {
	PacketInfo info;
	
	while (recv(sock, (char*)&info, sizeof(PacketInfo), 0) > 0) {
		other_playercnt = info.player_cnt;
		//printf("%d\n", other_playercnt);
		switch (info.type) {
		case TYPE_MESSAGE:
		{
			Packet_Message message;
			recv(sock, (char*)&message, sizeof(Packet_Message), 0);
			chatlist.push_back(message.buf);
			//puts(message.buf);
			displayChatlist();
		}
		break;
		case TYPE_RECV_CHARACTER_POSITIONS:
		{
			if (info.player_cnt > 0) {
				//puts("come");
				
				delete[] cps;
				cps = new CharacterPosition[info.player_cnt];
				recv(sock, (char*)cps, info.size, 0);
				
				
			}
		}
		break;
		case TYPE_HEARTBEAT:
		{
			//send(sock, (char*)&info, sizeof(PacketInfo), 0);
		}
		break;
		}

	}
	delete[] cps;
}

void WS_CLIENT::disconnect() {
	closesocket(sock);
	WSACleanup();
}

void WS_CLIENT::displayChatlist() {
	std::list<std::string>::iterator it;
	//system("cls");
	for (it = chatlist.begin(); it != chatlist.end(); it++) {
		std::cout << *it << std::endl;
	}
	std::cout << "-----------------------------" << std::endl;
	std::cout << "chat : ";
}