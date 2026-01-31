// license:BSD-3-Clause
// copyright-holders:Dirk Thierbach
/*
    Two-board TTL CPU "PUCE" made by Olivetti.

		Used in the TC800, PC6060, and with a variant in the P6066.

    Written by Dirk Thierbach
*/

#include "emu.h"
#include "puce.h"
#include "pucedasm.h"

#define OP          ((op >> 011) & 07)

DEFINE_DEVICE_TYPE(PUCE, puce_device, "puce_cpu", "Olivetti PUCE")

//-------------------------------------------------
//  puce_device - constructor
//-------------------------------------------------

puce_device::puce_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: puce_device(mconfig, PUCE, tag, owner, clock)
{
}

puce_device::puce_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	cpu_device(mconfig, type, tag, owner, clock),
	// ..., data width, address width, addr shift
	m_program_config("program", ENDIANNESS_BIG, 8, 16)
{
}

void puce_device::device_start()
{
	m_program = &space(AS_PROGRAM);

	// register our state for the debugger

	/*
	state_add(STATE_GENPC,     "GENPC",     m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC",     m_pc).noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS",  m_l).callimport().callexport().formatstr("%1s").noshow();
	*/
	for(int r = 0; r < 16; r++)
		state_add(PUCE_L0 + r, string_format("L%d", r).c_str(), m_l[r]);
	state_add(PUCE_DI,         "DI",        m_di).mask(0xff);
	state_add(PUCE_LVL,        "LVL",       m_lvl).mask(0xf);

	// setup regtable
	save_item(NAME(m_lvl));
	save_item(NAME(m_di));
	save_item(NAME(m_l));

	// set our instruction counter
	set_icountptr(m_icount);
}

void puce_device::device_stop()
{
}

void puce_device::device_reset()
{
	// Not sure if registers actually reset...
	memset(m_l, 0, sizeof(m_l));
	m_di = 0;
	// Start with Lvl3 at 0x8000
	m_lvl = 3;
	m_l[1] = 0x8000;
}


//-------------------------------------------------
//  memory_space_config - return a vector of
//  address space configurations for this device
//-------------------------------------------------

device_memory_interface::space_config_vector puce_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
	};
}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void puce_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
}


//-------------------------------------------------
//  disassemble - call the disassembly
//  helper function
//-------------------------------------------------

std::unique_ptr<util::disasm_interface> puce_device::create_disassembler()
{
	return std::make_unique<puce_disassembler>();
}


//**************************************************************************
//  CORE EXECUTION LOOP
//**************************************************************************

//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t puce_device::execute_min_cycles() const noexcept
{
	return 1; // TODO
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t puce_device::execute_max_cycles() const noexcept
{
	return 1; // TODO
}


//-------------------------------------------------
//  execute_set_input - set the state of an input
//  line during execution
//-------------------------------------------------

void puce_device::execute_set_input(int inputnum, int state)
{
	// TODO
}


//-------------------------------------------------
//  execute_run - execute a timeslice's worth of
//  opcodes
//-------------------------------------------------

void puce_device::execute_run()
{
	while (m_icount > 0)
	{
		u16 pc;
		switch (m_lvl) {
		case 4:
			pc = m_l[0];
			break;
		case 3:
			pc = m_l[1];
			break;
		case 2:
			pc = 0x8200 | get_a(m_l[12]);
			break;
		case 1:
			pc = 0x8100 | get_a(m_l[13]);
			break;
		}

		debugger_instruction_hook(pc);
		// everything is a nop for now

		--m_icount;
		switch (m_lvl) {
		case 4:
			m_l[0] = (m_l[0] + 1) & 0xffff;
			break;
		case 3:
			m_l[1] = (m_l[1] + 1) & 0xffff;
			break;
		case 2:
			set_a(m_l[12], (m_l[12] + 1) & 0xff);
			break;
		case 1:
			set_a(m_l[13], (m_l[13] + 1) & 0xff);
			break;
		}
	}
}
