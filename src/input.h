#pragma once



struct ButtonState
{
	bool a;
	bool b;
	bool select;
	bool start;
	bool up;
	bool down;
	bool left;
	bool right;
};


class Input
{
public:
	virtual const ButtonState& GetButtons() = 0;
};

