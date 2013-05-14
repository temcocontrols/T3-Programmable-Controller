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
 * Module Name : onewireapi.h
 * Purpose     : 
 * Author      : Robin Lee
 * Date        : 2005-01-12
 * Notes       :
 * $Log$
 *================================================================================
 */
#ifndef ONEWIREAPI_H
#define ONEWIREAPI_H

/* INCLUDE FILE DECLARATIONS */
#include	"onewire_cfg.h"


/* NAMING CONSTANT DECLARATIONS */


/* MACRO DECLARATIONS */


/* TYPE DECLARATIONS */


/* GLOBAL VARIABLES */


/* EXPORTED SUBPROGRAM SPECIFICATIONS */
BOOL	ONEWIRE_WriteScratchpad(U16_T addrOfMem, U8_T *ptByteData, U16_T devNum);
BOOL	ONEWIRE_ReadScratchpad(U16_T *addrOfMem, U8_T *esByte, U8_T *ptByteData, U16_T devNum);
BOOL	ONEWIRE_CopyScratchpad(U16_T addrOfMem, U8_T esByte, U16_T devNum);
#if MEMORY_SECTIONS
BOOL	ONEWIRE_8ByteWrite(U16_T addrOfMem, U8_T *ptByteData, U16_T devNum);
BOOL	ONEWIRE_8ByteRead(U16_T addrOfMem, U8_T *ptByteData, U16_T devNum);
BOOL	ONEWIRE_PageRead(U16_T addrOfMem, U8_T *ptByteData, U16_T rdLen, U16_T devNum);
BOOL	ONEWIRE_ReadMemory(U16_T addrOfMem, U16_T devNum);
#endif


#endif /* End of ONEWIREAPI_H */
