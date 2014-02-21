/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/
#include "types.h"
#include "../sd/sddriver.h"

#include "integer.h"
#include "diskio.h"

#define BLOCK_SIZE            512 /* Block Size in Bytes */


/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */

DSTATUS disk_initialize (
	BYTE drv				/* Physical drive nmuber (0..) */
)
{
	U8_T Status;
	U8_T try_times = 0;
	/* Supports only single drive */
	if (drv)
	{
		return STA_NOINIT;
	}
/*-------------------------- SD Init ----------------------------- */
	do
	{
		Status = SD_Initialize();
		if(Status == SD_NO_ERR)
			break;
	} while(try_times++ > 50);
	
	if(try_times > 50)
		return STA_NOINIT;
	else
		return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0..) */
)
{
	drv = drv;
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */

DRESULT disk_read (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..255) */
)
{
	drv = drv;
	if (count > 1)
	{
//		SD_ReadMultiBlocks(buff, sector*BLOCK_SIZE, BLOCK_SIZE, count);
		SD_ReadMultiBlock(sector, count, buff);

//		/* Check if the Transfer is finished */
//	     SD_WaitReadOperation();  //循环查询dma传输是否结束
	
//	    /* Wait until end of DMA transfer */
//	    while(SD_GetStatus() != SD_TRANSFER_OK);

	}
	else
	{
		
//		SD_ReadBlock(buff, sector*BLOCK_SIZE, BLOCK_SIZE);
		SD_ReadBlock(sector, buff);

//		/* Check if the Transfer is finished */
//	     SD_WaitReadOperation();  //循环查询dma传输是否结束
	
//	    /* Wait until end of DMA transfer */
//	    while(SD_GetStatus() != SD_TRANSFER_OK);

	}
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..255) */
)
{
	drv = drv;
	if (count > 1)
	{
//		SD_WriteMultiBlocks((uint8_t *)buff, sector*BLOCK_SIZE, BLOCK_SIZE, count);
		SD_WriteMultiBlock(sector, count, buff);
//		  /* Check if the Transfer is finished */
//	  	 SD_WaitWriteOperation();	   //等待dma传输结束
//	    while(SD_GetStatus() != SD_TRANSFER_OK); //等待sdio到sd卡传输结束
	}
	else
	{
//		SD_WriteBlock((uint8_t *)buff,sector*BLOCK_SIZE, BLOCK_SIZE);
		SD_WriteBlock(sector, buff);
//		  /* Check if the Transfer is finished */
//	   		SD_WaitWriteOperation();	   //等待dma传输结束
//	    while(SD_GetStatus() != SD_TRANSFER_OK); //等待sdio到sd卡传输结束
	}
	return RES_OK;
}
#endif /* _READONLY */




/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */

DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	drv = drv;
	ctrl = ctrl;
	buff = buff;
	return RES_OK;
}
							 
/*-----------------------------------------------------------------------*/
/* Get current time                                                      */
/*-----------------------------------------------------------------------*/ 
DWORD get_fattime(void)
{

 	return 0;

} 
