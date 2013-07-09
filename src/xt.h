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

    xt.h: public functions and KEY definitions

*/

#ifndef XT_H
#define XT_H

//#define XT_ENABLE_HOST
#define XT_ENABLE_DEVICE


#ifndef XT_BUFFER_SHIFT
#define XT_BUFFER_SHIFT     5
#endif

#ifndef XT_CLK_DDR
#  define XT_CLK_DDR        DDRB
#  define XT_CLK_OUT        PORTB
#  define XT_CLK_IN         PINB
#  define XT_CLK_PIN        _BV(PB5)
#  define XT_DATA_DDR       DDRB
#  define XT_DATA_OUT       PORTB
#  define XT_DATA_IN        PINB
#  define XT_DATA_PIN       _BV(PB4)
#endif

typedef enum { XT_MODE_DEVICE = 1, XT_MODE_HOST = 2 } xtmode_t;

#define XT_KEY_EXT          0xe0
#define XT_KEY_EXT_2        0xe1

// normal keys
#define XT_KEY_ESC          0x01
#define XT_KEY_1            0x02
#define XT_KEY_2            0x03
#define XT_KEY_3            0x04
#define XT_KEY_4            0x05
#define XT_KEY_5            0x06
#define XT_KEY_6            0x07
#define XT_KEY_7            0x08
#define XT_KEY_8            0x09
#define XT_KEY_9            0x0A
#define XT_KEY_0            0x0B
#define XT_KEY_MINUS        0x0C
#define XT_KEY_EQUALS       0x0D
#define XT_KEY_BS           0x0E
#define XT_KEY_TAB          0x0F
#define XT_KEY_Q            0x10
#define XT_KEY_W            0x11
#define XT_KEY_E            0x12
#define XT_KEY_R            0x13
#define XT_KEY_T            0x14
#define XT_KEY_Y            0x15
#define XT_KEY_U            0x16
#define XT_KEY_I            0x17
#define XT_KEY_O            0x18
#define XT_KEY_P            0x19
#define XT_KEY_LBRACKET     0x1A
#define XT_KEY_RBRACKET     0x1B
#define XT_KEY_ENTER        0x1C
#define XT_KEY_LCTRL        0x1D
#define XT_KEY_A            0x1E
#define XT_KEY_S            0x1F
#define XT_KEY_D            0x20
#define XT_KEY_F            0x21
#define XT_KEY_G            0x22
#define XT_KEY_H            0x23
#define XT_KEY_J            0x24
#define XT_KEY_K            0x25
#define XT_KEY_L            0x26
#define XT_KEY_SEMICOLON    0x27
#define XT_KEY_APOSTROPHE   0x28
#define XT_KEY_BACKQUOTE    0x29
#define XT_KEY_LSHIFT       0x2A
#define XT_KEY_BACKSLASH    0x2B
#define XT_KEY_INT2         0x2B
#define XT_KEY_Z            0x2C
#define XT_KEY_X            0x2D
#define XT_KEY_C            0x2E
#define XT_KEY_V            0x2F
#define XT_KEY_B            0x30
#define XT_KEY_N            0x31
#define XT_KEY_M            0x32
#define XT_KEY_COMMA        0x33
#define XT_KEY_PERIOD       0x34
#define XT_KEY_SLASH        0x35
#define XT_KEY_RSHIFT       0x36
#define XT_KEY_NUM_STAR     0x37
#define XT_KEY_ALT          0x38
#define XT_KEY_SPACE        0x39
#define XT_KEY_CAPS_LOCK    0x3A
#define XT_KEY_F1           0x3B
#define XT_KEY_F2           0x3C
#define XT_KEY_F3           0x3D
#define XT_KEY_F4           0x3E
#define XT_KEY_F5           0x3F
#define XT_KEY_F6           0x40
#define XT_KEY_F7           0x41
#define XT_KEY_F8           0x42
#define XT_KEY_F9           0x43
#define XT_KEY_F10          0x44
#define XT_KEY_NUM_LOCK     0x45
#define XT_KEY_SCROLL_LOCK  0x46
#define XT_KEY_NUM_7        0x47
#define XT_KEY_NUM_9        0x49
#define XT_KEY_NUM_MINUS    0x4A
#define XT_KEY_NUM_4        0x4B
#define XT_KEY_NUM_6        0x4D
#define XT_KEY_NUM_PLUS     0x4E
#define XT_KEY_NUM_1        0x4F
#define XT_KEY_NUM_8        0x48
#define XT_KEY_NUM_5        0x4C
#define XT_KEY_NUM_2        0x50
#define XT_KEY_NUM_3        0x51
#define XT_KEY_NUM_0        0x52
#define XT_KEY_NUM_PERIOD   0x53
#define XT_KEY_INT1         0x56
#define XT_KEY_F11          0x57
#define XT_KEY_F12          0x58


// extended keys
#define XT_KEY_NUM_ENTER    0x1c
#define XT_KEY_RCTRL        0x1d
#define XT_KEY_NUM_SLASH    0x35
#define XT_KEY_RALT         0x38
#define XT_KEY_HOME         0x47
#define XT_KEY_CRSR_UP      0x48
#define XT_KEY_PAGE_UP      0x49
#define XT_KEY_CRSR_LEFT    0x4b
#define XT_KEY_CRSR_RIGHT   0x4d
#define XT_KEY_END          0x4f
#define XT_KEY_CRSR_DOWN    0x50
#define XT_KEY_PAGE_DOWN    0x51
#define XT_KEY_INSERT       0x52
#define XT_KEY_DELETE       0x53

//significantly extended keys
//E02A E037
#define XT_KEY_PRINT_SCREEN 0x37
//E11D 45 E19D C5
#define XT_KEY_PAUSE        0x45


#define XT_BUFFER_MASK   (_BV(XT_BUFFER_SHIFT) - 1)

/* PS2 Clock INT */
#if defined __AVR_ATmega8__ ||  defined __AVR_ATmega16__ || defined __AVR_ATmega32__ || defined __AVR_ATmega162__
//#  define XT_CLK_INTDR     MCUCR     // INT Direction Register
//#  define XT_CLK_INTCR     GICR      // INT Control Register
//#  define XT_CLK_INTFR     GIFR      // INT Flag Register
//#  if XT_PIN_CLK == _BV(PD3)
//#    define XT_CLK_ISC0      ISC10
//#    define XT_CLK_ISC1      ISC11
//#    define XT_CLK_INT       INT1
//#    define XT_CLK_INTF      INTF1
//#    define XT_CLK_INT_vect  INT1_vect
//#  else
//#    define XT_CLK_ISC0      ISC00
//#    define XT_CLK_ISC1      ISC01
//#    define XT_CLK_INT       INT0
//#    define XT_CLK_INTF      INTF0
//#    define XT_CLK_INT_vect  INT0_vect
//#  endif
#elif defined __AVR_ATmega28__ || defined __AVR_ATmega48__ || defined __AVR_ATmega88__ || defined __AVR_ATmega168__
#  if XT_CLK_PIN == _BV(PD3)
#    define XT_CLK_INTDR     EICRA     // INT Direction Register
#    define XT_CLK_INTCR     EIMSK     // INT Control Register
#    define XT_CLK_INTFR     EIFR      // INT Flag Register
#    define XT_CLK_ISC0      ISC10
#    define XT_CLK_ISC1      ISC11
#    define XT_CLK_INT       INT1
#    define XT_CLK_INTF      INTF1
#    define XT_CLK_INT_vect  INT1_vect
#  elif XT_CLK_PIN == _BV(PD2)
#    define XT_CLK_INTDR     EICRA     // INT Direction Register
#    define XT_CLK_INTCR     EIMSK     // INT Control Register
#    define XT_CLK_INTFR     EIFR      // INT Flag Register
#    define XT_CLK_ISC0      ISC00
#    define XT_CLK_ISC1      ISC01
#    define XT_CLK_INT       INT0
#    define XT_CLK_INTF      INTF0
#    define XT_CLK_INT_vect  INT0_vect
#  elif XT_CLK_PIN == _BV(PB5) // should add more pins to this.
#    define XT_CLK_INTFR     PCIFR     // INT Flag Register
#    define XT_CLK_INTF      PCIF0
#    define XT_CLK_INTCR     PCICR
#    define XT_CLK_INT       PCIE0
#    define XT_CLK_INT_vect  PCINT0_vect
#  endif
#endif

/* XT Timer */
#if defined __AVR_ATmega8__

#  define XT_TIMER_COMP_vect   TIMER2_COMP_vect
#  define XT_OCR               OCR2
#  define XT_TCNT              TCNT2
#  define XT_TCCR1             TCCR2
#  define XT_TCCR1_DATA        _BV(CS21)
#  define XT_TCCR2             TCCR2
#  define XT_TCCR2_DATA        _BV(WGM21)
#  define XT_TIFR              TIFR
#  define XT_TIFR_DATA         _BV(OCF2)
#  define XT_TIMSK             TIMSK
#  define XT_TIMSK_DATA        _BV(OCIE2)

#elif defined __AVR_ATmega28__ || defined __AVR_ATmega48__ || defined __AVR_ATmega88__ || defined __AVR_ATmega168__
#  define XT_TIMER_COMP_vect   TIMER0_COMPA_vect
#  define XT_OCR               OCR0A
#  define XT_TCNT              TCNT0
#  define XT_TCCR1             TCCR0B
#  define XT_TCCR1_DATA        _BV(CS01)
#  define XT_TCCR2             TCCR0A
#  define XT_TCCR2_DATA        _BV(WGM01)
#  define XT_TIFR              TIFR0
#  define XT_TIFR_DATA         _BV(OCF0A)
#  define XT_TIMSK             TIMSK0
#  define XT_TIMSK_DATA        _BV(OCIE0A)

#elif defined __AVR_ATmega16__ || defined __AVR_ATmega32__ || defined __AVR_ATmega162__

#  define XT_TIMER_COMP_vect   TIMER0_COMP_vect
#  define XT_OCR               OCR0
#  define XT_TCNT              TCNT0
#  define XT_TCCR1             TCCR0
#  define XT_TCCR1_DATA        _BV(CS01)
#  define XT_TCCR2             TCCR0
#  define XT_TCCR2_DATA        _BV(WGM01)
#  define XT_TIFR              TIFR
#  define XT_TIFR_DATA         _BV(OCF0)
#  define XT_TIMSK             TIMSK
#  define XT_TIMSK_DATA        _BV(OCIE0)

#else
#  error Unknown chip!
#endif

#define XT_HALF_CYCLE 50

typedef enum {XT_ST_IDLE
             ,XT_ST_PREP_PSTART  // initial 0 on data line
             ,XT_ST_SEND_PSTART
             ,XT_ST_PREP_START
             ,XT_ST_SEND_START
             ,XT_ST_PREP_BIT
             ,XT_ST_SEND_BIT
             ,XT_ST_HOLDOFF
             ,XT_ST_GET_START
             ,XT_ST_GET_BIT
             } xtstate_t;

static inline __attribute__((always_inline)) void xt_init_timer(void) {
  // set prescaler to System Clock/8
  XT_TCCR1 |= XT_TCCR1_DATA;
  // CTC mode
  XT_TCCR2 |= XT_TCCR2_DATA;
}

static inline __attribute__((always_inline)) void xt_set_clk(void) {
  XT_CLK_OUT |= XT_CLK_PIN;
  XT_CLK_DDR &= (uint8_t)~XT_CLK_PIN;
}

static inline __attribute__((always_inline)) void xt_clear_clk(void) {
  XT_CLK_DDR |= XT_CLK_PIN;
  XT_CLK_OUT &= (uint8_t)~XT_CLK_PIN;
}

static inline __attribute__((always_inline)) uint8_t xt_read_clk(void) {
  return XT_CLK_IN & XT_CLK_PIN;
}

static inline __attribute__((always_inline)) void xt_set_data(void) {
  XT_DATA_OUT |= XT_DATA_PIN;
  XT_DATA_DDR &= (uint8_t)~XT_DATA_PIN;
}

static inline __attribute__((always_inline)) void xt_clear_data(void) {
  XT_DATA_DDR |= XT_DATA_PIN;
  XT_DATA_OUT &= (uint8_t)~XT_DATA_PIN;
}

static inline __attribute__((always_inline)) uint8_t xt_read_data(void) {
  return XT_DATA_IN & XT_DATA_PIN;
}

#if defined XT_ENABLE_HOST && defined XT_ENABLE_DEVICE
#define XT_CALL(dev,host) \
  switch(xt_mode) {\
  case XT_MODE_DEVICE: \
    dev; \
    break; \
  case XT_MODE_HOST: \
    host; \
    break; \
  }
#else
#  if defined XT_ENABLE_DEVICE
#    define XT_CALL(dev,host) dev
#  else
#    define XT_CALL(dev,host) host
#  endif
#endif

void xt_init(xtmode_t mode);
uint8_t xt_getc(void);
#ifdef XT_ENABLE_DEVICE
void xt_putc(uint8_t data);
#endif
uint8_t xt_data_available(void);
void xt_clear_buffers(void);

#endif

