/*
    C=Key - Commodore <-> PS/2 Interface
    Copyright Jim Brain and RETRO Innovations, 2004-2012

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

    kb.h: Definitions for generic keyboard routines

*/
#ifndef MATRIX_H
#define MATRIX_H

typedef enum {MAT_ST_PREP,
              MAT_ST_READ
             } mat_state_t;

#define MAT_SET_ROW_LO(x)         do {  \
                                        MAT_ROW_LO_DDR &= (uint8_t)~(MAT_ROW_MASK & 0xff); \
                                        MAT_ROW_LO_DDR |= (x) &0xff; \
                                        MAT_ROW_LO_OUT &= (uint8_t)~(MAT_ROW_MASK & 0xff); \
                                        MAT_ROW_LO_OUT |= (uint8_t)~((x) & 0xff); \
                                     } while(0)
#ifdef MAT_ROW_HI_OUT
#  define MAT_SCAN_SHIFT          4
#  define MAT_SET_ROW_HI(x)       do {  \
                                        MAT_ROW_HI_DDR &= (uint8_t)~(MAT_ROW_MASK >> 8); \
                                        MAT_ROW_HI_DDR |= ((x) >> 8); \
                                        MAT_ROW_HI_OUT &= (uint8_t)~(MAT_ROW_MASK >> 8); \
                                        MAT_ROW_HI_OUT |= (uint8_t)~((x) >> 8); \
                                     } while(0)
#  define MAT_SET_ROW(x)          do {MAT_SET_ROW_LO(x); MAT_SET_ROW_HI(x); } while(0)
#else
#  define MAT_SCAN_SHIFT          3
#  define MAT_SET_ROW(x)          MAT_SET_ROW_LO(x)
#endif

#define MAT_SET_COL_MASK_LO()     do { \
                                        MAT_COL_LO_OUT |= (MAT_COL_MASK & 0xff); \
                                        MAT_COL_LO_DDR &= (uint8_t)~(MAT_COL_MASK & 0xff); \
                                     } while(0)
#ifdef MAT_COL_HI_IN
#  define MAT_COL_DTYPE           uint16_t
#  define MAT_SET_COL_MASK_HI()   do { \
                                        MAT_COL_HI_OUT |= (MAT_COL_MASK >> 8); \
                                        MAT_COL_HI_DDR &= ~(MAT_COL_MASK >> 8); \
                                     } while(0)
#  define MAT_SET_COL_MASK()      do { MAT_SET_COL_MASK_LO(); MAT_SET_COL_MASK_HI(); } while (0)
#  define MAT_GET_COL()           (MAT_COL_HI_IN << 8) | (MAT_COL_LO_IN)
#else
#  define MAT_COL_DTYPE           uint8_t
#  define MAT_GET_COL()           (MAT_COL_LO_IN)
#  define MAT_SET_COL_MASK()      MAT_SET_COL_MASK_LO()
#endif

#define _B15 0
#define _B14 0
#define _B13 0
#define _B12 0
#define _B11 0
#define _B10 0
#define _B9 0
#define _B8 0
#define _B7 0
#define _B6 0
#define _B5 0
#define _B4 0
#define _B3 0
#define _B2 0
#define _B1 0
#define _B0 0
#if MAT_COL_MASK & 32768
#  undef _B15
#  define _B15 1
#endif
#if MAT_COL_MASK & 16384
#  undef _B14
#  define _B14 1
#endif
#if MAT_COL_MASK & 8192
#  undef _B13
#  define _B13 1
#endif
#if MAT_COL_MASK & 4096
#  undef _B12
#  define _B12 1
#endif
#if MAT_COL_MASK & 2048
#  undef _B11
#  define _B11 1
#endif
#if MAT_COL_MASK & 1024
#  undef _B10
#  define _B10 1
#endif
#if MAT_COL_MASK & 512
#  undef _B9
#  define _B9 1
#endif
#if MAT_COL_MASK & 256
#  undef _B8
#  define _B8 1
#endif
#if MAT_COL_MASK & 128
#  undef _B7
#  define _B7 1
#endif
#if MAT_COL_MASK & 64
#  undef _B6
#  define _B6 1
#endif
#if MAT_COL_MASK & 32
#  undef _B5
#  define _B5 1
#endif
#if MAT_COL_MASK & 16
#  undef _B4
#  define _B4 1
#endif
#if MAT_COL_MASK & 8
#  undef _B3
#  define _B3 1
#endif
#if MAT_COL_MASK & 4
#  undef _B2
#  define _B2 1
#endif
#if MAT_COL_MASK & 2
#  undef _B1
#  define _B1 1
#endif
#if MAT_COL_MASK & 1
#  undef _B0
#  define _B0 1
#endif
#define MAT_COL_LEN (_B0 + _B1 + _B2 + _B3 + _B4 + _B5 + _B6 + _B7 + _B8 + _B9 + _B10 + _B11 + _B12 + _B13 + _B14 + _B15)

#define MAT_KEY_UP                0x80

void mat_init(void);
void mat_set_repeat_delay(uint16_t ms);
void mat_set_repeat_period(uint16_t period);
void mat_set_repeat_code(uint8_t code);
void mat_clear_repeat_code(void);
uint8_t mat_data_available( void );
uint8_t mat_recv( void );
void mat_scan(void);

#endif

