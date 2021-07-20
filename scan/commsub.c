#include "main.h"
#include "serial.h"
#include "commsub.h"
//#include "schedule.h"
#include "scan.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

U8_T far uart0_sub_addr[SUB_NO];// _at_ 0x41600;
U8_T far sub_no;
U8_T far online_no;	  
U8_T far uart0_sub_no;



U8_T far uart1_sub_no;
U8_T far uart2_sub_no;
U8_T far uart1_sub_addr[SUB_NO];// _at_ 0x41700;
U8_T far uart2_sub_addr[SUB_NO];// _at_ 0x41800;


STR_MAP_table far sub_map[SUB_NO];

S16_T get_net_point_value( Point_Net *p, S32_T *val_ptr , U8_T mode,U8_T flag );

#if BAC_COMMON
U16_T far sub_info_AI[SUB_NO][32];
U8_T far sub_info_DI[SUB_NO][5];
U16_T far sub_info_AO[SUB_NO][12];
U16_T far sub_info_DO[SUB_NO][13];
//U8_T far sub_info_Range[SUB_NO][32];
#endif



// BinSearch xdata binsearch_Table[511];  // 129 is tested, the max time
#if (ASIX_MINI || ASIX_CM5)
extern xSemaphoreHandle sem_subnet_tx_uart0;
extern xSemaphoreHandle sem_subnet_tx_uart1;
extern xSemaphoreHandle sem_subnet_tx_uart2;
#endif

U8_T base_in;
U8_T base_out;
U8_T base_var;

// if the current search item is i, then the next search item is (2i + 1 , 2(i + 1))
void Comm_Tstat_Initial_Data(void)
{
	if((Modbus.mini_type == MINI_BIG) || (Modbus.mini_type == MINI_BIG_ARM))	{	base_in = 32;		base_out = 24;   }
	else if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM))	{	base_in = 16;		base_out = 10;	}
	else if(Modbus.mini_type == MINI_TINY)	{	base_in = 11;		base_out = 8;	}
	else if(Modbus.mini_type == MINI_VAV)	{	base_in = 6;		base_out = 3;	}
	else if(Modbus.mini_type == MINI_CM5) {	base_in = 24;		base_out = 10;}
	else if((Modbus.mini_type == MINI_NEW_TINY) || (Modbus.mini_type == MINI_TINY_ARM)) {	base_in = 8;		base_out = 14;}
	else if(Modbus.mini_type == MINI_TINY_11I) 	{	base_in = 11;		base_out = 11;}
	else if(Modbus.mini_type == MINI_TSTAT10) {base_in = TSTAT10_MAX_AIS;		base_out = TSTAT10_MAX_DOS + TSTAT10_MAX_AOS;}
	else if(Modbus.mini_type == MINI_T10P) {base_in = T10P_MAX_AIS;		base_out = T10P_MAX_DOS + T10P_MAX_AOS;}
	else if(Modbus.mini_type == MINI_NANO) {base_in = 0;		base_out = 0;}
	base_var = 0;

	memset(sub_map,0,sizeof(STR_MAP_table) * SUB_NO);
//	memset(RP_modbus_tb,0,sizeof(STR_SCAN_TB) * MAXREMOTEPOINTS);


}



#if  T3_MAP
// only MAP T3
void remap_table(U8_T index,U8_T type)
{
	Point_Net point;
	S32_T value;
	U8_T i,j;
	U16_T reg;

	if(sub_map[index].add_in_map == 0)
	{
		if(type == PM_T3IOA)
		{
			for(i = 0;i < 8;i++)  // 8 ai
			{  
				reg = T3_8AO_AI_REG_START + i;
				point.number = LOW_BYTE(reg);
				point.point_type = (HIGH_BYTE(reg) << 5) + VAR + 1;
				point.panel = panel_number;//Modbus.network_ID[port];
				point.sub_id = scan_db[index].id;
				point.network_number = 0;//Setting_Info.reg.network_number;										
				get_net_point_value(&point,&value, 0 , 0); 				
			}
			
			for(i = 0;i < 8;i++)  // 8 ao
			{ 
				reg = T3_8AO_AO_REG_START + i;
			  point.number = LOW_BYTE(reg);
				point.point_type = (HIGH_BYTE(reg) << 5) + VAR + 1;
				point.panel = panel_number;//Modbus.network_ID[port];
				point.sub_id = scan_db[index].id;
				point.network_number = 0;//Setting_Info.reg.network_number;										
				get_net_point_value(&point,&value ,0 , 0);
			}
		
			sub_map[index].do_len = 0;
			sub_map[index].ao_len = 8;
			sub_map[index].ai_len = 8;
		}	
		else if(type == PM_T38I13O)
		{
			for(i = 0;i < 8;i++)  // 8 ai
			{  
				reg = T3_8A13O_AI_REG_START + i * 2;				
				point.number = LOW_BYTE(reg);
				point.point_type = (HIGH_BYTE(reg) << 5) + VAR + 1;
				point.panel = panel_number;//Modbus.network_ID[port];
				point.sub_id = scan_db[index].id;
				point.network_number = 0;//Setting_Info.reg.network_number;	
				get_net_point_value(&point,&value ,0, 0 ); 
				
			}
			for(i = 0;i < 13;i++)  // 13 do
			{  
				reg = T3_8A13O_DO_REG_START + i;
				point.number = LOW_BYTE(reg);
				point.point_type = (HIGH_BYTE(reg) << 5) + VAR + 1;
				point.panel = panel_number;//Modbus.network_ID[port];
				point.sub_id = scan_db[index].id;
				point.network_number = 0;//Setting_Info.reg.network_number;	
				
				get_net_point_value(&point,&value ,0, 0 ); 
			}			
			
			sub_map[index].do_len = 13;
			sub_map[index].ao_len = 0;
			sub_map[index].ai_len = 8;
		}
		else if(type == PM_T34AO)
		{
			for(i = 0;i < 8;i++)  // 8 do
			{  
				reg = T3_4AO_DO_REG_START + i;
				point.number = LOW_BYTE(reg);
				point.point_type = (HIGH_BYTE(reg) << 5) + VAR + 1;
				point.panel = panel_number;//Modbus.network_ID[port];
				point.sub_id = scan_db[index].id;
				point.network_number = 0;//Setting_Info.reg.network_number;	
				get_net_point_value(&point,&value , 0 , 0); 	
			}
			for(i = 0;i < 4;i++)  // 4 ao
			{ 
				reg = T3_4AO_AO_REG_START + i;
				point.number = LOW_BYTE(reg);
				point.point_type = (HIGH_BYTE(reg) << 5) + VAR + 1;
				point.panel = panel_number;//Modbus.network_ID[port];
				point.sub_id = scan_db[index].id;
				point.network_number = 0;//Setting_Info.reg.network_number;	
				get_net_point_value(&point,&value , 0 , 0); 	
			}
			for(i = 0;i < 10;i++)  // 10 ai
			{  
				reg = T3_4AO_AI_REG_START + i;
				point.number = LOW_BYTE(reg);
				point.point_type = (HIGH_BYTE(reg) << 5) + VAR + 1;
				point.panel = panel_number;//Modbus.network_ID[port];
				point.sub_id = scan_db[index].id;
				point.network_number = 0;//Setting_Info.reg.network_number;	
				get_net_point_value(&point,&value , 0 , 0); 
			}
			
			sub_map[index].do_len = 8;
			sub_map[index].ao_len = 4;
			sub_map[index].ai_len = 10;
		}
		else if(type == PM_T332AI)
		{
			for(i = 0;i < 32;i++)  // 32 ai
			{  
				reg = T3_32I_AI_REG_START + i;
				point.number = LOW_BYTE(reg);
				point.point_type = (HIGH_BYTE(reg) << 5) + VAR + 1;
				point.panel = panel_number;//Modbus.network_ID[port];
				point.sub_id = scan_db[index].id;
				point.network_number = 0;//Setting_Info.reg.network_number;	
				get_net_point_value(&point,&value , 0 , 0); 	
			}	
			
			sub_map[index].do_len = 0;
			sub_map[index].ao_len = 0;
			sub_map[index].ai_len = 32;
			
		}
		else if(type == PM_T3LC)
		{
			for(i = 0;i < 7;i++)  // 7 do
			{  
				reg = T3_LC_DO_REG_START + i;
				point.number = LOW_BYTE(reg);
				point.point_type = (HIGH_BYTE(reg) << 5) + VAR + 1;
				point.panel = panel_number;//Modbus.network_ID[port];
				point.sub_id = scan_db[index].id;
				point.network_number = 0;//Setting_Info.reg.network_number;	
				get_net_point_value(&point,&value , 0, 0); 	
			}
			
			for(i = 0;i < 7;i++)  // 7 ai
			{  
				reg = T3_LC_AI_REG_START + i;
				point.number = LOW_BYTE(reg);
				point.point_type = (HIGH_BYTE(reg) << 5) + VAR + 1;
				point.panel = panel_number;//Modbus.network_ID[port];
				point.sub_id = scan_db[index].id;
				point.network_number = 0;//Setting_Info.reg.network_number;	
				get_net_point_value(&point,&value,0, 0); 
			}
			
			sub_map[index].do_len = 7;
			sub_map[index].ai_len = 7;			
		}
		else if(type == PM_T36CTA)
		{
			for(i = 0;i < 2;i++)  // 2 do
			{  
				reg = T3_6CTA_DO_REG_START + i;
				point.number = LOW_BYTE(reg);
				point.point_type = (HIGH_BYTE(reg) << 5) + VAR + 1;
				point.panel = panel_number;//Modbus.network_ID[port];
				point.sub_id = scan_db[index].id;
				point.network_number = 0;//Setting_Info.reg.network_number;	
				get_net_point_value(&point,&value,0 , 0); 	
			}
			
			for(i = 0;i < 20;i++)  // 20 ai
			{  
				reg = T3_6CTA_AI_REG_START + i;
				point.number = LOW_BYTE(reg);
				point.point_type = (HIGH_BYTE(reg) << 5) + VAR + 1;
				point.panel = panel_number;//Modbus.network_ID[port];
				point.sub_id = scan_db[index].id;
				point.network_number = 0;//Setting_Info.reg.network_number;	
				get_net_point_value(&point,&value,0 , 0); 
			}
			
			sub_map[index].do_len = 2;
			sub_map[index].ai_len = 20;			
		}
		else if(type == PM_T322AI)
		{			
			for(i = 0;i < 22;i++)  // 22 ai
			{  
				//reg = T3_22I_AI_REG_START + i * 2;  read
				reg = T3_22I_AI_REG_START + i;  // mult-read
				point.number = LOW_BYTE(reg);
				point.point_type = (HIGH_BYTE(reg) << 5) + VAR + 1;
				point.panel = panel_number;//Modbus.network_ID[port];
				point.sub_id = scan_db[index].id;
				point.network_number = 0;//Setting_Info.reg.network_number;	
				get_net_point_value(&point,&value,0 , 0); 	
			}	
			
			sub_map[index].do_len = 0;
			sub_map[index].ao_len = 0;
			sub_map[index].ai_len = 22;			
			
		}	
		else if(type == PM_T3PT12)
		{			
			for(i = 0;i < 12;i++)  // 12 ai
			{  
				reg = T3_PT12_AI_REG_START + i;  // mult-read
				point.number = LOW_BYTE(reg);
				point.point_type = (HIGH_BYTE(reg) << 5) + VAR + 1;
				point.panel = panel_number;//Modbus.network_ID[port];
				point.sub_id = scan_db[index].id;
				point.network_number = 0;//Setting_Info.reg.network_number;	
				get_net_point_value(&point,&value,0 , 0); 	
			}	
			
			sub_map[index].do_len = 0;
			sub_map[index].ao_len = 0;
			sub_map[index].ai_len = 12;			
			
		}	
		else if(type == PM_T38AI8AO6DO)
		{		
	
			for(i = 0;i < 6;i++)  // 6 do
			{  
				reg = T3_8AIAO6DO_DO_REG_START + i;
				point.number = LOW_BYTE(reg);
				point.point_type = (HIGH_BYTE(reg) << 5) + VAR + 1;
				point.panel = panel_number;//Modbus.network_ID[port];
				point.sub_id = scan_db[index].id;
				point.network_number = 0;//Setting_Info.reg.network_number;	
				get_net_point_value(&point,&value,0 , 0); 	
			}
			for(i = 0;i < 8;i++)  // 8 ao
			{ 
				reg = T3_8AIAO6DO_AO_REG_START + i;
				point.number = LOW_BYTE(reg);
				point.point_type = (HIGH_BYTE(reg) << 5) + VAR + 1;
				point.panel = panel_number;//Modbus.network_ID[port];
				point.sub_id = scan_db[index].id;
				point.network_number = 0;//Setting_Info.reg.network_number;	
				get_net_point_value(&point,&value,0 , 0); 	
			}
			for(i = 0;i < 8;i++)  // 8 ai
			{  
				//reg = T3_8AIAO6DO_AI_REG_START + i * 2;
				reg = T3_8AIAO6DO_AI_REG_START + i;
				point.number = LOW_BYTE(reg);
				point.point_type = (HIGH_BYTE(reg) << 5) + VAR + 1;
				point.panel = panel_number;//Modbus.network_ID[port];
				point.sub_id = scan_db[index].id;
				point.network_number = 0;//Setting_Info.reg.network_number;	
				get_net_point_value(&point,&value,0 , 0); 
			}
			
			sub_map[index].do_len = 6;
			sub_map[index].ao_len = 8;
			sub_map[index].ai_len = 8;			
		}			
		else
		{			
			sub_map[index].do_len = 0;
			sub_map[index].ao_len = 0;
			sub_map[index].ai_len = 0;
		}
		
		sub_map[index].do_start = base_out;				
		base_out += sub_map[index].do_len;
		sub_map[index].ao_start = base_out;				
		base_out += sub_map[index].ao_len ;
		sub_map[index].ai_start = base_in;	
		base_in += sub_map[index].ai_len;
		
		refresh_extio_by_database(base_in - sub_map[index].ai_len + 1,base_in,\
					base_out - sub_map[index].do_len - sub_map[index].ao_len + 1,base_out,1);


		sub_map[index].sub_index = index;
		sub_map[index].type = type;
		sub_map[index].id = scan_db[index].id;

		sub_map[index].add_in_map  = 1;

		// add T3 IO point to remote point list
		
		if(base_out > MAX_OUTS)	// avoid overflow
			base_out = MAX_OUTS;	
		if(base_in > MAX_INS)	// avoid overflow
			base_in = MAX_INS;	
		if(base_var > MAX_VARS)	// avoid overflow
			base_var = MAX_VARS;	
		
		// remap 
//		AOS += sub_map[index].ao_len;
//		AIS += sub_map[index].ai_len;
//		BOS += sub_map[index].do_len;
	}
			
}




void update_remote_map_table(U8_T id,U16_T reg,U16_T value,U8_T *buf)
{
	U8_T product;
	U8_T index;
	U8_T i,j;
	U8_T temp_res_buf[50];
	
	if(get_index_by_id(id,&index) == 1)
	{
		product = scan_db[index].product_model;
		if((product == PM_T34AO) || (product == PM_T3IOA) || (product == PM_T38I13O) || (product == PM_T332AI)
			|| (product == PM_T322AI) || (product == PM_T38AI8AO6DO) || (product == PM_T36CTA) || (product == PM_T3LC)
		|| (product == PM_T3PT12))
		{
			
		if(product == PM_T34AO)
		{
			if((reg >= T3_4AO_DO_REG_START) && (reg < T3_4AO_DO_REG_START + 8))
			{ // DO
				i = reg - T3_4AO_DO_REG_START;
				if(sub_map[index].do_start + i > MAX_OUTS) return;
				if(value == 1)
				{								
					outputs[sub_map[index].do_start + i].control = 1;
				}
				else
					outputs[sub_map[index].do_start + i].control = 0;

//  have to delete control, otherwise remote_io not work				
				outputs[sub_map[index].do_start + i].switch_status = SW_AUTO;
				
				outputs[sub_map[index].do_start + i].sub_id = scan_db[index].id;
				outputs[sub_map[index].do_start + i].sub_product = PM_T34AO;
				outputs[sub_map[index].do_start + i].sub_number = i;
				
				

			}
			if((reg >= T3_4AO_AO_REG_START) && (reg < T3_4AO_AO_REG_START + 4))
			{ // AO
				i = reg - T3_4AO_AO_REG_START;
				if(sub_map[index].ao_start + i > MAX_OUTS) return;
				output_raw[sub_map[index].ao_start + i] = value / 4;
				outputs[sub_map[index].ao_start + i].value = (value / 4) * 10;
//				outputs[sub_map[index].ao_start + i].read_remote = 1;
				outputs[sub_map[index].ao_start + i].digital_analog = 1;
				outputs[sub_map[index].ao_start + i].range = V0_10; 
				outputs[sub_map[index].ao_start + i].switch_status = SW_AUTO;
				
				outputs[sub_map[index].ao_start + i].sub_id = scan_db[index].id;
				outputs[sub_map[index].ao_start + i].sub_product = PM_T34AO;
				outputs[sub_map[index].ao_start + i].sub_number = 0x80 + i;
				
			}
			if((reg >= T3_4AO_AI_REG_START) && (reg < T3_4AO_AI_REG_START + 10))
			{ // AI
				i = reg - T3_4AO_AI_REG_START;
				if(sub_map[index].ai_start + i > MAX_INS) return;
				input_raw[sub_map[index].ai_start + i] = value / 4; 
				inputs[sub_map[index].ai_start + i].value = (value / 4) * 1000;
				if(value == 0)
					inputs[sub_map[index].ai_start + i].control = 0;
				else 
					inputs[sub_map[index].ai_start + i].control = 1;
//				inputs[sub_map[index].ai_start + i].digital_analog = 1; // analog
				inputs[sub_map[index].ai_start + i].auto_manual = 0;
				inputs[sub_map[index].ai_start + i].filter = 0;
				inputs[sub_map[index].ai_start + i].calibration_sign = 0;
				inputs[sub_map[index].ai_start + i].calibration_hi = 0;
				inputs[sub_map[index].ai_start + i].calibration_lo = 0;				

				inputs[sub_map[index].ai_start + i].sub_id = scan_db[index].id;
				inputs[sub_map[index].ai_start + i].sub_product = PM_T34AO;
				inputs[sub_map[index].ai_start + i].sub_number = i;
			}
		}
		else if(product == PM_T3IOA)
		{
			if((reg >= T3_8AO_AO_REG_START) && (reg < T3_8AO_AO_REG_START + 8))
			{ // AO
				i = reg - T3_8AO_AO_REG_START;
				if(sub_map[index].ao_start + i > MAX_OUTS) return;
				output_raw[sub_map[index].ao_start + i] = value;
				
				outputs[sub_map[index].ao_start + i].value = value * 10;
				
				outputs[sub_map[index].ao_start + i].digital_analog = 1;
				outputs[sub_map[index].ao_start + i].range = V0_10; 
				outputs[sub_map[index].ao_start + i].switch_status = SW_AUTO;
				
				outputs[sub_map[index].ao_start + i].sub_id = scan_db[index].id;
				outputs[sub_map[index].ao_start + i].sub_product = PM_T3IOA;
				outputs[sub_map[index].ao_start + i].sub_number = 0x80 + i;

			}
			if((reg >= T3_8AO_AI_REG_START) && (reg < T3_8AO_AI_REG_START + 8))
			{ // AI
				i = reg - T3_8AO_AI_REG_START;
				if(sub_map[index].ai_start + i > MAX_INS) return;
				input_raw[sub_map[index].ai_start + i] = value; 
//				inputs[sub_map[index].ai_start + i].digital_analog = 1; // analog
				
				inputs[sub_map[index].ai_start + i].value = value * 1000;
				if(value == 0)
					inputs[sub_map[index].ai_start + i].control = 0;
				else 
					inputs[sub_map[index].ai_start + i].control = 1;
				inputs[sub_map[index].ai_start + i].auto_manual = 0;
				inputs[sub_map[index].ai_start + i].filter = 0;
				inputs[sub_map[index].ai_start + i].calibration_sign = 0;
				inputs[sub_map[index].ai_start + i].calibration_hi = 0;
				inputs[sub_map[index].ai_start + i].calibration_lo = 0;
				//sprintf(str,"id %d T3-8AO IN%d ",(U16_T)scan_db[index].id,(U16_T)i);	
				inputs[sub_map[index].ai_start + i].sub_id = scan_db[index].id;
				inputs[sub_map[index].ai_start + i].sub_product = PM_T3IOA;
				inputs[sub_map[index].ai_start + i].sub_number = i;
				
				
			}
		}
		else 	if(product == PM_T38I13O)
		{
			if((reg >= T3_8A13O_DO_REG_START) && (reg < T3_8A13O_DO_REG_START + 13))
			{ // 13 DO
				i = reg - T3_8A13O_DO_REG_START;
				if(sub_map[index].do_start + i > MAX_OUTS) return;
				outputs[sub_map[index].do_start + i].digital_analog = 0;
				outputs[sub_map[index].do_start + i].range = OFF_ON; 

				if(value == 0)
				{								
					outputs[sub_map[index].do_start + i].control = 0;
				}
				else
					outputs[sub_map[index].do_start + i].control = 1;		
				
//  have to delete control, otherwise remote_io not work
//				output_raw[sub_map[index].do_start + i] = value;
 
				outputs[sub_map[index].do_start + i].value = value;
				
				outputs[sub_map[index].do_start + i].switch_status = SW_AUTO;
				
				outputs[sub_map[index].do_start + i].sub_id = scan_db[index].id;
				outputs[sub_map[index].do_start + i].sub_product = PM_T38I13O;
				outputs[sub_map[index].do_start + i].sub_number = i;

			}

			if((reg >= T3_8A13O_AI_REG_START) && (reg < T3_8A13O_AI_REG_START + 16))
			{ // 8 AI
				
				i = (reg - T3_8A13O_AI_REG_START) / 2;
				if(sub_map[index].ai_start + i > MAX_AIS) return;
				//input_raw[sub_map[index].ai_start + i] = value; 
				inputs[sub_map[index].ai_start + i].value = value * 1000;
				// assume current input is OFF/ON
				if(value == 0)
					inputs[sub_map[index].ai_start + i].control = 0;
				else 
					inputs[sub_map[index].ai_start + i].control = 1;
//				inputs[sub_map[index].ai_start + i].digital_analog = 1; // analog
				inputs[sub_map[index].ai_start + i].auto_manual = 0;
				inputs[sub_map[index].ai_start + i].filter = 0;
				inputs[sub_map[index].ai_start + i].calibration_sign = 0;
				inputs[sub_map[index].ai_start + i].calibration_hi = 0;
				inputs[sub_map[index].ai_start + i].calibration_lo = 0;
				inputs[sub_map[index].ai_start + i].sub_id = scan_db[index].id;
				inputs[sub_map[index].ai_start + i].sub_product = PM_T38I13O;
				inputs[sub_map[index].ai_start + i].sub_number = i;

			}
		}
		else if(product == PM_T332AI)
		{
			if((reg >= T3_32I_AI_REG_START) && (reg < T3_32I_AI_REG_START + 32))
			{ // 32 AI
				i = reg - T3_32I_AI_REG_START;
				if(sub_map[index].ai_start + i > MAX_AIS) return;
				
				input_raw[sub_map[index].ai_start + i] = value; 
				
				inputs[sub_map[index].ai_start + i].value = value;  
				
				if(value == 0)
					inputs[sub_map[index].ai_start + i].control = 0;
				else 
					inputs[sub_map[index].ai_start + i].control = 1;
				
//				inputs[sub_map[index].ai_start + i].digital_analog = 1; // analog
				inputs[sub_map[index].ai_start + i].auto_manual = 0;
				inputs[sub_map[index].ai_start + i].filter = 0;
				inputs[sub_map[index].ai_start + i].calibration_sign = 0;
				inputs[sub_map[index].ai_start + i].calibration_hi = 0;
				inputs[sub_map[index].ai_start + i].calibration_lo = 0;
				inputs[sub_map[index].ai_start + i].sub_id = scan_db[index].id;
				inputs[sub_map[index].ai_start + i].sub_product = PM_T332AI;
				inputs[sub_map[index].ai_start + i].sub_number = i;

			}
		}
		else if(product == PM_T322AI)
		{
			if((reg >= T3_22I_AI_REG_START) && (reg < T3_22I_AI_REG_START + 22))
			{ // AI
				i = (reg - T3_22I_AI_REG_START);
				if(sub_map[index].ai_start + i > MAX_AIS) 
					return;

				for(j = 0;j < (sizeof(Str_in_point) + 1)/ 2;j++)
				{
					temp_res_buf[j * 2] = buf[j * 2 + 4];
					temp_res_buf[j * 2 + 1] = buf[j * 2 + 3];
				}
				memcpy(&inputs[sub_map[index].ai_start + i],&temp_res_buf,sizeof(Str_in_point));		
				inputs[sub_map[index].ai_start + i].sub_id = scan_db[index].id;
				inputs[sub_map[index].ai_start + i].sub_product = PM_T322AI;
				inputs[sub_map[index].ai_start + i].sub_number = i;
				input_raw[sub_map[index].ai_start + i] = value; 
			}	
			else if((reg >= MODBUS_INPUT_BLOCK_FIRST) && (reg < MODBUS_INPUT_BLOCK_LAST))
			{ // AI																	
				i = (reg - MODBUS_INPUT_BLOCK_FIRST) / ((sizeof(Str_in_point) + 1) / 2);
				j = sub_map[index].ai_start + i;
				if(j > MAX_AIS) 
					return;
				memcpy(&inputs[j],buf,sizeof(Str_in_point));	
				inputs[j].sub_id = scan_db[index].id;
				inputs[j].sub_product = PM_T322AI;
				inputs[j].sub_number = i;
			}	
		}
		else if(product == PM_T3PT12)
		{
			if((reg >= T3_PT12_AI_REG_START) && (reg < T3_PT12_AI_REG_START + 12))
			{ // AI
				i = (reg - T3_PT12_AI_REG_START);
				if(sub_map[index].ai_start + i > MAX_AIS) 
					return;

				for(j = 0;j < (sizeof(Str_in_point) + 1)/ 2;j++)
				{
					temp_res_buf[j * 2] = buf[j * 2 + 4];
					temp_res_buf[j * 2 + 1] = buf[j * 2 + 3];
				}
				memcpy(&inputs[sub_map[index].ai_start + i],&temp_res_buf,sizeof(Str_in_point));		
				inputs[sub_map[index].ai_start + i].sub_id = scan_db[index].id;
				inputs[sub_map[index].ai_start + i].sub_product = PM_T3PT12;
				inputs[sub_map[index].ai_start + i].sub_number = i;
				input_raw[sub_map[index].ai_start + i] = value; 
			}	
			else if((reg >= MODBUS_INPUT_BLOCK_FIRST) && (reg < MODBUS_INPUT_BLOCK_LAST))
			{ // AI																	
				i = (reg - MODBUS_INPUT_BLOCK_FIRST) / ((sizeof(Str_in_point) + 1) / 2);
				j = sub_map[index].ai_start + i;
				if(j > MAX_AIS) 
					return;
				memcpy(&inputs[j],buf,sizeof(Str_in_point));	
				inputs[j].sub_id = scan_db[index].id;
				inputs[j].sub_product = PM_T3PT12;
				inputs[j].sub_number = i;
			}	
		}
		else if(product == PM_T38AI8AO6DO)
		{
//#define T3_8AIAO6DO_AO_REG_START 100 // 8*1
//#define T3_8AIAO6DO_AI_REG_START 116 // 8*2
//#define T3_8AIAO6DO_DO_REG_START 108 // 6*1

		  if((reg >= T3_8AIAO6DO_DO_REG_START) && (reg < T3_8AIAO6DO_DO_REG_START + 6))
			{ // 6 DO
				i = reg - T3_8AIAO6DO_DO_REG_START;
				if(sub_map[index].do_start + i > MAX_OUTS) 
					return;

				for(j = 0;j < (sizeof(Str_out_point) + 1) / 2;j++)
				{
					temp_res_buf[j * 2] = buf[j * 2 + 4];
					temp_res_buf[j * 2 + 1] = buf[j * 2 + 3];
				}
				memcpy(&outputs[sub_map[index].do_start + i],&temp_res_buf,sizeof(Str_out_point));
//				outputs[sub_map[index].do_start + i].read_remote = 1;				
				outputs[sub_map[index].do_start + i].sub_id = scan_db[index].id;
				outputs[sub_map[index].do_start + i].sub_product = PM_T38AI8AO6DO;
				//if(i < 8)
					outputs[sub_map[index].do_start + i].sub_number = i;   // DO	
//				else
//					outputs[sub_map[index].ao_start + i].sub_number = 0x80 + i; // bit7 is 1, AO	
			}
			else if((reg >= MODBUS_OUTPUT_BLOCK_FIRST) && (reg < MODBUS_OUTPUT_BLOCK_FIRST + (sizeof(Str_out_point) + 1) / 2 * 6))
			{ // 6 DO
				i = (reg - MODBUS_OUTPUT_BLOCK_FIRST) / ((sizeof(Str_out_point) + 1) / 2);
				if(sub_map[index].do_start + i > MAX_OUTS) 
					return;
				memcpy(&outputs[sub_map[index].do_start + i],buf,sizeof(Str_out_point));
//				outputs[sub_map[index].do_start + i].read_remote = 1;				
				outputs[sub_map[index].do_start + i].sub_id = scan_db[index].id;
				outputs[sub_map[index].do_start + i].sub_product = PM_T38AI8AO6DO;
				outputs[sub_map[index].do_start + i].sub_number = i;   // DO	
			}
			
			if((reg >= T3_8AIAO6DO_AO_REG_START) && (reg < T3_8AIAO6DO_AO_REG_START + 8))
			{ // 8 AO
				i = reg - T3_8AIAO6DO_AO_REG_START;
				
				if(sub_map[index].ao_start + i > MAX_OUTS) 
					return;
			
				for(j = 0;j < (sizeof(Str_out_point) + 1) / 2;j++)
				{
					temp_res_buf[j * 2] = buf[j * 2 + 4];
					temp_res_buf[j * 2 + 1] = buf[j * 2 + 3];
				}
				memcpy(&outputs[sub_map[index].ao_start + i],&temp_res_buf,sizeof(Str_out_point));
//				outputs[sub_map[index].ao_start + i].read_remote = 1;				
				outputs[sub_map[index].ao_start + i].sub_id = scan_db[index].id;
				outputs[sub_map[index].ao_start + i].sub_product = PM_T38AI8AO6DO;
//				if(i < 8)
//					outputs[sub_map[index].ao_start + i].sub_number = i;   // DO	
//				else
					outputs[sub_map[index].ao_start + i].sub_number = 0x80 + i; // bit7 is 1, AO	
			}
			else if((reg >= MODBUS_OUTPUT_BLOCK_FIRST + (sizeof(Str_out_point) + 1) / 2 * 6) && (reg < MODBUS_OUTPUT_BLOCK_LAST))
			{ // AO
				i = (reg - MODBUS_OUTPUT_BLOCK_FIRST - (sizeof(Str_out_point) + 1) / 2 * 6) / ((sizeof(Str_out_point) + 1) / 2);
				if(sub_map[index].ao_start + i > MAX_OUTS) 
					return;

				memcpy(&outputs[sub_map[index].ao_start + i],buf,sizeof(Str_out_point));
//				outputs[sub_map[index].ao_start + i].read_remote = 1;				
				outputs[sub_map[index].ao_start + i].sub_id = scan_db[index].id;
				outputs[sub_map[index].ao_start + i].sub_product = PM_T38AI8AO6DO;
				outputs[sub_map[index].ao_start + i].sub_number = i;   // DO	
			}
			
			if((reg >= T3_8AIAO6DO_AI_REG_START) && (reg < T3_8AIAO6DO_AI_REG_START + 8))
			{ // AI
				i = (reg - T3_8AIAO6DO_AI_REG_START);
				if(sub_map[index].ai_start + i > MAX_AIS) 
					return;
				
				for(j = 0;j < (sizeof(Str_in_point) + 1 ) / 2 ;j++)
				{
					temp_res_buf[j * 2] = buf[j * 2 + 4];
					temp_res_buf[j * 2 + 1] = buf[j * 2 + 3];
				}
				
				memcpy(&inputs[sub_map[index].ai_start + i],&temp_res_buf,sizeof(Str_in_point));	
				inputs[sub_map[index].ai_start + i].sub_id = scan_db[index].id;
				inputs[sub_map[index].ai_start + i].sub_product = PM_T38AI8AO6DO;
				inputs[sub_map[index].ai_start + i].sub_number = i;
			}	
			else if((reg >= MODBUS_INPUT_BLOCK_FIRST) && (reg < MODBUS_INPUT_BLOCK_LAST + (sizeof(Str_in_point) + 1) / 2 * 8))
			{ // AI																	
				i = (reg - MODBUS_INPUT_BLOCK_FIRST) / ((sizeof(Str_in_point) + 1) / 2);
				j = sub_map[index].ai_start + i;
				if(j > MAX_AIS) 
					return;
				
				memcpy(&inputs[j],buf,sizeof(Str_in_point));	
				inputs[j].sub_id = scan_db[index].id;
				inputs[j].sub_product = PM_T38AI8AO6DO;
				inputs[j].sub_number = i;
			}	

		}
		else if(product == PM_T3LC)
		{

		  if((reg >= T3_LC_DO_REG_START) && (reg < T3_LC_DO_REG_START + 7))
			{ // DO
				i = reg - T3_LC_DO_REG_START;
				if(sub_map[index].do_start + i > MAX_OUTS) 
					return;

				for(j = 0;j < (sizeof(Str_out_point) + 1) / 2;j++)
				{
					temp_res_buf[j * 2] = buf[j * 2 + 4];
					temp_res_buf[j * 2 + 1] = buf[j * 2 + 3];
				}
				memcpy(&outputs[sub_map[index].do_start + i],&temp_res_buf,sizeof(Str_out_point));
//				outputs[sub_map[index].do_start + i].read_remote = 1;				
				outputs[sub_map[index].do_start + i].sub_id = scan_db[index].id;
				outputs[sub_map[index].do_start + i].sub_product = PM_T3LC;
				outputs[sub_map[index].do_start + i].sub_number = i;   // DO	
			}
			else if((reg >= MODBUS_OUTPUT_BLOCK_FIRST) && (reg < MODBUS_OUTPUT_BLOCK_FIRST + (sizeof(Str_out_point) + 1) / 2 * 7))
			{ // DO
				i = (reg - MODBUS_OUTPUT_BLOCK_FIRST) / ((sizeof(Str_out_point) + 1) / 2);
				if(sub_map[index].do_start + i > MAX_OUTS) 
					return;
				memcpy(&outputs[sub_map[index].do_start + i],buf,sizeof(Str_out_point));
//				outputs[sub_map[index].do_start + i].read_remote = 1;				
				outputs[sub_map[index].do_start + i].sub_id = scan_db[index].id;
				outputs[sub_map[index].do_start + i].sub_product = PM_T3LC;
				outputs[sub_map[index].do_start + i].sub_number = i;   // DO	
				
			}			
			if((reg >= T3_LC_AI_REG_START) && (reg < T3_LC_AI_REG_START + 7))
			{ // AI
				i = (reg - T3_LC_AI_REG_START);
				if(sub_map[index].ai_start + i > MAX_AIS) 
					return;
				
				for(j = 0;j < (sizeof(Str_in_point) + 1 ) / 2 ;j++)
				{
					temp_res_buf[j * 2] = buf[j * 2 + 4];
					temp_res_buf[j * 2 + 1] = buf[j * 2 + 3];
				}
				
				memcpy(&inputs[sub_map[index].ai_start + i],&temp_res_buf,sizeof(Str_in_point));	
				inputs[sub_map[index].ai_start + i].sub_id = scan_db[index].id;
				inputs[sub_map[index].ai_start + i].sub_product = PM_T3LC;
				inputs[sub_map[index].ai_start + i].sub_number = i;
			}	
			else if((reg >= MODBUS_INPUT_BLOCK_FIRST) && (reg < MODBUS_INPUT_BLOCK_LAST + (sizeof(Str_in_point) + 1) / 2 * 7))
			{ // AI																	
				i = (reg - MODBUS_INPUT_BLOCK_FIRST) / ((sizeof(Str_in_point) + 1) / 2);
				j = sub_map[index].ai_start + i;
				if(j > MAX_AIS) 
					return;
				
				memcpy(&inputs[j],buf,sizeof(Str_in_point));	
				inputs[j].sub_id = scan_db[index].id;
				inputs[j].sub_product = PM_T3LC;
				inputs[j].sub_number = i;
			}	
		}
		else if(product == PM_T36CTA)
		{
//#define T3_8AIAO6DO_AI_REG_START 116 // 20*2
//#define T3_8AIAO6DO_DO_REG_START 108 // 2*1

		  if((reg >= T3_6CTA_DO_REG_START) && (reg < T3_6CTA_DO_REG_START + 2))
			{ // DO
				i = reg - T3_6CTA_DO_REG_START;
				if(sub_map[index].do_start + i > MAX_OUTS) 
					return;

				for(j = 0;j < (sizeof(Str_out_point) + 1) / 2;j++)
				{
					temp_res_buf[j * 2] = buf[j * 2 + 4];
					temp_res_buf[j * 2 + 1] = buf[j * 2 + 3];
				}
				memcpy(&outputs[sub_map[index].do_start + i],&temp_res_buf,sizeof(Str_out_point));
//				outputs[sub_map[index].do_start + i].read_remote = 1;				
				outputs[sub_map[index].do_start + i].sub_id = scan_db[index].id;
				outputs[sub_map[index].do_start + i].sub_product = PM_T36CTA;
				outputs[sub_map[index].do_start + i].sub_number = i;   // DO	
			}
			else if((reg >= MODBUS_OUTPUT_BLOCK_FIRST) && (reg < MODBUS_OUTPUT_BLOCK_FIRST + (sizeof(Str_out_point) + 1) / 2 * 2))
			{ // DO
				i = (reg - MODBUS_OUTPUT_BLOCK_FIRST) / ((sizeof(Str_out_point) + 1) / 2);
				if(sub_map[index].do_start + i > MAX_OUTS) 
					return;
				memcpy(&outputs[sub_map[index].do_start + i],buf,sizeof(Str_out_point));
//				outputs[sub_map[index].do_start + i].read_remote = 1;				
				outputs[sub_map[index].do_start + i].sub_id = scan_db[index].id;
				outputs[sub_map[index].do_start + i].sub_product = PM_T36CTA;
				outputs[sub_map[index].do_start + i].sub_number = i;   // DO	
				
			}			
			if((reg >= T3_6CTA_AI_REG_START) && (reg < T3_6CTA_AI_REG_START + 20))
			{ // AI
				i = (reg - T3_6CTA_AI_REG_START);
				if(sub_map[index].ai_start + i > MAX_AIS) 
					return;
				
				for(j = 0;j < (sizeof(Str_in_point) + 1 ) / 2 ;j++)
				{
					temp_res_buf[j * 2] = buf[j * 2 + 4];
					temp_res_buf[j * 2 + 1] = buf[j * 2 + 3];
				}
				
				memcpy(&inputs[sub_map[index].ai_start + i],&temp_res_buf,sizeof(Str_in_point));	
				inputs[sub_map[index].ai_start + i].sub_id = scan_db[index].id;
				inputs[sub_map[index].ai_start + i].sub_product = PM_T36CTA;
				inputs[sub_map[index].ai_start + i].sub_number = i;
			}	
			else if((reg >= MODBUS_INPUT_BLOCK_FIRST) && (reg < MODBUS_INPUT_BLOCK_LAST + (sizeof(Str_in_point) + 1) / 2 * 20))
			{ // AI																	
				i = (reg - MODBUS_INPUT_BLOCK_FIRST) / ((sizeof(Str_in_point) + 1) / 2);
				j = sub_map[index].ai_start + i;
				if(j > MAX_AIS) 
					return;
				
				memcpy(&inputs[j],buf,sizeof(Str_in_point));	
				inputs[j].sub_id = scan_db[index].id;
				inputs[j].sub_product = PM_T36CTA;
				inputs[j].sub_number = i;
			}	
		}
	
//			Count_Object_Number(OBJECT_ANALOG_INPUT);
//			Count_Object_Number(OBJECT_BINARY_INPUT);
//			Count_Object_Number(OBJECT_ANALOG_OUTPUT);
//			Count_Object_Number(OBJECT_BINARY_OUTPUT);
//	Count_Object_Number(OBJECT_ANALOG_VALUE);
//	Count_Object_Number(OBJECT_BINARY_VALUE);
			Count_IN_Object_Number();
			Count_OUT_Object_Number();
			Count_VAR_Object_Number();
	}
	

	}
}


U16_T count_output_reg(U8_T * sub_index,U8_T map_type,U8_T point)
{
	U16_T reg = 0;
	U8_T i;
	if(sub_no <= 0) return reg;
	
	*sub_index = 0;	

	for(i = 0;i < sub_no;i++)
	{
		
		if((scan_db[i].product_model == PM_T34AO) || (scan_db[i].product_model == PM_T38I13O) 
			|| (scan_db[i].product_model == PM_T3IOA) || (scan_db[i].product_model == PM_T38AI8AO6DO)
			|| (scan_db[i].product_model == PM_T36CTA) || (scan_db[i].product_model == PM_T3LC))		
		{	

			if(map_type == MAP_DO && sub_map[i].do_len > 0)
			{

				if(i + 1 < sub_no)
				{		

					if((point >= sub_map[i].do_start) && (point < sub_map[i].do_start + sub_map[i].do_len))
					{
						*sub_index = i;
						if(scan_db[i].product_model == PM_T34AO)
							reg = T3_4AO_DO_REG_START + point - sub_map[i].do_start;
						if(scan_db[i].product_model == PM_T38I13O)						
							reg = T3_8A13O_DO_REG_START + point - sub_map[i].do_start;
						if(scan_db[i].product_model == PM_T38AI8AO6DO)
							reg = T3_8AIAO6DO_DO_REG_START + point - sub_map[i].do_start;
						if(scan_db[i].product_model == PM_T36CTA)
							reg = T3_6CTA_DO_REG_START + point - sub_map[i].do_start;
						if(scan_db[i].product_model == PM_T3LC)
							reg = T3_LC_DO_REG_START + point - sub_map[i].do_start;
						break;
					}
				}
				else
				{
					*sub_index = i;
					if(scan_db[i].product_model == PM_T34AO)
						reg = T3_4AO_DO_REG_START + point - sub_map[i].do_start;
					if(scan_db[i].product_model == PM_T38I13O)
						reg = T3_8A13O_DO_REG_START + point - sub_map[i].do_start;
					if(scan_db[i].product_model == PM_T38AI8AO6DO)
						reg = T3_8AIAO6DO_DO_REG_START + point - sub_map[i].do_start;
					if(scan_db[i].product_model == PM_T36CTA)
							reg = T3_6CTA_DO_REG_START + point - sub_map[i].do_start;
					if(scan_db[i].product_model == PM_T3LC)
							reg = T3_LC_DO_REG_START + point - sub_map[i].do_start;
					
				}
			// to be add more
			}
			
			if(map_type == MAP_AO && sub_map[i].ao_len > 0)
			{
				
				if(i + 1 < sub_no)
				{
					if((point >= sub_map[i].ao_start) && (point < sub_map[i].ao_start + sub_map[i].ao_len))
					{
						*sub_index = i;
						if(scan_db[i].product_model == PM_T34AO)
							reg = T3_4AO_AO_REG_START + point - sub_map[i].ao_start;
						if(scan_db[i].product_model == PM_T3IOA)
							reg = T3_8AO_AO_REG_START + point - sub_map[i].ao_start;
						if(scan_db[i].product_model == PM_T38AI8AO6DO)
							reg = T3_8AIAO6DO_AO_REG_START + point - sub_map[i].ao_start;
						break;
					}
				}
				else
				{
					*sub_index = i;
					if(scan_db[i].product_model == PM_T34AO)
						reg = T3_4AO_AO_REG_START + point - sub_map[i].ao_start;
					if(scan_db[i].product_model == PM_T3IOA)
						reg = T3_8AO_AO_REG_START + point - sub_map[i].ao_start;
					if(scan_db[i].product_model == PM_T38AI8AO6DO)
							reg = T3_8AIAO6DO_AO_REG_START + point - sub_map[i].ao_start;

				}
			// to be add more
			}
		}
		else
		{
			
		}
		
	}
	
	return reg;
	
}

void update_extio_to_database(void)
{
	//extio_points
	uint8_t i;
//	U8_T index;
	Str_Extio_point * ptr;
	U8_T j;
	ptr = &extio_points[1];
	// clear T3-expasion io
	for(i = 0;i < sub_no;i++)
	{		
		if(scan_db[i].product_model == PM_T322AI || scan_db[i].product_model == PM_T38AI8AO6DO
			|| scan_db[i].product_model == PM_T3PT12)
		{		
			db_online[scan_db[i].id / 8] &= ~(1 << (scan_db[i].id % 8));
			db_occupy[scan_db[i].id / 8] &= ~(1 << (scan_db[i].id % 8));
			
			scan_db[i].id = 0;
			scan_db[i].sn = 0;
			scan_db[i].port = 0;
			scan_db[i].product_model = 0;

		}
	}
	
	// recount db
	j = 0;
	for(i = 0;i < sub_no;i++)
	{		
		if(scan_db[i].id != 0)
		{		
			memcpy(&scan_db[j],&scan_db[i],sizeof(SCAN_DB));
			j++;
			
		}
	}
	db_ctr = j;

	for(i = 0;i < MAX_EXTIO - 1;i++,ptr++)
	{	
		if(ptr->reg.product_id == PM_T322AI || ptr->reg.product_id == PM_T38AI8AO6DO
			|| ptr->reg.product_id == PM_T3PT12)
		{  // external io must be T3 module
			// 扩展的IO手动添加时port是有优先顺序的get_baut_by_port(0) uart2>uart0>uart1
#if (ASIX_MINI || ASIX_CM5)
			check_id_in_database(ptr->reg.modbus_id,ptr->reg.sn,ptr->reg.port,get_baut_by_port(0),ptr->reg.product_id);
#else			
			check_id_in_database(ptr->reg.modbus_id,my_honts_arm(ptr->reg.sn),ptr->reg.port,get_baut_by_port(0),ptr->reg.product_id);
		
#endif
		
		}
		
	}
	
	recount_sub_addr();
	ChangeFlash = 1;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
	write_page_en[TSTAT] = 1;	
#endif
	Comm_Tstat_Initial_Data();
	
	for(i = 0;i < sub_no;i++)
	{
		remap_table(i,scan_db[i].product_model);
	}
}



#endif



U8_T Get_AOx_by_index(uint8_t index,uint8_t *ao_index)
{
	U8_T i;
	
	S8_T ret;
	
	ret = Get_Bacnet_Index_by_Number(OBJECT_ANALOG_OUTPUT,index);
	if( ret != -1)
	{
		*ao_index = ret;
		return 1;
	}
	else 
		return 0;

}

U8_T Get_BOx_by_index(uint8_t index,uint8_t *bo_index)
{
	U8_T i;
	S8_T ret;
	
	ret = Get_Bacnet_Index_by_Number(OBJECT_BINARY_OUTPUT,index);
	if( ret != -1)
	{
		*bo_index = ret;
		return 1;
	}
	else 
		return 0;
	
}

U8_T Get_index_by_AVx(uint8_t av_index,uint8_t *var_index)
{
	U8_T i;
		
	S8_T ret;
	ret = Get_Number_by_Bacnet_Index(OBJECT_ANALOG_VALUE,av_index);
	if(ret != -1)
	{
		*var_index = ret;
		return 1;
	}
	else
	{
		return 0;
	}
}

U8_T Get_index_by_BVx(uint8_t bv_index,uint8_t *var_index)
{
	U8_T i;
		
	S8_T ret;
	ret = Get_Number_by_Bacnet_Index(OBJECT_BINARY_VALUE,bv_index);
	if(ret != -1)
	{
		*var_index = ret;
		return 1;
	}
	else
	{
		return 0;
	}
}

U8_T Get_index_by_AIx(uint8_t ai_index,uint8_t *in_index)
{
	U8_T i;
		
	S8_T ret;
	
	ret = Get_Number_by_Bacnet_Index(OBJECT_ANALOG_INPUT,ai_index);
	if(ret != -1)
	{
		*in_index = ret;
		return 1;
	}
	else
	{
		return 0;
	}
}

U8_T Get_index_by_BIx(uint8_t bi_index,uint8_t *in_index)
{
	U8_T i;
		
	S8_T ret;
	
	ret = Get_Number_by_Bacnet_Index(OBJECT_BINARY_INPUT,bi_index);
	if(ret != -1)
	{
		*in_index = ret;
		return 1;
	}
	else
	{
		return 0;
	}
}

U8_T Get_index_by_AOx(uint8_t ao_index,uint8_t *out_index)
{
	U8_T i;
		
	S8_T ret;
	
	ret = Get_Number_by_Bacnet_Index(OBJECT_ANALOG_OUTPUT,ao_index);
	if(ret != -1)
	{
		*out_index = ret;
		return 1;
	}
	else
	{
		return 0;
	}

}

U8_T Get_index_by_BOx(uint8_t do_index,uint8_t *out_index)
{
	U8_T i;
	S8_T ret;
	
	ret = Get_Number_by_Bacnet_Index(OBJECT_BINARY_OUTPUT,do_index);
	if(ret != -1)
	{
		*out_index = ret;
		return 1;
	}
	else
	{
		return 0;
	}

}


#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
S32_T my_honts_arm(S32_T val)
{
	U8_T temp[4];
	S32_T temp_32;
	temp[0] = (val & 0xff000000) >> 24;
	temp[1] = (val & 0x00ff0000) >> 16;
	temp[2] = (val & 0x0000ff00) >> 8;
	temp[3] = (val & 0x000000ff);
	temp_32 = temp[0] + (U16_T)(temp[1] << 8) + ((U32_T)temp[2] << 16) + ((U32_T)temp[3] << 24);
	return temp_32;
}
#endif

void refresh_extio_by_database(uint8_t ai_start,uint8_t ai_end,uint8_t out_start,uint8_t out_end,uint8_t update_time)
{
	uint8_t i,j;
	Str_Extio_point * ptr;
	uint8_t in_len;
	uint8_t out_len;

	// clear buffer
	memset(extio_points,0,sizeof(Str_Extio_point) * MAX_EXTIO);
	// tbd: add semphore for extio_points
	ptr = &extio_points[0];

	if(Modbus.mini_type >= 1 && Modbus.mini_type <= 4)
		ptr->reg.product_id = PRODUCT_MINI_BIG;
	else
		ptr->reg.product_id = PRODUCT_MINI_ARM;
	
	ptr->reg.modbus_id = Modbus.address;
	ptr->reg.sn = swap_double(Setting_Info.reg.sn);
	ptr->reg.port = 0;	
  ptr->reg.last_contact_time = 0;	
	ptr->reg.input_start = 1;	
	ptr->reg.output_start = 1;

	if((Modbus.mini_type == MINI_BIG) || (Modbus.mini_type == MINI_BIG_ARM))			{	ptr->reg.input_end = 32;		ptr->reg.output_end = 24;   }
	else if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM))	{	ptr->reg.input_end = 16;		ptr->reg.output_end = 10;		}
	else if(Modbus.mini_type == MINI_TINY)	{	ptr->reg.input_end = 11;		ptr->reg.output_end = 8;		}
	else if((Modbus.mini_type == MINI_NEW_TINY) || (Modbus.mini_type == MINI_TINY_ARM))	{	ptr->reg.input_end = 8;		ptr->reg.output_end = 14;		}
	else if(Modbus.mini_type == MINI_TINY_11I)	{	ptr->reg.input_end = 11;		ptr->reg.output_end = 11;		}
	else if(Modbus.mini_type == MINI_NANO)	{	// no I/O
		ptr->reg.input_start = 0;	
	ptr->reg.output_start = 0;ptr->reg.input_end = 0;		ptr->reg.output_end = 0;		}

	j = 0;
	ptr = &extio_points[1];
	
	
	for(i = 0;i < sub_no;i++)
	{
		if(scan_db[i].product_model == PM_T322AI || scan_db[i].product_model == PM_T38AI8AO6DO || scan_db[i].product_model == PM_T36CTA
			|| scan_db[i].product_model == PM_T3PT12)
		{			
			j++;			
			if(j >= MAX_EXTIO) 
				return;
		
			ptr->reg.product_id = scan_db[i].product_model;
			ptr->reg.modbus_id = scan_db[i].id;
			ptr->reg.sn = scan_db[i].sn;
			ptr->reg.port = scan_db[i].port & 0x0f - 1;
			

			if(update_time == 1)
			{
				if(current_online[scan_db[i].id / 8] & (1 << (scan_db[i].id % 8)))
				{
					
#if (ASIX_MINI || ASIX_CM5)
					ptr->reg.last_contact_time = get_current_time();
#else
					ptr->reg.last_contact_time = my_honts_arm(get_current_time());
#endif
				}
			}
			if((ai_start == 0) && (ai_end == 0) && (out_start == 0) && (out_end == 0))
			{
				
				if(scan_db[i].product_model == PM_T322AI)
				{
					in_len = 22;
					out_len = 0;
				}
				else if(scan_db[i].product_model == PM_T3PT12)
				{
					in_len = 12;
					out_len = 0;	
				}
				else if(scan_db[i].product_model == PM_T38AI8AO6DO)
				{
					in_len = 8;
					out_len = 14;	
				}	
				else if(scan_db[i].product_model == PM_T36CTA)
				{
					in_len = 20;
					out_len = 2;	
				}	
				
				ptr->reg.input_start = extio_points[j -1].reg.input_end + 1;
				ptr->reg.input_end = ptr->reg.input_start + in_len - 1;
				ptr->reg.output_start = extio_points[j -1].reg.output_end + 1;
				ptr->reg.output_end = ptr->reg.output_start + out_len - 1;			
		
			}
			else
			{
				ptr->reg.input_start = ai_start;
				ptr->reg.input_end = ai_end;
				ptr->reg.output_start = out_start;
				ptr->reg.output_end = out_end;			
			}

			
			ptr++;
		}
	}
	

	
}



// type -> 0 , write
// type -> 1, multi-write
void push_expansion_out_stack(Str_out_point* ptr,uint8 point,uint8_t type)
{	
#if T3_MAP
	// protect ptr->control
		if(ptr->sub_product == PM_T38AI8AO6DO || ptr->sub_product == PM_T36CTA)
		{
			if(type == 1)
			{
				if(!(ptr->sub_number & 0x80))  // digital output
				{
					write_parameters_to_nodes(0x10,ptr->sub_id,MODBUS_OUTPUT_BLOCK_FIRST + (ptr->sub_number & 0x7f) * ((sizeof(Str_out_point) + 1) / 2),\
					(U16_T*)ptr,((sizeof(Str_out_point) + 1)));
				}
				else  // analog output
				{
					if(ptr->sub_product == PM_T38AI8AO6DO)
					write_parameters_to_nodes(0x10,ptr->sub_id,MODBUS_OUTPUT_BLOCK_FIRST + ((sizeof(Str_out_point) + 1) / 2) * 6 + (ptr->sub_number & 0x7f) * ((sizeof(Str_out_point) + 1) / 2),\
					(U16_T*)ptr,((sizeof(Str_out_point) + 1)));
				}	
				return;
			}
			
		}

	
	if(ptr->sub_product == PM_T38I13O || ptr->sub_product == PM_T34AO || ptr->sub_product == PM_T38AI8AO6DO)
	{
		U16_T reg;
		U8_T sub_index;
		U16_T value;
		//if( ptr->digital_analog == 0 )	 // DO
		if(!(ptr->sub_number & 0x80))  // digital output
		{	
			output_raw[point] = ptr->control ? 1000 : 0;
			//if(output_raw[point] != output_raw_back[point])
			{		
				reg = count_output_reg(&sub_index,MAP_DO,point);
				
				if(reg > 0)
				{	
//					if((sub_map[sub_index].type == PM_T38I13O) || (sub_map[sub_index].type == PM_T34AO)
//						|| (sub_map[sub_index].type == PM_T38AI8AO6DO))   // T3 1 byte for DO
					if(!(ptr->sub_number & 0x80))  // digital output
					{
						if(output_raw[point] >= 512)
						{	// tbd: set tstat_type, different tstat have different register list
							// choose 0 for now  	
							value = 1;	
							write_parameters_to_nodes(WRITE_VARIABLES,scan_db[sub_index].id,reg,(uint16*)&value,1);
						}
						else if(output_raw[point] == 0)
						{	
							value = 0;  
							write_parameters_to_nodes(WRITE_VARIABLES,scan_db[sub_index].id,reg,(uint16*)&value,1);	
						}						
					}
				}
				
				//output_raw_back[point] = output_raw[point];
			} 	
			
		}
		else  // AO
		{
			//output_raw[point] = (float)swap_double(ptr->value) / 10000 * 4095  ;
			//if(output_raw[point] != output_raw_back[point])
			{
				reg = count_output_reg(&sub_index,MAP_AO,point);
				if(reg > 0)
				{	
					if(sub_map[sub_index].type == PM_T34AO)
					{						
						write_parameters_to_nodes(WRITE_VARIABLES,scan_db[sub_index].id,reg,(uint16*)&output_raw[point],1);
					}
					if(sub_map[sub_index].type == PM_T38AI8AO6DO)
					{
						write_parameters_to_nodes(WRITE_VARIABLES,scan_db[sub_index].id,reg,(uint16*)&output_raw[point],1);

					}

				}
				//output_raw_back[point] = output_raw[point];
			}
		}
	}
#endif
}


void push_expansion_in_stack(Str_in_point* ptr)
{
#if T3_MAP
	if(ptr->sub_product == PM_T38AI8AO6DO || ptr->sub_product == PM_T322AI || ptr->sub_product == PM_T36CTA || ptr->sub_product == PM_T3PT12)
	{		
		write_parameters_to_nodes(0x10,ptr->sub_id,MODBUS_INPUT_BLOCK_FIRST + (ptr->sub_number & 0x7f) * ((sizeof(Str_in_point) + 1) / 2),\
			(U16_T*)ptr,((sizeof(Str_in_point) + 1)));
	}
#endif
}


#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )

#define TSTAT_DAY_SP 		345
#define TSTAT_NIGHT_SP 	350
#define TSTAT_AWAY_SP 	780
#define TSTAT_SLEEP_SP 	773

void refresh_zone(void)
{
	U8_T i,j;
	UN_ID * ptr;
	Point_Net point;

	// tbd: add semphore for ID_CONFIG
	j = 0;
	ptr = &ID_Config[0];
	for(i = 0;i < sub_no;i++)
	{
		if(scan_db[i].product_model == PM_TSTAT8 || scan_db[i].product_model == PM_TSTAT7
			|| scan_db[i].product_model == PM_TSTAT7_ARM)
		{
			j++;			
			if(j >= 254) 
				return;
			
			ptr->Str.id = scan_db[i].id;
			ptr->Str.on_line = (current_online[scan_db[i].id / 8] & (1 << (scan_db[i].id % 8))) ? 1 : 0;
						
			// add default register into remote modbus point table, and get the value
			point.number = LOW_BYTE(TSTAT_DAY_SP);
			point.point_type = (HIGH_BYTE(TSTAT_DAY_SP) << 5) + VAR + 1;
//			if(((scan_db[i].port & 0x0f) >= 1) && ((scan_db[i].port & 0x0f) <= 3))
//				point.panel = Modbus.network_ID[(scan_db[i].port & 0x0f) - 1];
//			else
//				point.panel = Modbus.network_ID[0];
			point.panel = panel_number;
			point.sub_id = scan_db[i].id;
			point.network_number = 0;//Setting_Info.reg.network_number;	
			if(get_net_point_value(&point,&value,1, 0) == 1)  // live time is 20s
			{
					ptr->Str.daysetpoint = value / 1000;	
			}
			
			point.number = LOW_BYTE(TSTAT_NIGHT_SP);
			point.point_type = (HIGH_BYTE(TSTAT_NIGHT_SP) << 5) + VAR + 1;
//			if(((scan_db[i].port & 0x0f) >= 1) && ((scan_db[i].port & 0x0f) <= 3))
//				point.panel = Modbus.network_ID[(scan_db[i].port & 0x0f) - 1];
//			else
//				point.panel = Modbus.network_ID[0];
			point.panel = panel_number;
			point.sub_id = scan_db[i].id;
			point.network_number = 0;//Setting_Info.reg.network_number;	
			if(get_net_point_value(&point,&value,1, 0) == 1) // live time is 20s
			{
					ptr->Str.nightsetpoint = value / 1000;
			}
			
			point.number = LOW_BYTE(TSTAT_AWAY_SP);
			point.point_type = (HIGH_BYTE(TSTAT_AWAY_SP) << 5) + VAR + 1;
//			if(((scan_db[i].port & 0x0f) >= 1) && ((scan_db[i].port & 0x0f) <= 3))
//				point.panel = Modbus.network_ID[(scan_db[i].port & 0x0f) - 1];
//			else
//				point.panel = Modbus.network_ID[0];
			point.panel = panel_number;
			point.sub_id = scan_db[i].id;
			point.network_number = 0;//Setting_Info.reg.network_number;	
			if(get_net_point_value(&point,&value,1, 0) == 1) // live time is 20s
			{
					ptr->Str.awaysetpoint = value / 1000;		
			}
			
			point.number = LOW_BYTE(TSTAT_SLEEP_SP);
			point.point_type = (HIGH_BYTE(TSTAT_SLEEP_SP) << 5) + VAR + 1;
//			if(((scan_db[i].port & 0x0f) >= 1) && ((scan_db[i].port & 0x0f) <= 3))
//				point.panel = Modbus.network_ID[(scan_db[i].port & 0x0f) - 1];
//			else
//				point.panel = Modbus.network_ID[0];
			point.panel = panel_number;
			point.sub_id = scan_db[i].id;
			point.network_number = 0;//Setting_Info.reg.network_number;	
			if(get_net_point_value(&point,&value,1, 0) == 1) // live time is 20s
			{
					ptr->Str.sleepsetpoint = value / 1000;		
			}
			
			memcpy(ptr->Str.name,tstat_name[i],15);
			ptr++;

		}
	}

}

void udpate_zone_table(uint8 i)
{
	UN_ID * ptr;
	Point_Net point;
	S32_T value;
	U8_T port;
	if(i >= sub_no) return;
	ptr = &ID_Config[i];

	// find port of id
//	port = get_port_by_id(ptr->Str.id) - 1;	
//	
//	if(port <= 2)
//		point.panel = Modbus.network_ID[port];
	point.panel = panel_number;	
	point.sub_id = ptr->Str.id;
	point.network_number = 0;//Setting_Info.reg.network_number;	
	
	
	value = ptr->Str.daysetpoint * 1000;	
	point.number = LOW_BYTE(TSTAT_DAY_SP);
	point.point_type = (HIGH_BYTE(TSTAT_DAY_SP) << 5) + VAR + 1;
	// write remote device
	put_net_point_value(&point,&value,0,1,2);  // OPERATOR  1
	
	value = ptr->Str.nightsetpoint * 1000;	
	point.number = LOW_BYTE(TSTAT_NIGHT_SP);
	point.point_type = (HIGH_BYTE(TSTAT_NIGHT_SP) << 5) + VAR + 1;
// write remote device
	put_net_point_value(&point,&value,0,1,2);  // OPERATOR  1

	value = ptr->Str.awaysetpoint * 1000;	
	point.number = LOW_BYTE(TSTAT_AWAY_SP);
	point.point_type = (HIGH_BYTE(TSTAT_AWAY_SP) << 5) + VAR + 1;
// write remote device
	put_net_point_value(&point,&value,0,1,2);  // OPERATOR  1
	
	value = ptr->Str.sleepsetpoint * 1000;	
	point.number = LOW_BYTE(TSTAT_SLEEP_SP);
	point.point_type = (HIGH_BYTE(TSTAT_SLEEP_SP) << 5) + VAR + 1;
// write remote device
	put_net_point_value(&point,&value,0,1,2);  // OPERATOR  1
	

}

#endif