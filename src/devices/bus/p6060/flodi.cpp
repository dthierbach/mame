// license:BSD-3-Clause
// copyright-holders:Dirk Thierbach
/*********************************************************************

	Olivetti P6060  FLODI  floppy controller card

	Cards floa and floab.

*********************************************************************/

#include "emu.h"
#include "flodi.h"

#define VERBOSE (1)
#include "logmacro.h"

namespace {

class p6060bus_flodi_device:
		public device_t,
		public device_p6060bus_card_interface
{
public:
	// construction/destruction
	p6060bus_flodi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	bool strobe_ecos() override;
	void strobe_ecot() override;
	void strobe_ecoc() override;
	void grant_irq(irq_priority::t priority) override;

	protected:
	virtual void device_start() override ATTR_COLD;

private:
};

p6060bus_flodi_device::p6060bus_flodi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, P6060BUS_FLODI, tag, owner, clock)
	, device_p6060bus_card_interface(mconfig, *this)
{
}

void p6060bus_flodi_device::device_start()
{
}

bool p6060bus_flodi_device::strobe_ecos() {
	// drive 0: 1001 1111 = 60
	// drive 1: 0001 1111 = e0
	if ((m_p6060bus->get_ecd() & 0x7f) == 0x60) {
		LOG("%s: ecos\n", machine().describe_context());
		// TODO save drive
		// request LEVEL3B
		m_p6060bus->request_irq(irq_priority::LEVEL3B, m_irq_mask);
		return true;
	}
	return false;
}

void p6060bus_flodi_device::strobe_ecoc() {
	LOG("%s: ecoc\n", machine().describe_context());
}

void p6060bus_flodi_device::strobe_ecot() {
	LOG("%s: ecot\n", machine().describe_context());
	// in state after select: do nothing, reads state
}

void p6060bus_flodi_device::grant_irq(irq_priority::t priority) {
	LOG("%s: grant_irq prio=%i\n", machine().describe_context(), priority);
	// C.* 1001 1111
	// D.* 1001 0111
	m_p6060bus->set_epn(0x60);
	switch (priority) {
		case irq_priority::LEVEL1:
			// 1111 111*  * = error
			m_p6060bus->set_ept(0x00);
			break;
		case irq_priority::LEVEL3A:
			// 1111 1011  end class C command
			// 1111 0111  controller function
			m_p6060bus->set_ept(0x08);
			break;
		case irq_priority::LEVEL3B:
			// 1111 1100  command response
			// 1111 1110  select response
			m_p6060bus->set_ept(0x01);
			break;
		default:
			break;
	}
	// selection:
	//    bit 1 = GOC0 = OR of via carello/testina abbassata/cambio disco
	//    bit 2 = GOC0
	//	  bit 3 = INOP = intervento operatore
	//	  bit 4 = LOCA = driver selezionato in locale
	//    bit 5 = MADI = machina disponibile
	//    bit 6 = PIZE = carello in traccia 0 = track 0
	//    bit 7 = DICE = disco non presento o sportello aperto = disk not present
	m_p6060bus->set_epd(0x00);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(P6060BUS_FLODI, device_p6060bus_card_interface, p6060bus_flodi_device, "flodi", "Olivetti P6060 FLODI floppy controller card")

/*

use state from ... upd765 or wd_fdc

DESELECTED -->   SELECTED ------>  INTR    ------>  CMD
            sel      L3B    L3B    n/t/s     CMD    L3B

*/