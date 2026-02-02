// license:BSD-3-Clause
// copyright-holders:Dirk Thierbach

#include "emu.h"
#include "pucedasm.h"

u32 puce_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t puce_disassembler::disassemble(std::ostream &stream, offs_t pc,
	const puce_disassembler::data_buffer &opcodes, const puce_disassembler::data_buffer &params)
{
	u16 opcode = opcodes.r16(pc);
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
		u16 b = (pc & 0xff00) | t;
		dis = string_format("SADE %04x", b);
		com = string_format("br EOCF,%04x", b);
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
		u8 e = BIT(r, 0);
		u8 d = BIT(r, 1, 3);
		u16 b = (pc & 0xff00) | t;
		dis = string_format("SAD%d D%x,%04x", e, d, b);
		com = string_format("br D%d=%d,%04x", d, e, b);
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
				break;
			}
		case 0xe9:
		{
			dis = string_format("TBA A%02x,B%02x", u, v);
			com = string_format("B%d := A%d", v, u);
			break;
		}
		default:
			dis = "???";
			com = "";
		}
	}
	util::stream_format(stream, "%-18s; %s", dis, com);
}

