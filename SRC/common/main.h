#ifndef MAIN_H
#define MAIN_H




//#define MINI  1
#define	CM5	2

#define MINI_BIG	 1
#define MINI_SMALL	 2


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
#include 	"output.h"
#include 	"e2prom.h"
#include 	"clock.h"
#include 	"define.h"
#include    "hsuart.h"
#include	"spi.h"
#include	"spiapi.h"
#include 	"comm.h"
#include    "flash.h"
#include 	"delay.h"
//#include    "schedule.h"

#include 	"gconfig.h"
#include 	"gudpbc.h"
//#include 	"sntpc.h"
#include 	"mstimer.h"
#include    "dma.h"

#include 	"lcd.h"
#include	"display.h"
#include	"key.h"
#include 	"commsub.h"
#include 	"scan.h"
#include 	"serial.h"
#include 	"input.h"


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <absacc.h>

#include "ch375_com.h"


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


extern xTaskHandle xDisplayTask;		/* handle for display task */
extern xTaskHandle xDisplayCheckTask;  /* handle for check display status task */
extern xTaskHandle xHandler_Output;
extern xTaskHandle xKeyTask;
extern xTaskHandle far xHandleUSB;

extern xTaskHandle xSoftWatchTask;
extern xTaskHandle xHandleTcp;
extern xTaskHandle far xHandleSchedule;
extern xTaskHandle far xHandleBACnetComm;
extern xTaskHandle far xHandleBacnetControl;
extern xTaskHandle xHandleCommon;
extern xTaskHandle xHandler_SPI;
extern xTaskHandle xdata Handle_MainSerial;
extern xTaskHandle Handle_Scan, Handle_ParameterOperation;


								

extern bit flagLED_ether_tx;
extern bit flagLED_ether_rx;
extern bit flagLED_uart0_rx;
extern bit flagLED_uart0_tx;
extern bit flagLED_uart1_rx; 
extern bit flagLED_uart1_tx;
extern bit flagLED_uart2_rx;
extern bit flagLED_uart2_tx;
extern bit flagLED_usb_rx;
extern bit flagLED_usb_tx;

extern U8_T uart0_heartbeat;
extern U8_T uart1_heartbeat;
extern U8_T uart2_heartbeat;
extern U8_T etr_heartbeat; 
extern U8_T usb_heartbeat; 

extern unsigned int far Test[50];

void vStartHandleMstpTasks( unsigned char uxPriority);	
void vStartSerialTasks( unsigned char uxPriority);
void initSerial(void);

void E2prom_Initial(void);
void watchdog(void);

bit read_PIC_AO( void);
void TCPIP_Task(void)reentrant ;


void set_default_parameters(void);

void control_logic(void); 
void Updata_Clock(void);


//void Start_Comm_Top(void);
//void Stop_Comm_Top(void);

extern U8_T IDATA ExecuteRuntimeFlag;
extern U8_T flag_update;




void SPI_Roution(void);
void Update_InputLed(void);
void refresh_led(void);
bit read_pic_version( void);
void pic_relay(unsigned int set_relay);

signed int RangeConverter(unsigned char function, signed int para,unsigned char i,unsigned int cal);
void responseCmd(U8_T type,U8_T* pData,HTTP_SERVER_CONN * pHttpConn);  
void USB_task(void);

void Set_transaction_ID(U8_T *str, U16_T id, U16_T num);
extern U16_T transaction_id;
extern U16_T transaction_num;
extern U8_T	TcpSocket_ME;
extern U8_T	TcpIp_to_sub;
extern U8_T count_resume_scan;

extern U8_T count_sub_comm;

//#define  KEY_COM 		P2_0


#define SERIAL  0
#define TCP		1
#define USB		2




// MINI	
#if defined(MINI)

#define  LCD_CLK  	   P1_3
#define  LCD_DATA 	   P1_2
#define  LCD_CS 		P1_4
#define  LCD_A0 		P1_5
#define  LCD_RESET 		P1_6
#define  BACKLIT		P1_7 

#define  RESET_8051	 	P2_3

#define	 KEYPAD			P3
#define  KEY_PRO 	    P3_4
#define  KEY_SEL 	    P3_5
#define  KEY_DOWN 		P3_6
#define  KEY_UP 		P3_7


#define MINI_PIC_REV 0x01  // pic882
#define MINI_BIG_HW	   18
#define MINI_SMALL_HW  2

#define  CHSEL3 P3_3

#define UART_MAIN 	1
#define UART_SUB1  	0
#define UART_SUB2  	2
#define UART_ZIG  	1  // ZIGBEE



#define UART0_TXEN	P2_2 //P1_1
#define UART2_TXEN	P2_0 //P2_2 

#define SUB1_TXEN	 UART0_TXEN
#define SUB2_TXEN	 UART2_TXEN

#define MAIN_UART_SBUF	SBUF1
#define SUB1_UART_SBUF	SBUF0
#define ZIG_UART_SBUF	SBUF1

#define MAIN_UART_VECTOR  6 // UART1
#define SUB1_UART_VECTOR  4 // UART0
#define ZIG_UART_VECTOR  6 // UART0


#define MAIN_UART_RI  RI1 // UART1
#define SUB1_UART_RI  RI0 // UART0
#define ZIG_UART_RI  RI1 // UART1

#define MAIN_UART_TI  TI1 // UART1
#define SUB1_UART_TI  TI0 // UART0
#define ZIG_UART_TI  TI1 // UART1

#else // defined(CM5)

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


#define CM5_PIC_REV 0x01  // pic688
#define CM5_HW  8

#define UART_MAIN  1
#define UART_SUB1  0

#define DI1		P0
#define DI2		P0
#define  RELAY1_8		P0
#define  RELAY_9 	   P1_2
#define  RELAY_10 	   P1_1	
#define  RELAY_LATCH  P3_3
#define  DI1_LATCH 	  P1_7
#define  DI2_LATCH 	  P1_3
#define  KEY_LATCH    P2_2
#define  UART0_TXEN 	  P1_5
#define  UART1_TXEN 	  P1_6


#define AI_CHANNEL 10
#define DI1_CHANNEL	8
#define DI2_CHANNEL 8
#define DO_CHANNEL 10
#define CHS_DI1	0		
#define CHS_DI2 1

#define MAIN_TXEN	 UART1_TXEN
#define SUB1_TXEN	 UART0_TXEN

#define MAIN_UART_SBUF	SBUF1
#define SUB1_UART_SBUF	SBUF0

#define MAIN_UART_VECTOR  6 // UART1
#define SUB1_UART_VECTOR  4 // UART0

#define MAIN_UART_RI  RI1 // UART1
#define SUB1_UART_RI  RI0 // UART0

#define MAIN_UART_TI  TI1 // UART1
#define SUB1_UART_TI  TI0 // UART0

#endif




extern U8_T far ChangeFlash;
extern U8_T far WriteFlash;
extern U8_T UpIndex;
extern U16_T far Test[50];

void responseCmd(U8_T type,U8_T* pData,HTTP_SERVER_CONN * pHttpConn); 



#endif

