#include "product.h"
#include "main.h"


#if REM_CONNECTION

RM_CONN  RM_Conns;
static U8_T  RM_InterAppId;

RM_HEART_CONN RM_Heart_Conns;
static U8_T  RM_Heart_InterAppId;

RM_REC_CONN RM_Rec_Conns;
static U8_T  RM_Rec_InterAppId;


U8_T  Recount_Check_serverip;

U8_T RM_Send_Status;



void RM_send_information(void);

void RM_Init(void)
{
	RM_Conns.State = RM_STATE_NONE;
	RM_Conns.ServerIp = 0;
	RM_InterAppId = TCPIP_Bind(NULL, RM_Event, RM_Receive);
//    RM_Buf = pBuf;
//    RM_Subject = pSubject;
	Recount_Check_serverip = 0;
	RM_Conns.connecting_count = 0;
	RM_Conns.connected_count = 0;
//	RM_Send_Status = RM_NO_CONNECTION;
	
	
	RM_Heart_Conns.State = RM_HEART_STATE_INITIAL;
	RM_Heart_InterAppId = TCPIP_Bind(NULL, RM_Heart_Event, RM_Heart_Receive);	
	
	if ((RM_Heart_Conns.UdpSocket = TCPIP_UdpNew(RM_Heart_InterAppId, 0, RM_Conns.ServerIp/*0xc0a80063*/,
		0, RM_HAERT_PORT)) == TCPIP_NO_NEW_CONN)
	{
		
	}
	
	RM_Rec_Conns.State = RM_REC_STATE_INITIAL;
	RM_Rec_InterAppId = TCPIP_Bind(NULL, RM_Rec_Event, RM_Rec_Receive);	
	
	if ((RM_Rec_Conns.UdpSocket = TCPIP_UdpNew(RM_Rec_InterAppId, 0, RM_Conns.ServerIp/*0xc0a80063*/,
		0, RM_REC_PORT)) == TCPIP_NO_NEW_CONN)
	{
		
	}
	
	
} /* End of SMTPC_Init() */


void RM_Start(void/*U32_T ip , U8_T* from, U8_T* to1, U8_T* to2, U8_T* to3*/)
{
	if(RM_Conns.State == RM_STATE_CONNECTED) 
	{
//		Test[10]++;
		if(RM_Conns.connecting_count % 5 == 0)
		{
//			Test[11]++;
			RM_send_information();	
		}
		if(RM_Conns.connecting_count > MAX_RM_RETRY_CONNECTING_COUNT) 
		{
//			Test[12]++;
			RM_Conns.connecting_count = 0;
			RM_Conns.State = RM_STATE_NONE;
		}
		else
			RM_Conns.connecting_count++;

		RM_Conns.connected_count = 0;	
	}	
	else if(RM_Conns.State == RM_STATE_RECEIVE)
	{
		RM_Conns.connecting_count = 0;
//		Test[13]++;
		if(RM_Conns.connected_count > MAX_RM_RETRY_CONNECTED_COUNT) 
		{
			RM_Conns.connected_count = 0;
			RM_Conns.State = RM_STATE_CONNECTED;
		}
		else
			RM_Conns.connected_count++;	
	}
	
	if (RM_Conns.State != RM_STATE_NONE)
	{
		return;
	}
	if(Recount_Check_serverip > 5) return;	

	/* Create RM client port */
	DNSC_Query("newfirmware.com",&RM_Conns.ServerIp);
	
	if((RM_Conns.TcpSocket = TCPIP_TcpNew(RM_InterAppId, 0,RM_Conns.ServerIp,
		0, RM_SERVER_PORT)) == TCPIP_NO_NEW_CONN)
	{  // successfully
		
		return;
	}
	else
	{ // cant connect old ip, try update remote_server_ip
//		Test[14]++;
		// check whether ip of remote server is changed or not
		DNSC_flag = REMOTE_SERVER;
		//DNSC_Start("chelseatest.f3322.net");
		DNSC_Start("newfirmware.com");
//		Recount_Check_serverip++;  only for test now
	}


	RM_Conns.State = RM_STATE_INITIAL;	
	TCPIP_TcpConnect(RM_Conns.TcpSocket);

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
	RM_info.panel = Station_NUM;
	RM_info.modbus_port = Setting_Info.reg.tcp_port;
	RM_info.bacnet_port = swap_word(Modbus.Bip_port);
	
	sprintf(RM_info.username,"%lu",swap_double(Setting_Info.reg.sn));
	sprintf(RM_info.password,"travel123");
	TCPIP_TcpSend(RM_Conns.TcpSocket, &RM_info, sizeof(STR_RM), TCPIP_SEND_NOT_FINAL);
}

void RM_Event(U8_T id, U8_T event)
{
	if(id != 0)
		return;
  
	
	if(event == TCPIP_CONNECT_CANCEL)
	{
		RM_Conns.State = event;
	}
	if(event == TCPIP_CONNECT_WAIT)
	{
		RM_Conns.State = event;
	}
	else if(event == TCPIP_CONNECT_ACTIVE)
	{
		RM_Conns.State = event;
	}

	
	
} /* End of RM_Event() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: RM_Receive
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void RM_Receive(U8_T XDATA* pbuf, U16_T length, U8_T id)
{
	U16_T		codes, len;
	U8_T		*point, *pData;

	if(id != 0)
		return;

	RM_Conns.State = RM_STATE_RECEIVE;	
	
}



void RM_Heart_Event(U8_T id, U8_T event)
{
	if(id != 0)
		return;  
	
	if(event == TCPIP_CONNECT_CANCEL)
	{
		RM_Heart_Conns.State = event;
	}
	if(event == TCPIP_CONNECT_WAIT)
	{
		RM_Heart_Conns.State = event;
	}
	else if(event == TCPIP_CONNECT_ACTIVE)
	{
		RM_Heart_Conns.State = event;
	}	
	
} /* End of RM_Event() */

void RM_Rec_Event(U8_T id, U8_T event)
{
	if(id != 0)
		return;  
	
	if(event == TCPIP_CONNECT_CANCEL)
	{
		RM_Rec_Conns.State = event;
	}
	if(event == TCPIP_CONNECT_WAIT)
	{
		RM_Rec_Conns.State = event;
	}
	else if(event == TCPIP_CONNECT_ACTIVE)
	{
		RM_Rec_Conns.State = event;
	}	
	
} /* End of RM_Event() */

void RM_Heart_Receive(U8_T XDATA* pbuf, U16_T length, U8_T id)
{
	U16_T		codes, len;
	U8_T		*point, *pData;

	if(id != 0)
		return;

	RM_Heart_Conns.State = RM_STATE_RECEIVE;	
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

void RM_Rec_Receive(U8_T XDATA* pbuf, U16_T length, U8_T id)
{
	U16_T		codes, len;
	U8_T		*point, *pData;
	if(id != 0)
		return;

	RM_Rec_Conns.State = RM_STATE_RECEIVE;	
//	Test[31]++;

	if(pbuf[0] == 0x55 && pbuf[1] == 0xff)
	{
		switch(pbuf[2])
		{
			case COMMAND_RECEIVE_SERIAL:
				break;
			//case COMMAND_DEVICE_SEND_SERIAL_TO_SERVER:
			case COMMAND_REPLY_T3000_INFO:
//#if (DEBUG_UART1)
//	uart_init_send_com(UART_SUB1);	// for test		
//	sprintf(debug_str," \r\n\ rec ip %u %x %x %x %x %x %x %x %x %x",(uint16)length,(uint16)pbuf[0],(uint16)pbuf[1],(uint16)pbuf[2],
//	(uint16)pbuf[3],(uint16)pbuf[4],(uint16)pbuf[5],(uint16)pbuf[6],(uint16)pbuf[7]);
//	uart_send_string(debug_str,strlen(debug_str),UART_SUB1);
//#endif 	
			
				// response -- 55 ff + cmd
				break;
		}
//		Test[32]++;
	}
}

void RM_Respons_Server(U32_T ip,U16_T port)
{
	RM_Rec_Conns.State = RM_REC_STATE_INITIAL;
	RM_Rec_InterAppId = TCPIP_Bind(NULL, RM_Rec_Event, RM_Rec_Receive);	
	
	if ((RM_Rec_Conns.UdpSocket = TCPIP_UdpNew(RM_Rec_InterAppId, 0, RM_Conns.ServerIp/*0xc0a80063*/,
		0, RM_REC_PORT)) == TCPIP_NO_NEW_CONN)
	{
		
	}
}


U8_T RM_Send_Heart(void)
{
	U8_T far Buf[HEARTBEAT_LENGTH + 3];
	Str_MSG str_heart_info;

//	RM_Heart_Conns.ServerIp = 0xc0a800f3;
//	if((RM_Heart_Conns.UdpSocket = TCPIP_UdpNew(RM_Heart_InterAppId, 0, RM_Heart_Conns.ServerIp,
//		0, RM_HAERT_PORT)) == TCPIP_NO_NEW_CONN)
//	{
//		Test[34]++;
//		return RM_HEART_STATE_NOTREADY;
//	}
//	Test[33]++;
	Buf[0] = 0x55;	
	Buf[1] = 0XFF;  
	Buf[2] = 0X07; 
	str_heart_info.reg_data.communication_version = 1;
	str_heart_info.reg_data.ideviceType = 0;
	str_heart_info.reg_data.m_serial_number = Setting_Info.reg.sn;
	str_heart_info.reg_data.m_product_type = PRODUCT_MINI_BIG;
	str_heart_info.reg_data.m_object_instance = Instance;
	str_heart_info.reg_data.m_panel_number = Station_NUM;
	str_heart_info.reg_data.modbus_port = Setting_Info.reg.tcp_port;
	str_heart_info.reg_data.soft_version = Panel_Info.reg.sw;
	str_heart_info.reg_data.last_connected_time = 0;
	
	memset(str_heart_info.reg_data.userName,0,30);
	str_heart_info.reg_data.userName[0] = 0x31;
	
	memset(str_heart_info.reg_data.password,0,20);
	str_heart_info.reg_data.password[0] = 0x31;

	memcpy(&Buf[3],str_heart_info.all_data,HEARTBEAT_LENGTH);

	
	TCPIP_UdpSend(RM_Heart_Conns.UdpSocket, 0, 0, Buf, HEARTBEAT_LENGTH + 3);

	return 1;	
}


U8_T RM_Send_SN(void)
{
	U8_T far Buf[MINI_UDP_DATA_LENGTH];
	Str_Device str_sn_info;
	
	//RM_Rec_Conns.ServerIp = 0xc0a80066;
//	if((RM_Rec_Conns.UdpSocket = TCPIP_UdpNew(RM_Rec_InterAppId, 0, RM_Rec_Conns.ServerIp,
//		0, RM_REC_PORT)) == TCPIP_NO_NEW_CONN)
//	{
//		return RM_REC_STATE_NOTREADY;
//	}
	Buf[0] = 0x55;	
	Buf[1] = 0XFF;  
	Buf[2] = 0X06; 
	str_sn_info.reg_data.m_serial_number = Setting_Info.reg.sn;
	
	memcpy(&Buf[3],str_sn_info.all_data, MINI_UDP_LENGTH);

	TCPIP_UdpSend(RM_Rec_Conns.UdpSocket, 0, 0, Buf,MINI_UDP_DATA_LENGTH);

	return 1;	
}

#endif
