#ifndef __SPI_H
#define __SPI_H

#include "stm32f10x.h"

//这部分应根据具体的连线来修改!
//Mini STM32使用的是PB12作为SD卡的CS脚.
#define	SD_CS_BIG	PCout(5) //SD卡片选引脚					    	  
#define	SD_CS_SMALL	PDout(3)
#define	SD_CS_NEW_TINY	PDout(9)


void SPI1_Init(u8 type);							//初始化SPI1口
void SPI1_SetSpeed(u8 SPI_BaudRatePrescaler);	//设置SPI1速度   
u8 SPI1_ReadWriteByte(u8 TxData);				//SPI1总线读写一个字节

void SPI2_Init(void);							//初始化SPI1口
void SPI2_SetSpeed(u8 SPI_BaudRatePrescaler);	//设置SPI1速度   
u8 SPI2_ReadWriteByte(u8 TxData);				//SPI1总线读写一个字节
void SPI_Select_SD(void);
void SPI_Select_TOP(void);

void SPI3_Init(void);							//初始化SPI1口
u8 SPI3_ReadWriteByte(u8 TxData);
void SPI3_SetSpeed(u8 SPI_BaudRatePrescaler);



#endif
