#include "product.h"
#include "main.h"


struct uip_udp_conn * RM_Heart_conn;
struct uip_udp_conn * RM_Rec_conn;
U16_T RM_T3000_PORT;
U8_T RM_IP[4];
void RM_Init(void)
{
	uip_ipaddr_t addr;
	
//	Test[3] = RM_IP[0];
//	Test[4] = RM_IP[1];
//	Test[5] = RM_IP[2];
//	Test[6] = RM_IP[3];
	if(RM_Heart_conn != NULL) 
	{
		uip_udp_remove(RM_Heart_conn);
	}
	
	uip_ipaddr(addr, RM_IP[0], RM_IP[1], RM_IP[2], RM_IP[3]);	
	RM_Heart_conn = uip_udp_new(&addr, HTONS(SERVER_HEARTBEAT_PORT));
	if(RM_Heart_conn != NULL) {
    uip_udp_bind(RM_Heart_conn, HTONS(SERVER_HEARTBEAT_PORT + 1000));
  }
//	RM_Send_Heart();
	
	if(RM_Rec_conn != NULL) 
	{
		uip_udp_remove(RM_Rec_conn);
	}
	uip_ipaddr(addr, RM_IP[0], RM_IP[1], RM_IP[2], RM_IP[3]);	
	RM_Rec_conn = uip_udp_new(&addr, HTONS(SERVER_MINI_PORT));
	if(RM_Rec_conn != NULL) {
    uip_udp_bind(RM_Rec_conn, HTONS(SERVER_MINI_PORT + 1000));
  }
//	RM_Send_SN();
//	RM_Heart_conn = uip_udp_new((uip_ipaddr_t *)0xc0a80063, HTONS(SERVER_HEARTBEAT_PORT));	
//	RM_Rec_conn = uip_udp_new((uip_ipaddr_t *)0xc0a80063, HTONS(SERVER_MINI_PORT));	
} /* End of SMTPC_Init() */


void RM_Start(void/*U32_T ip , U8_T* from, U8_T* to1, U8_T* to2, U8_T* to3*/)
{
	uint16_t * ptr_rm_ip;
	static U8_T RM_IP_B[4];
	static U8_T count = 0;

	for(;count < 10;count++)
	{
		ptr_rm_ip = resolv_lookup("newfirmware.com");	
		memcpy(RM_IP,ptr_rm_ip,4);
		
		if(memcmp(RM_IP,RM_IP_B,4))
		{ // ip is changed
			//memcpy(RM_IP_B,RM_IP,4);
//			RM_Init();
		}	
	}
} 


void RM_Heart_appcall(void)
{
	static u32 t1,t2;
	
  uint8_t pbuf[50];
	uint8_t sbuf[50];
	struct uip_udp_conn *conn;
	uip_ipaddr_t addr;
	if(uip_poll()) 
	{	
		// auto send
		t1 = uip_timer;
		if(t1 - t2 >= 10000)
		{
			RM_Send_Heart();
			t2 = uip_timer;
		}
	}
	
	if(uip_newdata()) 
	{
// deal with receiving data
		if(uip_len < 50)
		memcpy(pbuf,uip_appdata,uip_len);
		if(pbuf[0] == 0x55 && pbuf[1] == 0xff)
		{
			switch(pbuf[2])
			{
				case COMMAND_RECEIVE_HEART_BEAT:
					break;
				case COMMAND_COMMUNICATION_VERSION_ERROR:
					break;
				case COMMAND_COMMAND_UNKNOWN:
				default:
					break;
			}
		}			
	}

}


//void RM_Rec_t3000_makehole_appcall(void)
//{
//	static u32 t1,t2;
//	uint8_t pbuf[50];
//	struct uip_udp_conn *conn;
//	uip_ipaddr_t addr;
//	
//	if(uip_poll()) 
//	{
//		// auto send
//		t1 = uip_timer;
//		if(t1 - t2 >= 10000)
//		{
//			RM_Send_T3000_MAKEHOLE();
//			Test[37]++;
//			t2 = uip_timer;
//		}
//	}	
//}



void RM_Rec_appcall(void)
{
	uint8_t pbuf[50];
	struct uip_udp_conn *conn;
	uip_ipaddr_t addr;
	static u32 t1 = 0;
	static u32 t2 = 0;

	
	if(uip_poll()) 
	{
		// auto send
		
		t1 = uip_timer;
		if(t1 - t2 >= 10000 || t2 == 0)
		{			
			RM_Send_SN();
			t2 = uip_timer;
		}
	}
	
	if(uip_newdata()) 
	{
		
// deal with receiving data
		if(uip_len < 50)
		memcpy(pbuf,uip_appdata,uip_len);
		if(pbuf[0] == 0x55 && pbuf[1] == 0xff)
		{
			switch(pbuf[2])
			{
				case COMMAND_RECEIVE_SERIAL: 
				break;
				case COMMAND_REPLY_T3000_INFO:
				// receive IP and port of REMOTE_T3000 from server
				
				RM_T3000_PORT = pbuf[7] * 256 + pbuf[8];
				Test[20]++;
				Test[22] = RM_T3000_PORT;
	
				// change ip and port
				//memcpy(conn , RM_Rec_conn , sizeof(struct uip_udp_conn));
				uip_ipaddr_copy(RM_Rec_conn->ripaddr, &pbuf[3]);
				RM_Rec_conn->rport = RM_T3000_PORT;
				RM_Send_T3000_MAKEHOLE();
				
//				uip_ipaddr_copy(RM_Rec_conn->ripaddr, 0xc0a80063);
//				RM_Rec_conn->rport = SERVER_MINI_PORT;
			
				break;
				case COMMAND_T3000_SEND_TO_DEVICE_MAKEHOLE:
					Test[19]++;
					break;
			}
		}			
	}

}

typedef struct
{
	U8_T head1;
	U8_T head2;
	U8_T cmd;
	U32_T sn;
	U8_T product_id;
	U32_T object_instance;
	U8_T panel;
	U16_T modbus_port;
  U16_T bacnet_port;
	
	char username[20];
	char password[10];

}STR_RM;

STR_RM RM_info;


void RM_send_information(void)
{
	RM_info.head1 = 0xff;
	RM_info.head2 = 0x55;
	RM_info.cmd = 1;
	RM_info.sn = Setting_Info.reg.sn;
	RM_info.product_id = Panel_Info.reg.product_type;
	RM_info.object_instance = Setting_Info.reg.instance;
	RM_info.panel = panel_number;
	RM_info.modbus_port = Setting_Info.reg.tcp_port;
	RM_info.bacnet_port = swap_word(Modbus.Bip_port);
	
	sprintf(RM_info.username,"%lu",swap_double(Setting_Info.reg.sn));
	sprintf(RM_info.password,"travel123");
//	TCPIP_TcpSend(RM_Conns.TcpSocket, &RM_info, sizeof(STR_RM), TCPIP_SEND_NOT_FINAL);
}



U8_T RM_Send_Heart(void)
{
	
	U8_T far Buf[HEARTBEAT_LENGTH + 3];
	Str_MSG str_heart_info;
	
	Buf[0] = 0x55;	
	Buf[1] = 0XFF;  
	Buf[2] = 0X07; 
	str_heart_info.reg_data.communication_version = 1;
	str_heart_info.reg_data.ideviceType = 0;
	str_heart_info.reg_data.m_serial_number = Setting_Info.reg.sn;
	str_heart_info.reg_data.m_product_type = PRODUCT_MINI_BIG;
	str_heart_info.reg_data.m_object_instance = Instance;
	str_heart_info.reg_data.m_panel_number = panel_number;
	str_heart_info.reg_data.modbus_port = Setting_Info.reg.tcp_port;
	str_heart_info.reg_data.soft_version = Panel_Info.reg.sw;
	str_heart_info.reg_data.last_connected_time = 0;
	
	memset(str_heart_info.reg_data.userName,0,30);
	str_heart_info.reg_data.userName[0] = 0x31;
	str_heart_info.reg_data.userName[1] = 0;
	
	memset(str_heart_info.reg_data.password,0,20);
	str_heart_info.reg_data.password[0] = 0x31;
	str_heart_info.reg_data.password[1] = 0;

	memcpy(&Buf[3],str_heart_info.all_data,HEARTBEAT_LENGTH);

	uip_send(Buf,HEARTBEAT_LENGTH + 3);

	return 1;	
}


U8_T RM_Send_SN(void)
{
	U8_T Buf[MINI_UDP_DATA_LENGTH];
	Str_Device str_sn_info;
	
	Buf[0] = 0x55;	
	Buf[1] = 0XFF;  
	Buf[2] = 0X06; 
	str_sn_info.reg_data.m_serial_number = Modbus.serialNum[0] + (U16_T)(Modbus.serialNum[1] << 8)
	 + ((U32_T)Modbus.serialNum[2] << 16) + ((U32_T)Modbus.serialNum[3] << 24);
	
	memcpy(&Buf[3],str_sn_info.all_data, MINI_UDP_LENGTH);
	
	uip_send(Buf,MINI_UDP_DATA_LENGTH);
	return 1;	
}


U8_T RM_Send_T3000_MAKEHOLE(void)
{
	U8_T Buf[3];
	
	Buf[0] = 0x55;	
	Buf[1] = 0XFF;  
	Buf[2] = COMMAND_DEVICE_SEND_TO_T3000_MAKEHOLE; 
	
	uip_send(Buf,3);
	return 1;	
}


