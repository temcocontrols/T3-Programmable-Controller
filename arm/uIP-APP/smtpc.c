/*
*********************************************************************************
*     Copyright (c) 2006   ASIX Electronic Corporation      All rights reserved.
*
*     This is unpublished proprietary source code of ASIX Electronic Corporation
*
*     The copyright notice above does not evidence any actual or intended
*     publication of such source code.
*********************************************************************************
*/
/*============================================================================
 * Module name: smtpc.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: smtpc.c,v $
 *
 * Revision 2.0 2011/02/22 20:33:00 by Smile
 * Modify the buffer address(smtp_Buf and smtp_Subject) to be get via SMTPC_Init() function.
 * 
 * Revision 1.3  2006/08/31 07:49:05  borbin
 * no message
 *
 * Revision 1.2  2006/07/25 05:36:23  borbin
 * no message
 *
 * Revision 1.1.1.1  2006/06/20 05:50:28  borbin
 * no message
 *=============================================================================
 */

/* INCLUDE FILE DECLARATIONS */
//#include "adapter.h"
#include "smtpc.h"
#include "tcpip.h"
#include "main.h"
#include "md5.h"
#include "product.h"
/* NAMING CONSTANT DECLARATIONS */
//#include <stdarg.h>
/* GLOBAL VARIABLES DECLARATIONS */
#if SMTP
S8_T each3toc;	/* Buffer each 3 byte to coding Base64 format*/
S8_T b64[4];	/* memory output 4 base64 data*/


//void cram_md5(char *s,unsigned char *cram);
unsigned char encode_base64(char * str);

/* STATIC VARIABLE DECLARATIONS */
S8_T smtpc_Buf[256];
U8_T smtpc_rcv_Buf[200];
U8_T smtpc_Subject[50];
char smtpc_Context[100];
SMTPC_CONN  smtpc_Conns;
static U8_T XDATA smtpc_InterAppId;
//char email_server[30];
//char email_pass[30];	
//U8_T EmailServerIP[4];




/* LOCAL SUBPROGRAM DECLARATIONS */

/*
 * ----------------------------------------------------------------------------
 * Function Name: SMTPC_Init
 * Purpose: to initial the FTP client connection information.
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void SMTPC_Init(U8_T *pBuf, U8_T *pSubject)
{
	smtpc_Conns.State = SMTP_STATE_NONE;
	smtpc_Conns.ServerIp = 0;
//	smtpc_InterAppId = TCPIP_Bind(NULL, SMTPC_Event, SMTPC_Receive);
//    smtpc_Buf = pBuf;
//    smtpc_Subject = pSubject;
} /* End of SMTPC_Init() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: SMTPC_Event
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
//void SMTPC_Event(U8_T id, U8_T event)
//{
//	if (id != 0)
//		return;

//	if (event <= TCPIP_CONNECT_ACTIVE)
//		smtpc_Conns.State = event;
//	else if (event == TCPIP_CONNECT_ACTIVE)
//		smtpc_Conns.State = SMTP_STATE_CONNECTED;
//	else if (event == TCPIP_CONNECT_XMIT_COMPLETE)
//	{
//		if (smtpc_Conns.State == SMTP_STATE_SEND_MESSAGE)
//		{
//			smtpc_Buf[0] = 0xd;
//			smtpc_Buf[1] = 0xa;
//			smtpc_Buf[2] = '.';
//			smtpc_Buf[3] = 0xd;
//			smtpc_Buf[4] = 0xa;
////			TCPIP_TcpSend(smtpc_Conns.TcpSocket, smtpc_Buf, 5, TCPIP_SEND_NOT_FINAL);
//			uip_send(smtpc_Buf, 5);
//			smtpc_Conns.State = SMTP_STATE_MESSAGE_SENT;
//		}
//	}

//} /* End of SMTPC_Event() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: SMTPC_Receive
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void SMTPC_Receive(U8_T * pbuf, U16_T length)
{
	U16_T		codes, len;
	U8_T		*point, *pData;


	codes = (pbuf[0] - '0') * 100 + (pbuf[1] - '0') * 10 + (pbuf[2] - '0');
#if 1//ARM_UART_DEBUG
	uart1_init(115200);
	DEBUG_EN = 1;
		printf("rcv %u, %u, %s\r\n",length,smtpc_Conns.State,pbuf);
#endif
	if(length == 0) { return;}
	
	if (smtpc_Conns.State == SMTP_STATE_CONNECTED)
	{	
		if (codes != 220)
		{
			goto sendquit;
		}
		len = 0;
		smtpc_Buf[len++] = 'E';
		smtpc_Buf[len++] = 'H';
		smtpc_Buf[len++] = 'L';
		smtpc_Buf[len++] = 'O';
		smtpc_Buf[len++] = ' ';
		
		
		pData = Email_Setting.reg.smtp_domain;
		point = &smtpc_Buf[len];
		while (*pData != 0)
		{
			*point++ = *pData++;
			len++;
		}
		smtpc_Buf[len++] = 0x0d;
		smtpc_Buf[len++] = 0x0a;
		
		uip_send(smtpc_Buf, len);
		smtpc_Conns.State = SMTP_STATE_HELO_SENT;

	}
	else if (smtpc_Conns.State == SMTP_STATE_HELO_SENT)
	{	
		if (codes != 250)
		{
			goto sendquit;
		}
		len = 0;
		smtpc_Buf[len++] = 'A';
		smtpc_Buf[len++] = 'U';
		smtpc_Buf[len++] = 'T';
		smtpc_Buf[len++] = 'H';
		smtpc_Buf[len++] = ' ';
//		smtpc_Buf[len++] = 'C';
//		smtpc_Buf[len++] = 'R';
//		smtpc_Buf[len++] = 'A';
//		smtpc_Buf[len++] = 'M';
//		smtpc_Buf[len++] = '-';
//		smtpc_Buf[len++] = 'M';
//		smtpc_Buf[len++] = 'D';
//		smtpc_Buf[len++] = '5';		
		smtpc_Buf[len++] = 'L';
		smtpc_Buf[len++] = 'O';
		smtpc_Buf[len++] = 'G';
		smtpc_Buf[len++] = 'I';		
		smtpc_Buf[len++] = 'N';
		smtpc_Buf[len++] = 0x0d;
		smtpc_Buf[len++] = 0x0a;		
		
		uip_send(smtpc_Buf, len);
		smtpc_Conns.State = SMTP_STATE_USERPASS1_SENT;

	}
	else if(smtpc_Conns.State == SMTP_STATE_USERPASS1_SENT)
	{
//		if (codes != 334)  
//			goto sendquit;
		if(codes == 334)
		{
			Test[11]++;
		// send user and password	
			len = encode_base64(Email_Setting.reg.user_name);
			//len = encode_base64("chelsea@temcocontrols.com");
			Test[21] = len;			
			uip_send(smtpc_Buf, len);
		
			smtpc_Conns.State = SMTP_STATE_USERPASS2_SENT;

		}
	}
	else if(smtpc_Conns.State == SMTP_STATE_USERPASS2_SENT)
	{
		if(codes == 334)
		{
		// send user and password	
			//len = encode_base64("Travel321");
		//len = encode_base64("jsrtcaw@#dD1");
			len = encode_base64(Email_Setting.reg.password);
			uip_send(smtpc_Buf, len);
		
			smtpc_Conns.State = SMTP_STATE_AUTH_SENT;
		}
	}
	else if (smtpc_Conns.State == SMTP_STATE_AUTH_SENT)
	{
		if (codes == 250)
			return;
		if (codes != 235)
			goto sendquit;
			
		smtpc_Buf[0] = 'M';
		smtpc_Buf[1] = 'A';
		smtpc_Buf[2] = 'I';
		smtpc_Buf[3] = 'L';
		smtpc_Buf[4] = ' ';
		smtpc_Buf[5] = 'F';
		smtpc_Buf[6] = 'R';
		smtpc_Buf[7] = 'O';
		smtpc_Buf[8] = 'M';
		smtpc_Buf[9] = ':';
		smtpc_Buf[10] = '<';

		len = 11;
		pData = smtpc_Conns.From;
		point = &smtpc_Buf[11];
		while (*pData != 0)
		{
			*point++ = *pData++;
			len++;
		}

		*point++ = '>';
		*point++ = 0x0d;
		*point = 0x0a;
		len += 3;
		uip_send(smtpc_Buf, len);
		smtpc_Conns.State = SMTP_STATE_FROM_SENT;
	}
	else if ((smtpc_Conns.State >= SMTP_STATE_FROM_SENT) &&	(smtpc_Conns.State < SMTP_STATE_RCV3_SENT))
	{
		if (smtpc_Conns.State == SMTP_STATE_FROM_SENT)
		{
			if (smtpc_Conns.To1[0])
			{
				pData = smtpc_Conns.To1;
				smtpc_Conns.State = SMTP_STATE_RCV1_SENT;
			}
			else if (smtpc_Conns.To2[0])
			{
				pData = smtpc_Conns.To2;
				smtpc_Conns.State = SMTP_STATE_RCV2_SENT;
			}
			else if (smtpc_Conns.To3[0])
			{
				pData = smtpc_Conns.To3;
				smtpc_Conns.State = SMTP_STATE_RCV3_SENT;
			}
			else
				goto sendquit;				
		}
		else if (smtpc_Conns.State == SMTP_STATE_RCV1_SENT)
		{
			if (smtpc_Conns.To2[0])
			{
				pData = smtpc_Conns.To2;
				smtpc_Conns.State = SMTP_STATE_RCV2_SENT;
			}
			else if (smtpc_Conns.To3[0])
			{
				pData = smtpc_Conns.To3;
				smtpc_Conns.State = SMTP_STATE_RCV3_SENT;
			}
			else
				goto senddata;
		}
		else if (smtpc_Conns.State == SMTP_STATE_RCV2_SENT)
		{
			if (smtpc_Conns.To3[0])
			{
				pData = smtpc_Conns.To3;
				smtpc_Conns.State = SMTP_STATE_RCV3_SENT;
			}
			else
				goto senddata;
		}

		smtpc_Buf[0] = 'R';
		smtpc_Buf[1] = 'C';
		smtpc_Buf[2] = 'P';
		smtpc_Buf[3] = 'T';
		smtpc_Buf[4] = ' ';
		smtpc_Buf[5] = 'T';
		smtpc_Buf[6] = 'O';
		smtpc_Buf[7] = ':';
		smtpc_Buf[8] = '<';

		len = 9;
		point = &smtpc_Buf[9];
		while (*pData != 0)
		{
			*point++ = *pData++;
			len++;
		}

		*point++ = '>';
		*point++ = 0x0d;
		*point = 0x0a;
		len += 3;
		
		
		// add MD5
		
		uip_send(smtpc_Buf, len);
	}
	else if (smtpc_Conns.State == SMTP_STATE_RCV3_SENT)
	{
senddata:
		smtpc_Buf[0] = 'D';
		smtpc_Buf[1] = 'A';
		smtpc_Buf[2] = 'T';
		smtpc_Buf[3] = 'A';
		smtpc_Buf[4] = 0x0d;
		smtpc_Buf[5] = 0x0a;
		len = 6;
		//TCPIP_TcpSend(smtpc_Conns.TcpSocket, smtpc_Buf, len, TCPIP_SEND_NOT_FINAL);
		uip_send(smtpc_Buf, len);
		smtpc_Conns.State = SMTP_STATE_DATA_SENT;
	}
	else if (smtpc_Conns.State == SMTP_STATE_DATA_SENT)
	{
		if (codes != 354)
			goto sendquit;
			
		smtpc_Conns.State = SMTP_STATE_WAIT_MESSAGE;//wait for user send the message.
#if ARM_UART_DEBUG
	uart1_init(115200);
	DEBUG_EN = 1;
	printf("send SMTP_STATE_WAIT_MESSAGE\r\n");
#endif
	}
	else if (smtpc_Conns.State == SMTP_STATE_MESSAGE_SENT)
	{
sendquit:
		smtpc_Buf[0] = 'Q';
		smtpc_Buf[1] = 'U';
		smtpc_Buf[2] = 'I';
		smtpc_Buf[3] = 'T';
		smtpc_Buf[4] = 0x0d;
		smtpc_Buf[5] = 0x0a;
		len = 6;
		uip_send(smtpc_Buf, len);
		smtpc_Conns.State = SMTP_STATE_QUIT_SENT;
#if 1//ARM_UART_DEBUG
	uart1_init(115200);
	DEBUG_EN = 1;
	printf("send SMTP_STATE_QUIT_SENT\r\n");
#endif
	}

} /* End of SMTPC_Receive() */





/*
 * ----------------------------------------------------------------------------
 * Function Name: SMTPC_SendMessage
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void TcpDebug_SendMessage(U8_T* pBuf, U16_T length)
{
	U16_T		len, j;
	U8_T		*point, *pData, *ptemp;
	S8_T*		pimage;		/* point to image data */
	U8_T		char_count, i;
	U8_T		multi = 0;

	/* From header */
	//smtpc_Buf[0] = 'F';
	if(length<256)
		memcpy(smtpc_Buf,pBuf,length);
	uip_send(smtpc_Buf, length);
	

} /* End of SMTPC_SendMessage() */


//char mystring_buffer[100];
////tcp_printf
//void tcp_printf(const char *fmt,...)  
//{  
////    va_list ap;  
////		 U16_T length = 0;
////    char xdata  mystring[50]; //xdata
////   va_start(ap,fmt);  
////    vsnprintf(mystring,49,fmt,ap);
////   va_end(ap);  
//	
//	
//	  va_list args;
//  va_start(args, fmt);    
//  //sprintf(mystring_buffer,fmt,args);        
//	//sprintf(mystring_buffer,"asd"); 
//  va_end(args);
//	
//	

//   
//	//length = strlen(mystring);
//	//  uip_send(mystring, length);
//}  



/*
 * ----------------------------------------------------------------------------
 * Function Name: SMTPC_SendMessage
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void SMTPC_SendMessage(U8_T* pBuf, U16_T length)
{
	U16_T		len, j;
	U8_T		*point, *pData, *ptemp;
	S8_T*		pimage;		/* point to image data */
	U8_T		char_count, i;
	U8_T		multi = 0;

	/* From header */
	smtpc_Buf[0] = 'F';
	smtpc_Buf[1] = 'r';
	smtpc_Buf[2] = 'o';
	smtpc_Buf[3] = 'm';
	smtpc_Buf[4] = ':';
	smtpc_Buf[5] = ' ';
	smtpc_Buf[6] = '"';

	len = 7;
	pData = smtpc_Conns.From;
	point = &smtpc_Buf[7];
	while (*pData != 0)
	{
		*point++ = *pData++;
		len++;
	}

	*point++ = '"';
	*point++ = ' ';
	*point++ = '<';
	len += 3;
	pData = smtpc_Conns.From;
	while (*pData != 0)
	{
		*point++ = *pData++;
		len++;
	}

	*point++ = '>';
	*point++ = 0x0d;
	*point++ = 0x0a;
	len += 3;

	/* To header */
	*point++ = 'T';
	*point++ = 'o';
	*point++ = ':';
	*point++ = ' ';
	len += 4;

	for (i = 0; i < 3; i++)
	{
		if (i == 0)
		{
			if (smtpc_Conns.To1[0])
			{
				pData = smtpc_Conns.To1;
				ptemp = smtpc_Conns.To1;
			}
			else
				continue;
		}
		else if (i == 1)
		{
			if (smtpc_Conns.To2[0])
			{
				pData = smtpc_Conns.To2;
				ptemp = smtpc_Conns.To2;
			}
			else
				continue;
		}
		else
		{
			if (smtpc_Conns.To3[0])
			{
				pData = smtpc_Conns.To3;
				ptemp = smtpc_Conns.To3;
			}
			else
				continue;
		}

		if (multi == 1)
		{
			*point++ = ',';
			len++;
		}

		*point++ = '"';
		len++;

		while (*pData != 0)
		{
			*point++ = *pData++;
			len++;
		}

		*point++ = '"';
		*point++ = ' ';
		*point++ = '<';
		len += 3;

		pData = ptemp;
		while (*pData != 0)
		{
			*point++ = *pData++;
			len++;
		}

		*point++ = '>';
		len += 1;
		multi = 1;
	}

	*point++ = 0x0d;
	*point++ = 0x0a;
	len += 2;
	/* Subject header */
	*point++ = 'S';
	*point++ = 'u';
	*point++ = 'b';
	*point++ = 'j';
	*point++ = 'e';
	*point++ = 'c';
	*point++ = 't';
	*point++ = ':';
	*point++ = ' ';
	len += 9;

	memcpy(smtpc_Subject,"email test\r\n",20);
	pData = smtpc_Subject;
	while (*pData != 0)
	{
		if (*pData == 'Z')
		{
			*point++ = 9;
			pData++;
		}
		else
			*point++ = *pData++;
		len++;
	}
	
	//Initial MIME64
//	Cmime64_Init();
//	char_count = 0;

	memcpy(smtpc_Context,"this is a test!!!\r\n",20);
	pData = smtpc_Context;
	while (*pData != 0)
	{
		if (*pData == 'Z')
		{
			*point++ = 9;
			pData++;
		}
		else
			*point++ = *pData++;
		len++;
	}
//	pimage = smtpc_Context;
//	for(j=0;j<length;j++)
//	{
//		cmime64((S8_T*)pimage);
//		each3toc++;
//		if(each3toc == 3)
//		{
//			if(char_count == SMTP_MAX_LENGTH)
//			{
//				*point++ = '\r';
//				*point++ = '\n';
//				len += 2;
//				char_count = 0;
//			}

//			for(i=0;i<4;i++)
//			{
//				*point++ = b64[3-i];/*output b64[3..0]array for original 4 byte*/
//				len++;
//			}
//			char_count +=4;
//			each3toc = 0;
//		} 
//		pimage++;
//	}
	*point++ = '\r';
	*point++ = '\n';
	len += 2;

	uip_send(smtpc_Buf, len);
	smtpc_Conns.State = SMTP_STATE_SEND_MESSAGE;

} /* End of SMTPC_SendMessage() */

///*
// * ----------------------------------------------------------------------------
// * Function Name: SMTPC_Start
// * Purpose: 
// * Params:
// * Returns:
// * Note:
// * ----------------------------------------------------------------------------
// */
//void SMTPC_Start(U8 *ip, U8_T* from, U8_T* to1, U8_T* to2, U8_T* to3)
//{
//	
//} /* End of SMTPC_Start() */





//void EthernetDebug_appcall(void)
//{
//	struct tcp_demo_appstate *s = (struct tcp_demo_appstate *)&uip_conn->appstate;
// static unsigned int n_test_value = 0;
//	char temp_char[20];
//	
//	if(uip_poll()) 
//	{
//		//if (smtpc_Conns.State == SMTP_STATE_WAIT_MESSAGE)
//		//{
//		//tcp_printf("test , %d", n_test_value++);
//		 sprintf(temp_char,"test%d",++n_test_value);
//			TcpDebug_SendMessage(temp_char,20);	
//		//}
//	}
//}



/*
 * ----------------------------------------------------------------------------
 * Function Name: SMTPC_GetState
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
//U8_T SMTPC_GetState(void)
//{
//	return smtpc_Conns.State; 

//} /* End of SMTPC_GetState() */

void SMTPC_appcall(void)
{
	struct tcp_demo_appstate *s = (struct tcp_demo_appstate *)&uip_conn->appstate;

	if(uip_poll()) 
	{
		if (smtpc_Conns.State == SMTP_STATE_WAIT_MESSAGE)
		{
			SMTPC_SendMessage("test",20);
			Test[18]++;			
		}
	}
	 	//接收到一个新的TCP数据包 
	if(uip_newdata())
	{	
		memset(&smtpc_rcv_Buf[0], 0,200);
		memcpy(&smtpc_rcv_Buf[0], uip_appdata,uip_len);
		// deal with receiving data
		SMTPC_Receive(smtpc_rcv_Buf, uip_len);
			
	}		
	if(uip_acked())//tcp_server_acked();			//发送的数据成功送达 
	{
		if (smtpc_Conns.State == SMTP_STATE_SEND_MESSAGE)
		{
			smtpc_Buf[0] = 0xd;
			smtpc_Buf[1] = 0xa;
			smtpc_Buf[2] = '.';
			smtpc_Buf[3] = 0xd;
			smtpc_Buf[4] = 0xa;
			uip_send(smtpc_Buf, 5);
			smtpc_Conns.State = SMTP_STATE_MESSAGE_SENT;
		}
	}
 	if(uip_connected()) //连接成功	
	{
		smtpc_Conns.State = SMTP_STATE_CONNECTED;
	}   

		

}



#if 1

unsigned char encode_base64(char * str)
{
	char j,i;
	//char * pimage = (char *)smtpc_Context;
	char * point = (char *)smtpc_Buf;
	char char_count;
	uint8 len;
	Cmime64_Init();
	memset(smtpc_Context,0,100);
	memcpy(smtpc_Context,str,strlen(str));
	char_count = 0;
	
	len = strlen(smtpc_Context);
	
	if(len % 3 != 0)		
		len = (strlen(smtpc_Context) + 3) / 3 * 3;
	// len不是3的倍数，一律补0
	for(j = 0;j < len;j++)
	{
		cmime64(&smtpc_Context[j]);			
		each3toc++;
		if(each3toc == 3)
		{
			if(char_count == SMTP_MAX_LENGTH)
			{
				*point++ = '\r';
				*point++ = '\n';
				char_count = 0;
			}
			
			for(i = 0;i < 4;i++)
			{
					*point++ = b64[3-i];/*output b64[3..0]array for original 4 byte*/
			}
			char_count +=4;
			each3toc = 0;
		} 
		//pimage++;
		
	}
	
	if(smtpc_Buf[char_count - 1] == 'A'
		&& smtpc_Buf[char_count - 2] == 'A')	
	{ // fixed by chelsea, ????????????????
		smtpc_Buf[char_count - 1] = '=';
		smtpc_Buf[char_count - 2] = '=';
	}
	
	*point++ = 0x0d;
	*point++ = 0x0a;
	
	char_count +=2;
	
	if(Test[3] == 200)
	{
#if 1//ARM_UART_DEBUG
	uart1_init(115200);
	DEBUG_EN = 1;
	printf("encode:%s\n",smtpc_Buf);	
#endif		
	}
	
	
	return char_count;
}

//void test_scram(void);
static struct uip_conn * debug_Client_Conn = NULL;
void Ethernet_Debug_Task()
{
	uip_ipaddr_t ipaddr;
	 unsigned char server_ip[4];
		if((debug_Client_Conn == NULL) || (debug_Client_Conn->tcpstateflags == UIP_CLOSED))
		{
			server_ip[0] = 192;
			server_ip[1] = 168;
			server_ip[2] = 0;
			server_ip[3] = 38;//Test[12];
			uip_ipaddr(ipaddr, server_ip[0], server_ip[1], server_ip[2], server_ip[3]);	
			debug_Client_Conn = uip_connect(&ipaddr,HTONS(1115));  //  消息发送函数 EthernetDebug_appcall
		}

}

void Email_Task(void)
{
	U8_T FromAddr[36];
	U8_T To1Addr[36];
	U8_T To2Addr[36];
	U8_T To3Addr[36];		
	uint16_t * ptr_rm_ip;
	
	
	uip_ipaddr_t ipaddr;
	struct uip_conn * Smtpc_Client_Conn = NULL;
	
//	memcpy(email_server,"mail.bravocontrols.com",36);
//	memcpy(email_pass,"Travel321",36);
//	memcpy(FromAddr,"alarms@temcocontrols.com",36);
	
//	memcpy(email_server,"localhost",36);
//	memcpy(FromAddr,"chelsea@temcocontrols.com",36);
//	memcpy(email_pass,"jsrtcaw@#dD1",36);
	
	memcpy(Email_Setting.reg.smtp_domain,"smtp.qq.com",36);
	memcpy(Email_Setting.reg.password,"903000lwh",36);
	memcpy(Email_Setting.reg.user_name,"57440569@qq.com",36);
	memcpy(Email_Setting.reg.email_address,"57440569@qq.com",36);
	
	memcpy(To1Addr,"chelsea@temcocontrols.com",36);
	memcpy(To2Addr,"57440569@qq.com",36);
	//memcpy(To3Addr,"fandu@temcocontrols.com",36);
	
	resolv_query(Email_Setting.reg.smtp_domain);

	if(memcmp(Email_Setting.reg.smtp_domain,"localhost",strlen(Email_Setting.reg.smtp_domain)))		
	{
		ptr_rm_ip = resolv_lookup(Email_Setting.reg.smtp_domain);
		memcpy(Email_Setting.reg.smtp_ip,ptr_rm_ip,4);
	}
	else
	{  // ip address of email server
		Email_Setting.reg.smtp_ip[0] = 192;
		Email_Setting.reg.smtp_ip[1] = 168;
		Email_Setting.reg.smtp_ip[2] = 0;
		Email_Setting.reg.smtp_ip[3] = 7;
	}

	if(Test[3] == 100)
	{
		Test[4]++;
		Test[3] = 0;
#if 1//ARM_UART_DEBUG
	uart1_init(115200);
	DEBUG_EN = 1;
	printf("server :%u %u %u %u\n",Email_Setting.reg.smtp_ip[0],Email_Setting.reg.smtp_ip[1],Email_Setting.reg.smtp_ip[2],Email_Setting.reg.smtp_ip[3]);	
#endif			
		memcpy(smtpc_Conns.From,FromAddr,50);
		memcpy(smtpc_Conns.To1,To1Addr,50);
		memcpy(smtpc_Conns.To2,To2Addr,50);
		memcpy(smtpc_Conns.To3,To3Addr,50);
		
		uip_ipaddr(ipaddr, Email_Setting.reg.smtp_ip[0], Email_Setting.reg.smtp_ip[1], Email_Setting.reg.smtp_ip[2], Email_Setting.reg.smtp_ip[3]);	
		Smtpc_Client_Conn = uip_connect(&ipaddr,HTONS(SMTP_SERVER_PORT));
	}

		
	if(Test[3] == 200)
	{		
		Test[5] =	encode_base64("test@test.com");
		Test[6] =	encode_base64("test");
		
		Test[3] = 0;
	}
	
} /* End of GEVENT_Task() */
#endif

#endif
/* End of smtpc.c */