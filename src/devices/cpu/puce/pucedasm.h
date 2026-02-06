// license:BSD-3-Clause
// copyright-holders:Dirk Thierbach

#ifndef MAME_CPU_PUCE_PUCEDASM_H
#define MAME_CPU_PUCE_PUCEDASM_H

#pragma once

class puce_disassembler : public util::disasm_interface
{
public:
	puce_disassembler() { }

protected:
	// util::disasm_interface overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
  void decode(std::ostream &stream, u16 pc, u16 opcode);

};

#endif // MAME_CPU_PUCE_PUCEDASM_H
