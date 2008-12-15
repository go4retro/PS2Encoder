/* sd2iec - SD/MMC to Commodore serial bus interface/controller
   Copyright (C) 2007,2008  Ingo Korb <ingo@akana.de>

   Inspiration and low-level SD/MMC access based on code from MMC2IEC
     by Lars Pontoppidan et al., see sdcard.c|h and config.h.

   FAT filesystem access based on code from ChaN and Jim Brain, see ff.c|h.

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


   avrcompat.h: Compatibility defines for multiple target chips
*/

#ifndef AVRCOMPAT_H
#define AVRCOMPAT_H

#if defined __AVR_ATmega8__ 
#  define SIG_OUTPUT_COMPARE SIG_OUTPUT_COMPARE2
#  define OCR OCR2
#  define TCNT  TCNT2
#  define TCCR  TCCR2
#  define TCCR_DATA_DELAY (1<<CS22) | (1<<CS21) | (1<<CS20) | (1<<WGM21)
#  define TCCR_DATA (1<<CS21) | (1<<WGM21)
#  define TIFR_DATA (1<<OCF2)
#  define TIMSK_DATA (1<<OCIE2)
#elif defined __AVR_ATmega28__ || defined __AVR_ATmega48__ || defined __AVR_ATmega88__
#  define SIG_OUTPUT_COMPARE SIG_OUTPUT_COMPARE2
#  define OCR OCR2
#  define TCNT  TCNT2
#  define TCCR  TCCR2
#  define TCCR_DATA_DELAY (1<<CS22) | (1<<CS21) | (1<<CS20) | (1<<WGM21)
#  define TCCR_DATA (1<<CS21) | (1<<WGM21)
#  define TIFR_DATA (1<<OCF2)
#  define TIMSK_DATA (1<<OCIE2)
#elif defined __AVR_ATmega16__ || defined __AVR_ATmega32__ || defined __VAR_ATmega162
#  define SIG_OUTPUT_COMPARE SIG_OUTPUT_COMPARE0
#  define OCR OCR0
#  define TCNT  TCNT0
#  define TCCR  TCCR0
#  define TCCR_DATA_DELAY (1<<CS02) | (1<<CS00) | (1<<WGM01)
#  define TCCR_DATA (1<<CS01) | (1<<WGM01)
#  define TIFR_DATA (1<<OCF0)
#  define TIMSK_DATA (1<<OCIE0)
#endif


/* USART */

#if defined __AVR_ATmega644__ || defined __AVR_ATmega644P__ || defined __AVR_ATmega1281__ || defined __AVR_ATmega2561__

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

#elif defined __AVR_ATmega32__
#  define TIMER2_COMPA_vect TIMER2_COMP_vect
#  define TCCR0B TCCR0
#  define TCCR2A TCCR2
#  define TCCR2B TCCR2
#  define TIFR0  TIFR
#  define TIMSK2 TIMSK
#  define OCIE2A OCIE2
#  define OCR2A  OCR2

#elif defined __AVR_ATmega8__ || defined __AVR_ATmega28__ || defined __AVR_ATmega48__ || defined __AVR_ATmega88__

#elif defined __AVR_ATmega128__
#  define UBRRH  UBRR0H
#  define UBRRL  UBRR0L
#  define UCSRA  UCSR0A
#  define UCSRB  UCSR0B
#  define UCSRC  UCSR0C
#  define UDR    UDR0
#  define USART_UDRE_vect USART0_UDRE_vect
#  define USART_RXC_vect USART0_RX_vect
#  define TIMER2_COMPA_vect TIMER2_COMP_vect
#  define TCCR0B TCCR0
#  define TCCR2A TCCR2
#  define TCCR2B TCCR2
#  define TIFR0  TIFR
#  define TIMSK1 TIMSK
#  define TIMSK2 TIMSK
#  define OCIE2A OCIE2
#  define OCR2A  OCR2
#  define U2X    U2X0 

#else
#  error Unknown chip!
#endif

/* SPI */
#if defined __AVR_ATmega32__ || defined __AVR_ATmega644__ || defined __AVR_ATmega644P__

#  define SPI_PORT PORTB
#  define SPI_DDR  DDRB
#  define SPI_SS   _BV(PB4)
#  define SPI_MOSI _BV(PB5)
#  define SPI_MISO _BV(PB6)
#  define SPI_SCK  _BV(PB7)

#elif defined __AVR_ATmega128__ || defined __AVR_ATmega1281__ || defined __AVR_ATmega2561__

#  define SPI_PORT PORTB
#  define SPI_DDR  DDRB
#  define SPI_SS   _BV(PB0)
#  define SPI_SCK  _BV(PB1)
#  define SPI_MOSI _BV(PB2)
#  define SPI_MISO _BV(PB3)

#elif defined __AVR_ATmega8__ || defined __AVR_ATmega28__ || defined __AVR_ATmega48__ || defined __AVR_ATmega88__

#  define SPI_PORT PORTB
#  define SPI_DDR  DDRB
#  define SPI_SS   _BV(PB2)
#  define SPI_SCK  _BV(PB5)
#  define SPI_MOSI _BV(PB3)
#  define SPI_MISO _BV(PB4)

#else
#  error Unknown chip!
#endif

#define SPI_MASK (SPI_SS|SPI_MOSI|SPI_MISO|SPI_SCK)

#endif /* AVRCOMPAT_H */
