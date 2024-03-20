#include "Player.h"


Player::Player() {
	pos.x = 0;
	pos.y = 0;
}
Player::Player(CharacterPosition p) {
	pos = p;
}
Player::Player(float x, float y) {
	pos.x = x;
	pos.y = y;
}

CharacterPosition Player::getPos() {
	return pos;
}

int Player::getPosX() {
	return pos.x;
}

int Player::getPosY() {
	return pos.y;
}

void Player::move(float x, float y, int left, int right, int top, int bottom) {
	if (pos.x < right && pos.x > left) {
		pos.x += x;
	}
	if (pos.y < bottom && pos.y > top) {
		pos.y += y;
	}
}