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

#include "autoconf.h"

#define FALSE 0
#define TRUE  (!FALSE)

// log2 of the PS2 buffer size, i.e. 6 for 64, 7 for 128, 8 for 256 etc.
#define PS2_RX_BUFFER_SHIFT   3
#define PS2_TX_BUFFER_SHIFT   3

// log2 of the UART buffer size, i.e. 6 for 64, 7 for 128, 8 for 256 etc.
#define UART1_TX_BUFFER_SHIFT  3

#define ENABLE_UART1
#define UART1_BAUDRATE CONFIG_UART_BAUDRATE
#define DYNAMIC_BPS_RATE

#if CONFIG_HARDWARE_VARIANT==1

#  define DATA_SETDDR()       do { DDRB |= 0x0f; DDRC |= 0x0f; } while(0)
#  define DATA_OUT(x)         do { PORTB = (PORTB & 0xf0) | (x & 0x0f); PORTC = (PORTC & 0xf0) | ((x & 0xf0) >> 4); } while(0)

#  define RESET_SETDDR()      DDRD |= _BV(PD6)
#  define RESET_ACTIVE()      PORTD &= ~_BV(PD6)
#  define RESET_INACTIVE()    PORTD |= _BV(PD6)

#  define STROBE_SETDDR()     DDRD  |= _BV(PD7)
#  define STROBE_HI()         PORTD |= _BV(PD7)
#  define STROBE_LO()         PORTD &= ~_BV(PD7)

#  define CONF_MODE_SETDDR()  do { DDRD &= ~_BV(PD5) ; PORTD |= _BV(PD5); } while(0)
// this must return zero for config
#  define CONF_MODE()         (!(PIND & _BV(PD5)))

#  define PS2_PORT_DDR_CLK    DDRD
#  define PS2_PORT_CLK_OUT    PORTD
#  define PS2_PORT_CLK_IN     PIND
#  define PS2_PIN_CLK         _BV(PD3)
#  define PS2_PORT_DDR_DATA   DDRD
#  define PS2_PORT_DATA_OUT   PORTD
#  define PS2_PORT_DATA_IN    PIND
#  define PS2_PIN_DATA        _BV(PD2)

#endif

#endif
