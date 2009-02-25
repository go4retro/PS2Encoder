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


   ps2_device.c: Internal functions for device PS/2 mode

*/

#include <inttypes.h>
#include <avr/io.h>
#include <util.delay.h>
#include "config.h"
#include "avrcompat.h"
#include "ps2.h"
#include "ps2_lib.h"
#include "ps2_device.h"

static volatile uint8_t PS2_dev_holdoff_count;

void PS2_dev_host_inhibit(void) {
  // CLK is low.  Host wants to talk to us.
  // turn off timer
  ps2_timer_irq_off();
  // look for rising clock
  PS2_enable_IRQ_CLK_Rise();
  PS2_set_state(PS2_ST_HOST_INHIBIT);
  // release DATA line, if we happen to have it.
  PS2_set_DATA();
}

void ps2_dev_init(uint8_t mode) {
  ps2_lib_init();

  PS2_set_state(PS2_ST_IDLE);
  PS2_disable_IRQ_CLK();
  ps2_timer_irq_off();
  PS2_set_CLK();
  PS2_set_DATA();
  // wait 600mS.
  _delay_ms(600);
  //PS2_clear_CLK();
  //PS2_clear_DATA();
  //_delay_ms(775);
  //PS2_set_CLK();
  //_delay_ms(80);
  //PS2_set_DATA();
  //_delay_ms(384);
  //PS2_clear_CLK();
  //_delay_ms(483);
  //PS2_set_CLK();
  //_delay_ms(60);
  ps2_putc(PS2_CMD_BAT);
  // need to do this once here, as CLK might already be low.
  //if(!PS2_read_CLK()) {
  //  PS2_dev_host_inhibit();
  //} else {
  //  PS2_enable_IRQ_CLK_Fall();
  //}
}
void ps2_init(uint8_t mode) __attribute__ ((weak, alias("ps2_dev_init")));

void ps2_dev_trigger_send(void) {
  // start clocking.
  // wait a half cycle
  //debug2('s');
  ps2_timer_irq_set(PS2_HALF_CYCLE);
  // bring DATA line low to ensure everyone knows our intentions
  PS2_clear_DATA();
  // set state
  PS2_set_state(PS2_ST_PREP_START);
}
void ps2_trigger_send(void) __attribute__ ((weak, alias("ps2_dev_trigger_send")));

void PS2_dev_check_data(void) {
  //debug2('d');
  // do we have data to send?
  if(PS2_data_to_send()) {
    ps2_dev_trigger_send();
  } else {
    //debug2('n');
    PS2_set_state(PS2_ST_IDLE);
    //PS2_set_DATA();
    ps2_timer_irq_off();
    PS2_enable_IRQ_CLK_Fall();
  }
}

void ps2_dev_clk_irq(void) {
  PS2_disable_IRQ_CLK();
  //debug2(']');
  switch(PS2_get_state()) {
    case PS2_ST_IDLE:
    case PS2_ST_PREP_START:
      // host is holding us off.  Wait for CLK hi...
      PS2_dev_host_inhibit();
      break;
    case PS2_ST_HOST_INHIBIT:
      //debug2('f');
      // CLK went hi
      if(PS2_read_DATA()) {
        //debug2('s');
        // we can send if we need to.
        PS2_dev_check_data();
      } else {
        //debug2('r');
        // host wants to send data, CLK is high.
        // wait half cycle to let things settle.
        // clock in data from host.
        ps2_timer_irq_set(PS2_HALF_CYCLE);
        PS2_set_state(PS2_ST_WAIT_START);
      }
      break;
    default:
      //debug('&');
      //uart_puthex(PS2_get_state());
      break;
  }
}
void ps2_clk_irq(void) __attribute__ ((weak, alias("ps2_dev_clk_irq")));

void ps2_dev_timer_irq(void) {
  switch (PS2_get_state()) {
    case PS2_ST_PREP_START:
      // disable the CLK IRQ
      PS2_disable_IRQ_CLK();
      // clk the start bit
      PS2_clear_CLK();
      PS2_set_state(PS2_ST_SEND_START);
      break;
    case PS2_ST_SEND_START:
      PS2_read_byte();
      PS2_set_CLK();  // bring CLK hi
      if(PS2_read_CLK()) {
        PS2_write_bit();
      } else {
        PS2_dev_host_inhibit();
      }
      break;
    case PS2_ST_PREP_BIT:
      PS2_clear_CLK();
      PS2_set_state(PS2_ST_SEND_BIT);
      break;
    case PS2_ST_SEND_BIT:
      PS2_set_CLK();  // bring CLK hi
      if(PS2_read_CLK()) {
        if(PS2_get_count() == 8) {
          // we are done..., do parity
          PS2_write_parity();
          PS2_set_state(PS2_ST_PREP_PARITY);
        } else {
          // state is set in function.
          PS2_write_bit();
        }
      } else {
        PS2_dev_host_inhibit();
      }
      break;
    case PS2_ST_PREP_PARITY:
      // clock parity
      PS2_clear_CLK();
      PS2_set_state(PS2_ST_SEND_PARITY);
      break;
    case PS2_ST_SEND_PARITY:
      PS2_set_CLK();  // bring CLK hi
      if(PS2_read_CLK()) {
        PS2_set_DATA();
        PS2_set_state(PS2_ST_PREP_STOP);
      } else {
        PS2_dev_host_inhibit();
      }
      break;
    case PS2_ST_PREP_STOP:
      PS2_clear_CLK();
      PS2_set_state(PS2_ST_SEND_STOP);
      break;
    case PS2_ST_SEND_STOP:
      // If host wanted to abort, they had to do it before now.
      PS2_commit_read_byte();
      PS2_set_CLK();  // bring CLK hi
      if(PS2_read_CLK()) {
        if(PS2_read_DATA()) {
          // for some reason, you have to wait a while before sending again.
          PS2_dev_holdoff_count=PS2_SEND_HOLDOFF_COUNT;
          PS2_set_state(PS2_ST_HOLDOFF);
        } else {
          // Host wants to talk to us.
          PS2_set_state(PS2_ST_WAIT_START);
        }
      } else {
        //debug2('*');
        PS2_dev_host_inhibit();
      }
      break;
    case PS2_ST_WAIT_START:
      // set CLK lo
      PS2_clear_CLK();
      PS2_clear_counters();
      // read start bit
      if(PS2_read_DATA()) {
        // not sure what you do if start bit is high...
        PS2_set_CLK();
        PS2_set_state(PS2_ST_IDLE);
        ps2_timer_irq_off();
        PS2_enable_IRQ_CLK_Fall();
        //debug2('-');
      } else {
        //debug2('+');
        PS2_set_state(PS2_ST_GET_START);
      }
      break;
    case PS2_ST_GET_START:
      PS2_set_CLK();  // bring CLK hi
      if(PS2_read_CLK()) {
        PS2_set_state(PS2_ST_WAIT_BIT);
      } else {
        PS2_dev_host_inhibit();
      }
      break;
    case PS2_ST_WAIT_BIT:
      PS2_clear_CLK();
      // you read incoming bits on falling clock.
      PS2_read_bit();
      PS2_set_state(PS2_ST_GET_BIT);
      break;
    case PS2_ST_GET_BIT:
      PS2_set_CLK();  // bring CLK hi
      if(PS2_read_CLK()) {
        if(PS2_get_count() == 8) {
          // done, do Parity bit
          PS2_set_state(PS2_ST_GET_PARITY);
        } else {
          PS2_set_state(PS2_ST_WAIT_BIT);
        }
      } else {
        // host aborted send.
        PS2_dev_host_inhibit();
      }
      break;
    case PS2_ST_GET_PARITY:
      PS2_clear_CLK();
      // ignore parity for now.
      PS2_set_state(PS2_ST_WAIT_STOP);
      break;
    case PS2_ST_WAIT_STOP:
      PS2_set_CLK();  // bring CLK hi
      if(PS2_read_CLK()) {
        if(PS2_read_DATA()) {
          PS2_set_state(PS2_ST_WAIT_ACK);
          // bing DATA low to ack
          PS2_clear_DATA();
          // commit data
          //PS2_write_byte();  jlb, moved.
        } else {
          PS2_set_state(PS2_ST_GET_PARITY);
        }
      } else {
        // host aborted send.
        PS2_dev_host_inhibit();
      }
      break;
    case PS2_ST_WAIT_ACK:
      PS2_clear_CLK();
      PS2_set_state(PS2_ST_GET_ACK);
      break;
    case PS2_ST_GET_ACK:
      PS2_set_CLK();
      PS2_set_DATA();
      // we just need to wait a 50uS or so, to ensure the host saw the CLK go high
      PS2_dev_holdoff_count=1;
      PS2_set_state(PS2_ST_HOLDOFF);
      PS2_write_byte();   //jlb moved
      break;
    case PS2_ST_HOLDOFF:
      PS2_dev_holdoff_count--;
      if(!PS2_dev_holdoff_count) {
        if(PS2_read_CLK()) {
          if(PS2_read_DATA()) {
            PS2_dev_check_data();
          } else {
            PS2_set_state(PS2_ST_WAIT_START);
          }
        } else {
          PS2_dev_host_inhibit();
        }
      }
      break;
    default:
      //debug('#');
      //uart_puthex(PS2_get_state());
      ps2_timer_irq_off();
      break;
  }
}
void ps2_timer_irq(void) __attribute__ ((weak, alias("ps2_dev_timer_irq")));
