/***********************************

this file is used for storing names of inputs and output

***********************************/

#include "goal_flash.h"
#include "flash_schedule.h"
#include "flash_name.h"


void Flash_Write_Name(unsigned char line,char* string)
{
	unsigned char xdata i;
	unsigned int xdata addr;


	addr = NAME_SIZE * line;


	Flash_Buffer_Read_Mass(P_NAME);

	for(i = 0;i < NAME_SIZE;i++)
	{
		Flash_Write_Code_Buffer_Value(addr + i,string[i]);
	}

	Flash_Buffer_Write_Mass(P_NAME);
}


unsigned char* Flash_Read_Name(unsigned char line)
{
	unsigned int i;
	unsigned int xdata addr;

	addr = NAME_SIZE * line;

	for(i = 0;i < NAME_SIZE;i++)
	{
		w_string[i] = Flash_Read(P_NAME,addr++);
			
	}
	return w_string;
	
}
	







