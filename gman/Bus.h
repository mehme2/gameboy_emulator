#pragma once

#include <stdint.h>
#include <fstream>

class Bus
{
	friend class GMan;
	friend class PPU;
public:
	Bus();
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
	uint8_t* memBuf;
	uint8_t* rom;
	uint8_t* boot;
	bool ramEnabled;
	size_t romSize;
	size_t bootSize;
	uint8_t trash = 0xFF;
	bool bootRom;
	uint8_t* keys;
	bool vramLock = false;
};