#include "main.h"
#include <string.h>

#if ASIX_CM5 || ARM_CM5
U8_T far input1[8] = 0;
U8_T far count1[8] = 0;
U8_T far counthigh1[8] = 0;
U8_T far DI_value[8] = 0;



void initial_input_value(void)
{
	U8_T loop = 0;
	U8_T count = 0;
	memset(input1,0,8);
	memset(count1,0,8);
	memset(counthigh1,0,8);
	memset(DI_value,0,8); 
}
#endif


#if ASIX_CM5
#include "pic.h"
#include "serial.h"

//#define CALIBRATION_OFFSET    128 //allows us to store FLASH_CALIBRATION as an U8_T

//U16_T far temperature[10];
//signed int far old_reading[10];
//S16_T xdata mul_analog_in_buffer[10];
//S16_T xdata mul_analog_filter[10];

U16_T  look_up_table(U16_T count);
signed int RangeConverter(unsigned char function, signed int para,unsigned char i,unsigned int cal);

#endif


#if ARM_CM5

#include "controls.h"

// DI1 <-->	PC3
// DI2 <-->	PC5
// DI3 <-->	PC13
// DI4 <-->	PB3
// DI5 <-->	PB4
// DI6 <-->	PD3
// DI7 <-->	PB5
// DI8 <-->	PC1
#define DI8 PCin(3)
#define DI7 PCin(5)
#define DI6 PCin(13)
#define DI5 PBin(3)
#define DI4 PBin(4)
#define DI3 PDin(3)
#define DI2 PBin(5)
#define DI1 PCin(1)

#define RANGE_SET0			PBout(6)
#define RANGE_SET1			PBout(7)

#define SEL1_IN		PAout(4)
#define SEL2_IN		PAout(5)
#define SEL3_IN		PAout(6)
#define SEL4_IN		PAout(7)

void DI_IO_initial(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_5 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD,&GPIO_InitStructure);
}

void AI_IO_Initial(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC1|RCC_APB2Periph_GPIOA |RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; // PB0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; 	// MOD_SEL
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
	GPIO_SetBits(GPIOB, GPIO_Pin_1);

	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; 	// TYPE
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7; 	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA, GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);	
}


void inputs_adc_init(void)
{
	ADC_InitTypeDef ADC_InitStructure;
    
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);		
  /* ADC1 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = 1;
  ADC_Init(ADC1, &ADC_InitStructure);

  /* ADC1 regular channel14 configuration */ 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_55Cycles5);

  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);

  /* Enable ADC1 reset calibaration register */   
  ADC_ResetCalibration(ADC1);
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));

  /* Start ADC1 calibaration */
  ADC_StartCalibration(ADC1);
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1));
     
  /* Start ADC1 Software Conversion */ 
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);

}


// PC1 - ADC123_IN11
/* IO initial 
	MOD_SEL - PC2
	INPUT_TYPE0/TYPE1 - PC5,PF6
	CHSEL_IN1 ~ CHSETL_IN3 -> PF10,PF9,PF8
*/




u32 ADC_getChannal(ADC_TypeDef* ADCx, u8 channal,uint8 rank)
{
	uint32_t tem = 0;
	ADC_ClearFlag(ADCx, ADC_FLAG_EOC);
	ADC_RegularChannelConfig(ADCx, channal, rank, ADC_SampleTime_55Cycles5);
	ADC_SoftwareStartConvCmd(ADCx, ENABLE);
	
	while(ADC_GetFlagStatus(ADCx, ADC_FLAG_EOC) == RESET);
	tem = ADC_GetConversionValue(ADCx);
	return tem;        
}

void range_set_func(u8 range)
{
	 if(range == INPUT_V0_5)
	 //if(inputs[channel].range == V0_5 || inputs[channel].range == P0_100_0_5V)
	 {
			RANGE_SET0 = 1 ;
			RANGE_SET1 = 0 ;
	 }
	 else if(range == INPUT_0_10V)
	 //else if (inputs[channel].range == V0_10_IN)
	 {
			RANGE_SET0 = 0 ;
			RANGE_SET1 = 1 ;
	 }
	 else if(range == INPUT_I0_20ma)
	 //else if (inputs[channel].range == I0_20ma)
	 {
			RANGE_SET0 = 0 ;
			RANGE_SET1 = 0 ;
	 }
	 else
	 {
			RANGE_SET0 = 1 ;
			RANGE_SET1 = 1 ;
	 }
}



void inpust_scan(void)
{
	static u8 channel_count = 0;
	u8 i;
	u32 temp;
	temp  = 0;
	for(i = 0;i < 10;i++)
	{
		temp += ADC_getChannal(ADC1,ADC_Channel_8,1);
	}
	
	input_raw[channel_count] = temp / 40;	
	channel_count++;
	channel_count %= 16;
	switch(channel_count)
	{
		case 0: 
			SEL1_IN = 0;	SEL2_IN = 0;	SEL3_IN = 0; SEL4_IN = 0;
			break;
		case 1: 
			SEL1_IN = 1;	SEL2_IN = 0;	SEL3_IN = 0; SEL4_IN = 0;
			break;
		case 2: 
			SEL1_IN = 0;	SEL2_IN = 1;	SEL3_IN = 0; SEL4_IN = 0;
			break;
		case 3: 
			SEL1_IN = 1;	SEL2_IN = 1;	SEL3_IN = 0; SEL4_IN = 0;
			break;		
		case 4: 
			SEL1_IN = 0;	SEL2_IN = 0;	SEL3_IN = 1; SEL4_IN = 0;
			break;	
		case 5: 
			SEL1_IN = 1;	SEL2_IN = 0;	SEL3_IN = 1; SEL4_IN = 0;
			break;		
		case 6: 
			SEL1_IN = 0;	SEL2_IN = 1;	SEL3_IN = 1; SEL4_IN = 0;
			break;		
		case 7: 
			SEL1_IN = 1;	SEL2_IN = 1;	SEL3_IN = 1; SEL4_IN = 0;
			break;		
		case 8: 
			SEL1_IN = 0;	SEL2_IN = 0;	SEL3_IN = 0; SEL4_IN = 1;
			break;
		case 9: 
			SEL1_IN = 1;	SEL2_IN = 0;	SEL3_IN = 0; SEL4_IN = 1;
			break;
		case 10: 
			SEL1_IN = 0;	SEL2_IN = 1;	SEL3_IN = 0; SEL4_IN = 1;
			break;
		case 11: 
			SEL1_IN = 1;	SEL2_IN = 1;	SEL3_IN = 0; SEL4_IN = 1;
			break;		
		case 12: 
			SEL1_IN = 0;	SEL2_IN = 0;	SEL3_IN = 1; SEL4_IN = 1;
			break;	
		case 13: 
			SEL1_IN = 1;	SEL2_IN = 0;	SEL3_IN = 1; SEL4_IN = 1;
			break;		
		case 14: 
			SEL1_IN = 0;	SEL2_IN = 1;	SEL3_IN = 1; SEL4_IN = 1;
			break;		
		case 15: 
			SEL1_IN = 1;	SEL2_IN = 1;	SEL3_IN = 1; SEL4_IN = 1;
		default:
			break;	
	}	
	

	range_set_func(input_type[channel_count]);

}


void Sampel_AI_Task(void)
{
	portTickType xDelayPeriod = ( portTickType ) 50 / portTICK_RATE_MS;
	AI_IO_Initial();
	inputs_adc_init();
	
	for( ; ;)
	{ 	
		vTaskDelay(xDelayPeriod);
		
		inpust_scan();
		
	}
}





/*
input voltage is 24AC, if input is 0 and last at least 50ms, it means the input is low.
else it is high.
sampel frequence is 10ms.
*/

void Sampel_DI_Task(void)
{
	portTickType xDelayPeriod = ( portTickType ) 5 / portTICK_RATE_MS;
  U8_T far temp1 = 0;
	U8_T start_pos = 0;
	char loop = 0;
#if ARM_CM5	
	DI_IO_initial();
#endif
	for( ; ;)
	{ 	
		vTaskDelay(xDelayPeriod);

		start_pos = 16;
#if ASIX_CM5	
    KEY_LATCH = 1;
		DI1_LATCH = 0;
		temp1 = DI1; 
		DI1_LATCH = 1;
		
		
#endif	


		for(loop = 0;loop < 8;loop++)		
		{			
			//if(inputs[loop + start_pos].digital_analog == 0)  // digital
			{
#if ASIX_CM5	
				if(temp1 & (0x01 << loop))
#else
				if(loop == 0)					temp1 = DI1;
				else if(loop == 1)		temp1 = DI2;
				else if(loop == 2)		temp1 = DI3;
				else if(loop == 3)		temp1 = DI4;
				else if(loop == 4)		temp1 = DI5;
				else if(loop == 5)		temp1 = DI6;
				else if(loop == 6)		temp1 = DI7;
				else if(loop == 7)		temp1 = DI8;
				
				if(temp1)
#endif
				{
					counthigh1[loop]++;
					if(counthigh1[loop] > 3) // keep high at least 100ms	
					{
						input1[loop] = 1;
						count1[loop] = 0;
						counthigh1[loop] = 0;
					}
				}
				else   // if input is 0, keep low for larger than 50s, it is low.
				{
					if(count1[loop] < 200)  
					{
						count1[loop]++;
					}
					else		// 50ms
					{				
						input1[loop] = 0;
						count1[loop] = 0;
						counthigh1[loop] = 0;
					}
				}	
				inputs[start_pos + loop].control = input1[loop];
				if(input1[loop] == 1)
					inputs[start_pos + loop].value = 1000;
				else
					inputs[start_pos + loop].value = 0;
			}
//			else
//				input1[loop] = 0;
		}	

	}

}



#endif