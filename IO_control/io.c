#include "main.h"
#include "controls.h"


void Set_Input_Type(uint8_t point)
{
#if !(ARM_WIFI)
		if((Modbus.mini_type == MINI_BIG)  || (Modbus.mini_type == MINI_BIG_ARM)
		|| (Modbus.mini_type == MINI_SMALL)	|| (Modbus.mini_type == MINI_SMALL_ARM))
		{
			InputLed[point] &= 0x0f;		
			if(input_type[point] >= 1)
				InputLed[point] |= ((input_type[point] - 1) << 4);			
			else
				InputLed[point] |= (input_type[point] << 4);
		}
	
		if((Modbus.mini_type == MINI_TINY && Modbus.hardRev < STM_TINY_REV)|| Modbus.mini_type == MINI_CM5)  
		{
#if (ASIX_MINI || ASIX_CM5)	
			if(input_type[point] > 0)
			{
				push_cmd_to_picstack(SET_INPUT1_TYPE + point,input_type[point] - 1);  // only for tiny
			}
			else
			{					
				push_cmd_to_picstack(SET_INPUT1_TYPE + point,3);  // only for tiny
			}
#endif
		}
#endif		
}

uint16_t get_input_raw(uint8_t point)
{
	return input_raw[point];
}

void set_output_raw(uint8_t point,uint16_t value)
{
	output_raw[point] = value;
}

uint16_t get_output_raw(uint8_t point)
{
	return output_raw[point];
}


uint32_t conver_by_unit_5v(uint32_t sample)
{	
	
	if((Modbus.mini_type == MINI_BIG) || (Modbus.mini_type == MINI_BIG_ARM))	
	{
		if(Modbus.hardRev >= 22)  // rev22  use input moudle
			
			return  (5000L * sample ) >> 10;
		else
			return  (3000L * sample) >> 10;
	}							
	else if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM)) // rev4  use input moudle
	{
		if(Modbus.hardRev >= 4)
		{
			return  (5000L * sample ) >> 10;
		}
		else
			return  (3000L * sample ) >> 10;
	}
	else if(Modbus.mini_type == MINI_TINY) // rev4  use input moudle
	{
		if(Modbus.hardRev >= STM_TINY_REV)
			return  (5000L * sample ) >> 10;
		else
			return (8300L * sample ) >> 10;	// input 模块有内阻，必须加调整		 
	}	
	else if((Modbus.mini_type == MINI_NEW_TINY) || (Modbus.mini_type == MINI_TINY_ARM) )
	{
		return (5000L * sample ) >> 10;	 
	}	
	else //if(Modbus.mini_type == MINI_CM5) // rev4  use input moudle
	{
		sample =  ( 5000L *  sample) >> 10;
	}			
}

uint32_t conver_by_unit_10v(uint32_t sample)
{
					
		if((Modbus.mini_type == MINI_BIG) || (Modbus.mini_type == MINI_BIG_ARM))
		{
			if(Modbus.hardRev >= 22)
				return (10000l * sample) >> 10;
			else 
				return  (9000L * sample) >> 10;
		}							
		else if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM)) // rev4  use input moudle
		{
			if(Modbus.hardRev >= 6)
				return (10000l * sample) >> 10;	
			else 
				return (9000L * sample) >> 10;	
		}
		else if(Modbus.mini_type == MINI_TINY) // rev6  use input moudle
		{
			if(Modbus.hardRev >= STM_TINY_REV)
				return (10000l * sample) >> 10;	
			else 
				return ( 9000L * sample ) >> 10;	
		}
		else if((Modbus.mini_type == MINI_NEW_TINY) || (Modbus.mini_type == MINI_TINY_ARM) )
		{
			return (10000L * sample ) >> 10;	 
		}	
		else //if(Modbus.mini_type == MINI_CM5) 
		{
			return  ( 10000L * sample) >> 10;
		}
}

uint32_t conver_by_unit_custable(uint8_t point,uint32_t sample)
{	
	vars[20].value = input_raw[3] * 1000;
	vars[21].value = input_raw[4] * 1000;
	if(input_type[point] == INPUT_V0_5)
	{
		if((Modbus.mini_type == MINI_BIG) || (Modbus.mini_type == MINI_BIG_ARM))	
		{
			if(Modbus.hardRev >= 22)  // rev22  use input moudle
				return  ( 5000L * sample) >> 10;
			else
				return  ( 3000L  * sample ) >> 10;
		}							
		else if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM)) // rev4  use input moudle
		{
			if(Modbus.hardRev >= 4)
				return  ( 5000L * sample  ) >> 10;
			else
				return  ( 3000L  * sample ) >> 10;
		}
		else if(Modbus.mini_type == MINI_TINY)
		{
			if(Modbus.hardRev >= STM_TINY_REV)
				return  ( 5000L * sample  ) >> 10;
			else
				return ( 8300L * sample  ) >> 10;	// input 模块有内阻，必须加调整		 
		}
		else if((Modbus.mini_type == MINI_NEW_TINY) || (Modbus.mini_type == MINI_TINY_ARM))
		{
			return (5000L * sample ) >> 10;	 
		}	
		else //if(Modbus.mini_type == MINI_CM5)
		{
			return  ( 5000L * sample ) >> 10;
		}
		
	}
	else if(input_type[point] == INPUT_I0_20ma)
	{
		return ( 20000L * sample ) >> 10; 
	}
	else if(input_type[point] == INPUT_0_10V)
	{
		if((Modbus.mini_type == MINI_BIG) || (Modbus.mini_type == MINI_BIG_ARM))	
		{
			if(Modbus.hardRev >= 22)
				return ( 10000l * sample) >> 10;
			else 
				return  (  9000L * sample) >> 10;
		}							
		else if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM)) // rev4  use input moudle
		{
			if(Modbus.hardRev >= 6)
				return (10000l * sample ) >> 10;	
			else 
				return (9000L *   sample) >> 10;	
		}
		else if(Modbus.mini_type == MINI_TINY) // rev6  use input moudle
		{
			if(Modbus.hardRev >= STM_TINY_REV)
				return (10000l * sample  ) >> 10;	
			else 
				return (9000L * sample  ) >> 10;	
		} 
		else if((Modbus.mini_type == MINI_NEW_TINY) || (Modbus.mini_type == MINI_TINY_ARM))
		{
			return (1000L * sample ) >> 10;	 
		}	
		else //if(Modbus.mini_type == MINI_CM5)
		{
			return  (10000L * sample) >> 10;
		}

	}
	else if(input_type[point] == INPUT_THERM)
	{
		return 0;//get_input_value_by_range( inputs[point].range, sample );
	}
		
}

uint8_t get_max_input(void)
{	
	return base_in;
}

uint8_t get_max_output(void)
{	
	return base_out;
}

uint8_t get_max_internal_input(void)
{	
	if((Modbus.mini_type == MINI_BIG) || (Modbus.mini_type == MINI_BIG_ARM))
	{
		return BIG_MAX_AIS;
	}
	else if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM))
	{
	  return SMALL_MAX_AIS;
	}
	else if(Modbus.mini_type == MINI_TINY)
	{
	  return TINY_MAX_AIS;
	}
	else if(Modbus.mini_type == MINI_VAV)
	{
	  return VAV_MAX_AIS;
	}
	else if(Modbus.mini_type == MINI_CM5)
	{
	  return CM5_MAX_AIS;
	}
	else if((Modbus.mini_type == MINI_NEW_TINY) || (Modbus.mini_type == MINI_TINY_ARM))
	{
	  return NEW_TINY_MAX_AIS;
	}
	return 0;
}

uint8_t get_max_internal_output(void)
{	
	if((Modbus.mini_type == MINI_BIG) || (Modbus.mini_type == MINI_BIG_ARM))
	{
		return BIG_MAX_AOS + BIG_MAX_DOS;
	}
	else if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM))
	{
	  return SMALL_MAX_AOS + SMALL_MAX_DOS;
	}
	else if(Modbus.mini_type == MINI_TINY)
	{
	  return TINY_MAX_AOS + TINY_MAX_DOS;
	}
	else if(Modbus.mini_type == MINI_VAV)
	{
	  return VAV_MAX_AOS + VAV_MAX_DOS;
	}
	else if(Modbus.mini_type == MINI_CM5)
	{
	  return CM5_MAX_AOS + CM5_MAX_DOS;
	}
	else if((Modbus.mini_type == MINI_NEW_TINY) || (Modbus.mini_type == MINI_TINY_ARM))
	{
	  return NEW_TINY_MAX_AOS + NEW_TINY_MAX_DOS;
	}
	return 0;
}


uint32_t get_high_spd_counter(uint8_t point)
{
	inputs[point].value = swap_double((high_spd_counter[point] + high_spd_counter_tempbuf[point]) * 1000);
	return (high_spd_counter[point] + high_spd_counter_tempbuf[point]) * 1000;
}

// old io.lib run it
void map_extern_output(uint8_t point)
{
}

//void map_extern_output(uint8_t point)
//{
//	U16_T reg;
//	U8_T sub_index;
//	U16_T value;

//	
//#if  T3_MAP
////	if((outputs[point].sub_product == PM_T38AI8AO6DO) || (outputs[point].sub_product == PM_T322AI))
////		return;
////	outputs[point].switch_status = SW_AUTO;
//	if(point >= get_max_internal_output())
//	{
//		if( outputs[point].digital_analog == 0 )	 // DO
//		{		
//			
//			output_raw[point] = outputs[point].control ? 1000 : 0;
//			if(output_raw[point] != output_raw_back[point])
//			{		
//				reg = count_output_reg(&sub_index,MAP_DO,point);
//				if(reg > 0)
//				{	
////					if((sub_map[sub_index].type == PM_T38I13O) || (sub_map[sub_index].type == PM_T34AO)
////						|| (sub_map[sub_index].type == PM_T38AI8AO6DO))   // T3 1 byte for DO
//					{
//						if(output_raw[point] >= 512)
//						{	// tbd: set tstat_type, different tstat have different register list
//							// choose 0 for now  	
//							value = 1;							
//							write_parameters_to_nodes(WRITE_VARIABLES,scan_db[sub_index].id,reg,(uint16*)&value,1);
//						}
//						else if(output_raw[point] == 0)
//						{	
//							value = 0;									
//							write_parameters_to_nodes(WRITE_VARIABLES,scan_db[sub_index].id,reg,(uint16*)&value,1);	
//						}						
//					}
//				}
//				
//				output_raw_back[point] = output_raw[point];
//			} 					
//		}
//		else  // AO
//		{
//			output_raw[point] = (float)swap_double(outputs[point].value) / 10000 * 4095  ;
//			if(output_raw[point] != output_raw_back[point])
//			{
//				reg = count_output_reg(&sub_index,MAP_AO,point);
//				if(reg > 0)
//				{	
//					if((sub_map[sub_index].type == PM_T34AO) || (sub_map[sub_index].type == PM_T38AI8AO6DO))
//					{						
//						write_parameters_to_nodes(WRITE_VARIABLES,scan_db[sub_index].id,reg,(uint16*)&output_raw[point],1);
//					}

//				}
//				output_raw_back[point] = output_raw[point];
//			}
//		}
//	}
//#endif
//	
//}


S8_T check_external_in_on_line(U8_T index)
{
	U8_T i;
	for(i = 0;i < sub_no;i++)
	{
		if((index < sub_map[i].ai_start + sub_map[i].ai_len) && (index >= sub_map[i].ai_start))
		{
			break;
		}
	}
	
	if(i == sub_no)
		return -1;
	else 
	{
		return (current_online[scan_db[i].id / 8] & (1 << (scan_db[i].id % 8))) ? 1 : 0;
	}
}


S8_T check_external_out_on_line(U8_T index)
{
	U8_T i;
	if(index < get_max_internal_output())
	{
		return -1;
	}
	for(i = 0;i < sub_no;i++)
	{
		if((index < sub_map[i].ao_start + sub_map[i].ao_len) && (index >= sub_map[i].ao_start))
		{
			break;
		}
		if((index < sub_map[i].do_start + sub_map[i].do_len) && (index >= sub_map[i].do_start))
		{
			break;
		}
	}
	
	if(i == sub_no)
		return -1;
	else 
	{
		return (current_online[scan_db[i].id / 8] & (1 << (scan_db[i].id % 8))) ? 1 : 0;
	}
}