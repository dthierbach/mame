// license:BSD-3-Clause
// copyright-holders:Dirk Thierbach
/***************************************************************************

  Olivetti P6060 card bus interface

	for use from PUCE CPU.

***************************************************************************/

#ifndef MAME_BUS_P6060_P6060INTF_H
#define MAME_BUS_P6060_P6060INTF_H

#pragma once

namespace irq_priority {
	enum t {
		FIRST = 0,
  	LEVEL1 = FIRST,
  	LEVEL2,
  	LEVEL3A,
  	LEVEL3B,
  	LAST
	};
}

// order of slots for irq, type is index and mask position
typedef unsigned irq_order_t;

// irq mask, bits in irq order
typedef unsigned irq_mask_t;

// order of slots for dna, type is index and mask position;
typedef unsigned dma_order_t;

// dma mask, bits in dma order
typedef unsigned dma_mask_t;

class p6060bus_interface
{
public:

// ---- from CPU

	// command/data
	virtual void set_ecd(u16 data) = 0;
	virtual u16 get_ecd() = 0;

  // reset: all cards
	virtual void set_ecor(int level) = 0;

	// select: in priority order to all cards
	virtual bool strobe_ecos() = 0;

  // transmit/sync: selected card
	virtual void strobe_ecot() = 0;

  // command (includes ecot): selected card
	virtual void strobe_ecoc() = 0;

  // finish: selected card
	virtual void strobe_ecof() = 0;

  // signal 1: selected card
	virtual void set_ec1f(int level) = 0;

  // signal 2: selected card
	virtual void set_ec2f(int level) = 0;

	virtual void grant_irq(int level) = 0;

	// ---- from periphery

	// data/state
	virtual void set_epd(u8 data) = 0;
	virtual u8 get_epd() = 0;

	// name of periphery
	virtual void set_epn(u8 name) = 0;
	virtual u8 get_epn() = 0;

	// type of interrupt
	virtual void set_ept(u8 type) = 0;
	virtual u8 get_ept() = 0;

	virtual void request_irq(irq_priority::t priority, irq_mask_t mask) = 0;

};

#endif  // MAME_BUS_P6060_P6060INTF_H
