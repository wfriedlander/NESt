#include "controller.h"

#include <cstdio>


Controller::Controller()
{
}

void Controller::SetInputBackend(Input* input)
{
	mInput = input;
}

byte Controller::RegisterRead(word address)
{
	if (address == 0)
	{
		byte r = state0 >> 7;
		state0 = state0 << 1;
		return r;
	}
	else
	{
		byte r = state1 >> 7;
		state1 = state1 << 1;
		return r;
	}
	
}

void Controller::RegisterWrite(byte value)
{
	if (value & 1)
	{
		Latch();
	}
}

void Controller::Latch()
{
	state0 = 0;
	auto buttons = mInput->GetButtons();
	
	state0 |= buttons.a ? 0x80 : 0;
	state0 |= buttons.b ? 0x40 : 0;
	state0 |= buttons.select ? 0x20 : 0;
	state0 |= buttons.start ? 0x10 : 0;
	state0 |= buttons.up ? 0x08 : 0;
	state0 |= buttons.down ? 0x04 : 0;
	state0 |= buttons.left ? 0x02 : 0;
	state0 |= buttons.right ? 0x01 : 0;
}


