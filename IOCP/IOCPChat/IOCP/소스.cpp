#include <iostream>
#include <thread>
#include <vector>
#include <list>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

constexpr int PORTS = 8888;
constexpr int MAXTHREADS = 4;
constexpr int BUFSIZE = 1024;

using namespace std;

HANDLE g_iocp;
vector<thread> g_worker_threads;
list<SOCKET> g_clients;

typedef struct CLIENT_SESSION {
	SOCKET sock;
	char buf[BUFSIZE];
};

void sendAll(SOCKET sock, char* buf, int len) {
	list<SOCKET>::iterator it;
	for (it = g_clients.begin(); it != g_clients.end(); it++) {
		if (*it != sock) {
			send(*it, buf, len, 0);
		}
	}
}

void threadFunc() {
	BOOL result;
	DWORD byteTransffered, flag;
	CLIENT_SESSION *c_session;
	ULONG_PTR key;
	LPOVERLAPPED pol;
	cout << "workerthread init"<<endl;
	while (1) {
		byteTransffered = 0; flag = 0;
		result = GetQueuedCompletionStatus(g_iocp, &byteTransffered, &key, &pol, INFINITE);//(PULONG_PTR)&c_session, &pol, INFINITE);
		if (result == false) {
			continue;
		}
		c_session = (CLIENT_SESSION*)key;
		if (byteTransffered == 0) {
			// free
			delete c_session;
			delete pol;
		}
		else {
			cout << "from " << c_session->sock << " : " << c_session->buf << endl;
			sendAll(c_session->sock, c_session->buf, byteTransffered);
			ZeroMemory(c_session->buf, BUFSIZE);

			WSABUF wsabuf;
			wsabuf.buf = c_session->buf;
			wsabuf.len = BUFSIZE;
			byteTransffered = 0; flag = 0;
			ZeroMemory(pol, sizeof(OVERLAPPED));
			WSARecv(c_session->sock, &wsabuf, 1, &byteTransffered, &flag, pol, 0);
		}
	}
}

int main() {
	WSADATA wsa;
	int errcode = WSAStartup(MAKEWORD(2, 2), &wsa);
	if (errcode == SOCKET_ERROR) {
		cerr << "ERROR : WSA_STARTUP " << errcode << endl;
		return 1;
	}

	SOCKET lsock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (lsock == INVALID_SOCKET) {
		cerr << "ERROR : CREATE SOCKET : " << GetLastError() << endl;
		return 0;
	}

	g_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (g_iocp == NULL) {
		cerr << "ERROR : CREATE COMPLETION PORT : " << GetLastError() << endl;
		return 0;
	}
	SOCKADDR_IN saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(PORTS);
	saddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (bind(lsock, (SOCKADDR*)&saddr, sizeof(saddr)) == SOCKET_ERROR) {
		cerr << "ERROR : bind : " << GetLastError() << endl;
		return 0;
	}
	if (listen(lsock, SOMAXCONN) == SOCKET_ERROR) {
		cerr << "ERROR : listen : " << GetLastError() << endl;
		return 0;
	}
	;

	for (int o = 0; o < MAXTHREADS; o++) {
		g_worker_threads.emplace_back(threadFunc);
	}

	SOCKET csock; // 통신소켓
	SOCKADDR_IN caddr;
	int caddr_size = sizeof(caddr);
	
	CLIENT_SESSION* client_session;
	WSABUF wsabuf;
	DWORD flag, byterecved;
	LPOVERLAPPED ol;

	while (1) {
		csock = accept(lsock, (SOCKADDR*)&caddr, &caddr_size);
		if (csock == INVALID_SOCKET) {
			cerr << "ERROR : SOCKET accept : " << WSAGetLastError() << endl;
			closesocket(csock);
			continue;
		}
		cout << "accepted." << endl;
		client_session = new CLIENT_SESSION;
		ZeroMemory(client_session, sizeof(CLIENT_SESSION));
		ol = new OVERLAPPED;
		ZeroMemory(ol, sizeof(OVERLAPPED));
		
		client_session->sock = csock;
		CreateIoCompletionPort((HANDLE)csock, g_iocp, (ULONG_PTR)client_session, 0);
		g_clients.push_back(csock);

		wsabuf.buf = client_session->buf;
		wsabuf.len = BUFSIZE;
		flag = 0;
		byterecved = 0;

		WSARecv(csock, &wsabuf, 1, &byterecved, &flag, ol, 0);
	}
}