#pragma once

#define MAX_Client 2

#define TYPE_BULLET 10000
#define TYPE_PLAYER 10001

#define SERVERPORT 8685
#define BUFSIZE    51200


#define MIN(a,b) a>b?b:a




//int GameServer();


struct vector2D {
    float x, y;
};


class BoundingBox {
public:
    BoundingBox() {}
    BoundingBox(float a, float b, float c, float d, int ty, int Player) { left = a; top = b; right = c; bottom = d; type = ty; bPlayer = Player; }

    void SetBB(float a, float b, float c, float d, int ty, int Player) { left = a; top = b; right = c; bottom = d; type = ty; bPlayer = Player; }
    void Update(float a, float b, float c, float d) { left = a; top = b; right = c; bottom = d; }

    RECT getBB() { RECT a; a.left = left; a.top = top; a.right = right; a.bottom = bottom; return a; }
private:
    float left, top, right, bottom;

    int type;
    int bPlayer;
};
bool collisionCheck(BoundingBox a, BoundingBox b);

#pragma pack(push, 1)
typedef struct Player_Socket
{
    int posX, posY;
    int hp = 9999;
    BoundingBox bb;
}Player_Socket;
#pragma pack(pop)

struct Bullet {
    vector2D shootDir;
    bool alive = false;
    int bType = 0;
    int bSize;
    int bSpeed;
    int bDamage;

    float bPosX = 0, bPosY = 0;

    int bPlayer = 0;
    BoundingBox bb;

    Bullet(int type, int size, int spd, int dmg)
    {
        bType = type; bSize = size; bSpeed = spd; bDamage = dmg;
        bb.SetBB(bPosX - (bSize / 2), bPosY - (bSize / 2), bPosX + (bSize / 2), bPosY + (bSize / 2), TYPE_BULLET, bPlayer);
    }
    Bullet() {}

    void updateBB() {
        bb.Update(bPosX - (bSize / 2), bPosY - (bSize / 2), bPosX + (bSize / 2), bPosY + (bSize / 2));
    }

    BoundingBox GetBB() { return bb; }

    void update(float deltaTime, RECT gameGround) {
        bPosX += shootDir.x * bSpeed * deltaTime;
        if (bPosX > gameGround.right + 40 || bPosX < gameGround.left) {
            alive = false;
        }
        bPosY += shootDir.y * bSpeed * deltaTime;
        if (bPosY > gameGround.bottom + 40 || bPosY < gameGround.top) {
            alive = false;
        }
        updateBB();
    }
};

struct Bullet_Arr {
    Bullet arr[10];
};

struct Bullet_Alive_Arr {
    int arr[10];
};

Bullet_Arr arr_to_struct(Bullet* arr);
Bullet* struct_to_arr(Bullet_Arr str);

void send_Player(SOCKET, Player_Socket);

void send_Bullet(SOCKET sock, Bullet_Arr bullet);

Bullet_Arr recv_Bullet(SOCKET sock);

void send_Bullet_Alive(SOCKET sock, Bullet_Alive_Arr bullet);

Bullet_Alive_Arr recv_Bullet_Alive(SOCKET sock);

DWORD WINAPI ProcessClient(LPVOID);

void err_quit(const char* msg);
void err_display(const char* msg);
int recvn(SOCKET s, char* buf, int len, int flags);