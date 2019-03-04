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
/*================================================================================
 * Module Name : flash.h
 * Purpose     : A header file of flash.c
 * Author      :
 * Date        :
 * Notes       :
 *
 /*********************************************************************************/
// Module Name : flash.h
// Purpose     :
// Author      : Chelsea
// Date        : 2008/11/10
// Revision    : rev0
/*********************************************************************************/
#ifndef __FLASH_H__
#define __FLASH_H__

/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */
#define	SOH_128				0x01
#define	SOH_1024			0x02
#define	EOT					0x04
#define	ACK					0x06
#define	NACK				0x15
#define	CAN					0x18
#define	ASCII_C				0x43

#define	X_LEN_128			128
#define	X_LEN_1024			1024
#define	MAX_RETRY			6

#define	STATE_SOH			BIT0
#define	STATE_BLKNUM		BIT1
#define	STATE_BLKNUM_CMP	BIT2
#define	STATE_DATA			BIT3
#define	STATE_ACK			BIT4
#define	STATE_EOT_ACK		BIT5

#define	DUMP_DATA_MEMORY	1
#define	DUMP_FLASH			2
#define	DUMP_SHADOW			3

#define	ERA_BLD				BIT0
#define	ERA_CFG				BIT1
#define	ERA_RUN				BIT2

//  use the last block for store datas and configure file
#define FLASH_BLOCK			0x70000   //  default using the last page



#define MAX_CODE_SIZE   0x27e0 // 0xffff - 0xd810  = 0x27f0  = 10224 byte
#define BASE_CODE			0x7D810  // code space = 0xffff - 0xd810 = 0x27f0 
#define BASE_CODE_LEN		0x7D7F0 - 2
#define BASE_CODE_INDEX 	0x7D7F0	  // MAX CODE INDEX IS 32, so MAX_PRGS must be less than 32

#define BASE_GSM_APN		0x7D700  // 254 byte for gsm
#define BASE_GSM_IP			0x7D700 + 100 // 254 byte for gsm

#define BASE_PANEL_NAME     0x7D700 - 20


#define BASE_DYNDNS_DONAME	0x7D680
#define BASE_DYNDNS_USER		0x7D6A0
#define BASE_DYNDNS_PASS		0x7D6C0

#define BASE_TST_NAME		0x7C680  // length is 1000H

#define BASE_MON_OPERATE_TIME 0x7c630 // length is 80, the lenght is at least 48

#define BASE_WEATHER  0x7c570  // lenth is 0xc0, 192
#define BASE_SNTP_SERVER 0x7c550 // length is 30
#define BASE_VAR_UINT			0x7c4ec // length is 100

#define BASE_WR_ON_OFF	0x7c2ac		//	length is 576


/* GLOBAL VARIABLES */
extern U8_T CODE* PFlash;

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
extern S16_T	IntFlashWrite(U32_T ProgAddr, U8_T *ptWrData, U32_T ProgLen, U8_T BootldrSel);
extern S16_T	IntFlashErase(U8_T EraseSel,U32_T EraAddr);
U8_T 	IntFlashReadByte(U32_T location, U8_T *value);
U8_T    IntFlashReadByte_1(U32_T location, U8_T *value);
U8_T 	IntFlashReadInt(U32_T location, U16_T *value);
U8_T 	IntFlashWriteByte(U32_T location,U8_T value);
U8_T 	IntFlashWriteInt(U32_T location,U16_T value);
void 	MassFlashWrite(U32_T location,U8_T *value,U32_T lenght);
void ISPMassFlashWrite(U32_T location,U8_T *value,U32_T lenght);

#endif /* End of __FLASH_H__ */
