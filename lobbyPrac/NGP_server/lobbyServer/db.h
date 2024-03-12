#pragma once

#include <Windows.h>
#include <sql.h>
#include <sqlext.h>
#include <string>

#define MAX_PLAYER_NAME_SIZE 21
#define MAX_PASSWORD_SIZE 21

typedef enum loginResult {
	LOGIN_SUCCESS = 99,
	LOGIN_FAIL = -1,
	LOGIN_CREATE_NEW = 101
}loginResult;

class DB
{
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLRETURN ret;
	SQLHSTMT hstmt;

	CRITICAL_SECTION db_cs;

	// for see errorcodes
	SQLSMALLINT length;
	SQLINTEGER rec = 0, native;
	SQLCHAR state[7], message[256];

	void lock();

	void unlock();

public:
	DB();

	~DB();

	void AllocationHandles();

	void Connect(SQLWCHAR* w_dbname, SQLWCHAR* w_username, SQLWCHAR* w_password);

	bool InsertPlayer(SQLCHAR* id, SQLCHAR* password, SQLINTEGER score);

	int Login(SQLCHAR* id, char* password);

	int getScore(SQLCHAR* id);

	bool setScore(SQLCHAR* id, SQLINTEGER score);
	
};

