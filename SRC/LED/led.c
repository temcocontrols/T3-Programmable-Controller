
#include <stdio.h>
#include "reg80390.h"
#include "types.h"


#define  	LED_LATCH 	P3_7
#define		LED_CS0		P2_0
#define		LED_CS1		P2_1

U8_T far Led_Data _at_ 0x88004;

void DELAY_Us(U16_T loop);

void LED_Test(void)
{
	LED_CS0 = 1;LED_CS1 = 1;
//	LED_LATCH = 0;
	while(1)
	{
		LED_CS0 = 0;LED_CS1 = 1;
		LED_LATCH = 0;
		Led_Data = 0x55;		
		LED_LATCH = 1;
		DELAY_Us(100000);
		DELAY_Us(100000);
		DELAY_Us(100000);
		
		LED_CS0 = 1;LED_CS1 = 0;
		LED_LATCH = 0;
		Led_Data = 0xa0;
		LED_LATCH = 1;
		DELAY_Us(100000);
		DELAY_Us(100000);
		DELAY_Us(100000);
	}
/*
//	LED_CS0 = 0;LED_CS1 = 1;
//	Led_Data = 0xaa;

	DELAY_Us(100000);
	DELAY_Us(100000);
	DELAY_Us(100000);
	DELAY_Us(100000);
	DELAY_Us(100000);*/

	
}
