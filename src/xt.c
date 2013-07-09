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
    and http://kbdbabel.sourceforge.net/doc/kbd_signaling_pcxt_ps2_adb.pdf
*/

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <util/delay.h>
#include "config.h"
#include "xt.h"
#include "uart.h"

static uint8_t buf[1 << XT_BUFFER_SHIFT];
static volatile uint8_t head;
static volatile uint8_t tail;

static volatile xtstate_t xt_state;
static volatile uint8_t xt_byte;
static volatile uint8_t xt_bit_count;

static xtmode_t xt_mode;

static volatile uint8_t xt_holdoff_count;

static void xt_enable_clk_rise(void) {
  // turn off IRQ
  XT_CLK_INTCR &= (uint8_t)~_BV(XT_CLK_INT);
  // reset flag
  XT_CLK_INTFR |= _BV(XT_CLK_INTF);
#if XT_CLK_PIN == _BV(PB5)
  PCMSK0 |= _BV(PCINT5);
#else
  // rising edge
  XT_CLK_INTDR |= _BV(XT_CLK_ISC1) | _BV(XT_CLK_ISC0);
#endif
  // turn on
  XT_CLK_INTCR |= _BV(XT_CLK_INT);
}

static void xt_enable_clk_fall(void) {
  // turn off IRQ
  XT_CLK_INTCR &= (uint8_t)~_BV(XT_CLK_INT);
  // reset flag
  XT_CLK_INTFR |= _BV(XT_CLK_INTF);
  // falling edge
#  if XT_CLK_PIN == _BV(PB5)
  PCMSK0 |= _BV(PCINT5);
#  else
  XT_CLK_INTDR = (XT_CLK_INTDR & (uint8_t)~(_BV(XT_CLK_ISC1) | _BV(XT_CLK_ISC0))) | _BV(XT_CLK_ISC1);
#  endif
  // turn on
  XT_CLK_INTCR |= _BV(XT_CLK_INT);
}

static inline __attribute__((always_inline)) void xt_disable_clk(void) {
  XT_CLK_INTCR &= (uint8_t)~_BV(XT_CLK_INT);
}

static void xt_enable_timer(uint8_t us) {
  // clear flag.
  XT_TIFR |= XT_TIFR_DATA;
  // clear TCNT;
  XT_TCNT = 0;
  // set the count...
#if F_CPU > 14000000
  // us is uS....  Need to * 14 to get ticks, then divide by 8...
  // cheat... * 14 / 8 = *2 = <<1
  XT_OCR = (uint8_t)(us << 1);
#elif F_CPU > 7000000
  XT_OCR = us;
#else
  XT_OCR = (us >> 1);
#endif
  // enable output compare IRQ
  XT_TIMSK |= XT_TIMSK_DATA;
}

static inline __attribute__((always_inline)) void xt_disable_timer(void) {
  // disable output compare IRQ
  XT_TIMSK &= (uint8_t)~XT_TIMSK_DATA;
}

void xt_clear_buffers(void) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    head = 0;
    tail = 0;
  }
}

#ifdef XT_ENABLE_HOST
static void xt_clear_counters(void) {
  xt_byte = 0;
  xt_bit_count = 0;
}

static void xt_read_bit(void) {
  xt_byte = xt_byte >> 1;
  xt_bit_count++;
  if(xt_read_data()) {
    xt_byte |= 0x80;
  }
}

static void xt_write_byte(void) {
  /* Calculate buffer index and store */
  head = ( head + 1 ) & XT_BUFFER_MASK;

  if ( head == tail ) {
    /* ERROR! Receive buffer overflow */
  }
  buf[head] = xt_byte; /* Store received data in buffer */
}

static inline void xt_host_timer_irq(void) {
  xt_disable_timer();
  switch (xt_state) {
    case XT_ST_GET_START:
    case XT_ST_GET_BIT:
      // something must have happened, and we should reset.
      xt_state = XT_ST_IDLE;
      break;
    default:
      break;
  }
}


static inline __attribute__((always_inline)) void xt_host_clk_irq(void) {
  // if we don't get another CLK in 200uS, timeout.
  xt_enable_timer(200);
  switch(xt_state) {
    case XT_ST_IDLE:  // CLK fell at IDLE.
      if(xt_read_data()) {
        xt_state = XT_ST_GET_BIT;
        xt_clear_counters();
      } else
        xt_state = XT_ST_GET_START;
      break;
    case XT_ST_GET_START:
      if(xt_read_data()) {
        xt_state = XT_ST_GET_BIT;
        xt_clear_counters();
      }
      break;
    case XT_ST_GET_BIT:
      // read bit;
      xt_read_bit();
      if(xt_bit_count == 8) {
        // done
        xt_write_byte();
        xt_disable_timer(); // disable watchdog timer.
        xt_state = XT_ST_IDLE;
      }
      break;
    default:
      break;
  }
}

static void xt_host_init(void) {
}
#endif

#ifdef XT_ENABLE_DEVICE
static void xt_write_bit(void) {
  xt_state=XT_ST_PREP_BIT;
  // set DATA..
  switch (xt_byte & 1) {
    case 0:
      xt_clear_data();
      break;
    case 1:
      xt_set_data();
      break;
  }
  // shift right.
  xt_byte= xt_byte >> 1;
  xt_bit_count++;
  // valid data now.
}

static void xt_read_byte(void) {
  xt_bit_count = 0;
  xt_byte = buf[( tail + 1 ) & XT_BUFFER_MASK];  /* Start transmission */
}

static void xt_commit_read_byte(void) {
  tail = ( tail + 1 ) & XT_BUFFER_MASK;      /* Store new index */
}

static void xt_device_holdoff(void) {
  xt_enable_clk_rise();
  xt_set_clk();  // bring CLK hi
  xt_state = XT_ST_HOLDOFF;
  xt_holdoff_count = 0;
  xt_enable_timer(100);
}

static void xt_trigger_send(void) {
  if(!xt_read_data()) { // we are being held off
    xt_device_holdoff();
  } else {
    // set state
    xt_state = XT_ST_PREP_PSTART;
    // bring DATA line low to comply with defacto protocol.
    xt_clear_data();
    // start clocking.
    // wait a half cycle
    xt_enable_timer(XT_HALF_CYCLE);
  }
}

static void xt_device_check_data(void) {
  // do we have data to send?
  if( head != tail ) {
    xt_trigger_send();
  } else {
    xt_state = XT_ST_IDLE;
    xt_disable_timer();
    xt_enable_clk_fall();
  }
}

static inline __attribute__((always_inline)) void xt_device_timer_irq(void) {
  switch (xt_state) {
    case XT_ST_PREP_PSTART:  //1
      // clk the pseudo start bit, which is already set low.
      xt_clear_clk();
      xt_state = XT_ST_SEND_PSTART;
      break;
    case XT_ST_SEND_PSTART:  //2
      xt_set_clk();  // bring CLK hi
      xt_set_data(); // set start bit
      xt_state = XT_ST_PREP_START;
      break;
    case XT_ST_PREP_START:  //3
      // clk the start bit
      xt_clear_clk();
      xt_state = XT_ST_SEND_START;
      break;
    case XT_ST_SEND_START:  //4
      xt_read_byte();
      xt_set_clk();  // bring CLK hi
      // state is set in function.
      xt_write_bit();
      break;
    case XT_ST_PREP_BIT:  //5
      xt_clear_clk();
      xt_state = XT_ST_SEND_BIT;
      break;
    case XT_ST_SEND_BIT:  //6
      if(xt_bit_count == 8) {
        // we are done
        xt_commit_read_byte();
        xt_set_data();
        // host will take CLK low until it reads the byte.
        // wait for CLK to go high
        xt_device_holdoff();
      } else {
        xt_set_clk();  // bring CLK hi
        // state is set in function.
        xt_write_bit();
      }
      break;
    case XT_ST_HOLDOFF:
      // we timed out
      if(xt_holdoff_count == 0)
        uart_putc('T');
      if(xt_holdoff_count < 254)
        xt_holdoff_count++;
      break;
    default:
      xt_disable_timer();
      break;
  }
}

static inline __attribute__((always_inline)) void xt_device_clk_irq(void) {
  xt_disable_clk();

  switch(xt_state) {
    case XT_ST_IDLE:
      uart_putc('H');
      // host is holding us off.  Might be trying to reset. Wait for CLK hi...
      xt_device_holdoff();
      break;
    case XT_ST_HOLDOFF:
      xt_disable_timer();
      if(xt_holdoff_count >= 60) {
        xt_putc(0xaa);  // TODO fix this so this bytes goes out next.
      }
      if (xt_read_data()) {
        xt_device_check_data();
      } else
        xt_state = XT_ST_IDLE;
      break;
    default:
      break;
  }
}

static void xt_device_init(void) {
}
#endif

ISR(XT_TIMER_COMP_vect) {
  XT_CALL(xt_device_timer_irq(),xt_host_timer_irq());
}

ISR(XT_CLK_INT_vect) {
  XT_CALL(xt_device_clk_irq(),xt_host_clk_irq());
}

#ifdef XT_ENABLE_HOST
uint8_t xt_getc( void ) {
  while ( head == tail ) {
    // wait for char to arrive, if none in Q
    ;
  }
  // Calculate buffer index and store
  tail = ( tail + 1 ) & XT_BUFFER_MASK;
  return buf[tail];
}

uint8_t xt_data_available( void ) {
  return ( head != tail ); /* Return 0 (FALSE) if the receive buffer is empty */
}
#endif

#ifdef XT_ENABLE_DEVICE
void xt_putc( uint8_t data ) {

  uint8_t tmphead;

  // Calculate buffer index
  tmphead = ( head + 1 ) & XT_BUFFER_MASK;
  while ( tmphead == tail ) {
    // Wait for free space in buffer
    ;
  }
  // Store data in buffer
  buf[tmphead] = data;
  // Store new index
  head = tmphead;

  // turn off IRQs
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    if(xt_state == XT_ST_IDLE) {
      // start transmission;
      xt_trigger_send();
    }
  }
}
#endif

void xt_init(xtmode_t mode) {
  xt_init_timer();

  xt_mode = mode;
  xt_clear_buffers();

  xt_set_clk();
  xt_set_data();

  xt_state = XT_ST_IDLE;
  XT_CALL(xt_device_init(),xt_host_init());
  xt_enable_clk_fall();
}
