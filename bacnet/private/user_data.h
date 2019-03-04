#ifndef USER_DATA_H
#define USER_DATA_H

#include "ud_str.h"
#include "monitor.h"
#include "dyndns_app.h"
#include "datetime.h"

typedef struct
{
	uint8 harware_rev;
	uint16 firmware_asix;	// ASIX
	uint8 frimware_pic;    // PIC
	uint8 firmware_c8051;	// C8051
	uint8 frimware_sm5964;	// SM5964
	uint8 bootloader_rev;

	uint8 no_used[10];
}Str_Pro_Info;


//#define ID_SIZE 19
//typedef	union
//{
//	uint8 all[ID_SIZE];
//	struct
//	{
//		uint8 id;		
//		uint8 on_line; // 0: offline    1: online
//		uint8 schedule;	
//		uint8 flag;
//		char name[15];
///*  flag formate:
//  7   6  5  4  3   2   1  0
// a/m  output state1 -  -  -  -  -
// */
//	}Str;
//}UN_ID; /*config roution */


#define STORE_ID_LEN	15
typedef	union
{
 int8_t all[39];
 struct
 {
  uint8 id;
  uint8 schedule;
  uint8 flag;
	uint16_t reserved_reg[6];
	 
  uint8 on_line; // 0: offline    1: online
  char name[15];
  uint16_t daysetpoint;
  uint16_t nightsetpoint;
  uint16_t awaysetpoint;
  uint16_t sleepsetpoint;
  
  /*  flag formate:
  7   6  5  4  3   2   1  0
  a/m  output state1 -  -  -  -  -
  */
 }Str;
}UN_ID; /*config roution */


typedef	union
	{
		U8_T all[10];
		struct 
		{
			U8_T sec;				/* 0-59	*/
			U8_T min;    		/* 0-59	*/
			U8_T hour;      		/* 0-23	*/
			U8_T day;       		/* 1-31	*/
			U8_T week;  		/* 0-6, 0=Sunday	*/
			U8_T mon;     		/* 1-12	*/
			U8_T year;      		/* 0-99	*/
			U16_T day_of_year; 	/* 0-365	*/
			S8_T is_dst;        /* daylight saving time on / off */		
				
		}Clk;
		struct
    {
        U32_T timestamp;
        S8_T time_zone;
        U8_T daylight_saving_time;
        U8_T reserved[3];
    }NEW;
	}UN_Time;

	

	
typedef	union
{
	uint8_t all[400];
	struct 
	{
	 uint8_t ip_addr[4];
	 uint8_t subnet[4];
	 uint8_t getway[4];		 
	 uint8_t mac_addr[6];

	 uint8_t tcp_type;   /* 1 -- DHCP, 0 -- STATIC */

	 uint8_t mini_type;
	 uint8_t debug;

	 Str_Pro_Info pro_info;
	 uint8_t com_config[3];
	 uint8_t refresh_flash_timer;
	 uint8_t en_plug_n_play;

	 uint8_t reset_default;	  // write 88
	 uint8_t com_baudrate[3]; 

	 uint8_t  en_username;
	 uint8_t  cus_unit;

	 uint8_t usb_mode; 
	 uint8_t network_number;
	 uint8_t panel_type;

	 S8_T panel_name[20];
	 uint8_t en_panel_name;
	 uint8_t panel_number;
	 
	uint8_t dyndns_user[MAX_USERNAME_SIZE];
	uint8_t dyndns_pass[MAX_PASSWORD_SIZE];
	uint8_t dyndns_domain[MAX_DOMAIN_SIZE];
	uint8_t en_dyndns;  			// 0 - no  1 - disable 2 - enable
	uint8_t dyndns_provider;  // 0- www.3322.org 1-www.dyndns.com  2 - www.no-ip.com 3 - temco server
	uint16_t dyndns_update_time;  // xx min
	uint8_t en_sntp;		// 0 - no  1 - disable 2 - timeserver1, 3 - timeserver2 , 4 - timeserver3 5 - customer timersverver  - 200 local PC
	S16_T time_zone;	
	uint32_t sn;
	
	UN_Time update_dyndns; 
	uint16_t mstp_network_number;
	uint8_t BBMD_EN;
	uint8_t sd_exist;
	
	uint16_t tcp_port;
	uint8_t modbus_id;
	uint32_t instance;
	
	uint32_t update_sntp_last_time;
	uint8_t Daylight_Saving_Time;
	
	S8_T sntp_server[30];
	U8_T zigbee_exist;  // 0x74 - exist 
	
	U8_T backlight;
	U8_T update_time_sync_pc;   //  0 - finished  1 - ask updating 
	U8_T en_time_sync_with_pc;  // 0 - time server 1 - pc
	U8_T sync_with_ntp_result; // 0 - fail , 1 - ok
	
//	U8_T network_ID[3];
	U8_T MSTP_ID;
	U16_T zigbee_module_id;
	U8_T MAX_MASTER;   
	}reg;
}Str_Setting_Info;

typedef	union
{
	uint8_t all[400];
	struct 
	{ 
	 uint16_t flag; // 0x55ff
	 uint32_t monitor_block_num[24];
	 uint8_t operate_time[12][4];	
		
	 uint8_t flag1; // network health  0x55
	 uint32_t com_rx[3];
	 uint32_t com_tx[3];	
	 uint16_t collision[3];  // id collision
	 uint16_t packet_error[3];  // bautrate not match
	 uint16_t timeout[3];
	}reg;
}Str_MISC;

typedef union
{
		uint8_t all[30];
		struct
		{
			uint8_t product_id;
			uint8_t port;
			uint8_t modbus_id;
			uint32_t last_contact_time;
			uint8_t input_start;
			uint8_t input_end;
			uint8_t output_start;
			uint8_t output_end;
			uint32_t sn;
			S8_T reserved_reg[15];
		}reg;
}Str_Extio_point;



#define PIC_PACKET_STACK 50
#define PIC_PACKET_LEN  400
typedef struct
{
	U8_T flag;
	U8_T buf[PIC_PACKET_LEN];
	U8_T retry;
	U16_T index;
	U8_T file; // which picture
}STR_STORE_PIC;


typedef enum { WRITE_SD_OK = 0,WAIT_FOR_SD} E_OPERATE_SD;



#define USER_DATA_HEADER_LEN 7

#define MAX_RETRY_SEND_BIP 5

extern U32_T	timeCount1,timeCount2;
//extern uint8_t invokeid_bip;
extern uint8_t far invokeid_mstp;
extern uint8_t far flag_receive_netp;	// network points 
extern uint8_t far flag_receive_netp_temcovar;
extern uint8_t far temcovar_panel;
extern uint8_t far temcovar_panel_invoke;
extern uint8_t far flag_receive_netp_modbus;	// network points 
extern uint8_t far flag_receive_rmbp;  // remote bacnet points
extern bool Send_Whois_Flag;
extern U8_T Send_bip_Flag;
extern U8_T Send_Time_Sync;
extern U8_T far count_send_bip;
//extern U8_T count_bip_connect;
extern U8_T far Send_bip_address[6];
extern U8_T far Send_bip_count;
//extern U8_T flag_get_network_point;
extern U8_T remote_modbus_index;	
extern U8_T remote_bacnet_index;
extern U8_T far network_point_index;
extern U8_T remote_mstp_panel_index;

extern uint8_t Send_Private_Flag;
extern uint16_t count_Private;
extern bool Send_I_Am_Flag;
extern bool Send_Read_Property;
extern U8_T far MSTP_Send_buffer[600];
extern U16_T MSTP_buffer_len;
extern STR_MSTP_REV_HEADER far MSTP_Rec_Header;
extern U8_T far MSTP_Rec_buffer[600];
extern S8_T   just_load;

extern FRAME_ENTRY 								 SendFrame[MAX_SEND_FRAMES];
extern STR_PTP  Ptp_para;
//extern UNITDATA_PARAMETERS  		NL_PARAMETERS;
//extern Routing_Table                Routing_table[MAX_Routing_table];

extern U8_T flag_Updata_Clock;
extern U32_T far update_sntp_last_time;
extern uint8_t far BACnet_Port;
extern U32_T  far	Instance;
extern U8_T far boot;
//extern U8_T far flag_E2_changed;
//extern U32_T far changed_index;
//extern U32_T far changed_index2;

//extern Str_Remote_TstDB    far        Remote_tst_db[SUB_NO];
extern Str_Panel_Info 	   far 		Panel_Info;
extern Str_Setting_Info    far 		Setting_Info;
extern Str_MISC  						far 	MISC_Info;
extern Str_Special far Write_Special;

extern Str_in_point far inputs[MAX_INS];
extern Str_out_point far	outputs[MAX_OUTS];
//extern Str_out_point  far	*outputs;
extern uint8_t				far				 no_outs;
//extern Str_in_point   far	*inputs;
extern uint8_t				far				 no_ins;

extern Info_Table			far						 info[18];

extern U8_T far month_length[12];
extern U32_T 				timestart;	   /* seconds since the beginning of the year */
extern U32_T 				time_since_1970;   /* seconds since the beginning of 2010 */
extern In_aux					far						 in_aux[MAX_IO_POINTS];
extern Con_aux				far							 con_aux[MAX_CONS];
//extern Mon_aux           far           mon_aux[MAX_MONITORS];
extern Monitor_Block		far			mon_block[2 * MAX_MONITORS];
//extern S8_T 				far         mon_data_buf[sizeof(Monitor_Block) * 2 * MAX_MONITORS];
extern Mon_Data 			far 		*Graphi_data;
extern S8_T 				far			Garphi_data_buf[sizeof(Mon_Data)];

extern S8_T  far var_unit[MAX_VAR_UNIT][VAR_UNIT_SIZE];
extern Str_Extio_point far extio_points[MAX_EXTIO];

//extern S16_T                          MAX_MONITOR_BLOCKS;
//extern U8_T           far              free_mon_blocks;

 
extern S8_T far panelname[20];
//extern U8_T 	client_ip[4];
//extern U8_T newsocket;

extern Alarm_point 		    far				 alarms[MAX_ALARMS];
extern U8_T 			    far							 ind_alarms;
extern Alarm_set_point 		far    			 alarms_set[MAX_ALARMS_SET];
extern U8_T 			    far							 ind_alarms_set;
extern U16_T                 alarm_id;
extern S8_T                         new_alarm_flag;

extern Units_element		    far				 digi_units[MAX_DIG_UNIT];
extern U8_T 					far 		ind_passwords;
extern Password_point			far			passwords[ MAX_PASSW ];

extern Str_variable_point		far		 vars[MAX_VARS + 12];
extern Str_controller_point 	far			 controllers[MAX_CONS];
extern Str_totalizer_point      far     totalizers[MAX_TOTALIZERS];
extern Str_monitor_point		far				 monitors[MAX_MONITORS];   
extern Str_monitor_point		far			backup_monitors[MAX_MONITORS]/* _at_ 0x12800*/;	
//extern Aux_group_point        	far 		 aux_groups[MAX_GRPS];
//extern S8_T                    far		 Icon_names[MAX_ICONS][14];
extern Control_group_point  	far 		 control_groups[MAX_GRPS];
extern Str_grp_element			far	    		 group_data[MAX_ELEMENTS];
extern S16_T 					far							 total_elements;
extern S16_T 					far							 group_data_length;

//extern U32_T 				far 		SD_lenght[MAX_MONITORS * 2];
extern U32_T 				far 		SD_block_num[MAX_MONITORS * 2];

extern Str_mon_element     far     read_mon_point_buf[MAX_MON_POINT];
extern Str_mon_element     far     write_mon_point_buf[MAX_MONITORS * 2][MAX_MON_POINT];

extern Str_weekly_routine_point far		 weekly_routines[MAX_WR] ;
extern Wr_one_day				far		wr_times[MAX_WR][MAX_SCHEDULES_PER_WEEK];
extern Str_annual_routine_point	far	 annual_routines[MAX_AR];
extern U8_T                   far      ar_dates[MAX_AR][AR_DATES_SIZE];	
extern U8_T	far  wr_time_on_off[MAX_WR][MAX_SCHEDULES_PER_WEEK][8];
 /* Assume bit0 from octet0 = Jan 1st */
extern Str_program_point	    far			 programs[MAX_PRGS];
extern S8_T 			    	far			*program_address[MAX_PRGS]; /*pointer to code*/
extern U8_T    	    	far				prg_code[MAX_PRGS][MAX_CODE * CODE_ELEMENT];
extern U16_T			far	 			Code_len[MAX_PRGS];
extern U16_T 			far				Code_total_length;
//extern Str_array_point 	    far			 arrays[MAX_ARRAYS];
extern S32_T  			    				*arrays_address[MAX_ARRAYS];
extern Str_table_point			far				 custom_tab[MAX_TBLS];
extern U16_T                far         PRG_crc;
extern U8_T far *prog;
extern S32_T far stack[20];
extern S32_T far *index_stack;
extern S8_T far *time_buf;
extern U32_T far cond;
extern S32_T far v, value;
extern S32_T far op1,op2;
extern S32_T far n,*pn;

extern S8_T far message[ALARM_MESSAGE_SIZE+26+10];
extern U8_T alarm_flag;
extern S8_T alarm_at_all;
extern S8_T ind_alarm_panel;
extern S8_T alarm_panel[5];
extern U16_T alarm_index;
extern U32_T      far                 miliseclast_cur;
extern U32_T      far                 miliseclast;


extern POINTS_HEADER			far      points_header[MAXREMOTEPOINTS];


extern NETWORK_POINTS        far  		 network_points_list_bacnet[MAXNETWORKPOINTS];	 /* points wanted by others */
extern Byte                 far			 number_of_network_points_bacnet; 

extern NETWORK_POINTS        far  		 network_points_list_modbus[MAXNETWORKPOINTS];	 /* points wanted by others */
extern Byte                 far			 number_of_network_points_modbus;
//extern U8_T far NT_bacnet_tb_func[MAXNETWORKPOINTS];
//extern STR_BAC_TB far NT_bacnet_tb[MAXNETWORKPOINTS];

extern REMOTE_POINTS		  far  		 remote_points_list_modbus[MAXREMOTEPOINTS];  /* points from other panels used localy */
//extern STR_SCAN_TB far RP_modbus_tb[MAXREMOTEPOINTS]; 
//extern U8_T far RP_modbus_tb_func[MAXREMOTEPOINTS];
extern Byte                far			 number_of_remote_points_modbus;

extern REMOTE_POINTS		  far  		 remote_points_list_bacnet[MAXREMOTEPOINTS];  /* points from other panels used localy */
//extern U8_T far RP_bacnet_tb_func[MAXREMOTEPOINTS];
//extern STR_BAC_TB far RP_bacnet_tb[MAXREMOTEPOINTS];
extern Byte                far			 number_of_remote_points_bacnet;

extern U8_T remote_panel_num;

#define MAX_REMOTE_PANEL_NUMBER 30
extern STR_REMOTE_PANEL_DB far remote_panel_db[MAX_REMOTE_PANEL_NUMBER];

extern BACNET_DATE Local_Date;
extern BACNET_TIME Local_Time;

extern Byte	 Station_NUM;
extern Byte  MAX_MASTER;
extern 	U8_T panel_number;


//extern U8_T far flag_Moniter_changed;
//extern U8_T count_monitor_changed;
//extern U8_T far table_bank[TABLE_BANK_LENGTH];
extern U8_T const code table_bank[TABLE_BANK_LENGTH];

extern STR_STORE_PIC far store_pic[PIC_PACKET_STACK];

void init_panel(void);



void update_timers( void );



#define DEFUATL_REMOTE_NETWORK_VALUE 0x55555555

#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
extern UN_ID far ID_Config[254];
extern U8_T far ID_Config_Sche[254];
#endif

extern U8_T far output_pri_live[MAX_OUTS];
extern float far output_priority[MAX_OUTS][16];
extern float far output_relinquish[MAX_OUTS];

S16_T swap_word( S16_T dat );	//	  mGetPointWord2
U16_T convert_pointer_to_word( U8_T *iAddr );  //	 mGetPointWord
U32_T convert_pointer_to_double( U8_T *iAddr ) ;  // DoulbemGetPointWord
S32_T swap_double( S32_T dat ) ;  //DoulbemGetPointWord2
void Initial_Panel_Info(void);


U8_T check_remote_point_list(Point_Net *point,U8_T *index, U8_T protocal);

void put_remote_point_value( S16_T index, S32_T *val_ptr, S16_T prog_op , uint8_t protocal);
void add_remote_point(U8_T id,U8_T point_type,U8_T high_5bit,U8_T number,S32_T val_ptr,U8_T specail);
void put_network_point_value( S16_T index, S32_T *val_ptr, S16_T prog_op );
void add_network_point(U8_T panel,U8_T id,U8_T point_type,U8_T number,S32_T val_ptr,U8_T specail);

void change_panel_number_in_code(U8_T old, U8_T new_panel);

void check_graphic_element(void);
void check_weekly_routines(void);
void check_annual_routines(void);
void check_output_priority_array(U8_T i);
void check_output_priority_HOA(U8_T i);
void check_output_priority_array_AM(U8_T i);

void push_expansion_out_stack(Str_out_point* ptr,uint8 point,uint8_t type);
void push_expansion_in_stack(Str_in_point* ptr);

U32_T get_current_time(void);


int GetPrivateBacnetToModbusData(uint32_t deviceid, uint16_t start_reg, int16_t readlength, uint16_t *data_out);
int WritePrivateBacnetToModbusData(uint32_t deviceid, int16_t start_reg, uint16_t writelength, uint32_t data_in);

#endif

