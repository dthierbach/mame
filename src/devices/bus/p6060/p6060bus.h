// license:BSD-3-Clause
// copyright-holders:Dirk Thierbach
/***************************************************************************

  Olivetti P6060 card bus

  Called EXT in some diagrams.

  3 interrupt levels, 4 interrupr priorities

  16-bit data to periphery
  8-bit data from periphery
  8-bit name and 8-bit type from periphery

***************************************************************************/

#ifndef MAME_BUS_P6060_P6060BUS_H
#define MAME_BUS_P6060_P6060BUS_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class p6060bus_device;
class device_p6060bus_card_interface;

class p6060bus_slot_device : public device_t, public device_single_card_slot_interface<device_p6060bus_card_interface>
{
public:
	// construction/destruction
	template <typename T, typename U>
	p6060bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&p6060bus_tag, U &&opts, const char *dflt)
		: p6060bus_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false); // TODO we have some fixed cards...
		m_p6060bus.set_tag(std::forward<T>(p6060bus_tag));
	}
	p6060bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	p6060bus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// configuration
	required_device<p6060bus_device> m_p6060bus;
};

// device type definition
DECLARE_DEVICE_TYPE(p6060BUS_SLOT, p6060bus_slot_device)


// ======================> p6060bus_device
class p6060bus_device : public device_t
{
public:
	// construction/destruction
	p6060bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	template <typename T> void set_space(T &&tag, int spacenum) { m_space.set_tag(std::forward<T>(tag), spacenum); }
	auto out_irq_callback() { return m_out_irq_cb.bind(); }
	auto out_nmi_callback() { return m_out_nmi_cb.bind(); }

	void add_p6060bus_card(device_p6060bus_card_interface *card);
	device_p6060bus_card_interface *get_p6060bus_card();

	void set_irq_line(int state);
	void set_nmi_line(int state);

	void install_device(offs_t start, offs_t end, read8sm_delegate rhandler, write8sm_delegate whandler);
	void install_bank(offs_t start, offs_t end, uint8_t *data);

	void irq_w(int state);
	void nmi_w(int state);

protected:
	p6060bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// internal state
	required_address_space m_space;

	devcb_write_line    m_out_irq_cb;
	devcb_write_line    m_out_nmi_cb;

	device_p6060bus_card_interface *m_device;
};


// device type definition
DECLARE_DEVICE_TYPE(p6060BUS, p6060bus_device)

// ======================> device_p6060bus_card_interface

// class representing interface-specific live p6060bus card
class device_p6060bus_card_interface : public device_interface
{
	friend class p6060bus_device;
public:
	// construction/destruction
	virtual ~device_p6060bus_card_interface();

	// inline configuration
	void set_p6060bus(p6060bus_device *p6060bus, const char *slottag) { m_p6060bus = p6060bus; m_p6060bus_slottag = slottag; }
	template <typename T> void set_onboard(T &&p6060bus) { m_p6060bus_finder.set_tag(std::forward<T>(p6060bus)); m_p6060bus_slottag = device().tag(); }

protected:
	void raise_slot_irq() { m_p6060bus->set_irq_line(ASSERT_LINE); }
	void lower_slot_irq() { m_p6060bus->set_irq_line(CLEAR_LINE); }
	void raise_slot_nmi() { m_p6060bus->set_nmi_line(ASSERT_LINE); }
	void lower_slot_nmi() { m_p6060bus->set_nmi_line(CLEAR_LINE); }

	void install_device(offs_t start, offs_t end, read8sm_delegate rhandler, write8sm_delegate whandler);
	void install_bank(offs_t start, offs_t end, uint8_t *data);

	device_p6060bus_card_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_validity_check(validity_checker &valid) const override;
	virtual void interface_pre_start() override;

private:
	optional_device<p6060bus_device> m_p6060bus_finder;
	p6060bus_device *m_p6060bus;
	const char *m_p6060bus_slottag;
};

#endif  // MAME_BUS_P6060_P6060BUS_H
