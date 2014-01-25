#include "pic.h"
#include "serial.h"
#include "main.h"
#include <string.h>


#if defined(CM5)



//#define CALIBRATION_OFFSET    128 //allows us to store FLASH_CALIBRATION as an U8_T

U16_T far temperature[10];
//signed int far old_reading[10];
//S16_T xdata mul_analog_in_buffer[10];
//S16_T xdata mul_analog_filter[10];

U16_T  look_up_table(U16_T count);
signed int RangeConverter(unsigned char function, signed int para,unsigned char i,unsigned int cal);
extern float AV_Present_Value[MAX_ANALOG_VALUES];



U8_T far input1[8] = 0;
U8_T far input2[8] = 0;
U8_T far count1[8] = 0;
U8_T far count2[8] = 0;
U8_T far counthigh1[8] = 0;
U8_T far counthigh2[8] = 0;	
U8_T far temp1 = 0;
U8_T far temp2 = 0;


void initial_input_value(void)
{
	U8_T loop = 0;
	U8_T count = 0;
	memset(input1,0,8);
	memset(input2,0,8);
	memset(count1,0,8);
	memset(count2,0,8);
	memset(counthigh1,0,8);
	memset(counthigh2,0,8);  
//	memset(old_reading,0,20);  
//	memset(temperature,0,20);
	// get stable input value power-up
/*	while(count < 5)
	{
		if(loop < AI_CHANNEL)
		{
			
			if(read_pic(loop))
			{
	
				temperature[loop] =	(RangeConverter(1,AI_Value[loop], loop , Input_CAL[loop]));	 // range is 1
				
				loop++;
			}
				
		}	
		else 
		{
			loop = 0; 
			count++; 
		}
	}*/ 
}


void Update_AI(void)
{
	static U8_T loop = 0;	
	if(loop < AI_CHANNEL)
	{
		{
			if(read_pic(loop))
			{ 
				//if(protocal <= TCP_IP)					
				//	temperature[loop] =	(RangeConverter(1,AI_Value[loop], loop , Input_CAL[loop]));	 // range is 1				

//				else
//				{		
					temperature[loop] =	(RangeConverter(1/*inputs[loop].range*/,inputs[loop].value, loop , inputs[loop].calibration));
//					AV_Present_Value[loop] =  temperature[loop];
//				}	
				loop++;
			}
		}	
	}	
	else loop = 0; 
}


/*
input voltage is 24AC, if input is 0 and last at least 50ms, it means the input is low.
else it is high.
sampel frequence is 10ms.
*/

void Sampel_DI_Task(void)
{
	portTickType xDelayPeriod = ( portTickType ) 10 / portTICK_RATE_MS;

	for( ; ;)
	{ 	
		char loop = 0;
		char far temp_no = 0;
		char far temp = 0;
		U8_T far temp_val = 0;	  		

		vTaskDelay(xDelayPeriod);
//		Test[5]++;

		if(WriteFlash == 1) 
		{ 			
			continue;
		}

//		DI2_LATCH = 1; KEY_LATCH = 1;RELAY_LATCH = 0;
//		DI1_LATCH = 0;
//		temp1 = DI1; 
//		DI1_LATCH = 1;
//		for(loop = 0;loop < 8;loop++)		
//		{
//			//if(DI_Type[loop] == DI_SWITCH)
//			if(inputs[loop].unused == DI_SWITCH)
//			{				
//				if(temp1 & (0x01 << loop))
//				{
//					counthigh1[loop]++;
//					if(counthigh1[loop] > 10) // keep high at least 100ms	
//					{
//						input1[loop] = 1;
//						count1[loop] = 0;
//						counthigh1[loop] = 0;
//					}
//				}
//				else   // if input is 0, keep low for larger than 50s, it is low.
//				{
//					if(count1[loop] < 50)  
//					{
//						count1[loop]++;
//					}
//					else		// 30ms
//					{				
//						input1[loop] = 0;
//						count1[loop] = 0;
//						counthigh1[loop] = 0;
//					}
//				}
//				temp_val |= (input1[loop] << loop);
//				temp++;						
//				switch_sub_no = temp;
//				
//				temp_no |= (0x01 << loop);	
//				sub_addr[loop] = input1[loop];
//			}
//			else
//				input1[loop] = 0;
//		}
//		else
//		{
//			for(loop = 0;loop < 8;loop++)		
//			{
//				if(inputs[loop].unused == DI_SWITCH)
//				{				
//					if(temp1 & (0x01 << loop))
//					{
//						counthigh1[loop]++;
//						if(counthigh1[loop] > 10) // keep high at least 100ms	
//						{
//							input1[loop] = 1;
//							count1[loop] = 0;
//							counthigh1[loop] = 0;
//						}
//					}
//					else   // if input is 0, keep low for larger than 50s, it is low.
//					{
//						if(count1[loop] < 50)  
//						{
//							count1[loop]++;
//						}
//						else		// 30ms
//						{				
//							input1[loop] = 0;
//							count1[loop] = 0;
//							counthigh1[loop] = 0;
//						}
//					}
//
//					temp_val |= (input1[loop] << loop);
//					temp++;						
//					switch_sub_no = temp;
//
//					temp_no = 0;
//					temp_no |= (0x01 << loop);	
//					sub_addr[loop] = input1[loop];
//				}
//				else
//					input1[loop] = 0;
//			}
//		}
//		
//		switch_tstat_val = temp_val;
//		switch_sub_bit = temp_no;
//		
//		//EA = 0;
//		DI1_LATCH = 1;  KEY_LATCH = 1;RELAY_LATCH = 0;
//		DI2_LATCH = 0;
//		temp2 = DI2; 
//		DI2_LATCH = 1;
//		//EA = 1;
//		for(loop = 0;loop < 8;loop++)		
//		{
//			if(temp2 & (0x01 << loop))
//			{
//				counthigh2[loop]++;
//				if(counthigh2[loop] > 10)	
//				{
//					input2[loop] = 1;
//					count2[loop] = 0;
//					counthigh2[loop] = 0;
//				}
//			}
//			else   // if input is 0, keep low for larger than 50s, it is low, otherwise it is still high.
//			{
//				if(count2[loop] < 50)  
//				{
//					count2[loop]++;
//				}
//				else		// 50ms
//				{				
//					input2[loop] = 0;
//					count2[loop] = 0;
//					counthigh2[loop] = 0;
//				}
//			}		
//		}
	
	}

}


void Update_DI_Task(void)
{
	portTickType xDelayPeriod = ( portTickType ) 100 / portTICK_RATE_MS;
	for( ; ;)
	{ 	
		U8_T loop = 0;	
//		Test[6]++;
		vTaskDelay(xDelayPeriod);

		if(Modbus.protocal <= TCP_IP)					
		for(loop = 0;loop < 8;loop++)
		{
	/* check the input type is AUTO Or Manual */
		
//			if(!( DInputAM & (0x01 << loop)))
//			{
//				 DI1_Value &= ~(0x01 << loop);
//				 DI1_Value |= (input1[loop] << loop);
//			}
//
//			if(!( DInputAM & (0x01 << (loop + 8))))
//			{
//				 DI2_Value &= ~(0x01 << loop);
//				 DI2_Value |= (input2[loop] << loop);
//			}
		}
		else 

		for(loop = 0;loop < 16;loop++)
		{
	/* check the input type is AUTO Or Manual */
		/*	if(loop < 8)
				if(inputs[loop].auto_manual)
				{
					 inputs[loop].value = input1[loop];
				}
			else
				if(inputs[loop].auto_manual)
				{
					 inputs[loop].value = input2[loop - 8];
				} */
		}		
	
	} 
}  

#endif
