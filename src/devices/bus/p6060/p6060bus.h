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
DECLARE_DEVICE_TYPE(P6060BUS_SLOT, p6060bus_slot_device)


// ======================> p6060bus_device

class p6060bus_device : public device_t
{
public:
	// construction/destruction
	p6060bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void add_p6060bus_card(device_p6060bus_card_interface *card);
	device_p6060bus_card_interface *get_p6060bus_card();

	// ---- from CPU

	// command/data
	void set_ecd(u16 data);
	u16 get_ecd();

  // reset: all cards
	void set_ecor(int level);

	// select: in priority order to all cards
	bool strobe_ecos();

  // transmit/sync: selected card
	void strobe_ecot();

  // command (includes ecot): selected card
	void strobe_ecoc();

  // finish: selected card
	void strobe_ecof();

  // signal 1: selected card
	void set_ec1f(int level);

  // signal 2: selected card
	void set_ec2f(int level);

	// ---- from periphery

	// data/state
	void set_epd(u8 data);
	u8 get_epd();

	// name of periphery
	void set_epn(u8 name);
	u8 get_epn();

	// type of interrupt
	void set_ept(u8 type);
	u8 get_ept();

protected:
	p6060bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// internal state
	required_address_space m_space;

	device_p6060bus_card_interface *m_device;

	u16 ecd;
	u8 epd;
	u8 epn;
	u8 ept;
	// should ec1f and ec2f be state?
};


// device type definition
DECLARE_DEVICE_TYPE(P6060BUS, p6060bus_device)

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

	void set_ecor(int level);
	bool strobe_ecos();
	void strobe_ecot();
	void strobe_ecoc();
	void strobe_ecof();
	void set_ec1f(int level);
	void set_ec2f(int level);

	device_p6060bus_card_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_validity_check(validity_checker &valid) const override;
	virtual void interface_pre_start() override;

private:
	optional_device<p6060bus_device> m_p6060bus_finder;
	p6060bus_device *m_p6060bus;
	const char *m_p6060bus_slottag;
};

#endif  // MAME_BUS_P6060_P6060BUS_H
