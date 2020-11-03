#include "main.h"


#if TIME_SYNC
U16_T crc16(U8_T *p, U8_T length);

uint8_t flag_update_tstat;
//uint8_t flag_update_panel;

uint8_t flag_stop_timesync;

#define SYNC_STATE_NOTREADY		0
#define SYNC_STATE_INITIAL		1  
#define SYNC_STATE_WAIT				2
#define SYNC_STATE_TIMEOUT		3
#define SYNC_STATE_GET_DONE		4


/** @file s_whois.c  Send a Who-Is request. */
BAC_CONN SYNC_Conns;
U8_T SYNC_InterAppId;
//void WhoIs_Send(U8_T InterUdpId);


U32_T time_sync_count;
U32_T check_panel_count;
uint8_t check_lost_master;
uint8_t flag_master_panel;
uint16_t timesync_period;

void SYNC_Init(void)
{
	SYNC_Conns.State = SYNC_STATE_INITIAL;
	SYNC_InterAppId = TCPIP_Bind(NULL, GUDPBC_Event, GUDPBC_Receive);	
	time_sync_count = 0;
	check_panel_count = 0;
	timesync_period = 24 * 3600;  // 1 day
	flag_master_panel = 1;
	check_lost_master = 0;
	flag_update_tstat = 0;
	flag_stop_timesync = 0;
	if ((SYNC_Conns.UdpSocket = TCPIP_UdpNew(SYNC_InterAppId, 0, 0xffffffff,
		0, 1234)) == TCPIP_NO_NEW_CONN)
	{
		
	}
	TimeSync_Heart();
}

U8_T TimeSync_Heart(void)
{
#if 0
	U8_T far SYNC_Buf[40];

	SYNC_Buf[0] = 0x6b;	// heartbeat

	SYNC_Buf[1] = Station_NUM;  // panel number

	TCPIP_UdpSend(SYNC_Conns.UdpSocket, 0, 0, SYNC_Buf, 2);
//	Test[10]++;
#endif
	return 1;	
}
void UdpData(U8_T type);

U8_T TimeSync_Scan(void)
{
#if 0
	UdpData(0);
	

	TCPIP_UdpSend(SYNC_Conns.UdpSocket, 0, 0, &Scan_Infor, sizeof(STR_SCAN_CMD));
//	Test[11]++;
#endif
	return 1;
}

U8_T Send_Network_Scan(void)
{
	U8_T far SYNC_Buf[5];

	SYNC_Buf[0] = 0x64;	
	SYNC_Buf[1] = 0; 
	SYNC_Buf[2] = 0;
	SYNC_Buf[3] = 0;
	SYNC_Buf[4] = 0;

	if(SYNC_Conns.UdpSocket != NULL) 
		TCPIP_UdpClose(SYNC_Conns.UdpSocket);
	
	if((SYNC_Conns.UdpSocket = TCPIP_UdpNew(SYNC_InterAppId, 0, 0xffffffff,
		0, 1234)) == TCPIP_NO_NEW_CONN)
	{
		
	}
	TCPIP_UdpSend(SYNC_Conns.UdpSocket, 0, 0, SYNC_Buf, 5);
	
	return 1;
}


U8_T TimeSync(void)
{
	/* Create SNTP client port */
#if 0
	U16_T i;
	U8_T far SYNC_Buf[500];
	U8_T *ptr;
	U16_T len;
//	SYNC_Init();
	if ((SYNC_Conns.UdpSocket = TCPIP_UdpNew(SYNC_InterAppId, 0, 0xffffffff,
		0, 1234)) == TCPIP_NO_NEW_CONN)
	{
		return 0;
	}


	SYNC_Buf[0] = 0x69;	
	for(i = 0;i < 10;i++)  // time
	{
		SYNC_Buf[1 + i] = Rtc.all[i];
	}
	len = 12;
	SYNC_Buf[11] = 0;	
	
#if USER_SYNC	 // for user and pass
	SYNC_Buf[11] = 0x55; // enable user and pass
	len += sizeof(Password_point) * MAX_PASSW;
	memcpy(&SYNC_Buf[11],(char *)passwords,sizeof(Password_point) * MAX_PASSW);
	
#endif
		
	TCPIP_UdpSend(SYNC_Conns.UdpSocket, 0, 0, SYNC_Buf, len);
#endif
	return 1;
}
		// get update command from T3000, update itself
		
		
		// update tstat at regular time		
		// if it is master, update other panel at regular time
void check_time_sync(void) // interval is 1s
{

	if(flag_master_panel == 1)
	{ // only master can update other panel
	
		if(time_sync_count < timesync_period)
		{
			time_sync_count++;
		}
		else
		{
			
			time_sync_count = 0;
			// update tstat
//			flag_update_tstat = 1;
//			flag_update_panel = 1;
			// update minipanel
			if(flag_stop_timesync == 0)
			{
				
				TimeSync();
			}
			// update tstat
			
			flag_update_tstat = 1;
		}

// if master, send heartbeat at an interval of 2 min		
		if(check_panel_count < 120)  // 10min
		{
			check_panel_count++;
		}
		else
		{
			check_panel_count = 0;
			if(flag_stop_timesync == 0)
				TimeSync_Heart();
			
		}
	}
	else
	{		// if not master		
		check_lost_master++;
		if(check_lost_master > 180)  // exceed 3 min, send heartbeat
		{
			check_lost_master = 0;
			flag_master_panel = 1;
			time_sync_count = 0;
			check_panel_count = 0;
			if(flag_stop_timesync == 0)
				TimeSync_Heart();
		}
	}
					
}




#if ALARM_SYNC

U8_T AlarmSync(uint8_t add_delete,uint8_t index,char *mes,uint8_t panel)
{
	/* Create SNTP client port */
	U16_T i;
	U8_T far SYNC_Buf[500];
	U8_T *ptr;
	U8_T len;
//	SYNC_Init();
	
//	if ((SYNC_Conns.UdpSocket = TCPIP_UdpNew(SYNC_InterAppId,0,0xffffffff, 0, 1234)) == TCPIP_NO_NEW_CONN)
//	{
//		return 0;
//	}

	
	SYNC_Buf[0] = 0x6c;	
	SYNC_Buf[1] = add_delete;	  // 0 - add , 1 - delete
	SYNC_Buf[2] = index;
	SYNC_Buf[3] = panel;

	if(add_delete == 1) // delete
		len = 0;
	else  // add
	{
		len = strlen(mes);
		if(strlen(mes) > ALARM_MESSAGE_SIZE)
			len = ALARM_MESSAGE_SIZE;
	}
	for(i = 0;i < len;i++)  // time
	{
		SYNC_Buf[4 + i] = mes[i];
	}
	SYNC_Buf[4 + len] = 0;
	
	TCPIP_UdpSend(SYNC_Conns.UdpSocket, 0, 0, SYNC_Buf, 5 + len);
	return 1;
}
#endif



#endif


