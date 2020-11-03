#ifndef	COMMSUB_H
#define	COMMSUB_H

#include "types.h"
#include "scan.h"
#include "ProductModel.h"

#define SUB_NO  254

// ¿©’πIO”≥…‰
typedef struct
{
  U8_T sub_index;
	U8_T type;
	U8_T id;
	U8_T do_start;
	U8_T ao_start;
	U8_T ai_start;
	U8_T var_start;
	U8_T do_len;
	U8_T ao_len;
	U8_T ai_len;
	U8_T var_len;
	U8_T add_in_map;
}STR_MAP_table;

extern STR_MAP_table far sub_map[SUB_NO];
//extern TST_INFO far tst_info[SUB_NO];

//extern U8_T far tst_addr_index[3];
//extern U8_T far tst_reg_index[3];


//extern U8_T far sub_addr[SUB_NO];
extern U8_T far uart0_sub_addr[SUB_NO];


extern U8_T far uart1_sub_addr[SUB_NO];
extern U8_T far uart2_sub_addr[SUB_NO];	
extern U8_T far uart1_sub_no;
extern U8_T far uart2_sub_no;

extern U8_T far sub_no;
extern U8_T far online_no;
extern U8_T far uart0_sub_no;

//#define TST_MAX_READ_LEN  10
#define TST_MAX_READ_COUNT 5


typedef struct 
{
// start reg   len	 mum
//	U8_T index;
	U16_T start_reg;
	U8_T len;
//	U8_T valid_reg[TST_MAX_READ_LEN];
}STR_Read_tst_by_block;

enum{ 
	TST_PRODUCT_MODEL = 0,
	TST_OCCUPIED,
	TST_COOL_SETPOINT ,
	TST_HEAT_SETPOINT,
	TST_ROOM_SETPOINT ,
	TST_ROOM_TEM,
	TST_MODE,
	TST_COOL_HEAT_MODE,
	TST_OUTPUT_STATE ,
	TST_NIGHT_HEAT_DB ,
	TST_NIGHT_COOL_DB ,
	TST_NIGHT_HEAT_SP,
	TST_NIGHT_COOL_SP,
	//TST_DAY_HEAT_DB,
	//TST_DAY_COOL_DB,
	TST_REG_END,
	
	TST_ADDRESS = TST_REG_END,
	//TST_OVER_RIDE,
	TST_PORT,
	TST_SERIAL_NUM_0,
	TST_SERIAL_NUM_1,
	TST_SERIAL_NUM_2,
	TST_SERIAL_NUM_3,
};


typedef enum
{
	TSTAT_5A =	0,
	TSTAT_6, 
	TSTAT_5E,
	
	T3_8AI8AO,	 // 21
	T3_8AI13DO, // 20
	T3_4AO,	 // 28
	T3_32AI,	 // 22
	
	MAX_SUB_TYPE
}E_DEVICE_TYPE;

//#define PRODUCT_4AO  28
//#define PRODUCT_8AO  21
//#define PRODUCT_8A13O  20
//#define PRODUCT_32AI  22
//#define PRODUCT_6CT  29

//#define PRODUCT_22AI  29
//#define PRODUCT_8IO6D  29


typedef enum
{
	MAP_AI =	0,
	MAP_DI, 
	MAP_DO,
	MAP_AO,
	MAP_VAR,	
	MAX_MAP_TYPE
}E_POINT_TYPE;


#define T3_4AO_DO_REG_START 100  // 8
#define T3_4AO_AO_REG_START 108  // 4
#define T3_4AO_AI_REG_START 190  // 10


#define T3_8AO_AO_REG_START	100
#define T3_8AO_AI_REG_START 108

#define T3_8A13O_DO_REG_START	100 // 13byte
#define T3_8A13O_AI_REG_START 118 // 16
// 16byte  119 121 123 is normal value, 118+119 is high spd counter

#define T3_32I_AI_REG_START  100

// T3-22I
#define T3_22I_AI_REG_START  100  // 22*2

// T3-PT12
#define T3_PT12_AI_REG_START 100		// 12 * 1

// T3-8O
#define T3_8AIAO6DO_AO_REG_START 100 // 8*1
#define T3_8AIAO6DO_AI_REG_START 116 // 8*2
#define T3_8AIAO6DO_DO_REG_START 108 // 6*1


// T3-6CT
#define T3_6CTA_AI_REG_START 104  // 19 * 2
#define T3_6CTA_DO_REG_START 100  // 2 * 1

#define T3_LC_AI_REG_START 		101  // 7 * 1		101	-	107
#define T3_LC_DO_REG_START 		111  // 7 * 1  	111 - 117




// STM32_CO2_NET				210
//#define CO2_TEMPERAUTE_REG_START 104
//#define CO2_TEMPERAUTE_REG_START 104
//#define CO2_TEMPERAUTE_REG_START 104



// STM32_HUM_NET				212


// STM32_PRESSURE_NET		214



//#define T5_DO		108
#define T6_DO		209
#define T6_AO_REG_START		210  // 210 211



#define Tst_reg_num 19 //21
//#define Tst_reg_num TST_REG_END	 // tstat important register
extern const U16_T code Tst_Register[Tst_reg_num][3];
extern const STR_Read_tst_by_block	code  Read_tst_by_block[MAX_SUB_TYPE][TST_MAX_READ_COUNT];
extern const U8_T  code len_sub_in_map[MAX_SUB_TYPE][MAX_MAP_TYPE];

extern U8_T base_in;
extern U8_T base_out;
extern U8_T base_var;
//enum
//{	
//	READ_PRODUCT_MODLE = 0,	
//	READ_ROOM_SETPOINT,
//
//	READ_COOLING_SETPOINT,
//	READ_HEATTING_SETPOINT,	
//	READ_TEMPERAUTE,
//	READ_MODE_OPERATION,
//	READ_COOL_HART_MODE,
//	READ_OUTPUT_STATE,
//	READ_OCCUPIED_STATE,
//	READ_NIGHT_COOL_DB,
//	READ_NIGHT_HEAT_DB,
//	READ_NIGHT_HEAT_SP,
//	READ_NIGHT_COOL_SP,
////	READ_PRODUCT_MODLE,
//	READ_OVER_RIDE,
//	READ_SERIAL_NUMBER_0,
//	READ_SERIAL_NUMBER_1,
//	READ_SERIAL_NUMBER_2,
//	READ_SERIAL_NUMBER_3,
//
//	READ_WALL_SETPOINT,
//	READ_ADDRESS, 			// read 
//	
//
//	WRITE_ROOM_SETPOINT,	
//	WRITE_COOLING_SETPOINT,		
//	WRITE_HEATTING_SETPOINT,	
//	WRITE_NIGHT_HEAT_DB,	
//	WRITE_NIGHT_COOL_DB,	
//	WRITE_NIGHT_HEAT_SP,		
//	WRITE_NIGHT_COOL_SP,	
//	WRITE_OVER_RIDE,	
//	  // for innvox tstat 
//	WRITE_WALL_SETPOINT,  // for innvox tstat 
//	SEND_SCHEDUEL,     // write
//	
//	
//};


typedef enum
{
	VAR_SN,
	VAR_ID,
	VAR_TYPE,
	VAR_PRODUCT,
	VAR_INTER_TEMP,
	VAR_C_H_MODE,
	VAR_MODE,
	VAR_OUTPUT_STATE,
	VAR_NH_DB,
	VAR_NC_DB,
	VAR_SP,
	VAR_COO_SP,
	VAR_HEAT_SP,
	VAR_NH_SP,
	VAR_NC_SP,
	VAR_OCC,
}TSTAT_VAR;



U8_T Get_index_by_AOx(U8_T ao_index,U8_T *out_index);
U8_T Get_index_by_BOx(U8_T do_index,U8_T *out_index);
U8_T Get_index_by_AIx(U8_T ai_index,U8_T *in_index);
U8_T Get_index_by_BIx(U8_T bi_index,U8_T *in_index);
U8_T Get_index_by_AVx(U8_T av_index,U8_T *var_index);
U8_T Get_index_by_BVx(U8_T bv_index,U8_T *var_index);

U8_T Get_AOx_by_index(U8_T index,U8_T *ao_index);
U8_T Get_BOx_by_index(U8_T index,U8_T *bo_index);
U8_T Get_AIx_by_index(U8_T index,U8_T *ai_index);
U8_T Get_BIx_by_index(U8_T index,U8_T *bi_index);
U8_T Get_AVx_by_index(U8_T index,U8_T *av_index);
U8_T Get_BVx_by_index(U8_T index,U8_T *bv_index);

S32_T my_honts_arm(S32_T val);

void Comm_Tstat_Initial_Data(void);
void Com_Tstat(U8_T  cmd,U8_T addr,U8_T port);
void internal_sub_deal(U8_T cmd_index,U8_T tst_addr_index,U8_T *sub_net_buf);
void vStartCommSubTasks( U8_T uxPriority);
void Comm_Tstat_task(void);
void remap_table(U8_T index,U8_T type);
void update_remote_map_table(U8_T id,U16_T reg,U16_T value,U8_T *buf);
U16_T count_output_reg(U8_T * sub_index,U8_T map_type,U8_T point);
U8_T get_index_from_id(U8_T id,U8_T *index);

void update_extio_to_database(void);
void refresh_extio_by_database(U8_T ai_start,U8_T ai_end,U8_T out_start,U8_T out_end,U8_T update_time);
U8_T Get_extio_index_by_id(U8_T id,U8_T *index);
#endif
