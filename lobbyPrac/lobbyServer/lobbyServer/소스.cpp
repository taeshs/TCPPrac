#include <iostream>
#include <WinSock2.h>
#include <thread>
#include <list>

#pragma comment(lib,"ws2_32.lib")

// Multithread echo -> Multithread chatting

// ������ Ŭ���̾�Ʈ�� ������ list 
std::list<SOCKET> tlist;

void MessageSender(char* buf, int size, int sockid) {
	std::list<SOCKET>::iterator it;
	for (it = tlist.begin(); it != tlist.end(); it++) {
		if(*it != sockid) send(*it, buf, size, 0); // �������� �Ⱥ�����
		//std::cout << "id" << *it << std::endl;
	}
}

// ������ �ϴ��� - ä�� ���� ����, ����� 
void threadFunc(SOCKET sock) {
	char buf[128];
	std::cout << "new client connected." << std::endl;
	while (recv(sock, buf, sizeof(buf), 0) > 0) {
		std::cout << buf << std::endl;
		MessageSender(buf, sizeof(buf), sock);
		memset(buf, 0, sizeof(buf));
	}
	std::cout << "client disconnected." << std::endl;
	// ���������� �ο�
	closesocket(sock);
}

int main() {
	WSADATA wsa = {0};
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { // 0 OR Errorcodes
		std::cout << "socket start error" << std::endl;
		return 0;
	}

	SOCKET lsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (lsock == INVALID_SOCKET) {
		std::cout << "socket init error" << std::endl;
		return 0;
	}

	SOCKADDR_IN addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8888);
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	if (bind(lsock, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) { // 0 OR SOCKET_ERROR ( WSAGetLastError() )
		std::cout << "socket bind error" << std::endl;
		return 0;
	}

	if(listen(lsock, SOMAXCONN)){ // 0 OR SOCKET_ERROR ( WSAGetLastError() )
		std::cout << "socket listen error" << std::endl;
		return 0;
	}

	SOCKADDR_IN caddr = { 0 };
	int csize = sizeof(caddr);
	SOCKET csock;
	
	while ((csock = accept(lsock, (SOCKADDR*)&caddr, &csize))) {
		std::thread t1(
			threadFunc, csock
		);
		std::cout << "id " << csock << " connected" << std::endl;
		tlist.push_back(csock);
		t1.detach();
	}	

	
	closesocket(lsock);

	WSACleanup();
}