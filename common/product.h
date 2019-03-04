#ifndef PRODUCT_H
#define PRODUCT_H

#include "types.h"

#define SW_REV	4808

#define ARM_MINI 0
#define ARM_CM5  0
#define ARM_WIFI 0

#define ASIX_MINI 1
#define ASIX_CM5  0

//#define ARM 1
//#define ASIX 0

//#if (ASIX_MINI || ASIX_CM5)

//#define MINI 1
//#define	CM5	0

//#else

//#define MINI 1
//#define TSTAT8 0

//#endif


#define BIG_MAX_AIS 32
#define BIG_MAX_DOS 12
#define BIG_MAX_AOS 12
#define BIG_MAX_AVS 128
#define BIG_MAX_DIS 0
#define BIG_MAX_SCS 8

#define SMALL_MAX_AIS 16
#define SMALL_MAX_DOS 6
#define SMALL_MAX_AOS 4
#define SMALL_MAX_AVS 128
#define SMALL_MAX_DIS 0
#define SMALL_MAX_SCS 8

#define TINY_MAX_AIS 11
#define TINY_MAX_DOS 6
#define TINY_MAX_AOS 2
#define TINY_MAX_AVS 128
#define TINY_MAX_DIS 0
#define TINY_MAX_SCS 8

#define NEW_TINY_MAX_AIS 8
#define NEW_TINY_MAX_DOS 8
#define NEW_TINY_MAX_AOS 6
#define NEW_TINY_MAX_AVS 128
#define NEW_TINY_MAX_DIS 0
#define NEW_TINY_MAX_SCS 8

#define VAV_MAX_AIS 6
#define VAV_MAX_DOS 1
#define VAV_MAX_AOS 2
#define VAV_MAX_AVS 5
#define VAV_MAX_DIS 0
#define VAV_MAX_SCS 8

#define CM5_MAX_AIS 24
#define CM5_MAX_DOS 10
#define CM5_MAX_AOS 0
#define CM5_MAX_AVS 128
#define CM5_MAX_DIS 8
#define CM5_MAX_SCS 8


#define MINI_CM5  0
#define MINI_BIG	 1
#define MINI_SMALL  2
#define MINI_TINY	 3			// ASIX CORE
#define MINI_NEW_TINY	 4  // ARM CORE


#define MINI_BIG_ARM	 	5
#define MINI_SMALL_ARM  6
#define MINI_TINY_ARM		7

#define MINI_VAV	 10


#define STM_TINY_REV 7






#if (ASIX_MINI || ASIX_CM5)

//#define TEST_RST_PIN 1

#define DEBUG_UART1 0
//#define CHAMBER
#if (DEBUG_UART1)

#include <stdio.h>

#define UART_SUB1 0

void uart_init_send_com(unsigned char port);
extern unsigned char far debug_str[200];
void uart_send_string(unsigned char *p, unsigned int length,unsigned char port);

#endif

#define MSTP 0  // < 1k
#define ALARM_SYNC  0 // ~ 2k
#define TIME_SYNC 0 // >1k
#define REM_CONNECTION 0

#endif



#if (ARM_MINI || ARM_CM5 || ARM_WIFI)



#define SD_BUS_TYPE 0
#define SPI_BUS_TYPE 1


#define SD_BUS SPI_BUS_TYPE

#define far
#define code
#define bit U8_T
#define data

#define sTaskCreate xTaskCreate
#define cQueueSend xQueueSend
#define cQueueReceive xQueueReceive
#define vSemaphoreCreateBinary xQueueCreateMutex
//#define xSemaphoreHandle QueueHandle_t
#define cSemaphoreTake xQueueTakeMutexRecursive
#define cSemaphoreGive xQueueGiveMutexRecursive


#define MSTP 1  // < 1k
#define ALARM_SYNC  1 // ~ 2k
#define TIME_SYNC 1 // >1k

#endif



#if (ARM_MINI || ASIX_MINI )


#define INCLUDE_DNS_CLIENT  1  // 3k
#define INCLUDE_DHCP_CLIENT 1

#define USB_HOST   0   // 9k
#define USB_DEVICE 0   // > 2k
#define WEBPAGE 0  // 60k
#define T3_MAP  1
#define STORE_TO_SD  1  // > 20k
#define USER_SYNC 0//1

#define NETWORK_MODBUS 0

#define ARM_UART_DEBUG 0
#define DEBUG_EN  UART0_TXEN_BIG//UART0_TXEN_TINY

#define PING  0


#endif

#if ARM_CM5
#define INCLUDE_DNS_CLIENT  1  // 3k
#define INCLUDE_DHCP_CLIENT 1

#define USB_HOST   0   // 9k
#define USB_DEVICE 0   // > 2k
#define WEBPAGE 0  // 60k
#define T3_MAP  1
#define STORE_TO_SD  1  // > 20k
#define USER_SYNC 0//1

#define MSTP 1
#define PING  0

#define ARM_UART_DEBUG 0
#define DEBUG_EN  UART0_TXEN

#endif


#if ASIX_CM5


#define MSTP 0

#define TIME_SYNC 0

#define WEBPAGE 0//1
#define T3_MAP  0
#define USB_HOST 0 
#define USB_DEVICE 0
#define STORE_TO_SD  0
#define ALARM_SYNC  0



#define INCLUDE_DNS_CLIENT  1
#define INCLUDE_DHCP_CLIENT 1

#define PING  0


#endif


#if ARM_WIFI

#define NETWORK_MODBUS 0

#define ARM_UART_DEBUG 0


#define DEBUG_EN  UART0_TXEN_BIG


#endif








#endif

