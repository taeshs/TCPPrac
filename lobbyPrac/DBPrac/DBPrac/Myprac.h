#pragma once
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <string>

#define MAX_ID_SIZE 50
#define MAX_PASSWORD_SIZE 50

class MyDB {
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLRETURN ret;
	SQLHSTMT hstmt;

	SQLSMALLINT length;
	SQLINTEGER rec = 0, native;
	SQLCHAR state[7], message[256];

public:
	MyDB();

	~MyDB();

	void AllocationHandles();

	void Connect(SQLWCHAR* w_dbname, SQLWCHAR* w_username, SQLWCHAR* w_password);
	
	bool InsertPlayer(SQLCHAR* id, SQLCHAR* password, SQLINTEGER score);
	
	bool DeletePlayerById(SQLCHAR* id);
	
	void SeeAll();

	int Login(SQLCHAR* id, std::string password);
};