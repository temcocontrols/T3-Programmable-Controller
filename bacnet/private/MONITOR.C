#include "bacnet.h"
#include "main.h"
#include "monitor.h"
#include "point.h"


#if BAC_PRIVATE

#define IN_SVAR_SAMPLE 20

//U8_T reading_sd;
//U8_T count_reading_sd;
#if ARM_WIFI
U8_T SD_exist = 2;
#endif

U8_T Read_Picture_from_SD(U8_T file,U16_T index);
void update_comport_health(void);
U32_T 				far 		SD_block_num[MAX_MONITORS * 2];

Str_mon_element     far     read_mon_point_buf[MAX_MON_POINT];
Str_mon_element     far     write_mon_point_buf[MAX_MONITORS * 2][MAX_MON_POINT];

Monitor_Block		far			mon_block[2 * MAX_MONITORS];

void monitor_reboot(void);
u8 Get_start_end_packet_by_time(u8 file_index,u32 start_time,u32 end_time, u32 * start_seg, u32 * end_seg,u32);

#if 1//ASIX
xSemaphoreHandle sem_SPI;
#endif
xSemaphoreHandle sem_monitor;


void init_new_analog_block( int mon_number, Str_monitor_point *mon_ptr/*, Monitor_Block *block_ptr*/ )
{
	Ulong sample_time;
	
	sample_time = mon_ptr->hour_interval_time * 3600L;
	sample_time += mon_ptr->minute_interval_time * 60;
	sample_time += mon_ptr->second_interval_time;

	memcpy( (void*)mon_block[mon_number * 2].inputs, (void*)mon_ptr->inputs,
			                mon_ptr->an_inputs*sizeof(Point_Net)  );

	mon_block[mon_number * 2].second_interval_time = mon_ptr->second_interval_time;
	mon_block[mon_number * 2].minute_interval_time = mon_ptr->minute_interval_time;
  mon_block[mon_number * 2].hour_interval_time = mon_ptr->hour_interval_time;
	mon_block[mon_number * 2].monitor = mon_number;
  mon_block[mon_number * 2].no_points = mon_ptr->an_inputs;
	mon_block[mon_number * 2].start_time = (U32_T)get_current_time() / sample_time;  
	mon_block[mon_number * 2].start_time++;
	mon_block[mon_number * 2].start_time *= sample_time;
//	block_ptr->start_time = (U32_T)(time_since_1970 + timestart + sample_time);
	mon_block[mon_number * 2].index = 0;

}


void init_new_digital_block( int mon_number, Str_monitor_point *mon_ptr/*,Monitor_Block *block_ptr */)
{
	memcpy( (void*)mon_block[mon_number * 2 + 1].inputs, (void*)(mon_ptr->inputs + mon_ptr->an_inputs) , \
		(mon_ptr->num_inputs - mon_ptr->an_inputs) *sizeof(Point_Net)  );

	
	mon_block[mon_number * 2 + 1].monitor = mon_number;
  mon_block[mon_number * 2 + 1].no_points = mon_ptr->num_inputs - mon_ptr->an_inputs;
	mon_block[mon_number * 2 + 1].index = 0;
	
}


/*=========================================================================*/
void sample_analog_points(char i, Str_monitor_point *mon_ptr/*, Mon_aux  *aux_ptr*/ )
{
	int j,k;
	Ulong time;
	Str_points_ptr ptr;
	S32_T value;

#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
	U8_T temp[4];
#endif
	
//	if(cSemaphoreTake(sem_monitor, 5) == pdFALSE)
//	{
//		return ;
//	}
	time = 3600L * mon_ptr->hour_interval_time;
	time += 60 * mon_ptr->minute_interval_time;
	time += mon_ptr->second_interval_time;	
	mon_ptr->next_sample_time += time;
	for(j = 0; j < mon_block[i * 2].no_points;j++)
	{
		ptr.pnet = mon_block[i * 2].inputs + j;	
		if( !mon_block[i * 2].index )	
		{				
			mon_block[i * 2].start_time = get_current_time();	
		}

		write_mon_point_buf[i * 2][mon_block[i * 2].index].index = mon_block[i * 2].monitor;
		
		write_mon_point_buf[i * 2][mon_block[i * 2].index].point.number = ptr.pnet->number;
		write_mon_point_buf[i * 2][mon_block[i * 2].index].point.point_type = ptr.pnet->point_type;
		write_mon_point_buf[i * 2][mon_block[i * 2].index].point.panel = ptr.pnet->panel;
		write_mon_point_buf[i * 2][mon_block[i * 2].index].point.sub_id = ptr.pnet->sub_id;
		write_mon_point_buf[i * 2][mon_block[i * 2].index].point.network_number = ptr.pnet->network_number;
		
		
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
		temp[0] = get_current_time();	
		temp[1] = get_current_time() >> 8;	
		temp[2] = get_current_time() >> 16;	
		temp[3] = get_current_time() >> 24;	

		write_mon_point_buf[i * 2][mon_block[i * 2].index].time = temp[3] + (U16_T)(temp[2] << 8)
	 + ((U32_T)temp[1] << 16) + ((U32_T)temp[0] << 24);
		
		write_mon_point_buf[i * 2][mon_block[i * 2].index].mark = 0x0a0d;
#endif
		
#if (ASIX_MINI || ASIX_CM5)
		write_mon_point_buf[i * 2][mon_block[i * 2].index].time = get_current_time();	
		write_mon_point_buf[i * 2][mon_block[i * 2].index].mark = 0x0d0a;
#endif

//		if(ptr.pnet->sub_id == ptr.pnet->panel)
//		{  // local point
//			if(ptr.pnet->point_type == IN + 1)
//				write_mon_point_buf[i * 2][mon_block[i * 2].index].value = swap_double(inputs[ptr.pnet->number].value);//get_input_sample( ptr.pnet->number );				
//			else if(ptr.pnet->point_type == OUT + 1)
//				write_mon_point_buf[i * 2][mon_block[i * 2].index].value = swap_double(outputs[ptr.pnet->number].value);//get_input_sample( ptr.pnet->number );				
//			else if(ptr.pnet->point_type == VAR + 1)
//				write_mon_point_buf[i * 2][mon_block[i * 2].index].value = swap_double(vars[ptr.pnet->number].value);//get_input_sample( ptr.pnet->number );				
//			else 
//				write_mon_point_buf[i * 2][mon_block[i * 2].index].value = 0;

//#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
//		temp[0] = (write_mon_point_buf[i * 2][mon_block[i * 2].index].value);	
//		temp[1] = (write_mon_point_buf[i * 2][mon_block[i * 2].index].value) >> 8;	
//		temp[2] = (write_mon_point_buf[i * 2][mon_block[i * 2].index].value) >> 16;	
//		temp[3] = (write_mon_point_buf[i * 2][mon_block[i * 2].index].value) >> 24;	

//		write_mon_point_buf[i * 2][mon_block[i * 2].index].value = temp[3] + (U16_T)(temp[2] << 8)
//	 + ((U32_T)temp[1] << 16) + ((U32_T)temp[0] << 24);
//		
//#endif	
//			
//			mon_block[i * 2].index++;
//		}
//		else
//		{		// remote points	or network points
			if(get_net_point_value( ptr.pnet, &value,1 ))  // get rid of default value 0x55555555				
			{
				write_mon_point_buf[i * 2][mon_block[i * 2].index].value =  swap_double(value);	
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
				temp[0] = (write_mon_point_buf[i * 2][mon_block[i * 2].index].value);	
				temp[1] = (write_mon_point_buf[i * 2][mon_block[i * 2].index].value) >> 8;	
				temp[2] = (write_mon_point_buf[i * 2][mon_block[i * 2].index].value) >> 16;	
				temp[3] = (write_mon_point_buf[i * 2][mon_block[i * 2].index].value) >> 24;	

				write_mon_point_buf[i * 2][mon_block[i * 2].index].value = temp[3] + (U16_T)(temp[2] << 8)
							+ ((U32_T)temp[1] << 16) + ((U32_T)temp[0] << 24);
	
#endif
				mon_block[i * 2].index++;
			}				
//		}			
	}
	if(mon_block[i * 2].index +  mon_block[i * 2].no_points > MAX_MON_POINT)
	{ 
		// current buffer is full, save data to SD card.	
		// clear no used point
		for(k = mon_block[i * 2].index;k < MAX_MON_POINT;k++)
		{
			memcpy(&write_mon_point_buf[i * 2][k],0,sizeof(Str_mon_element));
		}
#if STORE_TO_SD	
		if(Write_SD(((SD_block_num[i * 2]) >> 8 & 0xfff),i,1,(U32_T)LOW_BYTE(SD_block_num[i * 2]) * MAX_MON_POINT * sizeof(Str_mon_element)) == 1)  // write ok
#endif
		{
			if(SD_exist == 2) // exist
			{
				U8_T temp;
				E2prom_Read_Byte(EEP_SD_BLOCK_HI1 + i,&temp);
				if((temp & 0x0f) != (SD_block_num[i * 2] >> 16 & 0x0f))
				{
					temp &= 0xf0;
					temp |= (SD_block_num[i * 2] >> 16 & 0x0f);
					E2prom_Write_Byte(EEP_SD_BLOCK_HI1 + i,temp);	

				}
				E2prom_Write_Byte(EEP_SD_BLOCK_A1 + i * 2,HIGH_BYTE(SD_block_num[i * 2]));
				E2prom_Write_Byte(EEP_SD_BLOCK_A1 + i * 2 + 1,LOW_BYTE(SD_block_num[i * 2]));		
				if(SD_block_num[i * 2] < 0xfffff)
					SD_block_num[i * 2]++;
				else
				{
					E2prom_Write_Byte(EEP_SD_BLOCK_A1 + i * 2,0);
					E2prom_Write_Byte(EEP_SD_BLOCK_A1 + i * 2 + 1,0);
					E2prom_Write_Byte(EEP_SD_BLOCK_HI1 + i,temp & 0xf0);							
					SD_block_num[i * 2] = 0;
				}
			}			
		}


		init_new_analog_block( i, mon_ptr);
	}
//	cSemaphoreGive(sem_monitor);
}

U8_T far last_digital_index[12];
U32_T far last_digital_sample_time[12];
void sample_digital_points( char i,Str_monitor_point *mon_ptr/*, Mon_aux *aux_ptr*/ )
{
	int j,k;//,A,B;
	S32_T value;
	Str_points_ptr ptr;
	static U8_T far first = 0;
	static U16_T far count_write_sd = 0;
  U8_T temp[4];
	monitor_reboot();
	if(SD_exist == 2) 
		count_write_sd++;
//	if(cSemaphoreTake(sem_monitor, 0) == pdFALSE)
//		return ;

	if( !mon_block[i * 2 + 1].index )
	{
		mon_block[i * 2 + 1].start_time = get_current_time();
	}

	for( j = 0; j < mon_block[i * 2 + 1].no_points; j++ )
	{		
		if(get_net_point_value( mon_block[i * 2 + 1].inputs + j, &value,1 ))
		{ 
			// store the points changed
			ptr.pnet = mon_block[i * 2 + 1].inputs + j;		
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)			
			if(get_current_time() >= last_digital_sample_time[i] + 250)   // record digital points if no change in 250s
			{  
				if(j == mon_block[i * 2 + 1].no_points - 1)
					last_digital_sample_time[i] = get_current_time();
				// 1 min exceed, store all digital points
					write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].index = mon_block[i * 2 + 1].monitor;
					write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].point.number = ptr.pnet->number;
					write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].point.point_type = ptr.pnet->point_type;
					write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].point.panel = ptr.pnet->panel;
					write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].point.sub_id = ptr.pnet->sub_id;
					write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].point.network_number = ptr.pnet->network_number;
					
					if(swap_double(value))
					{		
							mon_block[i * 2 + 1].last_digital_state |= (1<<j);						
		#if (ASIX_MINI || ASIX_CM5)
					write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].time = get_current_time();		
					write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].value = 1;//get_input_sample( ptr.pnet->number );
					write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].mark = 0x0d0a;	
		#endif
					
		#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
					temp[0] = get_current_time();	
					temp[1] = get_current_time() >> 8;	
					temp[2] = get_current_time() >> 16;	
					temp[3] = get_current_time() >> 24;	
					
					write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].time = ((U32_T)temp[0] << 24) + ((U32_T)temp[1] << 16)\
					+ ((U16_T)temp[2] << 8) + temp[3];
					write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].value = 0x01000000;
					write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].mark = 0x0a0d;	
		#endif								
					
						
					}
					else
					{
		#if (ASIX_MINI || ASIX_CM5)
					write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].time = get_current_time();		
					write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].value = 0;
					write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].mark = 0x0d0a;	
		#endif
					
		#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
					temp[0] = get_current_time();	
					temp[1] = get_current_time() >> 8;	
					temp[2] = get_current_time() >> 16;	
					temp[3] = get_current_time() >> 24;	
					
					write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].time = ((U32_T)temp[0] << 24) + ((U32_T)temp[1] << 16)\
					+ ((U16_T)temp[2] << 8) + temp[3];
					write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].value = 0;
					write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].mark = 0x0a0d;	
		#endif						
					}
					mon_block[i * 2 + 1].index++;
			}
			else
#endif
			{			
//					if(first == 0)  
//					{
//						first = 1;
//					}
//					else
					{	
						if( 1 << j & mon_block[i * 2 + 1].last_digital_state )	
						{
							if(!swap_double(value))
							{
								
								mon_block[i * 2 + 1].last_digital_state &= ~(1<<j);				
								write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].index = mon_block[i * 2 + 1].monitor;
								
								
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
								temp[0] = get_current_time();	
								temp[1] = get_current_time() >> 8;	
								temp[2] = get_current_time() >> 16;	
								temp[3] = get_current_time() >> 24;	
								
								write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].time = ((U32_T)temp[0] << 24) + ((U32_T)temp[1] << 16)\
										+ ((U16_T)temp[2] << 8) + temp[3];
								write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].mark = 0x0a0d;
#endif
				
#if (ASIX_MINI || ASIX_CM5)
								write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].time = get_current_time();	
								write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].mark = 0x0d0a;
#endif							
								write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].point.number = ptr.pnet->number;
								write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].point.point_type = ptr.pnet->point_type;
								write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].point.panel = ptr.pnet->panel;
								write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].point.sub_id = ptr.pnet->sub_id;
								write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].point.network_number = ptr.pnet->network_number;
								write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].value = 0;//get_input_sample( ptr.pnet->number );
								mon_block[i * 2 + 1].index++;
							}
							else
							{
									mon_block[i * 2 + 1].last_digital_state |= (1<<j);
									
									if(first == 0 )
									{
										first = 1;
										write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].index = mon_block[i * 2 + 1].monitor;
										write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].point.number = ptr.pnet->number;
										write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].point.point_type = ptr.pnet->point_type;
										write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].point.panel = ptr.pnet->panel;
										write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].point.sub_id = ptr.pnet->sub_id;
										write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].point.network_number = ptr.pnet->network_number;
								
		#if (ASIX_MINI || ASIX_CM5)
										write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].time = get_current_time();		
										write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].value = 1;//get_input_sample( ptr.pnet->number );
										write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].mark = 0x0d0a;	
		#endif
										
		#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
										temp[0] = get_current_time();	
										temp[1] = get_current_time() >> 8;	
										temp[2] = get_current_time() >> 16;	
										temp[3] = get_current_time() >> 24;	
										
										write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].time = ((U32_T)temp[0] << 24) + ((U32_T)temp[1] << 16)\
										+ ((U16_T)temp[2] << 8) + temp[3];
										write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].value = 0x01000000;
										write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].mark = 0x0a0d;	
		#endif
										mon_block[i * 2 + 1].index++;
									}
									
							}
						}
						else
						{
							if( swap_double(value) )
							{
								mon_block[i * 2 + 1].last_digital_state |= (1<<j);
								write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].index = mon_block[i * 2 + 1].monitor;
								write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].point.number = ptr.pnet->number;
								write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].point.point_type = ptr.pnet->point_type;
								write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].point.panel = ptr.pnet->panel;
								write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].point.sub_id = ptr.pnet->sub_id;
								write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].point.network_number = ptr.pnet->network_number;
		#if (ASIX_MINI || ASIX_CM5)
								write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].time = get_current_time();	
								write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].value = 1;//get_input_sample( ptr.pnet->number );
								write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].mark = 0x0d0a;

		#endif
		#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
								temp[0] = get_current_time();	
								temp[1] = get_current_time() >> 8;	
								temp[2] = get_current_time() >> 16;	
								temp[3] = get_current_time() >> 24;	
								
								write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].time = ((U32_T)temp[0] << 24) + ((U32_T)temp[1] << 16)\
								+ ((U16_T)temp[2] << 8) + temp[3];
								write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].value = 0x01000000;
								write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].mark = 0x0a0d;	
		#endif						
								mon_block[i * 2 + 1].index++;
							}
							else
							{
								mon_block[i * 2 + 1].last_digital_state &= ~(1<<j);		
								if(first == 0 )
								{
										// first time or 			
										// if digial value keep same for 1 minute, still record
									first = 1;
									
									write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].index = mon_block[i * 2 + 1].monitor;
									write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].point.number = ptr.pnet->number;
									write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].point.point_type = ptr.pnet->point_type;
									write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].point.panel = ptr.pnet->panel;
									write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].point.sub_id = ptr.pnet->sub_id;
									write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].point.network_number = ptr.pnet->network_number;
									write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].value = 0;//get_input_sample( ptr.pnet->number );
		#if (ASIX_MINI || ASIX_CM5)
									write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].time = get_current_time();			
									write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].mark = 0x0d0a;				
		#endif
									
		#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
									temp[0] = get_current_time();	
									temp[1] = get_current_time() >> 8;	
									temp[2] = get_current_time() >> 16;	
									temp[3] = get_current_time() >> 24;	
									
									write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].time = ((U32_T)temp[0] << 24) + ((U32_T)temp[1] << 16)\
									+ ((U16_T)temp[2] << 8) + temp[3];
									write_mon_point_buf[i * 2 + 1][mon_block[i * 2 + 1].index].mark = 0x0a0d;	
		#endif
									mon_block[i * 2 + 1].index++;
								}
							}
							
						}				
					}
				}
		}
	
			
  }
	// exceed 1 min, if packet is not full
		if( (mon_block[i * 2 + 1].index + mon_block[i * 2 + 1].no_points > MAX_MON_POINT) || \
			( (count_write_sd >= 600) && (last_digital_index[i] != mon_block[i * 2 + 1].index)) )
  	{
//			count_write_sd = 0;
			for(k = mon_block[i * 2 + 1].index;k < MAX_MON_POINT;k++)
			{				
				memcpy(&write_mon_point_buf[i * 2 + 1][k],0,sizeof(Str_mon_element));
			}
#if STORE_TO_SD	
			if(Write_SD(HIGH_BYTE(SD_block_num[i * 2 + 1]) + ((SD_block_num[i * 2 + 1] >> 24) << 16),i,0,(U32_T)LOW_BYTE(SD_block_num[i * 2 + 1]) * MAX_MON_POINT * sizeof(Str_mon_element))==1)
#endif
			{
				if(SD_exist == 2) 
				{				
					U8_T temp;
					E2prom_Read_Byte(EEP_SD_BLOCK_HI1 + i,&temp);  // high 4 bits
					if((temp & 0xf0) != (SD_block_num[i * 2] >> 16 & 0xf0))
					{
						temp &= 0x0f;
						temp |= (SD_block_num[i * 2] >> 16 & 0xf0);
						E2prom_Write_Byte(EEP_SD_BLOCK_HI1 + i,temp);	

					}
					E2prom_Write_Byte(EEP_SD_BLOCK_D1 + i * 2,HIGH_BYTE(SD_block_num[i * 2 + 1]));
					E2prom_Write_Byte(EEP_SD_BLOCK_D1 + i * 2 + 1,LOW_BYTE(SD_block_num[i * 2 + 1]));	
					if(SD_block_num[i * 2 + 1] < 0xfffff)
						SD_block_num[i * 2 + 1] ++;	
					else
					{
						E2prom_Write_Byte(EEP_SD_BLOCK_D1 + i * 2,0);	
						E2prom_Write_Byte(EEP_SD_BLOCK_D1 + i * 2 + 1,0);
						E2prom_Write_Byte(EEP_SD_BLOCK_HI1 + i,temp & 0x0f);
						SD_block_num[i * 2 + 1] = 0;
					}
				}					
				
			}			
			
//			if(mon_block[i * 2 + 1].index + mon_block[i * 2 + 1].no_points > MAX_MON_POINT)
					init_new_digital_block( i, mon_ptr);
		}	
		last_digital_index[i] = mon_block[i * 2 + 1].index;
//		cSemaphoreGive(sem_monitor);
}


/*======================================================================*/
void sample_points( void  )
{
	int i;

	Str_monitor_point *mon_ptr;
	mon_ptr = monitors;


	for( i = 0; i < MAX_MONITORS; i++, mon_ptr++)
	{		
		if((mon_ptr->status > 0) /*&& (max_monitor_time[i] > 0)*/)
		{
			//if(count_max_time[i] > 0)
			{
				//count_max_time[i]--;
				if( mon_ptr->an_inputs > 0)
				{	
					if( mon_ptr->next_sample_time <= get_current_time() )
					{		
						sample_analog_points(i, mon_ptr);
					}
				}
				if( mon_ptr->num_inputs - mon_ptr->an_inputs > 0)
				{		
					sample_digital_points(i, mon_ptr);
					
				}
			}
//			else
//			{
//				monitors[i].status = 0;
//			}
		}
	}
}

//void check_whehter_reading_sd(void)
//{
//	if(reading_sd)
//	{
//		count_reading_sd++;
//		if(count_reading_sd > 5)  // keep reaading flag 5s
//		{
//			count_reading_sd = 0;
//			reading_sd = 0;
//		}
//	}
//	
//}


U8_T ReadMonitor( Mon_Data *PTRtable) 
{
	U8_T ana_dig;
	U32_T block_no;
	U8_T ret;
	
	ret = 1;
//	reading_sd = 1;
//	count_reading_sd = 0;

	ana_dig = PTRtable->sample_type;
	if(ana_dig == 1)
	{
		block_no = SD_block_num[PTRtable->index * 2] + 1;		
	}
	else
	{
		block_no = SD_block_num[PTRtable->index * 2 + 1] + 1;
	}
	
	if(SD_exist != 2) // inexist
		block_no = 1;
	
// current packet from T3000
	if(PTRtable->seg_index > 0)
	{		
		if(SD_exist != 2) // inexist
		{ // check if no sd card, whether currect data should be stored into PC's DB 
			PTRtable->special = 0;
		}
		else
		{
			if(PTRtable->seg_index < block_no)
			{
				PTRtable->special = 0;  
			}
			else
			{					
				PTRtable->special = 1;	
			}
		}
		if((PTRtable->seg_index < block_no) && (PTRtable->seg_index >= 1))
		{	
#if STORE_TO_SD		
			if(Read_SD(((PTRtable->seg_index - 1) >> 8) & 0xfff,PTRtable->index,ana_dig,(U32_T)LOW_BYTE(PTRtable->seg_index - 1) * MAX_MON_POINT * sizeof(Str_mon_element)) == 1)   // analog data	
#endif
			{	 // read ok	
				memset( PTRtable->asdu,0,MAX_MON_POINT * sizeof(Str_mon_element));
				memcpy( PTRtable->asdu,&read_mon_point_buf, MAX_MON_POINT * sizeof(Str_mon_element));
			}
#if STORE_TO_SD	
			else  // else read error
#endif
				ret = 0;
			
		}
		else // read last packet, not store into SD
		{		
				ret = 1;
				if(ana_dig == 1) // an
				{
					memset( PTRtable->asdu,0,MAX_MON_POINT * sizeof(Str_mon_element));
					memcpy( PTRtable->asdu,&write_mon_point_buf[PTRtable->index * 2], mon_block[PTRtable->index * 2].index * sizeof(Str_mon_element));
					if(SD_exist != 2) // inexist
						mon_block[PTRtable->index * 2].index = 0;
				}
				else  // digital
				{
					memset( PTRtable->asdu,0,MAX_MON_POINT * sizeof(Str_mon_element));
					memcpy( PTRtable->asdu,&write_mon_point_buf[PTRtable->index * 2 + 1], mon_block[PTRtable->index * 2 + 1].index * sizeof(Str_mon_element));			
					if(SD_exist != 2) // inexist
					{
						mon_block[PTRtable->index * 2 + 1].index = 0;
					}
				}			
		}
	}
//	else
//	{
//		memset( PTRtable->asdu,0,MAX_MON_POINT * sizeof(Str_mon_element));
//	}
#if !(ARM_WIFI)
	if(PTRtable->seg_index == 0 && PTRtable->total_seg == 0)
	{
		if(PTRtable->sample_type == 1) // analog
			Get_start_end_packet_by_time(PTRtable->index * 2,PTRtable->comm_arg.monupdate.oldest_time,PTRtable->comm_arg.monupdate.most_recent_time,&PTRtable->seg_index,&PTRtable->total_seg,block_no);
		else
			Get_start_end_packet_by_time(PTRtable->index * 2 + 1,PTRtable->comm_arg.monupdate.oldest_time,PTRtable->comm_arg.monupdate.most_recent_time,&PTRtable->seg_index,&PTRtable->total_seg,block_no);
			
	}
#endif


	return ret;

}


void check_monitor_sample_points(U8_T i)
{
	U8_T j,k;	

	monitors[i].num_inputs = 0;
	monitors[i].an_inputs = 0;
	
	for(j = 0;j < MAX_POINTS_IN_MONITOR;j++)
	{	
// 0.0. --> invalid points
		if(monitors[i].inputs[j].panel != 0)
		{
			monitors[i].num_inputs++;
			k = monitors[i].inputs[j].number;
			if(monitors[i].inputs[j].sub_id == panel_number || monitors[i].inputs[j].sub_id == 0)
			{ // Local points
				if(monitors[i].inputs[j].point_type == IN + 1)
				{
					monitors[i].range[j] = inputs[k].range;
					if(inputs[k].digital_analog == 1)
					{
						monitors[i].an_inputs++;
					}
					
				}
				else if(monitors[i].inputs[j].point_type == OUT + 1)
				{
					monitors[i].range[j] = outputs[k].range;
					if(outputs[k].digital_analog == 1)
					{
						monitors[i].an_inputs++;
					}
				}
				else if(monitors[i].inputs[j].point_type == VAR + 1)
				{
					monitors[i].range[j] = vars[k].range;
					if(vars[k].digital_analog == 1)
					{
						monitors[i].an_inputs++;
					}
				}	
			}
			else
			{ // remote points or network points
					monitors[i].an_inputs++;
			}
		}
	}
}





void monitor_init(void)
{
	U8_T i;
//	U8_T far time_unit;
//	reading_sd = 0;
//  count_reading_sd = 0;
	for(i = 0;i < MAX_MONITORS - 1;i++)
	{
		//if(monitors[i].status == 1) //
		{  // enalble monitor
			check_monitor_sample_points(i);
			if((monitors[i].second_interval_time != 0) || 
				(monitors[i].minute_interval_time != 0) || 
				(monitors[i].hour_interval_time != 0) )
			{  // sample time is not 0
					if(monitors[i].num_inputs)
					{ // check number of inputs
							if(monitors[i].an_inputs > 0)
							{ // exist analog inputs
								init_new_analog_block(i,&monitors[i]);
							}
							if(monitors[i].num_inputs - monitors[i].an_inputs)
							{// exist digital inputs
								init_new_digital_block(i,&monitors[i]);								
							}
					}
			}
			
		}
			
//	  time_unit = (monitors[i].max_time & 0xc0) >> 6;
//		
//		if(time_unit == 0) // sec		
//			max_monitor_time[i] = monitors[i].max_time & 0x3f;
//		else if(time_unit == 1) // min		
//			max_monitor_time[i] = 60L * (monitors[i].max_time & 0x3f);
//		else if(time_unit == 2) // hour		
//			max_monitor_time[i] = 3600L * (monitors[i].max_time & 0x3f);
//		else if(time_unit == 3) // day		
//			max_monitor_time[i] = 3600L * 24L * (monitors[i].max_time & 0x3f);		
//		
//		count_max_time[i] = max_monitor_time[i];
		
		monitors[i].next_sample_time = get_current_time();
		memcpy(&backup_monitors[i],&monitors[i], sizeof(Str_monitor_point)); 
		
	}	

	// fix monitor1 for store network health
	memcpy(monitors[NET_HEALTH_INDEX].label,"NETWORK",9);


	for(i = 0;i < 8;i++)
	{		
		monitors[NET_HEALTH_INDEX].inputs[i].number = i + NET_HEALTH_VAR_BASE;
		monitors[NET_HEALTH_INDEX].inputs[i].point_type = VAR + 1;
		monitors[NET_HEALTH_INDEX].inputs[i].panel = panel_number;
		monitors[NET_HEALTH_INDEX].inputs[i].sub_id = panel_number;
		monitors[NET_HEALTH_INDEX].inputs[i].network_number = 0;
		monitors[NET_HEALTH_INDEX].range[i] = HI_spd_count;
		vars[NET_HEALTH_VAR_BASE + i].range = HI_spd_count;
		vars[NET_HEALTH_VAR_BASE + i].digital_analog = 1;
		vars[NET_HEALTH_VAR_BASE + i].value = 0;
	}
	
	// BOOT 
	monitors[NET_HEALTH_INDEX].inputs[8].number = 8 + NET_HEALTH_VAR_BASE;
	monitors[NET_HEALTH_INDEX].inputs[8].point_type = VAR + 1;
	monitors[NET_HEALTH_INDEX].inputs[8].panel = panel_number;
	monitors[NET_HEALTH_INDEX].inputs[8].sub_id = panel_number;
	monitors[NET_HEALTH_INDEX].inputs[8].network_number = 0;
	monitors[NET_HEALTH_INDEX].range[8] = OFF_ON;
	vars[NET_HEALTH_VAR_BASE + 8].range = OFF_ON;
	vars[NET_HEALTH_VAR_BASE + 8].digital_analog = 0;
	vars[NET_HEALTH_VAR_BASE + 8].control = 0;
	
	
	monitors[NET_HEALTH_INDEX].second_interval_time = IN_SVAR_SAMPLE;
	monitors[NET_HEALTH_INDEX].minute_interval_time = 0;
	monitors[NET_HEALTH_INDEX].hour_interval_time = 0;
	monitors[NET_HEALTH_INDEX].max_time = 0x81; // 1hour
	monitors[NET_HEALTH_INDEX].num_inputs = 9;
	monitors[NET_HEALTH_INDEX].an_inputs = 8;
	monitors[NET_HEALTH_INDEX].status = 0;  // only for test
	
	monitors[NET_HEALTH_INDEX].next_sample_time = get_current_time();
	
	init_new_analog_block(NET_HEALTH_INDEX,&monitors[NET_HEALTH_INDEX]);
	init_new_digital_block(NET_HEALTH_INDEX,&monitors[NET_HEALTH_INDEX]);

#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
#if ARM_MINI
	sem_SPI = vSemaphoreCreateBinary(0);
#endif
	sem_monitor = vSemaphoreCreateBinary(0);
	
	
#else

#if ASIX_MINI
	 vSemaphoreCreateBinary(sem_SPI);
#endif
	 vSemaphoreCreateBinary(sem_monitor);

#endif
	boot = 0;
	
}


void update_comport_health(void)
{
	static U32_T far com_rx_back[3] = {0 , 0, 0};
	static U32_T far com_tx_back[3] = {0 , 0, 0};
	static U32_T far etr_rx = 0;
	static U32_T far etr_tx = 0;
	static U8_T far count = 0;
	U8_T i;

	
	if(count < IN_SVAR_SAMPLE)
	{
		count++;
		return;
	}
	else
		count = 0;
	
	if(monitors[NET_HEALTH_INDEX].status == 0) 
		return;
	
	for(i = 0;i < 3;i++)
	{
		if(((com_tx[i] - com_tx_back[i]) < 1000) && (com_tx[i] > com_tx_back[i]))		
			vars[NET_HEALTH_VAR_BASE + i * 2].value = swap_double((com_tx[i] - com_tx_back[i]) * 1000l / IN_SVAR_SAMPLE);
		
		if(((com_rx[i] - com_rx_back[i]) < 1000) && (com_rx[i] > com_rx_back[i]))
			vars[NET_HEALTH_VAR_BASE + i * 2 + 1].value = swap_double((com_rx[i] - com_rx_back[i]) * 1000l/ IN_SVAR_SAMPLE);		
		
		if(com_rx_back[i] != com_rx[i])
			com_rx_back[i] = com_rx[i];
		if(com_tx_back[i] != com_tx[i])
			com_tx_back[i] = com_tx[i];
	}
	
	
	if(((ether_rx_packet - etr_rx) < 1000) && (ether_rx_packet > etr_rx))		
			vars[NET_HEALTH_VAR_BASE + 6].value = swap_double((ether_rx_packet - etr_rx) * 1000l / IN_SVAR_SAMPLE);
		
	if(((ether_tx_packet - etr_tx) < 1000) && (ether_tx_packet > etr_tx))
		vars[NET_HEALTH_VAR_BASE + 7].value = swap_double((ether_tx_packet - etr_tx) * 1000l/ IN_SVAR_SAMPLE);		
	
	if(etr_rx != ether_rx_packet)
		etr_rx = ether_rx_packet;
	if(com_tx_back[i] != ether_tx_packet)
		etr_tx = ether_tx_packet;
		
		
	
//	// BOOT
//	//vars[base + 6].control = boot;

//	if(boot <= 30)
//	{
//		boot++;
//		if(boot % 2 == 0)		
//			vars[NET_HEALTH_VAR_BASE + 8].control = 1;		
//		else
//			vars[NET_HEALTH_VAR_BASE + 8].control = 0;

//	}
	
}

void monitor_reboot(void)
{
	// BOOT
	//vars[base + 6].control = boot;

	if(boot <= 25)
	{
		boot++;
		if(boot % 2 == 0)		
			vars[NET_HEALTH_VAR_BASE + 8].control = 1;		
		else
			vars[NET_HEALTH_VAR_BASE + 8].control = 0;

	}
}

	
void dealwithMonitor(uint8_t bank)
{
	Str_points_ptr ptr;
	Str_points_ptr ptr2;
	U8_T flag;

	ptr.pmon = (Str_monitor_point*)(&backup_monitors[bank]);
	ptr2.pmon = (Str_monitor_point*)(&monitors[bank]);
	//for( bank = 0; bank < MAX_MONITORS; bank++, ptr.pmon++, ptr2.pmon++ )
	{
		flag = 0;	

		//if(ptr2.pmon->status == 1)
		{	
	// check whether change monitor setting, if changed, change next_sample_time			
			monitors[bank].next_sample_time = get_current_time();
	// update an_inputs and num_inputs
			check_monitor_sample_points(bank);		
	
		/* compare the old definition with the new one except for the label */
		if( memcmp( ptr.pmon->inputs, ptr2.pmon->inputs, ( sizeof(Str_monitor_point) - 9 ) ) )
		{
		/* compare sample rate */
			if( memcmp( (void*)&ptr.pmon->second_interval_time,
						(void*)&ptr2.pmon->second_interval_time, 3 ))

			{ /* different sample rate */
				if( ptr2.pmon->an_inputs )
					flag |= 0x01;  /* need a new analog block */
				/* compare number of digital points */
				if( ( ptr.pmon->num_inputs - ptr.pmon->an_inputs ) !=  ( ptr2.pmon->num_inputs - ptr2.pmon->an_inputs ) )
					flag |= 0x02;  /* need a new digital block */
				else
				{
					/* compare digital points' definition */
					if( memcmp( (void*)(ptr.pmon->inputs+ptr.pmon->an_inputs),
							(void*)(ptr2.pmon->inputs+ptr2.pmon->an_inputs),
						( ptr.pmon->num_inputs - ptr.pmon->an_inputs ) *
							sizeof( Point_Net ) ) )
						flag |= 0x02;  /* need a new digital block */
				}
			}
			else /* same sample rate */
			{
				
				/* compare number of analog points */
				if( ptr.pmon->an_inputs == ptr2.pmon->an_inputs )
				{
					/* compare analog points' definition */
					if( memcmp( (void*)(ptr.pmon->inputs),
								(void*)(ptr2.pmon->inputs),
						( ptr.pmon->an_inputs ) * sizeof( Point_Net ) ) )
						flag |= 0x01;  /* need a new analog block */
				}
				else
				{
					flag |= 0x01;  /* need a new analog block */
				}
				/* compare number of digital points */
				if( ( ptr.pmon->num_inputs - ptr.pmon->an_inputs ) !=
						( ptr2.pmon->num_inputs - ptr2.pmon->an_inputs ) )
				{
					flag |= 0x02;  /* need a new digital block */ 
				}
				else
				{
					/* compare digital points' definition */
					if( memcmp( (void*)(ptr.pmon->inputs+ptr.pmon->an_inputs),
							(void*)(ptr2.pmon->inputs+ptr2.pmon->an_inputs),
						( ptr.pmon->num_inputs - ptr.pmon->an_inputs ) *
							sizeof( Point_Net ) ) )
						flag |= 0x02;  /* need a new digital block */
				}
			}
			
		}

	//	no_points = ( ptr.pmon - monitors );/* / sizeof(Str_monitor_point);*/
			if( flag & 0x01 ) /* get a new analog block */
			{ 
#if  STORE_TO_SD			
				if(Write_SD((SD_block_num[bank * 2] >> 8) & 0xfff,bank,1,(U32_T)LOW_BYTE(SD_block_num[bank * 2]) * sizeof(Str_mon_element)) == 1)
#endif
				{
					if(SD_exist == 2) 
					{
						U8_T temp;
						E2prom_Read_Byte(EEP_SD_BLOCK_HI1 + bank,&temp);
						if((temp & 0x0f) != (SD_block_num[bank * 2] >> 16 & 0x0f))
						{
							temp &= 0xf0;
							temp |= (SD_block_num[bank * 2] >> 16 & 0x0f);
							E2prom_Write_Byte(EEP_SD_BLOCK_HI1 + bank,temp);	

						}
				
						E2prom_Write_Byte(EEP_SD_BLOCK_A1 + bank * 2,HIGH_BYTE(SD_block_num[bank * 2]));
						E2prom_Write_Byte(EEP_SD_BLOCK_A1 + bank * 2 + 1,LOW_BYTE(SD_block_num[bank * 2]));	
						if(SD_block_num[bank * 2] < 0xfffff)
							SD_block_num[bank * 2]++;
						else
						{
							E2prom_Write_Byte(EEP_SD_BLOCK_A1 + bank * 2,0);
							E2prom_Write_Byte(EEP_SD_BLOCK_A1 + bank * 2 + 1,0);
							E2prom_Write_Byte(EEP_SD_BLOCK_HI1 + bank,temp & 0xf0);							
							SD_block_num[bank * 2] = 0;
						}
					}	
						
					init_new_analog_block( bank, ptr2.pmon);
				}		
	
			}
			if( flag & 0x02 ) /* get a new digital block */
			{
#if  STORE_TO_SD
				
				if(Write_SD(HIGH_BYTE(SD_block_num[bank * 2 + 1]) + ((SD_block_num[bank * 2 + 1] >> 24) << 16),bank,0,(U32_T)LOW_BYTE(SD_block_num[bank * 2 + 1]) * sizeof(Str_mon_element)) == 1)
#endif					
				{
					if(SD_exist == 2) 
					{
						U8_T temp;
						E2prom_Read_Byte(EEP_SD_BLOCK_HI1 + bank,&temp);  // high 4 bits
						if((temp & 0xf0) != (SD_block_num[bank * 2] >> 16 & 0xf0))
						{
							temp &= 0x0f;
							temp |= (SD_block_num[bank * 2] >> 16 & 0xf0);
							E2prom_Write_Byte(EEP_SD_BLOCK_HI1 + bank,temp);	

						}
					
						E2prom_Write_Byte(EEP_SD_BLOCK_D1 + bank * 2,HIGH_BYTE(SD_block_num[bank * 2 + 1]));	
						E2prom_Write_Byte(EEP_SD_BLOCK_D1 + bank * 2 + 1,LOW_BYTE(SD_block_num[bank * 2 + 1]));						
						if(SD_block_num[bank * 2 + 1] < 0xfffff)
							SD_block_num[bank * 2 + 1] ++;
						else
						{
							E2prom_Write_Byte(EEP_SD_BLOCK_D1 + bank * 2,0);	
							E2prom_Write_Byte(EEP_SD_BLOCK_D1 + bank * 2 + 1,0);
							E2prom_Write_Byte(EEP_SD_BLOCK_HI1 + bank,temp & 0x0f);
							SD_block_num[bank * 2 + 1] = 0;
						}
					}
					init_new_digital_block( bank, ptr2.pmon);	
				}		
				
			}
			
		}
		memcpy(&backup_monitors[bank],&monitors[bank], sizeof(Str_monitor_point));  // record moinitor data
	}

}




#if 0//STORE_TO_SD
void ReadPicture( Mon_Data *PTRtable) 
{
  memset( PTRtable->asdu,0,PIC_PACKET_LEN);	
	if(Read_Picture_from_SD(PTRtable->index,PTRtable->seg_index))
	{
		if(SD_exist == 2)
		{			
			memcpy( PTRtable->asdu,&read_mon_point_buf, PIC_PACKET_LEN);
		}	
	}
}
#endif



#endif
