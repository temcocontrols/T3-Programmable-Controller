/*
*********************************************************************************
*     Copyright (c) 2006   ASIX Electronic Corporation      All rights reserved.
*
*     This is unpublished proprietary source code of ASIX Electronic Corporation
*
*     The copyright notice above does not evidence any actual or intended
*
*     publication of such source code.
*********************************************************************************


/* INCLUDE FILE DECLARATIONS */

#include "main.h"
#include 	"bac_client.h"

#if ASIX
#include "adapter.h"
#include "uart.h"
#include "mstimer.h"
#endif

#if ARM
#include "uip.h"
#endif

#include "tcpip.h"
#include <stdio.h>


static BAC_CLIENT_CONN Bac_Client_Conns;
static U8_T Bac_Client_InterAppId;




#if ARM
static struct uip_udp_conn *Bac_Client_Conns = NULL;
//U8_T flag_Bac_Client_Send;
#endif

/* LOCAL SUBPROGRAM DECLARATIONS */


/*
 * ----------------------------------------------------------------------------
 * Function Name: SNTPC_Init
 * Purpose: to initial the SNTP client connection information.
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */

void Bac_Client_Init(void)
{	
	U8_T i;

	Bac_Client_Conns.State = BAC_CLIENT_STATE_INITIAL;
	
#if ASIX
	Bac_Client_InterAppId = TCPIP_Bind(NULL, Bac_Client_Event, Bac_Client_Receive);

#endif

#if ARM

#endif

} /* End of SNTPC_Init() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: SNTPC_Event
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
#if ASIX
void Bac_Client_Event(U8_T id, U8_T event)
{
	if (id != 0)
		return;

	if (event == TCPIP_CONNECT_CANCEL)
	{
		TCPIP_UdpClose(Bac_Client_Conns.UdpSocket);
		Bac_Client_Conns.State = BAC_CLIENT_STATE_INITIAL;
	}

} /* End of SNTPC_Event() */
#endif
/*
 * ----------------------------------------------------------------------------
 * Function Name: SNTPC_Receive
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void Bac_Client_Receive(U8_T XDATA* pData, U16_T length, U8_T id)
{

	if (id != 0)
		return;

	// TBD:
	
	
	
#if ASIX
	TCPIP_UdpClose(Bac_Client_Conns.UdpSocket);
#endif
	
#if ARM
	//sntp_conn = NULL;
	uip_udp_remove(sntp_conn);
#endif
	Bac_Client_Conns.State = SNTP_STATE_GET_DONE;
	
	

} /* End of SNTPC_Receive() */






/*
 * ----------------------------------------------------------------------------
 * Function Name: Bac_Client_Send
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void Bac_Client_Send_Whois(U8_T InterUdpId)
{
	U8_T len = 48;
	U8_T i;
	U8_T far Buf[48];
#if 1
	Buf[0] = 0x0b;
	Buf[1] = 0x00;
	Buf[2] = 0x04;
	Buf[3] = 0xfa;
	Buf[4] = 0x00;
	Buf[5] = 0x01;
	Buf[6] = 0x00;
	Buf[7] = 0x00;
	Buf[8] = 0x00;
	Buf[9] = 0x01;
	for(i = 10;i < len;i++)
	{
		Buf[i] = 0;
	}
#endif
	
	

#if ASIX
	TCPIP_UdpSend(InterUdpId, 0, 0, Buf, len);
#endif
	
#if ARM	
	uip_send(Buf, len);
  	
	Test[12]++;
#endif

} /* End of Bac_Client_Send() */


U8_T Bac_Client_Start(U32_T IP)
{
	U8_T buf[48];
#if ASIX
	//static u32 ip_backup;
	Bac_Client_Conns.ServerIp = 0XFFFFFFFF;
	Test[11]++;
  
	/* Create SNTP client port */

	if(Bac_Client_Conns.UdpSocket != NULL) 
		TCPIP_UdpClose(Bac_Client_Conns.UdpSocket);
	
	if((Bac_Client_Conns.UdpSocket = TCPIP_UdpNew(Bac_Client_InterAppId, 0, Bac_Client_Conns.ServerIp,
		0, 0xBAC0)) == TCPIP_NO_NEW_CONN)
	{
		Test[12]++;
		return BAC_CLIENT_STATE_NOTREADY;
	}
	
		
	Bac_Client_Send_Whois(Bac_Client_Conns.UdpSocket);
//	printd("In NTP_Start End\r\n");
	return BAC_CLIENT_STATE_WAIT;
#endif


#if ARM
	uip_ipaddr_t addr;
		
	if(sntp_conn != NULL) 
		uip_udp_remove(Bac_Client_Conns);
			
	uip_ipaddr(addr, (U8_T)(timesrIP >> 24), (U8_T)(timesrIP >> 16), (U8_T)(timesrIP >> 8), (U8_T)(timesrIP));	
	Bac_Client_Conns = uip_udp_new(&addr, HTONS(0xBAC0));
	if(Bac_Client_Conns != NULL) {	
		Test[14]++;
	}	
	flag_Bac_Client_Send = 1;
#endif	
} /* End of SNTPC_Start() */




U8_T Bac_Client_Send_Info(U32_T IP)
{

#if ASIX
	//static u32 ip_backup;
	Bac_Client_Conns.ServerIp = 0XFFFFFFFF;
	Test[11]++;
  
	/* Create SNTP client port */

	if(Bac_Client_Conns.UdpSocket != NULL) 
		TCPIP_UdpClose(Bac_Client_Conns.UdpSocket);
	
	if((Bac_Client_Conns.UdpSocket = TCPIP_UdpNew(Bac_Client_InterAppId, 0, Bac_Client_Conns.ServerIp,
		0, 0xBAC0)) == TCPIP_NO_NEW_CONN)
	{
		Test[12]++;
		return BAC_CLIENT_STATE_NOTREADY;
	}

	


//	printd("In NTP_Start End\r\n");
	return BAC_CLIENT_STATE_WAIT;
#endif


#if ARM
	uip_ipaddr_t addr;
		
	if(sntp_conn != NULL) 
		uip_udp_remove(Bac_Client_Conns);
			
	uip_ipaddr(addr, (U8_T)(timesrIP >> 24), (U8_T)(timesrIP >> 16), (U8_T)(timesrIP >> 8), (U8_T)(timesrIP));	
	Bac_Client_Conns = uip_udp_new(&addr, HTONS(0xBAC0));
	if(Bac_Client_Conns != NULL) {	
		Test[14]++;
	}	
	flag_Bac_Client_Send = 1;
#endif	
} 
/*
 * ----------------------------------------------------------------------------
 * Function Name: SNTPC_Stop
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */

void Bac_Client_Stop(void)
{
#if ASIX
	if(Bac_Client_Conns.State != SNTP_STATE_INITIAL)
	{
		TCPIP_UdpClose(Bac_Client_Conns.UdpSocket);
		Bac_Client_Conns.State = SNTP_STATE_INITIAL;
	}
#endif
	
#if ARM
	uip_udp_remove(Bac_Client_Conns);
#endif

} /* End of SNTPC_Stop() */


#if ARM

void sntp_appcall(void)
{
//	Test[34]++;
	if(uip_poll()) 
	{	
		// auto send
		if(flag_Bac_Client_Send)
		{
			flag_Bac_Client_Send = 0;
//			Test[11]++;			
			Bac_Client_Send(0);
		}
	}
	
	if(uip_newdata()) 
	{
		Test[16]++;
// deal with receiving data
		SNTPC_Receive(uip_appdata, uip_len, 0);	
	}
}

#endif


/* End of Bac_Client.c */
