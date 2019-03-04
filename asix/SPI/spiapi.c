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
 * Module Name : spiapi.c
 * Purpose     : 
 * Author      : Robin Lee
 * Date        : 2006-01-10
 * Notes       :
 * $Log$
 *================================================================================
 */
#include "product.h"

#if ASIX_MINI

/* INCLUDE FILE DECLARATIONS */
#include	"reg80390.h"
#include	"types.h"
#include	"spi.h"
#include	"spiapi.h"
#include 	"stdio.h"

extern unsigned int far Test[50];
/* STATIC VARIABLE DECLARATIONS */
U16_T spi_delay;

/* LOCAL SUBPROGRAM DECLARATIONS */


/* LOCAL SUBPROGRAM BODIES */


/* EXPORTED SUBPROGRAM BODIES */

/*
 *--------------------------------------------------------------------------------
 * BOOL SPI_WriteEnable(void)
 * Purpose : Enable a write process before sending a packet to write.
 * Params  : none
 * Returns : TRUE - successful
 * Note    : Write Enable must be initial when executing all write function.
 *--------------------------------------------------------------------------------
 */
BOOL SPI_WriteEnable(void)
{
	U8_T	xmitByte = 0;
	
	xmitByte = SPI_WREN;
	SPI_FlagEnb(SPI_BUSY);
	SPI_MstXmit(&xmitByte, 8, SPI_NORMAL_LEN|SPI_GO_BSY);
	spi_delay = 1000;
	while (SPI_FlagChk(SPI_BUSY) && (spi_delay--)) {}

	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * BOOL SPI_WriteDisable(void)
 * Purpose : Disable a write process.
 * Params  : none
 * Returns : TRUE - successful
 * Note    : After WriteDisable executed, all writing function will be fail.
 *--------------------------------------------------------------------------------
 */
BOOL SPI_WriteDisable(void)
{
	U8_T	xmitByte = 0;
	
	xmitByte = SPI_WRDI;
	SPI_FlagEnb(SPI_BUSY);
	SPI_MstXmit(&xmitByte, 8, SPI_NORMAL_LEN|SPI_GO_BSY);

	spi_delay = 1000;
	while (SPI_FlagChk(SPI_BUSY) && (spi_delay--)) {}

	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * BOOL SPI_WriteStatus(U8_T status)
 * Purpose : Change a device status register.
 * Params  : status - changed value.
 * Returns : TRUE - successful.
 * Note    :
 *--------------------------------------------------------------------------------
 */
BOOL SPI_WriteStatus(U8_T status)
{
	U8_T	writeStatus[2];
	
	writeStatus[1] = SPI_WRSR;
	writeStatus[0] = status;

	if (!SPI_WriteEnable())
		return FALSE;
	SPI_FlagEnb(SPI_BUSY);
	SPI_MstXmit(&writeStatus, 16, SPI_NORMAL_LEN|SPI_GO_BSY);
	spi_delay = 1000;
	while (SPI_FlagChk(SPI_BUSY) && (spi_delay--)) {}
	
	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * BOOL SPI_ReadStatus(U8_T *status)
 * Purpose : Read a device status register.
 * Params  : *status - a pointer of status value.
 * Returns : TRUE - successful
 * Note    : 
 *--------------------------------------------------------------------------------
 */
BOOL SPI_ReadStatus(U8_T *status)
{
	U8_T	readStatus[2];
	
	readStatus[1] = SPI_RDSR;
	readStatus[0] = 0;
	
	SPI_FlagEnb(SPI_BUSY);
	SPI_MstXmit(&readStatus, 16, SPI_NORMAL_LEN|SPI_GO_BSY);
	//printf("a\n");  
	spi_delay = 1000;
	while (SPI_FlagChk(SPI_BUSY) && (spi_delay--)) {}

	SPI_GetData(&readStatus);
	//printf("b\n");
	*status = readStatus[0];
	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * BOOL SPI_ByteWrite(U16_T addrOfMem, U8_T byteData)
 * Purpose : Write one byte data into spi devices.
 * Params  : addrOfMem - address of accessing.
 *           byteData - data to be written.
 * Returns : TRUE - successful.
 * Note    :
 *--------------------------------------------------------------------------------
 */

BOOL SPI_ByteWrite(U8_T byteData)
{
	U8_T	byteWrite;

	byteWrite = byteData;
	SPI_FlagEnb(SPI_BUSY);
	SPI_MstXmit(&byteWrite,8, SPI_NORMAL_LEN|SPI_GO_BSY);

	spi_delay = 1000;
	while (SPI_FlagChk(SPI_BUSY) && (spi_delay--)) {}

	
	return TRUE;
}

 
 #endif
/* End of spiapi.c */
