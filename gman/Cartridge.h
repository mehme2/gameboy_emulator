#pragma once

#include <stdint.h>

class Cartridge
{
public:
	void Init(uint8_t* pRom);
	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t val);
	const char* GetTitle();
private:
	uint8_t* rom = nullptr;
	uint8_t* ram = nullptr;
	char title[17];
	uint8_t type;
	uint8_t bankingMode = 0;
	uint8_t romSize;
	uint8_t ramSize;
	uint8_t bank = 1;
	uint8_t ramBank = 0;
	bool enableRam = false;
};