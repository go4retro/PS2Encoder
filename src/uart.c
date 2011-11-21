/*
    PS2Encoder - PS2 Keyboard to serial/parallel converter
    Copyright Jim Brain and RETRO Innovations, 2008-2011

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

#ifdef ENABLE_UART1
static uint8_t tx1_buf[1 << UART1_TX_BUFFER_SHIFT];
static volatile uint8_t tx1_tail;
static volatile uint8_t tx1_head;

#  if defined UART1_RX_BUFFER_SHIFT && UART1_RX_BUFFER_SHIFT > 0

static uint8_t rx1_buf[1 << UART1_RX_BUFFER_SHIFT];
static volatile uint8_t rx1_tail;
static volatile uint8_t rx1_head;

#  endif
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

#if defined ENABLE_UART1
ISR(USARTA_UDRE_vect) {
  if (tx1_tail == tx1_head) return;
  UDRA = tx1_buf[tx1_tail];
  tx1_tail = (tx1_tail+1) & (sizeof(tx1_buf)-1);
  if (tx1_tail == tx1_head)
    UCSRAB &= ~ _BV(UDRIEA);
}

void uart_putc(char c) {
  uint8_t t=(tx1_head+1) & (sizeof(tx1_buf)-1);
  while (t == tx1_tail);   // wait for free space
  tx1_buf[tx1_head] = c;
  tx1_head = t;
  UCSRAB |= _BV(UDRIEA);
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
  loop_until_bit_is_set(UCSRAA,RXCA);
  return UDRA;
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
#endif

#ifdef ENABLE_UART2
ISR(USARTB_UDRE_vect) {
  if (tx2_tail == tx2_head) return;
  UDR1 = tx2_buf[tx2_tail];
  tx2_tail = (tx2_tail+1) & (sizeof(tx2_buf)-1);
  if (tx2_tail == tx2_head)
    UCSRBB &= ~ _BV(UDRIEB);
}

ISR(USART1_RX_vect) {
  uint8_t data;
  uint8_t tmphead;

  /* Read the received data */
  data = UDRB;

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
  UCSR1BB &= ~ _BV(UDRIEB);   // turn off RS232 irq
  tx2_buf[tx2_head] = c;
  tx2_head = t;
  UCSR1BB |= _BV(UDRIEB);
}

uint8_t uart2_getc(void) {
#if !defined UART2_RX_BUFFER_SHIFT || UART2_RX_BUFFER_SHIFT == 0
  loop_until_bit_is_set(UCSRBA,RXCB);
  return UDRB;
#else
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

#ifdef DYNAMIC_UART

#  ifdef ENABLE_UART1

void uart_config(uint16_t rate, uartlen_t length, uartpar_t parity, uartstop_t stopbits) {
  UBRRAH = rate >> 8;
  UBRRAL = rate & 0xff;

  UARTA_CONFIG(length, parity, stopbits);
}
#  endif

#  ifdef ENABLE_UART2
void uart2_config(uint16_t rate, uartlen_t length, uartpar_t parity, uartstop_t stopbits) {
  UBRRBH = rate >> 8;
  UBRRBL = rate & 0xff;

  UARTB_CONFIG(length, parity, stopbits);
}
#  endif
#endif


#if defined ENABLE_UART1 || defined ENABLE_UART2
void uart_init(void) {
  /* Seriellen Port konfigurieren */

  UART1_MODE_SETUP();

  UBRRAH = CALC_BPS(UART1_BAUDRATE) >> 8;
  UBRRAL = CALC_BPS(UART1_BAUDRATE) & 0xff;

  UCSRAB = _BV(RXENA) | _BV(TXENA);

  stdout = &mystdout;

  tx1_tail  = 0;
  tx1_head = 0;
#    if defined UART1_RX_BUFFER_SHIFT && UART1_RX_BUFFER_SHIFT > 0
  rx1_tail  = 0;
  rx1_head = 0;
#    endif

#  ifdef ENABLE_UART2
  UBRRBH = CALC_BPS(ENABLE_UART2_BAUDRATE) >> 8;
  UBRRBL = CALC_BPS(ENABLE_UART2_BAUDRATE) & 0xff;

  UCSRBB = _BV(RXCIEB) | _BV(RXENB) | _BV(TXENB);
  UCSRBC = _BV(UCSZB1) | _BV(UCSZB0);

  tx2_tail  = 0;
  tx2_head = 0;
#    if defined UART2_RX_BUFFER_SHIFT && UART2_RX_BUFFER_SHIFT > 0
  rx2_tail  = 0;
  rx2_head = 0;
#    endif
#  endif
}
#endif
