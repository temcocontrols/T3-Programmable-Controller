#include "bacnet.h"
#include <string.h>
#include "alarm.h"
#include "main.h"
#include "user_data.h"
#include "main.h"


#if (ASIX_MINI || ASIX_CM5)
extern xSemaphoreHandle sem_alarm;
#endif


//U8_T Write_ALARM_TO_SD(Alarm_point alarm,U32_T star_pos);

///**************************************************
//// return:  0 - no space; >=1 - alarm index
///**************************************************/
S16_T putmessage(S8_T *mes, S16_T prg, S16_T panel, S16_T type, S8_T alarmatall,S8_T indalarmpanel,S8_T *alarmpanel, S16_T j)
{
/* S8_T buf[30],*p;*/
	S16_T k; /*j*/
	S8_T *p;
	Alarm_point *ptr;
	/* j = checkalarmentry();*/
	if(j>=0)
	{
		ptr = &alarms[j];
		memset(ptr,0,sizeof(Alarm_point));
		ptr->change_flag  = 2;
		ptr->alarm        = 1;
		ptr->no           = j;
		ptr->alarm_panel  = panel;
		ptr->alarm_time   = swap_double(get_current_time());
		ptr->alarm_count  = ALARM_MESSAGE_SIZE;
		ptr->prg          = prg;
		ptr->alarm_id     = alarm_id++;
		ptr->type         = type;
		//	ptr->panel_type   = panel_net_info.panel_type;

		ptr->alarm_count = strlen(mes);
		strcpy(ptr->alarm_message,mes);
		if(alarmatall)
		{
			ptr->where1  = 255;
		}
		if( indalarmpanel )
		{
			p = (S8_T *)&ptr->where1;
			for(k=0;(k<indalarmpanel)&&(k<5);k++,p++)
			{
				*p = alarmpanel[k];
			}
		}
		ptr->change_flag  = 0;
		if( ++ind_alarms > MAX_ALARMS) ind_alarms = MAX_ALARMS;
		/*	GAlarm = 1;*/
		
//  save alarm to SD card
		alarm_index++;
#if STORE_TO_SD
//		Write_ALARM_TO_SD(alarms[j],alarm_index * 100);
#endif
	}
	else
	j=-1;
	return j+1;    /* 0 - no space; n - alarm index*/
}



S16_T checkforalarm(S8_T *mes, S16_T prg, S16_T panel, S16_T id, S16_T *free_entry)
{
	Alarm_point *ptr;
	S16_T j;
	ptr = alarms;
	for(j = 0;j < MAX_ALARMS;ptr++,j++)
	{ 		
		if( ptr->alarm == 1)
		{			
		 	if( ptr->alarm_panel == panel )
				//if( ptr->prg == prg )
			 	if( !id )
			 	{	
					if( !ptr->restored )
				 		if (ptr->alarm_count == strlen(mes) )
							if( !strcmp(ptr->alarm_message, mes) )
					{ 	
					 return j+1;          /* existing alarm*/
					}
			 }
			 else
			 { 
				if( ptr->alarm_id == *((S16_T *)mes) )
				{
					return j+1;           /* existing alarm*/
				}
			 }
		}
		else
		{
		 	if( ptr->ddelete == 0)   /* ddelete=1; the user delete the alarm*/
		  {                 /* but it was not sent yet to the destination panels*/
		 		*free_entry = j;
				return 0;
			}
			else
			{  // if deleted, it is free for new alarm
				//ptr->alarm = 1;
				ptr->ddelete = 0;
				return j+1;
			}
		}
	}
	return 0;  /* alarm does not exist*/
}





U8_T AlarmSync(uint8_t add_delete,uint8_t index,S8_T *mes,uint8_t panel);

void check_input_alarm(void)
{
	U8_T i,j;
	U32_T far alarm_in_short;
	U32_T far alarm_in_open;
	U8_T alarm_in_short_num;
	U8_T alarm_in_open_num;
	U8_T len;
	S8_T far str[200];
	
	// check input alarm  
	alarm_in_short = 0;
	alarm_in_open = 0;
	alarm_in_short_num = 0;
	alarm_in_open_num = 0;
	
  for(j = 0;j < 32;j++)
	{
		if(inputs[j].decom & IN_OPEN)
		{
			alarm_in_open_num++;
			alarm_in_open |= (1L << j);
		}
		else if(inputs[j].decom & IN_SHORT)
		{
			alarm_in_short_num++;
			alarm_in_short |= (1L << j);
		}
	}


	memset(str,0,200);
	len = 0;
	if(alarm_in_open_num > 0)
	{
		str[len++] = 'O';
		str[len++] = 'p';
		str[len++] = 'e';
		str[len++] = 'n';
		str[len++] = ' ';
		str[len++] = 'c';
		str[len++] = 'i';
		str[len++] = 'r';
		str[len++] = 'c';
		str[len++] = 'u';
		str[len++] = 'i';
		str[len++] = 't';
		str[len++] = ' ';
		str[len++] = 'o';
		str[len++] = 'n';
		str[len++] = ' ';
		str[len++] = 'I';
		str[len++] = 'N';
		
//		str[len++] = 'I';
//		str[len++] = 'N';
//		str[len++] = ' ';
//		str[len++] = 'O';
//		str[len++] = 'P';
//		str[len++] = 'E';
//		str[len++] = 'N';
//		str[len++] = ' ';
		 
		for(j = 0;j < 32 ;j++)
		{				
			if(alarm_in_open & (1L << j))
			{
				if(j < 9)
				{
					str[len++] = '0' + (j + 1) % 10;
					str[len++] = ',';
				}
				else
				{
					str[len++] = '0' + (j + 1) / 10;
					str[len++] = '0' + (j + 1) % 10;
					str[len++] = ',';
				}
			}
		}

		if(len > ALARM_MESSAGE_SIZE) len = ALARM_MESSAGE_SIZE;
		str[len] = '\0';
		i = generatealarm(str,255, Station_NUM, VIRTUAL_ALARM, alarm_at_all, ind_alarm_panel, alarm_panel, 0); /*printAlarms=1*/
		if ( i > 0 )    /* new alarm message*/
		{
			alarm_flag = 1;
		}		
	}

	
	memset(str,0,200);
	len = 0;
	if(alarm_in_short_num > 0)
	{
		str[len++] = 'S';
		str[len++] = 'h';
		str[len++] = 'o';
		str[len++] = 'r';
		str[len++] = 't';
		str[len++] = ' ';
		str[len++] = 'c';
		str[len++] = 'i';
		str[len++] = 'r';
		str[len++] = 'c';
		str[len++] = 'u';
		str[len++] = 'i';
		str[len++] = 't';
		str[len++] = ' ';
		str[len++] = 'o';
		str[len++] = 'n';
		str[len++] = ' ';
		str[len++] = 'I';
		str[len++] = 'N';
		
//		str[len++] = 'I';
//		str[len++] = 'N';
//		str[len++] = ' ';
//		str[len++] = 'S';
//		str[len++] = 'H';
//		str[len++] = 'O';
//		str[len++] = 'R';
//		str[len++] = 'T';
//		str[len++] = ' ';
		
		for(j = 0;j < 32;j++)
		{				
			if(alarm_in_short & (1L << j))
			{
				if(j < 9)
				{
					str[len++] = '0' + (j + 1) % 10;
					str[len++] = ',';
				}
				else
				{
					str[len++] = '0' + (j + 1) / 10;
					str[len++] = '0' + (j + 1) % 10;
					str[len++] = ',';
				}
			}
		}

		if(len > ALARM_MESSAGE_SIZE) len = ALARM_MESSAGE_SIZE;
		str[len] = '\0';
		i = generatealarm(str,255, Station_NUM, VIRTUAL_ALARM, alarm_at_all, ind_alarm_panel, alarm_panel, 0); /*printAlarms=1*/
		if ( i > 0 )    /* new alarm message*/
		{
			alarm_flag = 1;
		}
	}	
	
}

// check SUB ID conflict
/*
type - 0: confilct with master, 1 - confilct with sub

*/
void check_id_alarm(uint8_t type, uint8_t id, uint32_t old_sn, uint32_t new_sn)
{
	S8_T far str[200];
	memset(str,0,200);
	if(type == 0) // confilct with master
	{
		sprintf(str, "ID :%u SN: %lu has conlift with T3 controller" , (uint16)id,new_sn);
	}
	else //  confilct with sub
	{
		sprintf(str, "ID :%u SN: %lu has conlift SN: %lu", (uint16)id,new_sn,old_sn);
	}
	
	if(strlen(str) > ALARM_MESSAGE_SIZE) 
		str[ALARM_MESSAGE_SIZE] = '\0';

	 /*printAlarms=1*/
	if ( generatealarm(str,255, Station_NUM, VIRTUAL_ALARM, alarm_at_all, ind_alarm_panel, alarm_panel, 0) > 0 )    /* new alarm message*/
	{
		alarm_flag = 1;
	}
}

void generate_common_alarm(U8_T index)
{
	S8_T far str[200];
	memset(str,0,200);
	
	if(index == ALARM_LOST_TOP)
		sprintf(str, "Lost communicaton with top board" );
	else if(index == ALARM_AO_FB)
		sprintf(str, "READ AO Feedback FAIL" );
	else if(index == ALARM_SNTP_FAIL)
		sprintf(str, "Update time server fail" );
	else if(index == ALARM_LOST_PIC)
		sprintf(str, "Lose communication with pic" );
	else if(index == ALARM_ABNORMAL_SD)
		sprintf(str, "SD CARD IS ABNORMAL" );
	else if(index == DNS_FAIL)
		sprintf(str, "CAN NOT CONNECT SNTP" );
	else
		return;
		
	if(strlen(str) > ALARM_MESSAGE_SIZE) 
		str[ALARM_MESSAGE_SIZE] = '\0';

	 /*printAlarms=1*/
	if ( generatealarm(str,255, Station_NUM, VIRTUAL_ALARM, alarm_at_all, ind_alarm_panel, alarm_panel, 0) > 0 )    /* new alarm message*/
	{
		alarm_flag = 1;
	}
}


void generate_program_alarm(U8_T type,U8_T prg)
{
	S8_T far str[20];
	memset(str,0,20);

	//if(index == ALARM_PROGRAM)
	if(type == 0) // dead cycle
		sprintf(str, "PRG %d error : dead cycle1",(U16_T)prg);
	else if(type == 1) // dead cycle
		sprintf(str, "PRG %d error : dead cycle2",(U16_T)prg);
	else if(type == 2) // run long time
		sprintf(str, "PRG %d error : takes long time",(U16_T)prg);
//	else
//		return;
		
	if(strlen(str) > ALARM_MESSAGE_SIZE) 
		str[ALARM_MESSAGE_SIZE] = '\0';

	 /*printAlarms=1*/
	if( generatealarm(str,255, Station_NUM, VIRTUAL_ALARM, alarm_at_all, ind_alarm_panel, alarm_panel, 0) > 0 )    /* new alarm message*/
	{
		alarm_flag = 1;
	}
}

//void generate_alarm_lost_top(void)
//{
//	S8_T far str[200];
//	memset(str,0,200);

//	sprintf(str, "Lost communicaton with top board" );
//	
//	
//	if(strlen(str) > ALARM_MESSAGE_SIZE) 
//		str[ALARM_MESSAGE_SIZE] = '\0';

//	 /*printAlarms=1*/
//	if ( generatealarm(str,255, Station_NUM, VIRTUAL_ALARM, alarm_at_all, ind_alarm_panel, alarm_panel, 0) > 0 )    /* new alarm message*/
//	{
//		alarm_flag = 1;
//	}
//}

//void generate_read_AO_FeedBack(void)
//{
//	S8_T far str[200];
//	memset(str,0,200);

//	sprintf(str, "READ AO Feedback FAIL" );	
//	
//	if(strlen(str) > ALARM_MESSAGE_SIZE) 
//		str[ALARM_MESSAGE_SIZE] = '\0';

//	 /*printAlarms=1*/
//	if ( generatealarm(str,255, Station_NUM, VIRTUAL_ALARM, alarm_at_all, ind_alarm_panel, alarm_panel, 0) > 0 )    /* new alarm message*/
//	{
//		alarm_flag = 1;
//	}
//}


S16_T generatealarm(S8_T *mes, S16_T prg, S16_T panel, S16_T type, S8_T alarmatall,S8_T indalarmpanel,S8_T *alarmpanel,S8_T printalarm)
{
	S16_T j;
	j = -1;
 
#if (ASIX_MINI || ASIX_CM5)
	if(cSemaphoreTake(sem_alarm, 50) == pdFALSE)
		return ;
#endif
	if(checkforalarm(mes,prg,panel,0,&j) > 0) 
	{
#if (ASIX_MINI || ASIX_CM5)
		cSemaphoreGive(sem_alarm);
#endif
		return -1;
	}

	if(j >= 0)
	{
		putmessage(mes,prg,panel,type,alarmatall,indalarmpanel,alarmpanel,j);    /* alarm */
		new_alarm_flag |= 0x01;
#if ALARM_SYNC
#if (ASIX_MINI || ASIX_CM5)
//		AlarmSync(0,0,mes,panel);
#endif
#endif 
	}
#if (ASIX_MINI || ASIX_CM5)
	cSemaphoreGive(sem_alarm);
#endif	
	return j + 1;
}



void dalarmrestore(S8_T *mes, S16_T prg, S16_T panel)
{
	S16_T j;
	Alarm_point *ptr;
	
	ptr = alarms;
	for(j = 0;j < MAX_ALARMS;ptr++,j++)
	{
		if( ptr->alarm )
	 		if( ptr->alarm_panel == panel )
				if( ptr->prg == prg )
		 			if( !ptr->restored )
						if (ptr->alarm_count == strlen(mes) )
			 				if( !strcmp(ptr->alarm_message, mes) )
							{
								ptr->restored = 1;
								ptr->where_state1 = 0;
								ptr->where_state2 = 0;
								ptr->where_state3 = 0;
								ptr->where_state4 = 0;
								ptr->where_state5 = 0;
								if( !ptr->acknowledged )
								{
									if(!ind_alarms--)  ind_alarms = 0;
								}
					    	new_alarm_flag |= 0x02;
								return;
							}
	}
}

S16_T sendalarm(S16_T arg, Alarm_point *ptr, S16_T t)
{
// Protocol_parameters *ps;
// unsigned S16_T i;
// S16_T ret=0;
//   	  ps = &Port_parameters[0];
//			i = sizeof(Alarm_point);
//			if(	ptr->where1==255 )
//			{
//				if( !ptr->where_state1 || t )
//				{
///*					net_call(SEND_ALARM_COMMAND+100, arg, (S8_T *)ptr, &i, 255, panel_net_info.network, BACnetUnconfirmedRequestPDU|NETCALL_NOTTIMEOUT);*/
//          ClientTransactionStateMachine( UNCONF_SERVrequest,
//              (S8_T *)ptr, &i, ps, 255, SEND_ALARM_COMMAND+100, arg, 0);
//					ptr->where_state1=1;
//				}
//			}
///*
//			else
//			{
//			 if( ptr->where1 && !t )
//			 {
//				if( arg || !ptr->where_state1 )
//				 if( panel_net_info.active_panels&(1<<(ptr->where1-1)) )
//				 {
//					if( net_call(SEND_ALARM_COMMAND+100, arg, (S8_T *)ptr, &i, ptr->where1, panel_net_info.network, NETCALL_NOTTIMEOUT)==SUCCESS )
//					 ptr->where_state1=1;
//					else
//					 ret=1;
//				 }
//				 else
//					 ret=1;
//				i = sizeof(Alarm_point);
//				if( ptr->where2 )
//				 if( arg || !ptr->where_state2 )
//					if( panel_net_info.active_panels&(1<<(ptr->where2-1)) )
//					{
//					 if( net_call(SEND_ALARM_COMMAND+100, arg, (S8_T *)ptr, &i, ptr->where2, panel_net_info.network, NETCALL_NOTTIMEOUT)==SUCCESS )
//						 ptr->where_state2=1;
//					 else
//						ret=1;
//					}
//					else
//					 ret=1;
//				i = sizeof(Alarm_point);
//				if( ptr->where3 )
//				 if( arg || !ptr->where_state3 )
//					if( panel_net_info.active_panels&(1<<(ptr->where3-1)) )
//					{
//					 if( net_call(SEND_ALARM_COMMAND+100, arg, (S8_T *)ptr, &i, ptr->where3, panel_net_info.network, NETCALL_NOTTIMEOUT)==SUCCESS )
//						 ptr->where_state3=1;
//					 else
//						ret=1;
//					}
//					else
//					 ret=1;
//				i = sizeof(Alarm_point);
//				if( ptr->where4 )
//				 if( arg || !ptr->where_state4 )
//					if( panel_net_info.active_panels&(1<<(ptr->where4-1)) )
//					{
//					 if( net_call(SEND_ALARM_COMMAND+100, arg, (S8_T *)ptr, &i, ptr->where4, panel_net_info.network, NETCALL_NOTTIMEOUT)==SUCCESS )
//						 ptr->where_state4=1;
//					 else
//						ret=1;
//					}
//					else
//					 ret=1;
//				i = sizeof(Alarm_point);
//				if( ptr->where5 )
//				 if( arg || !ptr->where_state5 )
//					if( panel_net_info.active_panels&(1<<(ptr->where5-1)) )
//					{
//					 if( net_call(SEND_ALARM_COMMAND+100, arg, (S8_T *)ptr, &i, ptr->where5, panel_net_info.network, NETCALL_NOTTIMEOUT)==SUCCESS )
//						 ptr->where_state5=1;
//					 else
//						ret=1;
//					}
//					else
//					 ret=1;
//			 }
//			}
//*/
// return ret;
} 

void update_alarm_tbl(Alarm_point *block, S16_T max_points_bank )
{ 
	S16_T i,j;
	Alarm_point *bl;
	Str_points_ptr ptr;
	S8_T alarmtask;
	
	alarmtask = 0;
	ptr.palrm = &alarms[0];
	for(j = 0; j < MAX_ALARMS; j++, ptr.palrm++)
 	{
		if(	ptr.palrm->alarm )
		{
		 	if( ptr.palrm->change_flag ) 
				continue;
			ptr.palrm->change_flag = 2;
			bl = block;
			for( i = 0; i < max_points_bank;i++, bl++ )
		 	{
				if(	bl->alarm )
				{
				 	if( ptr.palrm->no == bl->no ) break;
				}
		 	}
		 	if( i >= max_points_bank )
		 	{
				ptr.palrm->change_flag=0;
				continue;
		 	}
		 	if( bl->ddelete )
		 	{
				if( (ptr.palrm->restored == 0) && (ptr.palrm->acknowledged == 0) )
				{								
					if(!ind_alarms--) ind_alarms = 0;					
				}
				ptr.palrm->alarm        = 0;
				ptr.palrm->change_flag  = 0;
				ptr.palrm->restored     = 0;
				ptr.palrm->acknowledged = 0;
				ptr.palrm->ddelete      = 1;
				ptr.palrm->original     = 0;
//				ptr.palrm->alarm_panel     = 0;
//				ptr.palrm->alarm_time     = 0;
//				memset(ptr.palrm->alarm_message,0,ALARM_MESSAGE_SIZE+1);
				if(ptr.palrm->alarm_panel == Station_NUM)
				{
					ptr.palrm->where_state1 = 0;
					ptr.palrm->where_state2 = 0;
					ptr.palrm->where_state3 = 0;
					ptr.palrm->where_state4 = 0;
					ptr.palrm->where_state5 = 0;
				}
				alarmtask |= 0x02;
				continue;
		 	}
		 	if( bl->acknowledged )
		 	{
				if( (ptr.palrm->acknowledged == 0) )
				{
					if( (ptr.palrm->restored == 0) )
					{
						if(!ind_alarms--) ind_alarms=0;
					}
					ptr.palrm->acknowledged = 1;
					ptr.palrm->original     = 0;
					if(ptr.palrm->alarm_panel==Station_NUM)
					{
						ptr.palrm->where_state1 = 0;
						ptr.palrm->where_state2 = 0;
						ptr.palrm->where_state3 = 0;
						ptr.palrm->where_state4 = 0;
						ptr.palrm->where_state5 = 0;
					}
					alarmtask |= 0x01;
				}
		 	}
		 	ptr.palrm->change_flag=0;
		}
 	}
	if( alarmtask )
	{
		new_alarm_flag |= 0x01;
		if( alarmtask & 0x02 )
			new_alarm_flag |= 0x02;
	}


}


void alarm_task(void)
{
//	portTickType xDelayPeriod  = ( portTickType ) 50 / portTICK_RATE_MS; // 1000#endif
//	S16_T j,ret, ret1, retry;
//	unsigned S16_T i;
//	S8_T ;
//	Alarm_point *ptr;
//	ret = 0;
//	ret1 = 0;

//	for (;;)
//	{
//		vTaskDelay(xDelayPeriod);
//		alf = 0;
//		
//		while(1)
//		{
//		if(ret1 || new_alarm_flag)
//		{
//			alf |= new_alarm_flag;
//			new_alarm_flag = 0;
//			ret1 = 0;
//			retry = 3;
//			while(retry)
//			{
//				alf |= new_alarm_flag;
//				ret = 0;
//				ptr = alarms;
//				for(j = 0;j < MAX_ALARMS;ptr++,j++)
//				{
//					if(	ptr->alarm_panel == Station_NUM )
//					{
//						if( ptr->alarm )
//						{
//							if( !ptr->restored && !ptr->acknowledged )
//							{
//								if( alf == 0x04 )
//								{
//									ret |= sendalarm(0, ptr, 1);
//								}
//								else
//									ret |= sendalarm(0, ptr, 0);
//							}
//						}
//						else
//						{
//							if( alf&0x02 )
//							{
//								if( ptr->ddelete )
//								{
//									if( sendalarm(1, ptr, 0) )  /*delete alarm*/
//									{        /*error*/
//									 ret |= 1;
//									}
//									else   /*success*/
//									 ptr->ddelete = 0;
//								}	
//							}
//						}
//					}
//					else
//					{           /* sent to the panel originated from*/
//						if( alf&0x02 || alf&0x01 )
//						{
//							i = sizeof(Alarm_point);
//							if( ptr->alarm )
//							{
//								if( !ptr->original )
//								{
///*
//					if( net_call(SEND_ALARM_COMMAND+100, 0, (S8_T *)ptr, &i, ptr->alarm_panel, panel_info->network, NETCALL_NOTTIMEOUT)==SUCCESS )
//					{
//					 ptr->original = 1;
//								}
//					else
//*/
//								{
//									if( retry==1 )
//									{
///*							 net_call(SEND_ALARM_COMMAND+100, 0, (S8_T *)ptr, &i, 255, panel_info->network, BACnetUnconfirmedRequestPDU|NETCALL_NOTTIMEOUT);*/
////        		  ClientTransactionStateMachine( UNCONF_SERVrequest,
////            	  (S8_T *)ptr, &i, ps, 255, SEND_ALARM_COMMAND+100, 0, 0);
////								 ptr->original = 1;
//									}
//									else
//										ret |= 1;
//								}
//							}
//						}
//						else if( alf&0x02 )
//						{
//							if( ptr->ddelete )
//							{
//								if( !ptr->original )
//								{
///*
//						if( net_call(SEND_ALARM_COMMAND+100, 1, (S8_T *)ptr, &i, ptr->alarm_panel, panel_info->network, NETCALL_NOTTIMEOUT)==SUCCESS )
//						{
//						 ptr->original = 1;
//						 ptr->ddelete = 0;
//						}
//						else
//*/
//								{
//									if( retry==1 )
//									{       /*broadcast*/
/////*							 net_call(SEND_ALARM_COMMAND+100, 1, (S8_T *)ptr, &i, 255, panel_info->network, BACnetUnconfirmedRequestPDU|NETCALL_NOTTIMEOUT);*/
////        		  ClientTransactionStateMachine( UNCONF_SERVrequest,
////            	  (S8_T *)ptr, &i, ps, 255, SEND_ALARM_COMMAND+100, 1, 0);
////							 ptr->original = 1;
////							 ptr->ddelete = 0;
//									}
//									else
//										ret |= 1;
//								}
//								}
//							}
//						}
//					}
//				}
//			}
//			if (ret)
//			{
//				retry--;
//			}
//		 else
//			break;
//		}  /* end while retry*/
//		continue;
//	 }
// }
//		

//	if(ret)
//	{
//		vTaskDelay(60000 / xDelayPeriod);       /*1 min*/
//		ret1 = 1;
//	}
//	else
//	{
///*		suspend(ALARMTASK);*/
//		vTaskDelay(270000 / xDelayPeriod);        /*4.5 min*/
//		ret1 = 1;
//		new_alarm_flag |= 0x04;
//	}
//}

}

