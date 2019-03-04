
#ifndef __REMOTE_H__
#define __REMOTE_H__


/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */
#define RM_SERVER_PORT		31234
#define RM_MAX_LENGTH			76

/*-------------------------------------------------------------*/
/* NAMING CONSTANT DECLARATIONS */
#define RM_STATE_NONE			0
#define RM_STATE_INITIAL		1
#define RM_STATE_CONNECTED	2  
#define RM_STATE_RECEIVE	3

#define RM_HEART_STATE_NOTREADY		0
#define RM_HEART_STATE_INITIAL		1  
#define RM_HEART_STATE_WAIT				2
#define RM_HEART_STATE_TIMEOUT		3
#define RM_HEART_STATE_GET_DONE		4

#define RM_REC_STATE_NOTREADY		0
#define RM_REC_STATE_INITIAL		1  
#define RM_REC_STATE_WAIT				2
#define RM_REC_STATE_TIMEOUT		3
#define RM_REC_STATE_GET_DONE		4

#define MAX_RM_RETRY_CONNECTING_COUNT 10
#define MAX_RM_RETRY_CONNECTED_COUNT  60


#define RM_HAERT_PORT 33335
#define RM_REC_PORT 	33333

/* TYPE DECLARATIONS */
typedef struct _RMC_CONN
{
	U8_T	State;
	U8_T	TcpSocket;
	U32_T	ServerIp;
	U8_T connecting_count;
	U8_T connected_count;
} RM_CONN;


typedef struct _RMC_HAERT_CONN
{
	U8_T	State;
	U8_T	UdpSocket;
	U32_T	ServerIp;
} RM_HEART_CONN;

typedef struct _RMC_REC_CONN
{
	U8_T	State;
	U8_T	UdpSocket;
	U32_T	ServerIp;
} RM_REC_CONN;

/* GLOBAL VARIABLES */

typedef enum
{
 RECEIVE_DATA_LEBGTH_ERROR = 1,
 RECEIVE_MINI_DATA = 2
};

typedef enum PTP_COMMAND_TYPE{
 COMMAND_RECEIVE_HEART_BEAT    = 0x01,
 COMMAND_RECEIVE_SERIAL     = 0x02,
 COMMAND_REPLY_DEVICE_INFO    = 0x03,
 COMMAND_REPLY_T3000_INFO    = 0x04,
 COMMAND_T3000_REQUEST     = 0x05,
 COMMAND_DEVICE_SEND_SERIAL_TO_SERVER = 0x06,
 COMMAND_DEVICE_SEND_HEART_BEAT_TO_SERVER= 0x07,
 COMMAND_T3000_SEND_TO_DEVICE_MAKEHOLE   = 0x08,
 COMMAND_DEVICE_SEND_TO_T3000_MAKEHOLE   = 0x09,
 COMMAND_COMMUNICATION_VERSION_ERROR     = 0x64,
 COMMAND_COMMAND_UNKNOWN     = 0x65,
 COMMAND_DEVICE_NOT_CONNECT_ERROR  = 0x66,
 COMMAND_DEVICE_NO_HEARTBEAT_ERROR  = 0x67,
 COMMAND_PASSWORD_ERROR     = 0x68
};


#define HEARTBEAT_LENGTH 200
#define COMMUNICATION_VERSION  1
#define DISCONNEC_TIME   300

#define MINI_UDP_LENGTH  4
#define MINI_UDP_DATA_LENGTH   (MINI_UDP_LENGTH + 3)

#define  T3000_CONNECT_LENGTH  100
#define  T3000_CONNECT_DATA_LENGTH  (T3000_CONNECT_LENGTH+3)

//#define  T3000_MINI_HEARTBEAT_LENGTH_WITH_MINI_PORT  (9 + HEARTBEAT_LENGTH)
//#define  DEVICE_SEND_HEART_BEAT_TO_SERVER_PACKAGE_LENGTH (3 + HEARTBEAT_LENGTH)

// Client????????????

typedef struct stLoginMessage
{
 S8_T userName[30];
 S8_T password[20];
};

// Client????????
//typedef struct stLogoutMessage
//{
// S8_T userName[10];
//};

typedef union
{
 U8_T  all_data[HEARTBEAT_LENGTH];
 struct 
 {  
  U8_T communication_version;
  U8_T ideviceType; // 0  ?T3000   1?????;
  
  U32_T m_serial_number;
  U8_T m_product_type;
  U32_T m_object_instance;
  U8_T m_panel_number;
  U16_T modbus_port;
  U16_T soft_version;
  U32_T last_connected_time; // = 0
  U8_T reserved_reg[130];
	S8_T userName[30];
	S8_T password[20];
//  stLoginMessage login_message;
 }reg_data;
}Str_MSG;

typedef union
{
 U8_T  all_data[MINI_UDP_LENGTH];
 struct 
 {  
  U32_T m_serial_number;
  //sockaddr_in addrClientMini;//??A?????
 }reg_data;
}Str_Device;


//typedef union
//{
// U8_T all_data[T3000_CONNECT_LENGTH];
// struct  
// {
//  U32_T m_serial_number; //T3000????? ???;
//  stLoginMessage login_message;
//  U8_T reserved_reg[46];
// }reg_data;
//}Str_T3000_Connect;

//#define RM_NO_CONNECTION 0
//#define RM_CONNECTING    1
//#define RM_CONNECT_OK    2

extern RM_CONN  RM_Conns;
extern U8_T RM_Send_Status;
extern U8_T  Recount_Check_serverip;


/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void RM_Init(void);
void RM_Event(U8_T, U8_T);
void RM_Receive(U8_T XDATA*, U16_T, U8_T);
void RM_Start(void);
//void RM_SendMessage(U8_T*, U16_T);
//U8_T RM_GetState(void);

void Cmime64_Init(void);
void cmime64(S8_T*);

void RM_Rec_Event(U8_T id, U8_T event);
void RM_Heart_Event(U8_T id, U8_T event);
void RM_Rec_Receive(U8_T XDATA* pbuf, U16_T length, U8_T id);
void RM_Heart_Receive(U8_T XDATA* pbuf, U16_T length, U8_T id);
U8_T RM_Send_Heart(void);
U8_T RM_Send_SN(void);

#endif /* __RMC_H__ */

