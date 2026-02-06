// license:BSD-3-Clause
// copyright-holders:Dirk Thierbach
/*********************************************************************

  Olivetti P6060 GOINO  Internal hardware controller

*********************************************************************/

#include "emu.h"

#include "goino.h"

#include "emupal.h"
#include "screen.h"

namespace {

class p6060bus_goino_device:
		public device_t,
		public device_p6060bus_card_interface
{
public:
	// construction/destruction
	p6060bus_goino_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_ioport m_dips;
	required_device<screen_device> m_screen;
	std::unique_ptr<u8[]> m_ram;
	bool m_installed;
};

void p6060bus_goino_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(p6060bus_goino_device::screen_update));
	m_screen->set_raw(8_MHz_XTAL, 512, 0, 320, 260, 0, 200);
	m_screen->set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);
}

static INPUT_PORTS_START( dips )
	PORT_START("DIPS")
	PORT_DIPNAME( 0xe000, 0x6000, "Address Range" )
	PORT_DIPSETTING(  0x2000, "2000-3FFF" )
	PORT_DIPSETTING(  0x4000, "4000-5FFF" )
	PORT_DIPSETTING(  0x6000, "6000-7FFF" )
	PORT_DIPSETTING(  0x8000, "8000-9FFF" )
	PORT_DIPSETTING(  0xa000, "A000-BFFF" )
	PORT_DIPSETTING(  0xc000, "C000-DFFF" )
INPUT_PORTS_END

ioport_constructor p6060bus_goino_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(dips);
}

p6060bus_goino_device::p6060bus_goino_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, P6060BUS_GOINO, tag, owner, clock)
	, device_p6060bus_card_interface(mconfig, *this)
	, m_dips(*this, "DIPS")
	, m_screen(*this, "screen")
	, m_installed(false)
{
}

void p6060bus_goino_device::device_start()
{
	m_ram = std::make_unique<u8[]>(0x2000);

	save_pointer(NAME(m_ram), 0x2000);
}

void p6060bus_goino_device::device_reset()
{
	if (!m_installed)
	{
		const u16 base = m_dips->read();
		install_bank(base, base + 0x1fff, &m_ram[0]);
		m_installed = true;
	}
}

u32 p6060bus_goino_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	auto const vram8 = &m_ram[0];

	for (int y = 0; y < 200; y++)
	{
		u16 *scanline = &bitmap.pix(y);
		for (int x = 0; x < 320 / 8; x++)
		{
			u8 const pixels = vram8[(y * 40) + x];

			*scanline++ = BIT(pixels, 7);
			*scanline++ = BIT(pixels, 6);
			*scanline++ = BIT(pixels, 5);
			*scanline++ = BIT(pixels, 4);
			*scanline++ = BIT(pixels, 3);
			*scanline++ = BIT(pixels, 2);
			*scanline++ = BIT(pixels, 1);
			*scanline++ = BIT(pixels, 0);
		}
	}
	return 0;
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(P6060BUS_GOINO, device_p6060bus_card_interface, p6060bus_goino_device, "goino", "Olivetti P6060 GOINO internal hardware controller card")
