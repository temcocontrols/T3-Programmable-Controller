/* INCLUDE FILE DECLARATIONS */
#include "MODBUSTCP.h"
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
static MODBUSTCP_SERVER_CONN  MODBUSTCP_Connects[MAX_MODBUSTCP_CONNECT];
static U8_T  MODBUSTCP_InterfaceId = 0;

U16_T transaction_id;
U16_T transaction_num;
//U8_T tmp_sendbuf[200];
//static U8_T send_tcp[300]={0};
U8_T firmware_update = FALSE;
U8_T	TcpSocket_ME;

//extern xSemaphoreHandle sembip;

extern U8_T far flag_sem_response_modbus;

void UdpData(U8_T type);
void responseCmd(U8_T type,U8_T* pData,MODBUSTCP_SERVER_CONN * pMODBUSTCPConn);  


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
 * Function Name: MODBUSTCP_Init
 * Purpose: Initialize MODBUSTCP server. 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void ModbusTCP_Init(void)
{
	U8_T	i;
//	U16_T   port;
	for (i = 0; i < MAX_MODBUSTCP_CONNECT; i++)
	{
		MODBUSTCP_Connects[i].State = MODBUSTCP_STATE_FREE;
		MODBUSTCP_Connects[i].FileId = 0xff;
	}
	MODBUSTCP_InterfaceId = TCPIP_Bind(MODBUSTCP_NewConn, MODBUSTCP_Event, MODBUSTCP_Receive);
	TCPIP_TcpListen(Modbus.tcp_port ,MODBUSTCP_InterfaceId);
//	FSYS_Init();   // for test
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: MODBUSTCP_NewConn
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */


U8_T MODBUSTCP_NewConn(U32_T XDATA* pip, U16_T remotePort, U8_T socket)
{
	U8_T	i;
	for (i = 0; i < MAX_MODBUSTCP_CONNECT; i++)
	{
		if (MODBUSTCP_Connects[i].State == MODBUSTCP_STATE_FREE)
		{
			MODBUSTCP_Connects[i].State = MODBUSTCP_STATE_ACTIVE;
			MODBUSTCP_Connects[i].Timer = (U16_T)xTaskGetTickCount();// changed by chelsea 
			MODBUSTCP_Connects[i].Ip = *pip;
			MODBUSTCP_Connects[i].Port = remotePort;
			MODBUSTCP_Connects[i].TcpSocket = socket;
			return i;
		}
	}
	return TCPIP_NO_NEW_CONN;
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: MODBUSTCP_Event
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void MODBUSTCP_Event(U8_T id, U8_T event)
{
	U8_T fileId = MODBUSTCP_Connects[id].FileId;


	MODBUSTCP_Connects[id].State = event;


} /* End of MODBUSTCP_Event() */
	






/*
 * ----------------------------------------------------------------------------
 * Function Name: MODBUSTCP_Receive
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */

void MODBUSTCP_Receive(U8_T XDATA* pData, U16_T length, U8_T conn_id)
{
	MODBUSTCP_SERVER_CONN XDATA*	pMODBUSTCPConn = &MODBUSTCP_Connects[conn_id];
	U8_T i;
//	if(cSemaphoreTake(sembip, 50) == pdFALSE)
//		return;

	
	if( (pData[0] == 0xee) && (pData[1] == 0x10) &&
		(pData[2] == 0x00) && (pData[3] == 0x00) &&
		(pData[4] == 0x00) && (pData[5] == 0x00) &&
		(pData[6] == 0x00) && (pData[7] == 0x00) )
	{		
		UdpData(0);
		if(cSemaphoreTake( xSemaphore_tcp_send, ( portTickType ) 10 ) == pdTRUE)
		{	
			
			TCPIP_TcpSend(pMODBUSTCPConn->TcpSocket, &Scan_Infor, sizeof(STR_SCAN_CMD), TCPIP_SEND_FINAL);					
			cSemaphoreGive( xSemaphore_tcp_send );
		}		
		firmware_update = TRUE;	
	}
	else 
	{  
		// only transfer boadcast show id
		if(pData[UIP_HEAD] == Modbus.address 
		|| ((pData[UIP_HEAD] == 255) && (pData[UIP_HEAD + 1] != 0x19) && (pData[UIP_HEAD + 2] * 256 + pData[UIP_HEAD + 3] != 725))
		)
		{	
			if(flag_sem_response_modbus == 0)
				responseCmd(TCP,pData,pMODBUSTCPConn);
		}
		else 
		{
			U8_T header[6];	
			
			if((pData[UIP_HEAD] == 0x00) || 
			((pData[UIP_HEAD + 1] != READ_VARIABLES) 
			&& (pData[UIP_HEAD + 1] != WRITE_VARIABLES) 
			&& (pData[UIP_HEAD + 1] != MULTIPLE_WRITE) 
			&& (pData[UIP_HEAD + 1] != CHECKONLINE)
			&& (pData[UIP_HEAD + 1] != READ_COIL)
			&& (pData[UIP_HEAD + 1] != READ_DIS_INPUT)
			&& (pData[UIP_HEAD + 1] != READ_INPUT)
			&& (pData[UIP_HEAD + 1] != WRITE_COIL)
			&& (pData[UIP_HEAD + 1] != WRITE_MULTI_COIL)
			&& (pData[UIP_HEAD + 1] != CHECKONLINE_WIHTCOM)))
			{
				return;
			}

			if((pData[UIP_HEAD + 1] == MULTIPLE_WRITE) && ((length - UIP_HEAD) != (pData[UIP_HEAD + 6] + 7)))
			{
				return;
			}	
#if ASIX_MINI

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
				if(pData[UIP_HEAD] == uart2_sub_addr[i])
				{
					Modbus.sub_port = 2;
					continue;
				}
				else if(pData[UIP_HEAD] == uart0_sub_addr[i])
				{
					 Modbus.sub_port = 0;
					 continue;
				}
				else if(pData[UIP_HEAD] == uart1_sub_addr[i])
				{	
					Modbus.sub_port = 1;
					continue;
				}
			}


			
			if(Modbus.mini_type == MINI_VAV)	
				Modbus.sub_port = 0;
#endif

		
#if ASIX_CM5
			Modbus.sub_port = 0;
#endif
			
#if TIME_SYNC	
			flag_stop_timesync = 1;
#endif
		
			flag_udp_scan = 0;
		
			if(flag_resume_rs485 == 0 || flag_resume_rs485 == 2)
			{
				vTaskSuspend(Handle_Scan); 
			}
//			vTaskSuspend(Handle_MainSerial);
			vTaskSuspend(xHandler_Output);			
			vTaskSuspend(xHandleCommon);
//			vTaskSuspend(xHandleBacnetControl);
			vTaskSuspend(xHandleMornitor_task);			
#if MSTP
			vTaskSuspend(xHandleMSTP);			
#endif 

#if ASIX_CM5			
			vTaskSuspend(Handle_SampleDI);
#endif
			
#if ASIX_MINI	
//#if (USB_HOST || USB_DEVICE)
//			if((Modbus.mini_type == MINI_BIG) || (Modbus.mini_type == MINI_SMALL))
//			 vTaskSuspend(xHandleUSB); 
//#endif
			if((Modbus.mini_type == MINI_BIG) || (Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_TINY))
			 vTaskSuspend(xHandler_SPI);
#endif

//			if(Modbus.mini_type <= MINI_BIG)
//				vTaskSuspend(xHandleLCD_task);
			
			task_test.inactive_count[0] = 0;

			TcpSocket_ME = pMODBUSTCPConn->TcpSocket;
			Set_transaction_ID(header, ((U16_T)pData[0] << 8) | pData[1], 2 * pData[UIP_HEAD + 5] + 3);
			Response_TCPIP_To_SUB(pData + UIP_HEAD,length - UIP_HEAD,Modbus.sub_port,header);
			
			flag_resume_rs485 = 1; // suspend rs485 task, resume it later, make the communication smoothly	
			resume_rs485_count = 0;
			

//			vTaskResume(Handle_MainSerial);
			vTaskResume(xHandler_Output); 
			vTaskResume(xHandleCommon);
//			vTaskResume(xHandleBacnetControl);
			vTaskResume(xHandleMornitor_task);
#if MSTP
			vTaskResume(xHandleMSTP);			
#endif 

#if ASIX_CM5			
			vTaskResume(Handle_SampleDI);
#endif

#if ASIX_MINI	

//#if (USB_HOST || USB_DEVICE)
//		if((Modbus.mini_type == MINI_BIG) || (Modbus.mini_type == MINI_SMALL))
//			vTaskResume(xHandleUSB); 
//#endif		
		if((Modbus.mini_type == MINI_BIG) || (Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_TINY))
			vTaskResume(xHandler_SPI);
#endif			
		}
	}

//  	cSemaphoreGive(sembip);

} /* End of MODBUSTCP_Receive() */







#endif