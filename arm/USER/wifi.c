#include "main.h"
#include "wifi.h"
#include "bsp_esp8266.h"

#define UIP_HEAD 6

#define WifiSTACK_SIZE 1024
xTaskHandle Wifi_Handler;
/**
  * @brief  初始化ESP8266用到的GPIO引脚
  * @param  无
  * @retval 无
  */
static void ESP8266_GPIO_Config ( void )
{
	/*定义一个GPIO_InitTypeDef类型的结构体*/
	GPIO_InitTypeDef GPIO_InitStructure;


	/* 配置 CH_PD 引脚*/
//	macESP8266_CH_PD_APBxClock_FUN ( macESP8266_CH_PD_CLK, ENABLE ); 
//											   
//	GPIO_InitStructure.GPIO_Pin = macESP8266_CH_PD_PIN;	

//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
//   
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 

//	GPIO_Init ( macESP8266_CH_PD_PORT, & GPIO_InitStructure );	 

	
	/* 配置 RST 引脚*/
	macESP8266_RST_APBxClock_FUN ( macESP8266_RST_CLK, ENABLE ); 
											   
	GPIO_InitStructure.GPIO_Pin = macESP8266_RST_PIN;	

	GPIO_Init ( macESP8266_RST_PORT, & GPIO_InitStructure );	 


}



void UART4_IRQHandler(void)                	
{		
	uint8_t ucCh;
	if ( USART_GetITStatus ( macESP8266_USARTx, USART_IT_RXNE ) != RESET )
	{
		ucCh  = USART_ReceiveData( macESP8266_USARTx );
		
		if ( strEsp8266_Fram_Record .InfBit .FramLength < ( RX_BUF_MAX_LEN - 1 ) )                       //预留1个字节写结束符
			strEsp8266_Fram_Record .Data_RX_BUF [ strEsp8266_Fram_Record .InfBit .FramLength ++ ]  = ucCh;

	}
	 	 
	if ( USART_GetITStatus( macESP8266_USARTx, USART_IT_IDLE ) == SET )                                         //数据帧接收完毕
	{
    strEsp8266_Fram_Record .InfBit .FramFinishFlag = 1;
		
		ucCh = USART_ReceiveData( macESP8266_USARTx );                                                              //由软件序列清除中断标志位(先读USART_SR，然后读USART_DR)
	
//		ucTcpClosedFlag = strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "CLOSED\r\n" ) ? 1 : 0;
		
  }			
		
}


uint16_t check_packet(uint8_t * str,uint8_t * dat)
{
	//  +IPD,0,12:...
	uint16_t len = 0;
	uint16_t i;
	if(str[8] == ',')
	{
		if((str[10] == ':') && (str[9] >= '0' && str[9] <= '9'))
		{
			len = str[9] - '0';
			for(i = 0;i < len;i++)
				*dat++ = str[11 + i];
			return len;
		}
		else if(str[11] == ':')
		{
			if(str[9] >= '0' && str[9] <= '9' && str[10] >= '0' && str[10] <= '9')
			{
				len = (str[9] - '0') * 10 + (str[10] - '0');
				for(i = 0;i < len;i++)
					*dat++ = str[12 + i];
				return len;
			}
		}
		return 0;
	}
	return 0;
}


#define UCID_BACNET 0
#define UCID_SCAN 1

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
	S8_T panelname[20]; // 37 - 56
	U16_T instance_hi; // 57 58 hight byte first
	
	U8_T bootloader;  // 0 - app, 1 - bootloader, 2 - wrong bootloader , 3 - mstp device
	U16_T BAC_port;  //  hight byte first
	U8_T zigbee_exist; // 0 - inexsit, 1 - exist
	
	U8_T subnet_protocal; // 0 - modbus, 12 - bip to mstp

}STR_SCAN_CMD;

STR_SCAN_CMD Infor[20];
STR_SCAN_CMD Scan_Infor;
U8_T 	state;
uint8_t bacnet_wifi_buf[500];
uint16_t bacnet_wifi_len;
uint8_t modbus_wifi_buf[500];
uint16_t modbus_wifi_len;
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
	Scan_Infor.product = 9;

	//modbus address
	Scan_Infor.address = (U16_T)Modbus.address;
	
	//Ip
	Scan_Infor.ipaddr[0] = (U16_T)Modbus.ip_addr[0];
	Scan_Infor.ipaddr[1] = (U16_T)Modbus.ip_addr[1];
	Scan_Infor.ipaddr[2] = (U16_T)Modbus.ip_addr[2];
	Scan_Infor.ipaddr[3] = (U16_T)Modbus.ip_addr[3];
	
	//port
	Scan_Infor.modbus_port = 502;//swap_word(Modbus.tcp_port);  tbd :???????????????? 

	// software rev
	Scan_Infor.firmwarerev = swap_word(SW_REV / 10 + SW_REV % 10);
	// hardware rev
	Scan_Infor.hardwarerev = swap_word(Modbus.hardRev);
	
	Scan_Infor.instance_low = HTONS(Instance); // hight byte first
	Scan_Infor.panel_number = panel_number; //  36	
	Scan_Infor.instance_hi = HTONS(Instance >> 16); // hight byte first
	
	Scan_Infor.bootloader = 0;  // 0 - app, 1 - bootloader, 2 - wrong bootloader
	Scan_Infor.BAC_port = Modbus.Bip_port;//((Modbus.Bip_port & 0x00ff) << 8) + (Modbus.Bip_port >> 8);  // 
	Scan_Infor.zigbee_exist = zigbee_exist; // 0 - inexsit, 1 - exist
	Scan_Infor.subnet_protocal = 0;
	state = 1;
//	scanstart = 0;

}

void WIFI_task(void) reentrant//one second base software timer
{	
	uint8_t ip[4];
	uint16_t count;
	char overtime = 20;
	char ucID;
	char ret;
	char packet_len;
	uint8_t packet[200];
	uint8_t cStr [ 100 ] = { 0 };
	uint8_t ip1[4];
	uint8_t ip2[4];
	
	ESP8266_Init();
	
	macESP8266_CH_ENABLE();
	ret = ESP8266_AT_Test ();
	ESP8266_Net_Mode_Choose ( STA_AP  );
	count = 0;
  while (! ESP8266_JoinAP ( "Temco", "Travel321" ))
	{count++;
		IWDG_ReloadCounter();		// TBD:????????
	}
	ESP8266_Inquire_ApIp(ip1,Modbus.ip_addr,40);
#if ARM_UART_DEBUG
	uart1_init(115200);
	DEBUG_EN = 1;
	printf("wifi join ap %u %d %d %d %d\r\n",count,Modbus.ip_addr[0],Modbus.ip_addr[1],Modbus.ip_addr[2],Modbus.ip_addr[3]);
#endif	
	//if(count < 200)
	{
	ESP8266_Enable_MultipleId ( ENABLE );
	IWDG_ReloadCounter();
	while (!	ESP8266_Link_UDP ( "255.255.255.255",47808,47808,2,UCID_BACNET) )
	IWDG_ReloadCounter();
	while (!	ESP8266_Link_UDP ( "255.255.255.255",1234,1234,2,UCID_SCAN) )
	IWDG_ReloadCounter();
	while (!  ESP8266_StartOrShutServer(ENABLE,"502",&overtime)) 
		IWDG_ReloadCounter();
	}
	
	//LCDtest();

	for(;;)
	{	
		IWDG_ReloadCounter();
#if 1
		memset(packet,0,200);
		packet_len = 0;
		memcpy( cStr,ESP8266_ReceiveString(DISABLE),strEsp8266_Fram_Record .InfBit .FramLength);
		ucID = cStr[7] - '0';
		packet_len = check_packet(cStr,packet);
//		printf("str = %d %d %d %d %d\r\n",packet_len,packet[0],packet[1],packet[2],packet[3]);
//		printf("%d,%s\r\n",strEsp8266_Fram_Record .InfBit .FramLength,&cStr);
	
		if(packet_len > 0)
		{
			if(ucID == UCID_SCAN) // private scan port
			{ 
				u8 n;
				u8 i;
			//	u8 rec_scan_index;
				
				if(packet[0] == 0x64)
				{
					state = 1;
					for(n = 0;n < (u8)packet_len / 4;n++)
					{       
						if((packet[4*n+1] == Modbus.ip_addr[0]) && (packet[4*n+2] == Modbus.ip_addr[1])
							 &&(packet[4*n+3] == Modbus.ip_addr[2]) && (packet[4*n+4] == Modbus.ip_addr[3]))
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
						memcpy(&Scan_Infor.panelname,panelname,20);			
						//uip_send((char *)&Scan_Infor, sizeof(STR_SCAN_CMD));
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
									
								ESP8266_SendString ( DISABLE, (uint8_t *)&Scan_Infor, sizeof(STR_SCAN_CMD),cStr[7] - '0' );
									//uip_send((char *)InformationStr, 60);
//									 if(rec_scan_index < 19)
//										 memcpy(&Infor[rec_scan_index++],&Scan_Infor, sizeof(STR_SCAN_CMD));
								}	
							}						
						}
						
		// if id confict, send it to T3000
					if(conflict_num == 1)
					{
						Scan_Infor.own_sn[0] = (U16_T)conflict_sn_old;
						Scan_Infor.own_sn[1] = (U16_T)(conflict_sn_old >> 8);
						Scan_Infor.own_sn[2] = (U16_T)(conflict_sn_old >> 16);
						Scan_Infor.own_sn[3] = (U16_T)(conflict_sn_old >> 24);					

						Scan_Infor.product = 0x08;//(U16_T)scan_db[i].product_model << 8;
						Scan_Infor.address = (U16_T)conflict_id;
				
						
						Scan_Infor.master_sn[0] = Modbus.serialNum[0];
						Scan_Infor.master_sn[1] = Modbus.serialNum[1];
						Scan_Infor.master_sn[2] = Modbus.serialNum[2];
						Scan_Infor.master_sn[3] = Modbus.serialNum[3];
						
//						if(rec_scan_index < 19)
//							memcpy(&Infor[rec_scan_index++],&Scan_Infor, sizeof(STR_SCAN_CMD));
						
						Scan_Infor.own_sn[0] = (U16_T)conflict_sn_new;
						Scan_Infor.own_sn[1] = (U16_T)(conflict_sn_new >> 8);
						Scan_Infor.own_sn[2] = (U16_T)(conflict_sn_new >> 16);
						Scan_Infor.own_sn[3] = (U16_T)(conflict_sn_new >> 24);		
						
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
									
									ESP8266_SendString ( DISABLE, (uint8_t *)&Scan_Infor, sizeof(STR_SCAN_CMD),cStr[7] - '0' );
//									if(rec_scan_index < 19)
//									memcpy(&Infor[rec_scan_index++],&Scan_Infor, sizeof(STR_SCAN_CMD));		
								}						
							}				
						}				
					}
				}
			}
			else if(ucID == UCID_BACNET) // udp bacnet port 47808
			{
				uint16_t pdu_len = 0;  
				BACNET_ADDRESS far src;

				pdu_len = datalink_receive(&src, &packet[0], packet_len, 0,BAC_IP);
				{
					if(pdu_len) 
					{
						npdu_handler(&src, &packet[0], pdu_len,BAC_IP);	

						if(bacnet_wifi_len > 0)
						{
							ESP8266_SendString ( DISABLE, (uint8_t *)&bacnet_wifi_buf, bacnet_wifi_len,cStr[7] - '0' );
							bacnet_wifi_len = 0;
						}
					}			
				}
			}
			else  if(ucID == 2)  // modbus TCP 502
			{
		// check modbus data
				if( (packet[0] == 0xee) && (packet[1] == 0x10) &&
				(packet[2] == 0x00) && (packet[3] == 0x00) &&
				(packet[4] == 0x00) && (packet[5] == 0x00) &&
				(packet[6] == 0x00) && (packet[7] == 0x00) )
				{		
		//			Udtcp_server_databuf(0);
		//			send_flag = 1;
		//			update_firmware = 1;

				}
				else if(packet[6] == Modbus.address 
				|| ((packet[6] == 255) && (packet[7] != 0x19))
				)
				{	
		//			net_tx_count  = 2 ;
					responseCmd(WIFI, packet);
					if(modbus_wifi_len > 0)
					{
						ESP8266_SendString ( DISABLE, (uint8_t *)&modbus_wifi_buf, modbus_wifi_len,cStr[7] - '0' );
						modbus_wifi_len = 0;
					}
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
						return;
					}
					if((packet[UIP_HEAD + 1] == MULTIPLE_WRITE) && ((packet_len - UIP_HEAD) != (packet[UIP_HEAD + 6] + 7)))
					{
						return;
					}	

					if(Modbus.com_config[2] == MODBUS_MASTER)
						Modbus.sub_port = 2;
					else if(Modbus.com_config[0] == MODBUS_MASTER)
						Modbus.sub_port = 0;
					else if(Modbus.com_config[1] == MODBUS_MASTER)
						Modbus.sub_port = 1;
					else
					{
						return;
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
			
		}
#endif
		delay_ms(5) ;
	}
}

void Set_transaction_ID(U8_T *str, U16_T id, U16_T num)
{
	str[0] = (U8_T)(id >> 8);		//transaction id
	str[1] = (U8_T)id;

	str[2] = 0;						//protocol id, modbus protocol = 0
	str[3] = 0;

	str[4] = (U8_T)(num >> 8);
	str[5] = (U8_T)num;
}

void vStartWifiTasks( U8_T uxPriority)
{
	sTaskCreate( WIFI_task, "WIFI_task", WifiSTACK_SIZE, NULL, uxPriority, &Wifi_Handler );

}

#if 0
uint8 ip_read_flag = 0;//use to indicate if IP has been read back successful   0: no 1:yes
uint8 wifi_cmd_num = NORMAL_MODE;
uint8 const wifi_cmd[WIFI_TOTOAL_CMD][10]=
{
	"",
	"+++",
	"AT+WANN",
	"AT+WAP",
	"AT+WSTA",
	"AT+MAC",
	"AT+WSLK",
	"AT+SOCKA",
	"AT+SOCKB",
	"AT+RELD",
	"AT+ENTM"
};


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
	S8_T panelname[20]; // 37 - 56
	U16_T instance_hi; // 57 58 hight byte first
	
	U8_T bootloader;  // 0 - app, 1 - bootloader, 2 - wrong bootloader , 3 - mstp device
	U16_T BAC_port;  //  hight byte first
	U8_T zigbee_exist; // 0 - inexsit, 1 - exist
	
	U8_T subnet_protocal; // 0 - modbus, 12 - bip to mstp

}STR_SCAN_CMD;

STR_SCAN_CMD Scan_Infor;



#define TCP_HEADER  6
uint8 transactionID[6];
//uint16 checkonline_cnt = 0;
uint8 checkonline_flag = 0;
uint8 bip_flag = 0;
uint8 bip_rcv = 0;
uint8 checkonline_rcv = 0;
uint8 modbustcp_rcv = 0;


xTaskHandle Wifi_Handler;
xTaskHandle Wifi_dealwith_Handler;

#define NEED_TO_UNLOCK  1
#define NEED_TO_LOCK  	0



u8 USART_RX_BUFC[512];   
//u8 USART_RX_BUFD[50];
u16 rece_countB = 0;
u8 dealwithTagB = 0;
u8 uart_sendB[USART_SEND_LEN];
u16 sendbyte_numB = 0 ;
//u8 SERIAL_RECEIVE_TIMEOUT ;
u16 rece_sizeB = 0 ;
uint8 update_flag = 0;

u8 serial_receive_timeout_countB = 0;

//void USART_SendDataStringB(U8_T *p, u16 num )
// {
//	 memcpy(uart_sendB, p, num);
//	 sendbyte_numB = num;
//	 USART_ITConfig(UART4, USART_IT_TXE, ENABLE);//	 	
// } 
 
 void serial_restartB(void)
{
	rece_countB = 0;
	dealwithTagB = 0;
	USART_ITConfig(UART4, USART_IT_RXNE/*|USART_IT_TC*/, ENABLE);	
} 

 void modbus_initB(void)
{
	serial_restartB();
	//SERIAL_RECEIVE_TIMEOUT = 10;
	serial_receive_timeout_countB = 10;//SERIAL_RECEIVE_TIMEOUT;
}


void uart4_init(u32 bound)
{
    //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);	//使能USART4，GPIOD时钟
//	GPIO_PinRemapConfig(GPIO_Remap_USART2,1);
 	USART_DeInit(UART4);  //复位串口1
	//USART2_TX   PC.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;				//PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			//复用推挽输出
    GPIO_Init(GPIOC, &GPIO_InitStructure);					//初始化PA9
   
    //USART2_RX	  PC.11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;				//PA.10
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//上拉输入
    GPIO_Init(GPIOC, &GPIO_InitStructure);					//初始化PA10
 
	
	//Usart2 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);								//根据指定的参数初始化VIC寄存器
  
	//USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;					//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_2;		//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;			//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//收发模式
    USART_Init(UART4, &USART_InitStructure); 					//初始化串口

    USART_ITConfig(UART4, USART_IT_RXNE/*|USART_IT_TC*/, ENABLE);				//开启中断
//    USART_ITConfig(USART1, USART_IT_TC, ENABLE);
		USART_Cmd(UART4, ENABLE);                    				
}
	

void UART4_IRQHandler(void)                	
{		

	uint8 i;
	uint8 ip_start,ip_end;
	static u16 send_countB = 0;
	static uint16 rev_cnt;
	
	if(USART_GetITStatus(UART4, USART_IT_RXNE) == SET)	//
	{
		  if(wifi_cmd_num != NORMAL_MODE)
		  {
				if(wifi_cmd_num == CMD_SWITCH_MODE)
				{
					USART_RX_BUFC[rev_cnt++] = USART_ReceiveData(UART4);
			    if(( rev_cnt >= 4)&&(ip_read_flag == 0))
					{
						if((USART_RX_BUFC[0] == 'a') && (USART_RX_BUFC[1] == '+') && (USART_RX_BUFC[2] == 'O') && (USART_RX_BUFC[3] == 'K'))
						{
							ip_read_flag = 1;
							rev_cnt = 0;							
						}
						else if( (USART_RX_BUFC[rev_cnt-3] == '+') && (USART_RX_BUFC[rev_cnt-2] == 'O') && (USART_RX_BUFC[rev_cnt-1] == 'K'))
						{
							ip_read_flag = 1;
							rev_cnt = 0;	
						}
					}					
				}
				else if(wifi_cmd_num == CMD_READ_STA_INFO)
				{
					USART_RX_BUFC[rev_cnt++] = USART_ReceiveData(UART4);
					//if(rev_cnt == 9)
					if(rev_cnt >= 50)//
					{
						if((USART_RX_BUFC[rev_cnt-2] == 0x0d) && (USART_RX_BUFC[rev_cnt-1] == 0x0a))//received whole package 
						{
							if((USART_RX_BUFC[13] == 'D') && (USART_RX_BUFC[14] == 'H') && (USART_RX_BUFC[15] == 'C') && (USART_RX_BUFC[16] == 'P'))//DHCP MODE
							{
								ip_start = 18;
								for(i=1;i<4;i++)
								{
									if(USART_RX_BUFC[ip_start+i] == '.')
									{
										ip_end = i;
										break;
									}
								}
								Modbus.ip_addr[0] = 0;

								if(ip_end==1)
									Modbus.ip_addr[0] += (USART_RX_BUFC[ip_start+ip_end-1]-0x30);
								else if(ip_end == 2)
								{
									Modbus.ip_addr[0] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.ip_addr[0] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}
								else if(ip_end == 3)
								{
									Modbus.ip_addr[0] += (USART_RX_BUFC[ip_start+ip_end-3]-0x30) * 100;
									Modbus.ip_addr[0] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.ip_addr[0] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}						


								ip_start = ip_start + ip_end + 1;
								for(i=1;i<4;i++)
								{
									if(USART_RX_BUFC[ip_start+i] == '.')
									{
										ip_end = i;
										break;
									}
								}
								Modbus.ip_addr[1] = 0;

								if(ip_end==1)
									Modbus.ip_addr[1] += (USART_RX_BUFC[ip_start+ip_end-1]-0x30);
								else if(ip_end == 2)
								{
									Modbus.ip_addr[1] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.ip_addr[1] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}
								else if(ip_end == 3)
								{
									Modbus.ip_addr[1] += (USART_RX_BUFC[ip_start+ip_end-3]-0x30) * 100;
									Modbus.ip_addr[1] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.ip_addr[1] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}
								
								ip_start = ip_start + ip_end + 1;
								for(i=1;i<4;i++)
								{
									if(USART_RX_BUFC[ip_start+i] == '.')
									{
										ip_end = i;
										break;
									}
								}
								Modbus.ip_addr[2] = 0;
								if(ip_end==1)
									Modbus.ip_addr[2] += (USART_RX_BUFC[ip_start+ip_end-1]-0x30);
								else if(ip_end == 2)
								{
									Modbus.ip_addr[2] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.ip_addr[2] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}
								else if(ip_end == 3)
								{
									Modbus.ip_addr[2] += (USART_RX_BUFC[ip_start+ip_end-3]-0x30) * 100;
									Modbus.ip_addr[2] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.ip_addr[2] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}
								ip_start = ip_start + ip_end + 1;
								for(i=1;i<4;i++)
								{
									if(USART_RX_BUFC[ip_start+i] == ',')
									{
										ip_end = i;
										break;
									}
								}
								Modbus.ip_addr[3] = 0;
								if(ip_end==1)
									Modbus.ip_addr[3] += (USART_RX_BUFC[ip_start+ip_end-1]-0x30);
								else if(ip_end == 2)
								{
									Modbus.ip_addr[3] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.ip_addr[3] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}
								else if(ip_end == 3)
								{
									Modbus.ip_addr[3] += (USART_RX_BUFC[ip_start+ip_end-3]-0x30) * 100;
									Modbus.ip_addr[3] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.ip_addr[3] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}	

								ip_start = ip_start + ip_end + 1;
								for(i=1;i<4;i++)
								{
									if(USART_RX_BUFC[ip_start+i] == '.')
									{
										ip_end = i;
										break;
									}
								}
								Modbus.subnet[0] = 0;

								if(ip_end==1)
									Modbus.subnet[0] += (USART_RX_BUFC[ip_start+ip_end-1]-0x30);
								else if(ip_end == 2)
								{
									Modbus.subnet[0] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.subnet[0] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}
								else if(ip_end == 3)
								{
									Modbus.subnet[0] += (USART_RX_BUFC[ip_start+ip_end-3]-0x30) * 100;
									Modbus.subnet[0] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.subnet[0] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}						


								ip_start = ip_start + ip_end + 1;
								for(i=1;i<4;i++)
								{
									if(USART_RX_BUFC[ip_start+i] == '.')
									{
										ip_end = i;
										break;
									}
								}
								Modbus.subnet[1] = 0;

								if(ip_end==1)
									Modbus.subnet[1] += (USART_RX_BUFC[ip_start+ip_end-1]-0x30);
								else if(ip_end == 2)
								{
									Modbus.subnet[1] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.subnet[1] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}
								else if(ip_end == 3)
								{
									Modbus.subnet[1] += (USART_RX_BUFC[ip_start+ip_end-3]-0x30) * 100;
									Modbus.subnet[1] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.subnet[1] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}
								
								ip_start = ip_start + ip_end + 1;
								for(i=1;i<4;i++)
								{
									if(USART_RX_BUFC[ip_start+i] == '.')
									{
										ip_end = i;
										break;
									}
								}
								Modbus.subnet[2] = 0;
								if(ip_end==1)
									Modbus.subnet[2] += (USART_RX_BUFC[ip_start+ip_end-1]-0x30);
								else if(ip_end == 2)
								{
									Modbus.subnet[2] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.subnet[2] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}
								else if(ip_end == 3)
								{
									Modbus.subnet[2] += (USART_RX_BUFC[ip_start+ip_end-3]-0x30) * 100;
									Modbus.subnet[2] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.subnet[2] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}
								ip_start = ip_start + ip_end + 1;
								for(i=1;i<4;i++)
								{
									if(USART_RX_BUFC[ip_start+i] == ',')
									{
										ip_end = i;
										break;
									}
								}
								Modbus.subnet[3] = 0;
								if(ip_end==1)
									Modbus.subnet[3] += (USART_RX_BUFC[ip_start+ip_end-1]-0x30);
								else if(ip_end == 2)
								{
									Modbus.subnet[3] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.subnet[3] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}
								else if(ip_end == 3)
								{
									Modbus.subnet[3] += (USART_RX_BUFC[ip_start+ip_end-3]-0x30) * 100;
									Modbus.subnet[3] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.subnet[3] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}	

								
								rev_cnt = 0;
								wifi_cmd_num = CMD_READ_MAC;
								ip_read_flag = 2;
							}
								
							else if((USART_RX_BUFC[13] == 'S') && (USART_RX_BUFC[14] == 'T') && (USART_RX_BUFC[15] == 'A') && (USART_RX_BUFC[16] == 'T'))//static mode
							{
								ip_start = 20;
								for(i=1;i<4;i++)
								{
									if(USART_RX_BUFC[ip_start+i] == '.')
									{
										ip_end = i;
										break;
									}
								}
								Modbus.ip_addr[0] = 0;

								if(ip_end==1)
									Modbus.ip_addr[0] += (USART_RX_BUFC[ip_start+ip_end-1]-0x30);
								else if(ip_end == 2)
								{
									Modbus.ip_addr[0] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.ip_addr[0] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}
								else if(ip_end == 3)
								{
									Modbus.ip_addr[0] += (USART_RX_BUFC[ip_start+ip_end-3]-0x30) * 100;
									Modbus.ip_addr[0] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.ip_addr[0] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}						


								ip_start = ip_start + ip_end + 1;
								for(i=1;i<4;i++)
								{
									if(USART_RX_BUFC[ip_start+i] == '.')
									{
										ip_end = i;
										break;
									}
								}
								Modbus.ip_addr[1] = 0;

								if(ip_end==1)
									Modbus.ip_addr[1] += (USART_RX_BUFC[ip_start+ip_end-1]-0x30);
								else if(ip_end == 2)
								{
									Modbus.ip_addr[1] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.ip_addr[1] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}
								else if(ip_end == 3)
								{
									Modbus.ip_addr[1] += (USART_RX_BUFC[ip_start+ip_end-3]-0x30) * 100;
									Modbus.ip_addr[1] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.ip_addr[1] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}
								
								ip_start = ip_start + ip_end + 1;
								for(i=1;i<4;i++)
								{
									if(USART_RX_BUFC[ip_start+i] == '.')
									{
										ip_end = i;
										break;
									}
								}
								Modbus.ip_addr[2] = 0;
								if(ip_end==1)
									Modbus.ip_addr[2] += (USART_RX_BUFC[ip_start+ip_end-1]-0x30);
								else if(ip_end == 2)
								{
									Modbus.ip_addr[2] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.ip_addr[2] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}
								else if(ip_end == 3)
								{
									Modbus.ip_addr[2] += (USART_RX_BUFC[ip_start+ip_end-3]-0x30) * 100;
									Modbus.ip_addr[2] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.ip_addr[2] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}
								ip_start = ip_start + ip_end + 1;
								for(i=1;i<4;i++)
								{
									if(USART_RX_BUFC[ip_start+i] == ',')
									{
										ip_end = i;
										break;
									}
								}
								Modbus.ip_addr[3] = 0;
								if(ip_end==1)
									Modbus.ip_addr[3] += (USART_RX_BUFC[ip_start+ip_end-1]-0x30);
								else if(ip_end == 2)
								{
									Modbus.ip_addr[3] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.ip_addr[3] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}
								else if(ip_end == 3)
								{
									Modbus.ip_addr[3] += (USART_RX_BUFC[ip_start+ip_end-3]-0x30) * 100;
									Modbus.ip_addr[3] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.ip_addr[3] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}

								ip_start = ip_start + ip_end + 1;
								for(i=1;i<4;i++)
								{
									if(USART_RX_BUFC[ip_start+i] == '.')
									{
										ip_end = i;
										break;
									}
								}
								Modbus.subnet[0] = 0;

								if(ip_end==1)
									Modbus.subnet[0] += (USART_RX_BUFC[ip_start+ip_end-1]-0x30);
								else if(ip_end == 2)
								{
									Modbus.subnet[0] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.subnet[0] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}
								else if(ip_end == 3)
								{
									Modbus.subnet[0] += (USART_RX_BUFC[ip_start+ip_end-3]-0x30) * 100;
									Modbus.subnet[0] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.subnet[0] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}						


								ip_start = ip_start + ip_end + 1;
								for(i=1;i<4;i++)
								{
									if(USART_RX_BUFC[ip_start+i] == '.')
									{
										ip_end = i;
										break;
									}
								}
								Modbus.subnet[1] = 0;

								if(ip_end==1)
									Modbus.subnet[1] += (USART_RX_BUFC[ip_start+ip_end-1]-0x30);
								else if(ip_end == 2)
								{
									Modbus.subnet[1] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.subnet[1] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}
								else if(ip_end == 3)
								{
									Modbus.subnet[1] += (USART_RX_BUFC[ip_start+ip_end-3]-0x30) * 100;
									Modbus.subnet[1] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.subnet[1] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}
								
								ip_start = ip_start + ip_end + 1;
								for(i=1;i<4;i++)
								{
									if(USART_RX_BUFC[ip_start+i] == '.')
									{
										ip_end = i;
										break;
									}
								}
								Modbus.subnet[2] = 0;
								if(ip_end==1)
									Modbus.subnet[2] += (USART_RX_BUFC[ip_start+ip_end-1]-0x30);
								else if(ip_end == 2)
								{
									Modbus.subnet[2] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.subnet[2] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}
								else if(ip_end == 3)
								{
									Modbus.subnet[2] += (USART_RX_BUFC[ip_start+ip_end-3]-0x30) * 100;
									Modbus.subnet[2] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.subnet[2] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}
								ip_start = ip_start + ip_end + 1;
								for(i=1;i<4;i++)
								{
									if(USART_RX_BUFC[ip_start+i] == ',')
									{
										ip_end = i;
										break;
									}
								}
								Modbus.subnet[3] = 0;
								if(ip_end==1)
									Modbus.subnet[3] += (USART_RX_BUFC[ip_start+ip_end-1]-0x30);
								else if(ip_end == 2)
								{
									Modbus.subnet[3] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.subnet[3] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}
								else if(ip_end == 3)
								{
									Modbus.subnet[3] += (USART_RX_BUFC[ip_start+ip_end-3]-0x30) * 100;
									Modbus.subnet[3] += (USART_RX_BUFC[ip_start+ip_end-2]-0x30) * 10;
									Modbus.subnet[3] += USART_RX_BUFC[ip_start+ip_end-1]-0x30;
								}									
								rev_cnt = 0;
								wifi_cmd_num = CMD_READ_MAC;
								ip_read_flag = 2;
							}
							else
								rev_cnt = 0;
							
				    }
					}
				}
				
				
			
			  else if(wifi_cmd_num == CMD_READ_MAC)
				{
					USART_RX_BUFC[rev_cnt++] = USART_ReceiveData(UART4);
					if(rev_cnt >= 26)
					{
						if((USART_RX_BUFC[8] == '+') && (USART_RX_BUFC[9] == 'O') && (USART_RX_BUFC[10] == 'K'))
						{
							for(i=0;i<16;i++)
							{
								if(USART_RX_BUFC[12+i] > 0x39)//a,b,c....
									USART_RX_BUFC[12+i] = USART_RX_BUFC[12+i] - 0x57;
								else
									USART_RX_BUFC[12+i] = USART_RX_BUFC[12+i] - 0x30;
							}
						for(i=0;i<6;i++)
							Modbus.mac_addr[i] = (USART_RX_BUFC[12+i*2]<<4) + USART_RX_BUFC[12+i*2+1];   
						}
						ip_read_flag = 3;
						wifi_cmd_num = CMD_BACK_TO_NOMRAL;
						rev_cnt = 0;	
					}
					
				}			
			
			}
		  else
			{
				//if(Modbus.com_config[0] == 3/*MODBUS*/)	
				{	
					if(rece_countB < 512 - 1)
					{
						USART_RX_BUFC[rece_countB++] = USART_ReceiveData(UART4);//(USART1->DR);	
					}							
					else
					{
						bip_flag = 0;	
						serial_restartB();
					}
					if(rece_countB == 1)
					{
						// This starts a timer that will reset communication.  If you do not
						// receive the full packet, it insures that the next receive will be fresh.
						// The timeout is roughly 7.5ms.  (3 ticks of the hearbeat)
						rece_sizeB = 50;
						serial_receive_timeout_countB = 10;//SERIAL_RECEIVE_TIMEOUT;		
						if(USART_RX_BUFC[0] == 0x64)
							checkonline_flag = 1;
						if(USART_RX_BUFC[0] == 0x81)
							bip_flag = 1;
					}
					else if((checkonline_flag == 1) && (rece_countB >= 4) &&(USART_RX_BUFC[rece_countB] == 0x00) && (USART_RX_BUFC[rece_countB-1] == 0x00) && (USART_RX_BUFC[rece_countB-2] == 0x00)&& (USART_RX_BUFC[rece_countB-3] == 0x00))//CHECKONLINE network scan command
					{ // temco private scan command
							checkonline_rcv = 1;
							dealwithTagB = 2;							
					}
					else if((bip_flag == 1) && (rece_countB >= 5) && ((USART_RX_BUFC[1] == 0x0a) || (USART_RX_BUFC[1] == 0x0b))
						&& (USART_RX_BUFC[4] == 0x01))
					{ // bip packet
							bip_rcv = 1;
							dealwithTagB = 2;							
					}
					else	if(rece_countB == 4 + TCP_HEADER)
					{
							//check if it is a scan command
							if((((vu16)(USART_RX_BUFC[2+TCP_HEADER] << 8) + USART_RX_BUFC[3+TCP_HEADER]) == 0x0a) && (USART_RX_BUFC[1+TCP_HEADER] == WRITE_VARIABLES))
							{
								rece_sizeB = DATABUFLEN_SCAN;
								serial_receive_timeout_countB = 10;//SERIAL_RECEIVE_TIMEOUT;	
							}
					}
					else if(rece_countB == 5 + TCP_HEADER)
					{
						if((USART_RX_BUFC[1+TCP_HEADER] == READ_VARIABLES) || (USART_RX_BUFC[1+TCP_HEADER] == WRITE_VARIABLES))
						{
							if(((vu16)(USART_RX_BUFC[2+TCP_HEADER] << 8) + USART_RX_BUFC[3+TCP_HEADER]) != 0x0a)
							{
							rece_sizeB = 6 + TCP_HEADER;
							serial_receive_timeout_countB = 10;//SERIAL_RECEIVE_TIMEOUT;
							}
						}
						else if(USART_RX_BUFC[1 + TCP_HEADER] == MULTIPLE_WRITE)
						{							
							rece_sizeB = USART_RX_BUFC[4 + TCP_HEADER] + 9;
							serial_receive_timeout_countB = USART_RX_BUFC[6 + TCP_HEADER] + 8;
						}
					}
					else if((rece_countB == rece_sizeB) && (USART_RX_BUFC[1 + TCP_HEADER] == READ_VARIABLES || USART_RX_BUFC[1 + TCP_HEADER] == WRITE_VARIABLES || USART_RX_BUFC[1 + TCP_HEADER] == MULTIPLE_WRITE))
					{	
						serial_receive_timeout_countB = 0;
						modbustcp_rcv = 1;
						dealwithTagB = 2;		// making this number big to increase delay
					}
				}
			}
		}

	else  if( USART_GetITStatus(UART4, USART_IT_TXE) == SET ) //Tansmit Data Register empty interrupt = transmit has finished
	{		
			//if(Modbus.com_config[0] == 3/*MODBUS*/ )/*||(modbus.com_config[0] == BAC_MSTP)*/
				{
//					#ifndef TSTAT7_ARM							
//					icon.cmnct_send = 1;	
//					#endif
				if( send_countB >= sendbyte_numB)
				{
					while(USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET);
					USART_ClearFlag(UART4, USART_FLAG_TC);
					
					USART_ITConfig(UART4, USART_IT_TXE, DISABLE);
					send_countB = 0;
					serial_restartB();			
				}
				else
				{
					USART_SendData(UART4, uart_sendB[send_countB++]);						
				}
			}					
	}		
		
		
}





void WIFI_task(void) reentrant//one second base software timer
{	
	//uint8 zgb_str =0;
	static uint8 wifi_cnt = 0;
	static uint8 read_ip_ok = 0;
	uint8 i;
  uint8 wifi_cmd_temp[10];
	uint8 onemin_cnt = 0;
  uint8 onehour_cnt = 0;
	for(;;)
	{	
		IWDG_ReloadCounter();

		if(1)//(wifi_state == WIFI_CONNECTED) // if wifi link state is true, switch to wifi module to command mode and read the IP back
		{
			if(ip_read_flag == 0)//first switch to command mode
			{
				wifi_cmd_num = CMD_SWITCH_MODE;
				wifi_cmd_temp[0] = '+';
				wifi_cmd_temp[1] = '+';
				wifi_cmd_temp[2] = '+';
				uart_send_string(wifi_cmd_temp,3,3);
        delay_ms(1000);
				wifi_cmd_temp[0] = 'a';
				uart_send_string(wifi_cmd_temp,1,3);
				delay_ms(500);	
				wifi_cnt++;
				if(wifi_cnt >=3)
				{
					wifi_cnt = 0;
					GPIO_ResetBits(GPIOD, GPIO_Pin_2);
					delay_ms(200);
					GPIO_SetBits(GPIOD, GPIO_Pin_2);
					delay_ms(1000);
					//ip_read_flag = 2;
				}
        				
			}
			else if(ip_read_flag == 1)//((wifi_cmd_num == CMD_SWITCH_MODE)||(wifi_cmd_num == CMD_READ_STA_INFO)) //send read IP command
			{
				if(update_flag == 20)
				{
					for(i = 0;i < 7;i++)
						wifi_cmd_temp[i] = wifi_cmd[CMD_FAC_RESET][i];
					wifi_cmd_num = NORMAL_MODE;
					//memcpy(uart_sendB, wifi_cmd_temp, 7);

					wifi_cmd_temp[7] = 0x0d;
					wifi_cmd_temp[8] = 0x0a;
//					USART_SendDataStringB(9);	
					uart_send_string(wifi_cmd_temp,9,3);
					
					update_flag = 0;
					ip_read_flag = 0;
				}
				else
				{
					read_ip_ok = 1;
					for(i=0;i<7;i++)
						wifi_cmd_temp[i] = wifi_cmd[CMD_READ_STA_INFO][i];
					wifi_cmd_num = CMD_READ_STA_INFO;
					//memcpy(uart_sendB, wifi_cmd_temp, 7);

					wifi_cmd_temp[7] = 0x0d;
					wifi_cmd_temp[8] = 0x0a;
					//USART_SendDataStringB(9);
					uart_send_string(wifi_cmd_temp,9,3);
				}
			}
			else if(ip_read_flag == 2)
			{
				for(i=0;i<7;i++)
					wifi_cmd_temp[i] = wifi_cmd[CMD_READ_MAC][i];
				wifi_cmd_num = CMD_READ_MAC;
				//memcpy(uart_sendB, wifi_cmd_temp, 6);

				wifi_cmd_temp[6] = 0x0d;
				wifi_cmd_temp[7] = 0x0a;
				//USART_SendDataStringB(8);	
				uart_send_string(wifi_cmd_temp,8,3);
				printf("wifi ip: %u %u %u %u \r\n",Modbus.ip_addr[0],
				Modbus.ip_addr[1],
				Modbus.ip_addr[2],
				Modbus.ip_addr[3]);
			}
			else if(ip_read_flag == 3)//(wifi_cmd_num == CMD_BACK_TO_NOMRAL)
			{
				wifi_cmd_num = NORMAL_MODE;
				for(i=0;i<7;i++)
					wifi_cmd_temp[i] = wifi_cmd[CMD_BACK_TO_NOMRAL][i];
				//memcpy(uart_sendB, wifi_cmd_temp, 7);
				wifi_cmd_temp[7] = 0x0d;
				wifi_cmd_temp[8] = 0x0a;
				uart_send_string(wifi_cmd_temp,9,3);
				
				if(read_ip_ok == 1)
					ip_read_flag = 4;
				else
					ip_read_flag = 0;

				printf("wifi initial ok \r\n");
			}
		
		}
			
			//+OK=DHCP,192.168.0.21,255.255.255.0,192.168.0.4,192.168.0.4
//		if(update_flag == 13)
//		{
//			flash_buf[0] = SerialNumber(0);
//			flash_buf[1] = SerialNumber(1);		
//			STMFLASH_Write(FLASH_SERIAL_NUM_LO, flash_buf, 2);
//		}	

//		if(update_flag == 14)
//		{
//			flash_buf[0] = SerialNumber(2);
//			flash_buf[1] = SerialNumber(3);		
//			STMFLASH_Write(FLASH_SERIAL_NUM_HI, flash_buf, 2);
//		}	
		
						
		delay_ms(1000);
	  }		
}
void main_init_crc16(void);

//uint8_t far PDUBuffer_BIP[MAX_APDU];
void dealwithDataB(void)
{
	u16 address;
	BACNET_ADDRESS far src; /* source address */
//	uint8_t * bip_Data;
//	uint16_t  bip_len;
	uint16_t pdu_len = 0;  
	
	if(bip_rcv == 1)
	{ // bip command
				
//		bip_Data = PDUBuffer_BIP;
//		memcpy(bip_Data,uip_appdata,uip_len);
//		bip_len = uip_len;
//		uip_ipaddr_copy(addr, uip_udp_conn->ripaddr);
//		bip_server_conn = uip_udp_new(&addr, uip_udp_conn->rport);
		
		pdu_len = datalink_receive(&src, USART_RX_BUFC/*&PDUBuffer_BIP[0]*/, rece_sizeB/*sizeof(PDUBuffer_BIP)*/, 0,BAC_IP);
	  {
			if(pdu_len) 
			{
				Test[17]++;
				Test[18] = pdu_len;
				npdu_handler(&src,USART_RX_BUFC/* &PDUBuffer_BIP[0]*/, pdu_len,BAC_IP);	
			}			
		}
		bip_flag = 0;
		bip_rcv = 0;
		serial_restartB();
	}
	else if(checkonline_rcv == 1)
	{ // TEMCO private scan command 64 00 00 00 00 
		printf("checkonline_rcv \r\n");
// response scan command	

		Scan_Infor.cmd = 0x0065;
		Scan_Infor.len = 0x001d;
	
		//serialnumber 4 bytes
		Scan_Infor.own_sn[0] = (U16_T)Modbus.serialNum[0];
		Scan_Infor.own_sn[1] = (U16_T)Modbus.serialNum[1];
		Scan_Infor.own_sn[2] = (U16_T)Modbus.serialNum[2];
		Scan_Infor.own_sn[3] = (U16_T)Modbus.serialNum[3];
	
	
		Scan_Infor.product = (U16_T)PRODUCT_MINI_ARM;

		//modbus address
		Scan_Infor.address = (U16_T)Modbus.address;
	
		//Ip
		Scan_Infor.ipaddr[0] = (U16_T)Modbus.ip_addr[0];
		Scan_Infor.ipaddr[1] = (U16_T)Modbus.ip_addr[1];
		Scan_Infor.ipaddr[2] = (U16_T)Modbus.ip_addr[2];
		Scan_Infor.ipaddr[3] = (U16_T)Modbus.ip_addr[3];
	
		//port
		Scan_Infor.modbus_port = swap_word(Modbus.tcp_port);

		// software rev
		Scan_Infor.firmwarerev = swap_word(SW_REV / 10 + SW_REV % 10);
		// hardware rev
		Scan_Infor.hardwarerev = swap_word(Modbus.hardRev);
		
		Scan_Infor.instance_low = HTONS(Instance); // hight byte first
		Scan_Infor.panel_number = panel_number; //  36	
		Scan_Infor.instance_hi = HTONS(Instance >> 16); // hight byte first
		
		Scan_Infor.bootloader = 0;  // 0 - app, 1 - bootloader, 2 - wrong bootloader
		Scan_Infor.BAC_port = Modbus.Bip_port;//((Modbus.Bip_port & 0x00ff) << 8) + (Modbus.Bip_port >> 8);  // 
		Scan_Infor.zigbee_exist = zigbee_exist; // 0 - inexsit, 1 - exist
		Scan_Infor.subnet_protocal = 0;
		
		uart_send_string((unsigned char *)(&Scan_Infor),sizeof(STR_SCAN_CMD),3);
		checkonline_flag = 0;
		checkonline_rcv = 0;
		serial_restartB();
	}
  else if(modbustcp_rcv == 1)
	{	
		Modbus.main_port = 3;
		memcpy(main_data_buffer,USART_RX_BUFC,MAX_BUF_LEN);
		main_dealwithTag = dealwithTagB;
		main_rece_size = rece_sizeB;

		main_init_crc16(); 	
		responseCmd(WIFI,main_data_buffer);		
		modbustcp_rcv = 0;
	}	
}

void vCOMMTask(void *pvParameters )
{
	static uint16 i = 0;

	for( ;; )
	{

		if (dealwithTagB)
		{ 
		 dealwithTagB--;
		  if(dealwithTagB == 1)//&& !Serial_Master )
			{				
				dealwithDataB();		
			}
		}		
		
		if(serial_receive_timeout_countB > 0)  
		{
				serial_receive_timeout_countB-- ; 
				if(serial_receive_timeout_countB == 0)
				{
					serial_restartB();
					//printf("serial restart\r\n");
				}
		}
		delay_ms(2) ;
	}
	
}


void vStartWifiTasks( U8_T uxPriority)
{
	modbus_initB();
	uart4_init(115200);
	sTaskCreate( WIFI_task, "WIFI_task", WifiSTACK_SIZE, NULL, uxPriority, &Wifi_Handler );
	sTaskCreate( vCOMMTask, "vCOMMTask", WifiSTACK_SIZE, NULL, uxPriority, &Wifi_dealwith_Handler );
}


#endif