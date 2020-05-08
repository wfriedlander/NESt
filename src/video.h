#pragma once

#include <cstdint>


using ScreenBuffer = uint8_t[240][256];

class Video
{
public:
	virtual void Update(const ScreenBuffer& buffer) = 0;
};

