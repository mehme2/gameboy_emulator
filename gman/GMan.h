#pragma once

#include "Bus.h"
#include "LR35902.h"

class GMan
{
public:
	GMan()
	{
		cpu.BindBus(bus);
	}
	void Tick()
	{
		cpu.Tick();
	}
private:
	Bus bus;
	LR35902 cpu;
};