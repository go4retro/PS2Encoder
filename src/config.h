/*
    PS2Encoder - PS2 Keyboard to serial/parallel converter
    Copyright Jim Brain and RETRO Innovations, 2008-2011

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

    config.h: User-configurable options to simplify hardware changes and/or
              reduce the code/ram requirements of the code.
*/

#ifndef CONFIG_H
#define CONFIG_H

#include <avr/io.h>
#include "autoconf.h"

#define PS2_ENABLE_HOST
#define PS2_ENABLE_DEVICE

#ifndef TRUE
#define FALSE                 0
#define TRUE                  (!FALSE)
#endif

// log2 of the PS2 buffer size, i.e. 6 for 64, 7 for 128, 8 for 256 etc.
#define PS2_RX_BUFFER_SHIFT   3
#define PS2_TX_BUFFER_SHIFT   3

#define ENABLE_UART0
// log2 of the UART buffer size, i.e. 6 for 64, 7 for 128, 8 for 256 etc.
#define UART0_TX_BUFFER_SHIFT  3

#define UART0_BAUDRATE CONFIG_UART_BAUDRATE
#define DYNAMIC_UART

#if CONFIG_HARDWARE_VARIANT==1
#  define PS2_CLK_DDR    DDRD
#  define PS2_CLK_OUT    PORTD
#  define PS2_CLK_IN     PIND
#  define PS2_CLK_PIN    _BV(PD2)
#  define PS2_DATA_DDR   DDRD
#  define PS2_DATA_OUT   PORTD
#  define PS2_DATA_IN    PIND
#  define PS2_DATA_PIN   _BV(PD3)

static inline __attribute__((always_inline)) void data_init(void) {
  DDRB |= 0x0f;
  PORTB &= ~0x0f;
  DDRC |= 0x0f;
  PORTC &= ~0x0f;
  DDRD  |= _BV(PD7); // strobe
}

static inline __attribute__((always_inline)) void data_out(uint8_t c) {
  PORTB = (PORTB & 0xf0) | (c & 0x0f);
  PORTC = (PORTC & 0xf0) | ((c & 0xf0) >> 4);
}

static inline __attribute__((always_inline)) void data_strobe_hi(void) {
  PORTD |= _BV(PD7);
}

static inline __attribute__((always_inline)) void data_strobe_lo(void) {
  PORTD &= ~_BV(PD7);
}

static inline __attribute__((always_inline)) void reset_init(void) {
  DDRD |= _BV(PD6);
  PORTD |= _BV(PD6);
}

static inline __attribute__((always_inline)) void reset_set_hi(void) {
  PORTD |= _BV(PD6);
}

static inline __attribute__((always_inline)) void reset_set_lo(void) {
  PORTD &= ~_BV(PD6);
}

static inline __attribute__((always_inline)) void mode_init(void) {
  DDRD &= ~_BV(PD5);
  PORTD |= _BV(PD5);
  DDRD &= ~_BV(PD4);
  PORTD |= _BV(PD4);
}

// this must return non-zero for config mode
static inline __attribute__((always_inline)) uint8_t mode_config(void) {
  return !(PIND & _BV(PD5));
}

// this must return non-zero for device mode
static inline __attribute__((always_inline)) uint8_t mode_device(void) {
  return !(PIND & _BV(PD4));
}

#  define SW_RX_BUFFER_SHIFT  2
#  define PORT_SW_OUT         PORTB
#  define PORT_SW_IN          PINB
#  define PORT_SW_DDR         DDRB
#  define SW_A                (PB4)
#  define SW_B                (PB5)
// can't use with Xtal
//#  define SW_C                (PB6)
//#  define SW_D                (PB7)

#  define MAT_RX_BUFFER_SHIFT 4
#  define MAT_ROW_LO_DDR      DDRB
#  define MAT_ROW_LO_OUT      PORTB
#  define MAT_ROW_MASK        0x0f
#  define MAT_COL_LO_DDR      DDRC
#  define MAT_COL_LO_OUT      PORTC
#  define MAT_COL_LO_IN       PINC
#  define MAT_COL_MASK        0x0f

static inline __attribute__((always_inline)) void timer_init (void) {
  TCCR0A |= _BV(WGM01);
  TCCR0B |= (_BV(CS02) | _BV(CS00));
  OCR0A = 65;
  TIMSK0 |= _BV(OCIE0A);
}

#  define TIMER_vect          TIMER0_COMPA_vect
#endif

#endif /*CONFIG_H*/
