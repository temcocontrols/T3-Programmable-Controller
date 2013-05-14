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
 * Module Name : onewire.c
 * Purpose     : This module handles the OneWire serial interface driver.
 * Author      : Robin Lee
 * Date        : 2005-03-31
 * Notes       :
 * $Log: onewire.c,v $
 * Revision 1.6  2005/11/24 12:51:35  robin6633
 * initiate values of use.
 *
 * Revision 1.5  2005/11/01 13:19:08  robin6633
 * fixed OD_MATCH_ROM state changed.
 *
 * Revision 1.4  2005/08/31 02:06:07  robin6633
 * Added the rom code CRC8 cheching and memory data CRC16 checking.
 *
 * Revision 1.3  2005/08/17 06:48:22  robin6633
 * no message
 *
 * Revision 1.2  2005/06/14 02:50:12  arthur
 * changed interrupt.h include
 *
 * Revision 1.1.1.1  2005/06/06 05:55:57  robin6633
 * no message
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include	<stdio.h>
#include	"reg80390.h"
#include	"types.h"
#include	"buffer.h"
#include	"onewire.h"


/* STATIC VARIABLE DECLARATIONS */
static OW_BUF	owPkt;
static U8_T		owRomCmd = 0;
static U8_T		owMemCmd = 0;
static U8_T		owRomBitArray[64];
static U8_T		owRcFlag = 0;
static U8_T		owOdFlag = 0;
static U16_T	owTxCnt = 0;
static U16_T	owRxCnt = 0;
static U8_T		owState = 0;
static U8_T		owRdByte = 0;
static U8_T		owCrc8 = 0;
static U16_T	owCrc16 = 0;
static U8_T		owRdMemData[144];
static U8_T		owIntrFlag = 0;
static U8_T		owIntrEnbFlag = 0;
static U8_T		owRomCodeBuf[MAX_SERIAL_BUF_NUM*8] = 
				{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

static U16_T	owLastDiscrepancy = 0;
static U16_T	owLastDeviceFlag = 0;
static U16_T	owLastFamilyDiscrepancy = 0;
static U16_T	owSearchResult = 0;
static U16_T	owSerialBufNum = 0;
static U16_T	owOddParity[16] = { 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 };
static U8_T		owCrc8Table[] = {
	  0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
	157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
	 35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
	190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
	 70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
	219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
	101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
	248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
	140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
	 17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
	175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
	 50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
	202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
	 87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
	233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
	116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53
};


/* LOCAL SUBPROGRAM DECLARATIONS */
static void		onewire_IntrBitsEnb(U8_T bits);
static void		onewire_IntrBitsDisb(U8_T bits);
static BOOL		onewire_ByteXmit(U8_T owByteData);
static U8_T		onewire_ByteRcvr(void);
static void		onewire_RomFunc(void);
static void		onewire_RomCodeToBit(void);

#if MEMORY_SECTIONS
static void		onewire_MemCmdChk(void);
static void		onewire_MemFunc(void);
#endif


/* LOCAL SUBPROGRAM BODIES */

/*
 *--------------------------------------------------------------------------------
 * static void onewire_IntrBitsEnb(U8_T Bits)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
static void onewire_IntrBitsEnb(U8_T bits)
{
	U8_T	regVal = 0;

	owIntrEnbFlag = bits;
	ONEWIRE_Cmd(SI_RD, OWIER, &regVal);
	regVal = regVal | bits;
	ONEWIRE_Cmd(SI_WR, OWIER, &regVal);
}

/*
 *--------------------------------------------------------------------------------
 * static void onewire_IntrBitsDisb(U8_T bits)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
static void onewire_IntrBitsDisb(U8_T bits)
{
	U8_T	regVal = 0;

	ONEWIRE_Cmd(SI_RD, OWIER, &regVal);
	regVal = regVal & (~bits);
	ONEWIRE_Cmd(SI_WR, OWIER, &regVal);
}

/*
 *--------------------------------------------------------------------------------
 * static BOOL onewire_ByteXmit(U8_T owByteData)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
static BOOL onewire_ByteXmit(U8_T owByteData)
{
	U8_T	tempRxByte = 0;
	U8_T	checkFlag = 0;

	ONEWIRE_Cmd(SI_WR, OWTRR, &owByteData);

	owIntrFlag = 0;
	onewire_IntrBitsEnb(OW_TSREI_ENB);
	while (1)
	{
		if (owIntrFlag & OW_TSRE)
			break;
	}
	owIntrFlag = 0;

//	SiCmd(SI_WR, OWTRR, &owbytedata);
	
	while (1)
	{
		ONEWIRE_Cmd(SI_RD, OWISR, &checkFlag);
		if (checkFlag & OW_RBF)
			break;
	}

	ONEWIRE_Cmd(SI_RD, OWTRR, &tempRxByte);

	return (tempRxByte == owByteData) ? TRUE : FALSE;
}

/*
 *--------------------------------------------------------------------------------
 * static U8_T onewire_ByteRcvr(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
static U8_T onewire_ByteRcvr(void)
{
	U8_T	tempTxByte = 0;
	U8_T	tempRxByte = 0;
	U8_T	checkFlag = 0;

	while (1)
	{
		ONEWIRE_Cmd(SI_RD, OWISR, &checkFlag);
		if (checkFlag & OW_TSRE)
			break;
	}
	tempTxByte = 0xFF;
	ONEWIRE_Cmd(SI_WR, OWTRR, &tempTxByte);

	onewire_IntrBitsEnb(OW_RBFI_ENB);
	while (!(owIntrFlag & OW_RBF)) {};
	owIntrFlag = 0;

	ONEWIRE_Cmd(SI_RD, OWTRR, &tempRxByte);

	return tempRxByte;
}

/*
 *--------------------------------------------------------------------------------
 * static void onewire_MemCmdChk(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if MEMORY_SECTIONS
static void onewire_MemCmdChk(void)
{
	if (owState & MEM_CMD_STATE)
	{
		if (onewire_ByteXmit(owMemCmd))
		{
			owState = MEM_SEQ_STATE;
			owTxCnt = 0;
			owRxCnt = 0;
			owCrc8 = 0;
			owCrc16 = 0;
		}
		else
		{
			ONEWIRE_Reset();
		}
	}
}
#endif


/*
 *--------------------------------------------------------------------------------
 * void onewire_RomFunc(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void onewire_RomFunc(void)
{
	U8_T	cmdRegVal = 0;
	U8_T	owCrc8Check = 0;
	U8_T	i;

	if ((owRomCmd == SKIP_ROM) || (owRomCmd == OD_SKIP_ROM))
	{
	}
	else if (owRomCmd == READ_ROM)
	{
		if (owRxCnt < 8)
		{
			owPkt.RomArray[owRxCnt] = onewire_ByteRcvr();
			owRxCnt ++;
			if (owRxCnt == 8)
			{
				owRxCnt = 0;
				owRcFlag = 0;
				ONEWIRE_SetCrc8(0);
				for (i=0 ; i<8 ; i++)
				{
					owCrc8Check = ONEWIRE_DoCrc8(owPkt.RomArray[i]);
				}
				if (owCrc8Check == 0)
				{
					if (owMemCmd)
						owState = MEM_CMD_STATE;
					else
						owState = IDLE_STATE;
				}
				else
				{
					owState = IDLE_STATE | CRC_ERR;
				}
			}
		}
	}
#if MULTI_DEVICES
	if ((owRomCmd == MATCH_ROM) || (owRomCmd == OD_MATCH_ROM))
	{
		if (owTxCnt < 8)
		{
			if (!onewire_ByteXmit(owPkt.RomArray[owTxCnt]))
			{
				owOdFlag = 0;
				/* Setup OneWire Command mode */
				ONEWIRE_Cmd(SI_RD, OWCMDR, &cmdRegVal);
				cmdRegVal = cmdRegVal & ~OW_OD;
				ONEWIRE_Cmd(SI_WR, OWCMDR, &cmdRegVal);
				ONEWIRE_Reset();
				return;
			}
			owTxCnt ++;
			if (owTxCnt == 8)
			{
				owTxCnt = 0;
				owRcFlag = 1;
				
				if (owMemCmd)
					owState = MEM_CMD_STATE;
				else
					owState = IDLE_STATE;
			}
		}
	}
#endif
	else if (owRomCmd == SEARCH_ROM)
	{
		ONEWIRE_SearchRomFunc();
		owState = IDLE_STATE;
	}
}

#if MEMORY_SECTIONS
/*
 *--------------------------------------------------------------------------------
 * static void onewire_MemFunc(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
static void onewire_MemFunc(void)
{
	U8_T	tempRxD = 0;
	U8_T	sysClk = 0;

	if (owState & MEM_SEQ_STATE)
	{
		if (owMemCmd == WRITE_SCRPAD)
		{
			if (owTxCnt < 11)
			{
				onewire_ByteXmit(owPkt.MemArray[owTxCnt]);
				owTxCnt ++;
				if (owTxCnt == 2)
				{
					owTxCnt ++;
				}
			}
			else
			{
				owPkt.MemArray[owTxCnt] = onewire_ByteRcvr();
				owTxCnt ++;
				if (owTxCnt == 13)
				{
					owTxCnt = 0;
					owState = IDLE_STATE;
				}
			}
		}
		else if (owMemCmd == READ_SCRPAD)
		{
			if (owRxCnt < 3)
			{
				owPkt.MemArray[owRxCnt] = onewire_ByteRcvr();
				owRxCnt ++;
				if (owRxCnt == 3)
				{
					// ES Byte check
					if (!(owPkt.MemArray[2] & AA_FLAG) && !(owPkt.MemArray[2] & PF_FLAG))
						owRdByte = (owPkt.MemArray[2] & 0x07) - (owPkt.MemArray[0] & 0x07) + 1;
					else
						owState = IDLE_STATE;
				}
			}
			else
			{
				owPkt.MemArray[owRxCnt] = onewire_ByteRcvr();
				owRxCnt ++;
				if (owRxCnt == 13)
				{
					owRxCnt = 0;
					owState = IDLE_STATE;
				}
			}
		}
		else if (owMemCmd == COPY_SCRPAD)
		{
			if (owTxCnt < 3)
			{
				U32_T	i;

				onewire_ByteXmit(owPkt.MemArray[owTxCnt]);
				owTxCnt ++;
				if (owTxCnt == 3)
				{
					sysClk = CSREPR & 0xC0;
					switch (sysClk)
					{
						case SCS_100M :
							// Baud rate = 9600 @ 100MHz.
							for (i=0 ; i<4800 ; i++)
							{
							}
							break;
						case SCS_50M :
							// Baud rate = 9600 @ 50MHz.
							for (i=0 ; i<2400 ; i++)
							{
							}
							break;
						case SCS_25M :
							// Baud rate = 9600 @ 25MHz.
							for (i=0 ; i<1200 ; i++)
							{
							}
							break;
					}
				}
			}
			else
			{
				tempRxD = onewire_ByteRcvr();
				if (tempRxD == 0xAA)
				{
					owTxCnt = 0;
					owState = IDLE_STATE;
				}
			}
		}
		else if (owMemCmd == READ_MEMORY)
		{
			if (owTxCnt < 2)
			{
				onewire_ByteXmit(owPkt.MemArray[owTxCnt]);
				owTxCnt ++;
				if (owTxCnt == 2)
				{
					owRxCnt = 0;
				}
			}
			else
			{
				owRdMemData[owRxCnt] = onewire_ByteRcvr();
				owRxCnt ++;
				if (owRxCnt == MAX_DATA_LEN)
				{
					owTxCnt = 0;
					owRxCnt = 0;
					owState = IDLE_STATE;
				}
			}
			
		}
	}
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void onewire_RomCodeToBit(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
static void onewire_RomCodeToBit(void)
{
	U8_T	i, j;

	for (i = 0 ; i < 8 ; i ++)
	{
		for (j = 0 ; j < 8 ; j ++)
		{
			owRomBitArray[8 * i + j] = (owPkt.RomArray[i] >> j) & BIT0;
		}
	}
}


/* EXPORTED SUBPROGRAM BODIES */

/*
 *--------------------------------------------------------------------------------
 * void ONEWIRE_RomCmdChk(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void ONEWIRE_RomCmdChk(void)
{
	U8_T	cmdRegVal = 0;

		if (onewire_ByteXmit(owRomCmd))
		{
			switch (owRomCmd)
			{
				case READ_ROM :
					owRcFlag = 0;
					owRxCnt = 0;
					owState = ROM_SEQ_STATE;
					break;
				#if MULTI_DEVICES
				case MATCH_ROM :
					owRcFlag = 0;
					owState = ROM_SEQ_STATE;
					onewire_RomCodeToBit();
				//	ONEWIRE_SetCmdReg(OW_BIT_CTRL);
					break;
				#endif
				case SEARCH_ROM :
					owRcFlag = 0;
					owState = ROM_SEQ_STATE;
				//	ONEWIRE_SetCtrlReg(OW_SRA);
					/* initialize for search */
				//	owLastDiscrepancy = 0;
				//	owLastDeviceFlag = FALSE;
				//	owLastFamilyDiscrepancy = 0;
					break;
				case SKIP_ROM :
					owRcFlag = 0;
					owState = MEM_CMD_STATE;
					break;
				case RESUME_ROM :
					if (owRcFlag == 1)
					{
						owState = MEM_CMD_STATE;
					}
					else
					{
						ONEWIRE_Reset();
					}
					break;
				case OD_SKIP_ROM :
					owRcFlag = 0;
					owOdFlag = 1;
					owState = MEM_CMD_STATE;
					/* Setup OneWire Command mode */
					ONEWIRE_Cmd(SI_RD, OWCMDR, &cmdRegVal);
					cmdRegVal = cmdRegVal | OW_OD;
					ONEWIRE_Cmd(SI_WR, OWCMDR, &cmdRegVal);
					break;
				#if MULTI_DEVICES
				case OD_MATCH_ROM :
					owRcFlag = 0;
					owOdFlag = 1;
					owState = ROM_SEQ_STATE;
				//	ONEWIRE_SetCmdReg(OW_BIT_CTRL);
					/* Setup OneWire Command mode */
					ONEWIRE_Cmd(SI_RD, OWCMDR, &cmdRegVal);
					cmdRegVal = cmdRegVal | OW_OD;
					ONEWIRE_Cmd(SI_WR, OWCMDR, &cmdRegVal);
					break;
				#endif
			}
		}

}

/*
 *--------------------------------------------------------------------------------
 * void ONEWIRE_Setup(U8_T ctrlCmd1, U8_T ctrlCmd2, U8_T intrEnb, U8_T clkDiv)
 * Purpose : Setup the operation mode of One Wire.
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void ONEWIRE_Setup(U8_T ctrlCmd1, U8_T ctrlCmd2, U8_T intrEnb, U8_T clkDiv)
{
	U16_T	i;

	owPkt.RomCmd = 0;
	owPkt.MemCmd = 0;
	for (i=0 ; i<8 ; i++)
		owPkt.RomArray[i] = 0;
	for (i=0 ;  i<8 ; i++)
		owPkt.MemArray[i] = 0;
	owRomCmd = 0;
	owMemCmd = 0;
	owRcFlag = 0;
	owOdFlag = 0;
	owTxCnt = 0;
	owRxCnt = 0;
	owState = 0;
	owRdByte = 0;
	owCrc8 = 0;
	owCrc16 = 0;
	owIntrFlag = 0;
	owIntrEnbFlag = 0;

	owLastDiscrepancy = 0;
	owLastDeviceFlag = 0;
	owLastFamilyDiscrepancy = 0;
	owSearchResult = 0;
	owSerialBufNum = 0;

	for (i=0 ; i<64 ; i++)
		owRomBitArray[i] = 0;
	for (i=0 ; i<144 ; i++)	
		owRdMemData[i] = 0;

	/* Setup OneWire Control mode */
	ONEWIRE_SetCtrlReg(ctrlCmd1);
	/* Setup OneWire Command mode */
	ONEWIRE_SetCmdReg(ctrlCmd2);	
	/* Enable intertupe flag type of OneWire */
	onewire_IntrBitsEnb(intrEnb);
	/* OneWire clock rate selection */
	ONEWIRE_SetClk(clkDiv);
}

/*
 *--------------------------------------------------------------------------------
 * void ONEWIRE_SetClk(U8_T clkDiv)
 * Purpose : Setup the clock devide of One Wire.
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void ONEWIRE_SetClk(U8_T clkDiv)
{
	/* OneWire clock rate selection */
	ONEWIRE_Cmd(SI_WR, OWCLKDIV, &clkDiv);
}

/*
 *--------------------------------------------------------------------------------
 * void ONEWIRE_SetCtrlReg(U8_T ctrlReg)
 * Purpose : Setup the control register of One Wire.
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void ONEWIRE_SetCtrlReg(U8_T ctrlReg)
{
	/* Setup OneWire Control mode */
	ONEWIRE_Cmd(SI_WR, OWCTRLR, &ctrlReg);
}

/*
 *--------------------------------------------------------------------------------
 * void ONEWIRE_SetCmdReg(U8_T cmdReg)
 * Purpose : Setup the command register of One Wire.
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void ONEWIRE_SetCmdReg(U8_T cmdReg)
{
	U8_T	regVal;

	ONEWIRE_Cmd(SI_RD, OWCMDR, &regVal);
	regVal = regVal | cmdReg;
	/* Setup OneWire Command mode */
	ONEWIRE_Cmd(SI_WR, OWCMDR, &regVal);
}

/*
 *--------------------------------------------------------------------------------
 * void ONEWIRE_Func(void)
 * Purpose : Handling serial interface OneWire interrupt function.
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void ONEWIRE_Func(void)
{
	U8_T	owStatus;

		/* Disable Transmit Buffer Empty Interrupt of OneWire */
		onewire_IntrBitsDisb(OW_RBFI_ENB | OW_RSRFI_ENB | OW_TBEI_ENB | OW_TSREI_ENB);
	
		ONEWIRE_Cmd(SI_RD, OWISR, &owStatus);

		if (owStatus & OW_PD)
		{
			owRomCmd = owPkt.RomCmd;
			owMemCmd = owPkt.MemCmd;
			owTxCnt  = 0;
			owRxCnt  = 0;
			owState = ROM_CMD_STATE;
			owIntrFlag = OW_PDR;
		}
		if ((owStatus & OW_RBF) && (owIntrEnbFlag & OW_RBFI_ENB))
		{
			owIntrFlag |= OW_RBF;
		}
		if ((owStatus & OW_TSRE) && (owIntrEnbFlag & OW_TSREI_ENB))
		{
			owIntrFlag |= OW_TSRE;
		}
}

/*
 *--------------------------------------------------------------------------------
 * void ONEWIRE_SearchRom(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void ONEWIRE_SearchRomFunc(void)
{
//	U16_T	i;
	U16_T	bitNumber = 1;
	U16_T	lastZero = 0;
	U16_T	romByteNumber = 0;
	U16_T	romBit = 0;
	U16_T	cmpRomBit = 0;
	U8_T	romByteMask = 1;
	U8_T	searchBit = 0;
	U8_T	lastOwCrc8 = 0;

	bitNumber = 1;
	lastZero = 0;
	romByteNumber = 0;
	romByteMask = 1;
	owSearchResult = 0;
	ONEWIRE_SetCrc8(0);
	// if the last search was not the last one
//	if (1)
//	{
		ONEWIRE_SetCmdReg(OW_BIT_CTRL);
		// loop to do the search
		do // loop until through all ROM bytes 0-7
		{
			// read a bit and its complement
			romBit = onewire_ByteRcvr();
			cmpRomBit = onewire_ByteRcvr();
			// check for no devices on 1-wire
			if ((romBit == 1) && (cmpRomBit == 1))
				ONEWIRE_Reset();
			else
			{
				// all devices coupled have 0 or 1
				if (romBit != cmpRomBit)
					searchBit = romBit; // bit write value for search
				else
				{
					// if this discrepancy is before the Last Discrepancy
					// on a previous next then pick the same as last time
					if (bitNumber < owLastDiscrepancy)
						searchBit = ((owPkt.RomArray[romByteNumber] & romByteMask) > 0);
					else
					// if equal to last pick 1, if not then pick 0
						searchBit = (bitNumber == owLastDiscrepancy);
					// if 0 was picked then record its position in LastZero
					if (searchBit == 0)
					{
						lastZero = bitNumber;
						// check for Last discrepancy in family
						if (lastZero < 9)
							owLastFamilyDiscrepancy = lastZero;
					}
				}
				// set or clear the bit in the ROM byte rom_byte_number
				// with mask rom_byte_mask
				if (searchBit == 1)
					owPkt.RomArray[romByteNumber] |= romByteMask;
				else
					owPkt.RomArray[romByteNumber] &= ~romByteMask;
				// serial number search direction write bit
				if (!onewire_ByteXmit(searchBit))
					owSearchResult = FALSE;
				// increment the byte counter id_bit_number
				// and shift the mask rom_byte_mask
				bitNumber ++;
				romByteMask <<= 1;
				// if the mask is 0 then go to new SerialNum byte RomByteNumber and reset mask
				if (romByteMask == 0)
				{
					lastOwCrc8 = ONEWIRE_DoCrc8(owPkt.RomArray[romByteNumber]); // accumulate the CRC
					romByteNumber ++;
					romByteMask = 1;
				}
			}
		}
		while (romByteNumber < 8) ;
		ONEWIRE_SetCmdReg(0);
		// if the search was successful then
		if ((bitNumber > 64) && (lastOwCrc8 == 0))
		{
			// search successful so set owLastDiscrepancy,owLastDeviceFlag,search_result
			owLastDiscrepancy = lastZero;
			// check for last device
			if (owLastDiscrepancy == 0)
			{
				owLastDeviceFlag = 1;
			}

			owSearchResult = TRUE;
		}
//	}
}

/*
 *--------------------------------------------------------------------------------
 * BOOL ONEWIRE_StateChk(U8_T state)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
BOOL ONEWIRE_StateChk(U8_T state)
{
	if (owState & state)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/*
 *--------------------------------------------------------------------------------
 * BOOL ONEWIRE_LastDeviceChk(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
BOOL ONEWIRE_LastDeviceChk(void)
{
	if (owLastDeviceFlag)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/*
 *--------------------------------------------------------------------------------
 * void ONEWIRE_StateFunc(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void ONEWIRE_StateFunc(void)
{
	if (owState & ROM_SEQ_STATE)
	{
		onewire_RomFunc();
	}
	else if (owState & ROM_CMD_STATE)
	{
		ONEWIRE_RomCmdChk();
	}
#if MEMORY_SECTIONS
	else if (owState & MEM_SEQ_STATE)
	{
		onewire_MemFunc();
	}
	else if (owState & MEM_CMD_STATE)
	{
		onewire_MemCmdChk();
	}
#endif
}

/*
 *--------------------------------------------------------------------------------
 * BOOL ONEWIRE_Reset(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
BOOL ONEWIRE_Reset(void)
{
	U8_T	resetCmd = 0;

	owState = ROM_CMD_STATE;
	owIntrFlag = 0;
	resetCmd = OW_RESET;
	ONEWIRE_Cmd(SI_WR, OWCTRLR, &resetCmd);

	while (1)
	{
		if (owIntrFlag & OW_PDR)
			break;
	}

	owIntrFlag = 0;

	ONEWIRE_Cmd(SI_RD, OWISR, &resetCmd);

	return (resetCmd & OW_PDR) ? FALSE : TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * void ONEWIRE_SetCrc8(U8_T setVal)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void ONEWIRE_SetCrc8(U8_T setVal)
{
   owCrc8 = setVal;
}

/*
 *--------------------------------------------------------------------------------
 * void ONEWIRE_SetCrc16(U16_T setVal)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void ONEWIRE_SetCrc16(U16_T setVal)
{
   owCrc16 = setVal;
}

/*
 *--------------------------------------------------------------------------------
 * void ONEWIRE_StartSearch(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void ONEWIRE_StartSearch(void)
{
	owLastDiscrepancy = 0;
	owLastDeviceFlag = 0;
	owLastFamilyDiscrepancy = 0;
	owSerialBufNum = 0;
}

/*
 *--------------------------------------------------------------------------------
 * U8_T ONEWIRE_DoCrc8(U8_T byteData)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
U8_T ONEWIRE_DoCrc8(U8_T byteData)
{
	owCrc8 = owCrc8Table[owCrc8 ^ byteData];
	return owCrc8;
}

/*
 *--------------------------------------------------------------------------------
 * U16_T DoCrc16(U16_T wordData)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
U16_T ONEWIRE_DoCrc16(U16_T wordData)
{
	wordData = (wordData ^ (owCrc16 & 0x00FF)) & 0x00FF;
	owCrc16 >>= 8;

	if (owOddParity[wordData & 0x000F] ^ owOddParity[wordData >> 4])
		owCrc16 ^= 0xC001;

	wordData <<= 6;
	owCrc16 ^= wordData;
	wordData <<= 1;
	owCrc16 ^= wordData;

	return owCrc16;
}

/*
 *--------------------------------------------------------------------------------
 * void ONEWIRE_SendPkt(OW_BUF *ptSend)
 * Purpose : 
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void ONEWIRE_SendPkt(OW_BUF *ptSend)
{
	owPkt = *ptSend;
}

/*
 *--------------------------------------------------------------------------------
 * OW_BUF *ONEWIRE_GetCurBuf(void)
 * Purpose : 
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
OW_BUF *ONEWIRE_GetCurBuf(void)
{
//	for (i=0 ; i<8 ; i++)
//		P0=owPkt.RomArray[i];
	return (&owPkt);
}

#if MEMORY_SECTIONS
/*
 *--------------------------------------------------------------------------------
 * U8_T *ONEWIRE_GetRdMemData(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
U8_T *ONEWIRE_GetRdMemData(void)
{
	return (&owRdMemData[0]);
}
#endif

/*
 *--------------------------------------------------------------------------------
 * U8_T *ONEWIRE_GetRomCode(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
U8_T *ONEWIRE_GetRomCode(void)
{
	return (&owRomCodeBuf[0]);
}

/*
 *--------------------------------------------------------------------------------
 * BOOL ONEWIRE_Search(U8_T *ptRom)
 * Purpose : Handling serial interface OneWire interrupt function.
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
BOOL ONEWIRE_Search(U8_T *ptRom)
{
	U16_T	i, j;
	OW_BUF *ptSearch = NULL;

	for (j=0 ; j<MAX_SERIAL_BUF_NUM ; j++)
	{
		for (i=0 ; i<8 ; i++)
		{
			owRomCodeBuf[i+(j*8)] = 0;
		}
	}
	i=0;
	j=0;
	ptSearch = (OW_BUF *)GetPktBuf();

	ptSearch->RomCmd = SEARCH_ROM;
	for (i=0 ; i<8 ; i++)
		ptSearch->RomArray[i] = 0;
	ptSearch->MemCmd = 0;
	for (i=0 ; i<8 ; i++)
		ptSearch->MemArray[i] = 0;
	ONEWIRE_SendPkt(ptSearch);
	ONEWIRE_StartSearch();

	while (1)
	{
		if (!ONEWIRE_Reset())
			break;
		while (1)
		{
			ONEWIRE_StateFunc();
			if (ONEWIRE_StateChk(IDLE_STATE) == TRUE)
				break;
		}
		ptSearch = ONEWIRE_GetCurBuf();

		for (i=0 ; i<8 ; i++)
		{
			*(ptRom + (i+(j*8))) = ptSearch->RomArray[i];
			owRomCodeBuf[i+(j*8)] = ptSearch->RomArray[i];
		}
		j ++;

		if (j == MAX_SERIAL_BUF_NUM)
			break;
		if (ONEWIRE_LastDeviceChk())
			break;
	}
	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * BOOL ONEWIRE_Match(OW_BUF *ptMatch)
 * Purpose : Handling serial interface OneWire interrupt function.
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if MULTI_DEVICES
BOOL ONEWIRE_Match(OW_BUF *ptMatch)
{
	ONEWIRE_SendPkt(ptMatch);

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

/*
 *--------------------------------------------------------------------------------
 * BOOL ONEWIRE_ReadRom(U8_T *ptRom)
 * Purpose : Handling serial interface OneWire interrupt function.
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
BOOL ONEWIRE_ReadRom(U8_T *ptRom)
{
	U16_T	i;
	OW_BUF XDATA	*ptReadRom = NULL;

	ptReadRom = (OW_BUF *)GetPktBuf();

	ptReadRom->RomCmd = READ_ROM;
	for (i=0 ; i<8 ; i++)
		ptReadRom->RomArray[i] = 0;
	ptReadRom->MemCmd = 0;

	ONEWIRE_SendPkt(ptReadRom);

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

	ptReadRom = ONEWIRE_GetCurBuf();
	for (i=0 ; i<8 ; i++)
	{
		*(ptRom + i) = ptReadRom->RomArray[i];
	}

	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * BOOL ONEWIRE_OdSkip(OW_BUF *ptOdSkip)
 * Purpose : Handling serial interface OneWire interrupt function.
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
BOOL ONEWIRE_OdSkip(OW_BUF *ptOdSkip)
{
	ONEWIRE_SendPkt(ptOdSkip);

	if (!ONEWIRE_Reset())
		return FALSE;

	while (1)
	{
		ONEWIRE_StateFunc();
		if (ONEWIRE_StateChk(MEM_CMD_STATE) == TRUE)
			break;
	}
	
	return TRUE;
}

/*
 *--------------------------------------------------------------------------------
 * void ONEWIRE_Cmd(U8_T cmdType, U8_T owCmdIndex, U8_T *owData)
 * Purpose : Accessing the OneWire interface indirectly through OneWire's SFR.
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void ONEWIRE_Cmd(U8_T cmdType, U8_T owCmdIndex, U8_T *owData)
{
	if (cmdType == SI_WR)
	{
		OWDR = *owData;
		OWCIR = owCmdIndex;
	}
	else if (cmdType == SI_RD)
	{
		OWCIR = owCmdIndex;
		*owData = OWDR;
	}
}

/*
 *--------------------------------------------------------------------------------
 * void ONEWIRE_Post(void)
 * Purpose : Handling serial interface OneWire interrupt function.
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void ONEWIRE_Post(void)
{
}


/* End of onewire.c */
