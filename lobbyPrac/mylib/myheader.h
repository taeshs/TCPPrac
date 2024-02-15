#include <iostream>
#include <WinSock2.h>
#include <thread>
#include <list>
#include <string>
#include <conio.h>

#define MAX_ROOM_PLAYER 3
#define MAX_PLAYER_NAME_SIZE 21
#define MAX_PASSWORD_SIZE 21

typedef enum class CMDCODE {
	CMD_NULL = 0,
	CMD_ECHO = 100,
	CMD_CHAT = 200,
	CMD_ENTERROOM = 1000,
	CMD_LEAVEROOM,
	CMD_ROOMCHAT = 2020,
	CMD_LOGIN_ACCEPT = 9998,
	CMD_CONNECT = 9999,
	CMD_LOGIN_FAIL,
}CMDCODE;

typedef struct MYCMD {
	CMDCODE nCode;
	//char player_name[MAX_PLAYER_NAME_SIZE];
	//std::string player_name; // 
}MYCMD;

typedef struct ClientSock {
	SOCKET socket;
	char player_name[MAX_PLAYER_NAME_SIZE];
	//std::string player_name;
}ClientSock;

typedef struct PlayerLoginData {
	char player_name[MAX_PLAYER_NAME_SIZE];
	char password[MAX_PASSWORD_SIZE];
}PlayerLoginData;

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


