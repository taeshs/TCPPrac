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
		cout << "�� �̸� : " << name << "  �ο� : " << now << endl << "-----�÷��̾� ���-----"<< endl;
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


// �׳� �߰��� �Ǵ� ���� 

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
		cout << "1. ���� 2. ��� 3. ���� : ";
		cin >> a;

		switch (a) {
		case 1:
			cout << "�÷��̾� �̸� �Է� : ";
			cin >> name;
			if (!((*it).GetIn(name))) {
				cout << "���� ���� á���ϴ�. ���� �����մϴ�...." << endl;
				cout << "���� ������ �� �̸��� �Է��ϼ��� : ";
				cin >> rname;
				p = new Room(rname);
				p->GetIn(name);
				rooms.push_back(*p);
				it++;
			}
			break;
		case 2:
			cout << "��� �� ������ �����." << endl;
			for (mit = rooms.begin(); mit != rooms.end(); mit++) {
				(*mit).SeeAll();
			}
			break;
		}

		if (a == 3) break;
	}
}