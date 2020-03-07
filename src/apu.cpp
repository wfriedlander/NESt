#include "apu.h"

#include <cstdio>

APU::APU(Bus* bus) : mBus(bus)
{
}

void APU::SetAudioBackend(Audio* audio)
{
	mAudio = audio;
}

void APU::Execute(word cycles)
{
	mFrameCount += cycles;
	if (mFrameCount >= mFrameTimings[mFramePosition])
	{
		AdvanceFrameSequence();
	}
}

void APU::Reset()
{
}

byte APU::RegisterRead(word address)
{
	return 0;
}

void APU::RegisterWrite(word address, byte value)
{
	switch (address)
	{
	case 0x00: 
		mPulseReg0 = value;
		break;
	case 0x01:
		mPulseSweepReg0 = value;
		break;
	case 0x02:
		mPulseTimerReg0 &= 0xFF00;
		mPulseTimerReg0 |= value;
		break;
	case 0x03:
		mPulseTimerReg0 &= 0x00FF;
		mPulseTimerReg0 |= ((value & 7) << 8);
		mPulseLengthReg0 = value >> 3;
		break;
	case 0x04: 
		mPulseReg1 = value;
		break;
	case 0x05:
		mPulseSweepReg1 = value;
		break;
	case 0x06:
		mPulseTimerReg1 &= 0xFF00;
		mPulseTimerReg1 |= value;
		break;
	case 0x07:
		mPulseTimerReg1 &= 0x00FF;
		mPulseTimerReg1 |= ((value & 7) << 8);
		mPulseLengthReg1 = value >> 3;
		break;
	case 0x17: 
		mFrameReg = value;
		if (!mFrameReg.m && mFramePosition == 4)
			mFramePosition = 0;
		break;
	default:
		break;
	}
}

//pulse mPulseReg0;
//pulse mPulseReg1;
//pulsesweep mPulseSweepReg0;
//pulsesweep mPulseSweepReg1;
//word mPulseTimerReg0;
//word mPulseTimerReg1;
//byte mPulseLengthReg0;
//byte mPulseLengthReg1;

void APU::AdvanceFrameSequence()
{
	if (mFrameReg.m) // 5 step sequence
	{
		switch (mFramePosition)
		{
		case 1:
		case 4:
			HalfClock();
		case 0:
		case 2:
			QuarterClock();
		default:
			break;
		}

		mFramePosition++;
		if (mFramePosition == 5)
			mFramePosition = 0;
	}
	else // 4 step sequence
	{
		switch (mFramePosition)
		{
		case 1:
		case 3:
			HalfClock();
		case 0:
		case 2:
			QuarterClock();
		default:
			break;
		}

		mFramePosition++;
		if (mFramePosition == 4)
		{
			mFramePosition = 0;
			if (!mFrameReg.i)
				mBus->TriggerIRQ();
		}
	}
}

void APU::QuarterClock()
{
}

void APU::HalfClock()
{
}

