/*
 ******************************************************************************
 *     Copyright (c) 2006	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ******************************************************************************
 */
 /*============================================================================
 * Module Name: gudpbc.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: gudpbc.c,v $
 *
 *=============================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include "reg80390.h"
#include "adapter.h"
#include "gudpbc.h"
#include "tcpip.h"
#include "mstimer.h"
#include "stoe.h"
#include "uart.h"
#include "gconfig.h"
#include "ax11000.h"
#include "mac.h"
#include "flash.h"
#include "i2c.h"
#include "i2capi.h"
//#include "printd.h"
#include "main.h"
#include <string.h>
#include "lcd.h"
#include "timesync.h"
#include "alarm.h"
#include "point.h"

//#include "8563.h"
//#include "ch375_RNDIS.h"

/* NAMING CONSTANT DECLARATIONS */
#define GS2E_ENABLE_STATE_MACHINE	0
#define GUDPBC_EEPROM_CONFIG		(GCONFIG_EEPROM_CONFIG)
#define GUDPBC_ENABLE_DEBUG_MSG		0 // 1: enable 0:disable

#define GUDPBC_MAX_CONNS			4
#define GUDPBC_NO_NEW_CONN			0xFF

#define GUDPBC_STATE_FREE			0
#define	GUDPBC_STATE_WAIT			1
#define	GUDPBC_STATE_CONNECTED		2


extern uint8_t flag_master_panel;
extern uint8_t check_lost_master;
/* GLOBAL VARIABLES DECLARATIONS */

/* LOCAL VARIABLES DECLARATIONS */
static GUDPBC_CONN far gudpbc_Conns[GUDPBC_MAX_CONNS];
U8_T far  gudpbc_InterAppId;
//static GCONFIG_CFG_PKT gudpbc_ConfigTxPkt;
//static GCONFIG_CFG_PKT gudpbc_ConfigRxPkt;

/* LOCAL SUBPROGRAM DECLARATIONS */
//void gudpbc_HandleSearchReq(U8_T XDATA* pData, U8_T id,U8_T type);
void gudpbc_HandleSetReq(U8_T XDATA* pData, U16_T length, U8_T id);
void gudpbc_HandleUpgradeReq(U8_T XDATA* pData, U16_T length, U8_T id);
void gudpbc_HandleResetReq(U8_T XDATA* pData, U16_T length, U8_T id);
void gudpbc_HandleRebootReq(U8_T XDATA* pData, U16_T length, U8_T id);
#if GUDPBC_EEPROM_CONFIG
void gudpbc_HandleEepromReadReq(U8_T XDATA* pData, U16_T length, U8_T id);
void gudpbc_HandleEepromWriteReq(U8_T XDATA* pData, U16_T length, U8_T id);
#endif


/* add for private application */
//U16_T 	UdpPort = 1234;
/* NC information structure */


STR_SCAN_CMD far Scan_Infor;

U8_T 	state=1;
U8_T 	scanstart=0;

uint16 far udp_scan_count;
u8 far flag_udp_scan;

//extern U8_T far flag_scan_sub;
//extern U8_T far count_scan_sub_by_hand;


void UdpData(U8_T type)
{
	// header 2 bytes
	memset(&Scan_Infor,0,sizeof(STR_SCAN_CMD));
	if(type == 0)
		Scan_Infor.cmd = 0x6500;
	else if(type == 1)
		Scan_Infor.cmd = 0x6700;

	Scan_Infor.len = 0x1D00;
	
	//serialnumber 4 bytes
	Scan_Infor.own_sn[0] = (U16_T)Modbus.serialNum[0] << 8;
	Scan_Infor.own_sn[1] = (U16_T)Modbus.serialNum[1] << 8;
	Scan_Infor.own_sn[2] = (U16_T)Modbus.serialNum[2] << 8;
	Scan_Infor.own_sn[3] = (U16_T)Modbus.serialNum[3] << 8;
	
	//nc 
	
	if(Modbus.mini_type == MINI_CM5)
		Scan_Infor.product = (U16_T)PRODUCT_CM5 << 8;
	else
		Scan_Infor.product = (U16_T)PRODUCT_MINI_BIG << 8;

	//modbus address
	Scan_Infor.address = (U16_T)Modbus.address << 8;
	
	//Ip
	Scan_Infor.ipaddr[0] = (U16_T)Modbus.ip_addr[0] << 8;
	Scan_Infor.ipaddr[1] = (U16_T)Modbus.ip_addr[1] << 8;
	Scan_Infor.ipaddr[2] = (U16_T)Modbus.ip_addr[2] << 8;
	Scan_Infor.ipaddr[3] = (U16_T)Modbus.ip_addr[3] << 8;
	
	//port
	Scan_Infor.modbus_port = swap_word(Modbus.tcp_port);

	// software rev
	Scan_Infor.firmwarerev = swap_word(SW_REV / 10 + SW_REV % 10);
	// hardware rev
	Scan_Infor.hardwarerev = swap_word(Modbus.hardRev);
	
	Scan_Infor.instance_low = (U16_T)(Instance); // hight byte first
	Scan_Infor.panel_number = panel_number; //  36	
	Scan_Infor.instance_hi = (U16_T)(Instance >> 16); // hight byte first
	
	Scan_Infor.bootloader = 0;  // 0 - app, 1 - bootloader, 2 - wrong bootloader
	Scan_Infor.BAC_port = Modbus.Bip_port;  // 
	Scan_Infor.zigbee_exist = zigbee_exist; // 0 - inexsit, 1 - exist
	
	state = 1;
	scanstart = 0;

}
 


#if GUDPBC_EEPROM_CONFIG
/*
 * ----------------------------------------------------------------------------
 * Function Name: I2C_Init
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void I2C_Init(void)
{
	switch (CSREPR & (BIT6|BIT7))
	{
		case SYS_CLK_100M :
			/* I2C master mode, interrupt enable, fast mode in slave, 7-bits address, 400KHz at 100M */
			I2C_Setup(I2C_ENB|I2C_FAST|I2C_MST_IE|I2C_7BIT|I2C_MASTER_MODE, 0x0031, 0x005A);
			break;
		case SYS_CLK_50M :
			/* I2C master mode, interrupt enable, fast mode in slave, 7-bits address, 400KHz at 50M */
			I2C_Setup(I2C_ENB|I2C_FAST|I2C_MST_IE|I2C_7BIT|I2C_MASTER_MODE, 0x0018, 0x005A);
			break;
		case SYS_CLK_25M :
			/* I2C master mode, interrupt enable, fast mode in slave, 7-bits address, 400KHz at 25M */
			I2C_Setup(I2C_ENB|I2C_FAST|I2C_MST_IE|I2C_7BIT|I2C_MASTER_MODE, 0x000c, 0x005A);
			break;
	}
}
#endif

/*
 * ----------------------------------------------------------------------------
 * Function Name: GUDPBC_Task
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GUDPBC_Task(void)
{

} /* End of GUDPBC_Task() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GUDPBC_Init()
 * Purpose: Initialization
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GUDPBC_Init(U16_T localPort)
{
	U8_T i;

	for (i = 0; i < GUDPBC_MAX_CONNS; i++)
	{
		gudpbc_Conns[i].State = GUDPBC_STATE_FREE;
		gudpbc_Conns[i].UdpSocket = 0;
	}

	gudpbc_InterAppId = TCPIP_Bind(GUDPBC_NewConn, GUDPBC_Event, GUDPBC_Receive);
	/* unicast packet */
	TCPIP_UdpListen(localPort, gudpbc_InterAppId);
	
} /* End of GUDPBC_Init() */


U8_T GUDPBC_NewConn(U32_T XDATA* pip, U16_T remotePort, U8_T socket)
{
	U8_T	i;
	
	for(i = 0; i < GUDPBC_MAX_CONNS; i++)
	{		
		if(gudpbc_Conns[i].State == GUDPBC_STATE_FREE)
		{		
		  gudpbc_Conns[i].State = GUDPBC_STATE_CONNECTED;
		  gudpbc_Conns[i].UdpSocket = socket;
			return i;
		}
// new connection
		udp_scan_count = 0;
		
	}
	if(i == GUDPBC_MAX_CONNS)
	{	 
		gudpbc_Conns[0].State = GUDPBC_STATE_FREE;
		TCPIP_UdpClose(gudpbc_Conns[0].UdpSocket);		

		gudpbc_Conns[0].State = GUDPBC_STATE_CONNECTED;
		gudpbc_Conns[0].UdpSocket = socket;
	
		return 0;
	}

	return GUDPBC_NO_NEW_CONN;



} /* End of GUDPBC_NewConn() */


void GUDPBC_Event(U8_T id, U8_T event)
{
	gudpbc_Conns[id].State = event;

} /* End of GUDPBC_Event() */


extern xSemaphoreHandle sembip;
void TCP_IP_Init(void);
// check whether should intial Tcp again
void Check_whether_reiniTCP(void)
{
	if(flag_udp_scan == 1)	
// when keep communication with device,monitor task is suspend, so only when resume normal communicaion,
// check whether need to initial TCP agian.
	{
		if((udp_scan_count % 60 == 0) && (udp_scan_count > 0))
		{
			
#if TIME_SYNC	
			//avoid interrupt updating minipanel
			if(flag_stop_timesync == 0)		
			{				
				TimeSync_Scan();
			}
#endif
		}
		
		udp_scan_count++;
		if(udp_scan_count > 60 * 5) // 5min
		{
			udp_scan_count = 0;	
			Test[10]++;
//			flag_reintial_tcpip = 1;
			TCP_IP_Init();
//			count_reintial_tcpip = 0;
		}
		
	}
}

//U8_T IDATA fwAutoUpdated[4] _at_ 0x31;
void GUDPBC_Receive(U8_T XDATA* pData, U16_T length, U8_T id)
{
	U8_T opcode = 0xFF;
	BOOL bValidReq = FALSE;
	U16_T i;
	
	U8_T  n = 0;	
	if(pData[0] == 0x64)
	{  
//		flag_udp_scan = 1;
//		udp_scan_count = 0;
		state = 1;		
		for(n = 0;n < (U8_T)length / 4;n++)
		{ 		
			if((pData[4*n + 1] == Modbus.ip_addr[0]) && (pData[4*n+2] == Modbus.ip_addr[1])
				&&(pData[4*n + 3] == Modbus.ip_addr[2]) && (pData[4*n+4] == Modbus.ip_addr[3]))
			{ 
			//	scanstart=0;
				state=0;
			//	break;
			}
		}

		if(state)
		{
			//use broadcast when scan			
			UdpData(0);
			//serialnumber 4 bytes
			Scan_Infor.master_sn[0] = 0;
			Scan_Infor.master_sn[1] = 0;
			Scan_Infor.master_sn[2] = 0;
			Scan_Infor.master_sn[3] = 0;
			memcpy(&Scan_Infor.panelname,panelname,20);
//			if(cSemaphoreTake( xSemaphore_tcp_send, ( portTickType ) 10 ) == pdTRUE)
			{	
				TCPIP_UdpSend(gudpbc_Conns[id].UdpSocket, 0, 0, &Scan_Infor, sizeof(STR_SCAN_CMD));	
//				cSemaphoreGive( xSemaphore_tcp_send );
			}
// for MODBUS device
			for(i = 0;i < sub_no;i++)
			{	 
				if((scan_db[i].product_model >= CUSTOMER_PRODUCT) || (current_online[scan_db[i].id / 8] & (1 << (scan_db[i].id % 8))))	  	 // in database but not on_line
				{
					if(scan_db[i].product_model != PRODUCT_MINI_BIG)
					{
						Scan_Infor.own_sn[0] = (U16_T)scan_db[i].sn  << 8;
						Scan_Infor.own_sn[1] = (U16_T)(scan_db[i].sn >> 8) << 8;
						Scan_Infor.own_sn[2] = (U16_T)(scan_db[i].sn >> 16) << 8;
						Scan_Infor.own_sn[3] = (U16_T)(scan_db[i].sn >> 24) << 8;					

						Scan_Infor.product = (U16_T)scan_db[i].product_model << 8;
						Scan_Infor.address = (U16_T)scan_db[i].id << 8;
				
						
						Scan_Infor.master_sn[0] = Modbus.serialNum[0];
						Scan_Infor.master_sn[1] = Modbus.serialNum[1];
						Scan_Infor.master_sn[2] = Modbus.serialNum[2];
						Scan_Infor.master_sn[3] = Modbus.serialNum[3];
						
						memcpy(&Scan_Infor.panelname,tstat_name[i],20);
						
						{	
	//						if(cSemaphoreTake( xSemaphore_tcp_send, ( portTickType ) 10 ) == pdTRUE)
							{//	Test[11]++;
								TCPIP_UdpSend(gudpbc_Conns[id].UdpSocket, 0, 0, &Scan_Infor, sizeof(STR_SCAN_CMD));				
	//							cSemaphoreGive( xSemaphore_tcp_send );
							}
						}
					}
				}				
			}
			
			// if id confict, send it to T3000
			if(conflict_num == 1)
			{
				if(conflict_product != PRODUCT_MINI_BIG)
				{
					Scan_Infor.own_sn[0] = (U16_T)conflict_sn_old  << 8;
					Scan_Infor.own_sn[1] = (U16_T)(conflict_sn_old >> 8) << 8;
					Scan_Infor.own_sn[2] = (U16_T)(conflict_sn_old >> 16) << 8;
					Scan_Infor.own_sn[3] = (U16_T)(conflict_sn_old >> 24) << 8;					

					Scan_Infor.product = (U16_T)conflict_product << 8;
				
					Scan_Infor.address = (U16_T)conflict_id << 8;
			
					
					Scan_Infor.master_sn[0] = Modbus.serialNum[0];
					Scan_Infor.master_sn[1] = Modbus.serialNum[1];
					Scan_Infor.master_sn[2] = Modbus.serialNum[2];
					Scan_Infor.master_sn[3] = Modbus.serialNum[3];

				
				{	
//					if(cSemaphoreTake( xSemaphore_tcp_send, ( portTickType ) 10 ) == pdTRUE)
					{	
						TCPIP_UdpSend(gudpbc_Conns[id].UdpSocket, 0, 0, &Scan_Infor, sizeof(STR_SCAN_CMD));					
//						cSemaphoreGive( xSemaphore_tcp_send );
					}
				}
			
				Scan_Infor.own_sn[0] = (U16_T)conflict_sn_new  << 8;
				Scan_Infor.own_sn[1] = (U16_T)(conflict_sn_new >> 8) << 8;
				Scan_Infor.own_sn[2] = (U16_T)(conflict_sn_new >> 16) << 8;
				Scan_Infor.own_sn[3] = (U16_T)(conflict_sn_new >> 24) << 8;					

				{	
//					if(cSemaphoreTake( xSemaphore_tcp_send, ( portTickType ) 10 ) == pdTRUE)
					{	
						TCPIP_UdpSend(gudpbc_Conns[id].UdpSocket, 0, 0, &Scan_Infor, sizeof(STR_SCAN_CMD));				
//						cSemaphoreGive( xSemaphore_tcp_send );
					}
				}
			}
		}
			
// for MSTP device		
			for(i = 0;i < remote_panel_num;i++)
			{	 
				//if((current_online[scan_db[i].id / 8] & (1 << (scan_db[i].id % 8))))	  	 // in database but not on_line
				{
//					remote_panel_db[i].sn = 1000 + i;

//					Scan_Infor.own_sn[0] = (U16_T)remote_panel_db[i].sn  << 8;
//					Scan_Infor.own_sn[1] = (U16_T)(remote_panel_db[i].sn >> 8) << 8;
//					Scan_Infor.own_sn[2] = (U16_T)(remote_panel_db[i].sn >> 16) << 8;
//					Scan_Infor.own_sn[3] = (U16_T)(remote_panel_db[i].sn >> 24) << 8;					

//					Scan_Infor.product = 0x2300;//(U16_T)scan_db[i].product_model << 8; tbd:
//					Scan_Infor.address = 0xfe00; // tbd:			
//					
//					Scan_Infor.master_sn[0] = Modbus.serialNum[0];
//					Scan_Infor.master_sn[1] = Modbus.serialNum[1];
//					Scan_Infor.master_sn[2] = Modbus.serialNum[2];
//					Scan_Infor.master_sn[3] = Modbus.serialNum[3];	
//				
//					Scan_Infor.instance_low = (U16_T)(remote_panel_db[i].device_id); // hight byte first
//					Scan_Infor.station_num = remote_panel_db[i].panel;	
//					Scan_Infor.instance_hi = (U16_T)(remote_panel_db[i].device_id >> 16); // hight byte first
					
	
//							TCPIP_UdpSend(gudpbc_Conns[id].UdpSocket, 0, 0, &Scan_Infor, sizeof(STR_SCAN_CMD));					

				}				
			}	

			
		}    
	        
	}
	else if((pData[0] == 0x66) && (pData[1] == Modbus.ip_addr[0]) && (pData[2] == Modbus.ip_addr[1]) && (pData[3] == Modbus.ip_addr[2]) && (pData[4] == Modbus.ip_addr[3]))
	{
		// cmd(1 byte) + changed ip(4 bytes) + new ip(4 bytes) + new subnet(4 bytes) + new getway(4)  --- old protocal
		 // + sn(4 bytes)  -- new protocal, used to change conflict ip
    	 
		 if(((pData[17] == Modbus.serialNum[0]) && (pData[18] == Modbus.serialNum[1]) && (pData[19] == Modbus.serialNum[2]) && (pData[20] == Modbus.serialNum[3]))
			 || ((pData[17] == 0) && (pData[18] == 0) && (pData[19] == 0) && (pData[20] == 0)))
		 {
				n = 5;
				
				Modbus.tcp_type = 0;
			
				Modbus.ip_addr[0] = pData[n++];
				Modbus.ip_addr[1] = pData[n++];
				Modbus.ip_addr[2] = pData[n++];
				Modbus.ip_addr[3] = pData[n++];	

				Modbus.subnet[0] = pData[n++];
				Modbus.subnet[1] = pData[n++];
				Modbus.subnet[2] = pData[n++];
				Modbus.subnet[3] = pData[n++];
			
				Modbus.getway[0] = pData[n++];
				Modbus.getway[1] = pData[n++];
				Modbus.getway[2] = pData[n++];
				Modbus.getway[3] = pData[n++];
			

				UdpData(1);
				TCPIP_UdpSend(gudpbc_Conns[id].UdpSocket, 0, 0, &Scan_Infor, sizeof(STR_SCAN_CMD));				
			
		//	  ChangeIP = 1;
				if((Modbus.ip_addr[0] != 0)  && (Modbus.ip_addr[1] != 0)  && (Modbus.ip_addr[3] != 0) )
				{
					E2prom_Write_Byte(EEP_IP, Modbus.ip_addr[3]);
					E2prom_Write_Byte(EEP_IP + 1, Modbus.ip_addr[2]);
					E2prom_Write_Byte(EEP_IP + 2, Modbus.ip_addr[1]);
					E2prom_Write_Byte(EEP_IP + 3, Modbus.ip_addr[0]);
					
					
					E2prom_Write_Byte(EEP_SUBNET, Modbus.subnet[3]);
					E2prom_Write_Byte(EEP_SUBNET + 1, Modbus.subnet[2]);
					E2prom_Write_Byte(EEP_SUBNET + 2, Modbus.subnet[1]);
					E2prom_Write_Byte(EEP_SUBNET + 3, Modbus.subnet[0]);
					
					E2prom_Write_Byte(EEP_GETWAY, Modbus.getway[3]);
					E2prom_Write_Byte(EEP_GETWAY + 1, Modbus.getway[2]);
					E2prom_Write_Byte(EEP_GETWAY + 2, Modbus.getway[1]);
					E2prom_Write_Byte(EEP_GETWAY + 3, Modbus.getway[0]);
					
					//	reg_temp = 0;   // clear 0 to let bootloader knows the ip changes cause the reboot
					//	IntFlashWrite(0x4001, &reg_temp, 1, ERA_RUN); // so it will back to runtime quickly

					IntFlashWriteByte(0x4001,0);
					AX11000_SoftReboot();
				}		
			}
	 }
	 else if(pData[0] == 0x69)  // time sync and password
	 {
			// receive scan info of sub minipanel
			for(i = 0;i < 10;i++)  // time
			{
				 Rtc.all[i] = pData[1+i];
			}			

			Rtc_Set(Rtc.Clk.year,Rtc.Clk.mon,Rtc.Clk.day,Rtc.Clk.hour,Rtc.Clk.min,Rtc.Clk.sec,0);
			
			// update password	
#if 	USER_SYNC		
			if(pData[11] == 0x55)  // enable user and pass
			memcpy((char *)passwords,&pData[11 + i],sizeof(Password_point) * MAX_PASSW);
#endif			
		}
#if TIME_SYNC		
		else if(pData[0] == 0x6b)  // heart beat 
		{			
			check_lost_master = 0;
			// receive other heart beat of sub minipanel
			if(Station_NUM > pData[1])  // receive small panel number,  
			{
				flag_master_panel = 0;
			}
		}
#endif	
    else if(pData[0] == 0x6c)  // alarm sync
		{		
				if(pData[1] == 0)  // add
				{	
					if(pData[3] != Station_NUM) 	
					{						
						generatealarm(&pData[4],255, pData[3], VIRTUAL_ALARM, alarm_at_all, ind_alarm_panel, alarm_panel, 0);		
					}
				}
//				else  // delete
//				{		
//					if(pData[3] != Station_NUM) 
//					{
//						Test[28] = pData[2];
//						alarms[pData[2]].ddelete = 1;
//						update_alarm_tbl(&alarms[pData[2]],1);
//					}
//				}
		
		}	
#if 1//ARM
		else if(pData[0] == 0x65)
		{  // receive scan command
			U32_T device_id;
			BACNET_ADDRESS src;
			STR_SCAN_CMD * ptr;
			
			ptr = &pData[0];
			if(ptr->product == (U16_T)(PRODUCT_MINI_BIG << 8))
			{
				
#if ARM		
		device_id = ptr->instance_low * 65536L + ptr->instance_hi;
		src.mac[4] = ptr->BAC_port;
		src.mac[5] = ptr->BAC_port >> 8;
		src.mac[0] = ptr->ipaddr[0];
		src.mac[1] = ptr->ipaddr[1];
		src.mac[2] = ptr->ipaddr[2];
		src.mac[3] = ptr->ipaddr[3];
#else
		device_id = swap_word(ptr->instance_hi) * 65536L + swap_word(ptr->instance_low);
		src.mac[4] = ptr->BAC_port >> 8;
		src.mac[5] = ptr->BAC_port;
		src.mac[0] = ptr->ipaddr[0] >> 8;
		src.mac[1] = ptr->ipaddr[1] >> 8;
		src.mac[2] = ptr->ipaddr[2] >> 8;
		src.mac[3] = ptr->ipaddr[3] >> 8;
#endif
			
			src.mac_len = 6;
		
			src.net = 0;
      src.len = 0;		
			address_add(device_id, 480, &src);
			add_remote_panel_db(device_id,&src,ptr->panel_number,NULL,0,BAC_IP);
			}
		}
#endif
//	cSemaphoreGive( xSemaphore_udp_receive );
} 
/*
 * ----------------------------------------------------------------------------
 * Function Name: gudpbc_HandleSearchReq
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
#if GUDPBC_EEPROM_CONFIG
/*
 * ----------------------------------------------------------------------------
 * Function Name: gudpbc_HandleSetReq
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void gudpbc_HandleSetReq(U8_T XDATA* pData, U16_T length, U8_T id)
{
#if GUDPBC_ENABLE_DEBUG_MSG
	printd("gudpbc_HandleSetReq()...\n\r");
#endif

#if GS2E_ENABLE_STATE_MACHINE	
	if (GS2E_GetTaskState() == GS2E_STATE_IDLE)
#else
	if (1)
#endif
	{
		GCONFIG_SetConfigPacket(&gudpbc_ConfigRxPkt);
		*(pData + GCONFIG_OPCODE_OFFSET) = GCONFIG_OPCODE_SET_ACK;
		TCPIP_UdpSend(gudpbc_Conns[id].UdpSocket, 0, 0, pData, length);
   	}
	else
	{
		*(pData + GCONFIG_OPCODE_OFFSET) = GCONFIG_OPCODE_SET_DENY;  	
		TCPIP_UdpSend(gudpbc_Conns[id].UdpSocket, 0, 0, pData, length);		
	}
} /* End of gudpbc_HandleSetReq() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: gudpbc_HandleUpgradeReq
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void gudpbc_HandleUpgradeReq(U8_T XDATA* pData, U16_T length, U8_T id)
{	
#if GUDPBC_ENABLE_DEBUG_MSG
	printd("gudpbc_HandleUpgradeReq()...\n\r");
#endif

#if GS2E_ENABLE_STATE_MACHINE	
	if (GS2E_GetTaskState() == GS2E_STATE_IDLE)
#else
	if (1)
#endif
	{
		GCONFIG_SetFirmwareUpgradeMode(GCONFIG_FW_UPGRADE_ENABLE);
		GCONFIG_WriteConfigData();
		*(pData + GCONFIG_OPCODE_OFFSET) = GCONFIG_OPCODE_UPGRADE_ACK;
		TCPIP_UdpSend(gudpbc_Conns[id].UdpSocket, 0, 0, pData, length);
	//	FirmwareUpdate();
	}
	else
	{
		*(pData + GCONFIG_OPCODE_OFFSET) = GCONFIG_OPCODE_UPGRADE_DENY;  	
		TCPIP_UdpSend(gudpbc_Conns[id].UdpSocket, 0, 0, pData, length);
	}
} /* End of gudpbc_HandleUpgradeReq() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: gudpbc_HandleResetReq
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void gudpbc_HandleResetReq(U8_T XDATA* pData, U16_T length, U8_T id)
{
#if GUDPBC_ENABLE_DEBUG_MSG
	printd("gudpbc_HandleResetReq()...\n\r");
#endif

#if GS2E_ENABLE_STATE_MACHINE	
	if (GS2E_GetTaskState() == GS2E_STATE_IDLE)
#else
	if (1)
#endif
	{
		GCONFIG_ReadDefaultConfigData();
		GCONFIG_WriteConfigData();
		GCONFIG_GetConfigPacket(&gudpbc_ConfigRxPkt);
	 	gudpbc_ConfigRxPkt.Opcode = GCONFIG_OPCODE_RESET_ACK;
		TCPIP_UdpSend(gudpbc_Conns[id].UdpSocket, 0, 0, (U8_T*) &gudpbc_ConfigRxPkt, GCONFIG_CFG_PKT_LEN);
	}
	else
	{
		*(pData + GCONFIG_OPCODE_OFFSET) = GCONFIG_OPCODE_RESET_DENY;  	
		TCPIP_UdpSend(gudpbc_Conns[id].UdpSocket, 0, 0, pData, length);
	}
} /* End of gudpbc_HandleResetReq() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: gudpbc_HandleRebootReq
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void gudpbc_HandleRebootReq(U8_T XDATA* pData, U16_T length, U8_T id)
{
#if GUDPBC_ENABLE_DEBUG_MSG
	printd("gudpbc_HandleRebootReq()...\n\r");
#endif

#if GS2E_ENABLE_STATE_MACHINE
	if (GS2E_GetTaskState() == GS2E_STATE_IDLE)
#else
	if (1)
#endif
	{
		*(pData + GCONFIG_OPCODE_OFFSET) = GCONFIG_OPCODE_REBOOT_ACK;  	
		TCPIP_UdpSend(gudpbc_Conns[id].UdpSocket, 0, 0, pData, length);
		// May store current status/setting here before restart

		IntFlashWriteByte(0x4001,0);
		AX11000_SoftReboot();
	}
	else
	{
		*(pData + GCONFIG_OPCODE_OFFSET) = GCONFIG_OPCODE_RESET_DENY;  	
		TCPIP_UdpSend(gudpbc_Conns[id].UdpSocket, 0, 0, pData, length);
	}
} /* End of gudpbc_HandleRebootReq() */


/*
 * ----------------------------------------------------------------------------
 * Function Name: gudpbc_HandleEepromReadReq
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void gudpbc_HandleEepromReadReq(U8_T XDATA* pData, U16_T length, U8_T id)
{
#if GUDPBC_ENABLE_DEBUG_MSG
	printd("gudpbc_HandleEepromReadReq()...\n\r");
#endif

	GCONFIG_GetConfigPacket(&gudpbc_ConfigRxPkt);
	*(pData + GCONFIG_OPCODE_OFFSET) = GCONFIG_OPCODE_EEPROM_READ_ACK;
	TCPIP_UdpSend(gudpbc_Conns[id].UdpSocket, 0, 0, pData, length);
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: gudpbc_HandleEepromWriteReq
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void gudpbc_HandleEepromWriteReq(U8_T XDATA* pData, U16_T length, U8_T id)
{
#if GUDPBC_ENABLE_DEBUG_MSG
	printd("gudpbc_HandleEepromWriteReq()...\n\r");
#endif

	GCONFIG_SetEpromData(&gudpbc_ConfigRxPkt);
	GCONFIG_SetConfigPacket(&gudpbc_ConfigRxPkt);
	*(pData + GCONFIG_OPCODE_OFFSET) = GCONFIG_OPCODE_EEPROM_WRITE_ACK;
	TCPIP_UdpSend(gudpbc_Conns[id].UdpSocket, 0, 0, pData, length);
}
#endif
/* End of gudpbc.c */
