#ifndef _TCPIP_H
#define _TCPIP_H

#include <string.h>
#include "stm32f10x.h"
#include "delay.h"
#include "dma.h"
#include "vmalloc.h"
#include "enc28j60.h"
#include "timerx.h"
#include "uip.h"
#include "uip_arp.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "tapdev.h"
#include "resolv.h"
#include "smtpc.h"
#include "tcp_debug.h"

#define	TCPIP_CONNECT_CANCEL		0
#define TCPIP_CONNECT_WAIT			1
#define TCPIP_CONNECT_ACTIVE		2
#define TCPIP_CONNECT_XMIT_COMPLETE	3
#define TCPIP_CONNECT_BUSY			0xf1

//#define BACNET_PORT 47808
#define SUB_NET_MAX_COUNT  29
#define TOTAL_REPLY_COUNT  (SUB_NET_MAX_COUNT + 1)

typedef struct 
{
	U16_T cmd;   // low byte first
	U16_T len;   // low byte first
	U16_T own_sn[4]; // low byte first
	U16_T product;   // low byte first
	U16_T address;   // low byte first
	U16_T ipaddr[4]; // low byte first
	U16_T modbus_port; // low byte first
	U16_T firmwarerev; // low byte first
	U16_T hardwarerev;  // 28 29	// low byte first
	
	U8_T master_sn[4];  // master's SN 30 31 32 33
	U16_T instance_low; // 34 35 hight byte first
	U8_T panel_number; //  36	
	U8_T panelname[20]; // 37 - 56
	U16_T instance_hi; // 57 58 hight byte first
	
	U8_T bootloader;  // 0 - app, 1 - bootloader, 2 - wrong bootloader , 3 - mstp device
	U16_T BAC_port;  //  hight byte first
	U8_T zigbee_exist; // BIT0: 0 - inexsit, 1 - exist
										 // BIT1: 0 - NO WIFI, 1 - WIFI
	
	U8_T subnet_protocal; // 0 - modbus, 12 - bip to mstp
			
	U8_T  command_version; //65 version number
  U8_T  subnet_port;  //设备属于哪个端口 1- MainPort      2-ZigbeePort      3-SubPort
  U8_T  subnet_baudrate;   // 子设备所用baud
}STR_SCAN_CMD;

typedef struct
{
	U8_T buf[600];
	U16_T len;
}STR_SEND_BUF;

extern U8_T 	state;
extern STR_SCAN_CMD far Scan_Infor;
extern STR_SCAN_CMD Infor[TOTAL_REPLY_COUNT];
extern u8 rec_scan_index;

#define REMOTE_SERVER 1

extern u16 Test[50];

extern u8 IP_Change;

extern u8 flag_response_scan;
extern u32 response_scan_ip;
extern u32 response_scan_port;
void Response_Scan_Start(void);
void Response_Scan_appcall(void);

extern u8 DNSC_flag;
//extern uint8_t flag_dhcp_configured;

extern u8 update_firmware;

extern u8 ip_addr[4];
extern u8 gateway[4];
extern u16 tcp_port;

extern u8 tcp_server_sendbuf[300];
extern u16 tcp_server_sendlen;

void dhcpc_configured(const struct dhcpc_state *s);



extern U8_T far dyndns_enable;
extern U8_T far dyndns_provider;
extern U16_T far dyndns_update_time;



extern STR_SEND_BUF bip_bac_buf;
extern STR_SEND_BUF bip_bac_buf2;
extern STR_SEND_BUF mstp_bac_buf[10];
extern U8_T send_scan_index;
extern U8_T rec_scan_index;

extern u8 rec_mstp_index;
extern u8 send_mstp_index;

extern u8 rec_mstp_index1; // response packets form   
extern u8 send_mstp_index1;
extern STR_SEND_BUF mstp_bac_buf1[10];

extern U8_T Send_bip_Flag;
//extern xSemaphoreHandle Sem_mstp_Flag;
extern char Send_mstp_Flag;

extern U8_T bip_send_mstp_rport;

void TCPIP_Task( void *pvParameters );

// UDP CLIENT SCAN 

#define UDP_SCAN_LPORT 1234
//#define UDP_BACNET_LPORT 47808
#define UDP_BIP_SEND_LPORT 40005
void udp_scan_init(void);
void bip_Init(void);
void UDP_bacnet_APP(void);
void UDP_SCAN_APP(void);
void tcp_server_appcall(struct uip_conn * conn);
void Set_broadcast_bip_address(uint32_t net_address);
// UDP SERVER TIMESERVER
#if (ARM_MINI || ARM_CM5)
void tcpip_intial(void);
#endif
// UDP CLIENT BACNET


//

// TCP SERVER MODBUS

// TCP SERVER WEBPAGE
#endif

