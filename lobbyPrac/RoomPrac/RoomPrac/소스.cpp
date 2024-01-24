#include <iostream>
#include <list>

using namespace std;

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

	bool LeaveRoom(string pname) {
		for (int i = 0; i < 4; i++) {
			if (players[i] == pname) {
				// delete ing
			}
		}
	}
};


// 그냥 추가만 되는 형태 

int main() {
	list<Room> rooms;
	Room* r = new Room("1st");
	rooms.push_back(*r);

	list<Room>::iterator it, mit;
	it = rooms.begin();

	int a;
	string name, rname;
	Room* p;
	while (true) {
		cout << "1. 입장 2. 출력 3. 종료 : ";
		cin >> a;

		switch (a) {
		case 1:
			cout << "플레이어 이름 입력 : ";
			cin >> name;
			if (!((*it).GetIn(name))) {
				cout << "방이 가득 찼습니다. 새로 생성합니다...." << endl;
				cout << "새로 생성할 방 이름을 입력하세요 : ";
				cin >> rname;
				p = new Room(rname);
				p->GetIn(name);
				rooms.push_back(*p);
				it++;
			}
			break;
		case 2:
			cout << "모든 방 정보를 출력함." << endl;
			for (mit = rooms.begin(); mit != rooms.end(); mit++) {
				(*mit).SeeAll();
			}
			break;
		}

		if (a == 3) break;
	}
}