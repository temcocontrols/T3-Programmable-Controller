#ifndef	_CLOCK_H
#define	_CLOCK_H

#include 	"types.h"
//#include	"i2c.h"
//#include	"i2capi.h"
//#include    <intrins.h>
#include    <string.h>
//#include	"interrupt.h"



//extern Date_block	ora_current;
extern U8_T far 	month_length[12];
//extern U32_T  	ora_current_sec;


U8_T Set_Clock(U8_T addr,U8_T dat);
U8_T Get_Clock(U8_T addr,U8_T *value);
void Initial_Clock(void);
void Updata_Clock(U8_T type);
void get_time_text(void);


#define  PCF_SEC  0x02
#define  PCF_MIN  0x03
#define  PCF_HOUR  0x04
#define  PCF_DAY  0x05
#define  PCF_WEEK  0x06
#define  PCF_MON  0x07
#define  PCF_YEAR  0x08

#define  PCF_MIN_ALARM  0x09
#define  PCF_HOUR_ALARM  0x0A
#define  PCF_DAY_ALARM  0x0B
#define  PCF_WEEK_ALARM  0x0C


#endif
