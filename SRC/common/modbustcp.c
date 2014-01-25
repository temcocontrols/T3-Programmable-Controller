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
#include "hsuart.h"
#include "main.h"

//U8_T far  Para[5670];

#if 1
/* NAMING CONSTANT DECLARATIONS */

/* GLOBAL VARIABLES DECLARATIONS */

/* LOCAL VARIABLES DECLARATIONS */
static HTTP_SERVER_CONN  HTTP_Connects[MAX_HTTP_CONNECT];
static U8_T  http_InterfaceId = 0;
U8_T	TcpIp_to_sub = 0;
U8_T count_resume_scan = 0;

U16_T transaction_id;
U16_T transaction_num;
U8_T tmp_sendbuf[200];

//unsigned int CRC16(unsigned char *puchMsg, unsigned char usDataLen);
extern U8_T count_sub_comm;
void forward_to_slave(U8_T *buf, U8_T length,U8_T port);
U8_T request_TCP(U8_T * buf,U16_T slen,U16_T rlen,U8_T port);

extern U16_T far Test[50];

void responseCmd(U8_T type,U8_T* pData,HTTP_SERVER_CONN * pHttpConn);  


void Set_transaction_ID(U8_T *str, U16_T id, U16_T num)
{
	str[0] = (U8_T)(id >> 8);		//transaction id
	str[1] = (U8_T)id;

	str[2] = 0;						//protocol id, modbus protocol = 0
	str[3] = 0;

	str[4] = (U8_T)(num >> 8);
	str[5] = (U8_T)num;
}

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
	U16_T   port;
	for (i = 0; i < MAX_HTTP_CONNECT; i++)
	{
		HTTP_Connects[i].State = HTTP_STATE_FREE;
		HTTP_Connects[i].FileId = 0xff;
	}
	port = Modbus.tcp_port;
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
	U8_T fileId = HTTP_Connects[id].FileId;


	 HTTP_Connects[id].State = event;


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
static U8_T send_tcp[300]={0};
extern U8_T InformationStr[40];
extern U8_T source;


U8_T firmware_update = FALSE;
void UdpData(void);
//U8_T E2prom_Write_Byte(U8_T addr,U8_T dat);
bit flag_transimit_from_tcpip = 0;
U8_T	TcpSocket_ME;
extern unsigned int far Test[50];
extern unsigned char tempvalue11;



void HTTP_Receive(U8_T XDATA* pData, U16_T length, U8_T conn_id)
{
	HTTP_SERVER_CONN XDATA*	pHttpConn = &HTTP_Connects[conn_id];
	U8_T i;
//	U8_T port = 0xff;
#if  defined(MINI)
	U8_T new_scan_buf[4];
#endif
	flagLED_ether_rx = 1;
	if( (pData[0] == 0xee) && (pData[1] == 0x10) &&
		(pData[2] == 0x00) && (pData[3] == 0x00) &&
		(pData[4] == 0x00) && (pData[5] == 0x00) &&
		(pData[6] == 0x00) && (pData[7] == 0x00) )
	{		
		UdpData();
		TCPIP_TcpSend(pHttpConn->TcpSocket, InformationStr, 40, TCPIP_SEND_FINAL);
		flagLED_ether_tx = 1;
		firmware_update = TRUE;	
	}
	else 
	{  	 	
		if(pData[UIP_HEAD] == Modbus.address 
		|| ((pData[UIP_HEAD] == 255)&& (pData[UIP_HEAD + 1] != 0x19)&&(pData[UIP_HEAD + 1] != 0x18))
		)
		{	
			responseCmd(TCP_IP,pData,pHttpConn);
		}
		else 
		{
		//	U16_T crc_val;
			U8_T header[6];			

			if((pData[UIP_HEAD] == 0x00) || 
			((pData[UIP_HEAD + 1] != READ_VARIABLES) 
			&& (pData[UIP_HEAD + 1] != WRITE_VARIABLES) 
			&& (pData[UIP_HEAD + 1] != MULTIPLE_WRITE) 
			&&(pData[UIP_HEAD + 1] != CHECKONLINE)
			&& (pData[UIP_HEAD + 1] != CHECKONLINE_WIHTCOM)))
				return;
//			if(((pData[UIP_HEAD + 4] << 8) | pData[UIP_HEAD + 5]) > 0x80)
//				return;
			if((pData[UIP_HEAD + 1] == MULTIPLE_WRITE) && ((length - UIP_HEAD) != (pData[UIP_HEAD + 6] + 7)))
				return;	
#if (defined(CM5))

			Modbus.sub_port = UART_SUB1;
#endif

#if  defined(MINI)

			if((pData[UIP_HEAD + 1] == CHECKONLINE)) // 0x19 UART0
			{
				Modbus.sub_port = UART_SUB1;
			}				
			else if((pData[UIP_HEAD + 1] == CHECKONLINE_WIHTCOM))  // 0x18 new scan com, attached com port
			{
				// create new scan buf				 
				memcpy(new_scan_buf,pData + UIP_HEAD,4);
				new_scan_buf[1] = 0x19;
				if(pData[UIP_HEAD + 4] == 1) 
					Modbus.sub_port = UART_ZIG;
				else  if(pData[UIP_HEAD + 4] == 2) 	
					Modbus.sub_port = UART_SUB2;
			}
			else 
			{	
				Modbus.sub_port = 0xff;			
				for(i = 0;i <  sub_no ;i++)
				{
					if(pData[UIP_HEAD] == uart0_sub_addr[i])
					{	
						Modbus.sub_port = UART_SUB1;
						continue;
					}
					else if(pData[UIP_HEAD] == uart2_sub_addr[i])
					{
						 Modbus.sub_port = UART_SUB2;
						 continue;
					}
					else if(pData[UIP_HEAD] == uart1_sub_addr[i])
					{	
						Modbus.sub_port = UART_ZIG;
						continue;
					}
				}
				if(Modbus.sub_port == 0xff)
				{  
					return;
				}	
			}		
#endif
			TcpIp_to_sub = 1;
			count_resume_scan = 0;

			vTaskSuspend(Handle_Scan); 
			vTaskSuspend(Handle_ParameterOperation);
	//		vTaskSuspend(Handle_MainSerial);
			vTaskSuspend(xHandler_Output); 
			vTaskSuspend(xHandleBACnetComm); 
			vTaskSuspend(xHandleCommon);
			vTaskSuspend(xHandleBacnetControl);
#if  defined(MINI)			
			 vTaskSuspend(xHandleUSB); 
			 vTaskSuspend(xHandler_SPI);
#endif

			TcpSocket_ME = pHttpConn->TcpSocket;

			Set_transaction_ID(header, ((U16_T)pData[0] << 8) | pData[1], 2 * pData[UIP_HEAD + 5] + 3);

#if  defined(MINI)			
			if(pData[UIP_HEAD + 1] == CHECKONLINE_WIHTCOM)
			{
				Response_TCPIP_To_SUB(new_scan_buf,4,Modbus.sub_port,header);
			}
			else 
#endif	
			{
	 			Response_TCPIP_To_SUB(pData + UIP_HEAD,length - UIP_HEAD,Modbus.sub_port,header);
			}
		//	vTaskResume(Handle_Scan); 
		//	vTaskResume(Handle_ParameterOperation);
		//	vTaskResume(Handle_MainSerial);
			vTaskResume(xHandler_Output); 
			vTaskResume(xHandleBACnetComm); 
			vTaskResume(xHandleCommon);
			vTaskResume(xHandleBacnetControl);
#if  defined(MINI)			
			vTaskResume(xHandleUSB); 
			vTaskResume(xHandler_SPI);
#endif			
		}
	}


} /* End of HTTP_Receive() */







#endif
