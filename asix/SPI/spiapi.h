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
 * Module Name : spiapi.h
 * Purpose     : 
 * Author      : Robin Lee
 * Date        : 2006-01-10
 * Notes       :
 * $Log$
 *================================================================================
 */
#ifndef SPIAPI_H
#define SPIAPI_H

/* INCLUDE FILE DECLARATIONS */


/* NAMING CONSTANT DECLARATIONS */
#define		SPI_READ	0x03	// read command
#define		SPI_WRITE	0x05	// write command
#define		SPI_WRDI	0x04	// write disable command
#define		SPI_WREN	0x08	// write enable command
#define		SPI_RDSR	0x05	// read status command
#define		SPI_WRSR	0x01	// write status command
#define		SPI_WPEN	BIT7	// write protect enable
#define		SPI_BP1		BIT3	// block protect control bit 1
#define		SPI_BP0		BIT2	// block protect control bit 0
#define		SPI_WEL		BIT1	// write enable latch
#define		SPI_WIP		BIT0	// write cycle is in progress



/* MACRO DECLARATIONS */


/* TYPE DECLARATIONS */


/* GLOBAL VARIABLES */


/* EXPORTED SUBPROGRAM SPECIFICATIONS */
BOOL	SPI_WriteEnable(void);
BOOL	SPI_WriteDisable(void);
BOOL	SPI_WriteStatus(U8_T status);
BOOL	SPI_ReadStatus(U8_T *status);
BOOL	SPI_ByteWrite(U8_T bytedata);
BOOL	SPI_ByteRead(U8_T *ptbytedata);

#endif /* End of SPIAPI_H */
