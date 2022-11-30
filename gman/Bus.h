#pragma once

#include <stdint.h>
#include <fstream>

class Bus
{
	friend class GMan;
public:
	Bus()
	{
		memBuf = new uint8_t[0x10000]; 
		memBuf[0xFF41] = 0x00;
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
		if (addr < 0x4000)
		{
			if (rom[0x0147] == 0x01)
			{
				if (addr < 0x2000)
				{
					ramEnabled = val == 0x0A;
				}
				else
				{
					uint8_t mask = 0x1F;
					while ((val & mask) > romSize / 0x4000)
					{
						mask >>= 1;
					}
					uint16_t bank = val & mask;
					if (bank == 0) bank = 1;
					memcpy(memBuf + 0x4000, rom + (bank * 0x4000), 0x4000);
				}
			}
			return;
		}
		else if (addr < 0x8000)
		{
			return;
		}
		else if (addr < 0xA000)
		{

		}
		else if (addr < 0xC000)
		{

		}
		else if (addr < 0xD000)
		{

		}
		else if (addr < 0xFE00)
		{

		}
		else if (addr < 0xFF00)
		{

		}
		else if (addr < 0xFF80)
		{

		}
		else if (addr < 0xFFFF)
		{

		}
		else
		{

		}
		if (addr == 0xFF46 && val<=0xDF)
		{
			memcpy(memBuf + 0xFE00, memBuf + int(val) * 0x100, 0x100);
		}
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
	void BindRom(void* pRom, size_t size)
	{
		romSize = size;
		rom = (uint8_t*)pRom;
		memcpy(memBuf, rom, 0x8000);
	}
private:
	uint8_t* memBuf;
	uint8_t* rom;
	bool ramEnabled;
	size_t romSize;
};