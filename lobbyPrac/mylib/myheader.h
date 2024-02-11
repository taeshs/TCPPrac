#include <iostream>
#include <WinSock2.h>
#include <thread>
#include <list>
#include <string>
#include <conio.h>

typedef enum class CMDCODE {
	CMD_CONNECT = 9999,
	CMD_ACCEPT = 9998,
	CMD_NULL = 0,
	CMD_ECHO = 100,
	CMD_CHAT = 200,
	CMD_ENTERROOM = 1000,
	CMD_LEAVEROOM,
	CMD_ROOMCHAT = 2020,
}CMDCODE;

typedef struct MYCMD {
	CMDCODE nCode;
	std::string player_name;
}MYCMD;

typedef struct ClientSock {
	SOCKET socket;
	std::string player_name;
};

// MYCMD에 플레이어 네임?

typedef enum class THREADSTATUS {
	TH_NO_ROOM = 1001,
	TH_ROOM_IN,
}THS;

typedef enum class FUNC_RETURN {
	FAIL,
	SUCCESS,
	SUCCESS_EMPTY,
	F_ERROR,
}FR;


#define MAX_ROOM_PLAYER 3
