#ifndef _MONITOR_H
#define _MONITOR_H				/* avoid recompilation */

#include "ud_str.h"

#define SELECT 1
#define SCAN   2


#define ANALOGDATA         1
#define DIGITALDATA        2
#define DIGITALBUFSIZE     3
#define ANALOGBUFSIZE      4
#define ANALOG_TIMES_SIZE  5
#define DIGITAL_TIMES_SIZE 6


#define NET_HEALTH_INDEX  11
#define NET_HEALTH_VAR_BASE 128




typedef struct {

	long max_length;
  	long last_time;

} MonitorDataArgs;

typedef struct {

	long oldest_time;
  	long most_recent_time;

} MonitorSizeArgs;

typedef struct {

  long size;
  long oldest_time;
  long most_recent_time;

} MonitorUpdateData;


typedef union {

	MonitorDataArgs   mondata;
	MonitorSizeArgs   monsize;
  	MonitorUpdateData monupdate;
  	char              string[12];

} Comm_Args;

//typedef struct {

//  Byte first_block;
//  Byte last_block;
//  int  start_index;
//  int  end_index;
//  char *last_data;
//  Byte header;
//  Byte dat;

//} MonitorDataPara;

//typedef union {


//  MonitorUpdateData montimes;
//  MonitorDataPara   mondata;
//  char              string[16];

//} Ret_Param;

typedef struct
{
	U16_T		noused;   // new  
	Byte		command;
	Byte    index;		   // monitor table index
	Byte		sample_type;	//  
	Comm_Args    comm_arg;            /* command's arguments */
//	Ret_Param    para;                /* used to return parameters */

	Byte	  special; 
	U32_T		seg_index;	 // new 
	U32_T		total_seg;   // new  
	char        asdu[450];
 
} Mon_Data;


//void update_blocks_number( Monitor_Block *block_ptr );

/*	int find_new_block( int monitor_no );*/
int find_new_block( int monitor_no, int ana_dig ); /* 0=analogic, 1=digital */

//void init_new_analog_block( int mon_number, Str_monitor_point *mon_ptr/*, Monitor_Block *block_ptr*/ );
void get_new_analog_block( int mon_number, int init/*, Str_monitor_point *mon_ptr */ );
//void init_new_digital_block( int mon_number, Str_monitor_point *mon_ptr/*, Monitor_Block *block_ptr */);
//void get_new_digital_block( int mon_number, int init, Str_monitor_point *mon_ptr  );
U32_T get_input_sample( int number );

//void sample_analog_points( char i, Str_monitor_point *mon_ptr/*,Mon_aux  *aux_ptr */);
//void sample_digital_points(char i, Str_monitor_point *mon_ptr/*, Mon_aux *aux_ptr */);
void sample_points( void );
void check_trendlog_1s(unsigned char count);
void ReadPicture( Mon_Data *PTRtable);


U8_T ReadMonitor( Mon_Data *PTRtable); 
void UpdateMonitor( Mon_Data *PTRtable);

void dealwithMonitor(uint8_t bank);
void monitor_init(void);
void check_whehter_reading_sd(void);

#endif
