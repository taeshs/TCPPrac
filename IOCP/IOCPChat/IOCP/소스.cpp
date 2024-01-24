#include <iostream>
#include <WinSock2.h>
#include <thread>
#include <list>

#pragma comment(lib,"ws2_32.lib")

#define MAX_THREAD_CNT	4

CRITICAL_SECTION g_cs;
HANDLE g_hIocp; 
SOCKET g_lsock;
std::list<SOCKET> g_listClient;

typedef struct _USERSESSION {
	SOCKET hSocket;
	char buffer[8192];
}USERSESSION;

void CloseClient(SOCKET sock){
	shutdown(sock, SD_BOTH);
	closesocket(sock);

	EnterCriticalSection(&g_cs);
	g_listClient.remove(sock);
	LeaveCriticalSection(&g_cs);
}

void SendMessageAll(char* buf, int nSize) {
	std::list<SOCKET>::iterator it;

	EnterCriticalSection(&g_cs);
	for (it = g_listClient.begin(); it != g_listClient.end(); it++) {
		send(*it, buf, nSize,0);
	}
	LeaveCriticalSection(&g_cs);
}

void CloseAll()
{
	std::list<SOCKET>::iterator it;

	::EnterCriticalSection(&g_cs);
	for (it = g_listClient.begin(); it != g_listClient.end(); ++it)
	{
		::shutdown(*it, SD_BOTH);
		::closesocket(*it);
	}
	::LeaveCriticalSection(&g_cs);
}

BOOL CtrlHandler(DWORD dwType)
{
	if (dwType == CTRL_C_EVENT)
	{
		CloseAll();
		::Sleep(500);

		//Listen 소켓을 닫는다.
		::shutdown(g_lsock, SD_BOTH);
		::closesocket(g_lsock);
		g_lsock = NULL;

		//IOCP 핸들을 닫는다. 이렇게 하면 GQCS() 함수가 FALSE를 반환하며
		//:GetLastError() 함수가 ERROR_ABANDONED_WAIT_0을 반환한다.
		//IOCP 스레드들이 모두 종료된다.
		::CloseHandle(g_hIocp);
		g_hIocp = NULL;

		//IOCP 스레드들이 종료되기를 일정시간 동안 기다린다.
		::Sleep(500);
		::DeleteCriticalSection(&g_cs);

		puts("*** 채팅서버를 종료합니다! ***");
		::WSACleanup();
		exit(0);
		return TRUE;
	}

	return FALSE;
}


DWORD ThreadComplete() {
	DWORD dwTransferredSize = 0;
	DWORD dwFlag = 0;
	USERSESSION* pSession = NULL;
	LPWSAOVERLAPPED pWol = NULL;
	BOOL bResult;

	puts("[IOCP 작업자 스레드 시작]");
	while (1) {
		bResult = GetQueuedCompletionStatus(
			g_hIocp,
			&dwTransferredSize,
			(PULONG_PTR)&pSession,
			&pWol,
			INFINITE
		);

		if (bResult) {
			// 정상
			if (dwTransferredSize == 0) {
				// 정상종료
				CloseClient(pSession->hSocket);
				delete pWol;
				delete pSession;
				puts("\tGQCS : 클라이언트가 정상 연결 종료함.");
			}
			else {
				SendMessageAll(pSession->buffer, dwTransferredSize);
				memset(pSession->buffer, 0, sizeof(pSession->buffer));

				DWORD dwRecievedSize = 0;
				DWORD dwFlag = 0;
				WSABUF wsaBuf = { 0 };
				wsaBuf.buf = pSession->buffer;
				wsaBuf.len = sizeof(pSession->buffer);
				WSARecv(
					pSession->hSocket,
					&wsaBuf,
					1,
					&dwRecievedSize,
					&dwFlag,
					pWol,
					NULL
				);
				if (::WSAGetLastError() != WSA_IO_PENDING)
					puts("\tGQCS: ERROR WSARecv()");
			}
		}
		else {
			// 비정상
			if (pWol == NULL) {
				// 완료 큐에서 완료 패킷을 꺼내지 못하고 반환한 경우.
				// iocp핸들이 닫힌(서버 종료)경우도 해당.
				puts("\tGQCS: IOCP 핸들이 닫혔습니다.");
				break;
			}
			else {
				// 클라이언트 비정상 종료 혹은 서버가 먼저 연결 종료.
				if (pSession != NULL) {
					CloseClient(pSession->hSocket);
					delete pWol;
					delete pSession;
				}
				puts("\tGQCS: 서버 종료 혹은 비정상적 연결 종료");
			}
		}
	}

	puts("[IOCP 작업자 스레드 종료]");
	return 0;
}

DWORD ThreadAcceptLoop() {
	LPWSAOVERLAPPED pWol = NULL;
	DWORD dwReceiveSize, dwFlag;
	USERSESSION* pNewUser;
	int nAddrSize = sizeof(SOCKADDR);
	WSABUF wsaBuf;
	SOCKADDR caddr;
	SOCKET csock;
	int nRecvResult = 0;

	while ((csock = accept(g_lsock, &caddr, &nAddrSize)) != INVALID_SOCKET) {
		std::cout << "새 클라이언트 연결." << std::endl;
		EnterCriticalSection(&g_cs);
		g_listClient.push_back(csock);
		LeaveCriticalSection(&g_cs);

		pNewUser = new USERSESSION;
		ZeroMemory(pNewUser, sizeof(USERSESSION));
		pNewUser->hSocket = csock;

		pWol = new WSAOVERLAPPED;
		ZeroMemory(pWol, sizeof(WSAOVERLAPPED));

		// 연결한 소켓을 IOCP에 연결
		CreateIoCompletionPort((HANDLE)csock, g_hIocp,
			(ULONG_PTR)pNewUser,
			0);

		dwReceiveSize = 0;
		dwFlag = 0;
		wsaBuf.buf = pNewUser->buffer;
		wsaBuf.len = sizeof(pNewUser->buffer);

		nRecvResult = WSARecv(csock, &wsaBuf, 1, &dwReceiveSize, &dwFlag, pWol, NULL);
		if (::WSAGetLastError() != WSA_IO_PENDING)
			puts("ERROR: WSARecv() != WSA_IO_PENDING");
	}

	return 0;
}

int main() {
	WSADATA wsa = { 0 };

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		std::cout << "ERROR : winsock 초기화 실패" << std::endl;
		return 0;
	}

	if (::SetConsoleCtrlHandler(
		(PHANDLER_ROUTINE)CtrlHandler, TRUE) == FALSE)
		puts("ERROR: Ctrl+C 처리기를 등록할 수 없습니다.");

	InitializeCriticalSection(&g_cs);
	g_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (g_hIocp == NULL) {
		std::cout << "ERROR : iocp 생성 실패" << std::endl;
		return 0;
	}

	for (int i = 0; i < MAX_THREAD_CNT; i++) {
		std::thread t1(ThreadComplete);
		t1.detach();
	}

	g_lsock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP,
		NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN saddr = { 0 };
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(8888);
	saddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (bind(g_lsock, (SOCKADDR*)&saddr, sizeof(saddr)) == SOCKET_ERROR) {
		std::cout << "ERROR : bind 실패" << std::endl;
		return 0;
	}

	if (listen(g_lsock, SOMAXCONN) == SOCKET_ERROR) {
		std::cout << "ERROR : listen 실패" << std::endl;
		return 0;
	}

	std::thread t1(ThreadAcceptLoop);
	t1.detach();

	std::cout << "채팅서버 시작." << std::endl;
	while (true) getchar(); // main 함수 반환 방지

	return 0;
}