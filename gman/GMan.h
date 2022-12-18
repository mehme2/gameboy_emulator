#pragma once

#include "Bus.h"
#include "LR35902.h"  
#include "PPU.h"
#include "APU.h"
#include <fstream>

class GMan
{
public:
	GMan();
	void DoFrame();
	void Tick();
	void SetPixelBuffer(void* ptr);
	void SetSoundBuffer(void* ptr, uint16_t samp, uint16_t frequency);
	void LoadRom(const char* path);
	void LoadBootRom(const char* path);
	void BindKeyPtr(uint8_t* ptr);
	const char* GetTitle();
private:
	Bus bus;
	LR35902 cpu;
	PPU ppu;
	APU apu;
	Timer timer;
	Cartridge cart;
	void* ptr = nullptr;
	uint8_t* pBoot = nullptr;
};