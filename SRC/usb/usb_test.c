/*================================================================================
 * Module Name : usb_test.c
 * Purpose     :
 * Author      : Chelsea Liu
 * Date        : 2009-6-5
 * Notes       :
 * 
 *
 *================================================================================
 */
#include <stdio.h>
#include "CH375INC.H"
#include "reg80390.h"
#include "types.h"

#define  UINT8     unsigned char
#define  UINT16    unsigned short
#define  UINT32    unsigned long
#define  UINT32X   unsigned long xdata
#define  UINT8X    unsigned char xdata
#define  UINT8VX   unsigned char volatile xdata
#define  UINT8VF   unsigned char volatile far

UINT8VF    CH375_CMD_PORT _at_ 0X88001;  
UINT8VF    CH375_DAT_PORT _at_ 0X88000; 
#define    CH375_INT_WIRE    		P2_7     // for test
UINT8VF     DISK_BUFFER[512 * 64] ;

UINT32  DiskStart;    /* start sector of logic disk			LBA */
UINT32  SecPerClus;   /* how many clusters per sector		[13] */
UINT8   RsvdSecCnt;   /* reserved sector 					[14] */
UINT16 	FATSz;
UINT32  BytesPesSec;	// how many bytes per sector						[11]   512
UINT16 	NumFAT;  		// the num of FAT 									[16]
UINT32  RootClus;      // the first cluster NO of the root directory      [44]  
UINT32 	TotalClus;
UINT8 	FlagFAT = 0;
UINT8 	FlagReadEnd = 0;
bit flag_out = 0;


void DELAY_Us(UINT16 loop);
//void //Lcd_Show_String(char pos_x,char pos_y,char* str,unsigned char mode,unsigned char length);
//U8_T IntFlashReadByte(U32_T location/*, U8_T *value*/);


void  mDelaymS( UINT8 delay ) {
  UINT8  i, j, c;
  for ( i = delay; i != 0; i -- ) {
    for ( j = 200; j != 0; j -- ) c += 3;
    for ( j = 200; j != 0; j -- ) c += 3;
  }
}


/*
 *--------------------------------------------------------------------------------
 * void CH375_WR_CMD_PORT( UINT8 cmd )
 * Purpose : write one command
 * Params  : cmd - the command you want to write
 * Returns : none
 * Note    :
 *--------------------------------------------------------------------------------
 */
void CH375_WR_CMD_PORT( UINT8 cmd ) 
{ 
	CH375_CMD_PORT = cmd;
	DELAY_Us(10);
}

/*
 *--------------------------------------------------------------------------------
 * void CH375_WR_DAT_PORT( UINT8 dat )
 * Purpose : write one data
 * Params  : dat - the data you want to write
 * Returns : none
 * Note    :
 *--------------------------------------------------------------------------------
 */
void CH375_WR_DAT_PORT( UINT8 dat ) { 
  CH375_DAT_PORT=dat;        
  DELAY_Us(10);
}

/*
 *--------------------------------------------------------------------------------
 * UINT8 CH375_RD_DAT_PORT( void )
 * Purpose : read a data from port
 * Params  : none
 * Returns : return the data of port
 * Note    :
 *--------------------------------------------------------------------------------
 */
UINT8 CH375_RD_DAT_PORT( void ) {   
	DELAY_Us(12);
  return( CH375_DAT_PORT );    

}

/*
 *--------------------------------------------------------------------------------
 * UINT8   mWaitInterrupt( void )
 * Purpose : enter interrupt and return the status
 * Params  : none
 * Returns : return the status
 * Note    :
 *--------------------------------------------------------------------------------
 */
UINT8 mWaitInterrupt( void ) {  
/* query the CH375_INT_WIRE, if this port is pulled down, indicate an interrupt event */
  while( CH375_INT_WIRE );
  CH375_WR_CMD_PORT( CMD_GET_STATUS ); 
  return( CH375_RD_DAT_PORT( ) );
}

/*
 *--------------------------------------------------------------------------------
 * UINT8  mInitDisk( void )  
 * Purpose : initial disk
 * Params  : 
 * Returns : if success ,return CH_OK, else return CH_ERROR 
 * Note    : some roution are from the fat.obj file which suppiled by the usb vendor
 *--------------------------------------------------------------------------------
 */
UINT8  mInitDisk( void ) {  
  UINT8 Status;
  CH375_WR_CMD_PORT( CMD_GET_STATUS );  /* get status */
  Status = CH375_RD_DAT_PORT( );
  if ( Status == USB_INT_DISCONNECT ) return( Status );  /* USB device disconnect */
  CH375_WR_CMD_PORT( CMD_DISK_INIT );  /* intial disk */
  Status = mWaitInterrupt( );  /* wait interrupt and get status*/
  if ( Status != USB_INT_SUCCESS ) return( Status );   /* fail */
  CH375_WR_CMD_PORT( CMD_DISK_SIZE );   /* get the disk size */
  Status = mWaitInterrupt( );   /* wait interrupt and get status*/
  if( Status != USB_INT_SUCCESS ) {  /* try again if error */
/* if fail, do CMD_DISK_R_SENSE */
	CH375_WR_CMD_PORT( CMD_DISK_R_SENSE );
    mDelaymS( 250 );
    CH375_WR_CMD_PORT( CMD_DISK_SIZE );  /* get the disk size */
    Status = mWaitInterrupt( );  /* wait interrupt and get status*/
  }
  if ( Status != USB_INT_SUCCESS ) return( Status );  /* fail */
  return( 0 );  /* success */
}


/*
 *--------------------------------------------------------------------------------
 * UINT8  mReadSector( UINT32 iLbaStart, UINT8 iSectorCount, UINT8X *oDataBuffer ) 
 * Purpose : read datas of many sectors and store them to the buffer
 * Params  : iLbaStart - start sector
 * 		     iSectorCount - sector num you want to read
 *			 mBufferPoint - buffer for storing datas from sectos	 
 * Returns : if success ,return 0, else return CH_ERROR 
 * Note    : some roution are from the fat.obj file which suppiled by the usb vendor
 *--------------------------------------------------------------------------------
 */
UINT8  mReadSector( UINT32 iLbaStart, UINT8 iSectorCount, UINT8VF *oDataBuffer ) 
{
  UINT16  mBlockCount;
  UINT8  c;

// DISK_READ  0x54 5 parameters 
 /* read data block form usb memory */
  CH375_WR_CMD_PORT( CMD_DISK_READ );  /* 0x54*/
  CH375_WR_DAT_PORT( (UINT8)iLbaStart );   /* start address*/
  CH375_WR_DAT_PORT( (UINT8)( iLbaStart >> 8 ) );
  CH375_WR_DAT_PORT( (UINT8)( iLbaStart >> 16 ) );
  CH375_WR_DAT_PORT( (UINT8)( iLbaStart >> 24 ) ); 
  CH375_WR_DAT_PORT( iSectorCount );  

 // mBlockCount     8 = 512 / 64     
// 512 byte numbers per sectors	  64  data block length 
// printf("enter read\r\n");
// printf("mBlockCount = %d\r\n",(int) iSectorCount * 8);
  for ( mBlockCount = iSectorCount * 8; mBlockCount != 0; mBlockCount -- )
 {

    c = mWaitInterrupt( );  
	
    if ( c == USB_INT_DISK_READ ) {  

      CH375_WR_CMD_PORT( CMD_RD_USB_DATA );  
      c = CH375_RD_DAT_PORT( );  /* get the lenght of remainder datas*/
 	  while ( c -- ) *oDataBuffer++ = CH375_RD_DAT_PORT( );
      CH375_WR_CMD_PORT( CMD_DISK_RD_GO );  /* continue read */
      
    }
    else 
	{
//	printf("c = %d,mBlockCount = %d\r\n",(int)c,(int)mBlockCount);
	break;  /*return error  */
	}
	
  }


  if ( mBlockCount == 0 ) {
    c = mWaitInterrupt( );  /* wait interrupt and return status */
//	printf("c = %d,\r\n",(int)c);
    if ( c== USB_INT_SUCCESS ) return( 0 );  /* success */
  }
  return( c );  /* fail */
}


/*
 *--------------------------------------------------------------------------------
 * UINT16  mGetPointWord( UINT8X *iAddr )
 * Purpose : transform the data to match the mcu
 * Params  :
 * Returns : 
 * Note    : because the mcu is big-end format, need to transform it
 *--------------------------------------------------------------------------------
 */
UINT16  mGetPointWord( UINT8VF *iAddr ) { 
  return( iAddr[0] | (UINT16)iAddr[1] << 8 );
}


/*
 *--------------------------------------------------------------------------------
 * UINT8  mIdenDisk( void )
 * Purpose : identify the current logic disk
 * Params  :
 * Returns : success, return 0, or return error code.
 * Note    : the following analyse is very simple, maybe it is not perfect , 
 *			 if you are familiar with the fat system, you will be clear about it 
 *--------------------------------------------------------------------------------
 */
UINT8  mIdenDisk( void ) {  
  UINT8  Status;
  DiskStart = 0;  	//MBR_LBA=0x00000000 
  Status = mReadSector( 0, 1, DISK_BUFFER );  /* read the boot information of the logic disk */ 
  
  if ( Status != 0 ) return( Status );
  if ( DISK_BUFFER[0] != 0xEB && DISK_BUFFER[0] != 0xE9 )
  {  /* check the flag of the disk, pls refer to the document of FAT  */   
    DiskStart = DISK_BUFFER[0x1C6] | (UINT16)DISK_BUFFER[0x1C7] << 8| (UINT32)DISK_BUFFER[0x1C8] << 16 | (UINT32)DISK_BUFFER[0x1C9] << 24; 
	// DBR_LBA=MBR.PT[0].RelativeSectors [457][456][455][454] 
    Status = mReadSector( DiskStart, 1, DISK_BUFFER );
    if ( Status != 0 ) return( Status );
  }
  SecPerClus = DISK_BUFFER[0x0D];  /* sector numbers per cluster */
  RsvdSecCnt = mGetPointWord( &DISK_BUFFER[0x0e] );   /* reserved cluster numbers of logic disk */
  FATSz = mGetPointWord( &DISK_BUFFER[0x16] );  
  NumFAT = DISK_BUFFER[0x10];
  BytesPesSec = mGetPointWord( &DISK_BUFFER[0x0b]);
  FlagFAT = 0;  // FAT16
  TotalClus = 0xFFF8;
/* the following is for fat 32*/
  if(FATSz == 0)	 // fat 32
	{
	FlagFAT = 1;
	TotalClus = 0x0FFFFFF8;
	FATSz = DISK_BUFFER[0x24] | (UINT16)DISK_BUFFER[0x25] << 8 | (UINT32)DISK_BUFFER[0x26] << 16 | (UINT32)DISK_BUFFER[0x27] << 24;;  
  	RootClus = DISK_BUFFER[0x2C] | (UINT16)DISK_BUFFER[0x2d] << 8 | (UINT32)DISK_BUFFER[0x2e] << 16 | (UINT32)DISK_BUFFER[0x2f] << 24;
	}
	printf("SecPerClus = %d\r\n,RsvdSecCnt = %d\r\n,FATSz = %d\r\n,NumFAT = %d\r\n,BytesPesSec = %d\r\n",(int)SecPerClus,(int)RsvdSecCnt,(int)FATSz,(int)NumFAT,(int)BytesPesSec);
  return( 0 );  /* success*/
}


/*
 *--------------------------------------------------------------------------------
 * UINT16  mLinkCluster( UINT16 iCluster )
 * Purpose : get the link-cluster of the appointed cluster
 * Params  : iCluster - appointed cluster
 * Returns : return the link-cluster
 * Note    : next cluster = (iCluster * FATsiz2) % BytesPesSec
 *--------------------------------------------------------------------------------
 */
UINT16  mLinkCluster( UINT16 iCluster ) {  
  UINT8  Status;
  if(FlagFAT == 0) Status = mReadSector( DiskStart + RsvdSecCnt + iCluster * 2 / BytesPesSec, SecPerClus, DISK_BUFFER ); 
  else Status = mReadSector( DiskStart + RsvdSecCnt + iCluster * 4 / BytesPesSec, SecPerClus, DISK_BUFFER ); 
 
  if ( Status != 0 ) return( 0 );  /* if return 0,error */
  if(FlagFAT == 0) return( mGetPointWord( &DISK_BUFFER[ ( iCluster * 2) % BytesPesSec ] ) );
  else	return( mGetPointWord( &DISK_BUFFER[ ( iCluster * 4) % BytesPesSec ]) );	 
}

/*
 *--------------------------------------------------------------------------------
 * UINT32  mClusterToLba( UINT16 iCluster ) 
 * Purpose : transform the cluster number to LBA sector address
 * Params  : iErrCode - error code
 * Returns :
 * Note    : logic sector NO = DiskStart + RsvdSecCnt + ( iCluster - 2 ) * SecPerClus
 *--------------------------------------------------------------------------------
 */
UINT32  mClusterToLba( UINT16 iCluster ) { 
  //return( DiskStart + RsvdSecCnt + FATSz16 * 2 + 32 + ( iCluster - 2 ) * SecPerClus );
	if(FlagFAT == 0) 	return( DiskStart + RsvdSecCnt + FATSz * NumFAT + 32 + ( iCluster - 2 ) * SecPerClus ) ;
	return( DiskStart + RsvdSecCnt + FATSz * NumFAT + ( iCluster - 2 ) * SecPerClus ) ;
}


/*
 *--------------------------------------------------------------------------------
 * void  mStopIfError( UINT8 iErrCode ) 
 * Purpose : show error imformation
 * Params  : iErrCode - error code
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void  mStopIfError( UINT8 iErrCode ) {  
  if ( iErrCode == 0 ) return;
  printf( "Error status, %02X\n", (UINT16)iErrCode );
//  //Lcd_Show_String(4,5,"USB ERROR",1,18);
  flag_out = 1; /* error handle when reading usb*/
}



/*
 *--------------------------------------------------------------------------------
 * void Usb_Read(void) 
 * Purpose : read usb disk, find file you want and write the data into memory
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void Usb_Read(void) 
{
	UINT8  Status;
	UINT32  loop = 1;
	UINT8 k = 0;
    UINT16  temp = 0;
    UINT16  temp1 = 0;
	UINT32 temp32 = 0;
	bit flag_find = 0;
    unsigned char *CurrentDir;
//	U8_T tempstr[16];
//	U32_T length;
	UINT32  count = 0;
//  	UINT32X  Cluster;	
	printf("enter usb \r\n");
Start:
	DELAY_Us(50000);  	
	DELAY_Us(50000);
	DELAY_Us(50000);
	DELAY_Us(50000);
  	DELAY_Us(50000);
	DELAY_Us(50000);
	DELAY_Us(50000);
	CH375_CMD_PORT = CMD_SET_USB_MODE; DELAY_Us(5);
	CH375_CMD_PORT = 0x06;	DELAY_Us(5);
	CH375_DAT_PORT = 0x55 ;  /* 0x06, test command ,check the usb chip is ready */	
	DELAY_Us(10);	
    printf("CH375_DAT_PORT = %d\r\n",(int)CH375_DAT_PORT);
	
	//Lcd_Show_String(1,1,"1. INSERT USB DISK",1,20);
	
	if(mWaitInterrupt( ) != USB_INT_CONNECT) goto Start; 
	printf("start \r\n");
	//Lcd_Show_String(1,1,"1. USB ready      ",1,20);
    mDelaymS( 250 );  /* delay , wait for the usb disk entering to normal work status */
	
    //init:
	/* initial usb disk, identify the type of the usb disk , it is important*/
	Status = mInitDisk( );  
	//if(Status != 0)	goto init;
	mStopIfError( Status );
	printf("init \r\n");
	mDelaymS( 250 );
	//identify:
	/* identify the file system of Usb Disk ,it is necessay and important*/
	Status = mIdenDisk( );  
	//if(Status != 0)	goto identify;
	mStopIfError( Status );
	printf("identify \r\n");
	//read:
	/* read the root directory of the logic disk, common lengh is 32 sectors*/
	Status = mReadSector( DiskStart + RsvdSecCnt + FATSz * NumFAT, 32,DISK_BUFFER ); 
    //if(Status != 0)	goto read;
	mStopIfError( Status );
	printf("read \r\n");
	/* query the names of the files in the disk, if find "ax1****.bin" ,erase the memory */
	for ( CurrentDir = DISK_BUFFER; ((CurrentDir[0] != 0) && (!flag_find)); CurrentDir += 32 ) 
	{
	    temp++;		      
		if((CurrentDir[8] == 'B') && (CurrentDir[9] == 'I') && (CurrentDir[10] == 'N') && (CurrentDir[0] == 'A') && (CurrentDir[1] == 'X')&& (CurrentDir[2] == '1'))
	    {
			flag_find = 1;
			printf("find\r\n");

		}
	} 
	/* if dont find "ax1****.bin",show "no right file" */
	printf("end\r\n");
	mDelaymS( 250 );
}



