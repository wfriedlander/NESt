#include "mappers.h"



Mapper::Mapper(CartridgeConfig& config, std::ifstream& rom) : mPpuRam(2048), mPaletteRam(32)
{
	for (int i = 0; i < config.prg_size; i++)
	{
		mPrgBank.emplace_back();
		rom.read(mPrgBank.back(), 16384);
	}

	for (int i = 0; i < config.chr_size; i++)
	{
		mChrBank.emplace_back();
		rom.read(mChrBank.back(), 8192);
	}

	mMirroring = config.mirroring;
	mPersistent = config.persistent;
	mFourScreen = config.four_screen;
}


byte Mapper::PpuRead(word address)
{
	switch (address >> 12)
	{
	case 0: 
	case 1: return PatternRead(address & 0x1FFF);
	case 2: return NameRead((address >> 10) & 0x3, address & 0x03FF);
	case 3: if (address & 0x0100) return PaletteRead(address & 0x1F);
	default: return PpuRead(address - 0x1000);
	}
}

void Mapper::PpuWrite(word address, byte value)
{
	switch (address >> 12)
	{
	case 0: 
	case 1: 
		PatternWrite(address & 0x1FFF, value);
		break;
	case 2: 
		NameWrite((address >> 10) & 0x3, address & 0x03FF, value);
		break;
	case 3: 
		if (address & 0x0100) PaletteWrite(address & 0x1F, value);
		break;
	default:
		PpuWrite(address - 0x1000, value);
		break;
	}
}

byte Mapper::PaletteRead(word address)
{
	word adjusted_addr = address & 0x3 ? address : address & 0xF;
	return mPaletteRam[adjusted_addr];
}

void Mapper::PaletteWrite(word address, byte value)
{
	word adjusted_addr = address & 0x3 ? address : address & 0xF;
	mPaletteRam[adjusted_addr] = value;
}







byte Mapper00::CpuRead(word address)
{
	if (address & 0x8000)
	{
		auto bank = 0;
		if (mPrgBank.size() > 1 && (address & 0x4000))
			bank = 1;
		return mPrgBank[bank][address & 0x3FFF];
	}
}

void Mapper00::CpuWrite(word address, byte value)
{

}

byte Mapper00::PatternRead(word address)
{
	return mChrBank[0][address];
}

void Mapper00::PatternWrite(word address, byte value)
{
	mChrBank[0][address] = value;
}

byte Mapper00::NameRead(byte table, word address)
{
	word adjusted_addr = (mMirroring ? (table & 1) << 10 : (table & 2) << 9) | address;
	return mPpuRam[adjusted_addr];
}

void Mapper00::NameWrite(byte table, word address, byte value)
{
	word adjusted_addr = (mMirroring ? (table & 1) << 10 : (table & 2) << 9) | address;
	mPpuRam[adjusted_addr] = value;
}


