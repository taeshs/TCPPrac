#include <iostream>
#include <WinSock2.h>

#pragma comment(lib,"ws2_32.lib")

int main() {
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8888);
	addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	connect(sock, (SOCKADDR*)&addr, sizeof(addr));

	char buf[128] = {};
	recv(sock, buf, sizeof(buf), 0);

	std::cout << buf << std::endl;

	memcpy(buf, "hi from client", 15);

	send(sock, buf, sizeof(buf), 0);

	closesocket(sock);

	WSACleanup();
}