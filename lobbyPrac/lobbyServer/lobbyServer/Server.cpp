
#include "Rooms.h"


#pragma comment(lib,"ws2_32.lib")

// Multithread echo -> Multithread chatting

// ������ Ŭ���̾�Ʈ�� ������ list 
std::list<ClientSock> g_cli_list;

CRITICAL_SECTION g_c_cs;

Rooms_manager g_rm;

void g_Lock() { // client list lock
	EnterCriticalSection(&g_c_cs);
}
void g_Unlock() {
	LeaveCriticalSection(&g_c_cs);
}

void custom_list_remove(ClientSock client) {
	std::list<ClientSock>::iterator it;
	for (it = g_cli_list.begin(); it != g_cli_list.end(); it++) {
		if ((*it).player_name == client.player_name) {
			g_cli_list.erase(it);
			return;
		}
	}
}


void MessageSenderAll(char* buf, int size, int sockid) {
	std::list<ClientSock>::iterator it;
	MYCMD cmd;
	cmd.nCode = CMDCODE::CMD_CHAT;
	EnterCriticalSection(&g_c_cs);
	for (it = g_cli_list.begin(); it != g_cli_list.end(); it++) {
		if ((*it).socket != sockid) {
			send((*it).socket, (char*)&cmd, sizeof(cmd), 0);
			send((*it).socket, buf, size, 0); // �������� �Ⱥ�����
		}
		//std::cout << "id" << *it << std::endl;
	}
	LeaveCriticalSection(&g_c_cs);
}

void RoomManagerThread() {
	// �� ����Ʈ ���� ��¿�
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
			ClientSock temp;
			temp.socket = sock;
			temp.player_name = cmd.player_name;
			g_cli_list.push_back(temp);
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
			ClientSock temp;
			temp.socket = sock;
			temp.player_name = cmd.player_name;
			custom_list_remove(temp); /// custom
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
	MYCMD firstCmd;
	ClientSock temp;
	
	while ((csock = accept(lsock, (SOCKADDR*)&caddr, &csize))) {
		
		recv(csock, (char*)&firstCmd, sizeof(firstCmd), 0);
		if (firstCmd.nCode == CMDCODE::CMD_CONNECT) {
			firstCmd.nCode = CMDCODE::CMD_ACCEPT;
			send(csock, (char*)&firstCmd, sizeof(firstCmd), 0);
		}
		

		
		g_Lock();
		temp.socket = csock;
		temp.player_name = firstCmd.player_name;
		g_cli_list.push_back(temp);
		g_Unlock();

		std::cout << "hi, " << firstCmd.player_name << ". id " << csock << " connected" << std::endl;
		std::thread t1(
			threadFunc, csock
		);
		
		

		t1.detach();
	}	

	th_rm.join();
	
	closesocket(lsock);

	WSACleanup();
}

// ���� �����忡�� Ŭ���̾�Ʈ ���Ʈ ���� �����带 ����. ���� ������ ��ü Ŭ���̾�Ʈ ����Ʈ�� ����.

// Ŭ���� ����. Ŭ���̾�Ʈ ���� �� �̸��� �޾Ƽ� cmd ��Ŷ���� ���� ����.
// db ������ ���