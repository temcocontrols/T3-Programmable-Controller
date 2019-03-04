#ifndef	_CLOCK_H
#define	_CLOCK_H

#include 	"types.h"
#include	"i2c.h"
#include	"i2capi.h"
#include    <intrins.h>
#include    <string.h>
#include	"interrupt.h"


typedef struct
{
	U8_T second;				/* 0-59	*/
	U8_T minute;    		/* 0-59	*/
	U8_T hour;      		/* 0-23	*/
	U8_T day;       		/* 1-31	*/
	U8_T month;     		/* 0-11	*/
	U8_T year;      		/* 0-99	*/
	U8_T dayofweek;  		/* 0-6, 0=Sunday	*/
	U16_T day_of_year; 	/* 0-365	*/
	S8_T is_dst;        /* daylight saving time on / off */
}	Date_block;  /*	1+1+1+1+1+1+1+2 = 9	*/


//extern Date_block	ora_current;
extern U8_T far 	month_length[12];
extern U32_T  	ora_current_sec;
extern U8_T far table_week[12];

U8_T Set_Clock(U8_T addr,U8_T dat);
U8_T Get_Clock(U8_T addr,U8_T *value);
void Initial_Clock(void);
void Updata_Clock(U8_T type);
void get_time_text(void);

U8_T Rtc_Set(U16_T syear, U8_T smon, U8_T sday, U8_T hour, U8_T min, U8_T sec, U8_T type);

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
