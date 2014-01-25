#include "main.h"
#include "point.h"
#include "monitor.h"

S16_T exec_program(S16_T current_prg, U8_T *prog_code);


#define NO_TABLE_RANGES 16
#define MIDDLE_RANGE     8
const long tab_int[10] = { 11875, 21375, 10000, 18000, 10000, 18000,10000, 18000, 10000, 18000 };

const long limit[10][2] = { { -40000L, 150000L }, { -40000L, 302000L },
							{ -40000L, 120000L }, { -40000L, 248000L },
							{ -40000L, 120000L }, { -40000L, 248000L },
							{ -40000L, 120000L }, { -40000L, 248000L },
							{ -50000L, 110000L }, { -58000L, 230000L }
						  };

const unsigned char def_tab[5][17] = {
 /* 3k termistor YSI44005 -40 to 150 Deg.C or -40 to 302 Deg.F */
	{ 233*4,  211*4, 179*4, 141*4, 103*4, 71*4, 48*4, 32*4,
		21*4, 14*4, 10*4, 7*4, 5*4, 4*4, 3*4, 2*4, 1*4 },

 /* 10k termistor GREYSTONE -40 to 120 Deg.C or -40 to 248 Deg.F */
	{ 246*4, 238*4, 227*4, 211*4, 191*4, 167*4, 141*4, 115*4,
	 92*4, 72*4, 55*4, 42*4, 33*4, 25*4, 19*4, 15*4, 12*4 },

 /* 3k termistor GREYSTONE -40 to 120 Deg.C or -40 to 248 Deg.F */
	{ 233*4, 215*4, 190*4, 160*4, 127*4, 96*4, 70*4, 50*4,
		35*4, 25*4, 18*4, 13*4, 9*4, 7*4, 5*4, 4*4, 3*4 },

 /* 10k termistor KM -40 to 120 Deg.C or -40 to 248 Deg.F */
	{ 246*4, 238*4, 227*4, 211*4, 191*4, 167*4, 141*4, 115*4,
		92*4, 72*4, 55*4, 42*4, 33*4, 25*4, 19*4, 15*4, 12*4 },

 /* 3k termistor AK -40 to 150 Deg.C or -40 to 302 Deg.F */
	{ 246*4, 238*4, 227*4, 211*4, 191*4, 167*4, 141*4, 115*4,
		92*4, 72*4, 55*4, 42*4, 33*4, 25*4, 19*4, 15*4, 12*4 }
};



U32_T get_input_value_by_range( int range, U16_T raw )
{
	int index;
	long val;
	int work_var;
	int ran_in;
	int delta = MIDDLE_RANGE;
	Byte *def_tbl;
	Byte end = 0;
	range--;
	ran_in = range;
	range >>= 1;
	def_tbl = ( Byte * )&def_tab[range];

	if( raw <= def_tbl[NO_TABLE_RANGES] )
		return limit[ran_in][1];
	if( raw >= def_tbl[0] )
		return limit[ran_in][0];
	index = MIDDLE_RANGE;

	while( !end )
	{
		if( ( raw >= def_tbl[index] ) && ( raw <= def_tbl[index-1] ) )
		{
			index--;
			delta = def_tbl[index] - def_tbl[index+1];
			if( delta )
			{
				work_var = (int)( ( def_tbl[index] - raw ) * 100 );
				work_var /= delta;
				work_var += ( index * 100 );
				val = tab_int[ran_in];
				val *= work_var;
				val /= 100;
				val += limit[ran_in][0];
			}
			return val;
		}
		else
		{
			if( !delta )
				end = 1;
			delta /= 2;
			if( raw < def_tbl[index] )
				index += delta;
			else
				index -= delta;
			if( index <= 0 )
				return limit[ran_in][0];
			if( index >= NO_TABLE_RANGES )
				return limit[ran_in][1];
		}
	}
}


void control_input(void)
{
	Str_in_point *ins;
	uint8_t point = 0;
	U32_T sample;
	U8_T max_input;	
#if defined(MINI)
	if(Modbus.mini_type == BIG)
   		max_input = 32;
	else 
		max_input = 16;
#else if(defined(CM5))
	max_input = 10;
#endif

	ins = inputs;
//	inx = in_aux;
	while( point < MAX_INS )
	{		
		if( ins->auto_manual == 0)  // auto			 
		{  	
			if(point >= max_input && point < (max_input + sub_no))	sample = tst_info[point - max_input].temperature * 100;			
			else if(point < max_input)
				sample = input_raw[point];   // raw value
			if( ins->range != not_used_input )
			{					
				if( ins -> digital_analog == 0)  // digital
				{
					if( ins->range >= ON_OFF  && ins->range <= HIGH_LOW )  // control 0=OFF 1=ON
					{
						ins->control = (sample >512 ) ? 0 : 1;
					}
					else
					{
						ins->control = (sample < 512 ) ? 0 : 1;					
					}
					if( ins->range >= custom_digital1 && ins->range <= custom_digital8 )
					{
						ins->control = (sample < 512 ) ? 0 : 1;	
					}
				//	ins->value = ins->control ? 1000L : 0;
					sample = ins->control ? 1000L : 0;
				}
				else if(ins -> digital_analog == 1)	// analog
				{
					if(point < max_input)
					{
						switch(ins->range)
						{
						case Y3K_40_150DegC:
						case Y3K_40_300DegF:
						case R3K_40_150DegC:
						case R3K_40_300DegF:
						case R10K_40_120DegC:
						case R10K_40_250DegF:
						case KM10K_40_120DegC:
						case KM10K_40_250DegF:
						case A10K_50_110DegC:
						case A10K_60_200DegF:
							sample = get_input_value_by_range( ins->range, sample );
							break;
						case V0_5:
							sample =  ( sample * 5000L ) >> 10;
							break;
						case I0_100Amps:
							sample = ( 100000L * sample ) >> 10;
							break;
						case I0_20ma:
							sample = ( 20000L * sample ) >> 10;
							break;
						case I0_20psi:
							sample = ( 20000L * sample ) >> 10;
							break;
						case N0_3000FPM_0_5V:
							sample = ( 3000000L * sample ) >> 10;
							break;
						case P0_100_0_5V:
							sample = ( 100000L * sample ) >> 10;
							break;
						case P0_100_4_20ma:
							sample = 100000L * ( sample - 255 ) / 768;
							break;
						default:
							break;
						}
					}
				}
			}
			else  // not_used_input
			{
				sample = 0;
			}
			ins->value = DoulbemGetPointWord2(sample);
		}
		else if(ins->auto_manual == 1)	// manual
		{

		}	
	   	point++;
	   	ins++;
	}
}

void SendSchedualData(unsigned char number,bit flag);
U8_T far  schedule_data[10] = {0,0,0,0,0,0,0,0,0,0};

void control_output(void)
{
	Str_out_point *outs;
	U8_T point = 0;
	U32_T val;
	U8_T loop;
	U8_T max_output;	
#if defined(MINI)
	if(Modbus.mini_type == BIG)
   		max_output = 24;
	else 
		max_output = 10;
#else if(defined(CM5))
	max_output = 10;
#endif
	outs = outputs;

	while( point < MAX_OUTS )
	{	
		if( outs->range == not_used_output )
		{
			outs->value = 0L;
			val = 0;
		}
		else
		{	
			if( outs->digital_analog == 0 ) // digital_analog 0=digital 1=analog
			{ // digtal input range 
				 if( outs->range >= OFF_ON && outs->range <= LOW_HIGH )
					if( outs->control ) val = 512;
					else val = 0;
				if( outs->range >= ON_OFF && outs->range <= HIGH_LOW )
					if( outs->control ) val = 0;
					else val = 512;
				if( outs->range >= custom_digital1 && outs->range <= custom_digital8 ) 
					if( outs->control ) val = 512;
					else val = 0; 

				output_raw[point] = val;

			}
			else if( outs->digital_analog == 1 )//  analog
			{
				value = DoulbemGetPointWord2(outs->value);
				switch( outs->range )
				{
					case V0_10:							
						val = value;
						output_raw[point] = value * 100;
						break;
					case P0_100_Open:
					case P0_100_Close:
					case P0_100:
						output_raw[point] = value * 10;
					/*	if( outs->m_del_low < outs->s_del_high )
						{
							delta = outs->s_del_high - outs->m_del_low;
							value *= delta;
							value += (long)( outs->m_del_low * 100000L );
						}
						else
						{
							delta =  outs->m_del_low - outs->s_del_high;
							value *= delta;
							value += (long)( outs->s_del_high * 100000L );
						}
						val = (Byte)( value * 213 / 10000000L );*/
						break;
					case P0_20psi:
					case I_0_20ma:
						val =  value;
						output_raw[point] = value * 50;
					default:
						val = 0;
              		outs->range = not_used_output;
							/*	printf("Range error !!! " ); break;*/
				 }	
			}
		}
		outs->value = DoulbemGetPointWord2(val);	
		point++;
		outs++;		
	}	
		
	for(loop = 0;loop < sub_no;loop++)
	{
		U8_T shecdual_dat = (output_raw[loop + max_output] >= 512 ? 1 : 0);
		if(shecdual_dat != schedule_data[loop])
		{
			
			if(output_raw[loop + max_output] >= 512)
			{	// tbd: set tstat_type, different tstat have different register list
				// choose 0 for now  
			//	Test[42]++;
				write_parameters_to_nodes(sub_addr[loop],Tst_Register[TST_OCCUPIED][TSTAT_6],1);
			}
			else
			{
			//	Test[41]++;
				write_parameters_to_nodes(sub_addr[loop],Tst_Register[TST_OCCUPIED][TSTAT_6],0);
	
			}
		}
		schedule_data[loop] = shecdual_dat;

	}
}


void control_program(void)
{
	

}

void pid_controller( S16_T p_number )
{
 /* The setpoint and input point can only be local to the panel even
		though the structure would allow for points from the network segment
		to which the local panel is connected */
	S32_T op, oi, od, err, erp, out_sum;
/*  err = percent of error = ( input_value - setpoint_value ) * 100 /
					proportional_band	*/
/*	sample_time = 10L;        seconds */

#if 1
	U16_T prop;
	S32_T l1;
	Str_controller_point *con;
	Con_aux *conx;
	static S32_T temp_input_value,temp_setpoint_value;
	
	con = &controllers[p_number];
	conx = &con_aux[p_number];
	get_point_value( (Point*)&con->input, &con->input_value );
	get_point_value( (Point*)&con->setpoint, &con->setpoint_value );

	od = oi = op = 0;

//	con->proportional = 20;

	prop = con->prop_high;	
	prop <<= 8;
	prop += con->proportional;

	temp_input_value = DoulbemGetPointWord2(con->input_value);
	temp_setpoint_value = DoulbemGetPointWord2(con->setpoint_value); 
//	if(temp_input_value < 50000)	
//		temp_input_value += 1000;
//	else
//		temp_input_value = 0;
//	temp_setpoint_value = 25000;
	err = temp_input_value - temp_setpoint_value;  /* absolute error */
	
	erp = 0L;

//	con->reset = 20;

/* proportional term*/
	if( prop )
		erp = 100L * err / prop;
	if( erp > 100000L ) erp = 100000L;
	if( con->action == 0)
		op = erp; /* + */
	else
		op = -erp; /* - */

	erp = 0L;
/* integral term	*/
	/* sample_time = 2s */
	l1 = ( conx->old_err + err ) * 1; /* 5 = sample_time / 2 */
	l1 += conx->error_area;
	if( conx->error_area >= 0 )
	{
		 if( l1 > 8388607L )
				l1 = 8388607L;
	}
	else
	{
		 if( l1 < -8388607L )
				l1 = -8388607L;
	}
	conx->error_area = l1;

	if( con->reset )
	{
		if( con->action == 1)
			oi = con->reset * conx->error_area;
		else
			oi -= con->reset * conx->error_area;
		if(con->repeats_per_min == 0)
			oi /= 60L;
		else
			oi /= 3600L;
	}
/* differential term	*/
	if( con->rate )
	{
		od = conx->old_err * 100;
		od /= prop;
		od = erp - od;
		if(con->action == 0)
		{
/*			od = ( erp - conx->old_err * 100 / prop ) * con->rate / 600L;
																						/* 600 = sample_time * 60  */
			od *= con->rate;
		}
		else
		{
/*			od = -con->rate * ( erp - conx->old_err
						* 100 / prop ) / 600L; /* 600 = sample_time * 60  */
			od *= ( -con->rate );
		}
		od /= 120;//600L; 	/* 600 = sample_time * 60  */
	}

	out_sum = op + con->bias + od;

	if( out_sum > 100000L ) out_sum = 100000L;
	if( out_sum < 0 ) out_sum = 0;
	if( con->reset )
	{
		 out_sum += oi;
		 if( out_sum > 100000L )
		 {
				out_sum = 100000L;
		 }
		 if( out_sum < 0 )
		 {
			out_sum = 0;
		 }
	}
	conx->old_err = err;
	con->value = DoulbemGetPointWord2(out_sum);
#endif

}

void check_weekly_routines(void)
{
 /* Override points can only be local to the panel even though the
		structure would allow for points from the network segment to which
		the local panel is connected */
	S8_T w, i, j;
	S32_T value;
	Str_weekly_routine_point *pw;
	Time_on_off *pt;
	
#if 1
	pw = &weekly_routines[0];
	for( i=0; i< MAX_WR; pw++, i++ )
	{
		w = RTC.Clk.week - 1;
		if( w < 0 ) w = 6;
		if( pw->auto_manual == 0 )
		{	
			if( pw->override_2.point_type )
			{
				get_point_value( (Point*)&pw->override_2, &value );
				
				pw->override_2_value = value?1:0;
				if( value )		w = 8;
			}
			else
			 	pw->override_2_value = 0;
		
			if(pw->override_1.point_type)
			{
				get_point_value( (Point*)&pw->override_1, &value );
				pw->override_1_value = value?1:0;
				if( value )
					w = 7;
			}
			else
			 	pw->override_1_value = 0;
		
//			i = 0;
//			w = 0;
			pt = &wr_times[i][w].time[2*MAX_INTERVALS_PER_DAY-1];

			j = 2 * MAX_INTERVALS_PER_DAY - 1;
		/*		for( j=2*MAX_INTERVALS_PER_DAY-1; j>=0; j-- )*/
		/*	do
			{				
				pt->hours = 10 + j;
				pt->minutes = 25 + j;
				pt--;
				
			}
			while( --j >= 0 );
			pt = &wr_times[i][w].time[2*MAX_INTERVALS_PER_DAY-1];
			j = 2 * MAX_INTERVALS_PER_DAY - 1; 
		 */

			do
			{				
				if( pt->hours || pt->minutes )
				{	
					if( RTC.Clk.hour > pt->hours ) break;
					if( RTC.Clk.hour == pt->hours )
						if( RTC.Clk.min >= pt->minutes )
							break;
				}
				pt--;
				
			}
			while( --j >= 0 );
			
			if( j < 0)		
				pw->value = 0;
			else
			{ 				
				if( j & 1 ) /* j % 2 */
				{
					pw->value = 0;//SetByteBit(&pw->flag,0,weekly_value,1);
				}
				else
				{
					pw->value = 1;//SetByteBit(&pw->flag,1,weekly_value,1);
				}    
			}
		}
	}
#endif
}


void check_annual_routines( void )
{
	S8_T i;
	S8_T mask;
	S8_T octet_index;
	Str_annual_routine_point *pr;

	pr = &annual_routines[0];
	for( i=0; i<MAX_AR; i++, pr++ )
	{
   		if( pr->auto_manual == 0 )
	 	{
			mask = 0x01;
			/* Assume bit0 from octet0 = Jan 1st */
			/* octet_index = ora_current.day_of_year / 8;*/
			octet_index = mGetPointWord2(RTC.Clk.day_of_year) >> 3;
			/* bit_index = ora_current.day_of_year % 8;*/    // ????????????????????????????
	/*		bit_index = ora_current.day_of_year & 0x07;*/
			mask = mask << (mGetPointWord2(RTC.Clk.day_of_year) & 0x07 );

			if( ar_dates[i][octet_index] & mask )
				pr->value = 1;
			else
				pr->value = 0;
	   	}
	}
  //	misc_flags.check_ar=0;


}


void Bacnet_Control(void) reentrant
{
	static U8_T count_10s = 0;
	static U8_T count_1min = 0;	
	static U8_T count_3s = 0;
	static U8_T current_day;
	U16_T i,j;
	Str_program_point *ptr;
	Str_points_ptr sptr, xptr;
	portTickType xDelayPeriod  = ( portTickType ) 1000 / portTICK_RATE_MS; // 1000#endif
	for (;;)
    {
		vTaskDelay(xDelayPeriod);
		count_3s++;
		ptr = programs;	
		/* deal with exec_program roution per 1s */	
		control_input();
		ptr = programs;
		for( i = 0; i < MAX_PRGS; i++, ptr++ )
		{
			if( ptr->bytes )
			//	if( program_address[i] )
					if(ptr->on_off	== 1)  // ptr->on_off		 
					{
								
						exec_program( i, prg_code[i]);
					}
		}
		control_output();

		if(miliseclast_cur > 100)	miliseclast_cur = 0;
		else
			miliseclast_cur++;


	   // dealwith controller roution per 1 sec		
		if(count_10s < 10) 	count_10s++;
		else
		{
			count_10s = 0;
			sptr.pcon = controllers;
			for( i=0; i<1/*MAX_CONS*/; i++, sptr.pcon++  )
			{
			//	if( sptr.pcon->input.point_type && sptr.pcon->setpoint.point_type )
					pid_controller( i );
			}
		}
	
		// dealwith check_weekly_routines per 1 min
		if(count_1min < 5) 	count_1min++;
		else
		{		
			count_1min = 0;
			check_weekly_routines();
		/*	if( no_ins )   // disable it now by chelsea
			{
				sptr.pin = inputs;
				xptr.pinx = in_aux;
				for( i=0; i<no_ins; i++, sptr.pin++, xptr.pinx++ )
				{
					if( sptr.pin->range == P0_255p_min )
					{
							sptr.pin->value = 1000L * xptr.pinx->ticks;
							xptr.pinx->ticks= 0;
					}
				}
			}*/
		}
	
		// dealwith check_annual_routines per 1 day
		if(RTC.Clk.day != current_day /*|| misc_flags.check_ar*/)
		{
			check_annual_routines();
			current_day = RTC.Clk.day;
		}

		//  1 second 
		timestart++;
		sample_points();

		taskYIELD();
    }		
	
}								