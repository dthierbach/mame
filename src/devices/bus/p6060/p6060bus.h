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

#include "p6060intf.h"
#include "cpu/puce/puce.h"

#define P6060_MAXSLOT 13

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

	void update_irq_level();

	// configuration
	required_device<p6060bus_device> m_p6060bus;
};

// device type definition
DECLARE_DEVICE_TYPE(P6060BUS_SLOT, p6060bus_slot_device)


// ======================> p6060bus_device

class p6060bus_device : public device_t, public p6060bus_interface
{
public:
	// construction/destruction
	template<typename T>
	p6060bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag)
			: p6060bus_device(mconfig, tag, owner, clock)
	{
		set_cpu(std::forward<T>(cpu_tag));
	}
	p6060bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	template <typename T> void set_cpu(T &&tag) { m_maincpu.set_tag(std::forward<T>(tag)); }

	device_p6060bus_card_interface *get_p6060bus_card(int slot);
	void add_p6060bus_card(int slot, device_p6060bus_card_interface *card);

// ---- from CPU

	// command/data
	virtual void set_ecd(u16 data) override;
	virtual u16 get_ecd() override;

  // reset: all cards
	virtual void set_ecor(int level) override;

	// select: in priority order to all cards
	virtual bool strobe_ecos() override;

  // transmit/sync: selected card
	virtual void strobe_ecot() override;

  // command (includes ecot): selected card
	virtual void strobe_ecoc() override;

  // finish: selected card
	virtual void strobe_ecof() override;

  // signal 1: selected card
	virtual void set_ec1f(int level) override;

  // signal 2: selected card
	virtual void set_ec2f(int level) override;

	virtual void grant_irq(int level) override;

	// ---- from periphery

	// data/state
	virtual void set_epd(u8 data) override;
	virtual u8 get_epd() override;

	// name of periphery
	virtual void set_epn(u8 name) override;
	virtual u8 get_epn() override;

	// type of interrupt
	virtual void set_ept(u8 type) override;
	virtual u8 get_ept() override;

	virtual void request_irq(irq_priority::t priority, irq_mask_t mask) override;

protected:
	p6060bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// internal state
	required_device<puce_device> m_maincpu;
	device_p6060bus_card_interface *m_device_list[P6060_MAXSLOT+1];

	int m_select; // slot of selected card

	u16 m_ecd;
	u8 m_epd;
	u8 m_epn;
	u8 m_ept;
	// should ec1f and ec2f be state?

	irq_mask_t m_requests[irq_priority::LAST];

private:

  void update_irq_level();

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

	void set_irq_mask(irq_mask_t mask) { m_irq_mask = mask; }
	void set_dma_mask(irq_mask_t mask) { m_dma_mask = mask; }

	virtual void grant_irq(irq_priority::t priority) { }

	virtual void set_ecor(int level) { };
	virtual bool strobe_ecos() { return false; };
	virtual void strobe_ecot() { };
	virtual void strobe_ecoc() { };
	virtual void strobe_ecof() { };
	virtual void set_ec1f(int level) { };
	virtual void set_ec2f(int level) { };

protected:

	device_p6060bus_card_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_validity_check(validity_checker &valid) const override;
	virtual void interface_pre_start() override;

	optional_device<p6060bus_device> m_p6060bus_finder;
	p6060bus_device *m_p6060bus;
	const char *m_p6060bus_slottag;
	irq_mask_t m_irq_mask;
	dma_mask_t m_dma_mask;

private:

};

#endif  // MAME_BUS_P6060_P6060BUS_H
