#ifndef E2PROM_H
#define E2PROM_H

#include "types.h"

#define USER_BASE_ADDR 0

void E2prom_Initial(void);
U8_T E2prom_Read_Byte(U16_T addr,U8_T *value);
U8_T E2prom_Read_Int(U16_T addr,U16_T *value);
U8_T E2prom_Write_Byte(U16_T addr,U16_T dat);



#endif

