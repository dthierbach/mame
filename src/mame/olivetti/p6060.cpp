// license:BSD-3-Clause
// copyright-holders:Dirk Thierbach
/******************************************************************************

Olivetti P6060

******************************************************************************/

#include "emu.h"

#include "cpu/puce/puce.h"
#include "machine/timer.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "p6060.lh"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

#define P6060_CPU_TAG "maincpu"

#define DISPLAY_WIDTH 222
#define DISPLAY_HEIGHT 7

class p6060_state : public driver_device
{
public:
	p6060_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, P6060_CPU_TAG)
		, m_screen(*this, "screen")
		, m_buttons(*this, "BUTTONS")
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(trigger_reset);
	DECLARE_INPUT_CHANGED_MEMBER(trigger_nmi);
	void p6060(machine_config &config);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<puce_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_ioport m_buttons;

	void program_mem_map(address_map &map) ATTR_COLD;
	void data_mem_map(address_map &map) ATTR_COLD;
};

void p6060_state::machine_start()
{
	// m_lamps.resolve();
	/*
	m_lamps[0] = 0;
	m_lamps[1] = 1;
	m_lamps[2] = 0;
	m_lamps[3] = 1;
	m_lamps[4] = 0;
	m_lamps[5] = 1;
	m_lamps[6] = 0;
	m_lamps[7] = 1;
	m_lamps[8] = 0;
	m_lamps[9] = 1;
	m_lamps[10] = 0;
	m_lamps[11] = 1;
	m_lamps[12] = 0;
	*/
	// Register for save states
}

void p6060_state::machine_reset()
{
}

//**************************************************************************
//  I/O
//**************************************************************************

// itmap_ind16
uint32_t p6060_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t const pen = 0xf09090f0;
	for (int y = 0; y < DISPLAY_HEIGHT; y++)
	{
		for (int x = 0; x < DISPLAY_WIDTH; x++)
		{
			bitmap.pix(y, x) = pen;
		}
	}
	return 0;
}

//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void p6060_state::program_mem_map(address_map &map)
{
	// TODO: Add byte access delegate to shift address for lower half.
	map(0x0000, 0x7fff).ram();
	map(0x8000, 0x87ff).rom().region("bootrom", 0);
	map(0x8800, 0x9fff).unmaprw();
	map(0xa000, 0xbfff).ram();
	map(0xc000, 0xffff).unmaprw();
}

void p6060_state::data_mem_map(address_map &map)
{
	// TODO: Add byte access delegate to shift address for lower half.
	map(0x0000, 0xffff).ram();
}

//**************************************************************************
//  INPUT PORTS
//**************************************************************************

static INPUT_PORTS_START( p6060 )
	PORT_START("BUTTONS")
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
	PUCE(config, m_maincpu, 1_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &p6060_state::program_mem_map);
	m_maincpu->set_addrmap(AS_DATA, &p6060_state::data_mem_map);

	// screen
	// Burroughs SSD0132 Plasma Display, 222x7
	// 1 MHz update freq ???
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// pixclock, htotal, hbend, hbstart, vtotal, vbend, vbstart)
	m_screen->set_raw(XTAL(8'000'000)/2, DISPLAY_WIDTH, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0, DISPLAY_HEIGHT);
	// m_screen->set_raw(1021800*14, (65*7)*2, 0, (40*7)*2, 262, 0, 192);
	m_screen->set_color(rgb_t::amber());
	// m_screen->set_palette(m_video);
	m_screen->set_screen_update(FUNC(p6060_state::screen_update));

	// video hardware

	config.set_default_layout(layout_p6060);

	SPEAKER(config, "mono").front_center();

}

//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( p6060 )
	ROM_REGION16_BE(0x0800*2, "bootrom", 0)
	ROM_LOAD("romca.bin", 0x0000, 0x0800*2, CRC(9caca305) SHA1(63a68b4787f33bef156f05c6b50269357d0d3c2f))
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY      FULLNAME  FLAGS
COMP( 1975, p6060, 0,      0,      p6060,   p6060,  p6060_state, empty_init, "Olivetti", "P6060",  MACHINE_SUPPORTS_SAVE)

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