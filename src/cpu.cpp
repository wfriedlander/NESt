#include "cpu.h"

#include <iostream>
#include <fstream>
#include <cstdio>
#include <functional>


CPU::CPU(Bus* bus) : bus(bus)
{
	auto e = fopen_s(&pFile, "log.txt", "w");
	reg.pc = 0x400;
}

CPU::~CPU()
{
	fclose(pFile);
}

void CPU::Reset()
{
	reg.sr.b = 0;
	auto [h, l] = Bytes(reg.pc);
	reg.sp--;
	reg.sp--;
	reg.sp--;
	reg.pc = Word(Read(0xFFFD), Read(0xFFFC));
	reg.sr.i = 1;
}

void CPU::SetNMI()
{
	mNMI = true;
}

void CPU::SetIRQ()
{
	mIRQ = true;
}

void CPU::DMA(byte address)
{
	for (word i = 0; i < 256; i++)
	{
		Write(0x2003, i & 0xFF);
		Write(0x2004, Read(Word(address, i & 0xFF)));
	}
}

void CPU::NMI()
{
	reg.sr.b = 0;
	auto [h, l] = Bytes(reg.pc);
	Write(Word(0x01, reg.sp--), h);
	Write(Word(0x01, reg.sp--), l);
	Write(Word(0x01, reg.sp--), reg.sr);
	reg.pc = Word(Read(0xFFFB), Read(0xFFFA));
	reg.sr.i = 1;

	if (mLog)
		fprintf(pFile, "######################################################################################################\n\n");
}

void CPU::IRQ()
{
	reg.sr.b = 0;
	auto [h, l] = Bytes(reg.pc);
	Write(Word(0x01, reg.sp--), h);
	Write(Word(0x01, reg.sp--), l);
	Write(Word(0x01, reg.sp--), reg.sr);
	reg.pc = Word(Read(0xFFFF), Read(0xFFFE));
	reg.sr.i = 1;
}

void CPU::Log(Instruction& ins)
{
	byte status = reg.sr;

	if (reg.pc == 0x8057)
		return;
	
	fprintf(pFile, "%d %s %s ", total_cycles, ins.op_name.c_str(), ins.addr_name.c_str());
	for (int i = 1; i < ins.bytes; i++)
	{
		fprintf(pFile, "%02X ", Read(reg.pc + i));
	}
	
	fprintf(pFile, "\n\n PC  AC XR YR SR SP  N V - B D I Z C\n");
	fprintf(pFile, "%04X %02X %02X %02X %02X %02X  %d %d %d %d %d %d %d %d\n\n",
		reg.pc, reg.a, reg.x, reg.y, status, reg.sp,
		reg.sr.n, reg.sr.v, 1, reg.sr.b, reg.sr.d, reg.sr.i, reg.sr.z, reg.sr.c);

	//fprintf(pFile, "\n\n");
}

byte CPU::Step()
{
	if (mNMI)
	{
		mNMI = false;
		NMI();
		return 7;
	}

	if (mIRQ)
	{
		mIRQ = false;
		IRQ();
		return 7;
	}

	auto& instruction = opcode[Read(reg.pc)];
	if (mLog)
		Log(instruction);
	reg.pc += 1;
	byte cycles = instruction.cycles + ((this)->*(instruction.addr))(instruction.op);
	total_cycles += cycles;
	return cycles;
}

void CPU::EnableLogging(bool enabled)
{
	mLog = enabled;
}

///////////////////////////////////////////////////////////////////
// READ/WRITE
///////////////////////////////////////////////////////////////////

void CPU::Write(word address, byte value)
{
	bus->CpuWrite(address, value);
}

byte CPU::Read(word address)
{
	return bus->CpuRead(address);
}


///////////////////////////////////////////////////////////////////
// ADDRESSING
///////////////////////////////////////////////////////////////////
byte CPU::Implied(const op_func& op)
{
	std::invoke(op, this, 0);
	return 0;
}

byte CPU::Immediate(const op_func& op)
{
	word address = reg.pc;
	reg.pc += 1;
	std::invoke(op, this, address);
	return 0;
}

byte CPU::Zeropage(const op_func& op)
{
	word address = Read(reg.pc);
	reg.pc += 1;
	std::invoke(op, this, address);
	return 0;
}

byte CPU::Zeropage_X(const op_func& op)
{
	word address = Byte(Read(reg.pc) + reg.x);
	reg.pc += 1;
	std::invoke(op, this, address);
	return 0;
}

byte CPU::Zeropage_Y(const op_func& op)
{
	word address = Byte(Read(reg.pc) + reg.y);
	reg.pc += 1;
	std::invoke(op, this, address);
	return 0;
}

byte CPU::Absolute(const op_func& op)
{
	word address = Word(Read(reg.pc + 1), Read(reg.pc));
	reg.pc += 2;
	std::invoke(op, this, address);
	return 0;
}

byte CPU::Absolute_X(const op_func& op)
{
	word address = Word(Read(reg.pc + 1), Read(reg.pc)) + reg.x;
	reg.pc += 2;
	std::invoke(op, this, address);
	// add cycle if address & 0x00FF < cpu.x
	return (reg.x < (address & 0x00FF)) ? 1 : 0;
}

byte CPU::Absolute_Y(const op_func& op)
{
	word address = Word(Read(reg.pc + 1), Read(reg.pc)) + reg.y;
	reg.pc += 2;
	std::invoke(op, this, address);
	// add cycle if address & 0x00FF < cpu.y
	return (reg.y < (address & 0x00FF)) ? 1 : 0;
}

byte CPU::Relative(const op_func& op)
{
	word address = reg.pc;
	reg.pc += 1;
	word t = reg.pc;
	std::invoke(op, this, address);
	// add cycle if reg.pc != cpu.alu;
	// add cycle if (reg.pc & 0xFF00) != (cpu.alu & 0xFF00);
	byte extra_cycle = ((reg.pc & 0x00FF) != (t & 0x00FF)) ? 1 : 0;
	return extra_cycle + ((reg.pc & 0xFF00) != (t & 0xFF00)) ? 1 : 0;
}

byte CPU::Indirect(const op_func& op)
{
	byte l = Read(reg.pc);
	byte h = Read(reg.pc + 1);
	word address = Word(Read(Word(h, Byte(l + 1))), Read(Word(h, l)));
	reg.pc += 2;
	std::invoke(op, this, address);
	return 0;
}

byte CPU::Indirect_X(const op_func& op)
{
	byte l = (Read(reg.pc) + reg.x);
	byte h = l + 1;
	word address = Word(Read(h), Read(l));
	reg.pc += 1;
	std::invoke(op, this, address);
	return 0;
}

byte CPU::Indirect_Y(const op_func& op)
{
	byte l = Read(reg.pc);
	byte h = l + 1;
	word address = Word(Read(h), Read(l)) + reg.y;
	reg.pc += 1;
	std::invoke(op, this, address);
	// add cycle if address & 0x00FF < cpu.y
	return (reg.y < (address & 0x00FF)) ? 1 : 0;
}


///////////////////////////////////////////////////////////////////
// INSTRUCTIONS
///////////////////////////////////////////////////////////////////

byte CPU::ADC(word address)
{
	auto m = Read(address);
	word result = reg.a + m + reg.sr.c;
	reg.sr.c = result > 255;
	reg.sr.v = !((reg.a ^ m) & 0x80) && ((reg.a ^ result) & 0x80);
	reg.a = Byte(result);
	reg.sr.z = reg.a == 0;
	reg.sr.n = (reg.a & 0x80) > 0;
	return 0;
}

byte CPU::AND(word address)
{
	reg.a &= Read(address);
	reg.sr.z = reg.a == 0;
	reg.sr.n = (reg.a & 0x80) > 0;
	return 0;
}

byte CPU::ASL(word address)
{
	word result = Read(address) << 1;
	byte t = Byte(result);
	reg.sr.c = result > 255;
	reg.sr.z = t == 0;
	reg.sr.n = (t & 0x80) > 0;
	Write(address, t);
	return 0;
}

byte CPU::ASL_A(word address)
{
	word result = reg.a << 1;
	reg.a = Byte(result);
	reg.sr.c = result > 255;
	reg.sr.z = reg.a == 0;
	reg.sr.n = (reg.a & 0x80) > 0;
	return 0;
}

byte CPU::BCC(word address)
{
	reg.pc += reg.sr.c ? 0 : (sbyte)Read(address);
	return 0;
}

byte CPU::BCS(word address)
{
	reg.pc += reg.sr.c ? (sbyte)Read(address) : 0;
	return 0;
}

byte CPU::BEQ(word address)
{
	reg.pc += reg.sr.z ? (sbyte)Read(address) : 0;
	return 0;
}

byte CPU::BIT(word address)
{
	auto m = Read(address);
	reg.sr.z = (reg.a & m) ? 0 : 1;
	reg.sr.v = (m & 0x40) ? 1 : 0;
	reg.sr.n = (m & 0x80) ? 1 : 0;
	return 0;
}

byte CPU::BMI(word address)
{
	reg.pc += reg.sr.n ? (sbyte)Read(address) : 0;
	return 0;
}

byte CPU::BNE(word address)
{
	reg.pc += reg.sr.z ? 0 : (sbyte)Read(address);
	return 0;
}

byte CPU::BPL(word address)
{
	reg.pc += reg.sr.n ? 0 : (sbyte)Read(address);
	return 0;
}

byte CPU::BRK(word address)
{
	reg.sr.b = 1;
	auto [h, l] = Bytes(reg.pc);
	Write(Word(0x01, reg.sp--), h);
	Write(Word(0x01, reg.sp--), l);
	Write(Word(0x01, reg.sp--), reg.sr);
	reg.pc = Word(Read(0xFFFF), Read(0xFFFE));
	reg.sr.i = 1;
	return 0;
}

byte CPU::BVC(word address)
{
	reg.pc += reg.sr.v ? 0 : (sbyte)Read(address);
	return 0;
}

byte CPU::BVS(word address)
{
	reg.pc += reg.sr.v ? (sbyte)Read(address) : 0;
	return 0;
}

byte CPU::CLC(word address)
{
	reg.sr.c = 0;
	return 0;
}

byte CPU::CLD(word address)
{
	reg.sr.d = 0;
	return 0;
}

byte CPU::CLI(word address)
{
	reg.sr.i = 0;
	return 0;
}

byte CPU::CLV(word address)
{
	reg.sr.v = 0;
	return 0;
}

byte CPU::CMP(word address)
{
	auto m = Read(address);
	reg.sr.c = reg.a >= m;
	reg.sr.z = reg.a == m;
	reg.sr.n = ((reg.a - m) & 0x80) >> 7;
	return 0;
}

byte CPU::CPX(word address)
{
	auto m = Read(address);
	reg.sr.c = reg.x >= m;
	reg.sr.z = reg.x == m;
	reg.sr.n = ((reg.x - m) & 0x80) >> 7;
	return 0;
}

byte CPU::CPY(word address)
{
	auto m = Read(address);
	reg.sr.c = reg.y >= m;
	reg.sr.z = reg.y == m;
	reg.sr.n = ((reg.y - m) & 0x80) >> 7;
	return 0;
}

byte CPU::DEC(word address)
{
	byte t = Read(address) - 1;
	reg.sr.z = t == 0;
	reg.sr.n = (t & 0x80) >> 7;
	Write(address, t);
	return 0;
}

byte CPU::DEX(word address)
{
	reg.x -= 1;
	reg.sr.z = reg.x == 0;
	reg.sr.n = (reg.x & 0x80) >> 7;
	return 0;
}

byte CPU::DEY(word address)
{
	reg.y -= 1;
	reg.sr.z = reg.y == 0;
	reg.sr.n = (reg.y & 0x80) >> 7;
	return 0;
}

byte CPU::EOR(word address)
{
	reg.a ^= Read(address);
	reg.sr.z = reg.a == 0;
	reg.sr.n = (reg.a & 0x80) >> 7;
	return 0;
}

byte CPU::INC(word address)
{
	byte t = Read(address) + 1;
	reg.sr.z = t == 0;
	reg.sr.n = (t & 0x80) >> 7;
	Write(address, t);
	return 0;
}

byte CPU::INX(word address)
{
	reg.x += 1;
	reg.sr.z = reg.x == 0;
	reg.sr.n = (reg.x & 0x80) >> 7;
	return 0;
}

byte CPU::INY(word address)
{
	reg.y += 1;
	reg.sr.z = reg.y == 0;
	reg.sr.n = (reg.y & 0x80) >> 7;
	return 0;
}

byte CPU::JMP(word address)
{
	reg.pc = address;
	return 0;
}

byte CPU::JSR(word address)
{
	auto [h, l] = Bytes(--reg.pc);
	Write(Word(0x01, reg.sp--), h);
	Write(Word(0x01, reg.sp--), l);
	reg.pc = address;
	return 0;
}

byte CPU::LDA(word address)
{
	reg.a = Read(address);
	reg.sr.z = reg.a == 0;
	reg.sr.n = (reg.a & 0x80) >> 7;
	return 0;
}

byte CPU::LDX(word address)
{
	reg.x = Read(address);
	reg.sr.z = reg.x == 0;
	reg.sr.n = (reg.x & 0x80) >> 7;
	return 0;
}

byte CPU::LDY(word address)
{
	reg.y = Read(address);
	reg.sr.z = reg.y == 0;
	reg.sr.n = (reg.y & 0x80) >> 7;
	return 0;
}

byte CPU::LSR(word address)
{
	byte t = Read(address);
	reg.sr.c = t & 0x01;
	t = t >> 1;
	reg.sr.z = t == 0;
	reg.sr.n = (t & 0x80) >> 7;
	Write(address, t);
	return 0;
}

byte CPU::LSR_A(word address)
{
	reg.sr.c = reg.a & 0x01;
	reg.a = reg.a >> 1;
	reg.sr.z = reg.a == 0;
	reg.sr.n = (reg.a & 0x80) >> 7;
	return 0;
}

byte CPU::NOP(word address)
{
	return 0;
}

byte CPU::ORA(word address)
{
	reg.a |= Read(address);
	reg.sr.z = reg.a == 0;
	reg.sr.n = (reg.a & 0x80) >> 7;
	return 0;
}

byte CPU::PHA(word address)
{
	Write(Word(0x01, reg.sp--), reg.a);
	return 0;
}

byte CPU::PHP(word address)
{
	reg.sr.b = 1;
	Write(Word(0x01, reg.sp--), reg.sr);
	return 0;
}

byte CPU::PLA(word address)
{
	reg.a = Read(Word(0x01, ++reg.sp));
	reg.sr.z = reg.a == 0;
	reg.sr.n = (reg.a & 0x80) >> 7;
	return 0;
}

byte CPU::PLP(word address)
{
	reg.sr = Read(Word(0x01, ++reg.sp));
	return 0;
}

byte CPU::ROL(word address)
{
	word result = (Read(address) << 1) | reg.sr.c;
	byte t = Byte(result);
	reg.sr.c = (result & 0x0100) > 0;
	reg.sr.z = t == 0;
	reg.sr.n = (t & 0x80) >> 7;
	Write(address, t);
	return 0;
}

byte CPU::ROL_A(word address)
{
	word result = (reg.a << 1) | reg.sr.c;
	reg.a = Byte(result);
	reg.sr.c = (result & 0x0100) > 0;
	reg.sr.z = reg.a == 0;
	reg.sr.n = (reg.a & 0x80) >> 7;
	return 0;
}

byte CPU::ROR(word address)
{
	word result = Word(reg.sr.c, Read(address));
	byte t = Byte(result >> 1);
	reg.sr.c = result & 0x01;
	reg.sr.z = t == 0;
	reg.sr.n = (t & 0x80) >> 7;
	Write(address, t);
	return 0;
}

byte CPU::ROR_A(word address)
{
	word result = Word(reg.sr.c, reg.a);
	reg.a = Byte(result >> 1);
	reg.sr.c = result & 0x01;
	reg.sr.z = reg.a == 0;
	reg.sr.n = (reg.a & 0x80) >> 7;
	return 0;
}

byte CPU::RTI(word address)
{
	reg.sr = Read(Word(0x01, ++reg.sp));
	reg.pc = Read(Word(0x01, ++reg.sp));
	reg.pc |= Read(Word(0x01, ++reg.sp)) << 8;
	return 0;
}

byte CPU::RTS(word address)
{
	reg.pc = Read(Word(0x01, ++reg.sp));
	reg.pc |= Read(Word(0x01, ++reg.sp)) << 8;
	reg.pc += 1;
	return 0;
}

byte CPU::SBC(word address)
{
	word result = reg.a - Read(address) - (1 - reg.sr.c);
	reg.sr.c = result <= 255;
	reg.sr.v = ((result ^ reg.a) & 0x80) && ((Read(address) ^ reg.a) & 0x80);
	reg.a = Byte(result);
	reg.sr.z = reg.a == 0;
	reg.sr.n = (reg.a & 0x80) >> 7;
	return 0;
}

byte CPU::SEC(word address)
{
	reg.sr.c = 1;
	return 0;
}

byte CPU::SED(word address)
{
	reg.sr.d = 1;
	return 0;
}

byte CPU::SEI(word address)
{
	reg.sr.i = 1;
	return 0;
}

byte CPU::STA(word address)
{
	Write(address, reg.a);
	return 0;
}

byte CPU::STX(word address)
{
	Write(address, reg.x);
	return 0;
}

byte CPU::STY(word address)
{
	Write(address, reg.y);
	return 0;
}

byte CPU::TAX(word address)
{
	reg.x = reg.a;
	reg.sr.z = reg.x == 0;
	reg.sr.n = (reg.x & 0x80) >> 7;
	return 0;
}

byte CPU::TAY(word address)
{
	reg.y = reg.a;
	reg.sr.z = reg.y == 0;
	reg.sr.n = (reg.y & 0x80) >> 7;
	return 0;
}

byte CPU::TSX(word address)
{
	reg.x = reg.sp;
	reg.sr.z = reg.x == 0;
	reg.sr.n = (reg.x & 0x80) >> 7;
	return 0;
}

byte CPU::TXA(word address)
{
	reg.a = reg.x;
	reg.sr.z = reg.a == 0;
	reg.sr.n = (reg.a & 0x80) >> 7;
	return 0;
}

byte CPU::TXS(word address)
{
	reg.sp = reg.x;
	return 0;
}

byte CPU::TYA(word address)
{
	reg.a = reg.y;
	reg.sr.z = reg.a == 0;
	reg.sr.n = (reg.a & 0x80) >> 7;
	return 0;
}