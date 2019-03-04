/**
  ******************************************************************************
  * @file    SdioEmmcDrive.c
  * @author  Luoxianhui 
  * @version V1.0.0
  * @date    12/27/2012
  * @brief   This file is Logic file.
  ******************************************************************************
	**/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SDIO_SD_H
#define __SDIO_SD_H

#ifdef __cplusplus
 extern "C" {
#endif
 
#include <stm32f10x.h>
//#include "define.h"
#include "types.h"
/** @defgroup SDIO_EMMC_Exported_Types
  * @{
  */

	 
/** 
  * @brief  SDIO specific error defines  
  */   
typedef enum
{
 
  EMMC_CMD_CRC_FAIL                    = (1), /*!< Command response received (but CRC check failed) */
  EMMC_DATA_CRC_FAIL                   = (2), /*!< Data bock sent/received (CRC check Failed) */
  EMMC_CMD_RSP_TIMEOUT                 = (3), /*!< Command response timeout */
  EMMC_DATA_TIMEOUT                    = (4), /*!< Data time out */
  EMMC_TX_UNDERRUN                     = (5), /*!< Transmit FIFO under-run */
  EMMC_RX_OVERRUN                      = (6), /*!< Receive FIFO over-run */
  EMMC_START_BIT_ERR                   = (7), /*!< Start bit not detected on all data signals in widE bus mode */
  EMMC_CMD_OUT_OF_RANGE                = (8), /*!< CMD's argument was out of range.*/
  EMMC_ADDR_MISALIGNED                 = (9), /*!< Misaligned address */
  EMMC_BLOCK_LEN_ERR                   = (10), /*!< Transferred block length is not allowed for the card or the number of transferred bytes does not match the block length */
  EMMC_ERASE_SEQ_ERR                   = (11), /*!< An error in the sequence of erase command occurs.*/
  EMMC_BAD_ERASE_PARAM                 = (12), /*!< An Invalid selection for erase groups */
  EMMC_WRITE_PROT_VIOLATION            = (13), /*!< Attempt to program a write protect block */
  EMMC_LOCK_UNLOCK_FAILED              = (14), /*!< Sequence or password error has been detected in unlock command or if there was an attempt to access a locked card */
  EMMC_COM_CRC_FAILED                  = (15), /*!< CRC check of the previous command failed */
  EMMC_ILLEGAL_CMD                     = (16), /*!< Command is not legal for the card state */
  EMMC_CARD_ECC_FAILED                 = (17), /*!< Card internal ECC was applied but failed to correct the data */
  EMMC_CC_ERROR                        = (18), /*!< Internal card controller error */
  EMMC_GENERAL_UNKNOWN_ERROR           = (19), /*!< General or Unknown error */
  EMMC_STREAM_READ_UNDERRUN            = (20), /*!< The card could not sustain data transfer in stream read operation. */
  EMMC_STREAM_WRITE_OVERRUN            = (21), /*!< The card could not sustain data programming in stream mode */
  EMMC_CID_CEMMC_OVERWRITE             = (22), /*!< CID/CSD overwrite error */
  EMMC_WP_ERASE_SKIP                   = (23), /*!< only partial address space was erased */
  EMMC_CARD_ECC_DISABLED               = (24), /*!< Command has been executed without using internal ECC */
  EMMC_ERASE_RESET                     = (25), /*!< Erase sequence was cleared before executing because an out of erase sequence command was received */
  EMMC_AKE_SEQ_ERROR                   = (26), /*!< Error in sequence of authentication. */
  EMMC_INVALID_VOLTRANGE               = (27),
  EMMC_ADDR_OUT_OF_RANGE               = (28),
  EMMC_SWITCH_ERROR                    = (29),
  EMMC_SDIO_DISABLED                   = (30),
  EMMC_SDIO_FUNCTION_BUSY              = (31),
  EMMC_SDIO_FUNCTION_FAILED            = (32),
  EMMC_SDIO_UNKNOWN_FUNCTION           = (33),

/** 
  * @brief  Standard error defines   
  */ 
  EMMC_INTERNAL_ERROR, 
  EMMC_NOT_CONFIGURED,
  EMMC_REQUEST_PENDING, 
  EMMC_REQUEST_NOT_APPLICABLE, 
  EMMC_INVALID_PARAMETER,  
  EMMC_UNSUPPORTED_FEATURE,  
  EMMC_UNSUPPORTED_HW,  
  EMMC_ERROR,  
  EMMC_OK = 0 
} EmmcError;

/** 
  * @brief  SDIO Transfer state  
  */   
typedef enum
{
  EMMC_TRANSFER_OK  = 0,
  EMMC_TRANSFER_BUSY = 1,
  EMMC_TRANSFER_ERROR
} EmmcTransferState;

/** 
  * @brief  SD Card States 
  */   
typedef enum
{
  EMMC_CARD_READY                  = ((INT32U)0x00000001),
  EMMC_CARD_IDENTIFICATION         = ((INT32U)0x00000002),
  EMMC_CARD_STANDBY                = ((INT32U)0x00000003),
  EMMC_CARD_TRANSFER               = ((INT32U)0x00000004),
  EMMC_CARD_SENDING                = ((INT32U)0x00000005),
  EMMC_CARD_RECEIVING              = ((INT32U)0x00000006),
  EMMC_CARD_PROGRAMMING            = ((INT32U)0x00000007),
  EMMC_CARD_DISCONNECTED           = ((INT32U)0x00000008),
  EMMC_CARD_ERROR                  = ((INT32U)0x000000FF)
}EmmcCardState;


/** 
  * @brief  Card Specific Data: CSD Register   
  */ 
typedef struct
{
  __IO INT8U  CSDStruct;            /*!< CSD structure */
  __IO INT8U  SysSpecVersion;       /*!< System specification version */
  __IO INT8U  Reserved1;            /*!< Reserved */
  __IO INT8U  TAAC;                 /*!< Data read access-time 1 */
  __IO INT8U  NSAC;                 /*!< Data read access-time 2 in CLK cycles */
  __IO INT8U  MaxBusClkFrec;        /*!< Max. bus clock frequency */
  __IO INT16U CardComdClasses;      /*!< Card command classes */
  __IO INT8U  RdBlockLen;           /*!< Max. read data block length */
  __IO INT8U  PartBlockRead;        /*!< Partial blocks for read allowed */
  __IO INT8U  WrBlockMisalign;      /*!< Write block misalignment */
  __IO INT8U  RdBlockMisalign;      /*!< Read block misalignment */
  __IO INT8U  DSRImpl;              /*!< DSR implemented */
  __IO INT8U  Reserved2;            /*!< Reserved */
  __IO INT32U DeviceSize;           /*!< Device Size */
  __IO INT8U  MaxRdCurrentVDDMin;   /*!< Max. read current @ VDD min */
  __IO INT8U  MaxRdCurrentVDDMax;   /*!< Max. read current @ VDD max */
  __IO INT8U  MaxWrCurrentVDDMin;   /*!< Max. write current @ VDD min */
  __IO INT8U  MaxWrCurrentVDDMax;   /*!< Max. write current @ VDD max */
  __IO INT8U  DeviceSizeMul;        /*!< Device size multiplier */
  __IO INT8U  EraseGrSize;          /*!< Erase group size */
  __IO INT8U  EraseGrMul;           /*!< Erase group size multiplier */
  __IO INT8U  WrProtectGrSize;      /*!< Write protect group size */
  __IO INT8U  WrProtectGrEnable;    /*!< Write protect group enable */
  __IO INT8U  ManDeflECC;           /*!< Manufacturer default ECC */
  __IO INT8U  WrSpeedFact;          /*!< Write speed factor */
  __IO INT8U  MaxWrBlockLen;        /*!< Max. write data block length */
  __IO INT8U  WriteBlockPaPartial;  /*!< Partial blocks for write allowed */
  __IO INT8U  Reserved3;            /*!< Reserded */
  __IO INT8U  ContentProtectAppli;  /*!< Content protection application */
  __IO INT8U  FileFormatGrouop;     /*!< File format group */
  __IO INT8U  CopyFlag;             /*!< Copy flag (OTP) */
  __IO INT8U  PermWrProtect;        /*!< Permanent write protection */
  __IO INT8U  TempWrProtect;        /*!< Temporary write protection */
  __IO INT8U  FileFormat;           /*!< File Format */
  __IO INT8U  ECC;                  /*!< ECC code */
  __IO INT8U  CSD_CRC;              /*!< CSD CRC */
  __IO INT8U  Reserved4;            /*!< always 1*/
} EMMC_CSD;



/** 
  * @brief  Card Specific Data: EXTCSD Register   
  */ 

typedef union
{
		struct _EXT_CSD
		{
			__IO INT8U  Reserved26[32];      		
			__IO INT8U 	FLUSH_CACHE;	
			__IO INT8U 	CACHE_CTRL; 	
			__IO INT8U 	POWER_OFF_NOTIFICATION;	
			__IO INT8U 	PACKED_FAILURE_INDEX;	
			__IO INT8U 	PACKED_COMMAND_STATUS;	
			__IO INT8U 	CONTEXT_CONF[15]; 	
			__IO INT8U 	EXT_PARTITIONS_ATTRIBUTE[2];	
			__IO INT8U 	EXCEPTION_EVENTS_STATUS[2];	
			__IO INT8U 	EXCEPTION_EVENTS_CTRL[2];	
			__IO INT8U 	DYNCAP_NEEDED; 	
			__IO INT8U 	CLASS_6_CTRL; 	
			__IO INT8U 	INI_TIMEOUT_EMU; 	
			__IO INT8U 	DATA_SECTOR_SIZE; 	
			__IO INT8U 	USE_NATIVE_SECTOR; 	
			__IO INT8U 	NATIVE_SECTOR_SIZE;	
			__IO INT8U 	VENDOR_SPECIFIC_FIELD[64];	
			__IO INT8U  Reserved25;		
			__IO INT8U 	PROGRAM_CID_CSD_DDR_SUPPORT;	
			__IO INT8U 	PERIODIC_WAKEUP; 	
			__IO INT8U 	TCASE_SUPPORT; 	
			__IO INT8U  Reserved24;		
			__IO INT8U 	SEC_BAD_BLK_MGMNT;	
			__IO INT8U  Reserved23;		
			__IO INT8U 	ENH_START_ADDR[4];	
			__IO INT8U 	ENH_SIZE_MULT[3]; 	
			__IO INT8U 	GP_SIZE_MULT[12]; 	
			__IO INT8U 	PARTITION_SETTING_COMPLETED;	
			__IO INT8U 	PARTITIONS_ATTRIBUTE; 	
			__IO INT8U 	MAX_ENH_SIZE_MULT[3]; 	
			__IO INT8U 	PARTITIONING_SUPPORT; 	
			__IO INT8U 	HPI_MGMT; 	
			__IO INT8U 	RST_n_FUNCTION; 	
			__IO INT8U 	BKOPS_EN; 	
			__IO INT8U 	BKOPS_START; 	
			__IO INT8U 	SANITIZE_START; 	
			__IO INT8U 	WR_REL_PARAM; 	
			__IO INT8U 	WR_REL_SET; 	
			__IO INT8U 	RPMB_SIZE_MULT; 	
			__IO INT8U 	FW_CONFIG; 	
			__IO INT8U  Reserved22;		
			__IO INT8U 	USER_WP;	
			__IO INT8U  Reserved21;		
			__IO INT8U 	BOOT_WP;	
			__IO INT8U 	BOOT_WP_STATUS; 	
			__IO INT8U 	ERASE_GROUP_DEF; 	
			__IO INT8U  Reserved20;		
			__IO INT8U  BOOT_BUS_CONDITIONS;		
			__IO INT8U  BOOT_CONFIG_PROT; 		
			__IO INT8U 	PARTITION_CONFIG; 	
			__IO INT8U  Reserved19;		
			__IO INT8U  ERASED_MEM_CONT;		
			__IO INT8U  Reserved18;		
			__IO INT8U  BUS_WIDTH;		
			__IO INT8U  Reserved17;		
			__IO INT8U  HS_TIMING;		
			__IO INT8U  Reserved16;		
			__IO INT8U  POWER_CLASS;		
			__IO INT8U  Reserved15;		
			__IO INT8U  CMD_SET_REV;		
			__IO INT8U  Reserved14;		
			__IO INT8U  CMD_SET;		
			__IO INT8U  Reserved13;		
			__IO INT8U  EXT_CSD_REV ;		
			__IO INT8U  Reserved12;		
			__IO INT8U  Reserved11;		
			__IO INT8U  Reserved10;		
			__IO INT8U  DEVICE_TYPE;		
			__IO INT8U  DRIVER_STRENGTH; 		
			__IO INT8U  OUT_OF_INTERRUPT_TIME;		
			__IO INT8U  PARTITION_SWITCH_TIME;		
			__IO INT8U  PWR_CL_52_195; 		
			__IO INT8U  PWR_CL_26_195; 		
			__IO INT8U  PWR_CL_52_360; 		
			__IO INT8U  PWR_CL_26_360; 		
			__IO INT8U  Reserved9;		
			__IO INT8U  MIN_PERF_R_4_26;		
			__IO INT8U  MIN_PERF_W_4_26; 		
			__IO INT8U  MIN_PERF_R_8_26_4_52; 		
			__IO INT8U  MIN_PERF_W_8_26_4_52; 		
			__IO INT8U  MIN_PERF_R_8_52; 		
			__IO INT8U  MIN_PERF_W_8_52; 		
			__IO INT8U  Reserved8;		
			__IO INT8U  SEC_COUNT[4];		
			__IO INT8U  Reserved7;		
			__IO INT8U  S_A_TIMEOUT;		
			__IO INT8U  Reserved6;		
			__IO INT8U  S_C_VCCQ; 		
			__IO INT8U  S_C_VCC; 		
			__IO INT8U  HC_WP_GRP_SIZE; 		
			__IO INT8U  REL_WR_SEC_C; 		
			__IO INT8U  ERASE_TIMEOUT_MULT; 		
			__IO INT8U  HC_ERASE_GRP_SIZE; 		
			__IO INT8U  ACC_SIZE; 		
			__IO INT8U  BOOT_SIZE_MULTI; 		
			__IO INT8U  Reserved5;		
			__IO INT8U  BOOT_INFO; 		
			__IO INT8U  obsolete2;		
			__IO INT8U  obsolete1;		
			__IO INT8U  SEC_FEATURE_SUPPORT; 		
			__IO INT8U  TRIM_MULT; 		
			__IO INT8U  Reserved4;		
			__IO INT8U  MIN_PERF_DDR_R_8_52;		
			__IO INT8U  MIN_PERF_DDR_W_8_52; 		
			__IO INT8U  PWR_CL_200_195; 		
			__IO INT8U  PWR_CL_200_360; 		
			__IO INT8U  PWR_CL_DDR_52_195; 		
			__IO INT8U  PWR_CL_DDR_52_360; 		
			__IO INT8U  Reserved3;		
			__IO INT8U  INI_TIMEOUT_AP;		
			__IO INT8U  CORRECTLY_PRG_SECTORS_NUM[4];		
			__IO INT8U  BKOPS_STATUS[2];		
			__IO INT8U  POWER_OFF_LONG_TIME;		
			__IO INT8U  GENERIC_CMD6_TIME; 		
			__IO INT8U  CACHE_SIZE[4]; 		
			__IO INT8U  Reserved2[255];		
			__IO INT8U  EXT_SUPPORT;		
			__IO INT8U  LARGE_UNIT_SIZE_M1; 		
			__IO INT8U  CONTEXT_CAPABILITIES; 		
			__IO INT8U  TAG_RES_SIZE; 		
			__IO INT8U  TAG_UNIT_SIZE; 		
			__IO INT8U  DATA_TAG_SUPPORT; 		
			__IO INT8U  MAX_PACKED_WRITES; 		
			__IO INT8U  MAX_PACKED_READS; 		
			__IO INT8U  BKOPS_SUPPORT;		
			__IO INT8U  HPI_FEATURES; 		
			__IO INT8U  S_CMD_SET;		
			__IO INT8U  Reserved1[7];	         
		} EXT_CSD;
   __IO INT8U CsdBuf[512];
} EMMCEXT_CSD;


/** 
  * @brief  Card Identification Data: CID Register   
  */
typedef struct
{
  __IO INT8U  ManufacturerID;       /*!< ManufacturerID */
  __IO INT16U OEM_AppliID;          /*!< OEM/Application ID */
  __IO INT32U ProdName1;            /*!< Product Name part1 */
  __IO INT8U  ProdName2;            /*!< Product Name part2*/
  __IO INT8U  ProdRev;              /*!< Product Revision */
  __IO INT32U ProdSN;               /*!< Product Serial Number */
  __IO INT8U  Reserved1;            /*!< Reserved1 */
  __IO INT16U ManufactDate;         /*!< Manufacturing Date */
  __IO INT8U  CID_CRC;              /*!< CID CRC */
  __IO INT8U  Reserved2;            /*!< always 1 */
} EMMC_CID;

/** 
  * @brief SD Card Status 
  */
typedef struct
{
  __IO INT8U DAT_BUS_WIDTH;
  __IO INT8U SECURED_MODE;
  __IO INT16U SD_CARD_TYPE;
  __IO INT32U SIZE_OF_PROTECTED_AREA;
  __IO INT8U SPEED_CLASS;
  __IO INT8U PERFORMANCE_MOVE;
  __IO INT8U AU_SIZE;
  __IO INT16U ERASE_SIZE;
  __IO INT8U ERASE_TIMEOUT;
  __IO INT8U ERASE_OFFSET;
} EmmcCardStatus;


/** 
  * @brief SD Card information 
  */
typedef struct
{
  EMMC_CSD EmmcCsd;
  EMMCEXT_CSD EmmcExtCsd; 
  EMMC_CID EmmcCid;
  INT64U CardCapacity;  /*!< Card Capacity */
  INT32U CardBlockSize; /*!< Card Block Size */
  INT16U RCA;
  INT8U CardType;
}EmmcCardInfo;
   
/**
  * @}
  */


/** @defgroup SDIO_EMMC_Exported_Macros
  * @{
  */
#define SDIO_FIFO_ADDRESS                ((INT32U)0x40012C80)
/** 
  * @brief  SDIO Intialization Frequency (400KHz max)
  */
#define SDIO_INIT_CLK_DIV                ((INT8U)0xFF)
/** 
  * @brief  SDIO Data Transfer Frequency (25MHz max) 
  */
#define SDIO_TRANSFER_CLK_DIV            ((INT8U)0x00) 

#define SD_SDIO_DMA                   DMA2
#define SD_SDIO_DMA_CLK               RCC_AHB1Periph_DMA2
 
#define EMMC_SDIO_DMA_STREAM3	            3
//#define EMMC_SDIO_DMA_STREAM6           6

#ifdef EMMC_SDIO_DMA_STREAM3
 #define EMMC_SDIO_DMA_STREAM            DMA2_Stream3
 #define EMMC_SDIO_DMA_CHANNEL           DMA_Channel_4
 #define EMMC_SDIO_DMA_FLAG_FEIF         DMA_FLAG_FEIF3
 #define EMMC_SDIO_DMA_FLAG_DMEIF        DMA_FLAG_DMEIF3
 #define EMMC_SDIO_DMA_FLAG_TEIF         DMA_FLAG_TEIF3
 #define EMMC_SDIO_DMA_FLAG_HTIF         DMA_FLAG_HTIF3
 #define EMMC_SDIO_DMA_FLAG_TCIF         DMA_FLAG_TCIF3 
 #define EMMC_SDIO_DMA_IRQn              DMA2_Stream3_IRQn
 #define EMMC_SDIO_DMA_IRQHANDLER        DMA2_Stream3_IRQHandler 
#elif defined EMMC_SDIO_DMA_STREAM6
 #define EMMC_SDIO_DMA_STREAM            DMA2_Stream6
 #define EMMC_SDIO_DMA_CHANNEL           DMA_Channel_4
 #define EMMC_SDIO_DMA_FLAG_FEIF         DMA_FLAG_FEIF6
 #define EMMC_SDIO_DMA_FLAG_DMEIF        DMA_FLAG_DMEIF6
 #define EMMC_SDIO_DMA_FLAG_TEIF         DMA_FLAG_TEIF6
 #define EMMC_SDIO_DMA_FLAG_HTIF         DMA_FLAG_HTIF6
 #define EMMC_SDIO_DMA_FLAG_TCIF         DMA_FLAG_TCIF6 
 #define EMMC_SDIO_DMA_IRQn              DMA2_Stream6_IRQn
 #define EMMC_SDIO_DMA_IRQHANDLER        DMA2_Stream6_IRQHandler
#endif /* EMMC_SDIO_DMA_STREAM3 */
/** 
  * @brief SDIO Commands  Index 
  */
#define EMMC_CMD_GO_IDLE_STATE                       ((INT8U)0)
#define EMMC_CMD_SEND_OP_COND                        ((INT8U)1)
#define EMMC_CMD_ALL_SEND_CID                        ((INT8U)2)
#define EMMC_CMD_SET_REL_ADDR                        ((INT8U)3) /*!< SDIO_SEND_REL_ADDR for SD Card */
#define EMMC_CMD_SET_DSR                             ((INT8U)4)
#define EMMC_CMD_SDIO_SEN_OP_COND                    ((INT8U)5)
#define EMMC_CMD_HS_SWITCH                           ((INT8U)6)
#define EMMC_CMD_SEL_DESEL_CARD                      ((INT8U)7)
#define EMMC_CMD_HS_SEND_EXT_CSD                     ((INT8U)8)
#define EMMC_CMD_SEND_CSD                            ((INT8U)9)
#define EMMC_CMD_SEND_CID                            ((INT8U)10)
#define EMMC_CMD_READ_DAT_UNTIL_STOP                 ((INT8U)11) /*!< SD Card doesn't support it */
#define EMMC_CMD_STOP_TRANSMISSION                   ((INT8U)12)
#define EMMC_CMD_SEND_STATUS                         ((INT8U)13)
#define EMMC_CMD_HS_BUSTEST_READ                     ((INT8U)14)
#define EMMC_CMD_GO_INACTIVE_STATE                   ((INT8U)15)
#define EMMC_CMD_SET_BLOCKLEN                        ((INT8U)16)
#define EMMC_CMD_READ_SINGLE_BLOCK                   ((INT8U)17)
#define EMMC_CMD_READ_MULT_BLOCK                     ((INT8U)18)
#define EMMC_CMD_HS_BUSTEST_WRITE                    ((INT8U)19)
#define EMMC_CMD_WRITE_DAT_UNTIL_STOP                ((INT8U)20) /*!< SD Card doesn't support it */
#define EMMC_CMD_SET_BLOCK_COUNT                     ((INT8U)23) /*!< SD Card doesn't support it */
#define EMMC_CMD_WRITE_SINGLE_BLOCK                  ((INT8U)24)
#define EMMC_CMD_WRITE_MULT_BLOCK                    ((INT8U)25)
#define EMMC_CMD_PROG_CID                            ((INT8U)26) /*!< reserved for manufacturers */
#define EMMC_CMD_PROG_CSD                            ((INT8U)27)
#define EMMC_CMD_SET_WRITE_PROT                      ((INT8U)28)
#define EMMC_CMD_CLR_WRITE_PROT                      ((INT8U)29)
#define EMMC_CMD_SEND_WRITE_PROT                     ((INT8U)30)
#define EMMC_CMD_ERASE_GRP_START                     ((INT8U)35) /*!< To set the address of the first write block to be erased.
                                                                  (For MMC card only spec 3.31) */

#define EMMC_CMD_ERASE_GRP_END                       ((INT8U)36) /*!< To set the address of the last write block of the
                                                                  continuous range to be erased. (For MMC card only spec 3.31) */

#define EMMC_CMD_ERASE                               ((INT8U)38)
#define EMMC_CMD_FAST_IO                             ((INT8U)39) /*!< SD Card doesn't support it */
#define EMMC_CMD_GO_IRQ_STATE                        ((INT8U)40) /*!< SD Card doesn't support it */
#define EMMC_CMD_LOCK_UNLOCK                         ((INT8U)42)
#define EMMC_CMD_APP_CMD                             ((INT8U)55)
#define EMMC_CMD_GEN_CMD                             ((INT8U)56)
#define EMMC_CMD_NO_CMD                              ((INT8U)64)



  
/* Uncomment the following line to select the SDIO Data transfer mode */  
#if !defined (SD_DMA_MODE) && !defined (SD_POLLING_MODE)
#define SD_DMA_MODE                                ((INT32U)0x00000000)
//#define SD_POLLING_MODE                            ((INT32U)0x00000002)
#endif

/**
  * @brief  SD detection on its memory slot
  */
#define SD_PRESENT                                 ((INT8U)0x01)
#define SD_NOT_PRESENT                             ((INT8U)0x00)

/** 
  * @brief Supported SD Memory Cards 
  */
#define SDIO_STD_CAPACITY_SD_CARD_V1_1             ((INT32U)0x00000000)
#define SDIO_STD_CAPACITY_SD_CARD_V2_0             ((INT32U)0x00000001)
#define SDIO_HIGH_CAPACITY_SD_CARD                 ((INT32U)0x00000002)
#define SDIO_MULTIMEDIA_CARD                       ((INT32U)0x00000003)
#define SDIO_SECURE_DIGITAL_IO_CARD                ((INT32U)0x00000004)
#define SDIO_HIGH_SPEED_MULTIMEDIA_CARD            ((INT32U)0x00000005)
#define SDIO_SECURE_DIGITAL_IO_COMBO_CARD          ((INT32U)0x00000006)
#define SDIO_HIGH_CAPACITY_MMC_CARD                ((INT32U)0x00000007)

  
/** 
  * @brief  SDIO Static flags, TimeOut, FIFO Address  
  */
#define NULL 0
#define SDIO_STATIC_FLAGS               ((INT32U)0x000005FF)
#define SDIO_CMD0TIMEOUT                ((INT32U)0x01000000)

/** 
  * @brief  Mask for errors Card Status R1 (OCR Register) 
  */
#define EMMC_OCR_ADDR_OUT_OF_RANGE        ((INT32U)0x80000000)
#define EMMC_OCR_ADDR_MISALIGNED          ((INT32U)0x40000000)
#define EMMC_OCR_BLOCK_LEN_ERR            ((INT32U)0x20000000)
#define EMMC_OCR_ERASE_SEQ_ERR            ((INT32U)0x10000000)
#define EMMC_OCR_BAD_ERASE_PARAM          ((INT32U)0x08000000)
#define EMMC_OCR_WRITE_PROT_VIOLATION     ((INT32U)0x04000000)
#define EMMC_OCR_LOCK_UNLOCK_FAILED       ((INT32U)0x01000000)
#define EMMC_OCR_COM_CRC_FAILED           ((INT32U)0x00800000)
#define EMMC_OCR_ILLEGAL_CMD              ((INT32U)0x00400000)
#define EMMC_OCR_CARD_ECC_FAILED          ((INT32U)0x00200000)
#define EMMC_OCR_CC_ERROR                 ((INT32U)0x00100000)
#define EMMC_OCR_GENERAL_UNKNOWN_ERROR    ((INT32U)0x00080000)
#define EMMC_OCR_STREAM_READ_UNDERRUN     ((INT32U)0x00040000)
#define EMMC_OCR_STREAM_WRITE_OVERRUN     ((INT32U)0x00020000)
#define EMMC_OCR_CID_CSD_OVERWRIETE       ((INT32U)0x00010000)
#define HOCR_WP_ERASE_SKIP                ((INT32U)0x00008000)
#define EMMC_OCR_CARD_ECC_DISABLED        ((INT32U)0x00004000)
#define EMMC_OCR_ERASE_RESET              ((INT32U)0x00002000)
#define EMMC_OCR_AKE_SEQ_ERROR            ((INT32U)0x00000008)
#define EMMC_OCR_ERRORBITS                ((INT32U)0xFDFFE008)

/** 
  * @brief  Masks for R6 Response 
  */
#define EMMC_R6_GENERAL_UNKNOWN_ERROR     ((INT32U)0x00002000)
#define EMMC_R6_ILLEGAL_CMD               ((INT32U)0x00004000)
#define EMMC_R6_COM_CRC_FAILED            ((INT32U)0x00008000)

#define EMMC_VOLTAGE_WINDOW_SD            ((INT32U)0x80100000)
#define EMMC_HIGH_CAPACITY                ((INT32U)0x40000000)
#define EMMC_STD_CAPACITY                 ((INT32U)0x00000000)
#define EMMC_CHECK_PATTERN                ((INT32U)0x000001AA)

#define EMMC_MAX_VOLT_TRIAL               ((INT32U)0x0000FFFF)
#define EMMC_ALLZERO                      ((INT32U)0x00000000)

#define EMMC_WIDE_BUS_SUPPORT             ((INT32U)0x00040000)
#define EMMC_SINGLE_BUS_SUPPORT           ((INT32U)0x00010000)
#define EMMC_CARD_LOCKED                  ((INT32U)0x02000000)

#define EMMC_DATATIMEOUT                  ((INT32U)0xFFFFFFFF)
#define EMMC_0TO7BITS                     ((INT32U)0x000000FF)
#define EMMC_8TO15BITS                    ((INT32U)0x0000FF00)
#define EMMC_16TO23BITS                   ((INT32U)0x00FF0000)
#define EMMC_24TO31BITS                   ((INT32U)0xFF000000)
#define EMMC_MAX_DATA_LENGTH              ((INT32U)0x01FFFFFF)

#define EMMC_HALFFIFO                     ((INT32U)0x00000008)
#define EMMC_HALFFIFOBYTES                ((INT32U)0x00000020)

/** 
  * @brief  Command Class Supported 
  */
#define EMMC_CCCC_LOCK_UNLOCK             ((INT32U)0x00000080)
#define EMMC_CCCC_WRITE_PROT              ((INT32U)0x00000040)
#define EMMC_CCCC_ERASE                   ((INT32U)0x00000020)

 
/**
  * @}
  */ 

#define EMMC_OCR_REG             0x40FF8080
#define EMMC_POWER_REG           0x03BB0800
#define EMMC_HIGHSPEED_REG       0x03B90100
#define EMMC_4BIT_REG            0x03B70100
#define EMMC_8BIT_REG            0x03B70200

/** @defgroup SDIO_EMMC_Exported_Functions
  * @{
  */ 
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
void EmmcProcessDMAIRQ(void);
void EmmcLowLevelDMATxConfig(INT32U *BufferSRC, INT32U BufferSize);
void EmmcLowLevelDMARxConfig(INT32U *BufferDST, INT32U BufferSize);
INT8U convert_from_bytes_to_power_of_two(INT16U NumberOfBytes);

extern EmmcCardInfo MyEmmcCardInfo;
/**
  * @}
  */ 
#ifdef __cplusplus
}
#endif

#endif /* __STM324xG_EVAL_SDIO_SD_H */
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */ 

/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
