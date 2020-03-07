#pragma once

#include "nest.h"
#include "bus.h"
#include "audio.h"


class APU
{
	struct pulse // write only
	{
		byte d = 0;    // duty cycle
		byte l = 0;    // length counter halt
		byte c = 0;    // constant volume
		byte v = 0;    // volume/envelope divider period
		void operator =(byte val) { d = (val >> 6) & 3; l = (val >> 5) & 1; c = (val >> 4) & 1; l = (val >> 0) & 15; }
	};

	struct pulsesweep // write only
	{
		byte e = 0;    // 
		byte p = 0;    // 
		byte n = 0;    // 
		byte s = 0;    // 
		void operator =(byte val) { e = (val >> 7) & 1; p = (val >> 4) & 7; n = (val >> 3) & 1; s = (val >> 0) & 7; }
	};

	struct framecount // write only
	{
		byte m = 0;    // sequencer mode - 0: 4 step; 1: 5 step
		byte i = 0;    // interrupt inhibit
		void operator =(byte val) { m = (val >> 7) & 1; i = (val >> 6) & 1; }
	};


public:
	APU(Bus* bus);
	void SetAudioBackend(Audio* audio);

public:
	void Execute(word cycles);
	void Reset();

public:
	byte RegisterRead(word address);
	void RegisterWrite(word address, byte value);

private:
	void AdvanceFrameSequence();
	void QuarterClock();
	void HalfClock();

private:
	Bus* mBus = nullptr;
	Audio* mAudio = nullptr;

private:
	pulse mPulseReg0;
	pulse mPulseReg1;
	pulsesweep mPulseSweepReg0;
	pulsesweep mPulseSweepReg1;
	word mPulseTimerReg0;
	word mPulseTimerReg1;
	byte mPulseLengthReg0;
	byte mPulseLengthReg1;
	
	framecount mFrameReg;

private:
	word mFrameCount = 0;
	byte mFramePosition = 0;

private:
	word mFrameTimings[5] = {
		7457,
		14913,
		22371,
		29829,
		37281
	};
};

