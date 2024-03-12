#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#include "framework.h"
#include "Global.h"

#define SERVERIP   "192.168.63.179"
#define SERVERPORT 8685
#define BUFSIZE    5000
#define MAX_Client 2

#pragma pack(push, 1)
typedef struct Player_Socket
{
	int posX, posY;
	int hp;
	BoundingBox bb;
}Player_Socket;
#pragma pack(pop)


void err_quit(const char* msg);
void err_display(const char* msg);
int recvn(SOCKET s, char* buf, int len, int flags);
SOCKET init_socket(HINSTANCE, DWORD);
void send_Player(SOCKET sock, Player_Socket player);
Player_Socket recv_Player(SOCKET sock);


BOOL CALLBACK IPGETTER_Dlg_Proc(HWND, UINT, WPARAM, LPARAM);

struct Bullet_Arr {
	Bullet arr[10];
};

struct Bullet_Alive_Arr {
	int arr[10];
};

void send_Bullet(SOCKET sock, Bullet_Arr bullet);

Bullet_Arr recv_Bullet(SOCKET sock);

void send_Bullet_Alive(SOCKET sock, Bullet_Alive_Arr bullet);

Bullet_Alive_Arr recv_Bullet_Alive(SOCKET sock);

