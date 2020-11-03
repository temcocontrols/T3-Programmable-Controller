/**
  ******************************************************************************
  * @file    SdioEmmcDrive.c
  * @author  Luoxianhui && Lifuqiang 
  * @version V1.0.0
  * @date    12/27/2013
  * @brief   This file is SdioEmmcDrive file.
  ******************************************************************************
  *          =========================
  *          +-----------------------------------------------------------+
  *          |                     Pin assignment                        |
  *          +-----------------------------+---------------+-------------+
  *          |  STM32 SDIO Pins            |   EMMC        |    Pin      |
  *          +-----------------------------+---------------+-------------+
  *          |      SDIO D2                |   D2          |    1        |
  *          |      SDIO D3                |   D3          |    2        |
  *          |      SDIO CMD               |   CMD         |    3        |
  *          |                             |   VCC         |    4 (3.3 V)|
  *          |      SDIO CLK               |   CLK         |    5        |
  *          |                             |   GND         |    6 (0 V)  |
  *          |      SDIO D0                |   D0          |    7        |
  *          |      SDIO D1                |   D1          |    8        |
  *          +-----------------------------+---------------+-------------+
  *
  *  @endverbatim
  */
	
/** @defgroup SDIO_EMMC_Private_Types
* @{
*/ 
/* Includes ------------------------------------------------------------------*/
#include "SdioEmmcDrive.h"
//#include "includes.h"
#include "define.h"
#include <string.h>
#include <stdio.h>
#include "main.h"
#include "delay.h"


#if 0//(SD_BUS == SD_BUS_TYPE)

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
SDIO_InitTypeDef SDIO_InitStructure;
SDIO_CmdInitTypeDef SDIO_CmdInitStructure;
SDIO_DataInitTypeDef SDIO_DataInitStructure;
__IO INT32U StopCondition = 0;
__IO INT32U TransferEnd = 0;
__IO INT32U DMAEndOfTransfer = 0;
__IO EmmcError TransferError;
EmmcCardInfo MyEmmcCardInfo;
static INT32U CardType =  SDIO_STD_CAPACITY_SD_CARD_V1_1;
//static INT8U SDSTATUS_Tab[16];
/* Private functions ---------------------------------------------------------*/
EmmcError EmmcInit(void);
EmmcError CmdError(void);
EmmcError CmdResp1Error(INT8U cmd);
EmmcError CmdResp7Error(void);
EmmcError CmdResp3Error(void);
EmmcError CmdResp2Error(void);
EmmcError CmdResp6Error(INT8U cmd, INT16U *prca);
EmmcError EmmcEnWideBus(FunctionalState NewState);
EmmcError IsCardProgramming(INT8U *pstatus);
EmmcError EmmcPowerON(void);
EmmcError EmmcInitializeCards(EmmcCardInfo *E);
EmmcError EmmcSendStatus(INT32U *pcardstatus);
EmmcError EmmcStopTransfer(void);
EmmcError EmmcGetCardInfo(EmmcCardInfo *E, INT32U *CSD_Tab, INT32U *CID_Tab, INT16U Rca);
EmmcError EmmcSelectDeselect(INT32U addr);
EmmcError EmmcProcessIRQSrc(void);
EmmcError EmmcReadExtCsd(EmmcCardInfo *E);
EmmcError EmmcEnableWideBusOperation(INT32U WideMode);
EmmcError EmmcReadBlock(INT8U *readbuff, INT32U ReadAddr, INT16U BlockSize);
EmmcError EmmcReadMultiBlocks(INT8U *readbuff, INT32U ReadAddr, INT16U BlockSize, INT32U NumberOfBlocks);
EmmcError EmmcWriteBlock(INT8U *writebuff, INT32U WriteAddr, INT16U BlockSize);
EmmcError EmmcWriteMultiBlocks(INT8U *writebuff, INT32U WriteAddr, INT16U BlockSize, INT32U NumberOfBlocks);
EmmcError EmmcErase(INT32U startaddr, INT32U endaddr);
EmmcCardState EmmcGetState(void);
EmmcError EmmcWaitReadOperation(void);
EmmcError EmmcWaitWriteOperation(void);
void EmmcProcessDMAIRQ(void);
void EmmcLowLevelDMATxConfig(INT32U *BufferSRC, INT32U BufferSize);
void EmmcLowLevelDMARxConfig(INT32U *BufferDST, INT32U BufferSize);
INT8U convert_from_bytes_to_power_of_two(INT16U NumberOfBytes);
/**
  * @}
  */ 

/**
  * @brief  Configures SDIO IRQ channel.
  * @param  None
  * @retval None
  */
void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure the NVIC Preemption Priority Bits */
 
  NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = DMA2_Channel4_5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_Init(&NVIC_InitStructure);  
}

/**
  * @brief  Initializes the Emmc and put it into StandBy State (Ready for 
  *         data transfer).
  * @param  None
  * @retval None
  */
void EmmcPeriphInit(void)
{
	
	 /* Configure SDIO interface GPIO */
 GPIO_InitTypeDef  GPIO_InitStructure;

  /* GPIOC and GPIOD Periph clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);

  /* Configure PC.08, PC.09, PC.10, PC.11, PC.12 pin: D0, D1, D2, D3, CLK pin */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;//GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* Configure PD.02 CMD line */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOD, &GPIO_InitStructure);


  /* Enable the SDIO AHB Clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_SDIO, ENABLE);

  /* Enable the DMA2 Clock */
//  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

	
	
//  GPIO_InitTypeDef  GPIO_InitStructure;

//  /* GPIOC and GPIOD Periph clock enable */
//  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD, ENABLE);

//  GPIO_PinAFConfig(GPIOC, GPIO_PinSource8, GPIO_AF_SDIO);
//  GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_SDIO);
//  GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_SDIO);
//  GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_SDIO);
//  GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_SDIO);
//  GPIO_PinAFConfig(GPIOD, GPIO_PinSource2, GPIO_AF_SDIO);

//  /* Configure PC.08, PC.09, PC.10, PC.11 pins: D0, D1, D2, D3 pins */
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
//  GPIO_Init(GPIOC, &GPIO_InitStructure);

//  /* Configure PD.02 CMD line */
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
//  GPIO_Init(GPIOD, &GPIO_InitStructure);

//  /* Configure PC.12 pin: CLK pin */
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
//  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
//  GPIO_Init(GPIOC, &GPIO_InitStructure);

//  /* Enable the SDIO APB2 Clock */
//  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SDIO, ENABLE);

//  /* Enable the DMA2 Clock */
//  RCC_AHB1PeriphClockCmd(SD_SDIO_DMA_CLK, ENABLE);
}

/**
  * @brief  Configures the DMA2 Channel4 for SDIO Tx request.
  * @param  BufferSRC: pointer to the source buffer
  * @param  BufferSize: buffer size
  * @retval None
  */
void EmmcLowLevelDMATxConfig(INT32U *BufferSRC, INT32U BufferSize)
{
  DMA_InitTypeDef DMA_InitStructure;

	DMA_ClearFlag(DMA2_FLAG_TC4 | DMA2_FLAG_TE4 | DMA2_FLAG_HT4 | DMA2_FLAG_GL4);
//  DMA_ClearFlag(DMA2_Channel4, EMMC_SDIO_DMA_FLAG_FEIF | EMMC_SDIO_DMA_FLAG_DMEIF | EMMC_SDIO_DMA_FLAG_TEIF | EMMC_SDIO_DMA_FLAG_HTIF | EMMC_SDIO_DMA_FLAG_TCIF);

  /* DMA2 Stream3  or Stream6 disable */
  DMA_Cmd(DMA2_Channel4, DISABLE);

  /* DMA2 Stream3  or Stream6 Config */
  DMA_DeInit(DMA2_Channel4);

//  DMA_InitStructure.DMA_Channel = EMMC_SDIO_DMA_CHANNEL;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (INT32U)SDIO_FIFO_ADDRESS;
  DMA_InitStructure.DMA_MemoryBaseAddr = (INT32U)BufferSRC;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_BufferSize = BufferSize;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA2_Channel4, &DMA_InitStructure);
  DMA_ITConfig(DMA2_Channel4, DMA_IT_TC, ENABLE);
//  DMA_FlowControllerConfig(DMA2_Channel4, DMA_FlowCtrl_Peripheral);

  /* DMA2 Stream3  or Stream6 enable */
  DMA_Cmd(DMA2_Channel4, ENABLE);
    

}

/**
  * @brief  Configures the DMA2 Channel4 for SDIO Rx request.
  * @param  BufferDST: pointer to the destination buffer
  * @param  BufferSize: buffer size
  * @retval None
  */
void EmmcLowLevelDMARxConfig(INT32U *BufferDST, INT32U BufferSize)
{
  DMA_InitTypeDef DMA_InitStructure;

	DMA_ClearFlag(DMA2_FLAG_TC4 | DMA2_FLAG_TE4 | DMA2_FLAG_HT4 | DMA2_FLAG_GL4);
	
  /* DMA2 Stream3  or Stream6 disable */
  DMA_Cmd(DMA2_Channel4, DISABLE);

  /* DMA2 Stream3 or Stream6 Config */
  DMA_DeInit(DMA2_Channel4);

//  DMA_InitStructure.DMA_Channel = EMMC_SDIO_DMA_CHANNEL;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (INT32U)SDIO_FIFO_ADDRESS;
  DMA_InitStructure.DMA_MemoryBaseAddr = (INT32U)BufferDST;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_BufferSize = BufferSize;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA2_Channel4, &DMA_InitStructure);
  DMA_ITConfig(DMA2_Channel4, DMA_IT_TC, ENABLE);
//  DMA_FlowControllerConfig(DMA2_Channel4, DMA_FlowCtrl_Peripheral);

  /* DMA2 Stream3 or Stream6 enable */
  DMA_Cmd(DMA2_Channel4, ENABLE);
}


EmmcError SD_Initialize(void)
{
	INT8U result;
	result = EmmcInit();
	SD_Type = CardType;	
	SDIO_ClockCmd(DISABLE);
	return result;
		
}

#define SECTOR_SIZE 512
INT8U SD_ReadDisk(INT8U *buf, INT32U sector, INT8U cnt)
{
	INT8U r1;
	if(SD_Type != SD_TYPE_V2HC) 
		sector <<= 9;//转换为字节地址
	
	if(cnt == 1)
	{ 
		EmmcReadBlock(&buf[0],sector << 9 ,SECTOR_SIZE);
	}
	else
	{ 
		EmmcReadMultiBlocks(&buf[0],sector << 9 ,SECTOR_SIZE,cnt);
	}  
	return r1;//
}

INT8U SD_WriteDisk(INT8U *buf, INT32U sector, INT8U cnt)
{
	if(cnt ==1)
  {
		//memcpy(buff2,buff,SECTOR_SIZE);
		EmmcWriteBlock(&buf[0],sector << 9 ,SECTOR_SIZE);
	}
	else
	{
		//memcpy(buff2,buff,SECTOR_SIZE * count);
		EmmcWriteMultiBlocks(&buf[0],sector << 9 ,SECTOR_SIZE,cnt);
	}
        
  return 0;  // res_ok == 0
}
/**
  * @brief  Initializes the SD Card and put it into StandBy State (Ready for data 
  *         transfer).
  * @param  None
  * @retval EmmcError: SD Card Error code.
  */
EmmcError EmmcInit(void)
{
  __IO EmmcError Result = EMMC_OK;
  NVIC_Configuration();
  EmmcPeriphInit();
  SDIO_DeInit();

  Result = EmmcPowerON();
	
  if (Result != EMMC_OK)
  {
    return(Result);
  }
  Result = EmmcInitializeCards(&MyEmmcCardInfo);
  if (Result != EMMC_OK)
  {
    return(Result);
  }
//  /*!< Configure the SDIO peripheral */
//  /*!< SDIO_CK = SDIOCLK / (SDIO_TRANSFER_CLK_DIV + 2) */
//  /*!< on STM32F4xx devices, SDIOCLK is fixed to 48MHz */
//  SDIO_InitStructure.SDIO_ClockDiv = SDIO_INIT_CLK_DIV;
//  SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
//  SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
//  SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
//  SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_1b;
//  SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
//  SDIO_Init(&SDIO_InitStructure);
   if (Result == EMMC_OK)
   {
     Result = EmmcEnableWideBusOperation(SDIO_BusWide_4b);
   }  

  return(Result);
}

/**
  * @brief  Gets the cuurent sd card data transfer status.
  * @param  None
  * @retval SDTransferState: Data Transfer state.
  *   This value can be: 
  *        - SD_TRANSFER_OK: No data transfer is acting
  *        - SD_TRANSFER_BUSY: Data transfer is acting
  */
EmmcTransferState EmmcGetStatus(void)
{
		EmmcCardState CardState =  EMMC_CARD_TRANSFER;

		CardState = EmmcGetState();
		
		if (CardState == EMMC_CARD_TRANSFER)
		{
			return(EMMC_TRANSFER_OK);
		}
		else if (CardState == EMMC_CARD_RECEIVING)
		{
			return(EMMC_TRANSFER_OK);
		}
		
		else if(CardState == EMMC_CARD_ERROR)
		{
			return (EMMC_TRANSFER_ERROR);
		}
		else
		{
			return(EMMC_TRANSFER_BUSY);
		}
}

/**
  * @brief  Returns the current card's state.
  * @param  None
  * @retval EmmcCardState: EMMC Card Error or EMMC Card Current State.
  */
EmmcCardState EmmcGetState(void)
{
    INT32U Resp = 0;
	
    if (EmmcSendStatus(&Resp) != EMMC_OK)
    {
      return EMMC_CARD_ERROR;
    }
    else
    {
      return (EmmcCardState)((Resp >> 9) & 0x0F);
    }
}



/**
  * @brief  Enquires cards about their operating voltage and configures 
  *   clock controls.
  * @param  None
  * @retval EmmcError: SD Card Error code.
  */
EmmcError EmmcPowerON(void)
{
  __IO EmmcError Result = EMMC_OK; 
	INT32U Cnt = 0;
	INT32U i = 0;

  /*!< Power ON Sequence -----------------------------------------------------*/
  /*!< Configure the SDIO peripheral */
  /*!< SDIO_CK = SDIOCLK / (SDIO_INIT_CLK_DIV + 2) */
  /*!< on STM32F4xx devices, SDIOCLK is fixed to 48MHz */
  /*!< SDIO_CK for initialization should not exceed 400 KHz */  
  SDIO_InitStructure.SDIO_ClockDiv = SDIO_INIT_CLK_DIV;
  SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
  SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
  SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
  SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_1b;
  SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
  SDIO_Init(&SDIO_InitStructure);

  /*!< Set Power State to ON */
  SDIO_SetPowerState(SDIO_PowerState_ON);

  /*!< Enable SDIO Clock */
  SDIO_ClockCmd(ENABLE);
  for(i=0; i<50000; i++);
	  /*!< CMD0: GO_IDLE_STATE ---------------------------------------------------*/
  /*!< No CMD response required */
	
  SDIO_CmdInitStructure.SDIO_Argument = 0x00;
  SDIO_CmdInitStructure.SDIO_CmdIndex = EMMC_CMD_GO_IDLE_STATE;
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_No;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);

  Result = CmdError();
#if ARM_UART_DEBUG
	uart1_init(115200);
	DEBUG_EN = 1;
	printf("\r\n CMD0 %u \r\n",Result);
#endif  
  if (Result != EMMC_OK)
  {
    return(Result);
  }
	
	for(i=0; i<500000; i++);
	
	CardType = SDIO_HIGH_CAPACITY_MMC_CARD; 
	do
	{
      /*!< SEND CMD1*/
      SDIO_CmdInitStructure.SDIO_Argument = EMMC_OCR_REG;
      SDIO_CmdInitStructure.SDIO_CmdIndex = EMMC_CMD_SEND_OP_COND;
      SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
      SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
      SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
      SDIO_SendCommand(&SDIO_CmdInitStructure);
			Result = CmdResp3Error();
			Cnt++;
	}while((Cnt<0xffff)&&(Result != EMMC_OK));
	
	for(i=0; i<500000; i++);
	
		/*!< SEND CMD1*/
	SDIO_CmdInitStructure.SDIO_Argument = EMMC_OCR_REG;
	SDIO_CmdInitStructure.SDIO_CmdIndex = EMMC_CMD_SEND_OP_COND;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);
	Result = CmdResp3Error();
#if ARM_UART_DEBUG
	uart1_init(115200);
	DEBUG_EN = 1;
	printf("\r\n CMD1 %u \r\n",Result);
#endif 	
  return(Result);
}

/**
  * @brief  Turns the SDIO output signals off.
  * @param  None
  * @retval EmmcError: SD Card Error code.
  */
EmmcError EmmcPowerOFF(void)
{
  EmmcError errorstatus = EMMC_OK;

  /*!< Set Power State to OFF */
  SDIO_SetPowerState(SDIO_PowerState_OFF);

  return(errorstatus);
}

/**
  * @brief  Intialises all cards or single card as the case may be Card(s) come 
  *         into standby state.
  * @param  None
  * @retval EmmcError: SD Card Error code.
  */
EmmcError EmmcInitializeCards(EmmcCardInfo *E)
{
		EmmcError Result = EMMC_OK;
		INT32U CSD_Tab[4] = {0};
		INT32U CID_Tab[4] = {10};
		INT16U Rca = 0x01;
		char i;

		if (SDIO_GetPowerState() == SDIO_PowerState_OFF)
		{
			Result = EMMC_REQUEST_NOT_APPLICABLE;
			return(Result);
		}

		/*!< Send CMD2 ALL_SEND_CID */
		SDIO_CmdInitStructure.SDIO_Argument = 0x00;
		SDIO_CmdInitStructure.SDIO_CmdIndex = EMMC_CMD_ALL_SEND_CID;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Long;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);
		
		Result = CmdResp2Error();

		if (EMMC_OK != Result)
		{
			return(Result);
		}
// get CID after send CMD2
    CID_Tab[0] = SDIO_GetResponse(SDIO_RESP1);
    CID_Tab[1] = SDIO_GetResponse(SDIO_RESP2);
    CID_Tab[2] = SDIO_GetResponse(SDIO_RESP3);
    CID_Tab[3] = SDIO_GetResponse(SDIO_RESP4);
#if ARM_UART_DEBUG
		uart1_init(115200);
		DEBUG_EN = 1;
		printf("\r\n CMD2 GET CID: %x %x %x %x \r\n",CID_Tab[0],CID_Tab[1],CID_Tab[2],CID_Tab[3]);
#endif		
    /*!< Send CMD3 SET_REL_ADDR with argument 0 */
    /*!< SD Card publishes its RCA. */
    SDIO_CmdInitStructure.SDIO_Argument = 0x00;
    SDIO_CmdInitStructure.SDIO_CmdIndex = EMMC_CMD_SET_REL_ADDR;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);

    Result = CmdResp6Error(EMMC_CMD_SET_REL_ADDR, &Rca);
#if ARM_UART_DEBUG
		uart1_init(115200);
		DEBUG_EN = 1;
		printf("\r\n CMD3 %u \r\n",Result);
#endif
    if (EMMC_OK != Result)
    {
      return(Result);
    }
    /*!< Send CMD9 SEND_CSD with argument as card's RCA */
    SDIO_CmdInitStructure.SDIO_Argument = (INT32U)(Rca << 16);
    SDIO_CmdInitStructure.SDIO_CmdIndex = EMMC_CMD_SEND_CSD;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Long;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);

    Result = CmdResp2Error();

    if (EMMC_OK != Result)
    {
      return(Result);
    }
    CSD_Tab[0] = SDIO_GetResponse(SDIO_RESP1);
    CSD_Tab[1] = SDIO_GetResponse(SDIO_RESP2);
    CSD_Tab[2] = SDIO_GetResponse(SDIO_RESP3);
    CSD_Tab[3] = SDIO_GetResponse(SDIO_RESP4);	
#if ARM_UART_DEBUG
	uart1_init(115200);
	DEBUG_EN = 1;
	printf("\r\n CMD9 %u ,CSD = %x %x %x %x \r\n",Result,CSD_Tab[0],CSD_Tab[1],CSD_Tab[2],CSD_Tab[3]);
#endif
	Result = EmmcSelectDeselect((INT32U) (MyEmmcCardInfo.RCA << 16));
//#if ARM_UART_DEBUG
//	uart1_init(115200);
//	DEBUG_EN = 1;
//	printf("\r\n EmmcSelectDeselect %u \r\n",Result);
//#endif
	if (EMMC_OK != Result)
	{
		return(Result);
	}

	Result = EmmcReadExtCsd(&MyEmmcCardInfo);

	if (EMMC_OK != Result)
	{
		return(Result);
	}
  Result = EmmcGetCardInfo(E, &CSD_Tab[0], &CID_Tab[0], Rca);

	if (EMMC_OK != Result)
	{
		return(Result);
	}
  Result = EMMC_OK; /*!< All cards get intialized */
  return(Result);
}

/**
  * @brief  Returns information about specific card.
  * @param  E: pointer to a SD_CardInfo structure that contains all SD card 
  *         information.
  * @retval EmmcError: SD Card Error code.
  */
EmmcError EmmcGetCardInfo(EmmcCardInfo *E, INT32U *CSD_Tab, INT32U *CID_Tab, INT16U Rca)
{
	
		EmmcError errorstatus = EMMC_OK;
		INT8U tmp = 0;

		E->CardType = (INT8U)CardType;
		E->RCA = (INT16U)Rca;

		/*!< Byte 0 */
		tmp = (INT8U)((CSD_Tab[0] & 0xFF000000) >> 24);
		E->EmmcCsd.CSDStruct = (tmp & 0xC0) >> 6;
		E->EmmcCsd.SysSpecVersion = (tmp & 0x3C) >> 2;
		E->EmmcCsd.Reserved1 = tmp & 0x03;

		/*!< Byte 1 */
		tmp = (INT8U)((CSD_Tab[0] & 0x00FF0000) >> 16);
		E->EmmcCsd.TAAC = tmp;

		/*!< Byte 2 */
		tmp = (INT8U)((CSD_Tab[0] & 0x0000FF00) >> 8);
		E->EmmcCsd.NSAC = tmp;

		/*!< Byte 3 */
		tmp = (INT8U)(CSD_Tab[0] & 0x000000FF);
		E->EmmcCsd.MaxBusClkFrec = tmp;

		/*!< Byte 4 */
		tmp = (INT8U)((CSD_Tab[1] & 0xFF000000) >> 24);
		E->EmmcCsd.CardComdClasses = tmp << 4;

		/*!< Byte 5 */
		tmp = (INT8U)((CSD_Tab[1] & 0x00FF0000) >> 16);
		E->EmmcCsd.CardComdClasses |= (tmp & 0xF0) >> 4;
		E->EmmcCsd.RdBlockLen = tmp & 0x0F;

		/*!< Byte 6 */
		tmp = (INT8U)((CSD_Tab[1] & 0x0000FF00) >> 8);
		E->EmmcCsd.PartBlockRead = (tmp & 0x80) >> 7;
		E->EmmcCsd.WrBlockMisalign = (tmp & 0x40) >> 6;
		E->EmmcCsd.RdBlockMisalign = (tmp & 0x20) >> 5;
		E->EmmcCsd.DSRImpl = (tmp & 0x10) >> 4;
		E->EmmcCsd.Reserved2 = 0; /*!< Reserved */


		E->EmmcCsd.DeviceSize = (tmp & 0x03) << 10;

		/*!< Byte 7 */
		tmp = (INT8U)(CSD_Tab[1] & 0x000000FF);
		E->EmmcCsd.DeviceSize |= (tmp) << 2;

		/*!< Byte 8 */
		tmp = (INT8U)((CSD_Tab[2] & 0xFF000000) >> 24);
		E->EmmcCsd.DeviceSize |= (tmp & 0xC0) >> 6;

		E->EmmcCsd.MaxRdCurrentVDDMin = (tmp & 0x38) >> 3;
		E->EmmcCsd.MaxRdCurrentVDDMax = (tmp & 0x07);

		/*!< Byte 9 */
		tmp = (INT8U)((CSD_Tab[2] & 0x00FF0000) >> 16);
		E->EmmcCsd.MaxWrCurrentVDDMin = (tmp & 0xE0) >> 5;
		E->EmmcCsd.MaxWrCurrentVDDMax = (tmp & 0x1C) >> 2;
		E->EmmcCsd.DeviceSizeMul = (tmp & 0x03) << 1;
		/*!< Byte 10 */
		tmp = (INT8U)((CSD_Tab[2] & 0x0000FF00) >> 8);
		E->EmmcCsd.DeviceSizeMul |= (tmp & 0x80) >> 7;


		E->CardBlockSize = 1 << (E->EmmcCsd.RdBlockLen);


		E->CardCapacity = (INT64U)((INT64U)(E->EmmcExtCsd.EXT_CSD.SEC_COUNT[3] << 24 | E->EmmcExtCsd.EXT_CSD.SEC_COUNT[2] << 16 | 
		E->EmmcExtCsd.EXT_CSD.SEC_COUNT[1] << 8 | E->EmmcExtCsd.EXT_CSD.SEC_COUNT[0]) * E->CardBlockSize);


		E->EmmcCsd.EraseGrSize = (tmp & 0x40) >> 6;
		E->EmmcCsd.EraseGrMul = (tmp & 0x3F) << 1;

		/*!< Byte 11 */
		tmp = (INT8U)(CSD_Tab[2] & 0x000000FF);
		E->EmmcCsd.EraseGrMul |= (tmp & 0x80) >> 7;
		E->EmmcCsd.WrProtectGrSize = (tmp & 0x7F);

		/*!< Byte 12 */
		tmp = (INT8U)((CSD_Tab[3] & 0xFF000000) >> 24);
		E->EmmcCsd.WrProtectGrEnable = (tmp & 0x80) >> 7;
		E->EmmcCsd.ManDeflECC = (tmp & 0x60) >> 5;
		E->EmmcCsd.WrSpeedFact = (tmp & 0x1C) >> 2;
		E->EmmcCsd.MaxWrBlockLen = (tmp & 0x03) << 2;

		/*!< Byte 13 */
		tmp = (INT8U)((CSD_Tab[3] & 0x00FF0000) >> 16);
		E->EmmcCsd.MaxWrBlockLen |= (tmp & 0xC0) >> 6;
		E->EmmcCsd.WriteBlockPaPartial = (tmp & 0x20) >> 5;
		E->EmmcCsd.Reserved3 = 0;
		E->EmmcCsd.ContentProtectAppli = (tmp & 0x01);

		/*!< Byte 14 */
		tmp = (INT8U)((CSD_Tab[3] & 0x0000FF00) >> 8);
		E->EmmcCsd.FileFormatGrouop = (tmp & 0x80) >> 7;
		E->EmmcCsd.CopyFlag = (tmp & 0x40) >> 6;
		E->EmmcCsd.PermWrProtect = (tmp & 0x20) >> 5;
		E->EmmcCsd.TempWrProtect = (tmp & 0x10) >> 4;
		E->EmmcCsd.FileFormat = (tmp & 0x0C) >> 2;
		E->EmmcCsd.ECC = (tmp & 0x03);

		/*!< Byte 15 */
		tmp = (INT8U)(CSD_Tab[3] & 0x000000FF);
		E->EmmcCsd.CSD_CRC = (tmp & 0xFE) >> 1;
		E->EmmcCsd.Reserved4 = 1;


		/*!< Byte 0 */
		tmp = (INT8U)((CID_Tab[0] & 0xFF000000) >> 24);
		E->EmmcCid.ManufacturerID = tmp;

		/*!< Byte 1 */
		tmp = (INT8U)((CID_Tab[0] & 0x00FF0000) >> 16);
		E->EmmcCid.OEM_AppliID = tmp << 8;

		/*!< Byte 2 */
		tmp = (INT8U)((CID_Tab[0] & 0x000000FF00) >> 8);
		E->EmmcCid.OEM_AppliID |= tmp;

		/*!< Byte 3 */
		tmp = (INT8U)(CID_Tab[0] & 0x000000FF);
		E->EmmcCid.ProdName1 = tmp << 24;

		/*!< Byte 4 */
		tmp = (INT8U)((CID_Tab[1] & 0xFF000000) >> 24);
		E->EmmcCid.ProdName1 |= tmp << 16;

		/*!< Byte 5 */
		tmp = (INT8U)((CID_Tab[1] & 0x00FF0000) >> 16);
		E->EmmcCid.ProdName1 |= tmp << 8;

		/*!< Byte 6 */
		tmp = (INT8U)((CID_Tab[1] & 0x0000FF00) >> 8);
		E->EmmcCid.ProdName1 |= tmp;

		/*!< Byte 7 */
		tmp = (INT8U)(CID_Tab[1] & 0x000000FF);
		E->EmmcCid.ProdName2 = tmp;

		/*!< Byte 8 */
		tmp = (INT8U)((CID_Tab[2] & 0xFF000000) >> 24);
		E->EmmcCid.ProdRev = tmp;

		/*!< Byte 9 */
		tmp = (INT8U)((CID_Tab[2] & 0x00FF0000) >> 16);
		E->EmmcCid.ProdSN = tmp << 24;

		/*!< Byte 10 */
		tmp = (INT8U)((CID_Tab[2] & 0x0000FF00) >> 8);
		E->EmmcCid.ProdSN |= tmp << 16;

		/*!< Byte 11 */
		tmp = (INT8U)(CID_Tab[2] & 0x000000FF);
		E->EmmcCid.ProdSN |= tmp << 8;

		/*!< Byte 12 */
		tmp = (INT8U)((CID_Tab[3] & 0xFF000000) >> 24);
		E->EmmcCid.ProdSN |= tmp;

		/*!< Byte 13 */
		tmp = (INT8U)((CID_Tab[3] & 0x00FF0000) >> 16);
		E->EmmcCid.Reserved1 |= (tmp & 0xF0) >> 4;
		E->EmmcCid.ManufactDate = (tmp & 0x0F) << 8;

		/*!< Byte 14 */
		tmp = (INT8U)((CID_Tab[3] & 0x0000FF00) >> 8);
		E->EmmcCid.ManufactDate |= tmp;

		/*!< Byte 15 */
		tmp = (INT8U)(CID_Tab[3] & 0x000000FF);
		E->EmmcCid.CID_CRC = (tmp & 0xFE) >> 1;
		E->EmmcCid.Reserved2 = 1;
  
  return(errorstatus);
}


/**
  * @brief  Enables wide bus opeartion for the requeseted card if supported by 
  *         card.
  * @param  WideMode: Specifies the SD card wide bus mode. 
  *   This parameter can be one of the following values:
  *     @arg SDIO_BusWide_8b: 8-bit data transfer (Only for MMC)
  *     @arg SDIO_BusWide_4b: 4-bit data transfer
  *     @arg SDIO_BusWide_1b: 1-bit data transfer
  * @retval EmmcError: SD Card Error code.
  */
EmmcError EmmcEnableWideBusOperation(INT32U WideMode)
{
  EmmcError errorstatus = EMMC_OK;


    if (SDIO_BusWide_8b == WideMode)
    {
      errorstatus = EMMC_UNSUPPORTED_FEATURE;
      return(errorstatus);
    }
    else if (SDIO_BusWide_4b == WideMode)
    {
      errorstatus = EmmcEnWideBus(ENABLE);

      if (EMMC_OK == errorstatus)
      {
        /*!< Configure the SDIO peripheral */
        SDIO_InitStructure.SDIO_ClockDiv = SDIO_TRANSFER_CLK_DIV; 
        SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
        SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
        SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
        SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_4b;
        SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
        SDIO_Init(&SDIO_InitStructure);
      }
    }
    else
    {
      errorstatus = EmmcEnWideBus(DISABLE);

      if (EMMC_OK == errorstatus)
      {
        /*!< Configure the SDIO peripheral */
        SDIO_InitStructure.SDIO_ClockDiv = SDIO_TRANSFER_CLK_DIV; 
        SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
        SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
        SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
        SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_1b;
        SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
        SDIO_Init(&SDIO_InitStructure);
      }
    }
//  }

  return(errorstatus);
}

/**
  * @brief  Selects od Deselects the corresponding card.
  * @param  addr: Address of the Card to be selected.
  * @retval EmmcError: SD Card Error code.
  */
EmmcError EmmcSelectDeselect(INT32U addr)
{
  EmmcError errorstatus = EMMC_OK;

  /*!< Send CMD7 SDIO_SEL_DESEL_CARD */
  SDIO_CmdInitStructure.SDIO_Argument =  addr;
  SDIO_CmdInitStructure.SDIO_CmdIndex = EMMC_CMD_SEL_DESEL_CARD;
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);

  errorstatus = CmdResp1Error(EMMC_CMD_SEL_DESEL_CARD);

  return(errorstatus);
}


EmmcError EmmcReadExtCsd(EmmcCardInfo *E)
{
	EmmcError Result = EMMC_OK;
	INT32U count = 0;
  INT32U *ExtCsdBuf;
	ExtCsdBuf = (INT32U *)(&(E->EmmcExtCsd.CsdBuf[0]));

	SDIO_DataInitStructure.SDIO_DataTimeOut = EMMC_DATATIMEOUT;
  SDIO_DataInitStructure.SDIO_DataLength = (INT32U)512;
  SDIO_DataInitStructure.SDIO_DataBlockSize = (INT32U) 9 << 4;
  SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
  SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
  SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
  SDIO_DataConfig(&SDIO_DataInitStructure);
	
	/* CMD8 */ 
  SDIO_CmdInitStructure.SDIO_Argument = 0;
  SDIO_CmdInitStructure.SDIO_CmdIndex = EMMC_CMD_HS_SEND_EXT_CSD;
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);

  Result = CmdResp1Error(EMMC_CMD_HS_SEND_EXT_CSD);
#if ARM_UART_DEBUG
	uart1_init(115200);
	DEBUG_EN = 1;
	printf("\r\n CMD8 %u \r\n",Result);
#endif	
  if (EMMC_OK != Result)
  {
    return(Result);
  }


while (!(SDIO->STA &(SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DBCKEND | SDIO_FLAG_STBITERR)))
{
	if (SDIO_GetFlagStatus(SDIO_FLAG_RXFIFOHF) != RESET)
	{
		for (count = 0; count < 8; count++)
		{
			*(ExtCsdBuf + count) = SDIO_ReadData();
		}
		ExtCsdBuf += 8;
	}
}
  if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)
  {
    SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);
    Result = EMMC_DATA_TIMEOUT;
    return(Result);
  }
  else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)
  {
    SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);
    Result = EMMC_DATA_CRC_FAIL;
    return(Result);
  }
  else if (SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET)
  {
    SDIO_ClearFlag(SDIO_FLAG_RXOVERR);
    Result =EMMC_RX_OVERRUN;
    return(Result);
  }
  else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET)
  {
    SDIO_ClearFlag(SDIO_FLAG_STBITERR);
    Result = EMMC_START_BIT_ERR;
    return(Result);
  }
  count = EMMC_DATATIMEOUT;
	
  while ((SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET) && (count > 0))
  {
    *ExtCsdBuf = SDIO_ReadData();
    ExtCsdBuf++;
    count--;
  }
  
  /*!< Clear all the static flags */
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);

  return Result;
}

/**
  * @brief  Allows to read one block from a specified address in a card. The Data
  *         transfer can be managed by DMA mode or Polling mode. 
  * @note   This operation should be followed by two functions to check if the 
  *         DMA Controller and SD Card status.
  *          - SD_ReadWaitOperation(): this function insure that the DMA
  *            controller has finished all data transfer.
  *          - SD_GetStatus(): to check that the SD Card has finished the 
  *            data transfer and it is ready for data.            
  * @param  readbuff: pointer to the buffer that will contain the received data
  * @param  ReadAddr: Address from where data are to be read.  
  * @param  BlockSize: the SD card Data block size. The Block size should be 512.
  * @retval EmmcError: SD Card Error code.
  */
EmmcError EmmcReadBlock(INT8U *readbuff, INT32U ReadAddr, INT16U BlockSize)
{
  EmmcError errorstatus = EMMC_OK;
#if defined (SD_POLLING_MODE) 
  INT32U count = 0, *tempbuff = (INT32U *)readbuff;
#endif

  TransferError = EMMC_OK;
  TransferEnd = 0;
  StopCondition = 0;
  
//  SDIO->DCTRL = 0x0;

//  /* Set Block Size for Card */ 
//  SDIO_CmdInitStructure.SDIO_Argument = (INT32U) BlockSize;
//  SDIO_CmdInitStructure.SDIO_CmdIndex = EMMC_CMD_SET_BLOCKLEN;
//  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//  SDIO_SendCommand(&SDIO_CmdInitStructure);

//  errorstatus = CmdResp1Error(EMMC_CMD_SET_BLOCKLEN);
//#if ARM_UART_DEBUG
//	uart1_init(115200);
//	DEBUG_EN = 1;
//	printf("READ EMMC_CMD_SET_BLOCKLEN %u \r\n",errorstatus);
//#endif
//  if (EMMC_OK != errorstatus)
//  {
//    return(errorstatus);
//  }
  
  SDIO_DataInitStructure.SDIO_DataTimeOut = EMMC_DATATIMEOUT;
  SDIO_DataInitStructure.SDIO_DataLength = (INT32U)512;//BlockSize;
  SDIO_DataInitStructure.SDIO_DataBlockSize = (INT32U) 9 << 4;
  SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
  SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
  SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Disable;
  SDIO_DataConfig(&SDIO_DataInitStructure);
	


  /*!< Send CMD17 READ_SINGLE_BLOCK */
  SDIO_CmdInitStructure.SDIO_Argument = (INT32U)ReadAddr;
  SDIO_CmdInitStructure.SDIO_CmdIndex = EMMC_CMD_READ_SINGLE_BLOCK;
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);

  errorstatus = CmdResp1Error(EMMC_CMD_READ_SINGLE_BLOCK);
#if ARM_UART_DEBUG
	uart1_init(115200);
	DEBUG_EN = 1;
	printf("READ EMMC_CMD_READ_SINGLE_BLOCK %u \r\n",errorstatus);
#endif
  if (errorstatus != EMMC_OK)
  {
    return(errorstatus);
  }

#if defined (SD_POLLING_MODE) 

  /*!< In case of single block transfer, no need of stop transfer at all.*/
  /*!< Polling mode */
  while (!(SDIO->STA &(SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DBCKEND | SDIO_FLAG_STBITERR)))
  {
    if (SDIO_GetFlagStatus(SDIO_FLAG_RXFIFOHF) != RESET)
    {
      for (count = 0; count < 8; count++)
      {
        *(tempbuff + count) = SDIO_ReadData();
      }
      tempbuff += 8;
    }
  }

  if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)
  {
    SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);
    errorstatus = EMMC_DATA_TIMEOUT;
    return(errorstatus);
  }
  else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)
  {
    SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);
    errorstatus = EMMC_DATA_CRC_FAIL;
    return(errorstatus);
  }
  else if (SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET)
  {
    SDIO_ClearFlag(SDIO_FLAG_RXOVERR);
    errorstatus = EMMC_RX_OVERRUN;
    return(errorstatus);
  }
  else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET)
  {
    SDIO_ClearFlag(SDIO_FLAG_STBITERR);
    errorstatus = EMMC_START_BIT_ERR;
    return(errorstatus);
  }

  count = EMMC_DATATIMEOUT;
  while ((SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET) && (count > 0))
  {
    *tempbuff = SDIO_ReadData();
    tempbuff++;
    count--;
  }
 
  /*!< Clear all the static flags */
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);
#endif

#if defined (SD_DMA_MODE)
    SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, ENABLE);
    SDIO_DMACmd(ENABLE);
    EmmcLowLevelDMARxConfig((INT32U *)readbuff, BlockSize);
		errorstatus = EmmcWaitReadOperation();

		if(errorstatus !=  EMMC_OK)
		{
			return errorstatus;
		}
	
#endif
		
#if ARM_UART_DEBUG
	uart1_init(115200);
	DEBUG_EN = 1;
	printf("EmmcWaitReadOperation %u \r\n",errorstatus);
#endif

  return(errorstatus);
}

/**
  * @brief  Allows to read blocks from a specified address  in a card.  The Data
  *         transfer can be managed by DMA mode or Polling mode. 
  * @note   This operation should be followed by two functions to check if the 
  *         DMA Controller and SD Card status.
  *          - SD_ReadWaitOperation(): this function insure that the DMA
  *            controller has finished all data transfer.
  *          - EmmcGetStatus(): to check that the SD Card has finished the 
  *            data transfmer and it is ready for data.   
  * @param  readbuff: pointer to the buffer that will contain the received data.
  * @param  ReadAddr: Address from where data are to be read.
  * @param  BlockSize: the SD card Data block size. The Block size should be 512.
  * @param  NumberOfBlocks: number of blocks to be read.
  * @retval EmmcError: SD Card Error code.
  */
EmmcError EmmcReadMultiBlocks(INT8U *readbuff, INT32U ReadAddr, INT16U BlockSize, INT32U NumberOfBlocks)
{
  EmmcError errorstatus = EMMC_OK;
//		OS_CPU_SR cpu_sr=0;????????????????????
#if defined (SD_POLLING_MODE) 
  INT32U count = 0, *tempbuff = (INT32U *)readbuff;
#endif

//  TransferError = EMMC_OK;
  TransferEnd = 0;
  StopCondition = 1;
			
//	OS_ENTER_CRITICAL();

  SDIO->DCTRL = 0x0;

  /*!< Set Block Size for Card */
  SDIO_CmdInitStructure.SDIO_Argument = (INT32U) BlockSize;
  SDIO_CmdInitStructure.SDIO_CmdIndex = EMMC_CMD_SET_BLOCKLEN;
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);

  errorstatus = CmdResp1Error(EMMC_CMD_SET_BLOCKLEN);

  if (EMMC_OK != errorstatus)
  {
    return(errorstatus);
  }
    
  SDIO_DataInitStructure.SDIO_DataTimeOut = EMMC_DATATIMEOUT;
  SDIO_DataInitStructure.SDIO_DataLength = NumberOfBlocks * BlockSize;
  SDIO_DataInitStructure.SDIO_DataBlockSize = (INT32U) 9 << 4;
  SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
  SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
  SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Disable;
  SDIO_DataConfig(&SDIO_DataInitStructure);
	
  SDIO_DataInitStructure.SDIO_DataTimeOut = EMMC_DATATIMEOUT;
  SDIO_DataInitStructure.SDIO_DataLength = NumberOfBlocks * BlockSize;
  SDIO_DataInitStructure.SDIO_DataBlockSize = (INT32U) 9 << 4;
  SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
  SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
  SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
  SDIO_DataConfig(&SDIO_DataInitStructure);

  /*!< Send CMD18 READ_MULT_BLOCK with argument data address */
  SDIO_CmdInitStructure.SDIO_Argument = (INT32U)ReadAddr;
  SDIO_CmdInitStructure.SDIO_CmdIndex = EMMC_CMD_READ_MULT_BLOCK;
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);

  errorstatus = CmdResp1Error(EMMC_CMD_READ_MULT_BLOCK);
//  OS_EXIT_CRITICAL();
  if (errorstatus != EMMC_OK)
  {
    return(errorstatus);
  }

#if defined (SD_POLLING_MODE)  

   while (!(SDIO->STA &(SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DATAEND | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_STBITERR)))
   {
     if (SDIO_GetFlagStatus(SDIO_FLAG_RXFIFOHF) != RESET)
     {
	   for (count = 0; count < EMMC_HALFFIFO; count++)
       {
         *(tempbuff + count) = SDIO_ReadData();
       }
       tempbuff += EMMC_HALFFIFO;
     }
   }

   if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)
   {
     SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);
     errorstatus = EMMC_DATA_TIMEOUT;
     return(errorstatus);
   }
   else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)
   {
     SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);
     errorstatus = EMMC_DATA_CRC_FAIL;
     return(errorstatus);
   }
   else if (SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET)
   {
     SDIO_ClearFlag(SDIO_FLAG_RXOVERR);
     errorstatus = EMMC_RX_OVERRUN;
     return(errorstatus);
   }
   else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET)
   {
     SDIO_ClearFlag(SDIO_FLAG_STBITERR);
     errorstatus = EMMC_START_BIT_ERR;
     return(errorstatus);
   }
   while (SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET)
   {
     *tempbuff = SDIO_ReadData();
     tempbuff++;
   }

   if (SDIO_GetFlagStatus(SDIO_FLAG_DATAEND) != RESET)
   {
     /* In Case Of SD-CARD Send Command STOP_TRANSMISSION */
//     if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_HIGH_CAPACITY_SD_CARD == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType))
//     {
       /* Send CMD12 STOP_TRANSMISSION */
       SDIO_CmdInitStructure.SDIO_Argument = 0x0;
       SDIO_CmdInitStructure.SDIO_CmdIndex = EMMC_CMD_STOP_TRANSMISSION;
       SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
       SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
       SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
       SDIO_SendCommand(&SDIO_CmdInitStructure);

       errorstatus = CmdResp1Error(EMMC_CMD_STOP_TRANSMISSION);

       if (errorstatus != EMMC_OK)
       {
         return(errorstatus);
       }
//     }
   }   
  /*!< Clear all the static flags */
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);
#endif

#if defined (SD_DMA_MODE)
//	 OS_ENTER_CRITICAL();
  SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, ENABLE);
  SDIO_DMACmd(ENABLE);
  EmmcLowLevelDMARxConfig((INT32U *)readbuff, (NumberOfBlocks * BlockSize));
//	  OS_EXIT_CRITICAL();
	errorstatus = EmmcWaitReadOperation();
	if(errorstatus !=  EMMC_OK)
	{
		return errorstatus;
	}
#endif			
  return(errorstatus);
}

/**
  * @brief  This function waits until the SDIO DMA data transfer is finished. 
  *         This function should be called after SDIO_ReadMultiBlocks() function
  *         to insure that all data sent by the card are already transferred by 
  *         the DMA controller.
  * @param  None.
  * @retval EmmcError: SD Card Error code.
  */
EmmcError EmmcWaitReadOperation(void)
{
  EmmcError errorstatus = EMMC_OK;//????????????????????????????
//	OS_CPU_SR cpu_sr=0;
  INT32U timeout;
	

  timeout = EMMC_DATATIMEOUT;
  
  while ((DMAEndOfTransfer == 0x00) && (TransferEnd == 0) && (TransferError == EMMC_OK) && (timeout > 0))
  {
    timeout--;
  }
  
  DMAEndOfTransfer = 0x00;

  timeout = EMMC_DATATIMEOUT;
  
  while(((SDIO->STA & SDIO_FLAG_RXACT)) && (timeout > 0))
  {
    timeout--;  
  }

  if (StopCondition == 1)
  {
//		OS_ENTER_CRITICAL();
    errorstatus = EmmcStopTransfer();
//		OS_EXIT_CRITICAL();
		StopCondition = 0;
  }
  
  if ((timeout == 0) && (errorstatus == EMMC_OK))
  {
    errorstatus = EMMC_DATA_TIMEOUT;
  }
  
  /*!< Clear all the static flags */
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);

  if (TransferError != EMMC_OK)
  {
    return(TransferError);
  }
  else
  {
    return(errorstatus);  
  }
}

/**
  * @brief  Allows to write one block starting from a specified address in a card.
  *         The Data transfer can be managed by DMA mode or Polling mode.
  * @note   This operation should be followed by two functions to check if the 
  *         DMA Controller and SD Card status.
  *          - SD_ReadWaitOperation(): this function insure that the DMA
  *            controller has finished all data transfer.
  *          - SD_GetStatus(): to check that the SD Card has finished the 
  *            data transfer and it is ready for data.      
  * @param  writebuff: pointer to the buffer that contain the data to be transferred.
  * @param  WriteAddr: Address from where data are to be read.   
  * @param  BlockSize: the SD card Data block size. The Block size should be 512.
  * @retval EmmcError: SD Card Error code.
  */
EmmcError EmmcWriteBlock(INT8U *writebuff, INT32U WriteAddr, INT16U BlockSize)
{

  EmmcError errorstatus = EMMC_OK;
	EmmcTransferState TranResult;
	INT8U cardstate = 0;

#if defined (SD_POLLING_MODE)
  INT32U bytestransferred = 0, count = 0, restwords = 0;
  INT32U *tempbuff = (INT32U *)writebuff;
#endif

  TransferError = EMMC_OK;
  TransferEnd = 0;
  StopCondition = 0;
  
				
//  SDIO->DCTRL = 0x0;

//  /* Set Block Size for Card */ 
//  SDIO_CmdInitStructure.SDIO_Argument = (INT32U) BlockSize;
//  SDIO_CmdInitStructure.SDIO_CmdIndex = EMMC_CMD_SET_BLOCKLEN;
//  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
//  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
//  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
//  SDIO_SendCommand(&SDIO_CmdInitStructure);

//  errorstatus = CmdResp1Error(EMMC_CMD_SET_BLOCKLEN);
//#if ARM_UART_DEBUG
//	uart1_init(115200);
//	DEBUG_EN = 1;
//	printf("WRITE EMMC_CMD_SET_BLOCKLEN %u \r\n",errorstatus);
//#endif	
//  if (EMMC_OK != errorstatus)
//  {
//    return(errorstatus);
//  }
    
	SDIO_DataInitStructure.SDIO_DataTimeOut = EMMC_DATATIMEOUT;
  SDIO_DataInitStructure.SDIO_DataLength = BlockSize;
  SDIO_DataInitStructure.SDIO_DataBlockSize = (INT32U) 9 << 4;
  SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
  SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
  SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Disable;
  SDIO_DataConfig(&SDIO_DataInitStructure);
		
  /*!< Send CMD24 WRITE_SINGLE_BLOCK */
  SDIO_CmdInitStructure.SDIO_Argument = WriteAddr;
  SDIO_CmdInitStructure.SDIO_CmdIndex = EMMC_CMD_WRITE_SINGLE_BLOCK;
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);

  errorstatus = CmdResp1Error(EMMC_CMD_WRITE_SINGLE_BLOCK);
#if ARM_UART_DEBUG
	uart1_init(115200);
	DEBUG_EN = 1;
	printf("WRITE EMMC_CMD_WRITE_SINGLE_BLOCK %u \r\n",errorstatus);
#endif
  if (errorstatus != EMMC_OK)
  {
    return(errorstatus);
  }
	

	

	
//  SDIO_DataInitStructure.SDIO_DataTimeOut = EMMC_DATATIMEOUT;
//  SDIO_DataInitStructure.SDIO_DataLength = BlockSize;
//  SDIO_DataInitStructure.SDIO_DataBlockSize = (INT32U) 9 << 4;
//  SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
//  SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
//  SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
//  SDIO_DataConfig(&SDIO_DataInitStructure);

//	do
//	{		
//	    TranResult = EmmcGetStatus();
//	}while(TranResult != EMMC_TRANSFER_OK);
	
  /*!< In case of single data block transfer no need of stop command at all */
#if defined (SD_POLLING_MODE) 
  while (!(SDIO->STA & (SDIO_FLAG_DBCKEND | SDIO_FLAG_TXUNDERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_STBITERR)))
  {
    if (SDIO_GetFlagStatus(SDIO_FLAG_TXFIFOHE) != RESET)
    {
      if ((512 - bytestransferred) < 32)
      {
        restwords = ((512 - bytestransferred) % 4 == 0) ? ((512 - bytestransferred) / 4) : (( 512 -  bytestransferred) / 4 + 1);
        for (count = 0; count < restwords; count++, tempbuff++, bytestransferred += 4)
        {
          SDIO_WriteData(*tempbuff);
        }
      }
      else
      {
        for (count = 0; count < 8; count++)
        {
          SDIO_WriteData(*(tempbuff + count));
        }
        tempbuff += 8;
        bytestransferred += 32;
      }
    }
  }
  if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)
  {
    SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);
    errorstatus = EMMC_DATA_TIMEOUT;
    return(errorstatus);
  }
  else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)
  {
    SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);
    errorstatus = EMMC_DATA_CRC_FAIL;
    return(errorstatus);
  }
  else if (SDIO_GetFlagStatus(SDIO_FLAG_TXUNDERR) != RESET)
  {
    SDIO_ClearFlag(SDIO_FLAG_TXUNDERR);
    errorstatus = EMMC_TX_UNDERRUN;
    return(errorstatus);
  }
  else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET)
  {
    SDIO_ClearFlag(SDIO_FLAG_STBITERR);
    errorstatus = EMMC_START_BIT_ERR;
    return(errorstatus);
  }
#endif
#if defined (SD_DMA_MODE)
	
  SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, ENABLE);
  EmmcLowLevelDMATxConfig((INT32U *)writebuff, BlockSize);
  SDIO_DMACmd(ENABLE);

	errorstatus = EmmcWaitWriteOperation();
#if ARM_UART_DEBUG
	uart1_init(115200);
	DEBUG_EN = 1;
	printf("EmmcWaitWriteOperation %u \r\n",errorstatus);
#endif	
	if(errorstatus !=  EMMC_OK)
		{
			return errorstatus;
		}
	
	errorstatus = IsCardProgramming(&cardstate);
#if ARM_UART_DEBUG
	uart1_init(115200);
	DEBUG_EN = 1;
	printf("IsCardProgramming %u \r\n",errorstatus);
#endif	
  while ((errorstatus == EMMC_OK) && ((EMMC_CARD_PROGRAMMING == cardstate) || (EMMC_CARD_RECEIVING == cardstate)))
  {

    errorstatus = IsCardProgramming(&cardstate);
	
  }
	
#endif
	#if ARM_UART_DEBUG
	uart1_init(115200);
	DEBUG_EN = 1;
	printf("write single block %u \r\n",errorstatus);
#endif
  return(errorstatus);
}

/**
  * @brief  Allows to write blocks starting from a specified address in a card.
  *         The Data transfer can be managed by DMA mode only. 
  * @note   This operation should be followed by two functions to check if the 
  *         DMA Controller and SD Card status.
  *          - SD_ReadWaitOperation(): this function insure that the DMA
  *            controller has finished all data transfer.
  *          - SD_GetStatus(): to check that the SD Card has finished the 
  *            data transfer and it is ready for data.     
  * @param  WriteAddr: Address from where data are to be read.
  * @param  writebuff: pointer to the buffer that contain the data to be transferred.
  * @param  BlockSize: the SD card Data block size. The Block size should be 512.
  * @param  NumberOfBlocks: number of blocks to be written.
  * @retval EmmcError: SD Card Error code.
  */
EmmcError EmmcWriteMultiBlocks(INT8U *writebuff, INT32U WriteAddr, INT16U BlockSize, INT32U NumberOfBlocks)
{
 EmmcError errorstatus = EMMC_OK;  
	INT32U i = 0;//??????????????????????
	EmmcTransferState TranResult;
	INT8U cardstate = 0;
//  OS_CPU_SR cpu_sr=0;
  TransferError = EMMC_OK;
  TransferEnd = 0;
//  StopCondition = 1;
  
  SDIO->DCTRL = 0x0;

//	OS_ENTER_CRITICAL();
	
  /* Set Block Size for Card */ 
  SDIO_CmdInitStructure.SDIO_Argument = (INT32U) BlockSize;
  SDIO_CmdInitStructure.SDIO_CmdIndex = EMMC_CMD_SET_BLOCKLEN;
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);

  errorstatus = CmdResp1Error(EMMC_CMD_SET_BLOCKLEN);

  if (EMMC_OK != errorstatus)
  {
    return(errorstatus);
  }
  

  /*!< To improve performance */
  SDIO_CmdInitStructure.SDIO_Argument = (INT32U) NumberOfBlocks;
  SDIO_CmdInitStructure.SDIO_CmdIndex = EMMC_CMD_SET_BLOCK_COUNT;
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);

  errorstatus = CmdResp1Error(EMMC_CMD_SET_BLOCK_COUNT);

  if (errorstatus != EMMC_OK)
  {
    return(errorstatus);
  }


  /*!< Send CMD25 WRITE_MULT_BLOCK with argument data address */
  SDIO_CmdInitStructure.SDIO_Argument = (INT32U)WriteAddr;
  SDIO_CmdInitStructure.SDIO_CmdIndex = EMMC_CMD_WRITE_MULT_BLOCK;
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);

  errorstatus = CmdResp1Error(EMMC_CMD_WRITE_MULT_BLOCK);

  if (EMMC_OK != errorstatus)
  {
    return(errorstatus);
  }

  SDIO_DataInitStructure.SDIO_DataTimeOut = EMMC_DATATIMEOUT;
  SDIO_DataInitStructure.SDIO_DataLength = NumberOfBlocks * BlockSize;
  SDIO_DataInitStructure.SDIO_DataBlockSize = (INT32U) 9 << 4;
  SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
  SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
  SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Disable;
  SDIO_DataConfig(&SDIO_DataInitStructure);
	
  SDIO_DataInitStructure.SDIO_DataTimeOut = EMMC_DATATIMEOUT;
  SDIO_DataInitStructure.SDIO_DataLength = NumberOfBlocks * BlockSize;
  SDIO_DataInitStructure.SDIO_DataBlockSize = (INT32U) 9 << 4;
  SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
  SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
  SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
  SDIO_DataConfig(&SDIO_DataInitStructure);
//  OS_EXIT_CRITICAL();
	do
	{			
//			OS_ENTER_CRITICAL();		
			TranResult = EmmcGetStatus();
//			OS_EXIT_CRITICAL();
	}while(TranResult != EMMC_TRANSFER_OK);
//	OS_ENTER_CRITICAL();
  SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, ENABLE);
  SDIO_DMACmd(ENABLE);
	EmmcLowLevelDMATxConfig((INT32U *)writebuff, (NumberOfBlocks * BlockSize));
//  OS_EXIT_CRITICAL();
	

	errorstatus = EmmcWaitWriteOperation();

	if (EMMC_OK != errorstatus)
  {
    return(errorstatus);
  }
//		OS_ENTER_CRITICAL();
	 errorstatus = IsCardProgramming(&cardstate);
//	  OS_EXIT_CRITICAL();
	while ((errorstatus == EMMC_OK) && ((EMMC_CARD_PROGRAMMING == cardstate) || (EMMC_CARD_RECEIVING == cardstate)))
  {
//		OS_ENTER_CRITICAL();
    errorstatus = IsCardProgramming(&cardstate);
//		OS_EXIT_CRITICAL();
		for(i=0; i<10000; i++);
  }
  return(errorstatus);
}

/**
  * @brief  This function waits until the SDIO DMA data transfer is finished. 
  *         This function should be called after SDIO_WriteBlock() and
  *         SDIO_WriteMultiBlocks() function to insure that all data sent by the 
  *         card are already transferred by the DMA controller.        
  * @param  None.
  * @retval EmmcError: SD Card Error code.
  */
EmmcError EmmcWaitWriteOperation(void)
{
  EmmcError errorstatus = EMMC_OK;
  INT32U timeout;

  timeout = EMMC_DATATIMEOUT;
  
  while ((DMAEndOfTransfer == 0x00) && (TransferEnd == 0) && (TransferError == EMMC_OK) && (timeout > 0))
  {
    timeout--;
  }
  
  DMAEndOfTransfer = 0x00;

  timeout = EMMC_DATATIMEOUT;
	
//	if (StopCondition == 1)
//	{
//		errorstatus = EmmcStopTransfer();
//		StopCondition = 0;
//	}
	
  while(((SDIO->STA & SDIO_FLAG_TXACT)) && (timeout > 0))
  {
    timeout--;  
  }
	/*!< Clear all the static flags */
		SDIO_ClearFlag(SDIO_STATIC_FLAGS);
  if ((timeout == 0) && (errorstatus == EMMC_OK))
  {
    errorstatus = EMMC_DATA_TIMEOUT;
  }
  if (TransferError != EMMC_OK)
  {
    return(TransferError);
  }
  else
  {
    return(errorstatus);
  }
}

/**
  * @brief  Gets the cuurent data transfer state.
  * @param  None
  * @retval SDTransferState: Data Transfer state.
  *   This value can be: 
  *        - SD_TRANSFER_OK: No data transfer is acting
  *        - SD_TRANSFER_BUSY: Data transfer is acting
  */
EmmcTransferState EmmcGetTransferState(void)
{
  if (SDIO->STA & (SDIO_FLAG_TXACT | SDIO_FLAG_RXACT))
  {
    return(EMMC_TRANSFER_BUSY);
  }
  else
  {
    return(EMMC_TRANSFER_OK);
  }
}

/**
  * @brief  Aborts an ongoing data transfer.
  * @param  None
  * @retval EmmcError: SD Card Error code.
  */
EmmcError EmmcStopTransfer(void)
{
  EmmcError errorstatus = EMMC_OK;

  /*!< Send CMD12 STOP_TRANSMISSION  */
  SDIO_CmdInitStructure.SDIO_Argument = (INT32U)0;//(MyEmmcCardInfo.RCA << 16);
  SDIO_CmdInitStructure.SDIO_CmdIndex = EMMC_CMD_STOP_TRANSMISSION;
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);

  errorstatus = CmdResp1Error(EMMC_CMD_STOP_TRANSMISSION);
  return(errorstatus);
}

/**
  * @brief  Allows to erase memory area specified for the given card.
  * @param  startaddr: the start address.
  * @param  endaddr: the end address.
  * @retval EmmcError: SD Card Error code.
  */
EmmcError EmmcErase(INT32U startaddr, INT32U endaddr)
{
  EmmcError errorstatus = EMMC_OK;
  INT32U delay = 0;
  __IO INT32U maxdelay = 0;
  INT8U cardstate = 0;
	
	/*!< Check if the card coomnd class supports erase command */
//  if (((CSD_Tab[1] >> 20) & SD_CCCC_ERASE) == 0)
		if ((MyEmmcCardInfo.EmmcCsd.CardComdClasses & EMMC_CCCC_ERASE) == 0)
		{
			errorstatus = EMMC_REQUEST_NOT_APPLICABLE;
			return(errorstatus);
		}

		maxdelay = 9200000 / ((SDIO->CLKCR & 0xFF) + 2);

		if (SDIO_GetResponse(SDIO_RESP1) & EMMC_CARD_LOCKED)
		{
			errorstatus = EMMC_LOCK_UNLOCK_FAILED;
			return(errorstatus);
		}

     /*!< Send CMD35 SD_ERASE_GRP_START with argument as addr  */
     SDIO_CmdInitStructure.SDIO_Argument = startaddr;
     SDIO_CmdInitStructure.SDIO_CmdIndex = EMMC_CMD_ERASE_GRP_START;
     SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
     SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
     SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
     SDIO_SendCommand(&SDIO_CmdInitStructure);

     errorstatus = CmdResp1Error(EMMC_CMD_ERASE_GRP_START);
     if (errorstatus != EMMC_OK)
     {
       return(errorstatus);
     }
//      for (delay = 0; delay < 10000; delay++);
     /*!< Send CMD36 SD_ERASE_GRP_END with argument as addr  */
     SDIO_CmdInitStructure.SDIO_Argument = endaddr;
     SDIO_CmdInitStructure.SDIO_CmdIndex = EMMC_CMD_ERASE_GRP_END;
     SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
     SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
     SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
     SDIO_SendCommand(&SDIO_CmdInitStructure);

     errorstatus = CmdResp1Error(EMMC_CMD_ERASE_GRP_END);
     if (errorstatus != EMMC_OK)
     {
       return(errorstatus);
     }
//		  for (delay = 0; delay < 10000; delay++);
   /*!< Send CMD38 ERASE */
//   SDIO_CmdInitStructure.SDIO_Argument = 0x03;
	SDIO_CmdInitStructure.SDIO_Argument = 0x00;
   SDIO_CmdInitStructure.SDIO_CmdIndex = EMMC_CMD_ERASE;
   SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
   SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
   SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
   SDIO_SendCommand(&SDIO_CmdInitStructure);

   errorstatus = CmdResp1Error(EMMC_CMD_ERASE);

   if (errorstatus != EMMC_OK)
   {
     return(errorstatus);
   }
  for (delay = 0; delay < maxdelay; delay++)
  {}

  /*!< Wait till the card is in programming state */
  errorstatus = IsCardProgramming(&cardstate);
  delay = EMMC_DATATIMEOUT;
  while ((delay > 0) && (errorstatus == EMMC_OK) && ((EMMC_CARD_PROGRAMMING == cardstate) || (EMMC_CARD_RECEIVING == cardstate)))
  {
    errorstatus = IsCardProgramming(&cardstate);
    delay--;
  }

  return(errorstatus);
}

/**
  * @brief  Returns the current card's status.
  * @param  pcardstatus: pointer to the buffer that will contain the SD card 
  *         status (Card Status register).
  * @retval EmmcError: SD Card Error code.
  */
EmmcError EmmcSendStatus(INT32U *pcardstatus)
{
  EmmcError errorstatus = EMMC_OK;

  if (pcardstatus == NULL)
  {
    errorstatus = EMMC_INVALID_PARAMETER;
    return(errorstatus);
  }

  SDIO_CmdInitStructure.SDIO_Argument = (INT32U) MyEmmcCardInfo.RCA << 16;
  SDIO_CmdInitStructure.SDIO_CmdIndex = EMMC_CMD_SEND_STATUS;
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);

  errorstatus = CmdResp1Error(EMMC_CMD_SEND_STATUS);

  if (errorstatus != EMMC_OK)
  {
    return(errorstatus);
  }

  *pcardstatus = SDIO_GetResponse(SDIO_RESP1);

  return(errorstatus);
}

/**
  * @brief  Allows to process all the interrupts that are high.
  * @param  None
  * @retval EmmcError: SD Card Error code.
  */
#ifdef SD_INTERRUPT_MODE
EmmcError SDIO_IRQHandler(void)
{ 
  if (SDIO_GetITStatus(SDIO_IT_DATAEND) != RESET)
  {
    TransferError = EMMC_OK;
    SDIO_ClearITPendingBit(SDIO_IT_DATAEND);
    TransferEnd = 1;
  }  
  else if (SDIO_GetITStatus(SDIO_IT_DCRCFAIL) != RESET)
  {
    SDIO_ClearITPendingBit(SDIO_IT_DCRCFAIL);
    TransferError = EMMC_DATA_CRC_FAIL;
  }
  else if (SDIO_GetITStatus(SDIO_IT_DTIMEOUT) != RESET)
  {
    SDIO_ClearITPendingBit(SDIO_IT_DTIMEOUT);
    TransferError = EMMC_DATA_TIMEOUT;
  }
  else if (SDIO_GetITStatus(SDIO_IT_RXOVERR) != RESET)
  {
    SDIO_ClearITPendingBit(SDIO_IT_RXOVERR);
    TransferError = EMMC_RX_OVERRUN;
  }
  else if (SDIO_GetITStatus(SDIO_IT_TXUNDERR) != RESET)
  {
    SDIO_ClearITPendingBit(SDIO_IT_TXUNDERR);
    TransferError = EMMC_TX_UNDERRUN;
  }
  else if (SDIO_GetITStatus(SDIO_IT_STBITERR) != RESET)
  {
    SDIO_ClearITPendingBit(SDIO_IT_STBITERR);
    TransferError = EMMC_START_BIT_ERR;
  }

  SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND |
                SDIO_IT_TXFIFOHE | SDIO_IT_RXFIFOHF | SDIO_IT_TXUNDERR |
                SDIO_IT_RXOVERR | SDIO_IT_STBITERR, DISABLE);
	
  return(TransferError);
}

#endif

/**
  * @brief  This function waits until the SDIO DMA data transfer is finished. 
  * @param  None.
  * @retval None.
  */
#ifdef SD_DMA_MODE
void DMA2_Channel4_5_IRQHandler(void)
{
//  if(DMA2->ISR & SDIO_DMA_FLAG_TCIF)  
//  {
//    DMAEndOfTransfer = 0x01;
//    DMA_ClearFlag(DMA2_Channel4, EMMC_SDIO_DMA_FLAG_TCIF|EMMC_SDIO_DMA_FLAG_FEIF);
//  }
	if(DMA2->ISR & DMA2_FLAG_TC4)  
  {
    DMAEndOfTransfer = 0x01;
    DMA_ClearFlag(DMA2_FLAG_TC4|DMA2_FLAG_TE4);
  }
}

#endif


/**
  * @brief  Checks for error conditions for CMD0.
  * @param  None
  * @retval EmmcError: SD Card Error code.
  */
static EmmcError CmdError(void)
{
  EmmcError errorstatus = EMMC_OK;
  INT32U timeout;

  timeout = SDIO_CMD0TIMEOUT; /*!< 10000 */

  while ((timeout > 0) && (SDIO_GetFlagStatus(SDIO_FLAG_CMDSENT) == RESET))
  {
    timeout--;
  }

  if (timeout == 0)
  {
    errorstatus = EMMC_CMD_RSP_TIMEOUT;
    return(errorstatus);
  }

  /*!< Clear all the static flags */
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);

  return(errorstatus);
}

/**
  * @brief  Checks for error conditions for R7 response.
  * @param  None
  * @retval EmmcError: SD Card Error code.
  */
EmmcError CmdResp7Error(void)
{
  EmmcError errorstatus = EMMC_OK;
  INT32U status;
  INT32U timeout = SDIO_CMD0TIMEOUT;

  status = SDIO->STA;

  while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)) && (timeout > 0))
  {
    timeout--;
    status = SDIO->STA;
  }

  if ((timeout == 0) || (status & SDIO_FLAG_CTIMEOUT))
  {
    /*!< Card is not V2.0 complient or card does not support the set voltage range */
    errorstatus = EMMC_CMD_RSP_TIMEOUT;
    SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
    return(errorstatus);
  }

  if (status & SDIO_FLAG_CMDREND)
  {
    /*!< Card is SD V2.0 compliant */
    errorstatus = EMMC_OK;
    SDIO_ClearFlag(SDIO_FLAG_CMDREND);
    return(errorstatus);
  }
  return(errorstatus);
}

/**
  * @brief  Checks for error conditions for R1 response.
  * @param  cmd: The sent command index.
  * @retval EmmcError: SD Card Error code.
  */
EmmcError CmdResp1Error(INT8U cmd)
{
  EmmcError errorstatus = EMMC_OK;
  INT32U status;
  INT32U response_r1;

  status = SDIO->STA;
  while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)))
  {
    status = SDIO->STA;
  }
  if (status & SDIO_FLAG_CTIMEOUT)
  {
    errorstatus = EMMC_CMD_RSP_TIMEOUT;
    SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
    return(errorstatus);
  }
  else if (status & SDIO_FLAG_CCRCFAIL)
  {
    errorstatus = EMMC_CMD_CRC_FAIL;
    SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);
    return(errorstatus);
  }

  /*!< Check response received is of desired command */
  if (SDIO_GetCommandResponse() != cmd)
  {
    errorstatus = EMMC_ILLEGAL_CMD;
    return(errorstatus);
  }

  /*!< Clear all the static flags */
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);

  /*!< We have received response, retrieve it for analysis  */
  response_r1 = SDIO_GetResponse(SDIO_RESP1);

  if ((response_r1 & EMMC_OCR_ERRORBITS) == EMMC_ALLZERO)
  {
    return(errorstatus);
  }

  if (response_r1 & EMMC_OCR_ADDR_OUT_OF_RANGE)
  {
    return(EMMC_ADDR_OUT_OF_RANGE);
  }

  if (response_r1 & EMMC_OCR_ADDR_MISALIGNED)
  {
    return(EMMC_ADDR_MISALIGNED);
  }

  if (response_r1 & EMMC_OCR_BLOCK_LEN_ERR)
  {
    return(EMMC_BLOCK_LEN_ERR);
  }

  if (response_r1 & EMMC_OCR_ERASE_SEQ_ERR)
  {
    return(EMMC_ERASE_SEQ_ERR);
  }

  if (response_r1 & EMMC_OCR_BAD_ERASE_PARAM)
  {
    return(EMMC_BAD_ERASE_PARAM);
  }

  if (response_r1 & EMMC_OCR_WRITE_PROT_VIOLATION)
  {
    return(EMMC_WRITE_PROT_VIOLATION);
  }

  if (response_r1 & EMMC_OCR_LOCK_UNLOCK_FAILED)
  {
    return(EMMC_LOCK_UNLOCK_FAILED);
  }

  if (response_r1 & EMMC_OCR_COM_CRC_FAILED)
  {
    return(EMMC_COM_CRC_FAILED);
  }

  if (response_r1 & EMMC_OCR_ILLEGAL_CMD)
  {
    return(EMMC_ILLEGAL_CMD);
  }

  if (response_r1 & EMMC_OCR_CARD_ECC_FAILED)
  {
    return(EMMC_CARD_ECC_FAILED);
  }

  if (response_r1 & EMMC_OCR_CC_ERROR)
  {
    return(EMMC_CC_ERROR);
  }

  if (response_r1 & EMMC_OCR_GENERAL_UNKNOWN_ERROR)
  {
    return(EMMC_GENERAL_UNKNOWN_ERROR);
  }

  if (response_r1 & EMMC_OCR_STREAM_READ_UNDERRUN)
  {
    return(EMMC_STREAM_READ_UNDERRUN);
  }

  if (response_r1 & EMMC_OCR_STREAM_WRITE_OVERRUN)
  {
    return(EMMC_STREAM_WRITE_OVERRUN);
  }

//   if (response_r1 & EMMC_OCR_CID_CSD_OVERWRIETE)
//   {
//   //  return(EMMC_CID_CSD_OVERWRITE);
//   }

//   if (response_r1 & EMMC_OCR_WP_ERASE_SKIP)
//   {
//     return(EMMC_WP_ERASE_SKIP);
//   }

  if (response_r1 & EMMC_OCR_CARD_ECC_DISABLED)
  {
    return(EMMC_CARD_ECC_DISABLED);
  }

  if (response_r1 & EMMC_OCR_ERASE_RESET)
  {
    return(EMMC_ERASE_RESET);
  }

  if (response_r1 & EMMC_OCR_AKE_SEQ_ERROR)
  {
    return(EMMC_AKE_SEQ_ERROR);
  }
  return(errorstatus);
}

/**
  * @brief  Checks for error conditions for R3 (OCR) response.
  * @param  None
  * @retval EmmcError: SD Card Error code.
  */
EmmcError CmdResp3Error(void)
{
  EmmcError errorstatus = EMMC_OK;
  INT32U status;

  status = SDIO->STA;

  while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)))
  {
    status = SDIO->STA;
  }

  if (status & SDIO_FLAG_CTIMEOUT)
  {
    errorstatus = EMMC_CMD_RSP_TIMEOUT;
    SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
    return(errorstatus);
  }
  /*!< Clear all the static flags */
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);
  return(errorstatus);
}

/**
  * @brief  Checks for error conditions for R2 (CID or CSD) response.
  * @param  None
  * @retval EmmcError: SD Card Error code.
  */
EmmcError CmdResp2Error(void)
{
  EmmcError errorstatus = EMMC_OK;
  INT32U status;

  status = SDIO->STA;

  while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CTIMEOUT | SDIO_FLAG_CMDREND)))
  {
    status = SDIO->STA;
  }

  if (status & SDIO_FLAG_CTIMEOUT)
  {
    errorstatus = EMMC_CMD_RSP_TIMEOUT;
    SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
    return(errorstatus);
  }
  else if (status & SDIO_FLAG_CCRCFAIL)
  {
    errorstatus = EMMC_CMD_CRC_FAIL;
    SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);
    return(errorstatus);
  }

  /*!< Clear all the static flags */
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);

  return(errorstatus);
}

/**
  * @brief  Checks for error conditions for R6 (RCA) response.
  * @param  cmd: The sent command index.
  * @param  prca: pointer to the variable that will contain the SD card relative 
  *         address RCA. 
  * @retval EmmcError: SD Card Error code.
  */
EmmcError CmdResp6Error(INT8U cmd, INT16U *prca)
{
  EmmcError errorstatus = EMMC_OK;
  INT32U status;
  INT32U response_r1;

  status = SDIO->STA;

  while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CTIMEOUT | SDIO_FLAG_CMDREND)))
  {
    status = SDIO->STA;
  }

  if (status & SDIO_FLAG_CTIMEOUT)
  {
    errorstatus = EMMC_CMD_RSP_TIMEOUT;
    SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
    return(errorstatus);
  }
  else if (status & SDIO_FLAG_CCRCFAIL)
  {
    errorstatus = EMMC_CMD_CRC_FAIL;
    SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);
    return(errorstatus);
  }

  /*!< Check response received is of desired command */
  if (SDIO_GetCommandResponse() != cmd)
  {
    errorstatus = EMMC_ILLEGAL_CMD;
    return(errorstatus);
  }

  /*!< Clear all the static flags */
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);

  /*!< We have received response, retrieve it.  */
  response_r1 = SDIO_GetResponse(SDIO_RESP1);

  if (EMMC_ALLZERO == (response_r1 & (EMMC_R6_GENERAL_UNKNOWN_ERROR | EMMC_R6_ILLEGAL_CMD | EMMC_R6_COM_CRC_FAILED)))
  {
    *prca = (INT16U) (response_r1 >> 16);
    return(errorstatus);
  }

  if (response_r1 & EMMC_R6_GENERAL_UNKNOWN_ERROR)
  {
    return(EMMC_GENERAL_UNKNOWN_ERROR);
  }

  if (response_r1 & EMMC_R6_ILLEGAL_CMD)
  {
    return(EMMC_ILLEGAL_CMD);
  }

  if (response_r1 & EMMC_R6_COM_CRC_FAILED)
  {
    return(EMMC_COM_CRC_FAILED);
  }

  return(errorstatus);
}

/**
  * @brief  Enables or disables the SDIO wide bus mode.
  * @param  NewState: new state of the SDIO wide bus mode.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval EmmcError: SD Card Error code.
  */
EmmcError EmmcEnWideBus(FunctionalState NewState)
{
  EmmcError Result = EMMC_ERROR;

//  INT32U scr[2] = {0, 0};

  if (SDIO_GetResponse(SDIO_RESP1) & EMMC_CARD_LOCKED)
  {
    Result = EMMC_LOCK_UNLOCK_FAILED;
    return(Result);
  }

  /*!< If wide bus operation to be enabled */
  if (NewState == ENABLE)
  {

				/* CMD6 */ 
			SDIO_CmdInitStructure.SDIO_Argument = EMMC_POWER_REG;
			SDIO_CmdInitStructure.SDIO_CmdIndex = EMMC_CMD_HS_SWITCH;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand(&SDIO_CmdInitStructure);

			Result = CmdResp1Error(EMMC_CMD_HS_SWITCH);
		  if (EMMC_OK != Result)
			{
				return(Result);
			}
			
							/* CMD6 */ 
			SDIO_CmdInitStructure.SDIO_Argument = EMMC_HIGHSPEED_REG;
			SDIO_CmdInitStructure.SDIO_CmdIndex = EMMC_CMD_HS_SWITCH;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand(&SDIO_CmdInitStructure);

			Result = CmdResp1Error(EMMC_CMD_HS_SWITCH);
		  if (EMMC_OK != Result)
			{
				return(Result);
			}
			
			/* CMD6 */ 
			SDIO_CmdInitStructure.SDIO_Argument = EMMC_4BIT_REG;
			SDIO_CmdInitStructure.SDIO_CmdIndex = EMMC_CMD_HS_SWITCH;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand(&SDIO_CmdInitStructure);

			Result = CmdResp1Error(EMMC_CMD_HS_SWITCH);
			
			if (EMMC_OK != Result)
			{
				return(Result);
			}
      return(Result);
  }   /*!< If wide bus operation to be disabled */
  else
  {
      Result = CmdResp1Error(EMMC_CMD_APP_CMD);

      if (Result != EMMC_OK)
      {
        return(Result);
      }

      if (Result != EMMC_OK)
      {
        return(Result);
      }

      return(Result);
  }
}

/**
  * @brief  Checks if the SD card is in programming state.
  * @param  pstatus: pointer to the variable that will contain the SD card state.
  * @retval EmmcError: SD Card Error code.
  */
EmmcError IsCardProgramming(INT8U *pstatus)
{
  EmmcError errorstatus = EMMC_OK;
  __IO INT32U respR1 = 0, status = 0;

  SDIO_CmdInitStructure.SDIO_Argument = (INT32U) MyEmmcCardInfo.RCA << 16;
  SDIO_CmdInitStructure.SDIO_CmdIndex = EMMC_CMD_SEND_STATUS;
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);

  status = SDIO->STA;
  while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)))
  {
    status = SDIO->STA;
  }

  if (status & SDIO_FLAG_CTIMEOUT)
  {
    errorstatus = EMMC_CMD_RSP_TIMEOUT;
    SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
    return(errorstatus);
  }
  else if (status & SDIO_FLAG_CCRCFAIL)
  {
    errorstatus = EMMC_CMD_CRC_FAIL;
    SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);
    return(errorstatus);
  }

  status = (INT32U)SDIO_GetCommandResponse();

  /*!< Check response received is of desired command */
  if (status != EMMC_CMD_SEND_STATUS)
  {
    errorstatus = EMMC_ILLEGAL_CMD;
    return(errorstatus);
  }

  /*!< Clear all the static flags */
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);


  /*!< We have received response, retrieve it for analysis  */
  respR1 = SDIO_GetResponse(SDIO_RESP1);

  /*!< Find out card status */
  *pstatus = (INT8U) ((respR1 >> 9) & 0x0000000F);

  if ((respR1 & EMMC_OCR_ERRORBITS) == EMMC_ALLZERO)
  {
    return(errorstatus);
  }

  if (respR1 & EMMC_OCR_ADDR_OUT_OF_RANGE)
  {
    return(EMMC_ADDR_OUT_OF_RANGE);
  }

  if (respR1 & EMMC_OCR_ADDR_MISALIGNED)
  {
    return(EMMC_ADDR_MISALIGNED);
  }

  if (respR1 & EMMC_OCR_BLOCK_LEN_ERR)
  {
    return(EMMC_BLOCK_LEN_ERR);
  }

  if (respR1 & EMMC_OCR_ERASE_SEQ_ERR)
  {
    return(EMMC_ERASE_SEQ_ERR);
  }

  if (respR1 & EMMC_OCR_BAD_ERASE_PARAM)
  {
    return(EMMC_BAD_ERASE_PARAM);
  }

  if (respR1 & EMMC_OCR_WRITE_PROT_VIOLATION)
  {
    return(EMMC_WRITE_PROT_VIOLATION);
  }

  if (respR1 & EMMC_OCR_LOCK_UNLOCK_FAILED)
  {
    return(EMMC_LOCK_UNLOCK_FAILED);
  }

  if (respR1 & EMMC_OCR_COM_CRC_FAILED)
  {
    return(EMMC_COM_CRC_FAILED);
  }

  if (respR1 & EMMC_OCR_ILLEGAL_CMD)
  {
    return(EMMC_ILLEGAL_CMD);
  }

  if (respR1 & EMMC_OCR_CARD_ECC_FAILED)
  {
    return(EMMC_CARD_ECC_FAILED);
  }

  if (respR1 & EMMC_OCR_CC_ERROR)
  {
    return(EMMC_CC_ERROR);
  }

  if (respR1 & EMMC_OCR_GENERAL_UNKNOWN_ERROR)
  {
    return(EMMC_GENERAL_UNKNOWN_ERROR);
  }

  if (respR1 & EMMC_OCR_STREAM_READ_UNDERRUN)
  {
    return(EMMC_STREAM_READ_UNDERRUN);
  }

  if (respR1 & EMMC_OCR_STREAM_WRITE_OVERRUN)
  {
    return(EMMC_STREAM_WRITE_OVERRUN);
  }

//   if (respR1 & EMMC_OCR_CID_CSD_OVERWRIETE)
//   {
//     return(EMMC_CID_CSD_OVERWRITE);
//   }

//   if (respR1 & EMMC_OCR_WP_ERASE_SKIP)
//   {
//     return(EMMC_WP_ERASE_SKIP);
//   }

  if (respR1 & EMMC_OCR_CARD_ECC_DISABLED)
  {
    return(EMMC_CARD_ECC_DISABLED);
  }

  if (respR1 & EMMC_OCR_ERASE_RESET)
  {
    return(EMMC_ERASE_RESET);
  }

  if (respR1 & EMMC_OCR_AKE_SEQ_ERROR)
  {
    return(EMMC_AKE_SEQ_ERROR);
  }

  return(errorstatus);
}


/**
  * @brief  Converts the number of bytes in power of two and returns the power.
  * @param  NumberOfBytes: number of bytes.
  * @retval None
  */
INT8U convert_from_bytes_to_power_of_two(INT16U NumberOfBytes)
{
  INT8U count = 0;

  while (NumberOfBytes != 1)
  {
    NumberOfBytes >>= 1;
    count++;
  }
  return(count);
}

#endif


