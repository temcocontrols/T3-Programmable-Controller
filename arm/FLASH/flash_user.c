#include "product.h"
#include "flash_user.h"
#include "define.h"
#include "stmflash.h"
#include "user_data.h"
#include "scan.h"
#include "sntpc.h"
//#include "delay.h"
#include "wifi.h"

uint8_t write_page_en[26]  = {0} ;  //fandu 长度 25改26 新增写msv的结构

static uint8_t  tempbuf[20000] = {0};

STR_Flash_POS xdata Flash_Position[24];
STR_flag_flash 	far bac_flash;

// page 125 0x0803 e800  - 0x0803 efff    2K  OUT
// page 126 0x0803 f000  - 0x0803 f7ff    2K  IN
// page 127 0x0803 f800  - 0x0803 ffff    2K  VAR



#define FLASH_BASE_ADDR	0x8068000  // - 8078000 len 64k
#define FLASH_CODE_ADDR	0x8060000  // - 8068000 code len 32k


#define FLASH_OTHER_ADDR 	0x8078000 // - other 2k
#define FLASH_OTHER_ADDR2 0x8078800 // - other 2k


// other PAGE1 2K

#define BASE_WIFI_SETTING			0x8078000  // length is 160
#define BASE_PANEL_NAME     	0x80780a0  // 20
#define BASE_DYNDNS_DONAME		0x80780c0  // 32
#define BASE_DYNDNS_USER			0x80780e0  // 32
#define BASE_DYNDNS_PASS			0x8078110  // 32
#define BASE_TST_NAME					0x8078130  // length is 1000H
#define BASE_MON_OPERATE_TIME 0x8078400 // length is 80, the lenght is at least 48
#define BASE_WEATHER  				0x8078450  // lenth is 0xc0, 192
#define BASE_SNTP_SERVER			0x8078510  // lenth is 0x20, 30
#define BASE_WEEKLY_ONOFF			0x8078530  // lenght is 0x240, 576
#define BASE_EMAIL_SETTING		0x8078770  // length is 200
	


// other page2
#define BASE_MSV_DATA					0x8078800  	// lenght is  3x8x23  0x228, 552
#define BASE_OUT_RELINQUISH	  0x8078a28   // length is 4*MAX_OUTS  0x100  256





/* caclulate detailed position for every table */
void Flash_Inital(void)
{
	uint8_t loop;
	uint16_t baseAddr = 0;	 	
	uint16_t  len = 0;
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
		case ID_ROUTION:			
			baseAddr += len;
			len = STORE_ID_LEN * 254;
			break;
		default:
	//		len = 0;
			break; 
		}
		
		if(len % 2048 != 0) 
				len = (len / 2048 + 1) * 2048;	
	
		Flash_Position[loop].addr = baseAddr;
		Flash_Position[loop].len = len;	
		write_page_en[loop] = 0;
	}
	
	for(loop = 0;loop < MAX_PRGS;loop++)
		programs[loop].real_byte = 0;		
}

void Flash_Write_Mass(void)
{
	STR_flag_flash ptr_flash;
	uint32_t base_addr;

//	uint16_t	len = 0 ;
	uint16_t loop,i;	
	uint8_t	 page;
		
	
	for(loop = 0;loop < MAX_POINT_TYPE ;loop++)
	{				
		ptr_flash.table = loop;	
		
		ptr_flash.len = Flash_Position[loop].len;
		base_addr = Flash_Position[loop].addr;
		switch(loop)
		{	
		case OUT: 
			memcpy(tempbuf,&outputs,sizeof(Str_out_point) * MAX_OUTS);					
			break;
		case IN:
			memcpy(&tempbuf,&inputs,sizeof(Str_in_point) * MAX_INS);					
			break;
		case VAR:  
			memcpy(&tempbuf,&vars,sizeof(Str_variable_point) * MAX_VARS);					
			break;
		case CON:
			memcpy(&tempbuf,&controllers,sizeof(Str_controller_point) * MAX_CONS);					
			break;
		case WRT:  				
			memcpy(&tempbuf,&weekly_routines,sizeof(Str_weekly_routine_point) * MAX_WR);					
			break;
		case AR: 
			memcpy(&tempbuf,&annual_routines,sizeof(Str_annual_routine_point) * MAX_AR);					
			break;
		case PRG:  
			memcpy(&tempbuf,&programs,sizeof(Str_program_point) * MAX_PRGS);					
			break;
		case TBL:
			memcpy(&tempbuf,&custom_tab,sizeof(Str_table_point) * MAX_TBLS);					
			break;
	/*	case TZ:
			memcpy(&tempbuf,&totalizers,sizeof(Str_totalizer_point) * MAX_TOTALIZERS);					
			break;	*/
		case AMON:
			memcpy(&tempbuf,&monitors,sizeof(Str_monitor_point) * MAX_MONITORS);					
			break;	
	case GRP: 
			memcpy(&tempbuf,&control_groups,sizeof(Control_group_point) * MAX_GRPS);					
			break;
/*			case ARRAY:
			memcpy(&tempbuf,&arrays,sizeof(Str_array_point) * MAX_ARRAYS);					
			break; 
		case ALARMM: 
//				memcpy(&tempbuf,&alarms,sizeof(Alarm_point) * MAX_ALARMS);					
			break; 	*/		
		case ALARM_SET:  
			memcpy(&tempbuf,&alarms_set,sizeof(Alarm_set_point) * MAX_ALARMS_SET);					
			break; 
		case UNIT:
			memcpy(&tempbuf,&digi_units,sizeof(Units_element) * MAX_DIG_UNIT);					
			break; 
		case USER_NAME:
			memcpy(&tempbuf,&passwords,sizeof(Password_point) * MAX_PASSW);					
			break;		
		case WR_TIME: 
			memcpy(&tempbuf,&wr_times,sizeof(Wr_one_day) * 9 * MAX_WR);
			break;
		case AR_DATA:
			memcpy(&tempbuf,&ar_dates,46 * sizeof(S8_T) * MAX_AR);					
			break;
		case TSTAT:
			memcpy(&tempbuf,&scan_db,sizeof(SCAN_DB) * SUB_NO);					
			break;		
		case GRP_POINT:	
			memcpy(&tempbuf,&group_data,sizeof(Str_grp_element) * 240);					
			break;
		case ID_ROUTION:
			for(i = 0;i < 254;i++)
			memcpy(&tempbuf[i * STORE_ID_LEN],&ID_Config[i], STORE_ID_LEN);		// store 15 bytes			
			break;
		default:	
			break;
	
		} 
		
		if(write_page_en[loop] == 1)
		{
			STMFLASH_Unlock();
			if(loop == 15)  // store code
			{
				Flash_Store_Code();
			}
			else
			{
				for(page = 0;page < ptr_flash.len / 2048;page++)
				{
					STMFLASH_ErasePage(FLASH_BASE_ADDR + base_addr + 2048 * page);	
					iap_write_appbin(FLASH_BASE_ADDR + base_addr + 2048 * page,(uint8_t*)(&tempbuf[2048 * page]), 2048);
				}
			}
			STMFLASH_Lock();			
			write_page_en[loop] = 0 ;	
		}	
		
	}	

	Flash_Write_Other();
    Flash_Write_Other_Page2();
	
}

void Flash_Read_Mass(void)
{
	STR_flag_flash ptr_flash;
	U16_T base_addr;
	U8_T loop,i;

	U8_T page;

	ptr_flash.index = 0;

	for(loop = 0;loop < MAX_POINT_TYPE;loop++)
	{
		ptr_flash.table = loop;
	
		ptr_flash.len = Flash_Position[loop].len;
		base_addr = Flash_Position[loop].addr;

	
		page = ptr_flash.len / 2048;
	
		for(page = 0;page < ptr_flash.len / 2048;page++)
		{
			if(FLASH_BASE_ADDR + base_addr + 2048 * page > 0x807F800)
			{
				break;
			}
			else
			{
				STMFLASH_MUL_Read(FLASH_BASE_ADDR + base_addr + 2048 * page,&tempbuf[page * 2048],2048);
			}
		}
		
		
		if((tempbuf[0] == 0xff) && (tempbuf[1] == 0xff) && (tempbuf[2] == 0xff) && (tempbuf[3] == 0xff) && (tempbuf[4] == 0xff)
			&& (tempbuf[5] == 0xff) && (tempbuf[6] == 0xff) && (tempbuf[7] == 0xff) && (tempbuf[8] == 0xff) && (tempbuf[9] == 0xff))
			continue;
 
		
		switch(loop)
		{
			case OUT: 
				memcpy(&outputs,&tempbuf,sizeof(Str_out_point) * MAX_OUTS);						
				break;
			case IN: 
				memcpy(&inputs,&tempbuf,sizeof(Str_in_point) * MAX_INS);					
				break;
			case VAR:
				memcpy(&vars,&tempbuf,sizeof(Str_variable_point) * MAX_VARS);					
				break;
			case CON:
				memcpy(&controllers,&tempbuf,sizeof(Str_controller_point) * MAX_CONS);					
				break;
			case WRT:  				
				memcpy(&weekly_routines,&tempbuf,sizeof(Str_weekly_routine_point) * MAX_WR);					
				break;
			case AR: 
				memcpy(&annual_routines,&tempbuf,sizeof(Str_annual_routine_point) * MAX_AR);					
				break;
			case PRG:  
				memcpy(&programs,&tempbuf,sizeof(Str_program_point) * MAX_PRGS);					
#if HANDLE_REBOOT_FLAG
                for ( i = 0; i < MAX_PRGS; i++)
                {
                    programs[i].on_off = 0;
                }
#endif
				break;
			case TBL:
				memcpy(&custom_tab,&tempbuf,sizeof(Str_table_point) * MAX_TBLS);					
				break;
		/*	case TZ:
				memcpy(&totalizers,&tempbuf,sizeof(Str_totalizer_point) * MAX_TOTALIZERS);					
				break;	*/
			case AMON:
				memcpy(&monitors,&tempbuf,sizeof(Str_monitor_point) * MAX_MONITORS);					
				break;	
		case GRP: 
				memcpy(&control_groups,&tempbuf,sizeof(Control_group_point) * MAX_GRPS);					
				break;
	/*			case ARRAY:
				memcpy(&arrays,&tempbuf,sizeof(Str_array_point) * MAX_ARRAYS);					
				break; */
			case ALARMM: 
	//				memcpy(&alarms,&tempbuf,sizeof(Alarm_point) * MAX_ALARMS);					
				break; 			
			case ALARM_SET:  
				memcpy(&alarms_set,&tempbuf,sizeof(Alarm_set_point) * MAX_ALARMS_SET);					
				break; 
			case UNIT:
				memcpy(&digi_units,&tempbuf,sizeof(Units_element) * MAX_DIG_UNIT);					
				break; 
			case USER_NAME:
				memcpy(&passwords,&tempbuf,sizeof(Password_point) * MAX_PASSW);					
				break;		
			case WR_TIME: 
				memcpy(&wr_times,&tempbuf,sizeof(Wr_one_day) * 9 * MAX_WR);
			
				break;
			case AR_DATA:
				memcpy(&ar_dates,&tempbuf,46 * sizeof(S8_T) * MAX_AR);					
				break; 
			case TSTAT:
				memcpy(&scan_db,&tempbuf,sizeof(SCAN_DB) * SUB_NO);	
//				Get_Tst_DB_From_Flash(); 
				break;	   	
			case GRP_POINT:	
				memcpy(&group_data,&tempbuf,sizeof(Str_grp_element) * 240);					
				break;
			case ID_ROUTION:	
				for(i = 0;i < 254;i++)
				{
					memcpy(&ID_Config[i],&tempbuf[i * STORE_ID_LEN],STORE_ID_LEN);	
					ID_Config_Sche[i] = ID_Config[i].Str.schedule;
				}
				break;
			default:
				break;

			}
	
	} 

	for (i = 0; i < MAX_INS; i++)
	{
		if(inputs[i].range == 0)
			inputs[i].digital_analog = 1;
	}	
	for (i = 0; i < MAX_OUTS; i++)
	{
		if(outputs[i].range == 0)
			outputs[i].digital_analog = 1;
	}
	for (i = 0; i < MAX_VARS; i++)
	{
		if(vars[i].range == 0)
			vars[i].digital_analog = 1;
	}
	Flash_Read_Code();

	Flash_Read_Other();
//    Flash_Read_Other_Page2();
}

void Flash_Store_Code(void)
{
	U8_T i;
//	U16_T temp = 0;
//	U32_T base_addr = 0;
//	U16_T loop;
//  U8_T page;

	for(i = 0;i < MAX_PRGS;i++)
	{
		STMFLASH_ErasePage(FLASH_CODE_ADDR + 2048 * i);	
		if(swap_word(programs[i].real_byte) > 0 && swap_word(programs[i].real_byte) <= CODE_ELEMENT * MAX_CODE)	
		{
			STMFLASH_WriteHalfWord(FLASH_CODE_ADDR + 2048 * i + 2000,swap_word(programs[i].real_byte));

			iap_write_appbin(FLASH_CODE_ADDR + 2048 * i,(uint8_t*)(&prg_code[i]), CODE_ELEMENT * MAX_CODE);
			
		}
		else
		{
			STMFLASH_WriteHalfWord(FLASH_CODE_ADDR + 2048 * i + 2000,0);
		}
		
	}

}


void Flash_Write_Other_Page2(void)
{
    if (write_page_en[25] == 1)
    {
        STMFLASH_Unlock();
				STMFLASH_ErasePage(FLASH_OTHER_ADDR2);
				iap_write_appbin(BASE_MSV_DATA, (u8 *)(msv_data), MAX_MSV * STR_MSV_MULTIPLE_COUNT * sizeof(multiple_struct));
				iap_write_appbin(BASE_OUT_RELINQUISH, (u8 *)(output_relinquish), 4 * MAX_OUTS);
				write_page_en[25] = 0;

			STMFLASH_Lock();
    }
}



void Flash_Write_Other(void)
{

 // name  20
	if(write_page_en[24] == 1)
	{
		STMFLASH_Unlock();
		
			STMFLASH_ErasePage(FLASH_OTHER_ADDR);
			
		//	iap_write_appbin(BASE_SNTP_SERVER,(u8 *)(&sntp_server), 30);
			
			iap_write_appbin(BASE_PANEL_NAME,(u8 *)(&panelname),20);
	#if ARM_MINI
			iap_write_appbin(BASE_DYNDNS_DONAME,dyndns_domain_name,MAX_DOMAIN_SIZE);

			iap_write_appbin(BASE_DYNDNS_USER,dyndns_username,MAX_USERNAME_SIZE);
			iap_write_appbin(BASE_DYNDNS_PASS,dyndns_password,MAX_PASSWORD_SIZE);

			// store name of tstat
			iap_write_appbin(BASE_TST_NAME,(u8 *)(&tstat_name),40/*MAX_ID*/ * 16);
			
			// store time of operating monitor 
			iap_write_appbin(BASE_MON_OPERATE_TIME,(u8 *)(&MISC_Info.reg.operate_time),MAX_MONITORS * 4);

			// store sntp
			iap_write_appbin(BASE_SNTP_SERVER,(u8 *)(&sntp_server), 30);
	#endif		
			iap_write_appbin(BASE_WEEKLY_ONOFF,(u8 *)(&wr_time_on_off),576);
			
			iap_write_appbin(BASE_EMAIL_SETTING,(u8 *)(&Email_Setting),100/*sizeof(Str_Email_point)*/);
#if (ARM_MINI || ARM_TSTAT_WIFI)			
			iap_write_appbin(BASE_WIFI_SETTING,(u8 *)(&SSID_Info),sizeof(STR_SSID));
#endif
			write_page_en[24] = 0;
	
		
		STMFLASH_Lock();
	}

}




void Flash_Read_Other(void)
{
	U16_T loop;
//	U16_T loop1;
// gsm
#if 0//USB_HOST
	if(Modbus.usb_mode == 1)
	{
		apnlen = STMFLASH_ReadHalfWord(BASE_GSM_APN); 
		if(apnlen > MAX_GSM_APN)	apnlen = MAX_GSM_APN;
		
		STMFLASH_MUL_Read(BASE_GSM_APN,apnstr,MAX_GSM_APN);

		iplen = STMFLASH_ReadHalfWord(BASE_GSM_IP); 
		if(iplen > MAX_GSM_IP)	iplen = MAX_GSM_IP;
		STMFLASH_MUL_Read(BASE_GSM_IP,ipstr,MAX_GSM_IP);
	}
#endif
// name

	STMFLASH_MUL_Read(BASE_PANEL_NAME,(u8 *)(&panelname),20);
	if((panelname[0] == 0xff) && (panelname[1] == 0xff))
	{ // default, clear it to empty
		memset(panelname,0,20);
	}

#if ARM_MINI
	for(loop = 0;loop < MAX_DOMAIN_SIZE;loop++)
	{
		STMFLASH_MUL_Read(BASE_DYNDNS_DONAME ,dyndns_domain_name,MAX_DOMAIN_SIZE);
	}
	if((dyndns_domain_name[0] == 0xff) && (dyndns_domain_name[1] == 0xff))
	{ // default, clear it to empty
		memset(dyndns_domain_name,0,MAX_DOMAIN_SIZE);
	}
	
	for(loop = 0;loop < MAX_USERNAME_SIZE;loop++)
	{
		STMFLASH_MUL_Read(BASE_DYNDNS_USER,dyndns_username,MAX_USERNAME_SIZE);
	}
	if((dyndns_username[0] == 0xff) && (dyndns_username[1] == 0xff))
	{ // default, clear it to empty
		memset(dyndns_username,0,MAX_USERNAME_SIZE);
	}
	for(loop = 0;loop < MAX_PASSWORD_SIZE;loop++)
	{
		STMFLASH_MUL_Read(BASE_DYNDNS_PASS ,dyndns_password,MAX_PASSWORD_SIZE);
	}	
	if((dyndns_password[0] == 0xff) && (dyndns_password[1] == 0xff))
	{ // default, clear it to empty
		memset(dyndns_password,0,MAX_PASSWORD_SIZE);
	}
	
	// read name of tstat	
	STMFLASH_MUL_Read(BASE_TST_NAME,(u8 *)(&tstat_name),40 * 16);

	// read time of operating monitor  
	STMFLASH_MUL_Read(BASE_MON_OPERATE_TIME,(u8 *)(&MISC_Info.reg.operate_time), 4 * MAX_MONITORS);

// read sntp server
	STMFLASH_MUL_Read(BASE_SNTP_SERVER,(u8 *)(sntp_server), 30);	
#endif	
	STMFLASH_MUL_Read(BASE_WEEKLY_ONOFF,(u8 *)(wr_time_on_off), 576);

	STMFLASH_MUL_Read(BASE_MSV_DATA,(u8 *)(msv_data), MAX_MSV * STR_MSV_MULTIPLE_COUNT * sizeof(multiple_struct));
#if ARM_TSTAT_WIFI
	//TSTAT 10 初始化 多态 用于TSTAT10 界面的默认显示;
//	Test[11] = msv_data[1][0].msv_value;
//	Test[12] = msv_data[2][0].msv_value;
//	if(msv_data[0][0].msv_value == 0xffff)
//	{Test[15]++;
//		msv_data[0][0].msv_value = 0;
//    strcpy(msv_data[0][0].msv_name, "T1");
//    msv_data[0][1].msv_value = 1;
//    strcpy(msv_data[0][1].msv_name, "T2");
//    msv_data[0][2].msv_value = 2;
//    strcpy(msv_data[0][2].msv_name, "T3");
//		write_page_en[25] = 1;
//	}
	if(msv_data[1][0].msv_value == 0xffff)
	{
		msv_data[1][0].status = 1;
		msv_data[1][0].msv_value = 0;
    strcpy(msv_data[1][0].msv_name, "Auto");
		msv_data[1][1].status = 1;
    msv_data[1][1].msv_value = 1;
    strcpy(msv_data[1][1].msv_name, "On");
		msv_data[1][2].status = 1;
    msv_data[1][2].msv_value = 2;
    strcpy(msv_data[1][2].msv_name, "Off");
		write_page_en[25] = 1;
	}
	if(msv_data[2][0].msv_value == 0xffff)
	{
		msv_data[2][0].status = 1;
    msv_data[2][0].msv_value = 0;
    strcpy(msv_data[2][0].msv_name, "Auto");
		msv_data[2][1].status = 1;
    msv_data[2][1].msv_value = 1;
    strcpy(msv_data[2][1].msv_name, "Cool");
		msv_data[2][2].status = 1;
    msv_data[2][2].msv_value = 2;
    strcpy(msv_data[2][2].msv_name, "Heat");
		write_page_en[25] = 1;
	}
#endif

	
	STMFLASH_MUL_Read(BASE_EMAIL_SETTING,(u8 *)(&Email_Setting),100/*sizeof(Str_Email_point)*/);
#if (ARM_MINI || ARM_TSTAT_WIFI)	
	STMFLASH_MUL_Read(BASE_WIFI_SETTING,(u8 *)(&SSID_Info),sizeof(STR_SSID));

	if((SSID_Info.MANUEL_EN == 0xff) && (SSID_Info.IP_Auto_Manual == 0xff))
	{ // default, clear it to empty
		memset(&SSID_Info,0,sizeof(STR_SSID));
	}
#endif
	
	STMFLASH_MUL_Read(BASE_OUT_RELINQUISH, (u8 *)(output_relinquish), 4 * MAX_OUTS);

	for(loop = 0;loop < MAX_OUTS;loop++)
	{
		if(output_relinquish[loop] == 0xffff)
		{
			output_relinquish[loop] = 0;
		}
	}
}



void Flash_Read_Code(void)
{
	U8_T i;
//	U16_T loop;
	U16_T temp = 0;
//	U32_T base_addr = 0;
	Code_total_length = 0;
	for(i = 0;i < MAX_PRGS;i++)
	{	
		temp = STMFLASH_ReadHalfWord(FLASH_CODE_ADDR + 2048 * i + 2000);
		
		if(temp > CODE_ELEMENT * MAX_CODE)
			temp = 0;
		programs[i].real_byte = swap_word(temp); 
		if(swap_word(programs[i].real_byte) > 0 && swap_word(programs[i].real_byte) <= CODE_ELEMENT * MAX_CODE)	
		{
			STMFLASH_MUL_Read(FLASH_CODE_ADDR + 2048 * i,prg_code[i],CODE_ELEMENT * MAX_CODE);
		}
		else
		{
			memset(&prg_code[i] ,0, CODE_ELEMENT * MAX_CODE);
		}
		
	}	
}






