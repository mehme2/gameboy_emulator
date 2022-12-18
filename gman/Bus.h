#pragma once

#include <stdint.h>
#include <fstream>
#include "Cartridge.h"
#include "PPU.h"
#include "APU.h"
#include "Timer.h"

class Bus
{
	friend class GMan;
public:
	Bus(Cartridge& cart, PPU& ppu, APU& apu, Timer& timer);
	uint8_t Read(uint16_t addr);
	uint16_t Read16(uint16_t addr);
	void Write(uint16_t addr, uint8_t val);
	void Write16(uint16_t addr, uint16_t val);
	void BindBootRom(uint8_t* pRom, size_t size);
private:
	uint8_t ie = 0xE0;
	uint8_t wram[0x2000];
	uint8_t hram[0x80];
	Cartridge& cart;
	PPU& ppu;
	APU& apu;
	Timer& timer;
	uint8_t joy = 0xFF;
	uint8_t* boot;
	size_t bootSize;
	bool bootRom;
	uint8_t* keys;
};