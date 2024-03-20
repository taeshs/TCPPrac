
#include <iostream>

#include "socket.h"

int main() {
	WS_SERVER server;

	server.Client_accept();
	
	getchar();

	server.stop_server();
}


//void print(int a) {
//	std::cout << "hi from print" << a;
//}
//
//void asd(void (*fp)(int), int a) {
//	std::thread t1(fp,a);
//	t1.join();
//}
////asd(print, 1);