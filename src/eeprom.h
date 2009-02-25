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


   eeprom.h: Definitions for EEPROM config code

*/


#ifndef EEPROM_H
#define EEPROM_H

void eeprom_read_config(void);
void eeprom_write_config(void);

#endif
