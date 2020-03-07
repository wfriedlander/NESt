#pragma once

#include "nest.h"
#include "input.h"


class Controller
{
public:
	Controller();
	void SetInputBackend(Input* input);

public:
	byte RegisterRead(word address);
	void RegisterWrite(byte value);

private:
	void Latch();

private:
	Input* mInput = nullptr;
	byte state0 = 0;
	byte state1 = 0;
};

