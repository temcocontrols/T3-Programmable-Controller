#include "types.h"

#define USART_REC_LEN  			512  	//定义最大接收字节数 200
#define USART_SEND_LEN			512


typedef enum 
{ 
	CMD_NULL,
	CMD_SWITCH_MODE,
	CMD_READ_STA_INFO,
	CMD_READ_AP_INFO,
	CMD_READ_LINK_INFO,
	CMD_READ_MAC,
	CMD_READ_WSLK,
	CMD_CFG_SOCKETA,
	CMD_CFG_SOCKETB,
	CMD_FAC_RESET,
	CMD_BACK_TO_NOMRAL,
	NORMAL_MODE,
	WIFI_TOTOAL_CMD
} WIFI_CMD;

extern uint8 ip_read_flag;
extern uint8 wifi_cmd_num;
extern uint8 const wifi_cmd[WIFI_TOTOAL_CMD][10];

extern uint8 USART_RX_BUFC[512];   
//extern uint8 USART_RX_BUFD[50];
extern uint16 rece_countB;
extern uint8 dealwithTagB;
extern uint8 uart_sendB[USART_SEND_LEN];
extern uint16 sendbyte_numB;
//u8 SERIAL_RECEIVE_TIMEOUT ;
extern uint16 rece_sizeB;
extern uint8 update_flag;
extern uint8 serial_receive_timeout_countB;

