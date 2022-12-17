#pragma once

#include <stdint.h>
#include <fstream>
#include "Cartridge.h"

class Bus
{
	friend class GMan;
	friend class PPU;
public:
	Bus(Cartridge& cart);
	~Bus();
	uint8_t Read(uint16_t addr);
	uint8_t PPURead(uint16_t addr);
	uint16_t Read16(uint16_t addr);
	void Write(uint16_t addr, uint8_t val);
	void Write16(uint16_t addr, uint16_t val);
	void BindRom(uint8_t* pRom, size_t size);
	void BindBootRom(uint8_t* pRom, size_t size);
	void PPUWrite(uint16_t addr, uint8_t val);
	void APUWrite(uint16_t addr, uint8_t val);
	uint8_t APURead(uint16_t addr);
private:
	Cartridge& cart;
	uint8_t* memBuf;
	uint8_t* boot;
	size_t bootSize;
	bool bootRom;
	uint8_t* keys;
	bool vramLock = false;
};