#pragma once

#include "nest.h"
#include "bus.h"
#include "video.h"


class Bus;


class PPU
{
	struct ppuctrl //write only
	{
		byte v = 0;     // generate NMI
		byte p = 0;     // master/slave
		byte h = 0;     // sprite size (0: 8x8 pixels; 1: 8x16 pixels)
		byte b = 0;     // background pattern table address (0: $0000; 1: $1000)
		byte s = 0;     // sprite pattern table address for 8x8 sprites (0: $0000; 1: $1000; ignored in 8x16 mode)
		byte i = 0;     // VRAM address increment per CPU read/write of PPUDATA (0: add 1, going across; 1: add 32, going down)
		byte n = 0;     // Base nametable address (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00)
		void operator =(byte val) { v = (val >> 7) & 0x01; p = (val >> 6) & 0x01; h = (val >> 5) & 0x01; b = (val >> 4) & 0x01; s = (val >> 3) & 0x01; i = (val >> 2) & 0x01; n = val & 0x03; }
	};
	struct ppumask //write only
	{
		byte eb = 0;    // emphasize blue
		byte eg = 0;    // emphasize green
		byte er = 0;    // emphasize red
		byte ss = 0;    // show sprites
		byte sb = 0;    // show background
		byte us = 0;    // unmask left 8 sprite pixels
		byte ub = 0;    // unmask left 8 background pixels
		byte gs = 0;    // grayscale
		void operator =(byte val) { eb = (val >> 7) & 0x01; eg = (val >> 6) & 0x01; er = (val >> 5) & 0x01; ss = (val >> 4) & 0x01; sb = (val >> 3) & 0x01; us = (val >> 2) & 0x01; ub = (val >> 1) & 0x01; gs = val & 0x01; }
	};
	struct ppustatus //read only
	{
		byte v = 0;     // vertical blank started
		byte s = 0;     // sprite 0 hit
		byte o = 0;     // sprite overflow
		void operator =(byte val) { v = (val >> 7) & 0x01; s = (val >> 6) & 0x01; o = (val >> 5) & 0x01; }
		operator byte() const { return (v << 7 | s << 6 | o << 5); }
	};
	struct oam
	{
		byte y;
		byte t;
		byte a;
		byte x;
	};
public:
	PPU(Bus* bus);
	void SetVideoBackend(Video* video);

public:
	void Execute(word cycles);
	void Reset();

public:
	byte RegisterRead(word address);
	void RegisterWrite(word address, byte value);
	void ActivateDMA(byte address);

private:
	void Write(word address, byte value);
	byte Read(word address);

private:
	void RenderLine(word line);
	word PatternTable(byte side, word tile, byte row);

private:
	Bus* mBus;
	Video* mVideo;

private:
	ppuctrl mControlReg;
	ppumask mMaskReg;
	ppustatus mStatusReg;
	
private:
	word mRegT;
	word mRegV;
	byte mRegX;
	byte mRegY;
	byte mRegW;


private:
	byte mLatch = 0;
	bool mFirstWrite = true;

	word mVaddr = 0;
	byte mVbuffer = 0;

	byte mAddrOAM = 0;
	byte mOAM[256] = { 0 };

	word mCycle = 0;
	word mLine = 0;
	word mFrame = 0;
	bool mLineExecuted = false;

	word mZeroPixel = 0;
	word mZeroLine = 0;

	byte mScreen[240][256] = { 0 };
	byte mDepth[240][256] = { 0 };

};

