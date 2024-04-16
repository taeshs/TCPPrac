#include "../../mylib/myheader.h"
#include "NGP_TermProject.h"
#include <CommCtrl.h>
#include "winmainapp.h"
#pragma comment(lib,"ws2_32.lib")

void Roomthread(SOCKET sock);
void LobbyRecvThread(SOCKET sock);
void RoomRecvThread(SOCKET sock);
void GameThread(SOCKET);

BOOL CALLBACK IPGETTER_Dlg_Proc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

char g_pname[20];

DWORD address;
SOCKET g_sock;

HINSTANCE g_hInst;
HINSTANCE g_hPrevInst;
LPWSTR    g_lpCmdLine;
int       g_nCmdShow;

#define MAX_LOADSTRING 100
WCHAR g_szWindowClass[MAX_LOADSTRING];

int g_gameStatus = -1;
void SendRoomOut() {
	MYCMD cmd;
	cmd.nCode = CMDCODE::CMD_LEAVEROOM;
	send(g_sock, (char*)&cmd, sizeof(cmd), 0);
}

void Roomthread(SOCKET sock) {
	std::cout << "Room entered." << std::endl;
	char buf[128];
	MYCMD cmd;
	//strcpy(cmd.player_name, g_pname);
	//std::thread th_RoomRecv(RoomRecvThread, sock);
	std::thread th_game(GameThread, sock);
	th_game.join();
	//th_RoomRecv.detach();
	puts("나오지마라");
	cmd.nCode = CMDCODE::CMD_LEAVEROOM;
	send(sock, (char*)&cmd, sizeof(cmd), 0);
	std::thread t1(LobbyRecvThread, sock);
	t1.detach();
	return;

	//while (true) {
	//	std::cout << "room thread sender" << std::endl;

	//	// 방 안.
	//	std::cin >> buf;
	//	if (g_gameStatus == 1) { // g_gameStatus 로 나가기 하기. Working
	//		cmd.nCode = CMDCODE::CMD_LEAVEROOM;
	//		send(sock, (char*)&cmd, sizeof(cmd), 0);
	//		std::thread t1(LobbyRecvThread, sock);
	//		t1.detach();
	//		return;
	//		// 종료및 기존 쓰레드 되돌아가기
	//	}
	//	else {
	//		cmd.nCode = CMDCODE::CMD_ROOMCHAT;
	//		send(sock, (char*)&cmd, sizeof(cmd), 0);
	//	}
	//	send(sock, buf, sizeof(buf), 0);
	//	ZeroMemory(buf, sizeof(buf));
	//}
}

//void RoomRecvThread(SOCKET sock) {
//	MYCMD cmd;
//	char buf[128] = {};
//	while (recv(sock, (char*)&cmd, sizeof(cmd), 0) > 0) {
//		std::cout << "room thread recver" << std::endl;
//		if (cmd.nCode == CMDCODE::CMD_LEAVEROOM) {
//			break;
//		}
//		recv(sock, buf, sizeof(buf), 0);
//		if (cmd.nCode == CMDCODE::CMD_ROOMCHAT) {
//			std::cout << "from another client in room : ";
//			std::cout << buf << std::endl;
//			ZeroMemory(buf, sizeof(buf));
//		}
//	}
//}

void LobbyRecvThread(SOCKET sock) {
	MYCMD cmd;
	char buf[128] = {};
	while (recv(sock, (char*)&cmd, sizeof(cmd), 0) > 0) {
		std::cout << "lobby thread reciever" << std::endl;
		if (cmd.nCode == CMDCODE::CMD_ENTERROOM) {
			puts("나와라");
			break;
		}
		recv(sock, buf, sizeof(buf), 0);
		switch (cmd.nCode) {
		case CMDCODE::CMD_CHAT:
			std::cout << "from another client : ";
			break;
		case CMDCODE::CMD_ECHO:
			std::cout << "ECHO from SERVER : ";
			break;
		}
		std::cout << buf << std::endl;
		ZeroMemory(buf, sizeof(buf));
	}
}

void GameThread(SOCKET sock) {
	game(g_hInst,
		g_hPrevInst,
		g_lpCmdLine,
		g_nCmdShow,
		address,
		sock);
	g_gameStatus = 1;
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow) {

	g_hInst = hInstance;
	g_hPrevInst = hPrevInstance;
	g_lpCmdLine = lpCmdLine;
	g_nCmdShow = nCmdShow;
	LoadStringW(hInstance, IDC_NGPTERMPROJECT, g_szWindowClass, MAX_LOADSTRING);
	AllocConsole();
	freopen("CONOUT$", "wt", stdout);
	freopen("CONIN$", "r", stdin);

	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)&IPGETTER_Dlg_Proc);

	char password[MAX_PASSWORD_SIZE];
	std::cout << "ENTER NAME : " << std::endl;
	std::cin >> g_pname;
	std::cout << "ENTER PASSWORD : " << std::endl;
	std::cin >> password;

	/*game(hInstance,
		hPrevInstance,
		lpCmdLine,
		nCmdShow,
		address);*/ // 일단 작동확인함. -> 룸스레드로 옮기기.

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
	addr.sin_addr.S_un.S_addr = htonl(address);//inet_addr("127.0.0.1");

	if (connect(sock, (SOCKADDR*)&addr, sizeof(addr))) { // 0 OR SOCKET_ERROR ( WSAGetLastError() )
		std::cout << "socket connection error" << std::endl;
		return 0;
	}
	std::cout << "server connected." << std::endl;
	g_sock = sock;

	PlayerLoginData LoginData;
	MYCMD firstCmd;
	firstCmd.nCode = CMDCODE::CMD_CONNECT;
	// 로그인 요청
	send(sock, (char*)&firstCmd, sizeof(firstCmd), 0);

	// 로그인 데이터 보내기
	strcpy(LoginData.player_name, g_pname);
	strcpy(LoginData.password, password);
	send(sock, (char*)&LoginData, sizeof(PlayerLoginData), 0);

	// 로그인 성공 여부
	memset(&firstCmd, 0, sizeof(firstCmd));
	recv(sock, (char*)&firstCmd, sizeof(firstCmd), 0);
	if (!(firstCmd.nCode == CMDCODE::CMD_LOGIN_ACCEPT)) {
		std::cout << "somethin wrong..." << std::endl;
		return 0;
	}

	char buf[128] = {};

	std::thread t1(LobbyRecvThread, sock);
	t1.detach();

	MYCMD cmd;
	//strcpy(cmd.player_name, g_pname);
	while (true) {
		puts("lobby thread sender");
		std::cin >> buf;
		if (strcmp(buf, "EXIT") == 0) {
			break;
		}
		else if (strcmp(buf, "ECHO") == 0) {
			std::cout << "ECHO 할 문자 입력 : ";
			std::cin >> buf;
			cmd.nCode = CMDCODE::CMD_ECHO;
			send(sock, (char*)&cmd, sizeof(cmd), 0);
			send(sock, buf, sizeof(buf), 0);
		}
		else if (strcmp(buf, "ENTER") == 0) {
			std::cout << "방에 진입합니다..." << std::endl;
			cmd.nCode = CMDCODE::CMD_ENTERROOM;
			send(sock, (char*)&cmd, sizeof(cmd), 0);
			std::thread room_thread(Roomthread, sock);
			room_thread.join();
		}
		else {
			std::cout << "chat 보냄" << std::endl;
			cmd.nCode = CMDCODE::CMD_CHAT;
			send(sock, (char*)&cmd, sizeof(cmd), 0);
			send(sock, buf, sizeof(buf), 0);
		}
		memset(buf, 0, sizeof(buf));

		//std::cout << buf << std::endl;
	}
	shutdown(sock, SD_BOTH);
	closesocket(sock);


	WSACleanup();
	FreeConsole();
}


BOOL CALLBACK IPGETTER_Dlg_Proc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		switch (wParam) {
		case IDOK:
			SendMessage(GetDlgItem(hDlg, IDC_IPADDRESS1), IPM_GETADDRESS, 0, (LPARAM)&address);
			//std::cout << FIRST_IPADDRESS(address) << SECOND_IPADDRESS(address) << THIRD_IPADDRESS(address) << FOURTH_IPADDRESS(address);
			EndDialog(hDlg, IDOK);
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		case IDC_LOCAL:
			address = MAKEIPADDRESS(127, 0, 0, 1);
			//std::cout << FIRST_IPADDRESS(address) << SECOND_IPADDRESS(address) << THIRD_IPADDRESS(address) << FOURTH_IPADDRESS(address);
			EndDialog(hDlg, IDOK);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

// 게임(was메인) / 송수신 스레드 구조.
// -> roomthread에서 기존 winmain만 실행하며될듯.