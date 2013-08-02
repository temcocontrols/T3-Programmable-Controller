#ifndef	COMMSUB_H
#define	COMMSUB_H

#include "types.h"

typedef struct
{
	U8_T id;
	U32_T sn;
}SCAN_DB;


typedef struct
{
//	U16_T max;
//	U16_T min;
	U8_T valid;
}BinSearch;

#define	NONE_ID						0
#define	UNIQUE_ID					1
#define	MULTIPLE_ID					2
#define	UNIQUE_ID_FROM_MULTIPLE		3
#define	ASSIGN_ID					4

//extern U8_T far scan_response_state;
extern U8_T source;

extern uint8 const code scan_id_table[511][2];

//extern xQueueHandle	xScanIndex;
//extern xQueueHandle	xScanResult;

extern SCAN_DB far scan_db[8];
extern SCAN_DB far temp_scan_db;
extern U8_T far current_scan_index;
extern BinSearch xdata binsearch_Table[511];

extern U8_T far tst_addr_index;
extern U8_T far tst_reg_index;
extern U8_T data by_tstat_index;

extern U16_T far tstat_temperature[8];
extern U16_T far tstat_mode[8];
extern U16_T far tstat_setpoint[8];
extern U16_T far tstat_cool_setpoint[8];
extern U16_T far tstat_heat_setpoint[8];
extern U8_T far tstat_occupied; // occupied is 1 bit, 8 BIT is CHAR
extern U8_T far tstat_output_state[8];
extern U8_T far tstat_night_heat_db[8];
extern U8_T far tstat_night_cool_db[8];
extern U8_T far tstat_night_heat_sp[8];
extern U8_T far tstat_night_cool_sp[8];
extern U8_T far tstat_product_model[8];
extern U8_T far tstat_over_ride[8];
extern U8_T far tstat_serial_number[8][4];

extern U16_T  far time_tstat_off[254];
extern U16_T  far time_tstat_on[254];

extern U8_T far WRT_Tst_Reg;
extern U8_T far  WRT_Tst_ID;

extern U8_T scan_status;
extern U16_T scan_index;

#define Tst_reg_num 18	 // tstat important register
enum{ 
TST_PRODUCT_MODEL ,
TST_OCCUPIED,
TST_COOL_SETPOINT ,
TST_HEAT_SETPOINT,
TST_ROOM_SETPOINT ,
TST_ROOM_TEM,
TST_MODE,
TST_OUTPUT_STATE ,
TST_NIGHT_HEAT_DB ,
TST_NIGHT_COOL_DB ,
TST_NIGHT_HEAT_SP,
TST_NIGHT_COOL_SP,
TST_ADDRESS,
TST_OVER_RIDE,
TST_SERIAL_NUM_0,
TST_SERIAL_NUM_1,
TST_SERIAL_NUM_2,
TST_SERIAL_NUM_3,
};


#define TSTAT_5A  	0
#define TSTAT_6 	1
#define TSTAT_5E	2


#define NO_SOURCE 	0
#define SCAN_CMD	1
#define GET_INFO_CMD 2
#define PC_SERIAL	 3
#define BUTTON		 4
#define SCHEDUEL	5
#define PC_TCPIP	6

#if 0  // TSTAT5
#define TSTAT_PRODUCT_MODEL    7
#define TSTAT_OCCUPIED		 184
#define TSTAT_COOL_SETPOINT  380
#define TSTAT_HEAT_SETPOINT  136
#define TSTAT_ROOM_SETPOINT  135 // 174
#define TSTAT_ROOM_TEM		 101
#define TSTAT_MODE			 107
#define TSTAT_OUTPUT_STATE   108
#define TSTAT_NIGHT_HEAT_DB  123
#define TSTAT_NIGHT_COOL_DB  124
#define TSTAT_NIGHT_HEAT_SP  182
#define TSTAT_NIGHT_COOL_SP  183
#define TSTAT_ADDRESS			6
#define TSTAT_OVER_RIDE		 211
#define TSTAT_SERIAL_NUM_0	 0
#define TSTAT_SERIAL_NUM_1	 1
#define TSTAT_SERIAL_NUM_2	 2
#define TSTAT_SERIAL_NUM_3	 3
#endif

#if 0 // tstat6

#define TSTAT_OCCUPIED		 184
#define TSTAT_COOL_SETPOINT  380
#define TSTAT_HEAT_SETPOINT  136
#define TSTAT_ROOM_SETPOINT  135 // 174
#define TSTAT_ROOM_TEM		 101
#define TSTAT_MODE			 107
#define TSTAT_OUTPUT_STATE   108
#define TSTAT_NIGHT_HEAT_DB  123
#define TSTAT_NIGHT_COOL_DB  124
#define TSTAT_NIGHT_HEAT_SP  182
#define TSTAT_NIGHT_COOL_SP  183
#define TSTAT_ADDRESS			6
#define TSTAT_PRODUCT_MODEL    7
#define TSTAT_OVER_RIDE		 211
#define TSTAT_SERIAL_NUM_0	 0
#define TSTAT_SERIAL_NUM_1	 1
#define TSTAT_SERIAL_NUM_2	 2
#define TSTAT_SERIAL_NUM_3	 3

#endif

enum
{	
	READ_PRODUCT_MODLE = 0,	
	READ_ROOM_SETPOINT,

	READ_COOLING_SETPOINT,
	READ_HEATTING_SETPOINT,	
	READ_TEMPERAUTE,
	READ_MODE_OPERATION,
	READ_OUTPUT_STATE,
	READ_OCCUPIED_STATE,
	READ_NIGHT_COOL_DB,
	READ_NIGHT_HEAT_DB,
	READ_NIGHT_HEAT_SP,
	READ_NIGHT_COOL_SP,
//	READ_PRODUCT_MODLE,
	READ_OVER_RIDE,
	READ_SERIAL_NUMBER_0,
	READ_SERIAL_NUMBER_1,
	READ_SERIAL_NUMBER_2,
	READ_SERIAL_NUMBER_3,

	READ_WALL_SETPOINT,
	READ_ADDRESS, 			// read 
	

	WRITE_ROOM_SETPOINT,	
	WRITE_COOLING_SETPOINT,		
	WRITE_HEATTING_SETPOINT,	
	WRITE_NIGHT_HEAT_DB,	
	WRITE_NIGHT_COOL_DB,	
	WRITE_NIGHT_HEAT_SP,		
	WRITE_NIGHT_COOL_SP,	
	WRITE_OVER_RIDE,	
	  // for innvox tstat 
	WRITE_WALL_SETPOINT,  // for innvox tstat 
	SEND_SCHEDUEL,     // write
	
	
};

void Comm_Tstat_Initial_Data(void);
void Com_Tstat(U8_T  types,U8_T addr);
void internal_sub_deal(U8_T cmd_index,U8_T tst_addr_index,U8_T *sub_net_buf);
void vStartCommSubTasks( U8_T uxPriority);
void Comm_Tstat_task(void);



#endif
