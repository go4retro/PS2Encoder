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

    main.c: Main application
*/

#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "config.h"
#include "eeprom.h"
#include "flags.h"
//#include "matrix.h"
#include "ps2.h"
//#include "switches.h"
#include "uart.h"
#include "xt.h"

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
static uint8_t xt_eshift;
static uint8_t config;
static uint8_t led_state=0;
uint8_t globalopts;
uint8_t holdoff;
uint8_t pulselen;
uint8_t resetlen;
uint16_t uart_bps;
uartlen_t uart_length;
uartpar_t uart_parity;
uartstop_t uart_stop;
uint8_t  type_delay;
uint8_t  type_rate;

static inline __attribute__((always_inline)) void delay_strobe(uint8_t delay) {
  uint8_t i;

  for(i = 0; i < delay; i++)
    _delay_us(1);
}

static inline __attribute__((always_inline)) void delay_reset(uint8_t delay) {
  uint8_t i;

  for(i = 0; i < delay; i++)
    _delay_us(10);
}

static inline void send_raw(uint8_t key) {
  // send via RS232
  uart_putc(key);
  data_out(key);
  if(globalopts & OPT_STROBE_LO) {
    data_strobe_lo();
    delay_strobe(pulselen);
    data_strobe_hi();
  } else {
    data_strobe_hi();
    delay_strobe(pulselen);
    data_strobe_lo();
  }
  key = holdoff;
  while(key--)
    _delay_us(10);
}

static void sendhex(uint8_t val) {
  uint8_t v = val & 0x0f;
  uint8_t i = val >> 4;
  send_raw(i > 9 ? i - 10 + 'a':i + '0');
  send_raw(v > 9 ? i - 10 + 'a':v + '0');
}

#define CTRL(x)    (x & 0x1f)
#define MAP(x,y,z) do { u = x; s = y; c = z;} while(0)

static void ps2_to_ascii(uint8_t code) {
  uint8_t u = 0,s = 0,c = 0;

  // Yes, there are many more elegant ways of handling the mapping.  But, this is simple, and easy to rework.
  switch(code) {
    case PS2_KEY_TAB:
      MAP(0x09,0x09,0x09);
      break;
    case PS2_KEY_BACKQUOTE:
      MAP('`','~',0);
      break;
    case PS2_KEY_Q:
      MAP('q','Q',CTRL('q'));
      break;
    case PS2_KEY_1:
      MAP('1','!',0);
      break;
    case PS2_KEY_NUM_1:
      MAP('1','1',0);
      break;
    case PS2_KEY_Z:
      MAP('z','Z',CTRL('z'));
      break;
    case PS2_KEY_S:
      MAP('s','S',CTRL('s'));
      break;
    case PS2_KEY_A:
      MAP('a','A',CTRL('a'));
      break;
    case PS2_KEY_W:
      MAP('w','W',CTRL('w'));
      break;
    case PS2_KEY_2:
      MAP('2','@',0);
      break;
    case PS2_KEY_NUM_2:
      MAP('2','2',0);
      break;
    case PS2_KEY_C:
      MAP('c','C',CTRL('c'));
      break;
    case PS2_KEY_X:
      MAP('x','X',CTRL('x'));
      break;
    case PS2_KEY_D:
      MAP('d','D',CTRL('d'));
      break;
    case PS2_KEY_E:
      MAP('e','E',CTRL('e'));
      break;
    case PS2_KEY_4:
      MAP('4','$',0);
      break;
    case PS2_KEY_NUM_4:
      MAP('4','4',0);
      break;
    case PS2_KEY_3:
      MAP('3','#',0);
      break;
    case PS2_KEY_NUM_3:
      MAP('3','3',0);
      break;
    case PS2_KEY_SPACE:
      MAP(' ',' ',' ');
      break;
    case PS2_KEY_V:
      MAP('v','V',CTRL('v'));
      break;
    case PS2_KEY_F:
      MAP('f','F',CTRL('f'));
      break;
    case PS2_KEY_T:
      MAP('t','T',CTRL('t'));
      break;
    case PS2_KEY_R:
      MAP('r','R',CTRL('r'));
      break;
    case PS2_KEY_5:
      MAP('5','%',0);
      break;
    case PS2_KEY_NUM_5:
      MAP('5','5',0);
      break;
    case PS2_KEY_N:
      MAP('n','N',CTRL('n'));
      break;
    case PS2_KEY_B:
      MAP('b','B',CTRL('b'));
      break;
    case PS2_KEY_H:
      MAP('h','H',CTRL('h'));
      break;
    case PS2_KEY_G:
      MAP('g','G',CTRL('g'));
      break;
    case PS2_KEY_Y:
      MAP('y','Y',CTRL('y'));
      break;
    case PS2_KEY_6:
      MAP('6','^',0);
      break;
    case PS2_KEY_NUM_6:
      MAP('6','6',0);
      break;
    case PS2_KEY_M:
      MAP('m','M',CTRL('m'));
      break;
    case PS2_KEY_J:
      MAP('j','J',CTRL('j'));
      break;
    case PS2_KEY_U:
      MAP('u','U',CTRL('u'));
      break;
    case PS2_KEY_7:
      MAP('7','&',0);
      break;
    case PS2_KEY_NUM_7:
      MAP('7','7',0);
      break;
    case PS2_KEY_8:
      MAP('8','*',0);
      break;
    case PS2_KEY_NUM_8:
      MAP('8','8',0);
      break;
    case PS2_KEY_COMMA:
      MAP(',','<',0);
      break;
    case PS2_KEY_K:
      MAP('k','K',CTRL('k'));
      break;
    case PS2_KEY_I:
      MAP('i','I',CTRL('i'));
      break;
    case PS2_KEY_O:
      MAP('o','O',CTRL('o'));
      break;
    case PS2_KEY_0:
      MAP('0',')',0);
      break;
    case PS2_KEY_NUM_0:
      MAP('0','0',0);
      break;
    case PS2_KEY_9:
      MAP('9','(',0);
      break;
    case PS2_KEY_NUM_9:
      MAP('9','9',0);
      break;
    case PS2_KEY_PERIOD:
      MAP('.','>',0);
      break;
    case PS2_KEY_NUM_PERIOD:
      MAP('.','.',0);
      break;
    case PS2_KEY_NUM_PLUS:
      MAP('+','+',0);
      break;
    case PS2_KEY_NUM_MINUS:
      MAP('-','-',0);
      break;
    case PS2_KEY_NUM_STAR:
      MAP('*','*',0);
      break;
    case PS2_KEY_SLASH:
    case PS2_KEY_NUM_SLASH | 0x80:
      MAP('/','?',0);
      break;
    case PS2_KEY_L:
      MAP('l','L',CTRL('l'));
      break;
    case PS2_KEY_SEMICOLON:
      MAP(';',':',0);
      break;
    case PS2_KEY_P:
      MAP('p','P',CTRL('p'));
      break;
    case PS2_KEY_MINUS:
      MAP('-','_',CTRL('-'));
      break;
    case PS2_KEY_APOSTROPHE:
      MAP('\'','"',0);
      break;
    case PS2_KEY_LBRACKET:
      MAP('[','{',0);
      break;
    case PS2_KEY_EQUALS:
      MAP('=','+',0);
      break;
    case PS2_KEY_ENTER:
    case PS2_KEY_NUM_ENTER | 0x80:
      MAP(13,13,13);
      break;
    case PS2_KEY_RBRACKET:
      MAP(']','}',0);
      break;
    case PS2_KEY_BACKSLASH:
      MAP('\\','|',CTRL('\\'));
      break;
    case PS2_KEY_BS:
      if(globalopts & OPT_BACKSPACE)
        MAP(0x08,0x08,0x08);
      else
        MAP(0x7f,0x7f,0x7f);
      break;
    case PS2_KEY_ESC:
      MAP(0x1b,0x1b,0x1b);
      break;

  }
  if(meta & POLL_FLAG_CONTROL && c)
    u = c;
  else if((meta & POLL_FLAG_SHIFT) && s)
    u = s;
  else if((meta & POLL_FLAG_CAPS_LOCK) && u >= 'a' && u <= 'z')
    u = s;

  if(u)
    send_raw(u);
  if((globalopts & OPT_CRLF) && u == 13 && !(meta & POLL_FLAG_CONTROL) )
    send_raw(10);
}

static void xt_to_ps2(uint8_t code) {
  uint8_t keydown = (code & 0x80 ? FALSE : TRUE);
  uint8_t key;
  code = code & 0x7f;

  if(!keydown)
    ps2_putc(PS2_KEY_UP);
  switch (code) {
    case XT_KEY_F1:           key = PS2_KEY_F1;           break;
    case XT_KEY_F2:           key = PS2_KEY_F2;           break;
    case XT_KEY_F3:           key = PS2_KEY_F3;           break;
    case XT_KEY_F4:           key = PS2_KEY_F4;           break;
    case XT_KEY_F5:           key = PS2_KEY_F5;           break;
    case XT_KEY_F6:           key = PS2_KEY_F6;           break;
    case XT_KEY_F7:           key = PS2_KEY_F7;           break;
    case XT_KEY_F8:           key = PS2_KEY_F8;           break;
    case XT_KEY_F9:           key = PS2_KEY_F9;           break;
    case XT_KEY_F10:          key = PS2_KEY_F10;          break;
    case XT_KEY_F11:          key = PS2_KEY_F11;          break;
    case XT_KEY_F12:          key = PS2_KEY_F12;          break;
    case XT_KEY_TAB:          key = PS2_KEY_TAB;          break;
    case XT_KEY_BACKQUOTE:    key = PS2_KEY_BACKQUOTE;    break;
    case XT_KEY_Q:            key = PS2_KEY_Q;            break;
    case XT_KEY_1:            key = PS2_KEY_1;            break;
    case XT_KEY_Z:            key = PS2_KEY_Z;            break;
    case XT_KEY_S:            key = PS2_KEY_S;            break;
    case XT_KEY_A:            key = PS2_KEY_A;            break;
    case XT_KEY_W:            key = PS2_KEY_W;            break;
    case XT_KEY_2:            key = PS2_KEY_2;            break;
    case XT_KEY_C:            key = PS2_KEY_C;            break;
    case XT_KEY_X:            key = PS2_KEY_X;            break;
    case XT_KEY_D:            key = PS2_KEY_D;            break;
    case XT_KEY_E:            key = PS2_KEY_E;            break;
    case XT_KEY_4:            key = PS2_KEY_4;            break;
    case XT_KEY_3:            key = PS2_KEY_3;            break;
    case XT_KEY_SPACE:        key = PS2_KEY_SPACE;        break;
    case XT_KEY_V:            key = PS2_KEY_V;            break;
    case XT_KEY_F:            key = PS2_KEY_F;            break;
    case XT_KEY_T:            key = PS2_KEY_T;            break;
    case XT_KEY_R:            key = PS2_KEY_R;            break;
    case XT_KEY_5:            key = PS2_KEY_5;            break;
    case XT_KEY_N:            key = PS2_KEY_N;            break;
    case XT_KEY_B:            key = PS2_KEY_B;            break;
    case XT_KEY_H:            key = PS2_KEY_H;            break;
    case XT_KEY_G:            key = PS2_KEY_G;            break;
    case XT_KEY_Y:            key = PS2_KEY_Y;            break;
    case XT_KEY_6:            key = PS2_KEY_6;            break;
    case XT_KEY_M:            key = PS2_KEY_M;            break;
    case XT_KEY_J:            key = PS2_KEY_J;            break;
    case XT_KEY_U:            key = PS2_KEY_U;            break;
    case XT_KEY_7:            key = PS2_KEY_7;            break;
    case XT_KEY_8:            key = PS2_KEY_8;            break;
    case XT_KEY_COMMA:        key = PS2_KEY_COMMA;        break;
    case XT_KEY_K:            key = PS2_KEY_K;            break;
    case XT_KEY_I:            key = PS2_KEY_I;            break;
    case XT_KEY_O:            key = PS2_KEY_O;            break;
    case XT_KEY_0:            key = PS2_KEY_0;            break;
    case XT_KEY_NUM_0:        key = PS2_KEY_NUM_0;        break;
    case XT_KEY_9:            key = PS2_KEY_9;            break;
    case XT_KEY_PERIOD:       key = PS2_KEY_PERIOD;       break;
    case XT_KEY_NUM_PERIOD:   key = PS2_KEY_NUM_PERIOD;   break;
    case XT_KEY_SLASH:        key = PS2_KEY_SLASH;        break;
    case XT_KEY_L:            key = PS2_KEY_L;            break;
    case XT_KEY_SEMICOLON:    key = PS2_KEY_SEMICOLON;    break;
    case XT_KEY_P:            key = PS2_KEY_P;            break;
    case XT_KEY_MINUS:        key = PS2_KEY_MINUS;        break;
    case XT_KEY_APOSTROPHE:   key = PS2_KEY_APOSTROPHE;   break;
    case XT_KEY_LBRACKET:     key = PS2_KEY_LBRACKET;     break;
    case XT_KEY_EQUALS:       key = PS2_KEY_EQUALS;       break;
    case XT_KEY_ENTER:        key = PS2_KEY_ENTER;        break;
    case XT_KEY_RBRACKET:     key = PS2_KEY_RBRACKET;     break;
    case XT_KEY_BACKSLASH:    key = PS2_KEY_BACKSLASH;    break;
    case XT_KEY_BS:           key = PS2_KEY_BS;           break;
    case XT_KEY_ESC:          key = PS2_KEY_ESC;          break;
    case XT_KEY_CAPS_LOCK:    key = PS2_KEY_CAPS_LOCK;    break;
    case XT_KEY_NUM_LOCK:     key = PS2_KEY_NUM_LOCK;     break;
    case XT_KEY_SCROLL_LOCK:  key = PS2_KEY_SCROLL_LOCK;  break;
    case XT_KEY_ALT:          key = PS2_KEY_ALT;          break;
    case XT_KEY_LCTRL:        key = PS2_KEY_LCTRL;        break;
    case XT_KEY_LSHIFT:       key = PS2_KEY_LSHIFT;       break;
    case XT_KEY_RSHIFT:       key = PS2_KEY_RSHIFT;       break;
    case XT_KEY_NUM_PLUS:     key = PS2_KEY_NUM_PLUS;     break;
    case XT_KEY_NUM_MINUS:    key = PS2_KEY_NUM_MINUS;    break;
    case XT_KEY_NUM_STAR:     key = PS2_KEY_NUM_STAR;     break;
    case XT_KEY_INT1:         key = PS2_KEY_INT1;         break;
    //case XT_KEY_INT2:         key = PS2_KEY_INT2;         break;
    case XT_KEY_NUM_1:        key = PS2_KEY_NUM_1;        break;
    case XT_KEY_NUM_2:        key = PS2_KEY_NUM_2;        break;
    case XT_KEY_NUM_3:        key = PS2_KEY_NUM_3;        break;
    case XT_KEY_NUM_4:        key = PS2_KEY_NUM_4;        break;
    case XT_KEY_NUM_5:        key = PS2_KEY_NUM_5;        break;
    case XT_KEY_NUM_6:        key = PS2_KEY_NUM_6;        break;
    case XT_KEY_NUM_7:        key = PS2_KEY_NUM_7;        break;
    case XT_KEY_NUM_8:        key = PS2_KEY_NUM_8;        break;
    case XT_KEY_NUM_9:        key = PS2_KEY_NUM_9;        break;
  }
  ps2_putc(key);
}

static void ps2_to_xt(uint8_t code,uint8_t keydown) {
  uint8_t key = 0;
  uint8_t eshift = FALSE;


  if(keydown && xt_eshift) { // remove extended shift)
    xt_putc(XT_KEY_EXT);
    xt_putc(XT_KEY_LSHIFT | 0x80);
    xt_eshift = FALSE;
  }

  // Yes, there are many more elegant ways of handling the mapping.  But, this is simple, and easy to rework.
  switch(code) {
    case PS2_KEY_F1:
      key = XT_KEY_F1;
      break;
    case PS2_KEY_F2:
      key = XT_KEY_F2;
      break;
    case PS2_KEY_F3:
      key = XT_KEY_F3;
      break;
    case PS2_KEY_F4:
      key = XT_KEY_F4;
      break;
    case PS2_KEY_F5:
      key = XT_KEY_F5;
      break;
    case PS2_KEY_F6:
      key = XT_KEY_F6;
      break;
    case PS2_KEY_F7:
      key = XT_KEY_F7;
      break;
    case PS2_KEY_F8:
      key = XT_KEY_F8;
      break;
    case PS2_KEY_F9:
      key = XT_KEY_F9;
      break;
    case PS2_KEY_F10:
      key = XT_KEY_F10;
      break;
    case PS2_KEY_F11:
      key = XT_KEY_F11;
      break;
    case PS2_KEY_F12:
      key = XT_KEY_F12;
      break;
    case PS2_KEY_TAB:
      key = XT_KEY_TAB;
      break;
    case PS2_KEY_BACKQUOTE:
      key = XT_KEY_BACKQUOTE;
      break;
    case PS2_KEY_Q:
      key = XT_KEY_Q;
      break;
    case PS2_KEY_1:
      key = XT_KEY_1;
      break;
    case PS2_KEY_Z:
      key = XT_KEY_Z;
      break;
    case PS2_KEY_S:
      key = XT_KEY_S;
      break;
    case PS2_KEY_A:
      key = XT_KEY_A;
      break;
    case PS2_KEY_W:
      key = XT_KEY_W;
      break;
    case PS2_KEY_2:
      key = XT_KEY_2;
      break;
    case PS2_KEY_C:
      key = XT_KEY_C;
      break;
    case PS2_KEY_X:
      key = XT_KEY_X;
      break;
    case PS2_KEY_D:
      key = XT_KEY_D;
      break;
    case PS2_KEY_E:
      key = XT_KEY_E;
      break;
    case PS2_KEY_4:
      key = XT_KEY_4;
      break;
    case PS2_KEY_3:
      key = XT_KEY_3;
      break;
    case PS2_KEY_SPACE:
      key = XT_KEY_SPACE;
      break;
    case PS2_KEY_V:
      key = XT_KEY_V;
      break;
    case PS2_KEY_F:
      key = XT_KEY_F;
      break;
    case PS2_KEY_T:
      key = XT_KEY_T;
      break;
    case PS2_KEY_R:
      key = XT_KEY_R;
      break;
    case PS2_KEY_5:
      key = XT_KEY_5;
      break;
    case PS2_KEY_N:
      key = XT_KEY_N;
      break;
    case PS2_KEY_B:
      key = XT_KEY_B;
      break;
    case PS2_KEY_H:
      key = XT_KEY_H;
      break;
    case PS2_KEY_G:
      key = XT_KEY_G;
      break;
    case PS2_KEY_Y:
      key = XT_KEY_Y;
      break;
    case PS2_KEY_6:
      key = XT_KEY_6;
      break;
    case PS2_KEY_M:
      key = XT_KEY_M;
      break;
    case PS2_KEY_J:
      key = XT_KEY_J;
      break;
    case PS2_KEY_U:
      key = XT_KEY_U;
      break;
    case PS2_KEY_7:
      key = XT_KEY_7;
      break;
    case PS2_KEY_8:
      key = XT_KEY_8;
      break;
    case PS2_KEY_COMMA:
      key = XT_KEY_COMMA;
      break;
    case PS2_KEY_K:
      key = XT_KEY_K;
      break;
    case PS2_KEY_I:
      key = XT_KEY_I;
      break;
    case PS2_KEY_O:
      key = XT_KEY_O;
      break;
    case PS2_KEY_0:
      key = XT_KEY_0;
      break;
    case PS2_KEY_NUM_0:
      key = XT_KEY_NUM_0;
      break;
    case PS2_KEY_9:
      key = XT_KEY_9;
      break;
    case PS2_KEY_PERIOD:
      key = XT_KEY_PERIOD;
      break;
    case PS2_KEY_NUM_PERIOD:
      key = XT_KEY_NUM_PERIOD;
      break;
    case PS2_KEY_SLASH:
      key = XT_KEY_SLASH;
      break;
    case PS2_KEY_L:
      key = XT_KEY_L;
      break;
    case PS2_KEY_SEMICOLON:
      key = XT_KEY_SEMICOLON;
      break;
    case PS2_KEY_P:
      key = XT_KEY_P;
      break;
    case PS2_KEY_MINUS:
      key = XT_KEY_MINUS;
      break;
    case PS2_KEY_APOSTROPHE:
      key = XT_KEY_APOSTROPHE;
      break;
    case PS2_KEY_LBRACKET:
      key = XT_KEY_LBRACKET;
      break;
    case PS2_KEY_EQUALS:
      key = XT_KEY_EQUALS;
      break;
    case PS2_KEY_ENTER:
      key = XT_KEY_ENTER;
      break;
    case PS2_KEY_RBRACKET:
      key = XT_KEY_RBRACKET;
      break;
    case PS2_KEY_BACKSLASH:
      key = XT_KEY_BACKSLASH;
      break;
    case PS2_KEY_BS:
      key = XT_KEY_BS;
      break;
    case PS2_KEY_ESC:
      key = XT_KEY_ESC;
      break;
    case PS2_KEY_CAPS_LOCK:
      key = XT_KEY_CAPS_LOCK;
      break;
    case PS2_KEY_NUM_LOCK:
      key = XT_KEY_NUM_LOCK;
      break;
    case PS2_KEY_SCROLL_LOCK:
      key = XT_KEY_SCROLL_LOCK;
      break;
    case PS2_KEY_ALT:
      key = XT_KEY_ALT;
      break;
    case PS2_KEY_LCTRL:
      key = XT_KEY_LCTRL;
      break;
    case PS2_KEY_LSHIFT:
      key = XT_KEY_LSHIFT;
      break;
    case PS2_KEY_RSHIFT:
      key = XT_KEY_RSHIFT;
      break;
    case PS2_KEY_NUM_PLUS:
      key = XT_KEY_NUM_PLUS;
      break;
    case PS2_KEY_NUM_MINUS:
      key = XT_KEY_NUM_MINUS;
      break;
    case PS2_KEY_NUM_STAR:
      key = XT_KEY_NUM_STAR;
      break;
    case PS2_KEY_INT1:
      key = XT_KEY_INT1;
      break;
    case PS2_KEY_INT2:
      key = XT_KEY_INT2;
      break;
    case PS2_KEY_NUM_1:
      key = XT_KEY_NUM_1;
      break;
    case PS2_KEY_NUM_2:
      key = XT_KEY_NUM_2;
      break;
    case PS2_KEY_NUM_3:
      key = XT_KEY_NUM_3;
      break;
    case PS2_KEY_NUM_4:
      key = XT_KEY_NUM_4;
      break;
    case PS2_KEY_NUM_5:
      key = XT_KEY_NUM_5;
      break;
    case PS2_KEY_NUM_6:
      key = XT_KEY_NUM_6;
      break;
    case PS2_KEY_NUM_7:
      key = XT_KEY_NUM_7;
      break;
    case PS2_KEY_NUM_8:
      key = XT_KEY_NUM_8;
      break;
    case PS2_KEY_NUM_9:
      key = XT_KEY_NUM_9;
      break;
  }
  if(key) {
    xt_putc(keydown ? key : key | 0x80);
  } else {
    // check for extended keys
    switch (code) {
      case PS2_KEY_RALT | 0x80:
        key = XT_KEY_RALT;
        break;
      case PS2_KEY_RCTRL | 0x80:
        key = XT_KEY_RCTRL;
        break;
      case PS2_KEY_NUM_ENTER | 0x80:
        key = XT_KEY_NUM_ENTER;
        break;
      case PS2_KEY_NUM_SLASH | 0x80:
        key = XT_KEY_NUM_SLASH;
        break;
    }
    if(key) {
      // send extended key
      xt_putc(XT_KEY_EXT);
      xt_putc(keydown ? key : key | 0x80);
    } else {
      // codes that are sent with conditional extended shift:
      switch(code) {
        case PS2_KEY_END | 0x80:
          key = XT_KEY_END;
          eshift = (meta & POLL_FLAG_NUM_LOCK);
          break;
        case PS2_KEY_HOME | 0x80:
          key = XT_KEY_HOME;
          eshift = (meta & POLL_FLAG_NUM_LOCK);
          break;
        case PS2_KEY_INSERT | 0x80:
          key = XT_KEY_INSERT;
          eshift = (meta & POLL_FLAG_NUM_LOCK);
          break;
        case PS2_KEY_DELETE | 0x80:
          key = XT_KEY_DELETE;
          eshift = (meta & POLL_FLAG_NUM_LOCK);
          break;
        case PS2_KEY_PAGE_DOWN | 0x80:
          key = XT_KEY_PAGE_DOWN;
          eshift = (meta & POLL_FLAG_NUM_LOCK);
          break;
        case PS2_KEY_PAGE_UP | 0x80:
          key = XT_KEY_PAGE_UP;
          eshift = (meta & POLL_FLAG_NUM_LOCK);
          break;
        case PS2_KEY_CRSR_DOWN | 0x80:
          key = XT_KEY_CRSR_DOWN;
          eshift = (meta & POLL_FLAG_NUM_LOCK);
          break;
        case PS2_KEY_CRSR_RIGHT | 0x80:
          key = XT_KEY_CRSR_RIGHT;
          eshift = (meta & POLL_FLAG_NUM_LOCK);
          break;
        case PS2_KEY_CRSR_UP | 0x80:
          key = XT_KEY_CRSR_UP;
          eshift = (meta & POLL_FLAG_NUM_LOCK);
          break;
        case PS2_KEY_CRSR_LEFT | 0x80:
          key = XT_KEY_CRSR_LEFT;
          eshift = (meta & POLL_FLAG_NUM_LOCK);
          break;
        case PS2_KEY_PRINT_SCREEN | 0x80:
          key = XT_KEY_PRINT_SCREEN;
          eshift = TRUE;
      }
      if(key) {
        if(eshift && keydown && !(meta & POLL_FLAG_SHIFT)) {
          // On keydown, put extended shift keydown first.
          xt_putc(XT_KEY_EXT);
          xt_putc(XT_KEY_LSHIFT);
          xt_eshift = TRUE;
        }
        // send extended key
        xt_putc(XT_KEY_EXT);
        xt_putc(keydown ? key : key | 0x80);
        if(eshift && !keydown && !(meta & POLL_FLAG_SHIFT)) {
          // on keyup, put extended shift keyup last.
          // even if we've already put key up on E Shift, do it again.
          xt_putc(XT_KEY_EXT);
          xt_putc(XT_KEY_LSHIFT | 0x80);
          xt_eshift = FALSE;
        }
      } else if(code == (XT_KEY_PAUSE | 0x80) && keydown) {
        xt_putc(XT_KEY_EXT_2);
        xt_putc(XT_KEY_LCTRL);
        xt_putc(XT_KEY_PAUSE);

        xt_putc(XT_KEY_EXT_2);
        xt_putc(XT_KEY_LCTRL | 0x80);
        xt_putc(XT_KEY_PAUSE | 0x80);
      }
    }
  }
}

static void send_option(uint8_t ch, uint8_t b) {
  send_raw(b ? ch : '!');
}

static void set_options(uint8_t key) {
  if(meta & POLL_FLAG_SHIFT) {
    switch(key) {
    case PS2_KEY_ENTER:
      globalopts |= OPT_CRLF;
      send_raw('c');
      send_raw('l');
      break;
    case PS2_KEY_1:       // 1 stop bit
      uart_stop = STOP_0;
      send_raw('s');
      send_raw('1');
      break;
    case PS2_KEY_2:       // 2 stop bits
      uart_stop = STOP_1;
      send_raw('s');
      send_raw('2');
      break;
    case PS2_KEY_7:       // 7 bit length
      uart_length = LENGTH_7;
      send_raw('l');
      send_raw('7');
      break;
    case PS2_KEY_8:       // 8 bit length
      uart_length = LENGTH_8;
      send_raw('l');
      send_raw('8');
      break;
    case PS2_KEY_P:       // Increase strobe pulse length
      if(pulselen < 0xff)
        pulselen++;
      send_option('P',pulselen < 0xff);
      break;
    case PS2_KEY_I:       // Increase reset pulse length
      if(resetlen < 0xff)
        resetlen++;
      send_option('I',resetlen < 0xff);
      break;
    case PS2_KEY_T:       // Increase send delay
      if(holdoff < 0xff)
        holdoff++;
      send_option('T',holdoff < 0xff);
      break;
    case PS2_KEY_R:       // Increase Typematic rate
      if(type_rate < 0x1f)
        type_rate++;
      send_option('R',type_rate < 0x1f);
      break;
    case PS2_KEY_D:       // Increase Typematic delay
      if(type_delay < 0x03)
        type_delay++;
      send_option('D',type_delay < 0x03);
      break;
    case PS2_KEY_S:       // Increase OSCCAL
      if(OSCCAL < 0xff)
        OSCCAL++;
      send_option('+',OSCCAL < 0xff);
      break;
    case PS2_KEY_L:   // LOW RESET STROBE
      globalopts &= (uint8_t)~OPT_RESET_HI;
      reset_set_hi();
      send_raw('L');
      break;
    case PS2_KEY_H:   // HI RESET STROBE
      globalopts |= OPT_RESET_HI;
      reset_set_lo();
      send_raw('H');
      break;
    }
  } else {
    switch(key) {
    case PS2_KEY_ENTER:
        globalopts &= (uint8_t)~OPT_CRLF;
        send_raw('c');
        send_raw('r');
      break;
    case PS2_KEY_P:       // Increase strobe pulse length
      if(pulselen)
        pulselen--;
      send_option('p',pulselen);
      break;
    case PS2_KEY_I:       // Increase reset pulse length
      if(resetlen)
        resetlen--;
      send_option('i',resetlen);
      break;
    case PS2_KEY_T:
      if(holdoff)
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
      data_strobe_hi();
      send_raw('l');
      break;
    case PS2_KEY_H:   // HI STROBE
      globalopts &= (uint8_t)~OPT_STROBE_LO;
      data_strobe_lo();
      send_raw('h');
      break;
    case PS2_KEY_0:   // 110 bps
      uart_bps = CALC_BPS(110);
      send_raw('0');
      break;
    case PS2_KEY_1:   // 300 bps
      uart_bps = CALC_BPS(300);
      send_raw('1');
      break;
    case PS2_KEY_2:   // 600 bps
      uart_bps = CALC_BPS(600);
      send_raw('2');
      break;
    case PS2_KEY_3:   // 1200 bps
      uart_bps = CALC_BPS(1200);
      send_raw('3');
      break;
    case PS2_KEY_4:   // 2400 bps
      uart_bps = CALC_BPS(2400);
      send_raw('4');
      break;
    case PS2_KEY_5:   // 4800 bps
      uart_bps = CALC_BPS(4800);
      send_raw('5');
      break;
    case PS2_KEY_6:   // 9600 bps
      uart_bps = CALC_BPS(9600);
      send_raw('6');
      break;
    case PS2_KEY_7:   // 19200 bps
      uart_bps = CALC_BPS(19200);
      send_raw('7');
      break;
    case PS2_KEY_8:   // 38400 bps
      uart_bps = CALC_BPS(38400);
      send_raw('8');
      break;
    case PS2_KEY_9:   // 57600 bps
      uart_bps = CALC_BPS(57600);
      send_raw('9');
      break;
    case PS2_KEY_O:   // Odd Parity
      uart_parity = PARITY_ODD;
      send_raw('o');
      break;
    case PS2_KEY_E:   // Even Parity
      uart_parity = PARITY_EVEN;
      send_raw('e');
      break;
    case PS2_KEY_N:   // No Parity
      uart_parity = PARITY_NONE;
      send_raw('n');
      break;
    case PS2_KEY_BS:   // Use Backspace
      globalopts |= OPT_BACKSPACE;
      send_raw('b');
      send_raw('s');
      break;
    case PS2_KEY_DELETE | 0x80:   // Use Delete
      globalopts &= (uint8_t)~OPT_BACKSPACE;
      send_raw('d');
      send_raw('l');
      break;
    case PS2_KEY_Q:
      send_raw('<');
      sendhex(OSCCAL);
      send_raw(':');
      sendhex(globalopts);
      send_raw(':');
      sendhex(holdoff);
      send_raw(':');
      sendhex(pulselen);
      send_raw(':');
      sendhex(resetlen);
      send_raw('>');
      break;
    case PS2_KEY_W:   // Save Data
      eeprom_write_config();
      send_raw('w');
      break;
    }
  }
}

static void parse_key(uint8_t key, uint8_t keydown) {

  if((key & 0x7f)==PS2_KEY_ALT) {
    // turn on or off the ALT META flag
    meta = (meta & (uint8_t)~POLL_FLAG_ALT) | (keydown ? POLL_FLAG_ALT : 0);
  } else if((key & 0x7f)==PS2_KEY_LCTRL) {
    // turn on or off the CTRL META flag
    meta = (meta & (uint8_t)~POLL_FLAG_CONTROL) | (keydown ? POLL_FLAG_CONTROL : 0);
  } else if(key == PS2_KEY_LSHIFT) {
    meta = (meta & (uint8_t)~POLL_FLAG_LSHIFT) | (keydown ? POLL_FLAG_LSHIFT : 0);
  } else if(key == PS2_KEY_RSHIFT) {
    meta = (meta & (uint8_t)~POLL_FLAG_RSHIFT) | (keydown ? POLL_FLAG_RSHIFT : 0);
  }
  if((meta & POLL_FLAG_CTRL_ALT) == POLL_FLAG_CTRL_ALT && key == (0x80 | PS2_KEY_DELETE) && keydown) {
    // CTRL/ALT/DEL is pressed.
    // bring RESET line low
    if(globalopts & OPT_RESET_HI) {
      reset_set_hi();
      delay_reset(resetlen);
      reset_set_lo();
    } else {
      reset_set_lo();
      delay_reset(resetlen);
      reset_set_hi();
    }
  } else if(mode_config() && (meta&POLL_FLAG_CTRL_ALT) == POLL_FLAG_CTRL_ALT && key == PS2_KEY_BS && keydown) {
    // CTRL/ALT/BS config mode
    config ^= KB_CONFIG;
    if(!config) {
      uart_config(uart_bps, uart_length, uart_parity, uart_stop);
      ps2_putc(PS2_CMD_SET_RATE);
      ps2_putc(CALC_RATE(type_delay, type_rate));
    }
  } else if (config) {
    if(keydown) { // set parms on keydown
      set_options(key);
    }
  } else if (keydown) {
    switch (key) {
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
      default:
        ps2_to_ascii(key);
        break;
    }
  }
  ps2_to_xt(key,keydown);
}

static inline __attribute__((always_inline)) void poll_ps2_kb(void) {
  uint8_t key;
  poll_state_t state = POLL_ST_IDLE;

  for(;;) {
    if(ps2_data_available() != 0) {
      // kb sent data...
      key = ps2_getc();
      if(key == PS2_CMD_BAT) {
        state = POLL_ST_IDLE;
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

static inline __attribute__((always_inline)) void poll_xt_kb(void) {
  uint8_t key;
  //poll_state_t state = POLL_ST_IDLE;

  for(;;) {
    if(xt_data_available() != 0) {
      // kb sent data...
      key = xt_getc();
      xt_to_ps2(key);
    }
  }
}

//ISR(TIMER_vect) {
  //mat_scan();
  //sw_scan();
//}

/*static inline __attribute__((always_inline)) void scan_inputs(void) {
  uint8_t data;

  for(;;) {
    if(mat_data_available()) {
      data=mat_recv();
      uart_puthex(data);
    }
    if(sw_data_available()) {

      // handle special switches.
      data=sw_getc();
      uart_puthex(data);
      if(data & SW_UP) {
      } else {
        // only act on key depress
        switch(data & (SW_UP - 1)) {
          case SW_A:
            ps2_putc(PS2_KEY_F1);
            ps2_putc(PS2_KEY_UP);
            ps2_putc(PS2_KEY_F1);
            break;
          case SW_B:
            ps2_putc(PS2_KEY_F2);
            ps2_putc(PS2_KEY_UP);
            ps2_putc(PS2_KEY_F2);
            break;
        }
      }
    }
  }
}*/

void main(void) {
  mode_init();
  uart_init();

  eeprom_read_config();

  uart_config(uart_bps, uart_length, uart_parity, uart_stop);

  if(mode_device()) {
    ps2_init(PS2_MODE_DEVICE);
    xt_init(XT_MODE_HOST);

    //mat_init();
    //sw_init(_BV(SW_A) | _BV(SW_B));

    //timer_init();

    sei();
    uart_putc('d');

    poll_xt_kb();

    //scan_inputs();
  } else {
    data_init();
    reset_init();
    reset_set_hi();

    if(globalopts & OPT_STROBE_LO)
      data_strobe_hi();
    else
      data_strobe_lo();
    if(globalopts & OPT_RESET_HI)
      reset_set_lo();
    else
      reset_set_hi();
    ps2_init(PS2_MODE_HOST);
    xt_init(XT_MODE_DEVICE);

    sei();

    uart_putc('h');
    ps2_putc(PS2_CMD_RESET);

    poll_ps2_kb();
  }
  while(TRUE);
}

