#include "PPU.h"

#define OAM 0xFE00
#define LCDC 0xFF40
#define STAT 0xFF41
#define SCY 0xFF42
#define SCX 0xFF43
#define LY 0xFF44
#define LYC 0xFF45
#define BGP 0xFF47
#define WY 0xFF4A
#define WX 0xFF4B
#define IF 0xFF0F
#define IE 0xFFFF

PPU::PPU(Bus& bus)
	:
	bus(bus)
{}


void PPU::BindPixelBuffer(void* buf)
{
	pBuffer = (uint8_t*)buf;
}

void PPU::Tick()
{
	tickCounter = (tickCounter + 1) % 456;
	if (tickCounter == 0)
	{
		bus.PPUWrite(LY, (bus.Read(LY) + 1) % 154);
		if (bus.Read(LY) == 0)
		{
			endFrame = true;
		}
		if (bus.Read(LY) < 144)
		{
			SetMode(2);
		}
		else if (mode != 1)
		{
			SetMode(1);
		}
	}
	if (sleep <= 0)
	{
		switch (mode)
		{
		case 0:
			sleep = 1;
			break;
		case 1:
			sleep = 1;
			break;
		case 2:
		{
			if (sprIndex < 10 && sprIndex >= 0)
			{
				uint8_t ly = bus.Read(LY);
				uint8_t size = 8 + 8 * ((bus.Read(LCDC) & 0x4) >> 2);
				Sprite spr;
				spr.y = bus.Read(OAM + oamIndex * 4);
				spr.x = bus.Read(OAM + oamIndex * 4 + 1);
				spr.index = bus.Read(OAM + oamIndex * 4 + 2);
				spr.flags = bus.Read(OAM + oamIndex * 4 + 3);
				if ((spr.y - ly < size && spr.y >= ly))
				{
					sprites[sprIndex] = spr;
					sprIndex++;
				}
			}
			oamIndex++;
			if (oamIndex > 40)
			{
				oamIndex = 0;
				sprIndex = 0;
				SetMode(3);
			}
			sleep = 2;
		}
		break;
		case 3:
		{
			switch (fetchStep)
			{
			case 1:
			{
				fetchAddr = 0x9800;
 				if ((window && ((bus.Read(LCDC) & 0x40) >> 6)) || (!window && ((bus.Read(LCDC) & 0x08) >> 3)))
				{
					  fetchAddr = 0x9C00;
				}
				fetcherX = !window ? bus.Read(SCX) + x : (x - (bus.Read(WX) - 7));
				fetcherY = !window ? bus.Read(SCY) + bus.Read(LY) : windowLineCounter;
				sleep = 2;
				fetchStep++;
			}
			break;
			case 2:
			{
				bool lcdc4 = ((bus.Read(LCDC) & 0x10)) >> 4;
				uint8_t tileIndex = bus.Read(fetchAddr + ((fetcherY / 8) * 32 + fetcherX / 8));
				uint16_t tileAddr = lcdc4 ? 0x8000 + tileIndex * 16 + (fetcherY % 8) * 2
					: 0x9000 + char(tileIndex) * 16 + (fetcherY % 8) * 2;
				fetchLow = bus.Read(tileAddr);
				fetchHigh = bus.Read(tileAddr + 1);
				sleep = 4;
				fetchStep++;
			}
			break;
			case 3:
			{
				if (fifo.size() <= 8)
				{
					for (int i = 7; i >= 0; i--)
					{
						Pixel px;
						px.color = ((fetchLow >> i) & 0x01) | ((fetchHigh >> i) << 1 & 0x02);
						fifo.push_back(px);
					}
					fetchStep = 1;
				}
			}
			break;
			}
			if (fifo.size() > 8)
			{
				auto palette = bus.Read(BGP);
				int index = bus.Read(LY) * 160 + x;
				uint8_t color = (palette >> (fifo.front().color * 2)) & 0x03;
				fifo.erase(fifo.begin());
				if ((bus.Read(LCDC) & 0x80) == 0 || (bus.Read(LCDC) & 0x01) == 0)
				{
					color = 0;
				}
				switch (color)
				{
				case 0:
					((Color*)pBuffer)[index] = { 0x0F,0xBC,0x9B,0xFF };
					break;
				case 1:
					((Color*)pBuffer)[index] = { 0x0F,0xAC,0x8B,0xFF };
					break;
				case 2:
					((Color*)pBuffer)[index] = { 0x30,0x62,0x30,0xFF };
					break;
				case 3:
					((Color*)pBuffer)[index] = { 0x0F,0x38,0x0F,0xFF };
					break;
				}
				x++;
				bool winOld = window;
				window = wy && (bus.Read(LCDC) & 0x20) != 0 && x >= (bus.Read(WX) - 7);
				if (window && !winOld)
				{
					windowLineCounter++;
					fifo.clear();
					fetchStep = 1;
				}
			}
			if (x >= 160)
			{
				SetMode(0);
				fifo.clear();
				x = 0;
			}
		}
		break;
		}
	}
	sleep--;
}
 
void PPU::SetMode(int mode)
{
	if (mode != this->mode)
	{
		this->mode = mode;
		bus.PPUWrite(STAT, (mode & 0x03) | (bus.Read(STAT) & 0xFC));
		if (mode == 1)
		{
			bus.PPUWrite(IF, bus.Read(IF) | 0x01);
			wy = false;
			windowLineCounter = -1;
		}
		if (mode == 2)
		{
			wy = wy || (bus.Read(LY) == bus.Read(WY));
			window = false;
		}
		UpdateStat();
	}
}

void PPU::UpdateStat()
{
	if (bus.Read(LY) == bus.Read(LYC))
	{
		bus.PPUWrite(STAT, bus.Read(STAT) | 0x04);
	}
	else
	{
		bus.PPUWrite(STAT, bus.Read(STAT) & ~0x04);
	}
	bool lineOld = statLine;
	auto stat = bus.Read(STAT);
	statLine = ((stat & 0x40) != 0 && (stat & 0x04) != 0) || (((stat & 0x20) != 0) && (mode == 2)) || (((stat & 0x10) != 0) && (mode == 1)) || (((stat & 0x08) != 0) && (mode == 0));
	if (lineOld == false && statLine == true)
	{
		bus.PPUWrite(IF, bus.Read(IF) | 0x02);
	}
}
