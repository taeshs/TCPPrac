
#include "Rooms.h"
#include "db.h"
#include "NGP/ngp.h"

#pragma comment(lib,"ws2_32.lib")

// Multithread echo -> Multithread chatting

// 접속한 클라이언트들 저장할 list 
std::list<ClientSock> g_cli_list;

CRITICAL_SECTION g_c_cs;

Rooms_manager g_rm;

DB g_db;
int g_gameStatus = -1;

void GameServerThread(ClientSock player);

void g_Lock() { // client list lock
	EnterCriticalSection(&g_c_cs);
}
void g_Unlock() {
	LeaveCriticalSection(&g_c_cs);
}

///////
/// db class 내부에서 다시 구현할것. (테스트용)
bool login(PlayerLoginData a, PlayerLoginData b) {
	if (strcmp(a.player_name, b.player_name) == 0) {
		if (strcmp(a.password, b.password) == 0) {
			return true;
		}
	}
	return false;
}

void custom_list_remove(SOCKET sock) {
	std::list<ClientSock>::iterator it;
	for (it = g_cli_list.begin(); it != g_cli_list.end(); it++) {
		if ((*it).socket == sock) {
			g_cli_list.erase(it);
			return;
		}
	}
}

void cli_list_display() {
	std::cout << "list display" << std::endl;
	std::list<ClientSock>::iterator it;
	std::cout << "--- " << g_cli_list.size() << "players. ---" << std::endl;
	for (it = g_cli_list.begin(); it != g_cli_list.end(); it++) {
		std::cout  << "name : " << (*it).player_name << " / id : " << (*it).socket << std::endl;
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
			send((*it).socket, buf, size, 0); // 본인한텐 안보내게
		}
		//std::cout << "id" << *it << std::endl;
	}
	LeaveCriticalSection(&g_c_cs);
}

void RoomManagerThread() {
	// 방 리스트 정보 출력용
	std::cout << "press any key to see all Room, or q to exit." << std::endl;

	while (true) {
		char c = _getch();
		if (c == 'q' || c == 'Q') {
			break;
		}
		
		g_rm.display();
		cli_list_display();
	}
}
// 방 들옴.
void RoomThread(ClientSock sock) {
	std::cout << "client '" << sock.socket << "' joined." << std::endl;
	
	char buf[128];
	MYCMD cmd;
	int score = g_db.getScore((SQLCHAR*)sock.player_name);
	cmd.nCode = CMDCODE::CMD_ENTERROOM;
	send(sock.socket, (char*)&cmd, sizeof(cmd), 0); // TO CLI ROOM RECVER

	g_rm.enter_room(sock);
	std::thread gameserverthread(GameServerThread, sock);
	gameserverthread.join();
	while (recv(sock.socket, (char*)&cmd, sizeof(cmd), 0) > 0) { // FROM CLI ROOM SENDER
		if (cmd.nCode == CMDCODE::CMD_LEAVEROOM) { // g_gameStatus 로 나가기 하기. Working
			g_Lock();
			g_cli_list.push_back(sock);
			g_Unlock();
			g_rm.leave(sock);
			g_db.setScore((SQLCHAR*)sock.player_name, score);
			return;
		}
	}


	//while(recv(sock.socket, (char*)&cmd, sizeof(cmd), 0) > 0) { // FROM CLI ROOM SENDER
	//	std::cout<<score++<<std::endl;
	//	std::cout << "room thread" << std::endl;
	//	if (cmd.nCode == CMDCODE::CMD_LEAVEROOM) { // g_gameStatus 로 나가기 하기. Working
	//		g_Lock();
	//		g_cli_list.push_back(sock);
	//		g_Unlock();
	//		g_rm.leave(sock);
	//		g_db.setScore((SQLCHAR*)sock.player_name, --score);
	//		return;
	//	}
	//	if (cmd.nCode != CMDCODE::CMD_ROOMCHAT) {
	//		puts("error");
	//	}
	//	recv(sock.socket, buf, sizeof(buf), 0);
	//	g_rm.chatToRoom(sock, buf, sizeof(buf));
	//	std::cout << "room chat from cli : " << buf << std::endl;
	//	ZeroMemory(buf, sizeof(buf));
	//}
	/*g_db.setScore((SQLCHAR*)sock.player_name, score);*/
}

 
void threadFunc(ClientSock sock) {
	char buf[128];
	std::cout << "new client connected." << std::endl;
	MYCMD cmd;
	ClientSock temp;
	int score = g_db.getScore((SQLCHAR*)sock.player_name);

	while (recv(sock.socket, (char*)&cmd, sizeof(cmd), 0) > 0) {
		std::cout << score++ << std::endl;
		std::cout << "lobby thread" << std::endl;
		switch (cmd.nCode) {
		case CMDCODE::CMD_ECHO:
			recv(sock.socket, buf, sizeof(buf), 0);
			std::cout << "ECHO : " << buf << std::endl;
			send(sock.socket, (char*)&cmd, sizeof(cmd), 0);
			send(sock.socket, buf, sizeof(buf), 0);
			break;
		case CMDCODE::CMD_CHAT:
			recv(sock.socket, buf, sizeof(buf), 0);
			std::cout << "CHAT : " << buf << std::endl;
			MessageSenderAll(buf, sizeof(buf), sock.socket);
			break;
		case CMDCODE::CMD_ENTERROOM: {
			score--;
			std::cout << "ENTERING ROOM.." << std::endl;
			g_Lock();
			custom_list_remove(sock.socket); /// custom
			g_Unlock();
			std::thread t1(RoomThread, sock);
			g_db.setScore((SQLCHAR*)sock.player_name, (SQLINTEGER)score);
			t1.join();
			score = g_db.getScore((SQLCHAR*)sock.player_name);
			puts("comeback");
			break;
		}
		default:
			puts("something gone wrong....");
		}
		memset(buf, 0, sizeof(buf));
		cmd.nCode = CMDCODE::CMD_NULL;
	}
	g_db.setScore((SQLCHAR*)sock.player_name,(SQLINTEGER)score);
	std::cout << "client disconnected." << std::endl;

	custom_list_remove(sock.socket);
	closesocket(sock.socket);
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
	ClientSock cs_sock;

	PlayerLoginData recvLoginData;

	int loginresult;
	
	while ((csock = accept(lsock, (SOCKADDR*)&caddr, &csize))) {
		
		////// login
		recv(csock, (char*)&firstCmd, sizeof(firstCmd), 0);
		if (firstCmd.nCode == CMDCODE::CMD_CONNECT) {
			recv(csock, (char*)&recvLoginData, sizeof(PlayerLoginData), 0);
			loginresult = g_db.Login((SQLCHAR*)recvLoginData.player_name, recvLoginData.password);
			if (loginresult == LOGIN_SUCCESS) {
				firstCmd.nCode = CMDCODE::CMD_LOGIN_ACCEPT;
				std::cout << "LOGIN : login successed." << std::endl;
			}
			else if (loginresult == LOGIN_FAIL) {
				firstCmd.nCode = CMDCODE::CMD_LOGIN_FAIL;
				std::cout << "LOGIN : login failed." << std::endl;
			}
			else if (loginresult == LOGIN_CREATE_NEW) {
				firstCmd.nCode = CMDCODE::CMD_LOGIN_ACCEPT;
				std::cout << "LOGIN : create new id." << std::endl;
			}
			else {
				std::cout <<"--------------" << loginresult << std::endl;
			}

			
			// accept or fail 예시 사용시 동작함. -> 이제 DB 연동해서 있으면 로그인 OR 없으면 등록으로 진행.
			
			send(csock, (char*)&firstCmd, sizeof(firstCmd), 0);
		}
		//////

		
		g_Lock();
		cs_sock.socket = csock;
		strcpy(cs_sock.player_name, recvLoginData.player_name);
		g_cli_list.push_back(cs_sock);
		g_Unlock();

		std::cout << "hi, " << recvLoginData.player_name << ". id " << csock << " connected" << std::endl;
		std::thread t1(
			threadFunc, cs_sock
		);
		
		

		t1.detach();
	}	

	th_rm.join();
	
	closesocket(lsock);

	WSACleanup();
}


#define BUFSIZE    512000

// 전역 변수만 룸으로 넘겨버리면 될듯
// 룸으로 전역변수들 넘기고 roommanager 에서 getmyroom(sock) 이런식으로 만들어서 room 가져와서 사용하자.
// 이 변수는 게임 종료 판단해서 초기화 하면 될듯.
// working


void GameServerThread(ClientSock playersocket) {
	SOCKET sock = playersocket.socket;
	Room* myroom = g_rm.GetMyRoom(sock);
	puts("gs_thread entered");

	myroom->lock();
	int m_no = myroom->GetClientNo();
	myroom->unlock();
	int enemy_no = m_no == 0 ? 1 : 0;

	SOCKET client_sock = sock;
	int retval;
	SOCKADDR_IN clientaddr;
	int addrlen;



	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (SOCKADDR*)&clientaddr, &addrlen);

	printf("%d번째 클라이언트 입니다", m_no + 1);

	/*if (no >= 3) {
		closesocket(client_sock);
		printf("클라이언트 종료: IP 주소=%s, 포트 번호=%d [인원 초과]\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
		return;
	}*/

	retval = send(client_sock, (char*)&m_no, sizeof(m_no), 0);
	if (retval == SOCKET_ERROR) {
		err_display("recv()");
		closesocket(client_sock);
	}

	int temp = 2;
	while (true) {
		if (myroom->isGameReady()) {
			retval = send(client_sock, (char*)&temp, sizeof(temp), 0);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				closesocket(client_sock);
			}
			printf("-> 클라 아이디 (번호): %d\n", temp);
			break;
		}
		//printf("-> 클라 아이디 (번호): %d\n", no);
	}


	myroom->g_connection[m_no] = CONN_OKAY;

	int seconds = 0;
	while (1) {
		int retval;
		int GetSize;




		Player_Socket* player;
		GetSize = recv(client_sock, myroom->Buffer[m_no], sizeof(Player_Socket), 0);
		if (GetSize == SOCKET_ERROR) {
			MessageBox(NULL, L"연결이 끊어졌습니다", L"error", 0);
			myroom->g_connection[m_no] = CONN_FAIL;
			//exit(1);
			break;
		}

		if (myroom->g_connection[enemy_no] == CONN_FAIL) {
			MessageBox(NULL, L"상대 연결이 끊어졌습니다", L"error", 0);
			myroom->g_connection[m_no] = CONN_FAIL;
			//exit(1);
			break;
		}

		myroom->Buffer[m_no][GetSize] = '\0';
		player = (Player_Socket*)myroom->Buffer[m_no];




		myroom->Player[m_no].posX = player->posX;
		myroom->Player[m_no].posY = player->posY;
		myroom->Player[m_no].hp = player->hp;
		myroom->Player[m_no].bb = player->bb;



		myroom->Bullets[m_no] = recv_Bullet(client_sock);

		for (int i = 0; i < 10; i++) {
			myroom->aArr[enemy_no].arr[i] = 0;
		}

		int asd;
		for (int i = 0; i < 10; i++) {
			if (myroom->Bullets[enemy_no].arr[i].alive) {
				if (collisionCheck(myroom->Player[m_no].bb, myroom->Bullets[enemy_no].arr[i].GetBB())) {
					myroom->Bullets[enemy_no].arr[i].alive = false;
					myroom->aArr[enemy_no].arr[i] = 1;
				}
			}
		}

		send_Player(client_sock, myroom->Player[enemy_no]);
		send_Bullet(client_sock, myroom->Bullets[enemy_no]);

		send_Player(client_sock, myroom->Player[m_no]);

		send_Bullet_Alive(client_sock, myroom->aArr[enemy_no]);
		send_Bullet_Alive(client_sock, myroom->aArr[m_no]);



		if (myroom->Player[0].hp <= 0) {
			myroom->setGameState(1);
		}
		else if (myroom->Player[1].hp <= 0) {
			myroom->setGameState(2);
		}
		else if (myroom->Player[0].hp <= 0 && myroom->Player[1].hp <= 0) {
			myroom->setGameState(3);
		}
		else {
			myroom->setGameState(0);
		}

		int gamestate = myroom->getGameState();
		retval = send(client_sock, (char*)&gamestate, sizeof(gamestate), 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			closesocket(client_sock);
		}
		if (seconds++ == 600) {
			seconds = 0;
			//std::cout << gamestate << std::endl;
		}
		if (gamestate != 0) {
			MYCMD cmd;
			recv(playersocket.socket, (char*)&cmd, sizeof(cmd), 0);
			if (cmd.nCode == CMDCODE::CMD_LEAVEROOM) {
				break;
			}
		}
	}
	g_rm.leave(playersocket);
	int score = g_db.getScore((SQLCHAR*)playersocket.player_name);
	g_db.setScore((SQLCHAR*)playersocket.player_name, score+1);
	myroom->resetRoom();
	closesocket(playersocket.socket);
	/*printf("스레드 종료 : %d\n[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n\n", m_no,
		inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));*/
	return;
}
// 메인 스레드에서 클라이언트 억셉트 마다 스레드를 생성. 수신 내용을 전체 클라이언트 리스트에 전송.

// 클래스 정리. 클라이언트 입장 시 이름을 받아서 cmd 패킷에서 같이 전송.
// db 연동에 사용