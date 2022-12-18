#pragma once

#include <stdint.h>

class Timer
{
public:
	void Tick();
	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t val);
private:
	int divCount = 0;
	int timCount = 0;
private:
	uint8_t div;
	uint8_t tima;
	uint8_t tma;
	uint8_t tac;
	uint8_t _if;
};