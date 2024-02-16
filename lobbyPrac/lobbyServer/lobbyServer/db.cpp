#include "db.h"


DB::DB() {
	AllocationHandles();
	Connect((SQLWCHAR*)L"MY_DATABASE_PRAC", (SQLWCHAR*)L"taesh", (SQLWCHAR*)L"1234");
}

DB::~DB() {
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	SQLDisconnect(hdbc);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, henv);
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
		return true;
	}
	else {
		puts("insertion failed");
		return false;
	}

}

int DB::Login(SQLCHAR* id, char* password) {
	SQLCHAR pwd[MAX_PASSWORD_SIZE];
	int score = -99;

	SQLPrepare(hstmt, (SQLWCHAR*)L"SELECT password, score FROM Player WHERE id = ?", SQL_NTS);

	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, MAX_PLAYER_NAME_SIZE, 0, id, 0, NULL);
	ret = SQLExecute(hstmt);

	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
		if (SQLFetch(hstmt) == SQL_SUCCESS) {
			SQLGetData(hstmt, 1, SQL_C_CHAR, pwd, MAX_PASSWORD_SIZE, NULL);
			SQLGetData(hstmt, 2, SQL_C_LONG, &score, sizeof(score), NULL);
			SQLFreeStmt(hstmt, SQL_CLOSE);
		}
		else{
			SQLFreeStmt(hstmt, SQL_CLOSE);
			puts("db: no data");
			if (InsertPlayer(id, (SQLCHAR*)password, 0)) {
				return LOGIN_CREATE_NEW;
			}
			else {
				return LOGIN_FAIL;
			}
		}
	}

	if (strcmp(password, (char*)pwd) == 0) {
		return LOGIN_SUCCESS;
	}
	else {
		return LOGIN_FAIL;
	}
}