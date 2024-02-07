#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <string>

#include <iostream>
#include "Myprac.h"

using namespace std;

MyDB::MyDB() {
	AllocationHandles();
	Connect((SQLWCHAR*)L"MY_DATABASE_PRAC", (SQLWCHAR*)L"taesh", (SQLWCHAR*)L"1234");
}

MyDB::~MyDB() {
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	SQLDisconnect(hdbc);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

void MyDB::AllocationHandles() {
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

void MyDB::Connect(SQLWCHAR* w_dbname, SQLWCHAR* w_username, SQLWCHAR* w_password) {
	ret = SQLConnect(hdbc, w_dbname, SQL_NTS, w_username, SQL_NTS, w_password, SQL_NTS);
	/*if (!(ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)) {
		cout << ret << endl; SQL_SUCCESS_WITH_INFO;
		ret = SQLDriverConnect(hdbc, NULL, (SQLWCHAR*)L"DRIVER={SQL Server};SERVER=DESKTOP-NVBNOM4;DATABASE=MY_DATABASE_PRAC;UID=taesh;PWD=1234;",
			SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);
		if (ret == -1) {
			cout << "connection failed." << endl;
			return;
		}
	}*/
	if ((ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)) {
		cout << "db connection successed." << endl;
		ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

		if ((ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)) {
			cout << "Allocation STMT successed." << endl;
		}
	}
}

bool MyDB::InsertPlayer(SQLCHAR* id, SQLCHAR* password, SQLINTEGER score) {
	SQLPrepare(hstmt, (SQLWCHAR*)L"INSERT INTO Player VALUES (?, ?, ?)", SQL_NTS);

	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, MAX_ID_SIZE, 0, id, 0, NULL);
	SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, MAX_PASSWORD_SIZE, 0, password, 0, NULL);
	SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 50, 0, &score, 0, NULL);

	ret = SQLExecute(hstmt);

	SQLFreeStmt(hstmt, SQL_CLOSE);
	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
		cout << "insertion successed" << endl;
		return true;
	}
	else {
		cout << "insertion failed" << ret << endl;
		return false;
	}

}

bool MyDB::DeletePlayerById(SQLCHAR* id) {
	SQLPrepare(hstmt, (SQLWCHAR*)L"DELETE FROM Player WHERE id = ?", SQL_NTS);

	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, MAX_ID_SIZE, 0, id, 0, NULL);

	ret = SQLExecute(hstmt);

	SQLFreeStmt(hstmt, SQL_CLOSE);
	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
		return true;
	}
	else {
		return false;
	}
}

void MyDB::SeeAll() {
	ret = SQLExecDirectW(hstmt, (SQLWCHAR*)L"SELECT * FROM Player;", SQL_NTS);

	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
		SQLWCHAR id[50];
		SQLWCHAR password[50];
		SQLINTEGER score = 9915;

		while (SQLFetch(hstmt) == SQL_SUCCESS) {
			SQLGetData(hstmt, 1, SQL_C_WCHAR, id, 50, NULL);
			SQLGetData(hstmt, 2, SQL_C_WCHAR, password, 50, NULL);
			SQLGetData(hstmt, 3, SQL_C_LONG, &score, sizeof(score), NULL);

			wcout << L"id : " << id << L", password : " << password << L", score : "
				<< score << endl;
		}
	}
	else {
		cout << "select failed." << endl;
	}
	SQLFreeStmt(hstmt, SQL_CLOSE);
}

int MyDB::Login(SQLCHAR* id, string password) {
	SQLCHAR pwd[MAX_PASSWORD_SIZE];
	int score = -99;

	SQLPrepare(hstmt, (SQLWCHAR*)L"SELECT password, score FROM Player WHERE id = ?", SQL_NTS);

	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, MAX_ID_SIZE, 0, id, 0, NULL);
	ret = SQLExecute(hstmt);
	
	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
		if (SQLFetch(hstmt) == SQL_SUCCESS) {
			SQLGetData(hstmt, 1, SQL_C_CHAR, pwd, MAX_PASSWORD_SIZE, NULL);
			SQLGetData(hstmt, 2, SQL_C_LONG, &score, sizeof(score), NULL);
		}
	}
	else {
		cout << "sql failed." << endl;
	}

	SQLFreeStmt(hstmt, SQL_CLOSE);

	if (strcmp(password.c_str(), (char*)pwd) == 0) {
		return score;
	}
	else {
		return -99;
	}
}

int main() {
	MyDB db;
	int ord;

	//db.InsertPlayer((SQLCHAR*)"hi", (SQLCHAR*)"hello", 355);
	//db.DeletePlayerById((SQLCHAR*)"std");
	
	while (true) {
		cout << endl << "1. seeall 2. insert 3. deleteById 4.login 5. exit" << endl;
		cin >> ord;

		if (ord == 5)break;

		switch (ord) {
		case 1:
			db.SeeAll();
			break;
		case 2: {
			string id, password;
			int sc = 0;
			cout << "생성할 id 입력 :"; cin >> id;
			cout << "생성할 password 입력 :"; cin >> password;
			db.InsertPlayer((SQLCHAR*)id.c_str(), (SQLCHAR*)password.c_str(), sc);
			break;
		}
		case 3: {
			string id;
			cout << "삭제할 id 입력 :"; cin >> id;
			db.DeletePlayerById((SQLCHAR*)id.c_str());
			break;
		}
		case 4:
			string id, password;
			int score;
			cout << "로그인할 id 입력 :"; cin >> id;
			cout << "로그인할 password 입력 :"; cin >> password;
			if ((score = db.Login((SQLCHAR*)id.c_str(),password)) != -99) {
				cout << "login successed. "<< id <<"'s score : "<< score << endl;
			}
			else {
				cout << "login failed." << endl;
			}
			break;
		}
	}

	

	return 0;
}