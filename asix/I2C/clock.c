
#if 1

#include 	"clock.h"



//U8_T  month_length[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

U8_T data sec,min,hour,day,week,mon,year;

//Date_block	ora_current;

I2C_BUF far Clock_Data;
//UN_Time Rtc;

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


char far time[20];
void get_time_text(void)
{
	time[0] = '2';
	time[1] = '0';
	time[2] =  Rtc.Clk.year / 10 + '0';
	time[3] =  Rtc.Clk.year % 10 + '0';
	time[4] = '-';
	time[5] =  Rtc.Clk.mon / 10 + '0';
	time[6] =  Rtc.Clk.mon % 10 + '0';
	time[7] = '-';
	time[8] =  Rtc.Clk.day / 10 + '0';
	time[9] =  Rtc.Clk.day % 10 + '0';
	time[10] = ' ';
	time[11] =  Rtc.Clk.hour / 10 + '0';
	time[12] =  Rtc.Clk.hour % 10 + '0';
	time[13] = ':';
	time[14] =  Rtc.Clk.min / 10 + '0';
	time[15] =  Rtc.Clk.min % 10 + '0';
	time[16] = ':';
	time[17] =  Rtc.Clk.sec / 10 + '0';
	time[18] =  Rtc.Clk.sec % 10 + '0';

}

U8_T Set_Clock(U8_T addr,U8_T dat)
{
	U16_T 	far i;
	if(I2C_ByteWrite(0x51, addr, HexToBcd(dat), I2C_STOP_COND) == TRUE)
	{
	for(i = 0; i < 1500; i++) 			_nop_ ();
	return 1;
	}
	else
	{
		I2C_Setup(I2C_ENB|I2C_STANDARD|I2C_MST_IE|I2C_7BIT|I2C_MASTER_MODE, 0xc7, 0x005A);
		return 0;
	}
}



U8_T Get_Clock(U8_T addr,U8_T *value)
{
	U8_T temp = 0;
	if(I2C_RdmRead(0x51, addr, &Clock_Data, 1, I2C_STOP_COND) == TRUE)
	{
	temp = Clock_Data.I2cData[0];
	if(addr == PCF_SEC)    	temp = temp & 0x7f;
	else if(addr == PCF_MIN)		temp = temp & 0x7f;
	else if(addr == PCF_HOUR)	temp = temp & 0x3f;
	else if(addr == PCF_DAY)		temp = temp & 0x3f;
	else if(addr == PCF_WEEK)	temp = temp & 0x07;
	else if(addr == PCF_MON)		temp = temp & 0x1f;
	else if(addr == PCF_YEAR)		temp = temp;
	
	*value = BcdToHex(temp);
	return 1;
	}
	else
	{
		I2C_Setup(I2C_ENB|I2C_STANDARD|I2C_MST_IE|I2C_7BIT|I2C_MASTER_MODE, 0xc7, 0x005A);
		return 0;
	}
}

U8_T Rtc_Set(U16_T syear, U8_T smon, U8_T sday, U8_T hour, U8_T min, U8_T sec, U8_T type)
{  // type is no used for ASIX
	Set_Clock(0,0);
	Set_Clock(1,0);
	Set_Clock(PCF_SEC,sec);
	Set_Clock(PCF_MIN,min);
	Set_Clock(PCF_HOUR,hour);
	Set_Clock(PCF_DAY,sday);
	Set_Clock(PCF_MON,smon);
	Set_Clock(PCF_YEAR,LOW_BYTE(syear));

	return 1;
}

//U8_T Rtc_Get(void)
//{



//}

extern BACNET_DATE Local_Date;
extern BACNET_TIME Local_Time;

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

U32_T protime;



u8 RTC_Get_Week(u16 year, u8 month, u8 day)
{	
	u16 far temp2;
	u8 far yearH, yearL;
	u8 far table_week[12] = {0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5};	//月修正数据表	  

	yearH = year / 100;
	yearL = year % 100;
	  
	if(yearH > 19)	// 如果为21世纪,年份数加100
		yearL += 100;
	
	// 所过闰年数只算1900年之后的  
	temp2 = yearL + yearL / 4;
	temp2 = temp2 % 7; 
//	if(month == 1 || month == 10) table_week = 0;
//	else if(month == 2 || month == 3 || month == 11 ) table_week = 3;
//	else if(month == 4 || month == 7) table_week = 6;
//	else if(month == 5) table_week = 1;
//	else if(month == 6) table_week = 4;
//	else if(month == 8) table_week = 2;
//	else if(month == 9 || month == 12) table_week = 5;
	
	temp2 = temp2 + day + table_week[month - 1];
	
	if(yearL % 4 == 0 && month < 3)
		temp2--;
	
	return(temp2 % 7);
}	

//  0.5s 
void Updata_Clock(U8_T type)
{
//	U8_T tempday,i;
	if(type == 0)
	{
		Get_Clock(PCF_SEC,&sec);
		Get_Clock(PCF_MIN,&min);
		Get_Clock(PCF_HOUR,&hour);
		Get_Clock(PCF_DAY,&day);
		Get_Clock(PCF_WEEK,&week);
		Get_Clock(PCF_MON,&mon);
		Get_Clock(PCF_YEAR,&year);
		
		if((sec == 0) && (min == 0) && (hour == 0) \
		&& (day == 0) && (week == 0) && (mon == 0))
		{
			// I2C ERROR
		}
		else
		{
			Rtc.Clk.sec = sec;
			Rtc.Clk.min = min;
			Rtc.Clk.hour = hour;
			Rtc.Clk.day = day;
			Rtc.Clk.week = week;
			Rtc.Clk.mon = mon;
			Rtc.Clk.year = year;

		}
		update_timers();
	}	
	else
	{
		int year;
		year = Rtc.Clk.year;
		if( ( year & '\x03' ) == '\x0' )
			month_length[1] = 29;
		else
			month_length[1] = 28;
		
		if(Rtc.Clk.sec < 59)	Rtc.Clk.sec++;
		else
		{
			Rtc.Clk.sec = 0;
			if(Rtc.Clk.min < 59)
				Rtc.Clk.min ++;
			else
			{
				Rtc.Clk.min = 0;
				if(Rtc.Clk.hour < 24)
					Rtc.Clk.hour++;
				else
				{
					Rtc.Clk.hour = 0;
					if(Rtc.Clk.mon <= 12 && Rtc.Clk.mon >= 1)
					{
						if(Rtc.Clk.day < month_length[Rtc.Clk.mon - 1])
						{
							Rtc.Clk.day++;
						}
						else
						{
							Rtc.Clk.day = 1;
							if(Rtc.Clk.mon < 12)
								Rtc.Clk.mon++;
							else
							{
								Rtc.Clk.mon = 0;
								Rtc.Clk.year++;
							}						
						}
					}
					
					 
//					if(Rtc.Clk.week < 6) Rtc.Clk.week++;
//					else
//						Rtc.Clk.week = 0;
				}
			}
		}
		Rtc.Clk.week = RTC_Get_Week(2000 + Rtc.Clk.year, Rtc.Clk.mon, Rtc.Clk.day);	//获取星期   
	}
		
//	protime = Rtc.Clk.hour * 100000L + (60000L * Rtc.Clk.min + 1000L * Rtc.Clk.sec) / 36L;
	
	Local_Date.year = Rtc.Clk.year + 2000;
	Local_Date.month = Rtc.Clk.mon;
	Local_Date.day = Rtc.Clk.day;
	Local_Date.wday = Rtc.Clk.week;
	
	Local_Time.hour = Rtc.Clk.hour;
	Local_Time.min = Rtc.Clk.min;
	Local_Time.sec = Rtc.Clk.sec;
}




#endif
