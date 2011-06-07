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

    uart.h: Definitions for the UART access routines

*/

#ifndef UART_H
#define UART_H

#define  CALC_BPS(x) ((int)((double)F_CPU / (16.0 * x ) - 1))

#if defined __AVR_ATmega162__ || defined __AVR_ATmega644__ || defined __AVR_ATmega644P__ || defined __AVR_ATmega1281__ || defined __AVR_ATmega2561__

#  ifdef SWAP_UART
#    define RXC   RXC1
#    define RXEN  RXEN1
#    define TXC   TXC1
#    define TXEN  TXEN1
#    define UBRRH UBRR1H
#    define UBRRL UBRR1L
#    define UCSRA UCSR1A
#    define UCSRB UCSR1B
#    define UCSRC UCSR1C
#    define UCSZ0 UCSZ10
#    define UCSZ1 UCSZ11
#    define UDR   UDR1
#    define UDRIE UDRIE1
#    define USART_UDRE_vect USART1_UDRE_vect
#  else
     /* Default is USART0 */
#    define RXC   RXC0
#    define RXEN  RXEN0
#    define TXC   TXC0
#    define TXEN  TXEN0
#    define UBRRH UBRR0H
#    define UBRRL UBRR0L
#    define UCSRA UCSR0A
#    define UCSRB UCSR0B
#    define UCSRC UCSR0C
#    define UCSZ0 UCSZ00
#    define UCSZ1 UCSZ01
#    define UDR   UDR0
#    define UDRIE UDRIE0
#    define USART_UDRE_vect USART0_UDRE_vect
#  endif

#elif defined __AVR_ATmega28__ || defined __AVR_ATmega48__ || defined __AVR_ATmega88__
     /* Default is USART0 */
#    define RXC   RXC0
#    define RXEN  RXEN0
#    define TXC   TXC0
#    define TXEN  TXEN0
#    define UBRRH UBRR0H
#    define UBRRL UBRR0L
#    define UCSRA UCSR0A
#    define UCSRB UCSR0B
#    define UCSRC UCSR0C
#    define UCSZ0 UCSZ00
#    define UCSZ1 UCSZ01
#    define UDR   UDR0
#    define UDRIE UDRIE0

#elif defined __AVR_ATmega16__ || defined __AVR_ATmega8__

#elif defined __AVR_ATmega128__
#    define UBRRH  UBRR0H
#    define UBRRL  UBRR0L
#    define UCSRA  UCSR0A
#    define UCSRB  UCSR0B
#    define UCSRC  UCSR0C
#    define UDR    UDR0
#    define USART_UDRE_vect USART0_UDRE_vect
#    define USART_RX_vect USART0_RX_vect
#    define U2X    U2X0

#else
#  error Unknown chip!
#endif

#if defined __AVR_ATmega8__ || __AVR_ATmega16__ || defined __AVR_ATmega32__
#  define UART1_MODE_SETUP()  do { UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0); } while(0)
#  else
#  define UART1_MODE_SETUP()  do { UCSRC = _BV(UCSZ1) | _BV(UCSZ0); } while(0)
#endif


#if defined ENABLE_UART1 || defined ENABLE_UART2
#include <avr/pgmspace.h>
void uart_init(void);
#else
#define uart_init()  do {} while(0)
#endif

#if defined ENABLE_UART1

uint8_t uart_getc(void);
void uart_putc(char c);
void uart_puthex(uint8_t num);
void uart_trace(void *ptr, uint16_t start, uint16_t len);
void uart_flush(void);
void uart_puts_P(prog_char *text);
void uart_putcrlf(void);

#include <stdio.h>
#define dprintf(str,...) printf_P(PSTR(str), ##__VA_ARGS__)

#else

#define uart_getc()    0
#define uart_putc(x)   do {} while(0)
#define uart_puthex(x) do {} while(0)
#define uart_flush()   do {} while(0)
#define uart_puts_P(x) do {} while(0)
#define uart_putcrlf() do {} while(0)
#define uart_trace(a,b,c) do {} while(0)

#endif

#ifdef ENABLE_UART2

uint8_t uart2_getc(void);
void uart2_putc(char c);
void uart2_puts(char* str);

#else

#define uart2_getc()    0
#define uart2_putc(x)   do {} while(0)
#define uart2_puts(x)   do {} while(0)

#endif

#if defined ENABLE_UART1 && defined DYNAMIC_BPS_RATE
void uart_set_bps(uint16_t bps);
#else
#define uart_set_bps(bps) do {} while(0)
#endif

#if defined ENABLE_UART2 && defined DYNAMIC_BPS_RATE
void uart2_set_bps(uint16_t bps);
#else
#define uart2_set_bps(bps) do {} while(0)
#endif

#endif
