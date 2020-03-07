#pragma once

#include "nest.h"
#include "bus.h"

class Bus;

class CPU
{
	using op_func = byte(CPU::*)(word);
	using addr_func = byte (CPU::*)(const op_func&);
	struct Registers
	{
		struct status
		{
			byte n = 0;     // Negative
			byte v = 0;     // Overflow
			byte b = 0;     // 1 on PHP/BRK, 0 on IRQ/NMI
			byte d = 0;     // Decimal
			byte i = 0;     // Interrupt Disable
			byte z = 0;     // Zero
			byte c = 0;     // Carry
			operator byte() const { return (n << 7 | v << 6 | 1 << 5 | b << 4 | d << 3 | i << 2 | z << 1 | c << 0); }
			void operator =(byte val) { n = (val >> 7) & 0x01; v = (val >> 6) & 0x01; b = 0; d = (val >> 3) & 0x01; i = (val >> 2) & 0x01; z = (val >> 1) & 0x01; c = (val >> 0) & 0x01; }
		};

		status sr;
		byte a = 0;
		byte x = 0;
		byte y = 0;
		byte sp = 0xFF;
		word pc = 0;

	};
	enum class op_type
	{
		branch,
		brk,
		flag,
		jsr,
		jump,
		pull,
		push,
		read,
		reg,
		rmw,
		rti,
		rts,
		write
	};
	struct Instruction
	{
		std::string op_name;
		std::string addr_name;
		op_func op;
		addr_func addr;
		byte bytes;
		byte cycles;
	};

public:
	CPU(Bus* bus);
	~CPU();

public:
	void SetNMI();
	void SetIRQ();
	void DMA(byte address);
	void NMI();
	void IRQ();
	void Reset();
	byte Step();

public:
	void EnableLogging(bool enabled);

private:
	void Write(word address, byte value);
	byte Read(word address);

private:
	void Log(Instruction& ins);
	FILE* pFile;
	bool mLog = false;
	int total_cycles = 0;

private:
	Registers reg;
	Bus* bus;

private:
	bool mNMI = false;
	bool mIRQ = false;

private:
	// Addressing
	byte Implied(const op_func & op);
	byte Immediate(const op_func & op);
	byte Zeropage(const op_func & op);
	byte Zeropage_X(const op_func & op);
	byte Zeropage_Y(const op_func & op);
	byte Absolute(const op_func & op);
	byte Absolute_X(const op_func & op);
	byte Absolute_Y(const op_func & op);
	byte Relative(const op_func & op);
	byte Indirect(const op_func & op);
	byte Indirect_X(const op_func & op);
	byte Indirect_Y(const op_func & op);

	// Instructions
	byte ADC(word address);
	byte AND(word address);
	byte ASL(word address);
	byte ASL_A(word address);
	byte BCC(word address);
	byte BCS(word address);
	byte BEQ(word address);
	byte BIT(word address);
	byte BMI(word address);
	byte BNE(word address);
	byte BPL(word address);
	byte BRK(word address);
	byte BVC(word address);
	byte BVS(word address);
	byte CLC(word address);
	byte CLD(word address);
	byte CLI(word address);
	byte CLV(word address);
	byte CMP(word address);
	byte CPX(word address);
	byte CPY(word address);
	byte DEC(word address);
	byte DEX(word address);
	byte DEY(word address);
	byte EOR(word address);
	byte INC(word address);
	byte INX(word address);
	byte INY(word address);
	byte JMP(word address);
	byte JSR(word address);
	byte LDA(word address);
	byte LDX(word address);
	byte LDY(word address);
	byte LSR(word address);
	byte LSR_A(word address);
	byte NOP(word address);
	byte ORA(word address);
	byte PHA(word address);
	byte PHP(word address);
	byte PLA(word address);
	byte PLP(word address);
	byte ROL(word address);
	byte ROL_A(word address);
	byte ROR(word address);
	byte ROR_A(word address);
	byte RTI(word address);
	byte RTS(word address);
	byte SBC(word address);
	byte SEC(word address);
	byte SED(word address);
	byte SEI(word address);
	byte STA(word address);
	byte STX(word address);
	byte STY(word address);
	byte TAX(word address);
	byte TAY(word address);
	byte TSX(word address);
	byte TXA(word address);
	byte TXS(word address);
	byte TYA(word address);

	// Op Codes
	Instruction opcode[256] = {
		{"BRK",    "Immediate",     &CPU::BRK,     &CPU::Immediate,     2,    7},    // 0x00
		{"ORA",    "Indirect_X",    &CPU::ORA,     &CPU::Indirect_X,    2,    6},    // 0x01
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x02
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x03
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x04
		{"ORA",    "Zeropage",      &CPU::ORA,     &CPU::Zeropage,      2,    3},    // 0x05
		{"ASL",    "Zeropage",      &CPU::ASL,     &CPU::Zeropage,      2,    5},    // 0x06
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x07
		{"PHP",    "Implied",       &CPU::PHP,     &CPU::Implied,       1,    3},    // 0x08
		{"ORA",    "Immediate",     &CPU::ORA,     &CPU::Immediate,     2,    2},    // 0x09
		{"ASL",    "Implied",       &CPU::ASL_A,   &CPU::Implied,       1,    2},    // 0x0A
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x0B
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x0C
		{"ORA",    "Absolute",      &CPU::ORA,     &CPU::Absolute,      3,    4},    // 0x0D
		{"ASL",    "Absolute",      &CPU::ASL,     &CPU::Absolute,      3,    6},    // 0x0E
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x0F
		{"BPL",    "Relative",      &CPU::BPL,     &CPU::Relative,      2,    2},    // 0x10
		{"ORA",    "Indirect_Y",    &CPU::ORA,     &CPU::Indirect_Y,    2,    5},    // 0x11
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x12
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x13
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x14
		{"ORA",    "Zeropage_X",    &CPU::ORA,     &CPU::Zeropage_X,    2,    4},    // 0x15
		{"ASL",    "Zeropage_X",    &CPU::ASL,     &CPU::Zeropage_X,    2,    6},    // 0x16
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x17
		{"CLC",    "Implied",       &CPU::CLC,     &CPU::Implied,       1,    2},    // 0x18
		{"ORA",    "Absolute_Y",    &CPU::ORA,     &CPU::Absolute_Y,    3,    4},    // 0x19
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x1A
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x1B
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x1C
		{"ORA",    "Absolute_X",    &CPU::ORA,     &CPU::Absolute_X,    3,    4},    // 0x1D
		{"ASL",    "Absolute_X",    &CPU::ASL,     &CPU::Absolute_X,    3,    7},    // 0x1E
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x1F
		{"JSR",    "Absolute",      &CPU::JSR,     &CPU::Absolute,      3,    6},    // 0x20
		{"AND",    "Indirect_X",    &CPU::AND,     &CPU::Indirect_X,    2,    6},    // 0x21
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x22
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x23
		{"BIT",    "Zeropage",      &CPU::BIT,     &CPU::Zeropage,      2,    3},    // 0x24
		{"AND",    "Zeropage",      &CPU::AND,     &CPU::Zeropage,      2,    3},    // 0x25
		{"ROL",    "Zeropage",      &CPU::ROL,     &CPU::Zeropage,      2,    5},    // 0x26
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x27
		{"PLP",    "Implied",       &CPU::PLP,     &CPU::Implied,       1,    4},    // 0x28
		{"AND",    "Immediate",     &CPU::AND,     &CPU::Immediate,     2,    2},    // 0x29
		{"ROL",    "Implied",       &CPU::ROL_A,   &CPU::Implied,       1,    2},    // 0x2A
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x2B
		{"BIT",    "Absolute",      &CPU::BIT,     &CPU::Absolute,      3,    4},    // 0x2C
		{"AND",    "Absolute",      &CPU::AND,     &CPU::Absolute,      3,    4},    // 0x2D
		{"ROL",    "Absolute",      &CPU::ROL,     &CPU::Absolute,      3,    6},    // 0x2E
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x2F
		{"BMI",    "Relative",      &CPU::BMI,     &CPU::Relative,      2,    2},    // 0x30
		{"AND",    "Indirect_Y",    &CPU::AND,     &CPU::Indirect_Y,    2,    5},    // 0x31
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x32
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x33
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x34
		{"AND",    "Zeropage_X",    &CPU::AND,     &CPU::Zeropage_X,    2,    4},    // 0x35
		{"ROL",    "Zeropage_X",    &CPU::ROL,     &CPU::Zeropage_X,    2,    6},    // 0x36
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x37
		{"SEC",    "Implied",       &CPU::SEC,     &CPU::Implied,       1,    2},    // 0x38
		{"AND",    "Absolute_Y",    &CPU::AND,     &CPU::Absolute_Y,    3,    4},    // 0x39
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x3A
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x3B
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x3C
		{"AND",    "Absolute_X",    &CPU::AND,     &CPU::Absolute_X,    3,    4},    // 0x3D
		{"ROL",    "Absolute_X",    &CPU::ROL,     &CPU::Absolute_X,    3,    7},    // 0x3E
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x3F
		{"RTI",    "Implied",       &CPU::RTI,     &CPU::Implied,       1,    6},    // 0x40
		{"EOR",    "Indirect_X",    &CPU::EOR,     &CPU::Indirect_X,    2,    6},    // 0x41
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x42
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x43
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x44
		{"EOR",    "Zeropage",      &CPU::EOR,     &CPU::Zeropage,      2,    3},    // 0x45
		{"LSR",    "Zeropage",      &CPU::LSR,     &CPU::Zeropage,      2,    5},    // 0x46
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x47
		{"PHA",    "Implied",       &CPU::PHA,     &CPU::Implied,       1,    3},    // 0x48
		{"EOR",    "Immediate",     &CPU::EOR,     &CPU::Immediate,     2,    2},    // 0x49
		{"LSR",    "Implied",       &CPU::LSR_A,   &CPU::Implied,       1,    2},    // 0x4A
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x4B
		{"JMP",    "Absolute",      &CPU::JMP,     &CPU::Absolute,      3,    3},    // 0x4C
		{"EOR",    "Absolute",      &CPU::EOR,     &CPU::Absolute,      3,    4},    // 0x4D
		{"LSR",    "Absolute",      &CPU::LSR,     &CPU::Absolute,      3,    6},    // 0x4E
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x4F
		{"BVC",    "Relative",      &CPU::BVC,     &CPU::Relative,      2,    2},    // 0x50
		{"EOR",    "Indirect_Y",    &CPU::EOR,     &CPU::Indirect_Y,    2,    5},    // 0x51
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x52
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x53
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x54
		{"EOR",    "Zeropage_X",    &CPU::EOR,     &CPU::Zeropage_X,    2,    4},    // 0x55
		{"LSR",    "Zeropage_X",    &CPU::LSR,     &CPU::Zeropage_X,    2,    6},    // 0x56
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x57
		{"CLI",    "Implied",       &CPU::CLI,     &CPU::Implied,       1,    2},    // 0x58
		{"EOR",    "Absolute_Y",    &CPU::EOR,     &CPU::Absolute_Y,    3,    4},    // 0x59
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x5A
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x5B
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x5C
		{"EOR",    "Absolute_X",    &CPU::EOR,     &CPU::Absolute_X,    3,    4},    // 0x5D
		{"LSR",    "Absolute_X",    &CPU::LSR,     &CPU::Absolute_X,    3,    7},    // 0x5E
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x5F
		{"RTS",    "Implied",       &CPU::RTS,     &CPU::Implied,       1,    6},    // 0x60
		{"ADC",    "Indirect_X",    &CPU::ADC,     &CPU::Indirect_X,    2,    6},    // 0x61
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x62
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x63
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x64
		{"ADC",    "Zeropage",      &CPU::ADC,     &CPU::Zeropage,      2,    3},    // 0x65
		{"ROR",    "Zeropage",      &CPU::ROR,     &CPU::Zeropage,      2,    5},    // 0x66
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x67
		{"PLA",    "Implied",       &CPU::PLA,     &CPU::Implied,       1,    4},    // 0x68
		{"ADC",    "Immediate",     &CPU::ADC,     &CPU::Immediate,     2,    2},    // 0x69
		{"ROR",    "Implied",       &CPU::ROR_A,   &CPU::Implied,       1,    2},    // 0x6A
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x6B
		{"JMP",    "Indirect",      &CPU::JMP,     &CPU::Indirect,      3,    5},    // 0x6C
		{"ADC",    "Absolute",      &CPU::ADC,     &CPU::Absolute,      3,    4},    // 0x6D
		{"ROR",    "Absolute",      &CPU::ROR,     &CPU::Absolute,      3,    6},    // 0x6E
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x6F
		{"BVS",    "Relative",      &CPU::BVS,     &CPU::Relative,      2,    2},    // 0x70
		{"ADC",    "Indirect_Y",    &CPU::ADC,     &CPU::Indirect_Y,    2,    5},    // 0x71
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x72
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x73
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x74
		{"ADC",    "Zeropage_X",    &CPU::ADC,     &CPU::Zeropage_X,    2,    4},    // 0x75
		{"ROR",    "Zeropage_X",    &CPU::ROR,     &CPU::Zeropage_X,    2,    6},    // 0x76
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x77
		{"SEI",    "Implied",       &CPU::SEI,     &CPU::Implied,       1,    2},    // 0x78
		{"ADC",    "Absolute_Y",    &CPU::ADC,     &CPU::Absolute_Y,    3,    4},    // 0x79
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x7A
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x7B
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x7C
		{"ADC",    "Absolute_X",    &CPU::ADC,     &CPU::Absolute_X,    3,    4},    // 0x7D
		{"ROR",    "Absolute_X",    &CPU::ROR,     &CPU::Absolute_X,    3,    7},    // 0x7E
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x7F
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x80
		{"STA",    "Indirect_X",    &CPU::STA,     &CPU::Indirect_X,    2,    6},    // 0x81
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x82
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x83
		{"STY",    "Zeropage",      &CPU::STY,     &CPU::Zeropage,      2,    3},    // 0x84
		{"STA",    "Zeropage",      &CPU::STA,     &CPU::Zeropage,      2,    3},    // 0x85
		{"STX",    "Zeropage",      &CPU::STX,     &CPU::Zeropage,      2,    3},    // 0x86
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x87
		{"DEY",    "Implied",       &CPU::DEY,     &CPU::Implied,       1,    2},    // 0x88
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x89
		{"TXA",    "Implied",       &CPU::TXA,     &CPU::Implied,       1,    2},    // 0x8A
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x8B
		{"STY",    "Absolute",      &CPU::STY,     &CPU::Absolute,      3,    4},    // 0x8C
		{"STA",    "Absolute",      &CPU::STA,     &CPU::Absolute,      3,    4},    // 0x8D
		{"STX",    "Absolute",      &CPU::STX,     &CPU::Absolute,      3,    4},    // 0x8E
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x8F
		{"BCC",    "Relative",      &CPU::BCC,     &CPU::Relative,      2,    2},    // 0x90
		{"STA",    "Indirect_Y",    &CPU::STA,     &CPU::Indirect_Y,    2,    6},    // 0x91
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x92
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x93
		{"STY",    "Zeropage_X",    &CPU::STY,     &CPU::Zeropage_X,    2,    4},    // 0x94
		{"STA",    "Zeropage_X",    &CPU::STA,     &CPU::Zeropage_X,    2,    4},    // 0x95
		{"STX",    "Zeropage_Y",    &CPU::STX,     &CPU::Zeropage_Y,    2,    4},    // 0x96
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x97
		{"TYA",    "Implied",       &CPU::TYA,     &CPU::Implied,       1,    2},    // 0x98
		{"STA",    "Absolute_Y",    &CPU::STA,     &CPU::Absolute_Y,    3,    5},    // 0x99
		{"TXS",    "Implied",       &CPU::TXS,     &CPU::Implied,       1,    2},    // 0x9A
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x9B
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x9C
		{"STA",    "Absolute_X",    &CPU::STA,     &CPU::Absolute_X,    3,    5},    // 0x9D
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x9E
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0x9F
		{"LDY",    "Immediate",     &CPU::LDY,     &CPU::Immediate,     2,    2},    // 0xA0
		{"LDA",    "Indirect_X",    &CPU::LDA,     &CPU::Indirect_X,    2,    6},    // 0xA1
		{"LDX",    "Immediate",     &CPU::LDX,     &CPU::Immediate,     2,    2},    // 0xA2
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xA3
		{"LDY",    "Zeropage",      &CPU::LDY,     &CPU::Zeropage,      2,    3},    // 0xA4
		{"LDA",    "Zeropage",      &CPU::LDA,     &CPU::Zeropage,      2,    3},    // 0xA5
		{"LDX",    "Zeropage",      &CPU::LDX,     &CPU::Zeropage,      2,    3},    // 0xA6
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xA7
		{"TAY",    "Implied",       &CPU::TAY,     &CPU::Implied,       1,    2},    // 0xA8
		{"LDA",    "Immediate",     &CPU::LDA,     &CPU::Immediate,     2,    2},    // 0xA9
		{"TAX",    "Implied",       &CPU::TAX,     &CPU::Implied,       1,    2},    // 0xAA
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xAB
		{"LDY",    "Absolute",      &CPU::LDY,     &CPU::Absolute,      3,    4},    // 0xAC
		{"LDA",    "Absolute",      &CPU::LDA,     &CPU::Absolute,      3,    4},    // 0xAD
		{"LDX",    "Absolute",      &CPU::LDX,     &CPU::Absolute,      3,    4},    // 0xAE
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xAF
		{"BCS",    "Relative",      &CPU::BCS,     &CPU::Relative,      2,    2},    // 0xB0
		{"LDA",    "Indirect_Y",    &CPU::LDA,     &CPU::Indirect_Y,    2,    5},    // 0xB1
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xB2
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xB3
		{"LDY",    "Zeropage_X",    &CPU::LDY,     &CPU::Zeropage_X,    2,    4},    // 0xB4
		{"LDA",    "Zeropage_X",    &CPU::LDA,     &CPU::Zeropage_X,    2,    4},    // 0xB5
		{"LDX",    "Zeropage_Y",    &CPU::LDX,     &CPU::Zeropage_Y,    2,    4},    // 0xB6
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xB7
		{"CLV",    "Implied",       &CPU::CLV,     &CPU::Implied,       1,    2},    // 0xB8
		{"LDA",    "Absolute_Y",    &CPU::LDA,     &CPU::Absolute_Y,    3,    4},    // 0xB9
		{"TSX",    "Implied",       &CPU::TSX,     &CPU::Implied,       1,    2},    // 0xBA
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xBB
		{"LDY",    "Absolute_X",    &CPU::LDY,     &CPU::Absolute_X,    3,    4},    // 0xBC
		{"LDA",    "Absolute_X",    &CPU::LDA,     &CPU::Absolute_X,    3,    4},    // 0xBD
		{"LDX",    "Absolute_Y",    &CPU::LDX,     &CPU::Absolute_Y,    3,    4},    // 0xBE
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xBF
		{"CPY",    "Immediate",     &CPU::CPY,     &CPU::Immediate,     2,    2},    // 0xC0
		{"CMP",    "Indirect_X",    &CPU::CMP,     &CPU::Indirect_X,    2,    6},    // 0xC1
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xC2
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xC3
		{"CPY",    "Zeropage",      &CPU::CPY,     &CPU::Zeropage,      2,    3},    // 0xC4
		{"CMP",    "Zeropage",      &CPU::CMP,     &CPU::Zeropage,      2,    3},    // 0xC5
		{"DEC",    "Zeropage",      &CPU::DEC,     &CPU::Zeropage,      2,    5},    // 0xC6
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xC7
		{"INY",    "Implied",       &CPU::INY,     &CPU::Implied,       1,    2},    // 0xC8
		{"CMP",    "Immediate",     &CPU::CMP,     &CPU::Immediate,     2,    2},    // 0xC9
		{"DEX",    "Implied",       &CPU::DEX,     &CPU::Implied,       1,    2},    // 0xCA
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xCB
		{"CPY",    "Absolute",      &CPU::CPY,     &CPU::Absolute,      3,    4},    // 0xCC
		{"CMP",    "Absolute",      &CPU::CMP,     &CPU::Absolute,      3,    4},    // 0xCD
		{"DEC",    "Absolute",      &CPU::DEC,     &CPU::Absolute,      3,    3},    // 0xCE
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xCF
		{"BNE",    "Relative",      &CPU::BNE,     &CPU::Relative,      2,    2},    // 0xD0
		{"CMP",    "Indirect_Y",    &CPU::CMP,     &CPU::Indirect_Y,    2,    5},    // 0xD1
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xD2
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xD3
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xD4
		{"CMP",    "Zeropage_X",    &CPU::CMP,     &CPU::Zeropage_X,    2,    4},    // 0xD5
		{"DEC",    "Zeropage_X",    &CPU::DEC,     &CPU::Zeropage_X,    2,    6},    // 0xD6
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xD7
		{"CLD",    "Implied",       &CPU::CLD,     &CPU::Implied,       1,    2},    // 0xD8
		{"CMP",    "Absolute_Y",    &CPU::CMP,     &CPU::Absolute_Y,    3,    4},    // 0xD9
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xDA
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xDB
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xDC
		{"CMP",    "Absolute_X",    &CPU::CMP,     &CPU::Absolute_X,    3,    4},    // 0xDD
		{"DEC",    "Absolute_X",    &CPU::DEC,     &CPU::Absolute_X,    3,    7},    // 0xDE
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xDF
		{"CPX",    "Immediate",     &CPU::CPX,     &CPU::Immediate,     2,    2},    // 0xE0
		{"SBC",    "Indirect_X",    &CPU::SBC,     &CPU::Indirect_X,    2,    6},    // 0xE1
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xE2
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xE3
		{"CPX",    "Zeropage",      &CPU::CPX,     &CPU::Zeropage,      2,    3},    // 0xE4
		{"SBC",    "Zeropage",      &CPU::SBC,     &CPU::Zeropage,      2,    3},    // 0xE5
		{"INC",    "Zeropage",      &CPU::INC,     &CPU::Zeropage,      2,    5},    // 0xE6
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xE7
		{"INX",    "Implied",       &CPU::INX,     &CPU::Implied,       1,    2},    // 0xE8
		{"SBC",    "Immediate",     &CPU::SBC,     &CPU::Immediate,     2,    2},    // 0xE9
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xEA
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xEB
		{"CPX",    "Absolute",      &CPU::CPX,     &CPU::Absolute,      3,    4},    // 0xEC
		{"SBC",    "Absolute",      &CPU::SBC,     &CPU::Absolute,      3,    4},    // 0xED
		{"INC",    "Absolute",      &CPU::INC,     &CPU::Absolute,      3,    6},    // 0xEE
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xEF
		{"BEQ",    "Relative",      &CPU::BEQ,     &CPU::Relative,      2,    2},    // 0xF0
		{"SBC",    "Indirect_Y",    &CPU::SBC,     &CPU::Indirect_Y,    2,    5},    // 0xF1
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xF2
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xF3
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xF4
		{"SBC",    "Zeropage_X",    &CPU::SBC,     &CPU::Zeropage_X,    2,    4},    // 0xF5
		{"INC",    "Zeropage_X",    &CPU::INC,     &CPU::Zeropage_X,    2,    6},    // 0xF6
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xF7
		{"SED",    "Implied",       &CPU::SED,     &CPU::Implied,       1,    2},    // 0xF8
		{"SBC",    "Absolute_Y",    &CPU::SBC,     &CPU::Absolute_Y,    3,    4},    // 0xF9
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xFA
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xFB
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xFC
		{"SBC",    "Absolute_X",    &CPU::SBC,     &CPU::Absolute_X,    3,    4},    // 0xFD
		{"INC",    "Absolute_X",    &CPU::INC,     &CPU::Absolute_X,    3,    7},    // 0xFE
		{"NOP",    "Implied",       &CPU::NOP,     &CPU::Implied,       1,    2},    // 0xFF
	};
};


