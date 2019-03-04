#ifndef REMOTE_CONNECT_H
#define REMOTE_CONNECT_H

#include "types.h"

#define SERVER_MINI_PORT 30000//33333
#define SERVER_T3000_PORT 33334
#define SERVER_HEARTBEAT_PORT 33335

#define HEARTBEAT_LENGTH 200
#define COMMUNICATION_VERSION  1
#define DISCONNEC_TIME   300

#define MINI_UDP_LENGTH  4
#define MINI_UDP_DATA_LENGTH   (MINI_UDP_LENGTH + 3)

#define  T3000_CONNECT_LENGTH  100
#define  T3000_CONNECT_DATA_LENGTH  (T3000_CONNECT_LENGTH+3)

#define  T3000_MINI_HEARTBEAT_LENGTH_WITH_MINI_PORT  (9 + HEARTBEAT_LENGTH)
#define  DEVICE_SEND_HEART_BEAT_TO_SERVER_PACKAGE_LENGTH (3 + HEARTBEAT_LENGTH)

// Client send message to server when log in
typedef struct stLoginMessage
{
 char userName[30];
 char password[20];
};

// Client send message to server when log out
typedef struct stLogoutMessage
{
 char userName[10];
};

// Client request another cient(userName)???????UDP????
struct stP2PTranslate
{
 char userName[10];
};

// Client send message to server
//typedef struct stMessage
//{
// int iMessageType;
// union //_message
// {
//  stLoginMessage loginmember;
//  stLogoutMessage logoutmember;
//  stP2PTranslate translatemessage;
// }message1;
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
  U32_T last_connected_time; //?????????? ?0??? ;
  
  U8_T reserved_reg[130];

	char userName[30];
  char password[20];
  //stLoginMessage login_message;
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


typedef union
{
 U8_T all_data[T3000_CONNECT_LENGTH];
 struct  
 {
  U32_T m_serial_number; //T3000????? ???;
//  stLoginMessage login_message;
  U8_T reserved_reg[46];
 }reg_data;
}Str_T3000_Connect;


typedef struct
{
  U32_T ip_address;
  U16_T port;
}Str_Mini_IP_Info;


typedef struct
{
 U32_T device_serial_number;
 Str_Mini_IP_Info device_add_info;
}Mini_IP_Manager;

extern U16_T RM_T3000_PORT;
extern U8_T RM_IP[4];
extern U8_T flag_start_RM;
//const int LIST_ITEM     = 0;
//const int LIST_SERIAL_NUMBER  = 1;
//const int LIST_LOGIN_NAME   = 2;
//const int LIST_LAST_UPDATE_TIME  = 3;
//const int LIST_IP_ADDRESS_PORT  = 4;


enum PTP_COMMAND_TYPE{
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
void RM_Init(void);
void RM_Heart_appcall(void);
void RM_Rec_appcall(void);
//void RM_Rec_t3000_makehole_appcall(void);

void RM_Start(void);
U8_T RM_Send_SN(void);
U8_T RM_Send_Heart(void);
U8_T RM_Send_T3000_MAKEHOLE(void);

#endif
