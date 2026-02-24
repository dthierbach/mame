// license:BSD-3-Clause
// copyright-holders:Dirk Thierbach
/*********************************************************************

	Olivetti P6060  FLODI  floppy controller card

	Cards floa and floab.

*********************************************************************/

#include "emu.h"
#include "flodi.h"

namespace {

class p6060bus_flodi_device:
		public device_t,
		public device_p6060bus_card_interface
{
public:
	// construction/destruction
	p6060bus_flodi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(P6060BUS_FLODI, device_p6060bus_card_interface, p6060bus_flodi_device, "flodi", "Olivetti P6060 FLODI floppy controller card")
