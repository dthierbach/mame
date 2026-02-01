// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    First-gen DEC PDP-8 CPU emulator

    Written by Ryan Holtz
*/

#ifndef MAME_CPU_PUCE_PUCE_H
#define MAME_CPU_PUCE_PUCE_H

#pragma once

// ======================> puce_device

// Used by core CPU interface
class puce_device : public cpu_device
{
public:
	// construction/destruction
	puce_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override;
	virtual uint32_t execute_max_cycles() const noexcept override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// address spaces
	address_space_config m_program_config;

protected:
	// construction/destruction
	puce_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	void set_pc();
	void inc_vpc();

private:

	// internal processor state, in inverse size order

	PAIR16 m_reg[16]; // 16 bit L, 8 bit A and B
	u16 m_pc; // calculated at begin of execution
  u8 m_di; // flags
  u8 m_lvl; // 0..3

	// other internal states
	int m_icount;

	// address spaces
	address_space *m_program;
};

// device type definition
DECLARE_DEVICE_TYPE(PUCE, puce_device)

// register enumeration (for state)

enum
{
	PUCE_LVL = 1,
	PUCE_DI,
	PUCE_L0, PUCE_L1, PUCE_L2, PUCE_L3, PUCE_L4, PUCE_L5, PUCE_L6, PUCE_L7,
	PUCE_L8, PUCE_L9, PUCE_L10, PUCE_L11, PUCE_L12, PUCE_L13, PUCE_L14, PUCE_L15,
	PUCE_A0, PUCE_A1, PUCE_A2, PUCE_A3, PUCE_A4, PUCE_A5, PUCE_A6, PUCE_A7,
	PUCE_A8, PUCE_A9, PUCE_A10, PUCE_A11, PUCE_A12, PUCE_A13, PUCE_A14, PUCE_A15,
	PUCE_B0, PUCE_B1, PUCE_B2, PUCE_B3, PUCE_B4, PUCE_B5, PUCE_B6, PUCE_B7,
	PUCE_B8, PUCE_B9, PUCE_B10, PUCE_B11, PUCE_B12, PUCE_B13, PUCE_B14, PUCE_B15
};

#endif // MAME_CPU_puce_puce_H
