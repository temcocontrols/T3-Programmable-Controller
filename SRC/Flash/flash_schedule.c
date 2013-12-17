#include "flash.h"
#include "flash_schedule.h"
#include "schedule.h"
#include <string.h>	
#include "user_data.h" 
#include "define.h"

STR_FLASH flash;

static STR_Flash_POS xdata Flash_Position[18] _at_ 0x800;
STR_flag_flash 	far bac_flash;

void Lcd_Show_Data(char pos_x,char pos_y,unsigned int number,char dot,char mode);

extern U16_T far Test[50];
static	U8_T xdata tempbuf[5000] = {0};


//extern U8_T  far Para[400]; 

/* caclulate detailed position for every table */
void Flash_Inital(void)
{
	U8_T loop;
	U8_T loop1;
	U16_T baseAddr = 0;	 	
	U16_T  len = 0;

	U8_T END = 0;
	if(protocal <= TCP_IP)	END = T_END_OLD;
	else 	
		END = 18; // BACNET TABLE
	for(loop = 0;loop < END;loop++)
	{
		if(protocal <= TCP_IP)
		{
			switch(loop) 
			{
		   	case T_WEEK_DES:	
				baseAddr = 0;
				len = WR_DESCRIPTION_SIZE * MAX_WR1; // 31*20
				break;
			case T_WEEK_ONTIME:
				baseAddr += len;
				len = WR_TIME_SIZE * MAX_WR1; // 72*20
				break;
			case T_WEEK_OFFTIME:
				baseAddr += len;
				len = WR_TIME_SIZE * MAX_WR1; // 72*20
				break;
			case T_ANNUAL_DES:
				baseAddr += len;
				len = AR_DESCRIPTION_SIZE * MAX_AR1; // 29*16
				break;
			case T_ANNUAL_TIME:
				baseAddr += len;
				len = AR_TIME_SIZE * MAX_AR1;  // 46*16
				break;
			case T_ID:
				baseAddr += len;
				len = ID_SIZE * MAX_ID; // 3*254  	
				break;			
			case T_NAME:  // FOR CM5
				baseAddr += len;
				len = NAME_SIZE * MAX_NAME;
//			case T_INPUT_RANGE:
//				baseAddr += len;
//				len = 1 * MAX_INPUT;
//			case T_INPUT_FILTER:
//				baseAddr += len;
//				len = 1 * MAX_INPUT;
//			case T_INPUT_CAL:
//				baseAddr += len;
//				len = 2 * MAX_INPUT;	 
//			#endif	
			default:
				break;
			}
		}
		else  //		#if (defined(BACDL_BIP) || defined(BACDL_MSTP))
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
		/*	case CON:
				baseAddr += len;
				len = sizeof(Str_controller_point) * MAX_CONS;
				break;*/
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
				break;
			case AMON:
				baseAddr += len;
				len = sizeof(Str_monitor_point) * MAX_MONITORS;
				break;
			case GRP:
				baseAddr += len;
				len = sizeof(Control_group_point) * MAX_GRPS;				
				break;
			case ARRAY:
				baseAddr += len;
				len = sizeof(Str_array_point) * MAX_ARRAYS;
				break;
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
				len = sizeof(Units_element) * MAX_UNITS;
				break;
			case USER_NAME:
				baseAddr += len;
				len = sizeof(Password_struct);
				break; */
			case WR_TIME:
				baseAddr += len; 
				len = sizeof(Wr_one_day) * MAX_WR * MAX_SCHEDULES_PER_WEEK;
				break;
			case AR_DATA:
				baseAddr += len; 
				len = sizeof(S8_T) * MAX_AR * AR_DATES_SIZE; 
				break;
			default:
				break; 
			}
			for(loop1 = 0;loop1 < MAX_PRGS;loop1++)
				programs[loop1].bytes = 0;

		}
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
	U8_T T_END = 0;

	if(protocal <= TCP_IP)	T_END = T_END_OLD;
	else 	
		T_END = 18; // BACNET TABLE
//	U16_T i;
 /* only the first block, erase memory */
	IntFlashErase(ERA_RUN,0x70000);	
//	IntFlashWriteByte(0x70000 + 0xfff0,0x55);
	// MassFlashWrite(0,Para,400); //LHN add
	ptr_flash.index = 0;

	for(loop = 0;loop < T_END ;loop++)
	{
		ptr_flash.table = loop;	
		ptr_flash.len = Flash_Position[loop].len;
		base_addr = Flash_Position[loop].addr;
		if(protocal <= TCP_IP)
		{
			switch(loop) {
			case T_WEEK_DES:
				for(loop1 = 0;loop1 < MAX_WR1;loop1++)
				{
					memcpy(&tempbuf[WR_DESCRIPTION_SIZE * loop1],WR_Roution[loop1].UN.all,WR_DESCRIPTION_SIZE);					
				}
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);	
				break;
			case T_WEEK_ONTIME:
				for(loop1 = 0;loop1 < MAX_WR1;loop1++)
				{
					memcpy(&tempbuf[WR_TIME_SIZE * loop1],WR_Roution[loop1].OnTime,WR_TIME_SIZE);					
				}
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);
				break;
			case T_WEEK_OFFTIME:
				for(loop1 = 0;loop1 < MAX_WR1;loop1++)
				{
					memcpy(&tempbuf[WR_TIME_SIZE * loop1],WR_Roution[loop1].OffTime,WR_TIME_SIZE);					
				}
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);		
				break;
			case T_ANNUAL_DES:
				for(loop1 = 0;loop1 < MAX_AR1;loop1++)
				{
					memcpy(&tempbuf[AR_DESCRIPTION_SIZE * loop1],AR_Roution[loop1].UN.all,AR_DESCRIPTION_SIZE);					
				}
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);		
				break;
			case T_ANNUAL_TIME:
				for(loop1 = 0;loop1 < MAX_AR1;loop1++)
				{
					memcpy(&tempbuf[AR_TIME_SIZE * loop1],AR_Roution[loop1].Time,AR_TIME_SIZE);					
				}
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);		
				break;

			case T_ID:
				for(loop1 = 0;loop1 < MAX_ID;loop1++)
				{
					memcpy(&tempbuf[ID_SIZE * loop1],ID_Config[loop1].all,ID_SIZE);					
				}
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);		
				break;
				 
			case T_NAME: 
				for(loop1 = 0;loop1 < MAX_NAME;loop1++)
				{
					memcpy(&tempbuf[NAME_SIZE * loop1],menu_name[loop1],NAME_SIZE);					
				}
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);		
				break;
//			case T_INPUT_RANGE:
//				
//				memcpy(tempbuf,Modbus.Input_Range,32);	
//								
//				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
//					IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);		
//				break;
//			case T_INPUT_FILTER:
//				
//				memcpy(tempbuf,Modbus.Input_Filter,32);	
//								
//				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
//					IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);		
//				break;
//			case T_INPUT_CAL:
//				
//				memcpy(tempbuf,Modbus.Input_CAL,64);								
//				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
//					IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);		
//				break;
			default:
				break;
			}

		}
		else
		{
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
		/*	case CON:
				for(loop1 = 0;loop1 < MAX_CONS;loop1++)
				{
					memcpy(&tempbuf[sizeof(Str_controller_point) * loop1],&controllers[loop1],sizeof(Str_controller_point));					
				}
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);	
				break;*/
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
					IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);	 Test[7]++;
				break;
		/*	case TBL:
				for(loop1 = 0;loop1 < MAX_TBLS;loop1++)
				{
					memcpy(&tempbuf[sizeof(custom_tab) * 16 * loop1],&custom_tab[loop1],sizeof(custom_tab) * 16);					
				}
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);	
				break;
			case TZ:
				for(loop1 = 0;loop1 < MAX_TOTALIZERS;loop1++)
				{
					memcpy(&tempbuf[sizeof(Str_totalizer_point) * loop1],&totalizers[loop1],sizeof(Str_totalizer_point));					
				}
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);	
				break;	
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
			case ARRAY:
				for(loop1 = 0;loop1 < MAX_ARRAYS;loop1++)
				{
					memcpy(&tempbuf[sizeof(Str_array_point) * loop1],&arrays[loop1],sizeof(Str_array_point));					
				}
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);	
				break;
			case ALARMM: 
				for(loop1 = 0;loop1 < MAX_ALARMS;loop1++)
				{
					memcpy(&tempbuf[sizeof(Alarm_point) * loop1],&alarms[loop1],sizeof(Alarm_point));					
				}
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);	
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
				for(loop1 = 0;loop1 < MAX_UNITS;loop1++)
				{
					memcpy(&tempbuf[sizeof(Units_element) * loop1],&units[loop1],sizeof(Units_element));					
				}
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);	
				break; 
			case USER_NAME:
				memcpy(tempbuf,passwords,sizeof(Password_struct));					
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashWriteByte(0x70000 + base_addr + loop2,tempbuf[loop2]);						
				break;		*/
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

	
			default:	
				break;
		
			}
			
		}
	}
	Flash_Store_Code();

   	IntFlashWriteByte(0x7fff0,0x55); 
	Test[13] = 66; 
}

void Flash_Read_Mass(void)
{
	STR_flag_flash ptr_flash;
	U16_T base_addr;
	U8_T loop;
	U16_T loop2,loop1;
	U8_T T_END = 0;

	if(protocal <= TCP_IP)	T_END = T_END_OLD;
	else 	
		T_END = 18; // BACNET TABLE
//	U8_T far tempbuf[500];

	
	ptr_flash.index = 0;

	for(loop = 0;loop < T_END;loop++)
	{
		ptr_flash.table = loop;	
		
		ptr_flash.len = Flash_Position[loop].len;
		base_addr = Flash_Position[loop].addr;

		if(protocal <= TCP_IP)
		{
			switch(loop)
			{
			case T_WEEK_DES:
				
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				for(loop1 = 0;loop1 < MAX_WR1;loop1++)
				{
					memcpy(WR_Roution[loop1].UN.all,&tempbuf[WR_DESCRIPTION_SIZE * loop1],WR_DESCRIPTION_SIZE);					
				}
				break;
			case T_WEEK_ONTIME:
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				
				for(loop1 = 0;loop1 < MAX_WR1;loop1++)
				{
					memcpy(&WR_Roution[loop1].OnTime,&tempbuf[WR_TIME_SIZE * loop1],WR_TIME_SIZE);					
				}
				break;
			case T_WEEK_OFFTIME:
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				for(loop1 = 0;loop1 < MAX_WR1;loop1++)
				{
					memcpy(WR_Roution[loop1].OffTime,&tempbuf[WR_TIME_SIZE * loop1],WR_TIME_SIZE);					
				}
				break;
			case T_ANNUAL_DES:
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				for(loop1 = 0;loop1 < MAX_AR1;loop1++)
				{
					memcpy(AR_Roution[loop1].UN.all,&tempbuf[AR_DESCRIPTION_SIZE * loop1],AR_DESCRIPTION_SIZE);					
				}
				break;
			case T_ANNUAL_TIME:
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				for(loop1 = 0;loop1 < MAX_AR1;loop1++)
				{
					memcpy(AR_Roution[loop1].Time,&tempbuf[AR_TIME_SIZE * loop1],AR_TIME_SIZE);					
				}
				break;
			case T_ID:
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				for(loop1 = 0;loop1 < MAX_ID;loop1++)
				{
					memcpy(ID_Config[loop1].all,&tempbuf[ID_SIZE * loop1],ID_SIZE);					
				}
				break;			
			case T_NAME:
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				for(loop1 = 0;loop1 < MAX_NAME;loop1++)
				{
					memcpy(menu_name[loop1],&tempbuf[NAME_SIZE * loop1],NAME_SIZE);					
				}  
				break;
//			case T_INPUT_RANGE:
//				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
//					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
//
//				memcpy(Modbus.Input_Range,tempbuf,32);	
//								
//				break;
//			case T_INPUT_FILTER:
//				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
//					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
//
//				memcpy(Modbus.Input_Filter,tempbuf,32);	
//								
//				break;
//			case T_INPUT_CAL:
//				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
//					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
//
//				memcpy(Modbus.Input_CAL,tempbuf,64);	
//								
				break;
			default:
				break;
			}

		}
		else
		{
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
		/*	case CON:
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				memcpy(controllers ,tempbuf, ptr_flash.len);
				break;*/
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
		/*	case TBL:
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				memcpy(custom_tab ,tempbuf, ptr_flash.len);
				break;
			case TZ:
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				memcpy(totalizers ,tempbuf, ptr_flash.len);
				break;
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
			case ARRAY:
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				memcpy(arrays ,tempbuf, ptr_flash.len);
				break;
			case ALARMM:  
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				memcpy(alarms ,tempbuf, ptr_flash.len);
				break;
			case ALARM_SET:  
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				memcpy(alarms_set ,tempbuf, ptr_flash.len); 
				break;
			case UNIT:
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				memcpy(units ,tempbuf, ptr_flash.len);
				break;
			case USER_NAME:
				for(loop2 = 0;loop2 < ptr_flash.len;loop2++)
					IntFlashReadByte(0x70000 + base_addr + loop2,&tempbuf[loop2]);
				memcpy(passwords ,tempbuf, ptr_flash.len);
				break; */
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
					 
			default:
				break;

			}

		}	
	
	} 	
	Flash_Read_Code();	

}

void Flash_Store_Code(void)
{
	U8_T i;
	U16_T temp = 0;
	U32_T base_addr = 0;
	U16_T loop;

	for(i = 0;i < MAX_PRGS;i++)
	{	
		if(mGetPointWord2(programs[i].bytes) > 0 && mGetPointWord2(programs[i].bytes) < CODE_ELEMENT)	
		{
//			IntFlashWriteByte(BASE_CODE_INDEX + i * 2,mGetPointWord2(programs[i].bytes));
//			IntFlashWriteByte(BASE_CODE_INDEX + i * 2 + 1,(mGetPointWord2(programs[i].bytes) >> 8));
			IntFlashWriteInt(BASE_CODE_INDEX + i * 2,mGetPointWord2(programs[i].bytes));
			IntFlashReadInt(BASE_CODE_INDEX + i * 2, &temp);
			Test[40 + i] = mGetPointWord2(programs[i].bytes);
			Test[30 + i] = temp;
			for(loop = 0;loop < mGetPointWord2(programs[i].bytes) + USER_DATA_HEADER_LEN;loop++)
			{
				IntFlashWriteByte(BASE_CODE + base_addr + loop,prg_code[i][loop]);
			}
			base_addr += (mGetPointWord2(programs[i].bytes) + USER_DATA_HEADER_LEN);
		}
		else
		{
			temp = 0;
//			IntFlashWriteByte(BASE_CODE_INDEX + i * 2,0);
//			IntFlashWriteByte(BASE_CODE_INDEX + i * 2 + 1,0);
			IntFlashWriteInt(BASE_CODE_INDEX + i * 2,temp);	
		}
	}
	IntFlashWriteInt(BASE_CODE_LEN,Code_total_length);
}

void Flash_Read_Code(void)
{
	U8_T i;
	U16_T loop;
	U16_T temp = 0;
	U32_T base_addr = 0;
	Code_total_length = 0;
	Test[29]++;
	for(i = 0;i < MAX_PRGS;i++)
	{	
		IntFlashReadInt(BASE_CODE_INDEX + i * 2, &temp);
		if(temp > CODE_ELEMENT)
			temp = 0;
		programs[i].bytes = mGetPointWord2(temp); 
		Code_total_length += mGetPointWord2(programs[i].bytes);
		if(mGetPointWord2(programs[i].bytes) > 0 && mGetPointWord2(programs[i].bytes) < CODE_ELEMENT)	
		{
			for(loop = 0;loop < mGetPointWord2(programs[i].bytes) + USER_DATA_HEADER_LEN;loop++)
			{
				IntFlashReadByte(BASE_CODE + base_addr + loop,&prg_code[i][loop]);
			}
			base_addr += (mGetPointWord2(programs[i].bytes) + USER_DATA_HEADER_LEN);
			//return;
		}
		else
			memcpy(&prg_code[i] ,0, CODE_ELEMENT);
	}
	//IntFlashReadInt(BASE_CODE_LEN, &Code_total_length);
	//if(Code_total_length == 0xffff)		Code_total_length = 0;
}



