/* 
*/

#ifndef CONFIG_H
#define CONFIG_H

#include "autoconf.h"

#define TRUE  1
#define FALSE 0

#define PS2_RX_BUFFER_SHIFT   3
#define PS2_TX_BUFFER_SHIFT   3

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
