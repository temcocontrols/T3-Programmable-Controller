
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

#define	SUB_NET_SPARE					0
#define	SEND_OUTPUT_WORD 				1
#define	READ_OUTPUT_FEEDBACK			2
#define	CHECK_ONLINE					3
#define	READ_SN_FOR_ASSIGN				4
#define	WRITE_ID_WITH_SN_FOR_ASSIGN		5
#define	FORWARD_COMMAND_TO_SUB			6
#define SET_RELAY_PULSE_DURATION		7
#define	SET_RELAY_DELAY_BETWEEN_PULSES	8


#define	SOURCE_TCPIP	0
#define SOURCE_RS485	1

enum
{
	F_READ = 1,F_WRITE,F_MUTIPLE
};

enum {
SPARE = 0,T3000_Control_Sub_Direct,CM5_Scan_Sub_Table,CM5_SCAN,CM5_Dealwith_Same_ID/*,CM5_Assign_ID*/,CM5_Check_Sub_OnLine,
Button_Control_Tst
};

extern U8_T forword_source;
extern bit flag_transimit_from_serial;

extern U8_T EthernetCRChi;
extern U8_T EthernetCRClo;
extern U8_T far sub_data_buffer[SUB_BUF_LEN];
extern U8_T sub_dealwithTag;

//extern U8_T far sub_net_buf[SUB_BUF_LEN];
extern U8_T far sub_net_state;
extern U8_T far sub_response_in; 
extern U16_T sub_rece_size; 
extern U8_T SubCRChi;
extern U8_T SubCRClo;

extern U8_T comm_tstat;
extern bit flag_control_by_button;
extern U16_T main_rece_count, sub_rece_count;
extern U8_T far main_serial_receive_timeout_count;
extern U8_T far sub_serial_receive_timeout_count;



void set_sub_net_state(U8_T state);
void reset_sub_net_state(void);
void ethernet_init_crc16(void);
void ethernet_crc16_byte(U8_T ch);
U16_T crc16(U8_T *p, U8_T length);

//void write_sub_card(U8_T mb_addr, U16_T mb_reg, U16_T value);
void send_to_slave(U8_T *buf, U8_T length);
U8_T wait_sub_response(U8_T delay);

void vStartMainSerialTasks(U8_T uxPriority);
void vStartSubSerialTasks(U8_T uxPriority);	 

U8_T checkData(U8_T address);
void sub_crc16_byte(U8_T ch);
void sub_init_crc16(void);
void sub_serial_restart(void);
void sub_send_byte(U8_T buffer, U8_T crc);
void sub_init_send_com(void);
void sub_send_string(unsigned char *bufs,char len);
U8_T wait_SubSerial(U16_T delay);

void initSerial(void);
void main_responseData(U16_T address);

void main_init_send_com(void);
void main_send_byte(U8_T buffer, U8_T crc);



#endif
