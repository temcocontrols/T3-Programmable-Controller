 /*
******************************************************************************
 *     Copyright (c) 2005	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ******************************************************************************
 */
/*================================================================================
 * Module Name : uart.h
 * Purpose     : A header file of uart.c includes UART0 and UART1 module.
 * Author      : Robin Lee
 * Date        : 2006-01-10
 * Notes       :
 * $Log: uart.h,v $
 * Revision 1.1  2006/05/12 14:24:47  robin6633
 * no message
 *
 * Revision 1.3  2006/05/03 02:46:00  robin6633
 * Changed the the function name UART_GetBufCount()
 * to UART_GetRxBufCount() .
 *
 * Revision 1.2  2006/05/02 01:43:45  robin6633
 * Add an expanding function to get the counter value of UART software buffer.
 *
 * Revision 1.1  2006/04/07 11:39:14  robin6633
 * no message
 *
 *================================================================================
 */

#ifndef __UART_H__
#define __UART_H__

#include	"uart_cfg.h"


/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void	UART_Init(unsigned char port);

#endif /* End of __UART_H__ */

/* End of uart.h */