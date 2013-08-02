#include "pic.h"
#include "serial.h"
#include "main.h"
#include <string.h>




//#define CALIBRATION_OFFSET    128 //allows us to store FLASH_CALIBRATION as an U8_T

U16_T far temperature[10];
signed int far old_reading[10];
//S16_T xdata mul_analog_in_buffer[10];
//S16_T xdata mul_analog_filter[10];

U16_T  look_up_table(U16_T count);
signed int RangeConverter(unsigned char function, signed int para,unsigned char i,unsigned int cal);


U8_T far input1[8] = 0;
U8_T far input2[8] = 0;
U8_T far count1[8] = 0;
U8_T far count2[8] = 0;
U8_T far counthigh1[8] = 0;
U8_T far counthigh2[8] = 0;	
U8_T far temp1 = 0;
U8_T far temp2 = 0;

#if 1

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
	memset(old_reading,0,20);  
	memset(temperature,0,20);
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



/* per 25ms read 1 channel, 250ms refresh 10 inputs*/
#if 0
void Update_AI_Task(void)
{
	portTickType xDelayPeriod = ( portTickType ) 100 / portTICK_RATE_MS;	 
	static U8_T loop = 0;	
	for( ; ;)
	{ 	
		Test[4]++;
		vTaskDelay(xDelayPeriod);
	//	if( AInputAM & (0x01 << loop) == 0)	    // auto 
		{
			if(loop < AI_CHANNEL)
			{
				if(read_pic(loop))
				{ 
					temperature[loop] =	(RangeConverter(1,AI_Value[loop], loop , Input_CAL[loop]));	 // range is 1				
					loop++;
				}
					
			}	
			else loop = 0; 
		}
	}
}
#endif

void Update_AI(void)
{
	static U8_T loop = 0;	
	if(loop < AI_CHANNEL)
	{
	//	if( AInputAM & (0x01 << loop) == 0)	  
		{
			if(read_pic(loop))
			{ 
				if(protocal <= TCP_IP)					
					temperature[loop] =	(RangeConverter(1,AI_Value[loop], loop , Input_CAL[loop]));	 // range is 1				

				else		
					temperature[loop] =	(RangeConverter(1/*inputs[loop].range*/,inputs[loop].value, loop , inputs[loop].calibration));	
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
		Test[5]++;

		if(WriteFlash == 1) 
		{ 			
			continue;
		}

		DI2_LATCH = 1; KEY_LATCH = 1;RELAY_LATCH = 0;
		DI1_LATCH = 0;
		temp1 = DI1; 
		DI1_LATCH = 1;
		if(protocal <= TCP_IP) 
		{ 
			for(loop = 0;loop < 8;loop++)		
			{
				if(DI_Type[loop] == DI_SWITCH)
				{				
					if(temp1 & (0x01 << loop))
					{
						counthigh1[loop]++;
						if(counthigh1[loop] > 10) // keep high at least 100ms	
						{
							input1[loop] = 1;
							count1[loop] = 0;
							counthigh1[loop] = 0;
						}
					}
					else   // if input is 0, keep low for larger than 50s, it is low.
					{
						if(count1[loop] < 50)  
						{
							count1[loop]++;
						}
						else		// 30ms
						{				
							input1[loop] = 0;
							count1[loop] = 0;
							counthigh1[loop] = 0;
						}
					}
					temp_val |= (input1[loop] << loop);
					temp++;						
					switch_sub_no = temp;
					
					temp_no |= (0x01 << loop);	
					sub_addr[loop] = input1[loop];
				}
				else
					input1[loop] = 0;
			}
		}
		else
		{
			for(loop = 0;loop < 8;loop++)		
			{
				if(inputs[loop].unused == DI_SWITCH)
				{				
					if(temp1 & (0x01 << loop))
					{
						counthigh1[loop]++;
						if(counthigh1[loop] > 10) // keep high at least 100ms	
						{
							input1[loop] = 1;
							count1[loop] = 0;
							counthigh1[loop] = 0;
						}
					}
					else   // if input is 0, keep low for larger than 50s, it is low.
					{
						if(count1[loop] < 50)  
						{
							count1[loop]++;
						}
						else		// 30ms
						{				
							input1[loop] = 0;
							count1[loop] = 0;
							counthigh1[loop] = 0;
						}
					}

					temp_val |= (input1[loop] << loop);
					temp++;						
					switch_sub_no = temp;

					temp_no = 0;
					temp_no |= (0x01 << loop);	
					sub_addr[loop] = input1[loop];
				}
				else
					input1[loop] = 0;
			}
		}
		
		switch_tstat_val = temp_val;
		switch_sub_bit = temp_no;
		
		//EA = 0;
		DI1_LATCH = 1;  KEY_LATCH = 1;RELAY_LATCH = 0;
		DI2_LATCH = 0;
		temp2 = DI2; 
		DI2_LATCH = 1;
		//EA = 1;
		for(loop = 0;loop < 8;loop++)		
		{
			if(temp2 & (0x01 << loop))
			{
				counthigh2[loop]++;
				if(counthigh2[loop] > 10)	
				{
					input2[loop] = 1;
					count2[loop] = 0;
					counthigh2[loop] = 0;
				}
			}
			else   // if input is 0, keep low for larger than 50s, it is low, otherwise it is still high.
			{
				if(count2[loop] < 50)  
				{
					count2[loop]++;
				}
				else		// 50ms
				{				
					input2[loop] = 0;
					count2[loop] = 0;
					counthigh2[loop] = 0;
				}
			}		
		}
	
	}

}


void Update_DI_Task(void)
{
	portTickType xDelayPeriod = ( portTickType ) 100 / portTICK_RATE_MS;
	for( ; ;)
	{ 	
		U8_T loop = 0;	
		Test[6]++;
		vTaskDelay(xDelayPeriod);

		if(protocal <= TCP_IP)					
		for(loop = 0;loop < 8;loop++)
		{
	/* check the input type is AUTO Or Manual */
		
			if(!( DInputAM & (0x01 << loop)))
			{
				 DI1_Value &= ~(0x01 << loop);
				 DI1_Value |= (input1[loop] << loop);
			}

			if(!( DInputAM & (0x01 << (loop + 8))))
			{
				 DI2_Value &= ~(0x01 << loop);
				 DI2_Value |= (input2[loop] << loop);
			}
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




unsigned char const code def_tab[11] =
			{					 // 10k termistor GREYSTONE -40 to 120 Deg.C or -40 to 248 Deg.F 
			 192, 209, 206, 187, 161, 131, 103, 79, 61, 45, 155
			};
//MHF:12-30-05,Added 4 values to make the tstat can measure minus degree
unsigned char const code def_tab_pic[15] =
			{					 // 10k termistor GREYSTONE -40 to 120 Deg.C or -40 to 248 Deg.F 
			 56, 41, 61, 83, 102, 113, 112, 101, 85, 67, 51, 38, 28, 21, 65
 			};


unsigned int   look_up_table(unsigned int count)
{			
	int   far val;
    char  far index=14;
	int   far work_var;
 
	if (1/*pic_exists*/)
		work_var= def_tab_pic[index];
	else
		work_var= def_tab[index];
		  
	if (work_var > count )
	{
		val =  index  * 100 ;
		return ( val );
	}

	do 
	{
		index--;

		if (1/*pic_exists*/)
			work_var += def_tab_pic[index];
		else
			work_var += def_tab[index];

		if( work_var > count)
			{
			val = ( work_var - count )*100;

			if (1/*pic_exists*/)
				val /= def_tab_pic[index];
			else
				val /= def_tab[index];
			if(index >= 4)
			{
				val +=  (index - 4) * 100;
				val = val & 0x7fff;
			}
			else
			{
				val += index*100;
				val = 400 - val;
				val = val | 0x8000;
			}			 
			return (val);
			}
	} while (index) ;

		val =  33768;

		return ( val );
}




/******************************************RangeConverter******************************************/
/*
Description: Convert the  raw data from adc to correspond engineer units.
parameter:	finction,	The engineer units want to get,
			para,		Raw data from ADC
			i, 			Be used for function = 4,customer sensor,because there are only two 
						customer tables,so should check this parameter not bigger than 2 on fun4.
			cal,		calibration data for the correspond input channel
Return:		Changed input to the expected engineer units.	
			
*/
/*********************************RangeConverter funtion start**************************************/
signed int RangeConverter(unsigned char function, signed int para,unsigned char i,unsigned int cal)
{
	signed int far siAdcResult;
	unsigned char far ucFunction;
	unsigned char far ucI;
	signed   int  far siInput;
	unsigned int  far uiCal;
	signed   int  far siResult;
	bit bAnalogInputStatus;
	ucFunction = function;
	siInput = para;
	ucI = i;
	uiCal = cal;

	if(ucFunction == 0)
	{
	 	siResult = siInput + uiCal - CALIBRATION_OFFSET;	
	}	 
	//-----------10K Thermistor---------------
	else if (ucFunction == 1)
	{
 
		siAdcResult = look_up_table(siInput);
 
		//MHF:01-02-06,Added minus temperature display
		if(siAdcResult & 0x8000)
		siResult = -(signed int)(siAdcResult & 0x7fff);
		else
		siResult = siAdcResult;
		//analog_input[i] = adc_result;
		
		if(unit)  
			siResult = (siResult * 9)/5 + 320; 

		// Add the calibration term to the input.
		siResult = siResult + uiCal - CALIBRATION_OFFSET;

		 	
	}
	//-----------0-100%---------------
	else if(ucFunction == 2)  //MHF: Feb 24th 2005 new range setting for analog inputs
	{
		siResult = (float)(siInput)/1023*100;
	}
	//-----------ON/OFF---------------
	else if(ucFunction == 3 || ucFunction == 5)
	{
		siAdcResult = (float)(siInput)/1023*50;
		if(siAdcResult <= 24)
		{
			if(ucFunction == 5)
				bAnalogInputStatus = 1; 
			else if(ucFunction == 3)
				bAnalogInputStatus = 0; 
		}
		else if(siAdcResult >= 26)
		{
			if(ucFunction == 5)
				bAnalogInputStatus = 0; 
			else if(ucFunction == 3)
				bAnalogInputStatus = 1; 
		}
		siResult = (unsigned int)(bAnalogInputStatus);
	 	
	}
	//-----------Custom Sensor---------------
	/*else if(ucFunction == 4 && ucI < 3)
	{
		siAdcResult = look_up_customtable(EEP_TABLE1_ZERO + (22*ucI),siInput,slope_type[ucI]);
		 
		// Add the calibration term to the input.
		siResult = siAdcResult + uiCal - CALIBRATION_OFFSET;
	} */
	
 	return siResult;
}


/**********************************Filter***********************************************************/
/*
Description: Filter the data sampled from ADC to get rid of illegal value caused by noise.
parameter:	channel ,there are eight channels for this product,they should be 0 to 7.if channel = 8,
			it means that the signal from internal thermistor.
			input, Reading from ADC ,need to be filtered
Return:		Filtered value for send to PC or use for other purpose	
			
*/
/*********************************Filter funtion start***********************************************/
unsigned int Filter(unsigned char channel,signed int input)
{
	// -------------FILTERING------------------
 	// -------------FILTERING------------------
	signed int far siDelta;
	signed int far siResult;
    signed int far siTemp;
	signed long far slTemp;
    unsigned char far I;

    I = channel;
	siTemp = input;
 
	siDelta = siTemp - (signed int)old_reading[I] ;    //compare new reading and old reading

	// If the difference in new reading and old reading is greater than 5 degrees, implement rough filtering.
    if (( siDelta >= 100 ) || ( siDelta <= -100 ) ) // deg f
	{
		old_reading[I] = old_reading[I] + (siDelta >> 1);
	}			
	// Otherwise, implement fine filtering.
	else
	{	
		if(protocal <= TCP_IP)
		{
		slTemp = (signed long)Input_Filter[I]*old_reading[I];
		slTemp += (signed long)siTemp;
	 	old_reading[I] = (signed int)(slTemp/(Input_Filter[I] +1));	
		}
		else
		{

		slTemp = (signed long)inputs[I].filter * old_reading[I];
		slTemp += (signed long)siTemp;
	 	old_reading[I] = (signed int)(slTemp/(inputs[I].filter + 1));	 

		}	      	    
				 
    }
	siResult = old_reading[I];
	return siResult;	
}


#endif
