#pragma once

#include "Bus.h"
#include "LR35902.h"
#include <fstream>

class GMan
{
public:
	GMan()
		:
		cpu(bus)
	{
	}
	void DoFrame()
	{
		for (int i = 0; i < 70224; i++)
		{
			Tick();
		}
		memcpy(ptr, bus.memBuf + 0x0000, 0xFFFF);
	}
	void Tick()
	{
		cpu.Tick();
	}
	void SetPixelBuffer(void* ptr)
	{
		this->ptr = ptr;
	}
	void LoadRom(const char* path)
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
private:
	Bus bus;
	LR35902 cpu;
	void* ptr = nullptr;
	uint8_t* pRom;
};