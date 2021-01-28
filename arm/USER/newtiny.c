#include "main.h"

#if ARM_MINI
#include "controls.h"
// only for new tiny
/*
-> led	
-> switch
-> input 

TOP board deal with these task

*/

#define LED_LATCH_1 PEout(1)
#define LED_LATCH_2 PDout(3)

#define LED_1 PAout(4)
#define LED_2 PEout(4)
#define LED_3 PEout(3)
#define LED_4 PDout(10)//PEout(11)//PAout(3)
#define LED_5 PBout(9)
#define LED_6 PEout(2)
#define LED_7 PEout(13)
#define LED_8 PEout(0)
#define LED_9 PGout(14)
#define LED_10 PFout(11)
#define LED_11 PGout(13)
#define LED_12 PDout(6)
#define LED_13 PGout(12)
#define LED_14 PAout(11)


#define SWITCH_AUTO	 PBout(2)
#define SWITCH_HAND	 PAout(12)



#define SWITCH_1  PBin(7) 
#define SWITCH_2  PGin(6)
#define SWITCH_3  PGin(7)
#define SWITCH_4  PEin(15)
#define SWITCH_5  PEin(14)
#define SWITCH_6  PEin(12)
#define SWITCH_7  PAin(5)
#define SWITCH_8  PEin(5)
#define SWITCH_9  PGin(11)
#define SWITCH_10 PCin(13)
#define SWITCH_11 PGin(10)
#define SWITCH_12 PCin(0)
#define SWITCH_13 PGin(15)
#define SWITCH_14 PGin(9)


#define SEL1_IN   PFout(10)
#define SEL2_IN   PFout(9)
#define SEL3_IN   PFout(8)
#define SEL4_IN   PFout(7)  // ADDED in TB-11I

#define RANGE_SET0			PCout(5)
#define RANGE_SET1			PFout(6)

#define PT1K_SET			PCout(2)  
// 	0-> select PT 1k sensor 
//	1-> select old input type

#define MAX_LEVEL 6


#define IN_NUM 8
#define OUT_NUM 14

#define IN_OUT_NUM IN_NUM + OUT_NUM

unsigned int Filter(unsigned char channel,unsigned int input);

u8 LED_status[IN_OUT_NUM];
u16 LED_Level[IN_OUT_NUM / 16][MAX_LEVEL];
u8 level;

u8_t flag_ready_to_scan = 0;

void LED_IO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	char i,j;

	if(Modbus.mini_type == MINI_NANO)
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);
		
		GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10; 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOG, &GPIO_InitStructure);
		GPIO_SetBits(GPIOG, GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10);
	}
	else  // NEW TINY
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG, ENABLE);
		
		GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4 | GPIO_Pin_11; 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		GPIO_SetBits(GPIOA, GPIO_Pin_4 | GPIO_Pin_11);
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		GPIO_SetBits(GPIOB, GPIO_Pin_9);
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_6 | GPIO_Pin_10; 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOD, &GPIO_InitStructure);
		GPIO_SetBits(GPIOD, GPIO_Pin_3 | GPIO_Pin_6 | GPIO_Pin_10);
		
		GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | /*GPIO_Pin_11 |*/ GPIO_Pin_13; 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOE, &GPIO_InitStructure);
		GPIO_SetBits(GPIOE, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 /*| GPIO_Pin_11*/ | GPIO_Pin_13);
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11; 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOF, &GPIO_InitStructure);
		GPIO_SetBits(GPIOF, GPIO_Pin_11);	
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14; 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOG, &GPIO_InitStructure);
		GPIO_SetBits(GPIOG, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14);
		
		for(j = 0;j < IN_OUT_NUM;j++) LED_status[j] = 0;
		for(i = 0;i < IN_OUT_NUM / 16;i++)
			for(j = 0;j < MAX_LEVEL;j++)
				LED_Level[i][j] = 0;
		
		flag_ready_to_scan = 0;
	}
}


void SWITCH_IO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;


	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0 | GPIO_Pin_13; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_12 | GPIO_Pin_14 | GPIO_Pin_15; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_15; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	// A12 - HAND    B2 - AUTO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_12);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_2);
}



void Check_switch_status(void)
{
	U8_T sw_1[14],sw_2[14];
	U8_T loop;
	U8_T temp;
	U8_T base;
	
	SWITCH_AUTO = 0;
	SWITCH_HAND = 1;
	delay_ms(1);
	sw_1[0] = SWITCH_1;
	sw_1[1] = SWITCH_2;
	sw_1[2] = SWITCH_3;
	sw_1[3] = SWITCH_14;
	sw_1[4] = SWITCH_13;
	sw_1[5] = SWITCH_12;
	sw_1[6] = SWITCH_11;
	sw_1[7] = SWITCH_10;
	sw_1[8] = SWITCH_9;
	sw_1[9] = SWITCH_8;
	sw_1[10] = SWITCH_7;
	sw_1[11] = SWITCH_6;
	sw_1[12] = SWITCH_5;
	sw_1[13] = SWITCH_4;

	SWITCH_AUTO = 1;
	SWITCH_HAND = 0;
	delay_ms(1);
	sw_2[0] = SWITCH_1;
	sw_2[1] = SWITCH_2;
	sw_2[2] = SWITCH_3;
	sw_2[3] = SWITCH_14;
	sw_2[4] = SWITCH_13;
	sw_2[5] = SWITCH_12;
	sw_2[6] = SWITCH_11;
	sw_2[7] = SWITCH_10;
	sw_2[8] = SWITCH_9;
	sw_2[9] = SWITCH_8;
	sw_2[10] = SWITCH_7;
	sw_2[11] = SWITCH_6;
	sw_2[12] = SWITCH_5;
	sw_2[13] = SWITCH_4;
	
	if(Modbus.mini_type == MINI_TINY_11I)
		base = 3;
	else
		base = 0;
	for(loop = base;loop < 14;loop++)
	{	
		temp = outputs[loop - base].switch_status;
		if(sw_1[loop] == sw_2[loop])
			outputs[loop - base].switch_status = SW_OFF;		
		else if(sw_1[loop] == 1) /* from 1 to 0 */
			outputs[loop - base].switch_status = SW_AUTO;
		else  /* from 0 to 1 */
			outputs[loop - base].switch_status = SW_HAND;	

		// if switch is changed
		if(temp != outputs[loop - base].switch_status)
			check_output_priority_HOA(loop - base);
	}
}

//#define LIJUN
void range_set_func(u8 range)
{
	if(range == INPUT_PT1K)
	{
		PT1K_SET = 0;
	}
	else if(range == INPUT_V0_5)
	//if(inputs[channel].range == V0_5 || inputs[channel].range == P0_100_0_5V)
	{PT1K_SET = 1;
		RANGE_SET0 = 1 ;
		RANGE_SET1 = 0 ;
	}
	else if(range == INPUT_0_10V)
	//else if (inputs[channel].range == V0_10_IN)
	{PT1K_SET = 1;
		RANGE_SET0 = 0 ;
		RANGE_SET1 = 1 ;
	}
	else if(range == INPUT_I0_20ma)
	//else if (inputs[channel].range == I0_20ma)
	{PT1K_SET = 1;
		RANGE_SET0 = 0 ;
		RANGE_SET1 = 0 ;
	}
	else
	{PT1K_SET = 1;
		RANGE_SET0 = 1 ;
		RANGE_SET1 = 1 ;
	}

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
  ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 1, ADC_SampleTime_55Cycles5);

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

void Check_PT_sensor(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOF, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 
	GPIO_Init(GPIOF, &GPIO_InitStructure);	
	
	delay_ms(5) ;
// check PB3, if it is high, PT sensor	moudle
	if(PFin(9)) 
	{
		chip_info[2] = 1;
		Setting_Info.reg.specila_flag |= 0x01;
	}
	else
	{
		chip_info[2] = 0;
		Setting_Info.reg.specila_flag &= 0xfe;
	}
	
	chip_info[1] = 42;	// check fw revison in input lib, must set it larger than 42
	Setting_Info.reg.pro_info.firmware_rev = chip_info[1];
}



void Input_IO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC1|RCC_APB2Periph_GPIOC |RCC_APB2Periph_GPIOF, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_5; 	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_SetBits(GPIOC, GPIO_Pin_2);  // PT1K_SET
	GPIO_ResetBits(GPIOC, GPIO_Pin_5);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10; 	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOF, GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10);	
}

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


void inpust_scan(void)
{
	static u8 channel_count = 0;
	u8 i;
	u32 sum;
	u8 first_read;
	u16 adc_temp,min,max;
	sum  = 0;
	first_read = 1;
	for(i = 0;i < 12;i++)
	{
		adc_temp = ADC_getChannal(ADC1,ADC_Channel_11,1);
		sum += adc_temp;
		if(first_read == 1)
		{
			 max = adc_temp;
			 min = adc_temp;
			 first_read = 0;               
		}               
		else
		{
			 max = (max > adc_temp) ? max : adc_temp;
			 min = (min < adc_temp) ? min : adc_temp;
		}  
		delay_us(10);
	}
	
	adc_temp = 1023L * (sum - max - min) / 10 / Modbus.vcc_adc;	
	

//		if(channel_count < 2)
//		{
//			Test[10 + channel_count] = adc_temp;
//			if(adc_temp < 1000) Test[14 + channel_count]++;
//		}	
		
	adc_temp = Filter(channel_count,adc_temp / 4);
	if(adc_temp < 65535)
	{
		input_raw[channel_count] = adc_temp * 4;	

	}

	channel_count++;
	if((Modbus.mini_type == MINI_NEW_TINY) || (Modbus.mini_type == MINI_TINY_ARM))
	{
		channel_count %= 8;
		switch(channel_count)
		{
			case 0: 
				SEL1_IN = 0;	SEL2_IN = 0;	SEL3_IN = 0; 
				break;
			case 1: 
				SEL1_IN = 1;	SEL2_IN = 0;	SEL3_IN = 0; 
				break;
			case 2: 
				SEL1_IN = 0;	SEL2_IN = 1;	SEL3_IN = 0;
				break;
			case 3: 
				SEL1_IN = 1;	SEL2_IN = 1;	SEL3_IN = 0; 
				break;		
			case 4: 
				SEL1_IN = 0;	SEL2_IN = 0;	SEL3_IN = 1; 
				break;	
			case 5: 
				SEL1_IN = 1;	SEL2_IN = 0;	SEL3_IN = 1;
				break;		
			case 6: 
				SEL1_IN = 0;	SEL2_IN = 1;	SEL3_IN = 1; 
				break;		
			case 7: 
				SEL1_IN = 1;	SEL2_IN = 1;	SEL3_IN = 1;
				break;
			default:
				break;	
		}	
	}
	else if(Modbus.mini_type == MINI_TINY_11I)
	{
		channel_count %= 11;
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
			default:
			break;	
		}
		
	}	
	

	range_set_func(input_type[channel_count]);

}



void Count_LED_Buffer(void)
{
	u8 i,j;
	
	if(Modbus.mini_type == MINI_NEW_TINY || Modbus.mini_type == MINI_TINY_ARM)
	{
		memcpy(LED_status,OutputLed,OUT_NUM);
		memcpy(&LED_status[OUT_NUM],InputLed,IN_NUM);
	}
	else if(Modbus.mini_type == MINI_TINY_11I)
	{
		memcpy(LED_status,OutputLed,11);
		memcpy(&LED_status[11],InputLed,11);
		
	}
	
	for(i = 0;i < MAX_LEVEL;i++)
	{		
		for(j = 0;j < IN_OUT_NUM;j++)
		{
			if((LED_status[j] & 0x0f) <= i)	 
			{
				LED_Level[j / 16][i] |= (0x01 << (j % 16));	  
			}
			else 
			{
				LED_Level[j / 16][i] &= ~(0x01 << (j % 16)); 
			}
		}
		 
	} 
}

U8_T led_heart;
// 1ms routions
void scan_led(void)
{  
	static U16_T scanled = 0;
	static U8_T  level1 = 0;
	static U8_T  level2 = 0;

	if((Modbus.mini_type == MINI_NEW_TINY) || (Modbus.mini_type == MINI_TINY_ARM) || (Modbus.mini_type == MINI_TINY_11I))
	{	
		if(flag_ready_to_scan != 1) return;
		if(scanled % 500 == 0) 
		{
			led_heart = ~led_heart;
		}
		scanled++;
		
		if(scanled % 4 == 0)
		{
			LED_LATCH_1 = 1;
			LED_LATCH_2 = 0;

			if(Modbus.mini_type == MINI_NEW_TINY || Modbus.mini_type == MINI_TINY_ARM)
			{
				LED_1 = (LED_Level[0][level1] & 0x01) ? 1 : 0;// DO1
				LED_2 = (LED_Level[0][level1] & 0x02) ? 1 : 0;  // DO2
				LED_3 = (LED_Level[0][level1] & 0x04) ? 1 : 0;  // DO3
				LED_4 = (LED_Level[0][level1] & 0x4000) ? 1 : 0;  // IN0
				LED_5 = (LED_Level[0][level1] & 0x8000) ? 1 : 0;	// IN1
				LED_6 = (LED_Level[1][level1] & 0x01) ? 1 : 0;	// IN2
				LED_7 = (LED_Level[1][level1] & 0x02) ? 1 : 0;	// IN3
				LED_8 = (LED_Level[1][level1] & 0x04) ? 1 : 0;  // IN4
				LED_9 = (LED_Level[1][level1] & 0x08) ? 1 : 0;  // IN5
				LED_10 = (LED_Level[1][level1] & 0x10) ? 1 : 0;	// IN6
				LED_11 = (LED_Level[1][level1] & 0x20) ? 1 : 0;	// IN7		
				LED_12 = flagLED_uart2_tx ? 0 : 1;	// RS485_TX MAIN
				LED_13 = flagLED_uart2_rx ? 0 : 1;	// RS485_RX MAIN
				LED_14 = led_heart;		
			}
			else  // 11I
			{
				LED_1 = (LED_Level[0][level1] & 0x0800) ? 1 : 0;// IN1
				LED_2 = (LED_Level[0][level1] & 0x1000) ? 1 : 0;  // IN2
				LED_3 = (LED_Level[0][level1] & 0x2000) ? 1 : 0;  // IN3
				LED_4 = (LED_Level[0][level1] & 0x4000) ? 1 : 0;  // IN4
				LED_5 = (LED_Level[0][level1] & 0x8000) ? 1 : 0;	// IN5
				LED_6 = (LED_Level[1][level1] & 0x01) ? 1 : 0;	// IN6
				LED_7 = (LED_Level[1][level1] & 0x02) ? 1 : 0;	// IN7
				LED_8 = (LED_Level[1][level1] & 0x04) ? 1 : 0;  // IN8
				LED_9 = (LED_Level[1][level1] & 0x08) ? 1 : 0;  // IN9
				LED_10 = (LED_Level[1][level1] & 0x10) ? 1 : 0;	// IN10
				LED_11 = (LED_Level[1][level1] & 0x20) ? 1 : 0;	// IN11		
				LED_12 = flagLED_uart2_tx ? 0 : 1;	// RS485_TX MAIN
				LED_13 = flagLED_uart2_rx ? 0 : 1;	// RS485_RX MAIN
				LED_14 = led_heart;		
			}
			
			if(level1 < 5)  // 6 level
			{  				
				level1++;
			}
			else
				level1 = 0;	
		}
		else if(scanled % 4 == 2)
		{
		// second block
			LED_LATCH_2 = 1;		
			LED_LATCH_1 = 0;

			if(Modbus.mini_type == MINI_NEW_TINY || Modbus.mini_type == MINI_TINY_ARM)
			{
				LED_1 = (LED_Level[0][level2] & 0x08) ? 1 : 0;	// DO4
				LED_2 = (LED_Level[0][level2] & 0x10) ? 1 : 0;	// DO5
				LED_3 = (LED_Level[0][level2] & 0x20) ? 1 : 0;	// DO6
				LED_4 = (LED_Level[0][level2] & 0x40) ? 1 : 0;	// DO7  ?
				LED_5 = (LED_Level[0][level2] & 0x80) ? 1 : 0;	// DO8
				LED_6 = (LED_Level[0][level2] & 0x100) ? 1 : 0;	// AO1
				LED_7 = (LED_Level[0][level2] & 0x200) ? 1 : 0;  // AO2
				LED_8 = (LED_Level[0][level2] & 0x400) ? 1 : 0;  // AO3  ?
				LED_9 = (LED_Level[0][level2] & 0x800) ? 1 : 0;  // AO4
				LED_10 = (LED_Level[0][level2] & 0x1000) ? 1 : 0;	// AO5
				LED_11 = (LED_Level[0][level2] & 0x2000) ? 1 : 0;	// AO6
				LED_12 = flagLED_uart0_tx ? 0 : 1;	// RS485_TX SUB
				LED_13 = flagLED_uart0_rx ? 0 : 1;	// RS485_RX SUB
				LED_14 = (flagLED_ether_rx || flagLED_ether_tx) ? 0 : 1;   // ETH TX & RX
			}
			else  // 11I
			{
				LED_1 = (LED_Level[0][level2] & 0x01) ? 1 : 0;	// DO1
				LED_2 = (LED_Level[0][level2] & 0x02) ? 1 : 0;	// DO2
				LED_3 = (LED_Level[0][level2] & 0x04) ? 1 : 0;	// DO3
				LED_4 = (LED_Level[0][level2] & 0x08) ? 1 : 0;	// DO4  ?
				LED_5 = (LED_Level[0][level2] & 0x10) ? 1 : 0;	// DO5
				LED_6 = (LED_Level[0][level2] & 0x20) ? 1 : 0;	// DO6
				LED_7 = (LED_Level[0][level2] & 0x40) ? 1 : 0;  // AO1
				LED_8 = (LED_Level[0][level2] & 0x80) ? 1 : 0;  // AO2  ?
				LED_9 = (LED_Level[0][level2] & 0x100) ? 1 : 0;  // AO3
				LED_10 = (LED_Level[0][level2] & 0x200) ? 1 : 0;	// AO4
				LED_11 = (LED_Level[0][level2] & 0x400) ? 1 : 0;	// AO5
				LED_12 = flagLED_uart0_tx ? 0 : 1;	// RS485_TX SUB
				LED_13 = flagLED_uart0_rx ? 0 : 1;	// RS485_RX SUB
				LED_14 = (flagLED_ether_rx || flagLED_ether_tx) ? 0 : 1;   // ETH TX & RX
			}
			if(level2 < 5)  // 6 level
			{  				
				level2++;
			}
			else
				level2 = 0;	
		}
		else //if((scanled % 4 == 1) || (scanled % 4 == 3))
		{
			LED_LATCH_2 = 1;		
			LED_LATCH_1 = 1;
			
			LED_1 = 1;	// DO4
			LED_2 = 1;	// DO5
			LED_3 = 1;	// DO6
			LED_4 = 1;	// DO7
			LED_5 = 1;	// DO8
			LED_6 = 1;	// AO1
			LED_7 = 1;  // AO2
			LED_8 = 1;  // AO3
			LED_9 = 1;  // AO4
			LED_10 = 1;	// AO5
			LED_11 = 1;	// AO6
			LED_12 = 1;	// ETH TX
			LED_13 = 1;	// ETH RX
			LED_14 = 1; // HEART	
			
			if(scanled % 21 == 0)
			{
				if(flagLED_ether_rx == 1) 
					flagLED_ether_rx = 0;
				if(flagLED_ether_tx == 1) 
					flagLED_ether_tx = 0;
			
				if(flagLED_uart2_rx == 1) 
					flagLED_uart2_rx = 0;
				if(flagLED_uart2_tx == 1) 
					flagLED_uart2_tx = 0;
				
				if(flagLED_uart0_rx == 1) 
					flagLED_uart0_rx = 0;
				if(flagLED_uart0_tx == 1) 
					flagLED_uart0_tx = 0;
			}
		}	
	}
	
	if(Modbus.mini_type == MINI_NANO)
	{
		if(scanled % 500 == 0) 
		{
			LED_ROUTER_HEART = ~LED_ROUTER_HEART;
		}
		scanled++;
		if(scanled % 20 == 0)
		{
			LED_ROUTER_SUB_RS485_TX = flagLED_uart0_tx ? 0 : 1;	// RS485_TX SUB
			LED_ROUTER_SUB_RS485_RX = flagLED_uart0_rx ? 0 : 1;	// RS485_RX SUB
			LED_ROUTER_MAIN_RS485_RX = flagLED_uart2_rx ? 0 : 1;	// RS485_RX MAIN
			LED_ROUTER_MAIN_RS485_TX = flagLED_uart2_tx ? 0 : 1;	// RS485_TX MAIN		
			
		}
		
		if(scanled % 21 == 0)
		{
			if(flagLED_uart2_rx == 1) 
					flagLED_uart2_rx = 0;
				if(flagLED_uart2_tx == 1) 
					flagLED_uart2_tx = 0;
				
				if(flagLED_uart0_rx == 1) 
					flagLED_uart0_rx = 0;
				if(flagLED_uart0_tx == 1) 
					flagLED_uart0_tx = 0;
		}
		
	}
}



extern U8_T flag_read_switch;
void Check_Pulse_Counter(void);
void Update_Led(void);
void refresh_led_switch_Task(void) reentrant
{	
	portTickType xDelayPeriod = ( portTickType ) 1000 / portTICK_RATE_MS;
	U8_T loop_1s,i;
	U16_T loop_200ms;
	Check_PT_sensor();
	LED_IO_Init();
	SWITCH_IO_Init();
	Input_IO_Init();
	inputs_adc_init();
	Check_switch_status();
	flag_read_switch = 1;
	// set first channel
	range_set_func(input_type[0]);
	
	for(;;)
	{		
		vTaskDelay( 20 / portTICK_RATE_MS);			
		inpust_scan();		
		if(loop_200ms++ % 10 == 0)	
		{
			Check_Pulse_Counter();
			Update_Led();
			Count_LED_Buffer();
			Check_switch_status();
		}
		
		if(loop_1s < 5)	loop_1s++;
		else
			flag_ready_to_scan = 1;
		
	}
}

#endif