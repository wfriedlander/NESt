#pragma once

#include "nest.h"


struct CartridgeConfig
{
	byte prg_size;
	byte chr_size;
	byte mirroring;
	byte persistent;
	byte trainer;
	byte four_screen;
};


class Cartridge
{
public:
	static std::unique_ptr<Cartridge> Load(std::string filename);

public:
	virtual byte CpuRead(word address) = 0;
	virtual void CpuWrite(word address, byte value) = 0;
	virtual byte PpuRead(word address) = 0;
	virtual void PpuWrite(word address, byte value) = 0;
};



