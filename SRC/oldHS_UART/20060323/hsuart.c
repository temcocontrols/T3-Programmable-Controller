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
 * Module Name : hsuart.c
 * Purpose     : 
 * Author      : Robin Lee
 * Date        : 
 * Notes       :
 * $Log: hsuart.c,v $
 * Revision 1.7  2005/11/24 12:51:26  robin6633
 * initiate values of use.
 *
 * Revision 1.6  2005/11/07 08:26:31  robin6633
 * Fixed the non-block getkey function
 *
 * Revision 1.5  2005/11/01 13:15:05  robin6633
 * Add non-block getkey function
 *
 * Revision 1.4  2005/10/06 10:18:06  robin6633
 * Added UART2 for console debug.
 *
 * Revision 1.3  2005/09/27 09:15:58  robin6633
 * Add loopback test function.
 *
 * Revision 1.2  2005/09/22 03:06:15  robin6633
 * Add High Speed Uart(UART2) driver for putchar() and getkey().
 *
 * Revision 1.1  2005/08/17 06:47:58  robin6633
 * no message
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include	"reg80390.h"
#include	"types.h"
#include	"hsuart.h"
#include	<stdio.h>


/* STATIC VARIABLE DECLARATIONS */
static U8_T		hsurIntrBusyType = 0;
static U8_T		hsurRxBuffer[UR2_MAX_RX_SIZE];
static U8_T		hsurTxBuffer[UR2_MAX_TX_SIZE];
static U16_T	hsurRxBufNum = 0;
static U16_T	hsurTxBufNum = 0;
static U8_T		hsurRxTrigLvl = 0;
static U16_T	hsurRxCount = 0;
static U16_T	hsurTxCount = 0;
static U16_T	hsurGetPtr = 0;
static U16_T	hsurPutPtr = 0;


/* LOCAL SUBPROGRAM DECLARATIONS */
static void		hsur_ReadLsr(void);
static void		hsur_RcvrTrig(void);
static void		hsur_Rcvr(void);
static void		hsur_Xmit(void);
static void		hsur_ReadMsr(void);


/* LOCAL SUBPROGRAM BODIES */

/*
 *--------------------------------------------------------------------------------
 * static void hsur_ReadLsr(void)
 * Purpose : Read Line Status Register and display.
 * Params  : None
 * Returns : None
 * Note    :
 *--------------------------------------------------------------------------------
 */
static void hsur_ReadLsr(void)
{
	U8_T	lineStatus = 0;

	lineStatus = UR2_LSR;

	if (lineStatus & UR2_OE)
	{
		// Overrun Error
		P3 = 0xE1;
	}
	else if (lineStatus & UR2_PE)
	{
		// Parity Error
		P3 = 0xE2;
	}
	else if (lineStatus & UR2_FE)
	{
		// Framing Error
		P3 = 0xE3;
	}
	else if (lineStatus & UR2_BI)
	{
		// Break Interrupt Occured
		P3 = 0xE4;
	}
	else if (lineStatus & UR2_FRAME_ERR)
	{
		// Mixing Error
		P3 = 0xE5;
	}
}

/*
 *--------------------------------------------------------------------------------
 * static void hsur_RcvrTrig(void)
 * Purpose : Get data and put into the receiver buffer continuously until trigger bytes
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
static void hsur_RcvrTrig(void)
{
	U16_T	i;

//	EA = 0;
	for (i=0 ; i<hsurRxTrigLvl ; i++)
	{
		hsurRxBuffer[hsurRxBufNum] = UR2_RBR;
		hsurRxBufNum ++;
		hsurRxCount ++;
		if (hsurRxBufNum == UR2_MAX_RX_SIZE)
			hsurRxBufNum = 0;
	}
//	EA = 1;
}

/*
 *--------------------------------------------------------------------------------
 * static void hsur_Rcvr(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
static void hsur_Rcvr(void)
{
	U8_T	lineStatus = 0;

	while (1)
	{
		lineStatus = UR2_LSR;
		if (lineStatus & UR2_DR)
		{
			hsurRxBuffer[hsurRxBufNum] = UR2_RBR;
			hsurRxBufNum ++;
			hsurRxCount ++;
			if (hsurRxBufNum == UR2_MAX_RX_SIZE)
				hsurRxBufNum = 0;
		}
		else
			break;
	}
}

/*
 *--------------------------------------------------------------------------------
 * static void hsur_Xmit(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
static void hsur_Xmit(void)
{
	U8_T	lineStatus = 0;

	lineStatus = UR2_LSR;
	if (lineStatus & UR2_THRE)
	{
		if (hsurTxCount > 0)
		{
			UR2_THR = hsurTxBuffer[hsurTxBufNum];
			hsurTxBufNum ++;
			hsurTxCount --;
		}
		else
		{
			UR2_IER &= ~UR2_THRI_ENB;
		}
		if (hsurTxBufNum == UR2_MAX_TX_SIZE)
			hsurTxBufNum = 0;
		if (hsurTxCount == 0)
			UR2_IER &= ~UR2_THRI_ENB;
	}
}

/*
 *--------------------------------------------------------------------------------
 * static void hsur_ReadMsr(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
static void hsur_ReadMsr(void)
{
	U8_T	modemStatus = 0;

	modemStatus = UR2_MSR;

	printf("%lx\n\r", (U32_T)modemStatus);

	if (modemStatus & UR2_CTS)
	{
		P3 = modemStatus;
	}
	else if (modemStatus & UR2_DSR)
	{
		P3 = modemStatus;
	}
	else if (modemStatus & UR2_RI)
	{
		P3 = modemStatus;
	}
	else if (modemStatus & UR2_DCD)
	{
		P3 = modemStatus;
	}
}


/* EXPORTED SUBPROGRAM BODIES */

/*
 *--------------------------------------------------------------------------------
 * void HSUR_Func(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void HSUR_Func(void)
{
	U8_T	intrStatus = 0;

	intrStatus = UR2_IIR & 0x0F;

	if (intrStatus == UR2_RLS_INTR)
	{
		hsur_ReadLsr();
	}
	else if (intrStatus == UR2_RD_TRIG_INTR)
	{
		if (hsurRxCount >= (UR2_MAX_RX_SIZE - 16))
			UR2_MCR |= UR2_RTS;
		hsurIntrBusyType = UR2_RD_TRIG_INTR;
		hsur_RcvrTrig();
	}
	else if (intrStatus == UR2_RD_TI_INTR)
	{
		hsurIntrBusyType = UR2_RD_TRIG_INTR;
		hsur_Rcvr();
	}
	else if (intrStatus == UR2_THRE_INTR)
	{
		if (hsurTxCount)
		{
			hsurIntrBusyType = UR2_THRE_INTR;
			hsur_Xmit();
		}
	}
	else if (intrStatus == UR2_MS_INTR)
	{
		hsurIntrBusyType = UR2_THRE_INTR;
		hsur_ReadMsr();
	}
}

/*
 *--------------------------------------------------------------------------------
 * void HSUR_Setup(U16_T divisor, U8_T lCtrl, U8_T intEnb, U8_T fCtrl, U8_T mCtrl)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void HSUR_Setup(U16_T divisor, U8_T lCtrl, U8_T intEnb, U8_T fCtrl, U8_T mCtrl)
{
	U16_T	i;

	UR2_LCR = UR2_DLAB_ENB;
	UR2_DLL = (U8_T)(divisor & 0x00FF);
	UR2_DLH = (U8_T)((divisor & 0xFF00) >> 8);
	UR2_LCR &= ~UR2_DLAB_ENB;
	UR2_LCR = lCtrl;
	UR2_IER = intEnb;
	UR2_FCR = fCtrl;
	UR2_MCR = mCtrl;

	for (i=0 ; i<UR2_MAX_RX_SIZE ; i++)
	{
		hsurRxBuffer[i] = 0;
		hsurTxBuffer[i] = 0;
	}
	hsurIntrBusyType = 0;
	hsurRxBufNum = 0;
	hsurTxBufNum = 0;
	hsurRxCount = 0;
	hsurTxCount = 0;
	hsurGetPtr = 0;
	hsurPutPtr = 0;

	switch (fCtrl & 0xC0)
	{
		case UR2_TRIG_01 :
			hsurRxTrigLvl = 1;
			break;
		case UR2_TRIG_04 :
			hsurRxTrigLvl = 4;
			break;
		case UR2_TRIG_08 :
			hsurRxTrigLvl = 8;
			break;
		case UR2_TRIG_14 :
			hsurRxTrigLvl = 14;
			break;
	}
}

/*
 *--------------------------------------------------------------------------------
 * S8_T HSUR_GetChar(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
S8_T HSUR_GetChar(void)
{
	S8_T	ch = 0;
    
	while (hsurRxCount == 0) ;
	
	ch = hsurRxBuffer[hsurGetPtr];
	hsurGetPtr ++;
	hsurRxCount --;
	if (hsurGetPtr == UR2_MAX_RX_SIZE)
		hsurGetPtr = 0;

	return ch;
}

/*
 *--------------------------------------------------------------------------------
 * S8_T HSUR_PutChar(S8_T ch)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
S8_T HSUR_PutChar(S8_T ch)
{
	if (ch == 0x0A)
	{
		while (hsurTxCount == UR2_MAX_TX_SIZE) ;
		
		hsurTxBuffer[hsurPutPtr] = 0x0D;
		hsurPutPtr ++;
		hsurTxCount ++;
		if (hsurPutPtr == UR2_MAX_TX_SIZE)
			hsurPutPtr = 0;
	}
	while (hsurTxCount == UR2_MAX_TX_SIZE) ;
	
	hsurTxBuffer[hsurPutPtr] = ch;
	hsurPutPtr ++;
	hsurTxCount ++;
	if (hsurPutPtr == UR2_MAX_TX_SIZE)
		hsurPutPtr = 0;

	if (hsurTxCount == 0)
	{
		UR2_THR = hsurTxBuffer[hsurTxBufNum];
		hsurTxBufNum ++;
	}
	UR2_IER |= UR2_THRI_ENB;

	return ch;
}

/*
* -----------------------------------------------------------------------------
 * Function Name: HSUR_InitValue
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void HSUR_InitValue(void)
{
	U8_T	sysClk = 0;
	U16_T	i;

	for (i=0 ; i<UR2_MAX_RX_SIZE ; i++)
	{
		hsurRxBuffer[i] = 0;
		hsurTxBuffer[i] = 0;
	}
	hsurIntrBusyType = 0;
	hsurRxBufNum = 0;
	hsurTxBufNum = 0;
	hsurRxTrigLvl = 0;
	hsurRxCount = 0;
	hsurTxCount = 0;
	hsurGetPtr = 0;
	hsurPutPtr = 0;

	sysClk = CSREPR & 0xC0;
	switch (sysClk)
	{
		case SCS_100M :
			P3 = 0x10;
			break;
		case SCS_50M :
			P3 = 0x50;
			break;
		case SCS_25M :
			P3 = 0x25;
			break;
		default :
			P3 = 0xAA;
			break;
	}

} /* End of UART_Init */

/*
 *--------------------------------------------------------------------------------
 * void HSUR_RxFunc(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void HSUR_RxFunc(void)
{
	U8_T	byteData = 0;

	while (1)
	{
		byteData = HSUR_GetChar();
		HSUR_PutChar(byteData);
	}
}

/*
 *--------------------------------------------------------------------------------
 * S8_T HSUR_GetCharNb(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
S8_T HSUR_GetCharNb(void)
{
	S8_T ch = 0;
    
	if (hsurRxCount != 0)
	{

		ch = hsurRxBuffer[hsurGetPtr];
		hsurGetPtr ++;
		hsurRxCount --;
		if (hsurGetPtr == UR2_MAX_RX_SIZE)
			hsurGetPtr = 0;
	
		return ch;
	}
	else
	{
		return FALSE;
	}
}


/* End of hsuart.c */
