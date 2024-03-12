#include "winsock.h"
#include "Resource.h"
//#include <iostream>

// 소켓 함수 오류 출력 후 종료
void err_quit(const char* msg) {
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}
// 소켓 함수 오류 출력
void err_display(const char* msg) {
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

int recvn(SOCKET s, char* buf, int len, int flags) {
	int received;
	char* ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}

SOCKET init_socket(HINSTANCE hinst, DWORD address) {
	int retval;

	

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
	}
	
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	//Nagle off
	BOOL optval = TRUE;
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, sizeof(optval));


	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(address); //inet_addr(SERVERIP); 
	serveraddr.sin_port = htons(8888);
	retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	return sock;
}


void send_Player(SOCKET sock, Player_Socket player) {
	int retval;

	retval = send(sock, (char*)&player, sizeof(Player_Socket), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		exit(1);
	}

}
char Buffer[BUFSIZE];
Player_Socket recv_Player(SOCKET sock) {
	int retval;
	int buf;
	int GetSize;

	Player_Socket* player;
	GetSize = recv(sock, Buffer, sizeof(Player_Socket), 0);
	if (GetSize <= 0) {
		err_quit("RECV()");
		exit(1);
	}

	Buffer[GetSize] = '\0'; 
	player = (Player_Socket*)Buffer;

	return *player;
}


void send_Bullet(SOCKET sock, Bullet_Arr bullet) {
	int retval;

	retval = send(sock, (char*)&bullet, sizeof(Bullet_Arr), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		exit(1);
	}
}


Bullet_Arr recv_Bullet(SOCKET sock) {
	int retval;
	int GetSize;

	Bullet_Arr bullet;
	GetSize = recv(sock, (char*)&bullet, sizeof(Bullet_Arr), 0);
	if (GetSize <= 0) {
		MessageBox(NULL, "error", "연결이 끊어졌습니다", 0);
		exit(1);
	}

	return bullet;
}


void send_Bullet_Alive(SOCKET sock, Bullet_Alive_Arr bullet_al) {
	int retval;

	retval = send(sock, (char*)&bullet_al, sizeof(Bullet_Alive_Arr), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		exit(1);
	}
}

Bullet_Alive_Arr recv_Bullet_Alive(SOCKET sock) {
	int retval;
	int GetSize;

	Bullet_Alive_Arr bullet_ar;
	GetSize = recv(sock, (char*)&bullet_ar, sizeof(Bullet_Alive_Arr), 0);
	if (GetSize <= 0) {
		MessageBox(NULL, "error", "연결이 끊어졌습니다", 0);
		exit(1);
	}

	return bullet_ar;
}
