#ifndef MAIN_H
#define MAIN_H



#include "product.h" 

#include 	"types.h"

#if (ASIX_MINI || ASIX_CM5)

#include 	"ax11000.h"
#include 	"uart.h"
#include	"stoe.h"
#include	"uip.h"
#include	"uip_arp.h"
#include	"adapter.h"
#include	"modbustcp.h"
#include 	"projdefs.h"
#include 	"portable.h"
#include 	"task.h"
#include	"queue.h"
#include	"semphr.h" 
#include 	"output.h"
#include 	"e2prom.h"
#include 	"clock.h"
#include  "hsuart.h"
#include	"spi.h"
#include	"spiapi.h"
#include 	"comm.h"
#include  "flash.h"
#include 	"delay.h"
#include 	"gconfig.h"
#include 	"gudpbc.h"
#include 	"mstimer.h"
#include   "dma.h"
#include  "pca.h"



#include 	"input.h"
#include "ch375usb.h"  
#include "usb.h"
#include "dyndns.h"
#include "dyndns_app.h"
#include "dnsc.h"
#include "time_sync.h"
#include "remote.h"



extern xQueueHandle xLCDQueue;


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
extern xTaskHandle far xHandleMSTP;
extern xTaskHandle xHandleCommon;
extern xTaskHandle xHandler_SPI;
extern xTaskHandle xdata Handle_MainSerial;
extern xTaskHandle Handle_Scan, Handle_Scan_net;
extern xTaskHandle far xHandleMornitor_task;
extern xTaskHandle far xHandleLCD_task;
extern xTaskHandle far Handle_SampleDI;




#endif

#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)

#include "stm32f10x.h"
#include "stdint.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "usart.h"
#include "delay.h"
#include "24cxx.h"
#include "spi.h"
#include "flash.h"
#include "stmflash.h"
#include "mmc_sd.h"
#include "dma.h"
#include "vmalloc.h"
//#include "ff.h"
//#include "fattester.h"
//#include "exfuns.h"
//#include "enc28j60.h"
#include "timerx.h"
#include "uip.h"
#include "uip_arp.h"
#include "tapdev.h"
#include "usb_app.h"
#include "rtc.h"
#include "resolv.h"
#include "sdcard.h"
#include "remote_connect.h"
#include "dyndns.h"
#include "dyndns_app.h"
#include "stm32f10x_adc.h"

#define reentrant

#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <absacc.h>
#include 	"define.h"
#include 	"commsub.h"
#include 	"scan.h"
#include 	"rs485.h"
#include "point.h"
#include "alarm.h"
#include "bacnet.h"
#include "ud_str.h"
#include "user_data.h"
#include 	"serial.h"
#include "flash_user.h"
#include 	"lcd.h"
#include	"display.h"
#include	"key.h"
#include 	"pic.h"
#include 	"sntpc.h"



//#define BACNET_PORT 47808




extern U8_T flag_Updata_Clock;


#if STORE_TO_SD
extern xSemaphoreHandle sem_SPI;

#endif



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
extern xTaskHandle far xHandleMSTP;
extern xTaskHandle xHandleCommon;
extern xTaskHandle xHandler_SPI;
extern xTaskHandle xdata Handle_MainSerial;
extern xTaskHandle Handle_Scan, Handle_Scan_net;
extern xTaskHandle Handle_COV;
extern xTaskHandle far xHandleMornitor_task;
extern xTaskHandle far xHandleLCD_task;
extern xTaskHandle far Handle_SampleDI;
extern xTaskHandle far xHandleLedRefresh;


extern xSemaphoreHandle sem_subnet_tx_uart0;
extern xSemaphoreHandle sem_subnet_tx_uart2;
extern xSemaphoreHandle sem_subnet_tx_uart1;


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


extern uint16_t pdu_len;  
extern BACNET_ADDRESS far src;

extern U16_T far Test[50];

extern U16_T far AO_feedback[16];

typedef	struct
{
	U16_T count[15];
	U16_T old_count[15];
	U8_T  enable[15];
	U8_T  inactive_count[15];
}STR_Task_Test;



unsigned int Filter(unsigned char channel,unsigned int input);

extern STR_Task_Test far task_test;
extern U8_T current_task;
//void GSM_Test();

void vStartHandleMstpTasks( unsigned char uxPriority);	
void vStartSerialTasks( unsigned char uxPriority);
void initSerial(void);

void E2prom_Initial(void);
void watchdog(void);

bit read_PIC_AO( void);


void set_default_parameters(void);

void control_logic(void); 
//void Updata_Clock(void);

extern S8_T far time[20];

extern U8_T IDATA ExecuteRuntimeFlag;
extern U8_T flag_update;
extern U8_T far SD_exist;
extern uint8_t far open_file_flag[24][256];
#if STORE_TO_SD	
//U8_T exfuns_init(void);	
U8_T SD_Initialize(void);
U8_T Write_SD(U16_T file_no,U8_T index,U8_T ana_dig,U32_T star_pos);
U8_T Read_SD(U16_T file_no,U8_T index,U8_T ana_dig,U32_T star_pos);
void check_SD_exist(void);
void check_SD_PnP(void);
#endif


void SPI_Roution(void);
void Update_InputLed(void);
void refresh_led(void);
bit read_pic_version( void);
void pic_relay(unsigned int set_relay);

signed int RangeConverter(unsigned char function, signed int para,unsigned char i,unsigned int cal);
//void responseCmd(U8_T type,U8_T* pData,MODBUSTCP_SERVER_CONN * pHttpConn);  


void Set_transaction_ID(U8_T *str, U16_T id, U16_T num);
extern U16_T transaction_id;
extern U16_T transaction_num;
extern U8_T	TcpSocket_ME;


#define SERIAL  0
#define TCP		1
#define USB		2
#define GSM		3
#define WIFI  5

#define BAC_TO_MODBUS 4

//#if defined(VAV)



//#define  LCD_CLK  	   P1_3
//#define  LCD_DATA 	   P1_2
//#define  LCD_CS 		P2_0
//#define  LCD_A0 		P1_1
//#define  LCD_RESET 		P1_6
//#define  BACKLIT		P1_7 


//#endif

	

// for rev2
//#define UART0_TXEN_VAV	P1_5 
//#define UART2_TXEN_VAV	P1_4
//#define LED_AO_1   P2_2		// for VAV
//#define LED_AO_2   P2_3		// for VAV
//#define LED_BEAT	 P2_7		// for VAV
//#define AI_TEMP		 P2_4 	// for VAV
//#define AI_0_10V	 P2_5  	// for VAV
//#define AI_0_20MA	 P2_6		// for VAV
//#define DO1        P1_0		// for VAV

// for VAV rev3, remap IO

#if ASIX_MINI

#define UART0_TXEN_VAV	P1_1 
#define UART2_TXEN_VAV	P1_0
#define LED_AO_1   P3_0		// for VAV
#define LED_AO_2   P3_1		// for VAV
#define LED_BEAT	 P2_7		// for VAV
#define AI_TEMP		 P2_4 	// for VAV
#define AI_0_10V	 P2_5  	// for VAV
#define AI_0_20MA	 P2_6		// for VAV
#define DO1        P3_2   // P1_0		// for VAV

#define  LCD_CLK  	   P1_3
#define  LCD_DATA 	   P1_2
#define  LCD_CS 		P1_4
#define  LCD_A0 		P1_5
#define  LCD_RESET 		P1_6
#define  BACKLIT		P1_7 

#define  RESET_TOP	 	P2_3

#define	 KEYPAD			P3
#define  KEY_PRO 	    P3_4
#define  KEY_SEL 	    P3_5
#define  KEY_DOWN 		P3_6
#define  KEY_UP 		P3_7
#define  CHSEL3 		P3_3 
#define UART0_TXEN_BIG	P2_2 //P1_1
#define UART0_TXEN_TINY	P2_2 //P1_1

#define UART2_TXEN_BIG 	P2_0 //P2_2 
#define UART2_TXEN_TINY	P2_0 //P2_2 
#endif

#if ARM_MINI

#define UART2_TXEN_VAV	PAout(0)//P1_0

#define  LCD_CLK  	   PDout(10)
#define  LCD_DATA 	   PDout(9)
#define  LCD_CS 		   PDout(8)
#define  LCD_A0 		 	 PDout(6)
#define  LCD_RESET 		 PFout(11)
#define  BACKLIT		 	 PGout(6)

#define  RESET_TOP	 	 PFout(7)//PDout(3)   ARM_BB && ARM_LB can reboot TOP

#define  KEY_PRO 	    PEin(12)//P3_4
#define  KEY_SEL 	    PEin(8)//P3_5
#define  KEY_DOWN 		PEin(9)//P3_6
#define  KEY_UP 			PEin(10)//P3_7


#define UART2_TXEN_BIG 	PEout(11)//P2_0
#define UART2_TXEN_TINY	PEout(11)//P2_0 

#define UART0_TXEN_BIG			PCout(2)//P2_2 
#define UART0_TXEN_TINY     PDout(8)

#define UART1_SW        PCout(3)  // 0 - RS232, 1 - ZIGBEE

#define	TOP_CS	PCout(1) 

/**** LED FOR T3_BAC_ROUTER******/
#define LED_ROUTER_HEART 				PGout(6)
#define LED_ROUTER_SUB_RS485_RX 	PGout(7)
#define LED_ROUTER_SUB_RS485_TX 	PGout(8)
#define LED_ROUTER_MAIN_RS485_RX 	PGout(9)
#define LED_ROUTER_MAIN_RS485_TX 	PGout(10)

#endif

#if ARM_TSTAT_WIFI

#include "LCD_TSTAT.h"
#include "voc.h"

#define  LCD_SCL  	   PEout(4)
#define  LCD_SDA 	     PEout(5)
#define  LCD_CS 		   PEout(3)
#define  LCD_RS 		 	 PBout(6)  
#define  LCD_RES 		   PEout(2) // reset
#define  BACKLIT		 	 PAout(0)


#define  KEY_PRO 	    PAin(12)//P3_4
#define  KEY_SEL 	    PAin(13)//P3_5
#define  KEY_DOWN 		PAin(14)//P3_6
#define  KEY_UP 			PAin(15)//P3_7


#define UART2_TXEN_BIG 	PEout(11)//P2_0
#define UART0_TXEN_BIG	PAout(8)//P2_2 



#endif

#define UART1_VECTOR  6 // UART1
#define UART0_VECTOR  4 // UART0
#define MINI_PIC_REV 0x01  // pic882


#if ASIX_CM5 

#define CM5_PIC_REV 0x01  // pic688
#define CM5_HW  		8


#define  BACKLIT		   P2_4
#define  LCD_SCL  	   P3_3
#define  LCD_SDA 	     P2_3
#define  LCD_CS 			 P2_5
#define  LCD_RES 			 P2_7
#define  LCD_RS 		 P2_6

#define	 KEYPAD			 P1
#define  KEY_PRO 	   P1_0
#define  KEY_SEL 	   P1_1
#define  KEY_DOWN 	 P1_2
#define  KEY_UP 		 P1_3

#define  DI1_LATCH 	  P3_0
//#define  DI2_LATCH 	  P1_3
#define  KEY_LATCH    P2_2
#define  UART0_TXEN 	  P3_2
#define  UART2_TXEN 	  P3_1

#define DI1		P1


#define AI_CHANNEL 10
#define DI1_CHANNEL	8
#define DI2_CHANNEL 8
#define DO_CHANNEL 10
#define CHS_DI1	0		
#define CHS_DI2 1


#endif


#if ARM_CM5 


#define CM5_HW  		9


#define  BACKLIT		   PFout(11)
#define  LCD_SCL  	   PDout(10)
#define  LCD_SDA 	     PDout(9)
#define  LCD_CS 			 PDout(8)
#define  LCD_RES 			 PFout(10)  // reset
#define  LCD_RS 		 	 PDout(6)



//#define  DI1_LATCH 	  P3_0
//#define  KEY_LATCH    P2_2
#define  UART0_TXEN 	  PCout(2)
#define  UART2_TXEN 	  PEout(11)



//#define DI1		P1
//#define AI_CHANNEL 10
//#define DI1_CHANNEL	8
//#define DI2_CHANNEL 8
//#define DO_CHANNEL 10
//#define CHS_DI1	0		
//#define CHS_DI2 1

void Sampel_AI_Task(void);
void Sampel_DI_Task(void);
#endif




extern U8_T far ChangeFlash;
extern U8_T far WriteFlash;
extern U8_T UpIndex;
//extern U8_T flag_retransmit;
#if (ASIX_MINI || ASIX_CM5)
void responseCmd(U8_T type,U8_T* pData,MODBUSTCP_SERVER_CONN * pHttpConn); 
extern xSemaphoreHandle xSemaphore_tcp_send;
extern xSemaphoreHandle xSemaphore_udp_receive;
void TCPIP_Task(void) reentrant;

typedef struct
{
	U8_T flag;  
	U32_T ip;  
	U32_T time;	
	U8_T intial_count;
	U16_T lose_count;
}STR_LOSE_UDP;

extern STR_LOSE_UDP far lose_upd_scan;
extern STR_LOSE_UDP far lose_udp_bip;

#endif

#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)


void responseCmd(U8_T type,U8_T* pData);
void UART_Init(U8_T port);
void DELAY_Us(U32_T delay);
void DELAY_Ms(U32_T delay);
void TCPIP_Task( void *pvParameters );
void SoftReset(void);
void QuickSoftReset(void);
void SPI_ByteWrite(u8 TxData);
u8 SPI_ByteRead(void);
void vStartWifiTasks( U8_T uxPriority);
void LED_IO_Init(void); // for tiny and router
#endif


extern U32_T ether_rx_packet;	 
extern U32_T ether_tx_packet;
extern U8_T flag_reset_default;
extern U8_T flag_resume_rs485;
extern U8_T resume_rs485_count;
extern U8_T flag_reboot;
extern U8_T get_verion;
extern U8_T flag_reintial_tcpip;
extern U16_T count_reintial_tcpip;


extern uint16 far udp_scan_count;
extern U8_T far flag_udp_scan;
extern uint32_t run_time;
extern uint32_t run_time_last;
extern uint8_t reboot_counter;

void vLCDTask( void ) reentrant;
void Check_whether_reiniTCP(void);
#endif

#define SENDBUFF_SIZE   600    



