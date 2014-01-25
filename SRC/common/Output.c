#include "main.h"
#include "serial.h"



#if defined(CM5)

xTaskHandle xdata xHandler_Output;

#define UpdateOutputSTACK_SIZE	((unsigned portSHORT)1024)
#define PWMoutputSTACK_SIZE		((unsigned portSHORT)1024)

extern xTaskHandle xdata xKeyTask;


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

void control_output(void);

//BACNET_BINARY_PV Binary_Value_Present_Value( uint32_t object_instance);

void vStartOutputTasks( U8_T uxPriority)
{	
	old_output = 0;
	new_output = 0;
	sTaskCreate( Update_Output_Task, "UpdateOutputtask", UpdateOutputSTACK_SIZE, NULL, uxPriority, &xHandler_Output );
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
	
		new_output = 0;
		for(loop = 0;loop < 10;loop++)
		{
			if(output_raw[loop] >= 512)
			{
				new_output |= (0x01 << loop);
			}
		}
		//#elif (defined(BACDL_MSTP) || defined(BACDL_IP))
	//	#endif
	//	flag_PWM = STABLE;
		/* compare new output and old one, check whether new relay is on,
		if there are new relays are on, must be keep relay on until it is completely truned on,
		then use PWM contronl it */
		if(old_output != new_output) // need keep some relay on , must disalbe PWM contrl now
		{
			Test[36]++;
			old_output = new_output;
			Wakeup_Count = 0; // wake up PWM
			flag_PWM = STABLE;	
		}

//		taskYIELD();
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
// 1ms 
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
#endif

#if defined(MINI)

#include "pca.h"
#include "pca_cfg.h"


#define OutputSTACK_SIZE				( ( unsigned portSHORT ) 256 )

U16_T far relay_value_auto;
UN_RELAY relay_value;


extern void DELAY_Us(U16_T loop);
extern U32_T Sysclk; 
U8_T  AnalogOutput_refresh_index = 0;

//--- variables define --------//

xTaskHandle xHandler_Output;
//extern U8_T data outputs[i].switch_status[24];  // read from top board

//extern U8_T OutputLed[24];

U8_T DigtalOutput_Channel = 0; // the current digtal output channel
U8_T AnalogOutput_Channel = 0; // the current analog output channel


U8_T tick; 
U8_T ccap_low;

void Refresh_Output(void);

/*
 *--------------------------------------------------------------------------------
 * void Initial_PWM(void)
 * Purpose : initial PWM moduel
 * Params  : none
 * Returns : none
 * Note    : used after Intial_E2prom
 //CPS Description
//000 Reference timing tick = operating system clock divided by 25.
//001 Reference timing tick = operating system clock divided by 19.
//010 Reference timing tick = operating system clock divided by 8.
//011 Reference timing tick = operating system clock divided by 6.
//100 Reference timing tick = Timer 0 overflow rate.
//111 Reference timing tick = external clock at ECI pin (the max input

 *--------------------------------------------------------------------------------
 */

U8_T table_pwm[6] = {0 , 4 , 2 ,6 , 1 , 5 /*, 3 , 7*/};


// note: 
void Initial_PWM(void)
{
	U8_T loop;
	U8_T sysclk = 0;
	U32_T Sysclk; 
	sysclk = AX11000_GetSysClk();
	switch (sysclk)
	{
		case SYS_CLK_100M :
			Sysclk = 100000000;
			break;
		case SYS_CLK_50M :
			Sysclk = 50000000;
			break;
		case SYS_CLK_25M :
			Sysclk = 25000000;
			break;
	}
	for(loop = 0;loop < 24;loop++)		outputs[loop].switch_status = 0;

//	_2003_ENALBE = 0;   // enable 2003, otherwise cant drive relays
	CHSEL3 = 0;
	  
	PCA_ModeSetup(0x07,0x40);   //  mode 011, con 0x40  ,so  tick is 6
	tick = 6;
	Sysclk = Sysclk / 6;
	ccap_low = 0x50;
	P3 &= 0xf0;	
	P3 |= 0x07; // dont select any channel
}



/* 
channel:		0 - 7
mode : 			0 - 2	OFF/ AUTO / HAND

*/
/* 
channel:		0 - 7
mode : 			0 - 2	OFF/ AUTO / HAND

*/
void Calucation_PWM_IO(U8_T refresh_index)
{
	U32_T duty_Cycle1,duty_Cycle2;
	U32_T temp1,temp2;
//	U8_T loop;
	U32_T adc1,adc2; /* adc1 is for the first 4051, adc2 is for the second 4051 */
	
	/* CHSEL0 - CHSEL3: P3_0 ~ P3_3*/
//	refresh_index = 0;
	P3 &= 0xf0;	
	P3 |= table_pwm[refresh_index] & 0x0f;
	/* must refresh all channel of the second 4051, the following is the refresh sequence
		C  		B  		A		first_4051			second_4051
		0 		0		0 		analogout1			analogout7
		0		0		1		analogout2			analogout8
		0		1		0		analogout3			analogout9
		0		1		1		analogout4			analogout10
		1		0		0		analogout5			analogout11
		1		0		1		analogout6			analogout12
		1		1		0			-					 -
		1		1		1			-					 -
	*/

	/* Analog OUTPUT1 - OUPUT6*/
	if(Modbus.mini_type == BIG)
	{
		if(outputs[refresh_index + 12].switch_status == SW_OFF)
		{
			adc1 = 0;
		}
		else if(outputs[refresh_index + 12].switch_status == SW_HAND)
		{
			adc1 = 1000;
		}
		else if(outputs[refresh_index + 12].switch_status == SW_AUTO)  
		{
			
			adc1 = output_raw[refresh_index + 12];
		}
	
		/* Analog OUTPUT7 - OUPUT12*/
	
		if(outputs[refresh_index + 18].switch_status == SW_OFF)
		{
			adc2 = 0;
		}
		else if(outputs[refresh_index + 18].switch_status == SW_HAND)
		{
			adc2 = 1000;
		}
		else if(outputs[refresh_index + 18].switch_status == SW_AUTO)
		{
			adc2 = output_raw[refresh_index + 18];
		}
			
		if(adc1 <= 10)	adc1 = 10;
		if(adc2 <= 10)	adc2 = 10;
	
		if((adc1 < 0) || (adc1 > 1000))  return;
		if((adc2 < 0) || (adc2 > 1000))  return;
	
		duty_Cycle1 = adc1 * 1000 / 1000;
		temp1 = 256 * (1000 - duty_Cycle1) / 1000;
		temp1 = temp1 * 256 + ccap_low;
	
		duty_Cycle2 = adc2;
		temp2 = 256 * (1000 - duty_Cycle2) / 1000;
		temp2 = temp2 * 256 + ccap_low;
	
		PCA_ModuleSetup(PCA_MODULE1,PCA_8BIT_PWM,PCA_CCF_ENB,(U16_T)temp1);	
		PCA_ModuleSetup(PCA_MODULE2,PCA_8BIT_PWM,PCA_CCF_ENB,(U16_T)temp2);	
	}
	else if(Modbus.mini_type == SMALL)
	{	/* Analog OUTPUT1 - OUPUT4*/
		if(outputs[refresh_index + 6].switch_status == SW_OFF)
		{
			adc1 = 0;
		}
		else if(outputs[refresh_index + 6].switch_status == SW_HAND)
		{
			adc1 = 1000;
		}
		else if(outputs[refresh_index + 6].switch_status == SW_AUTO)  
		{
			adc1 = output_raw[refresh_index + 6];
		}
		if(adc1 <= 10)	adc1 = 10;
	
		if((adc1 < 0) || (adc1 > 1000))  return;
	
		duty_Cycle1 = adc1;
		temp1 = 256 * (1000 - duty_Cycle1) / 1000;
		temp1 = temp1 * 256 + ccap_low;
	
		PCA_ModuleSetup(PCA_MODULE1,PCA_8BIT_PWM,PCA_CCF_ENB,(U16_T)temp1);	
	}
		

}

/*--------------------------------------------------------------------------------
 * void vStartSPITasks( unsigned char uxPriority)
 * Purpose : start SPI_task and create Queue management for Cmd 
 * Params  : uxPriority - priority for spi_task
 * Returns : none
 * Note    :
 *--------------------------------------------------------------------------------
 */
void vStartOutputTasks( unsigned char uxPriority)
{
	sTaskCreate( Refresh_Output, "OutputTask1", OutputSTACK_SIZE, NULL, uxPriority, &xHandler_Output );

	relay_value.word = 0;
}



void Refresh_Output(void)
{
	portTickType xDelayPeriod = ( portTickType ) 50 / portTICK_RATE_MS;
	U8_T i = 0;			
	U16_T temp1;

	for (;;)
	{
		vTaskDelay(xDelayPeriod);	
		if(Modbus.mini_type == BIG)
		{		
		//	U8_T temp2;
			temp1 = relay_value_auto;
			for(i = 0;i < 12;i++)
			{											  
				if (outputs[i].switch_status == SW_OFF )			temp1 &= ~(BIT0 << i); 		
				else if (outputs[i].switch_status == SW_HAND )	temp1 |= (BIT0 << i);
				else 
				{
					if(output_raw[i] >= 512)						
						temp1 |= (0x01 << i);
					else
						temp1 &= ~(BIT0 << i); 	
				}							
			}

			relay_value.word  = temp1;

			Calucation_PWM_IO(AnalogOutput_refresh_index);	
	
			if(AnalogOutput_refresh_index < 5)	  
				AnalogOutput_refresh_index++;
			else 	
				AnalogOutput_refresh_index = 0;	
					
			
		} 
		else if(Modbus.mini_type == SMALL)
		{
			for(i = 0;i < 6;i++)
			{											  
				if (outputs[i].switch_status == SW_OFF )			temp1 &= ~(BIT0 << i); 		
				else if (outputs[i].switch_status == SW_HAND )	temp1 |= (BIT0 << i);
				else 
				{
					if(output_raw[i] >= 512)						
						temp1 |= (0x01 << i);
					else
						temp1 &= ~(BIT0 << i); 	
				}							
			}

			relay_value.byte[1] = temp1;		
		
			Calucation_PWM_IO(AnalogOutput_refresh_index);	
	
			if(AnalogOutput_refresh_index < 3)	  
				AnalogOutput_refresh_index++;
			else 	
				AnalogOutput_refresh_index = 0;			

		}
	}
} 

#endif

