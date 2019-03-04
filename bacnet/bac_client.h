#ifndef __BAC_CLIENT_H__
#define __BAC_CLIENT_H__


/* INCLUDE FILE DECLARATIONS */
#include "types.h"

typedef struct _BAC_CLIENT_CONN
{
	U8_T	State;
	U8_T	UdpSocket;
	U32_T	ServerIp;

} BAC_CLIENT_CONN;


#define BAC_CLIENT_STATE_NOTREADY		0
#define BAC_CLIENT_STATE_INITIAL		1  
#define BAC_CLIENT_STATE_WAIT			2
#define BAC_CLIENT_STATE_TIMEOUT		3
#define BAC_CLIENT_STATE_GET_DONE		4


void Bac_Client_Event(U8_T id, U8_T event);
void Bac_Client_Receive(U8_T XDATA* pData, U16_T length, U8_T id);
U8_T Bac_Client_Start(U32_T IP);
U8_T Bac_Client_Send_Whois(U32_T IP);
void Bac_Client_Send_Whois(U8_T InterUdpId);

#endif