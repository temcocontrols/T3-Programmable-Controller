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
 * Module Name : onewireapi.c
 * Purpose     : 
 * Author      : Robin Lee
 * Date        : 2006-01-12
 * Notes       :
 * $Log$
 *================================================================================
 */
/* INCLUDE FILE DECLARATIONS */
#include	"types.h"
#include	"reg80390.h"
#include	"buffer.h"
#include	"onewire.h"
#include	"onewireapi.h"


/* STATIC VARIABLE DECLARATIONS */


/* LOCAL SUBPROGRAM DECLARATIONS */


/* LOCAL SUBPROGRAM BODIES */

/*
 *--------------------------------------------------------------------------------
 * BOOL ONEWIRE_WriteScratchpad(U16_T addrOfMem, U8_T *ptByteData, U16_T devNum)
 * Purpose : write 8 bytes data into scratchpad.
 * Params  : addrOfMem - address of accessing.
 *           *ptByteData - pointer of data string which be writen
 * Returns : TRUE - successful
 * Note    : 
 *--------------------------------------------------------------------------------
 */
BOOL ONEWIRE_WriteScratchpad(U16_T addrOfMem, U8_T *ptByteData, U16_T devNum)
{
	OW_BUF XDATA	*ptWrScrpad = NULL;
	U8_T			i;
	U8_T			regVal = 0;
	U16_T			calCrc = 0;
	U8_T			*ptRomCodes = NULL;

	ONEWIRE_Cmd(SI_RD, OWCMDR, &regVal);
	ptWrScrpad = (OW_BUF *)GetPktBuf();
	if (devNum == 0)
	{
		if (regVal & OW_OD)
		{
			ptWrScrpad->RomCmd = OD_SKIP_ROM;
		}
		else
		{
			ptWrScrpad->RomCmd = SKIP_ROM;
		}
		for (i=0 ; i<8 ; i++)
			ptWrScrpad->RomArray[i] = 0;
	}
	else
	{
		if (regVal & OW_OD)
		{
			ptWrScrpad->RomCmd = OD_MATCH_ROM;
		}
		else
		{
			ptWrScrpad->RomCmd = MATCH_ROM;
		}
		ptRomCodes = ONEWIRE_GetRomCode();
		for (i=0 ; i<8 ; i++)
			ptWrScrpad->RomArray[i] = *(ptRomCodes + (i + (8*(devNum-1))));
	}
	ONEWIRE_SetCrc16(0);
	ptWrScrpad->MemCmd = WRITE_SCRPAD;
	calCrc = ONEWIRE_DoCrc16(ptWrScrpad->MemCmd);
	// target address low byte
	ptWrScrpad->MemArray[0] = (U8_T)(addrOfMem & 0x00FF);
	calCrc = ONEWIRE_DoCrc16(ptWrScrpad->MemArray[0]);
	// target address high byte
	ptWrScrpad->MemArray[1] = (U8_T)((addrOfMem & 0xFF00) >> 8);
	calCrc = ONEWIRE_DoCrc16(ptWrScrpad->MemArray[1]);
	// ES byte
	ptWrScrpad->MemArray[2] = 0;
	// 8 data bytes for writing
	for (i=0 ; i<8 ; i++)
	{
		ptWrScrpad->MemArray[3+i] = *(ptByteData + i);
		calCrc = ONEWIRE_DoCrc16(ptWrScrpad->MemArray[3+i]);
	}
	
	ONEWIRE_SendPkt(ptWrScrpad);
		
	if (!ONEWIRE_Reset())
		return FALSE;
	
	while (1)
	{
		ONEWIRE_StateFunc();
		if (ONEWIRE_StateChk(IDLE_STATE) == TRUE)
			break;
	}
	if (ONEWIRE_StateChk(CRC_ERR) == TRUE)
	{
		return FALSE;
	}
	
	ptWrScrpad = ONEWIRE_GetCurBuf();

	if ((U16_T)(~calCrc) != ((U16_T)(ptWrScrpad->MemArray[11]) | ((U16_T)(ptWrScrpad->MemArray[12]) << 8)))
	{
		return FALSE;
	}

	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * BOOL ONEWIRE_ReadScratchpad(U16_T *addrOfMem, U8_T *esByte, U8_T *ptByteData, U16_T devNum)
 * Purpose : read 8 bytes data from scratchpad.
 * Params  : *addrOfMem - pointer of memory address to be stored for reading.
 *           *esByte - pointer of ESByte to be stored for reading.
 *           *ptByteData - pointer of data block to be stored for reading.
 *           devNum - device number
 * Returns : TRUE - successful.
 * Note    :
 *--------------------------------------------------------------------------------
 */
BOOL ONEWIRE_ReadScratchpad(U16_T *addrOfMem, U8_T *esByte, U8_T *ptByteData, U16_T devNum)
{
	OW_BUF XDATA	*ptRdScrpad = NULL;
	U16_T			i;
	U16_T			calCrc = 0;
	U8_T			regVal = 0;
	U8_T			*ptRomCodes = NULL;

	ONEWIRE_Cmd(SI_RD, OWCMDR, &regVal);
	ptRdScrpad = (OW_BUF *)GetPktBuf();
	if (devNum == 0)
	{
		if (regVal & OW_OD)
		{
			ptRdScrpad->RomCmd = OD_SKIP_ROM;
		}
		else
		{
			ptRdScrpad->RomCmd = SKIP_ROM;
		}
		for (i=0 ; i<8 ; i++)
			ptRdScrpad->RomArray[i] = 0;
	}
	else
	{
		if (regVal & OW_OD)
		{
			ptRdScrpad->RomCmd = OD_MATCH_ROM;
		}
		else
		{
			ptRdScrpad->RomCmd = MATCH_ROM;
		}
		ptRomCodes = ONEWIRE_GetRomCode();
		for (i=0 ; i<8 ; i++)
			ptRdScrpad->RomArray[i] = *(ptRomCodes + (i + (8*(devNum-1))));
	}

	ptRdScrpad->MemCmd = READ_SCRPAD;
	
	ONEWIRE_SendPkt(ptRdScrpad);
	
	if (!ONEWIRE_Reset())
		return FALSE;

	while (1)
	{
		ONEWIRE_StateFunc();
		if (ONEWIRE_StateChk(IDLE_STATE) == TRUE)
			break;
	}
	if (ONEWIRE_StateChk(CRC_ERR) == TRUE)
	{
		return FALSE;
	}

	ptRdScrpad = ONEWIRE_GetCurBuf();

	*addrOfMem = (U16_T)ptRdScrpad->MemArray[0] | ((U16_T)ptRdScrpad->MemArray[1] << 8);
	*esByte = ptRdScrpad->MemArray[2];
	for (i=0 ; i<8 ; i++)
	{
		*(ptByteData+i) = ptRdScrpad->MemArray[3+i];
	}

	if (*esByte != 0x07)
	{
		return FALSE;
	}

	ONEWIRE_SetCrc16(0);
	calCrc = ONEWIRE_DoCrc16(ptRdScrpad->MemCmd);
	for (i=0 ; i<11 ; i++)
		calCrc = ONEWIRE_DoCrc16(ptRdScrpad->MemArray[i]);
	if ((U16_T)(~calCrc) != ((U16_T)(ptRdScrpad->MemArray[11]) | ((U16_T)(ptRdScrpad->MemArray[12]) << 8)))
	{
		return FALSE;
	}

	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * BOOL ONEWIRE_CopyScratchpad(U16_T addrOfMem, U8_T esByte, U16_T devNum)
 * Purpose : copy 8 bytes data from scratchpad into memory sections.
 * Params  : addrOfMem - target address of memory sections.
 *           esByte - ending address with data status
 * Returns : TRUE - successful
 * Note    :
 *--------------------------------------------------------------------------------
 */
BOOL ONEWIRE_CopyScratchpad(U16_T addrOfMem, U8_T esByte, U16_T devNum)
{
	OW_BUF XDATA	*ptCpScrpad = NULL;
	U16_T			i;
	U8_T			regVal = 0;
	U8_T			*ptRomCodes = NULL;

	ONEWIRE_Cmd(SI_RD, OWCMDR, &regVal);
	ptCpScrpad = (OW_BUF *)GetPktBuf();
	if (devNum == 0)
	{
		if (regVal & OW_OD)
		{
			ptCpScrpad->RomCmd = OD_SKIP_ROM;
		}
		else
		{
			ptCpScrpad->RomCmd = SKIP_ROM;
		}
		for (i=0 ; i<8 ; i++)
			ptCpScrpad->RomArray[i] = 0;
	}
	else
	{
		if (regVal & OW_OD)
		{
			ptCpScrpad->RomCmd = OD_MATCH_ROM;
		}
		else
		{
			ptCpScrpad->RomCmd = MATCH_ROM;
		}
		ptRomCodes = ONEWIRE_GetRomCode();
		for (i=0 ; i<8 ; i++)
			ptCpScrpad->RomArray[i] = *(ptRomCodes + (i + (8*(devNum-1))));		
	}
	ptCpScrpad->MemCmd = COPY_SCRPAD;

	// target address low byte
	ptCpScrpad->MemArray[0] = (U8_T)(addrOfMem & 0x00FF);
	// target address high byte
	ptCpScrpad->MemArray[1] = (U8_T)((addrOfMem & 0xFF00) >> 8);
	// ES byte
	ptCpScrpad->MemArray[2] = esByte;
	
	for (i=0 ; i<10 ; i++)
	{
		ptCpScrpad->MemArray[3+i] = 0;
	}

	ONEWIRE_SendPkt(ptCpScrpad);
	
	if (!ONEWIRE_Reset())
		return FALSE;
	
	while (1)
	{
		ONEWIRE_StateFunc();
		if (ONEWIRE_StateChk(IDLE_STATE) == TRUE)
			break;
	}
	if (ONEWIRE_StateChk(CRC_ERR) == TRUE)
	{
		return FALSE;
	}
	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * BOOL ONEWIRE_ReadMemory(U16_T addrOfMem, U16_T devNum)
 * Purpose : read whole data from memory sections.
 * Params  : addrOfMem - address of memory sections.
 *           devNum - device number.
 * Returns : TRUE - successful.
 * Note    : read memory always reads starting from the address until end address.
 *--------------------------------------------------------------------------------
 */
#if MEMORY_SECTIONS
BOOL ONEWIRE_ReadMemory(U16_T addrOfMem, U16_T devNum)
{
	OW_BUF XDATA	*ptRdMem = NULL;
	U16_T			i;
	U8_T			regVal = 0;
	U8_T			*ptRomCodes = NULL;

	ONEWIRE_Cmd(SI_RD, OWCMDR, &regVal);
	ptRdMem = (OW_BUF *)GetPktBuf();
	if (devNum == 0)
	{
		if (regVal & OW_OD)
		{
			ptRdMem->RomCmd = OD_SKIP_ROM;
		}
		else
		{
			ptRdMem->RomCmd = SKIP_ROM;
		}
		for (i=0 ; i<8 ; i++)
			ptRdMem->RomArray[i] = 0;
	}
	else
	{
		if (regVal & OW_OD)
		{
			ptRdMem->RomCmd = OD_MATCH_ROM;
		}
		else
		{
			ptRdMem->RomCmd = MATCH_ROM;
		}
		ptRomCodes = ONEWIRE_GetRomCode();
		for (i=0 ; i<8 ; i++)
			ptRdMem->RomArray[i] = *(ptRomCodes + (i + (8*(devNum-1))));
	}

	ptRdMem->MemCmd = READ_MEMORY;

	// target address low byte
	ptRdMem->MemArray[0] = (U8_T)(addrOfMem & 0x00FF);
	// target address high byte
	ptRdMem->MemArray[1] = (U8_T)((addrOfMem & 0xFF00) >> 8);
	// ES byte
	ptRdMem->MemArray[2] = 0;
	
	for (i=0 ; i<10 ; i++)
	{
		ptRdMem->MemArray[3+i] = 0;
	}

	ONEWIRE_SendPkt(ptRdMem);

	if (!ONEWIRE_Reset())
		return FALSE;

	while (1)
	{
		ONEWIRE_StateFunc();
		if (ONEWIRE_StateChk(IDLE_STATE) == TRUE)
			break;
	}
	if (ONEWIRE_StateChk(CRC_ERR) == TRUE)
	{
		return FALSE;
	}

	return TRUE;
}
#endif


/* EXPORTED SUBPROGRAM BODIES */

/*
 *--------------------------------------------------------------------------------
 * void ONEWIRE_8ByteWrite(U16_T addrOfMem, U8_T *ptByteData, U16_T devNum)
 * Purpose : write 8 bytes data into memory sections.
 * Params  : addrOfMem - address of memory sections.
 *           *ptByteData - pointer of data block to be writen.
 *           devNum - device number.
 * Returns : TRUE - successful.
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if MEMORY_SECTIONS
BOOL ONEWIRE_8ByteWrite(U16_T addrOfMem, U8_T *ptByteData, U16_T devNum)
{
	U16_T	authAddr = 0;
	U8_T	authEsByte = 0;
	U8_T	tempBytes[8] = {0,0,0,0,0,0,0,0};

	if (!ONEWIRE_WriteScratchpad(addrOfMem, ptByteData, devNum))
		return FALSE;
	if (!ONEWIRE_ReadScratchpad(&authAddr, &authEsByte, &tempBytes, devNum))
		return FALSE;
	if (!ONEWIRE_CopyScratchpad(authAddr, authEsByte, devNum))
		return FALSE;

	return TRUE;
}
#endif

/*
 *--------------------------------------------------------------------------------
 * BOOL ONEWIRE_8ByteRead(U16_T addrOfMem, U8_T *ptByteData, U16_T devNum)
 * Purpose : read 8 bytes data from memory sections.
 * Params  : addrOfMem - address of memory sections.
 *           *ptByteData - pointer of data block to be stored.
 *           devNum - device number.
 * Returns : TRUE - successful.
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if MEMORY_SECTIONS
BOOL ONEWIRE_8ByteRead(U16_T addrOfMem, U8_T *ptByteData, U16_T devNum)
{
	U8_T	i;
	U8_T	*ptRead = NULL;

	if (!ONEWIRE_ReadMemory(addrOfMem, devNum))
		return FALSE;

	ptRead = ONEWIRE_GetRdMemData();
	for (i=0 ; i<8 ; i++)
	{
		*(ptByteData + i) = *(ptRead + i);
	}

	return TRUE;
}
#endif

/*
 *--------------------------------------------------------------------------------
 * BOOL ONEWIRE_PageRead(U16_T addrOfMem, U8_T *ptByteData, U16_T rdLen, U16_T devNum)
 * Purpose : read data block from memory with a length.
 * Params  : addrOfMem - address of memory sections.
 *           *ptByteData - pointer of data block to be stored.
 *           rdLen - length to be read.
 *           devNum - device number.
 * Returns : TRUE - successful.
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if MEMORY_SECTIONS
BOOL ONEWIRE_PageRead(U16_T addrOfMem, U8_T *ptByteData, U16_T rdLen, U16_T devNum)
{
	U8_T	i;
	U8_T	*ptRead = NULL;

	if (!ONEWIRE_ReadMemory(addrOfMem, devNum))
		return FALSE;

	ptRead = ONEWIRE_GetRdMemData();
	for (i=0 ; i<rdLen ; i++)
	{
		*(ptByteData + i) = *(ptRead + i);
	}

	return TRUE;
}
#endif


/* End of onewireapi.c */
