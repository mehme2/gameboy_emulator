#include "Bus.h"

class APU
{
public:
	APU(Bus& bus);
	void Tick();
	void BindBuffer(uint8_t* buf, uint16_t samp, uint16_t frequency);
private:
	void TickCH1();
	void TickCH2();
	void TickCH3();
	void TickCH4();
private:
	Bus& bus;
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
};
