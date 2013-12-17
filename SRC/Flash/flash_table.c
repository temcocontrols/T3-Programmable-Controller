#include "flash_table.h"
#include "flash.h"
#include "types.h"

#define FLASH_PROG_ERROR			1
#define FLASH_TABLE_FULL			2

#define FLASH_BLOCK_NOT_USE			0xFF
#define FLASH_BLOCK_IN_USE 			0x77
#define FLASH_BLOCK_FULL			0x33                 // can't add new ID
#define FLASH_BLOCK_INVALID			0x00
#define FLASH_BLOCK_TOBE_REFRESH	0x00

#define BLOCK1	0
#define BLOCK2	1

#define	BLOCK_SIZE_CONST	65536
#define	ELEMENT_SIZE_CONST	2
uint32 const xdata START_BLOCK_CONST[2] =
{
	0x60000,
	0x70000,
};


static uint8 flash_set_block(void);
static uint8 flash_garbage_collector(void);
static void flash_find_next_entry(uint32 *scan_ptr);
static uint8 flash_write_no_recovery(uint16 id, uint8 *value);

uint8 flash_unused_block;
uint32 data flash_ptr, block_base, next_block_top;


static void delay_us(uint16 time)
{
	while(time--);
}

static void flash_find_next_entry(uint32 *scan_ptr)
{
	uint8 size;
	IntFlashReadByte(*scan_ptr, &size);
	*scan_ptr += size + 3;
}

static uint8 flash_table_full(void)
{
	if(flash_ptr >= (next_block_top - (ELEMENT_SIZE_CONST + 3)))
		return FLASH_TABLE_FULL;

	return 0;
}

uint8 flash_write_int(uint16 id, uint16 value)
{
	uint8 j = 0;
	uint8 transfer_value[ELEMENT_SIZE_CONST];	// this will always be size 2 given we always enter integer values

	// put value into array or char, just the way Robert wrote the code
	transfer_value[0] = value & 0x00FF;		// enter lower bits
	transfer_value[1] = (value >> 8) & 0x00FF;	// enter upper bits

	// Make sure flash table is not full
	if(flash_table_full())
	{
		// If the table was full run the garbage collector routine.
		while(j < 10 && flash_garbage_collector())
			j++;		// if garbage collector failed once, try it out again

		delay_us(5);

		if(j >= 10)	// if garbage collector too many times, we are in trouble
			return 0;
	}

	// Write the register to the table.
	while(j < 10 && !flash_write_no_recovery(id, transfer_value))
		j++;

	if(j >= 10)
		return 0;

	return 1;
}

uint8 flash_read_int(uint16 id, uint16 *value)
{
	uint32 scan_ptr;
//	uint32 ptr;
	uint8 size_temp;
	uint16 id_temp;
	
	scan_ptr = block_base;
//	ptr = 0;

	// Increment id by one to avoid trying to read register address 0, which is reserved.	
	id++;
	while(scan_ptr < next_block_top)
	{
		IntFlashReadByte(scan_ptr, &size_temp);
		if(size_temp == 0xff)
			break;

		IntFlashReadInt(scan_ptr + 1, &id_temp);
		if(id_temp == id)
		{
//			if(ptr)
//				IntFlashWriteInt(ptr + 1, 0x0000);
//
//			ptr = scan_ptr;
			IntFlashReadInt(scan_ptr + 3, value);
			return 1;
		}
		
		flash_find_next_entry(&scan_ptr);			
	}

	// if an entry found then return pointer to value
//	if(ptr) 
//	{
//		IntFlashReadInt(ptr + 3, value);
//		return 1;	// return a true to indicate value was found
//	}

	return 0;
}

uint8 flash_read_char(uint16 id, uint8 *value)
{
	uint8 size_temp;
	uint16 id_temp;
	uint32 scan_ptr;
//	uint32 ptr;

	scan_ptr = block_base;
//	ptr = 0;

	// Increment id by one to avoid trying to read register address 0, which is reserved.	
	id++;
	while(scan_ptr < next_block_top)
	{
		IntFlashReadByte(scan_ptr, &size_temp);
		if(size_temp == 0xff)
			break;

		IntFlashReadInt(scan_ptr + 1, &id_temp);
		if(id_temp == id)
		{
//			if(ptr)
//				IntFlashWriteInt(ptr + 1, 0x0000);
//
//			ptr = scan_ptr;
			IntFlashReadByte(scan_ptr + 3, value);
			return 1;
		}
		
		flash_find_next_entry(&scan_ptr);			
	}
	
	// if an entry found then return pointer to value
//	if(ptr) 
//	{
//		IntFlashReadByte(ptr + 3, value);
//		return 1;	// return a true to indicate value was found
//	}

	return 0;
}

static uint8 flash_write_no_recovery(uint16 id, uint8 *value)
{
	uint32 scan_ptr;
	uint32 old_entry_id = 0;
	uint8 size_temp;
	uint16 id_temp;

	scan_ptr = block_base;

	// Increment id by one to avoid trying to write register address 0, which is reserved.	 
	id++;  

	// check to find old value if stored
	// (continue scanning until you find an ID match or until you reach the first unused entry
	while(1)
	{
		IntFlashReadByte(scan_ptr, &size_temp);
		IntFlashReadInt(scan_ptr + 1, &id_temp);
		if((size_temp == 0xff) || (id_temp == id))
			break;

		flash_find_next_entry(&scan_ptr);
	}
		
	if(id_temp == id)  // found old entry so record location of id
		old_entry_id = scan_ptr + 1;
  	
	// store size
	IntFlashWriteByte(flash_ptr, ELEMENT_SIZE_CONST);

	flash_ptr += 3;  // increase flash_ptr by size + 3

 	// store data
	for(size_temp = 0; size_temp < ELEMENT_SIZE_CONST; size_temp++)
		IntFlashWriteByte(flash_ptr + size_temp, *(value + size_temp));

	// adjust pointer to point to next empty register
 	flash_ptr += ELEMENT_SIZE_CONST;
	
	// store id
	IntFlashWriteInt(flash_ptr - (ELEMENT_SIZE_CONST + 2), id);
	
	// if there is an old entry then mark it is as unused now new entry has been written
	if(old_entry_id)
		IntFlashWriteInt(old_entry_id, 0x0000);

	return 1;
}

static uint8 flash_garbage_collector(void)
{
	uint32 scan_ptr;
	uint32 code_ptr;
	uint32 temp_ptr;
	uint8 i, size, value;
	uint16 id;

	scan_ptr = block_base;
	code_ptr = START_BLOCK_CONST[flash_unused_block];

	// erase unused block
	IntFlashErase(ERA_RUN, START_BLOCK_CONST[flash_unused_block]);

	temp_ptr = scan_ptr + BLOCK_SIZE_CONST - 1;
	code_ptr++;

	// scan through entries in table until blank memory reached
	// debug, for some reason the first one always gets deleted
	while(scan_ptr < temp_ptr)
	{
		IntFlashReadByte(scan_ptr, &size);
		if(size == 0xff)
			break;

		IntFlashReadInt(scan_ptr + 1, &id);
		if((id != 0x0000) && (id != 0xffff))
		{
			IntFlashWriteByte(code_ptr, size);

			// copy id
			code_ptr++;
			scan_ptr++;
			IntFlashWriteInt(code_ptr, id);

			// copy value
			code_ptr += 2;
			scan_ptr += 2;
			for(i = 0; i < BLOCK_SIZE_CONST; i++)
			{
				IntFlashReadByte(scan_ptr++, &value);
				IntFlashWriteByte(code_ptr++, value);
			}
		}
		else
		{
			flash_find_next_entry(&scan_ptr);
		}
	}

	// record the top of the compressed table
	flash_ptr = code_ptr;
	
	// swap values,  unused_block <--> block_base
	block_base = START_BLOCK_CONST[flash_unused_block];
	flash_unused_block = flash_unused_block^0x01;

	next_block_top = block_base + BLOCK_SIZE_CONST;     

	// mark new block as valid
	IntFlashWriteByte(block_base, FLASH_BLOCK_IN_USE);
	
	// mark old block as invalid
	IntFlashWriteByte(START_BLOCK_CONST[flash_unused_block], FLASH_BLOCK_INVALID);
	
	// actual base starts after block status byte
	block_base++;

	return 1;
}

void flash_init(void)
{
	flash_set_block();
}

static uint8 flash_set_block(void)
{
	uint16 block_size1, block_size2;
	uint8 block1_used_flag, block2_used_flag;
	uint8 temp;

	IntFlashReadByte(START_BLOCK_CONST[BLOCK1], &block1_used_flag);
	IntFlashReadByte(START_BLOCK_CONST[BLOCK2], &block2_used_flag);
	if((block1_used_flag == FLASH_BLOCK_IN_USE) && (block2_used_flag == FLASH_BLOCK_IN_USE))
	{
		// find size of table in block 1
		flash_ptr = START_BLOCK_CONST[BLOCK1] + 1;
		while(flash_ptr < START_BLOCK_CONST[BLOCK2])
		{
			IntFlashReadByte(flash_ptr, &temp);
			if(temp == 0xff) 
				break;
	
			flash_find_next_entry(&flash_ptr);
		}
		// record size
		block_size1 = flash_ptr - START_BLOCK_CONST[BLOCK1];
		
		// find size of table in block 2
		flash_ptr = START_BLOCK_CONST[BLOCK2] + 1;
		while(flash_ptr < (START_BLOCK_CONST[BLOCK2] + BLOCK_SIZE_CONST))
		{
			IntFlashReadByte(flash_ptr, &temp);
			if(temp == 0xff) 
				break;
	
			flash_find_next_entry(&flash_ptr);
		}
		// record size
		block_size2 = flash_ptr - START_BLOCK_CONST[BLOCK2];

		if(block_size2 <= block_size1)
		{      
			// store block 2 base skipping block status byte
			block_base = START_BLOCK_CONST[BLOCK2] + 1;
			// store block 1 as being unused
			flash_unused_block = BLOCK1;
			// mark block 1 as invalid
			IntFlashWriteByte(START_BLOCK_CONST[BLOCK1], FLASH_BLOCK_INVALID);
  		}
		else
		{
			// store block 1 base skipping block status byte
			block_base = START_BLOCK_CONST[BLOCK1] + 1;
			// store block 2 as being unused
			flash_unused_block = BLOCK2;
			// mark block 2 as invalid
			IntFlashWriteByte(START_BLOCK_CONST[BLOCK2], FLASH_BLOCK_INVALID);
		}  
	}
	// if block 1 is only block in use
	else if(block1_used_flag == FLASH_BLOCK_IN_USE)
	{
		// store block 1 base skipping block status byte
		block_base = START_BLOCK_CONST[BLOCK1] + 1;
		// store block 2 as being unused
		flash_unused_block = BLOCK2;
	}
	// if block 2 is only block in use
	else if(block2_used_flag == FLASH_BLOCK_IN_USE)
	{
		// store block 2 base skipping block status byte
		block_base = START_BLOCK_CONST[BLOCK2] + 1;
		// store block 2 as being unused
		flash_unused_block = BLOCK1;
	}
	else
	{
		// store block 1 base skipping block status byte
		block_base = START_BLOCK_CONST[BLOCK1] + 1;
		// store block 2 as being unused
		flash_unused_block = BLOCK2;
		// mark block 1 as in use
		IntFlashWriteByte(START_BLOCK_CONST[BLOCK1], FLASH_BLOCK_IN_USE);
	}
	
	// initialize flash_ptr
	flash_ptr = block_base;

	// calculate top address of next block
	next_block_top = block_base + BLOCK_SIZE_CONST;     

	// set flash_ptr to top of table
	while(1)
	{
		IntFlashReadByte(flash_ptr, &temp);
		if(temp == 0xff)
			break;

		flash_find_next_entry(&flash_ptr);
	}
	
	return 1;
}