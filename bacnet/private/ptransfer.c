/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2009 Steve Karg

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to:
 The Free Software Foundation, Inc.
 59 Temple Place - Suite 330
 Boston, MA  02111-1307, USA.

 As a special exception, if other files instantiate templates or
 use macros or inline functions from this file, or you compile
 this file and link it with other works to produce a work based
 on this file, this file does not by itself cause the resulting
 work to be covered by the GNU General Public License. However
 the source code for this file must still be made available in
 accordance with section (3) of the GNU General Public License.

 This exception does not invalidate any other reasons why a work
 based on this file might be covered by the GNU General Public
 License.
 -------------------------------------------
####COPYRIGHTEND####*/



#include <stdint.h>


// for temoc private application
#include "bacnet.h"
#include "point.h"
#include "clock.h"
#include "rs485.h"
#include "user_data.h"
#include "scan.h"
#include "define.h"
#include "stdlib.h"
#include "sntpc.h"
#include "flash.h"



void Send_Time_Sync_To_Network(void);
void Get_RTC_by_timestamp(U32_T timestamp,TimeInfo *tt,UN_Time* Rtc,U8_T source);
UN_Time Rtc2;
extern 	TimeInfo t;
U32_T get_current_timestamp(void);
void uart_send_string(U8_T *p, U16_T length,U8_T port);
//U8_T AlarmSync(uint8_t add_delete,uint8_t index,char *mes,uint8_t panel);
extern uint8_t bacnet_wifi_buf[500];
extern uint16_t bacnet_wifi_len;
/** @file ptransfer.c  Encode/Decode Private Transfer data */
/* 
	handler roution for private transfer
	created by chelsea
*/
void check_SD_PnP(void);

#if ARM_TSTAT_WIFI 
U16_T Test[50];
void Check_Pulse_Counter(void);
#endif

#if BAC_PRIVATE


#ifndef MAX_CODE_SIZE 
#define MAX_CODE_SIZE 10200
#endif


void dealwith_write_setting(Str_Setting_Info * ptr);

//extern uint8_t flag_load_prg;
//extern uint8_t count_load_prg;

U8_T    	    	far		prg_code[MAX_PRGS][CODE_ELEMENT * MAX_CODE];


uint8_t invoke_id;
extern U8_T far ChangeFlash;

uint8_t header_len;
uint16_t transfer_len;

void TCP_IP_Init(void);
void update_timers( void );

U8_T far MSTP_Send_buffer[600];
U16_T MSTP_buffer_len;

STR_MSTP_REV_HEADER far MSTP_Rec_Header;
U8_T far MSTP_Rec_buffer[600];
U8_T MSTP_Write_OK;
U8_T  MSTP_Transfer_OK;
U16_T  MSTP_Transfer_Len;



u8 flag_writing_code;
u8 count_wring_code;
// 避免写code的时候执行出问题，写完后再执行
u8 check_whehter_running_code(void)
{
	if(flag_writing_code)  // 正在写code
	{
		if(count_wring_code-- > 0)
			return 0;
		else
			flag_writing_code = 0;
	}
	return 1;
}


#if 1//(ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
#include "tcpip.h"
extern xTaskHandle xHandler_Output;
extern xTaskHandle Handle_Scan;

extern uint8_t write_page_en[26];
void refresh_zone(void);
//void responseCmd(U8_T type,U8_T* pData);
void udpate_zone_table(uint8 i);
U16_T crc16(U8_T *p, U8_T length);

U8_T 	far bacnet_to_modbus[300];
void Get_Pkt_Bac_to_Modbus(Str_user_data_header * header)
{  
	uint8_t buf[300];
	uint16_t len;
	uint16_t crc_check; 
	//  for read command
	buf[0] = Modbus.address;
	if(header->command == READ_BACNET_TO_MDOBUS)
	{
		buf[1] = 0x03;
		buf[4] = HIGH_BYTE(header->entitysize);		//0x00;
		buf[5] = LOW_BYTE(header->entitysize);   	// len		
		len = 6;
	}
	else if(header->command == WRITE_BACNET_TO_MDOBUS)
	{
		if(header->total_length - 7 > 2) // muti-write
		{
			buf[1] = 0x10;
			buf[4] = HIGH_BYTE(header->entitysize);		//0x00;
			buf[5] = LOW_BYTE(header->entitysize);   	// len	
			buf[6] = header->total_length - 7;
			memcpy(&buf[7],bacnet_to_modbus,header->total_length - 7);
			len = header->total_length;
		}
		else
		{
			buf[1] = 0x06;
			buf[4] = bacnet_to_modbus[0];
			buf[5] = bacnet_to_modbus[1];
			len = 6;
		}
	}
	
	buf[2] = header->point_end_instance;  // high_byte(start_addr)
	buf[3] = header->point_start_instance;		// low_byte(start_addr)
	crc_check = crc16(buf, len);
	buf[len] = HIGH_BYTE(crc_check);
	buf[len + 1] = LOW_BYTE(crc_check);
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
	responseCmd(4/*BAC_TO_MODBUS*/,buf);	// get bacnet_to_modbus
#endif
	
#if ASIX_MINI || ASIX_CM5
	responseCmd(4,buf,NULL);	// get bacnet_to_modbus
#endif

}


#endif

//U8_T pic_packet_index;
// for storing picture
#if 0
#if STORE_TO_SD
U8_T Write_Picture(U8_T file,U8_T * buf,U16_T index);


STR_STORE_PIC far store_pic[PIC_PACKET_STACK];

U8_T store_PIC_to_buffer(U8_T* buf,U8_T file,U16_T seg)
{	
	U8_T i;
	for(i = 0;i < PIC_PACKET_STACK;i++)
	{
		if(store_pic[i].flag == WRITE_SD_OK)
			break;
	}
	
	if(i == PIC_PACKET_STACK)		
	{  // stack full
	// tbd
		return 0;	
	}
	else
	{
		store_pic[i].flag = WAIT_FOR_SD;
		memcpy(store_pic[i].buf , buf , PIC_PACKET_LEN);
		store_pic[i].retry = 0;
		store_pic[i].file = file;
		store_pic[i].index = seg;
		return 1;
	}
}


void store_buffer_to_SD(void)
{
	U8_T i;
	for(i = 0;i < PIC_PACKET_STACK;i++)
	{
		if(store_pic[i].flag == WAIT_FOR_SD) //	get current index, 1 -- WAIT_FOR_WRITE, 0 -- WRITE_OK
		{
			if(store_pic[i].retry < 5)
			{
				store_pic[i].retry++;
				break;
			}
			else
			{  	// retry 5 time, give up
				store_pic[i].flag = WRITE_SD_OK; 
			}
		}
	}
	if(i == PIC_PACKET_STACK)		// no WAIT_FOR_WRITE
	{	
		return;	
	}
	Write_Picture(store_pic[i].file,store_pic[i].buf,store_pic[i].index);
	store_pic[i].flag = WRITE_SD_OK; 
	
}

#endif

#endif

void change_panel_number_in_code(U8_T old, U8_T new_panel)
{
	U8_T i;
	U16_T j;
	U8_T changed = 0;

	// change panel number in program
	for( i = 0; i < MAX_PRGS; i++ )
	{
		if( swap_word(programs[i].real_byte) > 0)
		{
			for(j = 0;j < swap_word(programs[i].real_byte);j++)
			{
				if(prg_code[i][j] == 0x9e)	// REMOTE_POINT_PRG
				{
					U8_T point_type;
					// it is not in 9D x x x x , 9e x VAR panel x network
					point_type = (prg_code[i][j + 2] & 0x1f) + (prg_code[i][j + 5] & 0x60);
					if(j > 4 && (prg_code[i][j - 1] != 0x9d) && (prg_code[i][j - 2] != 0x9d) \
					&& (prg_code[i][j - 3] != 0x9d) && (prg_code[i][j - 4] != 0x9d)	\
					&& (
					(point_type == VAR + 1) ||
					((point_type >= MB_COIL_REG + 1) && (point_type <= BAC_BO + 1)) ||
					((point_type >= BAC_FLOAT_ABCD + 1) && (point_type <= BAC_FLOAT_DCBA + 1))
						)
					&& (prg_code[i][j + 3] == old) \
					// old panel number
					//&& (prg_code[i][j + 5] == Setting_Info.reg.network_number) 
					)
					{
						prg_code[i][j + 3] = new_panel;
						changed = 1;
					}
						
				}
			}
		}
	}

	
	// change panel number in monitor
	for( i = 0; i < MAX_MONITORS; i++ )
	{
		for( j = 0; j < MAX_POINTS_IN_MONITOR; j++ )
		{
			if(monitors[i].inputs[j].panel == old)
			{
				monitors[i].inputs[j].panel = new_panel;
				if(monitors[i].inputs[j].sub_id == old)
					monitors[i].inputs[j].sub_id = new_panel;
				changed = 1;
			}
		}  
	}
	
	// change panel number in graphic

	for( i = 0; i < MAX_ELEMENTS; i++ )
	{
		if(group_data[i].reg.nMain_Panel == old)
		{
			group_data[i].reg.nMain_Panel = new_panel;
			if(group_data[i].reg.nSub_Panel == old)
				group_data[i].reg.nSub_Panel = new_panel;
			changed = 1;
		}
	}
	
	if(changed)	{
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
		write_page_en[15] = 1;
#endif
		Flash_Write_Mass();	// write into flash now
	}

}

void change_panel_number_in_trendlog(U8_T old, U8_T new_panel)
{
	U8_T i;
	U16_T j;
	U8_T changed = 0;

	// change panel number in program
	for( i = 0; i < MAX_PRGS; i++ )
	{
		if( swap_word(programs[i].real_byte) > 0)
		{
			for(j = 0;j < swap_word(programs[i].real_byte);j++)
			{
				if(prg_code[i][j] == 0x9e)	// REMOTE_POINT_PRG
				{
					U8_T point_type;
					// it is not in 9D x x x x , 9e x VAR panel x network
					point_type = (prg_code[i][j + 2] & 0x1f) + (prg_code[i][j + 5] & 0x60);
					
					if(j > 4 && (prg_code[i][j - 1] != 0x9d) && (prg_code[i][j - 2] != 0x9d) \
					&& (prg_code[i][j - 3] != 0x9d) && (prg_code[i][j - 4] != 0x9d)	\
					&& (
					(point_type == VAR + 1) ||
					((point_type >= MB_COIL_REG + 1) && (point_type <= BAC_BO + 1)) ||
					((point_type >= BAC_FLOAT_ABCD + 1) && (point_type <= BAC_FLOAT_DCBA + 1))
					)
					&& (prg_code[i][j + 3] == old) \
					// old panel number
					//&& (prg_code[i][j + 5] == Setting_Info.reg.network_number) 
					)
					{
						prg_code[i][j + 3] = new_panel;
						changed = 1;
					}
						
				}
			}
		}
	}

	
	// change panel number in monitor
	for( i = 0; i < MAX_MONITORS; i++ )
	{
		for( j = 0; j < MAX_POINTS_IN_MONITOR; j++ )
		{
			if(monitors[i].inputs[j].panel == old)
			{
				monitors[i].inputs[j].panel = new_panel;
				if(monitors[i].inputs[j].sub_id == old)
					monitors[i].inputs[j].sub_id = new_panel;
				changed = 1;
			}
		}  
	}
	
	// change panel number in graphic

	for( i = 0; i < MAX_ELEMENTS; i++ )
	{
		if(group_data[i].reg.nMain_Panel == old)
		{
			group_data[i].reg.nMain_Panel = new_panel;
			if(group_data[i].reg.nSub_Panel == old)
				group_data[i].reg.nSub_Panel = new_panel;
			changed = 1;
		}
	}
	
	if(changed)	{
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
		write_page_en[15] = 1;
#endif
		Flash_Write_Mass();	// write into flash now
	}

}

extern unsigned char far Handler_Transmit_Buffer[2][MAX_PDU];

U8_T flag_mstp_source;

#if (ASIX_MINI || ASIX_CM5)

// tbd:  
void Tansfer_Mstp_pdu( uint8_t * pdu,uint16_t pdu_len)
{
	
}


#endif


#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )

uint16_t Send_Mstp(uint8_t flag,uint8_t * type)
{
	uint8_t len;

	if(flag == 2)  // transfer pdu, BIP TO MSTP
	{
		Send_Private_Flag = 0;
		*type = 4;
		return MSTP_Transfer_Len;
	}
	if(flag == 3)  // read slave mstp device
	{ 
		Send_Private_Flag = 0;
		*type = 5;
		
		return MSTP_Transfer_Len;			
	}
	if(flag == 4)  // write slave mstp device
	{ 
		Send_Private_Flag = 0;
		*type = 6;
		
		return MSTP_Transfer_Len;			
	}
	if(flag == 1)  
	{ // get panel info, pro		memset(&Handler_Transmit_Buffer[0][0],0,MAX_PDU);

		Send_Private_Flag = 0;
		
//		GetPrivateData(MSTP_Rec_Header.device_id,MSTP_Rec_Header.subcmd,\
//			MSTP_Rec_Header.start,MSTP_Rec_Header.end ,MSTP_Rec_Header.entitysize,BAC_MSTP);
		
		if(MSTP_Rec_Header.subcmd < 100) // read command, length is same
			len = 22; // MSTP_Rec_Header.total_length + 10;  MSTP_Rec_Header.total_length is only header, it is 12
		else  
			len = 10 + MSTP_Rec_Header.total_length;
		*type = 3;

		return len;			
	}
}
u32 t_1,t_2;
uint8_t count_hold_on_bip_to_mstp;  // 当yabe或者T3000软件正在访问时，不要读写下面的设备

// BIP TO MSTP
void Transfer_Bip_To_Mstp_pdu( uint8_t * pdu,uint16_t pdu_len)
{
#if (ARM_MINI || ASIX_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
	U8_T i;
	U8_T start = 0;
	if(pdu_len < 7) 
		return;
	if(pdu[10] == 0x12) // T3000 reading tstat by private transfer command
	{
		count_hold_on_bip_to_mstp = 10;		
	}
	else
	{
		if(count_hold_on_bip_to_mstp > 0)		
		{			
			return;
		}
	}

	MSTP_Transfer_Len = pdu_len + 4;
	TransmitPacket_panel = pdu[5];
	TransmitPacket[0] = pdu[0];
	TransmitPacket[1] = 0x0c;
	TransmitPacket[2] = (U8_T)(mstp_network >> 8);
	TransmitPacket[3] = mstp_network;
	TransmitPacket[4] = 0x06;
	TransmitPacket[5] = Modbus.ip_addr[0];
	TransmitPacket[6] = Modbus.ip_addr[1];
	TransmitPacket[7] = Modbus.ip_addr[2];
	TransmitPacket[8] = Modbus.ip_addr[3];
	TransmitPacket[9] = 0xba;
	TransmitPacket[10] = 0xc0;	
	
	t_1 = uip_timer;
	flag_mstp_source = 1;   // T3000 or BIP client
	
	if(Send_Private_Flag == 0)
	{
		Send_Private_Flag = 2;
		// source is bip client,if it is private 
		memcpy(&TransmitPacket[11],&pdu[7],pdu_len - 7);
	}
#endif
}
extern u8 count_transfer_bip_to_mstp;

void Transfer_Mstp_To_Bip_pdu(uint8_t src,uint8_t * pdu,uint16_t pdu_len)
{
#if (ARM_MINI || ASIX_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
	MSTP_Send_buffer[0] = 0x81;
	MSTP_Send_buffer[1] = 0x0a;
	MSTP_Send_buffer[2] = (uint8_t)((pdu_len - 2) >> 8);
	MSTP_Send_buffer[3] = (uint8_t)(pdu_len - 2);
	MSTP_Send_buffer[4] = pdu[0]; // version
	MSTP_Send_buffer[5] = 0x08; // control
	MSTP_Send_buffer[6] = pdu[2];  // mstp network
	MSTP_Send_buffer[7] = pdu[3];
	MSTP_Send_buffer[8] = 0x01; // mac layer length;
	MSTP_Send_buffer[9] = src; //  SADR
	memcpy(&MSTP_Send_buffer[10],&pdu[12],pdu_len - 12);

	// wait send_mstp_flag free
	//Send_mstp_Flag = 1;
//	send_mstp_index1 = 0;
//	rec_mstp_index1 = 0;
#if (ARM_MINI || ASIX_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
	if(rec_mstp_index1 < 9)
	{
		memcpy(mstp_bac_buf1[rec_mstp_index1].buf,MSTP_Send_buffer, pdu_len - 2);
		mstp_bac_buf1[rec_mstp_index1].len = pdu_len - 2;
	}
	
	if(rec_mstp_index1 < 10)
		rec_mstp_index1++;
	else
		rec_mstp_index1 = 0;
#endif
	//count_transfer_bip_to_mstp = 0;
#endif
}


S8_T get_rbp_index_by_invoke_id(uint8_t invokeid)
{
	char i;
	if(invokeid == 0) 
		return -1;
	for(i = 0;i < number_of_remote_points_bacnet;i++)
	{
		if(invokeid == remote_points_list_bacnet[i].invoked_id)
			return i;
	}
	
	return -1;
	
}


S8_T get_netpoint_index_by_invoke_id(uint8_t invokeid)
{
	char i;
	if(invokeid == 0) 
		return -1;
	for(i = 0;i < number_of_network_points_bacnet;i++)
	{
		if(invokeid == network_points_list_bacnet[i].invoked_id)
		{
			return i;
		}
	}
	return -1;	
}


S8_T get_netpoint_index_by_invoke_id_modbus(uint8_t invokeid)
{
	char i;
	if(invokeid == 0) 
		return -1;
	for(i = 0;i < number_of_network_points_modbus;i++)
	{
		if(invokeid == network_points_list_modbus[i].invoked_id)
		{
			return i;
		}
	}
	
	return -1;
	
}

#endif

// add by chelsea
 // response from MSTP device
void Handler_Complex_Ack(
    uint8_t * apdu,
    int apdu_len,      /* total length of the apdu */
    uint8_t protocal)
{
	float val_ptr;
	uint8_t index;
	U8_T invokeid_bip;
	S8_T far network_point_index;	
//	U8_T far remote_bacnet_index;	
		
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
	if(protocal == BAC_MSTP)
	{ 	
		if(flag_mstp_source == 1) // source is BIP client, T3000 or yabe or other panel...
		{ // it is for BIP TO MSTP, need transfer to BIP cilent
// BIP TO MSTP
			MSTP_Send_buffer[0] = 0x81;
			MSTP_Send_buffer[1] = 0x0a;
			MSTP_Send_buffer[2] = (uint8_t)((apdu_len + 6) >> 8);
			MSTP_Send_buffer[3] = (uint8_t)(apdu_len + 6);
			MSTP_Send_buffer[4] = 0x01;
			MSTP_Send_buffer[5] = 0x00;
			memcpy(&MSTP_Send_buffer[6],apdu,apdu_len);
	

	//		uip_send((char *)MSTP_Send_buffer, apdu_len + 6);	

			//Send_mstp_Flag = 1;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)	
			if(rec_mstp_index1 < 9)
			{
				memcpy(mstp_bac_buf1[rec_mstp_index1].buf,MSTP_Send_buffer, apdu_len + 6);
				mstp_bac_buf1[rec_mstp_index1].len = apdu_len + 6;
			}
			if(rec_mstp_index1 < 10)
				rec_mstp_index1++;
			else
				rec_mstp_index1 = 0;
#endif

    /* since object property == object_id, decode the application data using
       the appropriate decode function */
		}
		else if(flag_mstp_source == 2)
		{ //  packet's source is from T3-BB,dont need transfer to BIP client
			
			if(apdu[1] == invokeid_mstp && invokeid_mstp >= 0)
			{
				flag_receive_rmbp	= 1;	
//				BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE value;
//				bacapp_decode_context_device_obj_property_ref(apdu,0,&value);
//				apdu[11] = 0xac;
//				decode_context_real(&apdu[11], 10, &val_ptr);	
				if(apdu[2] == 0x12)  // ptransfer
				{// only for temoc private bac_to_modbus 
					uint8 vendorid_len;
					vendorid_len = apdu[4];
/*
30 49 12 09 00 19 00 
2e 
65 1b 
14 00 5e 00 00 0a 00 
00 9d 00 00 00 01 00 00 00 4d 00 00 00 10 00 09 00 03 00 00 
2f
*/
					if(apdu[12 + vendorid_len] == READ_BACNET_TO_MDOBUS)	
					{
						uint8_t len;
						uint16_t reg;	
						
						len = apdu[10 + vendorid_len];
						
						if(len == apdu[15 + vendorid_len] * 2)
						{	
							remote_panel_db[remote_mstp_panel_index].sn = 
								apdu[18 + vendorid_len] + ((U16_T)apdu[20 + vendorid_len] << 8) + ((U32_T)apdu[22 + vendorid_len] << 16) + ((U32_T)apdu[24 + vendorid_len] << 24);
							remote_panel_db[remote_mstp_panel_index].product_model = apdu[32 + vendorid_len];
						}
					}
				}
				else
				{
					if(apdu[11] == 0x44) // decode real
					{
						apdu[11] = 0xac;
						decode_context_real(&apdu[11], 10, &val_ptr);	
					}
					else if(apdu[11] == 0x91)  // decode ON/OFF
					{
						val_ptr = apdu[12];					
					}	

					add_remote_point(remote_points_list_bacnet[remote_bacnet_index].tb.RP_bacnet.panel,
					remote_points_list_bacnet[remote_bacnet_index].tb.RP_bacnet.object + 
					(U8_T)((remote_points_list_bacnet[remote_bacnet_index].tb.RP_bacnet.instance & 0xff00) >> 3),
					0,
					remote_points_list_bacnet[remote_bacnet_index].tb.RP_bacnet.instance & 0xff,
					val_ptr * 1000,2,0);
				}
				
			}			
		}
			
		flag_mstp_source = 0;
	}
#if (ARM_MINI || ARM_CM5)	
	else if(protocal == BAC_IP)
	{	
// response from ptranfer reading
// check invoke_id		
		invokeid_bip = apdu[1];	
		network_point_index = get_netpoint_index_by_invoke_id(invokeid_bip);
		if(network_point_index != -1)
		{
			//memcpy(&Test[2],&apdu[0],28);
//			if((apdu[1] == invokeid_bip) && (invokeid_bip > 0) )
//			{	
			flag_receive_netp = 1;
			if(apdu[11] == 0x44) // decode real
			{
				apdu[11] = 0xac;
				decode_context_real(&apdu[11], 10, &val_ptr);	

			}
			else if(apdu[11] == 0x91)  // decode ON/OFF
			{
				val_ptr = apdu[12];
			}	
			else 
			{ // temco private bacnet to modbus
				if(apdu[13] == READ_BACNET_TO_MDOBUS)
				{
					uint8_t len;
					uint16_t reg;
					len = apdu[11];
					reg = apdu[14] * 256 + apdu[15];
					if(len == apdu[16] * 2)
					{
						val_ptr = (float)((U32_T)(apdu[18] << 24) + (U32_T)(apdu[19] << 16) +
						(U16_T)(apdu[20] << 8) + apdu[21]) / 1000;
					}
				}
				else 
				{
					return;
				}
			}
			
			add_network_point( network_points_list_bacnet[network_point_index].point.panel,
			network_points_list_bacnet[network_point_index].point.sub_id,
			network_points_list_bacnet[network_point_index].point.point_type - 1,
			network_points_list_bacnet[network_point_index].point.number + 1,
			val_ptr * 1000,
			2,0);
				
//			}
		}
		else
		{ // PROPRIETARY_BACNET_OBJECT_TYPE panel number
			//char index;
			//index = get_rbp_index_by_invoke_id(remote_panel_db[0].device_id);
// read VARX or REGX	
			if(temcovar_panel_invoke == invokeid_bip)
			{	
				if(apdu[11] == 0x44) // decode real
				{	
					apdu[11] = 0xac;
					decode_context_real(&apdu[11], 10, &val_ptr);	
					flag_receive_netp_temcovar = 1;
					temcovar_panel = val_ptr;
				}
			}	
//			if(temcoreg_panel_invoke == invokeid_bip)
//			{
//				if(apdu[13] == READ_BACNET_TO_MDOBUS)
//				{
//					uint8_t len;
//					uint16_t reg;
//									

//					len = apdu[11];
//					reg = apdu[14] * 256 + apdu[15];
//					if(len == apdu[16] * 2)
//					{
//						flag_receive_netp_temcoreg = 1;
//						memcpy(&temcoreg,&apdu[19],len);
//					}
//					
//				}				
//			}
		}
		
	}
#endif
#endif
}


int local_ProcessPTA(	BACNET_PRIVATE_TRANSFER_DATA * dat, U8_T * index)
{
	int iLen = 0;   /* Index to current location in data */
	int tag_len = 0;
	uint8_t tag_number;
	uint32_t len_value_type;
	BACNET_OCTET_STRING far Temp_CS;
	int command_type;
	
	iLen = 0;
	/* Error code is returned for read and write operations */

	tag_len =  decode_tag_number_and_value(&dat->serviceParameters[iLen],   &tag_number, &len_value_type);
	iLen += tag_len;

	if (tag_number != BACNET_APPLICATION_TAG_OCTET_STRING) 
	{
		/* if (tag_number != BACNET_APPLICATION_TAG_UNSIGNED_INT) {*/
		return 0;
	}

	decode_octet_string(&dat->serviceParameters[iLen], len_value_type,&Temp_CS);
	command_type = Temp_CS.value[2];	
	if(command_type == GET_PANEL_INFO)
	{
		*index = Temp_CS.value[4];	
	}
  return command_type;

}


uint8_t Get_panel_by_deviceid(uint16_t deviceid)
{
	U8_T i;
	for(i = 0;i < remote_panel_num;i++)
	{
		if(deviceid == remote_panel_db[i].device_id)
			return remote_panel_db[i].panel;
	}
	return 0;
}

U8_T Get_current_panel(void)
{
	U8_T i;
	for(i = 0;i < remote_panel_num;i++)
	{
		if(MSTP_Rec_Header.device_id == remote_panel_db[i].device_id)
			return remote_panel_db[i].panel;
	}
	return 0;
}


extern xSemaphoreHandle sem_mstp;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )

uint16_t get_reg_from_list(uint8_t type,uint8_t index,uint8_t * len)
{
	uint16_t reg;
	switch (type)
	{
		case VAR + 1:
			reg = MODBUS_VAR_FIRST + 2 * index;
			break;
		case IN + 1:
			reg = MODBUS_INPUT_FIRST + 2 * index;
			break;
		case OUT + 1:
			reg = MODBUS_OUTPUT_FIRST + 2 * index;
			break;
		
		default:
		break;
	}
	*len = 2;
	return reg;
}

int WriteRemotePoint(uint8_t object_type,uint32_t object_instance,uint8_t panel,uint8_t sub_id,float value,uint8_t protocal)
{
	uint32 deviceid;
//	U8_T invokeid_bip;
//	U8_T invokeid_mstp;
	BACNET_APPLICATION_DATA_VALUE val;

	deviceid = Get_device_id_by_panel(panel,sub_id,protocal);
	if(deviceid > 0)	
	{	
		if(protocal == BAC_MSTP)
		{
			if(Modbus.com_config[2] == BACNET_MASTER || Modbus.com_config[0] == BACNET_MASTER)
			{
//				if(cSemaphoreTake(sem_mstp, 200) == 1)
				if(count_hold_on_bip_to_mstp > 0)									
					return -1;
				
				{			
					flag_mstp_source = 2;    // T3-CONTROLLER
					Send_Private_Flag = 4;   // send normal bacnet packet
					TransmitPacket_panel = sub_id;
//				val.tag = BACNET_APPLICATION_TAG_REAL;
//				val.context_specific = 0;
//				val.context_tag = 4;
//				val.next = 0;	
//				val.type.Real = value;
					//invokeid_mstp = 				
					Send_Write_Property_Request(deviceid,object_type,object_instance,PROP_PRESENT_VALUE,&val,0,value,BACNET_ARRAY_ALL,protocal);
				}
				
//				cSemaphoreGive(sem_mstp);	
				return 1;//invokeid_mstp;
			}
		}
#if (ARM_MINI || ASIX_MINI || ARM_CM5)
		else if(protocal == BAC_IP_CLIENT)
		{
			Send_bip_Flag = 1;	
			count_send_bip = 0;
			Get_address_by_panel(panel,Send_bip_address);						
			Send_bip_count = MAX_RETRY_SEND_BIP;			

			return	Send_Write_Property_Request(deviceid,object_type,object_instance,PROP_PRESENT_VALUE,&val,0,value,BACNET_ARRAY_ALL,protocal);
		}
#endif
	}
	return -1;

}



// get standard bacnet points, AV, AI , AO ...
/*
	type -> ANALOG_VALUE_OBJECT, ANALOG_INPUT,OBJECT ...
	
	protoal -> 1. mstp  (	remote device is mstp device)	
					-> 2. bip (	network decive is bacnet device)

*/
int GetRemotePoint(uint8_t object_type,uint32_t object_instance,uint8_t panel,uint8_t sub_id,uint8_t protocal)
{
	uint32 deviceid;
	U8_T invokeid_bip;
//	U8_T invokeid_mstp;
	invokeid_bip = -1;
	deviceid = Get_device_id_by_panel(panel,sub_id,protocal);
	
	if(deviceid > 0)
	{	
		if(protocal == BAC_MSTP)
		{
			if(count_hold_on_bip_to_mstp > 0)		
			{				
					return -1;
			}
			flag_mstp_source = 2;   // T3-CONTROLLER
			Send_Private_Flag = 3;   // send normal bacnet packet
			TransmitPacket_panel = sub_id;
			invokeid_mstp = Send_Read_Property_Request(deviceid,object_type,object_instance,PROP_PRESENT_VALUE,0,protocal);
			return invokeid_mstp;
		}
#if !(ARM_TSTAT_WIFI )
		else if(protocal == BAC_IP_CLIENT)
		{
			Send_bip_Flag = 1;	
			count_send_bip = 0;
			Get_address_by_panel(panel,Send_bip_address);	
			Send_bip_count = MAX_RETRY_SEND_BIP;
			
			if(object_type == VAR + 1 || object_type == IN + 1 || object_type == OUT + 1)
			{
				uint16_t databuf[10];
				uint16_t reg;
				uint8_t len;
				reg = get_reg_from_list(object_type,object_instance,&len);				
				invokeid_bip = GetPrivateBacnetToModbusData(deviceid,reg,len,databuf,BAC_IP_CLIENT);
			}
			else if(object_type == BAC_AV + 1)
			{
				invokeid_bip = Send_Read_Property_Request(deviceid,OBJECT_ANALOG_VALUE,object_instance/* + 1*/,PROP_PRESENT_VALUE,0,protocal);
			}
			else if(object_type == BAC_AI + 1)
			{
				invokeid_bip = Send_Read_Property_Request(deviceid,OBJECT_ANALOG_INPUT,object_instance/* + 1*/,PROP_PRESENT_VALUE,0,protocal);
			}			
			else if((object_type == BAC_AO + 1) || ((object_type == OUT + 1) && (object_instance >= max_dos)))
			{
				if(object_type == BAC_AO + 1)
					invokeid_bip = Send_Read_Property_Request(deviceid,OBJECT_ANALOG_OUTPUT,object_instance/* + 1*/,PROP_PRESENT_VALUE,0,protocal);
				else
				{
					invokeid_bip = Send_Read_Property_Request(deviceid,OBJECT_ANALOG_OUTPUT,object_instance - max_dos/* + 1*/,PROP_PRESENT_VALUE,0,protocal);
				}
			}
			else if((object_type == BAC_BO + 1) || ((object_type == OUT + 1) && (object_instance < max_dos)))
			{
				invokeid_bip = Send_Read_Property_Request(deviceid,OBJECT_BINARY_OUTPUT,object_instance/* + 1*/,PROP_PRESENT_VALUE,0,protocal);
			}
			else	if(object_type == BAC_BV + 1)
			{
				invokeid_bip = Send_Read_Property_Request(deviceid,OBJECT_BINARY_VALUE,object_instance/* + 1*/,PROP_PRESENT_VALUE,0,protocal);
			}
			else if(object_type == BAC_BI + 1)
			{
				invokeid_bip = Send_Read_Property_Request(deviceid,OBJECT_BINARY_INPUT,object_instance/* + 1*/,PROP_PRESENT_VALUE,0,protocal);
			}
			
			return invokeid_bip;
		}
#endif
	}
	return -1;
}

void handler_conf_private_trans_ack(
    uint8_t * service_request,
    uint16_t service_len,
    uint8_t * apdu,
    int apdu_len,
		uint8_t protocal	)
{

    BACNET_PRIVATE_TRANSFER_DATA far dat;
    int len;
	  U8_T command;
	  U8_T index;
//	 bool each_end_flag = false;
/*
 * Note:
 * We currently don't look at the source address and service data
 * but we probably should to verify that the ack is oneit is what
 * we were expecting. But this is just to silence some compiler
 * warnings from Borland.
 */
//	src = src;
//	service_data = service_data;

	len = 0;
	len = ptransfer_decode_service_request(service_request, service_len, &dat);        /* Same decode for ack as for service request! */
		
	command = local_ProcessPTA(&dat,&index);

	if(dat.serviceParametersLen == 0)  // write cmd response 0
	{		
		MSTP_Write_OK = 1;
	}
 if(dat.serviceParametersLen < 500)
 {

	 memcpy(&MSTP_Send_buffer[0],dat.serviceParameters,dat.serviceParametersLen);
	 MSTP_buffer_len = dat.serviceParametersLen;
	 
	if(command == GET_PANEL_INFO)
	{
		Str_Panel_Info *ptr;
		U8_T remote_i;
		U32_T device_id;
		
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )		
		device_id = ptr->reg.instance * 65536L + ptr->reg.instance_hi;
#else
		device_id = swap_word(ptr->reg.instance_hi) * 65536L + swap_word(ptr->reg.instance);
#endif		
		
		ptr = (Str_Panel_Info *)&MSTP_Send_buffer[9];
		
		// get remote_index by device_id
		
		if(Get_remote_index_by_device_id(device_id,&remote_i) != -1)
		{
			remote_panel_db[remote_i].sub_id = ptr->reg.modbus_addr;
			remote_panel_db[remote_i].product_model = ptr->reg.product_type;
			remote_panel_db[remote_i].sn =  ptr->reg.serial_num[0] + (U16_T)(ptr->reg.serial_num[1] << 8)
					+ (U32_T)(ptr->reg.serial_num[2] << 16) + (U32_T)(ptr->reg.serial_num[3] << 24);
			remote_panel_db[remote_i].panel = ptr->reg.panel_number;	
		
			remote_panel_db[remote_i].device_id = device_id;
			
		
		}
	}
	else
	{	
		Handler_Complex_Ack(apdu,apdu_len,protocal);
	}
	 
 }
 else
 {
	MSTP_buffer_len = 0;
 }

}



/*
Author: Fance
Get  Private  Bacnet  To  ModbusData
//????????bacnet????modbus ??????
*/
/************************************************************************/

typedef struct
{
    uint16_t start_add;
    uint16_t nlength;
    uint16_t org_start_add;
    uint16_t org_nlength;  
    uint16_t ndata[400];
}Str_modbus_reg;

typedef struct
{
    uint16_t  		total_length;        /*	total length to be received or sent	*/
    uint8_t		command;
    uint16_t    start_reg;            //??????
    uint16_t		nlength;          //?????

}Str_bacnet_to_modbus_header;

Str_modbus_reg bacnet_to_modbus_struct; 

int GetPrivateBacnetToModbusData(uint32_t deviceid, uint16_t start_reg, int16_t readlength, uint16_t *data_out,uint8_t protocal)
{
    int n_ret = 0;

    uint8_t apdu[50] = { 0 };
    uint8_t test_value[50] = { 0 };
    int apdu_len = 0;
    int private_data_len = 0;
    unsigned short max_apdu = 0;
    BACNET_APPLICATION_DATA_VALUE data_value = { 0 };
    BACNET_PRIVATE_TRANSFER_DATA private_data = { 0 };
    bool status = false;
//		char SendBuffer[1000];
 //   char * temp_buffer = SendBuffer;
    Str_bacnet_to_modbus_header private_data_chunk;
    int HEADER_LENGTH = USER_DATA_HEADER_LEN;
    BACNET_ADDRESS dest = { 0 };
		int i;
		
		bacnet_to_modbus_struct.org_nlength = readlength;
    bacnet_to_modbus_struct.org_start_add = start_reg;
		
    private_data.vendorID = BACNET_VENDOR_ID;
    private_data.serviceNumber = 1;

//    memset(SendBuffer, 0, 1000);


    HEADER_LENGTH = USER_DATA_HEADER_LEN;
    private_data_chunk.total_length = USER_DATA_HEADER_LEN;
    private_data_chunk.command = READ_BACNET_TO_MDOBUS;
    private_data_chunk.start_reg = start_reg; // ??
    private_data_chunk.nlength = readlength;
    transfer_len = 0;
    status = bacapp_parse_application_data(BACNET_APPLICATION_TAG_OCTET_STRING, (char *)&private_data_chunk, &data_value);

    private_data_len = bacapp_encode_application_data(&test_value[0], &data_value);
    private_data.serviceParameters = &test_value[0];
    private_data.serviceParametersLen = private_data_len;

    status = address_get_by_device(deviceid, &max_apdu, &dest);
    if (status)
    { 
        return Send_ConfirmedPrivateTransfer(&dest, &private_data,BAC_IP_CLIENT);
    }

    return -7;
}



int WritePrivateBacnetToModbusData(uint32_t deviceid, int16_t start_reg, uint16_t writelength, uint32_t data_in)
{
    uint8_t temp_data[50];
    int n_ret = 0;


    uint8_t apdu[50] = { 0 };
    uint8_t test_value[50] = { 0 };
    int apdu_len = 0;
    int private_data_len = 0;
    unsigned short max_apdu = 0;
    BACNET_APPLICATION_DATA_VALUE data_value = { 0 };
    BACNET_PRIVATE_TRANSFER_DATA private_data = { 0 };
    bool status = false;
    char SendBuffer[50];
    char * temp_buffer = SendBuffer;
    int HEADER_LENGTH = USER_DATA_HEADER_LEN;
		int i;
    BACNET_ADDRESS dest = { 0 };
		Str_bacnet_to_modbus_header private_data_chunk;
    if ((writelength == 0) || (writelength > 128))
        return -4; //????;

    memset(temp_data, 0, 50);

    private_data.vendorID = BACNET_VENDOR_ID;
    private_data.serviceNumber = 1;

    header_len = USER_DATA_HEADER_LEN;
    private_data_chunk.total_length = USER_DATA_HEADER_LEN + writelength*2;
    private_data_chunk.command = WRITE_BACNET_TO_MDOBUS;
    private_data_chunk.start_reg = start_reg; // ??
    private_data_chunk.nlength = writelength;

    transfer_len =  writelength * 2;
		memcpy(SendBuffer, &private_data_chunk, USER_DATA_HEADER_LEN);
    memcpy(temp_data, &data_in, writelength * 2);
		
// only for write value
		{
			uint8_t temp[4];
			temp[0] = temp_data[3];
			temp[1] = temp_data[2];
			temp[2] = temp_data[1];
			temp[3] = temp_data[0];
			
			memcpy(temp_data,temp,4);

		}

		memcpy(SendBuffer + USER_DATA_HEADER_LEN, &temp_data, writelength * 2);


    status = bacapp_parse_application_data(BACNET_APPLICATION_TAG_OCTET_STRING, (char *)&SendBuffer, &data_value);

    private_data_len = bacapp_encode_application_data(&test_value[0], &data_value);
    private_data.serviceParameters = &test_value[0];
    private_data.serviceParametersLen = private_data_len;
#if !(ARM_TSTAT_WIFI )
		Send_bip_Flag = 1;
		count_send_bip = 0;
		Send_bip_count = MAX_RETRY_SEND_BIP;	
#endif
    status = address_get_by_device(deviceid, &max_apdu, &dest);
    if (status)
    {
		    return Send_ConfirmedPrivateTransfer(&dest, &private_data,BAC_IP_CLIENT);
			
    }
    else
        return -2;


}


int Send_Ptransfer_to_Sub(U8_T *p, U16_T length,U8_T port)
{	
	Str_user_data_header head;
	BACNET_PRIVATE_TRANSFER_DATA private_data = { 0 };
	BACNET_APPLICATION_DATA_VALUE data_value = { 0 };
	uint8_t far test_value[MAX_APDU] = { 0 };
	bool status = false;
	int private_data_len = 0;	
	BACNET_ADDRESS dest;
	uint8_t far temp[500];
	int i = 0;  /* counter */

	private_data.vendorID = 148;//BACNET_VENDOR_ID;
	private_data.serviceNumber = 1;	
	
	if(p[0] == 0x10)  // multi-write
	{
		head.total_length = length - 9 + 7;
		head.command = 194;
		head.point_start_instance = p[2];
		head.point_end_instance = p[3];
		head.entitysize = 1;
		
		memcpy(&temp[0],&head,sizeof(Str_user_data_header));  // size of structrue of header is 7
		memcpy(&temp[7],&p[7],p[6]);
	}
	else if(p[0] == 0x03 && length == 8)
	{
		head.total_length = 7;
		head.command = 94;
		head.point_start_instance = p[2];
		head.point_end_instance = p[3];
		head.entitysize = 1;
		memcpy(&temp[0],&head,sizeof(Str_user_data_header));  // size of structrue of header is 7
	}
	else if(p[0] == 0x06 && length == 8)
	{
		head.total_length = 9;
		head.command = 194;
		head.point_start_instance = p[2];
		head.point_end_instance = p[3];
		head.entitysize = 1;
		
		memcpy(&temp[0],&head,sizeof(Str_user_data_header));  // size of structrue of header is 7
		memcpy(&temp[7],&p[4],2);
	}
	
	
	header_len = USER_DATA_HEADER_LEN;	
	transfer_len = head.total_length;
	status = bacapp_parse_application_data(BACNET_APPLICATION_TAG_OCTET_STRING,(char *)&temp, &data_value);
	private_data_len = bacapp_encode_application_data(&test_value[0], &data_value);
	private_data.serviceParameters = &test_value[0];
	private_data.serviceParametersLen = private_data_len;


	dest.mac_len = 1;
	dest.mac[0] = p[0];  // sub id
	dest.net = 0;        /* local only, no routing */
	dest.len = 0;
	for (i = 0; i < MAX_MAC_LEN; i++) {
			dest.adr[i] = 0;
	}
	

	if(count_hold_on_bip_to_mstp > 0)									
		return -1;
	flag_mstp_source = 2;   // T3-CONTROLLER
	Send_Private_Flag = 3;   // send normal bacnet packet
	TransmitPacket_panel = p[0];

	return Send_ConfirmedPrivateTransfer(&dest,&private_data,BAC_MSTP);
}


#endif
uint8_t temp[MAX_APDU] = {0};
BACNET_OCTET_STRING Temp_CS;			
uint8_t test_value[MAX_APDU] = { 0 };

void handler_private_transfer( 	
	uint8_t * apdu,
  unsigned apdu_len,
	BACNET_ADDRESS * src,uint8_t protocal)/*,
    BACNET_PRIVATE_TRANSFER_DATA * private_data */ 
{
    BACNET_APPLICATION_DATA_VALUE data_value = { 0 };
    BACNET_APPLICATION_DATA_VALUE rec_data_value = { 0 };
    BACNET_PRIVATE_TRANSFER_DATA private_data = { 0 };
		BACNET_PRIVATE_TRANSFER_DATA rec_data = { 0 };
		Str_user_data_header	private_header;	
		
		uint8_t* ptr = NULL;
		int len = 0;
		int private_data_len = 0;
		int property_len = 0;
		//    BACNET_NPDU_DATA npdu_data;
		int bytes_sent = 0;
		bool status = false;
		
		uint8_t command = 0;
		uint8_t packet_index;
		uint8_t flag_write_pic;
		
		uint8_t flag_read_monitor;
		uint8_t delay_write_setting;
		
	 U8_T j;

	 int iLen;   /* Index to current location in data */
   int tag_len;
	 uint8_t tag_number;
	 uint32_t len_value_type;
//   decode ptransfer
		delay_write_setting = 0;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
		if(protocal < 0xa0)  // mstp or bip
#endif
		{
			len = ptransfer_decode_apdu(&apdu[0], apdu_len, &invoke_id, &rec_data);
			iLen = 0;
		//ok	
				/* Error code is returned for read and write operations */

			tag_len =
					decode_tag_number_and_value(&rec_data.serviceParameters[iLen],
					&tag_number, &len_value_type);
			iLen += tag_len;

			if(tag_number == BACNET_APPLICATION_TAG_OCTET_STRING)
			{
				decode_octet_string(&rec_data.serviceParameters[iLen], len_value_type,&Temp_CS);

			}
			private_data.vendorID =  rec_data.vendorID;
			private_data.serviceNumber = rec_data.serviceNumber;
		}
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
		else  // TEMCO modbus private
		{
			if((apdu[7] + apdu[8] * 256) > (MAX_APDU - 6)) 
				return;
			memcpy(&Temp_CS.value[0],&apdu[7],apdu[7] + apdu[8] * 256);	

		}
#endif
			 //bacapp_decode_application_data(rec_data.serviceParameters,
			 //    rec_data.serviceParametersLen, &rec_data_value);	
		command = Temp_CS.value[2];
		if( command  == READMONITORDATA_T3000 || command  == UPDATEMEMMONITOR_T3000
			|| command == READPIC_T3000 || command == WRITEPIC_T3000)
		{
			 header_len = 26;//18;

			 Graphi_data->command = Temp_CS.value[2];
			 Graphi_data->index = Temp_CS.value[3];	  // monitor table index
			 Graphi_data->sample_type = Temp_CS.value[4];  // 1 - an 0 - digital

			 memcpy(Graphi_data->comm_arg.string,&Temp_CS.value[5],4);  // size oldest_time most_recent_time
			 memcpy(Graphi_data->comm_arg.string + 4,&Temp_CS.value[9],4);
			 memcpy(Graphi_data->comm_arg.string + 8,&Temp_CS.value[13],4);

			 Graphi_data->comm_arg.monupdate.size = swap_double(Graphi_data->comm_arg.monupdate.size);
			 Graphi_data->comm_arg.monupdate.most_recent_time = swap_double(Graphi_data->comm_arg.monupdate.most_recent_time);
			 Graphi_data->comm_arg.monupdate.oldest_time = swap_double(Graphi_data->comm_arg.monupdate.oldest_time);
		 
			 Graphi_data->special = Temp_CS.value[17];
			 Graphi_data->total_seg = Temp_CS.value[22] + ((U16_T)Temp_CS.value[23] << 8) + ((U32_T)Temp_CS.value[24] << 16) + ((U32_T)Temp_CS.value[25] << 24);
			 Graphi_data->seg_index = Temp_CS.value[18] + ((U16_T)Temp_CS.value[19] << 8) + ((U32_T)Temp_CS.value[20] << 16) + ((U32_T)Temp_CS.value[21] << 24);
		}
		else
		{
			
			private_header.total_length = Temp_CS.value[1] * 256 + Temp_CS.value[0];
			private_header.command = Temp_CS.value[2];
			private_header.point_start_instance = Temp_CS.value[3];
			private_header.point_end_instance = Temp_CS.value[4];
			
			private_header.entitysize = (U16_T)((Temp_CS.value[6] & 0x01)	<< 8)	+ Temp_CS.value[5];
		
			if(private_header.command == WRITEPROGRAMCODE_T3000 || private_header.command == READPROGRAMCODE_T3000)
			{  // the fisrt 7 bits is for code_packet_index;
				packet_index = (Temp_CS.value[6] & 0xfe) >> 1;
			}
			header_len = USER_DATA_HEADER_LEN;			
		}
	
		
	if(command > 100)   // write command
	{
		transfer_len = 0;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
		if(protocal < 0xa0)  // mstp or bip
		{}
		else
		{
			memcpy(&temp,&apdu[0],14);
		}
#endif
		if(command ==  WRITEPRGFLASH_COMMAND)   // other commad
		{		
			ChangeFlash = 2;
		} 
		else
		{
			if((command != WRITETIME_COMMAND) && 
				(command != WRITE_SETTING))
			{
				ChangeFlash = 1;	
			}
		// TBD: add more write command
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
			if(command - WRITEOUTPUT_T3000 < MAX_POINT_TYPE)
			{
				write_page_en[command - WRITEOUTPUT_T3000] = 1;	
			}
			if(command == WRITE_ZONE_T3000)
			{
				write_page_en[ID_ROUTION] = 1;
			}
			else if(command == WRITEPROGRAMCODE_T3000)  // if CODE changed, enable monitor write
			{
				write_page_en[WRITEPROGRAM_T3000 - WRITEOUTPUT_T3000] = 1;	
			}
			if(command == WRITETABLE_T3000)  // store customer table
			{
				write_page_en[TBL] = 1;
			}
			if(command == WRITEGROUPELEMENTS_T3000)  // store group elmemts
			{
				write_page_en[GRP_POINT] = 1;	
			}
			if(command == WRITE_SCHEDULE_FLAG) 
			{
				write_page_en[24] = 1; // store schedule flag
			}
			
#endif	
			switch(command)
			{
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
				case WRITE_BACNET_TO_MDOBUS:
					ptr = (uint8_t *)(&bacnet_to_modbus);				
					break;
#endif
				case WRITEINPUT_T3000:
					if(private_header.point_end_instance <= MAX_INS)
					ptr = (uint8_t *)(&inputs[private_header.point_start_instance]);				
					break;	
				case WRITEOUTPUT_T3000:
					if(private_header.point_end_instance <= MAX_OUTS)
					ptr = (uint8_t *)(&outputs[private_header.point_start_instance]);
					break;
				case WRITEVARIABLE_T3000:        /* write variables  */
					if(private_header.point_end_instance <= MAX_VARS)
					ptr = (uint8_t *)(&vars[private_header.point_start_instance]);
					break;
			 	case WRITEWEEKLYROUTINE_T3000:         /* write weekly routines*/
					if(private_header.point_end_instance <= MAX_WR)
					ptr = (uint8_t *)(&weekly_routines[private_header.point_start_instance]);
					//check_weekly_routines();
					break;
			 	case WRITEANNUALROUTINE_T3000:         /* write annual routines*/
					if(private_header.point_end_instance <= MAX_AR)
					ptr = (uint8_t *)(&annual_routines[private_header.point_start_instance]);
					//check_annual_routines();
					break;
			 	case WRITEPROGRAM_T3000:
					if(private_header.point_end_instance <= MAX_PRGS)
					ptr = (uint8_t *)(&programs[private_header.point_start_instance]);
					break;	
				case WRITEPROGRAMCODE_T3000:
					if(private_header.point_end_instance <= MAX_PRGS)
					ptr = (uint8_t *)(&prg_code[private_header.point_start_instance][CODE_ELEMENT * packet_index]);
					break;
				case WRITETIMESCHEDULE_T3000:
					if(private_header.point_end_instance <= MAX_WR)
					ptr = (uint8_t *)(wr_times[private_header.point_start_instance]);
					break;
				case WRITE_SCHEDULE_FLAG: 
					if(private_header.point_end_instance <= MAX_WR)
					ptr = (uint8_t *)(wr_time_on_off[private_header.point_start_instance]);
					break;
				case WRITEANNUALSCHEDULE_T3000:
					if(private_header.point_end_instance <= MAX_AR)
					ptr = (uint8_t *)(ar_dates[private_header.point_start_instance]);
					break;
				case WRITETIME_COMMAND:
					ptr = (uint8_t *)(Rtc2.all);
					break;
				case WRITECONTROLLER_T3000:
					if(private_header.point_end_instance <= MAX_CONS)
					ptr = (uint8_t *)&controllers[private_header.point_start_instance];
					break;

				case WRITEMONITOR_T3000 :
					if(private_header.point_end_instance <= MAX_MONITORS)
					ptr = (uint8_t *)&monitors[private_header.point_start_instance];
					break;

			 	case WRITESCREEN_T3000  :   //CONTROL_GROUP
					if(private_header.point_end_instance <= MAX_GRPS)
					ptr = (uint8_t *)&control_groups[private_header.point_start_instance];
					break;
				case WRITEGROUPELEMENTS_T3000:
					if(private_header.point_end_instance <= MAX_ELEMENTS)
					ptr = (uint8_t *)(&group_data[private_header.point_start_instance]);
					break;

				case WRITEREMOTEPOINT:
					if(private_header.point_end_instance <= MAXREMOTEPOINTS)
					ptr = (uint8_t *)(&remote_points_list_modbus[private_header.point_start_instance]);
					break;
					break;
//
				case WRITE_SETTING:
					ptr = (uint8_t *)(&Setting_Info.all[0]);
					break;
				case WRITEALARM_T3000:
					if(private_header.point_end_instance <= MAX_ALARMS)
					ptr = (uint8_t *)&alarms[private_header.point_start_instance];
			    	break;
				case WRITEUNIT_T3000:
					if(private_header.point_end_instance <= MAX_DIG_UNIT)
					ptr = (uint8_t *)&digi_units[private_header.point_start_instance];
			    	break;
				case WRITETABLE_T3000:
					//if(private_header.point_end_instance <= MAX_DIG_UNIT)
					ptr = (uint8_t *)&custom_tab[private_header.point_start_instance];
			    	break;
				case WRITEUSER_T3000:
					if(private_header.point_end_instance <= MAX_PASSW)
					ptr = (uint8_t *)&passwords[private_header.point_start_instance];
			    	break;
				case WRITE_MISC:
					ptr = (uint8_t *)(MISC_Info.all);	
					break;
				case WRITE_SPECIAL_COMMAND:
					ptr = (uint8_t *)(Write_Special.all);	
					break;
				case WRITEVARUNIT_T3000:
					write_page_en[25] = 1;
					ptr = (uint8_t *)(var_unit);
					break;
//				case WRITEWEATHER_T3000:
//					ptr = (char *)(&weather);
//					break;
				case WRITEEXT_IO_T3000:					
					ptr = (uint8_t *)(extio_points);	
				// update database
				 
					break;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
				case WRITE_ZONE_T3000:
					ptr = (uint8_t *)(&ID_Config[private_header.point_start_instance]);
					break;
#endif
#if 0
#if STORE_TO_SD
				case WRITEPIC_T3000:
					ptr = (uint8_t *)(Graphi_data->asdu);
					memcpy(ptr,&Temp_CS.value[header_len],400);
				 	//Write_Picture(Graphi_data->index,Graphi_data->asdu,Graphi_data->seg_index);
					flag_write_pic = store_PIC_to_buffer(Graphi_data->asdu,Graphi_data->index,Graphi_data->seg_index);					
					break;
#endif
#if USB_HOST
 				case WRITE_AT_CMD:
					ptr = (uint8_t *)gsm_str;
					break;
#endif	
#endif				

#if 1//STORE_TO_SD
				case CLEAR_MONITOR:
					// check whether clear moniotr
					{
						U8_T i;
						for(i = 0;i < MAX_MONITORS * 2;i++)
						{
							if(Temp_CS.value[header_len + i] == 1) // clear current monitor
							{ // clear current monitor
								SD_block_num[i] = 0;	
								if(i % 2 == 0)
								{// analog data
									E2prom_Write_Byte(EEP_SD_BLOCK_A1 + i,HIGH_BYTE(SD_block_num[i]));
									E2prom_Write_Byte(EEP_SD_BLOCK_A1 + i + 1,LOW_BYTE(SD_block_num[i]));	
									E2prom_Write_Byte(EEP_SD_BLOCK_HI1 + i / 2,0);	
								}
								else // digital data
								{
									E2prom_Write_Byte(EEP_SD_BLOCK_D1 + i - 1,HIGH_BYTE(SD_block_num[i]));
									E2prom_Write_Byte(EEP_SD_BLOCK_D1 + i,LOW_BYTE(SD_block_num[i]));	
									E2prom_Write_Byte(EEP_SD_BLOCK_HI1 + i / 2,0);	
								}
								
								//clear_sd_file(i);
							}
							
						}
					}
					break;
#endif
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
				case WRITE_MSV_COMMAND:			
					write_page_en[25] = 1;
					ptr = (uint8_t *)&msv_data[private_header.point_start_instance];
					break;				
			case WRITE_EMAIL_ALARM:
					write_page_en[4] = 1;
					ptr = (uint8_t *)&Email_Setting;	
					break;
#endif
				default:
					break;	
					
			} 
				
			// write
			if(ptr != NULL)	
			{	 
				status = true;
				//				TST_INFO old_tst_info;
#if 1//(ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
				if(command == WRITE_BACNET_TO_MDOBUS)
				{
					memcpy(ptr,&Temp_CS.value[header_len],private_header.total_length - header_len);
					// WRITE_BACNET_TO_MDOBUS
					Get_Pkt_Bac_to_Modbus(&private_header);
				}
				else
#endif
				if(private_header.total_length  == private_header.entitysize * (private_header.point_end_instance - private_header.point_start_instance + 1) + header_len)
				{	// check is length is correct 
					if(command != WRITEOUTPUT_T3000)
						memcpy(ptr,&Temp_CS.value[header_len],private_header.total_length - header_len);
					if(command == WRITEVARIABLE_T3000)
					{
//						Count_Object_Number(OBJECT_ANALOG_VALUE);
//						Count_Object_Number(OBJECT_BINARY_VALUE);
						Count_VAR_Object_Number();
						// check MSV state
						
					}	
					
					if(command == WRITEWEEKLYROUTINE_T3000)          /* write weekly routines*/	
					{		
						check_weekly_routines();						
					}
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
					if(command == WRITEUSER_T3000)
					{
						//if(Modbus.network_master == 1)
						{// to bo check, when load prg will be dead
//									Send_UserList_Broadcast(private_header.point_start_instance,private_header.point_end_instance);
						}
					}
					if(command == WRITETIMESCHEDULE_T3000) 
					{
						// check whether update Sch_To_T8[].f_schedule_sync
						U8_T k,j;
						for(k = 0;k < 100;k++)
						{
							if(Sch_To_T8[k].schedule_index  == private_header.point_start_instance + 1)
							{
								Sch_To_T8[k].f_schedule_sync = 1;
								Sch_To_T8[k].count_send_schedule = 0;
								break;
							}
						}


//						for(k = private_header.point_start_instance;k <= private_header.point_end_instance;k++)
//						{
//							for(j = 0;j < MAX_SCHEDULES_PER_WEEK;j++)
//							{
//								Check_wr_time_on_off(k,j,0);
//							}
//						}

					}
#endif
					if(command == WRITEANNUALROUTINE_T3000)           /* write annual routines*/					
						check_weekly_routines();
					if(command == WRITEINPUT_T3000)
					{	 
						char i;			
						i = private_header.point_start_instance;
						if(private_header.point_start_instance == private_header.point_end_instance)							
						{							
							// ONLY FOR NEW T3, work as external IO card
//							if(inputs[i].sub_product == PM_T38AI8AO6DO || inputs[i].sub_product == PM_T322AI)
//							{
//								write_parameters_to_nodes(0x10,inputs[i].sub_id,MODBUS_INPUT_BLOCK_FIRST + inputs[i].sub_number * ((sizeof(Str_in_point) + 1) / 2),\
//									(uint16*)&inputs[i],((sizeof(Str_in_point) + 1)));						
//								
//							}
#if  T3_MAP
							push_expansion_in_stack(&inputs[i]);
#endif

						}	
#if (ARM_MINI || ASIX_MINI || ARM_TSTAT_WIFI)
						for(i = private_header.point_start_instance;i <= private_header.point_end_instance;i++)
						{					
							if(inputs[i].auto_manual == 1)  // manual
							{
								if((inputs[i].range == HI_spd_count) || (inputs[i].range == N0_2_32counts)
									|| (inputs[i].range == RPM)	)
								{							
									if(swap_double(inputs[i].value) == 0) 
									{
										high_spd_counter[i] = 0; // clear high spd count	
#if ARM_TSTAT_WIFI
										high_spd_counter_tempbuf[i] = 0;
#endif
										Input_RPM[i] = 0;
										clear_high_spd[i] = 1;
									}											
								}
							}							
						}	
					
#endif
						Count_IN_Object_Number();
					}
					if(command == WRITEOUTPUT_T3000)
					{	 
						char i;	

						// suspend output task			
						// if expasion io
#if (ARM_MINI || ASIX_MINI || ARM_CM5)
						vTaskSuspend(Handle_Scan);	// dont not read expansion io
						// if local io

						vTaskSuspend(xHandler_Output);  // do not control local io
#endif
						memcpy(ptr,&Temp_CS.value[header_len],private_header.total_length - header_len);
				
						for(i = private_header.point_start_instance;i <= private_header.point_end_instance;i++)
						//i = private_header.point_start_instance;
						//if(private_header.point_start_instance == private_header.point_end_instance)
						{		
							check_output_priority_array(i,0);
#if OUTPUT_DEATMASTER
			clear_dead_master();
#endif
							// if extern io, write extern output
#if  T3_MAP	
							//if(outputs[i].sub_product == PM_T38AI8AO6DO || outputs[i].sub_product == PM_T36CTA || outputs[i].sub_product == PM_T3LC )  // new T3
							if(i < base_out)
							{		
								push_expansion_out_stack(&outputs[i],i,1);								
							}

#endif
									
						}							
						// resume output task
						// if local io	
#if (ARM_MINI || ASIX_MINI || ARM_CM5)
						vTaskResume(xHandler_Output); 

						// if expasion io		
						
						vTaskResume(Handle_Scan);							
#endif
//						Count_Object_Number(OBJECT_ANALOG_OUTPUT);
//						Count_Object_Number(OBJECT_BINARY_OUTPUT);
						Count_OUT_Object_Number();
					}
#if  T3_MAP	
					if(command == WRITEEXT_IO_T3000)
						update_extio_to_database();
#endif
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
					if(command == WRITE_ZONE_T3000)
					{
						if(ID_Config_Sche[private_header.point_start_instance] != ID_Config[private_header.point_start_instance].Str.schedule)
						{
							ID_Config_Sche[private_header.point_start_instance] = ID_Config[private_header.point_start_instance].Str.schedule;
							Sch_To_T8[private_header.point_start_instance].f_schedule_sync = 1;
							Sch_To_T8[private_header.point_start_instance].count_send_schedule = 0;
							Sch_To_T8[private_header.point_start_instance].f_time_sync = 1;						
						}
						udpate_zone_table(private_header.point_start_instance);
					}
#endif
					if(command == WRITEPROGRAMCODE_T3000)
					{	
						flag_writing_code = 1;
						count_wring_code = 5;
						programs[private_header.point_start_instance].bytes = swap_word(prg_code[private_header.point_start_instance][1] * 256 + 
							prg_code[private_header.point_start_instance][0]);			
						
						if(/*packet_index == 0 && */programs[private_header.point_start_instance].bytes == 0)
						{
							programs[private_header.point_start_instance].real_byte = 0;
						}
						else
						{
							//programs[private_header.point_start_instance].real_byte = swap_word(private_header.total_length - header_len + CODE_ELEMENT * packet_index);
							programs[private_header.point_start_instance].real_byte = swap_word((swap_word(programs[private_header.point_start_instance].bytes) / CODE_ELEMENT + 1) * CODE_ELEMENT);
					
						}												
						if(private_header.total_length - header_len + CODE_ELEMENT * packet_index > CODE_ELEMENT * MAX_CODE)
						{
							programs[private_header.point_start_instance].real_byte = 0;
						}
						
							/* recount code lenght once update program code */ 	
						Code_total_length = 0;
						for(j = 0;j < MAX_PRGS;j++)
						{		
							Code_total_length += swap_word(programs[j].real_byte);
							if(Code_total_length >= MAX_CODE_SIZE)
									Code_total_length = MAX_CODE_SIZE;
						}
						
					}
					
#if 1//STORE_TO_SD
					else if(command == WRITEMONITOR_T3000)
					{ 
//						char i;	
//						i = private_header.point_start_instance;
//						if(private_header.point_start_instance == private_header.point_end_instance)							
							dealwithMonitor(private_header.point_start_instance);
					}					
#endif


					else if(command == WRITETIME_COMMAND)
					{ 
						// update current panel	
						Get_RTC_by_timestamp(swap_double(Rtc2.NEW.timestamp),&t,&Rtc,1);
						// time sync with PC 
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
						Setting_Info.reg.update_time_sync_pc = 0;
						//flag_Updata_Clock = 1;
						//Rtc_Set(Rtc.Clk.year,Rtc.Clk.mon,Rtc.Clk.day,Rtc.Clk.hour,Rtc.Clk.min,Rtc.Clk.sec,0);
						Rtc_Set(Rtc.Clk.year,Rtc.Clk.mon,Rtc.Clk.day,Rtc.Clk.hour,Rtc.Clk.min,Rtc.Clk.sec,0);
						RTC_Get();
#endif


						// time sync other panel
#if (ASIX_MINI || ASIX_CM5)
						Rtc_Set(Rtc.Clk.year,Rtc.Clk.mon,Rtc.Clk.day,Rtc.Clk.hour,Rtc.Clk.min,Rtc.Clk.sec,0);
						//Updata_Clock(0);
						flag_Updata_Clock = 1;
#endif

#if TIME_SYNC	

#if (ASIX_MINI || ASIX_CM5)
						TimeSync(); 
#endif

#if ARM_MINI 
						Send_Time_Sync_To_Network();
#endif
						
#endif

					}
					else if(command == WRITE_SETTING)
					{	
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
						write_page_en[24] = 1;
#endif
						// deal with writen_setting
						// 如果当前是串口，延时操作波特率的修改
						//if(protocal < 0xa0)
							dealwith_write_setting(&Setting_Info);
//						else
//						{
//							// delay_write_setting
//							delay_write_setting = 1;
//						}
					}
					else if(command == WRITEALARM_T3000)
					{
						U8_T i;
						update_alarm_tbl(&alarms[private_header.point_start_instance], private_header.point_end_instance - private_header.point_start_instance + 1 );
//						for(i = private_header.point_start_instance;i <= private_header.point_end_instance;i++ )
//							AlarmSync(1,i,0,Station_NUM);
					}

					else if(command == WRITEGROUPELEMENTS_T3000)
					{
						// check whether exist remote point, if exist, add it to remote point list
						U8_T i/*,j*/;
						Point_Net point; 
						S32_T value;		
						
						for(i = private_header.point_start_instance;i < private_header.point_end_instance;i++)
						{  
							if(i < MAX_ELEMENTS) 
							{
								point.number = group_data[i].reg.nPoint_number;
								point.point_type = group_data[i].reg.nPoint_type;
								point.panel = group_data[i].reg.nMain_Panel;
								point.sub_id = group_data[i].reg.nSub_Panel;
								point.network_number = Setting_Info.reg.network_number;	
								
						//		if(point.panel == 0)
								// add points to network points table
								get_net_point_value(&point,&value,1,1);
							}							
						}
							
						check_graphic_element();
					}

					else if(command == WRITEREMOTEPOINT)
					{	// write remote point value
						put_net_point_value(&(remote_points_list_modbus[private_header.point_start_instance].point), \
						&(remote_points_list_modbus[private_header.point_start_instance].point_value) \
						,0,1,1);  // OPERATOR  1					   		
					}
#if 0//ASIX
#if USB_HOST
					else if(command == WRITE_AT_CMD)
					{	
						if(gsm_str[0] == 0)  // clear send buf
						{
							flag_clear_send_buf = 1;
						}
						else if(gsm_str[0] == 1)  // at cmd
						{
						 	flag_receive_AT_CMD = 1;
						}
						else if(gsm_str[0] == 2)   // open T3000 at cmd window
						{
							flag_open_windows = 1;
							flag_close_windows = 0;
							flag_reinit_APN = 0;
						}
						else if(gsm_str[0] == 3)   // close T3000 at cmd window
						{
						  flag_close_windows = 1;
							flag_open_windows = 0;
						}
					}
#endif	
#endif					
					else if(command == WRITE_MISC)
					{
						// store it to flash memory
					}
					else if(command == WRITE_SPECIAL_COMMAND)
					{
						// clear com_TX com_Rx ...

						if(Write_Special.reg.clear_health_rx_tx == 0x11)
						{
							char i;

							for( i = 0; i < 3;i++)
							{
								com_tx[i] = 0;
								com_rx[i] = 0;
								collision[i] = 0;
								packet_error[i] = 0;
								timeout[i] = 0;
							}	
						}
					}
					
//					else if(command == WRITE_SUB_ID_BY_HAND)
//					{ 
//					    list_tstat_pos(); 	
//					}

				}
			}
		}
	}
	else  // read
	{	 
		if( Temp_CS.value[2]  == READMONITORDATA_T3000 || Temp_CS.value[2]  == UPDATEMEMMONITOR_T3000
			|| command == READPIC_T3000)
		{
			temp[2] = Graphi_data->command;
			temp[3] = Graphi_data->index;
			temp[4] = Graphi_data->sample_type;
			memcpy(&temp[5],Graphi_data->comm_arg.string,12);
		}
		else
		{
			if(private_header.command == READ_BACNET_TO_MDOBUS)
			{
				transfer_len = private_header.entitysize * 2; 
			}
			else
			{
				transfer_len = private_header.entitysize * (private_header.point_end_instance - private_header.point_start_instance + 1);
			}
			
			if((transfer_len >= 0) && (transfer_len <= 500 ))
			{
				temp[1] = (uint8_t)(transfer_len >> 8);
				temp[0] = (uint8_t)transfer_len;
				temp[2] = private_header.command;
				temp[3] = private_header.point_start_instance;
				temp[4] = private_header.point_end_instance;
				temp[5] = (uint8_t)private_header.entitysize ; 				
				temp[6] = (uint8_t)(private_header.entitysize >> 8); 					
				
				if(private_header.command == READPROGRAMCODE_T3000 || private_header.command == WRITEPROGRAMCODE_T3000)
				{
						temp[6] |= (packet_index << 1);
				}				
			}		
		}

		switch(command)
		{
#if 1//(ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
			case READ_BACNET_TO_MDOBUS:
			// get packet (transfer bacnet to modbus )	
				Get_Pkt_Bac_to_Modbus(&private_header);
				ptr = (uint8_t *)(&bacnet_to_modbus[0]);
				break;
#endif
			case READOUTPUT_T3000:
				if(private_header.point_end_instance <= MAX_OUTS)
				ptr = (uint8_t *)(&outputs[private_header.point_start_instance]);
				break;
			case READINPUT_T3000:					
				if(private_header.point_end_instance <= MAX_INS)
				ptr = (uint8_t *)(&inputs[private_header.point_start_instance]);
				break;
			case READVARIABLE_T3000:
				if(private_header.point_end_instance <= MAX_VARS)
				ptr = (uint8_t *)(&vars[private_header.point_start_instance]);
				break;
			case READWEEKLYROUTINE_T3000:
				if(private_header.point_end_instance <= MAX_WR)
				ptr = (uint8_t *)(&weekly_routines[private_header.point_start_instance]);
				break;
			case READANNUALROUTINE_T3000:
				if(private_header.point_end_instance <= MAX_AR)
				ptr = (uint8_t *)(&annual_routines[private_header.point_start_instance]);
				break;
			case READPROGRAM_T3000:
				if(private_header.point_end_instance <= MAX_PRGS)
				ptr = (uint8_t *)(&programs[private_header.point_start_instance]);
				break;
			case READPROGRAMCODE_T3000:
				if(private_header.point_end_instance <= MAX_PRGS)
				ptr = (uint8_t *)&prg_code[private_header.point_start_instance][CODE_ELEMENT * packet_index];
				break;
			case READTIMESCHEDULE_T3000:   /* read time schedule  */
				if(private_header.point_end_instance <= MAX_WR)
				ptr = (uint8_t *)&wr_times[private_header.point_start_instance];
				break;
		 	case READANNUALSCHEDULE_T3000:    /* read annual schedule*/
				if(private_header.point_end_instance <= MAX_AR)
				ptr = (uint8_t *)&ar_dates[private_header.point_start_instance];				
				break;
			case READ_SCHEDULE_FLAG:
				if(private_header.point_end_instance <= MAX_WR)
				ptr = (uint8_t *)&wr_time_on_off[private_header.point_start_instance];	
				break;
			case READTIME_COMMAND:
				// if daylight_saving_time
				{ 
//				if(Rtc.Clk.year % 5 == 0)
					Rtc2.NEW.timestamp = swap_double(get_current_time());
					//memcpy(&Test[25],&Rtc2.NEW.timestamp,4);
//				else
//					Rtc2.NEW.timestamp = swap_double(get_current_time()) - 86400;
				Rtc2.NEW.time_zone = timezone;
				Rtc2.NEW.daylight_saving_time = Daylight_Saving_Time;
				ptr = (uint8_t *)(Rtc2.all);
				break;
			}
			case READCONTROLLER_T3000:
				if(private_header.point_end_instance <= MAX_CONS)
				{
					ptr = (uint8_t *)(&controllers[private_header.point_start_instance]);

					for( j=0; j<MAX_CONS; j++ )
					{
						get_point_value( (Point*)&controllers[j].input, &controllers[j].input_value );
						get_point_value( (Point*)&controllers[j].setpoint, &controllers[j].setpoint_value );
					}
				}
				break;
			case READMONITOR_T3000 :
				if(private_header.point_end_instance <= MAX_MONITORS)
				ptr = (uint8_t *)(&monitors[private_header.point_start_instance]);				
				break;
	 		case READSCREEN_T3000 :
				if(private_header.point_end_instance <= MAX_GRPS)
				ptr = (uint8_t *)(&control_groups[private_header.point_start_instance]);
				break;
			case READGROUPELEMENTS_T3000:
				if(private_header.point_end_instance <= MAX_ELEMENTS)
				{
				ptr = (uint8_t *)(&group_data[private_header.point_start_instance]);
				}
				break;
			case READREMOTEPOINT:
				if(private_header.point_end_instance <= MAXREMOTEPOINTS)
				ptr = (uint8_t *)(&points_header[private_header.point_start_instance]);
				break;	 
			case READMONITORDATA_T3000:				
			// check whether get correct data, if fail no response
				flag_read_monitor = ReadMonitor(Graphi_data);	
				transfer_len = 400;
				temp[22] = LOW_BYTE(Graphi_data->total_seg);
				temp[23] = HIGH_BYTE(Graphi_data->total_seg);
				temp[24] = (Graphi_data->total_seg) >> 16;
				temp[25] = (Graphi_data->total_seg) >> 24;
			
				temp[18] = LOW_BYTE(Graphi_data->seg_index);
				temp[19] = HIGH_BYTE(Graphi_data->seg_index);
				temp[20] = Graphi_data->seg_index >> 16;
				temp[21] = Graphi_data->seg_index >> 24;
			
				temp[17] = Graphi_data->special;
				ptr = (uint8_t *)(Graphi_data->asdu);	
				
				break;
#if 0//STORE_TO_SD
		 case READPIC_T3000:			 
				ReadPicture(Graphi_data);
				transfer_len = 400;
				temp[22] = LOW_BYTE(Graphi_data->total_seg);
				temp[23] = HIGH_BYTE(Graphi_data->total_seg);
				temp[24] = (Graphi_data->total_seg) >> 16;
				temp[25] = (Graphi_data->total_seg) >> 24;
			
				temp[18] = LOW_BYTE(Graphi_data->seg_index);
				temp[19] = HIGH_BYTE(Graphi_data->seg_index);
				temp[20] = Graphi_data->seg_index >> 16;
				temp[21] = Graphi_data->seg_index >> 24;
			
				temp[17] = Graphi_data->special;
				ptr = (uint8_t *)(Graphi_data->asdu);	
				break;
#endif			
//			case UPDATEMEMMONITOR_T3000:
//				UpdateMonitor(Graphi_data);
//				ptr = (char *)(Graphi_data->asdu);
//				break;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
			case GET_PANEL_INFO:   // other commad
				Sync_Panel_Info();	
				Panel_Info.reg.protocal = protocal;
				ptr = (uint8_t *)(Panel_Info.all);	
				break;
#endif
			case READ_SETTING:	
				Sync_Panel_Info(); 
#if ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI
				//check_SD_PnP();
#endif
				
			  ptr = (uint8_t *)(Setting_Info.all);
				break;
			case READVARUNIT_T3000:
					ptr = (uint8_t *)(var_unit);
					break;
			case READEXT_IO_T3000:					
					ptr = (uint8_t *)(extio_points);	
					break;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
			case READ_ZONE_T3000:		
					refresh_zone();
					ptr = (uint8_t *)(ID_Config);	
					break;
#endif
			case READALARM_T3000:   // 13
				if(private_header.point_end_instance <= MAX_ALARMS)
				ptr = (uint8_t *)(&alarms[private_header.point_start_instance]);
				break;
			case READUNIT_T3000: // digital customer range
				if(private_header.point_end_instance <= MAX_DIG_UNIT)
				ptr = (uint8_t *)(&digi_units[private_header.point_start_instance]);
				break;
			case READTABLE_T3000: // analog customer range
				ptr = (uint8_t *)(&custom_tab[private_header.point_start_instance]);
				break;
			case READUSER_T3000:
				if(private_header.point_end_instance <= MAX_PASSW)
				ptr = (uint8_t *)(&passwords[private_header.point_start_instance]);
				break;
//			case READTSTAT_T3000:
//				ptr = (char *)(&scan_db[private_header.point_start_instance]);
//				break;
#if 0//ASIX
#if USB_HOST
			case READ_AT_CMD:
				ptr = (uint8_t *)usb_buf;
				break;
#endif
#endif
//			case READWEATHER_T3000:
//					ptr = (char *)(&weather);
//					break;
			case READ_TSTAT_DB:
				{					
					U8_T i,index;	
					index = 0; 
					
					for(i = 0;i < sub_no;i++)
					{	 
						if((scan_db[i].product_model >= CUSTOMER_PRODUCT) || (current_online[scan_db[i].id / 8] & (1 << (scan_db[i].id % 8))))	  	 // in database but not on_line
						{
							if(scan_db[i].product_model != PRODUCT_MINI_BIG)
							{
								Remote_tst_db.sub[index].modbus_id = (U16_T)scan_db[i].id;
								Remote_tst_db.sub[index].instance = 0;
								Remote_tst_db.sub[index].protocal = 0;								
								index++;
							}	
						}						
					}
					
					for(i = 0;i < remote_panel_num;i++)
					{
						if(remote_panel_db[i].protocal == BAC_MSTP )
						{
							Remote_tst_db.sub[index].protocal = 1;
							Remote_tst_db.sub[index].modbus_id = 0;
							Remote_tst_db.sub[index].instance = remote_panel_db[i].device_id;
							index++;
						}
					}
					Remote_tst_db.number = index;
					
					ptr = (char *)(&Remote_tst_db);
				}			
				break;
			case READ_MISC:
				{
					char i;
					for( i = 0; i < 24;i++)
						MISC_Info.reg.monitor_block_num[i] = swap_double(SD_block_num[i]);
					MISC_Info.reg.flag = swap_word(0xff55);
					MISC_Info.reg.flag1 = 0x55;
					for( i = 0; i < 3;i++)
					{
						MISC_Info.reg.com_tx[i] = swap_double(com_tx[i]);
						MISC_Info.reg.com_rx[i] = swap_double(com_rx[i]);
						MISC_Info.reg.collision[i] = swap_word(collision[i]);
						MISC_Info.reg.packet_error[i] = swap_word(packet_error[i]);
						MISC_Info.reg.timeout[i] = swap_word(timeout[i]);
					}	
				}
				ptr = (uint8_t *)(MISC_Info.all);	
				break;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
			case READ_MSV_COMMAND:	
				ptr = (uint8_t *)&msv_data[private_header.point_start_instance];	
				break;
			case READ_EMAIL_ALARM:
				ptr = (uint8_t *)&Email_Setting;	
				break;
#endif
			default:
				break;
		}

		if(ptr != NULL)	
		{
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
			if(protocal < 0xa0)  // mstp or bip
#endif
			{
				if(header_len + transfer_len < MAX_OCTET_STRING_BYTES) // avoid overrun
					memcpy(&temp[header_len],ptr,transfer_len);
				else	
				{				
					Test[11]++;	
					return ;
				}
			}
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
			else
			{
				memcpy(&temp,&apdu[0],14);
				if(header_len + transfer_len < MAX_OCTET_STRING_BYTES) // avoid overrun
					memcpy(&temp[14],ptr,transfer_len);
				else				
					return ;
				
			}
#endif
		}
		
		status = bacapp_parse_application_data(BACNET_APPLICATION_TAG_OCTET_STRING,	temp, &data_value);
		
	} 
	
#if 0	
	if(command == WRITEPIC_T3000)
	{
		status = flag_write_pic;
	}
#endif	
	if((command == WRITEPRGFLASH_COMMAND) || (command == CLEAR_MONITOR))
	{
		status = true;
	}
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
	if(protocal < 0xa0)  // mstp or bip
#endif
	{
		if(status == true)
		{
				memset(temp,0,MAX_APDU);
				private_data_len = 
						bacapp_encode_application_data(&temp[0],&data_value);
			
				private_data.serviceParameters = &temp[0];
				private_data.serviceParametersLen = private_data_len;
				len = uptransfer_encode_apdu(&apdu[0], &private_data);
#if 0		
			if(command == WRITEPIC_T3000)
			{
				if(flag_write_pic == 1)
					Send_UnconfirmedPrivateTransfer(src,&private_data,protocal);
			}
			else 
#endif
			if(command == WRITEPRGFLASH_COMMAND)
			{
				Send_UnconfirmedPrivateTransfer(src,&private_data,protocal);
			}
			if(command == READMONITORDATA_T3000)
			{
				if(flag_read_monitor == 1)
				{
					Send_UnconfirmedPrivateTransfer(src,&private_data,protocal);
				}
			}
			else
			{
				Send_UnconfirmedPrivateTransfer(src,&private_data,protocal);
			}
		}
	}
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
	else  // Temco private modbus
	{
		U16_T crc_check;
		// send data via temco private modbus
		uart_init_send_com(protocal - 0xa0);
		crc_check = crc16(temp, transfer_len + 14);
		temp[transfer_len + 14] = HIGH_BYTE(crc_check);
		temp[transfer_len + 15] = LOW_BYTE(crc_check);
		uart_send_string(temp,transfer_len + 16,protocal - 0xa0);
		
		// 回复完再修改
//		if(delay_write_setting == 1)
//			dealwith_write_setting(&Setting_Info);
	}
#endif
  return;

}

int ptransfer_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    uint8_t * invoke_id,
    BACNET_PRIVATE_TRANSFER_DATA * private_data)
{
    int len = 0;
    unsigned offset = 0;
    if (!apdu)
        return -1;
    /* optional checking - most likely was already done prior to this call */
    if (apdu[0] != PDU_TYPE_CONFIRMED_SERVICE_REQUEST)
        return -1;
    /*  apdu[1] = encode_max_segs_max_apdu(0, MAX_APDU); */
    /* invoke id - filled in by net layer */
    *invoke_id = apdu[2];
    if (apdu[3] != SERVICE_CONFIRMED_PRIVATE_TRANSFER)
        return -1;
    offset = 4;
    if (apdu_len > offset) {
        len =
            ptransfer_decode_service_request(&apdu[offset], apdu_len - offset,
            private_data);
    }

    return len;
}

int uptransfer_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_PRIVATE_TRANSFER_DATA * private_data)
{
    int len = 0;
    unsigned offset = 0;

    if (!apdu) {
        return -1;
    }
    /* optional checking - most likely was already done prior to this call */
    if (apdu[0] != PDU_TYPE_UNCONFIRMED_SERVICE_REQUEST) {
        return -1;
    }
    if (apdu[1] != SERVICE_UNCONFIRMED_PRIVATE_TRANSFER) {
        return -1;
    }
    offset = 2;
    if (apdu_len > offset) {
        len =
		  ptransfer_decode_service_request(&apdu[offset], apdu_len - offset,
            private_data);
    }
    return len;
}





/* encode service */
static int pt_encode_apdu(
    uint8_t * apdu,
    uint16_t max_apdu,
    BACNET_PRIVATE_TRANSFER_DATA * private_data)
{
    int len = 0;        /* length of each encoding */
    int apdu_len = 0;   /* total length of the apdu, return value */
/*
        Unconfirmed/ConfirmedPrivateTransfer-Request ::= SEQUENCE {
        vendorID               [0] Unsigned,
        serviceNumber          [1] Unsigned,
        serviceParameters      [2] ABSTRACT-SYNTAX.&Type OPTIONAL
    }
*/
    /* unused parameter */
    max_apdu = max_apdu;
    if (apdu) {
        len =
            encode_context_unsigned(&apdu[apdu_len], 0,
            private_data->vendorID);
        apdu_len += len;
        len =
            encode_context_unsigned(&apdu[apdu_len], 1,
            private_data->serviceNumber);
        apdu_len += len;
        len = encode_opening_tag(&apdu[apdu_len], 2);
        apdu_len += len;
        for (len = 0; len < private_data->serviceParametersLen; len++) {
            apdu[apdu_len] = private_data->serviceParameters[len];
            apdu_len++;
        }
        len = encode_closing_tag(&apdu[apdu_len], 2);
        apdu_len += len;
    }

    return apdu_len;
}

int ptransfer_encode_apdu(
    uint8_t * apdu,
    uint8_t invoke_id,
    BACNET_PRIVATE_TRANSFER_DATA * private_data)
{
    int apdu_len = 0;   /* total length of the apdu, return value */
    int len = 0;

    if (apdu) {
        apdu[0] = PDU_TYPE_CONFIRMED_SERVICE_REQUEST;
        apdu[1] = encode_max_segs_max_apdu(0, MAX_APDU);
        apdu[2] = invoke_id;
        apdu[3] = SERVICE_CONFIRMED_PRIVATE_TRANSFER;
        apdu_len = 4;
        len =
            pt_encode_apdu(&apdu[apdu_len], (uint16_t) (MAX_APDU - apdu_len),
            private_data);
        apdu_len += len;
    }

    return apdu_len;
}

int uptransfer_encode_apdu(
    uint8_t * apdu,
    BACNET_PRIVATE_TRANSFER_DATA * private_data)
{
    int apdu_len = 0;   /* total length of the apdu, return value */
    int len = 0;

    if (apdu) {
        apdu[0] = PDU_TYPE_UNCONFIRMED_SERVICE_REQUEST;
        apdu[1] = SERVICE_UNCONFIRMED_PRIVATE_TRANSFER;
        apdu_len = 2;
        len =
            pt_encode_apdu(&apdu[apdu_len], (uint16_t) (MAX_APDU - apdu_len),
            private_data);
        apdu_len += len;
    }

    return apdu_len;
}

/* decode the service request only */
int ptransfer_decode_service_request(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_PRIVATE_TRANSFER_DATA * private_data)
{
    int len = 0;        /* return value */
    int decode_len = 0; /* return value */
    uint32_t unsigned_value = 0;
    /* check for value pointers */
    if (apdu_len && private_data) {
        /* Tag 0: vendorID */
        decode_len = decode_context_unsigned(&apdu[len], 0, &unsigned_value);
        if (decode_len < 0) {
            return -1;
        }
        len = decode_len;
        private_data->vendorID = (uint16_t) unsigned_value;
        /* Tag 1: serviceNumber */
        decode_len = decode_context_unsigned(&apdu[len], 1, &unsigned_value);
        if (decode_len < 0) {
            return -1;
        }
				

        len += decode_len;
        private_data->serviceNumber = unsigned_value;
        /* Tag 2: serviceParameters */
        if (decode_is_opening_tag_number(&apdu[len], 2)) {
            /* a tag number of 2 is not extended so only one octet */
            len++;
            /* don't decode the serviceParameters here */
						
            private_data->serviceParameters = &apdu[len];
            private_data->serviceParametersLen =
                (int) apdu_len - len - 1 /*closing tag */ ;
            /* len includes the data and the closing tag */
            len = (int) apdu_len;
        } else {
            return -1;
        }
    }

    return len;
}

int ptransfer_error_encode_apdu(
    uint8_t * apdu,
    uint8_t invoke_id,
    BACNET_ERROR_CLASS error_class,
    BACNET_ERROR_CODE error_code,
    BACNET_PRIVATE_TRANSFER_DATA * private_data)
{
    int apdu_len = 0;   /* total length of the apdu, return value */
    int len = 0;        /* length of the part of the encoding */

    if (apdu) {
        apdu[0] = PDU_TYPE_ERROR;
        apdu[1] = invoke_id;
        apdu[2] = SERVICE_CONFIRMED_PRIVATE_TRANSFER;
        apdu_len = 3;
        /* service parameters */
/*
        ConfirmedPrivateTransfer-Error ::= SEQUENCE {
        errorType       [0] Error,
        vendorID        [1] Unsigned,
        serviceNumber [2] Unsigned,
        errorParameters [3] ABSTRACT-SYNTAX.&Type OPTIONAL
    }
*/
        len = encode_opening_tag(&apdu[apdu_len], 0);
        apdu_len += len;
        len = encode_application_enumerated(&apdu[apdu_len], error_class);
        apdu_len += len;
        len = encode_application_enumerated(&apdu[apdu_len], error_code);
        apdu_len += len;
        len = encode_closing_tag(&apdu[apdu_len], 0);
        apdu_len += len;
        len =
            encode_context_unsigned(&apdu[apdu_len], 1,
            private_data->vendorID);
        apdu_len += len;
        len =
            encode_context_unsigned(&apdu[apdu_len], 2,
            private_data->serviceNumber);
        apdu_len += len;
        len = encode_opening_tag(&apdu[apdu_len], 3);
        apdu_len += len;
        for (len = 0; len < private_data->serviceParametersLen; len++) {
            apdu[apdu_len] = private_data->serviceParameters[len];
            apdu_len++;
        }
        len = encode_closing_tag(&apdu[apdu_len], 3);
        apdu_len += len;
    }

    return apdu_len;
}

/* decode the service request only */
int ptransfer_error_decode_service_request(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_ERROR_CLASS * error_class,
    BACNET_ERROR_CODE * error_code,
    BACNET_PRIVATE_TRANSFER_DATA * private_data)
{
    int len = 0;        /* return value */
    int decode_len = 0; /* return value */
    uint8_t tag_number = 0;
    uint32_t len_value_type = 0;
    uint32_t unsigned_value = 0;

    /* check for value pointers */
    if (apdu_len && private_data) {
        /* Tag 0: Error */
        if (decode_is_opening_tag_number(&apdu[len], 0)) {
            /* a tag number of 0 is not extended so only one octet */
            len++;
            /* error class */
            decode_len =
                decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value_type);
            len += decode_len;
            if (tag_number != BACNET_APPLICATION_TAG_ENUMERATED) {
                return 0;
            }
            decode_len =
                decode_enumerated(&apdu[len], len_value_type, &unsigned_value);
            len += decode_len;
            if (error_class) {
                *error_class = (BACNET_ERROR_CLASS) unsigned_value;
            }
            /* error code */
            decode_len =
                decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value_type);
            len += decode_len;
            if (tag_number != BACNET_APPLICATION_TAG_ENUMERATED) {
                return 0;
            }
            decode_len =
                decode_enumerated(&apdu[len], len_value_type, &unsigned_value);
            len += decode_len;
            if (error_code) {
                *error_code = (BACNET_ERROR_CODE) unsigned_value;
            }
            if (decode_is_closing_tag_number(&apdu[len], 0)) {
                /* a tag number of 0 is not extended so only one octet */
                len++;
            } else {
                return 0;
            }
        }
        /* Tag 1: vendorID */
        decode_len = decode_context_unsigned(&apdu[len], 1, &unsigned_value);
        if (decode_len < 0) {
            return -1;
        }
        len += decode_len;
        private_data->vendorID = (uint16_t) unsigned_value;
        /* Tag 2: serviceNumber */
        decode_len = decode_context_unsigned(&apdu[len], 2, &unsigned_value);
        if (decode_len < 0) {
            return -1;
        }
        len += decode_len;
        private_data->serviceNumber = unsigned_value;
        /* Tag 3: serviceParameters */
        if (decode_is_opening_tag_number(&apdu[len], 3)) {
            /* a tag number of 2 is not extended so only one octet */
            len++;
            /* don't decode the serviceParameters here */
            private_data->serviceParameters = &apdu[len];
            private_data->serviceParametersLen =
                (int) apdu_len - len - 1 /*closing tag */ ;
        } else {
            return -1;
        }
        /* we could check for a closing tag of 3 */
    }

    return len;
}

int ptransfer_ack_encode_apdu(
    uint8_t * apdu,
    uint8_t invoke_id,
    BACNET_PRIVATE_TRANSFER_DATA * private_data)
{
    int len = 0;        /* length of each encoding */
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu) {
        apdu[0] = PDU_TYPE_COMPLEX_ACK; /* complex ACK service */
        apdu[1] = invoke_id;    /* original invoke id from request */
        apdu[2] = SERVICE_CONFIRMED_PRIVATE_TRANSFER;   /* service choice */
        apdu_len = 3;
        /* service ack follows */
/*
        ConfirmedPrivateTransfer-ACK ::= SEQUENCE {
        vendorID               [0] Unsigned,
        serviceNumber          [1] Unsigned,
        resultBlock            [2] ABSTRACT-SYNTAX.&Type OPTIONAL
    }
*/
				
				 len =
            encode_context_unsigned(&apdu[apdu_len], 0,
            private_data->vendorID);
        apdu_len += len;
			
        len =
            encode_context_unsigned(&apdu[apdu_len], 1,
            private_data->serviceNumber);
        apdu_len += len;
				
        len = encode_opening_tag(&apdu[apdu_len], 2);
        apdu_len += len;
        for (len = 0; len < private_data->serviceParametersLen; len++) {
            apdu[apdu_len] = private_data->serviceParameters[len];
            apdu_len++;
        }
				
        len = encode_closing_tag(&apdu[apdu_len], 2);
        apdu_len += len;
				
    }

    return apdu_len;
}

/* ptransfer_ack_decode_service_request() is the same as
       ptransfer_decode_service_request */
#endif

