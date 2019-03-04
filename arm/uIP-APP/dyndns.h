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
 * Module Name:dyndns.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 *=============================================================================
 */

#ifndef __DYNDNS_H__
#define __DYNDNS_H__

/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* TYPE DECLARATIONS */

/* GLOBAL VARIABLES */

#define DYNDNS_STATE_IDLE				0
#define DYNDNS_STATE_MAKE_CONNECT		1
#define DYNDNS_STATE_WAIT_CONNECT		2
#define DYNDNS_STATE_SEND_COMMAND		3
#define DYNDNS_STATE_WAIT_RESPONSE		4
#define DYNDNS_STATE_UPDATE_OK		5  // added by chelsea

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void DynDNS_Init(void);
void DynDNS_DoUpdateDynDns(U8_T Type, U32_T HostIp, U8_T *Domain, U8_T *User, U8_T *Passwd);
void DynDNS_EventHandle(U8_T connID, U8_T event);
void DynDNS_ReceiveHandle(U8_T XDATA *pData, U16_T length, U8_T connId);
BOOL DynDNS_GetUpdateState(void);
U8_T DynDNS_GetState(void);
#endif /* End of __DYNDNS_H__ */



