#include "main.h"
#include "monitor.h"
#include "point.h"




int ofset = 0;
int grote = 3005;

int x1 = 3005; /* size        */
int x2 = 0;    /* first block */
int x3 = 4;    /* last block  */
int x4 = 0;    /* first index */
int x5 = 68;   /* last index  */

int magicx = 1;

int threshold = 3000; /*830;*/
extern  unsigned char TSM_22_READ1;


int start_of_xfere =0;


/*
MAX_MONITORS             8
MAX_POINTS_IN_MONITOR    14
MAX_MONITOR_POINTS       MAX_MONITORS * MAX_POINTS_IN_MONITOR

	Max number of analog samples  = MAX_MONITOR_BLOCKS * 140;
	Max number of digital samples = MAX_MONITOR_BLOCKS * 112;

	When I sample the AMONS I assume that I have enaugh space to
store them in the current block. After storing the current
samples I update the start_time in case that wrap_around is set
and then check if there is room for a complete group of
samples in the current block.

1.If there is enaugh room I just update the next_sample_time from
the mon_aux and go to check the digital points.

2.If there isn't enaugh room for a complete group of samples
I ask for a new block.

2.1. If I get a new block I update the next_block and last
block fields in the current block and then initialize the
new block by copying the inputs[MAX_POINTS_IN_MONITOR](70bytes),
second_interval_time(1 byte ),minute_interval_time(1 byte)
hour_interval_time(1 byte ), monitor(4 bits), and
no_points(4 bits) fields (total 74 bytes) from the current
block to the new block. Then I set the block_no, next_block,
first_block, last_block, analog_digital, wrap_around, and
start_time fields.


2.2 If I don't get a new block I will use the first block in
the chain.

2.2.1 If there is only 1 block in the chain then I will use
wrap around and set the index to 0.

2.2.2 If there are more blocks in the chain I check if the
first block has the same parameters (
inputs[MAX_POINTS_IN_MONITOR](70bytes), second_interval_time
(1 byte ),minute_interval_time(1 byte), hour_interval_time
(1 byte ), monitor(4 bits), and no_points(4 bits) (total 74
bytes) ) as the last one.

2.2.2.1 In case the parameters are the same I only set the
wrap_around, and last_block fields.

2.2.2.2 When the parameters are different the first block
will became the last and the second the first. Then I will
do all the things from point 2.1.

*/


/*#define MAX_MONITORS 8*/

void update_blocks_number( Monitor_Block *block_ptr )
{ /* this function is called with block_ptr being the pointer to the
		 first block in chain */
	do
	{
		block_ptr->block_no--; /* block #0 will became #255 */
		if( !block_ptr->block_no )
		{
			block_ptr->first_block = 1;
		}
		if( block_ptr->next_block < MAX_MONITOR_BLOCKS )
			block_ptr = mon_block+block_ptr->next_block;
		else
			break;
	}
	while( ( block_ptr->next_block < MAX_MONITOR_BLOCKS  ) && ( block_ptr->block_no != 255 ) );
	if( block_ptr->block_no != 255 )
	 	block_ptr->block_no--;
}


int find_new_block( int monitor_no, int ana_dig ) /* 0=analogic, 1=digital */
{
	Monitor_Block *block_ptr;
	register Mon_aux *aux_ptr;
  	Byte longest_chain_no, max_len;
	int i, pri;
	unsigned long earliest_start_time;
	if( free_mon_blocks )
	{ /* If there are unused blocks find the first unused and
	     return its number */
		block_ptr = mon_block;
		for( i = 0; i<MAX_MONITOR_BLOCKS; i++ )
		{
			if( block_ptr->no_points || block_ptr->block_state )
    			block_ptr++;
			else
			{
			  	free_mon_blocks--;
				return i;
			}
    	}
   		return -1;
  	}
  	else
  	{		
		pri = mon_aux[monitor_no].priority;
		aux_ptr = mon_aux;
		max_len = 0;
		longest_chain_no = 255;
		earliest_start_time = 0x0FFFFFFFFL;
  		for( i = 0; i < 2*MAX_MONITORS; i+=2, aux_ptr++ )
		{
			if( aux_ptr->priority <= pri )
			{
        		block_ptr = mon_block + aux_ptr->first_analog_block;
	    		if( aux_ptr->no_analog_blocks >= max_len &&
						block_ptr->start_time <= earliest_start_time &&
								 !block_ptr->wrap_around )
	      		{
		      		max_len = aux_ptr->no_analog_blocks;
					longest_chain_no = i;	// even num 
	    	  	}
				block_ptr = mon_block + aux_ptr->first_digital_block;

				if( ( aux_ptr->no_digital_blocks > max_len ) &&
					block_ptr->start_time <= earliest_start_time &&
						!block_ptr->wrap_around )
				{
					max_len = aux_ptr->no_digital_blocks;
					longest_chain_no = i + 1; // odd num
				}
			}
		}
		if( longest_chain_no == ( ( monitor_no << 1 ) + ana_dig ) )
			return -1;
		else
		{
			aux_ptr = mon_aux + (longest_chain_no>>1);
			if( longest_chain_no & 1 )
			{
				if( aux_ptr->no_digital_blocks > 1 )
				{
					block_ptr = mon_block + aux_ptr->first_digital_block;
					i = aux_ptr->first_digital_block;
					aux_ptr->first_digital_block = block_ptr->next_block;
					update_blocks_number( block_ptr );
  	      			aux_ptr->no_digital_blocks--;
					return i;
        		}
			}
      		else
			{
      			if( aux_ptr->no_analog_blocks > 1 )
				{
        			block_ptr = mon_block + aux_ptr->first_analog_block;
      	  			i = aux_ptr->first_analog_block;
					aux_ptr->first_analog_block = block_ptr->next_block;
					update_blocks_number( block_ptr );
					aux_ptr->no_analog_blocks--;
					return i;
				}
      		}
		}
	}
	return -1;
}

void init_new_analog_block( int mon_number, Str_monitor_point *mon_ptr, Monitor_Block *block_ptr )
{
	Ulong sample_time;

	sample_time = mon_ptr->hour_interval_time * 3600L;
	sample_time += mon_ptr->minute_interval_time * 60;
	sample_time += mon_ptr->second_interval_time;

	memcpy( (void*)block_ptr, (void*)mon_ptr->inputs,
			                mon_ptr->an_inputs*sizeof(Point_Net)  );
	block_ptr->second_interval_time = mon_ptr->second_interval_time;
	block_ptr->minute_interval_time = mon_ptr->minute_interval_time;
    block_ptr->hour_interval_time = mon_ptr->hour_interval_time;
	block_ptr->monitor = mon_number;
    block_ptr->no_points = mon_ptr->an_inputs;
	block_ptr->start_time = (time_since_1970 + timestart) / sample_time;  
	block_ptr->start_time++;
	block_ptr->start_time *= sample_time;
}

void get_new_analog_block( int mon_number, int init, Str_monitor_point *mon_ptr  )
{
	int j;

	Str_points_ptr ptr;
	register Monitor_Block *block_ptr;
	Mon_aux  *aux_ptr;
	aux_ptr = mon_aux + mon_number;
	if( aux_ptr->no_analog_blocks )
		ptr.pmb = mon_block + aux_ptr->current_analog_block;
	else
		ptr.pmb = NULL;
	j = find_new_block( mon_number, 0 );

	if( j >= 0 )
	{ /* there is a new block available */
		/* initialize the new block */
		block_ptr = mon_block + j;
	  	memset( (void*)block_ptr, 0, sizeof(Monitor_Block_Header) );
		if(!init ) /* the monitor is already using some blocks */
    	{
			memcpy( (void*)block_ptr, (void*)ptr.pmb, 75 ); /* inputs +	monitor + no_points + second_interval_time +
														minute_interval_time + hour_interval_time */
	  	block_ptr->start_time = aux_ptr->next_sample_time;
		}
    	else  /* this will be the first block for this monitor */
    	{
			init_new_analog_block( mon_number, mon_ptr, block_ptr );
			aux_ptr->next_sample_time = block_ptr->start_time;
		}
		/* update the last block if there is one */
		if( ptr.pmb )
		{
			ptr.pmb->last_block = 0;
			ptr.pmb->next_block = j;
    	}	
		aux_ptr->current_analog_block = j;
	    if( !aux_ptr->no_analog_blocks )
	    {
	    	aux_ptr->first_analog_block = j;
		    block_ptr->first_block = 1;
    	}
    	else
	    block_ptr->first_block = 0;	

    	block_ptr->block_no = aux_ptr->no_analog_blocks;
		aux_ptr->no_analog_blocks++;

		block_ptr->next_block = 255;
    	block_ptr->last_block = 1;
		block_ptr->analog_digital = 0;
		block_ptr->wrap_around = 0;
	}
	else
	{ /* there are no more new block available -
		will wrap starting with the first block */
    	if( !aux_ptr->no_analog_blocks )
			return;
	    if( aux_ptr->no_analog_blocks == 1 )
	    {
			if( !init )  /* the monitor is already using one block */
			{
				ptr.pmb->wrap_around = 1;
				ptr.pmb->last_block = 1;
			}
			else
			{ /* the only one block in chain will be initialized with
					 new parameters */
				block_ptr = ptr.pmb;
				memset( (void*)block_ptr, 0, 84 );

				init_new_analog_block( mon_number, mon_ptr, block_ptr );
				aux_ptr->next_sample_time = block_ptr->start_time;

				block_ptr->next_block = 255;
				block_ptr->first_block = 0;
				block_ptr->last_block = 1;
				block_ptr->analog_digital = 0;
				block_ptr->wrap_around = 0;
			}
		}
		else
		{ /* update the last block (there are more then one blocks in chain ) */
			ptr.pmb->first_block = 0;
			ptr.pmb->last_block = 0;
			block_ptr = mon_block+aux_ptr->first_analog_block;
			update_blocks_number( block_ptr );
			ptr.pmb->next_block = aux_ptr->first_analog_block;
			aux_ptr->current_analog_block = aux_ptr->first_analog_block;
			aux_ptr->first_analog_block = block_ptr->next_block;
			block_ptr->block_no = aux_ptr->no_analog_blocks - 1;
/*			ptr.pmb->next_block = 255;*/
/*			update_blocks_number( block_ptr );*/

			if( !memcmp( (void*)ptr.pmb, (void*)block_ptr, 75 ) && !init  )
			 /* 74 = inputs + monitor + no_points + second_interval_time +
														minute_interval_time + hour_interval_time */
			{ /* same monitor definition */
/*				block_ptr->wrap_around = 1;*/
				block_ptr->last_block = 1;
			}
			else
			{ /* monitor definition has changed */
				/* we'll lose the data previously stored in this block */
				if( !init )
				{ /* copy the last block's header into the new block  */
					memcpy( (void*)block_ptr, (void*)ptr.pmb, 75 );
					/* inputs + monitor + no_points + second_interval_time +
												 minute_interval_time + hour_interval_time */
					block_ptr->start_time = aux_ptr->next_sample_time;
				}
				else /* this will be the first block for this monitor */
				{
					memset( (void*)block_ptr, 0, sizeof(Monitor_Block_Header) );
					init_new_analog_block( mon_number, mon_ptr, block_ptr );
					aux_ptr->next_sample_time = block_ptr->start_time;
				}
				block_ptr->wrap_around = 0;
				block_ptr->next_block = 255;
				block_ptr->first_block = 0;
				block_ptr->last_block = 1;
				block_ptr->analog_digital = 0;
				block_ptr->wrap_around = 0;
				block_ptr->block_no = aux_ptr->no_analog_blocks - 1;
			}
		}
	}
	block_ptr->priority = aux_ptr->priority;
	block_ptr->index = 0;



/*

	if( aux_ptr->no_analog_blocks && aux_ptr->no_analog_blocks != 1 )
	{
		ptr.pmb = mon_block + aux_ptr->current_analog_block;

		if( ptr.pmb->first_block && ptr.pmb->last_block)

		{
			ptr.pmb->first_block = 0;
		}

	}
*/

}

void init_new_digital_block( int mon_number, Str_monitor_point *mon_ptr,Monitor_Block *block_ptr )
{
	memcpy( (void*)block_ptr, (void*)(mon_ptr->inputs +
						mon_ptr->an_inputs), (mon_ptr->num_inputs -
						 mon_ptr->an_inputs)*sizeof(Point_Net)  );
	block_ptr->monitor = mon_number;
	block_ptr->no_points = mon_ptr->num_inputs - mon_ptr->an_inputs;

}

void get_new_digital_block( int mon_number, int init,Str_monitor_point *mon_ptr  )
{
	int j, reseted;
	Str_points_ptr ptr;
	register Monitor_Block *block_ptr;
	Mon_aux  *aux_ptr;

	reseted = 0;
	aux_ptr = mon_aux + mon_number;
	if( aux_ptr->no_digital_blocks )
		ptr.pmb = mon_block + aux_ptr->current_digital_block;
	else
		ptr.pmb = NULL;

	j = find_new_block( mon_number, 1 );
	if( j >= 0 )
	{ /* there is a new block available */
		/* initialize the new block */
		block_ptr = mon_block + j;
		memset( (void*)block_ptr, 0, sizeof(Monitor_Block_Header) );
		if( !init ) /* copy the header from the previous block */
		{
			memcpy( (void*)block_ptr, (void*)ptr.pmb, 75 );	/* 71 = inputs + monitor + no_points */
		}
		else
		{
			init_new_digital_block( mon_number, mon_ptr, block_ptr );
		}
		/* update the last block if there is one */
		if( ptr.pmb )
		{
			ptr.pmb->last_block = 0;
			ptr.pmb->next_block = j;
		}
		aux_ptr->current_digital_block = j;
		if( !aux_ptr->no_digital_blocks )
		{
			aux_ptr->first_digital_block = j;
			block_ptr->first_block = 1;
		}
		else
			block_ptr->first_block = 0;

		block_ptr->block_no = aux_ptr->no_digital_blocks;
		aux_ptr->no_digital_blocks++;

		block_ptr->next_block = 255;
		block_ptr->last_block = 1;
		block_ptr->analog_digital = 1;
		block_ptr->wrap_around = 0;
	}
	else
	{ /* there are no more new block available - will wrap starting
			 with the first block if there is one */
		if( !aux_ptr->no_digital_blocks )
			return;
		if( aux_ptr->no_digital_blocks >= 1 )
		{
			/* update the last block (there is more then one block in chain ) */
			ptr.pmb->first_block = 0;
			ptr.pmb->last_block = 0;
			block_ptr = mon_block + aux_ptr->first_digital_block;
			update_blocks_number( block_ptr );
			ptr.pmb->next_block = aux_ptr->first_digital_block;
			aux_ptr->current_digital_block = aux_ptr->first_digital_block;
			aux_ptr->first_digital_block = block_ptr->next_block;
			block_ptr->block_no = aux_ptr->no_digital_blocks - 1;

/*			ptr.pmb->next_block = 255;*/
		}
		if( init )
		{ /* the user sent a monitor's definition that differs of the
				 previous one and the server required a new block */
			if( memcmp( (void*)ptr.pmb->inputs,(void*)(mon_ptr->inputs+mon_ptr->an_inputs),ptr.pmb->no_points*sizeof(Point_Net) ) )
			{ /* points' definition sent differs of that in this block;
					 the block will be initialized. We'll lose the data
					 previously stored in this block */
/*
				if( aux_ptr->no_digital_blocks > 1 )
				{
					update_blocks_number( block_ptr );
					aux_ptr->first_digital_block = block_ptr->next_block;
				}
*/
				memset( (void*)block_ptr, 0, sizeof(Monitor_Block_Header) );
				init_new_digital_block( mon_number, mon_ptr, block_ptr );
				reseted = 1;
			}
		}
		else
		{  /* the last block is full, there are no other blocks available so use
					the first one. Compare the headers of the last and first block (
					only inputs, no_points and monitor fields ) */
			if( memcmp( (void*)ptr.pmb, (void*)block_ptr, 71 ) )
			{ /* headers are different; data stored in the first block is lost;
					 copy the  header of the last block in the this block */
				memcpy( (void*)block_ptr, (void*)ptr.pmb, 74 );
				block_ptr->start_time = (time_since_1970 + timestart);	
				reseted = 1;
			}
			else
			{ /* the same headers so just wrap arround */
/*				block_ptr->wrap_around = 1;*/
				reseted = 1;
				block_ptr->last_block = 1;
			}
		}
		if( reseted )
		{
			block_ptr->next_block = 255;
			block_ptr->first_block = 0;
			block_ptr->last_block = 1;
			block_ptr->analog_digital = 1;
			block_ptr->block_no = aux_ptr->no_digital_blocks - 1;
		}
	}
	aux_ptr->no_of_digital_points = mon_ptr->num_inputs - mon_ptr->an_inputs;
	block_ptr->priority = aux_ptr->priority;
	block_ptr->index = 0;
}


Byte get_input_sample( int number )
{
	static U16_T count1 = 0;
	static U16_T count2 = 0;
/*	Byte mask;
	int input_number, index;

	input_number = 0;
#ifdef MINI64
	index = 0;
	mask = 1;
	while( !( mask & channel_desc ) )
	{
		mask <<= 1;
		index += 8;
	}
	while( ( input_number + 8 ) < number )
	{
		while( !( mask & channel_desc ) )
		{
			mask <<= 1;
			index += 8;
		}
		input_number += 8;
	}	 
#endif	 */
// for test now 
//	return in_out_data[index+(number-input_number)];  
	if(number == 0)	 {Test[21]++; return count1++;}
	else if(number == 1)	{Test[22]++; count2 = count2 + 2; return count2;}
	//return (AI_Value[number]);
}

/*=========================================================================*/
void sample_analog_points( Str_monitor_point *mon_ptr, Mon_aux  *aux_ptr )
{
	int j;//,z,A,B,C,D;
	Ulong time;
	Str_points_ptr ptr;
	register Monitor_Block *block_ptr;

	block_ptr = mon_block + aux_ptr->current_analog_block;


	time = 3600L * mon_ptr->hour_interval_time;
	time += 60 * mon_ptr->minute_interval_time;
	time += mon_ptr->second_interval_time;
	aux_ptr->next_sample_time += time;
//	aux_ptr->next_sample_time = time_since_1970 + timestart;	  
	if( block_ptr->wrap_around )
		block_ptr->start_time += time;
//	Test[30] = time;
//	Test[28] = aux_ptr->next_sample_time >> 16;
//	Test[29] = aux_ptr->next_sample_time;
	for( j = 0; j < block_ptr->no_points;j++ )
	{
	/*	if( mon_ptr->double_flag )
		{
			get_net_point_value( block_ptr->inputs + j,block_ptr->dat.analog+block_ptr->index );
			block_ptr->index++;
			if( block_ptr->wrap_around )
				block_ptr->start_time += time;
		}
		else */
		{
			ptr.pnet = block_ptr->inputs + j;
			if( ptr.pnet->point_type == IN && ptr.pnet->panel == Station_NUM )
			{	  
				if( !block_ptr->index )
					block_ptr->start_time = (time_since_1970 + timestart);  
				//block_ptr->dat.raw_byte[block_ptr->index] = get_input_sample( ptr.pnet->number );
				block_ptr->dat.analog[block_ptr->index] = DoulbemGetPointWord2(get_input_sample( ptr.pnet->number ));
				block_ptr->index++;				
			}
		}
	}
	/*if( mon_ptr->double_flag )
		j = block_ptr->index + block_ptr->no_points - MAX_ANALOG_SAMPLES_PER_BLOCK;
	else */
	{
		j = block_ptr->index + block_ptr->no_points;
	
		j -= ( MAX_ANALOG_SAMPLES_PER_BLOCK << 2 );		// ?????????????????
	}
	if( j > 0 )
	{
		if( block_ptr->wrap_around )
		{
			if( aux_ptr->no_analog_blocks > 1 )
			{
				block_ptr->first_block = 0;
				block_ptr->last_block = 1;
				update_blocks_number( block_ptr );
				aux_ptr->first_analog_block = block_ptr->next_block;
				block_ptr->block_no = aux_ptr->no_analog_blocks - 1;
				block_ptr->next_block = 255;
			}
			block_ptr->wrap_around = 0;
		}
		get_new_analog_block( block_ptr->monitor, 0, mon_ptr );

	}

}


void sample_digital_points( Str_monitor_point *mon_ptr, Mon_aux *aux_ptr )
{
	int j;//,A,B;
	Ulong value;
	Digital_sample *dsample_ptr;
	register Monitor_Block *block_ptr;

	block_ptr = mon_block + aux_ptr->current_digital_block;
	if( !block_ptr->index )
		block_ptr->start_time = (time_since_1970 +timestart);
	Test[43] = block_ptr->no_points;	
	for( j=0; j<block_ptr->no_points; j++ )
	{
		get_net_point_value( block_ptr->inputs + j, &value );
		if( 1 << j & block_ptr->last_digital_state )	//   ????????????????
		{
			if( !value )
			{
				dsample_ptr = (block_ptr->dat.digital + block_ptr->index);	// store digtal sample
				dsample_ptr->time = (time_since_1970 +timestart);	 
				dsample_ptr->pointno_and_value = j + 0 << 7;
			//	dsample_ptr->value = 0;
				block_ptr->last_digital_state &= ~(1<<j);
				if( !block_ptr->index )
					block_ptr->start_time = (time_since_1970 +timestart);	
				block_ptr->index++;
			}
		}
		else
		{
			if( value )
			{
				dsample_ptr = (block_ptr->dat.digital+block_ptr->index);
				dsample_ptr->time = (time_since_1970 +timestart);
				dsample_ptr->pointno_and_value = j + 1 << 7;
			//	dsample_ptr->value = 1;
				block_ptr->last_digital_state |= (1<<j);
				if( !block_ptr->index )
					block_ptr->start_time = (time_since_1970 +timestart);	
				block_ptr->index++;
			}
    }
  }
	if( block_ptr->index + block_ptr->no_points > MAX_DIGITAL_SAMPLES_PER_BLOCK )
  	{
		if( block_ptr->wrap_around )
		{
		  if( aux_ptr->no_digital_blocks > 1 )
		  {
				block_ptr->first_block = 0;
				block_ptr->last_block = 1;
				update_blocks_number( block_ptr );
				aux_ptr->first_digital_block = block_ptr->next_block;
				block_ptr->block_no = aux_ptr->no_digital_blocks - 1;
				block_ptr->next_block = 255;
			}
			block_ptr->wrap_around = 0;
		}
		get_new_digital_block( block_ptr->monitor, 0, mon_ptr  );
	}


}

/*======================================================================*/
void sample_points( void  )
{
	int i;
	Mon_aux  *aux_ptr;
	Str_monitor_point *mon_ptr;

	mon_ptr = monitors;
	aux_ptr = mon_aux;
	for( i=0; i<MAX_MONITORS; i++, mon_ptr++, aux_ptr++ )
	{
		if(mon_ptr->status)
		{
			if( mon_ptr->an_inputs )
			{
//				Test[35] = aux_ptr->next_sample_time >> 16;
//				Test[36] = aux_ptr->next_sample_time;
//				Test[37] = (time_since_1970 + timestart) >> 16;
//				Test[38] = (time_since_1970 + timestart);

				if( aux_ptr->next_sample_time <= (time_since_1970 + timestart) )
				{
        //  set_semaphore(&monitor_flag);
					sample_analog_points( mon_ptr, aux_ptr );
        //  clear_semaphore(&monitor_flag);
				}
			}
			if( aux_ptr->no_of_digital_points )
			{
      //  set_semaphore(&monitor_flag);
				sample_digital_points( mon_ptr, aux_ptr );
       // clear_semaphore(&monitor_flag);
			}
		}

	}
}

Ulong get_sample_time( Monitor_Block *ptr )
{
	Ulong time;

	time = ptr->second_interval_time;
	time += 60*ptr->minute_interval_time;
	time += (Ulong)3600*ptr->hour_interval_time;
	return time;
}

Ulong get_time_of_last_sample( Monitor_Block *ptr )
{
	Ulong time;

	if( !ptr->analog_digital )
	{
		time = ptr->second_interval_time;
		time += 60*ptr->minute_interval_time;
		time += (Ulong)3600*ptr->hour_interval_time;
		time *= ( ptr->index / ptr->no_points );
		time += ptr->start_time; /* this the time of last sample */
	}
  else
	{
		time = ptr->dat.digital[ptr->index-1].time;
	}
  return time;
}

int get_index_at_time( Monitor_Block *ptr, Ulong time )
{
	int index;
	Ulong sample_time;


	if( !ptr->analog_digital )
	{
		sample_time = get_sample_time( ptr );
		index = (int )( ( time - ptr->start_time ) / sample_time ) +
						(((time-ptr->start_time)%sample_time)?1:0);
		index *= ptr->no_points;
	}
	else
	{
		for( index=0; index<ptr->index; index++ )
			if( time >= ptr->dat.digital[index].time )
				break;

	}
	return index;
}

int get_sample_size( Monitor_Block *ptr )
{
	if( ptr->analog_digital )
	{
			return 5;
	}
	else
	{
			if( !ptr->fast_sampling )
				return 4;
			else
				return 1;
	}
	return 0;
}

int get_data_size_from_index( Monitor_Block *ptr, int index )
{
	int size;

	size = ptr->index - index;
	size *= get_sample_size( ptr );
	return size;
}

int get_data_size_till_index( Monitor_Block *ptr, int index )
{
	int size;

	size = index;
	size *= get_sample_size( ptr );
	return size;
}

int get_data_size_from_time( Monitor_Block *ptr, Ulong time, int *position )
{
	int size, index;

	size = 0;
	index =  get_index_at_time( ptr, time );
	if( index < ptr->index )
	{
		size = ptr->index - index;
		size *= get_sample_size( ptr );
	}
	*position = index;
	return size;
}

int get_data_size_till_time( Monitor_Block *ptr, Ulong time, int *position )
{ /* returns the size of data from the beginning of the block till "time" */

	int size, index;

	size = 0;
	index =  get_index_at_time( ptr, time );

	if( index <= ptr->index )
	{
		size = index * get_sample_size( ptr );
	}
	*position = index;
	return size;
}

Ulong get_time_at_index( Monitor_Block *ptr, int index )
{
	Ulong time;

	if( !ptr->analog_digital )
	{
		time = get_sample_time( ptr );			
		time *= index;
		time += ptr->start_time;
		return time;
	}
	else
	{
		if( index < ptr->index )
			return ptr->dat.digital[index].time;
	}
}


/*=======================================================================*/
void scan_monitor_blocks( Mon_Data *PTRtable, Monitor_Block *ptr,int max_blocks, int order )
/*=======================================================================*/
{
	int i, first_block, last_block, first_block_in_chain;
	int start_index, end_index;
	int 	scan_size = 0;
	int number_of_blocks = 0;
	int current_index =0;

	Byte k = 0;
	Ulong time, length;

//	Str_points_ptr ptr2; 

	length = 0; /* length tells total size of header and data for this
													 monitor up to this moment */
	first_block = 0x0FF;
	last_block = 0x0FF;
	start_index = 0;
	end_index = 0;

	if( (PTRtable->comm_arg.monupdate.most_recent_time > ptr->start_time) && max_blocks)
	{
		for( i = 0; i< max_blocks; i++ )
		{
			time = get_time_of_last_sample( ptr );
			if( first_block == 0x0FF )
			{
				Test[35] = time >> 16;
				Test[36] = time;
				Test[37] = PTRtable->comm_arg.monupdate.oldest_time >> 16;
				Test[38] = PTRtable->comm_arg.monupdate.oldest_time;
				if( time >= PTRtable->comm_arg.monupdate.oldest_time )
				{ /* start with this block */
					first_block =  ( (char*)ptr - (char*)mon_block ) / sizeof( Monitor_Block );
					first_block_in_chain = ptr->block_no;
					length += sizeof(Monitor_Block_Header);
					length += get_data_size_from_time( ptr,
											 PTRtable->comm_arg.monupdate.oldest_time, &start_index );
				}
			}
			/* first_block can be changed in the above paragraph so has to be
																													checked again  */
			if( first_block != 0x0FF )
			{
/*				time = get_time_of_last_sample( ptr );*/
				if( time >= PTRtable->comm_arg.monupdate.most_recent_time )
				{
					last_block = ( (char*)ptr - (char*)mon_block ) / sizeof(Monitor_Block);
					/* last_block points to the block in which is the sample taken
						 at the closest time to the time indicated in the command */
					if( last_block != first_block )
					{  /* this block is not the same with the first block */
						length += sizeof(Monitor_Block_Header);
						length += get_data_size_till_time( ptr,
									 PTRtable->comm_arg.monupdate.most_recent_time, &end_index );
					}
					else
					{  /* the first block is also the last block so substract the
								data that was sampled after the most_recent_time */
						length -= get_data_size_from_time( ptr,
								 PTRtable->comm_arg.monupdate.most_recent_time, &end_index );
					}
					break;
				}
				else
				{
					if( ptr->block_no != first_block_in_chain )
					{  /* this block is not the same with the first block */
						length += sizeof(Monitor_Block_Header);
						length += get_data_size_till_time( ptr, time, &end_index );
					}
				}
			}
			if( !ptr->last_block && ptr->next_block != 0x0FF )
				ptr = ( mon_block + ptr->next_block );
			else
			{
				last_block = ( (char*)ptr - (char*)mon_block ) / sizeof( Monitor_Block );
				end_index = ptr->index;
				break;
			}
		}
	 if( order == SELECT )
	 {
//	 	Test[20] = PTRtable->comm_arg.monupdate.size;
//		Test[21] = sizeof( Monitor_Block );
		if( PTRtable->comm_arg.monupdate.size <= sizeof( Monitor_Block ) )
		{
	//		Test[22]++;
			length = 0;
		}
		else
		{
			/* "first_block" points to the first block which will be sent.
					It is chosen such that the total size of the data sent is less
					than the size of the buffer indicated in the command. */

			ptr = mon_block + first_block;
			i = first_block;
			if( length > PTRtable->comm_arg.monupdate.size )
			{ /* total length of the samples for the monitor exced the length
				 of the requester's buffer */
				while( length > PTRtable->comm_arg.monupdate.size )
				{ /* calculate curent difference */
					first_block_in_chain = 0x0FF;
					max_blocks = length - PTRtable->comm_arg.monupdate.size;
					if( i != last_block )
					{
						if( max_blocks >= get_data_size_from_index( ptr, start_index ) )
						{
							first_block_in_chain = 0;
						}
					}
					else
					{
						if( max_blocks >= (end_index-start_index)*get_sample_size( ptr ) )
						{
							first_block_in_chain = 0;
						}
					}
					if( !first_block_in_chain )
					{ /* this cannot be the first block */
						length -= sizeof(Monitor_Block_Header);
						length -= get_data_size_from_index( ptr, start_index );
						if( !ptr->last_block && ( i != last_block ) )
						{
							first_block = ptr->next_block;
							ptr = mon_block + first_block;
							start_index = 0;
							first_block_in_chain = 0x0FF;
						}
					}
					else
					{ /* just reduce the no of sample taken from this block and it's OK */
						max_blocks /= get_sample_size( ptr );
						max_blocks /= ptr->no_points;
						max_blocks++;
						max_blocks *= ptr->no_points;
						if( i != last_block )
						{
							if( max_blocks <= ptr->index )
								start_index = ptr->index - max_blocks;
							else
								start_index = ptr->index;
						}
						else
						{
							if( end_index > max_blocks )
								start_index = end_index - max_blocks;
							else
								start_index = end_index;
						}
						length -= start_index*get_sample_size( ptr );
						break;
					}
				}
			}
		}
	 }
	}

	if( order == SELECT )
	{
		PTRtable->length = length;
		PTRtable->para.mondata.first_block = first_block;
		PTRtable->para.mondata.last_block = last_block;
		PTRtable->para.mondata.start_index = start_index;
		PTRtable->para.mondata.end_index = end_index;
		PTRtable->para.mondata.header = 0;
		PTRtable->para.mondata.dat= 0;

//		Test[40] = PTRtable->para.mondata.first_block;
		Test[41] = PTRtable->para.mondata.last_block;
		Test[42] = PTRtable->para.mondata.start_index;
		Test[43] = PTRtable->para.mondata.end_index;
		Test[44] = PTRtable->length;

	}

	if( order == SCAN )
	{
	 	PTRtable->para.montimes.size = length;
	}
}

void send_monitor_frame( Mon_Data *PTRtable, Monitor_Block *ptr )
{
	int space, index, max_frame_length, last_block;
	Ulong time, length;

	PTRtable->length_asdu = 0;
	/* last_block indicates that "ptr" points to the last block to be sent */
	if( PTRtable->para.mondata.first_block == PTRtable->para.mondata.last_block )
		last_block = 1;
	else
		last_block = 0;
	if( PTRtable->length < 480 )
	{
		max_frame_length = PTRtable->length;
		last_block = 1;
	}
	else
		max_frame_length = 480;	

	ptr = ( mon_block + PTRtable->para.mondata.first_block );

	while(  PTRtable->length_asdu < max_frame_length )
	{
		if( PTRtable->para.mondata.header == 0 ) /* whole block's header needs to be sent */
	 /* this can happen anywhere within a frame */
		{
			
			space = max_frame_length - PTRtable->length_asdu;
			if( space >  sizeof(Monitor_Block_Header) )
			{ /* there is enaugh room for the whole header */				
				memcpy( PTRtable->asdu + PTRtable->length_asdu, (void*)ptr, sizeof(Monitor_Block_Header) );
				PTRtable->length_asdu += sizeof(Monitor_Block_Header);
				PTRtable->para.mondata.header = 0x0FF;
			}
			else
			{
				PTRtable->para.mondata.last_data = (char*)ptr;
				/* this points to the next location to be copied */
	/*				PTRtable->entitysize = sizeof(Monitor_Block_Header) - 0;*/
				/* indicates the length of the header left to be copied */
				PTRtable->para.mondata.header = 0;
	
				break;
	
			}
		}
		if( space >= ( sizeof(Monitor_Block_Header) - 6 ) )
		{ /* sizeof(Monitor_Block_Header) - 6 = offset of the "index" in the block's header */
			index = ptr->index; /* it will be changed in case this is the first	or last block */
			

			if( PTRtable->special == 1 ) /* first block to be sent */
			{
				if( last_block )
					/* update index of first and last block - here index will
						 indicate the number of sample between oldest_time and
						 most_recent_time */
					index = PTRtable->para.mondata.end_index - PTRtable->para.mondata.start_index;
				else
					/* update index of first block */
					index = ptr->index - PTRtable->para.mondata.start_index;
				Test[3] = ptr->index;
			}
			else
			{	
				if( last_block )
					/* update index of last block - index tell the number of samples
						 in this block before "most_recent_time" */
					index = PTRtable->para.mondata.end_index;
				
			}
	//		Test[20] = index;
			index = mGetPointWord2(index);
			memcpy( PTRtable->asdu + PTRtable->length_asdu-6, &index, 2 );
		}
		/* update start_time of first block */
		if( space >= ( sizeof(Monitor_Block_Header) - 10 ) )
		{
			if( PTRtable->special == 1 )
			{
				time = get_time_at_index( ptr, PTRtable->para.mondata.start_index );
				Test[17] = time >> 16;
				Test[18] = time;
				time = DoulbemGetPointWord2(time);
				
				memcpy( PTRtable->asdu + PTRtable->length_asdu-10, &time, 4 );
			}
		
		}

		if( PTRtable->para.mondata.dat == 0 )
		{	//Test[7] = PTRtable->length_asdu;
			space = max_frame_length - PTRtable->length_asdu;
			//Test[8] = space;
			if( PTRtable->special == 1 ) /* first block to send */
			{
				
				PTRtable->entitysize = get_sample_size( ptr ) * ( ptr->index - PTRtable->para.mondata.start_index );
				PTRtable->para.mondata.last_data = (char*)(ptr->dat.raw_byte) +	(PTRtable->para.mondata.start_index * get_sample_size( ptr ) );
			}
			else
			{
				if( last_block )
				{
					PTRtable->entitysize = get_sample_size( ptr ) *	PTRtable->para.mondata.end_index;
				}
				else
				{
					PTRtable->entitysize = get_sample_size( ptr ) * ptr->index;
				}
				PTRtable->para.mondata.last_data = (char*)ptr->dat.raw_byte;
			}
			if( space >= PTRtable->entitysize )
			{
//				Test[9] = PTRtable->entitysize;
//				Test[10] = PTRtable->length_asdu;
				memcpy( PTRtable->asdu+PTRtable->length_asdu,PTRtable->para.mondata.last_data, PTRtable->entitysize );
				PTRtable->length_asdu += PTRtable->entitysize;
				PTRtable->para.mondata.header = 0; /* both header and data of the */
				PTRtable->para.mondata.dat = 0;   /* new block have to be sent   */
				if( !ptr->last_block )
				{
					PTRtable->para.mondata.first_block = ptr->next_block;
					ptr = ( mon_block + ptr->next_block );
					PTRtable->para.mondata.start_index = 0;
					if( PTRtable->para.mondata.first_block == PTRtable->para.mondata.last_block )
						last_block = 1;
					if( ptr->last_block )
						last_block = 1;
					PTRtable->special++;
				}
				else
				{  
					break;
				}
			}
			else
			{			
				memcpy( PTRtable->asdu + PTRtable->length_asdu,PTRtable->para.mondata.last_data, space );
				PTRtable->length_asdu += space;
				PTRtable->entitysize -= space;
				PTRtable->para.mondata.last_data += space;
				PTRtable->para.mondata.dat = 1;  /* the rest of the data will be sent in the next frame  */
				break;
			}
		}

		if( PTRtable->para.mondata.dat == 1 )
		{//	Test[8]++;
			space = max_frame_length - PTRtable->length_asdu;
			if( space >= PTRtable->entitysize )
			{//	Test[9]++;
				memcpy( PTRtable->asdu+PTRtable->length_asdu,(void*)PTRtable->para.mondata.last_data, PTRtable->entitysize );
				PTRtable->length_asdu += PTRtable->entitysize;
				PTRtable->entitysize = 0;
				PTRtable->para.mondata.header = 0; /* both header and data of the */
				PTRtable->para.mondata.dat = 0;   /* new block have to be sent */
				if( !ptr->last_block )
				{
					PTRtable->para.mondata.first_block = ptr->next_block;
					ptr = ( mon_block + ptr->next_block );
					PTRtable->para.mondata.start_index = 0;
					if( PTRtable->para.mondata.first_block == PTRtable->para.mondata.last_block )
						last_block = 1;
					if( ptr->last_block )
						last_block = 1;
				}
				else
					break;
			}
			else
			{
				memcpy( PTRtable->asdu+PTRtable->length_asdu,PTRtable->para.mondata.last_data, space );
			//	Test[10]++;
				PTRtable->length_asdu += space;
				PTRtable->entitysize -= space;
				PTRtable->para.mondata.last_data += space;
/*				PTRtable->para.mondata.data = 1; */
			}
		}
	}
	PTRtable->special++; 

}

void ReadMonitor( Mon_Data *PTRtable) 
{
//		Point point;
	Str_points_ptr ptr;
	Str_points_ptr ptr2;


	ptr2.pmaux = mon_aux + (PTRtable->index);
//	Test[11] = PTRtable->sample_type;

	PTRtable->sample_type = ANALOGDATA;
//	if( !PTRtable->special )
	{
		switch( PTRtable->sample_type )
		{
			case ANALOGDATA:
			{
				if( ptr2.pmaux->first_analog_block != 0x0FF )
				{
					ptr.pmb = mon_block + ptr2.pmaux->first_analog_block;
					memcpy( &PTRtable->comm_arg.monupdate.size, PTRtable->comm_arg.string, 4 );
					memcpy( &PTRtable->comm_arg.monupdate.oldest_time,PTRtable->comm_arg.string + 4, 4 );
					memcpy( &PTRtable->comm_arg.monupdate.most_recent_time,PTRtable->comm_arg.string + 8, 4 );
					//

//					Test[12] = PTRtable->comm_arg.monupdate.most_recent_time >> 16;
//					Test[13] = PTRtable->comm_arg.monupdate.most_recent_time;
//					Test[14] = PTRtable->comm_arg.monupdate.oldest_time >> 16;
//					Test[15] = PTRtable->comm_arg.monupdate.oldest_time;

					PTRtable->comm_arg.monupdate.size = 1000;
					PTRtable->comm_arg.monupdate.most_recent_time = time_since_1970 + 3600;
					PTRtable->comm_arg.monupdate.oldest_time = ptr.pmb->start_time;	  

				//	Test[16] = PTRtable->comm_arg.monupdate.size >> 16;
				//	Test[17] = PTRtable->comm_arg.monupdate.size;
					
					scan_monitor_blocks( PTRtable, ptr.pmb,ptr2.pmaux->no_analog_blocks, SELECT );

					PTRtable->special = 1;
				}
				break;
			}
			case DIGITALDATA:
			{
				if( ptr2.pmaux->first_digital_block != 0x0FF )
				{
					ptr.pmb = mon_block + ptr2.pmaux->first_digital_block;
					memcpy( &PTRtable->comm_arg.monupdate.size, PTRtable->comm_arg.string, 4 );
					memcpy( &PTRtable->comm_arg.monupdate.most_recent_time,PTRtable->comm_arg.string + 4, 4 );
					PTRtable->comm_arg.monupdate.oldest_time = ptr.pmb->start_time;
					scan_monitor_blocks( PTRtable, ptr.pmb,	ptr2.pmaux->no_digital_blocks, SELECT );
					PTRtable->special = 1;
				}
				break;
			}
			case DIGITALBUFSIZE:
			{
				if( ptr2.pmaux->first_digital_block != 0x0FF )
				{
					ptr.pmb = mon_block + ptr2.pmaux->first_digital_block;
					memcpy( &PTRtable->comm_arg.monupdate.oldest_time,PTRtable->comm_arg.string, 8 );
					scan_monitor_blocks( PTRtable, ptr.pmb, ptr2.pmaux->no_digital_blocks, SCAN );
				}
//				PTRtable->add_para = 1;
//				PTRtable->para_length = 4;
				break;
			}
		}
	}
//	else
	{
		send_monitor_frame( PTRtable, ptr.pmb );
	}
}



void UpdateMonitor( Mon_Data *PTRtable)
{
	Str_points_ptr ptr;
	Str_points_ptr ptr2;

	ptr2.pmaux = mon_aux + ( PTRtable->index );
//	Test[11] = PTRtable->special;
//	if( !PTRtable->special )
	{
		switch( PTRtable->sample_type )
		{
			case ANALOGDATA:
			{
				memcpy( &PTRtable->comm_arg.monupdate.size, PTRtable->comm_arg.string, sizeof(MonitorUpdateData) );
				if( ptr2.pmaux->first_analog_block != 0x0FF )
				{
					ptr.pmb = mon_block+ptr2.pmaux->first_analog_block;
					scan_monitor_blocks( PTRtable, ptr.pmb,	ptr2.pmaux->no_analog_blocks, SELECT );
					PTRtable->special = 1;
				}
				break;
			}
			case DIGITALDATA:
			{
				memcpy( &PTRtable->comm_arg.monupdate.size, PTRtable->comm_arg.string, sizeof(MonitorUpdateData) );
				if( ptr2.pmaux->first_digital_block != 0x0FF )
				{
					ptr.pmb = mon_block+ptr2.pmaux->first_digital_block;
					scan_monitor_blocks( PTRtable, ptr.pmb,	ptr2.pmaux->no_digital_blocks, SELECT );
					PTRtable->special = 1;
				}
				break;
			}
			case ANALOG_TIMES_SIZE:
			{
				if( ptr2.pmaux->first_analog_block != 0x0FF )
				{
					ptr.pmb = mon_block+ptr2.pmaux->first_analog_block;
					PTRtable->comm_arg.monupdate.most_recent_time = time_since_1970;
					PTRtable->comm_arg.monupdate.oldest_time = ptr.pmb->start_time;
					scan_monitor_blocks( PTRtable, ptr.pmb,ptr2.pmaux->no_analog_blocks, SCAN );
					PTRtable->para.montimes.most_recent_time = time_since_1970;
					PTRtable->para.montimes.oldest_time = ptr.pmb->start_time;
				}
//				PTRtable->add_para = 1;
//				PTRtable->para_length = 12;
				break;
			}
			case DIGITAL_TIMES_SIZE:
			{
				if( ptr2.pmaux->first_digital_block != 0x0FF )
				{
					ptr.pmb = mon_block+ptr2.pmaux->first_digital_block;
					PTRtable->comm_arg.monupdate.most_recent_time = time_since_1970;	
					PTRtable->comm_arg.monupdate.oldest_time = ptr.pmb->start_time;
					scan_monitor_blocks( PTRtable, ptr.pmb,ptr2.pmaux->no_digital_blocks, SCAN  );
					PTRtable->para.montimes.most_recent_time = time_since_1970;
					PTRtable->para.montimes.oldest_time = ptr.pmb->start_time;
				}
//				PTRtable->add_para = 1;
//				PTRtable->para_length = 12;
				break;
			}
		}
	}
//	else
	{
		send_monitor_frame( PTRtable, ptr.pmb );
	}

}


void dealwithMonitor(Str_user_data_header header)
{
	Str_points_ptr ptr;
	Str_points_ptr ptr2;
	U8_T bank;
	U8_T flag;
	U8_T no_points;
	U8_T j;


//	if( header.command > 100 )
	{

		ptr.pmon = (Str_monitor_point*)backup_monitors;
		ptr2.pmon = (Str_monitor_point*)monitors;
		for( bank = 0; bank < MAX_MONITORS/*sizeof(Str_monitor_point)*/; bank++, ptr.pmon++, ptr2.pmon++ )
		{
			flag = 0;
		
			/* compare the old definition with the new one except for the label */
			if( memcmp( ptr.pmon->inputs, ptr2.pmon->inputs, ( sizeof(Str_monitor_point) - 9 ) ) )
			{
			/* compare sample rate */
				if( memcmp( (void*)&ptr.pmon->second_interval_time,
							(void*)&ptr2.pmon->second_interval_time, 3 ) )

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
				get_new_analog_block( bank, 1, ptr2.pmon );
			}
			if( flag & 0x02 ) /* get a new digital block */
			{
				get_new_digital_block( bank, 1, ptr2.pmon );
			}
		}

	}

	for(j = header.point_start_instance;j <= header.point_end_instance;j++)
	 	memcpy(&backup_monitors[j],&monitors[j], sizeof(Str_monitor_point));  // record moinitor data

}





