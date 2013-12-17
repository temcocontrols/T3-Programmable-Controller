#ifndef _MONITOR_H
#define _MONITOR_H				/* avoid recompilation */

//#include "bacnet.h"

#define SELECT 1
#define SCAN   2


#define ANALOGDATA         1
#define DIGITALDATA        2
#define DIGITALBUFSIZE     3
#define ANALOGBUFSIZE      4
#define ANALOG_TIMES_SIZE  5
#define DIGITAL_TIMES_SIZE 6



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

typedef struct {

  Byte first_block;
  Byte last_block;
  int  start_index;
  int  end_index;
  char *last_data;
  Byte header;
  Byte dat;

} MonitorDataPara;

typedef union {


  MonitorUpdateData montimes;
  MonitorDataPara   mondata;
  char              string[16];

} Ret_Param;

typedef struct
{
	Byte         index;		   // monitor table index
	Byte		 sample_type;	//  
	Comm_Args    comm_arg;            /* command's arguments */
	Ret_Param    para;                /* used to return parameters */
	Byte		command;
	Byte	     special;

	char        asdu[500];
//	char 		*dat; 
	Uint         length_asdu;
	Uint 		entitysize;
	Uint  		length;  
} Mon_Data;


void update_blocks_number( Monitor_Block *block_ptr );

/*	int find_new_block( int monitor_no );*/
int find_new_block( int monitor_no, int ana_dig ); /* 0=analogic, 1=digital */

void init_new_analog_block( int mon_number, Str_monitor_point *mon_ptr,
                               Monitor_Block *block_ptr );
void get_new_analog_block( int mon_number, int init,
                            Str_monitor_point *mon_ptr  );
void init_new_digital_block( int mon_number, Str_monitor_point *mon_ptr,
                               Monitor_Block *block_ptr );
void get_new_digital_block( int mon_number, int init,
                            Str_monitor_point *mon_ptr  );
Byte get_input_sample( int number );

void sample_analog_points( Str_monitor_point *mon_ptr,
                                      Mon_aux  *aux_ptr );
void sample_digital_points( Str_monitor_point *mon_ptr,
                                        Mon_aux *aux_ptr );
void sample_points( void );

Ulong get_time_of_last_sample( Monitor_Block *ptr );

Ulong get_sample_time( Monitor_Block *ptr );

Ulong get_time_at_index( Monitor_Block *ptr, int index );

void send_monitor_frame( Mon_Data *PTRtable, Monitor_Block *ptr );

int get_data_size_from_time( Monitor_Block *ptr, Ulong time, int *position );

int get_data_size_till_time( Monitor_Block *ptr, Ulong time, int *position );

int get_sample_size( Monitor_Block *ptr );

int get_data_size_from_index( Monitor_Block *ptr, int index );

int get_data_size_till_index( Monitor_Block *ptr, int index );

void scan_monitor_blocks( Mon_Data *PTRtable, Monitor_Block *ptr,int max_blocks, int order );
			  
void ReadMonitor( Mon_Data *PTRtable); 
void UpdateMonitor( Mon_Data *PTRtable);

void dealwithMonitor(Str_user_data_header header);

#endif
