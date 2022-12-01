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
		for (int i = 0; i < 456; i++)
		{
			Tick();
			divCount = (divCount + 1) & 256;
			if (divCount = 0)
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
		if (ppu.mode != 1)
		{
			ppu.SetMode(2);
		}
	}
	ppu.SetMode(2);
	bus.memBuf[0xFF44] = 0;
	memcpy(ptr, bus.memBuf + 0x0000, 0xFFFF);
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
