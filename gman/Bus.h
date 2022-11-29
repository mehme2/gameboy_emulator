#pragma once

#include <stdint.h>

class Bus
{
public:
	Bus()
	{
		memBuf = new uint8_t[0x10000]; 
	}
	~Bus()
	{
		delete[] memBuf; 
	}
	uint8_t Read(uint16_t addr)
	{
		if (addr >= 0 && addr <= 0xFF)
		{ 
			return memBuf[addr];
		}
		else
		{
			return 0xFF;
		}
	}
	void Write(uint16_t addr, uint8_t val)
	{
		if (addr >= 0 && addr <= 0xFF)
		{
		 memBuf[addr] = val;
		}
	}
	void Write16(uint16_t addr, uint16_t val)
	{
		uint8_t low = val & 0x00FF;
		uint8_t high = (val & 0xFF00)>>8;
		Write(addr, low);
		Write(addr + 1, high);
	}
private:
	uint8_t* memBuf;
};