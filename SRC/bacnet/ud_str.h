#ifndef UD_STR_H
#define UD_STR_H

#include <stdint.h>
#include <stdbool.h>
#include "types.h"
#include "flash_schedule.h"



#define MAX_OUTS	10
#define MAX_INS     10
#define MAX_VARS               16//64
#define MAX_CONS               16

#define MAX_MONITORS           8

#define MAX_TOTALIZERS         2  /* MAX_IO_POINTS */

#define MAX_ELEMENTS        240    /* total number of group element allowed */

#define MAX_GRPS               16
#define MAX_ICONS              16
#define MAX_WR                  8
#define MAX_SCHEDULES_PER_WEEK  9
#define MAX_INTERVALS_PER_DAY	  4
#define MAX_AR                  4
#define AR_DATES_SIZE          46
#define MAX_PRGS               6//16
#define MAX_TBLS                5
#define PROGRAMS_POOL_SIZE 		0x2800  // 640 * 16
#define MAX_ALARMS             16
#define MAX_ALARMS_SET         16
#define MAX_ARRAYS             16
#define MAX_ARRAYS_DATA		 2048

#define MAX_UNITS               8
#define MAX_INFO_TYPE		       20
#define MAX_VIEWS		            3
#define MAX_PASSW		            8
#define MAX_STATIONS           32
#define MAXREMOTEPOINTS		     30
#define MAX_REMOTE_POINTS      30
#define MAXNETWORKPOINTS       32
#define TABLE_BANK_LENGTH      15


typedef enum
	{
		OUT=0, IN, VAR, CON, WRT, AR, PRG, TBL, TZ, AMON, GRP, ARRAY, ALARMM,
		UNIT, USER_NAME, ALARM_SET, WR_TIME, AR_DATA
	}	Point_type_equate;

typedef enum { 
		 READOUTPUT_T3000          = OUT+1,  /* read outputs */
		 READINPUT_T3000           = IN+1,   /* read inputs  */
		 READVARIABLE_T3000        = VAR+1,        /* read variables*/
		 READCONTROLLER_T3000      = CON+1,        /* read controllers*/
		 READWEEKLYROUTINE_T3000   = WRT+1,         /* read weekly routines*/
		 READANNUALROUTINE_T3000   = AR+1,         /* read annual routines*/
		 READPROGRAM_T3000         = PRG+1,        /* read programs       */
		 READTABLE_T3000           = TBL+1,        /* read tables         */
         READTOTALIZER_T3000       = TZ+1,         /* read totalizers     */
		 READMONITOR_T3000         = AMON+1,       /* read monitors       */
		 READSCREEN_T3000          = GRP+1,        /* read screens        */
		 READARRAY_T3000           = ARRAY+1,      /* read arrays         */
//		 READARRAYVALUE_T3000      = AYVALUE+1,    /* read array elements */

		 READPROGRAMCODE_T3000     = 16,           /* read program code   */
		 READTIMESCHEDULE_T3000    = WR_TIME+1,    /* read time schedule  */
		 READANNUALSCHEDULE_T3000  = AR_DATA+1,    /* read annual schedule*/
		 READGROUPELEMENTS_T3000   = 19,           /* read group elements */
		 READPOINTINFOTABLE_T3000  = 24,           /* read pointinfo table*/
		 UPDATEMEMMONITOR_T3000    = 23,           /* read monitor updates*/
		 READMONITORDATA_T3000     = 22,           /* read monitor data   */
		 READINDIVIDUALPOINT_T3000 = 20,           /* read individual point*/
		 READGROUPELEMENT_T3000    = 25,           /* read point info      */
		 TIME_COMMAND              = 21,           /* read time            */
		 CLEARPANEL_T3000          = 28,           /* clear panel          */
		 SEND_ALARM_COMMAND        = 32,

		 WRITEOUTPUT_T3000         = 100+OUT+1,  /* write outputs          */
		 WRITEINPUT_T3000          = 100+IN+1,   /* write inputs           */
		 WRITEVARIABLE_T3000       = 100+VAR+1,        /* write variables  */
		 WRITECONTROLLER_T3000     = 100+CON+1,        /* write controllers*/
		 WRITEWEEKLYROUTINE_T3000  = 100+WRT+1,         /* write weekly routines*/
		 WRITEANNUALROUTINE_T3000  = 100+AR+1,         /* write annual routines*/
		 WRITEPROGRAM_T3000        = 100+PRG+1,        /* write programs       */
		 WRITETABLE_T3000          = 100+TBL+1,        /* write tables         */
     	 WRITETOTALIZER_T3000      = 100+TZ+1,         /* write totalizers     */
		 WRITEMONITOR_T3000        = 100+AMON+1,       /* write monitors       */
		 WRITESCREEN_T3000         = 100+GRP+1,        /* write screens        */
		 WRITEARRAY_T3000          = 100+ARRAY+1,      /* write arrays         */
		 WRITETIMESCHEDULE_T3000   = 100+WR_TIME+1,    /* write time schedule  */
		 WRITEANNUALSCHEDULE_T3000 = 100+AR_DATA+1,     /* write annual schedule*/
		 WRITEPROGRAMCODE_T3000    = 100+16,           /* write program code    */
		 WRITEINDIVIDUALPOINT_T3000 = 100+READINDIVIDUALPOINT_T3000,  /* write individual point*/

		 COMMAND_50                = 50,
		 READ_COMMAND_50           = 50,
		 WRITE_COMMAND_50          = 150,
		 STATION_LIST_COMMAND      = 21,
		 SAVEPROGRAM_COMMAND       = 30,
		 LOADPROGRAM_COMMAND       = 31,
		 DEFAULT_PRG_COMMAND       = 32, 
		 READFILES_COMMAND         = 40,  /* read the files of type define in*/
																					/* a previous SETFILETYPE_COMMAND command*/
																					/* from the current path.                */
																					/* The data returned are an array of type*/
																					/*  int8_t [13];                           */
		 READDIRECTORIES_COMMAND   = 41,  /* read the directories name from            */
																					/* the current path.                     */
																					/* The data returned are of type         */
																					/*  (*int8_t)[13];                         */
		 GETCURRENTPATH_COMMAND    = 42,  /* get the current path                      */
																					/* The command is a read command:        */
																					/*  command  = READ_COMMAND_50           */
																					/*  arg      = GETCURRENTPATH_COMMAND    */
		 SETCURRENTPATH_COMMAND    = 43,  /* set the current path for the subsequent   */
																					/* read directories and read files command*/
																					/* The command is a write command:        */
																					/*  command  = WRITE_COMMAND_50           */
																					/*  arg      = SETCURRENTPATH_COMMAND     */
		 SETFILETYPE_COMMAND       = 44,  /* set the file type (ex. "*.*", "*.prg").    */
																					/* the next read files command will       */
																					/* return only the files of type          */
																					/* set in a SETFILETYPE_COMMAND command.  */
																					/* The command is a write command:        */
																					/*  command  = WRITE_COMMAND_50           */
																					/*  arg      = SETFILETYPE_COMMAND        */
		 ALARM_NOTIFY_COMMAND       = 51,
		 SEND_INFO_COMMAND          = 52,
		 SEND_WANTPOINTS_COMMAND    = 72,
		 SEND_NETWORKPOINTS_COMMAND = 73,


		 TABLEPOINTS_COMMAND       = 75,
		 PANEL_INFO1_COMMAND       = 110,
		 PANEL_INFO2_COMMAND       = 111,
		 MINICOMMINFO_COMMAND      = 112,
		 PANELID_COMMAND           = 113,
		 ICON_NAME_TABLE_COMMAND   = 114,
		 WRITEDATAMINI_COMMAND     = 116,
		 SENDCODEMINI_COMMAND      = 117,
		 SENDDATAMINI_COMMAND      = 118,
		 READFLASHSTATUS_COMMAND   = 119,
		 READSTATUSWRITEFLASH_COMMAND = 120,
		 RESTARTMINI_COMMAND       = 121,
		 WRITEPRGFLASH_COMMAND     = 200,//122,
		 OPENSCREEN_COMMAND        = 123

} CommandRequest;

typedef enum { OUTPUT=1, INPUT, VARIABLE, CONTROLLER, WEEKLY_ROUTINE,
	ANNUAL_ROUTINE, PROGRAM, TABLES, TOTALIZER, ANALOG_MONITOR,
	CONTROL_GROUP, ARRAYS, ALARM, UNITs, USERS, COD, CONTROL_GROUP_DATA=19,
	AMON_DATA=22, DMON_DATA, MONITOR=40, DESCRIPTORS=47, ARRAYS2
	} Command_type_equate;

typedef enum  {
	unused, degC, degF, FPM, Pa, KPa, psi, in_w, Watts, KW, KWH,
	Volts, KV, Amps, ma, CFM, Sec, Min, Hours, Days, time_unit, ohms,
	procent, RH, ppm, counts,	Open, CFH, GPM, GPH, GAL, CF, BTU, CMH,
	custom1, custom2, custom3, custom4, custom5, custom6, custom7, custom8
	} Analog_units_equate;	



typedef enum { UNUSED=100,
	OFF_ON, CLOSED_OPEN, STOP_START, DISABLED_ENABLED,
	NORMAL_ALARM, NORMAL_HIGH, NORMAL_LOW, NO_YES,
	COOL_HEAT, UNOCCUPIED_OCCUPIED, LOW_HIGH,
	ON_OFF , OPEN_CLOSED, START_STOP, ENABLED_DISABLED,
	ALARM_NORMAL, HIGH_NORMAL, LOW_NORMAL, YES_NO,
	HEAT_COOL, OCCUPIED_UNOCCUPIED, HIGH_LOW,
	custom_digital1, custom_digital2, custom_digital3, custom_digital4,
	custom_digital5, custom_digital6, custom_digital7, custom_digital8
	} Digital_units_equate;



typedef enum { not_used_input, Y3K_40_150DegC, Y3K_40_300DegF, R10K_40_120DegC,
	R10K_40_250DegF, R3K_40_150DegC, R3K_40_300DegF, KM10K_40_120DegC,
	KM10K_40_250DegF, A10K_50_110DegC, A10K_60_200DegF, V0_5, I0_100Amps,
	I0_20ma, I0_20psi, N0_2_32counts, N0_3000FPM_0_5V, P0_100_0_5V,
	P0_100_4_20ma, P0_255p_min, table1, table2, table3, table4,
	table5 } Analog_input_range_equate;

typedef enum { not_used,KM_10K,I_4_20ma,V_0_10,V_0_5V,V_0_24AC,TST_Normal} Analog_input_new_range_equate;


typedef enum { not_used_output, V0_10, P0_100_Open, P0_20psi, P0_100,
						P0_100_Close, I_0_20ma }Analog_output_range_equate;



typedef struct
{
	U8_T number;
	U8_T point_type;
}	Point;

typedef struct
	{
		U8_T 	panel_type;
		U8_T	active_panels[4];
		U16_T	desc_length;
		U16_T 	version_number;
		U8_T 	panel_number;
		S8_T 	panel_name[17];
		U16_T	network_number;
		S8_T	network_name[17];
	} Panel_Net_Info;   /* PanelInfo1 */

typedef struct {

/*	Point_Net point_name; /* 5 bytes*/

	U8_T number;
	U8_T point_type;
	U8_T panel;
	S16_T	network_number;
	S32_T point_value;
	U8_T auto_manual;  	 /* 0=auto, 1=manual*/
	U8_T digital_analog;  /* 0=digital, 1=analog*/
	U8_T description_label;  /* 0=display description, 1=display label*/
	U8_T security	  ;  /* 0-3 correspond to 2-5 access level*/
	U8_T decomisioned;  /* 0=normal, 1=point decommissioned*/

	U8_T units ;

} Point_info; 		/*  5+4+1+1=11*/


typedef struct
{
	uint8_t number;
	uint8_t point_type;
	uint8_t panel;
}	Point_T3000;

typedef struct
{
	uint8_t number;
	uint8_t point_type;
	uint8_t panel;
	int16_t	network_number;
}	Point_Net;



typedef struct
{
	int8_t description[21]; 	       /* (21 bytes; string)*/
	int8_t label[9];		       /* (9 bytes; string)*/

	int32_t value;		       /* (4 bytes; int32_t) */

	int8_t auto_manual;  /* (1 bit; 0=auto, 1=manual)*/
	int8_t digital_analog;  /* (1 bit; 0=digital, 1=analog)*/
	int8_t access_level;  /* (3 bits; 0-5)*/
	int8_t control ;  /* (1 bit; 0=off, 1=on)*/
	int8_t digital_control;  /* (1 bit)*/
	int8_t decom;  /* (1 bit; 0=ok, 1=point decommissioned)*/
	int8_t range;	/* (1 Byte ; output_range_equate)*/

	uint8_t m_del_low;  /* (1 uint8_t ; if analog then low)*/
	uint8_t s_del_high; /* (1 uint8_t ; if analog then high)*/
	uint16_t delay_timer;      /* (2 bytes;  seconds,minutes)*/

} Str_out_point;  /* 21+9+4+2+2+2 = 40 */

typedef struct
{

	int8_t description[21]; 	      /* (21 bytes; string)*/
	int8_t label[9];		      	/* (9 bytes; string)*/

	int32_t value;		     						/* (4 bytes; int32_t)*/

	int8_t  filter;  /* (3 bits; 0=1,1=2,2=4,3=8,4=16,5=32, 6=64,7=128,)*/
	int8_t decom;/* (1 bit; 0=ok, 1=point decommissioned)*/
	int8_t sen_on;/* (1 bit)*/
	int8_t sen_off;  /* (1 bit)*/
	int8_t control; /*  (1 bit; 0=OFF, 1=ON)*/
	int8_t auto_manual; /* (1 bit; 0=auto, 1=manual)*/
	int8_t digital_analog ; /* (1 bit; 1=analog, 0=digital)*/
	int8_t calibration_sign; /* (1 bit; sign 0=positiv 1=negative )*/
	int8_t calibration_increment; /* (1 bit;  0=0.1, 1=1.0)*/
	int8_t unused; /* (5 bits - spare )*/

	uint8_t calibration;  /* (8 bits; -25.6 to 25.6 / -256 to 256 )*/

	uint8_t range;	      /* (1 uint8_t ; input_range_equate)*/

} Str_in_point; /* 21+1+4+1+1+9 = 38 */

typedef struct
{
	int8_t description[21];	      /*  (21 bytes; string)*/
	int8_t label[9];		      /*  (9 bytes; string)*/

	int32_t value;		      /*  (4 bytes; float)*/

	uint8_t auto_manual;  /*  (1 bit; 0=auto, 1=manual)*/
	uint8_t digital_analog;  /*  (1 bit; 1=analog, 0=digital)*/
	uint8_t control	;
	uint8_t unused	;
	uint8_t range ; /*  (1 uint8_t ; variable_range_equate)*/

	
}	Str_variable_point; /* 21+9+4+1+1 = 36*/


typedef struct
{
	Point_T3000 input;	        /* (3 bytes; point)*/
	int32_t input_value; 	        /* (4 bytes; int32_t)*/
	int32_t value;		              /* (4 bytes; int32_t)*/
	Point_T3000 setpoint;	      /* (3 bytes; point)*/
	int32_t setpoint_value;	      /* (4 bytes; float)*/
	uint8_t units;    /* (1 uint8_t ; Analog_units_equate)*/

	uint8_t auto_manual; /* (1 bit; 0=auto, 1=manual)*/
	uint8_t action; /* (1 bit; 0=direct, 1=reverse)*/
	uint8_t repeats_per_min; /* (1 bit; 0=repeats/hour,1=repeats/min)*/
	uint8_t unused; /* (1 bit)*/
	uint8_t prop_high; /* (4 bits; high 4 bits of proportional bad)*/

	uint8_t proportional;

	uint8_t reset;	      /* (1 uint8_t ; 0-255)*/
	uint8_t bias;	      /* (1 uint8_t ; 0-100)*/
	uint8_t rate;	      /* (1 uint8_t ; 0-2.00)*/

}	Str_controller_point; /* 3+4+4+3+4+1+1+4 = 24*/

typedef struct {
	int32_t	old_err;
	int32_t 	error_area;
	int32_t 	oi;
	}	Con_aux;

typedef struct
{
	int8_t description[21];		     /* (21 bytes; string)*/
	int8_t label[9];		      	     /*	(9 bytes; string)*/

	uint8_t value ;  /* (1 bit; 0=off, 1=on)*/
	uint8_t auto_manual;  /* (1 bit; 0=auto, 1=manual)*/
	uint8_t override_1_value;  /* (1 bit; 0=off, 1=on)*/
	uint8_t override_2_value;  /* (1 bit; 0=off, 1=on)*/
	uint8_t off  ;
	uint8_t unused	; /* (11 bits)*/

//	Point_T3000 override_1;	     /* (3 bytes; point)*/
//	Point_T3000 override_2;	     /* (3 bytes; point)*/

} Str_weekly_routine_point; /* 21+9+2+3+3 = 38*/


typedef struct
{
	uint8_t	minutes;		/* (1 byte ; 0-59)	*/
	uint8_t	hours; 		/* (1 byte ; 0-23)	*/

} Time_on_off;				/* (size = 2 bytes)	*/

typedef struct
{
	Time_on_off	time[2*MAX_INTERVALS_PER_DAY];

} Wr_one_day;		/* (size = 16 bytes)	*/


typedef struct
{
	int8_t description[21]; 	    /* (21 bytes; string)*/
	int8_t label[9];		      		/* (9 bytes; string)*/

	uint8_t value		;  /* (1 bit; 0=off, 1=on)*/
	uint8_t auto_manual;/* (1 bit; 0=auto, 1=manual)*/
//	unsigned unused				: 14; 	/* ( 12 bits)*/
	uint8_t unused;

}	Str_annual_routine_point;   /* 21+9+2=32 bytes*/

#if 1
typedef struct
{
	int8_t description[21]; 	      	  /* (21 bytes; string)*/
	int8_t label[9];			  /* (9 bytes; string)*/  
	uint16_t bytes;		/* (2 bytes; size in bytes of program)*/ 
	uint8_t on_off;	//	      : 1; /* (1 bit; 0=off; 1=on)*/
	uint8_t auto_manual;//	  : 1; /* (1 bit; 0=auto; 1=manual)*/
	uint8_t com_prg;	//	    : 1; /* (6 bits; 0=normal use, 1=com program)*/
	uint8_t errcode;	//      : 5; /* (6 bits; 0=normal end, 1=too int32_t in program)*/
	uint8_t unused;   //      : 8;

} Str_program_point;	  /* 21+9+2+2 = 34 bytes*/
 #endif

typedef struct {
	S8_T *address;
	U8_T size;
	U8_T max_points;
	U8_T label;
	U8_T desc;
	} Info_Table;



typedef union {
		Str_out_point             *pout;
		Str_in_point 			  *pin;
	//	In_aux 					  *pinx;
		Str_variable_point 		  *pvar;
		Str_controller_point 	  *pcon;
		Str_weekly_routine_point  *pwr;
		Str_annual_routine_point  *panr;
		Str_program_point 		  *pprg;
	/*	Str_array_point 		  *pary;
		Str_monitor_point		  *pmon;
	    Str_totalizer_point       *ptot;
	    Monitor_Block             *pmb;
	    Mon_aux                   *pmaux;
		Control_group_point       *pgrp;
		Aux_group_point           *pgaux;
		Str_grp_element           *pgrel;
		Alarm_point               *palrm;
		MiniInfo2 				  *pmi2;
		MiniCommInfo			  *pmci;
		Point_Net		          *pnet;
		Point_info                *ppinf;
		System_Name_Number        *psnn;
		Program_remote_points     *pprp;
		Info_Table                *pinf;
		REMOTE_POINTS             *prp;
		NETWORK_POINTS            *pnp;
		WANT_POINTS               *pwp;*/
	 } Str_points_ptr;


typedef struct
{
	 uint16_t  		total_length;        /*	total length to be received or sent	*/
	 uint8_t		command;
	 uint8_t		point_start_instance;
	 uint8_t		point_end_instance;
	 uint8_t		entitysize;

}Str_user_data_header;














#endif