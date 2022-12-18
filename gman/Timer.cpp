#include "Timer.h"

#define DIV 0xFF04
#define TIMA 0xFF05
#define TMA 0xFF06
#define TAC 0xFF07
#define IF 0xFF0F

void Timer::Tick()
{
	divCount = (divCount + 1) % 256;
	if (divCount == 0)
	{
		div++;
	}
	if ((tac & 0x04) != 0)
	{
		int mod;
		switch (tac & 0x03)
		{
		case 0:
			mod = 1024;
			break;
		case 1:
			mod = 16;
			break;
		case 2:
			mod = 64;
			break;
		case 3:
			mod = 256;
			break;
		}
		timCount = (timCount + 1) % mod;
		if (timCount == 0)
		{
			tima++;
			if (tima == 0)
			{
				tima = tma;
				_if |= 0x04;
			}
		}
	}
}

uint8_t Timer::Read(uint16_t addr)
{
	uint8_t ret = 0xFF;
	switch (addr)
	{
	case DIV:
		ret = div;
		break;
	case TIMA:
		ret = tima;
		break;
	case TMA:
		ret = tma;
		break;
	case TAC:
		ret = tac | 0xF8;
		break;
	case IF:
		ret = _if;
		break;
	}
	return ret;
}

void Timer::Write(uint16_t addr, uint8_t val)
{
	switch (addr)
	{
	case DIV:
		div = 0;
		tima = 0;
		break;
	case TIMA:
		tima = val;
		break;
	case TMA:
		tma = val;
		break;
	case TAC:
		tac = val;
		break;
	case IF:
		_if = val;
		break;
	}
}