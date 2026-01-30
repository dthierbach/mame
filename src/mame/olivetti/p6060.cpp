// license:GPL-2.0+
// copyright-holders:Dirk Thierbach
/******************************************************************************

Olivetti P6060

******************************************************************************/

#include "emu.h"

#include "bus/kim1/cards.h"
#include "bus/kim1/kim1bus.h"
#include "cpu/m6502/m6502.h"
#include "machine/mos6530.h"
#include "machine/timer.h"
#include "video/pwm.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "p6060.lh"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

#define P6060_CPU_TAG "maincpu"

class p6060_state : public driver_device
{
public:
	p6060_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, P6060_CPU_TAG)
		, m_screen(*this, "screen")
		, m_miot(*this, "miot%u", 0)
		, m_digit_pwm(*this, "digit_pwm")
		, m_lights(*this, "LIGHTS")
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(trigger_reset);
	DECLARE_INPUT_CHANGED_MEMBER(trigger_nmi);
	void p6060(machine_config &config);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<m6502_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device_array<mos6530_device, 2> m_miot;
	required_device<pwm_display_device> m_digit_pwm;
	required_ioport m_lights;

	int m_sync_state = 0;
	bool m_k7 = false;
	bool m_tty_in = false;
	uint8_t m_u2_port_b = 0;
	uint8_t m_311_output = 0;

	void mem_map(address_map &map) ATTR_COLD;
	void sync_map(address_map &map) ATTR_COLD;

	uint8_t sync_r(offs_t offset);
	void sync_w(int state);

	uint8_t u2_read_a();
	void u2_write_a(uint8_t data);
	uint8_t u2_read_b();
	void u2_write_b(uint8_t data);

	void tty_callback(int data);
};

void p6060_state::machine_start()
{
	// Register for save states
	save_item(NAME(m_sync_state));
	save_item(NAME(m_k7));
	save_item(NAME(m_tty_in));
	save_item(NAME(m_u2_port_b));
	save_item(NAME(m_311_output));
}

void p6060_state::machine_reset()
{
	m_311_output = 0;
}

//**************************************************************************
//  I/O
//**************************************************************************

INPUT_CHANGED_MEMBER(p6060_state::trigger_reset)
{
	// RS key triggers system reset via 556 timer
	if (newval)
		machine().schedule_soft_reset();
}

INPUT_CHANGED_MEMBER(p6060_state::trigger_nmi)
{
	// ST key triggers NMI via 556 timer
	if (newval)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

// itmap_ind16
uint32_t p6060_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t const pen = 0x1234;
	for (int y = 0; y < 200; y++)
	{
		for (int sx = 0; sx < 40; sx++)
		{
			for (int x = 0; x < 8; x++)
			{
				bitmap.pix(y, (sx * 8) + x) = pen;
			}
		}
	}
	return 0;
}

uint8_t p6060_state::sync_r(offs_t offset)
{
	// A10-A12 to 74145
	if (!machine().side_effects_disabled())
		m_k7 = bool(~offset & 0x1c00);

	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

void p6060_state::sync_w(int state)
{
	// Signal NMI at falling edge of SYNC when SST is enabled and K7 line is high
	/*
	if (m_sync_state && !state && m_k7 && BIT(m_special->read(), 2))
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	*/
	m_sync_state = state;
}

uint8_t p6060_state::u2_read_a()
{
	uint8_t data = 0x7f;

	// Read from keyboard
	/*
	offs_t const sel = (m_u2_port_b >> 1) & 0x0f;
	if (4U > sel)
		data = m_row[sel]->read() & 0x7f;
  */
	return data;
}

void p6060_state::u2_write_a(uint8_t data)
{
	// Write to 7-segment LEDs
	m_digit_pwm->write_mx(data & 0x7f);
}

uint8_t p6060_state::u2_read_b()
{
	return 0xff;
}

void p6060_state::u2_write_b(uint8_t data)
{
	m_u2_port_b = data;

	// Select 7-segment LED
	m_digit_pwm->write_my(1 << (data >> 1 & 0xf) >> 4);

}

//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void p6060_state::mem_map(address_map &map)
{
	map(0x0000, 0x03ff).mirror(0xe000).ram();
	map(0x1700, 0x170f).mirror(0xe030).m(m_miot[1], FUNC(mos6530_device::io_map));
	map(0x1740, 0x174f).mirror(0xe030).m(m_miot[0], FUNC(mos6530_device::io_map));
	map(0x1780, 0x17bf).mirror(0xe000).m(m_miot[1], FUNC(mos6530_device::ram_map));
	map(0x17c0, 0x17ff).mirror(0xe000).m(m_miot[0], FUNC(mos6530_device::ram_map));
	map(0x1800, 0x1bff).mirror(0xe000).m(m_miot[1], FUNC(mos6530_device::rom_map));
	map(0x1c00, 0x1fff).mirror(0xe000).m(m_miot[0], FUNC(mos6530_device::rom_map));
}

void p6060_state::sync_map(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(p6060_state::sync_r));
}

// Called when serial data comes in from console.
void p6060_state::tty_callback(int data)
{
	// Save state as it is needed by u2_write_b()
	m_tty_in = data;

}


//**************************************************************************
//  INPUT PORTS
//**************************************************************************

static INPUT_PORTS_START( p6060 )
	PORT_START("LIGHTS")
	PORT_BIT( 0x001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("NoPrint")
	PORT_BIT( 0x002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PrintAll")
	PORT_BIT( 0x004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Step")
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Trace")
	PORT_BIT( 0x010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Continue")
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Break")
	// No 0x40
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CalcMode")
	// 4 LEDs...
INPUT_PORTS_END


//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

void p6060_state::p6060(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 1_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &p6060_state::mem_map);
	m_maincpu->set_addrmap(AS_OPCODES, &p6060_state::sync_map);
	m_maincpu->sync_cb().set(FUNC(p6060_state::sync_w));

	// screen
	// Burroughs SSD0132 Plasma Display, 222x7
	// 1 MHz update freq ???
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// pixclock, htotal, hbend, hbstart, vtotal, vbend, vbstart)
	m_screen->set_raw(XTAL(8'000'000)/2, 320, 0, 320, 200, 0, 200);
	// m_screen->set_raw(1021800*14, (65*7)*2, 0, (40*7)*2, 262, 0, 192);
	m_screen->set_color(rgb_t::green());
	// m_screen->set_palette(m_video);
	m_screen->set_screen_update(FUNC(p6060_state::screen_update));


	// video hardware
	PWM_DISPLAY(config, m_digit_pwm).set_size(6, 7);
	m_digit_pwm->set_segmask(0x3f, 0x7f);
	config.set_default_layout(layout_p6060);

	// devices
	MOS6530(config, m_miot[0], 1_MHz_XTAL); // U2
	m_miot[0]->pa_rd_callback().set(FUNC(p6060_state::u2_read_a));
	m_miot[0]->pa_wr_callback().set(FUNC(p6060_state::u2_write_a));
	m_miot[0]->pb_rd_callback().set(FUNC(p6060_state::u2_read_b));
	m_miot[0]->pb_wr_callback().set(FUNC(p6060_state::u2_write_b));

	MOS6530(config, m_miot[1], 1_MHz_XTAL); // U3

	SPEAKER(config, "mono").front_center();

	// KIM-1 has two edge connectors for expansion; you could plug them into a backplane,
	// and that's what we're abstracting here.
	KIM1BUS(config, "bus", 0).set_space(m_maincpu, AS_PROGRAM);
	KIM1BUS_SLOT(config, "sl1", 0, "bus", kim1_cards, nullptr);
	KIM1BUS_SLOT(config, "sl2", 0, "bus", kim1_cards, nullptr);
	KIM1BUS_SLOT(config, "sl3", 0, "bus", kim1_cards, nullptr);
	KIM1BUS_SLOT(config, "sl4", 0, "bus", kim1_cards, nullptr);
	KIM1BUS_SLOT(config, "sl5", 0, "bus", kim1_cards, nullptr);

}

//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( p6060 )
	ROM_REGION( 0x400, "miot0", 0 )
	ROM_LOAD("6530-002.u2", 0x0000, 0x0400, CRC(2b08e923) SHA1(054f7f6989af3a59462ffb0372b6f56f307b5362))

	ROM_REGION( 0x400, "miot1", 0 )
	ROM_LOAD("6530-003.u3", 0x0000, 0x0400, CRC(a2a56502) SHA1(60b6e48f35fe4899e29166641bac3e81e3b9d220))
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY      FULLNAME  FLAGS
COMP( 1975, p6060, 0,      0,      p6060,   p6060,  p6060_state, empty_init, "Olivetti", "P6060",  MACHINE_SUPPORTS_SAVE)
