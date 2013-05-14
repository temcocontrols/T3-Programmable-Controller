#include "main.h"

#ifdef BACNET
void TimerMSTP_task(void) reentrant
{
	Protocol_parameters *ps;
	static U16_T count_1s = 0;
	portTickType xDelayPeriod = ( portTickType ) 5 / portTICK_RATE_MS;
	for (;;)
	{	
		vTaskDelay(xDelayPeriod);
		ps = Port_parameters;
		ps->SilenceTimer++; // add for MSTP
		if(Modbus.PROTOCAL == BACNET_MSTP)
			ps->InactivityTimer++;  
		if( ps->timepoints )
			ps->timepoints--;
		
	}
}





void BACnet_Common_task(void) reentrant
{
	Protocol_parameters *ps;
	//Info_Table *inf;
	static U8_T count_1s = 0;
	portTickType xDelayPeriod = ( portTickType ) 50 / portTICK_RATE_MS;
	for (;;)
	{	
		//if( port )
		{			
			ps = Port_parameters;
		}
	/*	else
		{
			ps = &Port_parameters[1];
		}*/	
		vTaskDelay(xDelayPeriod);
		/* add 3 timers for PTP state machine */		
	
		ps->ResponseTimer++;	// 5s 

		// add for MSTP
		//	misc_flags.sendtime = 1;

		if(count_1s < 20)	count_1s++;
		else
		{	
			count_1s = 0;
			if(Modbus.PROTOCAL == BACNET_PTP)
				ps->InactivityTimer++;  // 60s    FOR PTP		
	
			ps->HeartbeatTimer++;  	// 15s
			if(flag_write_mass)
			{	
				Flash_Write_Mass();	
				flag_write_mass = 0;
			}	
			
			control_logic();  // implement this task per 1 second
	
			if( flash.flag)
			{
				switch(flash.table)
				{
					case OUT:	
					memcpy(&Output[flash.index * 40],flash.dat,flash.len);
					break;
					case IN:
					memcpy(&Input[flash.index * 38],flash.dat,flash.len);
					break;
					case PRG:
					default: 
					break;
				}
				flash.flag = 0;
			} 
		
		}
		
	}

}


#endif