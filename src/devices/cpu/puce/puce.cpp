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

#define SETM(x,y) (x) = ((x) & 0xf0) | (y)
#define SETP(x,y) (x) = ((x) & 0x0f) | (y)
#define GETM(x) ((x) & 0xf0)
#define GETP(x) ((x) & 0x0f)

#define DIZERO(x) m_di = m_di & 0xfd | ((x==0)?0x2:0)

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
	m_program_config("program", ENDIANNESS_BIG, 16, 16, -1),
	m_data_config("data", ENDIANNESS_BIG, 8, 16, 0)
{
}

void puce_device::device_start()
{
	LOG("%s: device_start\n", machine().describe_context());
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);

	// register our state for the debugger
	state_add(PUCE_LVL,        "LVL",       m_lvl).mask(0x3);
	state_add(STATE_GENPC,     "GENPC",     m_pc); // .noshow();
	state_add(STATE_GENPCBASE, "CURPC",     m_pc); // .noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS",  m_di).callexport().formatstr("%9s");
	state_add(PUCE_DI,         "DI",        m_di);
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
		std::make_pair(AS_DATA, &m_data_config),
	};
}


uint16_t puce_device::read16_delegate(offs_t offset) {
	// LOG("%s: read16_delegate %04x\n", machine().describe_context(), offset);
	u16 data = m_data->read_word(offset << 1);
	return data;
}

void puce_device::write16_delegate(offs_t offset, uint16_t data) {
	// LOG("%s: write16_delegate %04x data=%04x\n", machine().describe_context(), offset, data);
	m_data->write_word(offset << 1, data);
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
		// LOG("%s: device_exec lvl=%i pc=%04x\n", machine().describe_context(), m_lvl, m_pc);
		debugger_instruction_hook(m_pc);
		// everything is a nop for now

		u16 opcode = m_program->read_word(m_pc);
		decode(m_pc, opcode);

		--m_icount;
		inc_vpc();
	}
}

void puce_device::op_illegal(u16 opcode) {
	// TODO cause debugger breakpoint
	logerror("Illegal opcode pc=%04x\n", m_pc);
}

// --------

inline void puce_device::op_sai(u16 j) {
  //jump %04x
  //no DI
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
  //[%02x] := A%d
  //no DI
	m_data->write_byte(t, s);
}

inline void puce_device::op_mad(u8 s, u8 t) {
  //A%d := [%02x]
  //no DI
	RA(s) = m_data->read_byte(t);
}

inline void puce_device::op_sade(u8 k) {
  //br EOCF,%04x
  //no DI
  op_illegal(NULL);
}

inline void puce_device::op_sadx(u8 e, u8 d, u8 k) {
  //br D%d=%d,%04x
  //no DI
	if (BIT(m_di,d) == e) {
		set_vpc_a(k);
	}
}

inline void puce_device::op_crta(u8 s, u8 t) {
  //A%d := 0x%02x
  //no DI
	RA(s) = t;
}

inline void puce_device::op_crtb(u8 s, u8 t) {
  //B%d := 0x%02x
  //no DI
	RB(s) = t;
}

// -------- memory

inline void puce_device::op_ami(u8 u, u8 v) {
  //[M%d] := A%d
  //no DI
	if (u <= 12) {
		m_data->write_byte(RL(u), RA(v));
	} else {
		m_data->write_byte(RA(u), RA(v));
	}
}

inline void puce_device::op_amim(u8 u, u8 v) {
  //[M%d--] := A%d
  //no DI
	if (u <= 12) {
		m_data->write_byte(RL(u), RA(v));
		RL(u)--;
	} else {
		m_data->write_byte(RA(u), RA(v));
		RA(u)--;
	}
}

inline void puce_device::op_amip(u8 u, u8 v) {
  //[M%d++] := A%d
  //no DI
	if (u <= 12) {
		m_data->write_byte(RL(u), RA(v));
		RL(u)++;
	} else {
		m_data->write_byte(RA(u), RA(v));
		RA(u)++;
	}
}

inline void puce_device::op_bmi(u8 u, u8 v) {
  //[M%d] := B%d
  //no DI
	if (u <= 12) {
		m_data->write_byte(RL(u), RB(v));
	} else {
		m_data->write_byte(RA(u), RB(v));
	}
}

inline void puce_device::op_bmim(u8 u, u8 v) {
  //[M%d--] := B%d
  //no DI
	if (u <= 12) {
		m_data->write_byte(RL(u), RB(v));
		RL(u)--;
	} else {
		m_data->write_byte(RA(u), RB(v));
		RA(u)--;
	}
}

inline void puce_device::op_bmip(u8 u, u8 v) {
  //[M%d++] := B%d
  //no DI
	if (u <= 12) {
		m_data->write_byte(RL(u), RB(v));
		RL(u)++;
	} else {
		m_data->write_byte(RA(u), RB(v));
		RA(u)++;
	}
}

inline void puce_device::op_lmi(u8 u, u8 v) {
  //[M%d}] := L%d
  //no DI
	if (u <= 12) {
		m_program->write_word(RL(u), RL(v));
	} else {
		m_program->write_word(RA(u), RL(v));
	}
}

inline void puce_device::op_lmim(u8 u, u8 v) {
  //[M%d--] := L%d
  //no DI
	if (u <= 12) {
		m_program->write_word(RL(u), RL(v));
		RL(u)--;
	} else {
		m_program->write_word(RA(u), RL(v));
		RA(u)--;
	}
}

inline void puce_device::op_lmip(u8 u, u8 v) {
  //[M%d++] := L%d
  //no DI
	if (u <= 12) {
		m_program->write_word(RL(u), RL(v));
		RL(u)++;
	} else {
		m_program->write_word(RA(u), RL(v));
		RA(u)++;
	}
}

inline void puce_device::op_lpmip(u8 u, u8 v) {
  //[M%d++] := ++L%d
  //no DI
	RL(v)++;
	if (u <= 12) {
		m_program->write_word(RL(u), RL(v));
		RL(u)++;
	} else {
		m_program->write_word(RA(u), RL(v));
		RA(u)++;
	}
}

inline void puce_device::op_mai(u8 u, u8 v) {
  //A%d := [M%d]
  //no DI
	if (u <= 12) {
		RA(v) = m_data->read_byte(RL(u));
	} else {
		RA(v) = m_data->read_byte(RA(u));
	}
}

inline void puce_device::op_maim(u8 u, u8 v) {
  //A%d := [M%d--]
  //no DI
	if (u <= 12) {
		RA(v) = m_data->read_byte(RL(u));
		RL(u)--;
	} else {
		RA(v) = m_data->read_byte(RA(u));
		RA(u)--;
	}
}

inline void puce_device::op_maip(u8 u, u8 v) {
  //A%d := [M%d++]
  //no DI
	if (u <= 12) {
		RA(v) = m_data->read_byte(RL(u));
		RL(u)++;
	} else {
		RA(v) = m_data->read_byte(RA(u));
		RA(u)++;
	}
}

inline void puce_device::op_mbi(u8 u, u8 v) {
  //B%d := [M%d]
  //no DI
	if (u <= 12) {
		RB(v) = m_data->read_byte(RL(u));
	} else {
		RB(v) = m_data->read_byte(RA(u));
	}
}

inline void puce_device::op_mbim(u8 u, u8 v) {
  //B%d := [M%d--]
  //no DI
	if (u <= 12) {
		RB(v) = m_data->read_byte(RL(u));
		RL(u)--;
	} else {
		RB(v) = m_data->read_byte(RA(u));
		RA(u)--;
	}
}

inline void puce_device::op_mbip(u8 u, u8 v) {
  //B%d := [M%d++]
  //no DI
	if (u <= 12) {
		RB(v) = m_data->read_byte(RL(u));
		RL(u)++;
	} else {
		RB(v) = m_data->read_byte(RA(u));
		RA(u)++;
	}
}

inline void puce_device::op_mli(u8 u, u8 v) {
  //L%d := [M%d]
  //no DI
	if (u <= 12) {
		RL(v) = m_program->read_word(RL(u));
	} else {
		RL(v) = m_program->read_word(RA(u));
	}
  op_illegal(NULL);
}

inline void puce_device::op_mlim(u8 u, u8 v) {
  //L%d := [M%d--]
  //no DI
	if (u <= 12) {
		RL(v) = m_program->read_word(RL(u));
		RL(u)--;
	} else {
		RL(v) = m_program->read_word(RA(u));
		RA(u)--;
	}
  op_illegal(NULL);
}

inline void puce_device::op_mlip(u8 u, u8 v) {
  //L%d := [M%d++]
  //no DI
	if (u <= 12) {
		RL(v) = m_program->read_word(RL(u));
		RL(u)++;
	} else {
		RL(v) = m_program->read_word(RA(u));
		RA(u)++;
	}
}

// ---- flags

inline void puce_device::op_redi(u8 t) {
  //reset DI 0x%02x
	m_di &= ~t;
  op_illegal(NULL);
}

inline void puce_device::op_sedi(u8 t) {
  //set DI 0x%02x
	m_di |= t;
  op_illegal(NULL);
}

// ---- arithmetic and logic

inline void puce_device::op_add(u8 u, u8 v) {
  //A%d + B%d + DI0
  //DI0,1,2 = CZH
	PAIR16 tmp;
	tmp.w = (m_di & 1) + RA(u) + RB(v);
	m_di &= 0xf8;
	m_di |= tmp.b.h;
	m_di |= (tmp.b.l == 0) ? 2 : 0;
	// TODO half-carry
  op_illegal(NULL);
}

inline void puce_device::op_adda(u8 u, u8 v) {
  //A%d := A%d + B%d + DI0
  //DI0,1,2 = CZH
	PAIR16 tmp;
	tmp.w = (m_di & 1) + RA(u) + RB(v);
	m_di &= 0xf8;
	m_di |= tmp.b.h;
	m_di |= (tmp.b.l == 0) ? 2 : 0;
	RA(u) = tmp.b.l;
	// TODO half-carry
  op_illegal(NULL);
}

inline void puce_device::op_addb(u8 u, u8 v) {
  //B%d := A%d + B%d + DI0
  //DI0,1,2 = CZH
	PAIR16 tmp;
	tmp.w = (m_di & 1) + RA(u) + RB(v);
	m_di &= 0xf8;
	m_di |= tmp.b.h;
	m_di |= (tmp.b.l == 0) ? 2 : 0;
	RB(v) = tmp.b.l;
  op_illegal(NULL);
}

inline void puce_device::op_sot(u8 u, u8 v) {
  //A%d - B%d + DI0
  //DI0,1,2 = CZH
	PAIR16 tmp;
	// TODO verify one-complement/twoc-omplement
	tmp.w = (m_di & 1) + RA(u) + (0xff ^ RB(v));
	m_di &= 0xf8;
	m_di |= tmp.b.h;
	m_di |= (tmp.b.l == 0) ? 2 : 0;
  op_illegal(NULL);
}

inline void puce_device::op_sota(u8 u, u8 v) {
  //A%d := A%d - B%d + DI0
  //DI0,1,2 = CZH
	PAIR16 tmp;
	// TODO verify one-complement/twoc-omplement
	tmp.w = (m_di & 1) + RA(u) + (0xff ^ RB(v));
	m_di &= 0xf8;
	m_di |= tmp.b.h;
	m_di |= (tmp.b.l == 0) ? 2 : 0;
	RA(u) = tmp.b.l;
  op_illegal(NULL);
}

inline void puce_device::op_sotb(u8 u, u8 v) {
  //B%d := A%d - B%d + DI0
  //DI0,1,2 = CZH
	PAIR16 tmp;
	// TODO verify one-complement/twoc-omplement
	tmp.w = (m_di & 1) + RA(u) + (0xff ^ RB(v));
	m_di &= 0xf8;
	m_di |= tmp.b.h;
	m_di |= (tmp.b.l == 0) ? 2 : 0;
	RB(v) = tmp.b.l;
  op_illegal(NULL);
}

inline void puce_device::op_and(u8 u, u8 v) {
  //A%d and B%d
  //DI1 = zero
	u8 tmp = RA(u) & RB(v);
	DIZERO(tmp);
}

inline void puce_device::op_anda(u8 u, u8 v) {
  //A%d := A%d and B%d
  //DI1 = zero
	u8 tmp = RA(u) = RA(u) & RB(v);
	DIZERO(tmp);
}

inline void puce_device::op_andb(u8 u, u8 v) {
  //B%d := A%d and B%d
  //DI1 = zero
	u8 tmp = RB(u) = RA(u) & RB(v);
	DIZERO(tmp);
}

inline void puce_device::op_or(u8 u, u8 v) {
  //A%d or B%d
  //DI1 = zero
	u8 tmp = RA(u) | RB(v);
	DIZERO(tmp);
}

inline void puce_device::op_ora(u8 u, u8 v) {
  //A%d := A%d or B%d
  //DI1 = zero
	u8 tmp = RA(u) = RA(u) | RB(v);
	DIZERO(tmp);
}

inline void puce_device::op_orb(u8 u, u8 v) {
  //B%d := A%d or B%d
  //DI1 = zero
	u8 tmp = RB(u) = RA(u) | RB(v);
	DIZERO(tmp);
}

inline void puce_device::op_ore(u8 u, u8 v) {
  //A%d xor B%d
  //DI1 = zero
	u8 tmp = RA(u) ^ RB(v);
	DIZERO(tmp);
}

inline void puce_device::op_orea(u8 u, u8 v) {
  //A%d := A%d xor B%d
  //DI1 = zero
	u8 tmp = RA(u) = RA(u) ^ RB(v);
	DIZERO(tmp);
}

inline void puce_device::op_oreb(u8 u, u8 v) {
  //B%d := A%d xor B%d
  //DI1 = zero
	u8 tmp = RB(u) = RA(u) ^ RB(v);
	DIZERO(tmp);
}

inline void puce_device::op_dca(u8 u) {
  //A%d--
  //DI1 = zero
	u8 tmp = RA(u) = RA(u) - 1;
	DIZERO(tmp);
}

inline void puce_device::op_dcb(u8 u) {
  //B%d--
  //DI1 = zero
	u8 tmp = RB(u) = RB(u) - 1;
	DIZERO(tmp);
}

inline void puce_device::op_dcl(u8 u) {
  //L%d--
  //DI1 = zero
	u16 tmp = RL(u) = RL(u) - 1;
	DIZERO(tmp);
}

inline void puce_device::op_ica(u8 u) {
  //A%d++
  //no DI
	RA(u)++;
}

inline void puce_device::op_icb(u8 u) {
  //B%d++
  //no DI
	RB(u)++;
}

inline void puce_device::op_icd(u8 g, u8 f, u8 v) {
  //L%d++ if D%x={%d}
  //no DI
	if (BIT(m_di,f) == g) {
		RL(v)++;
	}
}

inline void puce_device::op_icl(u8 u) {
  //L%d++
  //no DI
	RL(u)++;
}

inline void puce_device::op_vra(u8 u) {
  //A%d==0
  //DI1 = zero
	DIZERO(RA(u));
}

inline void puce_device::op_vrb(u8 u) {
  //B%d==0
  //DI1 = zero
	DIZERO(RB(u));
}

inline void puce_device::op_vrl(u8 u) {
  //L%d==0
  //DI1 = zero
	DIZERO(RL(u));
}

// ---- exchange

inline void puce_device::op_sab(u8 u, u8 v) {
  //A%d <-> B%d
  //no DI
	u8 tmp = RA(u); RA(u) = RB(v); RB(v) = tmp;
}

inline void puce_device::op_sll(u8 u, u8 v) {
  //L%d <-> L%d
  //no DI
	u16 tmp = RL(u); RL(u) = RL(v); RL(v) = tmp;
}

inline void puce_device::op_sdia(u8 u) {
  //A%d <-> DI
  //DI set
	u8 tmp = m_di; m_di = RA(u); RA(u) = tmp;
}

inline void puce_device::op_sdib(u8 u) {
  //B%d <-> DI
  //DI set
	u8 tmp = m_di; m_di = RB(u); RB(u) = tmp;
  op_illegal(NULL);
}

// ---- transfer

inline void puce_device::op_tadi(u8 u) {
  //DI := A%d
  //DI set
	m_di = RA(u);
}

inline void puce_device::op_tbdi(u8 u) {
  //DI := B%d
  //DI set
	m_di = RB(u);
}

inline void puce_device::op_tdia(u8 u) {
  //A%d := DI
  //no DI
  RA(u) = m_di;
}

inline void puce_device::op_tdib(u8 u) {
  //B%d := DI
  //no DI
  RB(u) = m_di;
}

inline void puce_device::op_tab(u8 u, u8 v) {
  //B%d := A%d
  //no DI
	RB(v) = RA(u);
}

inline void puce_device::op_tabp(u8 u, u8 v) {
  //B%d.P := A%d.P
  //no DI
	SETP(RB(v),GETP(RA(u)));
}

inline void puce_device::op_tabm(u8 u, u8 v) {
  //B%d.M := A%d.M
  //no DI
	SETM(RB(v),GETM(RA(u)));
}

inline void puce_device::op_tba(u8 u, u8 v) {
  //B%d := A%d
  //no DI
	RA(u) = RB(v);
}

inline void puce_device::op_tbap(u8 u, u8 v) {
  //A%d.P := B%d.P
  //no DI
	SETP(RA(u),GETP(RB(v)));
}

inline void puce_device::op_tbam(u8 u, u8 v) {
  //A%d.M := B%d.M
  //no DI
	SETM(RA(u),GETM(RB(v)));
}

inline void puce_device::op_azam(u8 u) {
  //A%d.M := 0
  //no DI
	SETM(RA(u),0);
}

inline void puce_device::op_azbm(u8 u) {
  //B%d.M := 0
  //no DI
	SETM(RB(u),0);
}

inline void puce_device::op_azap(u8 u) {
  //A%d.P := 0
  //no DI
	SETP(RA(u),0);
}

inline void puce_device::op_azbp(u8 u) {
  //B%d.P := 0
  //no DI
	SETP(RB(u),0);
}

// -------- swap halfbytes

inline void puce_device::op_rota(u8 u) {
  //A%d.M <-> A%d.P
  //no DI
	u8 tmp = RA(u);
	RA(u) = ((tmp & 0xf) << 4) | (tmp >> 4);
}

inline void puce_device::op_rotb(u8 u) {
  //B%d.M <-> B%d.P
  //no DI
	u8 tmp = RB(u);
	RB(u) = ((tmp & 0xf) << 4) | (tmp >> 4);
}

// -------- shift

inline void puce_device::op_shda(u8 u) {
  //A%d, DI0 := 0 >> A%d
  //DI0
	m_di &= 0xfe;
	m_di |= RA(u);
	RA(u) = RA(u) >> 1;
}

inline void puce_device::op_shdb(u8 u) {
  //B%d, DI0 := 0 >> B%d
  //DI0
	m_di &= 0xfe;
	m_di |= RB(u);
	RB(u) = RB(u) >> 1;
}

inline void puce_device::op_slda(u8 u) {
  //A%d, DI0 := DI0 >> A%d
  //DI0
	PAIR16 tmp;
	tmp.b.l = RA(u);
	tmp.b.h = m_di;
	m_di &= 0xfe;
	m_di |= RA(u);
	RA(u) = (tmp.w >> 1) & 0xff;
}

inline void puce_device::op_sldb(u8 u) {
  //B%d, DI0 := DI0 >> B%d
  //DI0
	PAIR16 tmp;
	tmp.b.l = RB(u);
	tmp.b.h = m_di;
	m_di &= 0xfe;
	m_di |= RB(u);
	RB(u) = (tmp.w >> 1) & 0xff;
}

inline void puce_device::op_shsa(u8 u) {
  //DI0, A%d := A%d << 0
  //DI0
	PAIR16 tmp;
	tmp.w = RA(u) << 1;
	m_di &= 0xfe;
	m_di |= tmp.b.h;
	RA(u) = tmp.b.l;
}

inline void puce_device::op_shsb(u8 u) {
  //DI0, B%d := B%d << 0
  //DI0
	PAIR16 tmp;
	tmp.w = RB(u) << 1;
	m_di &= 0xfe;
	m_di |= tmp.b.h;
	RB(u) = tmp.b.l;
}

inline void puce_device::op_slsa(u8 u) {
  //DI0, A%d := A%d << DI0
  //DI0
	PAIR16 tmp;
	tmp.w = (RA(u) << 1) | (m_di & 1);
	m_di &= 0xfe;
	m_di |= tmp.b.h;
	RA(u) = tmp.b.l;
}

inline void puce_device::op_slsb(u8 u) {
  //DI0, B%d := B%d << DI0
  //DI0
	PAIR16 tmp;
	tmp.w = (RB(u) << 1) | (m_di & 1);
	m_di &= 0xfe;
	m_di |= tmp.b.h;
	RB(u) = tmp.b.l;
}

// -------- external periphery

inline void puce_device::op_comx(u8 u) {
  //  C%d
  //no DI
	switch(u) {
	case 0:
		m_lvl = 4;
		break;
	case 1:
		// TODO: what if level is 1 or 2? Stay in level?
		// TODO: wait?
		m_lvl = 3;
		break;
	default:
	  op_illegal(NULL);
	}
}

// ======== -------- not implemented -------- ========

inline void puce_device::op_emi(u8 u) {
  //[M%d] <- data.A
  //no DI        
  op_illegal(NULL);
}

inline void puce_device::op_mei(u8 u) {
  //data.A <- [M%d]
  //no DI        
  op_illegal(NULL);
}

inline void puce_device::op_meip(u8 u) {
  //data.A <- [M%d++]
  //no DI        
  op_illegal(NULL);
}

inline void puce_device::op_meim(u8 u) {
  //data.A <- [M%d--]
  //no DI        
  op_illegal(NULL);
}

inline void puce_device::op_emim(u8 u) {
  //[M%d--] <- data.A
  //no DI        
  op_illegal(NULL);
}

inline void puce_device::op_emip(u8 u) {
  //[M%d++] <- data.A
  //no DI        
  op_illegal(NULL);
}

inline void puce_device::op_edb(u8 u) {
  //B%d <- data.A
  //no DI        
  op_illegal(NULL);
}

inline void puce_device::op_entl(u8 u) {
  //A%d <- name, B%d <- type
  //no DI        
  op_illegal(NULL);
}

inline void puce_device::op_edc(u8 u) {
  //L%d.MMM--, ECOF if zero
  //no DI        
  op_illegal(NULL);
}

inline void puce_device::op_ese(u8 u) {
  //sel <- [M%d]
  //no DI        
  op_illegal(NULL);
}

inline void puce_device::op_etib(u8 u) {
  //B%d <- type
  //no DI        
  op_illegal(NULL);
}

inline void puce_device::op_eco(u8 u) {
  //cmd <- [M%d]
  //no DI        
  op_illegal(NULL);
}

inline void puce_device::op_eda(u8 u) {
  //A%d <- data.A
  //no DI        
  op_illegal(NULL);
}

inline void puce_device::op_enua(u8 u) {
  //A%d <- name
  //no DI        
  op_illegal(NULL);
}

inline void puce_device::op_tcca(u8 u) {
  //A%d <- con
  //no DI        
  op_illegal(NULL);
}

inline void puce_device::op_tdma(u8 u) {
  //A%d <- con.M
  //no DI        
  op_illegal(NULL);
}

inline void puce_device::op_esi(u8 u) {
  //[M%d] <- data/type
  //no DI        
  op_illegal(NULL);
}

inline void puce_device::op_tdpa(u8 u) {
  //A%d <- con.P
  //no DI        
  op_illegal(NULL);
}

inline void puce_device::op_esip(u8 u) {
  //[M%d++] <- data/type
  //no DI        
  op_illegal(NULL);
}

inline void puce_device::op_esim(u8 u) {
  //[M%d--] <- data/type
  //no DI        
  op_illegal(NULL);
}

inline void puce_device::op_sei(u8 u) {
  //data.BA <- [%Md]
  //no DI        
  op_illegal(NULL);
}

inline void puce_device::op_seip(u8 u) {
  //data.BA <- [{M%d++]
  //no DI        
  op_illegal(NULL);
}

inline void puce_device::op_tabc(u8 u, u8 v) {
  //con <- A%d,B%d
  //no DI        
  op_illegal(NULL);
}

inline void puce_device::op_dea(u8 u) {
  //data.B <- B%d, A%d <- data.A
  //no DI        
  op_illegal(NULL);
}

inline void puce_device::op_dae(u8 u) {
  //data.BA <- L%d
  //no DI        
  op_illegal(NULL);
}

inline void puce_device::op_cae(u8 u) {
  //cmd.BA <- L%d
  //no DI        
  op_illegal(NULL);
}

inline void puce_device::op_seim(u8 u) {
  //data.BA <- [M%d--]
  //no DI        
  op_illegal(NULL);
}

// copy and paste from pucemake.py

inline void
puce_device::decode(u16 pc, u16 opcode)
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
		u16 k = (pc & 0xff00) | t;
		op_sade(k);
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
		u16 k = (pc & 0xff00) | t;
		op_sadx(e, d, k);
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
		case 0x82:
		{
			op_amim(u, v);
			break;
		}
		case 0x83:
			switch (v)
			{
			case 0xf:
			{
				op_tadi(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0x85:
			switch (v)
			{
			case 0xf:
			{
				op_ica(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0x86:
		{
			op_add(u, v);
			break;
		}
		case 0x87:
		{
			op_orb(u, v);
			break;
		}
		case 0x88:
		{
			op_amip(u, v);
			break;
		}
		case 0x89:
		{
			op_bmi(u, v);
			break;
		}
		case 0x8a:
		{
			op_bmim(u, v);
			break;
		}
		case 0x8b:
			switch (v)
			{
			case 0xf:
			{
				op_rota(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0x8c:
		{
			op_bmip(u, v);
			break;
		}
		case 0x8d:
			switch (v)
			{
			case 0x8:
			{
				op_emi(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0x8e:
			switch (v)
			{
			case 0xf:
			{
				op_vra(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0x90:
			switch (v)
			{
			case 0x0:
			{
				op_mei(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0x91:
		{
			op_mai(u, v);
			break;
		}
		case 0x92:
		{
			op_maim(u, v);
			break;
		}
		case 0x93:
			switch (v)
			{
			case 0xf:
			{
				op_tbdi(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0x94:
			switch (v)
			{
			case 0x0:
			{
				op_meip(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0x95:
			switch (v)
			{
			case 0xf:
			{
				op_icb(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0x96:
		{
			op_adda(u, v);
			break;
		}
		case 0x97:
		{
			op_and(u, v);
			break;
		}
		case 0x98:
		{
			op_maip(u, v);
			break;
		}
		case 0x99:
		{
			op_mbi(u, v);
			break;
		}
		case 0x9a:
		{
			op_mbim(u, v);
			break;
		}
		case 0x9b:
			switch (v)
			{
			case 0xf:
			{
				op_rotb(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0x9c:
		{
			op_mbip(u, v);
			break;
		}
		case 0x9d:
			switch (v)
			{
			case 0x0:
			{
				op_meim(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0x9e:
			switch (v)
			{
			case 0xf:
			{
				op_vrb(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xa0:
		{
			u8 g = BIT(u, 0);
			u8 f = BIT(u, 1, 3);
			op_icd(g, f, v);
			break;
		}
		case 0xa1:
			switch (v)
			{
			case 0x8:
			{
				op_emim(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xa2:
			switch (v)
			{
			case 0x8:
			{
				op_emip(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xa3:
			switch (v)
			{
			case 0xf:
			{
				op_sdia(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xa5:
			switch (v)
			{
			case 0xf:
			{
				op_icl(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xa6:
		{
			op_addb(u, v);
			break;
		}
		case 0xa7:
		{
			op_anda(u, v);
			break;
		}
		case 0xa8:
		{
			op_ami(u, v);
			break;
		}
		case 0xa9:
			switch (v)
			{
			case 0x8:
			{
				op_edb(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xaa:
			switch (v)
			{
			case 0x0:
			{
				op_entl(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xab:
			switch (v)
			{
			case 0xf:
			{
				op_azam(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xad:
			switch (v)
			{
			case 0xf:
			{
				op_edc(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xae:
			switch (v)
			{
			case 0xf:
			{
				op_dca(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xb1:
			switch (v)
			{
			case 0x4:
			{
				op_ese(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xb2:
			switch (v)
			{
			case 0xf:
			{
				op_etib(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xb3:
			switch (v)
			{
			case 0xf:
			{
				op_sdib(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xb4:
			switch (v)
			{
			case 0x2:
			{
				op_eco(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xb6:
		{
			op_sot(u, v);
			break;
		}
		case 0xb7:
		{
			op_andb(u, v);
			break;
		}
		case 0xb8:
			switch (v)
			{
			case 0x8:
			{
				op_eda(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xb9:
			switch (v)
			{
			case 0x0:
			{
				op_enua(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xba:
		{
			op_sab(u, v);
			break;
		}
		case 0xbb:
			switch (v)
			{
			case 0xf:
			{
				op_azap(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xbc:
		{
			op_sll(u, v);
			break;
		}
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
			}
			break;
		case 0xbe:
			switch (v)
			{
			case 0xf:
			{
				op_dcb(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xc3:
			switch (v)
			{
			case 0x0:
			{
				op_shda(u);
				break;
			}
			case 0x1:
			{
				op_slda(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xc4:
			switch (v)
			{
			case 0x0:
			{
				op_shsa(u);
				break;
			}
			case 0x1:
			{
				op_slsa(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xc5:
			switch (v)
			{
			case 0xf:
			{
				op_tdia(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xc6:
		{
			op_sota(u, v);
			break;
		}
		case 0xc7:
		{
			op_ore(u, v);
			break;
		}
		case 0xc8:
		{
			op_redi(t);
			break;
		}
		case 0xc9:
		{
			op_sedi(t);
			break;
		}
		case 0xca:
			switch (v)
			{
			case 0x0:
			{
				op_tcca(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xcb:
			switch (v)
			{
			case 0xf:
			{
				op_azbm(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xd1:
		{
			op_mli(u, v);
			break;
		}
		case 0xd3:
			switch (v)
			{
			case 0x0:
			{
				op_shdb(u);
				break;
			}
			case 0x1:
			{
				op_sldb(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xd4:
			switch (v)
			{
			case 0x0:
			{
				op_shsb(u);
				break;
			}
			case 0x1:
			{
				op_slsb(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xd5:
			switch (v)
			{
			case 0xf:
			{
				op_tdib(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xd6:
		{
			op_sotb(u, v);
			break;
		}
		case 0xd7:
		{
			op_orea(u, v);
			break;
		}
		case 0xd8:
		{
			op_tab(u, v);
			break;
		}
		case 0xd9:
		{
			op_tabp(u, v);
			break;
		}
		case 0xda:
			switch (v)
			{
			case 0x1:
			{
				op_tdma(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xdb:
			switch (v)
			{
			case 0xf:
			{
				op_azbp(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xdd:
		{
			op_mlim(u, v);
			break;
		}
		case 0xde:
		{
			op_mlip(u, v);
			break;
		}
		case 0xe0:
			switch (v)
			{
			case 0x8:
			{
				op_esi(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xe1:
		{
			op_lmi(u, v);
			break;
		}
		case 0xe2:
		{
			op_lpmip(u, v);
			break;
		}
		case 0xe5:
			switch (v)
			{
			case 0xf:
			{
				op_dcl(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xe6:
		{
			op_or(u, v);
			break;
		}
		case 0xe7:
		{
			op_oreb(u, v);
			break;
		}
		case 0xe8:
		{
			op_tabm(u, v);
			break;
		}
		case 0xe9:
		{
			op_tba(u, v);
			break;
		}
		case 0xea:
			switch (v)
			{
			case 0x2:
			{
				op_tdpa(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xeb:
			switch (v)
			{
			case 0x8:
			{
				op_esip(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xec:
			switch (v)
			{
			case 0x8:
			{
				op_esim(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xed:
		{
			op_lmim(u, v);
			break;
		}
		case 0xee:
		{
			op_lmip(u, v);
			break;
		}
		case 0xf1:
			switch (v)
			{
			case 0x0:
			{
				op_sei(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xf5:
			switch (v)
			{
			case 0xf:
			{
				op_vrl(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xf6:
		{
			op_ora(u, v);
			break;
		}
		case 0xf7:
			switch (v)
			{
			case 0x0:
			{
				op_seip(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xf8:
		{
			op_tbap(u, v);
			break;
		}
		case 0xf9:
		{
			op_tbam(u, v);
			break;
		}
		case 0xfa:
		{
			op_tabc(u, v);
			break;
		}
		case 0xfb:
			switch (v)
			{
			case 0x8:
			{
				op_dea(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xfc:
			switch (v)
			{
			case 0x0:
			{
				op_dae(u);
				break;
			}
			case 0x2:
			{
				op_cae(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		case 0xfd:
			switch (v)
			{
			case 0x0:
			{
				op_seim(u);
				break;
			}
			default:
				op_illegal(opcode);
			}
			break;
		default:
			op_illegal(opcode);
		}
	}
}
