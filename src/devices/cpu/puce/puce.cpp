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

#define VERBOSE (1)
#include "logmacro.h"

#define RL(n) m_reg[n].w
#define RA(n) m_reg[n].b.l
#define RB(n) m_reg[n].b.h

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
	m_program_config("program", ENDIANNESS_BIG, 16, 16)
{
}

void puce_device::device_start()
{
	LOG("%s: device_start\n", machine().describe_context());
	m_program = &space(AS_PROGRAM);

	// register our state for the debugger

	/*
	state_add(STATE_GENFLAGS,  "GENFLAGS",  m_l).callimport().callexport().formatstr("%1s").noshow();
	state_add(STATE_GENPC,     "PC",        m_pc.w).callimport();
	state_add(STATE_GENPCBASE, "CURPC",     m_prvpc.w).callimport().noshow();
	*/
	state_add(STATE_GENPC,     "GENPC",     m_pc); // .noshow();
	state_add(STATE_GENPCBASE, "CURPC",     m_pc); // .noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS",  m_di).callexport().formatstr("%9s");
	for(int r = 0; r < 16; r++) {
		state_add(PUCE_L0 + r, string_format("L%d", r).c_str(), RL(r));
		state_add(PUCE_A0 + r, string_format("A%d", r).c_str(), RA(r)); // .noshow()
		state_add(PUCE_B0 + r, string_format("B%d", r).c_str(), RB(r)); // .noshow()
	}
	state_add(PUCE_DI,         "DI",        m_di).mask(0xf);
	state_add(PUCE_LVL,        "LVL",       m_lvl).mask(0x3);

	// setup regtable
	save_item(m_lvl, "Lvl");
	save_item(m_di, "DI");
	for(int r = 0; r < 16; r++) {
		save_item(RL(r), string_format("L%d", r).c_str());
	}

	// set our instruction counter
	set_icountptr(m_icount);
}

void puce_device::device_stop()
{
	LOG("%s: device_stop\n", machine().describe_context());
}

void puce_device::device_reset()
{
	LOG("%s: device_reset\n", machine().describe_context());
	// Not sure if registers actually reset...
	memset(m_reg, 0, sizeof(m_reg));
	m_di = 0;
	// Start with Lvl3 at 0x8000
	m_lvl = 3;
	RL(1) = 0x8000;
	set_pc();
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
	switch (entry.index())
	{
		case STATE_GENFLAGS:
		{
			str = string_format("%c%c%c%c %c%c%c%c",
				BIT(m_di,0)     ? 'C':'c',
				BIT(m_di,1)     ? 'Z':'z',
				BIT(m_di,2)     ? 'H':'h',
				BIT(m_di,3)     ? '3':'.',
				BIT(m_di,4)     ? '4':'.',
				BIT(m_di,5)     ? '5':'.',
				BIT(m_di,6)     ? '6':'.',
				BIT(m_di,7)     ? '7':'.');
		}
		break;
	}
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

void puce_device::set_pc() {
		switch (m_lvl) {
		case 4:
			m_pc = RL(0);
			break;
		case 3:
			m_pc = RL(1);
			break;
		case 2:
			m_pc = 0x8200 | RA(12);
			break;
		case 1:
			m_pc = 0x8100 | RA(13);
			break;
		}
}

void puce_device::inc_vpc() {
		switch (m_lvl) {
		case 4:
			RL(0)++;
			break;
		case 3:
			RL(1)++;
			break;
		case 2:
			RA(12)++;
			break;
		case 1:
			RB(13)++;
			break;
		}
}

void puce_device::execute_run()
{
	while (m_icount > 0)
	{
		set_pc();
		LOG("%s: device_exec lvl=%i pc=%04x\n", machine().describe_context(), m_lvl, m_pc);
		debugger_instruction_hook(m_pc);
		// everything is a nop for now

		--m_icount;
		inc_vpc();
	}
}
