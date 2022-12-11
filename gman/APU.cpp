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

APU::APU(Bus& bus)
	:
	bus(bus)
{
}

void APU::Tick()
{
	TickCH1();
	TickCH2();
	TickCH3();
	TickCH4();
	div++;
	if (div % (4194304 / freq) == 0)
	{
		uint8_t nr51 = bus.APURead(NR51);
		uint8_t L[4];
		uint8_t R[4];
		for (int i = 0;i < 4;i++)
		{
			L[i] = (nr51 >> (4 + i)) & 0x01;
			R[i] = (nr51 >> i) & 0x01;
		}
		buffer[bufIndex] = (bus.APURead(NR52) & 0x80) == 0 ? 0 : 
			ch1AMP * (L[0] + R[0])
			+ (ch2AMP * (L[1] + R[1]))
			+ ((ch3AMP >> ch3SHIFT)) * (L[2] + R[2]);
		bufIndex = (bufIndex + 1) % samples;
	}
}

void APU::BindBuffer(uint8_t* buf, uint16_t samp, uint16_t frequency)
{
	buffer = buf;
	freq = frequency;
	samples = samp;
}

void APU::TickCH1()
{
	if (ch1ON)
	{
		uint8_t nr10 = bus.APURead(NR10);
		uint8_t nr11 = bus.APURead(NR11);
		uint8_t nr12 = bus.APURead(NR12);
		uint16_t wavelength = bus.APURead(NR13) + ((bus.APURead(NR14) & 0x07) << 8);
		uint8_t sweepPace = (nr10 & 0x70) >> 4;
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
		if ((bus.APURead(NR14) & 0x40) != 0)
		{
			if (div % 16384 == 0)
			{
				uint8_t timer = nr11 & 0x3F;
				timer++;
				if (timer == 64)
				{
					ch1ON = false;
				}
				bus.APUWrite(NR11, (nr11 & 0xC0) | (timer & 0x3F));
			}
		}
		if (div % (4*(2048-wavelength)) == 0)
		{
			uint8_t step = (div / (4 * (2048 - wavelength))) % 8;
			uint8_t waveform;
			uint8_t amp = (nr12 & 0xF0) >> 4;
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
			ch1AMP = amp * ((waveform >> (7 - step) & 0x01));
		}
		bus.APUWrite(NR13, wavelength & 0xFF);
		bus.APUWrite(NR14, ((wavelength >> 8) & 0x07) | (bus.APURead(NR14) & 0xF8));
	}
	else if ((bus.APURead(NR14) & 0x80) != 0)
	{
		ch1ON = true;
	}
	else
	{
		ch1AMP = 0;
	}
}

void APU::TickCH2()
{
	if (ch2ON)
	{
		uint8_t nr21 = bus.APURead(NR21);
		uint8_t nr22 = bus.APURead(NR22);
		uint16_t wavelength = bus.APURead(NR23) + ((bus.APURead(NR24) & 0x07) << 8);
		if ((bus.APURead(NR24) & 0x40) != 0)
		{
			if (div % 16384 == 0)
			{
				uint8_t timer = nr21 & 0x3F;
				timer++;
				if (timer == 64)
				{
					ch2ON = false;
				}
				bus.APUWrite(NR21, (nr21 & 0xC0) | (timer & 0x3F));
			}
		}
		if (div % (4*(2048 - wavelength)) == 0)
		{
			uint8_t step = (div / (4 * (2048 - wavelength))) % 8;
			uint8_t waveform;
			uint8_t amp = (nr22 & 0xF0) >> 4;
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
			ch2AMP = amp * ((waveform >> (7 - step) & 0x01));
		}
		bus.APUWrite(NR23, wavelength & 0xFF);
		bus.APUWrite(NR24, ((wavelength >> 8) & 0x07) | (bus.APURead(NR24) & 0xF8));
	}
	else if ((bus.APURead(NR24) & 0x80) != 0)
	{
		ch2ON = true;
	}
	else
	{
		ch2AMP = 0;
	}
}

void APU::TickCH3()
{
	if (ch3ON)
	{
		uint8_t nr31 = bus.APURead(NR31);
		uint8_t nr32 = bus.APURead(NR32);
		uint16_t wavelength = bus.APURead(NR33) + ((bus.APURead(NR34) & 0x07) << 8);
		if ((bus.APURead(NR34) & 0x40) != 0)
		{
			if (div % 16384 == 0)
			{
				uint8_t timer = nr31;
				timer++;
				if (timer == 0)
				{
					ch2ON = false;
				}
				bus.APUWrite(NR31, timer);
			}
		}
		if (div % (2*(2048 - wavelength)) == 0)
		{
			uint8_t step = (div / (2*(2048 - wavelength))) % 32;
			uint8_t amp = bus.APURead(0xFF30 + (step / 2));
			ch3AMP = (amp >> (4 * (step % 2))) & 0x0F;
		}
		switch ((bus.APURead(NR32) & 0x60) >> 5)
		{
		case 0:
			ch3SHIFT = 4;
			break;
		case 1:
			ch3SHIFT = 0;
			break;
		case 2:
			ch3SHIFT = 1;
			break;
		case 3:
			ch3SHIFT = 2;
			break;
		}
		bus.APUWrite(NR33, wavelength & 0xFF);
		bus.APUWrite(NR34, ((wavelength >> 8) & 0x07) | (bus.APURead(NR34) & 0xF8));
	}
	else if ((bus.APURead(NR34) & 0x80) != 0)
	{
		ch3ON = true;
	}
	else
	{
		ch3AMP = 0;
	}
}

void APU::TickCH4()
{
}
