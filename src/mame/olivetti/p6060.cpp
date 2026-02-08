// license:BSD-3-Clause
// copyright-holders:Dirk Thierbach
/******************************************************************************

	Olivetti P6060

******************************************************************************/

#include "emu.h"

#include "cpu/puce/puce.h"
#include "bus/p6060/p6060bus.h"
#include "bus/p6060/cards.h"
#include "machine/timer.h"

#include "softlist_dev.h"
#include "speaker.h"

#include "p6060.lh"

#define VERBOSE (1)
#include "logmacro.h"

namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

#define P6060_CPU_TAG "maincpu"
#define P6060_BUS_TAG "extbus"

class p6060_state : public driver_device
{
public:
	p6060_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, P6060_CPU_TAG)
		, m_extbus(*this, P6060_BUS_TAG)
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
	required_device<p6060bus_device> m_extbus;
	required_ioport m_buttons;

	void program_mem_map(address_map &map) ATTR_COLD;
	void data_mem_map(address_map &map) ATTR_COLD;
};

void p6060_state::machine_start()
{
	LOG("%s: machine_start\n", machine().describe_context());
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
	LOG("%s: machine_reset\n", machine().describe_context());
}

//**************************************************************************
//  I/O
//**************************************************************************

// itmap_ind16
uint32_t p6060_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

//**************************************************************************
//  ADDRESS MAPS and delegates
//**************************************************************************

void p6060_state::program_mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rw(m_maincpu, FUNC(puce_device::read16_delegate), FUNC(puce_device::write16_delegate));
	map(0x8000, 0x87ff).rom().region("bootrom", 0);
	map(0x8800, 0x9fff).unmaprw();
	map(0xa000, 0xbfff).ram();
	map(0xc000, 0xffff).unmaprw();
}

void p6060_state::data_mem_map(address_map &map)
{
	// When switching to managed RAM: "User RAM" sizes are 16K, 24K, 32K, 40K, 48K (bytes)
	// That corresponds to 32K. 40K, 48K, 56K, 64K total lower RAM (bytes).
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

	// extbus
	P6060BUS(config, m_extbus, 0);
	m_maincpu->set_extbus(m_extbus);
	m_extbus->set_cpu(m_maincpu);

	P6060BUS_SLOT(config, "sl13", 0, m_extbus, p6060_cards, "goino");
	P6060BUS_SLOT(config, "sl1", 0, m_extbus, p6060_cards, nullptr);
	/*
	TODO: goino and lfoppy in fixed position
	P6060BUS(config, "bus", 0);
	P6060BUS_SLOT(config, "sl1", 0, "bus", p6060_cards, nullptr);
	P6060BUS_SLOT(config, "sl2", 0, "bus", p6060_cards, nullptr);
	P6060BUS_SLOT(config, "sl3", 0, "bus", p6060_cards, nullptr);
	P6060BUS_SLOT(config, "sl4", 0, "bus", p6060_cards, nullptr);
	P6060BUS_SLOT(config, "sl5", 0, "bus", p6060_cards, nullptr);
  */
	/*
	A2BUS_SLOT(config, "sl0", XTAL(14'318'181) / 2, m_a2bus, apple2_slot0_cards, "lang");
	A2BUS_SLOT(config, "sl1", XTAL(14'318'181) / 2, m_a2bus, apple2_cards, nullptr);
	A2BUS_SLOT(config, "sl2", XTAL(14'318'181) / 2, m_a2bus, apple2_cards, nullptr);
	A2BUS_SLOT(config, "sl3", XTAL(14'318'181) / 2, m_a2bus, apple2_cards, nullptr);
	A2BUS_SLOT(config, "sl4", XTAL(14'318'181) / 2, m_a2bus, apple2_cards, "mockingboard");
	A2BUS_SLOT(config, "sl5", XTAL(14'318'181) / 2, m_a2bus, apple2_cards, nullptr);
	A2BUS_SLOT(config, "sl6", XTAL(14'318'181) / 2, m_a2bus, apple2_cards, "diskiing");
	A2BUS_SLOT(config, "sl7", XTAL(14'318'181) / 2, m_a2bus, apple2_cards, nullptr);
  */

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
