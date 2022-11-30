#include "GMan.h"

GMan::GMan()
	:
	cpu(bus),
	ppu(bus)
{
}

void GMan::DoFrame()
{
	for (int y = 0; y < 154;y++)
	{
		for (int i = 0; i < 456; i++)
		{
			Tick();
		}
		if (ppu.mode != 1)
		{
			ppu.SetMode(2);
		}
	}
	ppu.SetMode(2);
	bus.Write(0xFF44, 0);
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
