#ifndef MAIN_H
#define MAIIN_H




#include 	"types.h"
#include 	"ax11000.h"
#include 	"uart.h"
#include	"stoe.h"
#include	"uip.h"
#include	"uip_arp.h"
#include	"adapter.h"
#include	"httpd.h"
#include 	"projdefs.h"
#include 	"portable.h"
#include 	"task.h"
#include	"queue.h"
#include	"semphr.h" 
#include 	"gconfig.h"
#include 	"gudpbc.h"
//#include 	"sntpc.h"
#include 	"mstimer.h"
#include    "dma.h"
#include 	"delay.h" 
#include	"spi.h"
#include	"spiapi.h"


#include 	"output.h"
#include 	"input.h"
#include 	"lcd.h"
#include 	"Display.h"
#include	"key.h"
#include 	"e2prom.h"
#include 	"clock.h"
#include 	"define.h"
#include 	"serial.h"
#include    "flash.h"
#include	"commsub.h"	 
#include 	"scan.h"
#include 	"flash_schedule.h"
//#include 	"decode.h"



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <absacc.h>


// BACNET
#include "bacdef.h"
#include "gudpmc.h"
#include "apdu.h"
/* hardware layer includes */
#include "timer.h"
#include "rs485.h"
/* BACnet Stack includes */
#include "datalink.h"
#include "npdu.h"
#include "handlers.h"
#include "client.h"
#include "txbuf.h"
#include "dcc.h"
#include "iam.h"
/* BACnet objects */
#include "device.h"
#include "bo.h"	
#include "..\bacnet\av.h"
#include "bv.h"
#include "wp.h"
#include "bip.h"
#include "dlenv.h"
#include "device.h"
#include "config.h"

#include "ud_str.h"
#include "user_data.h"





#define  RELAY_LATCH  P3_3
#define  DI1_LATCH 	  P1_7
#define  DI2_LATCH 	  P1_3
#define  KEY_LATCH    P2_2
#define  UART0_TXEN 	  P1_5
#define  UART1_TXEN 	  P1_6

#define  I2C_SCL	 P3_7	
#define  I2C_SDA	 P2_3


#define  LCD_CLK  	   P1_4
#define  LCD_DATA 	   P2_3
#define  LCD_CS 		P3_2
#define  LCD_A0 		P3_1
#define  LCD_RESET 		P3_0
#define  BACKLIT		P3_6 


#define	 KEYPAD			P0
#define  KEY_PRO 	   P0_2
#define  KEY_SEL 	   P0_3
#define  KEY_DOWN 		P0_6
#define  KEY_UP 		P0_7
//#define  KEY_COM 		P3_0



/* Definition of Digital Inputs */
#define DI1		P0
#define DI2		P0

#define  RELAY1_8		P0
#define  RELAY_9 	   P1_2
#define  RELAY_10 	   P1_1

#define AI_CHANNEL 10
#define DI1_CHANNEL	8
#define DI2_CHANNEL 8
#define DO_CHANNEL 10

#define CHS_DI1	0		
#define CHS_DI2 1

/*enum
{
	OFF = 0, TIMER, ON	
};*/



extern U16_T far Test[50];
extern U8_T far ChangeFlash;
extern U8_T far WriteFlash;




void E2prom_Initial(void);
void watchdog(void);

bit read_PIC_AO( void);
void TCPIP_Task(void)reentrant ;


void set_default_parameters(void);

void control_logic(void); 
void Updata_Clock(void);




extern U8_T IDATA ExecuteRuntimeFlag;
extern U8_T flag_update; 




void SPI_Roution(void);
void Update_InputLed(void);
void refresh_led(void);
bit read_pic_version( void);
void pic_relay(unsigned int set_relay);

signed int RangeConverter(U8_T function, signed int para,U8_T i,U16_T cal);


#endif

