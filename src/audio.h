#pragma once

#include <cstdint>

using AudioBuffer = int16_t[800];

class Audio
{
public:
	virtual void Update(const AudioBuffer& buffer) = 0;
};

