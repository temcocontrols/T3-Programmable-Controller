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
 * Module Name: MODBUSTCPd.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: MODBUSTCPd.h,v $
 * Revision 1.1.1.1  2006/06/20 05:50:28  borbin
 * no message
 *
 * Revision 1.1.1.1  2006/02/23 00:55:10  borbin
 * no message
 *
 *=============================================================================
 */

#ifndef __MODBUSTCP_H__
#define __MODBUSTCP_H__

/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */
//#define MODBUSTCP_SERVER_PORT		6001//80

#define MAX_MODBUSTCP_CONNECT		4

#define MODBUSTCP_STATE_FREE			0
#define MODBUSTCP_STATE_RESERVED		1
#define MODBUSTCP_STATE_ACTIVE		2
#define MODBUSTCP_STATE_SEND_HEADER	3
#define MODBUSTCP_STATE_SEND_DATA	4
#define MODBUSTCP_STATE_SEND_FINAL	5
#define MODBUSTCP_STATE_SEND_NONE	6

#define MODBUSTCP_CMD_UNKNOW			0
#define MODBUSTCP_CMD_GET			1
#define MODBUSTCP_CMD_POST			2

#define MODBUSTCP_POST_SUCCESS		0
#define MODBUSTCP_POST_FAILURE		1
#define MODBUSTCP_POST_CANCEL		2
#define MODBUSTCP_POST_CONTINUE		0xff

#define MAX_POST_COUNT			20

#define MAX_DIVIDE_NUM			25

#define UIP_HEAD  6
#define UIP_CODE_HEAD 3
#define READ_VARIABLES 3
#define WRITE_VARIABLES 6
//#define MULTIPLE_WRITE 0x10
#define DEEPSCAN  0x1a

#define READ_MSPT  20

//#define READ_COIL  			0X01
//#define READ_DIS_INPUT 	0X02
//#define READ_INPUT      0x04
//#define WRITE_COIL 			0X05
//#define WRITE_MULTI_COIL 0x0f 



/* TYPE DECLARATIONS */
//typedef struct _FILE_DIVIDE
//{
//	U8_T	Fragment;
//	U8_T	CurIndex;
//	U8_T	PadFlag;
//	U8_T*	PData;
//	U16_T	LeftLen;
//	U16_T	Offset[MAX_DIVIDE_NUM];
//	U8_T	RecordIndex[MAX_DIVIDE_NUM];
//	U8_T	PostType[MAX_DIVIDE_NUM];
//	U8_T	SetFlag[MAX_DIVIDE_NUM]; /* for radio and select */
//
//} FILE_DIVIDE;

/*-------------------------------------------------------------*/
typedef struct _MODBUSTCP_SERVER_CONN
{
	U32_T	Ip;
	U16_T	Port;
	U8_T	State;
	U8_T	TcpSocket;
	U16_T	Timer;
	U8_T	FileId;

//	FILE_DIVIDE	Divide;

} MODBUSTCP_SERVER_CONN;

/*-------------------------------------------------------------*/

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void MODBUSTCP_Init(void);
U8_T MODBUSTCP_NewConn(U32_T XDATA*, U16_T, U8_T);
void MODBUSTCP_Event(U8_T, U8_T);
void MODBUSTCP_Receive(U8_T XDATA*, U16_T, U8_T);

/* for debug */
void MODBUSTCP_Debug(void);


#endif /* End of __MODBUSTCPD_H__ */


/* End of MODBUSTCPd.h */
