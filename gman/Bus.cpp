#include "Bus.h"

Bus::Bus()
{
	memBuf = new uint8_t[0x10000];
	memset(memBuf, 0, 0x10000);
}

Bus::~Bus()
{
	delete[] memBuf;
}

uint8_t Bus::Read(uint16_t addr)
{
	if (addr >= 0 && addr <= 0xFFFF)
	{
		return memBuf[addr];
	}
	else
	{
		return 0xFF;
	}
}

void Bus::Write(uint16_t addr, uint8_t val)
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

	switch (addr)
	{
	case 0xFF04:
		memBuf[0xFF04] = 0;
		return;
	case 0xFF41:
		memBuf[0xFF41] = (memBuf[0xFF41] & 0x07) | (val & 0xF8);
		return;
	case 0xFF44:
		return;
	case 0xFF46:
		memcpy(memBuf + 0xFE00, memBuf + val * 0x100, 0x100);
		break;
	}
	if (addr >= 0 && addr <= 0xFFFF)
	{
		memBuf[addr] = val;
	}
}

void Bus::Write16(uint16_t addr, uint16_t val)
{
	uint8_t low = val & 0x00FF;
	uint8_t high = (val & 0xFF00) >> 8;
	Write(addr, low);
	Write(addr + 1, high);
}

void Bus::BindRom(void* pRom, size_t size)
{
	romSize = size;
	rom = (uint8_t*)pRom;
	memcpy(memBuf, rom, 0x8000);
}

void Bus::PPUWrite(uint16_t addr, uint8_t val)
{
	if (addr >= 0x8000 && addr <= 0xFFFF)
	{
		memBuf[addr] = val;
	}
}
