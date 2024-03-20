#pragma once

#include "../../mylib.h"
class Player
{
	CharacterPosition pos;
public:
	Player();
	Player(CharacterPosition);
	Player(float, float);
	CharacterPosition getPos();
	int getPosX();
	int getPosY();
	void move(float x, float y, int left, int right, int top, int bottom);
};

