#ifndef	DEFINE_H
#define	DEFINE_H

#include "types.h"
#include "e2prom.h"
#include "schedule.h"

#define PIC_REV 0x01  // pic688
#define ISP_REV 11


#define CM5 0
#define TSTAT	1


#define CRC_YES 1
#define CRC_NO 0


#define READ_VARIABLES      3
#define WRITE_VARIABLES     6
#define MULTIPLE_WRITE		16
#define CHECKONLINE			25

#define DATABUFLEN			137

#define ORIGINALADDRESSVALUE	100
#define CALIBRATION_OFFSET    128 //allows us to store FLASH_CALIBRATION as an unsigned char

// 199 is the highest address we can use when storing variables in the flash memory
#define 	EEP_SERINALNUMBER_WRITE_FLAG	199 
#define 	TOTAL_EE_PARAMETERS		208
 
#define DHCP   1
#define STATIC 0

#define MODBUS 0 
#define TCP_IP 1 

#define BAC_MSTP 2
#define BAC_IP 3


#define WR_DESCRIPTION_SIZE			31
#define AR_DESCRIPTION_SIZE			29
#define ID_SIZE						3
#define AR_TIME_SIZE				46
#define WR_TIME_SIZE			    72
#define SCH_TIME_SIZE				1


#define PRODUCT_CM5		50

#define UART0 0
#define UART1 1
#define UART2 2

//typedef struct
//{
extern U16_T far wait;
extern U8_T xdata protocal;
extern  unsigned char far demo_enable;
extern	unsigned char far serialNum[4];
//	unsigned char firmwareVer[2];
extern	unsigned char far Modbus_address;
extern	unsigned char far product_model;
extern	unsigned char far hardRev;
extern	unsigned char far baudrate;
//	unsigned char PICversion;
//	unsigned char update;
extern	unsigned char far unit;
extern	unsigned char far sub_addr[8];
extern  U8_T far switch_tstat_val;
extern	U8_T far IspVer;
//	U8_T UPDATE_STATUS;
extern	U8_T far BASE_ADRESS;
extern	U8_T far TCP_TYPE;   /* 0 -- DHCP, 1-- STATIC */
extern	U8_T xdata IP_Addr[4];
extern	U8_T xdata Mac_Addr[6];
extern	U8_T xdata SUBNET[4];
extern	U8_T xdata GETWAY[4];
extern	U16_T far TCP_PORT;


extern	unsigned int  far AI_Value[10];	
extern	unsigned char far Input_Range[10];
extern	unsigned char far Input_Filter[10];	
extern	unsigned int far Input_CAL[10];

extern	S8_T xdata menu_name[36][14];
extern	unsigned char far dis_temp_num;
extern	unsigned char far dis_temp_interval;
extern	unsigned char far dis_temp_seq[10];

extern	unsigned char far DI1_Value;
extern	unsigned char far DI2_Value;
extern	unsigned char far DI_Type[8]; 		// Input is tstat or switch
extern	unsigned int  far DO_Value;  		// control relay
extern	unsigned char far DO_SoftSwitch;  	// software switch logic
extern	unsigned char far Priority;			// Zone1 has priority,
extern	unsigned int far count_priority;   // count priority timer 
//	unsigned char Master;
extern	unsigned int far DI_Enable;  // 
extern	unsigned int far AI_Enable;  //
extern	unsigned int far DInputAM;  // digital input 
extern	unsigned int far OuputAM;
extern	unsigned int far AInputAM;  // input 1 - 10 sensor
extern    U8_T far sn_write_flag;
extern	U8_T far update_status;
extern	U8_T far sub_no;
extern 	U8_T far switch_sub_no;
extern  U8_T far switch_sub_bit;
extern	U8_T far heat_no;
extern	U8_T far cool_no;

typedef	union
	{
		unsigned char all[10];
		struct 
		{
		/*	unsigned int year;	
			unsigned char mon;
			unsigned char week;
			unsigned char day;
			unsigned char hour;
			unsigned char min;
			unsigned char sec;	*/
			U8_T sec;				/* 0-59	*/
			U8_T min;    		/* 0-59	*/
			U8_T hour;      		/* 0-23	*/
			U8_T day;       		/* 1-31	*/
			U8_T mon;     		/* 0-11	*/
			U8_T year;      		/* 0-99	*/
			U8_T week;  		/* 0-6, 0=Sunday	*/
			U16_T day_of_year; 	/* 0-365	*/
			S8_T is_dst;        /* daylight saving time on / off */		
				
		}Clk;
	}UN_Time;
	 

extern UN_Time RTC;

//}STR_MODBUS;

//extern STR_MODBUS far Modbus;

#define BASE_ADDR   0x30 

#define HW_REV	7
#define SW_REV	15
#define PIC_REV 0x01  // pic688


#define CM5_ADDRESS 254
#define PRODUCT_MODEL	50

// define flash address

enum{
	//EEP_IP = 0x19
	EEP_MAC = 0x06,	  // 6 bytes 0x06 - 0x0b
	EEP_IP = 0x19,  // 4 bytes	  0x19 - 0x1c
	EEP_SUBNET = 0x1d, // 4 bytes  0x1d - 0x20
//--- other thing
	EEP_SERIALNUMBER_LOWORD = 0 + USER_BASE_ADDR,             
	EEP_SERIALNUMBER_HIWORD = 2 + USER_BASE_ADDR,
	EEP_FIRMWARE_VERSION_NUMBER_LO = 4 + USER_BASE_ADDR,
	EEP_FIRMWARE_VERSION_NUMBER_HI,
	EEP_ADDRESS						= 6 + USER_BASE_ADDR,
	EEP_PRODUCT_MODEL,
	EEP_HARDWARE_REV,
 	EEP_BAUDRATE,				//	63
	EEP_PIC,
    EEP_ADDRESS_PLUG_N_PLAY ,
	EEP_UNIT,
	EEP_PROTOCAL,

	EEP_DAYLIGHT_ENABLE,   
	EEP_DAYLIGHT_STATUS,
	// registers needed for updating status
	EEP_UPDATE_STATUS = 16 + USER_BASE_ADDR,	


	EEP_SUBADDR1 = 17 + USER_BASE_ADDR,
	EEP_SUBADDR2,
	EEP_SUBADDR3,
	EEP_SUBADDR4,
	EEP_SUBADDR5,
	EEP_SUBADDR6,
	EEP_SUBADDR7,
	EEP_SUBADDR8,

 


	EEP_INFO_BYTE,

  	EEP_CALIBRATION , //  = 10 + USER_BASE_ADDR,
	EEP_UPDTE_STATUS ,	
	EEP_BASE_ADDRESS,
	EEP_TCP_TYPE,


	EEP_INPUT1_RANGE,	  // 25							 
	EEP_INPUT2_RANGE,
	EEP_INPUT3_RANGE,
	EEP_INPUT4_RANGE,	
	EEP_INPUT5_RANGE,
	EEP_INPUT6_RANGE,
	EEP_INPUT7_RANGE,
	EEP_INPUT8_RANGE,
	EEP_INPUT9_RANGE,
	EEP_INPUT10_RANGE,

	EEP_INPUT1_FILTER, // 35
	EEP_INPUT2_FILTER,
	EEP_INPUT3_FILTER,
	EEP_INPUT4_FILTER,
	EEP_INPUT5_FILTER,
	EEP_INPUT6_FILTER,
	EEP_INPUT7_FILTER,
	EEP_INPUT8_FILTER,
	EEP_INPUT9_FILTER,
	EEP_INPUT10_FILTER,

	
	EEP_INPUT1_CAL,
	EEP_INPUT2_CAL = EEP_INPUT1_CAL + 2,
	EEP_INPUT3_CAL = EEP_INPUT1_CAL + 4,
	EEP_INPUT4_CAL = EEP_INPUT1_CAL + 6,
	EEP_INPUT5_CAL = EEP_INPUT1_CAL + 8,
	EEP_INPUT6_CAL = EEP_INPUT1_CAL + 10,
	EEP_INPUT7_CAL = EEP_INPUT1_CAL + 12,
	EEP_INPUT8_CAL = EEP_INPUT1_CAL + 14,
	EEP_INPUT9_CAL = EEP_INPUT1_CAL + 16,
	EEP_INPUT10_CAL = EEP_INPUT1_CAL + 18,

	EEP_DI_TYPE1 = EEP_INPUT10_CAL + 2,  // 65
	EEP_DI_TYPE2,
	EEP_DI_TYPE3,
	EEP_DI_TYPE4,
	EEP_DI_TYPE5,
	EEP_DI_TYPE6,
	EEP_DI_TYPE7,
	EEP_DI_TYPE8,

	
	EEP_OUTPUT_LOW,   /* OUTPUT GROUP1 RELAY 1- 8 */
	EEP_OUTPUT_HIGH,   /* K9 and K10 */

	EEP_SWITCH = 146 + USER_BASE_ADDR, 
	EEP_PRIORTITY,

	EEP_DI_ENABLE_LOW,   // Only ON/OFF  170
  	EEP_DI_ENABLE_HIGH,	// Only ON/OFF  171
	EEP_AI_ENABLE_LOW,   
  	EEP_AI_ENABLE_HIGH,
	EEP_DINPUT_AM_LOW,   // 
	EEP_DINPUT_AM_HIGH,   //  
	EEP_OUTPUT_AM_LOW,
	EEP_OUTPUT_AM_HIGH, 
	EEP_AINPUT_AM_LOW,   // 
	EEP_AINPUT_AM_HIGH,   //  


//	EEP_FIRST_TIME,    // whether have tstat address
	EEP_SERIALNUMBER_WRITE_FLAG,

	EEP_DIS_TEMP_NUM,
	EEP_DIS_TEMP_INTERVAL,
	EEP_DIS_TEMP_SEQ_FIRST,
	EEP_DIS_TEMP_SEQ_LAST = EEP_DIS_TEMP_SEQ_FIRST + 9,

	EEP_PORT_LOW,
	EEP_PORT_HIGH,

	EEP_INSTANCE_LOW,
	EEP_INSTANCE_HIGH,
	EEP_STATION_NUM,

	MAX_EEP_CONSTRANGE = 208,
};




#define SCHEDUAL_MODBUS_ADDRESS		200

enum {

	MODBUS_SERIALNUMBER_LOWORD = 0,             
	MODBUS_SERIALNUMBER_HIWORD	= 2,
	MODBUS_FIRMWARE_VERSION_NUMBER_LO   	= 4  ,
	MODBUS_FIRMWARE_VERSION_NUMBER_HI,
	MODBUS_ADDRESS					= 6,
	MODBUS_PRODUCT_MODEL,
	MODBUS_HARDWARE_REV,

 	
//	MODBUS_INIT_WR_TIME, 
	MODBUS_PIC = 9,
    MODBUS_ADDRESS_PLUG_N_PLAY= 10,
	MODBUS_TIME_ZONE = 11,	

	MODBUS_BAUDRATE,	 
//	MODBUS_DAYLIGHT_ENABLE,
//	MODBUS_DAYLIGHT_STATUS,
//	MODBUS_RESET_FLASH,	
	MODBUS_ISP_VER = 14,

// registers needed for updating status
	MODBUS_UPDATE_STATUS = 16,	
	MODBUS_UNIT =17,



//------------------ FOR SUB TSTAT ------------------------------
	// 193 - 203

	MODBUS_SUBADDR_FIRST = 18 ,	// 193
	MODBUS_SUBADDR_LAST = 25 , // 200

	MODBUS_TOTAL_NO ,  // NUMBER OF ZONES
	MODBUS_TOTAL_HEAT, //		NUMBER OF ZONES CALLING FOR HEAT
	MODBUS_TOTAL_COOL, //		NUMBER OF ZONES CALLING FOR COOLING	

//	MODBUS_TEST = 180,
	MODBUS_REFRESH_STATUS, // 196  


//  add ethernet register
	MODBUS_DEMO_ENABLE = 31,
	MODBUS_PROTOCAL = 32,
	MODBUS_RESET_PARAMETER = 33,
	MODBUS_BACNET_PORT,
	MODBUS_INSTANCE,
	MODBUS_STATION_NUM,


	MODBUS_ENABLE_WRITE_MAC = 93,

	/* 100 ~ 133 */
   	MODBUS_MAC_1 = 100,
	MODBUS_MAC_2,
	MODBUS_MAC_3,
	MODBUS_MAC_4,
	MODBUS_MAC_5,
	MODBUS_MAC_6,

	MODBUS_TCP_TYPE = 106,	  // DHCP OR STATIC

	MODBUS_IP_1,	  // IP have 4 bytes
	MODBUS_IP_2,
	MODBUS_IP_3,
	MODBUS_IP_4,

	MODBUS_SUBNET_1,	  // subnet have 4 bytes
	MODBUS_SUBNET_2,
	MODBUS_SUBNET_3,
	MODBUS_SUBNET_4,

	MODBUS_GETWAY_1,	  // getway have 4 bytes
	MODBUS_GETWAY_2,
	MODBUS_GETWAY_3,
	MODBUS_GETWAY_4,

	MODBUS_TCPSERVER,		// no used
	MODBUS_TCP_LISTEN_PORT,	
	/*
	120 TO 133 reserved
		
	*/	
	MODBUS_AINPUT1 = 134,        // Input 1, register 100
	MODBUS_AINPUT2  ,                   			// Input 2 
	MODBUS_AINPUT3  ,                   			// Input 3 
	MODBUS_AINPUT4  ,                   			// Input 4 
	MODBUS_AINPUT5  ,                   			// Input 5 
	MODBUS_AINPUT6  ,                   			// Input 6 
	MODBUS_AINPUT7  ,                   			// Input 7 
	MODBUS_AINPUT8  ,                   			// Input 8
	MODBUS_AINPUT9  ,                   			// Input 9
	MODBUS_AINPUT10 ,     

	MODBUS_INPUT1_RANGE = 144,								 
	MODBUS_INPUT2_RANGE,
	MODBUS_INPUT3_RANGE,
	MODBUS_INPUT4_RANGE,	
	MODBUS_INPUT5_RANGE,
	MODBUS_INPUT6_RANGE,
	MODBUS_INPUT7_RANGE,
	MODBUS_INPUT8_RANGE,
	MODBUS_INPUT9_RANGE,
	MODBUS_INPUT10_RANGE,

	MODBUS_INPUT1_FILTER = 154,
	MODBUS_INPUT2_FILTER,
	MODBUS_INPUT3_FILTER,
	MODBUS_INPUT4_FILTER,
	MODBUS_INPUT5_FILTER,
	MODBUS_INPUT6_FILTER,
	MODBUS_INPUT7_FILTER,
	MODBUS_INPUT8_FILTER,
	MODBUS_INPUT9_FILTER,
	MODBUS_INPUT10_FILTER,

	MODBUS_INPUT1_CAL = 164,								 
	MODBUS_INPUT2_CAL,
	MODBUS_INPUT3_CAL,
	MODBUS_INPUT4_CAL,	
	MODBUS_INPUT5_CAL,
	MODBUS_INPUT6_CAL,
	MODBUS_INPUT7_CAL,
	MODBUS_INPUT8_CAL,
	MODBUS_INPUT9_CAL,
	MODBUS_INPUT10_CAL,

	MODBUS_DI_TYPE1 = 174,
	MODBUS_DI_TYPE2,
	MODBUS_DI_TYPE3,
	MODBUS_DI_TYPE4,
	MODBUS_DI_TYPE5,
	MODBUS_DI_TYPE6,
	MODBUS_DI_TYPE7,
	MODBUS_DI_TYPE8,

	MODBUS_DI1,  // DI1 - DI8  182
	MODBUS_DI2,  // DI9 - DI16  183     
	MODBUS_DOUTPUT,   /* OUTPUT 1 - 10     184*/  
	MODBUS_SWITCH,  // 185   priority mode
	MODBUS_PRIORTITY,  /* priority interval    1min to 100min */
	MODBUS_COUNT_PRI, //  count priority 
//	MODBUS_MASTER,  // 1 - DI9-DI16 are used Tstat ports ,CM5 is MASTER, control tstat
					// 0 - DI9-DI16 are used input ports

	MODBUS_DI_ENABLE,   // Only ON/OFF  188
	MODBUS_AI_ENABLE,	// if enable ,only on/off
	MODBUS_DINPUT_AM,   // 
	MODBUS_OUTPUT_AM,
	MODBUS_AINPUT_AM,   




/*
	MODBUS_TIMER_ADDRESS		= SCHEDUAL_MODBUS_ADDRESS , // 200
	
	MODBUS_WR_DESCRIP_FIRST		= MODBUS_TIMER_ADDRESS + 8 ,
	MODBUS_WR_DESCRIP_LAST		= MODBUS_WR_DESCRIP_FIRST + WR_DESCRIPTION_SIZE * MAX_WR ,
	
	MODBUS_AR_DESCRIP_FIRST		= MODBUS_WR_DESCRIP_LAST ,
	MODBUS_AR_DESCRIP_LAST		= MODBUS_AR_DESCRIP_FIRST + AR_DESCRIPTION_SIZE * MAX_AR ,
	
	MODBUS_ID_FIRST				= MODBUS_AR_DESCRIP_LAST ,
	MODBUS_ID_LAST				= MODBUS_ID_FIRST + ID_SIZE * MAX_ID ,
	
	MODBUS_AR_TIME_FIRST		= MODBUS_ID_LAST ,
	MODBUS_AR_TIME_LAST			= MODBUS_AR_TIME_FIRST + AR_TIME_SIZE * MAX_AR ,
	
	MODBUS_WR_ONTIME_FIRST		= MODBUS_AR_TIME_LAST ,
	MODBUS_WR_ONTIME_LAST		= MODBUS_WR_ONTIME_FIRST + WR_TIME_SIZE * MAX_WR ,
	
	MODBUS_WR_OFFTIME_FIRST		= MODBUS_WR_ONTIME_LAST ,
	MODBUS_WR_OFFTIME_LAST		= MODBUS_WR_OFFTIME_FIRST + WR_TIME_SIZE * MAX_WR ,

//	MODBUS_TOTAL_PARAMETERS		= MODBUS_WR_OFFTIME_LAST
*/
	

/*		FOR ALL ZONES
    MODBUS ADDRESS 
    COOLING SETPOINT
    HEATING SETPOINT
    ROOM SETPOINT
    ROOM TEMP 
    MODE
    OUTPUT STATE, binary and analog outputs
    OCCUPIED  state
	
	NUMBER OF ZONES:           <maintain a register to show how many thermostats are connected
	NUMBER OF ZONES CALLING FOR HEAT:   , one register
	NUMBER OF ZONE CALLING FOR COOL:    , one register
	PRIORITY REGISTER            basic function already done by Chelsea
	OUTDOOR TEMP:  

*/

	MODBUS_OCCUPIED = 5670,  // 5670   // TSTAT REGISTER 184.0

	MODBUS_COOL_SETPOINT_FIRST = 5671,  // TSTAT REGISTER 380
	MODBUS_COOL_SETPOINT_LAST = 5678,

	MODBUS_HEAT_SETPOINT_FIRST = 5679,//28, // TSTAT REGISTER 136
	MODBUS_HEAT_SETPOINT_LAST = 5686,//35,

	MODBUS_ROOM_SETPOINT_FIRST = 5687,//36,	// TSTAT REGISTER 135  // 374 is the two byte setpt
	MODBUS_ROOM_SETPOINT_LAST = 5694,//43,

	MODBUS_ROOM_TEM_FIRST = 5695,//44,	  //	TSTAT REGISTER 101
	MODBUS_ROOM_TEM_LAST = 5702,//51,

	MODBUS_MODE_FIRST = 5703,//52,    // TSTAT REGISTER 107
	MODBUS_MODE_LAST = 5710,//59,

	MODBUS_OUTPUT_STATE_FIRST = 5711,//60,   //  TSTAT REGISTER 108
	MODBUS_OUTPUT_STATE_LAST = 5718,//67,

	MODBUS_NIGHT_HEAT_DB_FIRST = 5719,//68,  //  TSTAT REGISTER  123 
	MODBUS_NIGHT_HEAT_DB_LAST = 5726,//75,

	MODBUS_NIGHT_COOL_DB_FIRST = 5727,//76,  //  TSTAT REGISTER  124
	MODBUS_NIGHT_COOL_DB_LAST = 5734,//83,

	MODBUS_NIGHT_HEAT_SP_FIRST = 5735,//84,  //  TSTAT REGISTER  182 
	MODBUS_NIGHT_HEAT_SP_LAST = 5742,//91,

	MODBUS_NIGHT_COOL_SP_FIRST = 5743,//92,  //  TSTAT REGISTER  183
	MODBUS_NIGHT_COOL_SP_LAST = 5750,//99,

	MODBUS_PRODUCT_MODEL_FIRST = 5751,//92,  //  TSTAT REGISTER  7
	MODBUS_PRODUCT_MODEL_LAST = 5758,//99,
	
	MODBUS_OVER_RIDE_FIRST = 5759,			// TSTAT REGISTER 211
	MODBUS_OVER_RIDE_LAST = 5766,			

	MODBUS_SERIAL_NUM_FIRST = 5767,			// TSTAT REGISTER 0-4
	MODBUS_SERIAL_NUM_LAST = 5798,			// serial number is 4 bytes length, so 4 * 8
	
	MODBUS_TSTAT_OFFTIME_FIRST = 5800,
	MODBUS_TSTAT_OFFTIME_LAST = MODBUS_TSTAT_OFFTIME_FIRST + 253,

	MODBUS_TSTAT_ONTIME_FIRST = 6100,
	MODBUS_TSTAT_ONTIME_LAST = MODBUS_TSTAT_ONTIME_FIRST + 253,

	MODBUS_TEST = 7000,
	MODBUS_TEST_50 = 7049,

	MODBUS_NAME_FIRST,  // 7050
	MODBUS_NAME_LAST = MODBUS_NAME_FIRST + 14/*NAME_SIZE*/ / 2 * 36/*MAX_NAME*/ - 1,

	MODBUS_DISPLAY_TEMP_NUM,  // 7302
	MODBUS_DISPLAY_TMEP_INTERVAL,
	MODBUS_DISPLAY_TEMP_SEQ_FIRST,
	MODBUS_DISPLAY_TEMP_SEQ_LAST = MODBUS_DISPLAY_TEMP_SEQ_FIRST + 9,

}; 




#endif
