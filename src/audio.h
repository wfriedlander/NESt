#pragma once

#include <cstdint>

using AudioBuffer = std::vector<float>;

class Audio
{
public:
	virtual void Update(const AudioBuffer& buffer) = 0;
};

