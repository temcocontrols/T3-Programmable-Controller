#include "product.h"


#include "flash.h"
#include "flash_user.h"
#include <string.h>	 
#include "define.h"
//#include "weather.h"

//STR_FLASH flash;

STR_Flash_POS xdata Flash_Position[23];
STR_flag_flash 	far bac_flash;


U8_T far tempbuf[8000] = {0};
 
/* caclulate detailed position for every table */
void Flash_Inital(void)
{
	U8_T loop;
	U8_T loop1;
	U16_T baseAddr = 0;	 	
	U16_T  len = 0;
  memset(tempbuf,0,8000);
	for(loop = 0;loop < MAX_POINT_TYPE;loop++)
	{  		
		switch(loop)
		{	
	  	case OUT:	
			baseAddr = 0;
			len = sizeof(Str_out_point) * MAX_OUTS;
			break;
		case IN:
			baseAddr += len;
			len = sizeof(Str_in_point) * MAX_INS;
			break;
		case VAR:
			baseAddr += len;
			len = sizeof(Str_variable_point) * MAX_VARS;
			break;
		case CON:
			baseAddr += len;
			len = sizeof(Str_controller_point) * MAX_CONS;
			break;
		case WRT: 
			baseAddr += len;
			len = sizeof(Str_weekly_routine_point) * MAX_WR;
			break;
		case AR:
			baseAddr += len;
			len = sizeof(Str_annual_routine_point) * MAX_AR;
			break;
		case PRG:
			baseAddr += len;
			len = sizeof(Str_program_point) * MAX_PRGS;
			break;
	/*	case TBL:
			baseAddr += len;
			len = sizeof(Tbl_point) * MAX_TBLS * 16;  
			break;
		case TZ:
			baseAddr += len;
			len = sizeof(Str_totalizer_point) * MAX_TOTALIZERS;
			break;*/
		case AMON:
			baseAddr += len;
			len = sizeof(Str_monitor_point) * MAX_MONITORS;
			break; 
		case GRP:
			baseAddr += len;
			len = sizeof(Control_group_point) * MAX_GRPS;				
			break;
	/*	case ARRAY:
			baseAddr += len;
			len = sizeof(Str_array_point) * MAX_ARRAYS;
			break;	*/
		case ALARMM:  
			baseAddr += len;
			len = sizeof(Alarm_point) * MAX_ALARMS;
			break;
		case ALARM_SET: 
			baseAddr += len;
			len = sizeof(Alarm_set_point) * MAX_ALARMS_SET;  
			break;
		case UNIT:
			baseAddr += len;
			len = sizeof(Units_element) * MAX_DIG_UNIT;
			break;
		case USER_NAME:
			baseAddr += len;
			len = sizeof(Password_point) * MAX_PASSW;
			break; 
		case WR_TIME:
			baseAddr += len; 
			len = sizeof(Wr_one_day) * MAX_WR * MAX_SCHEDULES_PER_WEEK;
			break;
		case AR_DATA:
			baseAddr += len; 
			len = sizeof(S8_T) * MAX_AR * AR_DATES_SIZE; 
			break;
		case TSTAT:
			baseAddr += len;
			len = sizeof(SCAN_DB) * SUB_NO;
			break;
		case GRP_POINT:			
			baseAddr += len;
			len = sizeof(Str_grp_element) * 240;
			break;
		case TBL:
			baseAddr += len;
			len = sizeof(Str_table_point) * MAX_TBLS ;  
			break;
		default:
			break; 
		}
		for(loop1 = 0;loop1 < MAX_PRGS;loop1++)
			programs[loop1].real_byte = 0;		
			
		Flash_Position[loop].addr = baseAddr;
		Flash_Position[loop].len = len;	
	}
		
}

void Flash_Write_Mass(void)
{
	STR_flag_flash ptr_flash;
	U16_T base_addr;
	U8_T loop;
	U16_T loop1,loop2;

 /* only the first block, erase memory */
	
	IntFlashErase(ERA_RUN,0x70000);	
	
//	IntFlashWriteByte(0x7fff0,0x55);
	// MassFlashWrite(0,Para,400); //LHN add
	ptr_flash.index = 0;

	for(loop = 0;loop < MAX_POINT_TYPE ;loop++)
	{
		ptr_flash.table = loop;	
		
		ptr_flash.len = Flash_Position[loop].len;
		base_addr = Flash_Position[loop].addr;
		switch(loop)
		{	
		case OUT: 
			for(loop1 = 0;loop1 < MAX_OUTS;loop1++)
			{
				memcpy(&tempbuf[sizeof(Str_out_point) * loop1],&outputs[loop1],sizeof(Str_out_point));					
			}
			for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
				IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);
			
			break;
		case IN:
			for(loop1 = 0;loop1 < MAX_INS;loop1++)
			{
				memcpy(&tempbuf[sizeof(Str_in_point) * loop1],&inputs[loop1],sizeof(Str_in_point));					
			}
			for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
				IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);

			break;
		case VAR:  
			for(loop1 = 0;loop1 < MAX_VARS;loop1++)
			{
				memcpy(&tempbuf[sizeof(Str_variable_point) * loop1],&vars[loop1],sizeof(Str_variable_point));					
			}
			for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
				IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);							
			break;
		case CON:
			for(loop1 = 0;loop1 < MAX_CONS;loop1++)
			{
				memcpy(&tempbuf[sizeof(Str_controller_point) * loop1],&controllers[loop1],sizeof(Str_controller_point));					
			}
			for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
				IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);	
			break;
		case WRT:  				
			for(loop1 = 0;loop1 < MAX_WR;loop1++)
			{
				memcpy(&tempbuf[sizeof(Str_weekly_routine_point) * loop1],&weekly_routines[loop1],sizeof(Str_weekly_routine_point));					
			}
			for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
				IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);	
						
			break;
		case AR: 
			for(loop1 = 0;loop1 < MAX_AR;loop1++)
			{
				memcpy(&tempbuf[sizeof(Str_annual_routine_point) * loop1],&annual_routines[loop1],sizeof(Str_annual_routine_point));					
			}
			for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
				IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);	
					
			break;
		case PRG:  
			for(loop1 = 0;loop1 < MAX_PRGS;loop1++)
			{
				memcpy(&tempbuf[sizeof(Str_program_point) * loop1],&programs[loop1],sizeof(Str_program_point));					
			}
			for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
				IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);	 
			break;
		case TBL:
			for(loop1 = 0;loop1 < MAX_TBLS;loop1++)
			{
				memcpy(&tempbuf[sizeof(Str_table_point) * loop1],&custom_tab[loop1],sizeof(Str_table_point));					
			}
			for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
				IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);	
			break;
	/*	case TZ:
			for(loop1 = 0;loop1 < MAX_TOTALIZERS;loop1++)
			{
				memcpy(&tempbuf[sizeof(Str_totalizer_point) * loop1],&totalizers[loop1],sizeof(Str_totalizer_point));					
			}
			for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
				IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);	
			break;	*/
		case AMON:
			for(loop1 = 0;loop1 < MAX_MONITORS;loop1++)
			{
				memcpy(&tempbuf[sizeof(Str_monitor_point) * loop1],&monitors[loop1],sizeof(Str_monitor_point));					
			}
			for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
				IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);	
			break;	
	case GRP: 
			for(loop1 = 0;loop1 < MAX_GRPS;loop1++)
			{
				memcpy(&tempbuf[sizeof(Control_group_point) * loop1],&control_groups[loop1],sizeof(Control_group_point));					
			}
			for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
				IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);	
			break;
/*			case ARRAY:
			for(loop1 = 0;loop1 < MAX_ARRAYS;loop1++)
			{
				memcpy(&tempbuf[sizeof(Str_array_point) * loop1],&arrays[loop1],sizeof(Str_array_point));					
			}
			for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
				IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);	
			break; */
		case ALARMM: 
//			for(loop1 = 0;loop1 < MAX_ALARMS;loop1++)
//			{
//				memcpy(&tempbuf[sizeof(Alarm_point) * loop1],&alarms[loop1],sizeof(Alarm_point));					
//			}
//			for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
//				IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);	
			break; 			
		case ALARM_SET:  
			for(loop1 = 0;loop1 < MAX_ALARMS_SET;loop1++)
			{
				memcpy(&tempbuf[sizeof(Alarm_set_point) * loop1],&alarms_set[loop1],sizeof(Alarm_set_point));					
			}
			for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
				IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);	
			break; 

		case UNIT:
			for(loop1 = 0;loop1 < MAX_DIG_UNIT;loop1++)
			{
				memcpy(&tempbuf[sizeof(Units_element) * loop1],&digi_units[loop1],sizeof(Units_element));					
			}
			for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
				IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);	
			break; 
		case USER_NAME:
			for(loop1 = 0;loop1 < MAX_PASSW;loop1++)
			{
				memcpy(&tempbuf[sizeof(Password_point) * loop1],&passwords[loop1],sizeof(Password_point));					
			}
			for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
				IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);
			break;		
		case WR_TIME: 	
			for(loop1 = 0;loop1 < MAX_WR;loop1++)
			{
				memcpy(&tempbuf[sizeof(Wr_one_day) * 9 * loop1],&wr_times[loop1],sizeof(Wr_one_day) * 9);					
			}
			for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
				IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);	
			break;
		case AR_DATA:  
			for(loop1 = 0;loop1 < MAX_AR;loop1++)
			{
				memcpy(&tempbuf[46 * sizeof(S8_T) * loop1],&ar_dates[loop1],46 * sizeof(S8_T));					
			}
			for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
				IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);	
			break; 			
	   	case TSTAT:
			for(loop1 = 0;loop1 < SUB_NO;loop1++)
			{
				memcpy(&tempbuf[sizeof(SCAN_DB) * loop1],&scan_db[loop1],sizeof(SCAN_DB));					
			}
			for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
				IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);	
			break;
		case GRP_POINT:	
			for(loop1 = 0;loop1 < 80;loop1++)
			{
				memcpy(&tempbuf[sizeof(Str_grp_element) * loop1],&group_data[loop1],sizeof(Str_grp_element));					
			}
			for(loop2 = 0;loop2 < 80 * sizeof(Str_grp_element);loop2++)
				IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);	
			
			for(loop1 = 0;loop1 < 80;loop1++)
			{
				memcpy(&tempbuf[sizeof(Str_grp_element) * loop1],&group_data[loop1 + 80],sizeof(Str_grp_element));					
			}
			for(loop2 = 0;loop2 < 80 * sizeof(Str_grp_element);loop2++)
				IntFlashWriteByte(0x70000 + base_addr + loop2 + 80 * sizeof(Str_grp_element),tempbuf[loop2]);
			
			for(loop1 = 0;loop1 < 80;loop1++)
			{
				memcpy(&tempbuf[sizeof(Str_grp_element) * loop1],&group_data[loop1 + 160],sizeof(Str_grp_element));					
			}
			for(loop2 = 0;loop2 < 80 * sizeof(Str_grp_element);loop2++)
				IntFlashWriteByte(0x70000 + base_addr + loop2 + 160 * sizeof(Str_grp_element),tempbuf[loop2]);
			break;	
		default:	
			break;
	
		} 			
	}
	
			
	Flash_Store_Code();
	Flash_Write_Other();
  IntFlashWriteByte(0x7fff0,0x55);

}

void Flash_Read_Mass(void)
{
	STR_flag_flash ptr_flash;
	U16_T base_addr;
	U8_T loop;
	U16_T loop2/*,loop1*/;
	U8_T T_END = 0;

	ptr_flash.index = 0;

	for(loop = 0;loop < MAX_POINT_TYPE;loop++)
	{
		ptr_flash.table = loop;	
		
		ptr_flash.len = Flash_Position[loop].len;
		base_addr = Flash_Position[loop].addr;
		// GRP_POINT start addr 36918 , len 525

			switch(loop)
			{
			case OUT:	 				
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				memcpy(outputs , tempbuf, ptr_flash.len); 
				break;
			case IN:   	
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				memcpy(inputs ,tempbuf, ptr_flash.len);
				
				break;
			case VAR:   	
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				memcpy(vars ,tempbuf, ptr_flash.len);
				break;
			case CON:
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				memcpy(controllers ,tempbuf, ptr_flash.len);
				break;
			case WRT: 
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				memcpy(weekly_routines ,tempbuf, ptr_flash.len);
				break;
			case AR:   	
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				memcpy(annual_routines ,tempbuf, ptr_flash.len);
				break;
			case PRG:
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				memcpy(programs ,tempbuf, ptr_flash.len);
				break;
			case TBL:
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				memcpy(custom_tab ,tempbuf, ptr_flash.len);
				break;
			/*case TZ:
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				memcpy(totalizers ,tempbuf, ptr_flash.len);
				break;*/
			case AMON:	 
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				memcpy(monitors ,tempbuf, ptr_flash.len);
				break; 
			case GRP:	
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				memcpy(control_groups ,tempbuf, ptr_flash.len);
				break;
		/*	case ARRAY:
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				memcpy(arrays ,tempbuf, ptr_flash.len);
				break;	*/
//			case ALARMM:  
//				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
//					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
//				memcpy(alarms ,tempbuf, ptr_flash.len);
//				break;
			case ALARM_SET:  
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				memcpy(alarms_set ,tempbuf, ptr_flash.len);
				break;
			case UNIT:
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				memcpy(digi_units ,tempbuf, ptr_flash.len);
				break;
			case USER_NAME:
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				memcpy(passwords ,tempbuf, ptr_flash.len);
				break; 	
			case WR_TIME:  	
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
				{	
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				}
				memcpy(wr_times ,tempbuf, ptr_flash.len);
				break; 
			case AR_DATA: 
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
				{		
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				}
				memcpy(ar_dates ,tempbuf, ptr_flash.len);
				break;
			case TSTAT:
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
				{		
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				}
				memcpy(scan_db ,tempbuf, ptr_flash.len);
				break;
			case GRP_POINT: ;
				for(loop2 = 0;loop2 < sizeof(Str_grp_element) * 80;loop2++)
				{		
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				}
				memcpy(group_data ,tempbuf, sizeof(Str_grp_element) * 80);
				for(loop2 = 0;loop2 < sizeof(Str_grp_element) * 80;loop2++)
				{		
					IntFlashReadByte(0x70000 + base_addr + loop2 + sizeof(Str_grp_element) * 80,&tempbuf[loop2]);
				}
				memcpy(&group_data[80] ,tempbuf,sizeof(Str_grp_element) * 80);
				for(loop2 = 0;loop2 < sizeof(Str_grp_element) * 80;loop2++)
				{		
					IntFlashReadByte(0x70000 + base_addr + loop2 + sizeof(Str_grp_element) * 160,&tempbuf[loop2]);
				}
				memcpy(&group_data[160] ,tempbuf,sizeof(Str_grp_element) * 80);
				break; 
			default:
				break;

			}
	
	} 

	Flash_Read_Code();
	Flash_Read_Other();

}

void Flash_Store_Code(void)
{
	U8_T i;
//	U16_T temp = 0;
	U32_T base_addr = 0;
	U16_T loop;

	for(i = 0;i < MAX_PRGS;i++)
	{	
//		Test[10 + i] = swap_word(programs[i].real_byte);
		if(swap_word(programs[i].real_byte) > 0 && swap_word(programs[i].real_byte) <= CODE_ELEMENT * MAX_CODE)	
		{
			IntFlashWriteInt(BASE_CODE_INDEX + i * 2,swap_word(programs[i].real_byte));
			
			for(loop = 0;loop < swap_word(programs[i].real_byte) + USER_DATA_HEADER_LEN;loop++)
			{
				if(base_addr + loop < MAX_CODE_SIZE/*CODE_ELEMENT * MAX_PRGS*/)
				{
					IntFlashWriteByte(BASE_CODE + base_addr + loop,prg_code[i][loop]);
				}
			}
			base_addr += (swap_word(programs[i].real_byte) + USER_DATA_HEADER_LEN);
			
		}
		else
		{
			IntFlashWriteInt(BASE_CODE_INDEX + i * 2,0);	
		}
	}

	//IntFlashWriteInt(BASE_CODE_LEN,Code_total_length);

}






void Flash_Write_Other(void)
{
	U16_T loop;
	U16_T loop1,loop2;
 // gsm
#if USB_HOST
	if(Modbus.usb_mode == 1)
	{
		IntFlashWriteByte(BASE_GSM_APN,apnlen);
		if(apnlen > MAX_GSM_APN)	apnlen = MAX_GSM_APN;
		for(loop = 0;loop < apnlen;loop++)
			IntFlashWriteByte(BASE_GSM_APN + 1 + loop,apnstr[loop]);
	
		IntFlashWriteByte(BASE_GSM_IP,iplen);
		if(iplen > MAX_GSM_IP)	iplen = MAX_GSM_IP;
		for(loop = 0;loop < iplen;loop++)
			IntFlashWriteByte(BASE_GSM_IP + 1 + loop,ipstr[loop]);
	}
#endif
 // name
 	for(loop = 0;loop < 20;loop++)
	{
		IntFlashWriteByte(BASE_PANEL_NAME + loop,panelname[loop]);
	}
	

	for(loop = 0;loop < MAX_DOMAIN_SIZE;loop++)
	{
		IntFlashWriteByte(BASE_DYNDNS_DONAME + loop,dyndns_domain_name[loop]);
	}

	for(loop = 0;loop < MAX_USERNAME_SIZE;loop++)
	{
		IntFlashWriteByte(BASE_DYNDNS_USER + loop,dyndns_username[loop]);
	}

	for(loop = 0;loop < MAX_PASSWORD_SIZE;loop++)
	{
		IntFlashWriteByte(BASE_DYNDNS_PASS + loop,dyndns_password[loop]);
	}	

	// store name of tstat
	for(loop = 0;loop < MAX_ID;loop++)
	{
		for(loop1 = 0;loop1 < 16/*TSTAT_NAME*/;loop1++)  // 
			IntFlashWriteByte(BASE_TST_NAME + 16 * loop + loop1,tstat_name[loop][loop1]);
	}	
	
	// store time of operating monitor
	for(loop = 0;loop < MAX_MONITORS;loop++)
	{
		for(loop1 = 0;loop1 < 4;loop1++)   
			IntFlashWriteByte(BASE_MON_OPERATE_TIME + 4 * loop + loop1,MISC_Info.reg.operate_time[loop][loop1]);
	}
		
	for(loop1 = 0;loop1 < 30;loop1++)   
			IntFlashWriteByte(BASE_SNTP_SERVER + loop1,sntp_server[loop1]);
	
	for(loop = 0;loop < MAX_VAR_UNIT;loop++)   
		for(loop1 = 0;loop1 < VAR_UNIT_SIZE;loop1++)   
			IntFlashWriteByte(BASE_VAR_UINT + loop * VAR_UNIT_SIZE + loop1,var_unit[loop][loop1]);
	// store weather
//	{
//		char *ptr = &weather;
//	for(loop1 = 0;loop1 < sizeof(STR_WEATHER);loop1++)   
//			IntFlashWriteByte(BASE_WEATHER + loop1,ptr[loop1]);
//	}
	
	for(loop = 0;loop < MAX_WR;loop++)  
		for(loop1 = 0;loop1 < MAX_SCHEDULES_PER_WEEK;loop1++) 
			for(loop2 = 0;loop2 < 8;loop2++) 	
				IntFlashWriteByte(BASE_WR_ON_OFF + loop * 72 + loop1 * 8 + loop2,wr_time_on_off[loop][loop1][loop2]);

}
void Flash_Read_other(void)
{
	U16_T loop;
	U16_T loop1,loop2;
// gsm
#if USB_HOST
	if(Modbus.usb_mode == 1)
	{
		IntFlashReadByte(BASE_GSM_APN ,&apnlen); 
		if(apnlen > MAX_GSM_APN)	apnlen = MAX_GSM_APN;
		for(loop = 0;loop < MAX_GSM_APN;loop++)
			IntFlashReadByte(BASE_GSM_APN + 1 + loop,&apnstr[loop]);

		IntFlashReadByte(BASE_GSM_IP ,&iplen); 
		if(iplen > MAX_GSM_IP)	iplen = MAX_GSM_IP;
		for(loop = 0;loop < MAX_GSM_IP;loop++)
			IntFlashReadByte(BASE_GSM_IP + 1 + loop,&ipstr[loop]);
	}
#endif
// name
 	for(loop = 0;loop < 20;loop++)
	{
		IntFlashReadByte(BASE_PANEL_NAME + loop,&panelname[loop]);
	}


	for(loop = 0;loop < MAX_DOMAIN_SIZE;loop++)
	{
		IntFlashReadByte(BASE_DYNDNS_DONAME + loop,&dyndns_domain_name[loop]);
	}
	for(loop = 0;loop < MAX_USERNAME_SIZE;loop++)
	{
		IntFlashReadByte(BASE_DYNDNS_USER + loop,&dyndns_username[loop]);
	}
	for(loop = 0;loop < MAX_PASSWORD_SIZE;loop++)
	{
		IntFlashReadByte(BASE_DYNDNS_PASS + loop,&dyndns_password[loop]);
	}	


	// read name of tstat
	for(loop = 0;loop < MAX_ID;loop++)
	{
		for(loop1 = 0;loop1 < 16/*TSTAT_NAME*/;loop1++)  // 
			IntFlashReadByte(BASE_TST_NAME + 16 * loop + loop1,&tstat_name[loop][loop1]);
	}
	// read time of operating monitor
	for(loop = 0;loop < MAX_MONITORS;loop++)
	{
		for(loop1 = 0;loop1 < 4;loop1++)   
			IntFlashReadByte(BASE_MON_OPERATE_TIME + 4 * loop + loop1,&MISC_Info.reg.operate_time[loop][loop1]);
	}
	
	
	for(loop1 = 0;loop1 < 30;loop1++)   
			IntFlashReadByte(BASE_SNTP_SERVER + loop1,&sntp_server[loop1]);
	
	for(loop = 0;loop < MAX_VAR_UNIT;loop++)   
		for(loop1 = 0;loop1 < VAR_UNIT_SIZE;loop1++)   
			IntFlashReadByte(BASE_VAR_UINT + loop * VAR_UNIT_SIZE + loop1,&var_unit[loop][loop1]);
	// read weather
//	{
//		char *ptr = &weather;
//		for(loop1 = 0;loop1 < sizeof(STR_WEATHER);loop1++)   
//			IntFlashReadByte(BASE_WEATHER + loop1,&ptr[loop1]);
//	}
	
	for(loop = 0;loop < MAX_WR;loop++)  
		for(loop1 = 0;loop1 < MAX_SCHEDULES_PER_WEEK;loop1++) 
			for(loop2 = 0;loop2 < 8;loop2++) 	
				IntFlashReadByte(BASE_WR_ON_OFF + loop * 72 + loop1 * 8 + loop2,&wr_time_on_off[loop][loop1][loop2]);

}


void Flash_Read_Code(void)
{
	U8_T i;
	U16_T loop;
	U16_T temp = 0;
	U32_T base_addr = 0;
	Code_total_length = 0;
	for(i = 0;i < MAX_PRGS;i++)
	{	
		IntFlashReadInt(BASE_CODE_INDEX + i * 2, &temp);
		if(temp > CODE_ELEMENT * MAX_CODE)
			temp = 0;
		programs[i].real_byte = swap_word(temp); 
		Code_total_length += swap_word(programs[i].real_byte);
		if(swap_word(programs[i].real_byte) > 0 && swap_word(programs[i].real_byte) <= CODE_ELEMENT * MAX_CODE)	
		{
			for(loop = 0;loop < swap_word(programs[i].real_byte) + USER_DATA_HEADER_LEN;loop++)
			{
				if(base_addr + loop < MAX_CODE_SIZE /*CODE_ELEMENT * MAX_PRGS*/)
					IntFlashReadByte(BASE_CODE + base_addr + loop,&prg_code[i][loop]);
			}
			base_addr += (swap_word(programs[i].real_byte) + USER_DATA_HEADER_LEN);
			//return;

		}
		else
			memset(&prg_code[i] ,0, CODE_ELEMENT * MAX_CODE);
	}
	//IntFlashReadInt(BASE_CODE_LEN, &Code_total_length);
	//if(Code_total_length == 0xffff)		Code_total_length = 0;

	
}



