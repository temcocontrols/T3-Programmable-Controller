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
 * Module Name : main_ow.c
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
#include	"onewire.h"


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

	/* enable presence detect, clock enable, 25MHz */
	ONEWIRE_Setup(0, 0, OW_PDI_ENB, OW_CLK_ENB|OW_DIV1|OW_DIV0|OW_PRE0); // 25M

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





/* End of main_ow.c */
