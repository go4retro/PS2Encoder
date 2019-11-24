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

    flags.h: Definitions for some global flags

*/

#ifndef FLAGS_H
#define FLAGS_H

#include "uart.h"

/* Global options, variable defined in main.c */
extern uint8_t globalopts;
extern uint8_t holdoff;
extern uint8_t pulselen;
extern uint8_t resetlen;
extern uint16_t uart_bps;
extern uartlen_t uart_length;
extern uartstop_t uart_stop;
extern uartpar_t uart_parity;

/* Values for those flags */
#define OPT_CRLF         (1 << 0)
#define OPT_STROBE_LO    (1 << 1)
#define OPT_BACKSPACE    (1 << 2)
#define OPT_RESET_HI     (1 << 3)

#endif
