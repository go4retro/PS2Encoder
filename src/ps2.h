/*
    Copyright Jim Brain and Brain Innovations, 2004
  
    This file is part of C=Key.

    C=Key is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    C=Key is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with C=Key; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef PS2_H
#define PS2_H 1

#define PS2_ST_IDLE           0
#define PS2_ST_PREP_START     1
#define PS2_ST_SEND_START     2
#define PS2_ST_PREP_BIT       3
#define PS2_ST_SEND_BIT       4
#define PS2_ST_PREP_PARITY    5
#define PS2_ST_SEND_PARITY    6
#define PS2_ST_PREP_STOP      7
#define PS2_ST_SEND_STOP      8

#define PS2_ST_HOLDOFF        9
#define PS2_ST_WAIT_START     10
#define PS2_ST_GET_START      11
#define PS2_ST_WAIT_BIT       12
#define PS2_ST_GET_BIT        13
#define PS2_ST_WAIT_PARITY    14
#define PS2_ST_GET_PARITY     15
#define PS2_ST_WAIT_STOP      16
#define PS2_ST_GET_STOP       17
#define PS2_ST_GET_ACK        18
#define PS2_ST_WAIT_ACK       19
#define PS2_ST_WAIT_ACK2      20
#define PS2_ST_HOST_INHIBIT   21
#define PS2_ST_WAIT_RESPONSE  22

#define PS2_MODE_DEVICE       1
#define PS2_MODE_HOST         2

#define PS2_KEY_UP            0xf0
#define PS2_KEY_EXT           0xe0
#define PS2_KEY_EXT_2         0xe1


// normal keys
#define PS2_KEY_F5            0x03
#define PS2_KEY_F3            0x04
#define PS2_KEY_F1            0x05
#define PS2_KEY_F2            0x06
#define PS2_KEY_F8            0x0a
#define PS2_KEY_F6            0x0b
#define PS2_KEY_F4            0x0c
#define PS2_KEY_TAB           0x0d
#define PS2_KEY_BACKQUOTE     0x0e
#define PS2_KEY_ALT           0x11
#define PS2_KEY_LSHIFT        0x12
#define PS2_KEY_LCTRL         0x14
#define PS2_KEY_Q             0x15
#define PS2_KEY_1             0x16
#define PS2_KEY_Z             0x1a
#define PS2_KEY_S             0x1b
#define PS2_KEY_A             0x1c
#define PS2_KEY_W             0x1d
#define PS2_KEY_2             0x1e
#define PS2_KEY_C             0x21
#define PS2_KEY_X             0x22
#define PS2_KEY_D             0x23
#define PS2_KEY_E             0x24
#define PS2_KEY_4             0x25
#define PS2_KEY_3             0x26
#define PS2_KEY_SPACE         0x29
#define PS2_KEY_V             0x2a
#define PS2_KEY_F             0x2b
#define PS2_KEY_T             0x2c
#define PS2_KEY_R             0x2d
#define PS2_KEY_5             0x2e
#define PS2_KEY_N             0x31
#define PS2_KEY_B             0x32
#define PS2_KEY_H             0x33
#define PS2_KEY_G             0x34
#define PS2_KEY_Y             0x35
#define PS2_KEY_6             0x36
#define PS2_KEY_M             0x3a
#define PS2_KEY_J             0x3b
#define PS2_KEY_U             0x3c
#define PS2_KEY_7             0x3d
#define PS2_KEY_8             0x3e
#define PS2_KEY_COMMA         0x41
#define PS2_KEY_K             0x42
#define PS2_KEY_I             0x43
#define PS2_KEY_O             0x44
#define PS2_KEY_0             0x45
#define PS2_KEY_9             0x46
#define PS2_KEY_PERIOD        0x49
#define PS2_KEY_SLASH         0x4a
#define PS2_KEY_L             0x4b
#define PS2_KEY_SEMICOLON     0x4c
#define PS2_KEY_P             0x4d
#define PS2_KEY_MINUS         0x4e
#define PS2_KEY_APOSTROPHE    0x52
#define PS2_KEY_LBRACKET      0x54
#define PS2_KEY_EQUALS        0x55
#define PS2_KEY_CAPS_LOCK     0x58
#define PS2_KEY_RSHIFT        0x59
#define PS2_KEY_ENTER         0x5a
#define PS2_KEY_RBRACKET      0x5b
#define PS2_KEY_BACKSLASH     0x5d
#define PS2_KEY_BS            0x66
#define PS2_KEY_NUM_1         0x69
#define PS2_KEY_NUM_4         0x6b
#define PS2_KEY_NUM_7         0x6c
#define PS2_KEY_NUM_0         0x70
#define PS2_KEY_NUM_PERIOD    0x71
#define PS2_KEY_NUM_2         0x72
#define PS2_KEY_NUM_5         0x73
#define PS2_KEY_NUM_6         0x74
#define PS2_KEY_NUM_8         0x75
#define PS2_KEY_ESC           0x76
#define PS2_KEY_NUM_LOCK      0x77
#define PS2_KEY_NUM_3         0x7a
#define PS2_KEY_NUM_9         0x7d
#define PS2_KEY_F7            0x83

// extended keys
#define PS2_KEY_RALT          0x11
#define PS2_KEY_ECTRL         0x12
#define PS2_KEY_RCTRL         0x14
#define PS2_KEY_NUM_SLASH     0x4a
#define PS2_KEY_NUM_ENTER     0x5a
#define PS2_KEY_END           0x69
#define PS2_KEY_CRSR_LEFT     0x6b
#define PS2_KEY_HOME          0x6c
#define PS2_KEY_INSERT        0x70
#define PS2_KEY_DELETE        0x71
#define PS2_KEY_CRSR_DOWN     0x72
#define PS2_KEY_CRSR_RIGHT    0x74
#define PS2_KEY_CRSR_UP       0x75
#define PS2_KEY_PAGE_DOWN     0x7a
#define PS2_KEY_PRINT_SCREEN  0x7c
#define PS2_KEY_PAGE_UP       0x7d

// new ones
#define PS2_KEY_PCTRL         0x14
#define PS2_KEY_PAUSE         0x77


#define PS2_CMD_LEDS          0xed
#define PS2_CMD_ECHO          0xee
#define PS2_CMD_SET_CODE_SET  0xf0
#define PS2_CMD_READ_ID       0xf2
#define PS2_CMD_SET_RATE      0xf3
#define PS2_CMD_ENABLE        0xf4
#define PS2_CMD_DISABLE       0xf5
#define PS2_CMD_DEFAULT       0xf6
#define PS2_CMD_RESEND        0xfe
#define PS2_CMD_RESET         0xff

#define PS2_CMD_ERROR         0x00
#define PS2_CMD_BAT           0xaa
#define PS2_CMD_ECHO_RESP     0xee
#define PS2_CMD_ACK           0xfa
#define PS2_CMD_BAT_FAILURE   0xfc
#define PS2_CMD_OVERFLOW      0xff

#define PS2_LED_SCROLL_LOCK   (1 << 0)
#define PS2_LED_NUM_LOCK      (1 << 1)
#define PS2_LED_CAPS_LOCK     (1 << 2)



void ps2_init(uint8_t mode);

uint8_t ps2_getc(void);
void ps2_putc(uint8_t data);
uint8_t ps2_data_available(void);

void ps2_handle_cmds(uint8_t data);
uint16_t ps2_get_typematic_delay(uint8_t rate);
uint16_t ps2_get_typematic_period(uint8_t rate);

// Add 1 and multiply by 250ms to get time
#define PS2_GET_DELAY(rate)   ((rate & 0x60) >> 5)
// Multiply by 4.17 to get CPS (or << 2)
#define PS2_GET_RATE(rate)    ((8 + (rate & 0x07)) * (1 << ((rate & 0x18) >> 3)))   
#define CALC_RATE(delay,rate) ((rate & 0x1f) + ((delay & 0x03) << 5))

#endif

