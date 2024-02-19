#include "db.h"


DB::DB() {
	InitializeCriticalSection(&db_cs);
	AllocationHandles();
	Connect((SQLWCHAR*)L"MY_DATABASE_PRAC", (SQLWCHAR*)L"taesh", (SQLWCHAR*)L"1234");
}

DB::~DB() {
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	SQLDisconnect(hdbc);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

void DB::lock() {
	EnterCriticalSection(&db_cs);
}

void DB::unlock() {
	LeaveCriticalSection(&db_cs);
}

void DB::AllocationHandles() {
	ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
		ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
		if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
			ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
			if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
				printf("Allocate Success\n");
			}
			else {
				SQLGetDiagRecA(SQL_HANDLE_ENV, henv, ++rec, state, &native, message, sizeof(message), &length);
				printf("%s : %ld : %ld : %s\n", state, rec, native, message);
			}
		}
		else {
			SQLGetDiagRecA(SQL_HANDLE_ENV, henv, ++rec, state, &native, message, sizeof(message), &length);
			printf("%s : %ld : %ld : %s\n", state, rec, native, message);
		}
	}
	else {
		SQLGetDiagRecA(SQL_HANDLE_ENV, henv, rec, state, &native, message, sizeof(message), &length);
		printf("%s : %ld : %ld : %s\n", state, rec, native, message);
	}
}

void DB::Connect(SQLWCHAR* w_dbname, SQLWCHAR* w_username, SQLWCHAR* w_password) {
	ret = SQLConnect(hdbc, w_dbname, SQL_NTS, w_username, SQL_NTS, w_password, SQL_NTS);

	if ((ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)) {
		puts("db connection successed.");
		ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

		if ((ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)) {
			puts("Allocation STMT successed.");
		}
	}
}

bool DB::InsertPlayer(SQLCHAR* id, SQLCHAR* password, SQLINTEGER score) {
	lock();
	SQLPrepare(hstmt, (SQLWCHAR*)L"INSERT INTO Player VALUES (?, ?, ?)", SQL_NTS);

	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, MAX_PLAYER_NAME_SIZE, 0, id, 0, NULL);
	SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, MAX_PASSWORD_SIZE, 0, password, 0, NULL);
	SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 50, 0, &score, 0, NULL);

	ret = SQLExecute(hstmt);
	if ((ret == SQL_ERROR)) {
		printf("%d\n", ret);
		SQLGetDiagRecA(SQL_HANDLE_STMT, hstmt, 1, state, &native, message, sizeof(message), &length);
		printf("%s : %ld : %ld : %s\n", state, 1, native, message);
	}
	

	SQLFreeStmt(hstmt, SQL_CLOSE);
	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
		puts("insertion successed");
		unlock();
		return true;
	}
	else {
		puts("insertion failed");
		unlock();
		return false;
	}
}

int DB::Login(SQLCHAR* id, char* password) {
	lock();
	SQLCHAR pwd[MAX_PASSWORD_SIZE];
	int score = -99;

	SQLPrepare(hstmt, (SQLWCHAR*)L"SELECT password, score FROM Player WHERE id = ?", SQL_NTS);

	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, MAX_PLAYER_NAME_SIZE, 0, id, 0, NULL);
	ret = SQLExecute(hstmt);

	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
		if (SQLFetch(hstmt) == SQL_SUCCESS) {
			SQLGetData(hstmt, 1, SQL_C_CHAR, pwd, MAX_PASSWORD_SIZE, NULL);
			SQLGetData(hstmt, 2, SQL_C_LONG, &score, sizeof(score), NULL); // 지금 안쓰긴함
			SQLFreeStmt(hstmt, SQL_CLOSE);
		}
		else{
			SQLFreeStmt(hstmt, SQL_CLOSE);
			puts("db: no data");
			if (InsertPlayer(id, (SQLCHAR*)password, 0)) {
				unlock();
				return LOGIN_CREATE_NEW;
			}
			else {
				unlock();
				return LOGIN_FAIL;
			}
		}
	}

	if (strcmp(password, (char*)pwd) == 0) {
		unlock();
		return LOGIN_SUCCESS;
	}
	else {
		unlock();
		return LOGIN_FAIL;
	}
}

// get score or -99
int DB::getScore(SQLCHAR* id) {
	lock();
	int score;

	SQLPrepare(hstmt, (SQLWCHAR*)L"SELECT score FROM Player WHERE id = ?", SQL_NTS);

	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, MAX_PLAYER_NAME_SIZE, 0, id, 0, NULL);
	ret = SQLExecute(hstmt);

	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
		if (SQLFetch(hstmt) == SQL_SUCCESS) {
			SQLGetData(hstmt, 1, SQL_C_LONG, &score, sizeof(score), NULL);
			SQLFreeStmt(hstmt, SQL_CLOSE);
			unlock();
			return score;
		}
		else {
			SQLFreeStmt(hstmt, SQL_CLOSE);
			puts("db: no data");
			unlock();
			return -99;
		}
	}
	puts("db: get score failed.");
	SQLFreeStmt(hstmt, SQL_CLOSE);
	unlock();
	return -99;
}

bool DB::setScore(SQLCHAR* id, SQLINTEGER score) {
	lock();
	SQLPrepare(hstmt, (SQLWCHAR*)L"UPDATE Player set score = ? WHERE id = ?", SQL_NTS);

	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 50, 0, &score, 0, NULL);
	SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, MAX_PLAYER_NAME_SIZE, 0, id, 0, NULL);
	ret = SQLExecute(hstmt);
	SQLFreeStmt(hstmt, SQL_CLOSE);

	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
		puts("DB : score update successed.");
		unlock();
		return true;
	}
	else {
		puts("DB : score update failed.");
		unlock();
		return false;
	}
}