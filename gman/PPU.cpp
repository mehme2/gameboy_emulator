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

PPU::PPU()
{
	fifo.clear();
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
		ly = (ly + 1) % 154;
		if (ly == 0)
		{
			endFrame = true;
		}
		if (ly < 144)
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
				uint8_t size = 8 + 8 * ((lcdc & 0x4) >> 2);
				Sprite spr =((Sprite*)oam)[oamIndex];
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
				fetchAddr = 0x1800;
				if ((window && ((lcdc & 0x40) >> 6)) || (!window && ((lcdc & 0x08) >> 3)))
				{
					fetchAddr = 0x1C00;
				}
				fetcherX = !window ? scx + x : (x - (wx - 7));
				fetcherX += fifo.size();
				fetcherY = !window ? scy + ly : windowLineCounter;
				sleep = 2;
				fetchStep++;
			}
			break;
			case 2:
			{
				uint16_t tileAddr;
				if (sprIndex != -1 && fifo.size() >= 8)
				{
					uint8_t size = 8 + 8 * ((lcdc & 0x4) >> 2);
					uint8_t offset = ((ly - (sprites[sprIndex].y - 16)) % (size)) * 2;
					if ((sprites[sprIndex].flags & 0x40) != 0)
					{
						offset = 2 * size - offset - 2;
					}
					uint8_t idx = size == 8 ? sprites[sprIndex].index : sprites[sprIndex].index & 0xFE;
					tileAddr = idx * 16 + offset;
				}
				else
				{
					uint8_t tileIndex = vram[fetchAddr + ((fetcherY / 8) * 32 + fetcherX / 8)];
					tileAddr =  (lcdc & 0x10) != 0 ? tileIndex * 16 + (fetcherY % 8) * 2
						: 0x1000 + char(tileIndex) * 16 + (fetcherY % 8) * 2;
				}
				fetchLow = vram[tileAddr];
				fetchHigh = vram[tileAddr + 1];
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
						uint8_t palette = (sprites[sprIndex].flags & 0x10) == 0 ? obp0 : obp1;
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
						uint8_t palette = bgp;
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
			while (shift > 0 && fifo.size() > 0)
			{
				shift--;
				fifo.erase(fifo.begin());
			}
			if ((lcdc & 0x02) != 0)
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
			if (sprIndex == -1&&fifo.size()>0)
			{
				if (x>=0)
				{
					auto palette = bgp;
					int index = ly * 160 + x;
					uint8_t color = fifo.front().color;
					if (lcdOff || (lcdc & 0x01) == 0)
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
				}
				fifo.erase(fifo.begin());
				x++;
				bool winOld = window;
				window = wyc && (lcdc & 0x20) != 0 && x >= (wx - 7);
				if (window && !winOld)
				{
					windowLineCounter++;
					fifo.clear();
					fetchStep = 1;
					shift = 0;
				}
				if (x == -7)
				{
					shift = window ? (wx - 7) % 8 : scx % 8;
				}
			}
		}
		if (x >= 160)
		{
			SetMode(0);
			fifo.clear();
			x = -8;
			fetchStep = 1;
			sprIndex = -1;
			stopFifo = false;
		}
	}
	sleep--;
}

uint8_t PPU::Read(uint16_t addr)
{
	uint8_t ret = 0xFF;
	if (addr >= 0x8000 && addr < 0xA000 && (mode != 3 || ((lcdc & 0x80) == 0)))
	{
		ret = vram[addr - 0x8000];
	}
	else if (addr >= 0xFE00 && addr < 0xFF00 && ((mode != 2 && mode != 3) || ((lcdc & 0x80) == 0)))
	{
		ret = oam[addr - 0xFE00];
	}
	switch (addr)
	{
	case IF:
		ret = _if;
		break;
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
	if (addr >= 0x8000 && addr < 0xA000 && (mode != 3 || ((lcdc & 0x80) == 0)))
	{
		vram[addr - 0x8000] = val;
	}
	else if (addr >= 0xFE00 && addr < 0xFEA0 && ((mode != 2 && mode != 3) || ((lcdc & 0x80) == 0)))
	{
		oam[addr - 0xFE00] = val;
	}
	switch (addr)
	{
	case IF:
		_if = (_if & 0xE0) | (val & 0x1F);
		break;
	case LCDC:
		lcdc = val;
		break;
	case STAT:
		stat = (stat & 0x07) | (val % 0xF8);
		break;
	case SCY:
		scy = val;
		break;
	case SCX:
		scx = val;
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

void PPU::DMA(uint8_t* addr)
{
	memcpy(oam, addr, 0xA0);
}
 
void PPU::SetMode(int mode)
{
	if (mode != this->mode)
	{
		this->mode = mode;
		stat = (mode & 0x03) | (stat & 0xFC);
		if (mode == 1)
		{
			_if = _if | 0x01;
			wyc = false;
			windowLineCounter = -1;
			lcdOff = (lcdc & 0x80) == 0;
		}
		if (mode == 2)
		{
			wyc = wyc || (ly == wy);
			window = false;
			sprites.clear();
		}
		UpdateStat();
		sleep = 0;
	}
}

void PPU::UpdateStat()
{
	if (ly == lyc)
	{
		stat = stat | 0x04;
	}
	else
	{
		stat = stat & ~0x04;
	}
	bool lineOld = statLine;
	statLine = ((stat & 0x40) != 0 && (stat & 0x04) != 0) || (((stat & 0x20) != 0) && (mode == 2)) || (((stat & 0x10) != 0) && (mode == 1)) || (((stat & 0x08) != 0) && (mode == 0));
	if (lineOld == false && statLine == true)
	{
		_if = _if | 0x02;
	}
}
