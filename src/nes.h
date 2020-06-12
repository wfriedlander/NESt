#pragma once

#include "nest.h"

#include "bus.h"
#include "cpu.h"
#include "ppu.h"
#include "apu.h"
#include "cartridge.h"
#include "controller.h"

#include "video.h"
#include "audio.h"
#include "input.h"


class Bus;
class CPU;
class PPU;
class APU;
class Cartridge;
class Controller;


class NES
{
public:
	NES(Video* video, Audio* audio, Input* input);

public:
	bool LoadGame(std::string filename);
	void ResetGame();

	void Step();
	void FrameDone();

public:
	void DebugKey(int key);

private:
	std::unique_ptr<Bus> mBus;
	std::unique_ptr<CPU> mCPU;
	std::unique_ptr<PPU> mPPU;
	std::unique_ptr<APU> mAPU;
	std::unique_ptr<Cartridge> mCartridge;
	std::unique_ptr<Controller> mController;

	Video* mVideo = nullptr;
	Audio* mAudio = nullptr;
	Input* mInput = nullptr;

	std::chrono::time_point<std::chrono::high_resolution_clock> mLast;
	int mMicro = 16666;

};

