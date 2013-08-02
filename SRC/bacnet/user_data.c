
#include "ud_str.h"
#include "string.h"
#include "types.h"


//#include "schedule.h"

Str_in_point far inputs[MAX_INS];
Str_out_point far	outputs[MAX_OUTS];
uint8_t				far				 no_outs;
uint8_t				far				 no_ins;

Info_Table			far						 info[18];


//In_aux					far						 in_aux[MAX_IO_POINTS];
//Con_aux				far							 con_aux[MAX_CONS];
//Mon_aux           far           mon_aux[MAX_MONITORS];
//Monitor_Block		far					   *mon_block;
//Alarm_point 		    far				 alarms[MAX_ALARMS];
//U8_T 			    far							 ind_alarms;
//Alarm_set_point 	far    			 alarms_set[MAX_ALARMS_SET];
//U8_T 			    far							 ind_alarms_set;
//Units_element		    far				 units[MAX_UNITS];
//Password_struct 	    far			 passwords;

Str_variable_point			far		 vars[MAX_VARS];
Str_controller_point 	far			 controllers[MAX_CONS];
//Str_totalizer_point     far     totalizers[MAX_TOTALIZERS];
//Str_monitor_point		far				 monitors[MAX_MONITORS];
//Aux_group_point        far 		 aux_groups[MAX_GRPS]/* _at_ 0x4000*/;
//S8_T                    far		 Icon_names[MAX_ICONS][14];
//Control_group_point  	far 		 control_groups[MAX_GRPS];
//Str_grp_element		far	    		 group_data[MAX_ELEMENTS];
S16_T 					far							 total_elements;
S16_T 					far							 group_data_length;


Str_weekly_routine_point far		 weekly_routines[MAX_WR] ;
Wr_one_day				far		wr_times[MAX_WR][MAX_SCHEDULES_PER_WEEK];
Str_annual_routine_point	far	 annual_routines[MAX_AR];
U8_T                   far      ar_dates[MAX_AR][AR_DATES_SIZE];	

 /* Assume bit0 from octet0 = Jan 1st */
Str_program_point	    far			 programs[MAX_PRGS];
S8_T 			    	far			*program_address[MAX_PRGS]; /*pointer to code*/
S8_T    	    	far				prg_code[MAX_PRGS][480];
U16_T				far	 			Code_len[MAX_PRGS];
U16_T 			    far				total_length;
//Str_array_point 	    far			 arrays[MAX_ARRAYS];
S32_T  			    				*arrays_address[MAX_ARRAYS];
//Str_table_point			far				 custom_tab[MAX_TBLS];
U16_T                far         PRG_crc;


U8_T far *prog;
S32_T far stack[20];
S32_T far *index_stack;
S8_T far *time_buf;
S16_T far cond;
S32_T far v, value;
S32_T far op1,op2;
S32_T far n,*pn;


extern STR_flag_flash 	far bac_flash;

#if 1
U8_T far table_bank[TABLE_BANK_LENGTH] =
{
	 MAX_OUTS,     	/*OUT*/
	 MAX_INS,     	/*IN*/
	 MAX_VARS,      /*VAR*/
	 MAX_CONS,     	/*CON*/
	 MAX_WR,        /*WR*/
	 MAX_AR,        /*AR*/
	 MAX_PRGS,     	/*PRG*/
	 MAX_TBLS,      /*TBL*/
	 MAX_TOTALIZERS, /*TOTAL*/
	 MAX_MONITORS,	/*AMON*/
	 MAX_GRPS,      /*GRP*/
	 MAX_ARRAYS,    /*AY*/
	 MAX_ALARMS,
	 MAX_UNITS,
	 MAX_PASSW
};
#endif

char * TEST_IN[10] = {"INPUT1","INPUT2","INPUT3","INPUT4","INPUT5","INPUT6","INPUT7","INPUT8","INPUT9","INPUT10"};
char * TEST_OUT[2] = {"OUTPUT1","OUTPUT2"};



void init_info_table( void )
{
	int i;
	Info_Table *inf;

	inf = &info[0];
	for( i=0; i< MAX_INFO_TYPE; i++, inf++ )
	{
		switch( i )
		{
			case OUT:
				inf->address = (S8_T *)outputs;
				inf->size = sizeof( Str_out_point );
				inf->max_points =  MAX_OUTS;
				break;
			case IN:
				inf->address = (S8_T *)inputs;
				inf->size = sizeof( Str_in_point );
				inf->max_points =  MAX_INS;
				break;
			case VAR:
				inf->address = (S8_T *)vars;
				inf->size = sizeof( Str_variable_point );
				inf->max_points =  MAX_VARS;
				break;
			case CON:
				inf->address = (S8_T *)controllers;
				inf->size = sizeof( Str_controller_point );
				inf->max_points = MAX_CONS;
				break;
			case WRT:
				inf->address = (S8_T *)weekly_routines;
				inf->size = sizeof( Str_weekly_routine_point );
				inf->max_points = MAX_WR;
				break;
			case AR:
				inf->address = (S8_T *)annual_routines;
				inf->size = sizeof( Str_annual_routine_point );
				inf->max_points = MAX_AR;
				break;
			case PRG:
				inf->address = (S8_T *)programs;
				inf->size = sizeof( Str_program_point );
				inf->max_points = MAX_PRGS;
				break;
		/*	case TBL:
				inf->address = (S8_T *)custom_tab;
				inf->size = sizeof( Str_table_point );
				inf->max_points = MAX_TBLS;
				break;
			case TZ:
				inf->address = (S8_T *)totalizers;
				inf->size = sizeof( Str_totalizer_point );
				inf->max_points = MAX_TOTALIZERS;
				break;
			case AMON:
				inf->address = (S8_T *)monitors;
				inf->size = sizeof( Str_monitor_point );
				inf->max_points = MAX_MONITORS;
				break;
			case GRP:
				inf->address = (S8_T *)control_groups;
				inf->size = sizeof( Control_group_point );
				inf->max_points = MAX_GRPS;
				break;
			case ARRAY:
				inf->address = (S8_T *)arrays;
				inf->size = sizeof( Str_array_point );
				inf->max_points = MAX_ARRAYS;
				break;
			case ALARMM:          // 12
				inf->address = (S8_T *)alarms;
				inf->size = sizeof( Alarm_point );
				inf->max_points = MAX_ALARMS;
				break;
			case ALARM_SET:         //15
				inf->address = (S8_T *)alarms_set;
				inf->size = sizeof( Alarm_set_point );
				inf->max_points = MAX_ALARMS_SET;
				break;
			case UNIT:
				inf->address = (S8_T *)units;
				inf->size = sizeof( Units_element );
				inf->max_points = MAX_UNITS;
				break;
			case USER_NAME:
				inf->address = (S8_T *)&passwords;
				inf->size = sizeof( Password_point );
				inf->max_points = MAX_PASSW;
				break;	  */
			case WR_TIME:
				inf->address = (S8_T *)wr_times;
				inf->size = 9*sizeof( Wr_one_day );
				inf->max_points = MAX_SCHEDULES_PER_WEEK;
				break;
			case AR_DATA:               // 17 ar_dates[MAX_AR][AR_DATES_SIZE];
				inf->address = (S8_T *)ar_dates;
				inf->size = AR_DATES_SIZE;
				inf->max_points = MAX_AR;
				break;
			default:
				break;	
		}
	}
}


void init_panel(void)
{
	char i;
	Str_points_ptr ptr;
//	U8_T flag_read;

//	ind_alarms = 0;
//	ind_alarms_set = 0;

   	memset(inputs, '\0', MAX_INS *sizeof(Str_in_point) );
	ptr.pin = inputs;
	for( i=0; i< MAX_INS; i++, ptr.pin++ )
	{
		ptr.pin->value = 0;  
		memcpy(ptr.pin->description,TEST_IN[i],10);	
		ptr.pin->filter = 5;  /* (3 bits; 0=1,1=2,2=4,3=8,4=16,5=32, 6=64,7=128,)*/
		ptr.pin->decom = 0;	   /* (1 bit; 0=ok, 1=point decommissioned)*/
		ptr.pin->sen_on = 1;/* (1 bit)*/
		ptr.pin->sen_off = 1;  /* (1 bit)*/
		ptr.pin->control = 1; /*  (1 bit; 0=OFF, 1=ON)*/
		ptr.pin->auto_manual = 0; /* (1 bit; 0=auto, 1=manual)*/
		ptr.pin->digital_analog = 1; /* (1 bit; 1=analog, 0=digital)*/
		ptr.pin->calibration_sign = 1;; /* (1 bit; sign 0=positiv 1=negative )*/
		ptr.pin->calibration_increment = 1;; /* (1 bit;  0=0.1, 1=1.0)*/
		ptr.pin->unused = 0; /* (5 bits - spare )*/	
		ptr.pin->calibration = 100;  /* (8 bits; -25.6 to 25.6 / -256 to 256 )*/		
	}


	memset(outputs,'\0', MAX_OUTS *sizeof(Str_in_point) );
	ptr.pout = outputs;
	for( i=0; i<MAX_OUTS; i++, ptr.pout++ )
	{
		ptr.pout->value = 0; 		
		if(i < 12)   // digital outputs
		{
			ptr.pout->range = 101;
			ptr.pout->digital_analog = 1;
		}
		else 	
		{			// analog outputs
			ptr.pout->range = 1;
			ptr.pout->digital_analog = 0;

		}

		memcpy(ptr.pout->description,"test1",5);	
		ptr.pout->auto_manual = 1;
	} 


	memset(controllers,'\0',MAX_CONS*sizeof(Str_controller_point));
	memset(programs,'\0',MAX_PRGS *sizeof(Str_program_point));
	ptr.pprg = programs;
	for( i=0; i<MAX_PRGS; i++, ptr.pprg++ )
	{
		ptr.pprg->on_off = 1;
		ptr.pprg->auto_manual = 1;
	} //test by chelsea	
	memset(program_address, '\0',MAX_PRGS *sizeof(S8_T *));
	memset(prg_code, '\0', MAX_PRGS * 480);
	for(i = 0;i < MAX_PRGS;i++)		
		program_address[i] = &prg_code[i]; // program_address pointer to code

	total_length = 0;
	 
	memset(vars,'\0',MAX_VARS*sizeof(Str_variable_point));
	ptr.pvar = vars;
	for( i=0; i<MAX_VARS; i++, ptr.pvar++ )
	{
		ptr.pvar->value = 0;
		ptr.pvar->auto_manual = 1;
		ptr.pvar->digital_analog = 1; //analog point 
		ptr.pvar->unused = 2; 
		ptr.pvar->range = 0;
	}
 #if 0 
	memset( control_groups,'\0', MAX_GRPS * sizeof( Control_group_point) );
	memset( aux_groups,'\0', MAX_GRPS * sizeof( Aux_group_point) );
	memset( group_data, '\0', MAX_ELEMENTS * sizeof( Str_grp_element) );
	total_elements = 0;
	group_data_length = 0;
	ptr.pgrp = control_groups;		
	for( i=0; i<MAX_GRPS; i++, ptr.pgrp++ )
	{
		ptr.pgrp->update = 15;
	}



	memset(controllers,'\0',MAX_CONS*sizeof(Str_controller_point));
	memset(con_aux,'\0',MAX_CONS*sizeof(Con_aux));
	memset(custom_tab,'\0',MAX_TBLS*16*sizeof(Tbl_point));
	memset(&passwords,'\0',sizeof(Password_struct));

	memset(network_points_list,0,sizeof(network_points_list));
	number_of_network_points=0;
	memset(remote_points_list,0,sizeof(remote_points_list));
	number_of_remote_points=0;
	

	memset( weekly_routines,'\0',MAX_WR*sizeof(Str_weekly_routine_point));
	memset( wr_times,'\0',MAX_WR*9*sizeof(Wr_one_day ));
	memset( annual_routines,'\0',MAX_AR*sizeof(Str_annual_routine_point));
	memset( ar_dates,'\0',MAX_AR*46*sizeof(S8_T));
	
	
	memset( totalizers,'\0',MAX_TOTALIZERS*sizeof(Str_totalizer_point));

	memset(arrays,'\0',MAX_ARRAYS*sizeof(Str_array_point));
	memset(arrays_data,'\0',MAX_ARRAYS*sizeof(S32_T *));
	memset(units,'\0',MAX_UNITS*sizeof(Units_element));


	memset( monitors,'\0',MAX_MONITORS*sizeof(Str_monitor_point));
	memset( mon_aux,'\0',MAX_MONITORS*sizeof(Mon_aux) );
//	mon_block = flash_data;  // tbd: changed by chelsea
	memset( mon_block,'\0',MAX_MONITOR_BLOCKS*sizeof(Monitor_Block) );
	ptr.pmon = monitors;
	for( i=0; i<MAX_MONITORS; i++, ptr.pmon++ )
	{
		ptr.pmon->minute_interval_time = 15;
	/*	monitors[i].data_length = 72;	*/
	/*	monitors[i].unit = 1;*/
		//ptr.pmon->double_flag=1;
		SetByteBit(&ptr.pmon->flag2,1,mon_double_flag,1);
	}
	free_mon_blocks = MAX_MONITOR_BLOCKS;

	
	memset(alarms,'\0',MAX_ALARMS*sizeof(Alarm_point));
	memset(alarms_set,'\0',MAX_ALARMS_SET*sizeof(Alarm_set_point));
#endif
/*	memset(network_points_list,0,sizeof(network_points_list));
	number_of_network_points=0;
	memset(remote_points_list,0,sizeof(remote_points_list));
	number_of_remote_points=0;*/
	init_info_table();
}


