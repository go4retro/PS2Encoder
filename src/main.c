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

    main.c: Main application
*/

#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "config.h"
#include "eeprom.h"
#include "flags.h"
#include "ps2.h"
#include "uart.h"

typedef enum {
              POLL_ST_IDLE,
              POLL_ST_GET_X_KEY,
              POLL_ST_GET_KEY_UP,
              POLL_ST_GET_X_KEY_UP,
              POLL_ST_GET_PAUSE_1,
              POLL_ST_GET_PAUSE_2,
              POLL_ST_GET_PAUSE_3,
              POLL_ST_GET_PAUSE_4,
              POLL_ST_GET_PAUSE_5,
              POLL_ST_GET_PAUSE_6,
              POLL_ST_GET_PAUSE_7
             } poll_state_t;

#define POLL_FLAG_LSHIFT      1
#define POLL_FLAG_RSHIFT      2
#define POLL_FLAG_SHIFT       (POLL_FLAG_LSHIFT | POLL_FLAG_RSHIFT)
#define POLL_FLAG_ALT         4
#define POLL_FLAG_CONTROL     8
#define POLL_FLAG_CTRL_ALT    (POLL_FLAG_CONTROL | POLL_FLAG_ALT)
#define POLL_FLAG_CAPS_LOCK   16
#define POLL_FLAG_NUM_LOCK    32
#define POLL_FLAG_SCROLL_LOCK 64

#define KB_CONFIG             1

static uint8_t meta;
static uint8_t config;
static uint8_t led_state=0;
uint8_t globalopts;
uint8_t holdoff;
uint16_t uart_bps;
uartlen_t uart_length;
uartpar_t uart_parity;
uartstop_t uart_stop;
uint8_t  type_delay;
uint8_t  type_rate;

static void send(uint8_t sh, uint8_t unshifted, uint8_t shifted) {
  uint8_t key;

  if(meta & POLL_FLAG_CONTROL) {
    unshifted = ((unshifted >= 'a' && unshifted <= 'z')?unshifted & 0x1f:unshifted);
  }
  key = (sh | ((meta & POLL_FLAG_CAPS_LOCK) && unshifted >= 'a' && unshifted <= 'z')?shifted:unshifted);
  // send via RS232
  uart_putc(key);
  DATA_OUT(key);
  if(globalopts & OPT_STROBE_LO) {
    STROBE_LO();
    _delay_us(1);
    STROBE_HI();
  } else {
    STROBE_HI();
    _delay_us(1);
    STROBE_LO();
  }
  sh = holdoff;
  while(sh--)
    _delay_us(10);
}

static void send_raw(uint8_t key) {
  send(FALSE,key,key);
}

static void sendhex(uint8_t val) {
  uint8_t v = val & 0x0f;
  uint8_t i = val >> 4;
  send(FALSE,i > 9 ? i - 10 + 'a':i + '0',0);
  send(FALSE,v > 9 ? i - 10 + 'a':v + '0',0);
}

static void map_key(uint8_t sh, uint8_t code,uint8_t state) {
  // Yes, there are many more elegant ways of handling the mapping.  But, this is simple, and easy to rework.
  if(state) {
    switch(code) {
      case PS2_KEY_F5:
      case PS2_KEY_F3:
      case PS2_KEY_F1:
      case PS2_KEY_F2:
      case PS2_KEY_F8:
      case PS2_KEY_F6:
      case PS2_KEY_F4:
      case PS2_KEY_F7:
        break;
      case PS2_KEY_TAB:
        send(sh,0x08,0x08);
        break;
      case PS2_KEY_BACKQUOTE:
        send(sh,'`','~');
        break;
      case PS2_KEY_Q:
        send(sh,'q','Q');
        break;
      case PS2_KEY_1:
      case PS2_KEY_NUM_1:
        send(sh,'1','!');
        break;
      case PS2_KEY_Z:
        send(sh,'z','Z');
        break;
      case PS2_KEY_S:
        send(sh,'s','S');
        break;
      case PS2_KEY_A:
        send(sh,'a','A');
        break;
      case PS2_KEY_W:
        send(sh,'w','W');
        break;
      case PS2_KEY_2:
      case PS2_KEY_NUM_2:
        send(sh,'2','@');
        break;
      case PS2_KEY_C:
        send(sh,'c','C');
        break;
      case PS2_KEY_X:
        send(sh,'x','X');
        break;
      case PS2_KEY_D:
        send(sh,'d','D');
        break;
      case PS2_KEY_E:
        send(sh,'e','E');
        break;
      case PS2_KEY_4:
      case PS2_KEY_NUM_4:
        send(sh,'4','$');
        break;
      case PS2_KEY_3:
      case PS2_KEY_NUM_3:
        send(sh,'3','#');
        break;
      case PS2_KEY_SPACE:
        send(sh,' ',' ');
        break;
      case PS2_KEY_V:
        send(sh,'v','V');
        break;
      case PS2_KEY_F:
        send(sh,'f','F');
        break;
      case PS2_KEY_T:
        send(sh,'t','T');
        break;
      case PS2_KEY_R:
        send(sh,'r','R');
        break;
      case PS2_KEY_5:
      case PS2_KEY_NUM_5:
        send(sh,'5','%');
        break;
      case PS2_KEY_N:
        send(sh,'n','N');
        break;
      case PS2_KEY_B:
        send(sh,'b','B');
        break;
      case PS2_KEY_H:
        send(sh,'h','H');
        break;
      case PS2_KEY_G:
        send(sh,'g','G');
        break;
      case PS2_KEY_Y:
        send(sh,'y','Y');
        break;
      case PS2_KEY_6:
      case PS2_KEY_NUM_6:
        send(sh,'6','^');
        break;
      case PS2_KEY_M:
        send(sh,'m','M');
        break;
      case PS2_KEY_J:
        send(sh,'j','J');
        break;
      case PS2_KEY_U:
        send(sh,'u','U');
        break;
      case PS2_KEY_7:
      case PS2_KEY_NUM_7:
        send(sh,'7','&');
        break;
      case PS2_KEY_8:
      case PS2_KEY_NUM_8:
        send(sh,'8','*');
        break;
      case PS2_KEY_COMMA:
        send(sh,',','<');
        break;
      case PS2_KEY_K:
        send(sh,'k','K');
        break;
      case PS2_KEY_I:
        send(sh,'i','I');
        break;
      case PS2_KEY_O:
        send(sh,'o','O');
        break;
      case PS2_KEY_0:
      case PS2_KEY_NUM_0:
        send(sh,'0',')');
        break;
      case PS2_KEY_9:
      case PS2_KEY_NUM_9:
        send(sh,'9','(');
        break;
      case PS2_KEY_PERIOD:
      case PS2_KEY_NUM_PERIOD:
        send(sh,'.','>');
        break;
      case PS2_KEY_SLASH:
        send(sh,'/','?');
        break;
      case PS2_KEY_L:
        send(sh,'l','L');
        break;
      case PS2_KEY_SEMICOLON:
        send(sh,';',':');
        break;
      case PS2_KEY_P:
        send(sh,'p','P');
        break;
      case PS2_KEY_MINUS:
        send(sh,'-','_');
        break;
      case PS2_KEY_APOSTROPHE:
        send(sh,'\'','"');
        break;
      case PS2_KEY_LBRACKET:
        send(sh,'[','{');
        break;
      case PS2_KEY_EQUALS:
        send(sh,'=','+');
        break;
      case PS2_KEY_ENTER:
        send(sh,13,13);
        if(globalopts & OPT_CRLF)
          send(sh,10,10);
        break;
      case PS2_KEY_RBRACKET:
        send(sh,']','}');
        break;
      case PS2_KEY_BACKSLASH:
        send(sh,'\\','|');
        break;
      case PS2_KEY_BS:
        if(globalopts & OPT_BACKSPACE)
          send(sh,0x09,0x09);
        else
          send(sh,0x7f,0x7f);
        break;
      case PS2_KEY_ESC:
        send(sh,0x1b,0x1b);
        break;
      case PS2_KEY_CAPS_LOCK:
        if(meta & POLL_FLAG_CAPS_LOCK) {
          meta &= (uint8_t)~POLL_FLAG_CAPS_LOCK;
          led_state &= (uint8_t)~PS2_LED_CAPS_LOCK;
        } else {
          meta |= POLL_FLAG_CAPS_LOCK;
          led_state |= PS2_LED_CAPS_LOCK;
        }
        ps2_putc(PS2_CMD_LEDS);
        ps2_putc(led_state);
        break;
      case PS2_KEY_NUM_LOCK:
        if(meta & POLL_FLAG_NUM_LOCK) {
          meta &= (uint8_t)~POLL_FLAG_NUM_LOCK;
          led_state &= (uint8_t)~PS2_LED_NUM_LOCK;
        } else {
          meta |= POLL_FLAG_NUM_LOCK;
          led_state |= PS2_LED_NUM_LOCK;
        }
        ps2_putc(PS2_CMD_LEDS);
        ps2_putc(led_state);
        break;
      case PS2_KEY_SCROLL_LOCK:
        if(meta & POLL_FLAG_SCROLL_LOCK) {
          meta &= (uint8_t)~POLL_FLAG_SCROLL_LOCK;
          led_state &= (uint8_t)~PS2_LED_SCROLL_LOCK;
        } else {
          meta |= POLL_FLAG_SCROLL_LOCK;
          led_state |= PS2_LED_SCROLL_LOCK;
        }
        ps2_putc(PS2_CMD_LEDS);
        ps2_putc(led_state);
        break;
    }
  }
}

static void send_option(uint8_t ch, uint8_t b) {
  send(b, '!', ch);
}

static void set_options(uint8_t key) {
  if(meta & POLL_FLAG_SHIFT) {
    switch(key) {
    case PS2_KEY_ENTER:
      globalopts |= OPT_CRLF;
      send_raw('C');
      send_raw('L');
      break;
    case PS2_KEY_7:       // 7 bit length
      uart_length = LENGTH_7;
      send_raw('&');
      break;
    case PS2_KEY_8:       // 8 bit length
      uart_length = LENGTH_8;
      send_raw('*');
      break;
    case PS2_KEY_T:       // Increase send delay
      if(holdoff <= 0xff)
        holdoff++;
      send_option('T',holdoff <= 0xff);
      break;
    case PS2_KEY_R:       // Increase Typematic rate
      if(type_rate < 0x1f)
        type_rate++;
      send_option('R',type_rate <= 0x1f);
      break;
    case PS2_KEY_D:       // Increase Typematic delay
      if(type_delay < 0x03)
        type_delay++;
      send_option('D',type_delay <= 0x03);
      break;
    case PS2_KEY_S:       // Increase OSCCAL
      if(OSCCAL <= 0xff)
        OSCCAL++;
      send_option('+',OSCCAL <= 0xff);
      break;
    }
  } else {
    switch(key) {
    case PS2_KEY_ENTER:
        globalopts &= (uint8_t)~OPT_CRLF;
        send_raw('C');
        send_raw('R');
      break;
    case PS2_KEY_T:
      holdoff--;
      send_option('t',holdoff);
      break;
    case PS2_KEY_R:       // Decrease Typematic rate
      if(type_rate)
        type_rate--;
      send_option('r',type_rate);
      break;
    case PS2_KEY_D:  // Decrease typematic delay
      if(type_delay)
        type_delay--;
      send_option('d',type_delay <= 0x03);
      break;
    case PS2_KEY_S:       // Decrease OSCCAL
      if(OSCCAL)
        OSCCAL--;
      send_option('-',OSCCAL);
      break;
    case PS2_KEY_L:   // LOW STROBE
      globalopts |= OPT_STROBE_LO;
      STROBE_HI();
      send_raw('L');
      break;
    case PS2_KEY_H:   // HI STROBE
      globalopts &= (uint8_t)~OPT_STROBE_LO;
      STROBE_LO();
      send_raw('H');
      break;
    case PS2_KEY_0:   // 110 bps
      uart_bps = CALC_BPS(110);
      uart_config(uart_bps, uart_length, uart_parity, uart_stop);
      send_raw('0');
      break;
    case PS2_KEY_1:   // 300 bps
      uart_bps = CALC_BPS(300);
      uart_config(uart_bps, uart_length, uart_parity, uart_stop);
      send_raw('1');
      break;
    case PS2_KEY_2:   // 600 bps
      uart_bps = CALC_BPS(600);
      uart_config(uart_bps, uart_length, uart_parity, uart_stop);
      send_raw('2');
      break;
    case PS2_KEY_3:   // 1200 bps
      uart_bps = CALC_BPS(1200);
      uart_config(uart_bps, uart_length, uart_parity, uart_stop);
      send_raw('3');
      break;
    case PS2_KEY_4:   // 2400 bps
      uart_bps = CALC_BPS(2400);
      uart_config(uart_bps, uart_length, uart_parity, uart_stop);
      send_raw('4');
      break;
    case PS2_KEY_5:   // 4800 bps
      uart_bps = CALC_BPS(4800);
      uart_config(uart_bps, uart_length, uart_parity, uart_stop);
      send_raw('5');
      break;
    case PS2_KEY_6:   // 9600 bps
      uart_bps = CALC_BPS(9600);
      uart_config(uart_bps, uart_length, uart_parity, uart_stop);
      send_raw('6');
      break;
    case PS2_KEY_7:   // 19200 bps
      uart_bps = CALC_BPS(19200);
      uart_config(uart_bps, uart_length, uart_parity, uart_stop);
      send_raw('7');
      break;
    case PS2_KEY_8:   // 38400 bps
      uart_bps = CALC_BPS(38400);
      uart_config(uart_bps, uart_length, uart_parity, uart_stop);
      send_raw('8');
      break;
    case PS2_KEY_9:   // 57600 bps
      uart_bps = CALC_BPS(57600);
      uart_config(uart_bps, uart_length, uart_parity, uart_stop);
      send_raw('9');
      break;
    case PS2_KEY_O:   // Odd Parity
      uart_parity = PARITY_ODD;
      send_raw('O');
      break;
    case PS2_KEY_E:   // Even Parity
      uart_parity = PARITY_EVEN;
      send_raw('E');
      break;
    case PS2_KEY_N:   // No Parity
      uart_parity = PARITY_NONE;
      send_raw('N');
      break;
    case PS2_KEY_BS:   // Use Backspace
      globalopts |= OPT_BACKSPACE;
      send_raw('B');
      send_raw('S');
      break;
    case PS2_KEY_DELETE | 0x80:   // Use Delete
      globalopts &= (uint8_t)~OPT_BACKSPACE;
      send_raw('D');
      send_raw('L');
      break;
    case PS2_KEY_P:
      send_raw('<');
      sendhex(OSCCAL);
      send_raw(':');
      sendhex(globalopts);
      send_raw(':');
      sendhex(holdoff);
      send_raw('>');
      break;
    case PS2_KEY_W:   // Save Data
      eeprom_write_config();
      send_raw('W');
      break;
    }
  }
}

void parse_key(uint8_t key, uint8_t state) {
  if((key&0x7f)==PS2_KEY_ALT) {
    // turn on or off the ALT META flag
    meta=(meta&(uint8_t)~POLL_FLAG_ALT) | (state?POLL_FLAG_ALT:0);
  } else if((key&0x7f)==PS2_KEY_LCTRL) {
    // turn on or off the CTRL META flag
    meta=(meta&(uint8_t)~POLL_FLAG_CONTROL) | (state?POLL_FLAG_CONTROL:0);
  } else if(key == PS2_KEY_LSHIFT) {
    meta = (meta & (uint8_t)~POLL_FLAG_LSHIFT) | (state ? POLL_FLAG_LSHIFT : 0);
  } else if(key == PS2_KEY_RSHIFT) {
    meta = (meta & (uint8_t)~POLL_FLAG_RSHIFT) | (state ? POLL_FLAG_RSHIFT : 0);
  }
  if((meta&POLL_FLAG_CTRL_ALT)==POLL_FLAG_CTRL_ALT && key==(0x80 | PS2_KEY_DELETE) && state) {
    // CTRL/ALT/DEL is pressed.
    // bring RESET line low
    // repeat this a few times so the pulse will be long enough to trigger the logic.
    RESET_ACTIVE();
    _delay_us(1);
    RESET_INACTIVE();
  } else if(CONF_MODE() && (meta&POLL_FLAG_CTRL_ALT)==POLL_FLAG_CTRL_ALT && key==PS2_KEY_BS && state) {
    // CTRL/ALT/BS config mode
    config ^= KB_CONFIG;
    if(!config) {
      uart_config(uart_bps, uart_length, uart_parity, uart_stop);
      ps2_putc(PS2_CMD_SET_RATE);
      ps2_putc(CALC_RATE(type_delay, type_rate));
    }

  } else if (config) {
    if(state) { // set parms on keydown
      set_options(key);
    }
  } else
    map_key(meta&POLL_FLAG_SHIFT,key,state);
}

int main(void) __attribute__((OS_main));
int main(void) {
  uint8_t key;
  poll_state_t state = POLL_ST_IDLE;

  uart_init();

  eeprom_read_config();

  DATA_SETDDR();
  RESET_SETDDR();
  RESET_INACTIVE();
  CONF_MODE_SETDDR();
  STROBE_SETDDR();

  if(globalopts & OPT_STROBE_LO)
    STROBE_HI();
  else
    STROBE_LO();

  uart_config(uart_bps, uart_length, uart_parity, uart_stop);

  ps2_init(PS2_MODE_HOST);

  sei();
  uart_putc('.');
  for(;;) {
    if(ps2_data_available() != 0) {
      // kb sent data...
      key=ps2_getc();
      if(key==PS2_CMD_BAT) {
        state=POLL_ST_IDLE;
      } else {
      switch(state) {
        case POLL_ST_IDLE:
          switch(key) {
            case PS2_KEY_EXT:
              // we got E0
              state=POLL_ST_GET_X_KEY;
              break;
            case PS2_KEY_UP:
              // get normal key up.
              state=POLL_ST_GET_KEY_UP;
              break;
            case PS2_KEY_EXT_2:
              // we got an E1
              state=POLL_ST_GET_PAUSE_1;
              // start on the Pause/Break sequence.
              break;
            case PS2_CMD_ACK:
            case PS2_CMD_ECHO:
            case PS2_CMD_ERROR:
            case PS2_CMD_OVERFLOW:
              break;
            default:
              parse_key(key,TRUE);
              state=POLL_ST_IDLE;
              break;
          }
          break;
        case POLL_ST_GET_KEY_UP:
          parse_key(key,FALSE);
          state=POLL_ST_IDLE;
          break;
        case POLL_ST_GET_X_KEY:
          if(key==PS2_KEY_UP) {
            state=POLL_ST_GET_X_KEY_UP;
          } else if(key==PS2_KEY_LSHIFT) {
            // when NumLock is pressed, INS and DEL prepend with EO 12 (extended shift), but we don't care, so eat code.
            // Also, when PrintScreen is pressed, it too sends an E0 12
            state=POLL_ST_IDLE;
          } else {
            parse_key(0x80 | key,TRUE);
            state=POLL_ST_IDLE;
          }
          break;
        case POLL_ST_GET_X_KEY_UP:
          if(key==PS2_KEY_LSHIFT) {
            // when NumLock is pressed, INS and DEL prepend with EO 12 (extended shift), but we don't care, so eat code.
            // Also, when PrintScreen is pressed, it too sends an E0 12
            state=POLL_ST_IDLE;
          } else {
            parse_key(0x80 | key,FALSE);
            state=POLL_ST_IDLE;
          }
          break;
        case POLL_ST_GET_PAUSE_1:
          // we get 14
          if(key==PS2_KEY_PCTRL) {
            state=POLL_ST_GET_PAUSE_2;
          } else {
            state=POLL_ST_IDLE;
          }
          break;
        case POLL_ST_GET_PAUSE_2:
          // we got 77
          if(key==PS2_KEY_PAUSE) {
            state=POLL_ST_GET_PAUSE_3;
          } else {
            state=POLL_ST_IDLE;
          }
          break;
        case POLL_ST_GET_PAUSE_3:
          // we get E1
          if(key==PS2_KEY_EXT_2) {
            state=POLL_ST_GET_PAUSE_4;
          } else {
            state=POLL_ST_IDLE;
          }
          break;
        case POLL_ST_GET_PAUSE_4:
          // we got F0
          if(key==PS2_KEY_UP) {
            state=POLL_ST_GET_PAUSE_5;
          } else {
            state=POLL_ST_IDLE;
          }
          break;
        case POLL_ST_GET_PAUSE_5:
          // we got 14
          if(key==PS2_KEY_PCTRL) {
            state=POLL_ST_GET_PAUSE_6;
          } else {
            state=POLL_ST_IDLE;
          }
          break;
        case POLL_ST_GET_PAUSE_6:
          // we got F0
          if(key==PS2_KEY_UP) {
            state=POLL_ST_GET_PAUSE_7;
          } else {
            state=POLL_ST_IDLE;
          }
          break;
        case POLL_ST_GET_PAUSE_7:
          // we got 77
          if(key==PS2_KEY_PAUSE) {
            //('R');
            // we received a complete Pause/Break, do something about it.
            parse_key(0x80|PS2_KEY_PAUSE,TRUE);
            parse_key(0x80|PS2_KEY_PAUSE,FALSE);
          }
          state=POLL_ST_IDLE;
          break;
        }
      }
    }
  }
}

