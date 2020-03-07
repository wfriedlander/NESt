#include "bus.h"


Bus::Bus() : mCpuRam(2048)
{
}

void Bus::Initialize(NES* nes, CPU* cpu, PPU* ppu, APU* apu, Controller* control)
{
	mNES = nes;
	mCPU = cpu;
	mPPU = ppu;
	mAPU = apu;
	mController = control;
}

void Bus::LoadGame(Cartridge* cart)
{
	mCartridge = cart;
}

void Bus::Reset()
{
	mCPU->Reset();
	mPPU->Reset();
	mAPU->Reset();
}

void Bus::FrameComplete()
{
	mNES->FrameDone();
}

void Bus::TriggerNMI()
{
	mCPU->SetNMI();
}

void Bus::TriggerIRQ()
{
	mCPU->SetIRQ();
}


byte Bus::CpuRead(word address)
{
	switch (address >> 12)
	{
	case 0: 
	case 1: return mCpuRam[address & 0x07FF];

	case 2:
	case 3: return mPPU->RegisterRead(address & 0x7);

	case 4:
		switch (address & 0x3FFF)
		{
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
		case 0x09:
		case 0x0A:
		case 0x0B:
		case 0x0C:
		case 0x0D:
		case 0x0E:
		case 0x0F:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x15: return mAPU->RegisterRead(address & 0x1F);

		case 0x16: return mController->RegisterRead(0);
		case 0x17: return mController->RegisterRead(1);

		case 0x14:
		case 0x18:
		case 0x19:
		case 0x1A:
		case 0x1B:
		case 0x1C:
		case 0x1D:
		case 0x1E:
		case 0x1F: return 0;
		}

	default:
		return mCartridge->CpuRead(address);
	}
}

void Bus::CpuWrite(word address, byte value)
{
	switch (address >> 12)
	{
	case 0:
	case 1: 
		mCpuRam[address & 0x07FF] = value;
		break;

	case 2:
	case 3: 
		mPPU->RegisterWrite(address & 0x7, value);
		break;

	case 4:
		switch (address & 0x3FFF)
		{
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
		case 0x09:
		case 0x0A:
		case 0x0B:
		case 0x0C:
		case 0x0D:
		case 0x0E:
		case 0x0F:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x15:
		case 0x17:
			mAPU->RegisterWrite(address & 0x1F, value);
			break;

		case 0x16: 
			mController->RegisterWrite(value);
			break;

		case 0x14:
			mPPU->ActivateDMA(value);
			break;
		case 0x18:
		case 0x19:
		case 0x1A:
		case 0x1B:
		case 0x1C:
		case 0x1D:
		case 0x1E:
		case 0x1F: break;
		}

	default:
		mCartridge->CpuWrite(address, value);
		break;
	}
}

byte Bus::PpuRead(word address)
{
	return mCartridge->PpuRead(address);
}

void Bus::PpuWrite(word address, byte value)
{
	mCartridge->PpuWrite(address, value);
}


