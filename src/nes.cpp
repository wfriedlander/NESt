#include "nes.h"

#include <iostream>
#include <chrono>
#include <thread>


NES::NES(Video* video, Audio* audio, Input* input) : mVideo(video), mAudio(audio), mInput(input)
{
	mBus = std::make_unique<Bus>();
	mCPU = std::make_unique<CPU>(mBus.get());
	mPPU = std::make_unique<PPU>(mBus.get());
	mAPU = std::make_unique<APU>(mBus.get());
	mController = std::make_unique<Controller>();

	mPPU->SetVideoBackend(mVideo);
	mAPU->SetAudioBackend(mAudio);
	mController->SetInputBackend(mInput);

	mBus->Initialize(this, mCPU.get(), mPPU.get(), mAPU.get(), mController.get());
}

bool NES::LoadGame(std::string filename)
{
	mCartridge = Cartridge::Load(filename);
	mBus->LoadGame(mCartridge.get());
	mBus->Reset();
	mLast = std::chrono::high_resolution_clock::now();
	return mCartridge ? true: false;
}

void NES::ResetGame()
{
	mBus->Reset();
}

void NES::Step()
{
	byte cycles = mCPU->Step();
	mPPU->Execute(cycles * 3);
	mAPU->Execute(cycles);
}

void NES::FrameDone()
{
	auto elapsed = std::chrono::high_resolution_clock::now() - mLast;
	std::this_thread::sleep_for(std::chrono::milliseconds(16) - elapsed);
	auto e = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
	std::this_thread::sleep_for(std::chrono::microseconds(mMicro) - elapsed);
	auto post = std::chrono::high_resolution_clock::now() - mLast;
	auto p = std::chrono::duration_cast<std::chrono::microseconds>(post).count();
	mMicro += (16666 - p);
	mLast = std::chrono::high_resolution_clock::now();
}

void NES::DebugKey(int key)
{
	//std::cout << key << "\n";
	if (key == 22)
		mCPU->EnableLogging(true);
	else if (key == 16)
		mCPU->EnableLogging(false);
}
