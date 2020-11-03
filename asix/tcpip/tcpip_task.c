#include "main.h"
//#include "gudpmc.h"
#include "dyndns_app.h"
//#include "httpd.h"



#if (INCLUDE_DHCP_CLIENT)
  #include "dhcpc.h"
#endif
#if (INCLUDE_DNS_CLIENT)
  #include "dnsc.h"
#endif

/* NAMING CONSTANT DECLARATIONS */
#ifdef DEBUG
#define DBGMSG(A) {A}
#else
#define DBGMSG(A) {}
#endif


extern U8_T firmware_update;

extern U32_T ether_rx_packet;	 
extern U32_T ether_tx_packet;

typedef struct app_buf 
{
	U32_T	ipaddr;
	U8_T	buf[100];
	U16_T	uip_len;
	U16_T	PayLoadOffset;
	U8_T	wait;
}APP_BUF;

APP_BUF	XDATA app_arp_buf;


//U16_T	BACNET_PORT;

#define TIME_OUT_COUNTER	(250/SWTIMER_INTERVAL)  
static U16_T ServerBroadcastListenPort;
static U16_T bacnetListenPort;
xSemaphoreHandle xSemaphore_tcp_send;
xSemaphoreHandle xSemaphore_udp_receive;
xSemaphoreHandle sembip;
//U8_T flag_retransmit;

/*
 * ----------------------------------------------------------------------------
 * Function Name: UpdateIpSettings
 * Purpose: Update IP address, subnet mak, gateway IP address and DNS IP address 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void UpdateIpSettings(U32_T ip)
{
 	 U32_T gateWay,subnet;

	 if(ip > 0)
	 {
	 	ip = STOE_GetIPAddr();
		subnet = STOE_GetSubnetMask();
		gateWay = STOE_GetGateway();
	
		Modbus.ip_addr[0] = (U8_T)(ip>>24); 		
		Modbus.ip_addr[1] = (U8_T)(ip>>16);		
		Modbus.ip_addr[2] = (U8_T)(ip>>8);		
		Modbus.ip_addr[3] = (U8_T)(ip);
	
		Modbus.subnet[0] = (U8_T)(subnet>>24);
		Modbus.subnet[1] = (U8_T)(subnet>>16);
		Modbus.subnet[2] = (U8_T)(subnet>>8);
		Modbus.subnet[3] = (U8_T)(subnet);
	
		Modbus.getway[0] = (U8_T)(gateWay>>24);
		Modbus.getway[1] = (U8_T)(gateWay>>16);
		Modbus.getway[2] = (U8_T)(gateWay>>8);
		Modbus.getway[3] = (U8_T)(gateWay);
	
	// UPDATE panel infomation 
		Panel_Info.reg.mac[0] = Modbus.ip_addr[0];
		Panel_Info.reg.mac[1] = Modbus.ip_addr[1];
		Panel_Info.reg.mac[2] = Modbus.ip_addr[2];
		Panel_Info.reg.mac[3] = Modbus.ip_addr[3];

		if((Modbus.ip_addr[0] != 0)  && (Modbus.ip_addr[1] != 0)  && (Modbus.ip_addr[3] != 0) )
		{		
			E2prom_Write_Byte(EEP_IP, Modbus.ip_addr[3]);
			E2prom_Write_Byte(EEP_IP + 1, Modbus.ip_addr[2]);
			E2prom_Write_Byte(EEP_IP + 2, Modbus.ip_addr[1]);
			E2prom_Write_Byte(EEP_IP + 3, Modbus.ip_addr[0]);
		}
//#if (DEBUG_UART1)
//	uart_init_send_com(UART_SUB1);	// for test		
//	sprintf(debug_str," \r\n\ dhcp %u %u %u %u",(uint16)Modbus.ip_addr[0], (uint16)Modbus.ip_addr[1], (uint16)Modbus.ip_addr[2], (uint16)Modbus.ip_addr[3]);
//	uart_send_string(debug_str,strlen(debug_str),UART_SUB1);
//#endif 

#if ASIX_MINI
		Display_IP();
#endif 
	}
	else
	{	
		ip = (((U32_T)Modbus.ip_addr[0]) << 24) | ((U32_T)Modbus.ip_addr[1] << 16) | ((U32_T)Modbus.ip_addr[2] << 8) | (Modbus.ip_addr[3]);
		subnet = (((U32_T)Modbus.subnet[0]) << 24) | ((U32_T)Modbus.subnet[1] << 16) | ((U32_T)Modbus.subnet[2] << 8) | (Modbus.subnet[3]);
		gateWay = (((U32_T)Modbus.getway[0]) << 24) | ((U32_T)Modbus.getway[1] << 16) | ((U32_T)Modbus.getway[2] << 8) | (Modbus.getway[3]);
	 
		TCPIP_SetIPAddr(ip); 
		TCPIP_SetSubnetMask(subnet);
		TCPIP_SetGateway(gateWay);
			
		STOE_SetIPAddr(ip); 
	  STOE_SetSubnetMask(subnet);
		STOE_SetGateway(gateWay);

//#if (DEBUG_UART1)
//	uart_init_send_com(UART_SUB1);	// for test		
//	sprintf(debug_str," \r\n\ static ip %u %u %u %u %u",(uint16)Modbus.ip_addr[0], (uint16)Modbus.ip_addr[1], (uint16)Modbus.ip_addr[2], (uint16)Modbus.ip_addr[3],(uint16)Modbus.tcp_port);
//	uart_send_string(debug_str,strlen(debug_str),UART_SUB1);
//#endif
	
#if ASIX_MINI
		Display_IP();
#endif
	//	GCONFIG_SetServerDynamicIP(ip);						
	//	GCONFIG_WriteConfigData();
	//	GUDPBC_Init(ServerBroadcastListenPort);
	} 

} /* End of UpdateIpSettings */

/*
 * ----------------------------------------------------------------------------
 * Function Name: CheckArpTable
 * Purpose: Update IP address, subnet mak, gateway IP address and DNS IP address 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void CheckArpTable(void)
{
	if (app_arp_buf.wait) 
	{
		U8_T valid = STOE_CHECK_MAC(&app_arp_buf.ipaddr);
		if (valid) 
		{ 			
			DMA_GrantXdata(uip_buf, app_arp_buf.buf, app_arp_buf.uip_len);
		//	PRINTD(DEBUG_MSG ,("send out the packet from arp buffer\n\r"));
			uip_len = app_arp_buf.uip_len;
			ETH_Send(app_arp_buf.PayLoadOffset);
			uip_len = 0;
			app_arp_buf.wait = 0;
		}
	}
} /* End of CheckArpTable */


void bip_Init(U16_T localPort);

 #if (INCLUDE_DHCP_CLIENT)
   	U8_T    cmdDhcpFlag = 0;
	U32_T 	dhcpTimeStart = 0;
	U32_T 	dhcpTimeStop = 0;
#endif


void TCP_IP_Init(void)
{
	/* Initialize Network adapter */
	ETH_Init();
	DHCP_Init();

#if GCONFIG_EEPROM_CONFIG
	I2C_Init();
#endif
	GCONFIG_Init();
#if (INCLUDE_DHCP_CLIENT)	
	if ( (Modbus.tcp_type == DHCP) && ((GCONFIG_GetNetwork() & GCONFIG_NETWORK_DHCP_ENABLE) == GCONFIG_NETWORK_DHCP_ENABLE) )
	{
	//	printd("DHCP request... ");
		DHCP_Start();
#if (!STOE_TRANSPARENT)
		STOE_DisableIpFilter();
#endif
		cmdDhcpFlag = 1;
		dhcpTimeStart = SWTIMER_Tick();
	}
	else
	{
		UpdateIpSettings(0);
	}
#else
//	printd("DHCP module is not included. Use static IP address\n\r");
	GCONFIG_SetServerDynamicIP(GCONFIG_GetServerStaticIP());						
	GCONFIG_WriteConfigData();
//	GUDPBC_Init(ServerBroadcastListenPort);
#endif

  ServerBroadcastListenPort = 1234; 
	GUDPBC_Init(ServerBroadcastListenPort);	

#if WEBPAGE	
	HTTP_Init();
#endif
	MODBUSTCP_Init();
//	if(Modbus.protocal == BAC_IP)
	{
			bip_set_socket(0);
			bip_set_addr(0);
			bip_set_port(0xBAC0/*Modbus.Bip_port*/); 		
			bip_Init(bip_get_port());	
	} 


//	FSYS_Init();

	

	ETH_Start();

	
#if  TIME_SYNC
	SYNC_Init();
#endif

#if (INCLUDE_DNS_CLIENT)
	DNSC_Init();	
#endif	
	
#if PING
	PING_Init();
#endif

	SNTPC_Init();  // must define DNSC_Init();
#if REM_CONNECTION
	RM_Init();	
#endif	
	init_dyndns();

}



void TCPIP_Task(void) reentrant
{
   //U32_T far iP,gateWay,subnet;
   portTickType xDelayPeriod  = ( portTickType )250 / portTICK_RATE_MS;//2 minutes writting flash.
   	
#if (INCLUDE_DNS_CLIENT)
	U8_T cmdDnsFlag = 0;
#endif
	
	
 #if (BOOTLDR_ISR)
	ERROR: BOOTLDR_ISR must set to '0' in non-bootloader driver.
   #endif
   #if (!AX_ETH_INT_ENABLE)
	 ERROR: Must enable ethernet module in this driver.
   #endif
	 U32_T	timeCount,preTimeCount;

#if STOE_TRANSPARENT
//	U8_T xdata arptimer;
#endif

//   U8_T WhichServer;

	ether_rx_packet = 0;
	ether_tx_packet = 0;   	

	vSemaphoreCreateBinary(xSemaphore_tcp_send);
	vSemaphoreCreateBinary(xSemaphore_udp_receive);
	vSemaphoreCreateBinary(sembip);
  TCP_IP_Init();
	firmware_update = FALSE;     

//	 202.120.2.101 
	task_test.enable[0] = 1;	
	 
	bvlc_intial();	
	 
	 
//	if(bbmd_en == 0)
//	{		// only for test 
//		register_ftd(0xc0a80044,0xbac0,20000);
//	}
//	else
//	{
//			WhoIs_Start(0xffffffff);
//	}


//	PING_Init();
	while (1)
	{
		
		if(firmware_update == TRUE)
		{
			if(Modbus.mini_type <= MINI_BIG)	
			{					
				Lcd_Show_String(1,1,"update firmware",NORMAL,15,0x0000,0xffff);
			}	
			
			IntFlashErase(ERA_RUN,0x4000);	// 0x4000 ,store some parameters about flash
			AX11000_SoftReboot();	

		}
#if (!STOE_TRANSPARENT)
		ETH_SendArpToGateway(ETH_CONTINUE_ARP_REQUEST_TO_GATEWAY_AFTER_REPLY);
#endif
		CheckArpTable();
#if (INCLUDE_DHCP_CLIENT)
		if (cmdDhcpFlag == 1)
		{			
			if (DHCP_GetState() > DHCP_IDLE_STATE)
			{
				DHCP_Send();
			}
			else
			{
					if(STOE_GetIPAddr() == 0)
					{
//						Test[18]++;
						UpdateIpSettings(0);
					}
					else
					{
//						Test[19]++;
						UpdateIpSettings(STOE_GetIPAddr());
					}
					
#if (!STOE_TRANSPARENT)
				STOE_EnableIpFilter();
#endif
				cmdDhcpFlag = 0;
			}
		}
#endif

#if (STOE_GET_INTSTATUS_MODE == STOE_INTERRUPT_MODE)	/* interrupt mode */
		if(STOE_GetInterruptFlag())
		{
			STOE_ProcessInterrupt();
		}
#else	/* polling mode */
		STOE_ProcessInterrupt(); 		

#endif

#if (!MAC_GET_INTSTATUS_MODE)
		if (MAC_GetInterruptFlag())
		{	
			MAC_ProcessInterrupt();
		}
#else
		MAC_LinkSpeedChk();

#endif
		timeCount = (U16_T)SWTIMER_Tick();
		if ((timeCount- preTimeCount)>= TIME_OUT_COUNTER)
		{				
			preTimeCount = timeCount;
			TCPIP_PeriodicCheck();
			task_test.count[0]++;
		}
	
		if(Modbus.en_dyndns == 2)  // enable this function
			do_dyndns(); 

		
     timeCount = (U16_T)SWTIMER_Tick();

//#if (INCLUDE_DNS_CLIENT)
//		if (cmdDnsFlag == 1)
//		{
//			U8_T state = DNSCTAB_GetState();/* include DNS table */

//			if(state == DNSC_STATE_FREE)
//			{
//				cmdDnsFlag = 0;
//				//printf ("Can not find DNS server.\n\r");
//			}
//			else if (state == DNSC_STATE_RESPONSED)
//			{
//				U32_T	ip;

//				cmdDnsFlag = 0;

//				if ((ip = DNSCTAB_GetIP()) == 0)
//				{
//				//	printf ("Can not find remote station via DNS server.\n\r");
//				}
//				else
//				{
//				//	printf ("Get IP address from DNS server : %lx\n\r", ip);
//				}
//			}
//			

//			
////			PING_Task();
//		}
//#endif

#if (INCLUDE_DNS_CLIENT)
	DNSC_Task();
#endif
		
#if PING
		PING_Task();
#endif
	}

} /* End of main() */

