#include "apu.h"

#include <cstdio>
#include <iostream>

APU::APU(Bus* bus) : mBus(bus), mPulse0(0), mPulse1(1)
{
}

void APU::SetAudioBackend(Audio* audio)
{
	mAudio = audio;
}

void APU::Execute(word cycles)
{
	mPendingCycles += cycles;
	mFrameCount += cycles;
	if (mFrameCount >= mFrameTimings[mFramePosition])
	{
		ProcessSamples();
		AdvanceFrameSequence();
	}
}

void APU::FrameDone()
{
	mActualFrame++;
}

void APU::Reset()
{
	RegisterWrite(0x15, 0);
}

byte APU::RegisterRead(word address)
{
	ProcessSamples();
	switch (address)
	{
	case 0x15:
		// DMC INTERRUPT
		// IRQ INTERRUPT
		//
		// DMC BYTES LEFT > 0
		// NOISE LENGTH COUNTER > 0
		// TRIANGLE LENGTH COUNTER > 0
		// PULSE 1 LENGTH COUNTER > 0
		// PULSE 0 LENGTH COUNTER > 0
		return 0;
	default:
		return 0;
	}
}

void APU::RegisterWrite(word address, byte value)
{
	ProcessSamples();
	

	switch (address)
	{
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03: 
		mPulse0.RegisterWrite(address, value);
		break;
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
		mPulse1.RegisterWrite(address & 0x3, value);
		break;
	case 0x08:
	case 0x09:
	case 0x0A:
	case 0x0B:
		// mTriangle.RegisterWrite(address & 0x3, value);
		break;
	case 0x0C:
	case 0x0D:
	case 0x0E:
	case 0x0F:
		// mNoise.RegisterWrite(address & 0x3, value);
		break;
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
		// mDMC.RegisterWrite(address & 0x3, value);
		break;
	case 0x15:
		// mPulse0.Enable(value & 1);
		// mPulse1.Enable((value >> 1) & 1);
		// mTriangle.Enable((value >> 2) & 1);
		// mNoise.Enable((value >> 3) & 1);
		// mDMC.Enable((value >> 4) & 1);
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

void APU::AdvanceFrameSequence()
{
	if (mFrameReg.m) // 5 step sequence
	{
		switch (mFramePosition)
		{
		case 1:
		case 4:  HalfClock();
		case 0:
		case 2:  QuarterClock();
		default: break;
		}

		mFramePosition++;
		if (mFramePosition == 5)
		{
			mFramePosition = 0;
			mFrameCount = 0;
		}
			
	}
	else // 4 step sequence
	{
		switch (mFramePosition)
		{
		case 1:
		case 3:  HalfClock();
		case 0:
		case 2:  QuarterClock();
		default: break;
		}

		mFramePosition++;
		if (mFramePosition == 4)
		{
			mFramePosition = 0;
			//if (!mFrameReg.i)
			//	mBus->TriggerIRQ();
			mFrameCount = 0;
		}
	}
}

void APU::QuarterClock()
{
	mPulse0.QuarterStep();
	mPulse1.QuarterStep();
}

void APU::HalfClock()
{
	mPulse0.HalfStep();
	mPulse1.HalfStep();
}

void APU::ProcessSamples()
{
	mAudio->Update(mPulse0.ProcessSamples(mPendingCycles));
	mPendingCycles = 0;
}








Pulse::Pulse(byte channel) : mSamples(8192)
{
	mChannel = channel;
}

void Pulse::RegisterWrite(word address, byte value)
{
	switch (address)
	{
	case 0x00:
		mDuty = value >> 6;
		mLoop = value & 0x20;
		mConstantVolume = value & 0x10;
		mEnvelope = value & 0xF;
		break;
	case 0x01:
		mSweepEnabled = (value & 0x80) > 0;
		mSweepNeg = (value & 0x8) > 0;
		mSweepPeriod = (value >> 4) & 0x7;
		mSweepShift = value & 0x7;
		mMuted = (!mSweepNeg && mTimerReg >= 0x400);
		mSweepReload = true;
		break;
	case 0x02:
		mTimerReg &= 0xFF00;
		mTimerReg |= value;
		break;
	case 0x03:
		mTimerReg &= 0x00FF;
		mTimerReg |= ((value & 7) << 8);
		if (mEnabled)
			mLength = mLengthTable[value >> 3];
		mSequencePos = 0;
		mStart = true;
		break;
	default:
		break;
	}
}

const std::vector<byte>& Pulse::ProcessSamples(word cycles)
{
	mSamples.clear();
	bool nonzero = false;
	for (word s = 0; s < cycles; s++)
	{
		if (s & 1)
		{
			if (mTimer == 0)
			{
				mTimer = mTimerReg;
				mSequencePos = (mSequencePos + 1) & 0x7;
			}
			else
			{
				mTimer -= 1;
			}
		}

		if (mSequenceTable[mDuty][mSequencePos] && !mMuted && mTimerReg >= 8)
			mSamples.push_back(mConstantVolume ? mEnvelope : mDecay);
		else
			mSamples.push_back(0);

		if (mSamples[s] > 0)
			nonzero = true;

	}

	//std::cout << nonzero << "\n";

	return mSamples;
}

void Pulse::QuarterStep()
{
	if (mStart)
	{
		mStart = false;
		mDecay = 15;
		mDivider = mEnvelope;
	}
	else
	{
		if (mDivider == 0)
		{
			mDivider = mEnvelope;
			if (mDecay == 0)
			{
				if (mLoop)
				{
					mDecay = 15;
				}
			}
			else
			{
				mDecay -= 1;
			}
		}
		else
			mDivider -= 1;
	}
}

void Pulse::HalfStep()
{

	if (mSweep == 0)
	{
		mSweep = mSweepPeriod;
		if (mSweepEnabled)
		{
			word new_period = mTimerReg;
			if (mSweepNeg)
				new_period -= (mTimerReg >> mSweepShift) - mChannel;
			else
				new_period += (mTimerReg >> mSweepShift);

			if (new_period >= 0x800)
				mMuted = true;

			if (mSweepEnabled && mSweepShift && mLength && !mMuted && mTimerReg >= 8)
				mTimerReg = new_period;
		}
	}
	else
		mSweep -= 1;

	if (mSweepReload)
	{
		mSweepReload = false;
		mSweep = mSweepPeriod;
	}

	if (mLength > 0 && !mLoop)
		mLength -= 1;

}

void Pulse::Enable(bool enabled)
{
	mEnabled = enabled;
	if (!enabled)
		mLength = 0;
}

