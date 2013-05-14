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
/*================================================================================
 * Module Name : uart.c
 * Purpose     : The UART module driver. It manages the character
 *               buffer and handles the ISR. This driver includes UART0 and UART1.
 * Author      : Robin Lee
 * Date        : 2006-01-10
 * Notes       : None.
 * $Log: uart.c,v $
 * Revision 1.3  2006/05/18 02:15:34  robin6633
 * 1.Add UART1 disable definition.
 *
 * Revision 1.2  2006/05/17 08:31:32  robin6633
 * 1. Changed the default baudrate to 115200.
 *
 * Revision 1.1  2006/05/12 14:24:47  robin6633
 * no message
 *
 * Revision 1.3  2006/05/03 02:45:55  robin6633
 * Changed the the function name UART_GetBufCount()
 * to UART_GetRxBufCount() .
 *
 * Revision 1.2  2006/05/02 01:43:40  robin6633
 * Add an expanding function to get the counter value of UART software buffer.
 *
 * Revision 1.1  2006/04/07 11:39:14  robin6633
 * no message
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include "../cpu/reg80390.h"
#include "uart.h"

static void uart0_Init(void)
{
	U8_T sysClk = 0;
	// Initialize TIMER1 for standard 8051 UART clock
//	PCON |= 0x80;			// Disable BaudRate doubler.
	SM01 = 1;			// Use serial port 0 in mode 1 with 8-bits data.
	REN0 = 1;			// Enable UART0 receiver.
//	TMOD |= 0x20;		// Use timer 1 in mode 2, 8-bit counter with auto-reload.

	TL2 = 0xD7;
	TH2 = 0xFF;
	RLDL = 0xD7;
	RLDH = 0xFF;

	T2CON = (1 << 2) | (1 << 4) | (1 << 5);

//	sysClk = CSREPR & 0xC0;
//	switch(sysClk)
//	{
//		case SCS_100M :
//			TH1 = 0xE4;		// Baud rate = 9600 @ 100MHz.
//			break;
//		case SCS_50M :
//			TH1 = 0xF2;		// Baud rate = 9600 @ 50MHz.
//			break;
//		case SCS_25M :
//			TH1 = 0xF9;		// Baud rate = 9600 @ 25MHz.
//			break;
//		default :
//			TH1 = 0xF9;		// Baud rate = 9600 @ 25MHz.
//			break;
//	}

	ES0	= 1;				// Enable serial port Interrupt request
//	TR1 = 1;				// Run Timer 1
	TI0 = 0;
	PS0 = 1;
} /* End of UART_Init */


static void uart1_Init(void)
{
	U8_T sysClk = 0;
	// Initialize TIMER1 for standard 8051 UART clock
	PCON |= 0x40;			// Disable BaudRate doubler.
	SM10 = 0;
	SM11 = 1;			// Use serial port 1 in mode 1 with 8-bits data.
	REN1 = 1;			// Enable UART1 receiver.
	TMOD |= 0x20;		// Use timer 1 in mode 2, 8-bit counter with auto-reload.

	sysClk = CSREPR & 0xC0;
	switch (sysClk)
	{
		case SCS_100M :
			TH1 = 0xE4;		// Baud rate = 9600 @ 100MHz.
			break;
		case SCS_50M :
			TH1 = 0xF2;		// Baud rate = 9600 @ 50MHz.
			break;
		case SCS_25M :
			TH1 = 0xF9;		// Baud rate = 9600 @ 25MHz.
			break;
		default :
			TH1 = 0xF9;		// Baud rate = 9600 @ 25MHz.
			break;
	}

	ES1	= 1;				// Enable serial port Interrupt request
	TR1 = 1;				// Run Timer 1
	TI1 = 0;
	PS1 = 1;
}

void UART_Init(U8_T port)
{
	if(port == 0)
	{
		uart0_Init();
	}
	else if(port == 1)
	{
		uart1_Init();
	}
}