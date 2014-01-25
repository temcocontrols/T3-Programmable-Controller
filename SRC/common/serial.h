
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

#define MAIN_BUF_LEN		140
#define	SUB_BUF_LEN			300//140
#define DATABUFLEN_SCAN		12
#define SENDPOOLLEN         8

#define RESPONSERANDVALUE	1

//#define MainSerialSTACK_SIZE	((unsigned portSHORT)256)
//#define SubSerialSTACK_SIZE		((unsigned portSHORT)256)
#define MainSerialSTACK_SIZE	((unsigned portSHORT)1024)
#define SubSerialSTACK_SIZE		((unsigned portSHORT)1024)

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


enum
{
	F_READ = 1,F_WRITE,F_MUTIPLE
};

extern U16_T main_rece_count;
extern U16_T sub1_rece_count;
extern U16_T zig_rece_count;
extern U8_T main_rece_size;
extern U16_T sub1_rece_size;
extern U16_T zig_rece_size;
extern U8_T main_dealwithTag;
extern U8_T sub1_dealwithTag;
extern U8_T zig_dealwithTag;
extern U8_T far main_data_buffer[MAIN_BUF_LEN];
extern U8_T far sub1_data_buffer[SUB_BUF_LEN];
extern U8_T far zig_data_buffer[SUB_BUF_LEN];



//void responseCmd(U8_T type,U8_T* pData,HTTP_SERVER_CONN * pHttpConn);  
void main_responseData(U16_T address);
void main_serial_restart(void);
U8_T checkData(U8_T address);
void uart_init_send_com(U8_T port);
void uart_send_byte(U8_T buffer,U8_T port);
void main_init_send_com(void);
void uart_send_string(U8_T *p, U16_T length,U8_T port);
void set_subnet_parameters(U8_T io, U8_T length,U8_T port);
U8_T wait_subnet_response(U16_T nDoubleTick,U8_T port);


void vStartMainSerialTasks( U8_T uxPriority);

#endif
