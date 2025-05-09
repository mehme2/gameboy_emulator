#pragma once

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
	PPU();   
	void BindPixelBuffer(void* buf);
	void Tick();
public:
	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t val);
	void DMA(uint8_t* addr);
private:
	void SetMode(int mode); 
	void UpdateStat();
private:
	int tickCounter = 0;
	bool endFrame = false;
	uint8_t* pBuffer;
	int mode = 2;
	int sleep = 0;
	int oamIndex = 0;
	int fetchStep = 1;
	int sprIndex = -1;
	bool lcdOff = false;
	bool wyc = false;
	bool stopFifo = false;
	int x = 0;
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
private:
	uint8_t vram[0x2000];
	uint8_t oam[0xA0];
	uint8_t lcdc = 0x81;
	uint8_t stat;
	uint8_t scy = 0;
	uint8_t scx = 0;
	uint8_t ly = 0;
	uint8_t lyc;
	uint8_t wy;
	uint8_t wx;
	uint8_t bgp;
	uint8_t obp0;
	uint8_t obp1;
	uint8_t _if = 0xE0;
};