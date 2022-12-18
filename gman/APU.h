#pragma once

#include <stdint.h>

class APU
{
public:
	void Tick();
	void BindBuffer(uint8_t* buf, uint16_t samp, uint16_t frequency);
	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t val);
private:
	void TickCH1();
	void TickCH2();
	void TickCH3();
	void TickCH4();
private:
	uint8_t* buffer = nullptr;
	uint16_t freq = 0;
	uint16_t samples = 0;
	uint16_t bufIndex = 0;
	size_t div = 0;
	bool ch1ON = false;
	bool ch2ON = false;
	bool ch3ON = false;
	bool ch4ON = false;
	uint8_t ch1AMP = 0;
	uint8_t ch2AMP = 0;
	uint8_t ch3AMP = 0;
	uint8_t ch4AMP = 0;
	uint8_t ch1OUT = 0;
	uint8_t ch2OUT = 0;
	uint8_t ch3OUT = 0;
	uint8_t ch4OUT = 0;
	uint16_t LSFR=0;
	size_t freq_timer;
private:
	uint8_t nr10;
	uint8_t nr11;
	uint8_t nr12;
	uint8_t nr13;
	uint8_t nr14;
	uint8_t nr21;
	uint8_t nr22;
	uint8_t nr23;
	uint8_t nr24;
	uint8_t nr30;
	uint8_t nr31;
	uint8_t nr32;
	uint8_t nr33;
	uint8_t nr34;
	uint8_t nr41;
	uint8_t nr42;
	uint8_t nr43;
	uint8_t nr44;
	uint8_t nr50;
	uint8_t nr51;
	uint8_t nr52;
	uint8_t wavePattern[16];
};
