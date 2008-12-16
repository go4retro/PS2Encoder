/* 

   eeprom.c: Persistent configuration storage

*/

#include <avr/eeprom.h>
#include <avr/io.h>
#include "config.h"
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
  uint16_t  baud_rate;
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
  baud_rate          = CALC_BPS(9600);         /* 9600 for starters */
  holdoff              = 0;
  
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

  baud_rate = eeprom_read_word(&epromconfig.baud_rate);
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
  eeprom_write_word(&epromconfig.baud_rate, baud_rate);
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
