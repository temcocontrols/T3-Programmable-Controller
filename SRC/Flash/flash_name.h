#ifndef __FLASH_NAME_H__
#define __FLASH_NAME_H__

/* 100 - 111 storing scheduel */
/* 112  storing names */

/* 119 - 127 ISP */

#define P_NAME				112

#define NAME_SIZE 	14  // 14 * 36 = 504
#define NAME_NUM	36  // 10 OUTPUT 26 INPUT


void Flash_Write_Name(unsigned char line,char* string);
unsigned char* Flash_Read_Name(unsigned char line);



#endif