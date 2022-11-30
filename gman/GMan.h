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
private:
	Bus bus;
	LR35902 cpu;
	PPU ppu;          
	void* ptr = nullptr;
	uint8_t* pRom;
};