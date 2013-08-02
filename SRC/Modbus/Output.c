#include "main.h"
#include "serial.h"

xTaskHandle far Handle_UpdateOutput;
xTaskHandle far Handle_PWMoutput;

#define UpdateOutputSTACK_SIZE	((unsigned portSHORT)1024)
#define PWMoutputSTACK_SIZE		((unsigned portSHORT)1024)

extern xTaskHandle xKeyTask;

enum
{
	START = 0,STABLE,PWM
};

U16_T far old_output,new_output;
U8_T far flag_PWM = START;
U16_T far Wakeup_Count = 0;
U8_T far PWM_inverse = 0;

bit flag_first_time = 0;

bit flag_priority = 0;


void vStartUpdateOutputTasks( U8_T uxPriority)
{	
	old_output = 0;
	new_output = 0;
	sTaskCreate( Update_Output_Task, "UpdateOutputtask", UpdateOutputSTACK_SIZE, NULL, uxPriority, &Handle_UpdateOutput );
}

/*
void vStartPWMoutputTasks( U8_T uxPriority)
{
	sTaskCreate( PWMoutput, "PWMoutputtask", PWMoutputSTACK_SIZE, NULL, uxPriority, &Handle_PWMoutput );
}
 */
/*
check software switch status, there are 3 types
OFF - all zones operate independently
TIMER - zone1 has priority for PRIORITY time(adjustable)
ON - manual, zone1 has priority always.
*/
void Update_Output_Task(void)
{
	portTickType far xDelayPeriod = ( portTickType ) 500 / portTICK_RATE_MS;
	U8_T  far	loop;
	U8_T far	temp;					   
	U16_T far 	tempvalue;
	U16_T far	tempout1;
	for(;;)
	{
		U8_T  	loop;
		U8_T	temp;					   
		U8_T  	tempvalue;
		U8_T 	tempout1;

		vTaskDelay(xDelayPeriod);
		if(protocal <= TCP_IP)
		{
		#if 0 // 
		if(demo_enable == 1)
		{
			if( OuputAM == 0)
			{	
				tempout1 = 0;
				
				if( DO_SoftSwitch == OFF)    // OFF == 0
				{
				/* all zones are all treated equally , outputs are controlled by inputs */
					for(loop = 0;loop < 8;loop++)
					{						
						temp =  DI2_Value & (0x01 << loop);	
						tempout1 |= temp;
						#if 0
						if(/* Master == 1 ||*/  DI_Type[loop] == CHS_DI1)	/* choose DI1 - DI8*/
						{
							temp =  DI1_Value & (0x01 << loop);				
						}	
						else  /* choose DI9 - DI16*/
						{
							temp =  DI2_Value & (0x01 << loop);
						}
						
						tempout1 |= temp;
						#endif
					}
						
				}
				else if( DO_SoftSwitch == ON)  // ON == 2
				{	
				/*
				Zone 1 has priority until the demand is satisfied on zone1 
			    (when zone1 thermostat goes on, only zone1 pump will be on, The other zones are off. 
			     When zone1 thermostat goes off, the other pumps K4 thru K8 can go on.
				*/	
					temp =  DI2_Value & (0x01 << loop);	
					tempout1 |= temp;
					#if 0
					if(/* Master == 1 ||*/  DI_Type[0] == CHS_DI1)		/* choose DI1 - DI8*/
					{
						temp =  DI1_Value & 0x01;
						tempvalue =  DI1_Value & 0xfe;				
					}
					else			 /* choose DI9 - DI16*/
					{
						temp =  DI2_Value & 0x01;
						tempvalue =  DI2_Value & 0xfe;		
					}
				 #endif
					if(temp == 0)  /* zone 1 is off,other zones can go on */
					{
						//Display_Character(1,"2out on ");
							tempout1 = tempvalue;
						/*for(loop = 1;loop < 8;loop++)
						{
							//tempvalue &= 0x01 ;
							 DO_Value[0] |= tempvalue << loop;
						}*/			
					}
					else /* zone 1 is on, other zones go off */
					{
						//Display_Character(1,"2out off");
						tempout1 = 0x01;
					}
				}	
				else if( DO_SoftSwitch == TIMER)   // TIMER == 1
				{
				/* 
				 Zone 1 has priority for 1-hour. 
				The remaining zones will stop operation until zone 1 is satisfied or until the 1-hour priority has expired
				If K3 is on, a timer starts
				      Before the timer hits one hour, Relays K3 thru K6 will be off. 
				When the timer hits one hour, 
				      Then the K3 thru K6 can follow the associated thermostat input. 
				*/
					if(flag_priority)  /* priotity is expried */
					{/*	When the timer hits one hour, all zones can follow the associated thermostat input. */
						//flag_priority = 0;
						 DO_SoftSwitch = OFF;
						for(loop = 0;loop < 8;loop++)
						{
							temp =  DI2_Value & (0x01 << loop);	
							tempout1 |= temp;
							#if 0
							if(/* Master == 1 ||*/  DI_Type[loop] == CHS_DI1)	/* choose DI1 - DI8*/
							{
								temp =  DI1_Value & (0x01 << loop);				
							}	
							else  /* choose DI9 - DI16*/
							{
								temp =  DI2_Value & (0x01 << loop);
							}
			
							tempout1 |= temp ;
							#endif
						}
					}
					else
					{		
						tempout1 = 0x01;  
					}
				}
				
				//DO_Value = tempout1;		
			}
	
		/*	if(( DO_Value & 0x037f) != 0) // if any relay is on, ture on pump, we define it RELAY8 now
			{
				 DO_Value |= 0x80;
			}*/	
			new_output = tempout1; 
		}
		else
		#endif
			new_output = DO_Value;
		}
		else
		//#elif (defined(BACDL_MSTP) || defined(BACDL_IP))
			for(loop = 0;loop < 10;loop++)
			{
				new_output = outputs[loop].value;
			}
	//	#endif
	//	flag_PWM = STABLE;
		/* compare new output and old one, check whether new relay is on,
		if there are new relays are on, must be keep relay on until it is completely truned on,
		then use PWM contronl it */
		if(old_output != new_output) // need keep some relay on , must disalbe PWM contrl now
		{
			old_output = new_output;
			Wakeup_Count = 0; // wake up PWM
			flag_PWM = STABLE;	
		}
	}

}




void Output_Count_Priority_Task(void)
{
	static U8_T t1 = 0;
	static U8_T t2 = 0;
	#if 0
	if( DO_SoftSwitch == TIMER)
	{	
		// count_priority =  Priority * 60;	
		t2 =  RTC.Clk.sec;
		if(t2 != t1)	// sec changed
		{
			 count_priority++;
			t1 = t2;
		}
		if( count_priority >= 60 *  Priority)  // expried
		{
			flag_priority = 1;
			 count_priority = 0;
		}
	}
	else 
	{
		 count_priority = 0;
		flag_priority = 0;
	}
	#endif
}



extern U8_T far WriteFlash;
// 2ms 
void PWMoutput(void)
{
	U16_T far temp_relay = 0;
	if(WriteFlash == 1)   return;

	if(flag_PWM == STABLE)  // 
	{
		Wakeup_Count++;
		if(Wakeup_Count > 200)
		{
			flag_PWM = PWM;
			PWM_inverse = 0;
		}
		RELAY1_8 = (U8_T)new_output;
		DELAY_Us(5);
	  	RELAY_LATCH = 0; 
		RELAY_LATCH = 1;  		
		DI2_LATCH = 1;
		KEY_LATCH = 1;
		DI1_LATCH = 1;

		if( new_output & 0x100)
			RELAY_9 = 0;
		else 
			RELAY_9 = 1;
	
		/* OUTPUT10  */
		if( new_output & 0x0200)
			RELAY_10 = 0;
		else 
			RELAY_10 = 1;
		
	}
	else  if(flag_PWM == PWM)
	{
	//	Test[41] = new_output;
		if(PWM_inverse == 1 )  
		{ 
			PWM_inverse = 0;  
			temp_relay =  new_output;
		}
		else if(PWM_inverse == 0) 
		{
			PWM_inverse = 1;
			temp_relay = 0;
		}
	/*	if(PWM_inverse < 1 ) 
		{
			PWM_inverse++;  
			temp_relay = DO_Value;
			
		}
		else
		{
		 	PWM_inverse = 0;
			temp_relay =  0;
		} */
//		Test[46] = temp_relay;
		RELAY1_8 = (U8_T)temp_relay;		
		DELAY_Us(5);
	  	RELAY_LATCH = 0; 		
		RELAY_LATCH = 1; 
		DI2_LATCH = 1;
		KEY_LATCH = 1;
		DI1_LATCH = 1;

		if( temp_relay & 0x100)
			RELAY_9 = 0;
		else 
			RELAY_9 = 1;
	
		/* OUTPUT10  */
		if( temp_relay & 0x0200)
			RELAY_10 = 0;
		else 
			RELAY_10 = 1;

	} 

}
