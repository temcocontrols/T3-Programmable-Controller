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


unsigned int CRC16(unsigned char *puchMsg, unsigned char usDataLen);


void responseCmd(U8_T type,U8_T* pData,HTTP_SERVER_CONN * pHttpConn); 



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
	
	http_InterfaceId = TCPIP_Bind(HTTP_NewConn, HTTP_Event, HTTP_Receive);
	TCPIP_TcpListen(HTTP_SERVER_PORT ,http_InterfaceId);
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
	#if 1
	//	b_Master_Slave_TCP = CM5;	
		//if(b_Master_Slave_TCP == CM5)
		//{
		   	if((pData[UIP_HEAD] ==  Modbus_address)||(pData[UIP_HEAD] == 0xff))//Address of NetControl 
			{	
				flag_transimit_from_tcpip = 0;		
				responseCmd(TCP_IP,pData,pHttpConn);
		   	} 
		//}			
		else 
		{
			for(i = 0;i < sub_no;i++)
			{
				if(pData[UIP_HEAD] == sub_addr[i])
				{	
					union {
						unsigned int  word;
						unsigned char byte[2];
					  }crc;
					
					//Test[30] = 101; 					
					i = sub_no;
					sub_init_send_com();
					TcpSocket_ME = pHttpConn->TcpSocket;
					if(pData[UIP_HEAD + 1] == READ_VARIABLES) 
					{
						sub_rece_size = pData[UIP_HEAD + 5] * 2 + UIP_HEAD + 3;
		
						tmp_sendbuf[0] = pData[0];			//	TransID
						tmp_sendbuf[1] = pData[1];	
						tmp_sendbuf[2] = 0;			//	ProtoID
						tmp_sendbuf[3] = 0;
						tmp_sendbuf[4] = (6 + pData[UIP_HEAD + 5] * 2) >> 8;	//	Len
						tmp_sendbuf[5] = (U8_T)(6 + pData[UIP_HEAD + 5] * 2) ;


						crc.word = CRC16(&pData[UIP_HEAD],0x06);
						for(i = 0;i < 6;i++)
						{
							sub_send_byte(pData[UIP_HEAD + i],CRC_YES);
						//	Test[15 + i] = pData[UIP_HEAD + i];
						}
						sub_send_byte(crc.byte[0],CRC_YES);
						sub_send_byte(crc.byte[1],CRC_YES);			
						
					}
					else if(pData[UIP_HEAD + 1] == WRITE_VARIABLES) 
					{
						sub_rece_size = 12;
						tmp_sendbuf[0] = pData[0];//0;			//	TransID
						tmp_sendbuf[1] = pData[1];//TransID++;	
						tmp_sendbuf[2] = 0;			//	ProtoID
						tmp_sendbuf[3] = 0;
						tmp_sendbuf[4] = (3 + UIP_HEAD) >> 8;	//	Len
						tmp_sendbuf[5] = (U8_T)(3 + UIP_HEAD) ;

						crc.word = CRC16(&pData[UIP_HEAD],0x06);
						for(i = 0;i < 6;i++)
						{
							sub_send_byte(pData[UIP_HEAD + i],CRC_YES);
						}
						sub_send_byte(crc.byte[0],CRC_YES);
						sub_send_byte(crc.byte[1],CRC_YES);	
						
					}
					else if(pData[UIP_HEAD + 1] == MULTIPLE_WRITE)  // MULTIPLE_WRITE
					{				
						sub_rece_size = 12;
						tmp_sendbuf[0] = pData[0];			//	TransID
						tmp_sendbuf[1] = pData[1];	
						tmp_sendbuf[2] = 0;			//	ProtoID
						tmp_sendbuf[3] = 0;
						tmp_sendbuf[4] = (3 + UIP_HEAD) >> 8;	//	Len
						tmp_sendbuf[5] = (U8_T)(3 + UIP_HEAD) ;
		
						crc.word = CRC16(&pData[UIP_HEAD],135);
						for(i = 0;i < 135;i++)
						{
							sub_send_byte(pData[UIP_HEAD + i],CRC_YES);
						}
						sub_send_byte(crc.byte[0],CRC_YES);
						sub_send_byte(crc.byte[1],CRC_YES);			
					}
					
					sub_serial_restart();  
					source = PC_TCPIP;
					flag_transimit_from_tcpip = 1;  // Tstat is sub device

				  /*
					wait_SubSerial(100);
					for(i = 0;i < sub_rece_size - 6;i++)
					{					
						tmp_sendbuf[6 + i] = sub_data_buffer[i];
						 	
					}					
					TCPIP_TcpSend(TcpSocket_ME, tmp_sendbuf, sub_rece_size, TCPIP_SEND_NOT_FINAL); */
				}

			}
		
		}
		#endif	
	}


} /* End of HTTP_Receive() */







#endif
