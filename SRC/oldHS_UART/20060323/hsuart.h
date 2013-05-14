/*
 *********************************************************************************
 *     Copyright (c) 2005   ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 *********************************************************************************
 */
/*================================================================================
 * Module Name : hsuart.h
 * Purpose     : 
 * Author      : Robin Lee
 * Date        : 
 * Notes       :
 * $Log: hsuart.h,v $
 * Revision 1.6  2005/11/07 08:18:56  robin6633
 * Fixed the non-block getkey function
 *
 * Revision 1.5  2005/11/01 13:15:10  robin6633
 * Add non-block getkey function
 *
 * Revision 1.4  2005/10/06 10:18:11  robin6633
 * Added UART2 for console debug.
 *
 * Revision 1.3  2005/09/27 09:16:13  robin6633
 * Add loopback test function.
 *
 * Revision 1.2  2005/09/22 03:06:20  robin6633
 * Add High Speed Uart(UART2) driver for putchar() and getkey().
 *
 * Revision 1.1  2005/08/17 06:48:03  robin6633
 * no message
 *
 *================================================================================
 */
#ifndef HSUART_H
#define HSUART_H

/* INCLUDE FILE DECLARATIONS */


/* NAMING CONSTANT DECLARATIONS */
#define	UR2_MAX_RX_SIZE		512
#define	UR2_MAX_TX_SIZE		512

#define	UR2_SLEEP			0
#define	UR2_HFDPX			BIT6
#define	UR2_SLAVE			BIT7
#define	UR2_MASTER			BIT6|BIT7


/* MACRO DECLARATIONS */


/* TYPE DECLARATIONS */


/* GLOBAL VARIABLES */


/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void	HSUR_Func(void);
void	HSUR_Setup(U16_T divisor, U8_T lCtrl, U8_T intEnb, U8_T fCtrl, U8_T mCtrl);
S8_T	HSUR_GetChar(void);
S8_T	HSUR_PutChar(S8_T ch);
void	HSUR_InitValue(void);
S8_T	HSUR_GetCharNb(void);



#endif /* End of HSUART_H */
