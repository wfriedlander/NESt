#include "ppu.h"

#include <iostream>


PPU::PPU(Bus* bus) : mBus(bus)
{
	
}

void PPU::SetVideoBackend(Video* video)
{
	mVideo = video;
}

void PPU::Execute(word cycles)
{
	mCycle += cycles;
	if (mCycle > 340)
	{
		mCycle -= 341;
		mLine += 1;
		mLineExecuted = false;
	}

	if (mLine < 240 && mZeroLine)
	{
		if (mLine > mZeroLine || (mLine == mZeroLine && mCycle >= mZeroPixel))
		{
			mStatusReg.s = 1;
		}
	}
	

	if (!mLineExecuted)
	{
		mLineExecuted = true;
		if (mLine < 240)
		{
			RenderLine2(mLine);
		}
		else if (mLine == 241)
		{
			mVideo->Update(mScreen);
			mStatusReg.v = 1;
			if (mControlReg.v)
			{
				mBus->TriggerNMI();
			}
		}
		else if (mLine == 261)
		{
			mRegV &= ~0x7BE0;
			mRegV |= (mRegT & 0x7BE0);
			mZeroLine = 0;
			mZeroPixel = 0;
			mStatusReg = 0;
		}
		else if (mLine == 262)
		{
			mLine = 0;
			mFrame += 1;
			mBus->FrameComplete();
		}
	}
}

void PPU::Reset()
{
	mCycle = 0;
	mLine = 0;
	mFrame = 0;
	mStatusReg = 0;
	mControlReg = 0;
	mMaskReg = 0;
	mLatch = 0;
	mFirstWrite = true;
}

byte PPU::RegisterRead(word address)
{
	switch (address)
	{
	case 0: break;
	case 1: break;
	case 2: 
		mLatch = mStatusReg | (mLatch & 0x1F);
		mStatusReg.v = 0;
		mFirstWrite = true;
		mRegW = 0;
		break;
	case 3: break;
	case 4: 
		mLatch = mOAM[mAddrOAM];
		mAddrOAM += 1;
		break;
	case 5: break;
	case 6: break;
	case 7:
		mLatch = mVbuffer;
		mVbuffer = Read(mVaddr);
		if (mVaddr > 0x3EFF && mVaddr < 0x4000)
		{
			mLatch = mVbuffer;
			mVbuffer = Read(mVaddr & 0x2FFF);
		}
		mVaddr += mControlReg.i ? 32 : 1;

		
		mRegV += mControlReg.i ? 32 : 1;


		break;
	default: break;
	}
	return mLatch;
}

void PPU::RegisterWrite(word address, byte value)
{
	switch (address)
	{
	case 0: 
		mControlReg = value;
		mRegT = (mRegT & ~0xC00) | ((value & 0x3) << 10);
		break;
	case 1: mMaskReg = value; break;
	case 2: break;
	case 3: mAddrOAM = value; break;
	case 4: mOAM[mAddrOAM] = value; break;
	case 5:
		if (!mRegW)
		{
			mRegX = value & 0x7;
			mRegT = (mRegT & 0xFFE0) | ((value & 0xF8) >> 3);
		}
		else
		{
			mRegT &= 0xC1F;
			mRegT |= ((value & 0xF8) << 2);
			mRegT |= ((value & 0x07) << 12);
		}
		mRegW = mRegW ^ 1;
		break;
	case 6:
		if (mRegW)
		{
			mRegT &= 0xFF;
			mRegT |= (value & 0x3F) << 8;
		}
		else
		{
			mRegT &= 0xFF00;
			mRegT |= value;
			mRegV = mRegT;
		}
		mRegW = mRegW ^ 1;

		if (mFirstWrite)
			mVaddr = (value << 8) | (mVaddr & 0xFF);
		else
			mVaddr = (mVaddr & 0xFF00) | value;
		mFirstWrite = !mFirstWrite;
		break;
	case 7:
		Write(mVaddr, value);
		mVaddr += mControlReg.i ? 32 : 1;
		mRegV += mControlReg.i ? 32 : 1;
	default: break;
	}
}

void PPU::ActivateDMA(byte address)
{
	for (word i = 0; i < 256; i++)
	{
		mOAM[i] = mBus->CpuRead(Word(address, i & 0xFF));
	}
	mCycle += 256 * 2 * 3;
}

void PPU::Write(word address, byte value)
{
	mBus->PpuWrite(address, value);
}

byte PPU::Read(word address)
{
	mVaddr = address;
	return mBus->PpuRead(address);
}

//void PPU::RenderLine(word line)
//{
//	if (mMaskReg.sb)
//	{
//		//std::cout << (int)line << " " << (int)mControlReg.n << " " << (int)scroll_x << "\n";
//
//		// THIS ISN'T RIGHT, NEED TO BLANK OUT LEFT 8 PIXELS
//		//byte start = mMaskReg.sb ? 0 : 1;
//
//		byte start = scroll_x & 0x7;
//		for (int x = 0; x < 33; x++)
//		{
//			auto [n, a] = NameTable2(mControlReg.n, line, x);
//			//auto [n, a] = NameTable(mControlReg.n, ((line & 0xFFF8) * 4) + x);
//			word p = PatternTable(mControlReg.b, n, line & 0x7);
//
//			for (int px = 0; px < 8; px++)
//			{
//				byte color_index = (p >> ((7 - px) * 2)) & 0x3;
//				byte palette_index = color_index ? a << 2 : 0;
//				word color_addr = 0x3F00 | palette_index | color_index;
//
//				int x_adj = ((x * 8) + px) - (scroll_x & 0x7);
//				if (x_adj >= 255)
//				{
//					break;
//				}
//				else if (x_adj >= 0)
//				{
//					mScreen[x_adj][line] = Read(color_addr) & 0x3F;
//					mDepth[x_adj][line] = color_index;
//				}
//				//mScreen[(x * 8) + px][line] = Read(color_addr) & 0x3F;
//			}
//		}
//	}
//	//else
//	//{
//	//	byte bg_color = mBus->PpuRead(0x3F00) & 0x3F;
//	//	//std::cout << (int)bg_color << "\n";
//	//	for (int i = 0; i < 256; i++)
//	//	{
//	//		mScreen[i][line] = bg_color;
//	//	}
//	//}
//
//	if (mMaskReg.ss)
//	{
//		oam* primary = reinterpret_cast<oam*>(&mOAM);
//		oam secondary[8];
//		byte oam_pos = 0;
//		bool zero = false;
//
//		for (byte i = 0; i < 64; i++)
//		{
//			if (primary[i].y < line && primary[i].y + 8 >= line)
//			{
//				if (oam_pos < 8)
//				{
//					if (i == 0)
//					{
//						zero = true;
//					}
//					secondary[oam_pos++] = primary[i];
//				}
//				else
//				{
//					mStatusReg.o = 1;
//					break;
//				}
//			}
//		}
//
//		for (byte i = 0; i < oam_pos; i++)
//		{
//			if (i > 0)
//			{
//				zero = false;
//			}
//			
//			auto& sprite = secondary[i];
//			int ypx = (line - sprite.y - 1) & 0x7;
//			if (sprite.a & 0x80)
//				ypx = 7 - ypx;
//			word p = PatternTable(mControlReg.s, sprite.t, ypx);
//			for (int px = 7; px >= 0; px--)
//			{
//				byte dx = sprite.a & 0x40 ? px : (7 - px);
//				if (sprite.x + dx < 256)
//				{
//					byte color_index = (p >> (px * 2)) & 0x3;
//					byte palette_index = (sprite.a & 0x3) << 2;
//					word color_addr = 0x3F10 | palette_index | color_index;
//
//					bool opaque = color_index > 0;
//					bool foreground = !(sprite.a & 0x20);
//					bool background = mDepth[sprite.x + dx][line] > 0;
//
//					if ((foreground || !background) && opaque)
//					//if (opaque)
//					{
//						mScreen[sprite.x + dx][line] = Read(color_addr) & 0x3F;
//					}
//
//					//if (zero && opaque && background)
//					if (zero && opaque)
//					{
//						mZeroPixel = sprite.x + px;
//						mZeroLine = line;
//						//if (mStatusReg.s == 0)
//							//std::cout << "l: " << (int)line << "   "<< "p: " << (int)mZeroPixel << std::endl;
//
//						mStatusReg.s = 1;
//					}
//				}
//			}
//		}
//	}
//}

void PPU::RenderLine2(word line)
{
	if (mMaskReg.sb)
	{
		int pixel = -1 * mRegX;
		while (pixel < 256)
		{
			word name_addr = 0x2000 | (mRegV & 0xFFF);
			word attr_addr = 0x23C0 | (mRegV & 0x0C00) | ((mRegV >> 4) & 0x38) | ((mRegV >> 2) & 0x07);

			byte name = Read(name_addr);
			byte attr = Read(attr_addr);

			byte aty = (mRegV >> 5) & 0x2;
			byte atx = (mRegV >> 1) & 0x1;
			byte a = (attr >> (2 * (aty | atx))) & 0x3;

			word p = PatternTable(mControlReg.b, name, (mRegV >> 12) & 0x7);

			for (int px = 0; px < 8; px++)
			{
				byte color_index = (p >> ((7 - px) * 2)) & 0x3;
				byte palette_index = color_index ? a << 2 : 0;
				word color_addr = 0x3F00 | palette_index | color_index;

				int x_adj = (pixel + px);
				if (x_adj >= 0 && x_adj < 256)
				{
					mScreen[x_adj][line] = Read(color_addr) & 0x3F;
					mDepth[x_adj][line] = color_index;
				}
			}

			if ((mRegV & 0x1F) == 31)
			{
				mRegV &= ~0x1F;
				mRegV ^= 0x400;
			}
			else
			{
				mRegV += 1;
			}

			pixel += 8;
		}



		//for (int x = 0; x < 32; x++)
		//{
		//	word name_addr = 0x2000 | (mRegV & 0xFFF);
		//	word attr_addr = 0x23C0 | (mRegV & 0x0C00) | ((mRegV >> 4) & 0x38) | ((mRegV >> 2) & 0x07);

		//	byte name = Read(name_addr);
		//	byte attr = Read(attr_addr);

		//	byte aty = (mRegV >> 5) & 0x2;
		//	byte atx = (mRegV >> 1) & 0x1;
		//	byte a = (attr >> (2 * (aty | atx))) & 0x3;

		//	word p = PatternTable(mControlReg.b, name, (mRegV >> 12) & 0x7);

		//	for (int px = 0; px < 8; px++)
		//	{
		//		byte color_index = (p >> ((7 - px) * 2)) & 0x3;
		//		byte palette_index = color_index ? a << 2 : 0;
		//		word color_addr = 0x3F00 | palette_index | color_index;

		//		int x_adj = ((x * 8) + px);
		//		mScreen[x_adj][line] = Read(color_addr) & 0x3F;
		//		mDepth[x_adj][line] = color_index;
		//	}

		//	if ((mRegV & 0x1F) == 31)
		//	{
		//		mRegV &= ~0x1F;
		//		mRegV ^= 0x400;
		//	}
		//	else
		//	{
		//		mRegV += 1;
		//	}
		//}

		if ((mRegV & 0x7000) != 0x7000)
		{
			mRegV += 0x1000;
		}
		else
		{
			mRegV &= ~0x7000;
			byte y = (mRegV & 0x03E0) >> 5;
			if (y == 29)
			{
				y = 0;
				mRegV ^= 0x0800;
			}
			else if (y == 31)
			{
				y = 0;
			}
			else
			{
				y += 1;
			}
			mRegV = (mRegV & ~0x03E0) | (y << 5); 
		}

		mRegV &= 0xFBE0;
		mRegV |= (mRegT & 0x41F);
	}

	if (mMaskReg.ss)
	{
		oam* primary = reinterpret_cast<oam*>(&mOAM);
		oam secondary[8];
		byte oam_pos = 0;
		bool zero = false;

		for (byte i = 0; i < 64; i++)
		{
			if (primary[i].y < line && primary[i].y + 8 >= line)
			{
				if (oam_pos < 8)
				{
					if (i == 0)
						zero = true;
					secondary[oam_pos++] = primary[i];
				}
				else
				{
					mStatusReg.o = 1;
					break;
				}
			}
		}

		for (byte i = 0; i < oam_pos; i++)
		{
			if (i > 0)
				zero = false;

			auto& sprite = secondary[i];
			int ypx = (line - sprite.y - 1) & 0x7;
			if (sprite.a & 0x80)
				ypx = 7 - ypx;
			word p = PatternTable(mControlReg.s, sprite.t, ypx);
			for (int px = 7; px >= 0; px--)
			{
				byte dx = sprite.a & 0x40 ? px : (7 - px);
				if (sprite.x + dx < 256)
				{
					byte color_index = (p >> (px * 2)) & 0x3;
					byte palette_index = (sprite.a & 0x3) << 2;
					word color_addr = 0x3F10 | palette_index | color_index;

					bool opaque = color_index > 0;
					bool foreground = !(sprite.a & 0x20);
					bool background = mDepth[sprite.x + dx][line] > 0;

					if ((foreground || !background) && opaque)
					{
						mScreen[sprite.x + dx][line] = Read(color_addr) & 0x3F;
					}

					if (zero && opaque && background && mZeroLine == 0)
					//if (zero && opaque)
					{
						mZeroPixel = sprite.x + dx;
						mZeroLine = line;
						std::cout << (int)mZeroLine << " " << (int)mZeroPixel << "\n";
						//mStatusReg.s = 1;
					}
				}
			}
		}
	}
}

word PPU::PatternTable(byte side, word tile, byte row)
{
	byte l = Read((side << 12) | (tile << 4) | (0 << 3) | row);
	byte h = Read((side << 12) | (tile << 4) | (1 << 3) | row);

	word pattern = 0;
	pattern |= (h & 0x80) << 8;
	pattern |= (h & 0x40) << 7;
	pattern |= (h & 0x20) << 6;
	pattern |= (h & 0x10) << 5;
	pattern |= (h & 0x08) << 4;
	pattern |= (h & 0x04) << 3;
	pattern |= (h & 0x02) << 2;
	pattern |= (h & 0x01) << 1;
	pattern |= (l & 0x80) << 7;
	pattern |= (l & 0x40) << 6;
	pattern |= (l & 0x20) << 5;
	pattern |= (l & 0x10) << 4;
	pattern |= (l & 0x08) << 3;
	pattern |= (l & 0x04) << 2;
	pattern |= (l & 0x02) << 1;
	pattern |= (l & 0x01) << 0;
	return pattern;
}

std::tuple<byte, byte> PPU::NameTable(byte table, word tile)
{
	word base = 0x2000 + (0x400 * table);
	word at_base = base + 0x3C0;

	byte row = (tile >> 4) & 0xF8;
	byte col = (tile & 0x1F) >> 2;
	byte at = Read(at_base + (row | col));

	row = (tile >> 5) & 0x2;
	col = (tile >> 1) & 0x1;
	byte p = (at >> (2 * (row | col))) & 0x3;

	return std::make_pair(Read(base + tile), p);
}

std::tuple<byte, byte> PPU::NameTable2(byte table, word line, byte tile)
{
	word base = 0x2000 + (0x400 * table);

	tile += (scroll_x >> 3);
	if (tile > 31)
	{
		base = (base + 0x400) & 0x2C00;
		tile &= 0x1F;
	}
	
	word at_base = base + 0x3C0;

	// line / 8 (>> 3) gets y tile number
	// y / 4 (>> 2) gets attribute table entry
	// at * 8 gets address location for y
	// combined that's >> 2 and mask at 111000
	byte y = (line >> 2) & 0x38;

	// tile number / 4 gets attribute table entry
	// it's the low 3 bits of address so just mask it
	byte x = (tile >> 2) & 0x07;

	byte at = Read(at_base + (y | x));


	// essentially divide tile by 2
	// and look at low bit to get quadrant
	y = (line >> 3) & 0x2;
	x = (tile >> 1) & 0x1;
	byte p = (at >> (2 * (y | x))) & 0x3;

	// y tile is line >> 3, but address
	// location is << 5, so << 2 with mask
	word nt = ((line << 2) & 0xFFE0) | tile;

	return std::make_pair(Read(base + nt), p);
}
