/*
    PS2Encoder - PS2 Keyboard to serial/parallel converter
    Copyright Jim Brain and RETRO Innovations, 2008-2012

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

    eeprom.c: Persistent configuration storage

*/

#include "config.h"
#include <avr/eeprom.h>
#include <avr/io.h>
#include "eeprom.h"
#include "flags.h"
#include "uart.h"

/**
 * struct storedconfig - in-eeprom data structure
 * @pad      : EEPROM position 0 is unused
 * @checksum   : Checksum over the EEPROM contents
 * @structsize : size of the eeprom structure
 * @osccal     : stored value of OSCCAL
 * @globalopts: subset of the globalopts variable
 *
 * This is the data structure for the contents of the EEPROM.
 */
static EEMEM struct {
  uint8_t   pad;
  uint8_t   checksum;
  uint16_t  structsize;
  uint8_t   osccal;
  uint8_t   globalopts;
  uint16_t  uart_bps;
  uint8_t   uart_length;
  uint8_t   uart_parity;
  uint8_t   uart_stop;
  uint8_t   holdoff;
} epromconfig;

/**
 * read_configuration - reads configuration from EEPROM
 *
 * This function reads the stored configuration values from the EEPROM.
 * If the stored checksum doesn't match the calculated one nothing will
 * be changed.
 */
void eeprom_read_config(void) {
  uint16_t i, size;
  uint8_t checksum, tmp;

  /* Set default values */
  globalopts         |= OPT_CRLF;              /* CRLF enabled */
  globalopts         |= OPT_BACKSPACE;         /* Use BS for Backspace */
  uart_bps           = CALC_BPS(9600);         /* 9600 for starters */
  uart_length        = LENGTH_8;
  uart_parity        = PARITY_NONE;
  uart_stop          = STOP_1;
  holdoff            = 0;

  size = eeprom_read_word(&epromconfig.structsize);

  /* Calculate checksum of EEPROM contents */
  checksum = 0;
  for (i=2; i<size; i++)
    checksum += eeprom_read_byte((uint8_t *)i);

  /* Abort if the checksum doesn't match */
  if (checksum != eeprom_read_byte(&epromconfig.checksum)) {
    EEAR = 0;
    return;
  }

  /* Read data from EEPROM */
  OSCCAL = eeprom_read_byte(&epromconfig.osccal);

  tmp = eeprom_read_byte(&epromconfig.globalopts);
  globalopts &= (uint8_t)~(OPT_CRLF | OPT_BACKSPACE |
                            OPT_STROBE_LO);
  globalopts |= tmp;

  uart_bps    = eeprom_read_word(&epromconfig.uart_bps);
  uart_length = (uartlen_t)eeprom_read_byte(&epromconfig.uart_length);
  uart_parity = (uartpar_t)eeprom_read_byte(&epromconfig.uart_parity);
  uart_stop   = (uartstop_t)eeprom_read_byte(&epromconfig.uart_stop);

  holdoff = eeprom_read_byte(&epromconfig.holdoff);

  /* Paranoia: Set EEPROM address register to the dummy entry */
  EEAR = 0;
}

/**
 * write_configuration - stores configuration data to EEPROM
 *
 * This function stores the current configuration values to the EEPROM.
 */
void eeprom_write_config(void) {
  uint16_t i;
  uint8_t checksum;

  /* Write configuration to EEPROM */
  eeprom_write_word(&epromconfig.structsize, sizeof(epromconfig));
  eeprom_write_byte(&epromconfig.osccal, OSCCAL);
  eeprom_write_byte(&epromconfig.globalopts,
                    globalopts & (OPT_CRLF | OPT_BACKSPACE |
                                   OPT_STROBE_LO));
  eeprom_write_word(&epromconfig.uart_bps, uart_bps);
  eeprom_write_byte(&epromconfig.uart_length, uart_length);
  eeprom_write_byte(&epromconfig.uart_parity, uart_parity);
  eeprom_write_byte(&epromconfig.uart_stop, uart_stop);
  eeprom_write_byte(&epromconfig.holdoff, holdoff);

  /* Calculate checksum over EEPROM contents */
  checksum = 0;
  for (i=2;i<sizeof(epromconfig);i++)
    checksum += eeprom_read_byte((uint8_t *) i);

  /* Store checksum to EEPROM */
  eeprom_write_byte(&epromconfig.checksum, checksum);

  /* Paranoia: Set EEPROM address register to the dummy entry */
  EEAR = 0;
}
