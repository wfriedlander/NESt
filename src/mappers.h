#pragma once

#include "nest.h"
#include "cartridge.h"


template <int N>
struct CartSegment
{
	operator char* ()
	{
		return reinterpret_cast<char*>(&mem[0]);
	}
	byte mem[N];
};


class Mapper : public Cartridge
{
public:
	Mapper(CartridgeConfig& config, std::ifstream& rom);

	byte PpuRead(word address) override;
	void PpuWrite(word address, byte value) override;

protected:
	virtual byte PatternRead(word address) = 0;
	virtual void PatternWrite(word address, byte value) = 0;
	virtual byte NameRead(byte table, word address) = 0;
	virtual void NameWrite(byte table, word address, byte value) = 0;

private:
	byte PaletteRead(word address);
	void PaletteWrite(word address, byte value);

protected:
	std::vector<CartSegment<16384>> mPrgBank;
	std::vector<CartSegment<8192>> mChrBank;
	std::vector<byte> mPpuRam;
	std::vector<byte> mPaletteRam;
	byte mMirroring = 0;
	byte mPersistent = 0;
	byte mFourScreen = 0;
};


class Mapper00 : public Mapper
{
	using Mapper::Mapper;

public:
	byte CpuRead(word address) override;
	void CpuWrite(word address, byte value) override;

protected:
	byte PatternRead(word address) override;
	void PatternWrite(word address, byte value) override;
	byte NameRead(byte table, word address) override;
	void NameWrite(byte table, word address, byte value) override;
};
