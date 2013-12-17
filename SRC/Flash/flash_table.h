
#ifndef _FLASH_TABLE_H_

#define _FLASH_TABLE_H_
// Header file for Rx2 Flash Variables Library
// Version 1.02 (13-Sep-2000)
// (C) Embedded Systems Academy 2000

#include "types.h"

//extern uint32 data block_base, next_block_top, flash_ptr;

void flash_init(void);
uint8 flash_read_char(uint16 id, uint8 *value);
uint8 flash_read_int(uint16 id, uint16 *value);
uint8 flash_write_int(uint16 id, uint16 value);
uint8 iap_program_data_byte(uint8 val, uint16 addr);
void iap_erase_block(uint8 block);

#endif