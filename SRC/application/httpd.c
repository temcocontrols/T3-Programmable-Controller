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
 * Module Name: httpd.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: httpd.c,v $
 * Revision 1.1.1.1  2006/06/20 05:50:28  borbin
 * no message
 *
 * Revision 1.2  2006/02/24 00:33:06  borbin
 * no message
 *
 * Revision 1.1.1.1  2006/02/23 00:55:10  borbin
 * no message
 *
 *=============================================================================
 */


/* INCLUDE FILE DECLARATIONS */
#include "httpd.h"
//#include "filesys.h"
#include "adapter.h"
#include "tcpip.h"
#include "mstimer.h"
#include "uart.h"


#if STOE_TRANSPARENT
#include "uip_arp.h"
#endif
#if (!STOE_TRANSPARENT)
#include "mstimer.h"
#include "projdefs.h"
#include "task.h"
#endif
#include "absacc.h"
#include "ax11000.h"
#include <string.h>
#include "interrupt.h"
#include "define.h"
#include "serial.h"
#include "commsub.h"

//U8_T far  Para[5670];

#if 1
/* NAMING CONSTANT DECLARATIONS */

/* GLOBAL VARIABLES DECLARATIONS */

/* LOCAL VARIABLES DECLARATIONS */
static HTTP_SERVER_CONN far HTTP_Connects[MAX_HTTP_CONNECT];
static U8_T far http_InterfaceId = 0;

extern U16_T far Test[50];
unsigned int CRC16(unsigned char *puchMsg, unsigned char usDataLen);


void responseCmd(U8_T type,U8_T* pData,HTTP_SERVER_CONN * pHttpConn); 

void Response_TCPIP_To_SUB(U8_T *buf, U16_T len,U8_T port,U8_T *header);


/*
 * ----------------------------------------------------------------------------
 * Function Name: HTTP_Init
 * Purpose: Initialize HTTP server. 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void HTTP_Init(void)
{
	U8_T	i;
	
	for (i = 0; i < MAX_HTTP_CONNECT; i++)
	{
		HTTP_Connects[i].State = HTTP_STATE_FREE;
		HTTP_Connects[i].FileId = 0xff;
	}

//	port = TCP_PORT;
	http_InterfaceId = TCPIP_Bind(HTTP_NewConn, HTTP_Event, HTTP_Receive);
	TCPIP_TcpListen(TCP_PORT ,http_InterfaceId);
//	FSYS_Init();   // for test
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: HTTP_NewConn
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T HTTP_NewConn(U32_T XDATA* pip, U16_T remotePort, U8_T socket)
{
	U8_T	i;

	for (i = 0; i < MAX_HTTP_CONNECT; i++)
	{
		if (HTTP_Connects[i].State == HTTP_STATE_FREE)
		{
			HTTP_Connects[i].State = HTTP_STATE_ACTIVE;
			HTTP_Connects[i].Timer = (U16_T)/*xTaskGetTickCount()*/SWTIMER_Tick();// changed by chelsea 
			HTTP_Connects[i].Ip = *pip;
			HTTP_Connects[i].Port = remotePort;
			HTTP_Connects[i].TcpSocket = socket;

			return i;
		}

	}


	return TCPIP_NO_NEW_CONN;
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: HTTP_Event
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void HTTP_Event(U8_T id, U8_T event)
{
	U8_T	fileId = HTTP_Connects[id].FileId;

	if (event < TCPIP_CONNECT_XMIT_COMPLETE)
	{	
		HTTP_Connects[id].State = event;
	}
	else if (event == TCPIP_CONNECT_XMIT_COMPLETE)
	{
		U8_T*	pSour;
		U16_T	dataLen;
		if (HTTP_Connects[id].State == HTTP_STATE_SEND_HEADER)
		{
			


		}
		else if (HTTP_Connects[id].State == HTTP_STATE_SEND_DATA)
		{


		}
		else if	(HTTP_Connects[id].State == HTTP_STATE_SEND_FINAL)
		{
			HTTP_Connects[id].State = HTTP_STATE_FREE;
		}
	}

} /* End of HTTP_Event() */


/*
 * ----------------------------------------------------------------------------
 * Function Name: HTTP_Receive
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */

//unsigned short sessonid=1;
//U32_T address=0;
//U8_T far send_tcp[300]={0};
extern U8_T 	InformationStr[40];
extern U8_T source;

extern U8_T tmp_sendbuf[200];

U8_T firmware_update = FALSE;
void UdpData(void);
//U8_T E2prom_Write_Byte(U8_T addr,U8_T dat);
bit flag_transimit_from_tcpip = 0;
U8_T	TcpSocket_ME;
extern unsigned int far Test[50];

void Set_transaction_ID(U8_T *str, U16_T id, U16_T num)
{
	str[0] = (U8_T)(id >> 8);		//transaction id
	str[1] = (U8_T)id;

	str[2] = 0;						//protocol id, modbus protocol = 0
	str[3] = 0;

	str[4] = (U8_T)(num >> 8);
	str[5] = (U8_T)num;
}


void HTTP_Receive(U8_T XDATA* pData, U16_T length, U8_T conn_id)
{
	HTTP_SERVER_CONN XDATA*	pHttpConn = &HTTP_Connects[conn_id];
	U8_T i;
/*	U8_T XDATA				str_post[] = {"POST"};
	U8_T					command, status, fileId, index, fileStatus;
	U8_T XDATA*			pFName;
	U8_T XDATA*			pFNameExt;
	U8_T CODE*				pSour;
	U8_T XDATA*			pExpanSour;
	U16_T					data_len;
*/
 /*  	U16_T far ReCount;
    U16_T far SeCount; 
    U32_T far StartAdd;  
    U8_T  far RealNum;
    U16_T far Sin_Add;
	U16_T far RegNum;
	U32_T far i;
	U32_T far loop;
	U8_T  far NTFlag=0;*/

//	flag_transimit_from_tcpip = 0; 
								

	if( (pData[0] == 0xee) && (pData[1] == 0x10) &&
		(pData[2] == 0x00) && (pData[3] == 0x00) &&
		(pData[4] == 0x00) && (pData[5] == 0x00) &&
		(pData[6] == 0x00) && (pData[7] == 0x00) )
	{		
		UdpData();
		TCPIP_TcpSend(pHttpConn->TcpSocket, InformationStr, 40, TCPIP_SEND_FINAL);

		firmware_update = TRUE;	
	}
	else 
	{
	
	   	if((pData[UIP_HEAD] ==  Modbus_address)||(pData[UIP_HEAD] == 0xff))//Address of NetControl 
		{	
			responseCmd(TCP_IP,pData,pHttpConn);
	   	}			
		else 
		{
			U8_T header[6];
			U8_T port = UART0;

//			for(i = 0;i <  sub_no ;i++)
//			{
//				if(pData[UIP_HEAD] == uart0_sub_addr[i])
//				{	
//					port = UART0;
//				}
//				else if(pData[UIP_HEAD] == uart2_sub_addr[i])
//				{
//					 port = UART2;
//				}
//			}
			
			if((pData[UIP_HEAD] == 0x00) || ((pData[UIP_HEAD + 1] != READ_VARIABLES) && (pData[UIP_HEAD + 1] != WRITE_VARIABLES) && (pData[UIP_HEAD + 1] != MULTIPLE_WRITE)))
				return;
			if(((pData[UIP_HEAD + 4] << 8) | pData[UIP_HEAD + 5]) > 0x80)
				return;
//			if(((pData[UIP_HEAD + 1] == READ_VARIABLES) || (pData[UIP_HEAD + 1] == WRITE_VARIABLES)) && ((length - UIP_HEAD) != 6))
//				return;
			if((pData[UIP_HEAD + 1] == MULTIPLE_WRITE) && ((length - UIP_HEAD) != (pData[UIP_HEAD + 6] + 7)))
				return;

//			vTaskPrioritySet(xHandleTcp,5);
//			vTaskSuspend(Handle_Scan); 
//			vTaskSuspend(Handle_ParameterOperation);
		//	uart_init_send_com(sub_source_port);
			TcpSocket_ME = pHttpConn->TcpSocket;
//			
			if(pData[UIP_HEAD + 1] == 0x03)
			Set_transaction_ID(header, ((U16_T)pData[0] << 8) | pData[1], 2 * pData[UIP_HEAD + 5] + 3);
			else
			Set_transaction_ID(header, ((U16_T)pData[0] << 8) | pData[1], UIP_HEAD + 3);
////			i = 3;
//			while(i--)
//			{
//				forward_to_slave(pData + UIP_HEAD, length - UIP_HEAD,sub_source_port);
//	//			if(wait_sub_response(20) == TRUE)
//	//				break;
//			}
			
			// response TCPIP
			
			Response_TCPIP_To_SUB(pData + UIP_HEAD,length - UIP_HEAD,UART0,header);
//			vTaskResume(Handle_Scan); 
//			vTaskResume(Handle_ParameterOperation);
//			vTaskPrioritySet(xHandleTcp,2);	
		
		}
	}


} /* End of HTTP_Receive() */







#endif
