/*
*********************************************************************************
*     Copyright (c) 2006   ASIX Electronic Corporation      All rights reserved.
*
*     This is unpublished proprietary source code of ASIX Electronic Corporation
*
*     The copyright notice above does not evidence any actual or intended
*
*     publication of such source code.
*********************************************************************************


/* INCLUDE FILE DECLARATIONS */

#include "main.h"
#include 	"sntpc.h"
#if (ASIX_MINI || ASIX_CM5)
#include "adapter.h"
#include "uart.h"
#include "mstimer.h"
#endif

#if (ARM_MINI || ARM_CM5)
#include "uip.h"
#endif

#include "sntpc.h"
#include "tcpip.h"
#include <stdio.h>



//#include "printd.h"

/* NAMING CONSTANT DECLARATIONS */

/* GLOBAL VARIABLES DECLARATIONS */

/* LOCAL VARIABLES DECLARATIONS */
TimeInfo t;
static SNTPHeader *psntpcpMsg;


static U16_T far timetickinfo;
//static S16_T far GMT;
const U8_T	code Month[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
const U8_T	code AddMonth[12] = {31,29,31,30,31,30,31,31,30,31,30,31};
static SNTPC_CONN far sntpc_Conns;
static U8_T far sntpc_InterAppId;
//U8_T	sntp_Buf[48];
static U8_T sntp_Retry = 0;


char far sntp_server[30];
//{
//	"ntp.sjtu.edu.cn",
//	"time.nist.gov",
//	"time.windows.com",
//	"time-nw.nist.gov",
//	"time-a.nist.gov",
//	"time-b.nist.gov",
//	"2.cn.pool.ntp.org"
//};

#if (ARM_MINI || ARM_CM5)
static struct uip_udp_conn *sntp_conn = NULL;
U8_T flag_sntpc_Send;

void Send_TimeSync_Broadcast(uint8_t protocal);

#endif

/* LOCAL SUBPROGRAM DECLARATIONS */


/*
 * ----------------------------------------------------------------------------
 * Function Name: SNTPC_Init
 * Purpose: to initial the SNTP client connection information.
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */

void SNTPC_Init(void)
{	
//	U8_T i;

	sntpc_Conns.State = SNTP_STATE_INITIAL;
	
#if (ASIX_MINI || ASIX_CM5)
	sntpc_InterAppId = TCPIP_Bind(NULL, SNTPC_Event, SNTPC_Receive);

	DNSC_Start("time.windows.com");
	DNSC_Start("time.nist.gov");
	DNSC_Start("ntp.sjtu.edu.cn");
#endif

#if (ARM_MINI || ARM_CM5)
	resolv_query("time.windows.com");
	resolv_query("time.nist.gov");
	resolv_query("ntp.sjtu.edu.cn");
#endif

} /* End of SNTPC_Init() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: SNTPC_Event
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
#if (ASIX_MINI || ASIX_CM5)
void SNTPC_Event(U8_T id, U8_T event)
{
	if (id != 0)
		return;

	if (event == TCPIP_CONNECT_CANCEL)
	{
		TCPIP_UdpClose(sntpc_Conns.UdpSocket);
		sntpc_Conns.State = SNTP_STATE_INITIAL;
	}

} /* End of SNTPC_Event() */
#endif



void Get_RTC_by_timestamp(U32_T timestamp,TimeInfo *tt,UN_Time* rtc,U8_T source)
{
	S8_T	signhour, signmin;
	U8_T	hour, min;	
	U8_T	i;
	U16_T temp_YY;
	
	i = 0;
	
	tt->timestamp = timestamp;
	
	signhour = timezone / 100;
	signmin = timezone % 100;
	
	if (signhour < 0)
	{
		hour = -signhour;
		min = -signmin;
		tt->timestamp -= (hour*3600 + min*60);
	}
	else
	{
		hour = signhour;
		min = signmin;
		tt->timestamp += (hour*3600 + min*60);
	}

	if(Daylight_Saving_Time)
	{
		tt->timestamp += 3600;
	}

	tt->second_remain = tt->timestamp % 86400;
	tt->day_total = tt->timestamp / 86400;
	tt->HH = tt->second_remain / 3600;
	tt->MI_r = tt->second_remain % 3600;
	tt->MI = tt->MI_r / 60;
	tt->SS = tt->MI_r % 60;
	tt->YY = tt->day_total / 365.2425;
	
	temp_YY = tt->YY;
	if(source == 0)  // time server
		temp_YY += 1900;
	else  // PC
		temp_YY += 1970;	

	if((temp_YY % 4) == 0)
	{
		tt->DD_r = tt->day_total-(tt->YY*365)-(tt->YY/4);
		tt->DD_r++;
//		tt->DD_r++;	
		while(tt->DD_r>0)
		{
			tt->DD = tt->DD_r;
			tt->DD_r -= AddMonth[i];
			i++;
		}
	}
	else
	{
		tt->DD_r = tt->day_total-(tt->YY*365)-(tt->YY/4);
		tt->DD_r++;
		if(tt->DD_r>365){
			tt->DD_r = 1;
			tt->YY++;
		}
		while(tt->DD_r > 0)
		{
			tt->DD = tt->DD_r;
			tt->DD_r -= Month[i];
			i++;
		}
	}
	tt->MM = i;	
	if(source == 0)  // time server
		tt->YY += 1900;
	else  // PC
		tt->YY += 1970;	
	

	
	rtc->Clk.sec = tt->SS;
	rtc->Clk.min = tt->MI;
	rtc->Clk.hour = tt->HH;
	rtc->Clk.day = tt->DD;
//	Rtc.Clk.week = tt.SS;
	rtc->Clk.mon = tt->MM;
	rtc->Clk.year = tt->YY - 2000;
//	rtc->Clk.day_of_year = tt->day_total;
	rtc->Clk.is_dst = Daylight_Saving_Time;
//	Test[20] = rtc->Clk.day_of_year;

//#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
//	Rtc_Set(rtc->Clk.year,rtc->Clk.mon,rtc->Clk.day,rtc->Clk.hour,rtc->Clk.min,rtc->Clk.sec,0);
//#endif

//#if (ASIX_MINI || ASIX_CM5)		
//		flag_Updata_Clock = 1;
//#endif
}


void Sync_timestamp(S16_T newTZ,S16_T oldTZ,S8_T newDLS,S8_T oldDLS)
{
	U32_T current;

	current = get_current_time();
	current += (newTZ - oldTZ) * 36;
	current += (newDLS - oldDLS) * 3600;
	Get_RTC_by_timestamp(current,&t,&Rtc,1);
#if (ARM_MINI || ARM_CM5)
	Rtc_Set(Rtc.Clk.year,Rtc.Clk.mon,Rtc.Clk.day,Rtc.Clk.hour,Rtc.Clk.min,Rtc.Clk.sec,0);
#endif

#if (ASIX_MINI || ASIX_CM5)		
		flag_Updata_Clock = 1;
#endif
}



/*
 * ----------------------------------------------------------------------------
 * Function Name: SNTPC_Receive
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void SNTPC_Receive(U8_T XDATA* pData, U16_T length, U8_T id)
{
	t.sntpcPktPtr = (SNTPHeader*)pData;
	psntpcpMsg = (SNTPHeader*)t.sntpcPktPtr;
	
	if(id != 0)
		return;
	
	length = length;

#if (ASIX_MINI || ASIX_CM5)	
	Get_RTC_by_timestamp(psntpcpMsg->receive_time1,&t,&Rtc,0);	
//	flag_Updata_Clock = 1;
	TCPIP_UdpClose(sntpc_Conns.UdpSocket);
#endif
	
#if (ARM_MINI || ARM_CM5)
	Get_RTC_by_timestamp(my_honts_arm(psntpcpMsg->receive_time1),&t,&Rtc,0);	
//	flag_Updata_Clock = 1;//Rtc_Set(Rtc.Clk.year,Rtc.Clk.mon,Rtc.Clk.day,Rtc.Clk.hour,Rtc.Clk.min,Rtc.Clk.sec,0);
#endif
	
	sntpc_Conns.State = SNTP_STATE_GET_DONE;	
	

} /* End of SNTPC_Receive() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: SNTPC_GetTime
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
#if 0
U8_T* SNTP_GetTime(void)
{

  	if (sntpc_Conns.State != SNTP_STATE_GET_DONE)
		return NULL;

	sntpc_Conns.State = SNTP_STATE_INITIAL;
//	sprintf (sntp_Buf, "%d/%.2bd/%.2bd %.2bd:%.2bd:%.2bd",
//		t.YY, t.MM, (U8_T)t.DD, t.HH, t.MI, t.SS);
		
		
	sntp_Buf[0] = t.YY / 100;
	sntp_Buf[1] = t.YY % 100;
	sntp_Buf[2] = t.MM;
	sntp_Buf[3] = t.DD;
	sntp_Buf[4] = t.HH;
	sntp_Buf[5] = t.MI;
	sntp_Buf[6] = t.SS;
	return sntp_Buf;

} /* End of SNTP_GetTime() */

#endif

/*
 * ----------------------------------------------------------------------------
 * Function Name: sntpc_Send
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void sntpc_Send(U8_T InterUdpId)
{
	U8_T len = 48;
	U8_T i;
	U8_T far Buf[48];
	len = 48;
#if 1
	Buf[0] = 0xdb;
	Buf[1] = 0x00;
	Buf[2] = 0x04;
	Buf[3] = 0xfa;
	Buf[4] = 0x00;
	Buf[5] = 0x01;
	Buf[6] = 0x00;
	Buf[7] = 0x00;
	Buf[8] = 0x00;
	Buf[9] = 0x01;
	for(i = 10;i < len;i++)
	{
		Buf[i] = 0;
	}
#endif
	

#if (ASIX_MINI || ASIX_CM5)
	TCPIP_UdpSend(InterUdpId, 0, 0, Buf, len);
#endif
	
#if (ARM_MINI || ARM_CM5)	
	uip_send(Buf, len);
#endif

} /* End of sntpc_Send() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: SNTPC_Start
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T SNTPC_Start(S16_T gmt, U32_T timesrIP)
{

#if (ASIX_MINI || ASIX_CM5)
	//static u32 ip_backup;
	if(timesrIP == 0) 
		return SNTP_STATE_NOTREADY;
	sntpc_Conns.ServerIp = timesrIP;
//	GMT = gmt;
	/* Create SNTP client port */

	if(sntpc_Conns.UdpSocket != NULL) 
		TCPIP_UdpClose(sntpc_Conns.UdpSocket);
	Test[25]++;
	if((sntpc_Conns.UdpSocket = TCPIP_UdpNew(sntpc_InterAppId, 0, sntpc_Conns.ServerIp,
		0, SNTP_SERVER_PORT)) == TCPIP_NO_NEW_CONN)
	{
		return SNTP_STATE_NOTREADY;
	}

	Test[26]++;
		
	sntpc_Send(sntpc_Conns.UdpSocket);
	sntpc_Conns.State = SNTP_STATE_WAIT;
	timetickinfo = (U16_T)SWTIMER_Tick();
	sntp_Retry = 0;

//	printd("In NTP_Start End\r\n");
	return SNTP_STATE_WAIT;
#endif


#if (ARM_MINI || ARM_CM5)
	uip_ipaddr_t addr;
		
	if(sntp_conn != NULL) 
		uip_udp_remove(sntp_conn);

	uip_ipaddr(addr, (U8_T)(timesrIP >> 24), (U8_T)(timesrIP >> 16), (U8_T)(timesrIP >> 8), (U8_T)(timesrIP));	
	sntp_conn = uip_udp_new(&addr, HTONS(123));

	flag_sntpc_Send = 1;
#endif	
} /* End of SNTPC_Start() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: SNTPC_Stop
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */

void SNTPC_Stop(void)
{
#if (ASIX_MINI || ASIX_CM5)
	if(sntpc_Conns.State != SNTP_STATE_INITIAL)
	{
		TCPIP_UdpClose(sntpc_Conns.UdpSocket);
		sntpc_Conns.State = SNTP_STATE_INITIAL;
	}
#endif
	
#if (ARM_MINI || ARM_CM5)
//	uip_udp_remove(sntp_conn);
#endif

} /* End of SNTPC_Stop() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: SNTPC_GetState
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
#if (ASIX_MINI || ASIX_CM5)
U8_T SNTPC_GetState(void)
{
	if(sntpc_Conns.State == SNTP_STATE_WAIT)
	{
		U16_T	CurTime = (U16_T)SWTIMER_Tick();
		if ((CurTime - timetickinfo) >= SNTPC_REQUEST_TIMEOUT)
		{
			if (++sntp_Retry >= SNTPC_MAX_RETRY)
			{
				TCPIP_UdpClose(sntpc_Conns.UdpSocket);
				sntpc_Conns.State = SNTP_STATE_INITIAL;
				return SNTP_STATE_TIMEOUT;
			}
//			sntpc_Send(sntpc_Conns.UdpSocket);
			timetickinfo = (U16_T)SWTIMER_Tick();
		}
	}
	return sntpc_Conns.State;
}
#endif

#if (ARM_MINI || ARM_CM5)

U8_T SNTPC_GetState(void)
{
#if 1//ARM
	if(Setting_Info.reg.en_time_sync_with_pc == 1)
	{
		if(Setting_Info.reg.update_time_sync_pc == 0)
			return SNTP_STATE_GET_DONE;
		else
			return 0;
	}
	else
#endif
		return sntpc_Conns.State;
}


void sntp_appcall(void)
{
	if(uip_poll()) 
	{	
		// auto send
		if(flag_sntpc_Send)
		{
			sntpc_Send(0);
			flag_sntpc_Send = 0;
			
		}
	}
	
	if(uip_newdata()) 
	{
// deal with receiving data
		SNTPC_Receive(uip_appdata, uip_len, 0);	
	}
}

void Send_Time_Sync_To_Network(void)
{
			
	U8_T k;
	for(k = 0;k < 100;k++)
	{ // only tstat8, other Temco's product dont have RTC
		if(Sch_To_T8[k].schedule_id > 0)
			Sch_To_T8[k].f_time_sync = 1;
	}	
	// send bacnet timesync
	if(Modbus.network_master == 1)
		Send_TimeSync_Broadcast(BAC_IP_CLIENT);

	if(Modbus.com_config[2] == BACNET_MASTER || Modbus.com_config[0] == BACNET_MASTER)
		Send_TimeSync_Broadcast(BAC_MSTP);
}

#endif


void sntp_select_time_server(U8_T type)
{

#if (ASIX_MINI || ASIX_CM5)

	SntpServer[0] = 0;
	SntpServer[1] = 0;
	SntpServer[2] = 0;
	SntpServer[3] = 0;
	

	if(type < 2 || type > 5) return;
	if(type == 2)
		DNSC_Query("ntp.sjtu.edu.cn",&SntpServer);
	else if(type == 3)
		DNSC_Query("time.nist.gov",&SntpServer);
	else if(type == 4)
		DNSC_Query("time.windows.com",&SntpServer);
	else if(type == 5)
	{
		DNSC_Query(sntp_server,&SntpServer);
	}
	
	if((SntpServer[0] == 0) && (SntpServer[1] == 0)&& (SntpServer[2] == 0)&& (SntpServer[3] == 0) )
	{
		// generate a alarm, DNSC error, did not get correct sntp server ip address
		generate_common_alarm(DNS_FAIL);
	}
//	else
//	{
//		if(type == 2)
//		{SntpServer[0] = 
//		else if(type == 3)
//			DNSC_Query("time.nist.gov",&SntpServer);
//		else if(type == 4)
//	}

#endif
	
#if (ARM_MINI || ARM_CM5)
	uint16_t * ptr_rm_ip;
	uint8_t tempip[4];
	
	if(type < 2 || type > 5) return;
	if(type == 2)
		ptr_rm_ip = resolv_lookup("ntp.sjtu.edu.cn");
	else if(type == 3)
		ptr_rm_ip = resolv_lookup("time.nist.gov");
	else if(type == 4)
		ptr_rm_ip = resolv_lookup("time.windows.com");
	else if(type == 5)
	{
		ptr_rm_ip = resolv_lookup(sntp_server);
	}
	memcpy(SntpServer,ptr_rm_ip,4);

	
#endif
}


#if (!ARM_TSTAT_WIFI)
void update_sntp(void)
{
	U8_T state;
	U8_T *time_ptr;
	U8_T temp[48];	
	time_ptr = temp;	

	if((Modbus.en_sntp >= 2) || (Setting_Info.reg.en_time_sync_with_pc == 1))  // enable
	{
		count_sntp++;	
		if(flag_Update_Sntp == 0)
		{		
			
				state = SNTPC_GetState();
				if(SNTP_STATE_GET_DONE  == state)
				{		
					Setting_Info.reg.sync_with_ntp_result = 1;
					// update successfullly	
#if 1//ARM					
					if(Setting_Info.reg.en_time_sync_with_pc == 0)  // time server
#endif
					{
//					time_ptr = SNTP_GetTime();
//											
//					Rtc_Set(time_ptr[1],time_ptr[2],time_ptr[3],time_ptr[4],time_ptr[5],time_ptr[6],0);
#if (ARM_MINI || ARM_CM5)			
					Rtc_Set(Rtc.Clk.year,Rtc.Clk.mon,Rtc.Clk.day,Rtc.Clk.hour,Rtc.Clk.min,Rtc.Clk.sec,0);	
					RTC_Get();
					
					Send_Time_Sync_To_Network();
#endif
					
#if (ASIX_MINI || ASIX_CM5)
					//Updata_Clock(0);
					flag_Updata_Clock = 1;
#endif
						
					}						
					update_sntp_last_time = get_current_time();
					E2prom_Write_Byte(EEP_SNTP_TIME4,(U8_T)(update_sntp_last_time >> 24));
					E2prom_Write_Byte(EEP_SNTP_TIME3,(U8_T)(update_sntp_last_time >> 16));
					E2prom_Write_Byte(EEP_SNTP_TIME2,(U8_T)(update_sntp_last_time >> 8));
					E2prom_Write_Byte(EEP_SNTP_TIME1,(U8_T)(update_sntp_last_time));
					Setting_Info.reg.update_sntp_last_time = swap_double(update_sntp_last_time);				

//  send time to network panel and remote devices					
				
					
					
					Update_Sntp_Retry = 0;
					flag_Update_Sntp = 1;
					count_sntp = 0;
				}
				else
				{			  
					if(Update_Sntp_Retry < MAX_SNTP_RETRY_COUNT)
					{
						if(count_sntp % 20 == 0)
						{	
							if(Setting_Info.reg.en_time_sync_with_pc == 0)
							{// udpate with NTP
								sntp_select_time_server(Modbus.en_sntp);
								SNTPC_Start(timezone, (((U32_T)SntpServer[0]) << 24) | ((U32_T)SntpServer[1] << 16) | ((U32_T)SntpServer[2] << 8) | (SntpServer[3]));
							}
							Update_Sntp_Retry++;
						}
					}
					else
					{
				// update SNTP fail
						generate_common_alarm(ALARM_SNTP_FAIL);
						Update_Sntp_Retry = 0;
						flag_Update_Sntp = 1;
					}					
			  }
		}
		else
		{
			if(count_sntp > 4 * 3600 * 24)  // update per 1 day
			{
				flag_Update_Sntp = 0;
				Update_Sntp_Retry = 0;
				count_sntp = 0;
			}
		}
	}			

}
#endif

/* End of sntpc.c */
