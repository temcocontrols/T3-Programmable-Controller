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
 * Module Name:gudpbc.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: gudpbc.h,v $
 *
 *=============================================================================
 */

#ifndef __GUDPBC_H__
#define __GUDPBC_H__

/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */

/* TYPE DECLARATIONS */
typedef struct _GUDPBC_CONN
{
	U8_T 	State;
	U8_T	UdpSocket;

} GUDPBC_CONN;


typedef struct 
{
	U16_T cmd;   // low byte first
	U16_T len;   // low byte first
	U16_T own_sn[4]; // low byte first
	U16_T product;   // low byte first
	U16_T address;   // low byte first
	U16_T ipaddr[4]; // low byte first
	U16_T modbus_port; // low byte first
	U16_T firmwarerev; // low byte first
	U16_T hardwarerev;  // 28 29	// low byte first
	
	U8_T master_sn[4];  // master's SN 30 31 32 33
	U16_T instance_low; // hight byte first
	U8_T panel_number; //  36	
	S8_T panelname[20]; // 37 - 56
	U16_T instance_hi; // hight byte first
	
	U8_T bootloader;  // 0 - app, 1 - bootloader, 2 - wrong bootloader
	U16_T BAC_port;  //  hight byte first
	U8_T zigbee_exist; // 0 - inexsit, 1 - exist
	
	U8_T subnet_protocal; // 0 - modbus, 12 - bip to mstp
}STR_SCAN_CMD;


extern STR_SCAN_CMD far Scan_Infor;
/* GLOBAL VARIABLES */

/* EXPORTED SUBPROGRAM SPECIFICATIONS */ 
void GUDPBC_Task(void);
void GUDPBC_Init(U16_T);
U8_T GUDPBC_NewConn(U32_T XDATA*, U16_T, U8_T);
void GUDPBC_Event(U8_T, U8_T);
void GUDPBC_Receive(U8_T XDATA*, U16_T, U8_T);
void I2C_Init(void);
#endif /* End of __GUDPBC_H__ */

/* End of gudpbc.h */
