#pragma once

#include <cstdint>

using AudioBuffer = std::vector<uint8_t>;

class Audio
{
public:
	virtual void Update(const AudioBuffer& buffer) = 0;
};

