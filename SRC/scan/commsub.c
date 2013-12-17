#include "main.h"
#include "serial.h"
#include "commsub.h"
#include "schedule.h"
#include "scan.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

xSemaphoreHandle sem_subnet_tx_uart0;


STR_MAP_ID_PORT far map_id_port[254];

U8_T far tst_addr_index[2] = {0 , 0};
U8_T far tst_reg_index[2] = {READ_PRODUCT_MODLE , READ_PRODUCT_MODLE};	

S8_T far uart0_sub_addr[8];
S8_T far uart2_sub_addr[8];
U8_T far uart0_sub_no;
U8_T far uart2_sub_no;


TST_INFO far tst_info[SUB_NO] _at_ 0x44000;



U8_T far  WRT_Tst_Reg = 0;
U8_T far  WRT_Tst_ID = 0;

U8_T scan_status = NONE_ID;



U8_T sub_source_port = 0;

const U16_T Tst_Register[Tst_reg_num][3] = 	// 0 : tstat5ABCDEFG  1: tstat6/tstat7	  2:TST5EH  
{
  	{7,7,7},	   	 // TSTAT_PRODUCT_MODEL
	{184,109,184},	 //	TSTAT_OCCUPIED
	{380,345,522},	 // TSTAT_COOL_SETPOINT
	{136,639,136},	 // TSTAT_HEAT_SETPOINT
	{135,638,135},
	{101,121,101},
	{107,102,107},
	{108,209,108},
	{123,352,123},
	{124,353,124},
	{182,354,182},
	{183,355,183},
	{6,6,6},
	{211,111,211},
	{0,0,0},
	{1,1,1},
	{2,2,2},
	{3,3,3}
}; 

//BinSearch xdata binsearch_Table[511];  // 129 is tested, the max time

U8_T far  test1;
U8_T far  test_index = 0;
extern U8_T  ttt[50];
// if the current search item is i, then the next search item is (2i + 1 , 2(i + 1))
void Comm_Tstat_Initial_Data(void)
{
	U8_T i;
	memset(tst_info,0,sizeof(TST_INFO) * SUB_NO);

//	memset(schedule_data,0,254);

	for(i = 0;i < SUB_NO;i++)
		tst_info[i].type = 0xff;

	memset(map_id_port,0,254 * sizeof(STR_MAP_ID_PORT));

	memset(uart0_sub_addr,0,8);
	memset(uart2_sub_addr,0,8);
	memset(sub_addr,0,8);

	vSemaphoreCreateBinary(sem_subnet_tx_uart0);

//	sub_no = 2;
//	uart0_sub_no = 1;
//	uart2_sub_no = 1;
//	uart0_sub_addr[0] = 210;
//	uart2_sub_addr[0] = 250;
//	sub_addr[0] = 253;
//	sub_addr[1] = 20; 
}


