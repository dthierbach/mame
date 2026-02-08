// license:BSD-3-Clause
// copyright-holders:Dirk Thierbach
/***************************************************************************

  Olivetti P6060 card bus

***************************************************************************/

#include "emu.h"
#include "p6060bus.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(P6060BUS_SLOT, p6060bus_slot_device, "p6060bus_slot", "P6060 Card Slot")

template class device_finder<device_p6060bus_card_interface, false>;
template class device_finder<device_p6060bus_card_interface, true>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  p6060bus_slot_device - constructor
//-------------------------------------------------
p6060bus_slot_device::p6060bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: p6060bus_slot_device(mconfig, P6060BUS_SLOT, tag, owner, clock)
{
}

p6060bus_slot_device::p6060bus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_single_card_slot_interface<device_p6060bus_card_interface>(mconfig, *this)
	, m_p6060bus(*this, finder_base::DUMMY_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void p6060bus_slot_device::device_resolve_objects()
{
	device_p6060bus_card_interface *const p6060bus_card(dynamic_cast<device_p6060bus_card_interface *>(get_card_device()));
	if (p6060bus_card)
		p6060bus_card->set_p6060bus(m_p6060bus, tag());
}

void p6060bus_slot_device::device_start()
{
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(P6060BUS, p6060bus_device, "p6060bus", "P6060 Card Bus")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  p6060bus_device - constructor
//-------------------------------------------------

p6060bus_device::p6060bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: p6060bus_device(mconfig, P6060BUS, tag, owner, clock)
{
}

p6060bus_device::p6060bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_space(*this, finder_base::DUMMY_TAG, -1)
	, m_device(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void p6060bus_device::device_start()
{
	// clear slot
	m_device = nullptr;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void p6060bus_device::device_reset()
{
}

device_p6060bus_card_interface *p6060bus_device::get_p6060bus_card()
{
	return m_device;
}

void p6060bus_device::add_p6060bus_card(device_p6060bus_card_interface *card)
{
	m_device = card;
}

device_p6060bus_card_interface::device_p6060bus_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "p6060bus")
	, m_p6060bus_finder(device, finder_base::DUMMY_TAG), m_p6060bus(nullptr)
	, m_p6060bus_slottag(nullptr)
{
}

device_p6060bus_card_interface::~device_p6060bus_card_interface()
{
}

void device_p6060bus_card_interface::interface_validity_check(validity_checker &valid) const
{
	if (m_p6060bus_finder && m_p6060bus && (m_p6060bus != m_p6060bus_finder))
		osd_printf_error("Contradictory buses configured (%s and %s)\n", m_p6060bus_finder->tag(), m_p6060bus->tag());
}

void device_p6060bus_card_interface::interface_pre_start()
{
	if (!m_p6060bus)
	{
		m_p6060bus = m_p6060bus_finder;
		if (!m_p6060bus)
			fatalerror("Can't find P6060 Bus device %s\n", m_p6060bus_finder.finder_tag());
	}

	if (!m_p6060bus->started())
		throw device_missing_dependencies();

	m_p6060bus->add_p6060bus_card(this);
}

/*

p6060bus:

EXT external bus

prio enum
  1 L1
	2 L2
	3 L3A
	4 L3B

state
  select controller
	from peri: data 8, name 8, type 8
	to peri: data/cmd 16

## CPU > Peri

reset
  all controllers

select
  all controllers in order
	stop at first
	save which

## CPU > selected peri:
# use signal line abstraction?


finish

data without ECOT

data with ECOT

command

EC1F
EC2F

## peri > CPU

store 

## Interrupt

intr_finish
  COM0 from CPU
	check queues in order
	found:
	   remove from queue
      grant

intr_request(prio)
  check CPU level
	if available, grant
	otherwise queue (for simplicity)

intr_grant(prio)
  set CPU level
  callback card

ECM1,2,3  CPU request irq ???

*/
