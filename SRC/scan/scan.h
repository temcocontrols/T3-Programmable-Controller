#ifndef	_SCAN_H
#define	_SCAN_H

#include "types.h"

void calculate_ID_table(void);
void Send_Scan_Cmd(U8_T max, U8_T min);
void update_tstat_list(U8_T tstat_id);
void read_sn_for_assignment_id(U8_T address);  
void assignment_id_with_sn(U8_T address, U8_T new_address,unsigned long current_sn); 
void check_on_line(void);	
char binarySearchforComDevice(unsigned char maxaddr,unsigned char minaddr);//reentrant

void Scan_Sub_ID(void);

#endif
