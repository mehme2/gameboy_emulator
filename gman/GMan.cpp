#include "GMan.h"

GMan::GMan()
	:
	bus(cart, ppu, apu, timer),
	cpu(bus)
{}

void GMan::DoFrame()
{
	while(!ppu.endFrame)
	{
		Tick();
	}
	ppu.endFrame = false;
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
	apu.Tick();
	cart.Tick();
	timer.Tick();
}

void GMan::SetPixelBuffer(void* ptr)
{
	this->ptr = ptr;
	ppu.BindPixelBuffer(ptr);
}

void GMan::SetSoundBuffer(void* ptr, uint16_t samp, uint16_t frequency)
{
	apu.BindBuffer((uint8_t*)ptr, samp, frequency);
}

void GMan::LoadRom(const char* path)
{
	cart.Init(path);
}

void GMan::LoadBootRom(const char* path)
{
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

void GMan::BindKeyPtr(uint8_t* ptr)
{
	bus.keys = ptr;
}

const char* GMan::GetTitle()
{
	return cart.GetTitle();
}