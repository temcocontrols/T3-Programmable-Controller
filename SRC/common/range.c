//#include "define.h"
#include "main.h"
 
#if 1
bit pic_exists = 0;

signed int xdata old_reading[32];


unsigned char const code def_tab_tstat[11] =
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
	int   xdata val;
    char  index=14;
	int   xdata work_var;
 
	if (1/*pic_exists*/)
		work_var= def_tab_pic[index];
	else
		work_var= def_tab_tstat[index];
		  
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
			work_var += def_tab_tstat[index];

		if( work_var > count)
			{
			val = ( work_var - count )*100;

			if (1/*pic_exists*/)
				val /= def_tab_pic[index];
			else
				val /= def_tab_tstat[index];
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
	signed int xdata siAdcResult;
	unsigned char xdata ucFunction;
	unsigned char xdata ucI;
	signed   int  xdata siInput;
	unsigned int  xdata uiCal;
	signed   int  xdata siResult;
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
		
		if(Modbus.unit)  
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


unsigned int Filter(unsigned char channel,signed int input)
{
	// -------------FILTERING------------------
 	// -------------FILTERING------------------
	signed int xdata siDelta;
	signed int xdata siResult;
    signed int xdata siTemp;
	signed long xdata slTemp;
    unsigned char xdata I;
	
   
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
		slTemp = (signed long)inputs[I].filter * old_reading[I];
		slTemp += (signed long)siTemp;
	 	old_reading[I] = (signed int)(slTemp/(inputs[I].filter +1));			 
    }
	siResult = old_reading[I];
	return siResult;	
}

#endif