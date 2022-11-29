#pragma once

#include "Bus.h"

class LR35902
{
public:
	void BindBus(Bus& bus);
	void Tick();
	bool Stopped();
private:
	Bus bus;
	uint8_t AF[2];
	uint8_t BC[2];
	uint8_t DE[2]; 
	uint8_t HL[2];
	uint16_t PC;
	uint16_t SP;
	unsigned int sleep;
	bool stop;
	bool halt;
	bool interrupt;
private:
	uint8_t Fetch();
	uint16_t Fetch16();
	uint8_t& GetRegister(uint8_t reg);
	uint16_t& GetRegister16(uint8_t reg);
	void SetFlag(uint8_t flag, uint8_t val);
	uint8_t GetFlag(uint8_t flag);
	void IncRegister(uint8_t reg);
	void DecRegister(uint8_t reg);
	void RotateRegisterLeft(uint8_t reg);
	void RotateRegisterLeftCarry(uint8_t reg);
	void RotateRegisterRight(uint8_t reg);
	void RotateRegisterRightCarry(uint8_t reg);
	void AddRegister(uint8_t lhs, uint8_t rhs);
	void AddRegisterCarry(uint8_t lhs, uint8_t rhs);
	void SubRegister(uint8_t lhs, uint8_t rhs);
	void SubRegisterCarry(uint8_t lhs, uint8_t rhs); 
	void AndRegister(uint8_t lhs, uint8_t rhs);
	void XorRegister(uint8_t lhs, uint8_t rhs);
	void OrRegister(uint8_t lhs, uint8_t rhs);
	void CpRegister(uint8_t lhs, uint8_t rhs);
	void AddRegister16(uint8_t lhs, uint8_t rhs);
	void Restart(uint8_t addr);
	void PrefixCB(uint8_t code);
};