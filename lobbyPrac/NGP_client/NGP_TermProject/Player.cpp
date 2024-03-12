#include "Player.h"
#include "Global.h"

Player::Player() {}

Player::Player(float x, float y) { posX = x; posY = y; maxMp = 10; mp = 0; maxHp = 10; hp = maxHp; bb.SetBB(posX,posY,posX+40,posY+40,TYPE_PLAYER, playerNo); }

Player::~Player() {}

void Player::move(int dirx, int diry, float deltaTime) {
	posX += (dirx * velocity * deltaTime);
	posY += (diry * velocity * deltaTime);
}

bool Player::shoot(int stX, int stY, int dstX, int dstY, float deltaTime) {

	float fx = float(dstX - stX), fy = float(dstY - stY);
	//
	bullets[bulletCount] = nowBullet;
	bullets[bulletCount].shootDir.x = fx / sqrt(fx * fx + fy * fy);
	bullets[bulletCount].shootDir.y = fy / sqrt(fx * fx + fy * fy);
	
	bullets[bulletCount].alive = true;
	bullets[bulletCount].bPosX = stX;
	bullets[bulletCount].bPosY = stY;
	bullets[bulletCount].bPlayer = playerNo;
	
	if (bulletCount == 9) {
		bulletCount = 0;
	}
	else {
		bulletCount++;
	}

	return true;
}

void Player::setPos(float x, float y) {
	posX = x;
	posY = y;
}

void Player::subMp(int val) {
	if(mp > 0)
		mp -= val;
}

void Player::subHp(int val) {
	if (hp > 0)
		hp -= 1;
}

void Player::setHp(int p_hp) {
	hp = p_hp;
}