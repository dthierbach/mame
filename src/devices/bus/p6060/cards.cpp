// license:BSD-3-Clause
// copyright-holders:Dirk Thierbach
/***************************************************************************

	Olivetti P6060 cards

***************************************************************************/

#include "emu.h"
#include "cards.h"

#include "goino.h"
#include "flodi.h"
#include "gips.h"


void p6060_cards(device_slot_interface &device)
{
	device.option_add("goino", P6060BUS_GOINO);
	device.option_add("flodi", P6060BUS_FLODI);
	device.option_add("gips", P6060BUS_GIPS);
}

/*

Business Name          DMA

IPSO 6600     GIPS        IPSO parallel interface
SIC 6629      GISA        EIA RS232 serial interface
PIC 6629      GO024    +  IEEE 488 interface
DSC 6681      GO011    -  Video (Text) controller
DSC 6683      GO011    -  Video (Graphics) controller
HDC 6614      DIFO     +  Harddisk controller
DCC 6609      DIMO     +  Removable disk controller
DMA 6680      RODMA    *  DMA Boot ROM

DMA order: (p32 "STAC 1980 03")
  7 => 2 -> 3 -> 4 -> 5 -> 6
IRQ order:
  1 -> 13 -> 12 -> 11 -> 10 -> 9 -> 8 -> 2 -> 3 -> 4 -> 5 -> 6

So:
  sl1
	sl2
	sl3
	sl4
	sl5
	sl6

in that priority
  sl13 GOINO fix?
	sl12/11 Floppy fix?

Variants
  GOINO (pos 13)
	FLOA,B (pos 12+11)
	PUCE (pos 9+10)
  GIPS (pos 1,2,3,4)
	  GIPS
		GIPS3
	GISA (pos 1,2,4)
	  GISA1
	  GISA2
  GO011
	  GO011
	???
	  PON44B
	ROM (pos 7)
	  ROMCA
	  RODMA
	DIFO (pos 2,3)
	  DIFO
	DIMO (pos 2,3)
	  DIMO


Cards

GOINO
FLOA
FLOB
PUCE1
PUCE2
MEMxxx
ROMCA


*/