#pragma once

#include "Bus.h"

class PPU
{
	friend class GMan;
	struct Sprite
	{
		uint8_t y;
		uint8_t x;
		uint8_t index;
		uint8_t flags;
	};
	struct Pixel
	{
		uint8_t color;
		uint8_t palette;
		uint8_t bgPriority;
	};
	struct Color
	{
		unsigned char b;
		unsigned char g;
		unsigned char r;
		unsigned char a;
	}; 
public:
	PPU(Bus& bus);   
	void BindPixelBuffer(void* buf);
	void Tick();
private:
	void SetMode(int mode); 
	void UpdateStat();
private:
	int tickCounter = 0;
	bool endFrame = false;
	Bus& bus;
	uint8_t* pBuffer;
	int mode = 2;
	int sleep = 0;
	int oamIndex = 0;
	int sprIndex = 0;   
	int fetchStep = 1;
	bool wy = false;
	uint8_t x = 0;
	uint8_t fetcherX = 0;
	uint8_t fetcherY = 0;
	uint16_t fetchAddr = 0;
	uint8_t fetchLow;
	uint8_t fetchHigh;
	Sprite sprites[10];
	Pixel bgFIFO[16];
	Pixel sprFIFO[16];
	bool statLine = false;
};