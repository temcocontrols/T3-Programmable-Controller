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
 * Module Name : spi.c
 * Purpose     : This file handles the I2C serial interface driver.
 * Author      : Robin Lee
 * Date        : 2005-03-31
 * Notes       :
 * $Log: spi.c,v $
 * Revision 1.10  2005/11/24 12:51:51  robin6633
 * initiate values of use.
 *
 * Revision 1.9  2005/08/17 06:48:22  robin6633
 * no message
 *
 * Revision 1.8  2005/08/11 09:00:06  borbin
 * no message
 *
 * Revision 1.7  2005/08/03 03:41:39  robin6633
 * Extended the spi receive length.
 *
 * Revision 1.6  2005/07/27 05:18:30  robin6633
 * Re-order the burst read/write command address byte of slave mode.
 *
 * Revision 1.5  2005/07/21 12:14:14  robin6633
 * Fixed the software receive buffer of single write/read SFR command process.
 *
 * Revision 1.4  2005/07/21 02:56:48  robin6633
 * Change the opcode definition of slave mode instruction set.
 *
 * Revision 1.3  2005/07/16 02:58:07  robin6633
 * Add Slave mode function
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
#include "product.h"

#if ASIX_MINI


#include	"reg80390.h"
#include	"types.h"
#include	"spi.h"

#if SPI_SLAVE_ENABLE
#include	"console_debug.h"
#endif

#include "stdio.h"


/* STATIC VARIABLE DECLARATIONS */
static U8_T		spiActF = 0;
static U8_T		spiCtrl = 0;
static U8_T		spiPktDir = 0;
static U8_T		spiPktLen = 0;
static U16_T	spiLenCnt = 0;
static U16_T	spiTransLoop = 0;
static U16_T	spiTransLoopCnt = 0;
static U8_T		spiRxBuf[4] = {0,0,0,0};
static U8_T		spiSlvTxBuf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static U8_T		spiSlvRxBuf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};


/* LOCAL SUBPROGRAM DECLARATIONS */
static void	spi_MstRcvr(void);
static void spi_SlvRcvr(void);

#if SPI_SLAVE_ENABLE
static void spi_SlvProcess(void);
#endif


/* LOCAL SUBPROGRAM BODIES */

/*
 *--------------------------------------------------------------------------------
 * static void spi_MstRcvr(void)
 * Purpose : SPI master get data from receive queue in one transfer.
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
static void spi_MstRcvr(void)
{
	U8_T	i;
	//P0_5 = 0;
	SPICIR = SPIRBR;
	for (i = 0 ; i < 4; i++ )
	{
		spiRxBuf[i] = SPIDR;
	}

}

/*
 *--------------------------------------------------------------------------------
 * static void spi_SlvRcvr(void)
 * Purpose : SPI receive function for one transfer smaller than 32 bits.
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if SPI_SLAVE_ENABLE
static void spi_SlvRcvr(void)
{
	U8_T	i;
	U8_T	rxLen = 0;
	U8_T	slvCmdType = 0;
	U8_T	dataLen = 0;

	SPICIR = SPISB;
	spiSlvRxBuf[0] = SPIDR;
	slvCmdType = spiSlvRxBuf[0] & 0xF0;
	dataLen = (spiSlvRxBuf[0] & 0x0F) + 1;
	if (slvCmdType == SPI_SLV_SRSFR)
	{
		rxLen = 1;
	}
	else if (slvCmdType == SPI_SLV_SWSFR)
	{
		rxLen = datalen + 1;
	}
	else if (slvCmdType == SPI_SLV_IRSFR)
	{
		rxLen = 2;
	}
	else if (slvCmdType == SPI_SLV_IWSFR)
	{
		rxLen = datalen + 2;
	}
	else if (slvCmdType == SPI_SLV_BRMEM)
	{
		rxLen = 3;
	}
	else if (slvCmdType == SPI_SLV_BWMEM)
	{
		rxLen = datalen + 3;
	}
	for (i = 0 ; i < rxLen ; i ++ )
	{
		spiSlvRxBuf[1+i] = SPIDR;
	}
	/* Command abort*/
	SPICIR = 0xFF;
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void spi_SlvProcess(void)
 * Purpose : SPI transmit function for one transfer smaller than 32 bits.
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if SPI_SLAVE_ENABLE
static void spi_SlvProcess(void)
{
	U8_T	slvCmdType = 0;
	U8_T	sfrAddr = 0;
	U8_T	cmdIndReg = 0;
	U8_T	realReg = 0;
	U8_T	realLen = 0;
	U32_T	memAddr = 0;
	U32_T	tempData = 0;
	U16_T	i;

	slvCmdType = spiSlvRxBuf[0] & 0xF0;
	if (slvCmdType == SPI_SLV_SRSFR)
	{
		realLen = (spiSlvRxBuf[0] & 0x07) + 1;
		sfrAddr = spiSlvRxBuf[1];
		for (i=0 ; i<realLen ; i++)
		{
			CLI_SfrRd((U32_T)sfrAddr, &tempData);
			spiSlvTxBuf[i] = (U8_T)tempData;
		}
		SPI_SlvXmit(SPI_SLV_RDY);
	}
	else if (slvCmdType == SPI_SLV_SWSFR)
	{
		realLen = (spiSlvRxBuf[0] & 0x07) + 1;
		sfrAddr = spiSlvRxBuf[1];
		for (i=0 ; i<realLen ; i++)
		{
			tempData = (U32_T)spiSlvRxBuf[2+i];
			CLI_SfrWr((U32_T)sfrAddr, tempData);
		}
		SPI_SlvXmit(SPI_SLV_RDY);
	}
	else if (slvCmdType  == SPI_SLV_IRSFR)
	{
		cmdIndReg = spiSlvRxBuf[1];
		realReg = spiSlvRxBuf[2];
		realLen = (spiSlvRxBuf[0] & 0x0F) + 1;
		if (cmdIndReg == SFR_SPICIR)
		{
			CLI_SpiRd((U32_T)realReg, &spiSlvTxBuf[0], (U8_T)realLen);
		}
		else if (cmdIndReg == SFR_I2CCIR)
		{
			CLI_I2cRd((U32_T)realReg, &spiSlvTxBuf[0], (U8_T)realLen);
		}
		else if (cmdIndReg == SFR_OWCIR)
		{
			CLI_OwRd((U32_T)realReg, &spiSlvTxBuf[0], (U8_T)realLen);
		}
		else if (cmdIndReg == SFR_CANCIR)
		{
			CLI_CanRd((U32_T)realReg, &spiSlvTxBuf[0], (U8_T)realLen);
		}
		else if (cmdIndReg == SFR_TCIR)
		{
			CLI_ToeRd((U32_T)realReg, &spiSlvTxBuf[0], (U8_T)realLen);
		}
		else if (cmdIndReg == SFR_MCIR)
		{
			CLI_MacRd((U32_T)realReg, &spiSlvTxBuf[0], (U8_T)realLen);
		}
		SPI_SlvXmit(SPI_SLV_RDY);
	}
	else if (slvCmdType  == SPI_SLV_IWSFR)
	{
		cmdIndReg = spiSlvRxBuf[1];
		realReg = spiSlvRxBuf[2];
		realLen = (spiSlvRxBuf[0] & 0x0F) + 1;
		if (cmdIndReg == SFR_SPICIR)
		{
			CLI_SpiWr((U32_T)realReg, &spiSlvRxBuf[3], (U8_T)realLen);
		}
		else if (cmdIndReg == SFR_I2CCIR)
		{
			CLI_I2cWr((U32_T)realReg, &spiSlvRxBuf[3], (U8_T)realLen);
		}
		else if (cmdIndReg == SFR_OWCIR)
		{
			CLI_OwWr((U32_T)realReg, &spiSlvRxBuf[3], (U8_T)realLen);
		}
		else if (cmdIndReg == SFR_CANCIR)
		{
			CLI_CanWr((U32_T)realReg, &spiSlvRxBuf[3], (U8_T)realLen);
		}
		else if (cmdIndReg == SFR_TCIR)
		{
			CLI_ToeWr((U32_T)realReg, &spiSlvRxBuf[3], (U8_T)realLen);
		}
		else if (cmdIndReg == SFR_MCIR)
		{
			CLI_MacWr((U32_T)realReg, &spiSlvRxBuf[3], (U8_T)realLen);
		}
		SPI_SlvXmit(SPI_SLV_RDY);
	}
	else if (slvCmdType  == SPI_SLV_BRMEM)
	{
		memAddr = ((U32_T)spiSlvRxBuf[3] << 16) | ((U32_T)spiSlvRxBuf[2] << 8) | ((U32_T)spiSlvRxBuf[1]);
		realLen = (spiSlvRxBuf[0] & 0x0F) + 1;
		for (i = 0 ; i < realLen ; i ++)
		{
			CLI_ExtMemRd((memAddr + i), &tempData);
			spiSlvTxBuf[i] = (U8_T)tempData;
		}
		SPI_SlvXmit(SPI_SLV_RDY);
	}
	else if (slvCmdType  == SPI_SLV_BWMEM)
	{
		memAddr = ((U32_T)spiSlvRxBuf[3] << 16) | ((U32_T)spiSlvRxBuf[2] << 8) | ((U32_T)spiSlvRxBuf[1]);
		realLen = (spiSlvRxBuf[0] & 0x0F) + 1;
		for (i = 0 ; i < realLen ; i ++)
		{
			tempData = (U32_T)spiSlvRxBuf[4 + i];
			CLI_ExtMemWr((memAddr + i), tempData);
		}
		SPI_SlvXmit(SPI_SLV_RDY);
	}
}
#endif

void SPI_CS_SET(void)
{
    U8_T XDATA ss0=0xff;
		SPI_Cmd(SI_WR, SPISSR, &ss0);	
}

void SPI_CS_CLR(void)
{
	 U8_T XDATA ss0=0xfe;
		SPI_Cmd(SI_WR, SPISSR, &ss0);	
}

/* EXPORTED SUBPROGRAM BODIES */

/*
 *--------------------------------------------------------------------------------
 * void SPI_Setup(U8_T ctrlCmd, U8_T intrEnb, U8_T baudrate, U8_T slvSel)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void SPI_Setup(U8_T ctrlCmd, U8_T intrEnb, U8_T baudrate, U8_T slvSel)
{
	U8_T	spiSlvCmd = 0;
	U16_T	i;


	spiActF = 0;
	spiCtrl = 0;
	spiPktDir = 0;
	spiPktLen = 0;
	spiLenCnt = 0;
	spiTransLoop = 0;
	spiTransLoopCnt = 0;
	for (i=0 ; i<4 ; i++)
		spiRxBuf[i] = 0;
	for (i=0 ; i<16 ; i++)
	{
		spiSlvTxBuf[i] = 0;
		spiSlvRxBuf[i] = 0;
	}

	/* Record the SPI control mode */
	spiCtrl = intrEnb;
	/* Setup SPI mode */
	SPI_Cmd(SI_WR, SPICTRLR, &ctrlCmd);
	/* Enable intertupe flag type of SPI */
	SPI_Cmd(SI_WR, SPIIER, &intrEnb);
	/* SPI baud rate selection */
	SPI_Cmd(SI_WR, SPIBRR, &baudrate);
	/* SPI slave select */
	SPI_Cmd(SI_WR, SPISSR, &slvSel);
	/* SPI slave is ready to receive */
	spiSlvCmd = SPI_SLV_RDY;
	SPI_Cmd(SI_WR, SPISCR, &spiSlvCmd);


}

/*
 *--------------------------------------------------------------------------------
 * void SPI_Func(void)
 * Purpose : Handling serial interface SPI interrupt function.
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void SPI_Func(void)
{
	U8_T	spiStatus = 0;
	/* Take down the interrupt type */
	EA = 0;
	SPI_Cmd(SI_RD, SPIISR, &spiStatus);
	EA = 1;
	if ((spiCtrl & SPI_STCFIE) || (spiCtrl & SPI_SRCFIE))
	{
		/* Read SPI interrupt status register */
		//printf("send\r\n");
		if (spiStatus & SPI_MCF) // when master complete a transfer
		{
			//P0_5 = 1;
			spi_MstRcvr();
			SPI_FlagClr(SPI_BUSY);
		}

		#if SPI_SLAVE_ENABLE
		else if (spiStatus & SPI_SCF) // when slave complete a transfer
		{
			spi_SlvRcvr();
			spi_SlvProcess();
		}
		#endif
	}
	else
	{
		//printf("receive\r\n");
		if (spiStatus & SPI_MCF) // when master complete a transfer
		{
			spi_MstRcvr();
			SPI_FlagClr(SPI_BUSY);
		}
		#if SPI_SLAVE_ENABLE
		else if (spiStatus & SPI_SCF) // when slave complete a transfer
		{
			spi_SlvRcvr();
			spi_SlvProcess();
		}
		#endif
	}
}

/*
 *--------------------------------------------------------------------------------
 * void SPI_MstXmit(U8_T *ptSpiTxPkt, U8_T xmitBit, U8_T cmd)
 * Purpose : SPI transmit function for one transfer smaller than 32 bits.
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void SPI_MstXmit(U8_T *ptSpiTxPkt, U8_T xmitBit, U8_T cmd)
{
	U16_T	i;
	U8_T	xmitCmd = 0;
	
	for (i = 0 ; i <= (xmitBit-1)/8 ; i++ )
	{
		SPIDR = *(ptSpiTxPkt + i);
	}
	SPICIR = SPITBR;
	/* order command */
	xmitCmd = ((xmitBit - 1)|cmd);
	SPI_Cmd(SI_WR, SPICMDR, &xmitCmd);
}

/*
 *--------------------------------------------------------------------------------
 * void SPI_SlvXmit(U8_T spiSlvCmd)
 * Purpose : SPI transmit function for one transfer smaller than 32 bits.
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void SPI_SlvXmit(U8_T spiSlvCmd)
{
	U16_T	i;

	for (i = 0 ; i <16 ; i++ )
	{
		SPIDR = spiSlvTxBuf[i];
	}
	SPICIR = SPISB;
	/* order command */
	SPI_Cmd(SI_WR, SPISCR, &spiSlvCmd);
}

/*
 *--------------------------------------------------------------------------------
 * BOOL SPI_FlagChk(U8_T chkBit)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
BOOL SPI_FlagChk(U8_T chkBit)
{
	if (spiActF & chkBit)
		return TRUE;
	else
		return FALSE;
}

/*
 *--------------------------------------------------------------------------------
 * void SPI_FlagEnb(U8_T enbBit)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void SPI_FlagEnb(U8_T enbBit)
{
	spiActF = spiActF | enbBit;
}

/*
 *--------------------------------------------------------------------------------
 * void SPI_FlagClr(U8_T clrBit)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void SPI_FlagClr(U8_T clrBit)
{
	spiActF = spiActF & ~clrBit;
}

/*
 *--------------------------------------------------------------------------------
 * void SPI_GetData(U8_T *ptBuf)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void SPI_GetData(U8_T *ptBuf)
{
	U8_T	i;

	for (i=0 ; i<4 ; i++)
	{
		*(ptBuf + i) = spiRxBuf[i];
	}
//	printf("int %d %d %d %d\r\n",(int)spiRxBuf[0],(int)spiRxBuf[1],(int)spiRxBuf[2],(int)spiRxBuf[3]);

}

/*
 *--------------------------------------------------------------------------------
 * void SPI_Cmd(U8_T cmdType, U8_T spiCmdIndex, U8_T *spiData)
 * Purpose : Accessing the SPI interface indirectly through SPI's SFR.
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void SPI_Cmd(U8_T cmdType, U8_T spiCmdIndex, U8_T *spiData)
{
	if (cmdType == SI_WR)
	{
		SPIDR = *spiData;
		SPICIR = spiCmdIndex;
	}
	else if (cmdType == SI_RD)
	{
		SPICIR = spiCmdIndex;
		*spiData = SPIDR;
	}
}

/*
 *--------------------------------------------------------------------------------
 * void SPI_Post(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void SPI_Post(void)
{
}

#endif
/* End of spi.c */
