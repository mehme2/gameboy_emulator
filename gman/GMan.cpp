#include "GMan.h"

GMan::GMan()
	:
	cpu(bus),
	ppu(bus)
{
}

void GMan::DoFrame()
{
	static int divCount;
	static int timCount;
	for (int y = 0; y < 154;y++)
	{
		bus.memBuf[0xFF44] = y;
 		if (y < 144)
		{
			ppu.SetMode(2);
		}
		else if (y == 144)
		{
			ppu.SetMode(1);
		}
		for (int i = 0; i < 456; i++)
		{
			Tick();
			divCount = (divCount + 1) & 256;
			if (divCount == 0)
			{
				bus.memBuf[0xFF04]++;
			}
			if ((bus.memBuf[0xFF07] & 0x04) != 0)
			{
				int mod;
				switch (bus.memBuf[0xFF07] & 0x03)
				{
					case 0:
						mod = 1024;
						break;
					case 1:
						mod = 16;
						break;
					case 2:
						mod = 64;
						break;
					case 3:
						mod = 256;
						break;
				}
				timCount = (timCount + 1) & mod;
				if (timCount == 0)
				{
					bus.memBuf[0xFF05]++;
					if (bus.memBuf[0xFF05] == 0)
					{
						bus.memBuf[0xFF0F] |= 0x04;
					}
				}
			}
		}
	}
	ppu.SetMode(2);
	bus.memBuf[0xFF44] = 0;
	//memcpy(ptr, bus.memBuf + 0x0000, 0xFFFF);
	/*for (int y = 0; y < 144;y++)
	{
		for (int x = 0;x < 160;x++)
		{
			((int*)ptr)[(y * 160 + x)] = 
				(bus.memBuf[0x8000 + ((y / 8) * 32 + x / 8) * 16 + ((y % 8) * 2)] >> (7 - (x % 8)) & 0x1) == 0 ? 0 : 0xFFFFFFFF;
		}
	}*/
}

void GMan::Tick()
{
	cpu.Tick();
	ppu.Tick();
}

void GMan::SetPixelBuffer(void* ptr)
{
	this->ptr = ptr;
	ppu.BindPixelBuffer(ptr);
}

void GMan::LoadRom(const char* path)
{
	uint8_t b;
	std::ifstream rom;
	rom.open(path, std::ios::binary);
	rom.seekg(0, rom.end);
	size_t size = rom.tellg();
	rom.seekg(0, rom.beg);
	pRom = new uint8_t[size];
	/*for (int i = 0;i < size;i++)
	{
	rom >> b;
	pRom[i] = b;
	}*/
	rom.read((char*)pRom, size);
	rom.close();
	bus.BindRom(pRom, size);
}

void GMan::LoadBootRom(const char* path)
{
	uint8_t b;
	std::ifstream rom;
	rom.open(path, std::ios::binary);
	rom.seekg(0, rom.end);
	size_t size = rom.tellg();
	rom.seekg(0, rom.beg);
	pBoot = new uint8_t[size];
	rom.read((char*)pBoot, size);
	rom.close();
	bus.BindBootRom(pBoot, size);
	cpu.PC = 0x0000;
}
