#pragma once

#include "nest.h"

#include "nes.h"
#include "cpu.h"
#include "ppu.h"
#include "apu.h"
#include "cartridge.h"
#include "controller.h"


class NES;


class Bus
{
public:
	Bus();

public:
	void Initialize(NES* nes, CPU* cpu, PPU* ppu, APU* apu, Controller* control);
	void LoadGame(Cartridge* cart);
	void Reset();

public:
	void FrameComplete();
	void TriggerNMI();
	void TriggerIRQ();

public:
	byte CpuRead(word address);
	void CpuWrite(word address, byte value);
	byte PpuRead(word address);
	void PpuWrite(word address, byte value);

private:
	NES* mNES = nullptr;
	CPU* mCPU = nullptr;
	PPU* mPPU = nullptr;
	APU* mAPU = nullptr;
	Cartridge* mCartridge = nullptr;
	Controller* mController = nullptr;

private:
	std::vector<byte> mCpuRam;
};