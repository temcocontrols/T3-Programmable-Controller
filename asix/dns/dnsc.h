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
 * Module Name:dnsc.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: dnsc.h,v $
 * Revision 1.1  2006/05/22 05:44:27  robin6633
 * no message
 *
 *
 *=============================================================================
 */

#ifndef __DNSC_C_H__
#define __DNSC_C_H__

/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */
#define DNSC_ENABLE_DEBUG_MSG	1

#define DNS_SERVER_PORT		53

//---- Flag bit define -----------------
#define DNS_FLAG_QR				0x8000//Query/Response : 0/1
#define DNS_FLAG_OP_QUERY	 	0x0000//Standard query
#define DNS_FLAG_OP_IQUERY  	0x0800//Inverse query
#define DNS_FLAG_OP_STATUS   	0x1000//Server status request
#define DNS_FLAG_OP_NOIFY   	0x2000
#define DNS_FLAG_OP_UPDATE   	0x2800
#define DNS_FLAG_AA		     	0x0400//Authoritative answer
#define DNS_FLAG_TC           	0x0200//Truncation
#define DNS_FLAG_RD            	0x0100//Recursion
#define DNS_FLAG_RA            	0x0080//Recursion Aailable
 
#define DNS_FLAG_RCODE_NO_ERR  			0x0000
#define DNS_FLAG_RCODE_FORMAT_ERR		0x0001
#define DNS_FLAG_RCODE_SERVER_FAIL		0x0002
#define DNS_FLAG_RCODE_NAME_ERR    		0x0003
#define DNS_FLAG_RCODE_NOT_IMPLEMENTED	0x0004
#define DNS_FLAG_RCODE_REFUSED			0x0005
#define DNS_FLAG_RCODE					0x000F

//---- Type define ------------------
#define DNS_TYPE_A				0x0001//IP Address
#define DNS_TYPE_NS				0x0002//Name Server
#define DNS_TYPE_CNAME			0x0005//Canonical Name
#define DNS_TYPE_SOA			0x0006//Start Of Authority
#define DNS_TYPE_PTR			0x000C//Pointer
#define DNS_TYPE_MX				0x000F//Mail Exchange
#define DNS_TYPE_TXT			0x0010//Text String

//---- Class define ------------------
#define DNS_CLASS_IN			0x0001//Internet
#define DNS_CLASS_CSNET			0x0002
#define DNS_CLASS_CS			0x0003
#define DNS_CLASS_HS			0x0004

//---- Config define ------------------
#define	MAX_HOST_NAME_LEN			64
#define	MAX_DNSC_RECORDE_CNT		10//3
#define	MAX_DNSC_RETRY_CNT			2
#define	MAX_DNSC_TIMER_OUT			5
#define	MAX_DNSC_RECORDE_EXPIRE_TIME	120 //unit = sec

/* TYPE DECLARATIONS */
typedef struct _DNSC_HEADER
{
	U16_T ID;
	U16_T Flag;
    U16_T QDCount;
    U16_T ANCount;
    U16_T NSCount;
    U16_T ARCount;
} tsDNSC_HEADER;

typedef struct _DNSC_QUESTION
{
	U16_T Type;
	U16_T Class;
} teDNSC_QUESTION;

typedef struct _DNSC_ANSWER
{
	U16_T OffsetPtr;
	U16_T Type;
	U16_T Class;
	U16_T TTL[2];
	U16_T ResurLen;
	U16_T Resur[2];
} teDNSC_ANSWER;

typedef enum _DNSC_STATE
{
	DNSC_QUERY_FREE = 0,
	DNSC_QUERY_OK,
	DNSC_QUERY_WAIT,
	
	DNSC_TASK_IDLE = 0,	
	DNSC_TASK_START,
	DNSC_TASK_QUERY,	
	DNSC_TASK_WAIT_RESPONSE,
	DNSC_TASK_RESPONSE,
	DNSC_TASK_CLOSE,	
} teDNSC_STATE;

typedef struct _DNSC_TAB
{
	U8_T	HostName[MAX_HOST_NAME_LEN];
	U32_T	HostIP;
	U8_T	TimerToLive;
	U8_T	Result;
} teDNSC_TAB;

typedef struct _DNSC
{
	U16_T	WaitTimer;
	U8_T	TaskState;
	U8_T	RetryCnt;
	
	U8_T	InterAppID;
	U8_T	UdpSocket;
	U32_T	QueryIP;
	U32_T	ServerIP;
		
	teDNSC_TAB	Table[MAX_DNSC_RECORDE_CNT];
} teDNSC;


#define REMOTE_SERVER 0

extern U8_T DNSC_flag;
/* GLOBAL VARIABLES */

/* EXPORTED SUBPROGRAM SPECIFICATIONS */

void DNSC_Init(void);
void DNSC_Task();
void DNSC_SetServerIP(U32_T ip);
teDNSC_STATE DNSC_Start(U8_T *pHostName);
teDNSC_STATE DNSC_Query(U8_T *pHostName, U32_T *pHostIP);

#endif /* End of __DNSC_C_H__ */

/* End of dnsc.h */
