
#if 1

#include 	"clock.h"
#include	"define.h"


//U8_T  month_length[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

U8_T data sec,min,hour,day,week,mon,year;

//Date_block	ora_current;

extern I2C_BUF  Read_Data;


U8_T BcdToHex(U8_T byte)  
{
	U8_T i,j;                 
    i = byte >> 4;
	j = byte & 0x0f;     
	return i*10+j;
}


U8_T HexToBcd(U8_T byte)
{
 	U8_T i,j;
	i = byte / 10;
	j = byte % 10;
 	return  (i << 4) + j ;
}




U8_T Set_Clock(U8_T addr,U8_T dat)
{
	U8_T far result;
	U16_T 	far i;
	result = I2C_ByteWrite(0x51, addr, HexToBcd(dat), I2C_STOP_COND);
	for(i = 0; i < 1500; i++) 			_nop_ ();
	return result;
}



U8_T Get_Clock(U8_T addr,U8_T *value)
{
	U8_T temp = 0;
	I2C_RdmRead(0x51, addr, &Read_Data, 1, I2C_STOP_COND);
	temp = Read_Data.I2cData[0];
	if(addr == PCF_SEC)    	temp = temp & 0x7f;
	else if(addr == PCF_MIN)		temp = temp & 0x7f;
	else if(addr == PCF_HOUR)	temp = temp & 0x3f;
	else if(addr == PCF_DAY)		temp = temp & 0x3f;
	else if(addr == PCF_WEEK)	temp = temp & 0x07;
	else if(addr == PCF_MON)		temp = temp & 0x1f;

	*value = BcdToHex(temp);
	return 1;
}




void Initial_Clock(void)
{
	Set_Clock(0,0);
	Set_Clock(1,0);
	Set_Clock(PCF_SEC,0);
	Set_Clock(PCF_MIN,0);
	Set_Clock(PCF_HOUR,0);
	Set_Clock(PCF_DAY,1);
	Set_Clock(PCF_WEEK,0);
	Set_Clock(PCF_MON,1);
	Set_Clock(PCF_YEAR,7);

/*	 Tim.Clk.sec = 0;
	 Tim.Clk.min = 0;
	 Tim.Clk.hour = 0;
	 Tim.Clk.day = 1;
	 Tim.Clk.week = 0;
	 Tim.Clk.mon = 1;
	 Tim.Clk.year = 7;
*/

}


//  0.5s 
void Updata_Clock(void)
{
	U8_T tempday,i;
	Get_Clock(PCF_SEC,&sec);
	Get_Clock(PCF_MIN,&min);
	Get_Clock(PCF_HOUR,&hour);
	Get_Clock(PCF_DAY,&day);
	Get_Clock(PCF_WEEK,&week);
	Get_Clock(PCF_MON,&mon);
	Get_Clock(PCF_YEAR,&year);

	RTC.Clk.sec = sec;
	RTC.Clk.min = min;
	RTC.Clk.hour = hour;
	RTC.Clk.day = day;
	RTC.Clk.week = week;
	RTC.Clk.mon = mon;
	RTC.Clk.year = year;

	

	
}


#endif
