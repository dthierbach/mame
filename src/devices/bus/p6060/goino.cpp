// license:BSD-3-Clause
// copyright-holders:Dirk Thierbach
/*********************************************************************

  Olivetti P6060 GOINO  Internal hardware controller

*********************************************************************/

#include "emu.h"
// #include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "machine/keyboard.h"
#include "sound/beep.h"

#include "goino.h"

#define VERBOSE (1)
#include "logmacro.h"

namespace {

#define DISPLAY_WIDTH 222
#define DISPLAY_HEIGHT 7

class p6060bus_goino_device:
		public device_t,
		public device_p6060bus_card_interface
{
public:
	// construction/destruction
	p6060bus_goino_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	bool strobe_ecos() override;
	void strobe_ecot() override;
	void strobe_ecoc() override;
	void lights_shiftin(int value);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	// virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(bell_off);

private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	// 	 u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<screen_device> m_screen;
	emu_timer *m_bell_timer;
	required_device<beep_device> m_beeper;
	output_finder<12> m_lamps;
	// generic_keyboard_device::output_delegate m_keyboard_cb;

	int m_lights_shift;
	u16 m_lights_buffer;
};

void p6060bus_goino_device::device_add_mconfig(machine_config &config)
{
	LOG("%s: device_add_mconfig\n");
	/*
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(p6060bus_goino_device::screen_update));
	m_screen->set_raw(8_MHz_XTAL, 512, 0, 320, 260, 0, 200);
	m_screen->set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);
	*/

	// screen
	// Burroughs SSD0132 Plasma Display, 222x7
	// 1 MHz update freq ???
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// pixclock, htotal, hbend, hbstart, vtotal, vbend, vbstart)
	m_screen->set_raw(XTAL(8'000'000)/2, DISPLAY_WIDTH, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0, DISPLAY_HEIGHT);
	// m_screen->set_raw(1021800*14, (65*7)*2, 0, (40*7)*2, 262, 0, 192);
	// m_screen->set_color(rgb_t::amber());
	// m_screen->set_palette(m_video);
	m_screen->set_no_palette();
	m_screen->set_screen_update(FUNC(p6060bus_goino_device::screen_update));

	SPEAKER(config, "bell").front_center();
	BEEP(config, m_beeper, 1'200); // Condy p.14: 1200 Hz
	m_beeper->add_route(ALL_OUTPUTS, "bell", 0.25);

}

/*
ioport_constructor p6060bus_goino_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(dips);
}
*/

p6060bus_goino_device::p6060bus_goino_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, P6060BUS_GOINO, tag, owner, clock)
	, device_p6060bus_card_interface(mconfig, *this)
	, m_screen(*this, "screen")
	, m_bell_timer(nullptr)
	, m_beeper(*this, "beeper")
	, m_lamps(*this, "lamp%u", 1U)
{
}

void p6060bus_goino_device::device_start()
{
	LOG("%s: device_start\n", machine().describe_context());
	m_bell_timer = timer_alloc(FUNC(p6060bus_goino_device::bell_off), this);
	// creates new outputs here.
	// when layout is rendered later, state is overwritten with -1.
	m_lamps.resolve();
	// m_keyboard_cb.resolve_safe();
}

void p6060bus_goino_device::device_reset()
{
	LOG("%s: device_reset\n", machine().describe_context());
	m_beeper->set_state(0);
	m_lights_shift = 0;
	m_lights_buffer = 0;
}

TIMER_CALLBACK_MEMBER(p6060bus_goino_device::bell_off)
{
	m_beeper->set_state(0);
}

uint32_t p6060bus_goino_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
// u32 p6060bus_goino_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// LOG("%s: screen_update\n", machine().describe_context());
	/*
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
	*/
	// pen_t const pen = 0xf09090f0;
	// rgb_t pix(0x90, 0x90, 0xf0);
	for (int y = 0; y < DISPLAY_HEIGHT; y++)
	{
		for (int x = 0; x < DISPLAY_WIDTH; x++)
		{
			rgb_t pix;
			if ((x ^ y) & 1) {
				pix = rgb_t::amber();
			} else {
				pix = rgb_t::black();
			}
			bitmap.pix(y, x) = pix;
		}
	}
	return 0;
}

void p6060bus_goino_device::lights_shiftin(int value) {
	m_lights_shift++;
	m_lights_buffer = m_lights_buffer << 1 | (value & 1);
	if (m_lights_shift >= 16) {
		if (m_lights_buffer & 0x4000) {
			m_beeper->set_state(1);
			m_bell_timer->reset(attotime::from_msec(200)); // Condy p.14: 200ms
		}
		LOG("%s: >>> >>>> lights %04x\n", machine().describe_context(), m_lights_buffer);
		for (int i = 0; i < 12; i++) {
			m_lamps[i] = m_lights_shift & 1;
			m_lights_shift = m_lights_shift >> 1;
		}
		m_lights_shift = 0;
		m_lights_buffer = 0;
	}
}

bool p6060bus_goino_device::strobe_ecos() {
	LOG("%s: ecos\n", machine().describe_context());
	return true;
}

void p6060bus_goino_device::strobe_ecoc() {
	LOG("%s: ecoc\n", machine().describe_context());
}

// Condy says CAE, but ROMCA uses DAE
// if necessary, call from ecoc, too.
void p6060bus_goino_device::strobe_ecot() {
	LOG("%s: ecot\n", machine().describe_context());
	u16 data = m_p6060bus->get_ecd();
	switch (data &0xff00) {
		case 0x4000: // PULSN-NOPPO  strobe dati per consolle luminosa e cicalino
			lights_shiftin(data & 1);
			break;
		default:
			logerror("%s: illegal command %04x", machine().describe_context(), data);
	}
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(P6060BUS_GOINO, device_p6060bus_card_interface, p6060bus_goino_device, "goino", "Olivetti P6060 GOINO internal hardware controller card")
