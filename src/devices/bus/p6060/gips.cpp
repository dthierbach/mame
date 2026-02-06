// license:BSD-3-Clause
// copyright-holders:Dirk Thierbach
/*********************************************************************

	Olivetti P6060 GIPS  IPSO parallel port controller card

*********************************************************************/

#include "emu.h"
#include "gips.h"

namespace {

class p6060bus_gips_device:
		public device_t,
		public device_p6060bus_card_interface
{
public:
	// construction/destruction
	p6060bus_gips_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;

private:
	std::unique_ptr<u8[]> m_ram;
};

p6060bus_gips_device::p6060bus_gips_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, P6060BUS_GIPS, tag, owner, clock)
	, device_p6060bus_card_interface(mconfig, *this)
{
}

void p6060bus_gips_device::device_start()
{
	m_ram = std::make_unique<u8[]>(0x4000);

	install_bank(0x2000, 0x5fff, &m_ram[0]);

	save_pointer(NAME(m_ram), 0x4000);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(P6060BUS_GIPS, device_p6060bus_card_interface, p6060bus_gips_device, "gips", "Olivetti P6060 GIPS IPSO parallel port controller card")
