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

// timing information derived from http://panda.cs.ndsu.nodak.edu/~achapwes/PICmicro/PS2/ps2.htm

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "config.h"
#include "avrcompat.h"
#include "ps2.h"
#include "ps2_lib.h"
#include "uart.h"

static unsigned char rxbuf[1 << PS2_RX_BUFFER_SHIFT];
static volatile uint8_t rx_head;
static volatile uint8_t rx_tail;
static unsigned char txbuf[1 << PS2_TX_BUFFER_SHIFT];
static volatile uint8_t tx_head;
static volatile uint8_t tx_tail;

static volatile uint8_t PS2_State;
static volatile uint8_t PS2_Byte;
static volatile uint8_t PS2_Bit_Count;
static volatile uint8_t PS2_One_Count;

static volatile uint8_t PS2_LEDs;
static volatile uint8_t PS2_CodeSet;

void PS2_enable_IRQ_CLK_Rise() {
	//GICR &= ~(1 << INT1);
	GIFR |= (1<<INTF1);
	// rising edge
	MCUCR |= (1 << ISC11) | (1<< ISC10);
	// turn on
	GICR |= (1 << INT1);
}

void PS2_enable_IRQ_CLK_Fall() {
	//GICR &= ~(1 << INT1);
	GIFR |= (1<<INTF1);
	// rising edge
	MCUCR |= (1 << ISC11);
	MCUCR &=(uint8_t)~(1<<ISC10);
	// turn on
	GICR |= (1 << INT1);
}

void PS2_disable_IRQ_CLK() {
	GICR &= (uint8_t)~(1 << INT1);
}

void PS2_delay(uint16_t ms) {
  // careful, we use the same timer as the IRQ.
  // set to /1024. and CTC mode
  OCR=14;  // 14 is ~1ms
  TCNT=0;
  TCCR = TCCR_DATA_DELAY;
  do {
    while(!(TIFR & TIFR_DATA));
    TIFR|=TIFR_DATA;
  } while (ms-- > 0);
  // shut timer off.
  TCCR=0;
}

void ps2_timer_irq_set(uint8_t us) {
	//TCCR0 &=~(1<<CS01);
	//TIMSK &= ~(1<<OCIE0);
	TIFR |= TIFR_DATA;
	// clear TCNT0;
	TCNT=0;
	// set the count...
#if F_CPU > 14000000
  // us is uS....  Need to * 14 to get ticks, then divide by 8...
  // cheat... * 14 / 8 = *2 = <<1  
	OCR=(uint8_t)(us<<1);
#elif F_CPU > 7000000
  OCR=us;
#else
  OCR=us >> 1;
#endif
	// set output compare IRQ
	TIMSK |= TIMSK_DATA;
	// set prescaler to System Clock/8 and Compare Timer
  TCCR =TCCR_DATA;
}

void ps2_timer_irq_off() {
	// turn off timer
  TCCR =0;
	TIMSK &=(uint8_t)~TIMSK_DATA;
}

uint8_t PS2_get_state(void) {
  return PS2_State;
}

void PS2_set_state(uint8_t state) {
  PS2_State=state;
}

uint8_t PS2_get_count(void) {
  return PS2_Bit_Count;
}

uint8_t ps2_getc( void ) {
	uint8_t tmptail;
	
	while ( rx_head == rx_tail ) {
    // wait for char to arrive, if none in Q
    ;
	}
  // Calculate buffer index
	tmptail = ( rx_tail + 1 ) & PS2_RX_BUFFER_MASK;
  // Store new index
	rx_tail = tmptail;
	return rxbuf[tmptail];
}

void ps2_putc( uint8_t data ) {
	uint8_t tmphead;
	// Calculate buffer index
	tmphead = ( tx_head + 1 ) & PS2_TX_BUFFER_MASK; 
	while ( tmphead == tx_tail ) {
    // Wait for free space in buffer
    ;
  }
  
  //printHex(data);
  // Store data in buffer
	txbuf[tmphead] = data;
  // Store new index
	tx_head = tmphead;

  // turn off IRQs
  cli();
  if(PS2_State == PS2_ST_IDLE) {
    // start transmission;
    ps2_trigger_send();
  }
  // turn on IRQs
  sei();
}

uint8_t ps2_data_available( void ) {
	return ( rx_head != rx_tail ); /* Return 0 (FALSE) if the receive buffer is empty */
}

void PS2_write_byte(void) {
  uint8_t tmp;
  /* Calculate buffer index */
  tmp = ( rx_head + 1 ) & PS2_RX_BUFFER_MASK;
  rx_head = tmp;      /* Store new index */

  if ( tmp == rx_tail ) {
    /* ERROR! Receive buffer overflow */
  }

  //debug('i');
  //printHex(PS2_Byte);
  rxbuf[tmp] = PS2_Byte; /* Store received data in buffer */
}

void PS2_read_byte(void) {
  PS2_Bit_Count=0;
  PS2_One_Count=0;
  PS2_Byte = txbuf[( tx_tail + 1 ) & PS2_TX_BUFFER_MASK];  /* Start transmition */
  //debug('o');
  //printHex(PS2_Byte);
}

void PS2_commit_read_byte(void) {
  tx_tail = ( tx_tail + 1 ) & PS2_TX_BUFFER_MASK;      /* Store new index */
}

uint8_t PS2_data_to_send(void) {
  return ( tx_head != tx_tail );
}

void PS2_write_bit() {
  PS2_State=PS2_ST_PREP_BIT;
	// set DATA..
	switch (PS2_Byte & 1) {
		case 0:
			PS2_clear_DATA();
			break;
		case 1:
			PS2_One_Count++;
			PS2_set_DATA();
			break;
	}
	// shift right.
	PS2_Byte= PS2_Byte >> 1;
	PS2_Bit_Count++;
	// valid data now.
}

void PS2_read_bit(void) {
  PS2_Byte=PS2_Byte>>1;
  PS2_Bit_Count++;
  if(PS2_read_DATA()) {
    PS2_Byte|=0x80;
    PS2_One_Count++;
  }
}

void PS2_write_parity(void) {
  if((PS2_One_Count & 1) == 1) {
    PS2_clear_DATA();
  } else {
    PS2_set_DATA();
  }
}

void PS2_clear_counters(void) {
  PS2_Byte = 0;
  PS2_Bit_Count=0;
  PS2_One_Count=0;
}

ISR(SIG_INTERRUPT1) {
  ps2_clk_irq();
}

ISR(SIG_OUTPUT_COMPARE) {
  ps2_timer_irq();
}

static void ps2_clear_buffers(void) {
  tx_head=0;
  tx_tail=0;
  rx_head=0;
  rx_tail=0;
}


void PS2_handle_cmds(uint8_t data) {
  uint8_t i;
  
    switch(data) {
      case PS2_CMD_ACK:
        //ignore.
        break;
      case PS2_CMD_RESET:
        ps2_putc(PS2_CMD_ACK);
        ps2_putc(PS2_CMD_BAT);
        break;
      case PS2_CMD_DISABLE:
        // we should disable sending output if we receive this command.
      case PS2_CMD_ENABLE:
        //clear out KB buffers 
        cli();
        ps2_clear_buffers();
        sei();
        ps2_putc(PS2_CMD_ACK);
        break;
      default:
        ps2_putc(PS2_CMD_ACK);
        break;
      case PS2_CMD_ECHO:
        ps2_putc(PS2_CMD_ECHO);
        break;
      case PS2_CMD_SET_CODE_SET:
        ps2_putc(PS2_CMD_ACK);
        i=ps2_getc();
        if(i == 0) {
          ps2_putc(PS2_CodeSet);
        } else {
          PS2_CodeSet=i;
        }
        break;
      case PS2_CMD_SET_RATE:
        // this should to be caught in another area, ignore if received here.
        break;
      case PS2_CMD_READ_ID:
        ps2_putc(PS2_CMD_ACK);
        ps2_putc(0xab);
        ps2_putc(0x83);
        break;
      case PS2_CMD_LEDS:
        ps2_putc(PS2_CMD_ACK);
        PS2_LEDs=ps2_getc()&0x07;
        ps2_putc(PS2_CMD_ACK);
        break;
      case PS2_CMD_RESEND:
        break;
    }
}

unsigned int PS2_get_typematic_delay(uint8_t rate) {
  return (((rate & 0x30) >> 5) + 1) * 250;
}

unsigned int PS2_get_typematic_period(uint8_t rate) {
  return ((8 + (rate & 0x07)) * (1 << ((rate & 0x18) >> 3)) << 2);
}

void ps2_lib_init(void) {
  ps2_clear_buffers();
  PS2_LEDs=0;
  PS2_CodeSet=2;
  
  PS2_set_CLK();
  PS2_set_DATA();
  
  PS2_State=PS2_ST_IDLE;
}
