#include "main.h"
//#include "gudpmc.h"

#define INCLUDE_DNS_CLIENT  0
#define INCLUDE_DHCP_CLIENT 1

#if (INCLUDE_DHCP_CLIENT)
  #include "dhcpc.h"
#endif
#if (INCLUDE_DNS_CLIENT)
  #include "dnsctab.h"

#endif

/* NAMING CONSTANT DECLARATIONS */
#ifdef DEBUG
#define DBGMSG(A) {A}
#else
#define DBGMSG(A) {}
#endif


extern U8_T firmware_update;


typedef struct app_buf 
{
	U32_T	ipaddr;
	U8_T	buf[100];
	U16_T	uip_len;
	U16_T	PayLoadOffset;
	U8_T	wait;
}APP_BUF;

APP_BUF	XDATA app_arp_buf;

#define TIME_OUT_COUNTER	(250/SWTIMER_INTERVAL)  //250
static U16_T ServerBroadcastListenPort;
static U16_T bacnetListenPort;


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
	
		/*IP_Addr[3] = 192;//(U8_T)(ip>>24); 		
		IP_Addr[2] = 168;//(U8_T)(ip>>16);		
		IP_Addr[1] = 0;//(U8_T)(ip>>8);		
		IP_Addr[0] = 173;//(U8_T)(ip);
	
		SUBNET[3] = 255;//(U8_T)(subnet>>24);
		SUBNET[2] = 255;//(U8_T)(subnet>>16);
		SUBNET[1] = 255;//(U8_T)(subnet>>8);
		SUBNET[0] = 0;//(U8_T)(subnet);
	
		GETWAY[3] = 192;//(U8_T)(gateWay>>24);
		GETWAY[2] = 168;//(U8_T)(gateWay>>16);
		GETWAY[1] = 0;//(U8_T)(gateWay>>8);
		GETWAY[0] = 1;//(U8_T)(gateWay);*/	
		IP_Addr[0] = (U8_T)(ip>>24); 		
		IP_Addr[1] = (U8_T)(ip>>16);		
		IP_Addr[2] = (U8_T)(ip>>8);		
		IP_Addr[3] = (U8_T)(ip);


	
		SUBNET[0] = (U8_T)(subnet>>24);
		SUBNET[1] = (U8_T)(subnet>>16);
		SUBNET[2] = (U8_T)(subnet>>8);
		SUBNET[3] = (U8_T)(subnet);
	
		GETWAY[0] = (U8_T)(gateWay>>24);
		GETWAY[1] = (U8_T)(gateWay>>16);
		GETWAY[2] = (U8_T)(gateWay>>8);
		GETWAY[3] = (U8_T)(gateWay);
		
				
		E2prom_Write_Byte(EEP_IP, IP_Addr[3]);
		E2prom_Write_Byte(EEP_IP + 1, IP_Addr[2]);
		E2prom_Write_Byte(EEP_IP + 2, IP_Addr[1]);
		E2prom_Write_Byte(EEP_IP + 3, IP_Addr[0]);



	}
	else
	{	
	//	IP_Addr[0] = 173;	IP_Addr[1] = 0;	IP_Addr[2] = 168;	IP_Addr[3] = 192;			
		ip = (((U32_T)IP_Addr[0]) << 24) | ((U32_T)IP_Addr[1] << 16) | ((U32_T)IP_Addr[2] << 8) | (IP_Addr[3]);
	//	SUBNET[0] = 0;	SUBNET[1] = 255;	SUBNET[2] = 255;	SUBNET[3] = 255;
		subnet = (((U32_T)SUBNET[0]) << 24) | ((U32_T)SUBNET[1] << 16) | ((U32_T)SUBNET[2] << 8) | (SUBNET[3]);
	//	GETWAY[0] = 1;	GETWAY[1] = 0;	GETWAY[2] = 168;	GETWAY[3] = 192;
		gateWay = (((U32_T)GETWAY[0]) << 24) | ((U32_T)GETWAY[1] << 16) | ((U32_T)GETWAY[2] << 8) | (GETWAY[3]);
	 
		TCPIP_SetIPAddr(ip); 
		TCPIP_SetSubnetMask(subnet);
		TCPIP_SetGateway(gateWay);
			
		STOE_SetIPAddr(ip); 
	    STOE_SetSubnetMask(subnet);
		STOE_SetGateway(gateWay);
	
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

void TCPIP_Task(void)reentrant
{
   //U32_T far iP,gateWay,subnet;
   U8_T linktype = 0;
   portTickType xDelayPeriod  = ( portTickType ) 250 / portTICK_RATE_MS;//2 minutes writting flash.
   	

 #if (BOOTLDR_ISR)
	ERROR: BOOTLDR_ISR must set to '0' in non-bootloader driver.
   #endif
   #if (!AX_ETH_INT_ENABLE)
	 ERROR: Must enable ethernet module in this driver.
   #endif
	 U32_T	timeCount,preTimeCount;
   #if (INCLUDE_DHCP_CLIENT)
   	U8_T    cmdDhcpFlag = 0;
	U32_T 	dhcpTimeStart = 0;
	U32_T 	dhcpTimeStop = 0;
#endif
#if STOE_TRANSPARENT
//	U8_T xdata arptimer;
#endif

//   U8_T WhichServer;

	/* Initialize Network adapter */
	ETH_Init();

	DHCP_Init();
	//if (DHCP_Init())
	//	printd("DHCP init ok.\n\r");

#if (INCLUDE_DNS_CLIENT)
	DNSCTAB_Init(); /* include DNS table */
#endif


#if GCONFIG_EEPROM_CONFIG
	I2C_Init();
#endif
	GCONFIG_Init();

//	ServerBroadcastListenPort = GCONFIG_GetServerBroadcastListenPort();
//	printd ("ServerBroadcastListenPort = %d\n\r", ServerBroadcastListenPort);
//	ServerBroadcastListenPort = 25122;
//	GUDPBC_Init(ServerBroadcastListenPort);

#if (INCLUDE_DHCP_CLIENT)	
	if ( (TCP_TYPE == DHCP) && ((GCONFIG_GetNetwork() & GCONFIG_NETWORK_DHCP_ENABLE) == GCONFIG_NETWORK_DHCP_ENABLE) )
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
	GUDPBC_Init(ServerBroadcastListenPort);
#endif

   	ServerBroadcastListenPort = 1234; 
	GUDPBC_Init(ServerBroadcastListenPort);

//	#if (defined(BACDL_BIP))

//	bacnetListenPort = 47808;
//	GUDPMC_Init(bacnetListenPort);	   		// bacnet port
	if(protocal == BAC_IP)
	{
		bip_set_port(47808);
		bip_Init(bip_get_port());
	}
//	#endif

	HTTP_Init();
//	FSYS_Init();

//	SNTPC_Init();
	
	ETH_Start();

//	WhichServer=Para[45]; //customer choose which server fro Sync.
//	SNTPC_Start(800, IpServer[WhichServer]);
//	should add condition if whichserver exceeds 6

	while (1)
	{
//		HSUR_ErrorRecovery(); 
		
		if(firmware_update == TRUE)
		{
			Lcd_All_Off();DELAY_Ms(1);
			Lcd_Show_String(1,1,"update firmware",NORMAL,15);
			IntFlashErase(ERA_RUN,0x4000);
			RELAY1_8 = 0;
		  	RELAY_LATCH = 0; 
			RELAY_LATCH = 1;  		
			DI2_LATCH = 1;
			KEY_LATCH = 1;
			DI1_LATCH = 1;
			P1 = 0xFF;	
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
			 	UpdateIpSettings(STOE_GetIPAddr());
#if (!STOE_TRANSPARENT)
				STOE_EnableIpFilter();
#endif
				cmdDhcpFlag = 0;
			}
		}
#endif

#if (STOE_GET_INTSTATUS_MODE == STOE_INTERRUPT_MODE)	/* interrupt mode */
		if (STOE_GetInterruptFlag())
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
		linktype = MAC_LinkSpeedChk();
		if(linktype == 0)  {flag_EthPort = 0;}
		else 	{flag_EthPort = 1;}	
#endif

		timeCount = (U16_T)SWTIMER_Tick();
		if ((timeCount- preTimeCount)>= TIME_OUT_COUNTER)
		{
			preTimeCount = timeCount;
			TCPIP_PeriodicCheck();
		
		}
		
       timeCount = (U16_T)SWTIMER_Tick();
	 #if 0    // dont understand it 
       if(Para[43]==2)
	   {
		  SNTPC_GetState();Para[43]=1;
       }
	#endif
      //  SNTPC_Debug();

#if (INCLUDE_DNS_CLIENT)
		if (cmdDnsFlag == 1)
		{
			U8_T state = DNSCTAB_GetState();/* include DNS table */

			if (state == DNSC_STATE_FREE)
			{
				cmdDnsFlag = 0;
			//	Test[15]++;
				//printf ("Can not find DNS server.\n\r");
			}
			else if (state == DNSC_STATE_RESPONSED)
			{
				U32_T	ip;

				cmdDnsFlag = 0;

				if ((ip = DNSCTAB_GetIP()) == 0)
				{
				//	Test[16]++;
				//	printf ("Can not find remote station via DNS server.\n\r");
				}
				else
				{
				//	printf ("Get IP address from DNS server : %lx\n\r", ip);
				}
			}
		}
#endif

	}
} /* End of main() */

