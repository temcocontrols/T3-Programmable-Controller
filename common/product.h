#ifndef PRODUCT_H
#define PRODUCT_H

#include "types.h"

#define SW_REV	6005//5404

#define ARM_MINI 0
#define ARM_CM5  0
#define ARM_TSTAT_WIFI 1

#define ASIX_MINI 0
#define ASIX_CM5  0

#define ARM_TSTAT_WIFI_DEBUG  0

#define HANDLE_REBOOT_FLAG 0  // 如果要编译为1 就是编译给客户一个特殊的 开机关闭 prg 的版本 解决客户编写有问题的 一直重启;
                              //正常情况下这个值请保持为 0   ;

extern U8_T cpu_type;

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

#define TINY_11I_MAX_AIS 11
#define TINY_11I_MAX_DOS 6
#define TINY_11I_MAX_AOS 5
#define TINY_11I_MAX_AVS 128
#define TINY_11I_MAX_DIS 0
#define TINY_11I_MAX_SCS 8

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


#define TSTAT10_MAX_AIS 13
#define TSTAT10_MAX_DOS 5
#define TSTAT10_MAX_AOS 4
#define TSTAT10_MAX_AVS 128
#define TSTAT10_MAX_DIS 8
#define TSTAT10_MAX_SCS 8

#define T10P_MAX_AIS 17
#define T10P_MAX_DOS 5
#define T10P_MAX_AOS 4
#define T10P_MAX_AVS 128
#define T10P_MAX_DIS 8
#define T10P_MAX_SCS 8


#define MINI_CM5  0
#define MINI_BIG	 1
#define MINI_SMALL  2
#define MINI_TINY	 3			// ASIX CORE
#define MINI_NEW_TINY	 4  // ARM CORE
#define MINI_BIG_ARM	 	5
#define MINI_SMALL_ARM  6
#define MINI_TINY_ARM		7
#define MINI_NANO    8
#define MINI_TSTAT10 9
#define MINI_T10P	 11
#define MINI_VAV	 10   // no used
#define MINI_TINY_11I		12

#define MAX_MINI_TYPE 12


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

#define OUTPUT_DEATMASTER  1
#define INCLUDE_DNS_CLIENT  1  // 3k
#define INCLUDE_DHCP_CLIENT 1

#endif



#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)

#define OUTPUT_DEATMASTER  1

#define SD_BUS_TYPE 0
#define SPI_BUS_TYPE 1

#define REBOOT_PER_WEEK 1
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

#define T3_MAP  1
#define STORE_TO_SD  1  // > 20k
#define USER_SYNC 0//1


#define COV   0
#define SMTP  0
#define NETWORK_MODBUS 	1
//#define ETHERNET_DEBUG  1  //网络端口的调试信息 往 192.168.0.38    端口1115打印数据;
#define ARM_UART_DEBUG 0
#define DEBUG_EN  UART0_TXEN_BIG 
//#define DEBUG_EN  UART0_TXEN_TINY  //Tiny 引脚不一样用这个

#define PING  0


#endif

#if ARM_CM5



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

#define T3_MAP  0
#define STORE_TO_SD  0
#define ALARM_SYNC  0




#define PING  0


#endif


#if ARM_TSTAT_WIFI


#define SD_BUS_TYPE 0
#define SPI_BUS_TYPE 1


#define SD_BUS SPI_BUS_TYPE


#define STORE_TO_SD  1 

#define NETWORK_MODBUS 0

#define ARM_UART_DEBUG 0


#define DEBUG_EN  UART0_TXEN_BIG


#endif








#endif

