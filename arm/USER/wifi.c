#include "main.h"
#include "wifi.h"
#include "bsp_esp8266.h"


void Sync_with_NTP_by_Wifi(void);
char Get_SSID_RSSI(void);
#if ARM_MINI
#include "tcpip.h"
#endif

uint8 flag_wifi;
uint8 flag_set_wifi;
#define UIP_HEAD 6

#define WIFI_CONNECTED 			1
#define WIFI_DISCONNECTED   0

#define WifiSTACK_SIZE 1000//2048
xTaskHandle Wifi_Handler;
extern uint8_t PDUBuffer_BIP[MAX_APDU];
STR_SSID	SSID_Info;

u8 wifi_send_buf[1000];
u16 wifi_sendbyte_num;
u16 wifi_send_count;
char * itoa( int value, char *string, int radix );	



#if ARM_TSTAT_WIFI
STR_SEND_BUF mstp_bac_buf[10];
u8 rec_mstp_index;
u8 send_mstp_index;
u8 rec_mstp_index1; // response packets form   
u8 send_mstp_index1;
STR_SEND_BUF mstp_bac_buf1[10];
#endif
u8 count_transfer_bip_to_mstp;

void check_transfer_bip_to_mstp(void)
{
	if(rec_mstp_index1 > 0)
	{
		count_transfer_bip_to_mstp++;
		if(count_transfer_bip_to_mstp > 5)
		{
			rec_mstp_index1 = 0;
			count_transfer_bip_to_mstp = 0;
			Test[26]++;
		}
	}
}

/**
  * @brief  初始化ESP8266用到的GPIO引脚
  * @param  无
  * @retval 无
  */
	
char ESP8266_AT_Test ( void )
{
	char count=0;

//	if(Modbus.mini_type == MINI_TINY_ARM)
//	{
//		macESP8266_RST_HIGH_LEVEL_TINY();	
//	}
//	else
//		macESP8266_RST_HIGH_LEVEL();	
//	delay_ms ( 1000 );

	while( count < 10 )
	{
		if( ESP8266_Cmd ( "AT", "OK", NULL, 100 )) 
			return 2;
		//ESP8266_Rst();
		IWDG_ReloadCounter();
		++ count;
	}
	if(count == 10)
		return 0;
	else
		return 1;
}


void ESP8266_Rst ( void )
{
#if ARM_MINI
	if((Modbus.mini_type == MINI_TINY_ARM) || (Modbus.mini_type == MINI_NANO) || (Modbus.mini_type == MINI_TINY_11I))
	{
		macESP8266_RST_LOW_LEVEL_TINY();
		delay_ms ( 2000 ); 
		macESP8266_RST_HIGH_LEVEL_TINY();
	}
	else
#endif
	{
		macESP8266_RST_LOW_LEVEL();
		delay_ms ( 2000 ); 
		macESP8266_RST_HIGH_LEVEL();
	}		

}

static void ESP8266_GPIO_Config ( void )
{
	/*定义一个GPIO_InitTypeDef类型的结构体*/
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* 配置 RST 引脚*/
					   
#if ARM_MINI
  if((Modbus.mini_type == MINI_TINY_ARM) || (Modbus.mini_type == MINI_NANO) || (Modbus.mini_type == MINI_TINY_11I))
	{
		macESP8266_RST_APBxClock_FUN ( macESP8266_RST_CLK_TINY, ENABLE );
		GPIO_InitStructure.GPIO_Pin = macESP8266_RST_PIN_TINY;
		GPIO_Init ( macESP8266_RST_PORT_TINY, & GPIO_InitStructure );
	}
	else
#endif
	{
		macESP8266_RST_APBxClock_FUN ( macESP8266_RST_CLK, ENABLE );
		GPIO_InitStructure.GPIO_Pin = macESP8266_RST_PIN;
		GPIO_Init ( macESP8266_RST_PORT, & GPIO_InitStructure );
	}		


}



void UART4_IRQHandler(void)                	
{		
	uint8_t ucCh;
	int i = 0;
	if( USART_GetITStatus ( macESP8266_USARTx, USART_IT_RXNE ) != RESET )
	{
		ucCh = USART_ReceiveData( macESP8266_USARTx );		
		if ( strEsp8266_Fram_Record .InfBit .FramLength < ( RX_BUF_MAX_LEN - 1 ) )                       //预留1个字节写结束符
			strEsp8266_Fram_Record .Data_RX_BUF [ strEsp8266_Fram_Record .InfBit .FramLength ++ ]  = ucCh;
	}
	 	 
	if( USART_GetITStatus( macESP8266_USARTx, USART_IT_IDLE ) == SET )                                         //数据帧接收完毕
	{
    strEsp8266_Fram_Record .InfBit .FramFinishFlag = 1;
//		tcp_printf_str(strEsp8266_Fram_Record .Data_RX_BUF);
		ucCh = USART_ReceiveData( macESP8266_USARTx );                                         //由软件序列清除中断标志位(先读USART_SR，然后读USART_DR)
  }	

}


uint16_t check_packet(uint8_t * str,uint8_t * dat)
{
	//0x0d 0x0a +IPD,0,12:...
	uint16_t len = 0;
	uint16_t i;
	char  *pos;
	
	pos = strstr(str, "+IPD");
	if(pos == 0)
		return 0;
	if(*(pos+6) == ',')
	{
		//USART_printf(USART1, "\r\n pos: %s \r\n", pos);
		if((*(pos+8) == ':') && (*(pos+7) >= '0' && *(pos+7) <= '9'))
		{
			len = *(pos+7) - '0';
			for(i = 0;i < len;i++)
				*dat++ = *(pos+9+i);//str[11 + i];
			return len;
		}
		else if(*(pos+9) == ':')
		{
			if(*(pos+7) >= '0' && *(pos+7) <= '9' && *(pos+8) >= '0' && *(pos+8) <= '9')
			{
				len = (*(pos+7) - '0') * 10 + (*(pos+8) - '0');
				for(i = 0;i < len;i++)
					*dat++ = *(pos+10+i);//str[12 + i];
				return len;
			}
		}
		else if(*(pos+10) == ':')
		{
			if(*(pos+7) >= '0' && *(pos+7) <= '9' && *(pos+8) >= '0' && *(pos+8) <= '9' && *(pos+9) >= '0' && *(pos+9) <= '9')
			{
				len = (*(pos+7) - '0') * 100 + (*(pos+8) - '0') * 10 + (*(pos+9) - '0');
				for(i = 0;i < len;i++)
					*dat++ = *(pos+11+i);//str[13 + i];
				return len;
			}
		}
		return 0;
	}
	return 0;
}



#define UCID_BACNET 0
#define UCID_SCAN 	1
#define UCID_NTP	 	2

#if ARM_TSTAT_WIFI
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
	U8_T zigbee_exist; // 0 - inexsit, 1 - exist
	
	U8_T subnet_protocal; // 0 - modbus, 12 - bip to mstp

}STR_SCAN_CMD;

STR_SCAN_CMD Infor[20];
STR_SCAN_CMD Scan_Infor;
U8_T 	state;

void UdpData(U8_T type)
{
	// header 2 bytes
	memset(&Scan_Infor,0,sizeof(STR_SCAN_CMD));
	if(type == 0)
		Scan_Infor.cmd = 0x0065;
	else if(type == 1)
		Scan_Infor.cmd = 0x0067;

	Scan_Infor.len = 0x001d;
	
	//serialnumber 4 bytes
	Scan_Infor.own_sn[0] = (U16_T)Modbus.serialNum[0];
	Scan_Infor.own_sn[1] = (U16_T)Modbus.serialNum[1];
	Scan_Infor.own_sn[2] = (U16_T)Modbus.serialNum[2];
	Scan_Infor.own_sn[3] = (U16_T)Modbus.serialNum[3];
	
	//nc 
	
//	if(Modbus.mini_type == MINI_CM5)
//		Scan_Infor.product = (U16_T)PRODUCT_CM5;
//	else if((Modbus.mini_type >= MINI_BIG_ARM) && (Modbus.mini_type <= MINI_TINY_ARM))
//		Scan_Infor.product = (U16_T)PRODUCT_MINI_ARM;
//	else
//		Scan_Infor.product = (U16_T)PRODUCT_MINI_BIG;
	Scan_Infor.product = 10;//PRODUCT_MINI_ARM;  // only for test now

	//modbus address
	Scan_Infor.address = (U16_T)Modbus.address;
	
	//Ip
	Scan_Infor.ipaddr[0] = (U16_T)SSID_Info.ip_addr[0];
	Scan_Infor.ipaddr[1] = (U16_T)SSID_Info.ip_addr[1];
	Scan_Infor.ipaddr[2] = (U16_T)SSID_Info.ip_addr[2];
	Scan_Infor.ipaddr[3] = (U16_T)SSID_Info.ip_addr[3];
	
	//port
	Scan_Infor.modbus_port = SSID_Info.modbus_port;  //tbd :???????????????? 

	// software rev
	Scan_Infor.firmwarerev = swap_word(SW_REV / 10 + SW_REV % 10);
	// hardware rev
	Scan_Infor.hardwarerev = swap_word(Modbus.hardRev);
	
	Scan_Infor.instance_low = HTONS(Instance); // hight byte first
	Scan_Infor.panel_number = panel_number; //  36	
	Scan_Infor.instance_hi = HTONS(Instance >> 16); // hight byte first
	
	Scan_Infor.bootloader = 0;  // 0 - app, 1 - bootloader, 2 - wrong bootloader
	Scan_Infor.BAC_port = SSID_Info.bacnet_port;//((Modbus.Bip_port & 0x00ff) << 8) + (Modbus.Bip_port >> 8);  // 
	Scan_Infor.zigbee_exist = zigbee_exist; // 0 - inexsit, 1 - exist
	Scan_Infor.subnet_protocal = 0;
	state = 1;
//	scanstart = 0;

}

#endif

//uint8_t wifi_state;

void check_linkStatus(void)
{
	char i;
	for(i = 0;i < 10;i++)
	{	
		SSID_Info.IP_Wifi_Status = ESP8266_Get_LinkStatus();
		if(	(SSID_Info.IP_Wifi_Status < 4) && (SSID_Info.IP_Wifi_Status > 0))
		{
			break;	
		}
		else
			delay_ms(1000);
		
		IWDG_ReloadCounter();
	}	

	if(SSID_Info.IP_Wifi_Status == 3) // 已建立连接
	{
		ESP8266_Cmd ( "AT+CIPCLOSE=5", "OK", 0, 500 );
#if ARM_UART_DEBUG
	uart1_init(115200);
	DEBUG_EN = 1;
	printf("close connect \r\n");
#endif
	}
}

uint8_t get_ip;

void Restore_WIFI(void)
{
	char count;
	count = 0;
	ESP8266_Cmd ( "AT+RESTORE", "OK", 0, 500);
//	while((ESP8266_Cmd ( "AT+RESTORE", "OK", 0, 1000) == 0) && (count++ < 5))	
//	{		
//		IWDG_ReloadCounter();
//		delay_ms(100);
//	}			
}
	
char flag_start_smart;
uint8 flag_connect_AP;
void Check_connect_AP(void)
{	
	if(flag_connect_AP == 1)
	{
			connect_AP();
			QuickSoftReset();
//			ChangeFlash = 1;
//			write_page_en[24] = 1;
			flag_connect_AP = 0;
	}
}
extern uint8_t cStr [ 1024 ];
void connect_AP(void)
{
	char count;
	uint8 testip[4];
	get_ip = 0;
	if(SSID_Info.IP_Wifi_Status == WIFI_NO_WIFI)		
		return;
	if(SSID_Info.MANUEL_EN == 2)  // disable wifi
		return;
	if(SSID_Info.MANUEL_EN != 0)
	{
		flag_start_smart = 0;
		ESP8266_Cmd( "AT+CWSTOPSMART", "OK", 0, 500 );
		delay_ms(200);
	}
	// 设置静态IP需要放在连接路由之前，连接路由需要时间太长
	if(SSID_Info.IP_Auto_Manual == 1)	
	{
		count = 0;
		ESP8266_CIPSTA_DEF();
		delay_ms(1000);
		//需要等待
		while(count++ < 5)
		{
			memcpy(cStr,ESP8266_ReceiveString(DISABLE),1024);
			if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "OK" ) )
			{
				break;
			}
			delay_ms(1000);
		}
		ESP8266_Cmd ( "AT+RST", "OK", "ready", 1000 );
		delay_ms(200);
		get_ip = 0;
	}
	if(SSID_Info.MANUEL_EN == 1 /*&& SSID_Info.IP_Auto_Manual == 1*/)	
	{		
		ESP8266_Net_Mode_Choose ( STA );
		delay_ms(200);
		count = 0;
		ESP8266_JoinAP(SSID_Info.name,SSID_Info.password);
		//需要等待
		while(count++ < 5)
		{
			memcpy(cStr,ESP8266_ReceiveString(DISABLE),1024);
			if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "OK" ) )
			{
				break;
			}
			delay_ms(1000);
		}
	}
	
	
	check_linkStatus();
}

extern uint8_t count_hold_on_bip_to_mstp;
extern uint8_t flag_send_ntp_by_wifi;
extern u32 t_1;
uint8 count_reboot_wifi = 0;
uint8_t bacnet_wifi_buf[600];
uint16_t bacnet_wifi_len;
uint8_t modbus_wifi_buf[500];
uint16_t modbus_wifi_len;
uint8_t packet[1024];
uint8_t cStr [ 1024 ] = { 0 };
void WIFI_task(void) reentrant//one second base software timer
{	
	uint8_t ip[4];
	uint16_t count;
	char overtime = 20;
	char ucID;
	char ATret;
	uint16_t packet_len;
	uint8_t ip1[4];
	uint8_t ip2[4];
	uint8_t i;
	uint8 status;
	char tcp_port[5];
	uint16_t count_checkip;
	
	flag_start_smart = 0;
	flag_connect_AP = 0;
	
	SSID_Info.rev = 4;
	flag_set_wifi = 0;
#if !(ARM_TSTAT_WIFI)		
	if(SSID_Info.modbus_port == 0 || SSID_Info.modbus_port == 0xffff)
	{
		SSID_Info.modbus_port = 502;
	}
	if(SSID_Info.bacnet_port == 0 || SSID_Info.bacnet_port == 0xffff)
	{
		SSID_Info.bacnet_port = 47808;
	}	
	itoa(SSID_Info.modbus_port,tcp_port,10);
#endif
	
#if ARM_TSTAT_WIFI 
	SSID_Info.modbus_port = 502;
	SSID_Info.bacnet_port = Modbus.Bip_port;//47808;
	itoa(SSID_Info.modbus_port,tcp_port,10);
	ESP8266_Init();	
	delay_ms ( 2000 ); 
	ESP8266_Rst();
	delay_ms(2000);
#endif
	
#if ARM_MINI

	ESP8266_USART_Config(); 
	ESP8266_GPIO_Config();
	dma_init_uart4();
	if((Modbus.mini_type == MINI_TINY_ARM) || (Modbus.mini_type == MINI_NANO) || (Modbus.mini_type == MINI_TINY_11I))
		GPIO_SetBits ( macESP8266_RST_PORT_TINY, macESP8266_RST_PIN_TINY );
	else
		GPIO_SetBits ( macESP8266_RST_PORT, macESP8266_RST_PIN);
	delay_ms ( 2000 ); 
	ESP8266_Rst();
	delay_ms(2000);
	
	
#endif
	// detect whether wifi module is exist
	ATret = ESP8266_AT_Test();

	if(ATret != 2)  // not response OK
	{
		if(ex_moudle.enable == 0xff || ex_moudle.enable == 0)
		{
			
		}
		SSID_Info.IP_Wifi_Status = WIFI_NO_WIFI; // no wifi
	}
	else
	{
		uint8_t temp[7];
		E2prom_Read_Byte(EEP_WRITE_WIFI_MAC,&temp[0]);

		if(temp[0] != 0x44)
		{
			temp[1] = 0x18;
			temp[2] = 0xfe;
			if(((Modbus.serialNum[0] == 0) && (Modbus.serialNum[1] == 0) && (Modbus.serialNum[2] == 0) && (Modbus.serialNum[3] == 0))
				|| ((Modbus.serialNum[0] == 0xff) && (Modbus.serialNum[1] == 0xff) && (Modbus.serialNum[2] == 0xff) && (Modbus.serialNum[3] == 0xff)))
			{
				// do nothing
			}
			else
			{
				if(Modbus.serialNum[3] < 0x10)
					temp[3] = Modbus.serialNum[3] + 0x10;
				else
					temp[3] = Modbus.serialNum[3];
				
				if(Modbus.serialNum[2] < 0x10)
					temp[4] = Modbus.serialNum[2] + 0x10;
				else
					temp[4] = Modbus.serialNum[2];
				
				if(Modbus.serialNum[1] < 0x10)
					temp[5] = Modbus.serialNum[1] + 0x10;
				else
					temp[5] = Modbus.serialNum[1];
				
				if(Modbus.serialNum[0] < 0x10)
					temp[6] = Modbus.serialNum[0] + 0x10;
				else
					temp[6] = Modbus.serialNum[0];			
				
				ESP8266_Set_MAC(&temp[1]);			
				
				flag_set_wifi = 1;	
			}			
		}
		else
			ESP8266_Get_MAC(SSID_Info.mac_addr);
		SSID_Info.IP_Wifi_Status = WIFI_NO_CONNECT;
		ESP8266_Net_Mode_Choose( STA );
		connect_AP();
	}
//	flag_send_wifi = 0;
	for(;;)
	{	
		delay_ms(5) ;
		if(ATret == 2)
		{
			if(flag_set_wifi == 1)
			{
				ESP8266_Get_MAC(SSID_Info.mac_addr);
				flag_set_wifi = 0;
				E2prom_Write_Byte(EEP_WRITE_WIFI_MAC,0x44);
			}
		}
		IWDG_ReloadCounter();
		if(ATret != 2)  // not response OK
			continue;
		if(count_reboot_wifi > 20)
		{
			count_reboot_wifi = 0;
			ESP8266_Rst();
		}
		
		if((SSID_Info.IP_Wifi_Status != WIFI_NORMAL) && (SSID_Info.IP_Wifi_Status != WIFI_NO_WIFI ))//go into smart config mode
			//go into smart config mode
		{
			if(SSID_Info.IP_Auto_Manual == 0)
			{
				SSID_Info.ip_addr[0] = 0;
				SSID_Info.ip_addr[1] = 0;
				SSID_Info.ip_addr[2] = 0;
				SSID_Info.ip_addr[3] = 0;
			}
			
			if(flag_start_smart == 0)
			{
				if(ESP8266_Cmd ("AT+CWSTARTSMART", "OK", 0, 1000 ) )
					flag_start_smart = 1;
			}
			
			if(flag_start_smart)
			{
					count = 0;	
					do
					{// try to wait samrtconfig 20s 
						SSID_Info.IP_Wifi_Status = ESP8266_Get_LinkStatus();
						delay_ms(100);
						IWDG_ReloadCounter();
					}while(SSID_Info.IP_Wifi_Status > 3 && count++ < 200);
					
					if(count >= 200)
					{
						connect_AP();	
						count_reboot_wifi++;
						continue;
					}

				// Clear SSID
					// 一旦通过smartconfig方式配置SSID_Info.MANUEL_EN要设为0
					SSID_Info.MANUEL_EN = 0;
					memset(SSID_Info.name,0,64);
					memset(SSID_Info.password,0,32);
					write_page_en[24] = 1; 
					Flash_Write_Mass();
					ESP8266_Cmd( "AT+CWSTOPSMART", "OK", 0, 500 );
					delay_ms(100);
					connect_AP();
					
			}
			delay_ms(200) ;
		}
		else
		{
			count_reboot_wifi = 0;
			if(!get_ip)
			{
				if(ESP8266_Inquire_ApIp(SSID_Info.mac_addr,SSID_Info.ip_addr,40))
				{
					if(SSID_Info.ip_addr[0] == 0 && SSID_Info.ip_addr[1] == 0
						&& SSID_Info.ip_addr[2] == 0 && SSID_Info.ip_addr[3] == 0)
						get_ip = 0;
					else
						get_ip = 1;
				}
				ESP8266_CIPSTA_CUR(0); 
				
				ESP8266_Cmd( "AT+CIPMODE=0", "OK", 0, 500 );
				
				ESP8266_Enable_MultipleId ( ENABLE );
				IWDG_ReloadCounter();
				
				ESP8266_Cmd( "AT+CIPCLOSE=5", "OK", 0, 500 );
				
				count = 0;
				while ((!ESP8266_Link_UDP("255.255.255.255",SSID_Info.bacnet_port,SSID_Info.bacnet_port,2,UCID_BACNET)) && (count++ < 5))
					IWDG_ReloadCounter();
				
				count = 0;
				while ((!ESP8266_Link_UDP("255.255.255.255",1234,1234,2,UCID_SCAN)) && (count++ < 5))
					IWDG_ReloadCounter();
				
//				count = 0;
//					while ((!ESP8266_Link_UDP("192.168.10.110",123,123,2,UCID_NTP)) && (count++ < 5))
//					IWDG_ReloadCounter();
					
				count = 0;				
				while ((!ESP8266_StartOrShutServer(ENABLE,tcp_port,&overtime)) && (count++ < 5)) 
					IWDG_ReloadCounter();

			}
			else
			{
				memset(packet,0,1024);
				memset(cStr,0,1024);
				packet_len = 0;

				memcpy(cStr,ESP8266_ReceiveString(DISABLE),1024);
// clear rx buffer	
				ucID = cStr[7] - '0';
				packet_len = check_packet(cStr,packet);
				if(packet_len > 0)
				{		
					if(ucID >= 2 && ucID <= 4)  // modbus TCP 502
					{
						// tbd: add more for dns
						if(packet[0] == 0x1c && packet_len == 48)
						{ // ntp
							// receive data
							// close 
							SNTPC_Receive(packet, packet_len, 0);
						}
				// check modbus data
						else if((packet[0] == 0xee) && (packet[1] == 0x10) &&
						(packet[2] == 0x00) && (packet[3] == 0x00) &&
						(packet[4] == 0x00) && (packet[5] == 0x00) &&
						(packet[6] == 0x00) && (packet[7] == 0x00) )
						{		
				//			Udtcp_server_databuf(0);
				//			send_flag = 1;
				//			update_firmware = 1;
								SoftReset();
						}
						else if(packet[6] == Modbus.address 
						|| ((packet[6] == 255) && (packet[7] != 0x19)))
						{	
							responseCmd(WIFI, packet);
							if(modbus_wifi_len > 0)
							{
								ESP8266_SendString ( DISABLE, (uint8_t *)&modbus_wifi_buf, modbus_wifi_len,cStr[7] - '0' );
								modbus_wifi_len = 0;								
							}
							
							Check_connect_AP();  // check whether wirte SSID

						}
						else
						{
							// transfer data to sub ,TCP TO RS485
							U8_T header[6];	
							U8_T i;
							if((packet[UIP_HEAD] == 0x00) || 
							((packet[UIP_HEAD + 1] != READ_VARIABLES) 
							&& (packet[UIP_HEAD + 1] != WRITE_VARIABLES) 
							&& (packet[UIP_HEAD + 1] != MULTIPLE_WRITE) 
							&& (packet[UIP_HEAD + 1] != CHECKONLINE)
							&& (packet[UIP_HEAD + 1] != READ_COIL)
							&& (packet[UIP_HEAD + 1] != READ_DIS_INPUT)
							&& (packet[UIP_HEAD + 1] != READ_INPUT)
							&& (packet[UIP_HEAD + 1] != WRITE_COIL)
							&& (packet[UIP_HEAD + 1] != WRITE_MULTI_COIL)
							&& (packet[UIP_HEAD + 1] != CHECKONLINE_WIHTCOM)))
							{
								continue;
							}
							if((packet[UIP_HEAD + 1] == MULTIPLE_WRITE) && ((packet_len - UIP_HEAD) != (packet[UIP_HEAD + 6] + 7)))
							{
								continue;
							}	

							if(Modbus.com_config[2] == MODBUS_MASTER)
								Modbus.sub_port = 2;
							else if(Modbus.com_config[0] == MODBUS_MASTER)
								Modbus.sub_port = 0;
							else if(Modbus.com_config[1] == MODBUS_MASTER)
								Modbus.sub_port = 1;
							else
							{
								continue;
							}

							for(i = 0;i <  sub_no ;i++)
							{
								if(packet[UIP_HEAD] == uart2_sub_addr[i])
								{
									Modbus.sub_port = 2;
									continue;
								}
								else if(packet[UIP_HEAD] == uart0_sub_addr[i])
								{
									 Modbus.sub_port = 0;
									 continue;
								}
								else if(packet[UIP_HEAD] == uart1_sub_addr[i])
								{	
									Modbus.sub_port = 1;
									continue;
								}
							}		

							Set_transaction_ID(header, ((U16_T)packet[0] << 8) | packet[1], 2 * packet[UIP_HEAD + 5] + 3);
							
							Response_TCPIP_To_SUB(packet + UIP_HEAD,packet_len - UIP_HEAD,Modbus.sub_port,header);
							if(modbus_wifi_len > 0)
							{
								ESP8266_SendString ( DISABLE, (uint8_t *)&modbus_wifi_buf, modbus_wifi_len,cStr[7] - '0' );
								modbus_wifi_len = 0;
							}	

						}

					}	
					else 	if(ucID == UCID_BACNET) // udp bacnet port 47808
					{
						U8_T i;
						uint16_t pdu_len = 0;  
						BACNET_ADDRESS far src;
						count = 0;
						if(flag_wifi == 0)
						{						
							flag_wifi = 1;
							pdu_len = datalink_receive(&src, &packet[0], packet_len, 0 ,BAC_IP);
							{
								if((pdu_len > 0) && (pdu_len < 512)) 
								{
									npdu_handler(&src, &packet[0], pdu_len, BAC_IP);	
									// check whether need wait rec_mstp_index1
									
									if(count_hold_on_bip_to_mstp > 0)
									{
										i = 0;
										while(rec_mstp_index1 == 0 && i++ < 10)
											delay_ms(50);
										if(rec_mstp_index1 > 0)
										{	
											count_hold_on_bip_to_mstp = 0;											
											for(i = 0;i < rec_mstp_index1;i++)									
											{
												ESP8266_SendString ( DISABLE, (uint8_t *)mstp_bac_buf1[i].buf, mstp_bac_buf1[i].len,cStr[7] - '0' );
												delay_ms(1);
											}										
											rec_mstp_index1 = 0;
										}
										
									}		
													
									if(bacnet_wifi_len > 0)
									{
										ESP8266_SendString ( DISABLE, (uint8_t *)&bacnet_wifi_buf, bacnet_wifi_len,cStr[7] - '0' );
										bacnet_wifi_len = 0;											
									}	
									
									if(rec_mstp_index > 0)
									{
										for(i = 0;i < rec_mstp_index;i++)
										{
											if(mstp_bac_buf[i].len > 0)
											{delay_ms(1);
												ESP8266_SendString ( DISABLE, (uint8_t *)&mstp_bac_buf[i].buf, mstp_bac_buf[i].len,cStr[7] - '0' );
											}
										}
										rec_mstp_index = 0;
									}																					
								}			
							}	
						}
						flag_wifi = 0;
					}					
					else if(ucID == UCID_SCAN) // private scan port
					{ 
						u8 n;
						u8 i;
						if(packet[0] == 0x64)
						{		
							state = 1;
							for(n = 0;n < (u8)packet_len / 4;n++)
							{       
								if((packet[4*n+1] == SSID_Info.ip_addr[0]) && (packet[4*n+2] == SSID_Info.ip_addr[1])
									 &&(packet[4*n+3] == SSID_Info.ip_addr[2]) && (packet[4*n+4] == SSID_Info.ip_addr[3]))
								{ 
									 state=0;
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
								
								Scan_Infor.ipaddr[0] = (U16_T)SSID_Info.ip_addr[0];
								Scan_Infor.ipaddr[1] = (U16_T)SSID_Info.ip_addr[1];
								Scan_Infor.ipaddr[2] = (U16_T)SSID_Info.ip_addr[2];
								Scan_Infor.ipaddr[3] = (U16_T)SSID_Info.ip_addr[3];
								
								memcpy(&Scan_Infor.panelname,panelname,20);			
								//uip_send((char *)&Scan_Infor, sizeof(STR_SCAN_CMD));
								Scan_Infor.zigbee_exist = zigbee_exist | 0x02; 
								ESP8266_SendString ( DISABLE, (uint8_t *)&Scan_Infor, sizeof(STR_SCAN_CMD),cStr[7] - '0' );							
							//	rec_scan_index = 0;
							
							// for MODBUS device
								for(i = 0;i < sub_no;i++)
								{	 
									if((scan_db[i].product_model >= CUSTOMER_PRODUCT) || (current_online[scan_db[i].id / 8] & (1 << (scan_db[i].id % 8))))	  	 // in database but not on_line
									{
										if(scan_db[i].product_model != PRODUCT_MINI_BIG)
										{
											Scan_Infor.own_sn[0] = (U16_T)scan_db[i].sn;
											Scan_Infor.own_sn[1] = (U16_T)(scan_db[i].sn >> 8) & 0x00ff;
											Scan_Infor.own_sn[2] = (U16_T)(scan_db[i].sn >> 16) & 0x00ff;
											Scan_Infor.own_sn[3] = (U16_T)(scan_db[i].sn >> 24) & 0x00ff;					

											Scan_Infor.product = (U16_T)scan_db[i].product_model;
											Scan_Infor.address = (U16_T)scan_db[i].id;									
											
											Scan_Infor.master_sn[0] = Modbus.serialNum[0];
											Scan_Infor.master_sn[1] = Modbus.serialNum[1];
											Scan_Infor.master_sn[2] = Modbus.serialNum[2];
											Scan_Infor.master_sn[3] = Modbus.serialNum[3];
											
											memcpy(&Scan_Infor.panelname,tstat_name[i],20);
												
											Scan_Infor.zigbee_exist = zigbee_exist | 0x02; 
												
											ESP8266_SendString ( DISABLE, (uint8_t *)&Scan_Infor, sizeof(STR_SCAN_CMD),cStr[7] - '0' );
											//uip_send((char *)InformationStr, 60);
		//									 if(rec_scan_index < 19)
		//										 memcpy(&Infor[rec_scan_index++],&Scan_Infor, sizeof(STR_SCAN_CMD));
										}	
									}						
								}
								
				// if id confict, send it to T3000
							for(i = 0;i < index_id_conflict;i++)	
							//if(conflict_num == 1)
							{
								Scan_Infor.own_sn[0] = (U16_T)id_conflict[i].sn_old;
								Scan_Infor.own_sn[1] = (U16_T)(id_conflict[i].sn_old >> 8);
								Scan_Infor.own_sn[2] = (U16_T)(id_conflict[i].sn_old >> 16);
								Scan_Infor.own_sn[3] = (U16_T)(id_conflict[i].sn_old >> 24);					

								Scan_Infor.product = 0x08;//(U16_T)scan_db[i].product_model << 8;
								Scan_Infor.address = (U16_T)id_conflict[i].id;
						
								
								Scan_Infor.master_sn[0] = Modbus.serialNum[0];
								Scan_Infor.master_sn[1] = Modbus.serialNum[1];
								Scan_Infor.master_sn[2] = Modbus.serialNum[2];
								Scan_Infor.master_sn[3] = Modbus.serialNum[3];
								
		//						if(rec_scan_index < 19)
		//							memcpy(&Infor[rec_scan_index++],&Scan_Infor, sizeof(STR_SCAN_CMD));
								
								Scan_Infor.own_sn[0] = (U16_T)id_conflict[i].sn_new;
								Scan_Infor.own_sn[1] = (U16_T)(id_conflict[i].sn_new >> 8);
								Scan_Infor.own_sn[2] = (U16_T)(id_conflict[i].sn_new >> 16);
								Scan_Infor.own_sn[3] = (U16_T)(id_conflict[i].sn_new >> 24);		
								Scan_Infor.zigbee_exist = zigbee_exist | 0x02; 
								
								ESP8266_SendString ( DISABLE, (uint8_t *)&Scan_Infor, sizeof(STR_SCAN_CMD),cStr[7] - '0' );
								//uip_send((char *)InformationStr, 60);
		//						if(rec_scan_index < 19)
		//						 memcpy(&Infor[rec_scan_index++],&Scan_Infor,  sizeof(STR_SCAN_CMD));
							}

								
					// for MSTP device		
								for(i = 0;i < remote_panel_num;i++)
								{	
									if(remote_panel_db[i].protocal == BAC_MSTP)
									{
				//						BACNET_ADDRESS dest = { 0 };
				//						uint16 max_apdu = 0;
				//						bool status = false;

										/* is the device bound? */
										//status = address_get_by_device(remote_panel_db[i].device_id, &max_apdu, &dest);
										
				//						if(status > 0)
										{
											char temp_name[20];
											Scan_Infor.own_sn[0] = (U16_T)remote_panel_db[i].device_id;
											Scan_Infor.own_sn[1] = (U16_T)(remote_panel_db[i].device_id >> 8);
											Scan_Infor.own_sn[2] = (U16_T)(remote_panel_db[i].device_id >> 16);
											Scan_Infor.own_sn[3] = (U16_T)(remote_panel_db[i].device_id >> 24);
											
											
											Scan_Infor.product = remote_panel_db[i].product_model;//0x08; //(U16_T)scan_db[i].product_model << 8; tbd:
											Scan_Infor.address = remote_panel_db[i].sub_id;			
											
											Scan_Infor.master_sn[0] = Modbus.serialNum[0];
											Scan_Infor.master_sn[1] = Modbus.serialNum[1];
											Scan_Infor.master_sn[2] = Modbus.serialNum[2];
											Scan_Infor.master_sn[3] = Modbus.serialNum[3];	
										
											
											Scan_Infor.instance_low = HTONS(remote_panel_db[i].device_id); // hight byte first
											Scan_Infor.panel_number = remote_panel_db[i].panel;	
											Scan_Infor.instance_hi = HTONS(remote_panel_db[i].device_id >> 16); // hight byte first
											
											memset(temp_name,0,20);
				//							strcmp(temp_name, "panel:"/*, remote_panel_db[i].sub_id*/);
											temp_name[0] = 'M';
											temp_name[1] = 'S';
											temp_name[2] = 'T';
											temp_name[3] = 'P';
											temp_name[4] = ':';
				//							temp_name[5] = Scan_Infor.station_num / 10 + '0';
				//							temp_name[6] = Scan_Infor.station_num % 10 + '0';							
											
											temp_name[19] = '\0';
											memcpy(&Scan_Infor.panelname,temp_name,20);
											
											Scan_Infor.subnet_protocal = 12;  // MSTP device
											Scan_Infor.zigbee_exist = zigbee_exist | 0x02;	
											delay_ms(1);
											ESP8266_SendString ( DISABLE, (uint8_t *)&Scan_Infor, sizeof(STR_SCAN_CMD),cStr[7] - '0' );
		//									if(rec_scan_index < 19)
		//									memcpy(&Infor[rec_scan_index++],&Scan_Infor, sizeof(STR_SCAN_CMD));		
										}						
									}				
								}								
							}
						}
					}			
				}
				if(count_checkip % 100 == 0)
				{
					static u8 count_normal = 0;
					IWDG_ReloadCounter();
					delay_ms(1000);
					if(Get_SSID_RSSI() == 0)
					{
						count_normal++;
					}
					else
					{
						SSID_Info.IP_Wifi_Status = WIFI_NORMAL;
						count_normal = 0;
					}
					
					if(count_normal > 3)
					{
						SSID_Info.IP_Wifi_Status = WIFI_NO_WIFI;
						get_ip = 0;
						count_normal = 0;
					}
				}
				
				if(count_checkip++ > 300)
				{
					count_checkip = 0;
					IWDG_ReloadCounter();
					if(ESP8266_CIPSTA_CUR(1) == 2)
						get_ip = 0;					
				}			
			}
		}	
	}
}

#if ARM_TSTAT_WIFI
void Set_transaction_ID(U8_T *str, U16_T id, U16_T num)
{
	str[0] = (U8_T)(id >> 8);		//transaction id
	str[1] = (U8_T)id;

	str[2] = 0;						//protocol id, modbus protocol = 0
	str[3] = 0;

	str[4] = (U8_T)(num >> 8);
	str[5] = (U8_T)num;
}
#endif

void vStartWifiTasks( U8_T uxPriority)
{
#if ARM_TSTAT_WIFI
	watchdog_init();	
#endif
	sTaskCreate( WIFI_task, "WIFI_task", WifiSTACK_SIZE, NULL, uxPriority, &Wifi_Handler );

}


uint8_t SendBuff[SENDBUFF_SIZE]; //用于DMA 串口发送



//dma2的初始化，注意tx和rx的dma初始化时不同的，包括存储器地址（外设地址相同，都是uart->DR），另外dma的方向也不相同。
void dma_init_uart4()
{
    DMA_InitTypeDef DMA_InitTypeStruct;
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

    DMA_DeInit(DMA2_Channel5);   //串口1的DMA传输通道是通道4
    DMA_InitTypeStruct.DMA_BufferSize = SENDBUFF_SIZE; //传输大小
    DMA_InitTypeStruct.DMA_DIR = DMA_DIR_PeripheralDST; //外设作为DMA的目的端
    DMA_InitTypeStruct.DMA_M2M = DMA_M2M_Disable;  //??????????
    DMA_InitTypeStruct.DMA_MemoryBaseAddr = (u32)SendBuff; //??????
    DMA_InitTypeStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //???????? ??
    DMA_InitTypeStruct.DMA_MemoryInc = DMA_MemoryInc_Enable; //???????
    DMA_InitTypeStruct.DMA_Mode = DMA_Mode_Circular; //DMA_Mode_Normal（只传送一次）, DMA_Mode_Circular （不停地传送）
                                                     //DMA_InitTypeStruct.DMA_PeripheralBaseAddr = (uint32_t)&(UART4->DR); //;(u32)&USART1->DR; //?????
    DMA_InitTypeStruct.DMA_PeripheralBaseAddr = (u32)&UART4->DR; //?????
    DMA_InitTypeStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //?????? ??
    DMA_InitTypeStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //外设地址不增加
    DMA_InitTypeStruct.DMA_Priority = DMA_Priority_Medium;  //(DMA传送优先级为中等)
    DMA_Init(DMA2_Channel5, &DMA_InitTypeStruct);

}


void dma_send_uart4_data(uint16 nlength)
{
    USART_DMACmd(UART4, USART_DMAReq_Tx, ENABLE);
    DMA_SetCurrDataCounter(DMA2_Channel5, nlength );
    DMA_Cmd(DMA2_Channel5, ENABLE);


    while (DMA_GetFlagStatus(DMA2_FLAG_TC5) == RESET);
    DMA_ClearFlag(DMA2_FLAG_TC5);

    DMA_Cmd(DMA2_Channel5, DISABLE);
    USART_DMACmd(UART4, USART_DMAReq_Tx, DISABLE);
}



unsigned char Send_Uart_Data(char * m_send_data, uint16 nlength)
{
    memcpy(SendBuff, m_send_data, nlength);
    dma_send_uart4_data(nlength);
}

void Sync_with_NTP_by_Wifi(void)
{
	char count;
	flag_send_ntp_by_wifi = 0;
	count = 0;
	while ((!ESP8266_Link_UDP("176.221.42.125",123,123,2,UCID_NTP)) && (count++ < 5))
	IWDG_ReloadCounter();
	{
		U8_T len = 48;
		U8_T i;
		U8_T far Buf[48];
		len = 48;
	
		Buf[0] = 0xdb;
		Buf[1] = 0x00;
		Buf[2] = 0x04;
		Buf[3] = 0xfa;
		Buf[4] = 0x00;
		Buf[5] = 0x01;
		Buf[6] = 0x00;
		Buf[7] = 0x00;
		Buf[8] = 0x00;
		Buf[9] = 0x01;
		for(i = 10;i < len;i++)
		{
			Buf[i] = 0;
		}						
		ESP8266_SendString ( DISABLE, (uint8_t *)&Buf, len,UCID_NTP);
	}
}


//void ntp_initial(void)
//{
//	resolv_init();
////		resolv_query("newfirmware.com");
//	if(Modbus.en_sntp > 1)  // 0 - no, 1-disable
//		SNTPC_Init();
//	check_entries();
//	if(Modbus.en_dyndns == 2)
//	{
//		resolv_query("www.3322.org");
//		resolv_query("www.dyndns.com");
//		resolv_query("www.no-ip.com");	
//	}
//}
