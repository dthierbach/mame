// license:BSD-3-Clause
// copyright-holders:Dirk Thierbach

#include "emu.h"
#include "pucedasm.h"

// #define VERBOSE (1)
// #include "logmacro.h"
#define LOG(args...) util::stream_format(std::cout, args)
#include <iostream>
#include <iomanip>

u32 puce_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t puce_disassembler::disassemble(std::ostream &stream, offs_t pc,
	const puce_disassembler::data_buffer &opcodes, const puce_disassembler::data_buffer &params)
{
	u16 opcode = opcodes.r16(pc);
	// LOG("  disasm pc=%04x opcode=%08x\n", pc, opcode);
	decode(stream, pc, opcode);

	return 1 | SUPPORTED;
}

// copy and paste from pucemake.py

inline void puce_disassembler::decode(std::ostream &stream, u16 pc, u16 opcode)
{
	std::string dis;
	std::string com;
	u8 r = BIT(opcode, 12, 4);
	u8 s = BIT(opcode, 8, 4);
	u8 t = BIT(opcode, 0, 8);
	switch (r)
	{
	case 0:
	case 1:
	{
		u16 j = (pc & 0xe000) | (opcode & 0x1fff);
		dis = string_format("SAI %04x", j);
		com = string_format("jump %04x", j);
		break;
	}
	case 2:
	{
		dis = string_format("AMD A%02x,C%02x", s, t);
		com = string_format("[%02x] := A%d", t, s);
		break;
	}
	case 3:
	{
		dis = string_format("MAD A%02x,C%02x", s, t);
		com = string_format("A%d := [%02x]", s, t);
		break;
	}
	case 4:
	{
		u16 k = (pc & 0xff00) | t;
		dis = string_format("SADE %04x", k);
		com = string_format("br EOCF,%04x", k);
		break;
	}
	case 5:
	{
		dis = string_format("CRTB B%02x,C%02x", s, t);
		com = string_format("B%d := 0x%02x", s, t);
		break;
	}
	case 6:
	{
		u8 e = BIT(s, 0);
		u8 d = BIT(s, 1, 3);
		u16 k = (pc & 0xff00) | t;
		dis = string_format("SAD%d D%x,%04x", e, d, k);
		com = string_format("br D%d=%d,%04x", d, e, k);
		break;
	}
	case 7:
	{
		dis = string_format("CRTA A%02x,C%02x", s, t);
		com = string_format("A%d := 0x%02x", s, t);
		break;
	}
	default:
		u8 u = BIT(opcode, 4, 4);
		u8 v = BIT(opcode, 0, 4);
		u8 w = BIT(opcode, 8, 8);
		switch (w)
		{
		case 0x82:
		{
			dis = string_format("AMIM M%02x,A%02x", u, v);
			com = string_format("[M%d--] := A%d", u, v);
			break;
		}
		case 0x83:
			switch (v)
			{
			case 0xf:
			{
				dis = string_format("TADI A%02x", u);
				com = string_format("DI := A%d", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0x85:
			switch (v)
			{
			case 0xf:
			{
				dis = string_format("ICA A%02x", u);
				com = string_format("A%d++", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0x86:
		{
			dis = string_format("ADD A%02x,B%02x", u, v);
			com = string_format("A%d + B%d + DI0", u, v);
			break;
		}
		case 0x87:
		{
			dis = string_format("ORB A%02x,B%02x", u, v);
			com = string_format("B%d := A%d or B%d", v, u, v);
			break;
		}
		case 0x88:
		{
			dis = string_format("AMIP M%02x,A%02x", u, v);
			com = string_format("[M%d++] := A%d", u, v);
			break;
		}
		case 0x89:
		{
			dis = string_format("BMI M%02x,B%02x", u, v);
			com = string_format("[M%d] := B%d", u, v);
			break;
		}
		case 0x8a:
		{
			dis = string_format("BMIM M%02x,B%02x", u, v);
			com = string_format("[M%d--] := B%d", u, v);
			break;
		}
		case 0x8b:
			switch (v)
			{
			case 0xf:
			{
				dis = string_format("ROTA A%02x", u);
				com = string_format("A%d.M <-> A%d.P", u, u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0x8c:
		{
			dis = string_format("BMIP M%02x,B%02x", u, v);
			com = string_format("[M%d++] := B%d", u, v);
			break;
		}
		case 0x8d:
			switch (v)
			{
			case 0x8:
			{
				dis = string_format("EMI M%02x", u);
				com = string_format("[M%d] <- data.A", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0x8e:
			switch (v)
			{
			case 0xf:
			{
				dis = string_format("VRA A%02x", u);
				com = string_format("A%d==0", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0x90:
			switch (v)
			{
			case 0x0:
			{
				dis = string_format("MEI M%02x", u);
				com = string_format("data.A <- [M%d]", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0x91:
		{
			dis = string_format("MAI M%02x,A%02x", u, v);
			com = string_format("A%d := [M%d]", v, u);
			break;
		}
		case 0x92:
		{
			dis = string_format("MAIM M%02x,A%02x", u, v);
			com = string_format("A%d := [M%d--]", v, u);
			break;
		}
		case 0x93:
			switch (v)
			{
			case 0xf:
			{
				dis = string_format("TBDI B%02x", u);
				com = string_format("DI := B%d", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0x94:
			switch (v)
			{
			case 0x0:
			{
				dis = string_format("MEIP M%02x", u);
				com = string_format("data.A <- [M%d++]", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0x95:
			switch (v)
			{
			case 0xf:
			{
				dis = string_format("ICB B%02x", u);
				com = string_format("B%d++", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0x96:
		{
			dis = string_format("ADDA A%02x,B%02x", u, v);
			com = string_format("A%d := A%d + B%d + DI0", u, u, v);
			break;
		}
		case 0x97:
		{
			dis = string_format("AND A%02x,B%02x", u, v);
			com = string_format("A%d and B%d", u, v);
			break;
		}
		case 0x98:
		{
			dis = string_format("MAIP M%02x,A%02x", u, v);
			com = string_format("A%d := [M%d++]", v, u);
			break;
		}
		case 0x99:
		{
			dis = string_format("MBI M%02x,B%02x", u, v);
			com = string_format("B%d := [M%d]", v, u);
			break;
		}
		case 0x9a:
		{
			dis = string_format("MBIM M%02x,B%02x", u, v);
			com = string_format("B%d := [M%d--]", v, u);
			break;
		}
		case 0x9b:
			switch (v)
			{
			case 0xf:
			{
				dis = string_format("ROTB B%02x", u);
				com = string_format("B%d.M <-> B%d.P", u, u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0x9c:
		{
			dis = string_format("MBIP M%02x,B%02x", u, v);
			com = string_format("B%d := [M%d++]", v, u);
			break;
		}
		case 0x9d:
			switch (v)
			{
			case 0x0:
			{
				dis = string_format("MEIM M%02x", u);
				com = string_format("data.A <- [M%d--]", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0x9e:
			switch (v)
			{
			case 0xf:
			{
				dis = string_format("VRB B%02x", u);
				com = string_format("B%d==0", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xa0:
		{
			u8 g = BIT(u, 0);
			u8 f = BIT(u, 1, 3);
			dis = string_format("ICD%d D%x,L%02x", g, f, v);
			com = string_format("L%d++ if D%x=%d", v, f, g);
			break;
		}
		case 0xa1:
			switch (v)
			{
			case 0x8:
			{
				dis = string_format("EMIM M%02x", u);
				com = string_format("[M%d--] <- data.A", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xa2:
			switch (v)
			{
			case 0x8:
			{
				dis = string_format("EMIP M%02x", u);
				com = string_format("[M%d++] <- data.A", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xa3:
			switch (v)
			{
			case 0xf:
			{
				dis = string_format("SDIA A%02x", u);
				com = string_format("A%d <-> DI", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xa5:
			switch (v)
			{
			case 0xf:
			{
				dis = string_format("ICL L%02x", u);
				com = string_format("L%d++", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xa6:
		{
			dis = string_format("ADDB A%02x,B%02x", u, v);
			com = string_format("B%d := A%d + B%d + DI0", v, u, v);
			break;
		}
		case 0xa7:
		{
			dis = string_format("ANDA A%02x,B%02x", u, v);
			com = string_format("A%d := A%d and B%d", u, u, v);
			break;
		}
		case 0xa8:
		{
			dis = string_format("AMI M%02x,A%02x", u, v);
			com = string_format("[M%d] := A%d", u, v);
			break;
		}
		case 0xa9:
			switch (v)
			{
			case 0x8:
			{
				dis = string_format("EDB B%02x", u);
				com = string_format("B%d <- data.A", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xaa:
			switch (v)
			{
			case 0x0:
			{
				dis = string_format("ENTL L%02x", u);
				com = string_format("A%d <- name, B%d <- type", u, u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xab:
			switch (v)
			{
			case 0xf:
			{
				dis = string_format("AZAM A%02x", u);
				com = string_format("A%d.M := 0", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xad:
			switch (v)
			{
			case 0xf:
			{
				dis = string_format("EDC L%02x", u);
				com = string_format("L%d.MMM--, ECOF if zero", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xae:
			switch (v)
			{
			case 0xf:
			{
				dis = string_format("DCA A%02x", u);
				com = string_format("A%d--", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xb1:
			switch (v)
			{
			case 0x4:
			{
				dis = string_format("ESE M%02x", u);
				com = string_format("sel <- [M%d]", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xb2:
			switch (v)
			{
			case 0xf:
			{
				dis = string_format("ETIB B%02x", u);
				com = string_format("B%d <- type", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xb3:
			switch (v)
			{
			case 0xf:
			{
				dis = string_format("SDIB B%02x", u);
				com = string_format("B%d <-> DI", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xb4:
			switch (v)
			{
			case 0x2:
			{
				dis = string_format("ECO M%02x", u);
				com = string_format("cmd <- [M%d]", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xb6:
		{
			dis = string_format("SOT A%02x,B%02x", u, v);
			com = string_format("A%d - B%d + DI0", u, v);
			break;
		}
		case 0xb7:
		{
			dis = string_format("ANDB A%02x,B%02x", u, v);
			com = string_format("B%d := A%d and B%d", v, u, v);
			break;
		}
		case 0xb8:
			switch (v)
			{
			case 0x8:
			{
				dis = string_format("EDA A%02x", u);
				com = string_format("A%d <- data.A", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xb9:
			switch (v)
			{
			case 0x0:
			{
				dis = string_format("ENUA A%02x", u);
				com = string_format("A%d <- name", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xba:
		{
			dis = string_format("SAB A%02x,B%02x", u, v);
			com = string_format("A%d <-> B%d", u, v);
			break;
		}
		case 0xbb:
			switch (v)
			{
			case 0xf:
			{
				dis = string_format("AZAP A%02x", u);
				com = string_format("A%d.P := 0", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xbc:
		{
			dis = string_format("SLL L%02x,L%02x", u, v);
			com = string_format("L%d <-> L%d", u, v);
			break;
		}
		case 0xbd:
			switch (v)
			{
			case 0x0:
			{
				dis = string_format("COM%d", u);
				com = string_format("  C%d", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xbe:
			switch (v)
			{
			case 0xf:
			{
				dis = string_format("DCB B%02x", u);
				com = string_format("B%d--", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xc3:
			switch (v)
			{
			case 0x0:
			{
				dis = string_format("SHDA A%02x", u);
				com = string_format("A%d, DI0 := 0 >> A%d", u, u);
				break;
			}
			case 0x1:
			{
				dis = string_format("SLDA A%02x", u);
				com = string_format("A%d, DI0 := DI0 >> A%d", u, u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xc4:
			switch (v)
			{
			case 0x0:
			{
				dis = string_format("SHSA A%02x", u);
				com = string_format("DI0, A%d := A%d << 0", u, u);
				break;
			}
			case 0x1:
			{
				dis = string_format("SLSA A%02x", u);
				com = string_format("DI0, A%d := A%d << DI0", u, u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xc5:
			switch (v)
			{
			case 0xf:
			{
				dis = string_format("TDIA A%02x", u);
				com = string_format("A%d := DI", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xc6:
		{
			dis = string_format("SOTA A%02x,B%02x", u, v);
			com = string_format("A%d := A%d - B%d + DI0", u, u, v);
			break;
		}
		case 0xc7:
		{
			dis = string_format("ORE A%02x,B%02x", u, v);
			com = string_format("A%d xor B%d", u, v);
			break;
		}
		case 0xc8:
		{
			dis = string_format("REDI C%02x", t);
			com = string_format("reset DI 0x%02x", t);
			break;
		}
		case 0xc9:
		{
			dis = string_format("SEDI C%02x", t);
			com = string_format("set DI 0x%02x", t);
			break;
		}
		case 0xca:
			switch (v)
			{
			case 0x0:
			{
				dis = string_format("TCCA A%02x", u);
				com = string_format("A%d <- con", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xcb:
			switch (v)
			{
			case 0xf:
			{
				dis = string_format("AZBM B%02x", u);
				com = string_format("B%d.M := 0", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xd1:
		{
			dis = string_format("MLI M%02x,L%02x", u, v);
			com = string_format("L%d := [M%d]", v, u);
			break;
		}
		case 0xd3:
			switch (v)
			{
			case 0x0:
			{
				dis = string_format("SHDB B%02x", u);
				com = string_format("B%d, DI0 := 0 >> B%d", u, u);
				break;
			}
			case 0x1:
			{
				dis = string_format("SLDB B%02x", u);
				com = string_format("B%d, DI0 := DI0 >> B%d", u, u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xd4:
			switch (v)
			{
			case 0x0:
			{
				dis = string_format("SHSB B%02x", u);
				com = string_format("DI0, B%d := B%d << 0", u, u);
				break;
			}
			case 0x1:
			{
				dis = string_format("SLSB B%02x", u);
				com = string_format("DI0, B%d := B%d << DI0", u, u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xd5:
			switch (v)
			{
			case 0xf:
			{
				dis = string_format("TDIB B%02x", u);
				com = string_format("B%d := DI", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xd6:
		{
			dis = string_format("SOTB A%02x,B%02x", u, v);
			com = string_format("B%d := A%d - B%d + DI0", v, u, v);
			break;
		}
		case 0xd7:
		{
			dis = string_format("OREA A%02x,B%02x", u, v);
			com = string_format("A%d := A%d xor B%d", u, u, v);
			break;
		}
		case 0xd8:
		{
			dis = string_format("TAB A%02x,B%02x", u, v);
			com = string_format("B%d := A%d", v, u);
			break;
		}
		case 0xd9:
		{
			dis = string_format("TABP A%02x,B%02x", u, v);
			com = string_format("B%d.P := A%d.P", v, u);
			break;
		}
		case 0xda:
			switch (v)
			{
			case 0x1:
			{
				dis = string_format("TDMA A%02x", u);
				com = string_format("A%d <- con.M", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xdb:
			switch (v)
			{
			case 0xf:
			{
				dis = string_format("AZBP B%02x", u);
				com = string_format("B%d.P := 0", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xdd:
		{
			dis = string_format("MLIM M%02x,L%02x", u, v);
			com = string_format("L%d := [M%d--]", v, u);
			break;
		}
		case 0xde:
		{
			dis = string_format("MLIP M%02x,L%02x", u, v);
			com = string_format("L%d := [M%d++]", v, u);
			break;
		}
		case 0xe0:
			switch (v)
			{
			case 0x8:
			{
				dis = string_format("ESI M%02x", u);
				com = string_format("[M%d] <- data/type", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xe1:
		{
			dis = string_format("LMI M%02x,L%02x", u, v);
			com = string_format("[M%d}] := L%d", u, v);
			break;
		}
		case 0xe2:
		{
			dis = string_format("LPMIP M%02x,L%02x", u, v);
			com = string_format("[M%d++] := ++L%d", u, v);
			break;
		}
		case 0xe5:
			switch (v)
			{
			case 0xf:
			{
				dis = string_format("DCL L%02x", u);
				com = string_format("L%d--", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xe6:
		{
			dis = string_format("OR A%02x,B%02x", u, v);
			com = string_format("A%d or B%d", u, v);
			break;
		}
		case 0xe7:
		{
			dis = string_format("OREB A%02x,B%02x", u, v);
			com = string_format("B%d := A%d xor B%d", v, u, v);
			break;
		}
		case 0xe8:
		{
			dis = string_format("TABM A%02x,B%02x", u, v);
			com = string_format("B%d.M := A%d.M", v, u);
			break;
		}
		case 0xe9:
		{
			dis = string_format("TBA A%02x,B%02x", u, v);
			com = string_format("B%d := A%d", v, u);
			break;
		}
		case 0xea:
			switch (v)
			{
			case 0x2:
			{
				dis = string_format("TDPA A%02x", u);
				com = string_format("A%d <- con.P", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xeb:
			switch (v)
			{
			case 0x8:
			{
				dis = string_format("ESIP M%02x", u);
				com = string_format("[M%d++] <- data/type", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xec:
			switch (v)
			{
			case 0x8:
			{
				dis = string_format("ESIM M%02x", u);
				com = string_format("[M%d--] <- data/type", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xed:
		{
			dis = string_format("LMIM M%02x,L%02x", u, v);
			com = string_format("[M%d--] := L%d", u, v);
			break;
		}
		case 0xee:
		{
			dis = string_format("LMIP M%02x,L%02x", u, v);
			com = string_format("[M%d++] := L%d", u, v);
			break;
		}
		case 0xf1:
			switch (v)
			{
			case 0x0:
			{
				dis = string_format("SEI M%02x", u);
				com = string_format("data.BA <- [%Md]", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xf5:
			switch (v)
			{
			case 0xf:
			{
				dis = string_format("VRL L%02x", u);
				com = string_format("L%d==0", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xf6:
		{
			dis = string_format("ORA A%02x,B%02x", u, v);
			com = string_format("A%d := A%d or B%d", u, u, v);
			break;
		}
		case 0xf7:
			switch (v)
			{
			case 0x0:
			{
				dis = string_format("SEIP M%02x", u);
				com = string_format("data.BA <- [{M%d++]", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xf8:
		{
			dis = string_format("TBAP A%02x,B%02x", u, v);
			com = string_format("A%d.P := B%d.P", u, v);
			break;
		}
		case 0xf9:
		{
			dis = string_format("TBAM A%02x,B%02x", u, v);
			com = string_format("A%d.M := B%d.M", u, v);
			break;
		}
		case 0xfa:
		{
			dis = string_format("TABC A%02x,B%02x", u, v);
			com = string_format("con <- A%d,B%d", u, v);
			break;
		}
		case 0xfb:
			switch (v)
			{
			case 0x8:
			{
				dis = string_format("DEA L%02x", u);
				com = string_format("data.B <- B%d, A%d <- data.A", u, u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xfc:
			switch (v)
			{
			case 0x0:
			{
				dis = string_format("DAE L%02x", u);
				com = string_format("data.BA <- L%d", u);
				break;
			}
			case 0x2:
			{
				dis = string_format("CAE L%02x", u);
				com = string_format("cmd.BA <- L%d", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		case 0xfd:
			switch (v)
			{
			case 0x0:
			{
				dis = string_format("SEIM M%02x", u);
				com = string_format("data.BA <- [M%d--]", u);
				break;
			}
			default:
				dis = "???";
				com = "";
			}
			break;
		default:
			dis = "???";
			com = "";
		}
	}
	util::stream_format(stream, "%-15s ; %s", dis, com);
}
