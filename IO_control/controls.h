#ifndef	CONTROLS_H
#define	CONTROLS_H

#include "product.h"



#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
#define ARM_CON 
#else
#define ASIX_CON
#endif




//#define MANUAL_JUMPER	
#define AUTO_JUMPER


#ifdef ARM_CON

#define far 
#define xdata 
#define code 

#define MAX_INS     	64	  	
#define MAX_OUTS        64  

#pragma pack(1) 

#endif


#include "ud_str.h"

#ifdef ASIX_CON
#define MAX_INS     	64	  	
#define MAX_OUTS        64  
#endif

#define INPUT
#define OUTPUT

typedef enum
{
	INPUT_NOUSED = 0,
	INPUT_I0_20ma,
	INPUT_V0_5,
	INPUT_0_10V,
	INPUT_THERM,
}E_IN_TYPE;

extern uint8_t far input_type[32];
extern Str_in_point  far  inputs[MAX_INS];
extern Str_out_point far	outputs[MAX_OUTS];
extern U8_T far control_auto[MAX_OUTS];
extern U8_T far switch_status_back[MAX_OUTS];

void control_input(void);
void control_output(void);
int32_t swap_double( int32_t dat );
int16_t swap_word( int16_t dat );
uint32_t get_input_value_by_range(uint8_t range, uint16_t raw);


extern U16_T far Test[50];



// do it in own code
extern void Set_Input_Type(uint8_t point);  
extern uint16_t get_input_raw(uint8_t point);
extern void set_output_raw(uint8_t point,uint16_t value);
extern uint16_t get_output_raw(uint8_t point);
//extern uint16_t Filter(uint8_t channel,uint16_t input);
extern uint8_t get_max_output(void);
extern uint8_t get_max_input(void);
extern uint32_t conver_by_unit_5v(uint32_t sample);
extern uint32_t conver_by_unit_10v(uint32_t sample);
extern uint32_t conver_by_unit_custable(uint8_t point,uint32_t sample);
extern uint32_t get_high_spd_counter(uint8_t point);


void map_extern_output(uint8_t point);
extern uint8_t get_max_internal_input(void);
extern uint8_t get_max_internal_output(void);
S8_T check_external_in_on_line(U8_T index);
S8_T check_external_out_on_line(U8_T index);


void Check_Send_bip(void);
void Check_Program_Output_Pri_valid(void);
void Send_UserList_Broadcast(U8_T start,U8_T end);
void Check_Remote_Panel_Table();
S8_T Get_rmp_index_by_panel(uint8_t panel,uint8_t sub_id,uint8_t * index,uint8_t protocal);


#endif

