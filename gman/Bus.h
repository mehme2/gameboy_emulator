#pragma once

#include <stdint.h>
#include <fstream>

class Bus
{
	friend class GMan;
public:
	Bus();
	~Bus();
	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t val);
	void Write16(uint16_t addr, uint16_t val);
	void BindRom(void* pRom, size_t size);
private:
	uint8_t* memBuf;
	uint8_t* rom;
	bool ramEnabled;
	size_t romSize;
};