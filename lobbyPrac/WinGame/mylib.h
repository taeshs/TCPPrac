#pragma once

#include <WinSock2.h>
#include <list>
#include <vector>
#include <string>
#include <iostream>

#pragma comment(lib,"ws2_32.lib")

typedef struct CharacterPosition {
	float x;
	float y;
}POS, CharPos, Position2d;

typedef enum PacketType {
	TYPE_MESSAGE = 1111,
	TYPE_SEND_CHARACTER_POSITION = 2222,
	TYPE_RECV_CHARACTER_POSITIONS = 2233,
	TYPE_CONNECT = 3333,
	TYPE_DISCONNECT = 4444,
	TYPE_HEARTBEAT = 9999,
};

typedef struct PacketInfo {
	PacketType type;
	int size;
	int player_cnt = 0;
};

typedef struct Packet_Message {
	char buf[256];
};

typedef struct Packet_Recv_Character_Positions {
	CharacterPosition* pcp;
	int numofplayer;
};

//typedef struct Packet_Connect {
//	// 필요 시 로그인 데이터
//};
//
//typedef struct Packet_Disconnect {
//
//};