#include "LR35902.h"

#define REGISTER_A 0x00
#define REGISTER_F 0x01
#define REGISTER_B 0x02
#define REGISTER_C 0x04
#define REGISTER_D 0x08
#define REGISTER_E 0x10
#define REGISTER_H 0x20
#define REGISTER_L 0x40

#define REGISTER_AF 0x00
#define REGISTER_BC 0x01
#define REGISTER_DE 0x02
#define REGISTER_HL 0x04
#define REGISTER_SP 0x10

#define FLAG_C 0x10
#define FLAG_H 0x20
#define FLAG_N 0x40
#define FLAG_Z 0x80

#define FLAG_SET 0xFF
#define FLAG_CLEAR 0x00

LR35902::LR35902(Bus& bus)
	:
	bus(bus)
{
	GetRegister16(REGISTER_AF) = 0;
	GetRegister16(REGISTER_BC) = 0;
	GetRegister16(REGISTER_DE) = 0;
	GetRegister16(REGISTER_HL) = 0;
	PC = 0x0100;
	SP = 0;
}

void LR35902::Tick()
{
	if (interrupt)
	{
		uint8_t flag = bus.Read(0xFFFF) & bus.Read(0xFF0F) & 0x1F;
		if(flag != 0)
		{
			halt = false;
			stop = false;
			interrupt = false;
			uint16_t addr;
			uint8_t mask;
			if ((flag & 0x01) != 0)
			{
				addr = 0x0040;
				mask = 0x01;
			}
			else if ((flag & 0x02) != 0)
			{
				addr = 0x0048 ;
				mask = 0x02;
			}
			else if ((flag & 0x04) != 0)
			{
				addr = 0x0050;
				mask = 0x04; 
			}
			else if ((flag & 0x08) != 0)
			{
				addr = 0x0058;
				mask = 0x08;
			}
			else
			{
				addr = 0x0060;
				mask = 0x10; 
			}
			SP -= 2;
			bus.Write16(SP, PC);
			PC = addr;
			bus.Write(0xFF0F, bus.Read(0xFF0F) & ~mask);
			sleep = 20;
		}
	}
	if (!stop && !halt)
	{
		if (sleep <= 0)
		{
			uint8_t instruction = Fetch();
			switch (instruction)
			{
			case 0x00:// NOP
				sleep = 4;
				break;
			case 0x01:// LD BC,d16
				GetRegister16(REGISTER_BC) = Fetch16();
				sleep = 12;
				break;
			case 0x02:// LD (BC),A
				bus.Write(GetRegister16(REGISTER_BC), GetRegister(REGISTER_A));
				sleep = 8;
				break;
			case 0x03:// INC BC
				GetRegister16(REGISTER_BC)++;
				sleep = 8;
				break;
			case 0x04:// INC B
				IncRegister(REGISTER_B);
				sleep = 4;
				break;
			case 0x05:// DEC B
				DecRegister(REGISTER_B);
				sleep = 4;
				break;
			case 0x06:// LD B,d8
				GetRegister(REGISTER_B) = Fetch();
				sleep = 8;
				break;
			case 0x07:// RLCA
				RotateRegisterLeft(REGISTER_A);
				sleep = 4;
				break;
			case 0x08:// LD (a16),SP
				SP = Fetch16();
				sleep = 20;
				break;
			case 0x09:// ADD HL,BC
				AddRegister16(REGISTER_HL, REGISTER_BC);
				sleep = 8;
				break;
			case 0x0A:// LD A,(BC)
				GetRegister(REGISTER_A) = bus.Read(GetRegister16(REGISTER_BC));
				sleep = 8;
				break;
			case 0x0B:// DEC BC
				GetRegister16(REGISTER_BC)--;
				sleep = 8;
				break;
			case 0x0C:// INC C
				IncRegister(REGISTER_C);
				sleep = 4;
				break;
			case 0x0D:// DEC C
				DecRegister(REGISTER_C);
				sleep = 4;
				break;
			case 0x0E:// LD C,d8
				GetRegister(REGISTER_C) = Fetch();
				sleep = 8;
				break;
			case 0x0F:// RRCA
				RotateRegisterRight(REGISTER_A);
				sleep = 4;
				break;
			case 0x10:// STOP 0
				stop = true;
				PC++;
				break;
			case 0x11:// LD DE,d16
				GetRegister16(REGISTER_DE) = Fetch16();
				sleep = 12;
				break;
			case 0x12:// LD (DE),A
				bus.Write(GetRegister16(REGISTER_DE), GetRegister(REGISTER_A));
				sleep = 8;
				break;
			case 0x13:// INC DE
				GetRegister16(REGISTER_DE)++;
				sleep = 8;
				break;
			case 0x14:// INC D
				IncRegister(REGISTER_D);
				sleep = 4;
				break;
			case 0x15:// DEC D
				DecRegister(REGISTER_D);
				sleep = 4;
				break;
			case 0x16:// LD D,d8
				GetRegister(REGISTER_D) = Fetch();
				sleep = 8;
				break;
			case 0x17:// RLA
				RotateRegisterLeftCarry(REGISTER_A);
				sleep = 4;
				break;
			case 0x18:// JR r8
				PC += char(Fetch());
				sleep = 12;
				break;
			case 0x19:// ADD HL,DE
				AddRegister16(REGISTER_HL, REGISTER_DE);
				sleep = 8;
				break;
			case 0x1A:// LD A,(DE)
				GetRegister(REGISTER_A) = bus.Read(GetRegister16(REGISTER_DE));
				sleep = 8;
				break;
			case 0x1B:// DEC DE
				GetRegister16(REGISTER_DE)--;
				sleep = 8;
				break;
			case 0x1C:// INC E
				IncRegister(REGISTER_E);
				sleep = 4;
				break;
			case 0x1D:// DEC E
				DecRegister(REGISTER_E);
				sleep = 4;
				break;
			case 0x1E:// LD E,d8
				GetRegister(REGISTER_E) = Fetch();
				sleep = 8;
				break;
			case 0x1F:// RRA
				RotateRegisterRightCarry(REGISTER_A);
				sleep = 4;
				break;
			case 0x20:// JR NZ,r8
				if (GetFlag(FLAG_Z) == 0)
				{
					PC += Fetch();
					sleep = 12;
				}
				else
				{
					Fetch();
					sleep = 8;
				}
				break;
			case 0x21:// LD HL,d16
				GetRegister16(REGISTER_HL) = Fetch16();
				sleep = 12;
				break;
			case 0x22:// LD (HL+),A
				bus.Write(GetRegister16(REGISTER_HL)++, GetRegister(REGISTER_A));
				sleep = 8;
				break;
			case 0x23:// INC HL  
				GetRegister16(REGISTER_HL)++;
				sleep = 8;
				break;
			case 0x24:// INC H
				IncRegister(REGISTER_H);
				sleep = 4;
				break;
			case 0x25:// DEC H
				DecRegister(REGISTER_H);
				sleep = 4;
				break;
			case 0x26:// LD H,d8
				GetRegister(REGISTER_H) = Fetch();
				sleep = 8;
				break;
			case 0x27:// DAA
			{
				auto& r = GetRegister(REGISTER_A);
				SetFlag(FLAG_C, r / 100 == 0 ? FLAG_CLEAR : FLAG_SET);
				SetFlag(FLAG_Z, r % 100 == 0 ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_N, FLAG_CLEAR);
				r = r % 10 + ((r / 10) % 10) * 16;
				break;
			}
			case 0x28:// JR Z,r8
				if (GetFlag(FLAG_Z) == 0)
				{
					Fetch();
					sleep = 8;
				}
				else
				{
					PC += Fetch();
					sleep = 12;
				}
				break;
			case 0x29:// ADD HL,HL
				AddRegister16(REGISTER_HL, REGISTER_HL);
				sleep = 8;
				break;
			case 0x2A:// LD A,(HL+)
				GetRegister(REGISTER_A) = bus.Read(GetRegister16(REGISTER_HL)++);
				sleep = 8;
				break;
			case 0x2B:// DEC HL
				GetRegister16(REGISTER_HL)--;
				sleep = 8;
				break;
			case 0x2C:// INC L
				IncRegister(REGISTER_L);
				sleep = 4;
				break;
			case 0x2D:// DEC L
				DecRegister(REGISTER_L);
				sleep = 4;
				break;
			case 0x2E:// LD L,d8
				GetRegister(REGISTER_L) = Fetch();
				sleep = 8;
				break;
			case 0x2F:// CPL
				GetRegister(REGISTER_A) ^= 0xFF;
				SetFlag(FLAG_N, FLAG_SET);
				SetFlag(FLAG_H, FLAG_SET);
				sleep = 4;
				break;
			case 0x30:// JR NC,r8
				if (GetFlag(FLAG_C) == 0)
				{
					PC += Fetch();
					sleep = 12;
				}
				else
				{
					Fetch();
					sleep = 8;
				}
				break;
			case 0x31:// LD SP,d16
				SP = Fetch16();
				sleep = 12;
				break;
			case 0x32:// LD (HL-),A
				bus.Write(GetRegister16(REGISTER_HL)--, GetRegister(REGISTER_A));
				sleep = 8;
				break;
			case 0x33:// INC SP
				SP++;
				sleep = 8;
				break;
			case 0x34:// INC (HL)
			{
				uint8_t a;
				bus.Write(GetRegister16(REGISTER_HL), a = (bus.Read(GetRegister16(REGISTER_HL) + 1)));
				SetFlag(FLAG_Z, a == 0 ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_N, FLAG_CLEAR);
				SetFlag(FLAG_H, (a & 0x0F) == 0 ? FLAG_SET : FLAG_CLEAR);
				sleep = 12;
			}
			break;
			case 0x35:// DEC (HL)
			{
				uint8_t a;
				bus.Write(GetRegister16(REGISTER_HL), a = (bus.Read(GetRegister16(REGISTER_HL) - 1)));
				SetFlag(FLAG_Z, a == 0 ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_N, FLAG_SET);
				SetFlag(FLAG_H, a == 0xFF ? FLAG_SET : FLAG_CLEAR);
				sleep = 12;
			}
			break;
			case 0x36:// LD (HL),d8
				bus.Write(GetRegister16(REGISTER_HL), Fetch());
				sleep = 12;
				break;
			case 0x37:// SCF
				SetFlag(FLAG_C, FLAG_SET);
				SetFlag(FLAG_N, FLAG_CLEAR);
				SetFlag(FLAG_H, FLAG_CLEAR);
				break;
			case 0x38:// JR C,r8
				if (GetFlag(FLAG_C) == 0)
				{
					Fetch();
					sleep = 8;
				}
				else
				{
					PC += Fetch();
					sleep = 12;
				}
				break;
			case 0x39:// ADD HL,SP
				AddRegister16(REGISTER_HL, REGISTER_SP);
				sleep = 8;
				break;
			case 0x3A:// LD A,(HL-)
				GetRegister(REGISTER_A) = bus.Read(GetRegister16(REGISTER_HL)--);
				sleep = 8;
				break;
			case 0x3B:// DEC SP
				SP--;
				sleep = 8;
				break;
			case 0x3C:// INC A
				IncRegister(REGISTER_A);
				sleep = 4;
				break;
			case 0x3D:// DEC A
				DecRegister(REGISTER_A);
				sleep = 4;
				break;
			case 0x3E:// LD A,d8
				GetRegister(REGISTER_A) = Fetch();
				sleep = 8;
				break;
			case 0x3F:// CCF
				SetFlag(FLAG_C, ~GetFlag(FLAG_C));
				SetFlag(FLAG_N, FLAG_CLEAR);
				SetFlag(FLAG_H, FLAG_CLEAR);
				sleep = 4;
				break;
			case 0x40:// LD B,B
				GetRegister(REGISTER_B) = GetRegister(REGISTER_B);
				sleep = 4;
				break;
			case 0x41:// LD B,C
				GetRegister(REGISTER_B) = GetRegister(REGISTER_C);
				sleep = 4;
				break;
			case 0x42:// LD B,D
				GetRegister(REGISTER_B) = GetRegister(REGISTER_D);
				sleep = 4;
				break;
			case 0x43:// LD B,E
				GetRegister(REGISTER_B) = GetRegister(REGISTER_E);
				sleep = 4;
				break;
			case 0x44:// LD B,H
				GetRegister(REGISTER_B) = GetRegister(REGISTER_H);
				sleep = 4;
				break;
			case 0x45:// LD B,L
				GetRegister(REGISTER_B) = GetRegister(REGISTER_L);
				sleep = 4;
				break;
			case 0x46:// LD B,(HL)
				GetRegister(REGISTER_B) = bus.Read(GetRegister16(REGISTER_HL));
				sleep = 8;
				break;
			case 0x47:// LD B,A
				GetRegister(REGISTER_B) = GetRegister(REGISTER_A);
				sleep = 4;
				break;
			case 0x48:// LD C,B
				GetRegister(REGISTER_C) = GetRegister(REGISTER_B);
				sleep = 4;
				break;
			case 0x49:// LD C,C
				GetRegister(REGISTER_C) = GetRegister(REGISTER_C);
				sleep = 4;
				break;
			case 0x4A:// LD C,D
				GetRegister(REGISTER_C) = GetRegister(REGISTER_D);
				sleep = 4;
				break;
			case 0x4B:// LD C,E
				GetRegister(REGISTER_C) = GetRegister(REGISTER_E);
				sleep = 4;
				break;
			case 0x4C:// LD C,H
				GetRegister(REGISTER_C) = GetRegister(REGISTER_H);
				sleep = 4;
				break;
			case 0x4D:// LD C,L
				GetRegister(REGISTER_C) = GetRegister(REGISTER_L);
				sleep = 4;
				break;
			case 0x4E:// LD C,(HL)
				GetRegister(REGISTER_C) = bus.Read(GetRegister16(REGISTER_HL));
				sleep = 8;
				break;
			case 0x4F:// LD C,A
				GetRegister(REGISTER_C) = GetRegister(REGISTER_A);
				sleep = 4;
				break;
			case 0x50:// LD D,B
				GetRegister(REGISTER_D) = GetRegister(REGISTER_B);
				sleep = 4;
				break;
			case 0x51:// LD D,C
				GetRegister(REGISTER_D) = GetRegister(REGISTER_C);
				sleep = 4;
				break;
			case 0x52:// LD D,D
				GetRegister(REGISTER_D) = GetRegister(REGISTER_D);
				sleep = 4;
				break;
			case 0x53:// LD D,E
				GetRegister(REGISTER_D) = GetRegister(REGISTER_E);
				sleep = 4;
				break;
			case 0x54:// LD D,H
				GetRegister(REGISTER_D) = GetRegister(REGISTER_H);
				sleep = 4;
				break;
			case 0x55:// LD D,L
				GetRegister(REGISTER_D) = GetRegister(REGISTER_L);
				sleep = 4;
				break;
			case 0x56:// LD D,(HL)
				GetRegister(REGISTER_D) = bus.Read(GetRegister16(REGISTER_HL));
				sleep = 8;
				break;
			case 0x57:// LD D,A
				GetRegister(REGISTER_D) = GetRegister(REGISTER_A);
				sleep = 4;
				break;
			case 0x58:// LD E,B
				GetRegister(REGISTER_E) = GetRegister(REGISTER_B);
				sleep = 4;
				break;
			case 0x59:// LD E,C
				GetRegister(REGISTER_E) = GetRegister(REGISTER_C);
				sleep = 4;
				break;
			case 0x5A:// LD E,D
				GetRegister(REGISTER_E) = GetRegister(REGISTER_D);
				sleep = 4;
				break;
			case 0x5B:// LD E,E
				GetRegister(REGISTER_E) = GetRegister(REGISTER_E);
				sleep = 4;
				break;
			case 0x5C:// LD E,H
				GetRegister(REGISTER_E) = GetRegister(REGISTER_H);
				sleep = 4;
				break;
			case 0x5D:// LD E,L
				GetRegister(REGISTER_E) = GetRegister(REGISTER_L);
				sleep = 4;
				break;
			case 0x5E:// LD E,(HL)
				GetRegister(REGISTER_E) = bus.Read(GetRegister16(REGISTER_HL));
				sleep = 8;
				break;
			case 0x5F:// LD E,A
				GetRegister(REGISTER_E) = GetRegister(REGISTER_A);
				sleep = 4;
				break;
			case 0x60:// LD H,B
				GetRegister(REGISTER_H) = GetRegister(REGISTER_B);
				sleep = 4;
				break;
			case 0x61:// LD H,C
				GetRegister(REGISTER_H) = GetRegister(REGISTER_C);
				sleep = 4;
				break;
			case 0x62:// LD H,D
				GetRegister(REGISTER_H) = GetRegister(REGISTER_D);
				sleep = 4;
				break;
			case 0x63:// LD H,E
				GetRegister(REGISTER_H) = GetRegister(REGISTER_E);
				sleep = 4;
				break;
			case 0x64:// LD H,H
				GetRegister(REGISTER_H) = GetRegister(REGISTER_H);
				sleep = 4;
				break;
			case 0x65:// LD H,L
				GetRegister(REGISTER_H) = GetRegister(REGISTER_L);
				sleep = 4;
				break;
			case 0x66:// LD H,(HL)
				GetRegister(REGISTER_H) = bus.Read(GetRegister16(REGISTER_HL));
				sleep = 8;
				break;
			case 0x67:// LD H,A
				GetRegister(REGISTER_H) = GetRegister(REGISTER_A);
				sleep = 4;
				break;
			case 0x68:// LD L,B
				GetRegister(REGISTER_L) = GetRegister(REGISTER_B);
				sleep = 4;
				break;
			case 0x69:// LD L,C
				GetRegister(REGISTER_L) = GetRegister(REGISTER_C);
				sleep = 4;
				break;
			case 0x6A:// LD L,D
				GetRegister(REGISTER_L) = GetRegister(REGISTER_D);
				sleep = 4;
				break;
			case 0x6B:// LD L,E
				GetRegister(REGISTER_L) = GetRegister(REGISTER_E);
				sleep = 4;
				break;
			case 0x6C:// LD L,H
				GetRegister(REGISTER_L) = GetRegister(REGISTER_H);
				sleep = 4;
				break;
			case 0x6D:// LD L,L 
				GetRegister(REGISTER_L) = GetRegister(REGISTER_L);
				sleep = 4;
				break;
			case 0x6E:// LD L,(HL)
				GetRegister(REGISTER_L) = bus.Read(GetRegister16(REGISTER_HL));
				sleep = 8;
				break;
			case 0x6F:// LD L,A
				GetRegister(REGISTER_L) = GetRegister(REGISTER_A);
				sleep = 4;
				break;
			case 0x70:// LD (HL),B
				bus.Write(GetRegister16(REGISTER_HL), GetRegister(REGISTER_B));
				sleep = 8;
				break;
			case 0x71:// LD (HL),C
				bus.Write(GetRegister16(REGISTER_HL), GetRegister(REGISTER_C));
				sleep = 8;
				break;
			case 0x72:// LD (HL),D
				bus.Write(GetRegister16(REGISTER_HL), GetRegister(REGISTER_D));
				sleep = 8;
				break;
			case 0x73:// LD (HL),E
				bus.Write(GetRegister16(REGISTER_HL), GetRegister(REGISTER_E));
				sleep = 8;
				break;
			case 0x74:// LD (HL),H
				bus.Write(GetRegister16(REGISTER_HL), GetRegister(REGISTER_H));
				sleep = 8;
				break;
			case 0x75:// LD (HL),L
				bus.Write(GetRegister16(REGISTER_HL), GetRegister(REGISTER_L));
				sleep = 8;
				break;
			case 0x76:// HALT
				halt = true;
				break;
			case 0x77:// LD (HL),A
				bus.Write(GetRegister16(REGISTER_HL), GetRegister(REGISTER_A));
				sleep = 8;
				break;
			case 0x78:// LD A,B
				GetRegister(REGISTER_A) = GetRegister(REGISTER_B);
				sleep = 4;
				break;
			case 0x79:// LD A,C
				GetRegister(REGISTER_A) = GetRegister(REGISTER_C);
				sleep = 4;
				break;
			case 0x7A:// LD A,D
				GetRegister(REGISTER_A) = GetRegister(REGISTER_D);
				sleep = 4;
				break;
			case 0x7B:// LD A,E
				GetRegister(REGISTER_A) = GetRegister(REGISTER_E);
				sleep = 4;
				break;
			case 0x7C:// LD A,H
				GetRegister(REGISTER_A) = GetRegister(REGISTER_H);
				sleep = 4;
				break;
			case 0x7D:// LD A,L
				GetRegister(REGISTER_A) = GetRegister(REGISTER_L);
				sleep = 4;
				break;
			case 0x7E:// LD A,(HL)
				GetRegister(REGISTER_A) = bus.Read(GetRegister16(REGISTER_HL));
				sleep = 8;
				break;
			case 0x7F:// LD A,A
				GetRegister(REGISTER_A) = GetRegister(REGISTER_A);
				sleep = 4;
				break;
			case 0x80:// ADD A,B
				AddRegister(REGISTER_A, REGISTER_B);
				sleep = 4;
				break;
			case 0x81:// ADD A,C
				AddRegister(REGISTER_A, REGISTER_C);
				sleep = 4;
				break;
			case 0x82:// ADD A,D
				AddRegister(REGISTER_A, REGISTER_D);
				sleep = 4;
				break;
			case 0x83:// ADD A,E
				AddRegister(REGISTER_A, REGISTER_E);
				sleep = 4;
				break;
			case 0x84:// ADD A,H
				AddRegister(REGISTER_A, REGISTER_H);
				sleep = 4;
				break;
			case 0x85:// ADD A,L
				AddRegister(REGISTER_A, REGISTER_L);
				sleep = 4;
				break;
			case 0x86:// ADD A,(HL)
			{
				auto old = GetRegister(REGISTER_A);
				auto res = GetRegister(REGISTER_A) += bus.Read(GetRegister16(REGISTER_HL));
				SetFlag(FLAG_Z, res == 0 ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_H, (old & 0x0F) > (res & 0x0F) ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_C, old > res ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_N, FLAG_CLEAR);
				sleep = 8;
			}
			break;
			case 0x87:// ADD A,A
				AddRegister(REGISTER_A, REGISTER_A);
				sleep = 4;
				break;
			case 0x88:// ADC A,B
				AddRegisterCarry(REGISTER_A, REGISTER_B);
				sleep = 4;
				break;
			case 0x89:// ADC A,C
				AddRegisterCarry(REGISTER_A, REGISTER_C);
				sleep = 4;
				break;
			case 0x8A:// ADC A,D
				AddRegisterCarry(REGISTER_A, REGISTER_D);
				sleep = 4;
				break;
			case 0x8B:// ADC A,E
				AddRegisterCarry(REGISTER_A, REGISTER_E);
				sleep = 4;
				break;
			case 0x8C:// ADC A,H
				AddRegisterCarry(REGISTER_A, REGISTER_H);
				sleep = 4;
				break;
			case 0x8D:// ADC A,L
				AddRegisterCarry(REGISTER_A, REGISTER_L);
				sleep = 4;
				break;
			case 0x8E:// ADC A,(HL)
			{
				auto old = GetRegister(REGISTER_A);
				auto res = GetRegister(REGISTER_A) += bus.Read(GetRegister16(REGISTER_HL));
				res += GetFlag(FLAG_C) & 0x01;
				SetFlag(FLAG_Z, res == 0 ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_H, (old & 0x0F) > (res & 0x0F) ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_C, old > res ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_N, FLAG_CLEAR);
				sleep = 8;
			}
			break;
			case 0x8F:// ADC A,A
				AddRegisterCarry(REGISTER_A, REGISTER_A);
				sleep = 4;
				break;
			case 0x90:// SUB B
				SubRegister(REGISTER_A, REGISTER_B);
				sleep = 4;
				break;
			case 0x91:// SUB C
				SubRegister(REGISTER_A, REGISTER_C);
				sleep = 4;
				break;
			case 0x92:// SUB D
				SubRegister(REGISTER_A, REGISTER_D);
				sleep = 4;
				break;
			case 0x93:// SUB E
				SubRegister(REGISTER_A, REGISTER_E);
				sleep = 4;
				break;
			case 0x94:// SUB H
				SubRegister(REGISTER_A, REGISTER_H);
				sleep = 4;
				break;
			case 0x95:// SUB L
				SubRegister(REGISTER_A, REGISTER_L);
				sleep = 4;
				break;
			case 0x96:// SUB (HL)
			{
				auto old = GetRegister(REGISTER_A);
				auto res = GetRegister(REGISTER_A) -= bus.Read(GetRegister16(REGISTER_HL));
				SetFlag(FLAG_Z, res == 0 ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_H, (old & 0x0F) > (res & 0x0F) ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_C, old > res ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_N, FLAG_SET);
				sleep = 8;
			}
			break;
			case 0x97:// SUB A
				SubRegister(REGISTER_A, REGISTER_A);
				sleep = 4;
				break;
			case 0x98:// SBC A,B
				SubRegisterCarry(REGISTER_A, REGISTER_B);
				sleep = 4;
				break;
			case 0x99:// SBC A,C
				SubRegisterCarry(REGISTER_A, REGISTER_C);
				sleep = 4;
				break;
			case 0x9A:// SBC A,D
				SubRegisterCarry(REGISTER_A, REGISTER_D);
				sleep = 4;
				break;
			case 0x9B:// SBC A,E
				SubRegisterCarry(REGISTER_A, REGISTER_E);
				sleep = 4;
				break;
			case 0x9C:// SBC A,H
				SubRegisterCarry(REGISTER_A, REGISTER_H);
				sleep = 4;
				break;
			case 0x9D:// SBC A,L
				SubRegisterCarry(REGISTER_A, REGISTER_L);
				sleep = 4;
				break;
			case 0x9E:// SBC A,(HL)
			{
				auto old = GetRegister(REGISTER_A);
				auto res = GetRegister(REGISTER_A) -= bus.Read(GetRegister16(REGISTER_HL));
				res -= GetFlag(FLAG_C) & 0x01;
				SetFlag(FLAG_Z, res == 0 ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_H, (old & 0x0F) > (res & 0x0F) ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_C, old > res ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_N, FLAG_SET);
				sleep = 8;
			}
				break;
			case 0x9F:// SBC A,A
				SubRegisterCarry(REGISTER_A, REGISTER_A);
				sleep = 4;
				break;
			case 0xA0:// AND B
				AndRegister(REGISTER_A, REGISTER_B);
				sleep = 4;
				break;
			case 0xA1:// AND C
				AndRegister(REGISTER_A, REGISTER_C);
				sleep = 4;
				break;
			case 0xA2:// AND D
				AndRegister(REGISTER_A, REGISTER_D);
				sleep = 4;
				break;
			case 0xA3:// AND E
				AndRegister(REGISTER_A, REGISTER_E);
				sleep = 4;
				break;
			case 0xA4:// AND H
				AndRegister(REGISTER_A, REGISTER_H);
				sleep = 4;
				break;
			case 0xA5:// AND L
				AndRegister(REGISTER_A, REGISTER_L);
				sleep = 4;
				break;
			case 0xA6:// AND (HL)
			{
				GetRegister(REGISTER_A) &= bus.Read(GetRegister16(REGISTER_HL));
				SetFlag(FLAG_Z, GetRegister(REGISTER_A) == 0 ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_N, FLAG_CLEAR);
				SetFlag(FLAG_H, FLAG_SET);
				SetFlag(FLAG_C, FLAG_CLEAR);
				sleep = 8;
			}
				break;
			case 0xA7:// AND A
				AndRegister(REGISTER_A, REGISTER_A);
				sleep = 4; 
				break;
			case 0xA8:// XOR B
				XorRegister(REGISTER_A, REGISTER_B);
				sleep = 4;
				break;
			case 0xA9:// XOR C
				XorRegister(REGISTER_A, REGISTER_C);
				sleep = 4;
				break;
			case 0xAA:// XOR D
				XorRegister(REGISTER_A, REGISTER_D);
				sleep = 4;
				break;
			case 0xAB:// XOR E
				XorRegister(REGISTER_A, REGISTER_E);
				sleep = 4;
				break;
			case 0xAC:// XOR H  
				XorRegister(REGISTER_A, REGISTER_H);
				sleep = 4;
				break;
			case 0xAD:// XOR L
				XorRegister(REGISTER_A, REGISTER_L);
				sleep = 4;
				break;
			case 0xAE:// XOR (HL)
			{
				GetRegister(REGISTER_A) ^= bus.Read(GetRegister16(REGISTER_HL));
				SetFlag(FLAG_Z, GetRegister(REGISTER_A) == 0 ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_N, FLAG_CLEAR);
				SetFlag(FLAG_H, FLAG_CLEAR);
				SetFlag(FLAG_C, FLAG_CLEAR);
				sleep = 8;
			}
				break;
			case 0xAF:// XOR A
				XorRegister(REGISTER_A, REGISTER_A);
				sleep = 4;
				break;
			case 0xB0:// OR B
				OrRegister(REGISTER_A, REGISTER_B);
				sleep = 4;
				break;
			case 0xB1:// OR C
				OrRegister(REGISTER_A, REGISTER_C);
				sleep = 4;
				break;
			case 0xB2:// OR D
				OrRegister(REGISTER_A, REGISTER_D);
				sleep = 4;
				break;
			case 0xB3:// OR E
				OrRegister(REGISTER_A, REGISTER_E);
				sleep = 4;
				break;
			case 0xB4:// OR H
				OrRegister(REGISTER_A, REGISTER_H);
				sleep = 4;
				break;
			case 0xB5:// OR L
				OrRegister(REGISTER_A, REGISTER_L);
				sleep = 4;
				break;
			case 0xB6:// OR (HL)
			{
				GetRegister(REGISTER_A) |= bus.Read(GetRegister16(REGISTER_HL));
				SetFlag(FLAG_Z, GetRegister(REGISTER_A) == 0 ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_N, FLAG_CLEAR);
				SetFlag(FLAG_H, FLAG_CLEAR);
				SetFlag(FLAG_C, FLAG_CLEAR);
				sleep = 8;
			}
				break;
			case 0xB7:// OR A
				OrRegister(REGISTER_A, REGISTER_A);
				sleep = 4;
				break;
			case 0xB8:// CP B
				CpRegister(REGISTER_A, REGISTER_B);
				sleep = 4;
				break;
			case 0xB9:// CP C
				CpRegister(REGISTER_A, REGISTER_C);
				sleep = 4;
				break;
			case 0xBA:// CP D
				CpRegister(REGISTER_A, REGISTER_D);
				sleep = 4;
				break;
			case 0xBB:// CP E
				CpRegister(REGISTER_A, REGISTER_E);
				sleep = 4;
				break;
			case 0xBC:// CP H
				CpRegister(REGISTER_A, REGISTER_H);
				sleep = 4;
				break;
			case 0xBD:// CP L
				CpRegister(REGISTER_A, REGISTER_L);
				sleep = 4;
				break;
			case 0xBE:// CP (HL)
			{
				auto old = GetRegister(REGISTER_A);
				auto res = GetRegister(REGISTER_A) - bus.Read(GetRegister16(REGISTER_HL));
				SetFlag(FLAG_Z, res == 0 ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_H, (old & 0x0F) > (res & 0x0F) ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_C, old > res ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_N, FLAG_SET);
				sleep = 8;
			}
				break;
			case 0xBF:// CP A  
				CpRegister(REGISTER_A, REGISTER_A);
				sleep = 4;
				break;
			case 0xC0:// RET NZ
				if (GetFlag(FLAG_Z) == 0)
				{
					PC = bus.Read(SP);
					SP += 2;
					sleep = 20;
				}
				else
				{
					sleep = 8;
				}
				break;
			case 0xC1:// POP BC
				GetRegister16(REGISTER_BC) = bus.Read(SP);
				SP += 2;
				sleep = 12;
				break;
			case 0xC2:// JP NZ,a16
				if (GetFlag(FLAG_Z) == 0)
				{
					PC = Fetch16();
					sleep = 16;
				}
				else
				{
					Fetch16();
					sleep = 12;
				}
				break;
			case 0xC3:// JP a16
				PC = Fetch16();
				sleep = 16;
				break;
			case 0xC4:// CALL NZ,a16
				if (GetFlag(FLAG_Z) == 0)
				{
					SP -= 2;
					bus.Write16(SP, PC);
					PC = Fetch16();
					sleep = 24;
				}
				else
				{
					Fetch16();
					sleep = 12;
				}
				break;
			case 0xC5:// PUSH BC
				SP -= 2;
				bus.Write16(SP, GetRegister16(REGISTER_BC));
				sleep = 16;
				break;
			case 0xC6:// ADD A,d8
			{
				auto old = GetRegister(REGISTER_A);
				auto res = GetRegister(REGISTER_A) += Fetch();
				SetFlag(FLAG_Z, res == 0 ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_H, (old & 0x0F) > (res & 0x0F) ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_C, old > res ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_N, FLAG_CLEAR);
				sleep = 8;
			}
				break;
			case 0xC7:// RST 00H
				Restart(0x00); 
				sleep = 16;
				break;
			case 0xC8:// RET Z
				if (GetFlag(FLAG_Z) == 0)
				{
					sleep = 8;
				}
				else
				{
					PC = bus.Read(SP);
					SP += 2;
					sleep = 20;
				}
				break;
			case 0xC9:// RET
				PC = bus.Read(SP);
				SP += 2;
				sleep = 16;
				break;
			case 0xCA:// JP Z,a16
				if (GetFlag(FLAG_Z) == 0)
				{
					Fetch16();
					sleep = 12;
				}
				else
				{
					PC = Fetch16();
					sleep = 16;
				}
				break;
			case 0xCB:// PREFIX CB
				PrefixCB(Fetch());
				break;
			case 0xCC:// CALL Z,a16
				if (GetFlag(FLAG_Z) == 0)
				{
					Fetch16();
					sleep = 12;
				}
				else
				{
					SP -= 2;
					bus.Write16(SP, PC);
					PC = Fetch16();
					sleep = 24;
				}
				break;
			case 0xCD:// CALL a16
				SP -= 2;
				bus.Write16(SP, PC);
				PC = Fetch16();
				sleep = 24;
				break;
			case 0xCE:// ADC A,d8
			{
				auto old = GetRegister(REGISTER_A);
				auto res = GetRegister(REGISTER_A) += Fetch();
				res += GetFlag(FLAG_C) & 0x01;
				SetFlag(FLAG_Z, res == 0 ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_H, (old & 0x0F) > (res & 0x0F) ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_C, old > res ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_N, FLAG_CLEAR);
				sleep = 8;
			}
				break;
			case 0xCF:// RST 08H
				Restart(0x08);
				sleep = 16;
				break;
			case 0xD0:// RET NC
				if (GetFlag(FLAG_C) == 0)
				{
					PC = bus.Read(SP);
					SP += 2;
					sleep = 20;
				}
				else
				{
					sleep = 8;
				}
				break;
			case 0xD1:// POP DE
				GetRegister16(REGISTER_DE) = bus.Read(SP);
				SP += 2;
				sleep = 12;
				break;
			case 0xD2:// JP NC,a16
				if (GetFlag(FLAG_C) == 0)
				{
					PC = Fetch16();
					sleep = 16;
				}
				else
				{
					Fetch16();
					sleep = 12;
				}
				break;
			case 0xD3:// 
				break;
			case 0xD4:// CALL NC,a16
				if (GetFlag(FLAG_C) == 0)
				{
					SP -= 2;
					bus.Write16(SP, PC);
					PC = Fetch16();
					sleep = 24;
				}
				else
				{
					Fetch16();
					sleep = 12;
				}
				break;
			case 0xD5:// PUSH DE
				SP -= 2;
				bus.Write16(SP, GetRegister16(REGISTER_DE));
				sleep = 16;
				break;
			case 0xD6:// SUB d8
			{
				auto old = GetRegister(REGISTER_A);
				auto res = GetRegister(REGISTER_A) -= Fetch();
				SetFlag(FLAG_Z, res == 0 ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_H, (old & 0x0F) > (res & 0x0F) ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_C, old > res ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_N, FLAG_SET);
				sleep = 8;
			}
				break;
			case 0xD7:// RST 10H
				Restart(0x10);
				sleep = 16;
				break;
			case 0xD8:// RET C
				if (GetFlag(FLAG_C) == 0)
				{
					sleep = 8;
				}
				else
				{
					PC = bus.Read(SP);
					SP += 2;
					sleep = 20;
				}
				break;
			case 0xD9:// RETI
				PC = bus.Read(SP);
				SP += 2;
				sleep = 16;
				interrupt = true;
				break;
			case 0xDA:// JP C,a16
				if (GetFlag(FLAG_C) == 0)
				{
					Fetch16();
					sleep = 12;
				}
				else
				{
					PC = Fetch16();
					sleep = 16;
				}
				break;
			case 0xDB:// 
				break;
			case 0xDC:// CALL C,a16
				if (GetFlag(FLAG_C) == 0)
				{
					Fetch16();
					sleep = 12;
				}
				else
				{
					SP -= 2;
					bus.Write16(SP, PC);
					PC = Fetch16();
					sleep = 24;
				}
				break;
			case 0xDD:// 
				break;
			case 0xDE:// SBC A,d8
			{
				auto old = GetRegister(REGISTER_A);
				auto res = GetRegister(REGISTER_A) -= Fetch();
				res -= GetFlag(FLAG_C) & 0x01;
				SetFlag(FLAG_Z, res == 0 ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_H, (old & 0x0F) > (res & 0x0F) ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_C, old > res ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_N, FLAG_SET);
				sleep = 8;
			}
				break;
			case 0xDF:// RST 18H
				Restart(0x18);
				sleep = 16;
				break;
			case 0xE0:// LDH (a8),A
				bus.Write(0xFF00 + Fetch(), GetRegister(REGISTER_A));
				sleep = 12;
				break;
			case 0xE1:// POP HL
				GetRegister16(REGISTER_HL) = bus.Read(SP);
				SP += 2;
				sleep = 12;
				break;
			case 0xE2:// LD (C),A
				bus.Write(0xFF00 + GetRegister(REGISTER_C), GetRegister(REGISTER_A));
				sleep = 8;
				break;
			case 0xE3:// 
				break;
			case 0xE4:// 
				break;
			case 0xE5:// PUSH HL
				SP -= 2;
				bus.Write16(SP, GetRegister16(REGISTER_HL));
				sleep = 16;
				break;
			case 0xE6:// AND d8
			{
				GetRegister(REGISTER_A) &= Fetch();
				SetFlag(FLAG_Z, GetRegister(REGISTER_A) == 0 ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_N, FLAG_CLEAR);
				SetFlag(FLAG_H, FLAG_SET);
				SetFlag(FLAG_C, FLAG_CLEAR);
				sleep = 8;
			}
				break;
			case 0xE7:// RST 20H
				Restart(0x20);
				sleep = 16;
				break;
			case 0xE8:// ADD SP,r8
				SP += char(Fetch());
				sleep = 16;
				break;
			case 0xE9:// JP (HL)
				PC = GetRegister16(REGISTER_HL);
				sleep = 4;
				break;
			case 0xEA:// LD (a16),A
				bus.Write(Fetch16(), GetRegister(REGISTER_A));
				sleep = 16;
				break;
			case 0xEB:// 
				break;
			case 0xEC:// 
				break;
			case 0xED:// 
				break;
			case 0xEE:// XOR d8
			{
				GetRegister(REGISTER_A) ^= Fetch();
				SetFlag(FLAG_Z, GetRegister(REGISTER_A) == 0 ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_N, FLAG_CLEAR);
				SetFlag(FLAG_H, FLAG_CLEAR);
				SetFlag(FLAG_C, FLAG_CLEAR);
				sleep = 8;
			}
				break;
			case 0xEF:// RST 28H
				Restart(0x28);
				sleep = 16;
				break;
			case 0xF0:// LDH A,(a8)
				GetRegister(REGISTER_A) = bus.Read(0xFF00 + Fetch());
				sleep = 12;
				break;
			case 0xF1:// POP AF
				GetRegister16(REGISTER_AF) = bus.Read(SP);
				SP += 2;
				sleep = 12;
				break;
			case 0xF2:// LD A,(C)
				GetRegister(REGISTER_A) = bus.Read(GetRegister(REGISTER_C));
				sleep = 8;
				break;
			case 0xF3:// DI
				interrupt = false;
				sleep = 4;
				break;
			case 0xF4:// 
				break;
			case 0xF5:// PUSH AF
				SP -= 2;
				bus.Write16(SP, GetRegister16(REGISTER_AF));
				sleep = 16;
				break;
			case 0xF6:// OR d8
			{
				GetRegister(REGISTER_A) |= Fetch();
				SetFlag(FLAG_Z, GetRegister(REGISTER_A) == 0 ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_N, FLAG_CLEAR);
				SetFlag(FLAG_H, FLAG_CLEAR);
				SetFlag(FLAG_C, FLAG_CLEAR);
				sleep = 8;
			}
				break;
			case 0xF7:// RST 30H
				Restart(0x30);
				sleep = 16;
				break;
			case 0xF8:// LD HL,SP+r8
				GetRegister16(REGISTER_HL) = bus.Read(SP + char(Fetch()));
				sleep = 12;
				break;
			case 0xF9:// LD SP,HL
				SP = GetRegister16(REGISTER_HL);
				sleep = 8;
				break;
			case 0xFA:// LD A,(a16)
				GetRegister(REGISTER_A) = bus.Read(Fetch16());
				sleep = 16;
				break;
			case 0xFB:// EI
				interrupt = true;
				break;
			case 0xFC:// 
				break;
			case 0xFD:// 
				break;
			case 0xFE:// CP d8
			{
				auto old = GetRegister(REGISTER_A);
				auto res = GetRegister(REGISTER_A) - Fetch();
				SetFlag(FLAG_Z, res == 0 ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_H, (old & 0x0F) > (res & 0x0F) ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_C, old > res ? FLAG_SET : FLAG_CLEAR);
				SetFlag(FLAG_N, FLAG_SET);
				sleep = 8;
			}
				break;
			case 0xFF:// RST 38H
				Restart(0x38);
				sleep = 16;
				break;
			}
		}
		if (sleep > 0)
		{
			sleep--;
		}
	}
}

bool LR35902::Stopped()
{
	return stop;
}

uint8_t LR35902::Fetch()
{
	return bus.Read(PC++);
}

uint16_t LR35902::Fetch16()
{
	uint16_t low = bus.Read(PC++);
	uint16_t high = bus.Read(PC++);
	high <<= 8;
	high |= low;
	return high;
}

uint8_t& LR35902::GetRegister(uint8_t reg)
{
	switch (reg)
	{
	case REGISTER_A:
		return AF[1];
	case REGISTER_F:
		return AF[0];
	case REGISTER_B:
		return BC[1];
	case REGISTER_C:
		return BC[0];
	case REGISTER_D:
		return DE[1];
	case REGISTER_E:
		return DE[0];
	case REGISTER_H:
		return HL[1];
	case REGISTER_L:
		return HL[0];
	}
}

uint16_t& LR35902::GetRegister16(uint8_t reg)
{
	switch (reg)
	{
		case REGISTER_AF:
		{
			uint16_t* ptr = (uint16_t*)AF;
			return *ptr;
		}
		case REGISTER_BC:
		{
			uint16_t* ptr = (uint16_t*)BC;
			return *ptr;
		}
		case REGISTER_DE:
		{
			uint16_t* ptr = (uint16_t*)DE;
			return *ptr;
		}
		case REGISTER_HL:
		{
			uint16_t* ptr = (uint16_t*)HL;
			return *ptr;
		}
		case REGISTER_SP:
		{
			return SP;
		}
	}
}

void LR35902::SetFlag(uint8_t flag, uint8_t val)
{
	uint8_t a = flag & val;
	uint8_t inv = ~flag;
	AF[0] &= (a | inv);
}

uint8_t LR35902::GetFlag(uint8_t flag)
{
	if ((AF[0] & flag) == 0)
	{
		return FLAG_CLEAR;
	}
	else
	{
		return FLAG_SET;
	}
}

void LR35902::IncRegister(uint8_t reg)
{
	uint8_t& r = GetRegister(reg);
	r++;
	SetFlag(FLAG_Z, r == 0 ? FLAG_SET : FLAG_CLEAR);
	SetFlag(FLAG_N, FLAG_CLEAR);
	SetFlag(FLAG_H, (r & 0x0F) == 0 ? FLAG_SET : FLAG_CLEAR); 
}

void LR35902::DecRegister(uint8_t reg)
{
	uint8_t& r = GetRegister(reg);
	r--;
	SetFlag(FLAG_Z, r == 0 ? FLAG_SET : FLAG_CLEAR);
	SetFlag(FLAG_N, FLAG_SET); 
	SetFlag(FLAG_H, r == 0xFF ? FLAG_SET : FLAG_CLEAR);
}

void LR35902::RotateRegisterLeft(uint8_t reg)
{
	uint8_t& r = GetRegister(reg);
	SetFlag(FLAG_C, (r & 0x80) == 0 ? FLAG_CLEAR : FLAG_SET);
	SetFlag(FLAG_N, FLAG_CLEAR);
	SetFlag(FLAG_H, FLAG_CLEAR);
	SetFlag(FLAG_Z, FLAG_CLEAR);
	r <<= 1;
	r |= (0x01 & GetFlag(FLAG_C));
}

void LR35902::RotateRegisterLeftCarry(uint8_t reg)
{
	auto oldC = GetFlag(FLAG_C);
	RotateRegisterLeft(reg);
	GetRegister(reg) &= 0xFE;
	GetRegister(reg) |= (0x01 & oldC);
}

void LR35902::RotateRegisterRight(uint8_t reg)
{
	uint8_t& r = GetRegister(reg);
	SetFlag(FLAG_C, (r & 0x01) == 0 ? FLAG_CLEAR : FLAG_SET);
	SetFlag(FLAG_N, FLAG_CLEAR);
	SetFlag(FLAG_H, FLAG_CLEAR);
	SetFlag(FLAG_Z, FLAG_CLEAR);
	r >>= 1;
	r |= (0x80 & GetFlag(FLAG_C));
}

void LR35902::RotateRegisterRightCarry(uint8_t reg)
{
	auto oldC = GetFlag(FLAG_C);
	RotateRegisterRight(reg);
	GetRegister(reg) &= 0x7F;
	GetRegister(reg) |= (0x80 & oldC);
}

void LR35902::AddRegister(uint8_t lhs, uint8_t rhs)
{
	auto old = GetRegister(lhs);
	auto res = GetRegister(lhs) += GetRegister(rhs);
	SetFlag(FLAG_Z, res == 0 ? FLAG_SET : FLAG_CLEAR);
	SetFlag(FLAG_H, (old & 0x0F) > (res & 0x0F) ? FLAG_SET : FLAG_CLEAR);
	SetFlag(FLAG_C, old > res ? FLAG_SET : FLAG_CLEAR);
	SetFlag(FLAG_N, FLAG_CLEAR);
} 

void LR35902::AddRegisterCarry(uint8_t lhs, uint8_t rhs)
{
	auto old = GetRegister(lhs);
	auto res = GetRegister(lhs) += GetRegister(rhs);
	res += GetFlag(FLAG_C) & 0x01;
	SetFlag(FLAG_Z, res == 0 ? FLAG_SET : FLAG_CLEAR);
	SetFlag(FLAG_H, (old & 0x0F) > (res & 0x0F) ? FLAG_SET : FLAG_CLEAR);
	SetFlag(FLAG_C, old > res ? FLAG_SET : FLAG_CLEAR);
	SetFlag(FLAG_N, FLAG_CLEAR);
}    

void LR35902::SubRegister(uint8_t lhs, uint8_t rhs)
{
	auto old = GetRegister(lhs);
	auto res = GetRegister(lhs) -= GetRegister(rhs);
	SetFlag(FLAG_Z, res == 0 ? FLAG_SET : FLAG_CLEAR);
	SetFlag(FLAG_H, (old & 0x0F) > (res & 0x0F) ? FLAG_SET : FLAG_CLEAR);
	SetFlag(FLAG_C, old > res ? FLAG_SET : FLAG_CLEAR);
	SetFlag(FLAG_N, FLAG_SET);
}

void LR35902::SubRegisterCarry(uint8_t lhs, uint8_t rhs)
{
	auto old = GetRegister(lhs);
	auto res = GetRegister(lhs) -= GetRegister(rhs);
	res -= GetFlag(FLAG_C) & 0x01;
	SetFlag(FLAG_Z, res == 0 ? FLAG_SET : FLAG_CLEAR);
	SetFlag(FLAG_H, (old & 0x0F) > (res & 0x0F) ? FLAG_SET : FLAG_CLEAR);
	SetFlag(FLAG_C, old > res ? FLAG_SET : FLAG_CLEAR);
	SetFlag(FLAG_N, FLAG_SET);
}

void LR35902::AndRegister(uint8_t lhs, uint8_t rhs)
{
	GetRegister(lhs) &= GetRegister(rhs);   
	SetFlag(FLAG_Z, GetRegister(lhs) == 0 ? FLAG_SET : FLAG_CLEAR);
	SetFlag(FLAG_N, FLAG_CLEAR);
	SetFlag(FLAG_H, FLAG_SET);
	SetFlag(FLAG_C, FLAG_CLEAR);
}

void LR35902::XorRegister(uint8_t lhs, uint8_t rhs)
{
	GetRegister(lhs) ^= GetRegister(rhs);
	SetFlag(FLAG_Z, GetRegister(lhs) == 0 ? FLAG_SET : FLAG_CLEAR);
	SetFlag(FLAG_N, FLAG_CLEAR);
	SetFlag(FLAG_H, FLAG_CLEAR);
	SetFlag(FLAG_C, FLAG_CLEAR);
}

void LR35902::OrRegister(uint8_t lhs, uint8_t rhs)
{
	GetRegister(lhs) |= GetRegister(rhs);
	SetFlag(FLAG_Z, GetRegister(lhs) == 0 ? FLAG_SET : FLAG_CLEAR);
	SetFlag(FLAG_N, FLAG_CLEAR);
	SetFlag(FLAG_H, FLAG_CLEAR);
	SetFlag(FLAG_C, FLAG_CLEAR);
}

void LR35902::CpRegister(uint8_t lhs, uint8_t rhs)
{
	auto old = GetRegister(lhs);
	auto res = GetRegister(lhs) - GetRegister(rhs);
	SetFlag(FLAG_Z, res == 0 ? FLAG_SET : FLAG_CLEAR);
	SetFlag(FLAG_H, (old & 0x0F) > (res & 0x0F) ? FLAG_SET : FLAG_CLEAR);
	SetFlag(FLAG_C, old > res ? FLAG_SET : FLAG_CLEAR);
	SetFlag(FLAG_N, FLAG_SET);
}

void LR35902::AddRegister16(uint8_t lhs, uint8_t rhs)
{
	auto old = GetRegister16(lhs);
	auto res = GetRegister16(lhs) += GetRegister16(rhs);
	SetFlag(FLAG_H, (old & 0x0FFF) > (res & 0x0FFF) ? FLAG_SET : FLAG_CLEAR);
	SetFlag(FLAG_C, old > res ? FLAG_SET : FLAG_CLEAR);
}

void LR35902::Restart(uint8_t addr)
{
	SP -= 2;
	bus.Write16(SP, PC);
	PC = addr;
}

void LR35902::PrefixCB(uint8_t code)
{
	uint8_t reg;
	switch (code & 0x07)
	{
	case 0x00:
		reg = REGISTER_B;
		sleep = 8;
		break;  
	case 0x01:
		reg = REGISTER_C;
		sleep = 8;
		break;
	case 0x02:
		reg = REGISTER_D;
		sleep = 8;
		break;
	case 0x03:
		reg = REGISTER_E;
		sleep = 8;
		break;
	case 0x04:
		reg = REGISTER_H;
		sleep = 8;
		break;
	case 0x05:
		reg = REGISTER_L;
		sleep = 8;
		break;
	case 0x06: // (HL)
		reg = 0xFF;
		sleep = 16;
		break;
	case 0x07:
		reg = REGISTER_A;
		sleep = 8;
		break;
	}
	switch (code & 0xF8)
	{
	case 0x00: // RLC X
		if (reg != 0xFF)
		{
			RotateRegisterLeft(reg);
			if (GetRegister(reg) == 0)
			{
				SetFlag(FLAG_Z, FLAG_SET); 
			}
		}
		else
		{
			uint8_t r = bus.Read(GetRegister16(REGISTER_HL));
			SetFlag(FLAG_C, (r & 0x80) == 0 ? FLAG_CLEAR : FLAG_SET);
			SetFlag(FLAG_N, FLAG_CLEAR);
			SetFlag(FLAG_H, FLAG_CLEAR);
			SetFlag(FLAG_Z, FLAG_CLEAR);
			r <<= 1;
			r |= (0x01 & GetFlag(FLAG_C));
			if (r == 0)
			{
				SetFlag(FLAG_Z, FLAG_SET);
			}
			bus.Write(GetRegister16(REGISTER_HL), r);
		}
		break;
	case 0x08: // RRC X 
		if (reg != 0xFF)
		{
			RotateRegisterRight(reg);
			if (GetRegister(reg) == 0)
			{
				SetFlag(FLAG_Z, FLAG_SET);
			}
		}
		else
		{
			uint8_t r = bus.Read(GetRegister16(REGISTER_HL));
			SetFlag(FLAG_C, (r & 0x01) == 0 ? FLAG_CLEAR : FLAG_SET);
			SetFlag(FLAG_N, FLAG_CLEAR);
			SetFlag(FLAG_H, FLAG_CLEAR);
			SetFlag(FLAG_Z, FLAG_CLEAR);
			r >>= 1;
			r |= (0x80 & GetFlag(FLAG_C));
			if (r == 0)
			{
				SetFlag(FLAG_Z, FLAG_SET);
			}
			bus.Write(GetRegister16(REGISTER_HL), r);
		}
		break;
	case 0x10: // RL X
		if (reg != 0xFF)
		{
			auto oldC = GetFlag(FLAG_C);
			RotateRegisterLeftCarry(reg);
			GetRegister(reg) |= (0x01 & oldC);
			if (GetRegister(reg) == 0)
			{
				SetFlag(FLAG_Z, FLAG_SET);
			}
		}
		else
		{
			auto oldC = GetFlag(FLAG_C);
			uint8_t r = bus.Read(GetRegister16(REGISTER_HL));
			SetFlag(FLAG_C, (r & 0x80) == 0 ? FLAG_CLEAR : FLAG_SET);
			SetFlag(FLAG_N, FLAG_CLEAR);
			SetFlag(FLAG_H, FLAG_CLEAR);
			SetFlag(FLAG_Z, FLAG_CLEAR);
			r <<= 1;
			r &= 0xFE;
			r |= (0x01 & oldC);
			if (r == 0)
			{
				SetFlag(FLAG_Z, FLAG_SET);
			}
			bus.Write(GetRegister16(REGISTER_HL), r);
		}
		break; 
	case 0x18: // RR X
		if (reg != 0xFF)
		{
			auto oldC = GetFlag(FLAG_C);
			RotateRegisterRightCarry(reg);
			GetRegister(reg) |= (0x80 & oldC);
			if (GetRegister(reg) == 0)
			{
				SetFlag(FLAG_Z, FLAG_SET);
			}
		}
		else
		{
			auto oldC = GetFlag(FLAG_C);
			uint8_t r = bus.Read(GetRegister16(REGISTER_HL));
			SetFlag(FLAG_C, (r & 0x01) == 0 ? FLAG_CLEAR : FLAG_SET);
			SetFlag(FLAG_N, FLAG_CLEAR);
			SetFlag(FLAG_H, FLAG_CLEAR);
			SetFlag(FLAG_Z, FLAG_CLEAR);
			r >>= 1;
			r &= 0x7F;
			r |= (0x80 & oldC);
			if (r == 0)
			{
				SetFlag(FLAG_Z, FLAG_SET);
			}
			bus.Write(GetRegister16(REGISTER_HL), r);
		}
		break; 
	case 0x20: // SLA X
		if (reg != 0xFF)
		{
			RotateRegisterLeft(reg);
			GetRegister(reg) &= 0xFE;
			if (GetRegister(reg) == 0)
			{
				SetFlag(FLAG_Z, FLAG_SET);
			}
		}
		else
		{
			uint8_t r = bus.Read(GetRegister16(REGISTER_HL));
			SetFlag(FLAG_C, (r & 0x80) == 0 ? FLAG_CLEAR : FLAG_SET);
			SetFlag(FLAG_N, FLAG_CLEAR);
			SetFlag(FLAG_H, FLAG_CLEAR);
			SetFlag(FLAG_Z, FLAG_CLEAR);
			r <<= 1;
			if (r == 0)
			{
				SetFlag(FLAG_Z, FLAG_SET);
			}
			bus.Write(GetRegister16(REGISTER_HL), r);
		}
		break; 
	case 0x28: // SRA X
		if (reg != 0xFF)
		{
			uint8_t msb = GetRegister(reg) & 0x80;
			RotateRegisterRight(reg);
			GetRegister(reg) &= 0x7F;
			GetRegister(reg) |= msb;
		}
		else
		{
			uint8_t r = bus.Read(GetRegister16(REGISTER_HL));
			uint8_t msb = r & 0x80;
			SetFlag(FLAG_C, (r & 0x01) == 0 ? FLAG_CLEAR : FLAG_SET);
			SetFlag(FLAG_N, FLAG_CLEAR);
			SetFlag(FLAG_H, FLAG_CLEAR);
			SetFlag(FLAG_Z, FLAG_CLEAR);
			r >>= 1;
			r |= msb;
			bus.Write(GetRegister16(REGISTER_HL), r);
		}
		break;
	case 0x30: // SWAP X
	{
		uint8_t r, low, high;
		if (reg != 0xFF)
		{
			r = GetRegister(reg);
		}
		else
		{
			r = bus.Read(GetRegister16(REGISTER_HL));
		}
		low = r & 0x0F;
		high = (r & 0xF0) >> 4;
		r = (low << 4) | high;
		SetFlag(FLAG_N, FLAG_CLEAR);
		SetFlag(FLAG_H, FLAG_CLEAR);
		SetFlag(FLAG_C, FLAG_CLEAR);
		SetFlag(FLAG_Z, r == 0 ? FLAG_SET : FLAG_CLEAR);
		if (reg == 0xFF)
		{
			bus.Write(GetRegister16(REGISTER_HL), r);
		}
		else
		{
			GetRegister(reg) = r;
		}
	}
		break;
	case 0x38: // SRL X
		if (reg != 0xFF)
		{
			RotateRegisterRight(reg);
			GetRegister(reg) &= 0x7F;
			if (GetRegister(reg) == 0)
			{
				SetFlag(FLAG_Z, FLAG_SET);
			}
		}
		else
		{
			uint8_t r = bus.Read(GetRegister16(REGISTER_HL));
			SetFlag(FLAG_C, (r & 0x01) == 0 ? FLAG_CLEAR : FLAG_SET);
			SetFlag(FLAG_N, FLAG_CLEAR);
			SetFlag(FLAG_H, FLAG_CLEAR);
			SetFlag(FLAG_Z, FLAG_CLEAR);
			r >>= 1;
			if (r == 0)
			{
				SetFlag(FLAG_Z, FLAG_SET);
			}
			bus.Write(GetRegister16(REGISTER_HL), r);
		}
		break;
	case 0x40: // BIT 0,X
	{
		uint8_t r = reg == 0xFF ? bus.Read(GetRegister16(REGISTER_HL)) : GetRegister(reg);
		SetFlag(FLAG_N, FLAG_CLEAR);
		SetFlag(FLAG_H, FLAG_SET);
		SetFlag(FLAG_Z, (r & 0x01) == 0 ? FLAG_SET : FLAG_CLEAR);
	}
		break;
	case 0x48: // BIT 1,X
	{
		uint8_t r = reg == 0xFF ? bus.Read(GetRegister16(REGISTER_HL)) : GetRegister(reg);
		SetFlag(FLAG_N, FLAG_CLEAR);
		SetFlag(FLAG_H, FLAG_SET);
		SetFlag(FLAG_Z, (r & 0x02) == 0 ? FLAG_SET : FLAG_CLEAR);
	}
		break;
	case 0x50: // BIT 2,X
	{
		uint8_t r = reg == 0xFF ? bus.Read(GetRegister16(REGISTER_HL)) : GetRegister(reg);
		SetFlag(FLAG_N, FLAG_CLEAR);
		SetFlag(FLAG_H, FLAG_SET);
		SetFlag(FLAG_Z, (r & 0x04) == 0 ? FLAG_SET : FLAG_CLEAR);
	}
		break;
	case 0x58: // BIT 3,X
	{
		uint8_t r = reg == 0xFF ? bus.Read(GetRegister16(REGISTER_HL)) : GetRegister(reg);
		SetFlag(FLAG_N, FLAG_CLEAR);
		SetFlag(FLAG_H, FLAG_SET);
		SetFlag(FLAG_Z, (r & 0x08) == 0 ? FLAG_SET : FLAG_CLEAR);
	}
		break;
	case 0x60: // BIT 4,X
	{
		uint8_t r = reg == 0xFF ? bus.Read(GetRegister16(REGISTER_HL)) : GetRegister(reg);
		SetFlag(FLAG_N, FLAG_CLEAR);
		SetFlag(FLAG_H, FLAG_SET);
		SetFlag(FLAG_Z, (r & 0x10) == 0 ? FLAG_SET : FLAG_CLEAR);
	}
		break;
	case 0x68: // BIT 5,X
	{
		uint8_t r = reg == 0xFF ? bus.Read(GetRegister16(REGISTER_HL)) : GetRegister(reg);
		SetFlag(FLAG_N, FLAG_CLEAR);
		SetFlag(FLAG_H, FLAG_SET);
		SetFlag(FLAG_Z, (r & 0x20) == 0 ? FLAG_SET : FLAG_CLEAR);
	}
		break;
	case 0x70: // BIT 6,X
	{
		uint8_t r = reg == 0xFF ? bus.Read(GetRegister16(REGISTER_HL)) : GetRegister(reg);
		SetFlag(FLAG_N, FLAG_CLEAR);
		SetFlag(FLAG_H, FLAG_SET);
		SetFlag(FLAG_Z, (r & 0x40) == 0 ? FLAG_SET : FLAG_CLEAR);
	}
		break;
	case 0x78: // BIT 7,X
	{
		uint8_t r = reg == 0xFF ? bus.Read(GetRegister16(REGISTER_HL)) : GetRegister(reg);
		SetFlag(FLAG_N, FLAG_CLEAR);
		SetFlag(FLAG_H, FLAG_SET);
		SetFlag(FLAG_Z, (r & 0x80) == 0 ? FLAG_SET : FLAG_CLEAR);
	}
		break;
	case 0x80: // RES 0,X
		if (reg != 0xFF)
		{
			GetRegister(reg) &= 0xFE;
		}
		else
		{
			bus.Write(GetRegister16(REGISTER_HL), bus.Read(GetRegister16(REGISTER_HL)) & 0xFE);
		}
		break;
	case 0x88: // RES 1,X 
		if (reg != 0xFF)
		{
			GetRegister(reg) &= 0xFD;
		}
		else
		{
			bus.Write(GetRegister16(REGISTER_HL), bus.Read(GetRegister16(REGISTER_HL)) & 0xFD);
		}
		break;
	case 0x90: // RES 2,X 
		if (reg != 0xFF)
		{
			GetRegister(reg) &= 0xFB;
		}
		else
		{
			bus.Write(GetRegister16(REGISTER_HL), bus.Read(GetRegister16(REGISTER_HL)) & 0xFB);
		}
		break;
	case 0x98: // RES 3,X
		if (reg != 0xFF)
		{
			GetRegister(reg) &= 0xF7;
		}
		else
		{
			bus.Write(GetRegister16(REGISTER_HL), bus.Read(GetRegister16(REGISTER_HL)) & 0xF7);
		}
		break;
	case 0xA0: // RES 4,X
		if (reg != 0xFF)
		{
			GetRegister(reg) &= 0xEF;
		}
		else
		{
			bus.Write(GetRegister16(REGISTER_HL), bus.Read(GetRegister16(REGISTER_HL)) & 0xEF);
		}
		break;
	case 0xA8: // RES 5,X
		if (reg != 0xFF)
		{
			GetRegister(reg) &= 0xDF;
		}
		else
		{
			bus.Write(GetRegister16(REGISTER_HL), bus.Read(GetRegister16(REGISTER_HL)) & 0xDF);
		}
		break;
	case 0xB0: // RES 6,X
		if (reg != 0xFF)
		{
			GetRegister(reg) &= 0xBF;
		}
		else
		{
			bus.Write(GetRegister16(REGISTER_HL), bus.Read(GetRegister16(REGISTER_HL)) & 0xBF);
		}
		break;
	case 0xB8: // RES 7,X
		if (reg != 0xFF)
		{
			GetRegister(reg) &= 0x7F;
		}
		else
		{
			bus.Write(GetRegister16(REGISTER_HL), bus.Read(GetRegister16(REGISTER_HL)) & 0x7F);
		}
		break;
	case 0xC0: // SET 0,X
		if (reg != 0xFF)
		{
			GetRegister(reg) |= 0x01;
		}
		else
		{
			bus.Write(GetRegister16(REGISTER_HL), bus.Read(GetRegister16(REGISTER_HL)) | 0x01);
		}
		break;
	case 0xC8: // SET 1,X
		if (reg != 0xFF)
		{
			GetRegister(reg) |= 0x02;
		}
		else
		{
			bus.Write(GetRegister16(REGISTER_HL), bus.Read(GetRegister16(REGISTER_HL)) | 0x02);
		}
		break;
	case 0xD0: // SET 2,X
		if (reg != 0xFF)
		{
			GetRegister(reg) |= 0x04;
		}
		else
		{
			bus.Write(GetRegister16(REGISTER_HL), bus.Read(GetRegister16(REGISTER_HL)) | 0x04);
		}
		break;
	case 0xD8: // SET 3,X
		if (reg != 0xFF)
		{
			GetRegister(reg) |= 0x08;
		}
		else
		{
			bus.Write(GetRegister16(REGISTER_HL), bus.Read(GetRegister16(REGISTER_HL)) | 0x08);
		}
		break;
	case 0xE0: // SET 4,X
		if (reg != 0xFF)
		{
			GetRegister(reg) |= 0x10;
		}
		else
		{
			bus.Write(GetRegister16(REGISTER_HL), bus.Read(GetRegister16(REGISTER_HL)) | 0x10);
		}
		break;
	case 0xE8: // SET 5,X
		if (reg != 0xFF)
		{
			GetRegister(reg) |= 0x20;
		}
		else
		{
			bus.Write(GetRegister16(REGISTER_HL), bus.Read(GetRegister16(REGISTER_HL)) | 0x20);
		}
		break;
	case 0xF0: // SET 6,X
		if (reg != 0xFF)
		{
			GetRegister(reg) |= 0x40;
		}
		else
		{
			bus.Write(GetRegister16(REGISTER_HL), bus.Read(GetRegister16(REGISTER_HL)) | 0x40);
		}
		break;
	case 0xF8: // SET 7,X
		if (reg != 0xFF)
		{
			GetRegister(reg) |= 0x80;
		}
		else
		{
			bus.Write(GetRegister16(REGISTER_HL), bus.Read(GetRegister16(REGISTER_HL)) | 0x80);
		}
		break;
	} 
}
