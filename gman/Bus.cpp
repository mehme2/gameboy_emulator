#include "Bus.h"

Bus::Bus(Cartridge& cart, PPU& ppu, APU& apu, Timer& timer)
	:
	cart(cart),
	ppu(ppu),
	apu(apu),
	timer(timer)
{}

uint8_t Bus::Read(uint16_t addr)
{
	uint8_t ret = 0xFF;
	if (bootRom && addr < 0x100)
	{
		ret = boot[addr];
	}
	if (addr < 0x8000)
	{
		ret = cart.Read(addr);
	}
	else if (addr < 0xA000)
	{
		ret = ppu.Read(addr);
	}
	else if (addr < 0xC000)
	{
		ret = cart.Read(addr);
	}
	else if (addr < 0xD000)
	{
		ret = wram[addr - 0xC000];
	}
	else if (addr < 0xE000)
	{
		ret = wram[addr - 0xC000];
	}
	else if (addr < 0xFE00)
	{
		ret = wram[addr - 0xE000];
	}
	else if (addr < 0xFEA0)
	{
		ret = ppu.Read(addr);
	}
	else if (addr < 0xFF00)
	{
	}
	else if (addr == 0xFF00)
	{
		if ((joy & 0x20) == 0)
		{
			joy = 0xC0 | (joy & 0x30) | ((*keys >> 4) & 0x0F);
		}
		else
		{
			joy = 0xC0 | (joy & 0x30) | (*keys & 0x0F);
		}
		ret = joy;
	}
	else if (addr < 0xFF03)
	{

	}
	else if (addr >= 0xFF04 && addr < 0xFF08)
	{
		ret = timer.Read(addr);
	}
	else if (addr == 0xFF0F)
	{
		ret = 0xE0;
		ret |= ppu.Read(addr);
		ret |= timer.Read(addr);
	}
	else if (addr >= 0xFF10 && addr < 0xFF40)
	{
		ret = apu.Read(addr);
	}
	else if (addr < 0xFF4C)
	{
		ret = ppu.Read(addr);
	}
	else if (addr == 0xFF50)
	{

	}
	else if (addr >= 0xFF80 && addr < 0xFFFF)
	{
		ret = hram[addr - 0xFF80];
	}
	else if (addr == 0xFFFF)
	{
		ret = ie;
	}
	return ret;
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
	}
	else if (addr < 0xA000)
	{
		ppu.Write(addr, val);
	}
	else if (addr < 0xC000)
	{
		cart.Write(addr, val);
	}
	else if (addr < 0xD000)
	{
		wram[addr - 0xC000] = val;
	}
	else if (addr < 0xE000)
	{
		wram[addr - 0xC000] = val;
	}
	else if (addr < 0xFE00)
	{
		wram[addr - 0xE000] = val;
	}
	else if (addr < 0xFEA0)
	{
		ppu.Write(addr, val);
	}
	else if (addr < 0xFF00)
	{
	} 
	else if (addr == 0xFF00)
	{
		joy = (joy & 0x0F) | (val & 0x30) | 0xC0;
		if ((joy & 0x20) == 0)
		{
			joy = (joy & 0x30) | ((*keys >> 4) & 0x0F);
		}
		else
		{
			joy = (joy & 0x30) | (*keys & 0x0F);
		}
	}
	else if (addr < 0xFF03)
	{
		
	}
	else if (addr >= 0xFF04 && addr < 0xFF08)
	{
		timer.Write(addr, val);
	}
	else if (addr == 0xFF0F)
	{
		ppu.Write(addr, val);
		timer.Write(addr, val);
	}
	else if (addr >= 0xFF10 && addr < 0xFF40)
	{
		apu.Write(addr, val);
	}
	else if (addr < 0xFF4C)
	{
		ppu.Write(addr, val);
		if (addr == 0xFF46 && val < 0xE0)
		{
			uint8_t buf[0xA0];
			for (uint8_t i = 0;i < 0xA0;i++)
			{
				buf[i] = Read((val << 8) | i);
			}
			ppu.DMA(buf);
		}
	}
	else if (addr == 0xFF50)
	{
		bootRom = false;
	}
	else if (addr >= 0xFF80 && addr < 0xFFFF)
	{
		hram[addr - 0xFF80] = val;
	}
	else if (addr == 0xFFFF)
	{
		ie = (ie & 0xE0) | (val & 0x1F);
	}
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
