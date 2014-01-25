#include "ud_str.h"
#include "user_data.h"
#include <string.h>
#include "alarm.h"
extern U16_T far Test[50];


/**************************************************
// return:  0 - no space; >=1 - alarm index
/**************************************************/
int putmessage(char *mes, int prg, int type, char alarmatall,char indalarmpanel,char *alarmpanel, int j)
{
/* char buf[30],*p;*/
	int k; /*j*/
	char *p;
	Alarm_point *ptr;
	/* j = checkalarmentry();*/
	if(j>=0)
	{
		ptr = &alarms[j];
		memset(ptr,0,sizeof(Alarm_point));
		ptr->change_flag  = 2;
		ptr->alarm        = 1;
		ptr->no           = j;
		ptr->alarm_panel  = Station_NUM;
		ptr->alarm_time   = time_since_1970+timestart;
		ptr->alarm_count  = ALARM_MESSAGE_SIZE;
		ptr->prg          = prg;
		ptr->alarm_id     = alarm_id++;
		ptr->type         = type;
		//	ptr->panel_type   = panel_net_info.panel_type;
		
		ptr->alarm_count = strlen(mes);
		strcpy(ptr->alarm_message,mes);
		if(alarmatall)
		{
			ptr->where1     = 255;
		}
		if( indalarmpanel )
		{
			p = &ptr->where1;
			for(k=0;(k<indalarmpanel)&&(k<5);k++,p++)
			{
				*p = alarmpanel[k];
			}
		}
		ptr->change_flag  = 0;
		if( ++ind_alarms > MAX_ALARMS) ind_alarms = MAX_ALARMS;
		/*	GAlarm = 1;*/
	}
	else
	j=-1;
	return j+1;    /* 0 - no space; n - alarm index*/
}



int checkforalarm(char *mes, int prg, int panel, int id, int *free_entry)
{
	Alarm_point *ptr;
	int j;
	ptr = alarms;
	for(j = 0;j < MAX_ALARMS;ptr++,j++)
	{ 	
		if( ptr->alarm == 1)
		{
		 	if( ptr->alarm_panel == panel )
				if( ptr->prg == prg )
			 	if( !id )
			 	{	
					if( !ptr->restored )
				 		if (ptr->alarm_count == strlen(mes) )
							if( !strcmp(ptr->alarm_message, mes) )
					{ 			Test[41] = j + 10;
 
					 return j+1;          /* existing alarm*/
					}
			 }
			 else
			 { 
				if( ptr->alarm_id == *((int *)mes) )
					return j+1;           /* existing alarm*/
			 }
		}
		else
		{
		 	if( ptr->ddelete == 0)   /* ddelete=1; the user delete the alarm*/
		    {                 /* but it was not sent yet to the destination panels*/
		 		*free_entry = j;
				return 0;
			}
		}
	}
//	Test[42]++;
	return 0;  /* alarm does not exist*/
}

int generatealarm(char *mes, int prg, int panel, int type, char alarmatall,char indalarmpanel,char *alarmpanel,char printalarm)
{
	int i;
	int j;
	j = -1;
	if( checkforalarm(mes,prg,panel,0,&j)>0 ) return -1;
	Test[43]++;
	Test[44] = j;
	if(j >= 0)
	{	Test[45]++;
		putmessage(mes,prg,type,alarmatall,indalarmpanel,alarmpanel,j);    /* alarm*/
		new_alarm_flag |= 0x01;
	/*
	if(	alarms[j].where1==255 )
	{
	 i = sizeof(Alarm_point);
	ClientTransactionStateMachine( UNCONF_SERVrequest,
	          (char *)&alarms[j], &i, &Port_parameters[0], 255, SEND_ALARM_COMMAND+100, 0, 0);
	 alarms[j].where_state1=1;
	}
	*/
	}
	/*
	if ( j > 0 )
	{
	 new_alarm_flag |= 0x01;
	 resume(ALARMTASK);
	*/
	/*
	 if(printalarm)
	 {
		 printalarmproc(mes,j);
	 }
	}
	*/
	return j+1;
}



void dalarmrestore(char *mes, int prg, int panel)
{
	int j;
	Alarm_point *ptr;
	
	ptr = alarms;
	for(j=0;j<MAX_ALARMS;ptr++,j++)
	{
		if( ptr->alarm )
	 		if( ptr->alarm_panel==panel )
				if( ptr->prg==prg )
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



