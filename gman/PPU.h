#pragma once

#include "Bus.h"
#include <vector>

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
		bool obj = false;
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
	int fetchStep = 1;
	int sprIndex = -1;
	bool lcdOff = false;
	bool wy = false;
	bool stopFifo = false;
	uint8_t x = 0;
	uint8_t fetcherX = 0;
	uint8_t fetcherY = 0;
	uint16_t fetchAddr = 0;
	uint8_t fetchLow;
	uint8_t fetchHigh;
	std::vector<Sprite> sprites{ 10 };
	std::vector<Pixel> fifo{ 16 };
	bool statLine = false;
	uint8_t windowLineCounter = 0;
	bool window = false;
	uint8_t shift = 0;
};