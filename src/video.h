#pragma once

#include <cstdint>


using ScreenBuffer = uint8_t[256][240];

class Video
{
public:
	virtual void Update(const ScreenBuffer& buffer) = 0;
};

