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

/*=============================================================================
 * Module Name: dyndns.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log:
 *=============================================================================
 */

/* INCLUDE FILE DECLARATIONS */

#include "main.h"




#include "uip.h"
#include "tcpip.h"
#include "dyndns.h"
#include "dyndns_app.h"
//#include "mstimer.h"
#include <string.h>
#include <stdio.h>

#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
#include "uip.h"
#endif

void tcp_client_reconnect(void);
void tcp_client_connected(void);
void tcp_client_aborted(void);
void tcp_client_timedout(void);
void tcp_client_closed(void);
void tcp_client_acked(void);
void tcp_client_senddata(void);


extern U16_T far Test[50];

#define CONF_DDNS_3322		0
#define CONF_DDNS_DYNDNS	1
#define CONF_DDNS_NOIP		2

/* NAMING CONSTANT DECLARATIONS */
#define MAX_CMDBUF_SIZE				256
#define MAX_TMP_ENCBUF_SIZE			48
#define MAX_ENCBUF_SIZE				64
#define MAX_USERNAME_SIZE			32
#define MAX_PASSWORD_SIZE			32
#define MAX_DOMAIN_SIZE				32
#define MAX_TIMEOUT_VALUE			500//50



/* TYPE DECLARATIONS */

/* MACRO DECLARATIONS */

/* STATIC VARIABLE DECLARATIONS */

/* LOCAL VARIABLE DECLARATIONS */

U8_T far dyndns_User[MAX_USERNAME_SIZE];// = {0};
U8_T far dyndns_Pass[MAX_PASSWORD_SIZE];// = {0};
U8_T far dyndns_Domain[MAX_DOMAIN_SIZE];// = {0};

S8_T far dyndns_Command[MAX_CMDBUF_SIZE];
U16_T far dyndns_CmdLen;
S8_T far dyndns_TmpEncData[MAX_TMP_ENCBUF_SIZE];
S8_T far dyndns_EncResult[MAX_ENCBUF_SIZE];

const S8_T code dyndns_Base64Table[65] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};
U8_T far dyndns_IpAddress[4];
S8_T far dyndns_CheckResult[16];
U8_T far dyndns_AppId;
//U8_T far dyndns_ConnId;

U32_T far dyndns_HostIp;
U16_T far dyndns_HostIpLen;
U8_T far dyndns_State;
U8_T far dyndns_bUpdateStart;
U8_T far dyndns_bUpdateDone;
U8_T far dyndns_Type;

u8 flag_DynDNS_Send;
static struct uip_udp_conn *dyndns_ConnId = NULL;


U32_T far dyndns_StartTime;

/* LOCAL SUBPROGRAM DECLARATIONS */
static BOOL dyndns_AssembleCommandPkt(void);
static BOOL dyndns_DoBase64Enc(void);
static void dyndns_Start(void);
static void dyndns_CloseConnection(void);

void tcp_client_reconnect(void);
/*
 * -----------------------------------------------------------------------------
 * Function Name: DynDNS_Init
 * Purpose: To initialize the parameter of this module and to bind a TCPIP connection
 * Params: none
 * Returns: none
 * Note:
 * ----------------------------------------------------------------------------
 */
void DynDNS_Init(void)
{
	uip_ipaddr_t addr;

//	test_dyndns_AppId = dyndns_AppId;
	// to initialize the parameters
	dyndns_State = DYNDNS_STATE_IDLE;
	dyndns_bUpdateStart = TRUE;
	dyndns_bUpdateDone = FALSE;
	flag_DynDNS_Send = 0;
	
}


/*
 * -----------------------------------------------------------------------------
 * Function Name: dyndns_CloseConnection
 * Purpose: to close the connection and reset the status
 * Params: none
 * Returns: none
 * Note:
 * ----------------------------------------------------------------------------
 */
void dyndns_CloseConnection(void)
{
	// to close the connection and reset the status
	dyndns_bUpdateDone = TRUE;
	dyndns_State = DYNDNS_STATE_IDLE;
}


/*
 * -----------------------------------------------------------------------------
 * Function Name: DynDNS_GetUpdateState
 * Purpose: To get the state of updating the dynamic dns 
 * Params: none
 * Returns: boolean value. TRUE if the procedure of update has done
 * Note:
 * ----------------------------------------------------------------------------
 */
BOOL DynDNS_GetUpdateState(void)
{
//	test_dyndns_state = dyndns_State;
	if(dyndns_bUpdateDone == TRUE)
	{
		dyndns_bUpdateStart = TRUE;
	}

	return dyndns_bUpdateDone;
}



/*
 * -----------------------------------------------------------------------------
 * Function Name: dyndns_DoBase64Enc
 * Purpose: to encode the username and password in BASE64
 * Params: none
 * Returns: boolean value. TRUE if no error occurs.
 * Note:
 * ----------------------------------------------------------------------------
 */
BOOL dyndns_DoBase64Enc(void)
{
	U16_T i, len;
	U16_T iResult;
	U32_T nValue, nTmpValue;
	U8_T nStart = 0;
	U16_T nAddr[2];
	
	i = 0;
	len = 0;
	while((len < MAX_TMP_ENCBUF_SIZE) && (dyndns_User[i] != 0))
	{
		dyndns_TmpEncData[len] = dyndns_User[i];
		len += 1;
		i += 1;
	}
	
	if(len > (MAX_TMP_ENCBUF_SIZE - 1))
	{
		return FALSE;
	}
	dyndns_TmpEncData[len] = ':';
	len += 1;
	i = 0;
	while((len < MAX_TMP_ENCBUF_SIZE) && (dyndns_Pass[i] != 0))
	{
		dyndns_TmpEncData[len] = dyndns_Pass[i];
		len += 1;
		i += 1;
	}
	
	if(len == MAX_TMP_ENCBUF_SIZE)
	{
		return FALSE;
	}

	dyndns_TmpEncData[len] = 0;

	// begin to encode in BASE64
	// 0x31='1'         0x32='2'         0x33='3'
	// _______________  _______________  _______________
	// 0 0 1 1 0 0 0 1  0 0 1 1 0 0 1 0  0 0 1 1 0 0 1 1
	// ___________ ____________ ____________ ___________
	// 12='M'      19='T'       8='I'        51='z'
	// "123" => "MTIz"
	nValue = 0;
	nStart = 0;
	iResult = 0;
	i = 0;
	while((i < MAX_ENCBUF_SIZE) && (dyndns_TmpEncData[i] != 0))
	{
		nValue <<= 8;
		nValue += dyndns_TmpEncData[i];
		i++;
		nStart++;
		if(nStart == 3)
		{
			nStart = 0;
			nTmpValue = nValue >> 18;
			dyndns_EncResult[iResult++] = dyndns_Base64Table[nTmpValue];
			nTmpValue = (nValue >> 12) & 0x0000003F;
			dyndns_EncResult[iResult++] = dyndns_Base64Table[nTmpValue];
			nTmpValue = (nValue >> 6) & 0x0000003F;
			dyndns_EncResult[iResult++] = dyndns_Base64Table[nTmpValue];
			nTmpValue = nValue & 0x0000003F;
			dyndns_EncResult[iResult++] = dyndns_Base64Table[nTmpValue];
			nValue = 0;
		}
	}
	if(nStart == 2)
	{
		nValue <<= 8;
		nTmpValue = nValue >> 18;
		dyndns_EncResult[iResult++] = dyndns_Base64Table[nTmpValue];
		nTmpValue = (nValue >> 12) & 0x0000003F;
		dyndns_EncResult[iResult++] = dyndns_Base64Table[nTmpValue];
		nTmpValue = (nValue >> 6) & 0x0000003F;
		dyndns_EncResult[iResult++] = dyndns_Base64Table[nTmpValue];
		dyndns_EncResult[iResult++] = '=';
	}else if(nStart == 1)
	{
		nValue <<= 16;
		nTmpValue = nValue >> 18;
		dyndns_EncResult[iResult++] = dyndns_Base64Table[nTmpValue];
		nTmpValue = (nValue >> 12) & 0x0000003F;
		dyndns_EncResult[iResult++] = dyndns_Base64Table[nTmpValue];
		dyndns_EncResult[iResult++] = '=';
		dyndns_EncResult[iResult++] = '=';
	}

	dyndns_EncResult[iResult] = 0;
	return TRUE;

}




/*
 * -----------------------------------------------------------------------------
 * Function Name: dyndns_AssembleCommandPkt
 * Purpose: to assemble the update command
 * Params: none
 * Returns: boolean value. TRUE if no error occurs.
 * Note:
 * ----------------------------------------------------------------------------
 */
BOOL dyndns_AssembleCommandPkt(void)
{
	U16_T nAddr[2];

	// to get the host ip of this device
	uip_gethostaddr(nAddr);
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
	dyndns_IpAddress[0] = (U8_T)(nAddr[0] % 256);
	dyndns_IpAddress[1] = (U8_T)(nAddr[0] / 256);
	dyndns_IpAddress[2] = (U8_T)(nAddr[1] % 256);
	dyndns_IpAddress[3] = (U8_T)(nAddr[1] / 256);
#else
	dyndns_IpAddress[0] = (U8_T)(nAddr[0] / 256);
	dyndns_IpAddress[1] = (U8_T)(nAddr[0] % 256);
	dyndns_IpAddress[2] = (U8_T)(nAddr[1] / 256);
	dyndns_IpAddress[3] = (U8_T)(nAddr[1] % 256);
#endif

	if(dyndns_Type == CONF_DDNS_3322)
	{
		// to assemble the update command for the 3322.org
		if(dyndns_DoBase64Enc() == FALSE)
		{
			return FALSE;
		}
		dyndns_CmdLen = sprintf(dyndns_Command, "GET /dyndns/update?system=dyndns&hostname=%s&myip=%u.%u.%u.%u&wildcard=OFF&mx=NOCHG&backmx=NO&offline=NO HTTP/1.0\r\n",
			dyndns_Domain, dyndns_IpAddress[0], dyndns_IpAddress[1], dyndns_IpAddress[2], dyndns_IpAddress[3]); 
		dyndns_CmdLen += sprintf(dyndns_Command + dyndns_CmdLen, "Host: members.3322.org\r\nAuthorization: Basic %s\r\nUser-Agent: myclient/1.0 me@null.net\r\n\r\n",
			dyndns_EncResult);
		
	
	}else if(dyndns_Type == CONF_DDNS_DYNDNS)
	{
		// to assemble the update command for the dyndns.org
		if(dyndns_DoBase64Enc() == FALSE)
		{
			return FALSE;
		}
		dyndns_CmdLen = sprintf(dyndns_Command, "GET /nic/update?system=dyndns&hostname=%s&myip=%u.%u.%u.%u&wildcard=OFF&mx=NOCHG&backmx=NO&offline=NO HTTP/1.0\r\n",
			dyndns_Domain, dyndns_IpAddress[0], dyndns_IpAddress[1], dyndns_IpAddress[2], dyndns_IpAddress[3]); 
		dyndns_CmdLen += sprintf(dyndns_Command + dyndns_CmdLen, "Host: members.dyndns.org\r\nAuthorization: Basic %s\r\nUser-Agent: myclient/1.0 me@null.net\r\n\r\n",
			dyndns_EncResult);
	}else if(dyndns_Type == CONF_DDNS_NOIP)
	{
		// to assemble the update command for the no-ip.com
		dyndns_CmdLen = sprintf(dyndns_Command, "GET /dns?username=%s&password=%s&hostname=%s&ip=%u.%u.%u.%u HTTP/1.0\r\n",
			dyndns_User, dyndns_Pass, dyndns_Domain, dyndns_IpAddress[0], dyndns_IpAddress[1], dyndns_IpAddress[2], dyndns_IpAddress[3]); 
		dyndns_CmdLen += sprintf(dyndns_Command + dyndns_CmdLen, "Host: dynupdate.no-ip.com\r\nUser-Agent: myclient/1.0 me@null.net\r\n\r\n");
	}
	dyndns_HostIpLen = sprintf(dyndns_CheckResult, "%u.%u.%u.%u", dyndns_IpAddress[0], dyndns_IpAddress[1], dyndns_IpAddress[2], dyndns_IpAddress[3]);
	return TRUE;

}


/*
* -----------------------------------------------------------------------------
 * Function Name: dyndns_Start
 * Purpose: to start the update dynamic DNS procedure
 * Params: none
 * Returns: none
 * Note:
 * ----------------------------------------------------------------------------
 */
void dyndns_Start(void)
{
	U32_T nCurrentTime;
	uip_ipaddr_t addr;
	switch(dyndns_State)
	{
	// query the host ip of the ddns server
	case DYNDNS_STATE_MAKE_CONNECT:
		// to get a TCPIP connection
//		dyndns_ConnId = TCPIP_TcpNew(dyndns_AppId, 0, dyndns_HostIp, 0, 80);
//		if(dyndns_ConnId != NULL) 
//			uip_udp_remove(dyndns_ConnId);
		uip_ipaddr(&addr,(U8_T)(dyndns_server_ip >> 24), (U8_T)(dyndns_server_ip >> 16), (U8_T)(dyndns_server_ip >> 8), (U8_T)(dyndns_server_ip));	
		//uip_ipaddr(&addr,8,23,224,110);
		dyndns_State = DYNDNS_STATE_WAIT_CONNECT;
		dyndns_StartTime = uip_timer;
		break;
	case DYNDNS_STATE_WAIT_CONNECT:
		// check if the connection is established?
		nCurrentTime = uip_timer;
		if((nCurrentTime - dyndns_StartTime) > MAX_TIMEOUT_VALUE)
		{
			// TCPIP connection fail, set the flag of done and return
			dyndns_CloseConnection();
		}
		break;
	case DYNDNS_STATE_SEND_COMMAND:
		// send the update command to the server
		if(dyndns_AssembleCommandPkt() == FALSE)
		{
			// fail to assemble the update command, set the flag of done and return
			dyndns_bUpdateDone = TRUE;
		}
		else
		{
			// send the update command to the dynamic DNS server
//			TCPIP_TcpSend(dyndns_ConnId, dyndns_Command, dyndns_CmdLen, TCPIP_SEND_NOT_FINAL);
			//uip_send(dyndns_Command, dyndns_CmdLen);
			dyndns_State = DYNDNS_STATE_WAIT_RESPONSE;
			dyndns_StartTime = uip_timer;
			flag_DynDNS_Send = 1;
		}
	   	break;
	case DYNDNS_STATE_WAIT_RESPONSE:
		// check if the response from the server is received?
		nCurrentTime = uip_timer;			

		if((nCurrentTime - dyndns_StartTime) > MAX_TIMEOUT_VALUE)
		{
			// no response from the DDNS server, set the flag of done and return
			dyndns_CloseConnection();
		}
		else
		{
			
		}
		break;
	default:
		break;
	}

}

/*
 * -----------------------------------------------------------------------------
 * Function Name: DynDNS_DoUpdateDynDns
 * Purpose: to begin to do the update procedure
 * Params:
 *		Type(CONF_DDNS_3322, CONF_DDNS_DYNDNS, CONF_DDNS_NOIP)
 *		HostIp : the server's ip
 *		User : username
 *		Passwd : password
 * Returns: none
 * Note:
 * ----------------------------------------------------------------------------
 */
void DynDNS_DoUpdateDynDns(U8_T Type, U32_T HostIp, U8_T *Domain, U8_T *User, U8_T *Pass)
{
	U16_T i;
	if(dyndns_bUpdateStart == TRUE)
	{
		// to get the string of domain name
		for(i = 0; i < MAX_DOMAIN_SIZE; i++)
		{
			if(Domain[i] == 0)
			{
				break;
			}
			dyndns_Domain[i] = Domain[i];
		}
		// chech if the size of username reach up to the max value?
		if(i == MAX_DOMAIN_SIZE)
		{ 
			dyndns_bUpdateDone = TRUE;
			return;		
		}
		// to get the string of username
		for(i = 0; i < MAX_USERNAME_SIZE; i++)
		{
			if(User[i] == 0)
			{
				break;
			}
			dyndns_User[i] = User[i];
		}
		// chech if the size of username reach up to the max value?
		if(i == MAX_USERNAME_SIZE)
		{
			dyndns_bUpdateDone = TRUE;
			return;		
		}
		// to get the string of password
		for(i = 0; i < MAX_PASSWORD_SIZE; i++)
		{
			if(Pass[i] == 0)
			{
				break;
			}
			dyndns_Pass[i] = Pass[i];
		}
		// chech if the size of password reach up to the max value?
		if(i == MAX_PASSWORD_SIZE)
		{
			dyndns_bUpdateDone = TRUE;
			return;		
		}
		// to get the Host IP
		dyndns_HostIp = HostIp;
		// to get the type of server
		dyndns_Type = Type;
		// set the state to query the host ip of the dynamic DNS server
		dyndns_State = DYNDNS_STATE_MAKE_CONNECT;
		// reset the flag of start and done
		dyndns_bUpdateStart = FALSE;
		dyndns_bUpdateDone = FALSE;
		// check if the ddns type is available in this module?
		if(/*(dyndns_Type == 0) ||*/ (dyndns_Type > 3))
		{
			// the ddns type is not found, set the flag of done and return;
			dyndns_bUpdateDone = TRUE;
			return;		
		}
	}
	dyndns_Start();
}


U8_T DynDNS_GetState(void)
{
	return dyndns_State;
}

u8 tcp_client_databuf[500];   	//发送数据缓存	  
//u8 tcp_client_sta;				//客户端状态
// tcp client

void DynDNS_appcall(void)
{
	struct tcp_demo_appstate *s = (struct tcp_demo_appstate *)&uip_conn->appstate;

	if(uip_poll()) 
	{
		if(dyndns_State == DYNDNS_STATE_WAIT_RESPONSE)
		{
			if(flag_DynDNS_Send)
			{
				flag_DynDNS_Send = 0;
				uip_send(dyndns_Command, dyndns_CmdLen);
			}
		}
	}
	
//	if(uip_aborted()) 	tcp_client_aborted();		//连接终止	   
//	if(uip_timedout())  tcp_client_timedout();	//连接超时   
//	if(uip_closed())  	tcp_client_closed();		//连接关闭	
 	if(uip_connected()) //连接成功	
	{
		dyndns_State = DYNDNS_STATE_SEND_COMMAND;
//#if ARM_UART_DEBUG
//		UART_Init(0);
//		DEBUG_EN = 1;
//		printf(" connected \r\n");
//#endif
		//uip_send(dyndns_Command, dyndns_CmdLen);
	}   

		
 	//接收到一个新的TCP数据包 
	if(uip_newdata())
	{
		if(dyndns_State == DYNDNS_STATE_WAIT_RESPONSE)
		{
//#if ARM_UART_DEBUG
//		UART_Init(0);
//		DEBUG_EN = 1;
//		printf(" received ok %d: %s\r\n",uip_len,uip_appdata);
//#endif
			dyndns_State = DYNDNS_STATE_UPDATE_OK;  // added by chelsea
		} 
	}
//	else if(tcp_client_sta & (1 << 5))			//有数据需要发送
//	{
//		s->textptr = tcp_client_databuf;
//		s->textlen = strlen((const char*)tcp_client_databuf);
//		tcp_client_sta &= ~(1 << 5);			//清除标记
//	}
//	
//	//当需要重发、新数据到达、数据包送达、连接建立时，通知uip发送数据 
//	if(uip_rexmit() || uip_newdata() || uip_acked() || uip_connected() || uip_poll())
//	{
//		//tcp_client_senddata();
//	}			
}



void tcp_client_reconnect(void)
{
//	struct uip_conn * server_conn;
//	U8_T count;
//	xTimeOutType x_timeout;
//	portTickType openTimeout;
//	uip_ipaddr_t *ipaddr;
//	uip_ipaddr(ipaddr, 192, 168, 0, 124);		//设置IP为192.168.1.103

//	server_conn = uip_connect(ipaddr,HTONS(10005));

}

//终止连接				    
//void tcp_client_aborted(void)
//{
//	tcp_client_sta &= ~(1 << 7);				//标志没有连接
////	tcp_client_reconnect();						//尝试重新连接
////	uip_log("tcp_client aborted!\r\n");			//打印log
//}

//////连接超时
//void tcp_client_timedout(void)
//{
//	tcp_client_sta &= ~(1 << 7);				//标志没有连接	   
////	uip_log("tcp_client timeout!\r\n");			//打印log
//}

//////连接关闭
//void tcp_client_closed(void)
//{
//	tcp_client_sta &= ~(1 << 7);				//标志没有连接
//}

////连接建立
//void tcp_client_connected(void)
//{ 
////struct tcp_demo_appstate *s = (struct tcp_demo_appstate *)&uip_conn->appstate;
//tcp_client_sta |= 1 << 7;					//标志连接成功
////// 	uip_log("tcp_client connected!\r\n");		//打印log
////s->state = STATE_CMD;				 		//指令状态
////s->textlen = 0;
////s->textptr = "Demo Board Connected Successfully!\r\n";//回应消息
////s->textlen = strlen((char *)s->textptr);	  
//}

////发送的数据成功送达
//void tcp_client_acked(void)
//{											    
////	struct tcp_demo_appstate *s = (struct tcp_demo_appstate *)&uip_conn->appstate;
////	s->textlen = 0;								//发送清零
//////	uip_log("tcp_client acked!\r\n");			//表示成功发送		 
//}

////发送数据给服务端
void tcp_client_senddata(void)
{
//	struct tcp_demo_appstate *s = (struct tcp_demo_appstate *)&uip_conn->appstate;
//	//s->textptr:发送的数据包缓冲区指针
//	//s->textlen:数据包的大小（单位字节）		   
//	if(s->textlen > 0)
//		uip_send(s->textptr, s->textlen);//发送TCP数据包	 
}

