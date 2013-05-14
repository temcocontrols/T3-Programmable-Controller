#include 	"types.h"
#include	"i2c.h"
#include	"i2capi.h"
#include    <intrins.h>
#include    <string.h>
#include	"interrupt.h"


#define ERASE_EVENT	1
#define WRITE_EVENT	2
#define READ_EVENT	3




#define  	USER_BASE_ADDR  0x30   // if use 24c02, the usable range is 0x30 - 0xff, 208 bytes


U8_T E2prom_Read_Byte(U8_T addr,U8_T *value);
U8_T E2prom_Read_Int(U8_T addr,U16_T *value);
U8_T E2prom_Write_Byte(U8_T addr,U8_T dat);
U8_T E2prom_Write_Int(U8_T addr,U16_T dat);
U8_T E2prom_Erase(void);
void Eeprom_Write_Cpu_Config(void);

