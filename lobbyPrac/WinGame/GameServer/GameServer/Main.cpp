
#include <iostream>
#include "socket.h"

void print(int a) {
	std::cout << "hi from print" << a;
}

void asd(void (*fp)(int), int a) {
	std::thread t1(fp,a);
	t1.join();
}

int main() {
	

	asd(print, 1);
}