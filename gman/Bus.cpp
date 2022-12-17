#include "Bus.h"

Bus::Bus(Cartridge& cart)
	:
	cart(cart)
{
	memBuf = new uint8_t[0x10000];
	memset(memBuf, 0, 0x10000);
	memBuf[0xFF41] = 0x81;
	memBuf[0xFF04] = 0xAB;
}

Bus::~Bus()
{
	delete[] memBuf;
}

uint8_t Bus::Read(uint16_t addr)
{
	if (bootRom && addr < 0x100 && memBuf[0xFF50] == 0)
	{
		return boot[addr];
	}
	if (addr < 0x8000)
	{
		return cart.Read(addr);
	}
	else if (addr < 0xA000)
	{
		if (vramLock && (memBuf[0xFF40] & 0x80) != 0)
		{
			return 0xFF;
		}
	}
	else if (addr < 0xC000)
	{
		return cart.Read(addr);
	}
	else if (addr < 0xD000)
	{

	}
	else if (addr < 0xE000)
	{

	}
	else if (addr < 0xFE00)
	{
		return memBuf[addr - 0x2000];
	}
	else if (addr < 0xFEA0)
	{

	}
	else if (addr < 0xFF00)
	{
		return 0xFF;
	}
	if (addr == 0xFF00)
	{
		if ((memBuf[0xFF00] & 0x20) == 0)
		{
			memBuf[0xFF00] = 0xC0 | (memBuf[0xFF00] & 0x30) | ((*keys >> 4) & 0x0F);
		}
		else
		{
			memBuf[0xFF00] = 0xC0 | (memBuf[0xFF00] & 0x30) | (*keys & 0x0F);
		}
	}
	switch (addr)
	{
	case 0xFF13:
	case 0xFF18:
	case 0xFF1B:
	case 0xFF1D:
	case 0xFF20:
		return 0xFF;
	}
	return memBuf[addr];
}

uint8_t Bus::PPURead(uint16_t addr)
{
	if ((addr >= 0x8000 && addr < 0xA000) || (addr >= 0xFE00 && addr < 0xFEA0) || (addr >= 0xFF40 && addr < 0xFF4C) || addr==0xFF0F)
	{
		return memBuf[addr];
	}
	return 0xFF;
}

uint16_t Bus::Read16(uint16_t addr)
{
	uint16_t low = Read(addr);
	uint16_t high = Read(addr + 1);
	high <<= 8;
	high |= low;
	return high;
}

void Bus::Write(uint16_t addr, uint8_t val)
{
	if (addr < 0x8000)
	{
		cart.Write(addr, val);
		return;
	}
	else if (addr < 0xA000)
	{
		if (vramLock && (memBuf[0xFF40] & 0x80) != 0)
		{
			return;
		}
	}
	else if (addr < 0xC000)
	{
		cart.Write(addr, val);
		return;
	}
	else if (addr < 0xD000)
	{

	}
	else if (addr < 0xE000)
	{

	}
	else if (addr < 0xFE00)
	{
		Write(addr - 0x2000, val);
	}
	else if (addr < 0xFEA0)
	{

	}
	else if (addr < 0xFF00)
	{
		return;
	}

	switch (addr)
	{
	case 0xFF00:
		memBuf[0xFF00] = (memBuf[0xFF00] & 0x0F) | (val & 0x30) | 0xC0;
		if ((memBuf[0xFF00] & 0x20) == 0)
		{
			memBuf[0xFF00] = (memBuf[0xFF00] & 0x30) | ((*keys >> 4) & 0x0F);
		}
		else
		{
			memBuf[0xFF00] = (memBuf[0xFF00] & 0x30) | (*keys & 0x0F);
		}
		return;
	case 0xFF04:
		memBuf[0xFF04] = 0;
		memBuf[0xFF05] = memBuf[0xFF06];
		return;
	//case 0xFF10:
	//	memBuf[0xFF10] = 0x80 | (val & 0x7F);
	//	return;
	//case 0xFF14:
	//	memBuf[0xFF14] = 0x38 | (val & 0xC7);
	//	return;
	//case 0xFF19:
	//	memBuf[0xFF19] = 0x38 | (val & 0xC7);
	//	return;
	//case 0xFF1A:
	//	memBuf[0xFF1A] = 0x7F | (val & 0x80);
	//	return;
	//case 0xFF1C:
	//	memBuf[0xFF1C] = 0x8F | (val & 0x70);
	//	return;
	//case 0xFF1E:
	//	memBuf[0xFF1E] = 0x38 | (val & 0xC7);
	//	return;
	//case 0xFF20:
	//	memBuf[0xFF20] = 0xE0 | (val & 0x1F);
	//	return;
	//case 0xFF23:
	//	memBuf[0xFF23] = 0x3F | (val & 0xC0);
	//	return;
	case 0xFF26:
		memBuf[0xFF26] = 0x70 | (memBuf[0xFF26] & 0x0F) | (val & 0x80);
		return;
	case 0xFF41:
		memBuf[0xFF41] = (memBuf[0xFF41] & 0x07) | (val & 0xF8);
		return;
	case 0xFF44:
		return;
	case 0xFF46:
		memcpy(memBuf + 0xFE00, memBuf + val * 0x100, 0xA0);
		break;
	case 0xFF50:
		bootRom = false;
		break;
	}
	memBuf[addr] = val;
 }

void Bus:: Write16(uint16_t addr, uint16_t val)
{
	uint8_t low = val & 0x00FF;   
	uint8_t high = (val & 0xFF00) >> 8;
	Write(addr, low);
	Write(addr + 1, high);
}

void Bus::BindBootRom(uint8_t* pRom, size_t size)
{
	bootSize = size;
	boot = pRom;
	bootRom = true;
}

void Bus::PPUWrite(uint16_t addr, uint8_t val)
{
	if ((addr >= 0x8000 && addr < 0xA000) || (addr >= 0xFE00 && addr < 0xFEA0) || (addr >= 0xFF40 && addr < 0xFF4C) || addr == 0xFF0F)
	{
		memBuf[addr] = val;
	}
}

void Bus::APUWrite(uint16_t addr, uint8_t val)
{
	if (addr >= 0xFF10 && addr < 0xFF40)
	{
		memBuf[addr] = val;
	}
}

uint8_t Bus::APURead(uint16_t addr)
{
	if (addr >= 0xFF10 && addr < 0xFF40)
	{
		return memBuf[addr];
	}
	return 0xFF;
}
