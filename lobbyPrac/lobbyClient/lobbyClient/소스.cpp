#include <iostream>
#include <WinSock2.h>

#pragma comment(lib,"ws2_32.lib")

int main() {
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { // 0 OR Errorcodes
		std::cout << "socket start error" << std::endl;
		return 0;
	}

	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET) {
		std::cout << "socket init error" << std::endl;
		return 0;
	}
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8888);
	addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	
	if (connect(sock, (SOCKADDR*)&addr, sizeof(addr))) { // 0 OR SOCKET_ERROR ( WSAGetLastError() )
		std::cout << "socket connection error" << std::endl;
		return 0;
	}
	std::cout << "server connected." << std::endl;

	char buf[128] = {};

	while(true) {
		std::cin >> buf;
		if (strcmp(buf, "EXIT") == 0) {
			break;
		}
		send(sock, buf, sizeof(buf), 0);
		recv(sock, buf, sizeof(buf), 0);
		std::cout << buf << std::endl;
		memset(buf, 0, sizeof(buf));
	}

	closesocket(sock);

	WSACleanup();
}