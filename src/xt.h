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

    ps2.h: public functions and KEY definitions

*/

#ifndef XT_H
#define XT_H

typedef enum { XT_MODE_DEVICE = 1, XT_MODE_HOST = 2 } ps2mode_t;

#define XT_KEY_UP            0xf0
#define XT_KEY_EXT           0xe0
#define XT_KEY_EXT_2         0xe1


// normal keys
#define XT_KEY_F5            0x03
#define XT_KEY_F3            0x04
#define XT_KEY_F1            0x05
#define XT_KEY_F2            0x06
#define XT_KEY_F8            0x0a
#define XT_KEY_F6            0x0b
#define XT_KEY_F4            0x0c
#define XT_KEY_TAB           0x0d
#define XT_KEY_BACKQUOTE     0x0e
#define XT_KEY_ALT           0x11
#define XT_KEY_LSHIFT        0x12
#define XT_KEY_LCTRL         0x14
#define XT_KEY_Q             0x15
#define XT_KEY_1             0x16
#define XT_KEY_Z             0x1a
#define XT_KEY_S             0x1b
#define XT_KEY_A             0x1c
#define XT_KEY_W             0x1d
#define XT_KEY_2             0x1e
#define XT_KEY_C             0x21
#define XT_KEY_X             0x22
#define XT_KEY_D             0x23
#define XT_KEY_E             0x24
#define XT_KEY_4             0x25
#define XT_KEY_3             0x26
#define XT_KEY_SPACE         0x29
#define XT_KEY_V             0x2a
#define XT_KEY_F             0x2b
#define XT_KEY_T             0x2c
#define XT_KEY_R             0x2d
#define XT_KEY_5             0x2e
#define XT_KEY_N             0x31
#define XT_KEY_B             0x32
#define XT_KEY_H             0x33
#define XT_KEY_G             0x34
#define XT_KEY_Y             0x35
#define XT_KEY_6             0x36
#define XT_KEY_M             0x3a
#define XT_KEY_J             0x3b
#define XT_KEY_U             0x3c
#define XT_KEY_7             0x3d
#define XT_KEY_8             0x3e
#define XT_KEY_COMMA         0x41
#define XT_KEY_K             0x42
#define XT_KEY_I             0x43
#define XT_KEY_O             0x44
#define XT_KEY_0             0x45
#define XT_KEY_9             0x46
#define XT_KEY_PERIOD        0x49
#define XT_KEY_SLASH         0x4a
#define XT_KEY_L             0x4b
#define XT_KEY_SEMICOLON     0x4c
#define XT_KEY_P             0x4d
#define XT_KEY_MINUS         0x4e
#define XT_KEY_APOSTROPHE    0x52
#define XT_KEY_LBRACKET      0x54
#define XT_KEY_EQUALS        0x55
#define XT_KEY_CAPS_LOCK     0x58
#define XT_KEY_RSHIFT        0x59
#define XT_KEY_ENTER         0x5a
#define XT_KEY_RBRACKET      0x5b
#define XT_KEY_BACKSLASH     0x5d
#define XT_KEY_BS            0x66
#define XT_KEY_NUM_1         0x69
#define XT_KEY_NUM_4         0x6b
#define XT_KEY_NUM_7         0x6c
#define XT_KEY_NUM_0         0x70
#define XT_KEY_NUM_PERIOD    0x71
#define XT_KEY_NUM_2         0x72
#define XT_KEY_NUM_5         0x73
#define XT_KEY_NUM_6         0x74
#define XT_KEY_NUM_8         0x75
#define XT_KEY_ESC           0x76
#define XT_KEY_NUM_LOCK      0x77
#define XT_KEY_NUM_3         0x7a
#define XT_KEY_NUM_9         0x7d
#define XT_KEY_SCROLL_LOCK   0x7e
#define XT_KEY_F7            0x83

// extended keys
#define XT_KEY_RALT          0x11
#define XT_KEY_ECTRL         0x12
#define XT_KEY_RCTRL         0x14
#define XT_KEY_NUM_SLASH     0x4a
#define XT_KEY_NUM_ENTER     0x5a
#define XT_KEY_END           0x69
#define XT_KEY_CRSR_LEFT     0x6b
#define XT_KEY_HOME          0x6c
#define XT_KEY_INSERT        0x70
#define XT_KEY_DELETE        0x71
#define XT_KEY_CRSR_DOWN     0x72
#define XT_KEY_CRSR_RIGHT    0x74
#define XT_KEY_CRSR_UP       0x75
#define XT_KEY_PAGE_DOWN     0x7a
#define XT_KEY_PRINT_SCREEN  0x7c
#define XT_KEY_PAGE_UP       0x7d

// new ones
#define XT_KEY_PCTRL         0x14
#define XT_KEY_PAUSE         0x77


#define XT_CMD_LEDS          0xed
#define XT_CMD_ECHO          0xee
#define XT_CMD_SET_CODE_SET  0xf0
#define XT_CMD_READ_ID       0xf2
#define XT_CMD_SET_RATE      0xf3
#define XT_CMD_ENABLE        0xf4
#define XT_CMD_DISABLE       0xf5
#define XT_CMD_DEFAULT       0xf6
#define XT_CMD_RESEND        0xfe
#define XT_CMD_RESET         0xff

#define XT_CMD_ERROR         0x00
#define XT_CMD_BAT           0xaa
#define XT_CMD_ECHO_RESP     0xee
#define XT_CMD_ACK           0xfa
#define XT_CMD_BAT_FAILURE   0xfc
#define XT_CMD_OVERFLOW      0xff

#define XT_LED_SCROLL_LOCK   (1 << 0)
#define XT_LED_NUM_LOCK      (1 << 1)
#define XT_LED_CAPS_LOCK     (1 << 2)

#define XT_MS_CMD_RESET      XT_CMD_RESET
#define XT_MS_CMD_RESEND     XT_CMD_RESEND
#define XT_MS_CMD_ACK        XT_CMD_ACK
#define XT_MS_CMD_REPORT     0xf4
#define XT_MS_CMD_REPORTOFF  0xf5
#define XT_MS_CMD_SET_SAMPLE 0xf3
#define XT_MS_CMD_READ_ID    XT_CMD_READ_ID
#define XT_MS_CMD_READ_DATA  0xeb

#define XT_RX_BUFFER_MASK   (_BV(XT_RX_BUFFER_SHIFT) - 1)
#define XT_TX_BUFFER_MASK   (_BV(XT_TX_BUFFER_SHIFT) - 1)
/*
 * After a device sends a byte to host, it has to holdoff for a while
 * before doing anything else.  One KB I tested this is 2.14mS.
 */

/* PS2 Clock INT */
#if defined __AVR_ATmega8__ ||  defined __AVR_ATmega16__ || defined __AVR_ATmega32__ || defined __AVR_ATmega162__
#  define CLK_INTDR     MCUCR     // INT Direction Register
#  define CLK_INTCR     GICR      // INT Control Register
#  define CLK_INTFR     GIFR      // INT Flag Register
#  if XT_PIN_CLK == _BV(PD3)
#    define CLK_ISC0      ISC10
#    define CLK_ISC1      ISC11
#    define CLK_INT       INT1
#    define CLK_INTF      INTF1
#    define CLK_INT_vect  INT1_vect
#  else
#    define CLK_ISC0      ISC00
#    define CLK_ISC1      ISC01
#    define CLK_INT       INT0
#    define CLK_INTF      INTF0
#    define CLK_INT_vect  INT0_vect
#  endif
#else
#  define CLK_INTDR     EICRA     // INT Direction Register
#  define CLK_INTCR     EIMSK     // INT Control Register
#  define CLK_INTFR     EIFR      // INT Flag Register
#  if XT_CLK_PIN == _BV(PD3)
#    define CLK_ISC0      ISC10
#    define CLK_ISC1      ISC11
#    define CLK_INT       INT1
#    define CLK_INTF      INTF1
#    define CLK_INT_vect  INT1_vect
#  else
#    define CLK_ISC0      ISC00
#    define CLK_ISC1      ISC01
#    define CLK_INT       INT0
#    define CLK_INTF      INTF0
#    define CLK_INT_vect  INT0_vect
#  endif
#endif

/* PS2 Timer */
#if defined __AVR_ATmega8__

#  define XT_TIMER_COMP_vect   TIMER2_COMP_vect
#  define XT_OCR               OCR2
#  define XT_TCNT              TCNT2
#  define XT_TCCR              TCCR2
#  define XT_TCCR_DATA         _BV(CS21)
#  define XT_TIFR              TIFR
#  define XT_TIFR_DATA         _BV(OCF2)
#  define XT_TIMSK             TIMSK
#  define XT_TIMSK_DATA        _BV(OCIE2)

#elif defined __AVR_ATmega28__ || defined __AVR_ATmega48__ || defined __AVR_ATmega88__ || defined __AVR_ATmega168__

#  define XT_TIMER_COMP_vect   TIMER2_COMPA_vect
#  define XT_OCR               OCR2A
#  define XT_TCNT              TCNT2
#  define XT_TCCR              TCCR2B
#  define XT_TCCR_DATA         _BV(CS21)
#  define XT_TIFR              TIFR2
#  define XT_TIFR_DATA         _BV(OCF2A)
#  define XT_TIMSK             TIMSK2
#  define XT_TIMSK_DATA        _BV(OCIE2A)

#elif defined __AVR_ATmega16__ || defined __AVR_ATmega32__ || defined __AVR_ATmega162__

#  define XT_TIMER_COMP_vect   TIMER0_COMP_vect
#  define XT_OCR               OCR0
#  define XT_TCNT              TCNT0
#  define XT_TCCR              TCCR0
#  define XT_TCCR_DATA         (_BV(CS01) | _BV(WGM01))
#  define XT_TIFR              TIFR
#  define XT_TIFR_DATA         _BV(OCF0)
#  define XT_TIMSK             TIMSK
#  define XT_TIMSK_DATA        _BV(OCIE0)

#else
#  error Unknown chip!
#endif



#define XT_HALF_CYCLE 40 // ~42 uS when all is said and done.
#define XT_SEND_HOLDOFF_COUNT  ((uint8_t)(2140/XT_HALF_CYCLE))

typedef enum {XT_ST_IDLE
             ,XT_ST_PREP_START
             ,XT_ST_SEND_START
             ,XT_ST_PREP_BIT
             ,XT_ST_SEND_BIT
             ,XT_ST_PREP_PARITY
             ,XT_ST_SEND_PARITY
             ,XT_ST_PREP_STOP
             ,XT_ST_SEND_STOP
             ,XT_ST_HOLDOFF
             ,XT_ST_WAIT_START
             ,XT_ST_GET_START
             ,XT_ST_WAIT_BIT
             ,XT_ST_GET_BIT
             ,XT_ST_WAIT_PARITY
             ,XT_ST_GET_PARITY
             ,XT_ST_WAIT_STOP
             ,XT_ST_GET_STOP
             ,XT_ST_GET_ACK
             ,XT_ST_WAIT_ACK
             ,XT_ST_WAIT_ACK2
             ,XT_ST_HOST_INHIBIT
             ,XT_ST_WAIT_RESPONSE
             } ps2state_t;

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

void xt_init(ps2mode_t mode);
uint8_t xt_getc(void);
void xt_putc(uint8_t data);
uint8_t xt_data_available(void);
void xt_handle_cmds(uint8_t data);
uint16_t xt_get_typematic_delay(uint8_t rate);
uint16_t xt_get_typematic_period(uint8_t rate);
void xt_clear_buffers(void);

// Add 1 and multiply by 250ms to get time
#define XT_GET_DELAY(rate)   ((rate & 0x60) >> 5)
// Multiply by 4.17 to get CPS (or << 2)
#define XT_GET_RATE(rate)    ((8 + (rate & 0x07)) * (1 << ((rate & 0x18) >> 3)))
#define CALC_RATE(delay,rate) ((rate & 0x1f) + ((delay & 0x03) << 5))

#endif

