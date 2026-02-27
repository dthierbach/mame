// license:BSD-3-Clause
// copyright-holders:Dirk Thierbach
/***************************************************************************

  Olivetti P6060 card bus

***************************************************************************/

#include "emu.h"
#include "p6060bus.h"

#define VERBOSE (1)
#include "logmacro.h"

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

// skip 11 as second FLODI slot
static const unsigned irq_slot[] = { 1, 13, 12, 10, 9, 8, 2, 3, 4, 5, 6, 0 };

// dma_slot[]

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
	, m_maincpu(*this, finder_base::DUMMY_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void p6060bus_device::device_start()
{
	// clear slots
	std::fill(std::begin(m_device_list), std::end(m_device_list), nullptr);
	m_select = -1;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void p6060bus_device::device_reset()
{
	m_select = -1;
	m_requests[irq_priority::LEVEL1] = 0;
	m_requests[irq_priority::LEVEL2] = 0;
	m_requests[irq_priority::LEVEL3A] = 0;
	m_requests[irq_priority::LEVEL3B] = 0;
}

device_p6060bus_card_interface *p6060bus_device::get_p6060bus_card(int slot)
{
	if (slot < 1 || slot > P6060_MAXSLOT)
	{
		return nullptr;
	}
	if (m_device_list[slot])
	{
		return m_device_list[slot];
	}
	return nullptr;
}

void p6060bus_device::add_p6060bus_card(int slot, device_p6060bus_card_interface *card)
{
	m_device_list[slot] = card;
	irq_mask_t irq_mask = 0;
	dma_mask_t dma_mask = 0;
	// one time search, can be expensive
	for (unsigned p = 0; irq_slot[p] != 0; p++) {
		if (irq_slot[p] == slot) {
			irq_mask = 1 << p;
			break;
		}
	}
	LOG("%s: attach slot %d irq=%04x dma=%04x\n", machine().describe_context(), slot, irq_mask, dma_mask);
	card->set_irq_mask(irq_mask);
	card->set_dma_mask(dma_mask);
}

// ---- from CPU

void p6060bus_device::set_ecd(u16 data) {
	LOG("%s: ecd=%04x\n", machine().describe_context(), data);
	m_ecd = data;
}

u16 p6060bus_device:: get_ecd() {
	return m_ecd;
}

  // reset: all cards
void p6060bus_device::set_ecor(int level) {
	LOG("%s: ecor=%d reset\n", machine().describe_context(), level);
	if (level == ASSERT_LINE) {
		// not sure which initializations reset does...
		// but name is checked.
		m_ecd = 0;
		m_epd = 0;
		m_epn = 0;
		m_ept = 0;
	}
	for (int slot = 1; slot <= P6060_MAXSLOT; slot++)
	{
		auto card = get_p6060bus_card(slot);
		if (card != nullptr)
		{
			card->set_ecor(level);
		}
	}
}

	// select: in priority order to all cards
bool p6060bus_device::strobe_ecos() {
	LOG("%s: ecos select ecd=%04x\n", machine().describe_context(), m_ecd);
	// TODO do this in priority order
	for (int slot = P6060_MAXSLOT; slot >= 1; slot--)
	{
		auto card = get_p6060bus_card(slot);
		if (card != nullptr)
		{
			if (card->strobe_ecos()) {
				LOG("%s: slot %d responded\n", machine().describe_context(), slot);
				m_select = slot;
				return true;
			}
		}
	}
	m_select = -1;
	return false;
}

  // transmit/sync: selected card
void p6060bus_device::strobe_ecot() {
	LOG("%s: ecot transmit\n", machine().describe_context());
	if (m_select >= 0) {
		auto card = get_p6060bus_card(m_select);
		if (card != nullptr)
		{
			card->strobe_ecot();
		}
	}
}

  // command (includes ecot): selected card
void p6060bus_device::strobe_ecoc() {
	LOG("%s: ecoc command\n", machine().describe_context());
	if (m_select >= 0) {
		auto card = get_p6060bus_card(m_select);
		if (card != nullptr)
		{
			card->strobe_ecoc();
		}
	}
}

  // finish: selected card
void p6060bus_device::strobe_ecof() {
	LOG("%s: ecof finish\n", machine().describe_context());
	if (m_select >= 0) {
		auto card = get_p6060bus_card(m_select);
		if (card != nullptr)
		{
			card->strobe_ecof();
		}
	}

}

  // signal 1: selected card
void p6060bus_device::set_ec1f(int level) {
	LOG("%s: ec1f=%d\n", machine().describe_context(), level);
	if (m_select >= 0) {
		auto card = get_p6060bus_card(m_select);
		if (card != nullptr)
		{
			card->set_ec1f(level);
		}
	}
}

  // signal 2: selected card
void p6060bus_device::set_ec2f(int level) {
	LOG("%s: ec2f=%d\n", machine().describe_context(), level);
	if (m_select >= 0) {
		auto card = get_p6060bus_card(m_select);
		if (card != nullptr)
		{
			card->set_ec2f(level);
		}
	}
}

	// ---- from periphery

// data/state
void p6060bus_device::set_epd(u8 data) {
	LOG("%s: epd=%02x\n", machine().describe_context(), data);
	m_epd = data;
}

u8 p6060bus_device::get_epd() {
	return m_epd;
}

// name of periphery
void p6060bus_device::set_epn(u8 name) {
	LOG("%s: epn=%02x\n", machine().describe_context(), name);
	m_epn = name;
}

u8 p6060bus_device::get_epn() {
	return m_epn;
}

// type of interrupt
void p6060bus_device::set_ept(u8 type) {
	LOG("%s: ept=%02x\n", machine().describe_context(), type);
	m_ept = type;
}

u8 p6060bus_device::get_ept() {
	return m_ept;
}

void p6060bus_device::update_irq_level() {
	if (m_requests[irq_priority::LEVEL1] != 0) {
		m_maincpu->set_irq_level(1);
	} else if (m_requests[irq_priority::LEVEL2] != 0) {
		m_maincpu->set_irq_level(2);
	} else if (m_requests[irq_priority::LEVEL3A] != 0 || m_requests[irq_priority::LEVEL3B] != 0) {
		m_maincpu->set_irq_level(3);
	} else {
		m_maincpu->set_irq_level(4);
	}
}

// from periphery
void p6060bus_device::request_irq(irq_priority::t priority, irq_mask_t mask) {
	m_requests[priority] |= mask;
	update_irq_level();
}

// from CPU
void p6060bus_device::grant_irq(int level) {
	irq_priority::t priority;
	irq_order_t order;
	irq_mask_t mask;
	// could do consistency check for level
	for (unsigned p = irq_priority::FIRST; p != irq_priority::LAST; p++) {
		priority = static_cast<irq_priority::t>(p);
		if (m_requests[priority] != 0) break;
	}
	if (priority == irq_priority::LAST) {
		osd_printf_error("Granting IRQ level %d but request found\n", level);
		return;
	}
	mask = m_requests[priority];
	LOG("%s: grant mask=%04x\n", machine().describe_context(), mask);
	mask >>= 1;
	for (order = 0; mask != 0; order++, mask >>= 1);
	// check index
	unsigned slot = irq_slot[order];
	LOG("%s: grant order=%d slot=%d\n", machine().describe_context(), order, slot);
	m_requests[priority] &= ~(1 << order);
	update_irq_level();
	// check index
	device_p6060bus_card_interface* card = m_device_list[slot];
	// check card
	card->grant_irq(priority);
}

// --------

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

	int slot;
	if (std::sscanf(m_p6060bus_slottag, ":sl%d", &slot) < 1) {
		fatalerror("Slot tag %s has wrong format for P6060 Bus\n", m_p6060bus_slottag);
	}
	if (slot < 1 || slot > P6060_MAXSLOT) {
		fatalerror("Slot %x out of range for P6060 Bus\n", slot);
	}

	m_p6060bus->add_p6060bus_card(slot, this);
}
