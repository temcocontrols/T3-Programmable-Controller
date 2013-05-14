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
 * Module Name : main_hsur.c
 * Purpose     : 
 * Author      : Robin Lee
 * Date        : 
 * Notes       :
 * $Log$
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include	"reg80390.h"
#include	"types.h"
#include	"ax11000.h"
#include	"interrupt.h"
#include	"hsuart.h"


/* STATIC VARIABLE DECLARATIONS */


/* LOCAL SUBPROGRAM DECLARATIONS */


/* LOCAL SUBPROGRAM BODIES */

/*
 *--------------------------------------------------------------------------------
 * void main(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void main(void)
{

	/* System Initiation */
	AX11000_Init();

	HSUR_Setup(0x0060, (UR2_CHAR_8|UR2_STOP_10), (UR2_RDI_ENB|UR2_RLSI_ENB),
	(UR2_FIFO_MODE|UR2_RXFIFO_RST|UR2_TXFIFO_RST|UR2_TRIG_08), UR2_RTS);

}


/* EXPORTED SUBPROGRAM BODIES */

/*
 *--------------------------------------------------------------------------------
 * void Function(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */





/* End of main_spi.c */
