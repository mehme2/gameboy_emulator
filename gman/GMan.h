#pragma once

#include "Bus.h"
#include "LR35902.h"  
#include "PPU.h"
#include <fstream>

class GMan
{
public:
	GMan();
	void DoFrame();
	void Tick();
	void SetPixelBuffer(void* ptr);
	void LoadRom(const char* path);
	void LoadBootRom(const char* path);
	void BindKeyPtr(uint8_t* ptr);
private:
	Bus bus;
	LR35902 cpu;
	PPU ppu;
	void* ptr = nullptr;
	uint8_t* pRom = nullptr;
	uint8_t* pBoot = nullptr;
	int divCount = 0xCC;
	int timCount = 0;
};