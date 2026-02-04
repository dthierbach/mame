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
	address_space_config m_data_config;

protected:
	// construction/destruction
	puce_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	void set_vpc_a(u16 a);
	void get_vpc();
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

	void op_sai(u16 j);
  void op_amd(u8 s, u8 t);
  void op_mad(u8 s, u8 t);
  void op_sade(u8 k);
  void op_sadx(u8 e, u8 d, u8 k);
  void op_crta(u8 s, u8 t);
  void op_crtb(u8 s, u8 t);

	void op_amim(u8 u, u8 v);
	void op_tadi(u8 u);
	void op_ica(u8 u);
	void op_add(u8 u, u8 v);
	void op_orb(u8 u, u8 v);
	void op_amip(u8 u, u8 v);
	void op_bmi(u8 u, u8 v);
	void op_bmim(u8 u, u8 v);
	void op_rota(u8 u);
	void op_bmip(u8 u, u8 v);
	void op_emi(u8 u);
	void op_vra(u8 u);
	void op_mei(u8 u);
	void op_mai(u8 u, u8 v);
	void op_main(u8 u, u8 v);
	void op_tbdi(u8 u);
	void op_meip(u8 u);
	void op_icb(u8 u);
	void op_adda(u8 u, u8 v);
	void op_and(u8 u, u8 v);
	void op_maip(u8 u, u8 v);
	void op_mbi(u8 u, u8 v);
	void op_mbim(u8 u, u8 v);
	void op_rotb(u8 u);
	void op_mbip(u8 u, u8 v);
	void op_meim(u8 u);
	void op_vrb(u8 u);
	void op_icd(u8 g, u8 f, u8 v);
	void op_emim(u8 u);
	void op_emip(u8 u);
	void op_sdia(u8 u);
	void op_icl(u8 u);
	void op_addb(u8 u, u8 v);
	void op_anda(u8 u, u8 v);
	void op_ami(u8 u, u8 v);
	void op_edb(u8 u);
	void op_entl(u8 u);
	void op_azam(u8 u);
	void op_edc(u8 u);
	void op_dca(u8 u);
	void op_ese(u8 u);
	void op_etib(u8 u);
	void op_sdib(u8 u);
	void op_eco(u8 u);
	void op_sot(u8 u, u8 v);
	void op_andb(u8 u, u8 v);
	void op_eda(u8 u);
	void op_enua(u8 u);
	void op_sab(u8 u);
	void op_azap(u8 u);
	void op_sll(u8 u, u8 v);
	void op_comx(u8 u);
	void op_dcb(u8 u);
	void op_shda(u8 u);
	void op_slda(u8 u);
	void op_shsa(u8 u);
	void op_slsa(u8 u);
	void op_tdia(u8 u);
	void op_sota(u8 u, u8 v);
	void op_ore(u8 u, u8 v);
	void op_redi(u8 t);
	void op_sedi(u8 t);
	void op_tcca(u8 u);
	void op_azbm(u8 u);
	void op_mli(u8 u, u8 v);
	void op_shdb(u8 u);
	void op_sldb(u8 u);
	void op_shsb(u8 u);
	void op_slsb(u8 u);
	void op_tdib(u8 u);
	void op_sotb(u8 u, u8 v);
	void op_orea(u8 u, u8 v);
	void op_tab(u8 u, u8 v);
	void op_tabp(u8 u, u8 v);
	void op_tdma(u8 u);
	void op_azbp(u8 u);
	void op_mlim(u8 u, u8 v);
	void op_mlip(u8 u, u8 v);
	void op_esi(u8 u);
	void op_lmi(u8 u, u8 v);
	void op_lpmip(u8 u, u8 v);
	void op_dcl(u8 u);
	void op_or(u8 u, u8 v);
	void op_oreb(u8 u, u8 v);
	void op_tabm(u8 u, u8 v);
	void op_tba(u8 u, u8 v);
	void op_tdpa(u8 u);
	void op_esip(u8 u);
	void op_esim(u8 u);
	void op_lmim(u8 u, u8 v);
	void op_lmip(u8 u, u8 v);
	void op_sei(u8 u);
	void op_vrl(u8 u);
	void op_ora(u8 u, u8 v);
	void op_seip(u8 u);
	void op_tbap(u8 u, u8 v);
	void op_tbam(u8 u, u8 v);
	void op_tabc(u8 u, u8 v);
	void op_dea(u8 u);
	void op_dae(u8 u);
	void op_cae(u8 u);
	void op_seim(u8 u);

	void op_illegal(u16 opcode);
	void decode(u16 pc, u16 opcode);

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
