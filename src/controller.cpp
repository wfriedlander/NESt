#include "controller.h"

#include <SFML/Window.hpp>

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
	state0 |= sf::Keyboard::isKeyPressed(sf::Keyboard::F) ? 0x80 : 0;
	state0 |= sf::Keyboard::isKeyPressed(sf::Keyboard::D) ? 0x40 : 0;
	state0 |= sf::Keyboard::isKeyPressed(sf::Keyboard::RShift) ? 0x20 : 0;
	state0 |= sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) ? 0x10 : 0;
	state0 |= sf::Keyboard::isKeyPressed(sf::Keyboard::Up) ? 0x08 : 0;
	state0 |= sf::Keyboard::isKeyPressed(sf::Keyboard::Down) ? 0x04 : 0;
	state0 |= sf::Keyboard::isKeyPressed(sf::Keyboard::Left) ? 0x02 : 0;
	state0 |= sf::Keyboard::isKeyPressed(sf::Keyboard::Right) ? 0x01 : 0;
}

