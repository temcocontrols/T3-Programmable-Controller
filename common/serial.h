
#ifndef	SERIAL_H
#define	SERIAL_H

#include "types.h"



#define SEND				1
#define	RECEIVE				0

#define	DO_CRC				TRUE
#define	NO_CRC				FALSE

#define SERIAL_COM_IDLE		0
#define INVALID_PACKET		1
#define VALID_PACKET		2

//#define MAIN_BUF_LEN		140
//#define	SUB_BUF_LEN			300//140
#define MAX_BUF_LEN         300
#define DATABUFLEN_SCAN		12
#define SENDPOOLLEN         8

#define RESPONSERANDVALUE	1


#define MainSerialSTACK_SIZE	((unsigned portSHORT)1024)

#define CUSTOMER_PRODUCT 220

//#define	SUB_NET_SPARE					0
//#define	SEND_OUTPUT_WORD 				1
//#define	READ_OUTPUT_FEEDBACK			2
//#define	CHECK_ONLINE					3
//#define	READ_SN_FOR_ASSIGN				4
//#define	WRITE_ID_WITH_SN_FOR_ASSIGN		5
//#define	FORWARD_COMMAND_TO_SUB			6
//#define SET_RELAY_PULSE_DURATION		7
//#define	SET_RELAY_DELAY_BETWEEN_PULSES	8
//

extern U8_T GSM_remote_IP[4];
extern U16_T GSM_remote_tcpport;
extern U16_T GSM_remote_tcpport1;
extern U8_T GSM_remote_tcp_link_id; 

//extern U8_T GSM_remote_udp_link_id;

enum
{
	F_READ = 1,F_WRITE,F_MUTIPLE
};

extern U16_T uart0_rece_count;
extern U16_T uart1_rece_count;
extern U16_T uart0_rece_size;
extern U16_T uart1_rece_size;
extern U16_T main_rece_size;
extern U8_T uart0_dealwithTag;
extern U8_T uart1_dealwithTag;
extern U8_T main_dealwithTag;
extern U8_T far uart0_data_buffer[MAX_BUF_LEN];
extern U8_T far uart1_data_buffer[MAX_BUF_LEN];
extern U8_T far main_data_buffer[MAX_BUF_LEN];
extern U8_T far subnet_response_buf[MAX_BUF_LEN];
extern U8_T uart0_transmit_finished;

//extern  U8_T tstat_position[SUB_NO];

//void responseCmd(U8_T type,U8_T* pData,HTTP_SERVER_CONN * pHttpConn);  
void main_responseData(void);
void main_serial_restart(void);
U8_T checkData(U8_T address);
void uart_init_send_com(U8_T port);
void uart_send_byte(U8_T buffer,U8_T port);
void uart_send_string(U8_T *p, U16_T length,U8_T port);
void set_subnet_parameters(U8_T io, U8_T length,U8_T port);
U8_T wait_subnet_response(U16_T nDoubleTick,U8_T port);	
void uart_serial_restart(U8_T port);
void vStartMainSerialTasks( U8_T uxPriority);
void Response_Speical_Logic(U8_T* str);
void logic_control_in_out(void);



#endif
