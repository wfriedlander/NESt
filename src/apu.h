#pragma once

#include "nest.h"
#include "bus.h"
#include "audio.h"



class LengthCounter
{
public:
	void SetEnabled(bool enabled);
	void SetHalted(bool halted);
	void SetLength(byte length);
	void HalfStep();
	byte Count();


private:
	bool mEnabled = false;
	bool mHalted = false;
	byte mCount = 0;
	byte mLength = 0;

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
};

class Noise
{
public:
	void RegisterWrite(word address, byte value);
	const std::vector<byte>& ProcessSamples(word cycles);
	void QuarterStep();
	void HalfStep();
	void Enable(bool enabled);

private:
	bool mStart = true;
	bool mConstantVolume = false;
	bool mLoop = false;
	byte mDecay = 0;
	byte mDivider = 0;
	byte mEnvelope = 0;
	byte mShiftMode = 0;
	word mTimer = 0;
	word mTimerReg = 0;
	word mShift = 0;
	LengthCounter mLengthCounter;

private:
	std::vector<byte> mSamples;

private:
	word mPeriodTable[16] = {
		4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068
	};
};

class Triangle
{
public:
	void RegisterWrite(word address, byte value);
	const std::vector<byte>& ProcessSamples(word cycles);
	void QuarterStep();
	void HalfStep();
	void Enable(bool enabled);

private:
	bool mControl = false;
	bool mLinearReload = false;
	word mTimer = 0;
	word mTimerReg = 0;
	word mLinearCounter = 0;
	word mLinearCounterReg = 0;
	byte mSequencePos = 0;
	LengthCounter mLengthCounter;

private:
	std::vector<byte> mSamples;

private:
	byte mSequenceTable[32] = {
		15, 14, 13, 12, 11, 10, 9,  8,
		7,  6,  5,  4,  3,  2,  1,  0,
		0,  1,  2,  3,  4,  5,  6,  7,
		8,  9,  10, 11, 12, 13, 14, 15
	};
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
	Triangle mTriangle;
	Noise mNoise;

private:
	framecount mFrameReg;
	AudioBuffer mSamples;

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

	std::vector<float> mPulseTable = {
		0.0,
		0.011609139523578026,
		0.022939481268011527,
		0.03400094921689606,
		0.04480300187617261,
		0.05535465924895688,
		0.06566452795600367,
		0.07574082464884459,
		0.08559139784946236,
		0.09522374833850243,
		0.10464504820333041,
		0.11386215864759427,
		0.12288164665523155,
		0.13170980059397538,
		0.14035264483627205,
		0.1488159534690486,
		0.15710526315789472,
		0.16522588522588522,
		0.1731829170024174,
		0.18098125249301955,
		0.18862559241706162,
		0.19612045365662886,
		0.20347017815646784,
		0.21067894131185272,
		0.21775075987841944,
		0.2246894994354535,
		0.2314988814317673,
		0.23818248984115256,
		0.2447437774524158,
		0.2511860718171926,
		0.25751258087706685,
		0.2637263982899893
	};

	std::vector<float> mTndTable = {
		0.0,
		0.006699823979696262,
		0.01334502018019487,
		0.01993625400950099,
		0.026474180112418616,
		0.032959442587297105,
		0.03939267519756107,
		0.04577450157816932,
		0.05210553543714433,
		0.05838638075230885,
		0.06461763196336215,
		0.07079987415942428,
		0.07693368326217241,
		0.08301962620468999,
		0.08905826110614481,
		0.09505013744240969,
		0.10099579621273477,
		0.10689577010257789,
		0.11275058364269584,
		0.11856075336459644,
		0.12432678795244785,
		0.1300491883915396,
		0.13572844811338536,
		0.1413650531375568,
		0.1469594822103333,
		0.15251220694025122,
		0.15802369193063237,
		0.16349439490917161,
		0.16892476685465738,
		0.1743152521209005,
		0.1796662885579421,
		0.18497830763060993,
		0.19025173453449087,
		0.19548698830938505,
		0.20068448195030472,
		0.20584462251608032,
		0.2109678112356332,
		0.2160544436119733,
		0.2211049095239788,
		0.22611959332601225,
		0.2310988739454269,
		0.23604312497801538,
		0.24095271478145042,
		0.24582800656676793,
		0.25066935848793903,
		0.25547712372957787,
		0.2602516505928307,
		0.26499328257948945,
		0.26970235847437257,
		0.27437921242601526,
		0.27902417402570834,
		0.28363756838492643,
		0.2882197162111822,
		0.292770933882345,
		0.29729153351945914,
		0.3017818230580978,
		0.3062421063182866,
		0.31067268307302937,
		0.31507384911547015,
		0.3194458963247213,
		0.32378911273039,
		0.3281037825758322,
		0.3323901863801631,
		0.33664860099905314,
		0.3408792996843372,
		0.34508255214246325,
		0.349258624591807,
		0.3534077798188791,
		0.3575302772334479,
		0.36162637292260397,
		0.3656963197037888,
		0.3697403671768112,
		0.3737587617748739,
		0.37775174681463214,
		0.38171956254530554,
		0.38566244619686446,
		0.3895806320273106,
		0.3934743513690717,
		0.3973438326745308,
		0.40118930156070615,
		0.405010980853104,
		0.4088090906287582,
		0.41258384825847705,
		0.4163354684483128,
		0.42006416328027124,
		0.4237701422522769,
		0.42745361231741014,
		0.4311147779224318,
		0.4347538410456096,
		0.43837100123386197,
		0.4419664556392331,
		0.44554039905471293,
		0.44909302394941686,
		0.4526245205031371,
		0.45613507664027986,
		0.4596248780632002,
		0.4630941082849479,
		0.4665429486614358,
		0.46997157842304194,
		0.47338017470565896,
		0.4767689125811996,
		0.48013796508757145,
		0.48348750325813084,
		0.48681769615062515,
		0.49012871087563703,
		0.493420712624537,
		0.49669386469695664,
		0.49994832852779125,
		0.5031842637137408,
		0.5064018280393993,
		0.5096011775029012,
		0.5127824663411329,
		0.5159458470545188,
		0.5190914704313901,
		0.5222194855719443,
		0.5253300399118033,
		0.528423279245178,
		0.5314993477476477,
		0.5345583879985607,
		0.5376005410030638,
		0.5406259462137686,
		0.5436347415520602,
		0.5466270634290563,
		0.5496030467662235,
		0.5525628250156552,
		0.5555065301800212,
		0.5584342928321915,
		0.5613462421345432,
		0.5642425058579547,
		0.5671232104004943,
		0.5699884808058077,
		0.5728384407812124,
		0.5756732127155,
		0.5784929176964575,
		0.5812976755281083,
		0.5840876047476803,
		0.5868628226423054,
		0.5896234452654553,
		0.5923695874531196,
		0.595101362839729,
		0.5978188838738291,
		0.6005222618335111,
		0.6032116068415997,
		0.6058870278806079,
		0.6085486328074569,
		0.6111965283679723,
		0.6138308202111536,
		0.6164516129032258,
		0.6190590099414757,
		0.6216531137678758,
		0.6242340257825014,
		0.6268018463567424,
		0.6293566748463153,
		0.6318986096040777,
		0.6344277479926501,
		0.6369441863968464,
		0.6394480202359187,
		0.6419393439756177,
		0.6444182511400732,
		0.6468848343234979,
		0.6493391852017159,
		0.6517813945435207,
		0.6542115522218658,
		0.6566297472248885,
		0.659036067666773,
		0.6614306007984521,
		0.6638134330181533,
		0.6661846498817908,
		0.6685443361132047,
		0.670892575614252,
		0.6732294514747513,
		0.6755550459822829,
		0.6778694406318475,
		0.6801727161353863,
		0.6824649524311629,
		0.684746228693012,
		0.6870166233394548,
		0.6892762140426848,
		0.6915250777374256,
		0.693763290629662,
		0.6959909282052493,
		0.6982080652383982,
		0.7004147758000423,
		0.7026111332660865,
		0.70479721032554,
		0.7069730789885358,
		0.7091388105942369,
		0.7112944758186339,
		0.7134401446822323,
		0.7155758865576349,
		0.7177017701770176,
		0.7198178636395035,
		0.7219242344184336,
		0.7240209493685391,
		0.7261080747330146,
		0.7281856761504939,
		0.7302538186619317,
		0.7323125667173908,
		0.734361984182737,
		0.7364021343462434,
		0.7384330799251054,
		0.7404548830718675,
		0.742467605380763
	};

};

