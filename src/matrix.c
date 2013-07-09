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

    matrix.c: generic switch matrix routines

*/

#include <inttypes.h>
#include <avr/io.h>
#include "config.h"
#include "uart.h"
#include "matrix.h"

static uint8_t                    rx_buf[1 << MAT_RX_BUFFER_SHIFT];
static volatile uint8_t           rx_head;
static volatile uint8_t           rx_tail;

static MAT_COL_DTYPE              mat_save[1 << MAT_SCAN_SHIFT];
static volatile mat_state_t       mat_state;
static volatile uint8_t           mat_scan_idx;

static volatile uint8_t           mat_repeat;
static volatile uint8_t           mat_repeat_code;
static volatile uint16_t          mat_repeat_count;
static volatile uint16_t          mat_repeat_delay;
static volatile uint16_t          mat_repeat_period;
static volatile MAT_COL_DTYPE     mat_curr_value;

static inline void mat_store(uint8_t data) {
  rx_head = (rx_head + 1) & (sizeof(rx_buf) - 1); /* Calculate and store new index */

  //if ( tmphead == rx_tail ) {
    /* ERROR! Receive buffer overflow */
  //}
  
  rx_buf[rx_head] = data; /* Store received data in buffer */
}

static inline void mat_decode(MAT_COL_DTYPE new, uint8_t t) {
  uint8_t i = 0;
  MAT_COL_DTYPE old = mat_save[t];
  MAT_COL_DTYPE mask, result;
  uint8_t base = t * MAT_COL_LEN;

  // we have a key change.
  /*
    If old was:     00001010
    and new was:    01001000
    we need to xor: 01000010
    which will tell us what changed.

    Then, new & xor gives us new keys
    and old and xor gives us keys no longer pressed.
  */
  mask = new ^ old;
  result = (old & mask);

  while(result) {
    // we have keys no longer pressed.
    if(result & 1)
      mat_store((base + i) | MAT_KEY_UP);
    result = result >> 1;
    i++;
  }
  result=(new & mask);
  i = 0;
  while(result) {
    // we have keys pressed.
    if(result & 1)
      mat_store(base + i);
    result = result >> 1;
    i++;
  }
  mat_save[t] = new;
}

inline void mat_scan(void) {
  // this should be called 120 times/sec for each scanned row.
  uint8_t t;
  MAT_COL_DTYPE in;

  // this is where we scan.
  // we scan at 120Hz
  switch(mat_state) {
    default:
    case MAT_ST_PREP:
      if(mat_repeat) {
        mat_repeat_count--;
        if(!mat_repeat_count) {
          mat_repeat_count = mat_repeat_period;
          mat_store(mat_repeat_code);
        }
      }
      // do housekeeping
      in = mat_curr_value;
      t = mat_scan_idx;
      if(in != mat_save[t]) {
        //mat_decode(in, &mat_save[t], t * MAT_COL_LEN );
        mat_decode(in, t);
      }
      for(;;) {  // skip unused pins
        t = (t + 1) % ((1 << MAT_SCAN_SHIFT) - 1);
        if((uint16_t)(1  << t) & (uint16_t)MAT_ROW_MASK) {
          break;
        }
      }
      mat_scan_idx = t;
      // we just read, prep now.
      // set pin low:
      MAT_SET_ROW(1 << t);
      mat_state = MAT_ST_READ;
      break;
    case MAT_ST_READ:
      // we just prepped, read.
      mat_curr_value = (MAT_COL_DTYPE)~(MAT_GET_COL()) & MAT_COL_MASK;
      mat_state = MAT_ST_PREP;
      break;
  }
}

void mat_init(void) {
  mat_state = MAT_ST_PREP;
  mat_repeat = FALSE;  // set keyboard repeat to 0.
  mat_set_repeat_delay(250);        // wait 250 ms
  mat_set_repeat_period(32);        // once every 32 ms
  MAT_SET_COL_MASK();               // turn on column pullups.
}

void mat_set_repeat_delay(uint16_t ms) {
  // 1800 ticks/sec, .5 ms per tick.
  mat_repeat_delay = (ms << 1);
  mat_repeat_count = mat_repeat_delay;
}

void mat_set_repeat_period(uint16_t period) {
  mat_repeat_period = (period << 1);
}

void mat_set_repeat_code(uint8_t code) {
  if(code != mat_repeat_code) {
    mat_repeat_count = mat_repeat_delay;
    mat_repeat_code = code;
    mat_repeat = TRUE;
  }
}

void mat_clear_repeat_code(void) {
  mat_repeat = FALSE;
}

uint8_t mat_data_available(void) {
  return ( rx_head != rx_tail ); /* Return 0 (FALSE) if the receive buffer is empty */
}

uint8_t mat_recv( void ) {
	
	while (rx_head == rx_tail);
	rx_tail = (rx_tail + 1) & (sizeof(rx_buf) - 1); /* Calculate and store buffer index */
	
	return rx_buf[rx_tail];           /* Return data */
}

