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
 /*============================================================================
 * Module Name: dnsc.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: dnsc.c,v $
 * Revision 1.5  2006/05/23 11:04:18  robin6633
 * 1.Add a condition for printd.
 *
 * Revision 1.4  2006/05/23 05:21:53  robin6633
 * 1.Add define condition.
 *
 * Revision 1.3  2006/05/23 01:53:31  robin6633
 * 1.Removed debug information from UART2.
 *
 * Revision 1.2  2006/05/22 13:22:33  robin6633
 * 1.Initial the dns server ip from data base.
 *
 * Revision 1.1  2006/05/22 05:44:27  robin6633
 * no message
 *
 *
 *=============================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include "main.h"


#if (INCLUDE_DNS_CLIENT)
#include "dnsc.h"
#include "tcpip.h"
#include "mstimer.h"
#include "uart.h"
//#include "printd.h"
#include <stdio.h>
#include <string.h>
#include "gconfig.h"
/* NAMING CONSTANT DECLARATIONS */

/* GLOBAL VARIABLES DECLARATIONS */

/* LOCAL VARIABLES DECLARATIONS */
static teDNSC DNSC;
/* LOCAL SUBPROGRAM DECLARATIONS */
static void DNSC_Event(U8_T, U8_T);
static void DNSC_Receive(U8_T XDATA*, U16_T, U8_T);
static U8_T DNSC_Send(U8_T InterUdpId, char *pName);
static U16_T DNSC_PrepareQueryPacket(S8_T *pName, U8_T *pBuf);
static U32_T DNSC_ParseResponsePacket(U8_T *pBuf, U16_T bufLen);

U8_T dnsc_Index;
U8_T DNSC_flag;
/*
 * ----------------------------------------------------------------------------
 * Function Name: DNSC_Init()
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void DNSC_Init(void)
{
	memset((U8_T*)&DNSC, 0 , sizeof(DNSC));
	DNSC.InterAppID = TCPIP_Bind(NULL, DNSC_Event, DNSC_Receive);
	DNSC_SetServerIP(0x08080808/*GCONFIG_GetDNS()*/);		
//	DNSC_SetServerIP((U32_T)(Modbus.getway[0] << 24) + (U32_T)(Modbus.getway[1] << 16) \
//	+ (U16_T)(Modbus.getway[2] << 8) + Modbus.getway[3]);	
//	printd("DNS Initial OK!\n\r");
	dnsc_Index = 0;
	
} /* End of DNSC_Init() */



/*
 * ----------------------------------------------------------------------------
 * Function Name: DNSC_Event
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void DNSC_Event(U8_T id, U8_T event)
{
	if (id) return;

	if (event == TCPIP_CONNECT_CANCEL)
	{
		DNSC.TaskState = DNSC_TASK_CLOSE;
		TCPIP_UdpClose(DNSC.UdpSocket);
	}

} /* End of DNSC_Event() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: DNSC_Receive
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void DNSC_Receive(U8_T XDATA* pData, U16_T length, U8_T id)
{
	if ((DNSC.TaskState != DNSC_TASK_WAIT_RESPONSE) && id)
		return;
	DNSC.QueryIP = DNSC_ParseResponsePacket(pData, length);
	DNSC.TaskState = DNSC_TASK_RESPONSE;
    TCPIP_UdpClose(DNSC.UdpSocket);
} /* End of DNSC_Receive() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: DNSC_Send
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T DNSC_Send(U8_T InterUdpId, S8_T *pName)
{
	U8_T pkt[320];
	U16_T len;

	if(strlen(pName) <= 255)
	{	
		len = DNSC_PrepareQueryPacket(pName, &pkt[0]);
		TCPIP_UdpSend(InterUdpId, 0, 0, pkt, len);
		return 0;
	}
	return 1;
} /* End of DNSC_Send() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: DNSC_PrepareQueryPacket
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U16_T DNSC_PrepareQueryPacket(S8_T *pName, U8_T *pBuf)
{
	U8_T DotOffset, DataOffset, DataCnt, NameLen = strlen(pName);
	tsDNSC_HEADER *pDNSC_HD;
	teDNSC_QUESTION *pDNSC_Q;

	// Prepare DNS header
	pDNSC_HD = (tsDNSC_HEADER*)pBuf;
	pDNSC_HD->ID = htons(0xABCD);
	pDNSC_HD->Flag = DNS_FLAG_RD;
	pDNSC_HD->QDCount = htons(0x0001);
	pDNSC_HD->ANCount = htons(0x0000);
	pDNSC_HD->NSCount = htons(0x0000);
	pDNSC_HD->ARCount = htons(0x0000);
	pBuf = pBuf + sizeof(tsDNSC_HEADER);

	// Prepare Question field
	DotOffset = 0;
	DataCnt = 0;
	for (DataOffset = 0 ; DataOffset < NameLen ; DataOffset ++)//Standard DNS name notation.
	{
		if (pName[DataOffset] == '.')
		{
			pBuf[DotOffset] = DataCnt;
			DataCnt = 0;			
			DotOffset = DataOffset + 1;
		}
		else
		{
			pBuf[DataOffset + 1] = pName[DataOffset];
			DataCnt ++;
	}
	}
	pBuf[DotOffset] = DataCnt;	
	pBuf[DataOffset + 1] = 0;

	pDNSC_Q = (teDNSC_QUESTION*)&(pBuf[DataOffset + 2]);
	pDNSC_Q->Type = htons(DNS_TYPE_A);
	pDNSC_Q->Class = htons(DNS_CLASS_IN);

	return (NameLen + 2 + sizeof(tsDNSC_HEADER) + 4);
} /* End of dnsc_PrepareQueryPacket() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: DNSC_ParseResponsePacket
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U32_T DNSC_ParseResponsePacket(U8_T *pBuf, U16_T bufLen)
{	
	U16_T len;
	U16_T i;
	U32_T ip = 0x00000000;
	tsDNSC_HEADER *pDNSC_HD;
	teDNSC_ANSWER *pDNSC_A;	

	if (bufLen < sizeof(tsDNSC_HEADER))
	{
		return 0;
	}

	pDNSC_HD = (tsDNSC_HEADER*)pBuf;
	len = sizeof(tsDNSC_HEADER);
	
	if (((pDNSC_HD->Flag) & DNS_FLAG_RCODE) == 0) // No error
	{
		if (pDNSC_HD->QDCount > 0)
		{
			for (i = 0; i < pDNSC_HD->QDCount; i++)
			{
				// Handle QDCOUNT field at here

				while (*(pBuf + len++) != 0)
				{
					// Handle QNAME field at here
					// ...					
				}
			
				// Handle QTYPE and QCLASS fields at here
				// ...
				len += sizeof(teDNSC_QUESTION);
			}
		}

		if (pDNSC_HD->ANCount > 0)
		{
			for (i = 0; i < pDNSC_HD->ANCount; i++)
			{
				// Handle ANCOUNT field at here

				// Currently only handles first valid answer
				pDNSC_A = (teDNSC_ANSWER*)(pBuf + len);
				if ((pDNSC_A->Type == DNS_TYPE_A) && (pDNSC_A->Class == DNS_CLASS_IN))
				{						
					if (pDNSC_A->ResurLen == 4)
					{									
						ip = ((((U32_T)pDNSC_A->Resur[0] << 16) & 0xFFFF0000) + 
						          ((U32_T)pDNSC_A->Resur[1] & 0x0000FFFF));
					}
				}
				else
				{
					// Shift to offset of next answer
					// Header length (Bytes)
					// Name:2  Type:2  Class:2  TTL:4  Data Len:2  Data:variable     
					len += 10; // Length of Name + Type + Class + TTL
					len += ((U16_T)((*(pBuf + len) << 8) & 0xFF00) + (U16_T)*(pBuf + len + 1) + 2);						
				}
			}
		}

		if (pDNSC_HD->ANCount > 0)
		{
			// Handle NSCOUNT field at here
			// ...
		}

		if (pDNSC_HD->ANCount > 0)
		{
			// Handle ARCOUNT field at here
			// ...
		}
	}
	
	return ip;
	
} /* End of DNSC_ParseResponsePacket() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: DNSC_SetServerIP
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void DNSC_SetServerIP(U32_T ip)
{
	DNSC.ServerIP = ip;	
} /* End of DNSC_SetServerIP() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: DNSC_Timer()
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void DNSC_Timer()
{
 	static U32_T TimerStop = 0, TimerStart = 0, TimerElapse;
	U8_T i;
	
	TimerStop = SWTIMER_Tick();

	if (TimerStop >= TimerStart)
		TimerElapse = TimerStop - TimerStart;
	else
		TimerElapse = TimerStop + (0xFFFFFFFF - TimerStart);
							
	if (TimerElapse > SWTIMER_COUNT_SECOND)
	{
		TimerStart = TimerStop;	
		
		if (DNSC.WaitTimer) 
			DNSC.WaitTimer --;
			
		for (i = 0; i< MAX_DNSC_RECORDE_CNT; i ++)	//Add for recorder expire function.
		{
			if (DNSC.Table[i].TimerToLive)
			{
				DNSC.Table[i].TimerToLive --;
			}
		}
	}
}  /* End of DNSC_Timer() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: teDNSC_STATE DNSC_Start()
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
teDNSC_STATE DNSC_Start(U8_T *pHostName)
{
	U8_T Index, timeTemp, i;
// add by chelsea
//	for (Index = 0 ; Index < MAX_DNSC_RECORDE_CNT ; Index ++)//Set the host name to query
//	{
//		if (DNSC.Table[Index].Result == DNSC_QUERY_FREE)
//			break;
//	}
	
	for (Index = 0 ; Index < MAX_DNSC_RECORDE_CNT ; Index ++)//Set the host name to query
	{
		if (DNSC.Table[Index].Result == DNSC_QUERY_FREE)
			break;
	}
	if (Index != MAX_DNSC_RECORDE_CNT)
	{
		strcpy(DNSC.Table[Index].HostName, pHostName);
		DNSC.Table[Index].Result = DNSC_QUERY_WAIT;
		return DNSC_QUERY_WAIT;
	}

	timeTemp = 0xff;	//Add for recorder expire function.
	i = 0;
	for (Index = 0; Index < MAX_DNSC_RECORDE_CNT; Index ++)
	{
		if ((DNSC.Table[Index].TimerToLive < timeTemp) && (DNSC.Table[Index].Result == DNSC_QUERY_OK))
		{
			timeTemp = DNSC.Table[Index].TimerToLive;
			i = Index;
		}
	}
	strcpy(DNSC.Table[i].HostName, pHostName);
	DNSC.Table[i].Result = DNSC_QUERY_WAIT;
	
	return DNSC_QUERY_WAIT;
}  /* End of DNSC_QueryStart() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: teDNSC_STATE DNSC_Query()
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
teDNSC_STATE DNSC_Query(U8_T *pHostName, U32_T *pHostIP)
{
	U8_T Index;
	
	// add by chelsea
//	for (Index = 0 ; Index < MAX_DNSC_RECORDE_CNT ; Index ++)//Find host IP in record table.
//	{
//		if(!strcmp(pHostName, DNSC.Table[Index].HostName))
//			break;
//	}
//	if(Index == MAX_DNSC_RECORDE_CNT)
//	{
//		DNSC_Start(pHostName);
////		return DNSC_QUERY_WAIT;
//	}
	
	
	for (Index = 0 ; Index < MAX_DNSC_RECORDE_CNT ; Index ++)//Find host IP in record table.
	{
		if ((!strcmp(pHostName, DNSC.Table[Index].HostName)) && (DNSC.Table[Index].Result == DNSC_QUERY_OK))
			break;
	}
	
	if (Index < MAX_DNSC_RECORDE_CNT)
	{
		*pHostIP = DNSC.Table[Index].HostIP;
		return DNSC_QUERY_OK;
	}

	for (Index = 0 ; Index < MAX_DNSC_RECORDE_CNT ; Index ++)//Check this host IP query.
	{
		if ((!strcmp(pHostName, DNSC.Table[Index].HostName)) && (DNSC.Table[Index].Result == DNSC_QUERY_WAIT))
			break;
	}
	
	if (Index != MAX_DNSC_RECORDE_CNT)
	{
		return DNSC_QUERY_WAIT;
	}	
	return DNSC_QUERY_FREE;
} /* End of DNSC_Query() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: void DNSC_Task()
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */


void DNSC_Task()
{
//	static U8_T dnsc_Index;
	DNSC_Timer();

	if ((DNSC.WaitTimer != 0) && ((DNSC.TaskState != DNSC_TASK_WAIT_RESPONSE) || (DNSC.TaskState != DNSC_TASK_CLOSE)))//Check excution condition
	{
		return;
	}
	switch (DNSC.TaskState)
	{
	case DNSC_TASK_IDLE://Function start or initial.
		DNSC.TaskState = DNSC_TASK_START;
		break;		
	case DNSC_TASK_START://Find the query request
		for (/*dnsc_Index = 0 */; dnsc_Index < MAX_DNSC_RECORDE_CNT ; dnsc_Index ++)
		{		
			if(DNSC.Table[dnsc_Index].Result == DNSC_QUERY_WAIT)
				break;
		}
		if (dnsc_Index != MAX_DNSC_RECORDE_CNT)
		{
			DNSC.RetryCnt = MAX_DNSC_RETRY_CNT;
			DNSC.TaskState = DNSC_TASK_QUERY;
		}
		
		break;		
	case DNSC_TASK_QUERY://Start query
		if ((DNSC.UdpSocket = TCPIP_UdpNew(DNSC.InterAppID, 0, DNSC.ServerIP, 0, DNS_SERVER_PORT)) == TCPIP_NO_NEW_CONN)
		{			
			DNSC.Table[dnsc_Index].Result = DNSC_QUERY_FREE;
			DNSC.TaskState = DNSC_TASK_START;
		}
		else
		{
			if (DNSC_Send(DNSC.UdpSocket, DNSC.Table[dnsc_Index].HostName))
			{
				DNSC.TaskState = DNSC_TASK_CLOSE;		
				TCPIP_UdpClose(DNSC.UdpSocket);
			}
			else
			{	
				DNSC.WaitTimer = MAX_DNSC_TIMER_OUT;//Set time out (unit = sec)
				DNSC.TaskState = DNSC_TASK_WAIT_RESPONSE;
			}
    }
		break;
		
	case DNSC_TASK_WAIT_RESPONSE://Wait response or timeout
		if (DNSC.WaitTimer == 0)
		{
			DNSC.TaskState = DNSC_TASK_CLOSE;
			TCPIP_UdpClose(DNSC.UdpSocket);
		}	
		break;
		
	case DNSC_TASK_RESPONSE://Receive response packet OK

		if (DNSC.QueryIP == 0)
		{
			DNSC.TaskState = DNSC_TASK_CLOSE;	
		}			
		else
		{
			DNSC.Table[dnsc_Index].HostIP = DNSC.QueryIP;			
			DNSC.Table[dnsc_Index].Result = DNSC_QUERY_OK;			
			DNSC.Table[dnsc_Index].TimerToLive = MAX_DNSC_RECORDE_EXPIRE_TIME;	//Add for recorder expire function.
			DNSC.TaskState = DNSC_TASK_START;
			
//			Test[31]++;
//			Test[32] = (U8_T)(DNSC.QueryIP >> 24);
//			Test[33] = (U8_T)(DNSC.QueryIP >> 16);
//			Test[34] = (U8_T)(DNSC.QueryIP >> 8);
//			Test[35] = (U8_T)(DNSC.QueryIP);
#if REM_CONNECTION			
			if(DNSC_flag == REMOTE_SERVER)
			{
				RM_Conns.ServerIp = DNSC.QueryIP;

				E2prom_Write_Byte(EEP_REMOTE_SERVER1,(U8_T)(DNSC.QueryIP >> 24));
				E2prom_Write_Byte(EEP_REMOTE_SERVER2,(U8_T)(DNSC.QueryIP >> 16));
				E2prom_Write_Byte(EEP_REMOTE_SERVER3,(U8_T)(DNSC.QueryIP >> 8));
				E2prom_Write_Byte(EEP_REMOTE_SERVER4,(U8_T)(DNSC.QueryIP));
			}
#endif
		}
		break;
			
	case DNSC_TASK_CLOSE://Retry again
		if (DNSC.RetryCnt == 0)
		{
			DNSC.Table[dnsc_Index].Result = DNSC_QUERY_FREE;
			DNSC.TaskState = DNSC_TASK_START;					
		}
		else
		{
			DNSC.RetryCnt --;		
			DNSC.TaskState = DNSC_TASK_QUERY;
		}
		DNSC.WaitTimer = 0;
		break;
		
	default:
		DNSC.WaitTimer = 0;	
		DNSC.TaskState = DNSC_TASK_IDLE;
		break;	
	};
} /* End of DNSC_Task() */

#endif /* INCLUDE_DNS_CLIENT */

/* End of dnsc.c */

