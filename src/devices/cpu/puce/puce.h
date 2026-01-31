// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    First-gen DEC PDP-8 CPU emulator

    Written by Ryan Holtz
*/

#ifndef MAME_CPU_PUCE_PUCE_H
#define MAME_CPU_PUCE_PUCE_H

#pragma once

// register access. are there generic defines for byte access?
#define M_A(n) (m_l[n] & 0xff)
#define M_A(n) (m_l[n] & 0xff)

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

	// helpers
	static u16 get_a(u16 v) { return v & 0x00ff; }
	static u16 get_b(u16 v) { return v >> 8; }
	static void set_a(u16 &r, u8 v) { r = (r & 0xff00) | v; }
	static void set_b(u16 &r, u8 v) { r = (r & 0x00ff) | (v << 8); }

private:

	// internal processor state, in inverse size order

	u16 m_l[16]; // 16 bit L, 8 bit A and B
  u16 m_di; // flags
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
	PUCE_L8, PUCE_L9, PUCE_L10, PUCE_L11, PUCE_L12, PUCE_L13, PUCE_L14, PUCE_L15
};

#endif // MAME_CPU_puce_puce_H
