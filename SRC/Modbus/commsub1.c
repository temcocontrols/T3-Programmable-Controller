#include "main.h"
#include "serial.h"
#include "commsub.h"
#include "schedule.h"
#include "scan.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if 1	

#define CommTst_STACK_SIZE	((unsigned portSHORT)1024)
#define ScanRes_STACK_SIZE	((unsigned portSHORT)1024)

xTaskHandle xdata xCommTstTask;
xTaskHandle xdata xScanResTask;
xQueueHandle	xSubRevQueue;
extern bit flag_transimit_from_tcpip;
extern U8_T	TcpSocket_ME;
U8_T tmp_sendbuf[200];


U8_T far tst_addr_index = 0;
U8_T far tst_reg_index = READ_PRODUCT_MODLE;	

U8_T far flag_send_scan_table;  

U8_T far tstat_product_model[8];
U16_T far tstat_temperature[8] = 0;
U16_T far tstat_mode[8] = 0;
U16_T far tstat_setpoint[8] = 15;
U16_T far tstat_cool_setpoint[8] = 0;
U16_T far tstat_heat_setpoint[8] = 0;
U8_T far tstat_occupied = 0; // occupied is 1 bit, 8 BIT is S8_T
U8_T far tstat_output_state[8];
U8_T far tstat_night_heat_db[8];
U8_T far tstat_night_cool_db[8];
U8_T far tstat_night_heat_sp[8];
U8_T far tstat_night_cool_sp[8];
U8_T far tstat_over_ride[8];
U8_T far tstat_serial_number[8][4];
U8_T far tstat_address[8];
U8_T far tstat_type[8];

U8_T far  schedule_data[254];
U8_T far  schedule_id;

U8_T far  WRT_Tst_Reg = 0;
U8_T far  WRT_Tst_ID = 0;

U8_T scan_status = NONE_ID;
U16_T scan_index;

extern U8_T far scheduel_id;

const U16_T Tst_Register[Tst_reg_num][3] = 	// 0 : tstat5ABCDEFG  1: tstat6/tstat7	  2:TST5EH  
{
  	{7,7,7},	   	 // TSTAT_PRODUCT_MODEL
	{184,109,184},	 //	TSTAT_OCCUPIED
	{380,345,522},	 // TSTAT_COOL_SETPOINT
	{136,639,136},	 // TSTAT_HEAT_SETPOINT
	{135,638,135},
	{101,121,101},
	{107,102,107},
	{108,209,108},
	{123,352,123},
	{124,353,124},
	{182,354,182},
	{183,355,183},
	{6,6,6},
	{211,111,211},
	{0,0,0},
	{1,1,1},
	{2,2,2},
	{3,3,3}
};



BinSearch xdata binsearch_Table[511];  // 129 is tested, the max time

U8_T far  test1;
U8_T far  test_index = 0;
extern U8_T  ttt[50];
// if the current search item is i, then the next search item is (2i + 1 , 2(i + 1))
void Comm_Tstat_Initial_Data(void)
{
	tstat_occupied = 0;
	memset(tstat_product_model,0,8);
	memset(tstat_temperature,0,16);
	memset(tstat_mode,0,16);
	memset(tstat_setpoint,0,16);
	memset(tstat_cool_setpoint,0,16);
	memset(tstat_heat_setpoint,0,16);
	memset(tstat_output_state,0,8);
	memset(tstat_night_heat_db,0,8);
	memset(tstat_night_cool_db,0,8);
	memset(tstat_night_heat_sp,0,8);
	memset(tstat_night_cool_sp,0,8);

	memset(tstat_over_ride,0,8);
	memset(tstat_serial_number,0,32);
	memset(tstat_address,0,8);

	memset(schedule_data,0,254);


	memset(tstat_type,0xff,8);

	sub_no = 0;
	memset(sub_addr,0,8);

}


void Com_Tstat(U8_T  types,U8_T addr)
{
	U8_T data send_buffer[8];
	U8_T i;
	U8_T type = tstat_type[tst_addr_index];	

	sub_init_send_com();
	sub_init_crc16();
	

	if(types == SEND_SCHEDUEL)
	{	  // write command
		char i;	
					
		send_buffer[0] = send_schedual[0];
		send_buffer[1] = send_schedual[1];
		type = tstat_type[scheduel_id];		
		send_buffer[2] = Tst_Register[TST_OCCUPIED][type] >> 8;  
		send_buffer[3] = Tst_Register[TST_OCCUPIED][type];
		send_buffer[4] = send_schedual[4];
		send_buffer[5] = send_schedual[5];
	//	send_buffer[6] = send_schedual[6];
	//	send_buffer[7] = send_schedual[7];
		sub_send_string(&send_buffer,6);
		for(i = 0;i < 6;i++)
		{
		//	sub_send_byte(send_schedual[i],CRC_YES);
			Test[14 + i] = send_buffer[i];
		}
		//Test[17]++;		
	}
	else
	{
		if(type == 0xff)
		{
			if(types == READ_PRODUCT_MODLE)
				type = 0;
			else
				return;
		}
		if(types == READ_ROOM_SETPOINT)
		{
			send_buffer[0] = addr;
			send_buffer[1] = READ_VARIABLES;
			send_buffer[2] = Tst_Register[TST_ROOM_SETPOINT][type]>>8;
			send_buffer[3] = Tst_Register[TST_ROOM_SETPOINT][type];
			send_buffer[4] = 0;
			send_buffer[5] = 1;	
				
			sub_send_string(&send_buffer,6);
		}
		else if(types == WRITE_ROOM_SETPOINT)
		{
			
			send_buffer[0] = addr;
			send_buffer[1] = WRITE_VARIABLES;
			send_buffer[2] = Tst_Register[TST_ROOM_SETPOINT][type] >> 8;
			send_buffer[3] = Tst_Register[TST_ROOM_SETPOINT][type];
			send_buffer[4] = 0;
			send_buffer[5] = tstat_setpoint[by_tstat_index]; 
			sub_send_string(&send_buffer,6);
		}
		else if(types == READ_HEATTING_SETPOINT)
		{	 
			send_buffer[0] = addr;
			send_buffer[1] = READ_VARIABLES;
			send_buffer[2] = Tst_Register[TST_HEAT_SETPOINT][type] >> 8;
			send_buffer[3] = Tst_Register[TST_HEAT_SETPOINT][type];//136;
			send_buffer[4] = 0;
			send_buffer[5] = 1;		
			sub_send_string(&send_buffer,6);
		}
		else if(types == WRITE_HEATTING_SETPOINT)
		{
			send_buffer[0] = addr;
			send_buffer[1] = WRITE_VARIABLES;	
			send_buffer[2] = Tst_Register[TST_HEAT_SETPOINT][type] >> 8;
			send_buffer[3] = Tst_Register[TST_HEAT_SETPOINT][type];				
			send_buffer[4] = 0;
			send_buffer[5] = tstat_heat_setpoint[by_tstat_index];
			sub_send_string(&send_buffer,6);
		}
		else if(types == READ_COOLING_SETPOINT)
		{
			send_buffer[0] = addr;
			send_buffer[1] = READ_VARIABLES;	
			send_buffer[2] = Tst_Register[TST_COOL_SETPOINT][type] >> 8; //0x01;
			send_buffer[3] = Tst_Register[TST_COOL_SETPOINT][type];//0x7c;//TSTAT_COOL_SETPOINT;//380 = 0x17c;	
			send_buffer[4] = 0;
			send_buffer[5] = 1;		
			sub_send_string(&send_buffer,6);
		}
		else if(types == WRITE_COOLING_SETPOINT)
		{
			send_buffer[0] = addr;
			send_buffer[1] = WRITE_VARIABLES;	
			send_buffer[2] = Tst_Register[TST_COOL_SETPOINT][type] >> 8;
			send_buffer[3] = Tst_Register[TST_COOL_SETPOINT][type];	
			send_buffer[4] = 0;
			send_buffer[5] = tstat_cool_setpoint[by_tstat_index];
			sub_send_string(&send_buffer,6);
		}
		else if(types == READ_TEMPERAUTE)
		{
			send_buffer[0] = addr;
			send_buffer[1] = READ_VARIABLES;	
			send_buffer[2] = Tst_Register[TST_ROOM_TEM][type] >> 8;
			send_buffer[3] = Tst_Register[TST_ROOM_TEM][type];//TSTAT_ROOM_TEM;//101;
			send_buffer[4] = 0;
			send_buffer[5] = 1;
			sub_send_string(&send_buffer,6);		
		}
		else if(types == READ_MODE_OPERATION)
		{
			send_buffer[0] = addr;
			send_buffer[1] = READ_VARIABLES;	
			send_buffer[2] = Tst_Register[TST_MODE][type] >> 8;//0;
			send_buffer[3] = Tst_Register[TST_MODE][type];//TSTAT_MODE;//107;
			send_buffer[4] = 0;
			send_buffer[5] = 1;
			sub_send_string(&send_buffer,6);		
		}
		else if(types == READ_OUTPUT_STATE)
		{	
			send_buffer[0] = addr;
			send_buffer[1] = READ_VARIABLES;	
			send_buffer[2] = Tst_Register[TST_OUTPUT_STATE][type] >> 8;
			send_buffer[3] = Tst_Register[TST_OUTPUT_STATE][type];//108;
			send_buffer[4] = 0;
			send_buffer[5] = 1;
			sub_send_string(&send_buffer,6);		
		}
		else if(types == READ_OCCUPIED_STATE)
		{   // 184.0   read 
			send_buffer[0] = addr;
			send_buffer[1] = READ_VARIABLES;	
			send_buffer[2] = Tst_Register[TST_OCCUPIED][type] >> 8;
			send_buffer[3] = Tst_Register[TST_OCCUPIED][type];
			send_buffer[4] = 0;
			send_buffer[5] = 1;
			sub_send_string(&send_buffer,6);		
		}
		else if(types == READ_NIGHT_HEAT_DB)
		{
			send_buffer[0] = addr;
			send_buffer[1] = READ_VARIABLES;
			send_buffer[2] = Tst_Register[TST_NIGHT_HEAT_DB][type] >> 8;
			send_buffer[3] = Tst_Register[TST_NIGHT_HEAT_DB][type];//123;
			send_buffer[4] = 0;
			send_buffer[5] = 1;
			sub_send_string(&send_buffer,6);				
		}
		else if(types == WRITE_NIGHT_HEAT_DB)
		{
			send_buffer[0] = addr;
			send_buffer[1] = WRITE_VARIABLES;
			send_buffer[2] = Tst_Register[TST_NIGHT_HEAT_DB][type] >> 8;
			send_buffer[3] = Tst_Register[TST_NIGHT_HEAT_DB][type];//123;
			send_buffer[4] = 0;
			send_buffer[5] = tstat_night_heat_db[by_tstat_index];
			sub_send_string(&send_buffer,6);		
		}
		else if(types == READ_NIGHT_COOL_DB)
		{			
			send_buffer[0] = addr;
			send_buffer[1] = READ_VARIABLES;
			send_buffer[2] = Tst_Register[TST_NIGHT_COOL_DB][type] >> 8;
			send_buffer[3] = Tst_Register[TST_NIGHT_COOL_DB][type];//124;
			send_buffer[4] = 0;
			send_buffer[5] = 1;
			sub_send_string(&send_buffer,6);		
		}		
		else if(types == WRITE_NIGHT_COOL_DB)
		{		
			send_buffer[0] = addr;
			send_buffer[1] = WRITE_VARIABLES;
			send_buffer[2] = Tst_Register[TST_NIGHT_COOL_DB][type] >> 8;
			send_buffer[3] = Tst_Register[TST_NIGHT_COOL_DB][type];//124;
			send_buffer[4] = 0;
			send_buffer[5] = tstat_night_cool_db[by_tstat_index];
			sub_send_string(&send_buffer,6);		
		}		 
		else if(types == READ_NIGHT_HEAT_SP)
		{			
			send_buffer[0] = addr;
			send_buffer[1] = READ_VARIABLES;
			send_buffer[2] = Tst_Register[TST_NIGHT_HEAT_SP][type] >> 8;
			send_buffer[3] = Tst_Register[TST_NIGHT_HEAT_SP][type];//182;
			send_buffer[4] = 0;
			send_buffer[5] = 1;
			sub_send_string(&send_buffer,6);	
		}		
		else if(types == WRITE_NIGHT_HEAT_SP)
		{
			send_buffer[0] = addr;
			send_buffer[1] = WRITE_VARIABLES;
			send_buffer[2] = Tst_Register[TST_NIGHT_HEAT_SP][type] >> 8;
			send_buffer[3] = Tst_Register[TST_NIGHT_HEAT_SP][type];//182;
			send_buffer[4] = 0;
			send_buffer[5] = tstat_night_heat_sp[by_tstat_index];
			sub_send_string(&send_buffer,6);
		}		
		else if(types == READ_NIGHT_COOL_SP)
		{			
			send_buffer[0] = addr;
			send_buffer[1] = READ_VARIABLES;
			send_buffer[2] = Tst_Register[TST_NIGHT_COOL_SP][type] >> 8;
			send_buffer[3] = Tst_Register[TST_NIGHT_COOL_SP][type];//183;
			send_buffer[4] = 0;
			send_buffer[5] = 1;
			sub_send_string(&send_buffer,6);		
		}
		else if(types == WRITE_NIGHT_COOL_SP)
		{			
			send_buffer[0] = addr;
			send_buffer[1] = WRITE_VARIABLES;
			send_buffer[2] = Tst_Register[TST_NIGHT_COOL_SP][type] >> 8;
			send_buffer[3] = Tst_Register[TST_NIGHT_COOL_SP][type];//183;		
			send_buffer[4] = 0;
			send_buffer[5] = tstat_night_cool_sp[by_tstat_index];
			sub_send_string(&send_buffer,6);		
		}		
		else if(types == READ_PRODUCT_MODLE)
		{			
			send_buffer[0] = addr;
			send_buffer[1] = READ_VARIABLES;
			send_buffer[2] = Tst_Register[TST_PRODUCT_MODEL][type] >> 8;
			send_buffer[3] = Tst_Register[TST_PRODUCT_MODEL][type];//7;		
			send_buffer[4] = 0;
			send_buffer[5] = 1;
			sub_send_string(&send_buffer,6);
		}		
		else if(types == READ_OVER_RIDE)
		{			
			send_buffer[0] = addr;
			send_buffer[1] = READ_VARIABLES;
			send_buffer[2] = Tst_Register[TST_OVER_RIDE][type] >> 8;
			send_buffer[3] = Tst_Register[TST_OVER_RIDE][type];//TSTAT_OVER_RIDE;//211		
			send_buffer[4] = 0;
			send_buffer[5] = 1;
			sub_send_string(&send_buffer,6);		
		}	/* tstat_over_ride have two bytes  */
		else if(types == WRITE_OVER_RIDE)
		{			
			send_buffer[0] = addr;
			send_buffer[1] = WRITE_VARIABLES;
			send_buffer[2] = Tst_Register[TST_OVER_RIDE][type] >> 8;
			send_buffer[3] = Tst_Register[TST_OVER_RIDE][type];//211		
			send_buffer[4] = tstat_over_ride[by_tstat_index] >> 8;
			send_buffer[5] = (U8_T)tstat_over_ride[by_tstat_index];
			sub_send_string(&send_buffer,6);		
		}	/*  serial number have 4 bytes */
		else if(types == READ_SERIAL_NUMBER_0)
		{			
			send_buffer[0] = addr;
			send_buffer[1] = READ_VARIABLES;
			send_buffer[2] = Tst_Register[TST_SERIAL_NUM_0][type] >> 8;
			send_buffer[3] = Tst_Register[TST_SERIAL_NUM_0][type]; // the start address of tstat serial number 
			send_buffer[4] = 0;
			send_buffer[5] = 1;  
			sub_send_string(&send_buffer,6);		
		}			
		else if(types == READ_SERIAL_NUMBER_1)
		{			
			send_buffer[0] = addr;
			send_buffer[1] = READ_VARIABLES;
			send_buffer[2] = Tst_Register[TST_SERIAL_NUM_1][type] >> 8;
			send_buffer[3] = Tst_Register[TST_SERIAL_NUM_1][type]; // the start address of tstat serial number 
			send_buffer[4] = 0;
			send_buffer[5] = 1;  
			sub_send_string(&send_buffer,6);		
		}			
		else if(types == READ_SERIAL_NUMBER_2)
		{			
			send_buffer[0] = addr;
			send_buffer[1] = READ_VARIABLES;
			send_buffer[2] = Tst_Register[TST_SERIAL_NUM_2][type] >> 8;
			send_buffer[3] = Tst_Register[TST_SERIAL_NUM_2][type]; // the start address of tstat serial number 
			send_buffer[4] = 0;
			send_buffer[5] = 1;  
			sub_send_string(&send_buffer,6);		
		}			
		else if(types == READ_SERIAL_NUMBER_3)
		{			
			send_buffer[0] = addr;
			send_buffer[1] = READ_VARIABLES;
			send_buffer[2] = Tst_Register[TST_SERIAL_NUM_3][type] >> 8;
			send_buffer[3] = Tst_Register[TST_SERIAL_NUM_3][type]; // the start address of tstat serial number 
			send_buffer[4] = 0;
			send_buffer[5] = 1;  
			sub_send_string(&send_buffer,6);		
		}			
	/*	else if(types == READ_WALL_SETPOINT)
		{		
			send_buffer[0] = addr;
			send_buffer[1] = READ_VARIABLES;
			send_buffer[2] = 1;  // 341 = 1 * 256 + 0x55
			send_buffer[3] = 0x55;
			send_buffer[4] = 0;
			send_buffer[5] = 1;
			sub_send_string(&send_buffer,6);
		}		 
		else if(types == WRITE_WALL_SETPOINT)
		{			
			send_buffer[0] = addr;
			send_buffer[1] = WRITE_VARIABLES;
			send_buffer[2] = 1;  // 341 = 1 * 256 + 0x55
			send_buffer[3] = 0x55;
			send_buffer[4] = 0;
			send_buffer[5] = tstat_setpoint[by_tstat_index];
			sub_send_string(&send_buffer,6);
		}*/	
		 
	/*	else if(types == SEND_SCHEDUEL)
		{	  // write command
			char i;	
						
			send_buffer[0] = send_schedual[0];
			send_buffer[1] = send_schedual[1];
			type = tstat_type[scheduel_id];		
			send_buffer[2] = Tst_Register[TST_OCCUPIED][type] >> 8;  
			send_buffer[3] = Tst_Register[TST_OCCUPIED][type];
			send_buffer[4] = send_schedual[4];
			send_buffer[5] = send_schedual[5];
		//	send_buffer[6] = send_schedual[6];
		//	send_buffer[7] = send_schedual[7];
			sub_send_string(&send_buffer,6);
			for(i = 0;i < 6;i++)
			{
			//	sub_send_byte(send_schedual[i],CRC_YES);
				Test[14 + i] = send_buffer[i];
			}
			//Test[17]++;		
		} */
		else if(types == READ_ADDRESS)	// read command 
		{			
			send_buffer[0] = addr;
			send_buffer[1] = READ_VARIABLES;
			send_buffer[2] = Tst_Register[TST_ADDRESS][type] >> 8;  
			send_buffer[3] = Tst_Register[TST_ADDRESS][type];//6;
			send_buffer[4] = 0;
			send_buffer[5] = 1;
			sub_send_string(&send_buffer,6);
		}	/*	case WRITE_ADDRESS:
				send_buffer[0] = addr;
				send_buffer[1] = WRITE_VARIABLES;			
				send_buffer[2] = 0;  
				send_buffer[3] = TSTAT_ADDRESS;//184;
				send_buffer[4] = 0;
				send_buffer[5] = schedule_data[schedule_id];
				sub_send_string(&send_buffer,6);
				break;
		*/	
	}


	if(send_buffer[1] == READ_VARIABLES)
		sub_rece_size = 7;
	else if(send_buffer[1] == WRITE_VARIABLES)
		sub_rece_size = 8;
	sub_serial_restart();
	for(i = 0;i < 12;i++)
		sub_data_buffer[i] = 0;	
		
}


void internal_sub_deal(U8_T cmd_index,U8_T tst_addr_index,U8_T *sub_net_buf)
{
	U8_T type = tstat_type[tst_addr_index];	
	if(type == 0xff)
	{
		if(cmd_index == READ_PRODUCT_MODLE)
			type = 0;
		else
			return;
	}
	if(sub_net_buf[1] == READ_VARIABLES)
	{		
		switch(cmd_index)
		{ 			
			case READ_ROOM_SETPOINT:			
				tstat_setpoint[tst_addr_index] = sub_net_buf[3] * 256 + sub_net_buf[4];						
				
				break;
			case READ_HEATTING_SETPOINT: 				
				tstat_heat_setpoint[tst_addr_index] = sub_net_buf[3] * 256 + sub_net_buf[4];	
				
				break;
			case READ_COOLING_SETPOINT:				
				tstat_cool_setpoint[tst_addr_index] = sub_net_buf[3] * 256 + sub_net_buf[4];
				break;
			case READ_TEMPERAUTE:
				tstat_temperature[tst_addr_index] = sub_net_buf[3] * 256 + sub_net_buf[4];
				break;				
			case READ_MODE_OPERATION:	
			
				tstat_mode[tst_addr_index] = sub_net_buf[4];	
				break;	
			case READ_OUTPUT_STATE:	
				tstat_output_state[tst_addr_index] = sub_net_buf[4];	
				break;	
			case READ_OCCUPIED_STATE:	
				if(	sub_net_buf[5] == 1)
					tstat_occupied |= (0x01 << tst_addr_index);
				else 
					tstat_occupied &= ~(0x01 << tst_addr_index);
				
				break;	
			case READ_NIGHT_HEAT_DB:	
				tstat_night_heat_db[tst_addr_index] = sub_net_buf[4];
				break;	
			case READ_NIGHT_COOL_DB:
				tstat_night_cool_db[tst_addr_index] = sub_net_buf[4];
				break;	
			case READ_NIGHT_HEAT_SP:
				tstat_night_heat_sp[tst_addr_index] = sub_net_buf[4];
				break;	
			case READ_NIGHT_COOL_SP:
				tstat_night_cool_sp[tst_addr_index] = sub_net_buf[4];
				break;	
			case READ_PRODUCT_MODLE:
				if(sub_net_buf[4] == 6 || sub_net_buf[4] == 7)
					tstat_type[tst_addr_index] = TSTAT_6;
				else if(sub_net_buf[4] == 1 || sub_net_buf[4] == 2 ||sub_net_buf[4] == 3 ||sub_net_buf[4] == 4 ||sub_net_buf[4] == 12||sub_net_buf[4] == 17)
					tstat_type[tst_addr_index] = TSTAT_5A;
				else if(sub_net_buf[4] == 16 || sub_net_buf[4] == 19)
					tstat_type[tst_addr_index] = TSTAT_5E;
				tstat_product_model[tst_addr_index] = sub_net_buf[4];
				break;
			case READ_OVER_RIDE:
				tstat_over_ride[tst_addr_index] = sub_net_buf[4];
				break;
			case READ_SERIAL_NUMBER_0:
				tstat_serial_number[tst_addr_index][0] = sub_net_buf[4];
				break;	
			case READ_SERIAL_NUMBER_1:
				tstat_serial_number[tst_addr_index][1] = sub_net_buf[4];
				break;	
			case READ_SERIAL_NUMBER_2:
				tstat_serial_number[tst_addr_index][2] = sub_net_buf[4];
				break;	
			case READ_SERIAL_NUMBER_3:
				tstat_serial_number[tst_addr_index][3] = sub_net_buf[4];
				break;	
			case READ_WALL_SETPOINT:
				tstat_setpoint[tst_addr_index] = sub_net_buf[4];/*((U16_T)sub_net_buf[4] * 256 + sub_net_buf[5])/100;*/
				break;
			case READ_ADDRESS:
				//ttt[0] = sub_net_buf[3];
				//ttt[1] = sub_net_buf[4];
				break;
	
			default:
				break;				
		}	
	}
	else if(sub_net_buf[1] == CHECKONLINE)	
	{
		
	}	
}



void scan_response_cope(void);




void vStartCommSubTasks( U8_T uxPriority)
{	
	xSubRevQueue = xQueueCreate(3,sizeof(U8_T) * 15);
//	xKeyQueue = xQueueCreate(KEY_QUEUE_SIZE,sizeof(U8_T));

	sTaskCreate(Comm_Tstat_task, (const signed portCHAR * const)"Comm_Tstat_task",CommTst_STACK_SIZE, NULL, uxPriority - 1, (xTaskHandle *)&xCommTstTask);
	sTaskCreate(scan_response_cope, (const signed portCHAR * const)"scan_response_cope",ScanRes_STACK_SIZE, NULL, uxPriority, (xTaskHandle *)&xScanResTask);

}

U8_T source = NO_SOURCE; 

void scan_response_cope(void)
{
	U16_T crc_val;
//	U8_T ret;
	portTickType xDelayPeriod = ( portTickType ) 25 / portTICK_RATE_MS;
	for (;;)
	{	
		vTaskDelay(xDelayPeriod);
//		if(flag_transimit_from_serial)  continue;	
		Test[13]++;
	//	if(source == NO_SOURCE)	   continue;
		if(source == SCAN_CMD)
		{	
			if(sub_rece_count >= 9)	  // response scan command
			{
			//	scan_status = MULTIPLE_ID;
				if(sub_data_buffer[0] == 0xff && sub_data_buffer[1] == 0x19)
				{
					crc_val = crc16(sub_data_buffer,7);
					if(crc_val == sub_data_buffer[7] * 256 + sub_data_buffer[8])
					{	
						if(sub_rece_count >= 12)
						{						
						//	Test[20]++;
							temp_scan_db.sn = ((unsigned long)sub_data_buffer[3] << 24) | ((unsigned long)sub_data_buffer[4] << 16) | 
												((unsigned long)sub_data_buffer[5] << 8) | sub_data_buffer[6];								
							temp_scan_db.id = sub_data_buffer[2];  
		
							binsearch_Table[2 * scan_index + 1].valid = 1;
							binsearch_Table[2 * (scan_index + 1)].valid = 1;	
						//	scan_status = UNIQUE_ID_FROM_MULTIPLE;
							update_tstat_list(temp_scan_db.id);
						//	source = NO_SOURCE;

						}
						else if(sub_rece_count == 9)
						{
						//	Test[21]++;
						//	Test[31] = scan_index;
							temp_scan_db.sn = ((unsigned long)sub_data_buffer[3] << 24) | ((unsigned long)sub_data_buffer[4] << 16) | 
												((unsigned long)sub_data_buffer[5] << 8) | sub_data_buffer[6];								
							temp_scan_db.id = sub_data_buffer[2];
							
							/*if(scan_id_table[scan_index][1] == scan_id_table[scan_index][0])
							{
								Test[25]++;
							} */
							update_tstat_list(temp_scan_db.id);	
							binsearch_Table[2 * scan_index + 1].valid = 1;
							binsearch_Table[2 * (scan_index + 1)].valid = 1;								
						}						
					}
					else
					{	
						if(sub_rece_count >= 12)
						{	 
						//	Test[22]++;	
						//	Test[32] = scan_index;
						//	binsearch_Table[2 * scan_index + 1].valid = 1;
						//	binsearch_Table[2 * (scan_index + 1)].valid = 1;				
						}
						else  if(sub_rece_count == 9)
						{
							//if(binsearch_Table[scan_index].max == binsearch_Table[scan_index].min)
							{
							//   Test[24] = binsearch_Table[scan_index].max;
							 //  	Test[23]++;
							}
													
							binsearch_Table[2 * scan_index + 1].valid = 1;
							binsearch_Table[2 * (scan_index + 1)].valid = 1;
							
						//	assignment_id_with_sn(sub_data_buffer[2],Get_Idle_ID(tempaddr,temp_sub_no),temp_scan_db.sn);	
						//	tstat_id = tempID;	
								
						}
					//	source = NO_SOURCE;					
					}
				
				}
			//	source = NO_SOURCE;
				
			}
			else
			{
				binsearch_Table[2 * scan_index + 1].valid = 0;
				binsearch_Table[2 * (scan_index + 1)].valid = 0;
			}
			
		}
		else if (source == GET_INFO_CMD)
		{
			U8_T CRC_OK = 0;
			if(sub_rece_count >= 7)
			{	
		 		crc_val = crc16(sub_data_buffer,5);	
				if(crc_val == sub_data_buffer[5] * 256 + sub_data_buffer[6])
				{	
					CRC_OK = 1;	
					internal_sub_deal(tst_reg_index,tst_addr_index,sub_data_buffer);
				}
				if(tst_reg_index == READ_PRODUCT_MODLE && CRC_OK == 1)	tst_reg_index++;
				else
				{	
					if(tst_reg_index < READ_SERIAL_NUMBER_3)
						tst_reg_index++;
					else
					{
						tst_reg_index = READ_PRODUCT_MODLE;//READ_ROOM_SETPOINT;
							
						if(tst_addr_index <  sub_no - 1)
						{						
							tst_addr_index++;
						}
						else
						{
							tst_addr_index = 0;	 
						}
					}
				}
				source = NO_SOURCE;
			}
		}
		else if(source == BUTTON)
		{
			Test[14]++;	
			if(flag_control_by_button == 1)
			{
			   	if(sub_rece_count >= 8)
				{					
					Test[17]++;	
//					memcpy(&Test[20],sub_data_buffer,8);
					crc_val = crc16(sub_data_buffer,6);	
					if(crc_val == sub_data_buffer[6] * 256 + sub_data_buffer[7])
					{	
						Test[18]++;				
						flag_control_by_button = 0;
					}
				   	source = NO_SOURCE;
				}
			}
		}
		else if(source == SCHEDUEL)
		{
			if(flag_send_schedual == 1)
			{
			   	if(sub_rece_count >= 8)
				{					
				//	Test[33]++;	
					memcpy(&Test[20],sub_data_buffer,8);
					crc_val = crc16(sub_data_buffer,6);	
					if(crc_val == sub_data_buffer[6] * 256 + sub_data_buffer[7])
					{	
				//		Test[34]++;				
						flag_send_schedual = 0;
					}
				   	source = NO_SOURCE;
				}
			}
		}
		else if (source == PC_SERIAL)
		{
			U8_T i;
		//	Test[31] = sub_rece_size;
			//if(flag_transimit_from_serial)
			{	
				if(sub_rece_count >= sub_rece_size)
				{
				//	Test[32]++;	
			 		crc_val = crc16(sub_data_buffer,5);	
					//if(crc_val == sub_data_buffer[5] * 256 + sub_data_buffer[6])
					{	
					//	Test[33]++;
					//	EA = 0;
					//	vTaskDelay(500 / portTICK_RATE_MS);
						main_init_send_com();	 // enable COM1 send 
						// send data to MBPOLL or T3000 
						for(i = 0;i < sub_rece_size;i++)					
							main_send_byte(sub_data_buffer[i],CRC_YES);	 
					//	EA = 1;
						
					}
				//	Test[49]++;	
				//	flag_transimit_from_serial = 0;			
					source = NO_SOURCE;
				}
			}
			
		} 
		else if (source == PC_TCPIP)
		{
			U8_T i;
		//	if(flag_transimit_from_tcpip)	
			{
				if(sub_rece_count >= sub_rece_size - 4)
				{
					Test[35]++;	
			 		crc_val = crc16(sub_data_buffer,5);	
				//	if(crc_val == sub_data_buffer[5] * 256 + sub_data_buffer[6])
					{	
						for(i = 0;i < sub_rece_size - 6;i++)
						{					
							tmp_sendbuf[6 + i] = sub_data_buffer[i];
							 	
						}
					}
					TCPIP_TcpSend(TcpSocket_ME, tmp_sendbuf, sub_rece_size, TCPIP_SEND_NOT_FINAL); 
					flag_transimit_from_tcpip = 0;
					source = NO_SOURCE;
				}
			}
			
		} 
	//	Test[43] = sub_serial_receive_timeout_count;
		if(sub_serial_receive_timeout_count > 0)
		{
			sub_serial_receive_timeout_count--;
			if(sub_serial_receive_timeout_count == 0)
			{
				sub_serial_restart();
			//	Test[44]++;
				source = NO_SOURCE;
			}
		} 
	}
}


void Comm_Tstat_task(void)
{
	portTickType xDelayPeriod = ( portTickType ) 100 / portTICK_RATE_MS;	  
	static U8_T tempindex = 0;
	static U8_T tmpcount = 0;   // count for  flag_transimit_from_serial
	char far i;

	for (;;)
	{
	   	vTaskDelay( xDelayPeriod); 	
	/*	if(flag_transimit_from_serial || flag_transimit_from_tcpip)
		{ // 5s reset flag_transimit_from_serial		   	
			if(tmpcount > 100)	   
			{
				if(flag_transimit_from_serial)	flag_transimit_from_serial = 0;	
				if(flag_transimit_from_tcpip)	flag_transimit_from_tcpip = 0; 
				tmpcount = 0;
			}
			else
				tmpcount++;
		}	
		if(flag_transimit_from_serial || flag_transimit_from_tcpip)  continue;

		if(flag_control_by_button == 1)
		{			
			Com_Tstat(WRT_Tst_Reg,WRT_Tst_ID);	
			source = BUTTON;
		}
		else if(flag_send_schedual == 1)
		{			
			Com_Tstat(SEND_SCHEDUEL,0);
			source = SCHEDUEL;
			
		}  

		else */
		{
			if(tempindex == 0 )
			{	
			   	Scan_Sub_ID();
				source = SCAN_CMD;
//				sub_no = 1;
//				sub_addr[0] = 36;
				tempindex = 1;
			}
			else if(tempindex == 1)	
			{	
			   	if(sub_no == 0)	 {tempindex = 0; continue;}				

				if(protocal <= TCP_IP)
				{	
					if(DI_Type[tst_addr_index] == DI_SWITCH)
						tst_addr_index++;
				}
				else 				
				{	
				//	if(inputs[tst_addr_index].unused == DI_SWITCH)
				//		tst_addr_index++;
				}
				Com_Tstat(tst_reg_index, sub_addr[0/*tst_addr_index*/]);
				source = GET_INFO_CMD;
				
				tempindex = 0;
			}
		} 
	}
}

#endif