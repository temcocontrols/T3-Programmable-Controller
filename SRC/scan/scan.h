

#ifndef	_SCAN_H_

#define	_SCAN_H_

#include "types.h"

typedef struct _SCAN_DATABASE_
{
	U8_T id;
	U32_T sn;
	U8_T port;
} SCAN_DB;

//#define MAX_ID		255

#define	NONE_ID						0
#define	UNIQUE_ID					1
#define	MULTIPLE_ID					2
#define	UNIQUE_ID_FROM_MULTIPLE		3
#define	ASSIGN_ID					4
#define SCAN_BUSY					0xff


#define	SCAN_BINSEARCH			1
#define	SCAN_ASSIGN_ID_WITH_SN	2

extern SCAN_DB xdata current_db;
extern SCAN_DB xdata scan_db[255];
extern U8_T db_ctr;
extern U8_T xdata db_online[32], db_occupy[32], get_para[32];
extern U8_T xdata current_online[32];
extern U8_T current_online_ctr;
extern U8_T external_nodes_plug_and_play;
extern U8_T reset_scan_db_flag;

//void check_write_to_nodes(void);
//void get_parameters_from_nodes(void);
void write_parameters_to_nodes(U8_T index, U16_T reg, U16_T value);


U8_T check_master_id_in_database(U8_T set_id, U8_T increase) reentrant;
void vStartScanTask(unsigned char uxPriority);
void modify_master_id_in_database(U8_T old_id, U8_T set_id);
void remove_id_from_db(U8_T index_of_scan_db);

void Response_MAIN_To_SUB(U8_T *buf, U16_T len);

#endif