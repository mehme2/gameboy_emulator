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
	if (mode != 1 && mode != 0 && (bus.Read(LCDC) & 0x80) != 0)
	{
		if (sleep <= 0)
		{
			switch (mode)
			{
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
					if (bus.Read(LY) == bus.Read(WY))
					{
						wy = true;
					}
					bool window = wy && (bus.Read(LCDC) & 0x20) >> 5 && wy && (bus.Read(WX) - x * 8) < 8 && ((bus.Read(WX) - x * 8) >= 0);
					fetchAddr = 0x9800;
 					if ((window && ((bus.Read(LCDC) & 0x40) >> 6)) || (!window && ((bus.Read(LCDC) & 0x08) >> 3)))
					{
						  fetchAddr = 0x9C00;
					}
					fetcherX = !window ? bus.Read(SCX) / 8 + x : (x * 8 - (bus.Read(WX) - 7)) / 8;
					fetcherX &= 0x1F;
  					fetcherY = !window ? bus.Read(SCY) + bus.Read(LY) : bus.Read(LY) - (bus.Read(WY) - 7);
					fetcherY &= 0xFF;
					//fetcherX = x + bus.Read(SCX)/8; 
					//fetcherY = bus.Read(LY) + bus.Read(SCY);
					sleep = 2;
					fetchStep++;
				}
				break;
				case 2:
				{
					bool lcdc4 = ((bus.Read(LCDC) & 0x10)) >> 4;
					uint8_t tileIndex = bus.Read(fetchAddr + ((fetcherY / 8) * 32 + fetcherX));
					uint16_t tileAddr = lcdc4 ? 0x8000 + tileIndex * 16 + (fetcherY % 8) * 2
						: 0x9000 + char(tileIndex) * 16 + (fetcherY % 8) * 2;
					fetchLow = bus.Read(tileAddr);
					fetchHigh = bus.Read(tileAddr + 1);
					sleep = 4;
					fetchStep++;
				}
				break;
				case 3:
					sleep = 2;
					fetchStep++;
					break;
				case 4:
				{
					for (int i = 7; i >= 0; i--)
					{
						Pixel px;
						px.color = ((fetchLow >> i) & 0x01) | ((fetchHigh >> i) << 1 & 0x02);
						bgFIFO[7 - i] = px;
					}
					sleep = 2;
					fetchStep++;
				}
				break;
				case 5:
				{
					auto y = bus.Read(LY);
					auto palette = bus.Read(BGP);
					for (int i = 0; i < 8; i++)
					{
						uint8_t color = (palette >> (bgFIFO[i].color * 2)) & 0x03;
						switch (color)
						{
						case 0:
							((Color*)pBuffer)[y * 160 + x * 8 + i] = { 0xFF,0xFF,0xFF,0xFF };
							break;
						case 1:
							((Color*)pBuffer)[y * 160 + x * 8 + i] = { 0xAA,0xAA,0xAA,0xAA };
							break;
						case 2:
							((Color*)pBuffer)[y * 160 + x * 8 + i] = { 0x55,0x55,0x55,0x55 };
							break;
						case 3:
							((Color*)pBuffer)[y * 160 + x * 8 + i] = { 0x00,0x00,0x00,0x00 };
							break;
						}
					}
					fetchStep = 1;
					x++;
					if (x > 20)
					{
						SetMode(0);  
						x = 0;
					}
				}
				break;
				}
			}
			break;
			}
		}
		else
		{
			sleep--;
		}
	}
}
 
void PPU::SetMode(int mode)
{
	if (mode != this->mode)
	{
		this->mode = mode;
		bus.PPUWrite(STAT, mode | (bus.Read(STAT) & 0xFC));
		if (mode == 1)
		{
			bus.PPUWrite(IF, bus.Read(IF) | 0x01);
			wy = false;
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
		bus.PPUWrite(IE, bus.Read(IE) | 0x02);
	}
}
