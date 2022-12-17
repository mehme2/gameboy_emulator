#include "Cartridge.h"
#include <fstream>

void Cartridge::Init(const char* path)
{
	this->path = path;
	std::ifstream from;
	from.open(path, std::ios::binary);
	from.seekg(0, from.end);
	size_t size = from.tellg();
	rom = new uint8_t[size];
	from.seekg(0, from.beg);
	from.read((char*)rom, size);
	from.close();
	for (uint16_t addr = 0x0134;addr <= 0x0143;addr++)
	{
		title[addr - 0x0134] = (char)rom[addr];
	}
	title[16] = 0;
	type = rom[0x0147];
	romSize = rom[0x0148];
	ramSize = rom[0x0149];
	if (type == 5 || type == 6)
	{
		ramSize = 0;
		ram = new uint8_t[512];
	}
	switch (ramSize)
	{
	case 2:
		ram = new uint8_t[0x2000];
		break;
	case 3:
		ram = new uint8_t[0x8000];
		break;
	case 4:
		ram = new uint8_t[0x20000];
		break;
	case 5:
		ram = new uint8_t[0x10000];
		break;
	}
	if (type == 3)
	{
		std::ifstream saveFile;
		std::string file = path;
		if (file.find_last_of('.') == file.length() - 3)
		{
			file.erase(file.end() - 1);
			file.erase(file.end() - 1);
			file.erase(file.end() - 1);
		}
		file += ".sav";
		saveFile.open(file, std::ios::binary);
		if (saveFile)
		{
			saveFile.read((char*)ram, ramSize == 3 ? 0x8000 : 0x2000);
			saveFile.close();
		}
	}
}

uint8_t Cartridge::Read(uint16_t addr)
{
	uint8_t val = 0xFF;
	switch (type)
	{
	case 0x00:
		if (addr < 0x8000) val = rom[addr];
		if (ramSize == 2 && addr >= 0xA000 && addr < 0xC000)
		{
			val = ram[addr - 0xA000];
		}
		break;
	case 0x03:
	case 0x02:
		if (enableRam)
		{
			if (addr >= 0xA000 && addr < 0xC000)
			{
				if (ramSize == 3)
				{
					val = ram[addr - 0xA000 + (bankingMode * ramBank * 0x2000)];
				}
				else if(ramSize > 1)
				{
					val = ram[addr - 0xA000];
				}
			}
		}
	case 0x01:
		if (romSize < 5)
		{
			if (addr < 0x4000)val = rom[addr];
			else if (addr < 0x8000)val = rom[(addr + 0x4000 * (bank - 1) + (bankingMode * ramBank * 0x180000)) & (0x1FFFFF >> (6 - romSize))];
		}
		else
		{
			if (addr < 0x4000)val = rom[addr + (bankingMode * ramBank * 0x180000)];
			else if (addr < 0x8000)val = rom[addr + 0x4000 * (bank - 1) + (bankingMode * ramBank * 0x180000)];
		}
		break;
	case 0x05:
	case 0x06:
		if (addr < 0x4000)
		{
			val = rom[addr];
		}
		else if (addr < 0x8000)
		{
			val = rom[addr + (bank - 1) * 0x4000];
		}
		else if (addr >= 0xA000 && addr < 0xC000 && enableRam) 
		{
			val = ram[(addr - 0xA000) % 0x200];
			val |= 0xF0;
		}
		break;
	}
	return val;
}

void Cartridge::Write(uint16_t addr, uint8_t val)
{
	switch (type)
	{
	case 0x00:
		if (ramSize == 2 && addr >= 0xA000 && addr < 0xC000)
		{
			ram[addr - 0xA000] = val;
		}
		break;
	case 0x03:
	case 0x02:
		if (addr < 0x2000)
		{
			enableRam = ((val & 0x0F) == 0x0A);
		}
		if (addr >= 0x4000 && addr < 0x6000)
		{
			ramBank = val & 0x03;
		}
		else if (addr >= 0x6000 && addr < 0x8000)
		{
			bankingMode = val & 0x01;
		}
		if (enableRam)
		{
			if (addr >= 0xA000 && addr < 0xC000)
			{
				if (ramSize == 3)
				{
					ram[addr - 0xA000 + (bankingMode * ramBank * 0x2000)] = val;
				}
				else
				{
					ram[addr - 0xA000] = val;
				}
			}
		}
	case 0x01:
		if (addr >= 0x2000 && addr < 0x4000)
		{
			uint8_t mask = ((2 << romSize) - 1) & 0x1F;
			bank = val & mask;
			if ((val & 0x1F) == 0) bank = 1;
 		}
		break;
	case 0x05:
	case 0x06:
		if (addr < 0x4000)
		{
			if ((addr & 0x0100) == 0)
			{
				enableRam = ((val & 0x0F) == 0x0A);
			}
			else
			{
				uint8_t mask = ((2 << romSize) - 1) & 0x0F;
				bank = val & mask;
				if ((val & 0x0F) == 0) bank = 1;
			}
		}
		else if (addr >= 0xA000 && addr < 0xC000 && enableRam)
		{
			ram[(addr - 0xA000) % 0x200] = val;
		}
		break;
	}
}

const char* Cartridge::GetTitle()
{
	return title;
}

void Cartridge::Tick()
{
	counter++;
	if (counter % 41943040 == 0 && type == 3)
	{
		std::ofstream saveFile;
		std::string file = path;
		if (file.find_last_of('.') == file.length() - 3)
		{
			file.erase(file.end() - 1);
			file.erase(file.end() - 1);
			file.erase(file.end() - 1);
		}
		file += ".sav";
		saveFile.open(file, std::ios::binary);
		saveFile.write((char*)ram, ramSize == 3 ? 0x8000 : 0x2000);
		saveFile.close();
	}
}