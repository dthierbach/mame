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
	m_program_config("program", ENDIANNESS_BIG, 16, 16, -1)
{
}

void puce_device::device_start()
{
	LOG("%s: device_start\n", machine().describe_context());
	m_program = &space(AS_PROGRAM);

	// register our state for the debugger
	state_add(PUCE_LVL,        "LVL",       m_lvl).mask(0x3);
	state_add(STATE_GENPC,     "GENPC",     m_pc); // .noshow();
	state_add(STATE_GENPCBASE, "CURPC",     m_pc); // .noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS",  m_di).callexport().formatstr("%9s");
	state_add(PUCE_DI,         "DI",        m_di).mask(0xf);
	for(int r = 0; r < 16; r++) {
		state_add(PUCE_L0 + r, string_format("L%d", r).c_str(), RL(r));
		state_add(PUCE_A0 + r, string_format("A%d", r).c_str(), RA(r)).noshow();
		state_add(PUCE_B0 + r, string_format("B%d", r).c_str(), RB(r)).noshow();
	}

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
	get_vpc();
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
				BIT(m_di,7)     ? '7':'.',
				BIT(m_di,6)     ? '6':'.',
				BIT(m_di,5)     ? '5':'.',
				BIT(m_di,4)     ? '4':'.',
				BIT(m_di,3)     ? '3':'.',
				BIT(m_di,2)     ? 'H':'h',
				BIT(m_di,1)     ? 'Z':'z',
				BIT(m_di,0)     ? 'C':'c');
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

void puce_device::get_vpc() {
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

void puce_device::set_vpc_a(u16 a) {
		switch (m_lvl) {
		case 4:
		  RA(0) = a;
			break;
		case 3:
			RA(1) = a;
			break;
		case 2:
			RA(12) = a;
			break;
		case 1:
			RA(13) = a;
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
		get_vpc();
		LOG("%s: device_exec lvl=%i pc=%04x\n", machine().describe_context(), m_lvl, m_pc);
		debugger_instruction_hook(m_pc);
		// everything is a nop for now

		--m_icount;
		inc_vpc();
	}
}

inline void puce_device::op_sai(u16 j) {
		switch (m_lvl) {
		case 4:
		  RL(0) = (RL(0) & 0xe000) | (j & 0x1fff);
			break;
		case 3:
		  RL(1) = (RL(1) & 0xe000) | (j & 0x1fff);
			break;
		case 2:
			RA(12) = j & 0xff;
			break;
		case 1:
			RA(13) = j & 0xff;
			break;
		}
}

inline void puce_device::op_amd(u8 s, u8 t) {
}

inline void puce_device::op_mad(u8 s, u8 t) {
}

inline void puce_device::op_sade(u16 b) {
	if (1 == 0) { // ECOF...
		set_vpc_a(b);
	}
}

inline void puce_device::op_sadx(u8 e, u8 d, u16 b) {
	if (BIT(m_di, d) == e) {
		set_vpc_a(b);
	}
}

inline void puce_device::op_crta(u8 s, u8 t) {
	RA(s) = t;
}

inline void puce_device::op_crtb(u8 s, u8 t) {
	RB(s) = t;
}

inline void puce_device::op_comx(u8 u) {
	switch(u) {
		case 0: m_lvl = 4; break;
		case 1: m_lvl = 3; break;
	}
}

inline void puce_device::op_tba(u8 u, u8 v) {
	RA(u) = RB(v);
}

inline void puce_device::op_illegal(u16 opcode) {
	// log
	logerror("Illegal opcode");
}

// copy and paste from pucemake.py
inline void puce_device::decode(u16 pc, u16 opcode)
{
	u8 r = BIT(opcode, 12, 4);
	u8 s = BIT(opcode, 8, 4);
	u8 t = BIT(opcode, 0, 8);
	switch (r)
	{
	case 0:
	case 1:
	{
		u16 j = (pc & 0xe000) | (opcode & 0x1fff);
		op_sai(j);
		break;
	}
	case 2:
	{
		op_amd(s, t);
		break;
	}
	case 3:
	{
		op_mad(s, t);
		break;
	}
	case 4:
	{
		u16 b = (pc & 0xff00) | t;
		op_sade(b);
		break;
	}
	case 5:
	{
		op_crtb(s, t);
		break;
	}
	case 6:
	{
		u8 e = BIT(r, 0);
		u8 d = BIT(r, 1, 3);
		u16 b = (pc & 0xff00) | t;
		op_sadx(e, d, b);
		break;
	}
	case 7:
	{
		op_crta(s, t);
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
				op_comx(u);
				break;
			}
			default:
				op_illegal(opcode);
				break;
			}
		case 0xe9:
		{
			op_tba(u, v);
			break;
		}
		default:
			op_illegal(opcode);
		}
	}
}
