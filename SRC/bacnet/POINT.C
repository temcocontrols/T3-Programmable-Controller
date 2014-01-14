#include "user_data.h"
#include "ud_str.h"
#include <string.h>


#define DIG1 100

extern U16_T far Test[50];

/*
 * ----------------------------------------------------------------------------
 * Function Name: rtrim
 * Purpose: retrim the text
 * Params:
 * Returns:
 * Note: this function is called in search_point funciton
 * ----------------------------------------------------------------------------
 */
S8_T *rtrim(S8_T *text)
{
	S16_T n,i;
	n=strlen(text);
	for (i=n-1;i>=0;i--)
			if (text[i]!=' ')
					break;
	text[i+1]='\0';
	return text;
}

#if 0
/*
 * ----------------------------------------------------------------------------
 * Function Name: search_point
 * Purpose: 
 * Params:
 * Returns:
 * Note: this function is called in SERVER.C(server_data())
 * ----------------------------------------------------------------------------
 */
U16_T search_point( U8_T *type, U8_T *number, S8_T *buff, Search_type order )
{
	U8_T i, j, no_of_points, desc;
	U16_T label_pos, name_length, label_length, end, length = 0;
	U16_T point_length;
	S8_T *data_pointer, *p_name, *p_label;

	if( order == DESCRIPTOR_POINT || order == LENGTH_POINT )
	{
		end = *type + 1;
	}
	else
	{
		end = 13;
	}
	for( i = *type; i < end; i++ )
	{
		name_length = 0;
		label_length = 0;
		label_pos = 0;
		desc = 0;
		no_of_points = info[i-1].max_points;
		data_pointer = (S8_T *) info[i-1].address;
		point_length = info[i-1].size;
		switch( i )
		{
			case OUTPUT:
			case INPUT:
			case VARIABLE:
			case WEEKLY_ROUTINE:
			case ANNUAL_ROUTINE:
			case PROGRAM:
			case CONTROL_GROUP:
      		case TOTALIZER:				label_pos = 21;				break;
			case TABLES:
			case ANALOG_MONITOR:
			case ARRAYS:				desc = 255;	 label_pos = 21; break;
			default:					continue;
		}

		for( j=*number; j<no_of_points; j++)
		{
			if( order == COMPARE )
			{
				if( !strcmp( buff, rtrim(data_pointer+j*point_length+label_pos) ) )
				{
					buff = data_pointer+j*point_length;
					*number = j;
					*type = i;
					return point_length;
				}
			}
			else
			{
				if( desc != 255 )
					name_length = 1+strlen( p_name=rtrim( data_pointer+j*point_length ) );
				if( label_pos != 255 )
					label_length = 1+strlen( p_label=rtrim( data_pointer+j*point_length+label_pos ) );
				length += ( name_length + label_length );
				if( order == ALL_DESCRIPTORS || order == DESCRIPTOR_POINT )
				{
					if( desc != 255 )
					{
						memcpy( buff, p_name, name_length );
						buff += name_length;
					}
					if( label_pos != 255 )
					{
						memcpy( buff, p_label, label_length );
						buff += label_length;
					}
					if( length > MAXASDUSIZE - 30 )
					{
						*type = i;
						*number = j+1;
						break;
					}
				}
			}
		}
	}
	if( order == COMPARE )
	{
		return 0;
	}
	else
		return length;
}


/*
 * ----------------------------------------------------------------------------
 * Function Name: update_alarm_tbl
 * Purpose: 
 * Params:
 * Returns:
 * Note: this function is called in SERVER.C(server_data())
 * ----------------------------------------------------------------------------
 */
void update_alarm_tbl(Alarm_point *block, S16_T max_points_bank )
{
	S16_T i,j;
	Alarm_point *bl;
	Str_points_ptr ptr;
	S8_T alarmtask;
	
	alarmtask=0;
	ptr.palrm = &alarms[0];
	for( j=0; j<MAX_ALARMS; j++, ptr.palrm++)
 	{
		if(	GetByteBit(ptr.palrm->flag1,alarm_alarm,1)/*ptr.palrm->alarm*/ )
		{
			if( GetByteBit(ptr.palrm->flag4,alarm_change_flag,2)/*ptr.palrm->change_flag*/ ) continue;
			 /*ptr.palrm->change_flag = 2;*/
			SetByteBit(&ptr.palrm->flag4,2,alarm_change_flag,2);
			bl = block;
			for( i=0; i<max_points_bank;i++, bl++ )
			{
				if(GetByteBit(bl->flag1,alarm_alarm,1)	/*bl->alarm */)
				{
				 	if( ptr.palrm->no == bl->no ) break;
				}
			}
			if( i>=max_points_bank )
			{
				SetByteBit(&ptr.palrm->flag4,0,alarm_change_flag,2);//ptr.palrm->change_flag=0;
				continue;
			}
			if(GetByteBit(bl->flag1,alarm_ddelete,1)/* bl->ddelete*/ )
			{
				if( !GetByteBit(ptr.palrm->flag1,alarm_restored,1)/*ptr.palrm->restored*/ && !GetByteBit(ptr.palrm->flag1,alarm_acknowledged,1)/*ptr.palrm->acknowledged*/ )
				{
					if(!ind_alarms--) ind_alarms=0;
				}
				SetByteBit(&ptr.palrm->flag1,0,alarm_alarm,1);//ptr.palrm->alarm        = 0;
				SetByteBit(&ptr.palrm->flag4,0,alarm_change_flag,2);//ptr.palrm->change_flag  = 0;
				SetByteBit(&ptr.palrm->flag1,0,alarm_restored,1);//ptr.palrm->restored     = 0;
				SetByteBit(&ptr.palrm->flag1,0,alarm_acknowledged,1);//ptr.palrm->acknowledged = 0;
				SetByteBit(&ptr.palrm->flag1,1,alarm_ddelete,1);//ptr.palrm->ddelete      = 1;
				SetByteBit(&ptr.palrm->flag4,0,alarm_original,1);//ptr.palrm->original     = 0;
				if(ptr.palrm->alarm_panel == Station_NUM)
				{
					SetByteBit(&ptr.palrm->flag4,0,alarm_where_state1,1);//ptr.palrm->where_state1 = 0;
					SetByteBit(&ptr.palrm->flag4,0,alarm_where_state2,1);//ptr.palrm->where_state2 = 0;
					SetByteBit(&ptr.palrm->flag4,0,alarm_where_state3,1);//ptr.palrm->where_state3 = 0;
					SetByteBit(&ptr.palrm->flag4,0,alarm_where_state4,1);//ptr.palrm->where_state4 = 0;
					SetByteBit(&ptr.palrm->flag4,0,alarm_where_state5,1);//ptr.palrm->where_state5 = 0;
				}
				alarmtask |= 0x02;
				continue;
			}
			if(GetByteBit(bl->flag1,alarm_acknowledged,1)/* bl->acknowledged*/ )
			{
				if( !GetByteBit(ptr.palrm->flag1,alarm_acknowledged,1)/*ptr.palrm->acknowledged*/ )
				{
					if( !GetByteBit(ptr.palrm->flag1,alarm_restored,1)/*ptr.palrm->restored*/ )
					{
						if(!ind_alarms--) ind_alarms=0;
					}
					SetByteBit(&ptr.palrm->flag1,1,alarm_acknowledged,1);//ptr.palrm->acknowledged = 1;
					SetByteBit(&ptr.palrm->flag4,0,alarm_original,1);//ptr.palrm->original     = 0;
					if(ptr.palrm->alarm_panel==Station_NUM)
					{
						SetByteBit(&ptr.palrm->flag4,0,alarm_where_state1,1);//ptr.palrm->where_state1 = 0;
						SetByteBit(&ptr.palrm->flag4,0,alarm_where_state2,1);//ptr.palrm->where_state2 = 0;
						SetByteBit(&ptr.palrm->flag4,0,alarm_where_state3,1);//ptr.palrm->where_state3 = 0;
						SetByteBit(&ptr.palrm->flag4,0,alarm_where_state4,1);//ptr.palrm->where_state4 = 0;
						SetByteBit(&ptr.palrm->flag4,0,alarm_where_state5,1);//ptr.palrm->where_state5 = 0;
					}
					alarmtask |= 0x01;
				}
			}
			SetByteBit(&ptr.palrm->flag4,0,alarm_change_flag,2);//ptr.palrm->change_flag=0;
		}
 	}
	if( alarmtask )
	{
		 new_alarm_flag |= 0x01;
		 if( alarmtask & 0x02 )
	     new_alarm_flag |= 0x02;
	}
}



/*
 * ----------------------------------------------------------------------------
 * Function Name: find_remote_point
 * Purpose: 
 * Params:
 * Returns:
 * Note: this function is called in POINT.C
 * ----------------------------------------------------------------------------
 */
S16_T find_remote_point( Point_Net *point )
{
	S16_T i;
	U8_T flag;
	REMOTE_POINTS *ptr;

	ptr = remote_points_list;

	if( point->network_number == 0x0FFFF )
	{
		point->network_number = panel_net_info.network_number;
		flag = 1;
	}
	else
		flag = 0;

	for( i=0; i<MAX_REMOTE_POINTS; i++, ptr++ )
	{
		if( ptr->count )
		{
			if( !memcmp( (void*)point, (void*)&ptr->point, sizeof(Point_Net) ) )
			{
				break;
			}
		}
	}
	if( flag )
		point->network_number = 0x0FFFF;
	if( i < MAX_REMOTE_POINTS )
		return i; // returns the index in the list 
	else
		return -1;
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: put_remote_point_value
 * Purpose: 
 * Params:
 * Returns:
 * Note:  this function is called in POINT.C
 * ----------------------------------------------------------------------------
 */
void put_remote_point_value( S16_T index, S32_T *val_ptr, S16_T prog_op )
{
	REMOTE_POINTS *ptr;

	ptr = (REMOTE_POINTS *)(&remote_points_list[0]) + index;

	if( ( !GetByteBit(ptr->flag1,REMOTE_auto_manual,1)/*ptr->auto_manual*/ && !prog_op ) || ( GetByteBit(ptr->flag1,REMOTE_auto_manual,1)/*ptr->auto_manual*/ && prog_op ) )
	{
		ptr->point_value = *val_ptr;
		//ptr->change = 1;
		SetByteBit(&ptr->flag2,1,REMOTE_change,2);
	}
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: insert_remote_point
 * Purpose: 
 * Params:
 * Returns:	returns the index in the list or -1 if there is no room for another point
 * Note: 
 * ----------------------------------------------------------------------------
 */
S16_T insert_remote_point( Point_Net *point, S16_T index )
{ 	
	S16_T i;
	REMOTE_POINTS *ptr;

	ptr = &remote_points_list[0];
	if( index < 0 )
	{ /* index < 0 means that the index is unknown */
		if( number_of_remote_points >= MAX_REMOTE_POINTS ) return -1;
		if( ( i = find_remote_point( point ) ) >= 0 )
		{ /* the point is in the list */
			(ptr+i)->count++;
			return i;
		}
		else
		{
			for( i=0; i<MAX_REMOTE_POINTS; i++, ptr++ )
			{
				if( !ptr->count )
				{
					number_of_remote_points++;
					memcpy( &ptr->point, point, sizeof(Point_Net) );
					if( point->network_number == 0x0FFFF )
						ptr->point.network_number = panel_net_info.network_number;
					ptr->count = 1;
					SetByteBit(&ptr->flag2,0,REMOTE_change,2);//ptr->change = 0;
					return i;
				}
			}
			return -1;
		}
	}
	else
	{
		ptr += index;
		ptr->count++;
		return index;
	}
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: delete_remote_point
 * Purpose: 
 * Params:
 * Returns:	returns the value of the count field
 *  		-1 means that the point was not in the list 
 *  		0 means that the point was deleted from the list
 * Note: this function is called in server.c
 * ----------------------------------------------------------------------------
 */
S16_T delete_remote_point( Point_Net *point, S16_T index )
{
	REMOTE_POINTS *ptr;
	ptr = &remote_points_list[0];
	if( index < 0 )
	{
		if( ( index = find_remote_point( point ) ) < 0 )
		{ /* the point is NOT in the list */
			return -1;
		}
	}
	ptr += index;
	ptr->count--;
	if( !ptr->count )
	{
		memset( ptr, 0, sizeof(REMOTE_POINTS) );
		number_of_remote_points--;
	}
	return ptr->count;
}

#endif

/*
 * ----------------------------------------------------------------------------
 * Function Name: get_point_value
 * Purpose: according to the point_type, get the value of the current point
 * Params:
 * Returns:
 * Note: 
 * ----------------------------------------------------------------------------
 */
S16_T get_point_value( Point *point, S32_T *val_ptr )
{
 	Str_points_ptr sptr;
 	if( ( OUTPUT <= point->point_type ) &&( point->point_type <= ARRAYS ) )
 	{
  		if( point->number < table_bank[point->point_type-1] )
  		{
			switch( point->point_type-1 )
			{
				case OUT:   Test[3] = 23;
					sptr.pout = &outputs[point->number];
					if(!sptr.pout->digital_analog) /* DIGITAL */
					{
						*val_ptr = sptr.pout->control? 1000L:0L;					
					}
					else
					{
						*val_ptr = sptr.pout->value;
					}
					break;
				case IN:
					Test[3] = 24;
					sptr.pin = &inputs[point->number];
					if( !sptr.pin->digital_analog)
					{
						*val_ptr = sptr.pin->control ? 1000L : 0L;
					}
					else
					{
						*val_ptr = sptr.pin->value;
					}
					break;
			 	case VAR: Test[3] = 25;
					sptr.pvar = &vars[point->number];
					if( !sptr.pvar->digital_analog)
					{
						*val_ptr = sptr.pvar->control ? 1000L : 0L;
					}
					else
					{
						*val_ptr = sptr.pvar->value;
					}
					break;
			 	case CON:  Test[3] = 26;
					*val_ptr = controllers[point->number].value;
					break;
				case WRT:	Test[3] = 27;
					*val_ptr = weekly_routines[point->number].value?1000L:0;
					break;
				case AR:
					*val_ptr = annual_routines[point->number].value?1000L:0;
					break;
				case PRG:
					*val_ptr = programs[point->number].on_off?1000L:0;
					break;
			/*	case AMON:
					*val_ptr = monitors[point->number].mon_status?1000L:0;
					value = GetByteBit(monitors[point->number].flag2,mon_status,1)?1000L:0;
					*val_ptr = DoulbemGetPointWord2(value);	
					break;
				
				case TZ:  
					*val_ptr = totalizers[point->number].active?1000L:0;
					break; */
				
			}
		return point->number;
  		}
 	}
/*
  point->point_type = 0;
  point->number = 0;
*/
	*val_ptr = 0;
	return -1;
}



/*
 * ----------------------------------------------------------------------------
 * Function Name: put_point_value
 * Purpose: according to the type of point, write the value to the Str_points_ptr
 * Params:
 * Returns:
 * Note:  
 * ----------------------------------------------------------------------------
 */
S16_T put_point_value( Point *point, S32_T *val_ptr, S16_T aux, S16_T prog_op )
{
 	Str_points_ptr sptr;
	U32_T temp;
/* write value to point	*/
// 	if( ( OUTPUT <= point->point_type ) &&	( point->point_type <= ARRAYS ) )
	{
		if( point->number < table_bank[point->point_type-1] )
		{
		 	switch( point->point_type-1 )
		 	{
			case OUT:
				sptr.pout = &outputs[point->number];
				sptr.pout->value = *val_ptr;
				if( (sptr.pout->auto_manual == 0) && (prog_op == 0) || (sptr.pout->auto_manual == 1) && (prog_op == 1) )
				{	 
					if( !sptr.pout->digital_analog ) 	// DIGITAL
					{	  
						sptr.pout->control = *val_ptr ? 1 : 0;
					}
					sptr.pout->value = DoulbemGetPointWord(*val_ptr); 					
				}
				break;
		  	case IN:
				sptr.pin = &inputs[point->number];
				if( (sptr.pin->auto_manual == 0) && (prog_op == 0) || (sptr.pin->auto_manual == 1) && (prog_op == 1) )
				{
					if( !sptr.pin->digital_analog )
					{
						 sptr.pin->control = *val_ptr ? 1 : 0;
					}
				//	sptr.pin->value = DoulbemGetPointWord(*val_ptr);
					temp = *val_ptr;
					sptr.pin->value = DoulbemGetPointWord2(temp);	
				}
				break;
		  	case VAR:
				sptr.pvar = &vars[point->number];
				if( (sptr.pvar->auto_manual == 0) && (prog_op == 0) ||(sptr.pvar->auto_manual == 1) && (prog_op == 1) )
				{	
							
					if(sptr.pvar->digital_analog == 0)
					{	
						sptr.pvar->control = *val_ptr ? 1 : 0;

					} 					
					temp = *val_ptr;
					sptr.pvar->value = DoulbemGetPointWord2(temp);				
				}
				break;
		  	case CON:
				sptr.pcon = &controllers[point->number];
				if( (sptr.pcon->auto_manual == 0) && (prog_op == 0) || (sptr.pcon->auto_manual == 1) && (prog_op == 1) )
				{		
				//	sptr.pcon->value = *val_ptr;
					temp = *val_ptr ? 1 : 0;
					sptr.pcon->value = DoulbemGetPointWord(temp); 
				}
				break;
		  	case WRT:
				sptr.pwr = &weekly_routines[point->number];
				if( (sptr.pwr->auto_manual == 0) && (prog_op == 0) || (sptr.pwr->auto_manual== 1) && (prog_op == 1) )		 
				{	
					temp = *val_ptr ? 1 : 0;
					sptr.pwr->value = DoulbemGetPointWord(temp); 
				}
				break;
		  	case AR:
				sptr.panr = &annual_routines[point->number];
				if( !sptr.panr->auto_manual && !prog_op || sptr.panr->auto_manual && prog_op )		 
				{
				//	sptr.panr->value = *val_ptr ? 1 : 0;
					temp = *val_ptr ? 1 : 0;
					sptr.pwr->value = DoulbemGetPointWord(temp); 
				}
				break;
		  	case PRG:
				sptr.pprg = &programs[point->number];
				if( (sptr.pprg->auto_manual == 0) && (prog_op == 0) || (sptr.pprg->auto_manual == 1) && (prog_op == 1) )
				{		 
				//	sptr.pprg->on_off = *val_ptr ? 1 : 0;
					temp = *val_ptr ? 1 : 0;
					sptr.pprg->on_off = DoulbemGetPointWord(temp); 
				}
				break;
		 /*	case ARRAY:
				sptr.pary = &arrays[point->number];
				if( aux >= sptr.pary->length || aux < 0 )				aux = 0;		
				arrays_address[point->number][aux] = *val_ptr;		
				break;
			  	case AMON:
				sptr.pmon = &monitors[point->number];
				sptr.pmon->status = *val_ptr ? 1 : 0;
				break;

		  case TOTAL:
				sptr.ptot = &totalizers[point->number];
				sptr.ptot->active = *val_ptr ? 1 : 0;
				break;
	*/
	/*			sptr.ptot->time_on = ptr->point_value; */
		 	}
	  	}
		return point->number;
 	}
	point->point_type = 0;
	point->number = 0;
	*val_ptr = 0;
	return -1;
}


#if 1
/*
 * ----------------------------------------------------------------------------
 * Function Name: put_net_point_value
 * Purpose: 
 * Params:
 * Returns:
 * Note:  it is not used now.
 * ----------------------------------------------------------------------------
 */
S16_T put_net_point_value( Point_Net *p, S32_T *val_ptr, S16_T aux, S16_T prog_op )
{
	S16_T index;
	Point_Net point;
	memcpy( &point, p, sizeof(Point_Net));
/*	if( point.network_number == 0x0FFFF ) point.network_number = panel_net_info.network_number;
	if( ( point.network_number != panel_net_info.network_number ) || ( point.panel != Station_NUM-1 ) )
	{
		if( ( index = find_remote_point( &point ) ) < 0 )
		{
			if( ( index = insert_remote_point( &point, -1 ) ) < 0 ) return -1;
		}
		put_remote_point_value( index, val_ptr, prog_op );
	}
	else*/
	{
		put_point_value( (Point *)p, val_ptr, aux, prog_op );
	}
	return 1;
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: get_net_point_value
 * Purpose: according to the network number of the networt point, get point value of the remote points,
 * Params:
 * Returns:
 * Note: 
 * ----------------------------------------------------------------------------
 */
S16_T get_net_point_value( Point_Net *p, S32_T *val_ptr )
{
	S16_T index;
	Point_Net point;
	S32_T tempval = 0;
	memcpy( &point, p, sizeof(Point_Net));
/*	if( point.network_number == 0x0FFFF ) point.network_number = panel_net_info.network_number;
	if(( point.network_number != panel_net_info.network_number ) ||( point.panel != Station_NUM-1 ))
	{
		if( ( index = find_remote_point( &point ) ) < 0 )
		{
			if( ( index = insert_remote_point( &point, -1 ) ) < 0 ) return -1;
		}
		*val_ptr = remote_points_list[index].point_value;
	}
	else*/
	{ /* modify by chelsea, value is S32_T, must change it */
	/*	get_point_value( (Point *)p, &tempval );
		val_ptr = DoulbemGetPointWord2(tempval); */
		get_point_value( (Point *)p, &val_ptr );
	}
}

#endif

/*
 * ----------------------------------------------------------------------------
 * Function Name: get_point_info
 * Purpose: 
 * Params:
 * Returns:
 * Note:   it is called back in sever.c
 * ----------------------------------------------------------------------------
 */
S16_T get_point_info( Point_info *ptr )
/* returns 1 if OK, 0 if point doesn't exist */
{
	Str_points_ptr sptr;
	S16_T absent = 1;
	bit tempbit;
	U8_T temp;
	ptr->decomisioned = 0;
  	if( ptr->number < table_bank[ptr->point_type-1] )
  	{
		ptr->point_value = 0;
		ptr->auto_manual = 0;
		ptr->digital_analog = 0;

		switch( ptr->point_type - 1)
		{
			case OUT:
				
				sptr.pout = &outputs[ptr->number];
				ptr->auto_manual = sptr.pout->auto_manual;
				ptr->digital_analog = sptr.pout->digital_analog;
				ptr->decomisioned = sptr.pout->decom;				
			
				ptr->units = sptr.pout->range;
	
				if( !sptr.pout->digital_analog ) // DIGITAL
				{
					ptr->point_value = sptr.pout->control? 1000L:0L;
					ptr->units -= DIG1;
				}
				else
				{
					ptr->point_value = sptr.pout->value;
				}     
				break;
			case IN:
				sptr.pin = &inputs[ptr->number];
			
				ptr->auto_manual = sptr.pin->auto_manual;
				ptr->digital_analog = sptr.pin->digital_analog;
				ptr->decomisioned = sptr.pin->decom;
				
				ptr->units = sptr.pin->range;
				if( !sptr.pin->digital_analog )
				{
					ptr->point_value = (( sptr.pin->control) ? 1000L : 0L);
					ptr->units -= DIG1;
					
				}
				else
				{
					ptr->point_value = sptr.pin->value;					
				}
				break;
			case VAR:
				sptr.pvar = &vars[ptr->number];
				ptr->auto_manual = sptr.pvar->auto_manual;
				ptr->digital_analog = sptr.pvar->digital_analog;
			
				ptr->units = sptr.pvar->range;

				if( !sptr.pvar->digital_analog )
				{
					ptr->point_value = sptr.pvar->control ? 1000L : 0L;
					ptr->units -= DIG1;
				}
				else
				{
					ptr->point_value = sptr.pvar->value;
				}
				break;
			case CON:
				sptr.pcon = &controllers[ptr->number];
				ptr->auto_manual = sptr.pcon->auto_manual;
				ptr->digital_analog = 1;
	
				ptr->units = procent;
				ptr->point_value = sptr.pcon->value;
				break;
/*
		case TOTAL:
				sptr.ptot = &totalizers[ptr->number];
				ptr->auto_manual = sptr.ptot->active;
				ptr->point_value = sptr.ptot->time_on;
				ptr->digital_analog = 1;
				ptr->units = time_unit;
				break;
*/
		case WRT:
				//Test[45]++;
				sptr.pwr = &weekly_routines[ptr->number];
				ptr->auto_manual = sptr.pwr->auto_manual;  
		
				ptr->point_value = sptr.pwr->value?1000L:0;
				Test[47] = ptr->point_value;
				ptr->units = 1;
				break;
		case AR:
			//	Test[46]++;
				sptr.panr = &annual_routines[ptr->number];
				ptr->auto_manual = sptr.panr->auto_manual; 
				ptr->point_value = sptr.panr->value?1000L:0;
			//	Test[48] = ptr->point_value;
				ptr->units = 1;
				break;
		case PRG:
				sptr.pprg = &programs[ptr->number];
				ptr->auto_manual = sptr.pprg->auto_manual;
				ptr->point_value = sptr.pprg->on_off?1000L:0;
				ptr->units = 1;
				break;
	/*	case AMON:
				sptr.pmon = &monitors[ptr->number];
				ptr->auto_manual = 0;
				ptr->point_value = sptr.pmon->status?1000L:0;
				ptr->digital_analog = 0;
				ptr->units = 1;
				break;
		case GRP: 
//				sptr.pgrp = &control_groups[ptr->number];
//				ptr->auto_manual = 0;
//				ptr->point_value = 0;
//				ptr->digital_analog = 0;
				ptr->units = 1;
				break;	*/
    	default:
				absent = 0;
//				ptr->point_value = 0;
//				ptr->auto_manual = 0;			
				ptr->digital_analog = 1;
				ptr->security = 0;
		}

  }
  else
  	absent = 0;
	return absent;
}

#if 0


/*
 * ----------------------------------------------------------------------------
 * Function Name: put_point_info
 * Purpose: 
 * Params:
 * Returns:
 * Note:   it is no used now.
 * ----------------------------------------------------------------------------
 */
S16_T put_point_info( Point_info *info )
{
	Str_points_ptr sptr;
	S16_T i;
	U8_T num_point;
	Point_Net point;

	memcpy( &point, info, sizeof(Point_Net));
	if( point.network_number == 0xFFFF )
		point.network_number = panel_net_info.network_number;
	num_point = info->number;
	if( point.network_number != panel_net_info.network_number )
	{   /*the element is NOT on the LOCAL_PANEL*/
		i = find_remote_point( (Point_Net *)info );
		if( i < 0 ) return -1;
		memcpy( (S8_T*)&remote_points_list[i].point, info, sizeof( Point_info ) );
	}
	else
	{ /* the element is ON the LOCAL_PANEL */
	  if( info->number < table_bank[info->point_type-1] )
    {
			switch( point.point_type - 1)
			{
			 case OUT:
				sptr.pout = &outputs[num_point];
				sptr.pout->auto_manual = info->auto_manual;
				sptr.pout->digital_analog = info->digital_analog;
				sptr.pout->decom = info->decomisioned;
				if( !sptr.pout->digital_analog ) // DIGITAL
				{
					sptr.pout->control = info->point_value ? 1 : 0;
				}
				sptr.pout->value = info->point_value;
				break;
			 case IN:
				sptr.pin = &inputs[num_point];
				sptr.pin->auto_manual = info->auto_manual;
				sptr.pin->digital_analog = info->digital_analog;
				sptr.pin->decom = info->decomisioned;
				if( !sptr.pin->digital_analog )
				{
					sptr.pin->control = info->point_value ? 1 : 0;
				}
				sptr.pin->value = info->point_value;
				break;
			 case VAR:
				sptr.pvar = &vars[num_point];
				sptr.pvar->auto_manual = info->auto_manual;
				sptr.pvar->digital_analog = info->digital_analog;
				if( !sptr.pvar->digital_analog )
				{
					sptr.pvar->control = info->point_value ? 1 : 0;
				}
				sptr.pvar->value = info->point_value;
				break;
			 case CON:
				sptr.pcon = &controllers[num_point];
				sptr.pcon->auto_manual = info->auto_manual;
				sptr.pcon->value = info->point_value;
				break;
			 case WRT:
				sptr.pwr = &weekly_routines[num_point];
				sptr.pwr->auto_manual = info->auto_manual;
				sptr.pwr->value = info->point_value ? 1 : 0;
				break;
			 case AR:
				sptr.panr = &annual_routines[num_point];
				sptr.panr->auto_manual = info->auto_manual;
				sptr.panr->value = info->point_value ? 1 : 0;
				break;
			 case PRG:
				sptr.pprg = &programs[num_point];
				sptr.pprg->auto_manual = info->auto_manual;
				sptr.pprg->on_off = info->point_value ? 1 : 0;
				break;
			 case AMON:
				sptr.pmon = &monitors[num_point];
				sptr.pmon->status = info->point_value ? 1 : 0;
				break;
/*
			 case TOTAL:
				sptr.ptot = &totalizers[num_point];
				sptr.ptot->active = info->auto_manual;
				break;
*/
/*				sptr.ptot->time_on = ptr->point_value; */
			}
		}
	}
}

#endif
/*
 * ----------------------------------------------------------------------------
 * Function Name: update_grp_element
 * Purpose: update the element of the group ponit
 * Params:
 * Returns:
 * Note:   it is called back in sever.c
 * ----------------------------------------------------------------------------
 */
#if 0
S16_T update_grp_element( Str_grp_element *ptr )
{
	Str_points_ptr sptr;
	S16_T i;
	if(GetWordBit(ptr->flag5Int,grp_location,2)/*ptr->location*/ ) /* the element is NOT on the LOCAL_PANEL */
	{
		i = find_remote_point( &ptr->point );
		if( i < 0 ) return -1;
		sptr.prp = remote_points_list + i;
		SetByteBit(&ptr->flag1,GetByteBit(sptr.prp->flag1,REMOTE_auto_manual,1),grp_auto_manual,1);  //auto_manual
		SetByteBit(&ptr->flag1,GetByteBit(sptr.prp->flag1,REMOTE_digital_analog,grp_digital_analog),1,1);	// digital_analog
		SetByteBit(&ptr->flag1,GetByteBit(sptr.prp->flag1,REMOTE_decomisioned,1),grp_decomisioned,1);	// decomisioned
	//	ptr->auto_manual = sptr.prp->auto_manual;
	//	ptr->digital_analog = sptr.prp->digital_analog;
/*		ptr->description_label = sptr.prp->description_label;
		ptr->security = sptr.prp->security; */
	//	ptr->decomisioned = sptr.prp->decomisioned;
		ptr->units = sptr.prp->units;
		ptr->point_value = sptr.prp->point_value;
	}
	else   /* the element is on the LOCAL_PANEL */
	{
		if( !get_point_info( (Point_info*)ptr ) )
	    {
			ptr->units = 1;
			SetWordBit(&ptr->flag5Int,1,grp_absent,1);   // absent
		} 
	}
	return 1;
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: move_groups
 * Purpose: 
 * Params:
 * Returns:
 * Note:   it is called back in sever.c
 * ----------------------------------------------------------------------------
 */
void move_groups( S8_T *dest, S8_T *source, S16_T length,S16_T no_elem, Str_grp_element *address )
{
  	U8_T direction, i;
	register Aux_group_point *ptr;

  	if( source > dest )
  	{
	  	direction = 0;
	    total_elements -= no_elem;
		group_data_length -= ( no_elem * sizeof( Str_grp_element ) );
  	}
	else
	{
		direction = 1;
		total_elements += no_elem;
		group_data_length += ( no_elem * sizeof( Str_grp_element ) );
	}
	memmove( dest, source, length );
  /* Update start addresses for the groups that were moved */
  	ptr = aux_groups;
	for( i=0; i<MAX_GRPS; i++, ptr++ )
		if( ptr->no_elements && ( address < ptr->address ) )
      		if( direction )
				ptr->address += no_elem;
      		else
				ptr->address -= no_elem;
}




/*
 * ----------------------------------------------------------------------------
 * Function Name: writepropertyvalue
 * Purpose: 
 * Params:
 * Returns:
 * Note:   it is called back in applayer.c(serverBACnet())
 * ----------------------------------------------------------------------------
 */
S16_T writepropertyvalue( BACnetObjectIdentifier *obj, S32_T lvalue )
{
 	Point point;

 	point.number       = obj->instance-1;
	point.point_type   = obj->object_type_low+(obj->object_type_hi<<2) - T3000_OBJECT_TYPE + 1;
 	put_point_value( &point, &lvalue, 0, OPERATOR );

	return 0;
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: writepropertyauto
 * Purpose: 
 * Params:
 * Returns:
 * Note:   it is called back in applayer.c(serverBACnet())
 * ----------------------------------------------------------------------------
 */
S16_T	writepropertyauto( BACnetObjectIdentifier *obj, S16_T auto_manual )
{
	Point point;
	point.number       = obj->instance-1;
	point.point_type   = obj->object_type_low+(obj->object_type_hi<<2) - T3000_OBJECT_TYPE + 1;

  	if( point.number < table_bank[point.point_type-1] )
  	{
		switch (point.point_type-1)
		{
			case OUT:
			/*	outputs[point.number].auto_manual = auto_manual;*/
				SetByteBit(&outputs[point.number].flag1,auto_manual,out_auto_manual,1);
				break;
		 	case IN:
			/*	inputs[point.number].auto_manual = auto_manual;*/
				SetByteBit(&inputs[point.number].flag1,auto_manual,in_auto_manual,1);
				break;
		 	case VAR:
			/*	vars[point.number].auto_manual = auto_manual;*/
				SetByteBit(&vars[point.number].flag,auto_manual,var_auto_manual,1);
				break;
	 		case WRT:
			/*	weekly_routines[point.number].auto_manual = auto_manual;*/
				SetByteBit(&weekly_routines[point.number].flag,auto_manual,weekly_auto_manual,1);
				break;
		 	case AR:
			/*	annual_routines[point.number].auto_manual = auto_manual;*/
				SetByteBit(&annual_routines[point.number].flag,auto_manual,annual_auto_manual,1);
        		misc_flags.check_ar=1;
				break;
	 		case CON:
			/*	controllers[point.number].auto_manual = auto_manual;*/
				SetByteBit(&controllers[point.number].flag,auto_manual,con_auto_manual,1);
      			break;
	 		case PRG:
			/*	programs[point.number].auto_manual = auto_manual;*/
				SetByteBit(&controllers[point.number].flag,auto_manual,prg_auto_manual,1);
      			break;
/*
		 	case TOTAL:
				totalizers[point.number].active = auto_manual;
  	    break;
*/
    	}
	}
	return 0;
}

#endif
