#include <iostream>
#include <list>
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")

using namespace std;

enum class CMD{
	FAIL = 100,
	SUCCESS = 200,
	SUCCESS_EMPTY,
};

class Room {
	string name;
	string players[4];
	int now = 0;

public:
	Room() : name("default") {

	}
	Room(string n) : name(n){
		for (int i = 0; i < 4; i++) {
			players[i] = "empty";
		}
	}
	~Room() {

	}

	bool GetIn(string playername) {
		if (now < 4) {
			//players[now++] = playername;
			for (int i = 0; i < 4; i++) {
				if (players[i] == "empty") {
					players[i] = playername;
					now++;
					break;
				}
			}
			return true;
		}
		else {
			cout << "Room is full" << endl;
			return false;
		}
	}

	string GetRoomName() {
		return name;
	}

	void SeeAll(){
		cout << "방 이름 : " << name << "  인원 : " << now << endl << "-----플레이어 목록-----"<< endl;
		for (int i = 0; i < 4; i++) {
			cout << players[i] << endl;
		}
		cout << "--------------------" << endl;
	}

	CMD LeaveRoom(string pname) {
		for (int i = 0; i < 4; i++) {
			if (players[i] == pname) {
				players[i] = "empty";
				now--;
				if (now == 0) {
					return CMD::SUCCESS_EMPTY;
				}
				return CMD::SUCCESS;
			}
		}
		return CMD::FAIL;
	}
};

class Rooms {
	list<Room*> rooms_list;

public:
	Rooms(){
		
	}
	~Rooms(){
		list<Room*>::iterator it;
		for (it = rooms_list.begin(); it != rooms_list.end(); it++) {
			delete(*it);
		}
	}

	void SeeAllRooms() {
		if (rooms_list.empty()) {
			cout << "no rooms." << endl;
			return;
		}
		list<Room*>::iterator it;
		cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
		for (it = rooms_list.begin(); it != rooms_list.end(); it++) {
			(*it)->SeeAll();
		}
		cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
	}

	void GetIn(string player_name) {
		list<Room*>::iterator it;
		bool dist;

		if (rooms_list.empty()) {
			Room* room = new Room("1st room");
			room->GetIn(player_name);
			rooms_list.push_back(room);
			return;
		}
		for (it = rooms_list.begin(); it != rooms_list.end(); it++) {
			if (dist = (*it)->GetIn(player_name)) {
				break;
			}
			// 성공 true 실패 false
		}
		if (dist == false) {
			Room* nroom = new Room("new room");
			nroom->GetIn(player_name);
			rooms_list.push_back(nroom);
		}
	}

	void LeaveRoom(string player_name) {
		list<Room*>::iterator it;
		CMD dist;
		for (it = rooms_list.begin(); it != rooms_list.end(); it++) {
			dist = (*it)->LeaveRoom(player_name);
			if ( dist == CMD::SUCCESS || dist == CMD::SUCCESS_EMPTY ) {
				break;
			}
		}
		if (dist == CMD::FAIL) {
			cout << "해당 플레이어 없음." << endl;
		}
		else if (dist == CMD::SUCCESS_EMPTY) {
			cout << "empty room." << endl;
			Room* temp = *it;
			rooms_list.erase(it);
			delete temp;
		}
	}

};


int main() {
	Rooms rooms;

	int a;
	string name;
	while (true) {
		cout << "1. 입장 2. 퇴장 3. 출력  99. 종료 : ";
		cin >> a;

		switch (a) {
		case 1:
			cout << "플레이어 이름 입력 : ";
			cin >> name;
			rooms.GetIn(name);
			cout << "'" << name << "' 입장함. " << endl;
			break;
		case 2:
			cout << "플레이어 이름 입력 : ";
			cin >> name;
			rooms.LeaveRoom(name);
			cout << "'" << name << "' 퇴장함. " << endl;
			break;
		case 3:
			rooms.SeeAllRooms();
			break;
		}

		if (a == 99) {
			break;
		}
	}
}
