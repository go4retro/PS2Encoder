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
#ifndef PS2_LIB_H
#define PS2_LIB_H 1

#define PS2_RX_BUFFER_MASK   ((1 << PS2_RX_BUFFER_SHIFT) - 1)
#define PS2_TX_BUFFER_MASK   ((1 << PS2_TX_BUFFER_SHIFT) - 1)
/*
 * After a device sends a byte to host, it has to holdoff for a while
 * before doing anything else.  One KB I tested this is 2.14mS.
 */
 
#define PS2_HALF_CYCLE 40 // ~42 uS when all is said and done.
#define PS2_SEND_HOLDOFF_COUNT  ((uint8_t)(2140/PS2_HALF_CYCLE)) 

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

void PS2_delay(uint16_t ms);
#define PS2_set_CLK()     do { PS2_PORT_CLK_OUT |= ( PS2_PIN_CLK); PS2_PORT_DDR_CLK &= (uint8_t)~(PS2_PIN_CLK); } while(0)

#define PS2_clear_CLK()   do { PS2_PORT_DDR_CLK |= (PS2_PIN_CLK); PS2_PORT_CLK_OUT &= (uint8_t)~( PS2_PIN_CLK); } while(0) 
#define PS2_read_CLK()    (PS2_PORT_CLK_IN & (PS2_PIN_CLK))

#define PS2_set_DATA()    do { PS2_PORT_DATA_OUT |= ( PS2_PIN_DATA); PS2_PORT_DDR_DATA &= (uint8_t)~(PS2_PIN_DATA); } while(0)
#define PS2_clear_DATA()  do { PS2_PORT_DDR_DATA |= (PS2_PIN_DATA); PS2_PORT_DATA_OUT &= (uint8_t)~( PS2_PIN_DATA); } while(0)
#define PS2_read_DATA()   (PS2_PORT_DATA_IN & (PS2_PIN_DATA)) 

void PS2_enable_IRQ_CLK_Rise(void);
void PS2_enable_IRQ_CLK_Fall(void);
void PS2_disable_IRQ_CLK(void);

void ps2_timer_irq_set(uint8_t us);
void ps2_timer_irq_off(void);

uint8_t PS2_get_state(void);
void PS2_set_state(uint8_t state);
uint8_t PS2_get_count(void);

void PS2_write_byte(void);
void PS2_read_byte(void);
void PS2_commit_read_byte(void);
uint8_t PS2_data_to_send(void);

void PS2_write_bit(void);
void PS2_write_parity(void);
void PS2_read_bit(void);
void PS2_clear_counters(void);

void ps2_trigger_send(void);
void ps2_clk_irq(void);
void ps2_timer_irq(void);

void ps2_lib_init(void);


#endif

