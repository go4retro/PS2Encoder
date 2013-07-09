/*
    PS2Encoder - PS2 Keyboard to serial/parallel converter
    Copyright Jim Brain and RETRO Innovations, 2008-2012

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    xt.c: Internal functions for host/device XT KB modes

    timing information derived from http://ilkerf.tripod.com/c64tower/F_Keyboard_FAQ.html#KEYBOARDFAQ_037
*/

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <util/delay.h>
#include "config.h"
#include "xt.h"
#include "uart.h"

static uint8_t rxbuf[1 << XT_RX_BUFFER_SHIFT];
static volatile uint8_t rx_head;
static volatile uint8_t rx_tail;
static uint8_t txbuf[1 << XT_TX_BUFFER_SHIFT];
static volatile uint8_t tx_head;
static volatile uint8_t tx_tail;

static volatile xtstate_t xt_state;
static volatile uint8_t xt_byte;
static volatile uint8_t xt_bit_count;
static volatile uint8_t xt_parity;

static xtmode_t xt_mode;

static volatile uint8_t xt_holdoff_count;

static void xt_enable_clk_rise(void) {
  // turn off IRQ
  CLK_INTCR &= (uint8_t)~_BV(CLK_INT);
  // reset flag
  CLK_INTFR |= _BV(CLK_INTF);
  // rising edge
  CLK_INTDR |= _BV(CLK_ISC1) | _BV(CLK_ISC0);
  // turn on
  CLK_INTCR |= _BV(CLK_INT);
}

static void xt_enable_clk_fall(void) {
  // turn off IRQ
  CLK_INTCR &= (uint8_t)~_BV(CLK_INT);
  // reset flag
  CLK_INTFR |= _BV(CLK_INTF);
  // falling edge
  CLK_INTDR = (CLK_INTDR & (uint8_t)~(_BV(CLK_ISC1) | _BV(CLK_ISC0))) | _BV(CLK_ISC1);
  // turn on
  CLK_INTCR |= _BV(CLK_INT);
}

static void xt_disable_clk(void) {
  CLK_INTCR &= (uint8_t)~_BV(CLK_INT);
}

static void xt_enable_timer(uint8_t us) {
  // clear flag.
  PS2_TIFR |= PS2_TIFR_DATA;
  // clear TCNT;
  PS2_TCNT = 0;
  // set the count...
#if F_CPU > 14000000
  // us is uS....  Need to * 14 to get ticks, then divide by 8...
  // cheat... * 14 / 8 = *2 = <<1
  PS2_OCR = (uint8_t)(us << 1);
#elif F_CPU > 7000000
  PS2_OCR = us;
#else
  PS2_OCR = (us >> 1);
#endif
  // enable output compare IRQ
  PS2_TIMSK |= PS2_TIMSK_DATA;
}

static void xt_disable_timer(void) {
  // disable output compare IRQ
  PS2_TIMSK &= (uint8_t)~PS2_TIMSK_DATA;
}

static void xt_write_byte(void) {
  uint8_t tmp;
  /* Calculate buffer index */
  tmp = ( rx_head + 1 ) & PS2_RX_BUFFER_MASK;
  rx_head = tmp;      /* Store new index */

  if ( tmp == rx_tail ) {
    /* ERROR! Receive buffer overflow */
  }
  rxbuf[tmp] = xt_byte; /* Store received data in buffer */
}

static void xt_read_byte(void) {
  xt_bit_count = 0;
  xt_parity = 0;
  xt_byte = txbuf[( tx_tail + 1 ) & PS2_TX_BUFFER_MASK];  /* Start transmition */
}

static void xt_commit_read_byte(void) {
  tx_tail = ( tx_tail + 1 ) & PS2_TX_BUFFER_MASK;      /* Store new index */
}

static uint8_t xt_data_to_send(void) {
  return ( tx_head != tx_tail );
}

static void xt_write_bit(void) {
  xt_state=PS2_ST_PREP_BIT;
  // set DATA..
  switch (xt_byte & 1) {
    case 0:
      xt_clear_data();
      break;
    case 1:
      xt_parity++;
      xt_set_data();
      break;
  }
  // shift right.
  xt_byte= xt_byte >> 1;
  xt_bit_count++;
  // valid data now.
}

static void xt_read_bit(void) {
  xt_byte = xt_byte >> 1;
  xt_bit_count++;
  if(xt_read_data()) {
    xt_byte |= 0x80;
    xt_parity++;
  }
}

static void xt_write_parity(void) {
  if((xt_parity & 1) == 1) {
    xt_clear_data();
  } else {
    xt_set_data();
  }
}

static void xt_clear_counters(void) {
  xt_byte = 0;
  xt_bit_count = 0;
  xt_parity = 0;
}

void xt_clear_buffers(void) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    tx_head = 0;
    tx_tail = 0;
    rx_head = 0;
    rx_tail = 0;
  }
}

#ifdef PS2_ENABLE_DEVICE
static void xt_device_trigger_send(void) {
  // start clocking.
  // wait a half cycle
  xt_enable_timer(PS2_HALF_CYCLE);
  // bring DATA line low to ensure everyone knows our intentions
  xt_clear_data();
}
#endif

#ifdef PS2_ENABLE_HOST
static void xt_host_trigger_send(void) {
  // need to get devices attention...
  xt_disable_clk();
  xt_clear_clk();
  // yes, bring CLK lo for 100uS
  xt_enable_timer(100);
}
#endif

static void xt_trigger_send(void) {
  // set state
  xt_state = PS2_ST_PREP_START;
  PS2_CALL(xt_device_trigger_send(),xt_host_trigger_send());
}

#ifdef PS2_ENABLE_HOST
static void xt_host_check_for_data(void) {
  if(xt_data_to_send() != 0) {
    xt_trigger_send();
  } else {
    // wait for something to receive
    xt_state = PS2_ST_IDLE;
    xt_enable_clk_fall();
  }
}

static inline void xt_host_timer_irq(void) {
  xt_disable_timer();
  switch (xt_state) {
    case PS2_ST_GET_BIT:
    case PS2_ST_GET_PARITY:
    case PS2_ST_GET_STOP:
      // do we have data to send to keyboard?
      xt_host_check_for_data();
      break;
    case PS2_ST_PREP_START:
      // we waited 100uS for device to notice us, bring DATA low and CLK hi
      xt_clear_data();
      xt_set_clk();
      if(!xt_read_clk()) {
        // kb wants to talk to us.
        xt_set_data();
        xt_enable_clk_fall();
        xt_state = PS2_ST_GET_BIT;
      } else {
        // really start bit...
        // now, wait for falling CLK
        xt_enable_clk_fall();
        //xt_state = PS2_ST_SEND_START;  JLB incorrect
        xt_state = PS2_ST_PREP_BIT;
        xt_read_byte();
      }
      break;
    default:
      break;
  }
}


static void xt_host_clk_irq(void) __attribute__((always_inline));
static inline void xt_host_clk_irq(void) {
  switch(xt_state) {
    case PS2_ST_WAIT_RESPONSE:
    case PS2_ST_IDLE:
      // keyboard sent start bit
      // should read it, but will assume it is good.
      xt_state = PS2_ST_GET_BIT;
      // if we don't get another CLK in 100uS, timeout.
      xt_enable_timer(100);
      xt_clear_counters();
      break;
    case PS2_ST_GET_BIT:
      // if we don't get another CLK in 100uS, timeout.
      xt_enable_timer(100);
      // read bit;
      xt_read_bit();
      if(xt_bit_count == 8) {
        // done, do Parity bit
        xt_state = PS2_ST_GET_PARITY;
      }
      break;
    case PS2_ST_GET_PARITY:
      // if we don't get another CLK in 100uS, timeout.
      xt_enable_timer(100);
      // grab parity
      // for now, assume it is OK.
      xt_state = PS2_ST_GET_STOP;
      break;
    case PS2_ST_GET_STOP:
      xt_disable_timer();
      // stop bit
      // for now, assume it is OK.
      xt_write_byte();
      // wait for CLK to rise before doing anything else.
      xt_state = PS2_ST_HOLDOFF;
      xt_enable_clk_rise();
      break;
    case PS2_ST_HOLDOFF:
      // CLK rose, so now, check for more data.
      // do we have data to send to keyboard?
      xt_host_check_for_data();
      break;
//    case PS2_ST_SEND_START:
//      xt_state = PS2_ST_PREP_BIT;
//      break;
    case PS2_ST_PREP_BIT:
      // time to send bits...
      if(xt_bit_count == 8) {
        // we are done..., do parity
        xt_write_parity();
        xt_state = PS2_ST_SEND_PARITY;
      } else {
        xt_write_bit();
      }
      break;
    case PS2_ST_SEND_PARITY:
      // send stop bit.
      xt_set_data();
      xt_state = PS2_ST_SEND_STOP;
      break;
    case PS2_ST_SEND_STOP:
      if(!xt_read_data()) {
        // commit the send
        xt_commit_read_byte();
        /*
         * We could wait for the CLK hi, then check to see if we have more
         * data to send.  However, all cmds out have a required ack or response
         * so we'll just set to a non-IDLE state and wait for the CLK
         */
        xt_state = PS2_ST_WAIT_RESPONSE;
        xt_enable_clk_fall();
      } else {
        // wait for another cycle.  We should timeout here, I think
      }
      break;
    default:
      break;
  }
}

static void xt_host_init(void) {
  xt_enable_clk_fall();
}
#endif

#ifdef PS2_ENABLE_DEVICE
static void xt_device_check_data(void) {
  // do we have data to send?
  if(xt_data_to_send()) {
    xt_trigger_send();
  } else {
    xt_state = PS2_ST_IDLE;
    xt_disable_timer();
    xt_enable_clk_fall();
  }
}

static void xt_device_host_inhibit(void) {
  // CLK is low.  Host wants to talk to us.
  // turn off timer
  xt_disable_timer();
  // look for rising clock
  xt_enable_clk_rise();
  xt_state = PS2_ST_HOST_INHIBIT;
  // release DATA line, if we happen to have it.
  xt_set_data();
}


static void xt_device_timer_irq(void) __attribute__((always_inline));
static inline void xt_device_timer_irq(void) {
  switch (xt_state) {
    case PS2_ST_PREP_START:
      // disable the CLK IRQ
      xt_disable_clk();
      // clk the start bit
      xt_clear_clk();
      xt_state = PS2_ST_SEND_START;
      break;
    case PS2_ST_SEND_START:
      xt_read_byte();
      xt_set_clk();  // bring CLK hi
      if(xt_read_clk()) {
        xt_write_bit();
      } else {
        xt_device_host_inhibit();
      }
      break;
    case PS2_ST_PREP_BIT:
      xt_clear_clk();
      xt_state = PS2_ST_SEND_BIT;
      break;
    case PS2_ST_SEND_BIT:
      xt_set_clk();  // bring CLK hi
      if(xt_read_clk()) {
        if(xt_bit_count == 8) {
          // we are done..., do parity
          xt_write_parity();
          xt_state = PS2_ST_PREP_PARITY;
        } else {
          // state is set in function.
          xt_write_bit();
        }
      } else {
        xt_device_host_inhibit();
      }
      break;
    case PS2_ST_PREP_PARITY:
      // clock parity
      xt_clear_clk();
      xt_state = PS2_ST_SEND_PARITY;
      break;
    case PS2_ST_SEND_PARITY:
      xt_set_clk();  // bring CLK hi
      if(xt_read_clk()) {
        xt_set_data();
        xt_state = PS2_ST_PREP_STOP;
      } else {
        xt_device_host_inhibit();
      }
      break;
    case PS2_ST_PREP_STOP:
      xt_clear_clk();
      xt_state = PS2_ST_SEND_STOP;
      break;
    case PS2_ST_SEND_STOP:
      // If host wanted to abort, they had to do it before now.
      xt_commit_read_byte();
      xt_set_clk();  // bring CLK hi
      if(xt_read_clk()) {
        if(xt_read_data()) {
          // for some reason, you have to wait a while before sending again.
          xt_holdoff_count=PS2_SEND_HOLDOFF_COUNT;
          xt_state = PS2_ST_HOLDOFF;
        } else {
          // Host wants to talk to us.
          xt_state = PS2_ST_WAIT_START;
        }
      } else {
        xt_device_host_inhibit();
      }
      break;
    case PS2_ST_WAIT_START:
      // set CLK lo
      xt_clear_clk();
      xt_clear_counters();
      // read start bit
      if(xt_read_data()) {
        // not sure what you do if start bit is high...
        xt_set_clk();
        xt_state = PS2_ST_IDLE;
        xt_disable_timer();
        xt_enable_clk_fall();
      } else {
        xt_state = PS2_ST_GET_START;
      }
      break;
    case PS2_ST_GET_START:
      xt_set_clk();  // bring CLK hi
      if(xt_read_clk()) {
        xt_state = PS2_ST_WAIT_BIT;
      } else {
        xt_device_host_inhibit();
      }
      break;
    case PS2_ST_WAIT_BIT:
      xt_clear_clk();
      // you read incoming bits on falling clock.
      xt_read_bit();
      xt_state = PS2_ST_GET_BIT;
      break;
    case PS2_ST_GET_BIT:
      xt_set_clk();  // bring CLK hi
      if(xt_read_clk()) {
        if(xt_bit_count == 8) {
          // done, do Parity bit
          xt_state = PS2_ST_GET_PARITY;
        } else {
          xt_state = PS2_ST_WAIT_BIT;
        }
      } else {
        // host aborted send.
        xt_device_host_inhibit();
      }
      break;
    case PS2_ST_GET_PARITY:
      xt_clear_clk();
      // ignore parity for now.
      xt_state = PS2_ST_WAIT_STOP;
      break;
    case PS2_ST_WAIT_STOP:
      xt_set_clk();  // bring CLK hi
      if(xt_read_clk()) {
        if(xt_read_data()) {
          xt_state = PS2_ST_WAIT_ACK;
          // bing DATA low to ack
          xt_clear_data();
          // commit data
          //xt_write_byte();  jlb, moved.
        } else {
          xt_state = PS2_ST_GET_PARITY;
        }
      } else {
        // host aborted send.
        xt_device_host_inhibit();
      }
      break;
    case PS2_ST_WAIT_ACK:
      xt_clear_clk();
      xt_state = PS2_ST_GET_ACK;
      break;
    case PS2_ST_GET_ACK:
      xt_set_clk();
      xt_set_data();
      // we just need to wait a 50uS or so, to ensure the host saw the CLK go high
      xt_holdoff_count = 1;
      xt_state = PS2_ST_HOLDOFF;
      xt_write_byte();   //jlb moved
      break;
    case PS2_ST_HOLDOFF:
      xt_holdoff_count--;
      if(!xt_holdoff_count) {
        if(xt_read_clk()) {
          if(xt_read_data()) {
            xt_device_check_data();
          } else {
            xt_state = PS2_ST_WAIT_START;
          }
        } else {
          xt_device_host_inhibit();
        }
      }
      break;
    default:
      xt_disable_timer();
      break;
  }
}


static void xt_device_clk_irq(void) __attribute__((always_inline));
static inline void xt_device_clk_irq(void) {
  xt_disable_clk();

  switch(xt_state) {
    case PS2_ST_IDLE:
    case PS2_ST_PREP_START:
      // host is holding us off.  Wait for CLK hi...
      xt_device_host_inhibit();
      break;
    case PS2_ST_HOST_INHIBIT:
      // CLK went hi
      if(xt_read_data()) {
        // we can send if we need to.
        xt_device_check_data();
      } else {
        // host wants to send data, CLK is high.
        // wait half cycle to let things settle.
        // clock in data from host.
        xt_enable_timer(PS2_HALF_CYCLE);
        xt_state = PS2_ST_WAIT_START;
      }
      break;
    default:
      break;
  }
}

static void xt_device_init(void) {
  xt_disable_clk();
  xt_disable_timer();
  xt_set_clk();
  xt_set_data();
  // wait 600mS.
  _delay_ms(600);
  xt_putc(PS2_CMD_BAT);
}
#endif

ISR(PS2_TIMER_COMP_vect) {
  PS2_CALL(xt_device_timer_irq(),xt_host_timer_irq());
}

ISR(CLK_INT_vect) {
  PS2_CALL(xt_device_clk_irq(),xt_host_clk_irq());
}

uint8_t xt_getc( void ) {
  uint8_t tmptail;

  while ( rx_head == rx_tail ) {
    // wait for char to arrive, if none in Q
    ;
  }
  // Calculate buffer index
  tmptail = ( rx_tail + 1 ) & PS2_RX_BUFFER_MASK;
  // Store new index
  rx_tail = tmptail;
  return rxbuf[tmptail];
}

void xt_putc( uint8_t data ) {
  uint8_t tmphead;
  // Calculate buffer index
  tmphead = ( tx_head + 1 ) & PS2_TX_BUFFER_MASK;
  while ( tmphead == tx_tail ) {
    // Wait for free space in buffer
    ;
  }
  // Store data in buffer
  txbuf[tmphead] = data;
  // Store new index
  tx_head = tmphead;

  // turn off IRQs
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    if(xt_state == PS2_ST_IDLE) {
      // start transmission;
      xt_trigger_send();
    }
  }
}

uint8_t xt_data_available( void ) {
  return ( rx_head != rx_tail ); /* Return 0 (FALSE) if the receive buffer is empty */
}

void xt_init(ps2mode_t mode) {
  // set prescaler to System Clock/8
  PS2_TCCR = PS2_TCCR_DATA;

  xt_mode = mode;
  xt_clear_buffers();

  xt_set_clk();
  xt_set_data();

  xt_state = PS2_ST_IDLE;
  PS2_CALL(xt_device_init(),xt_host_init());
}
