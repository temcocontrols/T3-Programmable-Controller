/*
 *********************************************************************************
 *     Copyright (c) 2005	ASIX Electronic Corporation      All rights reserved.
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
 * $Log: smtpc.h,v $
 * Revision 1.2  2006/07/25 05:36:23  borbin
 * no message
 *
 * Revision 1.1.1.1  2006/06/20 05:50:28  borbin
 * no message
 *
 *
 *=============================================================================
 */

#ifndef __SMTPC_H__
#define __SMTPC_H__


/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */
#define SMTP_SERVER_PORT		25
#define SMTP_MAX_LENGTH			100//76

/*-------------------------------------------------------------*/
/* NAMING CONSTANT DECLARATIONS */
#define SMTP_STATE_NONE			0
#define SMTP_STATE_INITIAL		1
#define SMTP_STATE_CONNECTED	2  
#define SMTP_STATE_HELO_SENT	3
#define SMTP_STATE_FROM_SENT	4
#define SMTP_STATE_RCV1_SENT	5
#define SMTP_STATE_RCV2_SENT	6
#define SMTP_STATE_RCV3_SENT	7
#define SMTP_STATE_DATA_SENT	8
#define SMTP_STATE_WAIT_MESSAGE	9
#define SMTP_STATE_SEND_MESSAGE	10
#define SMTP_STATE_MESSAGE_SENT	11
#define SMTP_STATE_QUIT_SENT	12

#define SMTP_STATE_AUTH_SENT	13
#define SMTP_STATE_USERPASS1_SENT	14
#define SMTP_STATE_USERPASS2_SENT	15

#define SMTP_STATE_HELO_SENT1  16

/* TYPE DECLARATIONS */
typedef struct _SMTPC_CONN
{
	U8_T	State;
	U8_T	TcpSocket;
	U32_T	ServerIp;
	U8_T	From[50];
	U8_T	To1[50];
	U8_T	To2[50];
	U8_T	To3[50];

} SMTPC_CONN;

/* GLOBAL VARIABLES */
extern SMTPC_CONN  smtpc_Conns;

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void SMTPC_Init(U8_T *pBuf, U8_T *pSubject);
void SMTPC_Event(U8_T, U8_T);
void SMTPC_Receive(U8_T *, U16_T);
void SMTPC_Start(U32_T, U8_T*, U8_T*, U8_T*, U8_T*);
void SMTPC_SendMessage(U8_T*, U16_T);
void TcpDebug_SendMessage(U8_T*, U16_T);
//void tcp_printf(const char *fmt,...)  ;
U8_T SMTPC_GetState(void);

void SMTPC_appcall(void);
//void EthernetDebug_appcall(void);
void Cmime64_Init(void);
void cmime64(S8_T*);

#endif /* __SMTPC_H__ */

