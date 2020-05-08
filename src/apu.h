#pragma once

#include "nest.h"
#include "bus.h"
#include "audio.h"



struct AudioSample
{
	byte pulse0 = 0;
	byte pulse1 = 0;
	byte triangle = 0;
	byte noise = 0;
	byte dmc = 0;
};


class Pulse
{
public:
	Pulse(byte channel);

public:
	void RegisterWrite(word address, byte value);
	const std::vector<byte>& ProcessSamples(word cycles);
	void QuarterStep();
	void HalfStep();
	void Enable(bool enabled);

private:
	// ENVELOPE
	bool mStart = true;
	bool mConstantVolume = false;
	bool mLoop = false;
	byte mDecay = 0;
	byte mDivider = 0;
	byte mEnvelope = 0;

private:
	// TIMER
	word mTimer = 0;
	word mTimerReg = 0;
	
private:
	// SEQUENCER
	byte mDuty = 0;
	byte mSequencePos = 0;

private:
	// LENGTH COUNTER
	bool mEnabled = false;
	byte mLength = 0;

private:
	// SWEEP
	bool mSweepReload = false;
	bool mSweepEnabled = false;
	bool mSweepNeg = false;
	bool mMuted = false;
	byte mSweep = 0;
	byte mSweepPeriod = 0;
	byte mSweepShift = 0;
	byte mChannel = 0;

private:
	std::vector<byte> mSamples;

private:
	byte mLengthTable[32] = {
		10,  254,   20,    2,
		40,    4,   80,    6,
		160,   8,   60,   10,
		14,   12,   26,   14,
		12,   16,   24,   18,
		48,   20,   96,   22,
		192,  24,   72,   26,
		16,   28,   32,   30
	};

	byte mSequenceTable[4][8] = {
		{0, 1, 0, 0, 0, 0, 0, 0},
		{0, 1, 1, 0, 0, 0, 0, 0},
		{0, 1, 1, 1, 1, 0, 0, 0},
		{1, 0, 0, 1, 1, 1, 1, 1}
	};
};


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
	void FrameDone();
	void Reset();

public:
	byte RegisterRead(word address);
	void RegisterWrite(word address, byte value);

private:
	void AdvanceFrameSequence();
	void QuarterClock();
	void HalfClock();
	void ProcessSamples();

private:
	Bus* mBus = nullptr;
	Audio* mAudio = nullptr;

private:
	Pulse mPulse0;
	Pulse mPulse1;

private:
	framecount mFrameReg;

private:
	word mPendingCycles = 0;
	word mFrameCount = 0;
	byte mFramePosition = 0;
	int mActualFrame = 0;

private:
	word mFrameTimings[5] = {
		7457,
		14913,
		22371,
		29829,
		37281
	};


};

