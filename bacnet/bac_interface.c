#include "main.h"
#include "bacnet.h"
#include "controls.h"
#include "bactimevalue.h"
#include "bacdevobjpropref.h"
#include <string.h>
#include "datetime.h"


char * itoa( int value, char *string, int radix );
#if ARM_TSTAT_WIFI
#include "Constcode.h"
#endif

extern BACNET_DATE Local_Date;
extern BACNET_TIME Local_Time;

extern uint8_t count_hold_on_bip_to_mstp;

char get_current_mstp_port(void);

#if BAC_COMMON 

char bacnet_vendor_name[20] = BACNET_VENDOR_TEMCO;
char bacnet_vendor_product[20] = BACNET_PRODUCT_TEMCO;
U16_T far Bacnet_Vendor_ID;
// MSTP
void Set_TXEN(uint8_t dir)
{	
	char mstp_port;
#if ARM_MINI	
	mstp_port = get_current_mstp_port();


	if(mstp_port == 2)
	{
		if(Modbus.mini_type == MINI_TINY || Modbus.mini_type == MINI_NEW_TINY || Modbus.mini_type == MINI_TINY_ARM || (Modbus.mini_type == MINI_TINY_11I))
		{			
				UART2_TXEN_TINY = dir;
		}
		else if(Modbus.mini_type == MINI_VAV)
		{			
				UART2_TXEN_VAV = dir;
		}
		else
		{
				UART2_TXEN_BIG = dir;
		}
	}
	else
	{
		if(Modbus.mini_type == MINI_TINY || Modbus.mini_type == MINI_NEW_TINY || Modbus.mini_type == MINI_TINY_ARM || (Modbus.mini_type == MINI_TINY_11I)
			|| Modbus.mini_type == MINI_NANO)
		{	
				UART0_TXEN_TINY = dir;
		}
		else
		{  
				UART0_TXEN_BIG = dir;
		}
	}
#endif

#if (ARM_CM5 || ASIX_CM5)
// dont have mstp for ASIX
	if(mstp_port == 2)
		UART2_TXEN = dir;
	else
		UART0_TXEN = dir;
#endif
	
#if ARM_TSTAT_WIFI
	UART0_TXEN_BIG = dir;
#endif
}

char* get_description(uint8_t type,uint8_t num)
{
	U8_T io_index;
	
#if BAC_TRENDLOG
	if(type == TRENDLOG)
	{
		if(num >= MAX_TRENDLOGS) return NULL;
		return (char*)"tendlog";
	}
#endif
	if(type == CALENDAR)
	{
		if(num >= MAX_CALENDARS) return NULL;
		return (char*)annual_routines[num].label;
	}
	if(type == SCHEDULE)
	{
		if(num >= MAX_SCHEDULES) return NULL;
		return (char*)weekly_routines[num].label;
	}
	if(type == AV)	
	{	
		Get_index_by_AVx(num,&io_index);
		if(io_index >= MAX_AVS) return NULL;

		return (char*)vars[io_index].label;

	}
	if(type == BV)	
	{	
		Get_index_by_BVx(num,&io_index);
		if(io_index >= MAX_BVS) return NULL;

		return (char*)vars[io_index].label;
	}
	if(type == AI)	
	{		
		Get_index_by_AIx(num,&io_index);
		if(io_index >= MAX_AIS) return NULL;
		return (char*)inputs[io_index].label;
	}
	if(type == BI)	
	{		
		Get_index_by_BIx(num,&io_index);
		if(io_index >= MAX_AIS) return NULL;
		return (char*)inputs[io_index].label;
	}
	if(type == AO)
	{		
		Get_index_by_AOx(num,&io_index);
		if(io_index >= MAX_AOS) return NULL;
		return (char*)outputs[io_index].label;
	}
	if(type == BO)
	{
		Get_index_by_BOx(num,&io_index);
		if(io_index >= MAX_BOS) return NULL;
		return (char*)outputs[io_index].label;
	}
#if BAC_PROPRIETARY
	if(type == TEMCOAV)
	{
		if(num == 0) return "panel number";
		else if(num == 1) return "dead master";
		else if(num == 2) return "lcd time off delay";
		else if(num == 3) return "disable icon";
		else if(num == 4) return "lcd display configure";
		else return "reserved";
	}
#endif
#if 1//ARM_TSTAT_WIFI
	if(type == MSV)
	{
		if(num == 0) return "FAN MODE";
		else return "reserved";
	}
#endif
	return NULL;
}

char* get_label(uint8_t type,uint8_t num)
{
	U8_T io_index;

	if(type == SCHEDULE)
	{
		if(num >= MAX_SCHEDULES) return NULL;
		return (char*)weekly_routines[num].description;
	}
	if(type == CALENDAR)
	{
		if(num >= MAX_CALENDARS) return NULL;
		return (char*)annual_routines[num].description;
	}
	if(type == AV)	
	{
		Get_index_by_AVx(num,&io_index);		
		if(io_index >= MAX_AVS) return NULL;
		return (char*)vars[io_index].description;
	}
	if(type == BV)	
	{
		Get_index_by_BVx(num,&io_index);		
		if(io_index >= MAX_BVS) return NULL;
		return (char*)vars[io_index].description;
	}
	if(type == AI)
	{	
		Get_index_by_AIx(num,&io_index);		
		if(io_index >= MAX_AIS) return NULL;
		return (char*)inputs[io_index].description;
	}
	if(type == BI)
	{		
		Get_index_by_BIx(num,&io_index);
		if(io_index >= MAX_AIS) return NULL;
		return (char*)inputs[io_index].description;
	}
	if(type == AO)
	{		
		Get_index_by_AOx(num,&io_index);
		if(io_index >= MAX_AOS) return NULL;
		return (char*)outputs[io_index].description;
	}
	if(type == BO)	
	{
		Get_index_by_BOx(num,&io_index);
		if(io_index >= MAX_BOS) return NULL;
		return (char*)outputs[io_index].description;
	}
#if 1//ARM_TSTAT_WIFI
	if(type == MSV)
	{
		if(num == 0) return "FAN MODE TYPE";
		else return "reserved";
	}
#endif	
	
#if BAC_PROPRIETARY
	if(type == TEMCOAV)
	{
		if(num == 0) return "panel number";
		else if(num == 1) return "dead master";
		else if(num == 2) return "lcd time off delay";
		else if(num == 3) return "disable icon";
		else if(num == 4) return "lcd display configure";
		else return "reserved";
	}
#endif
	
#if BAC_TRENDLOG
	if(type == TRENDLOG)
	{
		return (char*)monitors[io_index].label;
	}
#endif
	return NULL;
}

// tbd: add more range
char get_range(uint8_t type,uint8_t num)
{
	uint8_t io_index;
	
	if(type == AV)	
	{
		Get_index_by_AVx(num,&io_index);
		if(io_index >= MAX_AVS) return 0;
// add unit
		if(vars[io_index].range == not_used_input)
			return UNITS_NO_UNITS;  
		if(vars[io_index].digital_analog == 0)  // digital
		{
			return UNITS_NO_UNITS;	
		}
		else  // analog
		{		
			if(vars[io_index].range == degC)  					return UNITS_DEGREES_CELSIUS;
			else if(vars[io_index].range == degF)  			return UNITS_DEGREES_FAHRENHEIT;
			
			else if(vars[io_index].range == Volts) 			return UNITS_VOLTS;
			else if(vars[io_index].range == KV) 			return UNITS_KILOVOLTS;

			else if(vars[io_index].range == Amps) 			return UNITS_AMPERES;
			else if(vars[io_index].range == ma) 			return UNITS_MILLIAMPERES;
			
			else if(vars[io_index].range == Sec) 			return UNITS_SECONDS;
			else if(vars[io_index].range == Min) 			return UNITS_MINUTES;
			else if(vars[io_index].range == Hours) 			return UNITS_HOURS;
			else if(vars[io_index].range == Days) 			return UNITS_DAYS;
			
			else if(vars[io_index].range == Pa) 			return UNITS_PASCALS;
			else if(vars[io_index].range == KPa) 			return UNITS_KILOPASCALS;
			
			else if(vars[io_index].range == Watts) 			return UNITS_WATTS;
			else if(vars[io_index].range == KW) 			return UNITS_KILOWATTS;
			
			else if(vars[io_index].range == in_w) 			return UNITS_LUMENS;
			
			else if(vars[io_index].range == RH) 			return UNITS_PERCENT_RELATIVE_HUMIDITY;
			
			else if(vars[io_index].range == ppm) 			return UNITS_PARTS_PER_MILLION;
			else if(vars[io_index].range == procent) 			return UNITS_PERCENT;
			else
				return UNITS_NO_UNITS;	
		}
	}
	if(type == AI)	
	{
		Get_index_by_AIx(num,&io_index);
		if(io_index >= MAX_AIS) return 0;
		if(inputs[io_index].digital_analog == 0)  // digital
		{
			return UNITS_NO_UNITS;	
		}
		else  // analog
		{
		
			if((inputs[io_index].range == R10K_40_120DegC) || (inputs[io_index].range == KM10K_40_120DegC)) 
				return UNITS_DEGREES_CELSIUS;
			else if((inputs[io_index].range == R10K_40_250DegF) || (inputs[io_index].range == KM10K_40_250DegF)) 
				return UNITS_DEGREES_FAHRENHEIT;
			else if(inputs[io_index].range == I0_20ma) 
				return UNITS_MILLIAMPERES;
			else if((inputs[io_index].range == V0_10_IN) || (inputs[io_index].range == V0_5)) 
				return UNITS_VOLTS;
			else
				return UNITS_NO_UNITS;	
		}
	}
	if(type == AO)
	{	
		Get_index_by_AOx(num,&io_index);
		
		if(outputs[io_index].range == 0)
			return UNITS_NO_UNITS;	
		else	
		{		
			if(outputs[io_index].range == V0_10)			
				return UNITS_VOLTS;
			else if((outputs[io_index].range == P0_100_Open) || 
				(outputs[io_index].range == P0_100_Close) ||
				(outputs[io_index].range == P0_100))			
				return UNITS_PERCENT;
			else if(outputs[io_index].range == I_0_20ma)			
				return UNITS_MILLIAMPERES;
			else
				return UNITS_NO_UNITS;	
		}
	}
	if(type == BO)	
	{
		if(num >= MAX_BOS) return 0;
		return UNITS_NO_UNITS;	
	}
	if(type == BI)	
	{
		if(num >= MAX_BIS) return 0;
		return UNITS_NO_UNITS;	
	}
	return 0;
}


char Get_Out_Of_Service(uint8_t type,uint8_t num)
{	
	
	uint8_t io_index;
	
	if(type == AO)
	{
		Get_index_by_AOx(num,&io_index);
		return outputs[io_index].out_of_service;
	}
	else if(type == BO)	
	{
		Get_index_by_BOx(num,&io_index);
		return outputs[io_index].out_of_service;
	}		
	else if(type == SCHEDULE)	
	{
		if(num >= MAX_SCHEDULES) return 0;
		return weekly_routines[num].auto_manual;
	}
	else if(type == CALENDAR)	
	{
		if(num >= MAX_CALENDARS) return 0;
		return annual_routines[num].auto_manual;
	}
	else if(type == AV)	
	{
		Get_index_by_AVx(num,&io_index);
		if(io_index >= MAX_AVS) return 0;
		return vars[io_index].auto_manual;
	}
	else if(type == BV)	
	{
		Get_index_by_BVx(num,&io_index);
		if(io_index >= MAX_BVS) return 0;
		return vars[io_index].auto_manual;
	}
	else
		return 0;
}


// i is box, aox,avx ...
float Get_bacnet_value_from_buf(uint8_t type,uint8_t priority,uint8_t i)
{	
	uint8_t io_index;
	switch(type)
	{
		case AV:		
			Get_index_by_AVx(i,&io_index);
//#if ARM_TSTAT_WIFI
//		return (float)(vars[io_index].value) / 1000;
//#endif
			if(vars[io_index].range > 0)
			{
				if(vars[io_index].digital_analog == 1)  // av
				{
					return (float)swap_double(vars[io_index].value) / 1000;
				}
			}
			return 0;
			
		case BV:
			Get_index_by_BVx(i,&io_index);
			if(vars[io_index].range > 0)
			{
				if(vars[io_index].digital_analog == 0)  // bv
				{
					if((vars[io_index].range >= ON_OFF  && vars[io_index].range <= HIGH_LOW)
					||(vars[io_index].range >= custom_digital1 // customer digital unit
					&& vars[io_index].range <= custom_digital8
					&& digi_units[vars[io_index].range - custom_digital1].direct == 1))
					{  // inverse logic
						if(vars[io_index].control == 1)
						{
							return 0;
						}
						else
						{
							return 1;
						}
					}	
					else
					{
						if(vars[io_index].control == 1)
						{
							return 1;
						}
						else
						{
							return 0;
						}
					}	
				}
			}
			return 0;
		case BI:
			Get_index_by_BIx(i,&io_index);
			if(inputs[io_index].range > 0)
			{
				if(inputs[io_index].digital_analog == 0)  // digital
				{
					if((inputs[io_index].range >= ON_OFF  && inputs[io_index].range <= HIGH_LOW)
					||(inputs[io_index].range >= custom_digital1 // customer digital unit
					&& inputs[io_index].range <= custom_digital8
					&& digi_units[inputs[io_index].range - custom_digital1].direct == 1))
					{  // inverse logic
						if(inputs[io_index].control == 1)
						{
							return 0;
						}
						else
						{
							return 1;
						}
					}	
					else
					{
						if(inputs[io_index].control == 1)
						{
							return 1;
						}
						else
						{
							return 0;
						}
					}	
				}
			}
			else
				return input_raw[io_index];
		//break;
		case AI:
			Get_index_by_AIx(i,&io_index);
			if(inputs[io_index].range > 0)
			{
				if(inputs[io_index].digital_analog == 1)  // analog
					return (float)swap_double(inputs[io_index].value) / 1000;
			}
			else
				return input_raw[io_index];
		//break;
		case AO:
		// find output index by AOx
		{
				u8 temp; 
				Get_index_by_AOx(i,&io_index);
#if 1//(ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
				if(io_index >= max_dos + max_aos)
				{
					if(priority == 15)
						return (float)swap_double(outputs[io_index].value) / 1000;
					else
						return 0xff;
				}
#endif				
				if(output_priority[io_index][priority] == 0xff)
					return 0xff;
				
				if(outputs[io_index].digital_analog == 0)
				{					
					temp = output_priority[io_index][priority] ? 1 : 0;
					if((outputs[io_index].range >= ON_OFF && outputs[io_index].range <= HIGH_LOW)
					||(outputs[io_index].range >= custom_digital1 // customer digital unit
					&& outputs[io_index].range <= custom_digital8
					&& digi_units[outputs[io_index].range - custom_digital1].direct == 1))
					{  // inverse logic
						
						if(temp == 1)
						{
							return 0;
						}
						else
						{
							return 1;
						}
					}	
					else
					{
						
						if(temp == 1)
						{
							return 1;
						}
						else
						{
							return 0;
						}
					}
				}
				else	
				{		
#if 1//(ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)  // IF ASIX, dead ????????????????				
					if(io_index >= get_max_internal_output())
							output_raw[io_index] = output_priority[io_index][priority] * 1000;
#endif
					return output_priority[io_index][priority];
				}

			}	
			
		case BO:	
		{
			u8 temp; 
			// priority			
			Get_index_by_BOx(i,&io_index);

			if(io_index >= max_dos + max_aos)
			{
				if(priority == 15)
					return outputs[io_index].value ? 1 : 0;
				else
					return 0xff;
			}
			if(output_priority[io_index][priority] == 0xff)
			{
				return 0xff;	
			}
			if(outputs[io_index].digital_analog == 0)
			{  // digital
				temp = output_priority[io_index][priority] ? 1 : 0;
				if((outputs[io_index].range >= ON_OFF  && outputs[io_index].range <= HIGH_LOW)
				||(outputs[io_index].range >= custom_digital1 // customer digital unit
					&& outputs[io_index].range <= custom_digital8
					&& digi_units[outputs[io_index].range - custom_digital1].direct == 1))
				{  // inverse logic
					if(temp == 1)
					{if(io_index < 2)		Test[18]++;
						return 0;
					}
					else
					{
						return 1;
					}
				}	
				else
				{
					if(temp == 1)
					{
						return 1;
					}
					else
					{
						return 0;
					}
				}
			}
			else
			{ // range is analog
				return output_priority[io_index][priority];
			}
		}
		case SCHEDULE:	
			return weekly_routines[i].value ? 1 : 0;
		//break;
		case CALENDAR:			
			return annual_routines[i].value ? 1 : 0;
		//break;	
#if BAC_PROPRIETARY
		case TEMCOAV:
		{
			uint32 value;
			switch(i)
			{
				case 0: value = panel_number; 
				
				break;
				case 1: 
					if(Modbus.dead_master & 0x80)  // bit7 is tha flag whether enable deadmaster
						value = Modbus.dead_master & 0x7f;
					else  // disable deadmaster
						value = 0;
					break;
					case 2:
						value = Modbus.LCD_time_off_delay;
					break;
					case 3:
						value = Modbus.disable_tstat10_display;
					break;
				break;
			}
			return value;
		}
#endif
		
#if ARM_TSTAT_WIFI
				case MSV:
				{
					switch(i)
					{
						case 0:
							return vars[7].value / 1000;
						default:
							break;

					}
				}
#else
//				case MSV:
//				{
//					switch(i)
//					{
//						case 0:
//							return msv_data[i][].status
//						default:
//							break;

//					}
//				}
				return 0;
#endif
				
		default:
			break;
				
	}	
	return 0;
}

char* Get_Object_Name(void)
{
	return panelname;
}

U8_T Get_WR_ON_OFF(uint8_t object_index,uint8_t day,uint8_t i)
{
	return wr_time_on_off[object_index][day][i];
}
//Wr_one_day					far		wr_times[MAX_WR][MAX_SCHEDULES_PER_WEEK];
// get time & value from weekly roution
BACNET_TIME_VALUE Get_Time_Value(uint8_t object_index,uint8_t day,uint8_t i)
{
	BACNET_TIME_VALUE far array;
	array.Time.hour = wr_times[object_index][day].time[i].hours;
	array.Time.min = wr_times[object_index][day].time[i].minutes;
	array.Time.sec = 0;
	array.Time.hundredths = 0;
	array.Value.type.Enumerated = wr_time_on_off[object_index][day][i];//(i + 1) % 2;
	array.Value.tag = BACNET_APPLICATION_TAG_ENUMERATED;
	return array;

}

uint8_t Get_TV_count(uint8_t object_index,uint8_t day)
{
	char count;
	char i;
	count = 0;
	for(i = 0;i < 8;i++)
	{
		if(wr_time_on_off[object_index][day][i] != 0xff)
			count++;
		//if((wr_times[object_index][day].time[i].hours == 0) && (wr_times[object_index][day].time[i].minutes == 0))
			//return i;
	}
	return count;
}




void Check_wr_time_on_off(uint8_t i,uint8_t j,uint8_t mode)
{
	U8_T k;
	U8_T k1,k2;
	U16_T tmp1,tmp2,tmp;
	U8_T tmp_onoff;
//	Time_on_off tmp;

	
	ChangeFlash = 1;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)	
	write_page_en[WR_TIME] = 1;
	write_page_en[24] = 1;
#endif
	for(k = 0;k < 8;k++)
	{
		if(mode == 0)  // T3000 write weekly roution
		{
			if(wr_times[i][j].time[k].hours == 0 && wr_times[i][j].time[k].minutes == 0 )
				wr_time_on_off[i][j][k] = 0xff;
			else
				wr_time_on_off[i][j][k] = (k + 1) % 2;
		}
		
		if(wr_time_on_off[i][j][k] == 0xff)
		{
			if(wr_times[i][j].time[k].hours == 0 && wr_times[i][j].time[k].minutes == 0 )
			{
				if(k == 0)  // first one is 0
				{
					if(wr_times[i][j].time[k + 1].hours == 0 && wr_times[i][j].time[k + 1].minutes == 0	)
						wr_time_on_off[i][j][k] = 0xff;
					else
						wr_time_on_off[i][j][k] = 1;
				}
				else
					wr_time_on_off[i][j][k] = 0xff;
			}
			else
				wr_time_on_off[i][j][k] = (k + 1) % 2;
		}
		
	}
	
	// sort
	for(k1 = 0;k1 < 8;k1++)
	{
		if(wr_time_on_off[i][j][k1] == 0xff)
			continue;

		tmp1 = 60L * wr_times[i][j].time[k1].hours + wr_times[i][j].time[k1].minutes;

		for(k2 = k1 + 1;k2 < 8;k2++)
		{			
			if(wr_time_on_off[i][j][k2] == 0xff)
				continue;

			tmp2 = 60L *  wr_times[i][j].time[k2].hours + wr_times[i][j].time[k2].minutes;

			if(tmp1 > tmp2)
			{				
				tmp = tmp1;
				tmp1 = tmp2;
				tmp2 = tmp;
				
				wr_times[i][j].time[k1].hours = tmp1 / 60;
				wr_times[i][j].time[k1].minutes = tmp1 % 60;
				
				wr_times[i][j].time[k2].hours = tmp2 / 60;
				wr_times[i][j].time[k2].minutes = tmp2 % 60;
				
				
				tmp_onoff = wr_time_on_off[i][j][k1];
				wr_time_on_off[i][j][k1] = wr_time_on_off[i][j][k2]; 
				wr_time_on_off[i][j][k2] = tmp_onoff;
				
			}
		}
	}
	
	
}

void Check_All_WR(void)
{
	U8_T i,j;
	for(i = 0;i < MAX_WR;i++)
		for(j = 0;j < 9;j++)	
			Check_wr_time_on_off(i,j,1);
	
}

BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE * Get_Object_Property_References(uint8_t i)
{
//	BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE obj;
//	memset(&obj,0,sizeof(BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE));
//	obj.arrayIndex = -1;
//	
//	obj.objectIdentifier.instance = 0;
//	obj.objectIdentifier.type = 0;

//	obj.propertyIdentifier = 85;

////	obj.arrayIndex = 0x5678;

//	obj.deviceIndentifier.instance = 0;
//	obj.deviceIndentifier.type = 0;
//	return &obj;
	return NULL;//&obj;
}


uint8_t Get_CALENDAR_count(uint8_t object_index)
{
	uint16_t i,j;
	uint16_t count;
	count = 0;
	
	for(i = 0;i < AR_DATES_SIZE;i++)
	{
		for(j = 0;j < 8;j++)
		{
			if(ar_dates[object_index][i] & (0x01 << j)) 
			{
				count++;
			}
		}
	}
	
	return count;
}

void clear_calendar_data(uint8 index)
{
	memset(&ar_dates[index],0,AR_DATES_SIZE);
}

void get_bacnet_date_by_dayofyear(uint16_t dayofyear,BACNET_DATE * array)
{
	uint16_t far day;
	uint16_t far i;//,j,k;

	day = dayofyear + 1;
	
	if( ( Rtc.Clk.year & '\x03' ) == '\x0' )
		month_length[1] = 29;
	else
		month_length[1] = 28;
	
	for(i = 0;i < 12;i++)
	{
		if(day > month_length[i])
		{
			day -= month_length[i];
		}
		else
		{
			array->month = i + 1;
			array->day = day;
//			if(array->day == month_length[i] )
//			{
//				array->month++;
//				array->day = 1;
//			}
			i = 12;
		}
	}
	
	if(Rtc.Clk.day_of_year > dayofyear)
	{
		day = Rtc.Clk.week + 7 - (Rtc.Clk.day_of_year - dayofyear) % 7;
	}
	else
		day = Rtc.Clk.week + (dayofyear - Rtc.Clk.day_of_year) % 7;
	
	array->wday = day % 7 + 1;
	array->year = 2000 + Rtc.Clk.year;
	

	
}


BACNET_DATE Get_Calendar_Date(uint8_t object_index,uint8_t k)
{
	BACNET_DATE far array;
	
	uint16_t i,j;
	uint16_t dayofyear;
	uint16_t count;
	count = 0;


	for(i = 0;i < AR_DATES_SIZE;i++)
	{	
		for(j = 0;j < 8;j++)
		{
			if(ar_dates[object_index][i] & (0x01 << j)) 
			{
				if(k == count) // find current count
				{
					dayofyear = i * 8 + j;
					// map to BACNET_DATE
					get_bacnet_date_by_dayofyear(dayofyear,&array);
				}
				count++;	
			}
					
		}
	}
	return array;
}




void write_bacnet_unit_to_buf(uint8_t type,uint8_t priority,uint8_t i,uint8_t unit)
{
	U8_T temp;
	uint8_t io_index;

	ChangeFlash = 1;

		switch(type)
		{
			case AI:
				Get_index_by_AIx(i,&io_index);
				if(io_index >=  MAX_AIS) break;

				if(unit == UNITS_NO_UNITS)
				{
					inputs[io_index].range = not_used_input;			
				}
				if(unit == UNITS_DEGREES_CELSIUS)
				{
					inputs[io_index].range = R10K_40_120DegC;
				}
				if(unit == UNITS_DEGREES_FAHRENHEIT)
				{
					inputs[io_index].range = R10K_40_250DegF;	
				}				
				if(unit == UNITS_AMPERES)
				{
						inputs[io_index].range = I0_20ma;
					// software jumper 
					temp = inputs[io_index].decom;
					temp &= 0x0f;
					temp |= (INPUT_I0_20ma << 4);
					inputs[io_index].decom = temp;
				}
				if(unit == UNITS_VOLTS)
				{
						inputs[io_index].range = V0_10_IN;
					// software jumper 
					temp = inputs[io_index].decom;
					temp &= 0x0f;
					temp |= (INPUT_0_10V << 4);
					inputs[io_index].decom = temp;
				}
				
				if( (unit != UNITS_VOLTS) && (unit != UNITS_AMPERES) )
				{
						// software jumper 
					temp = inputs[io_index].decom;
					temp &= 0x0f;
					temp |= (INPUT_THERM << 4);
					inputs[io_index].decom = temp;
				}
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
				write_page_en[IN] = 1;
#endif
#if  T3_MAP
				push_expansion_in_stack(&inputs[io_index]);
#endif
				break;
			case BO:
				Get_index_by_BOx(i,&io_index);

				if(io_index >= MAX_BOS) break;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
				write_page_en[OUT] = 1;
#endif
				outputs[io_index].digital_analog = 0;
				if(unit == UNITS_NO_UNITS)
					outputs[io_index].range = 0;
				else
					outputs[io_index].range = OFF_ON;
#if  T3_MAP
				push_expansion_out_stack(&outputs[io_index],io_index,1);
#endif
				break;
			case AO:
				Get_index_by_AOx(i,&io_index);

				if(io_index >= MAX_AOS) break;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
				write_page_en[OUT] = 1;
#endif
			
				outputs[io_index].digital_analog = 1;
				if(unit == UNITS_NO_UNITS)
					outputs[io_index].range = 0;
				else
					outputs[io_index].range = V0_10;
#if  T3_MAP				
				push_expansion_out_stack(&outputs[io_index],io_index,1);	
#endif				
				break;
	
			default:
			break;
		}
}

void write_bacnet_description_to_buf(uint8_t type,uint8_t priority,uint8_t i,char* str)
{
	uint8_t io_index;

	//if((AV_Present_Value[0] == Modbus.address) ||(AV_Present_Value[0] == 0) ) // internal AV, DI ,AV ...

	ChangeFlash = 1;

		switch(type)
		{
			case AI:
				Get_index_by_AIx(i,&io_index);
				if(io_index >= MAX_AIS) break;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
				write_page_en[IN] = 1;
#endif
				memcpy(inputs[io_index].label,str,8);
#if  T3_MAP
				push_expansion_in_stack(&inputs[io_index]);
#endif
				break;
			case BI:
				Get_index_by_BIx(i,&io_index);
				if(io_index >= MAX_AIS) break;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
				write_page_en[IN] = 1;
#endif
				memcpy(inputs[io_index].label,str,8);
#if  T3_MAP
				push_expansion_in_stack(&inputs[io_index]);
#endif
				break;
			case BO:
				Get_index_by_BOx(i,&io_index);
				if(io_index >= MAX_BOS) break;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
				write_page_en[OUT] = 1;
#endif
				memcpy(outputs[io_index].label,str,8);
#if  T3_MAP
				push_expansion_out_stack(&outputs[io_index],io_index,1);
#endif
				break;
			case AO:
				Get_index_by_AOx(i,&io_index);
				if(io_index >= MAX_AOS) break;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
			write_page_en[OUT] = 1;
#endif
				memcpy(outputs[io_index].label,str,8);
#if  T3_MAP
				push_expansion_out_stack(&outputs[io_index],io_index,1);
#endif
				break;
			case AV:
				Get_index_by_AVx(i,&io_index);
				if(io_index >= MAX_AVS) break;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
			write_page_en[VAR] = 1;
#endif
				memcpy(vars[io_index].label,str,8);
				break;
			case BV:
				Get_index_by_BVx(i,&io_index);
				if(io_index >= MAX_BVS) break;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
			write_page_en[VAR] = 1;
#endif
				memcpy(vars[io_index].label,str,8);
				break;
			case SCHEDULE:
				if(i >= MAX_SCHEDULES) break;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
			write_page_en[WRT] = 1;
#endif
				memcpy(weekly_routines[i].label,str,8);
				break;
			case CALENDAR:
				if(i >= MAX_CALENDARS) break;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
			write_page_en[AR] = 1;
#endif
				memcpy(annual_routines[i].label,str,8);
				break;
//#if ARM_TSTAT_WIFI
//			case MSV:
//				memcpy(annual_routines[i].label,str,8);
//				break;
//#endif
			default:
			break;
		}
			
}


void wirte_bacnet_value_to_buf(uint8_t type,uint8_t priority,uint8_t i,float value)
{
	uint8_t io_index;

	ChangeFlash = 1;

		switch(type)
		{
			case AV:

			Get_index_by_AVx(i,&io_index);
			if(io_index >= MAX_AVS) break;
#if ARM_TSTAT_WIFI
			vars[io_index].value = swap_double(value * 1000); 
			switch(io_index)
			{
				case 0:	uart0_baudrate = value; break;
				case 1: Station_NUM = value;break;
				case 2: Modbus.com_config[0] = value;break;
				case 3:	Instance  = value;break;
				default:
					break;
			}
			break;
#endif			
			if(vars[io_index].digital_analog == 1)  // analog
			{	
				if(vars[io_index].range == 0)
					vars[io_index].range = R10K_40_120DegC;
				vars[io_index].value = swap_double(value * 1000) ;
			}
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
		write_page_en[VAR] = 1;
#endif
			vars[io_index].value = swap_double(value * 1000);
				break;
			case BV:
			Get_index_by_BVx(i,&io_index);			
			if(io_index >= MAX_BVS) break;
			
			if(vars[io_index].digital_analog == 0)  // digital
			{
				if(vars[io_index].range == 0)
					vars[io_index].range = OFF_ON;
				vars[io_index].value = swap_double(value * 1000);	

				if(( vars[io_index].range >= ON_OFF && vars[io_index].range <= HIGH_LOW )
				||(vars[io_index].range >= custom_digital1 // customer digital unit
					&& vars[io_index].range <= custom_digital8
					&& digi_units[vars[io_index].range - custom_digital1].direct == 1))
				{ // inverse
					vars[io_index].control = value ? 0 : 1;		
				}
				else
				{
					vars[io_index].control = value ? 1 : 0;	
				}
			}
			
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
		write_page_en[VAR] = 1;
#endif
				break;
			case AI:
				Get_index_by_AIx(i,&io_index);
				if(io_index >= MAX_AIS) break;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)			
				write_page_en[IN] = 1;
#endif
				if(inputs[io_index].digital_analog == 1)  // digital
				{	
					if(inputs[io_index].range == 0)
						inputs[io_index].range = R10K_40_120DegC;
					inputs[io_index].value = swap_double(value * 1000) ;
				}
#if  T3_MAP
				push_expansion_in_stack(&inputs[io_index]);
#endif
				break;
				
			case BI:
				Get_index_by_BIx(i,&io_index);
				if(io_index >= MAX_AIS) break;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)			
				write_page_en[IN] = 1;
#endif
				if(inputs[io_index].digital_analog == 0)  // digital
				{
					if(inputs[io_index].range == 0)
						inputs[io_index].range = OFF_ON;
					inputs[io_index].value = swap_double(value * 1000);	
//					inputs[i].control = value ? 1 : 0;	

					if(( inputs[io_index].range >= ON_OFF && inputs[io_index].range <= HIGH_LOW )
					||(inputs[io_index].range >= custom_digital1 // customer digital unit
					&& inputs[io_index].range <= custom_digital8
					&& digi_units[inputs[io_index].range - custom_digital1].direct == 1))
					{
						inputs[io_index].control = value ? 0 : 1;		
					}	
					else
					{
						inputs[io_index].control = value ? 1 : 0;							
					}	
				}
				else
				{	
					if(inputs[io_index].range == 0)
						inputs[io_index].range = R10K_40_120DegC;
					inputs[io_index].value = swap_double(value * 1000) ;
				}
#if  T3_MAP
				push_expansion_in_stack(&inputs[io_index]);
#endif
				break;
			case BO:
				Get_index_by_BOx(i,&io_index);
				if(io_index >= MAX_BOS) break;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
				write_page_en[OUT] = 1;
#endif
				if(value == 0xff)	
					output_priority[io_index][priority] = 0xff;	
				else
				{
					if(( outputs[io_index].range >= ON_OFF && outputs[io_index].range <= HIGH_LOW )
					||(outputs[io_index].range >= custom_digital1 // customer digital unit
					&& outputs[io_index].range <= custom_digital8
					&& digi_units[outputs[io_index].range - custom_digital1].direct == 1))
					{ // inverse
						if(io_index < max_dos + max_aos)
						{
							output_priority[io_index][priority] = value ? 0 : 1;
							outputs[io_index].control = Binary_Output_Present_Value(io_index)/*value*/ ? 0 : 1;		
						}
						else
						{
							outputs[io_index].control = value ? 0 : 1;
							output_priority[io_index][15] = value ? 0 : 1;
							if(priority != 15)
								output_priority[io_index][priority] = 0xff;							
						}						
					}
					else
					{						
						if(io_index < max_dos + max_aos)
						{
							output_priority[io_index][priority] = value ? 1 : 0;						
							outputs[io_index].control = Binary_Output_Present_Value(io_index)/*value*/ ? 1 : 0;	
						}
						else
						{
							outputs[io_index].control = value ? 1 : 0;	
							if(priority != 15)
								output_priority[io_index][priority] = 0xff;
							output_priority[io_index][15] = value ? 1 : 0;
						}					
					}	
			
				}
				
				outputs[io_index].digital_analog = 0;
				check_output_priority_array_without_AM(io_index);
#if OUTPUT_DEATMASTER
			clear_dead_master();
#endif				
				if(io_index >= max_dos + max_aos)
				{
					outputs[io_index].auto_manual = 1;
				}

				if(outputs[io_index].range == 0)
					outputs[io_index].range = OFF_ON;			
					

#if  T3_MAP

					push_expansion_out_stack(&outputs[io_index],io_index,0);
#endif
				break;
			case AO:	
				Get_index_by_AOx(i,&io_index);
					if(io_index >= MAX_AOS) break;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
			write_page_en[OUT] = 1;
#endif				
				
				if(outputs[io_index].range == 0)
					outputs[io_index].range = V0_10;
				outputs[io_index].digital_analog = 1;

				if(io_index < max_aos + max_dos)
				{
					output_priority[io_index][priority] = value;	
					outputs[io_index].value = swap_double(Analog_Output_Present_Value(io_index) * 1000);//swap_double(value * 1000);
				}

				if(io_index >= max_dos + max_aos)
				{
					outputs[io_index].auto_manual = 1;
					output_priority[io_index][15] = value;
					if(priority != 15)
						output_priority[io_index][priority] = 0xff;
					outputs[io_index].value = swap_double(value * 1000);
					//outputs[out_index].value = swap_double(Analog_Output_Present_Value(out_index) * 1000);//swap_double(value * 1000);
					output_raw[io_index] = swap_double(value * 1000);
				}
//				output_priority[out_index][priority] = value;	
//				outputs[out_index].value = swap_double(Analog_Output_Present_Value(out_index) * 1000);//swap_double(value * 1000);
				check_output_priority_array_without_AM(io_index);
#if OUTPUT_DEATMASTER
			clear_dead_master();
#endif
				
#if  T3_MAP
				push_expansion_out_stack(&outputs[io_index],io_index,0);
#endif
				break;
		 case SCHEDULE:
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
			 write_page_en[WRT] = 1;
#endif
				//if(weekly_routines[i].auto_manual == 1)
				{
					weekly_routines[i].value = swap_double(value * 1000);
				}
				break;
			case CALENDAR:
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
				write_page_en[AR] = 1;
#endif
				//if(annual_routines[i].auto_manual == 1)
				{
					annual_routines[i].value = swap_double(value * 1000);
				}
				break;
#if BAC_PROPRIETARY
			case TEMCOAV:
			if(i == 1)
			{
				Modbus.dead_master = value;				
				E2prom_Write_Byte(EEP_DEAD_MASTER,Modbus.dead_master);
				clear_dead_master();
			}
			else if(i == 2)
			{
				Modbus.LCD_time_off_delay = value;				
				E2prom_Write_Byte(EEP_LCD_TIME_OFF_DELAY,value);
				count_lcd_time_off_delay = 0;
			}
			else if(i == 3)
			{
				Modbus.disable_tstat10_display = value;
				E2prom_Write_Byte(EEP_DISABLE_T10_DIS,value);
			}
			break;
#endif
			
#if ARM_TSTAT_WIFI
			vars[7].value = value * 1000;
			
#endif
			default:
			break;
		}			
}


void write_Out_Of_Service(uint8_t type,uint8_t i,uint8_t am)
{
	uint8_t io_index;

	ChangeFlash = 1;

	if(type == BO)
	{	
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
		write_page_en[OUT] = 1;
#endif
		Get_index_by_BOx(i,&io_index);
		outputs[io_index].auto_manual = am;
#if  T3_MAP		
		push_expansion_out_stack(&outputs[io_index],io_index,1);
#endif
	}
	if(type == AO)
	{
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
		write_page_en[OUT] = 1;
#endif
		Get_index_by_AOx(i,&io_index);
		outputs[io_index].auto_manual = am;
#if  T3_MAP
		push_expansion_out_stack(&outputs[io_index],io_index,1);
#endif 
	}
	if(type == BV)
	{	
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
		write_page_en[VAR] = 1;
#endif
		Get_index_by_BVx(i,&io_index);
		vars[io_index].auto_manual = am;
	}
	if(type == AV)
	{
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
		write_page_en[VAR] = 1;
#endif
		Get_index_by_AVx(i,&io_index);
		vars[io_index].auto_manual = am;
	}
}

void write_bacnet_name_to_buf(uint8_t type,uint8_t priority,uint8_t i,char* str)
{
	uint8_t io_index;

	ChangeFlash = 1;

	switch(type)
	{
		case AI:
			Get_index_by_AIx(i,&io_index);
			if(io_index >= MAX_AVS) break;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
		write_page_en[IN] = 1;
#endif
			memcpy(inputs[io_index].description,str,21);
#if  T3_MAP
			push_expansion_in_stack(&inputs[io_index]);
#endif
			break;
		case BI:
			Get_index_by_BIx(i,&io_index);
			if(io_index >= MAX_AVS) break;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
		write_page_en[IN] = 1;
#endif
			memcpy(inputs[io_index].description,str,21);
#if  T3_MAP
			push_expansion_in_stack(&inputs[io_index]);
#endif
			break;
		case BO:
			Get_index_by_BOx(i,&io_index);
			if(io_index >= MAX_BOS) break;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
		write_page_en[OUT] = 1;
#endif
			memcpy(outputs[io_index].description,str,19);
#if  T3_MAP
			push_expansion_out_stack(&outputs[io_index],io_index,1);
#endif
			break;
		case AO:
			Get_index_by_AOx(i,&io_index);
			if(io_index >= MAX_AOS) break;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
		write_page_en[OUT] = 1;
#endif
			memcpy(outputs[io_index].description,str,19);
#if  T3_MAP
			push_expansion_out_stack(&outputs[io_index],io_index,1);
#endif 
			break;
		case AV:
			Get_index_by_AVx(i,&io_index);
			if(io_index >= MAX_AVS) break;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
		write_page_en[VAR] = 1;
#endif
			memcpy(vars[io_index].description,str,21);
			break;
		case BV:
			Get_index_by_BVx(i,&io_index);
			if(io_index >= MAX_AVS) break;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
		write_page_en[VAR] = 1;
#endif
			memcpy(vars[io_index].description,str,21);
			break;
		case SCHEDULE:
			if(i >= MAX_SCHEDULES) break;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
		write_page_en[WRT] = 1;
#endif
			memcpy(weekly_routines[i].description,str,21);
			break;
		case CALENDAR:
			if(i >= MAX_CALENDARS) break;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
		write_page_en[AR] = 1;
#endif
			memcpy(annual_routines[i].description,str,21);
			break;
		default:
		break;
	}

}



void Set_Object_Name(char * name)
{	
	ChangeFlash = 1;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
	write_page_en[24] = 1;
#endif
	// store it to flash memory
	memcpy(panelname,name,20);	
}


void Clear_Time_Value(uint8_t index,uint8_t day)
{
	char i;
	for(i = 0;i < 8;i++)
	{
		wr_times[index][day].time[i].hours = 0;
		wr_times[index][day].time[i].minutes = 0;
		wr_time_on_off[index][day][i] = 0xff;
	}
}



#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
void write_Time_Value(uint8_t index,uint8_t day,uint8_t i,uint8_t hour,uint8_t min/*BACNET_TIME_VALUE time_value*/,uint8_t value)
{	
	ChangeFlash = 1;
	write_page_en[WR_TIME] = 1;
	wr_times[index][day].time[i].hours = hour;//time_value.Time.hour;
	wr_times[index][day].time[i].minutes = min;//time_value.Time.min;
	wr_time_on_off[index][day][i] = value;	
}
#endif

#if (ASIX_MINI || ASIX_CM5)
void write_Time_Value(uint8_t index,uint8_t day,uint8_t i,BACNET_TIME_VALUE time_value)
{	
	ChangeFlash = 1;
	wr_times[index][day].time[i].hours = time_value.Time.hour;
	wr_times[index][day].time[i].minutes = time_value.Time.min;

}
#endif


void write_annual_date(uint8_t index,BACNET_DATE date)
{
	uint16_t day;
	char j;

	ChangeFlash = 1;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
	write_page_en[AR_DATA] = 1;
#endif
	if(date.year != 2000 + Rtc.Clk.year) return;
	
	if( ( Rtc.Clk.year & '\x03' ) == '\x0' )
		month_length[1] = 29;
	else
		month_length[1] = 28;
	
	day = 0;
	if(date.month >= 1 && date.month <= 12)
	{
		for(j = 0;j < date.month - 1;j++)
			day += month_length[j];
	}
	day += date.day;
	ar_dates[index][(day - 1) / 8] |= (0x01 << ((day - 1) % 8));

}


#endif


U16_T Get_Vendor_ID(void)
{
	switch(Bacnet_Vendor_ID)
	{// 保留0 1 2 255 65535 这几个特殊id，兼容老的做法
		case 1: //netixcontrols
		case BACNET_VENDOR_ID_NETIX:
			memcpy(bacnet_vendor_name,BACNET_VENDOR_NETIX,20);
			memcpy(bacnet_vendor_product,BACNET_PRODUCT_NETIX,20);
//				bacnet_vendor_name = BACNET_VENDOR_NETIX;
//				bacnet_vendor_product = BACNET_PRODUCT_NETIX;
				return BACNET_VENDOR_ID_NETIX;
		
		case 2: // jet controls
		case BACNET_VENDOR_ID_JET:
//				bacnet_vendor_name = BACNET_VENDOR_JET;
//				bacnet_vendor_product = BACNET_PRODUCT_JET;
		memcpy(bacnet_vendor_name,BACNET_VENDOR_JET,20);
		memcpy(bacnet_vendor_product,BACNET_PRODUCT_JET,20);
				return BACNET_VENDOR_ID_JET;
		case 3: // NEWRON
		case BACNET_VENDOR_ID_NEWRON:
//				bacnet_vendor_name = BACNET_VENDOR_JET;
//				bacnet_vendor_product = BACNET_PRODUCT_JET;
		memcpy(bacnet_vendor_name,BACNET_VENDOR_NEWRON,20);
		memcpy(bacnet_vendor_product,BACNET_PRODUCT_NEWRON,20);
		return BACNET_VENDOR_ID_JET;
		
		case BACNET_VENDOR_ID_TEMCO:
		case 255:
		case 65535:
		case 0:
//			bacnet_vendor_name = BACNET_VENDOR_TEMCO;
//			bacnet_vendor_product = BACNET_PRODUCT_TEMCO;
		memcpy(bacnet_vendor_name,BACNET_VENDOR_TEMCO,20);
		memcpy(bacnet_vendor_product,BACNET_PRODUCT_TEMCO,20);
			return BACNET_VENDOR_ID_TEMCO;
		
		default:
//			bacnet_vendor_name = BACNET_VENDOR_TEMCO;
//			bacnet_vendor_product = BACNET_PRODUCT_TEMCO;
			return Bacnet_Vendor_ID;
	 }
}



const char*  Get_Vendor_Name(void)
{
	return bacnet_vendor_name;
}

const char*  Get_Vendor_Product(void)
{

	return bacnet_vendor_product;
}

// T3-IO里面有加下面的函数，让客户自己编辑，T3-Controller还没有加
void Set_Vendor_Name(char* name)
{
#if ARM_MINI	
	write_page_en[25] = 1;
#endif
	memcpy(bacnet_vendor_name,name,20);
	Flash_Write_Mass();
}

void Set_Vendor_Product(char* product)
{
#if ARM_MINI	
	write_page_en[25] = 1;
#endif
	memcpy(bacnet_vendor_product,product,20);
	Flash_Write_Mass();
}

void Set_Vendor_ID(uint16_t vendor_id)
{
//	if(vendor_id == 1)
//	{
//		//Bacnet_Vendor_ID = BACNET_VENDOR_ID_NETIX;
//	}
//	if(vendor_id == 2)
//	{
//		//Bacnet_Vendor_ID = 
//	}
#if ARM_MINI	
	AT24CXX_WriteOneByte(EEP_BAC_VENDOR_ID_LO ,vendor_id);
	AT24CXX_WriteOneByte(EEP_BAC_VENDOR_ID_HI ,vendor_id >> 8);
#endif
	if((vendor_id == 0) || (vendor_id == 255) || (vendor_id == 65535) // temco
		|| (vendor_id == 1)  // netIX
		|| (vendor_id == 2))  // JET
	{
		//Bacnet_Vendor_ID = 
		
		return;
	}
	else
		Bacnet_Vendor_ID = vendor_id;
	
}

void Set_Daylight_Saving_Status(bool status)
{
	Rtc.Clk.is_dst = status;
}
	
bool Get_Daylight_Savings_Status(void)
{
	return Rtc.Clk.is_dst;
}




#if 1//BAC_TIMESYNC
bool write_Local_Date(BACNET_DATE* array)
{
	memcpy(&Local_Date,array,sizeof(BACNET_DATE));
	Rtc.Clk.day = array->day;
	Rtc.Clk.mon = array->month;
	Rtc.Clk.week = array->wday;
	Rtc.Clk.year = (array->year - 60) & 0xff;
#if (ASIX_MINI || ASIX_CM5)
	Rtc_Set(Rtc.Clk.year,Rtc.Clk.mon,Rtc.Clk.day,Rtc.Clk.hour,Rtc.Clk.min,Rtc.Clk.sec,0);
	//Updata_Clock(0);
#endif
	
	Local_Date.year = Rtc.Clk.year + 2000;

	flag_Updata_Clock = 1;
	return 1;

}


bool write_Local_Time(BACNET_TIME* array)
{
	memcpy(&Local_Time,array,sizeof(BACNET_TIME));
	Rtc.Clk.hour = array->hour;
	Rtc.Clk.min = array->min;
	Rtc.Clk.sec = array->sec;
#if (ASIX_MINI || ASIX_CM5)
	Rtc_Set(Rtc.Clk.year,Rtc.Clk.mon,Rtc.Clk.day,Rtc.Clk.hour,Rtc.Clk.min,Rtc.Clk.sec,0);
#endif


	flag_Updata_Clock = 1;
	return 1;
}


#endif

S16_T Get_UTC_Offset(void)
{
	return timezone;
}


void Set_UTC_OFFset(void/*S16_T tz*/)  //??????????????????
{
//	timezone = tz;
}


void Send_TimeSync_Broadcast(uint8_t protocal)
{
	BACNET_DATE bdate;
	BACNET_TIME btime;
	
	memcpy(&bdate,/*Get_Local_Date()*/&Local_Date,sizeof(BACNET_DATE));
	memcpy(&btime,/*Get_Local_Time()*/&Local_Time,sizeof(BACNET_TIME));

#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
#if (ARM_MINI || ARM_CM5)
  if(protocal == BAC_IP_CLIENT)
	{
		Send_bip_Flag = 1;	
		count_send_bip = 0;
		Send_bip_count = MAX_RETRY_SEND_BIP;	
//		Set_broadcast_bip_address(0xffffffff);	
		Send_TimeSync(&bdate,&btime,protocal);
	}
	else if(protocal == BAC_MSTP)
	{
//		Send_Time_Sync = 1;  //?????????????????
	}
#endif	
#endif
}

//void store_output_relinguish(U8_T i, U32_T value)
//{
//	AT24CXX_WriteOneByte(EEP_OUTPUT_RELINQUISH_START + i * 4,(U32_T)value >> 24);
//	AT24CXX_WriteOneByte(EEP_OUTPUT_RELINQUISH_START + i * 4 + 1,(U32_T)value >> 16);
//	AT24CXX_WriteOneByte(EEP_OUTPUT_RELINQUISH_START + i * 4 + 2,(U32_T)value >> 8);
//	AT24CXX_WriteOneByte(EEP_OUTPUT_RELINQUISH_START + i * 4 + 3,(U8_T)value);
//}


void write_Output_Relinguish(uint8_t type,uint8_t i,float value)
{
	ChangeFlash = 1;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
  write_page_en[25] = 1;
#endif
	if(type == BO)
	{
		if(i < max_dos)
		{			
			output_relinquish[i] = value;	
			if(outputs[i].digital_analog == 0)
			{				
				if(( outputs[i].range >= ON_OFF && outputs[i].range <= HIGH_LOW )
					||(outputs[i].range >= custom_digital1 // customer digital unit
					&& outputs[i].range <= custom_digital8
					&& digi_units[outputs[i].range - custom_digital1].direct == 1))
				{// inverse
					output_relinquish[i] = value ? 0 : 1;			
					outputs[i].control = Binary_Output_Present_Value(i) ? 0 : 1;	
				}		
				else
				{
					output_relinquish[i] = value ? 1 : 0;	
					outputs[i].control = Binary_Output_Present_Value(i) ? 1 : 0;	
				}
				
				if(outputs[i].control) 
					set_output_raw(i,1000);
				else 
					set_output_raw(i,0);
			}
			else
				output_relinquish[i] = value;
			
			check_output_priority_array(i,0);
		}
	}
	else if(type == AO)
	{
		output_relinquish[i + max_dos] = value;
		output_raw[i + max_dos] = value;
		// set output_raw
		check_output_priority_array(i + max_dos,0);
	}
}

float Get_Output_Relinguish(uint8_t type,uint8_t i)
{
	if(type == BO)
	{
		if(i < max_dos)
			return output_relinquish[i];			
	}
	else
		return output_relinquish[i + max_dos];
}

uint8_t AI_Index_To_Instance[MAX_INS];
uint8_t BI_Index_To_Instance[MAX_INS];
uint8_t AO_Index_To_Instance[MAX_AOS];
uint8_t BO_Index_To_Instance[MAX_AOS];
uint8_t AV_Index_To_Instance[MAX_AVS];
uint8_t BV_Index_To_Instance[MAX_AVS];

uint8_t AI_Instance_To_Index[MAX_INS];
uint8_t BI_Instance_To_Index[MAX_INS];
uint8_t AO_Instance_To_Index[MAX_AOS];
uint8_t BO_Instance_To_Index[MAX_AOS];
uint8_t AV_Instance_To_Index[MAX_AVS];
uint8_t BV_Instance_To_Index[MAX_AVS];

void Count_IN_Object_Number(void)
{
	U8_T count1,count2,i;
	count1 = 0;
	count2 = 0;
	for(i = 0;i < base_in;i++)
	{
		if(inputs[i].digital_analog == 1)
		{
			AI_Index_To_Instance[count1] = i;
			AI_Instance_To_Index[i] = count1;
			count1++;
		}
		else
		{
			BI_Index_To_Instance[count2] = i;
			BI_Instance_To_Index[i] = count2;
			count2++;
		}
	}
	AIS = count1;
	BIS = count2;
}


void Count_OUT_Object_Number(void)
{
	U8_T count1,count2,i;
	count1 = 0;
	count2 = 0;
	for(i = 0;i < base_out;i++)
	{
		if(outputs[i].digital_analog == 1)
		{
			AO_Index_To_Instance[count1] = i;
			AO_Instance_To_Index[i] = count1;
			count1++;
		}
		else
		{
			BO_Index_To_Instance[count2] = i;
			BO_Instance_To_Index[i] = count2;
			count2++;
		}
	}
	AOS = count1;
	BOS = count2;
}



void Count_VAR_Object_Number(void)
{
	U8_T count1,count2,i;
	count1 = 0;
	count2 = 0;
	for(i = 0;i < MAX_VARS;i++)
	{
		if((vars[i].range != 0) &&(vars[i].digital_analog == 1))
		{
			AV_Index_To_Instance[count1] = i;
			AV_Instance_To_Index[i] = count1;
			count1++;
		}
		if((vars[i].range != 0) &&(vars[i].digital_analog == 0))
		{
			BV_Index_To_Instance[count2] = i;
			BV_Instance_To_Index[i] = count2;
			count2++;
		}
	}
	AVS = count1;
	BVS = count2;

//#if ARM_TSTAT_WIFI 
//	
//	AVS = 20;
//	BVS = 0;
//	for(i = 0;i < AVS;i++)
//	{
//		AV_Index_To_Instance[i] = i;
//		AV_Instance_To_Index[i] = i;
//	}
//#endif
}


int Get_Bacnet_Index_by_Number(U8_T type,U8_T number)
{
	U8_T count,i;
	count = 0;
	switch(type)
	{
		case OBJECT_ANALOG_INPUT:
			for(i = 0;i < base_in;i++)
			{
				if(inputs[i].digital_analog == 1)
				{					
					if(i == number)
						return count;
					count++;
				}
			}
			break;
		case OBJECT_BINARY_INPUT:
			for(i = 0;i < base_in;i++)
			{
				if(inputs[i].digital_analog == 0)
				{
					if(i == number)
						return count;
					count++;
				}
			}
			break;
		case OBJECT_BINARY_OUTPUT:
			for(i = 0;i < base_out;i++)
			{
				if(outputs[i].digital_analog == 0)
				{
					if(i == number)
						return count;
					count++;
				}
			}
			break;
		case OBJECT_ANALOG_OUTPUT:
			for(i = 0;i < base_out;i++)
			{
				if(outputs[i].digital_analog == 1)
				{
					if(i == number)
						return count;
					count++;
				}
			}
			break;
		case OBJECT_ANALOG_VALUE:
			for(i = 0;i < MAX_VARS;i++)
			{
				if((vars[i].range != 0) &&(vars[i].digital_analog == 1))
				{					
					if(i == number)
						return count;
					count++;
				}
			}
			break;
		case OBJECT_BINARY_VALUE:
			for(i = 0;i < MAX_INS;i++)
			{
				if((vars[i].range != 0) &&(vars[i].digital_analog == 0))
				{
					if(i == number)
						return count;
					count++;
				}
			}
			break;
		default:
			break;
	}	
	
	return -1;
}

int Get_Number_by_Bacnet_Index(U8_T type,U8_T index)
{
	U8_T count,i;
	count = 0;
	switch(type)
	{
		case OBJECT_ANALOG_INPUT:
			for(i = 0;i < base_in;i++)
			{
				if(inputs[i].digital_analog == 1)
				{
					if(count == index)
						return i;
					count++;
				}
			}
			break;
		case OBJECT_BINARY_INPUT:
			for(i = 0;i < base_in;i++)
			{
				if(inputs[i].digital_analog == 0)
				{					
					if(count == index)
						return i;
					count++;
				}
			}
			break;
		case OBJECT_BINARY_OUTPUT:
			for(i = 0;i < base_out;i++)
			{
				if(outputs[i].digital_analog == 0)
				{					
					if(count == index)
						return i;
					count++;
				}
			}
			break;
		case OBJECT_ANALOG_OUTPUT:
			for(i = 0;i < base_out;i++)
			{
				if(outputs[i].digital_analog == 1)
				{					
					if(count == index)
						return i;
					count++;
				}
			}
			break;
		case OBJECT_ANALOG_VALUE:
			for(i = 0;i < MAX_VARS;i++)
			{
				if((vars[i].range != 0) &&(vars[i].digital_analog == 1))
				{
					if(count == index)
						return i;
					count++;
				}
			}
			break;
		case OBJECT_BINARY_VALUE:
			for(i = 0;i < MAX_VARS;i++)
			{
				if((vars[i].range != 0) &&(vars[i].digital_analog == 0))
				{					
					if(count == index)
						return i;
					count++;
				}
			}
			break;
		default:
			break;
	}
		
	return -1;
}


BACNET_POLARITY Binary_Input_Polarity(
    uint32_t object_instance)
{
	uint8_t index;
	
	index = BI_Index_To_Instance[object_instance];
	
	if (index < MAX_BIS) {
			
		if(inputs[index].range >= OFF_ON && inputs[index].range <= LOW_HIGH)
			return POLARITY_NORMAL;
		else
			return POLARITY_REVERSE;	
		
	}
	
	return POLARITY_NORMAL;
}

bool Binary_Input_Polarity_Set(
    uint32_t object_instance,
    BACNET_POLARITY polarity)
{
		bool status = false;
		uint8_t index;
	
		index = BI_Instance_To_Index[object_instance];
	
		if(object_instance < MAX_BIS) 
		{
			if(polarity == POLARITY_NORMAL)
			{
				if(inputs[object_instance - OBJECT_BASE].range >= ON_OFF && inputs[object_instance - OBJECT_BASE].range <= HIGH_LOW)
				{ 
					inputs[object_instance - OBJECT_BASE].range = inputs[object_instance - OBJECT_BASE].range - 11;
					return true;
				}
			}
			else 
			{	
				if(inputs[object_instance - OBJECT_BASE].range >= OFF_ON && inputs[object_instance - OBJECT_BASE].range <= LOW_HIGH)
				{
					inputs[object_instance - OBJECT_BASE].range = inputs[object_instance - OBJECT_BASE].range + 11;
					return true;
				}
			}
		}

		return status;
}



BACNET_POLARITY Binary_Output_Polarity(
    uint32_t object_instance)
{
	uint8_t index;
	
	index = BO_Index_To_Instance[object_instance];
	
	if (index < MAX_BOS) {
			
		if(outputs[index].range >= OFF_ON && outputs[index].range <= LOW_HIGH)
			return POLARITY_NORMAL;
		else
			return POLARITY_REVERSE;	
		
	}
	return POLARITY_NORMAL;
}

bool Binary_Output_Polarity_Set(
    uint32_t object_instance,
    BACNET_POLARITY polarity)
{
		bool status = false;
		uint8_t index;
	
		index = BO_Instance_To_Index[object_instance];
	
		if(object_instance < MAX_BOS) 
		{
			if(polarity == POLARITY_NORMAL)
			{
				if(outputs[object_instance - OBJECT_BASE].range >= ON_OFF && outputs[object_instance - OBJECT_BASE].range <= HIGH_LOW)
				{ 
					outputs[object_instance - OBJECT_BASE].range = outputs[object_instance - OBJECT_BASE].range - 11;
					return true;
				}
			}
			else 
			{	
				if(outputs[object_instance - OBJECT_BASE].range >= OFF_ON && outputs[object_instance - OBJECT_BASE].range <= LOW_HIGH)
				{
					outputs[object_instance - OBJECT_BASE].range = outputs[object_instance - OBJECT_BASE].range + 11;
					return true;
				}
			}
		}

		return status;
}


BACNET_POLARITY Binary_Value_Polarity(
    uint32_t object_instance)
{
	uint8_t index;
	
	index = BV_Index_To_Instance[object_instance];
	
	if (index < MAX_BVS) {
			
		if(vars[index].range >= OFF_ON && vars[index].range <= LOW_HIGH)
			return POLARITY_NORMAL;
		else
			return POLARITY_REVERSE;	
		
	}
	return POLARITY_NORMAL;
}

bool Binary_Value_Polarity_Set(
    uint32_t object_instance,
    BACNET_POLARITY polarity)
{
		bool status = false;
		uint8_t index;
	
		index = BV_Instance_To_Index[object_instance];
	
		if(object_instance < MAX_BVS) 
		{
			if(polarity == POLARITY_NORMAL)
			{
				if(vars[object_instance - OBJECT_BASE].range >= ON_OFF && vars[object_instance - OBJECT_BASE].range <= HIGH_LOW)
				{ 
					vars[object_instance - OBJECT_BASE].range = vars[object_instance - OBJECT_BASE].range - 11;
					return true;
				}
			}
			else 
			{	
				if(vars[object_instance - OBJECT_BASE].range >= OFF_ON && vars[object_instance - OBJECT_BASE].range <= LOW_HIGH)
				{
					vars[object_instance - OBJECT_BASE].range = vars[object_instance - OBJECT_BASE].range + 11;
					return true;
				}
			}
		}

		return status;
}




extern uint8 FAN_MODE[8][9];
uint8 Get_State_Text_Len(uint8_t i)
{
#if 1//ARM_TSTAT_WIFI	
	
	char j;
	char text_len = 0;
	for(j = 0;j < STR_MSV_MULTIPLE_COUNT;j++)
	{
		if(strlen(msv_data[i][j].msv_name) > 0 && strlen(msv_data[i][j].msv_name) <= STR_MSV_NAME_LENGTH)
			text_len++;
		else
			j = STR_MSV_MULTIPLE_COUNT;
	}
	vars[20].value = text_len;
	return text_len;
	
#endif	
}

char * Get_State_Text(uint8_t i,uint8_t j)
{


		if(strlen(msv_data[i][j].msv_name) > 0 && strlen(msv_data[i][j].msv_name) <= STR_MSV_NAME_LENGTH)		
			return (char *)msv_data[i][j].msv_name;
#if ARM_TSTAT_WIFI	
		else 
			return (char *)&FAN_MODE[j];
	
#endif
}

void write_value_state(uint32_t object_instance,
    uint32_t state_index,
    char *new_name)
{
#if ARM_TSTAT_WIFI	
	if(object_instance == 0)
	{
		memcpy(&FAN_MODE[state_index],new_name,9);
	}
	
#endif

}

int32_t backup_AI_value[MAX_INS];
bool Analog_Input_Change_Of_Value(unsigned int object_instance)
{
	unsigned object_index;
	object_index = AI_Instance_To_Index[object_instance];
			
	if(inputs[object_index].value != backup_AI_value[object_index])
	{
		backup_AI_value[object_index] = inputs[object_index].value;
		return true;
	}
	else
	{
		return false;
	}
}

int32_t backup_AV_value[MAX_INS];
bool Analog_Value_Change_Of_Value(unsigned int object_instance)
{
	unsigned object_index;
	bool status = false;
	object_index = AV_Instance_To_Index[object_instance];

	if(vars[object_index].value != backup_AV_value[object_index])
	{	
		backup_AV_value[object_index] = vars[object_index].value;
		status = true;
	}
	else
	{	
		status = false;
	}
	
	return status;
}


void Store_Instance_To_Eeprom(uint32_t Instance)
{
	E2prom_Write_Byte(EEP_INSTANCE1, Instance);
	E2prom_Write_Byte(EEP_INSTANCE2, (U8_T)(Instance >> 8));
	E2prom_Write_Byte(EEP_INSTANCE3, (U8_T)(Instance >> 16));
	E2prom_Write_Byte(EEP_INSTANCE4, (U8_T)(Instance >> 24));

}

// tbd:
//void Set_Vendor_ID(uint16_t vendor_id)
//{}
//void Set_Vendor_Name(char* name)
//{}
//void Set_Vendor_Product(char* product)
//{}


#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)

extern uint8_t flag_suspend_mstp;
extern uint16_t count_suspend_mstp;

void check_whether_suspend_mstp(void)
{
	if(flag_suspend_mstp)
	{
		if(count_suspend_mstp > 0)	
			count_suspend_mstp--;
		else
			flag_suspend_mstp = 0;
	}
}



void Store_MASTER_To_Eeprom(uint8_t master)
{
	
	E2prom_Write_Byte(EEP_MAX_MASTER,master);
}

#if ARM_CM5
// 2021/02/23 CM5 编译不能通过，不得不添加ruxia函数
uint32_t get_rpm(uint8_t point)
{
	return 0;
}
char * itoa( int value, char *string, int radix )
{
	int     i, d;
	int     flag = 0;
	char    *ptr = string;

	/* This implementation only works for decimal numbers. */
	if (radix != 10)
	{
		*ptr = 0;
		return string;
	}

	if (!value)
	{
		*ptr++ = 0x30;
		*ptr = 0;
		return string;
	}

	/* if this is a negative value insert the minus sign. */
	if (value < 0)
	{
		*ptr++ = '-';

		/* Make the value positive. */
		value *= -1;
		
	}

	for (i = 10000; i > 0; i /= 10)
	{
		d = value / i;

		if (d || flag)
		{
			*ptr++ = (char)(d + 0x30);
			value -= (d * i);
			flag = 1;
		}
	}

	/* Null terminate the string. */
	*ptr = 0;

	return string;

} /* NCL_Itoa */

#endif

#if BAC_PROPRIETARY
char* Get_temcovars_string_from_buf(uint8_t number)
{
	char type,num;
	char str[20];
	if(number == 4)
	{
		memset(str,'\0',20);
		type = Setting_Info.reg.display_lcd.lcd_mod_reg.npoint.point_type;
		num = Setting_Info.reg.display_lcd.lcd_mod_reg.npoint.number;
		
		if(type == IN)
		{
			memcpy(&str,"IN",2);
			itoa(num,&str[2],10);
			return (char *)(str);
		}
		else if(type == VAR)
		{
			memcpy(&str,"VAR",3);
			itoa(num,&str[3],10);
			return (char *)(str);
		}
		else
			return "null";
	}
	else
		return "error";
}


char Write_temcovars_string_to_buf(uint8_t number,char * str)
{
	char type,num;
	if(number == 4)
	{
		if(str[0] == 'I' && str[1] == 'N')
		{
			Setting_Info.reg.display_lcd.lcd_mod_reg.npoint.point_type = IN;
			
			if((str[2] >= '0' && str[2] <= '9') && (str[3] >= '0' && str[3] <= '9'))
				num = str[3] - '0' + 10 * (str[2] - '0');
			else if(str[2] >= '0' && str[2] <= '9')
				num = str[2] - '0';
			
			if(num >= 1)
				Setting_Info.reg.display_lcd.lcd_mod_reg.npoint.number = num;
			memcpy(Modbus.display_lcd.lcddisplay,Setting_Info.reg.display_lcd.lcddisplay,sizeof(lcdconfig));
			write_page_en[25] = 1;	
			Flash_Write_Mass();
		}
		else if(str[0] == 'V' && str[1] == 'A' && str[2] == 'R')
		{
			Setting_Info.reg.display_lcd.lcd_mod_reg.npoint.point_type = VAR;
			
			if((str[3] >= '0' && str[3] <= '9') && (str[4] >= '0' && str[4] <= '9'))
				num = str[4] - '0' + 10 * (str[3] - '0');
			else if(str[3] >= '0' && str[3] <= '9')
				num = str[3] - '0';
			
			if(num >= 1)
				Setting_Info.reg.display_lcd.lcd_mod_reg.npoint.number = num;
			memcpy(Modbus.display_lcd.lcddisplay,Setting_Info.reg.display_lcd.lcddisplay,sizeof(lcdconfig));
			write_page_en[25] = 1;	
			Flash_Write_Mass();
		}
		return 1;
	}
	return 0;
}
#endif

extern uint8_t header_len;
extern uint16_t transfer_len;
#if (ARM_MINI || ASIX_MINI || ARM_CM5)

void Send_UserList_Broadcast(U8_T start,U8_T end)
{
	BACNET_PRIVATE_TRANSFER_DATA private_data = { 0 };
	BACNET_APPLICATION_DATA_VALUE data_value = { 0 };
	uint8_t far test_value[480] = { 0 };
	bool status = false;
	int private_data_len = 0;	
	BACNET_ADDRESS dest;
	uint8_t far temp[500];
	
	private_data.vendorID = BACNET_VENDOR_ID;
	private_data.serviceNumber = 1;	
	
	temp[0] = LOW_BYTE(USER_DATA_HEADER_LEN + sizeof(Password_point));
	temp[1] = HIGH_BYTE(USER_DATA_HEADER_LEN + sizeof(Password_point));
	temp[2] = WRITEUSER_T3000;
	temp[3] = start;
	temp[4] = end;
	temp[5] = LOW_BYTE(sizeof(Password_point)); 
	temp[6] = HIGH_BYTE(sizeof(Password_point));
	
	header_len = USER_DATA_HEADER_LEN;	
	transfer_len = sizeof(Password_point) * (end - start + 1);
	memcpy(&temp[7],(char *)&passwords[0],transfer_len);
	status = bacapp_parse_application_data(BACNET_APPLICATION_TAG_OCTET_STRING,(char *)&temp, &data_value);
	private_data_len = bacapp_encode_application_data(&test_value[0], &data_value);
	private_data.serviceParameters = &test_value[0];
	private_data.serviceParametersLen = private_data_len;
	
		
	Send_bip_Flag = 1;	
	count_send_bip = 0;
	Send_bip_count = MAX_RETRY_SEND_BIP;	
//	Set_broadcast_bip_address(0xffffffff);
	bip_get_broadcast_address(&dest);
	Send_ConfirmedPrivateTransfer(&dest,&private_data,BAC_IP_CLIENT);
	
}
#endif

int Send_private_scan(U8_T index)
{
	BACNET_PRIVATE_TRANSFER_DATA private_data = { 0 };
	BACNET_APPLICATION_DATA_VALUE data_value = { 0 };
	uint8_t far test_value[480] = { 0 };
	bool status = false;
	int private_data_len = 0;	
	BACNET_ADDRESS dest;
	uint8_t far temp[500];
	unsigned short max_apdu = 0;
	
	private_data.vendorID = BACNET_VENDOR_ID;
	private_data.serviceNumber = 1;	
	
	temp[0] = 0x07;
	temp[1] = 0x00;
	temp[2] = READ_BACNET_TO_MDOBUS;
	temp[3] = 0;
	temp[4] = 0; // start addr is 0
	temp[5] = 10; 
	temp[6] = 0;  // len is 10
	
	header_len = USER_DATA_HEADER_LEN;	
	transfer_len = 0;

	status = bacapp_parse_application_data(BACNET_APPLICATION_TAG_OCTET_STRING,(char *)&temp, &data_value);
	private_data_len = bacapp_encode_application_data(&test_value[0], &data_value);
	private_data.serviceParameters = &test_value[0];
	private_data.serviceParametersLen = private_data_len;
	
	if(count_hold_on_bip_to_mstp > 0)									
		return -1;
		
	flag_mstp_source = 2;   // T3-CONTROLLER
	Send_Private_Flag = 3;   // send normal bacnet packet
	TransmitPacket_panel = remote_panel_db[index].sub_id;
	status = address_get_by_device(remote_panel_db[index].device_id, &max_apdu, &dest);
	
	if(status)
	{
		if((TransmitPacket_panel < 255) && (TransmitPacket_panel > 0))
		{
			invokeid_mstp = Send_ConfirmedPrivateTransfer(&dest,&private_data,BAC_MSTP);
		}	
	}

	return invokeid_mstp;
}

#endif


