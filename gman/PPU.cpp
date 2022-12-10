#include "PPU.h"

#define OAM 0xFE00
#define LCDC 0xFF40
#define STAT 0xFF41
#define SCY 0xFF42
#define SCX 0xFF43
#define LY 0xFF44
#define LYC 0xFF45
#define BGP 0xFF47
#define OBP0 0xFF48
#define OBP1 0xFF49
#define WY 0xFF4A
#define WX 0xFF4B
#define IF 0xFF0F
#define IE 0xFFFF

PPU::PPU(Bus& bus)
	:
	bus(bus)
{
	fifo.clear();
	vram = new uint8_t[0x2000];
	oam = new uint8_t[0x100];
}


void PPU::BindPixelBuffer(void* buf)
{
	pBuffer = (uint8_t*)buf;
}

void PPU::Tick()
{
	tickCounter = (tickCounter + 1) % 456;
	if (tickCounter == 0)
	{
		bus.PPUWrite(LY, (bus.PPURead(LY) + 1) % 154);
		if (bus.PPURead(LY) == 0)
		{
			endFrame = true;
		}
		if (bus.PPURead(LY) < 144)
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
			if (sprites.size() < 10)
			{
				uint8_t ly = bus.PPURead(LY);
				uint8_t size = 8 + 8 * ((bus.PPURead(LCDC) & 0x4) >> 2);
				Sprite spr;
				spr.y = bus.PPURead(OAM + (oamIndex * 4));
				spr.x = bus.PPURead(OAM + (oamIndex * 4) + 1);
				spr.index = bus.PPURead(OAM + (oamIndex * 4) + 2);
				spr.flags = bus.PPURead(OAM + (oamIndex * 4) + 3);
				if ((ly - (spr.y - 16) < size && ly >= (spr.y - 16)))
				{
					sprites.emplace_back(spr);
				}
			}
			oamIndex++;
			if (oamIndex >= 40)
			{
				oamIndex = 0;
				SetMode(3);
				bus.vramLock = true;
			}
			else
			{
				sleep = 2;
			}
		}
		break;
		case 3:
		{
			switch (fetchStep)
			{
			case 1:
			{
				fetchAddr = 0x9800;
				if ((window && ((bus.PPURead(LCDC) & 0x40) >> 6)) || (!window && ((bus.PPURead(LCDC) & 0x08) >> 3)))
				{
					fetchAddr = 0x9C00;
				}
				fetcherX = !window ? bus.PPURead(SCX) + x : (x - (bus.PPURead(WX) - 7));
				fetcherX += fifo.size();
				fetcherY = !window ? bus.PPURead(SCY) + bus.PPURead(LY) : windowLineCounter;
				sleep = 2;
				if (x == 0)
				{
					shift = window ? (bus.PPURead(WX) - 7) % 8 : bus.PPURead(SCX) % 8;
				}
				fetchStep++;
			}
			break;
			case 2:
			{
				uint16_t tileAddr;
				if (sprIndex != -1 && fifo.size() >= 8)
				{
					uint8_t size = 8 + 8 * ((bus.PPURead(LCDC) & 0x4) >> 2);
					uint8_t offset = ((bus.PPURead(LY) - (sprites[sprIndex].y - 16)) % (size)) * 2;
					if ((sprites[sprIndex].flags & 0x40) != 0)
					{
						offset = 2*size - offset-2;
					}
					uint8_t idx = size == 8 ? sprites[sprIndex].index : sprites[sprIndex].index & 0xFE;
					tileAddr = 0x8000 + idx * 16 + offset;
				}
				else
				{
					bool lcdc4 = ((bus.PPURead(LCDC) & 0x10)) >> 4;
					uint8_t tileIndex = bus.PPURead(fetchAddr + ((fetcherY / 8) * 32 + fetcherX / 8));
					tileAddr = lcdc4 ? 0x8000 + tileIndex * 16 + (fetcherY % 8) * 2
						: 0x9000 + char(tileIndex) * 16 + (fetcherY % 8) * 2;
				}
				fetchLow = bus.PPURead(tileAddr);
				fetchHigh = bus.PPURead(tileAddr + 1);
				sleep = 4;
				fetchStep++;
			}
			break;
			case 3:
			{
				if (sprIndex != -1 && fifo.size() >= 8)
				{
					for (int i = 7; i >= 0; i--)
					{
						Pixel px;
						px.obj = true;
						px.bgPriority = (sprites[sprIndex].flags & 0x80) >> 7;
						uint8_t palette = bus.PPURead(OBP0 + ((sprites[sprIndex].flags & 0x10) >> 4));
						uint8_t color = ((fetchLow >> i) & 0x01) | ((fetchHigh >> i) << 1 & 0x02);
						px.color = ((palette) >> (color * 2)) & 0x03;
						uint8_t idx = (sprites[sprIndex].flags & 0x20) == 0 ? 7 - i : i;
						if (!fifo[idx].obj && color != 0)
						{
							if (px.bgPriority != 0 && fifo[idx].color != 0)
							{
								px.color = fifo[idx].color;
							}
							fifo[idx] = px;
						}
					}
					sprites.erase(sprites.begin() + sprIndex);
					sprIndex = -1;
					stopFifo = false;
					fetchStep = 1;
				}
				else if (fifo.size() <= 8)
				{
					for (int i = 7; i >= 0; i--)
					{
						Pixel px;
						px.bgPriority = 0;
						uint8_t palette = bus.PPURead(BGP);
						uint8_t color = ((fetchLow >> i) & 0x01) | ((fetchHigh >> i) << 1 & 0x02);
						px.color = (palette) >> (color * 2) & 0x03;
						fifo.push_back(px);
					}
					fetchStep = 1;
				}
			}
			break;
			}
		}
		break;
		}
	} 
	if (mode == 3)
	{
		if (fifo.size() > 8 && !stopFifo)
		{
			while (shift > 0)
			{
				shift--;
				fifo.erase(fifo.begin());
			}
			if ((bus.PPURead(LCDC) & 0x02) != 0)
			{
				for (int i = 0;i < sprites.size();i++)
				{
					if (sprites[i].x - 8 == x)
					{
						sprIndex = i;
						fetchStep = 1;
						stopFifo = true;
						break;
					}
				}
			}
			if (sprIndex == -1)
			{
				auto palette = bus.PPURead(BGP);
				int index = bus.PPURead(LY) * 160 + x;
				uint8_t color = fifo.front().color;
				fifo.erase(fifo.begin());
				if (lcdOff || (bus.PPURead(LCDC) & 0x01) == 0)
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
				window = wyc && (bus.PPURead(LCDC) & 0x20) != 0 && x >= (bus.PPURead(WX) - 7);
				if (window && !winOld)
				{
					windowLineCounter++;
					fifo.clear();
					fetchStep = 1;
				}
			}
		}
		if (x >= 160)
		{
			SetMode(0);
			fifo.clear();
			x = 0;
			fetchStep = 1;
			sprIndex = -1;
			stopFifo = false;
			bus.vramLock = false;
		}
	}
	sleep--;
}

uint8_t PPU::Read(uint16_t addr)
{
	uint8_t ret = 0xFF;
	if (addr >= 0x8000 && addr < 0xA000)
	{
		ret = vram[addr - 0x8000];
	}
	else if (addr >= 0xFE00 && addr < 0xFF00)
	{
		ret = oam[addr - 0xFE00];
	}
	switch (addr)
	{
	case LCDC:
		ret = lcdc;
		break;
	case STAT:
		ret = stat;
		break;
	case SCY:
		ret = scy;
		break;
	case SCX:
		ret = scx;
		break;
	case LY:
		ret = ly;
		break;
	case LYC:
		ret = lyc;
		break;
	case WY:
		ret = wy;
		break;
	case WX:
		ret = wx;
		break;
	case BGP:
		ret = bgp;
		break;
	case OBP0:
		ret = obp0;
		break;
	case OBP1:
		ret = obp1;
		break;
	}
	return ret;
}

void PPU::Write(uint16_t addr, uint8_t val)
{
	if (addr >= 0x8000 && addr < 0xA000)
	{
		vram[addr - 0x8000] = val;
	}
	else if (addr >= 0xFE00 && addr < 0xFEA0)
	{
		oam[addr - 0xFE00];
	}
	switch (addr)
	{
	case LCDC:
		lcdc = val;
		break;
	case STAT:
		stat = val;
		break;
	case SCY:
		scy = val;
		break;
	case SCX:
		scx = val;
		break;
	case LY:
		ly = val;
		break;
	case LYC:
		lyc = val;
		break;
	case WY:
		wy = val;
		break;
	case WX:
		wx = val;
		break;
	case BGP:
		bgp = val;
		break;
	case OBP0:
		obp0 = val;
		break;
	case OBP1:
		obp1 = val;
		break;
	}
}
 
void PPU::SetMode(int mode)
{
	if (mode != this->mode)
	{
		this->mode = mode;
		bus.PPUWrite(STAT, (mode & 0x03) | (bus.PPURead(STAT) & 0xFC));
		if (mode == 1)
		{
			bus.PPUWrite(IF, bus.PPURead(IF) | 0x01);
			wyc = false;
			windowLineCounter = -1;
			lcdOff = (bus.PPURead(LCDC) & 0x80) == 0;
		}
		if (mode == 2)
		{
			wyc = wyc || (bus.PPURead(LY) == bus.PPURead(WY));
			window = false;
			sprites.clear();
		}
		UpdateStat();
		sleep = 0;
	}
}

void PPU::UpdateStat()
{
	if (bus.PPURead(LY) == bus.PPURead(LYC))
	{
		bus.PPUWrite(STAT, bus.PPURead(STAT) | 0x04);
	}
	else
	{
		bus.PPUWrite(STAT, bus.PPURead(STAT) & ~0x04);
	}
	bool lineOld = statLine;
	auto stat = bus.PPURead(STAT);
	statLine = ((stat & 0x40) != 0 && (stat & 0x04) != 0) || (((stat & 0x20) != 0) && (mode == 2)) || (((stat & 0x10) != 0) && (mode == 1)) || (((stat & 0x08) != 0) && (mode == 0));
	if (lineOld == false && statLine == true)
	{
		bus.PPUWrite(IF, bus.PPURead(IF) | 0x02);
	}
}
