#include "APU.h"

#define NR10 0xFF10
#define NR11 0xFF11
#define NR12 0xFF12
#define NR13 0xFF13
#define NR14 0xFF14
#define NR21 0xFF16
#define NR22 0xFF17
#define NR23 0xFF18
#define NR24 0xFF19
#define NR30 0xFF1A
#define NR31 0xFF1B
#define NR32 0xFF1C
#define NR33 0xFF1D
#define NR34 0xFF1E
#define NR41 0xFF20
#define NR42 0xFF21
#define NR43 0xFF22
#define NR44 0xFF23
#define NR50 0xFF24
#define NR51 0xFF25
#define NR52 0xFF26

void APU::Tick()
{
	TickCH1();
	TickCH2();
	TickCH3();
	TickCH4();
	div++;
	if (div % (4194304 / freq) == 0)
	{
		uint8_t L[4];
		uint8_t R[4];
		for (int i = 0;i < 4;i++)
		{
			L[i] = (nr51 >> (4 + i)) & 0x01;
			R[i] = (nr51 >> i) & 0x01;
		}
		buffer[bufIndex] = (nr52 & 0x80) == 0 ? 0 :
			ch1OUT * (L[0] + R[0])
			+ (ch2OUT * (L[1] + R[1]))
			+ (ch3OUT * (L[2] + R[2]))
			+ (ch4OUT * (L[3] + R[3]));
		bufIndex = (bufIndex + 1) % samples;
	}
}

void APU::BindBuffer(uint8_t* buf, uint16_t samp, uint16_t frequency)
{
	buffer = buf;
	freq = frequency;
	samples = samp;
}

uint8_t APU::Read(uint16_t addr)
{
	uint8_t ret = 0xFF;
	if (addr >= 0xFF30 && addr < 0xFF40)
	{
		ret = wavePattern[addr - 0xFF30];
	}
	switch (addr)
	{
	case NR10:
		ret = nr10 | 0x80;
		break;
	case NR11:
		ret = nr11 | 0x3F;
		break;
	case NR12:
		ret = nr12;
		break;
	case NR14:
		ret = nr14 | 0xDF;
		break;
	case NR21:
		ret = nr21 | 0x3F;
		break;
	case NR22:
		ret = nr22;
		break;
	case NR24:
		ret = nr24 | 0xDF;
		break;
	case NR30:
		ret = nr30 | 0x7F;
		break;
	case NR32:
		ret = nr32 | 0x9F;
		break;
	case NR34:
		ret = nr34 | 0xDF;
		break;
	case NR42:
		ret = nr42;
		break;
	case NR43:
		ret = nr43;
		break;
	case NR44:
		ret = nr44 | 0xDF;
		break;
	case NR50:
		ret = nr50;
		break;
	case NR51:
		ret = nr51;
		break;
	case NR52:
		ret = nr52;
		break;
	}
	return ret;
}

void APU::Write(uint16_t addr, uint8_t val)
{
	if (addr >= 0xFF30 && addr < 0xFF40)
	{
		wavePattern[addr - 0xFF30] = val;
	}
	switch (addr)
	{
	case NR10:
		nr10 = val;
		break;
	case NR11:
		nr11 = val;
		break;
	case NR12:
		nr12 = val;
		break;
	case NR13:
		nr13 = val;
		break;
	case NR14:
		nr14 = val;
		break;
	case NR21:
		nr21 = val;
		break;
	case NR22:
		nr22 = val;
		break;
	case NR23:
		nr23 = val;
		break;
	case NR24:
		nr24 = val;
		break;
	case NR30:
		nr30 = val;
		break;
	case NR31:
		nr31 = val;
		break;
	case NR32:
		nr32 = val;
		break;
	case NR33:
		nr33 = val;
		break;
	case NR34:
		nr34 = val;
		break;
	case NR41:
		nr41 = val;
		break;
	case NR42:
		nr42 = val;
		break;
	case NR43:
		nr43 = val;
		break;
	case NR44:
		nr44 = val;
		break;
	case NR50:
		nr50 = val;
		break;
	case NR51:
		nr51 = val;
		break;
	case NR52:
		nr52 = (nr52 & 0x0F) | (val & 0x80);
		break;
	}
}

void APU::TickCH1()
{
	if (ch1ON)
	{
		uint16_t wavelength = nr13 + ((nr14 & 0x07) << 8);
		uint8_t sweepPace = (nr10 & 0x70) >> 4;
		uint8_t envSweepPace = (nr12 & 0x07);
		//if (envSweepPace != 0 && (div % (65536 / envSweepPace) == 0))
		//{
		//	bool add = (nr12 & 0x08) >> 3;
		//	if (add)
		//	{
		//		ch1AMP++;
		//	}
		//	else
		//	{
		//		if (ch1AMP != 0)
		//		{
		//			ch1AMP--;
		//		}
		//	}
		//	ch1AMP &= 0x0F;
		//}
		if (sweepPace != 0 && (div % (32768 / sweepPace) == 0))
		{
			bool sub = (nr10 & 0x08) >> 3;
			uint8_t n = nr10 & 0x07;
			if (sub)
			{
				wavelength = wavelength - (wavelength >> n);
			}
			else
			{
				wavelength = wavelength + (wavelength >> n);
			}
			if (wavelength == 0x0800)
			{
				ch1ON = false;
			}
		}
		if ((nr14 & 0x40) != 0)
		{
			if (div % 16384 == 0)
			{
				uint8_t timer = nr11 & 0x3F;
				timer++;
				if (timer == 64)
				{
					ch1ON = false;
				}
				nr11 = (nr11 & 0xC0) | (timer & 0x3F);
			}
		}
		if (div % (4*(2048-wavelength)) == 0)
		{
			uint8_t step = (div / (4 * (2048 - wavelength))) % 8;
			uint8_t waveform;
			switch ((nr11 & 0xC0) >> 6)
			{
			case 0:
				waveform = 0b00000001;
				break;
			case 1:
				waveform = 0b00000011;
				break;
			case 2:
				waveform = 0b00001111;
				break;
			case 3:
				waveform = 0b11111100;
				break;
			}
			ch1OUT = ch1AMP * ((waveform >> (7 - step) & 0x01));
		}
		nr13 = wavelength & 0xFF;
		nr14 = ((wavelength >> 8) & 0x07) | (nr14 & 0xF8);
	}
	else
	{
		ch1OUT = 0;
	}
	if ((nr14 & 0x80) != 0)
	{
		ch1ON = true;
		ch1AMP = (nr12 & 0xF0) >> 4;
		nr14 &= 0x7F;
	}
}

void APU::TickCH2()
{
	if (ch2ON)
	{
		uint16_t wavelength = nr23 + ((nr24 & 0x07) << 8);
		uint8_t envSweepPace = (nr22 & 0x07);
		//if (envSweepPace != 0 && (div % (65536 / envSweepPace) == 0))
		//{
		//	bool add = (nr22 & 0x08) >> 3;
		//	if (add)
		//	{
		//		ch2AMP++;
		//	}
		//	else
		//	{
		//		if (ch2AMP != 0)
		//		{
		//			ch2AMP--;
		//		}
		//	}
		//	ch2AMP &= 0x0F;
		//}
		if ((nr24 & 0x40) != 0)
		{
			if (div % 16384 == 0)
			{
				uint8_t timer = nr21 & 0x3F;
				timer++;
				if (timer == 64)
				{
					ch2ON = false;
				}
				nr21 = (nr21 & 0xC0) | (timer & 0x3F);
			}
		}
		if (div % (4*(2048 - wavelength)) == 0)
		{
			uint8_t step = (div / (4 * (2048 - wavelength))) % 8;
			uint8_t waveform;
			switch ((nr21 & 0xC0) >> 6)
			{
			case 0:
				waveform = 0b00000001;
				break;
			case 1:
				waveform = 0b00000011;
				break;
			case 2:
				waveform = 0b00001111;
				break;
			case 3:
				waveform = 0b11111100;
				break;
			}
			ch2OUT = ch2AMP * ((waveform >> (7 - step) & 0x01));
		}
		nr23 = wavelength & 0xFF;
		nr24 = ((wavelength >> 8) & 0x07) | (nr24 & 0xF8);
	}
	else
	{
		ch2OUT = 0;
	}
	if ((nr24 & 0x80) != 0)
	{
		nr24 = nr24 & 0x7F;
		ch2ON = true;
		ch2AMP = (nr22 & 0xF0) >> 4;
	}
}

void APU::TickCH3()
{
	if (ch3ON)
	{
		uint16_t wavelength = nr33 + ((nr34 & 0x07) << 8);
		if ((nr34 & 0x40) != 0)
		{
			if (div % 16384 == 0)
			{
				nr31++;
				if (nr31 == 0)
				{
					ch3ON = false;
				}
			}
		}
		if (div % (2*(2048 - wavelength)) == 0)
		{
			uint8_t step = (div / (2*(2048 - wavelength))) % 32;
			uint8_t shift;
			switch ((nr32 & 0x60) >> 5)
			{
			case 0:
				shift = 4;
				break;
			case 1:
				shift = 0;
				break;
			case 2:
				shift = 1;
				break;
			case 3:
				shift = 2;
				break;
			}
			uint8_t amp = wavePattern[step / 2];
			ch3AMP = (amp >> (4 * (step % 2))) & 0x0F;
			ch3OUT = ch3AMP >> shift;
		}
	}
	else
	{
		ch3OUT = 0;
	}
	if ((nr34 & 0x80) != 0)
	{
		nr34 &= 0x7F;
		ch3ON = true;
	}
}

void APU::TickCH4()
{
	if (ch4ON)
	{
		uint8_t envSweepPace = (nr42 & 0x07);
		if (envSweepPace != 0 && (div % (65536 / envSweepPace) == 0))
		{
			bool add = (nr42 & 0x08) >> 3;
			if (add)
			{
				ch4AMP++;
			}
			else
			{
				ch4AMP--;
			}
			ch4AMP &= 0x0F;
		}
		if ((nr44 & 0x40) != 0)
		{
			if (div % 16384 == 0)
			{
				uint8_t timer = nr41 & 0x3F;
				timer++;
				if (timer == 64)
				{
					ch4ON = false;
				}
				nr41 = (nr41 & 0xC0) | (timer & 0x3F);
			}
		}
		freq_timer--;
		if (freq_timer == 0)
		{
			uint8_t clockDivider = nr43 & 0x07;
			uint8_t clockShift = (nr43 & 0xF0) >> 4;
			uint8_t lsfrWidth = (nr43 & 0x08) >> 3;
			freq_timer = size_t(clockDivider > 0 ? (clockDivider << 4) : 8) << clockShift;
			uint16_t xor_r = (LSFR & 0b01) ^ ((LSFR & 0b10) >> 1);
			LSFR = (LSFR >> 1) | (xor_r << 14);

			if (lsfrWidth == 1)
			{
				LSFR &= ~0x04;
				LSFR |= xor_r << 6;
			}
		}
		ch4OUT = ch4AMP * (LSFR & 0x01);
	}
	else
	{
		ch4OUT = 0;
	}
	if ((nr44 & 0x80) != 0)
	{
		nr44 &= 0x7F;
		ch4ON = true;
		ch4AMP = (nr42 & 0xF0) >> 4;
		LSFR = 1;
		uint8_t clockDivider = nr43 & 0x07;
		uint8_t clockShift = (nr43 & 0xF0) >> 4;
		freq_timer = size_t(clockDivider > 0 ? (clockDivider << 4) : 8) << clockShift;
	}
}
