#include "e2prom.h"
#include "ax11000.h"
//  -------------
//  note:	
// 	1. every time use new chip, should initial e2prom using WINDOWS ISP 
//	2. make sure the I2C interrupt vector is turned on.
//  -------------


#if 1

I2C_BUF  Read_Data;
static U16_T    ADDROFDEV = 0x50;
static U16_T	delay_count = 1500;
/*
the following array is for AX11015 configure, about the detail please refer to the datatsheet (user_guide).
different product must change them according to its requiremnt.


I2C Configuration EEPROM Memory Map

EEPROM Offset Descriptor
0x00 		Length
0x01 		Flag
0x03~0x02 	Multi-function Pin Setting 1 Multi-function Pin Setting 0
0x04 		Programmable Output Driving Strength
0x05 Reserved = 0x00
0x0B~0x06 	Node ID 5 Node ID 4 Node ID 3 Node ID 2 Node ID 1 Node ID 0 (0x06)
0x0D~0x0C 	Maximum Packet Size 1 Maximum Packet Size 0 (0x0C)
0x0F~0x0E 	Secondary PHY Type and PHY ID Primary PHY Type and PHY ID (0x0E)
0x11~0x10 	Pause Frame Low Water Mark Pause Frame High Water Mark (0x10)
0x13~0x12 	Local Bus Setting 1 Local Bus Setting 0 (0x12)
0x15~0x14 	TOE TX VLAN Tag 1 TOE TX VLAN Tag 0 (0x14)
0x17~0x16 	TOE RX VLAN Tag 1 TOE RX VLAN Tag 0 (0x16)
0x18 		TOE ARP Cache Timeout
0x1C~0x19 	TOE Source IP Address ( 0x03 0x00 0xa8 0xc0)
0x20~0x1D   TOE Subnet Mask 3 TOE Subnet Mask 2 TOE Subnet Mask 1 TOE Subnet Mask 0	(0x1D)
0x21 		TOE L4 DMA Transfer Gap
0x2F~0x22 	Reserved for HW future use
0x7F~0x30 	Reserved for Software and Driver Settings
*/
const U8_T CPU_Config[40] = 
{
	0x21,0xfc,0x03,0x30,0x30,0x00,0x01,0x00,
	0x00,0x00,0x00,0x00,0xf2,0x05,0x10,0xe0,
	0x1d,0x19,0x87,0x00,0xff,0xff,0xff,0xff,
	0x10,0x03,0x00,0xa8,0xc0,0x00,0xff,0xff,
	0xff,0x04,0xff,0xff,0xff,0xff,0xff,0xff
};


void E2prom_Initial(void)
{
	U16_T 	i2cpreclk = 0;
	U8_T 	sysclk = 0;
	U32_T 	Sysclk;
	sysclk = AX11000_GetSysClk();
	switch (sysclk)
	{
		case SYS_CLK_100M :
			Sysclk = 100000000;
			i2cpreclk = I2C_STD_100M;
			break;
		case SYS_CLK_50M :
			Sysclk = 50000000;
			i2cpreclk = I2C_STD_50M;
			break;
		case SYS_CLK_25M :
			Sysclk = 25000000;
			i2cpreclk = I2C_STD_25M;
			break;
	}
	I2C_Setup(I2C_ENB|I2C_STANDARD|I2C_MST_IE|I2C_7BIT|I2C_MASTER_MODE, i2cpreclk, 0x005A);
//	E2prom_Erase();
}



U8_T E2prom_Read_Byte(U8_T addr,U8_T *value)
{
	I2C_RdmRead(ADDROFDEV, addr, &Read_Data, 1, I2C_STOP_COND);
	*value = Read_Data.I2cData[0];
	return 1;
}


U8_T E2prom_Read_Int(U8_T addr,U16_T *value)
{
//	U8_T temp,temp1;
//	E2prom_Read_Byte(addr,&temp);
//	E2prom_Read_Byte(addr + 1,&temp1);
//	*value = temp + temp1 * 256;
	I2C_RdmRead(ADDROFDEV, addr, &Read_Data, 2, I2C_STOP_COND);
	*value = Read_Data.I2cData[0];
			   
	return 1;
}


U8_T E2prom_Write_Byte(U8_T addr,U8_T dat)
{
	U8_T result;
	U16_T 	i;
	result = I2C_ByteWrite(ADDROFDEV, addr, dat, I2C_STOP_COND);
	for(i = 0; i < delay_count; i++) 			_nop_ ();
	return result;
}


U8_T E2prom_Write_Int(U8_T addr,U16_T dat)
{	
	U16_T 	i;
	U8_T temp;
	temp = (U8_T)dat;   // first low byte
	I2C_ByteWrite(ADDROFDEV, addr, temp, I2C_STOP_COND);	
	for(i = 0; i < delay_count; i++) 			_nop_ ();
	temp = dat >> 8;
	I2C_ByteWrite(ADDROFDEV, addr + 1, temp, I2C_STOP_COND);	
	for(i = 0; i < delay_count; i++) 			_nop_ ();
	return 1;
}

// erase e2prom
U8_T E2prom_Erase(void)
{
	U8_T result;
	U16_T j,i;
	for(j = 0; j < 208; j++) 
	{
		result = I2C_ByteWrite(ADDROFDEV, USER_BASE_ADDR + j, 0xFF, I2C_STOP_COND);
		if(!result) 	return result;
		for(i = 0; i < delay_count; i++) 	_nop_ ();
	}


	return result;
}


/* 
implement it when programming it or change it by hand
*/
void Eeprom_Write_Cpu_Config(void)
{
	U8_T loop;
	E2prom_Erase();
	for(loop = 0;loop < 40;loop++)
	{
		I2C_ByteWrite(ADDROFDEV, loop, CPU_Config[loop], I2C_STOP_COND);
	}
}
#endif













