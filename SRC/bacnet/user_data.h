#ifndef USER_DATA_H
#define USER_DATA_H

#include "ud_str.h"
#include "monitor.h"

extern uint8_t far BACnet_Port;
extern uint16_t 	Instance;

extern Str_Panel_Info 		far 		Panel_Info;

extern Str_in_point far inputs[MAX_INS];
extern Str_out_point far	outputs[MAX_OUTS];
//extern Str_out_point  far	*outputs;
extern uint8_t				far				 no_outs;
//extern Str_in_point   far	*inputs;
extern uint8_t				far				 no_ins;

extern Info_Table			far						 info[18];

extern unsigned long 				timestart;	   /* seconds since the beginning of the year */
extern unsigned long 				ora_current_sec;  /* seconds since the beginning of the day */
extern unsigned long 				time_since_1970;   /* seconds since the beginning of 2010 */
extern In_aux					far						 in_aux[MAX_IO_POINTS];
extern Con_aux				far							 con_aux[MAX_CONS];
extern Mon_aux           far           mon_aux[MAX_MONITORS];
extern Monitor_Block		far			*mon_block;
extern char far              mon_data_buf[10000];
extern Mon_Data 			far 		*Graphi_data;
extern char 				far			Garphi_data_buf[sizeof(Mon_Data)];

extern S16_T                          MAX_MONITOR_BLOCKS;
extern U8_T           far              free_mon_blocks;

extern Byte			               	Station_NUM;
extern U8_T 	client_ip[4];
extern U8_T newsocket;
extern U8_T flag_old_seocket;

//extern Alarm_point 		    far				 alarms[MAX_ALARMS];
//extern U8_T 			    far							 ind_alarms;
//extern Alarm_set_point 	far    			 alarms_set[MAX_ALARMS_SET];
//extern U8_T 			    far							 ind_alarms_set;
//extern Units_element		    far				 units[MAX_UNITS];
//extern Password_struct 	    far			 passwords;

extern Str_variable_point			far		 vars[MAX_VARS];
extern Str_controller_point 	far			 controllers[MAX_CONS];
extern Str_totalizer_point     far     totalizers[MAX_TOTALIZERS];
extern Str_monitor_point		far				 monitors[MAX_MONITORS];   
extern Str_monitor_point		far			backup_monitors[MAX_MONITORS]/* _at_ 0x12800*/;	
extern Aux_group_point        far 		 aux_groups[MAX_GRPS];
//extern S8_T                    far		 Icon_names[MAX_ICONS][14];
extern Control_group_point  	far 		 control_groups[MAX_GRPS];
extern Str_grp_element		far	    		 group_data[MAX_ELEMENTS];
extern S16_T 					far							 total_elements;
extern S16_T 					far							 group_data_length;


extern Str_weekly_routine_point far		 weekly_routines[MAX_WR] ;
extern Wr_one_day				far		wr_times[MAX_WR][MAX_SCHEDULES_PER_WEEK];
extern Str_annual_routine_point	far	 annual_routines[MAX_AR];
extern U8_T                   far      ar_dates[MAX_AR][AR_DATES_SIZE];	

 /* Assume bit0 from octet0 = Jan 1st */
extern Str_program_point	    far			 programs[MAX_PRGS];
extern S8_T 			    	far			*program_address[MAX_PRGS]; /*pointer to code*/
extern U8_T    	    	far				prg_code[MAX_PRGS][CODE_ELEMENT];
extern U16_T			far	 			Code_len[MAX_PRGS];
extern U16_T 			far				Code_total_length;
//extern Str_array_point 	    far			 arrays[MAX_ARRAYS];
extern S32_T  			    				*arrays_address[MAX_ARRAYS];
//Str_table_point			far				 custom_tab[MAX_TBLS];
extern U16_T                far         PRG_crc;
extern U8_T far *prog;
extern S32_T far stack[20];
extern S32_T far *index_stack;
extern S8_T far *time_buf;
extern U32_T far cond;
extern S32_T far v, value;
extern S32_T far op1,op2;
extern S32_T far n,*pn;





extern U8_T far table_bank[TABLE_BANK_LENGTH];



void init_panel(void);


#endif



