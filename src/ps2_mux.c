/* PS2Encoder - PS/2 Keyboard Encoder
   Copyright 2008,2009 Jim Brain <brain@jbrain.com>

   This code is a modification of uart functions in sd2iec:
   Copyright (C) 2007,2008  Ingo Korb <ingo@akana.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License only.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


   ps2_mux.c: Multiplexer for code that needs to support both PS/2 directions

*/
// timing information derived from http://panda.cs.ndsu.nodak.edu/~achapwes/PICmicro/PS2/ps2.htm

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "config.h"
#include "avrcompat.h"
#include "ps2.h"
#include "ps2_device.h"
#include "ps2_host.h"
#include "uart.h"

static uint8_t mode;

void ps2_trigger_send(void) {
  if(mode==PS2_MODE_DEVICE) {
    ps2_dev_trigger_send();
  } else {
    ps2_host_trigger_send();
  }
}

void ps2_clk_irq(void) {
  if(mode==PS2_MODE_DEVICE) {
    ps2_dev_clk_irq();
  } else {
    ps2_host_clk_irq();
  }
}

void ps2_timer_irq(void) {
  if(mode==PS2_MODE_DEVICE) {
    ps2_dev_timer_irq();
  } else {
    ps2_host_timer_irq();
  }
}

void ps2_init(uint8_t mode) {
  mode=mode;
  if(mode==PS2_MODE_DEVICE) {
    ps2_dev_init(mode);
  } else {
    ps2_host_init(mode);
  }
}
