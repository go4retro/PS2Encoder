/* PS2Encoder - PS/2 Keyboard Encoder
   Copyright 2008,2009 Jim Brain <brain@jbrain.com>

   This code is a modification of uart functions in sd2iec:
   Copyright (C) 2007,2008  Ingo Korb <ingo@akana.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License only.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   uart.c: UART access routines

*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include "config.h"
#include "uart.h"

static uint8_t tx1_buf[1 << UART1_TX_BUFFER_SHIFT];
static volatile uint8_t tx1_tail;
static volatile uint8_t tx1_head;

#if defined UART1_RX_BUFFER_SHIFT && UART1_RX_BUFFER_SHIFT > 0

static uint8_t rx1_buf[1 << UART1_RX_BUFFER_SHIFT];
static volatile uint8_t rx1_tail;
static volatile uint8_t rx1_head;

#endif

#ifdef ENABLE_UART2

static uint8_t tx2_buf[1 << UART2_TX_BUFFER_SHIFT];
static volatile uint8_t tx2_tail;
static volatile uint8_t tx2_head;

#  if defined UART2_RX_BUFFER_SHIFT && UART2_RX_BUFFER_SHIFT > 0

static uint8_t rx2_buf[1 << UART2_RX_BUFFER_SHIFT];
static volatile uint8_t rx2_head;
static volatile uint8_t rx2_tail;
#  endif

#endif

ISR(USART_UDRE_vect) {
  if (tx1_tail == tx1_head) return;
  UDR = tx1_buf[tx1_tail];
  tx1_tail = (tx1_tail+1) & (sizeof(tx1_buf)-1);
  if (tx1_tail == tx1_head)
    UCSRB &= ~ _BV(UDRIE);
}

void uart_putc(char c) {
  uint8_t t=(tx1_head+1) & (sizeof(tx1_buf)-1);
  while (t == tx1_tail);   // wait for free space
  tx1_buf[tx1_head] = c;
  tx1_head = t;
  //if (tx1_tail == tx1_head) PORTD |= _BV(PD7);
  UCSRB |= _BV(UDRIE);
}

void uart_puthex(uint8_t num) {
  uint8_t tmp;
  tmp = (num & 0xf0) >> 4;
  if (tmp < 10)
    uart_putc('0'+tmp);
  else
    uart_putc('a'+tmp-10);

  tmp = num & 0x0f;
  if (tmp < 10)
    uart_putc('0'+tmp);
  else
    uart_putc('a'+tmp-10);
}

void uart_trace(void *ptr, uint16_t start, uint16_t len) {
  uint16_t i;
  uint8_t j;
  uint8_t ch;
  uint8_t *data = ptr;

  data+=start;
  for(i=0;i<len;i+=16) {

    uart_puthex(start>>8);
    uart_puthex(start&0xff);
    uart_putc('|');
    uart_putc(' ');
    for(j=0;j<16;j++) {
      if(i+j<len) {
        ch=*(data + j);
        uart_puthex(ch);
      } else {
        uart_putc(' ');
        uart_putc(' ');
      }
      uart_putc(' ');
    }
    uart_putc('|');
    for(j=0;j<16;j++) {
      if(i+j<len) {
        ch=*(data++);
        if(ch<32 || ch>0x7e)
          ch='.';
        uart_putc(ch);
      } else {
        uart_putc(' ');
      }
    }
    uart_putc('|');
    uart_putcrlf();
    start+=16;
  }
}

static int ioputc(char c, FILE *stream) {
  if (c == '\n') uart_putc('\r');
  uart_putc(c);
  return 0;
}

uint8_t uart_getc(void) {
#if !defined UART1_RX_BUFFER_SHIFT || UART1_RX_BUFFER_SHIFT == 0
  loop_until_bit_is_set(UCSRA,RXC);
  return UDR;
#else
  uint8_t tmptail;

  while ( rx2_head == rx2_tail ) { ; }
  tmptail = ( rx2_tail + 1 ) & (sizeof(rx2_buf)-1);/* Calculate buffer index */

  rx2_tail = tmptail;                /* Store new index */

  return rx2_buf[tmptail];           /* Return data */
#endif
}


void uart_flush(void) {
  while (tx1_tail != tx1_head) ;
}

void uart_puts_P(prog_char *text) {
  uint8_t ch;

  while ((ch = pgm_read_byte(text++))) {
    uart_putc(ch);
  }
}

void uart_putcrlf(void) {
  uart_putc(13);
  uart_putc(10);
}

static FILE mystdout = FDEV_SETUP_STREAM(ioputc, NULL, _FDEV_SETUP_WRITE);

#ifdef ENABLE_UART2
ISR(USART1_UDRE_vect) {
  if (tx2_tail == tx2_head) return;
  UDR1 = tx2_buf[tx2_tail];
  tx2_tail = (tx2_tail+1) & (sizeof(tx2_buf)-1);
  if (tx2_tail == tx2_head)
    UCSR1B &= ~ _BV(UDRIE);
}

ISR(USART1_RX_vect) {
  uint8_t data;
  uint8_t tmphead;

  /* Read the received data */
  data = UDR1;

  /* Calculate buffer index */
  tmphead = ( rx2_head + 1 ) & (sizeof(rx2_buf)-1);
  rx2_head = tmphead;      /* Store new index */

  if ( tmphead == rx2_tail ) {
    /* ERROR! Receive buffer overflow */
  }

  rx2_buf[tmphead] = data; /* Store received data in buffer */
}

void uart2_putc(char c) {
  uint8_t t=(tx2_head+1) & (sizeof(tx2_buf)-1);
  UCSR1B &= ~ _BV(UDRIE);   // turn off RS232 irq
  tx2_buf[tx2_head] = c;
  tx2_head = t;
  UCSR1B |= _BV(UDRIE);
}

#if !defined UART2_RX_BUFFER_SHIFT || UART2_RX_BUFFER_SHIFT == 0
  loop_until_bit_is_set(UCSR1A,RXC);
  return UDR;
#else
uint8_t uart2_getc(void) {
  uint8_t tmptail;

  while ( rx2_head == rx2_tail ) { ; }
  tmptail = ( rx2_tail + 1 ) & (sizeof(rx2_buf)-1);/* Calculate buffer index */

  rx2_tail = tmptail;                /* Store new index */

  return rx2_buf[tmptail];           /* Return data */
#endif
}

void uart2_puts(char* str) {
  while(*str)
    uart2_putc(*str++);
}

#endif

#ifdef DYNAMIC_BPS_RATE
void uart_set_bps(uint16_t rate) {
  UBRRH = rate >> 8;
  UBRRL = rate & 0xff;
}

#  ifdef ENABLE_UART2
void uart2_set_bps(uint16_t rate) {
  UBRR1H = rate >> 8;
  UBRR1L = rate & 0xff;
}
#  endif
#endif

void uart_init(void) {
  /* Seriellen Port konfigurieren */

  UBRRH = CALC_BPS(UART1_BAUDRATE) >> 8;
  UBRRL = CALC_BPS(UART1_BAUDRATE) & 0xff;

  UCSRB = _BV(RXEN) | _BV(TXEN);
  // I really don't like random #ifdefs in the code =(
#if defined __AVR_ATmega8__ || __AVR_ATmega16__ || defined __AVR_ATmega32__
  UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0);
#else
  UCSRC = _BV(UCSZ1) | _BV(UCSZ0);
#endif

  stdout = &mystdout;

  //UCSRB |= _BV(UDRIE);
  tx1_tail  = 0;
  tx1_head = 0;

#ifdef ENABLE_UART2
  UBRR1H = CALC_BPS(ENABLE_UART2_BAUDRATE) >> 8;
  UBRR1L = CALC_BPS(ENABLE_UART2_BAUDRATE) & 0xff;

  UCSR1B = _BV(RXCIE1) | _BV(RXEN1) | _BV(TXEN1);
  UCSR1C = _BV(UCSZ11) | _BV(UCSZ10);

  tx2_tail  = 0;
  tx2_head = 0;
//#ifdef ENABLE_UART2_ASYNC_RECV
  rx2_tail  = 0;
  rx2_head = 0;
//#endif
#endif
}
