// license:BSD-3-Clause
// copyright-holders:Dirk Thierbach

#include "emu.h"
#include "pucedasm.h"

u32 puce_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t puce_disassembler::disassemble(std::ostream &stream, offs_t pc, const puce_disassembler::data_buffer &opcodes, const puce_disassembler::data_buffer &params)
{
	/*
	const u16 inst = opcodes.r16(pc);

	// Bits 0:2 (DEC numbering) specify instruction code
	switch (BIT(inst, 9, 3))
	{
	case 0:
		stream << "AND";
		//
		break;
	}
  */

  // Everything is NOP for now.
	stream << "NOP";

	return 1 | SUPPORTED;
}
