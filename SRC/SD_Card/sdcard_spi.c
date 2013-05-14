/*
note:	this file for testing SD card using hardware SPI, we use software SPI before.
author: chelsea
date:	Aug 23th, 2011 

*/

#include <intrins.h>
#include "mmc.h"
#include "fat.h"
#include "stdio.h"


#include "spi.h"
#include "spiapi.h"


extern void DELAY_Us(U16_T loop);
S8_T uart0_PutChar (S8_T c);
S8_T uart0_GetKey (void);

U8_T mmc_init ()
{
	U8_T data Timeout = 0;
	U8_T data a,b;
	U8_T CMD[] = {0x40,0x00,0x00,0x00,0x00,0x95};	

	for ( b = 0;b < 0x0f;b++) /* send 16 bytes 0xff */
	{
		SPI_ByteWrite(0xff);
	}
	//mmc_write_command (CMD);
	while(mmc_write_command (CMD) !=1)
	{
		if (Timeout++ > 5)
		{	
			uart0_PutChar('1');		
			return(1); 			
		}
		
	}
	
//	uart0_PutChar('2');
	Timeout = 0;
	CMD[0] = 0x41; // Commando 1
	CMD[5] = 0xFF;
	while( mmc_write_command (CMD) !=0)
	{
		if (Timeout++ > 100)
		{			
			return(2); 
		}
		uart0_PutChar('2');
	}
//	mmc_write_command (CMD);
	uart0_PutChar('0');
	return(0);  
//	printf("success\r\n");  
	while(1){};//  成功
}



U8_T mmc_write_command (U8_T  *cmd)
{
	U8_T data tmp = 0xff;
	U8_T data retry = 0;
	U16_T data Timeout = 0;
	U8_T a;

	SPI_ByteWrite(0xFF);  // send 8 clock signal
	
	for ( a = 0;a < 0x06;a++) //send 6 Byte Command 
	{
		SPI_ByteWrite(cmd[a]);
	}

	SPI_ByteWrite(0xff);
	while (tmp == 0xff)	
	{
		SPI_GetData(&tmp);
		Timeout++;
		if(Timeout > 500)
		{			
			break;
		}		
	}
	uart0_PutChar('a');
	return(tmp);
	// get 16 bits reponse 
/*	SPI_GetData(&tmp); //read the first byte,ignore it. 
	do 
	{  
		SPI_GetData(&tmp);  // get the last 8 bits
		retry++;
		uart0_PutChar('a');
	}
	while((tmp == 0xff) && (retry < 100)); 

	return(tmp);


	while(tmp == 0xff)	
	{
		SPI_GetData(&tmp);
		if (Timeout++ > 500)
		{
			break;
		}
//		ComSendByte( tmp);
//		ComSendByte('b');
	}
	*/

}




U8_T mmc_write_sector (U32_T addr,U8_T *Buffer)
{
	U8_T data tmp;
	U16_T data a ;
	U8_T cmd[] = {0x58,0x00,0x00,0x00,0x00,0xFF}; 
	U8_T retry = 0;

//	LED_MMC_WR=0;	  
	  
	addr = addr << 9; //addr = addr * 512
	
	cmd[1] = ((addr & 0xFF000000) >>24 );
	cmd[2] = ((addr & 0x00FF0000) >>16 );
	cmd[3] = ((addr & 0x0000FF00) >>8 );

	tmp = mmc_write_command (cmd);
	if (tmp != 0)
	{
//		LED_MMC_WR=1;	  
		return(tmp);
	}
			
	for (a = 0;a < 100;a++)
	{
		SPI_GetData(&tmp); //  tbd:
	}
	
	SPI_ByteWrite(0xFE);	
	
	for ( a = 0;a < 512;a++)
	{
		SPI_ByteWrite(*Buffer++);
	}
	
	SPI_ByteWrite(0xFF); //Schreibt Dummy CRC
	SPI_ByteWrite(0xFF); //CRC Code wird nicht benutzt

//	while (SPI_GetData() != 0xff){};
	// tbd:-----
	// get 16 bits reponse 
	SPI_GetData(&tmp); //read the first byte,ignore it. 
	do 
	{  
	  	SPI_GetData(&tmp);  // get the last 8 bits
	  	retry++;
	}
	while((tmp == 0xff) && (retry < 100)); 

/*	SPI_GetData(&tmp);
	while ( tmp != 0xff)
	{
		SPI_GetData(&tmp);
	};
*/	
//	MMC_Disable();
	
//	LED_MMC_WR=1;	  
	return(0);
}


void mmc_read_block(U8_T *cmd,U8_T *Buffer,U16_T Bytes)
{	
	U16_T  a;
	U8_T tmp;
	U8_T retry = 0;

	if (mmc_write_command (cmd) != 0)
	{
		 return;
	}
	
// tbd:-----
// get 16 bits reponse 
	SPI_GetData(&tmp); //read the first byte,ignore it. 
	do 
	{  
	  	SPI_GetData(&tmp);  // get the last 8 bits
	  	retry++;
	}
	while((tmp == 0xff) && (retry < 100)); 

/*	SPI_GetData(&tmp);
	while ( tmp != 0xfe)
	{
		SPI_GetData(&tmp);
	};*/
//	while (SPI_GetData() != 0xfe){};

	for ( a = 0;a < Bytes;a++)
	{
		SPI_GetData(&tmp);
		*Buffer++ = tmp;
	}
	SPI_GetData(&tmp);//CRC - Byte wird nicht ausgewertet
	SPI_GetData(&tmp);//CRC - Byte wird nicht ausgewertet
	
//	MMC_Disable();
	
	return;
}


U8_T mmc_read_sector (U32_T addr,U8_T *Buffer)
{	
	U8_T cmd[] = {0x51,0x00,0x00,0x00,0x00,0xFF}; 
	
//	LED_MMC_WR=0;	  
	addr = addr << 9; //addr = addr * 512

	cmd[1] = ((addr & 0xFF000000) >>24 );
	cmd[2] = ((addr & 0x00FF0000) >>16 );
	cmd[3] = ((addr & 0x0000FF00) >>8 );

    mmc_read_block(cmd,Buffer,512);

//	LED_MMC_WR=1;	  
	return(0);
}

/* test code */

U16_T   xdata   datatemp=0;


void SD_test(void)
{
	S8_T input;
	U8_T temp = 0;
	U16_T i;
	U16_T xdata Clustervar=0;
	U32_T xdata Size = 0;
	U8_T xdata Dir_Attrib = 0;

// select SS0
	uart0_PutChar('1');
//	printf("test sd\r\n");
	DELAY_Us(1000);
	SPI_Setup(SPI_SSO_ENB|SPI_MST_SEL|SPI_SS_AUTO|SPI_ENB, SPI_STCFIE, 0x80, SLAVE_SEL_0); // 25M

	
	//while(1)
	{
	//	scanf("input = %c\r\n",&input);
	//	if(input == '1') // write
		{
		//	printf("write\r\n");
		//	uart0_PutChar('1');
			temp = mmc_init ();//初始化SD卡		
			fat_cluster_data_store();    
			datatemp=0;
			if(fat_write_file_lock ("TEST    TXT"))//文件名8个字节  
			{						 
				//printf("test.txt\r\n");
	 			for(i=0;i<512;i++)//一次写一个扇区，512个字节
				{
					SectorBuffer[i] = datatemp+0x30;//写入缓冲，ASCII码
					datatemp++;
					if(datatemp == 5)//存储数据0-9循环
						datatemp = 0;
				}
				fat_write_file();  //写入数据 
			}
		}
	//	if(input == '2') // read
		{
		//	printf("read\r\n");
		//	uart0_PutChar('2');
			temp = mmc_init ();	
			fat_cluster_data_store();			
			if (fat_search_file("TEST    TXT",&Clustervar,&Size,&Dir_Attrib) == 1)//创建文件名
			{
				uart0_PutChar('t');
				if(fat_read_file ( Clustervar,0,512))//512个字节
				{	
					uart0_PutChar('4');				
					for(Clustervar=0;Clustervar<512;Clustervar++)
					{
					
					//	printf("%c",SectorBuffer[Clustervar]);
						uart0_PutChar(SectorBuffer[Clustervar]);
						//ComSendByte( SectorBuffer[Clustervar]);//读出的数据从串口发送出
						//ComSendByte( 'a');//读出的数据从串口发送出
						
					}	
				}
			}
		}
	}
}



