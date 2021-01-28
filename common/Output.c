#include "main.h"
#include "serial.h"


#if ASIX_CM5

xTaskHandle xdata xHandler_Output;

#define UpdateOutputSTACK_SIZE	((unsigned portSHORT)1024)
//#define PWMoutputSTACK_SIZE		((unsigned portSHORT)1024)

//extern xTaskHandle xdata xKeyTask;
U8_T far flag_output;

//U8_T flag_out[10] = {0,0,0,0,0,0,0,0,0,0};
//U8_T relay_status[10] = {0,0,0,0,0,0,0,0,0,0};
//U16_T stable_count[10] = {0,0,0,0,0,0,0,0,0,0};
//U16_T far relay_changed = 0;
//bit flag_priority = 0;

U16_T far old_output,new_output;
//U8_T far flag_PWM = START;
//U16_T far Wakeup_Count = 0;
//U8_T far PWM_inverse = 0;


void control_output(void);


UN_RELAY relay_value;

//BACNET_BINARY_PV Binary_Value_Present_Value( uint32_t object_instance);

void vStartOutputTasks( U8_T uxPriority)
{	
	char i;
	old_output = 0;
	new_output = 0;
	relay_value.word = 0;	
	flag_output = 0;
	for(i = 0;i < CM5_MAX_DOS;i++)
		outputs[i].switch_status = SW_AUTO;	
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
	portTickType far xDelayPeriod = ( portTickType ) 100 / portTICK_RATE_MS;
	U8_T  far	loop;
	U8_T far	temp;					   
	U16_T far tempvalue;
	U16_T far	tempout1;
	task_test.enable[4] = 1;
	for(;;)
	{
		U8_T  loop;
		U8_T	temp;					   
		U8_T  tempvalue;
		U8_T 	tempout1;
		vTaskDelay(xDelayPeriod);
		task_test.count[4] ++;
		new_output = 0;

		for(loop = 0;loop < 8;loop++)
		{				
			if(output_raw[loop] >= 512)
			{
				new_output |= (0x01 << loop);
			}
		}

// inverse RELAY9, RELAY10, hardware 		
		if(output_raw[8] < 512)
		{
			outputs[8].switch_status = SW_AUTO;
			new_output |= (0x01 << 8);
		}
			
		if(output_raw[9] < 512)
		{
			outputs[9].switch_status = SW_AUTO;
			new_output |= (0x01 << 9);
		}
		//#elif (defined(BACDL_MSTP) || defined(BACDL_IP))
	//	#endif
	//	flag_PWM = STABLE;
		/* compare new output and old one, check whether new relay is on,
		if there are new relays are on, must be keep relay on until it is completely truned on,
		then use PWM contronl it */

		if(old_output != new_output) // need keep some relay on , must disalbe PWM contrl now
		{ 			

			old_output = new_output;
			relay_value.word  = new_output;
			if(Modbus.mini_type == MINI_TINY && Modbus.hardRev >= STM_TINY_REV)
			{
				// ARM board, relays are controlled by ARM, not by PIC
				// deal with relay in comm.c, which is communicaion with top board.
			}
			else
			{
				push_cmd_to_picstack(SET_RELAY_LOW,relay_value.byte[1]); 
				push_cmd_to_picstack(SET_RELAY_HI,relay_value.byte[0]);
			}

		}
	}
}

#endif


#if (ARM_MINI || ASIX_MINI || ARM_CM5)
#if ARM_MINI
	// PB.7 -> AO_GP1_EN
	// PC.0 -> AO_GP2_EN
	// PA.13 -> AO_CHSEL0
	// PA.14 -> AO_CHSEL1
	// PB.6 -> AO_CHSEL2
/*   FOR BB's IO defination  */
#define AO_GP1_EN 	PBout(7)
#define AO_GP2_EN 	PCout(0)
#define AO_CHSEL0 	PAout(13)
#define AO_CHSEL1 	PAout(14)
#define AO_CHSEL2 	PBout(6)


/*   FOR LB's IO defination  */
// PB.0 -> AO1 TIM3_CH3
// PB.1 -> AO2 TIM3_CH4
// PB.6 -> AO3 TIM4_CH1
// PB.7 -> AO4 TIM4_CH2

// PA.4 -> AO_FB1 
// PC.0 -> AO_FB2 
// PC.3 -> AO_FB3 
// PC.5 -> AO_FB4 

#define LB_REALY1	PEout(2)
#define LB_REALY2	PEout(3)
#define LB_REALY3	PGout(7)
#define LB_REALY4	PGout(8)
#define LB_REALY5	PGout(9)
#define LB_REALY6	PGout(10)


/*   FOR TB's IO defination  */
// PB.6 -> AO1 TIM4_CH1
// PB.7 -> AO2 TIM4_CH2
// PB.8 -> AO3 TIM4_CH3
// PB.9 -> AO4 TIM4_CH4
// PC.0 -> AO_FB1 ADC123_IN10
// PC.1 -> AO_FB2 ADC123_IN11
// PC.2 -> AO_FB3 ADC123_IN12
// PC.3 -> AO_FB4 ADC123_IN13



#define TB_REALY1	PCout(6)
#define TB_REALY2	PCout(7)
#define TB_REALY3	PBout(8)
#define TB_REALY4	PBout(6)
#define TB_REALY5	PAout(0)
#define TB_REALY6	PAout(1)
#define TB_REALY7	PFout(7)
#define TB_REALY8	PCout(3)  // -> TB11 RELAY5

void Choose_AO(u8 i)
{
	switch(i)
	{
		case 0:
			AO_GP1_EN = 1;AO_GP2_EN = 1;AO_CHSEL0 = 0;AO_CHSEL1 = 0;AO_CHSEL2 = 0;
			break;
		case 1:
			AO_GP1_EN = 1;AO_GP2_EN = 1;AO_CHSEL0 = 1;AO_CHSEL1 = 0;AO_CHSEL2 = 0;
			break;
		case 2:
			AO_GP1_EN = 1;AO_GP2_EN = 1;AO_CHSEL0 = 0;AO_CHSEL1 = 1;AO_CHSEL2 = 0;
			break;
		case 3:
			AO_GP1_EN = 1;AO_GP2_EN = 1;AO_CHSEL0 = 1;AO_CHSEL1 = 1;AO_CHSEL2 = 0;
			break;
		case 4:
			AO_GP1_EN = 1;AO_GP2_EN = 1;AO_CHSEL0 = 0;AO_CHSEL1 = 0;AO_CHSEL2 = 1;
			break;
		case 5:
			AO_GP1_EN = 1;AO_GP2_EN = 1;AO_CHSEL0 = 1;AO_CHSEL1 = 0;AO_CHSEL2 = 1;
			break;
		case 6:
			AO_GP1_EN = 1;AO_GP2_EN = 1;AO_CHSEL0 = 0;AO_CHSEL1 = 0;AO_CHSEL2 = 0;
			break;
		case 7:
			AO_GP1_EN = 1;AO_GP2_EN = 1;AO_CHSEL0 = 1;AO_CHSEL1 = 0;AO_CHSEL2 = 0;
			break;
		case 8:
			AO_GP1_EN = 1;AO_GP2_EN = 1;AO_CHSEL0 = 0;AO_CHSEL1 = 1;AO_CHSEL2 = 0;
			break;
		case 9:
			AO_GP1_EN = 1;AO_GP2_EN = 1;AO_CHSEL0 = 1;AO_CHSEL1 = 1;AO_CHSEL2 = 0;
			break;
		case 10:
			AO_GP1_EN = 1;AO_GP2_EN = 1;AO_CHSEL0 = 0;AO_CHSEL1 = 0;AO_CHSEL2 = 1;
			break;
		case 11:
			AO_GP1_EN = 1;AO_GP2_EN = 1;AO_CHSEL0 = 1;AO_CHSEL1 = 0;AO_CHSEL2 = 1;
			break;
		default:
			break;

	}
		
}

//u16 ADC_getChannal(ADC_TypeDef* ADCx, u8 channal);

#define ADC1_DR_ADDRESS  0x4001244C  
vu16 ADC_ConvertedValue[4];

void LB_AO_FB_Intial(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC1|RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_3 |GPIO_Pin_5;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
//  /* DMA channel1 configuration ----------------------------------------------*/
 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 , ENABLE);

	DMA_DeInit(DMA1_Channel1);
  DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_ADDRESS;
  DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&ADC_ConvertedValue[0];
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = 4;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);
  
  /* Enable DMA channel1 */
  DMA_Cmd(DMA1_Channel1, ENABLE);	
	
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	  /* ADC1 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = 4;
  ADC_Init(ADC1, &ADC_InitStructure);

  /* ADC1 regular channel14 configuration */ 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 1, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 2, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 3, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 4, ADC_SampleTime_55Cycles5);


  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);
  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);

  /* Enable ADC1 reset calibaration register */   
  ADC_ResetCalibration(ADC1);
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));
	/* Start ADC1 calibaration */
  ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
	  /* Start ADC1 Software Conversion */ 
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}
 

void Output_IO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;	

	if((Modbus.mini_type == MINI_BIG) || (Modbus.mini_type == MINI_BIG_ARM))
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;  //TIM3_CH3-4
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		
		// PB.7 -> AO_GP1_EN
		// PC.0 -> AO_GP2_EN
		// PA.13 -> AO_CHSEL0
		// PA.14 -> AO_CHSEL1
		// PB.6 -> AO_CHSEL2
		
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;			
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  						
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);										
		GPIO_ResetBits(GPIOB, GPIO_Pin_6 | GPIO_Pin_7);

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;			
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  						
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOC, &GPIO_InitStructure);										
		GPIO_ResetBits(GPIOC, GPIO_Pin_0);
		
		TIM_TimeBaseStructure.TIM_Period = 1000;
		TIM_TimeBaseStructure.TIM_Prescaler = 0;
		TIM_TimeBaseStructure.TIM_ClockDivision = 0;
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
		TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
		
		TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
		TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
		TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
		
		TIM_OC3Init(TIM3, &TIM_OCInitStructure);
		TIM_OC4Init(TIM3, &TIM_OCInitStructure);
		
		TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
		TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);
		
		TIM_Cmd(TIM3, ENABLE);
	}
	else if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM))
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 , ENABLE);
		RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOG | RCC_APB2Periph_AFIO, ENABLE);
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;  //REALY 1 RELAY 2
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOE, &GPIO_InitStructure);
		GPIO_ResetBits(GPIOE, GPIO_Pin_2 | GPIO_Pin_3);
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;  //REALY 3 RELAY 4 REALY 5 RELAY 6
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOG, &GPIO_InitStructure);
		GPIO_ResetBits(GPIOG, GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10);
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5;		// AO 1,2,3,4	
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  						
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);
		
	
		
		TIM_TimeBaseStructure.TIM_Period = 1000;
		TIM_TimeBaseStructure.TIM_Prescaler = 0;
		TIM_TimeBaseStructure.TIM_ClockDivision = 0;
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
		TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
		
		TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
		TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
		TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
		
		TIM_OC3Init(TIM3, &TIM_OCInitStructure);  // AO1
		TIM_OC4Init(TIM3, &TIM_OCInitStructure);  // AO2
		TIM_OC1Init(TIM3, &TIM_OCInitStructure);  // AO3
		TIM_OC2Init(TIM3, &TIM_OCInitStructure);  // AO4
		
		TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
		TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);
		TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
		TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
		
		TIM_Cmd(TIM3, ENABLE);
		
		LB_AO_FB_Intial();

	}
	else if((Modbus.mini_type == MINI_NEW_TINY) || (Modbus.mini_type == MINI_TINY_ARM))
	{
		RCC_APB1PeriphClockCmd(/*RCC_APB1Periph_TIM2 |*/ RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM5, ENABLE);
		RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);
//		GPIO_PinRemapConfig(GPIO_PartialRemap2_TIM2, ENABLE);
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_3;  //REALY 1 - RELAY 2 RELAY8
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOC,&GPIO_InitStructure);
		GPIO_ResetBits(GPIOC, GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_3);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_8;  //REALY 3 - RELAY 4
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB,&GPIO_InitStructure);
		GPIO_ResetBits(GPIOB, GPIO_Pin_6 | GPIO_Pin_8);
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 /*| GPIO_Pin_2 | GPIO_Pin_3*/;  // REALY 5 - RELAY 6
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		GPIO_ResetBits(GPIOA, GPIO_Pin_0 | GPIO_Pin_1 /*| GPIO_Pin_2 | GPIO_Pin_3*/);
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;  // REALY 7 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOF, &GPIO_InitStructure);
		GPIO_ResetBits(GPIOF, GPIO_Pin_7);
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_6 | GPIO_Pin_7;		// AO 1,2 AO5,AO6
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  						
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);										

		GPIO_InitStructure.GPIO_Pin = /*GPIO_Pin_10 | GPIO_Pin_11 |*/ GPIO_Pin_0 | GPIO_Pin_1;		// AO 3,4
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  						
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);		
	
		TIM_TimeBaseStructure.TIM_Period = 1000;
		TIM_TimeBaseStructure.TIM_Prescaler = 0;
		TIM_TimeBaseStructure.TIM_ClockDivision = 0;
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//		TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
		TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
		TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
		
		TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
		TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
		TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
		
		TIM_OC3Init(TIM5, &TIM_OCInitStructure);  // AO1 PA2 	TIM5_CH3
		TIM_OC4Init(TIM5, &TIM_OCInitStructure);  // AO2 PA3 	TIM5_CH4
//		TIM_OC3Init(TIM2, &TIM_OCInitStructure);  // AO3 PB10 TIM2_CH3
//		TIM_OC4Init(TIM2, &TIM_OCInitStructure);  // AO4 PB11 TIM2_CH4
		TIM_OC3Init(TIM3, &TIM_OCInitStructure);  // AO3 PB0	TIM3_CH3 	
		TIM_OC4Init(TIM3, &TIM_OCInitStructure);  // AO4 PB1	TIM3_CH4	
		TIM_OC1Init(TIM3, &TIM_OCInitStructure);  // AO5 PA6 	TIM3_CH1
		TIM_OC2Init(TIM3, &TIM_OCInitStructure);  // AO6 PA7	TIM3_CH2
		
		TIM_OC3PreloadConfig(TIM5, TIM_OCPreload_Enable);
		TIM_OC4PreloadConfig(TIM5, TIM_OCPreload_Enable);
//		TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);
//		TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);
		TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
		TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);
		TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
		TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
		
//		TIM_Cmd(TIM2, ENABLE);
		TIM_Cmd(TIM3, ENABLE);
		TIM_Cmd(TIM5, ENABLE);
	}
	else if(Modbus.mini_type == MINI_TINY_11I)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM5, ENABLE);
		RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_3;  //REALY 1 - RELAY 2 REALY5
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOC,&GPIO_InitStructure);
		GPIO_ResetBits(GPIOC, GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_3);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_8;  //REALY 3 - RELAY 4
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB,&GPIO_InitStructure);
		GPIO_ResetBits(GPIOB, GPIO_Pin_6 | GPIO_Pin_8);
		
		GPIO_InitStructure.GPIO_Pin = /*GPIO_Pin_0 |*/ GPIO_Pin_1 ;  // REALY 5 - RELAY 6
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		GPIO_ResetBits(GPIOA, /*GPIO_Pin_0 |*/ GPIO_Pin_1);

		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_6;		// AO 1,2 AO5
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  						
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);										

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;		// AO 3,4
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  						
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);		
	
		TIM_TimeBaseStructure.TIM_Period = 1000;
		TIM_TimeBaseStructure.TIM_Prescaler = 0;
		TIM_TimeBaseStructure.TIM_ClockDivision = 0;
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//		TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
		TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
		TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
		
		TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
		TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
		TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
		
		TIM_OC3Init(TIM5, &TIM_OCInitStructure);  // AO1 PA2 	TIM5_CH3
		TIM_OC4Init(TIM5, &TIM_OCInitStructure);  // AO2 PA3 	TIM5_CH4
		TIM_OC3Init(TIM3, &TIM_OCInitStructure);  // AO3 PB0	TIM3_CH3 	
		TIM_OC4Init(TIM3, &TIM_OCInitStructure);  // AO4 PB1	TIM3_CH4	
		TIM_OC1Init(TIM3, &TIM_OCInitStructure);  // AO5 PA6 	TIM3_CH1
		
		TIM_OC3PreloadConfig(TIM5, TIM_OCPreload_Enable);
		TIM_OC4PreloadConfig(TIM5, TIM_OCPreload_Enable);
		TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
		TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);
		TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
		
		TIM_Cmd(TIM3, ENABLE);
		TIM_Cmd(TIM5, ENABLE);
	}
}
	
#endif


#if ARM_CM5

#define CM5_REALY1	PGout(6)
#define CM5_REALY2	PGout(7)
#define CM5_REALY3	PGout(8)
#define CM5_REALY4	PGout(9)
#define CM5_REALY5	PGout(10)
#define CM5_REALY6	PGout(11)
#define CM5_REALY7	PGout(12)
#define CM5_REALY8	PGout(13)
#define CM5_REALY9	PGout(14)
#define CM5_REALY10	PGout(15)

void Output_IO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOG, ENABLE);
//		GPIO_PinRemapConfig(GPIO_PartialRemap2_TIM2, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;  
//REALY 1 - RELAY 10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOG,&GPIO_InitStructure);
	GPIO_ResetBits(GPIOG, GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);


}
	
#endif

#define OutputSTACK_SIZE				( ( unsigned portSHORT ) 1024 )

U16_T far relay_value_auto;
UN_RELAY relay_value;

#define ADJUST_AUTO 1

U8_T far flag_output;
U16_T flag_output_changed;


U8_T  AnalogOutput_refresh_index = 0;

//--- variables define --------//

xTaskHandle xHandler_Output;
//extern U8_T data outputs[i].switch_status[24];  // read from top board

//extern U8_T OutputLed[24];

U8_T DigtalOutput_Channel = 0; // the current digtal output channel
U8_T AnalogOutput_Channel = 0; // the current analog output channel


U8_T tick; 
U8_T ccap_low;
extern U16_T test_adc;
extern U8_T test_adc_flag;



void Refresh_Output(void);
void Calucation_PWM_IO(U8_T refresh_index);


U8_T table_pwm[8];
U8_T slop[10];
//U16_T start_adc[11];
//U8_T const code table_pwm[6] = {0 , 4 , 2 ,6 , 1 , 5 /*, 3 , 7*/};
//U8_T slop[10] = {70,30,28,22,70,90,80,100,80,20};
//U16_T start_adc[10] = {0,70,100,128,160,230,420,600,700,880};



void cal_slop(void)
{
	U8_T i;
	for(i = 0;i < 10;i++)
	{
		slop[i] = Modbus.start_adc[i+1] - Modbus.start_adc[i];
	}
}

//U8_T const code slop[10] = {65,30,25,35,95,170,180,100,80,20};
//U16_T const code start_adc[10] = {0,65,95,120,155,250,420,600,700,880};
//U16_T const code start_adc[10] = {0,100,200,300,400,500,600,700,800,900};
/* add adjust output */
U16_T conver_ADC(U16_T adc)
{
	U8_T far seg;
	U16_T far real_adc = 0;

	if(adc <= 0)	return 0;
	if((adc >= 1000))  return 1000;

	
	seg = adc / 100;
	real_adc = Modbus.start_adc[seg] + (U16_T)(adc % 100) * slop[seg] / 100;
	
	if(real_adc > 1000) return 1000;
	else if(real_adc < 0) return 0;
	
	return real_adc;


}


// Feedback of TB_ARM
#if ARM_MINI || ASIX_MINI

#if ARM_MINI
void scan_AO_FB(void)
{	
	U8_T i;
	static U8_T j = 0;
	u32 temp[4];
	temp[0] = 0;
	temp[1] = 0;
	temp[2] = 0;
	temp[3] = 0;
	for(i = 0; i < 10; i++)
	{
		while((DMA_GetFlagStatus(DMA1_FLAG_TC1)) == RESET);    
		/* Clear DMA TC flag */
		DMA_ClearFlag(DMA1_FLAG_TC1); 
		temp[0] += ADC_ConvertedValue[0];
		temp[1] += ADC_ConvertedValue[1];
		temp[2] += ADC_ConvertedValue[2];
		temp[3] += ADC_ConvertedValue[3];
	}
	AO_feedback[0] = (temp[0] * 1000 / 40 / Modbus.vcc_adc);
	AO_feedback[1] = (temp[1] * 1000 / 40 / Modbus.vcc_adc);
	AO_feedback[2] = (temp[2] * 1000 / 40 / Modbus.vcc_adc);
	AO_feedback[3] = (temp[3] * 1000 / 40 / Modbus.vcc_adc);

}


#endif


U16_T far AO_auto[24];
U8_T far AO_auto_count[12];

void Read_feedback(void);

U16_T Auto_Calibration_AO(U8_T channel,U16_T adc)
{	
//	U8_T step = 0;
	U8_T base;
	U16_T error;
	static U32_T last_update_time[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
	if(Modbus.mini_type == MINI_BIG_ARM)
	{
		if(xTaskGetTickCount() - last_update_time[channel] < 2000)
		{
			return adc;
		}
		last_update_time[channel] = xTaskGetTickCount();
	}
	
	if(AO_auto_count[channel] == 0) // first time, use static, calibrate AO on next time
	{
		AO_auto_count[channel]++;
		return adc;
	}
//	if(flag_output_changed & (0x01 << channel))
//	{
//		if(Modbus.mini_type != MINI_TINY)
//		{
//			Read_feedback();
//		}
//		// if tiny ,get feedback from SPI
//		// FOR big & small, get feedback from PIC
//	}
//	else
//	{
//		return adc;
//	}
	
	AO_auto_count[channel]++;
	if(AO_auto_count[channel] > 20)
		// generate alarm
	{
		AO_auto_count[channel] = 0;
//		flag_output = 0;					
//// generate a alarm
//		generate_common_alarm(ALARM_AO_FB);
	}
	if((Modbus.mini_type == MINI_BIG) || (Modbus.mini_type == MINI_BIG_ARM))	
	{
		base = 12;
	}
	else if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM))
	{
		base = 6;
	}
	else if(Modbus.mini_type == MINI_TINY)
	{
		base = 4;
	}
	else
		return 0;
	if(adc <= 0)	adc = 0;
	if((adc > 1000))  adc = 1000;

	if(output_raw[base + channel] >= 990)
	{
		if(AO_feedback[channel] > 980)
		{
			flag_output_changed &= ~(0x01 << channel);			
		}
		AO_auto_count[channel] = 0;
		return 1000;
	}
	else if(output_raw[base + channel] <= 10)
	{
		if(AO_feedback[channel] < 10)
		{
			flag_output_changed &= ~(0x01 << channel);			
		}
		AO_auto_count[channel] = 0;
		return 0;
	}
	else
	{
		if((AO_feedback[channel] > output_raw[base + channel] - 5)  && (AO_feedback[channel] < output_raw[base + channel] + 5))
		{	// error is 20, in this range, dont adjust
			if(output_raw[base + channel] % 100 == 0)  // 1v 2v ... 10v
			{
#if (ASIX_MINI || ASIX_CM5)
				E2prom_Write_Byte(EEP_OUT_1V + output_raw[base + channel] / 100 - 1,adc / 10);
#endif
				
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
				E2prom_Write_Byte(EEP_OUT_1V + (output_raw[base + channel] / 100 - 1) * 2,adc / 256);
				E2prom_Write_Byte(EEP_OUT_1V + (output_raw[base + channel] / 100 - 1) * 2 + 1,adc % 256);
#endif
				Modbus.start_adc[output_raw[base + channel] / 100] = adc;
			}

			AO_auto_count[channel] = 0;
			flag_output_changed &= ~(0x01 << channel);
			if(AO_feedback[channel] > output_raw[base + channel])
			{
				return adc - 1;// - 3;
			}
			else 
			{
				return adc + 1;//+ 3;
			}
		}
		else
		{			
			if(AO_feedback[channel] > output_raw[base + channel])	  // larger then aim value, decrease adc
			{	
				error = AO_feedback[channel] - output_raw[base + channel];
				if(adc > error)
				{
					if(adc - error / 2 >= 0)
					{
						return (adc - error / 2);						
					}
					else 
					{
						return adc;
					}
				}
				else
				{
					if(adc > 0)
						return adc--;
					else
						return adc;
				}
			}
			else 		// less then aim value, increase adc
			{
				error =  output_raw[base + channel] - AO_feedback[channel];
				if(adc < Modbus.start_adc[10] - error)
				{		
					return (adc + error / 2);
				}
				else
				{
					if(adc < Modbus.start_adc[10])
						return adc++;
					else
						return adc;
				}
			}
		}
	}
}


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
void Initial_PWM(void)
{
	U8_T loop;
#if (ASIX_MINI || ASIX_CM5)
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

#if (ARM_MINI || ASIX_MINI)
	CHSEL3 = 1;
#endif	  
	PCA_ModeSetup(0x07,0x40);   //  mode 011, con 0x40  ,so  tick is 6
	tick = 6;
	Sysclk = Sysclk / 6;
	ccap_low = 0xff;
	
#if (ARM_MINI || ASIX_MINI)
	P3 &= 0xf0;	
	P3 |= 0x07; // dont select any channel 
#endif

	table_pwm[0] = 0;
	table_pwm[1] = 4;
	table_pwm[2] = 2;
	table_pwm[3] = 6;
	table_pwm[4] = 1;
	table_pwm[5] = 5;
	table_pwm[6] = 3;
	table_pwm[7] = 7;	
	
	
#endif

#if ARM_MINI
	Output_IO_Init();
//	flag_output = 0;
#endif
	

	DigtalOutput_Channel = 0; // the current digtal output channel
	AnalogOutput_Channel = 0;


	test_adc = 0;
	test_adc_flag = 0;
	


	flag_output_changed = 0;

	memset(AO_feedback,0,32);
	memset(AO_auto,0,48);
	memset(AO_auto_count,0,12);

	for(loop = 0;loop < 24;loop++)		
		outputs[loop].switch_status = SW_AUTO;

}



 
void Calucation_PWM_IO(U8_T refresh_index)
{
	U32_T duty_Cycle1,duty_Cycle2;
	U32_T temp1,temp2;
//	U8_T loop;
	U32_T far adc1;
	U32_T far adc2; /* adc1 is for the first 4051, adc2 is for the second 4051 */

#if ARM_MINI
	if((Modbus.mini_type == MINI_BIG) || (Modbus.mini_type == MINI_BIG_ARM))
		Choose_AO(refresh_index);
#endif

#if ASIX_MINI		
	CHSEL3 = 1;

	if(Modbus.mini_type == MINI_TINY)
	{
		P3 &= 0xf8;	
		P3 |= table_pwm[refresh_index + 4] & 0x07;
	}
	else if(Modbus.mini_type != MINI_VAV)
	{
		P3 &= 0xf8;	
		P3 |= table_pwm[refresh_index] & 0x07;
	}
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
#endif

	/* Analog OUTPUT1 - OUPUT6*/
	if((Modbus.mini_type == MINI_BIG) || (Modbus.mini_type == MINI_BIG_ARM))
	{
//		if(outputs[refresh_index + 12].switch_status == SW_AUTO)
//		{
			if(outputs[refresh_index + 12].digital_analog == 0) // digital
			{
				if(output_raw[refresh_index + 12] >= 512)
					adc1 = Modbus.start_adc[10];//1000;
				else
					adc1 = 0;
			}
			else // analog
			{
				adc1 = output_raw[refresh_index + 12];
						
				if(flag_output == ADJUST_AUTO)
				{
					adc1 = Auto_Calibration_AO(refresh_index,AO_auto[refresh_index + 12]);
					AO_auto[refresh_index + 12] = adc1;
				}
				else
					adc1 = conver_ADC(adc1);
			}
//		}
//		else if(outputs[refresh_index + 12].switch_status == SW_OFF)
//			adc1 = 0;
//		else if(outputs[refresh_index + 12].switch_status == SW_HAND)
//			adc1 = Modbus.start_adc[10];//1000;
			
			/* Analog OUTPUT7 - OUPUT12*/
//		if(outputs[refresh_index + 18].switch_status == SW_AUTO)		
//		{
			if(outputs[refresh_index + 18].digital_analog == 0) // digital
			{
				if(output_raw[refresh_index + 18] >= 512)
					adc2 = Modbus.start_adc[10];//1000;
				else
					adc2 = 0;
			}
			else // analog
			{
				adc2 = output_raw[refresh_index + 18];
				if(flag_output == ADJUST_AUTO)
				{ 
					adc2 = Auto_Calibration_AO(refresh_index + 6,AO_auto[refresh_index + 18]);
					AO_auto[refresh_index + 18] = adc2;
				}
				else
					adc2 = conver_ADC(adc2);	
			}
//		}
//		else if(outputs[refresh_index + 18].switch_status == SW_OFF)
//			adc2 = 0;
//		else if(outputs[refresh_index + 18].switch_status == SW_HAND)
//			adc2 = Modbus.start_adc[10];//1000;
		// if output is used for a digtal output, do not use feedback to adjust

		
			


		if(refresh_index == 0) 
		{
			if(test_adc_flag == 1)	adc1 = test_adc;
			test_adc = adc1;
		}
		
		if(outputs[refresh_index + 12].range == 0)
			adc1 = 0;
			
		if(outputs[refresh_index + 18].range == 0)
			adc2 = 0;
		
		duty_Cycle1 = adc1;
		if(duty_Cycle1 > Modbus.start_adc[10]) duty_Cycle1 = Modbus.start_adc[10];//1000;

	
		duty_Cycle2 = adc2;
		if(duty_Cycle2 > Modbus.start_adc[10]) duty_Cycle2 = Modbus.start_adc[10];//1000;
		
#if ASIX_MINI
		temp1 = (U32_T)255 * (1000 - duty_Cycle1) / 1000;
		temp1 = temp1 * 256 + ccap_low;
		temp2 = (U32_T)255 * (1000 - duty_Cycle2) / 1000;
		temp2 = temp2 * 256 + ccap_low;
		PCA_ModuleSetup(PCA_MODULE1,PCA_8BIT_PWM,PCA_CCF_ENB,(U16_T)temp1);	
		PCA_ModuleSetup(PCA_MODULE2,PCA_8BIT_PWM,PCA_CCF_ENB,(U16_T)temp2);	

#endif		

#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )		
		TIM_SetCompare3(TIM3, duty_Cycle1);
		TIM_SetCompare4(TIM3, duty_Cycle2);
#endif
	}
	else if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM))
	{	/* Analog OUTPUT1 - OUPUT4*/			

//		if(outputs[refresh_index + 6].switch_status == SW_AUTO)
//		{
			if(outputs[refresh_index + 6].digital_analog == 0) // digital
			{
				if(output_raw[refresh_index + 6] >= 512)
					adc1 = Modbus.start_adc[10];//1000;
				else
					adc1 = 0;
			}
			else // analog
			{
				adc1 = output_raw[refresh_index + 6];
				
				if(flag_output == ADJUST_AUTO)
				{
					adc1 = Auto_Calibration_AO(refresh_index,AO_auto[refresh_index + 6]);
					AO_auto[refresh_index + 6] = adc1;
				}
				else
					adc1 = conver_ADC(adc1);
			}
//		}
//		else if(outputs[refresh_index + 6].switch_status == SW_OFF)
//			adc1 = 0;
//		else if(outputs[refresh_index + 6].switch_status == SW_HAND)
//			adc1 = Modbus.start_adc[10];//1000;
		
// if output is used for a digtal output, do not use feedback to adjust


		if(refresh_index == 0) 
		{
			if(test_adc_flag == 1)	adc1 = test_adc;
			test_adc = adc1;
		}
		
		if(outputs[refresh_index + 6].range == 0)		
			adc1 = 0;

		duty_Cycle1 = adc1;
		if(duty_Cycle1 > Modbus.start_adc[10]) duty_Cycle1 = Modbus.start_adc[10];//1000;
		temp1 = (U32_T)255 * (1000 - duty_Cycle1) / 1000;
		temp1 = temp1 * 256 + ccap_low;	

#if ASIX_MINI
		PCA_ModuleSetup(PCA_MODULE1,PCA_8BIT_PWM,PCA_CCF_ENB,(U16_T)temp1);	
#endif

#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
		if(refresh_index == 0)		
			TIM_SetCompare3(TIM3, duty_Cycle1);			
		else if(refresh_index == 1)
			TIM_SetCompare4(TIM3, duty_Cycle1);
		else if(refresh_index == 2)
			TIM_SetCompare1(TIM3, duty_Cycle1);
		else if(refresh_index == 3)
			TIM_SetCompare2(TIM3, duty_Cycle1);
#endif
	}
#if ASIX_MINI
	else if(Modbus.mini_type == MINI_TINY)
	{	/* Analog OUTPUT1 - OUPUT4*/
//		if(outputs[refresh_index + 4].switch_status == SW_AUTO)
//		{
			if(outputs[refresh_index + 4].digital_analog == 0) // digital
			{
				if(output_raw[refresh_index + 4] >= 512)
					adc1 = Modbus.start_adc[10];//1000;
				else
					adc1 = 0;
			}
			else // analog
			{
				adc1 = output_raw[refresh_index + 4];
				if(flag_output == ADJUST_AUTO)
				{					
				 adc1 = Auto_Calibration_AO(refresh_index,AO_auto[refresh_index + 4]);
				 AO_auto[refresh_index + 4] = adc1;
				}
				else
					adc1 = conver_ADC(adc1);

			}
//		}
//		else if(outputs[refresh_index + 4].switch_status == SW_OFF)
//			adc1 = 0;
//		else if(outputs[refresh_index + 4].switch_status == SW_HAND)
//			adc1 = Modbus.start_adc[10];//1000;

// if output is used for a digtal output, do not use feedback to adjust

		
		if(refresh_index == 0) 
		{
			if(test_adc_flag == 1)	
				adc1 = test_adc;
			test_adc = adc1;
		}

		duty_Cycle1 = adc1;
		if(duty_Cycle1 > Modbus.start_adc[10]) duty_Cycle1 = Modbus.start_adc[10];//1000;
		temp1 = (U32_T)255 * (1000 - duty_Cycle1) / 1000;
		temp1 = temp1 * 256 + ccap_low;		
		
		//if(outputs[refresh_index + 4].digital_analog == 1)	
			PCA_ModuleSetup(PCA_MODULE1,PCA_8BIT_PWM,PCA_CCF_ENB,(U16_T)temp1);	

	}
#endif
	
#if ARM_MINI
	else if((Modbus.mini_type == MINI_NEW_TINY) || (Modbus.mini_type == MINI_TINY_ARM) )
	{
//		if(outputs[refresh_index + 8].switch_status == SW_AUTO)
//		{
			if(outputs[refresh_index + 8].digital_analog == 0) // digital
			{
				if(output_raw[refresh_index + 8] >= 512)
					adc1 = Modbus.start_adc[10];
				else
					adc1 = 0;
			}
			else // analog
			{
				adc1 = output_raw[refresh_index + 8];
				
				if(flag_output == ADJUST_AUTO)
				{					
				 adc1 = Auto_Calibration_AO(refresh_index,AO_auto[refresh_index + 8]);
				 AO_auto[refresh_index + 8] = adc1;
				}
				else
				{
					adc1 = conver_ADC(adc1);
				}
			}
//		}
//		else if(outputs[refresh_index + 8].switch_status == SW_OFF)
//			adc1 = 0;
//		else if(outputs[refresh_index + 8].switch_status == SW_HAND)
//			adc1 = Modbus.start_adc[10];//1000;
// if output is used for a digtal output, do not use feedback to adjust

		
		if(refresh_index == 0) 
		{
			if(test_adc_flag == 1)	
				adc1 = test_adc;
			test_adc = adc1;
		}
		
		duty_Cycle1 = adc1;
		if(duty_Cycle1 > Modbus.start_adc[10]) duty_Cycle1 = Modbus.start_adc[10];
		temp1 = (U32_T)255 * (1000 - duty_Cycle1) / 1000;
		temp1 = temp1 * 256 + ccap_low;		
		
		if(refresh_index == 0)
			TIM_SetCompare3(TIM5, duty_Cycle1);	
		else if(refresh_index == 1)
			TIM_SetCompare4(TIM5, duty_Cycle1);
//		else if(refresh_index == 2)
//			TIM_SetCompare3(TIM2, duty_Cycle1);	
//		else if(refresh_index == 3)
//			TIM_SetCompare4(TIM2, duty_Cycle1);
		else if(refresh_index == 2)
			TIM_SetCompare3(TIM3, duty_Cycle1);
		else if(refresh_index == 3)
			TIM_SetCompare4(TIM3, duty_Cycle1);
		else if(refresh_index == 4)
			TIM_SetCompare1(TIM3, duty_Cycle1);	
		else if(refresh_index == 5)
			TIM_SetCompare2(TIM3, duty_Cycle1);
	}
	else if(Modbus.mini_type == MINI_TINY_11I )
	{ // 6DO5AO
			if(outputs[refresh_index + 6].digital_analog == 0) // digital
			{
				if(output_raw[refresh_index + 6] >= 512)
					adc1 = Modbus.start_adc[10];
				else
					adc1 = 0;
			}
			else // analog
			{
				adc1 = output_raw[refresh_index + 6];
				
				if(flag_output == ADJUST_AUTO)
				{					
				 adc1 = Auto_Calibration_AO(refresh_index,AO_auto[refresh_index + 6]);
				 AO_auto[refresh_index + 6] = adc1;
				}
				else
				{
					adc1 = conver_ADC(adc1);
				}
			}


		
		if(refresh_index == 0) 
		{
			if(test_adc_flag == 1)	
				adc1 = test_adc;
			test_adc = adc1;
		}
		
		duty_Cycle1 = adc1;
		if(duty_Cycle1 > Modbus.start_adc[10]) duty_Cycle1 = Modbus.start_adc[10];
		temp1 = (U32_T)255 * (1000 - duty_Cycle1) / 1000;
		temp1 = temp1 * 256 + ccap_low;		
		
		if(refresh_index == 0)
			TIM_SetCompare3(TIM5, duty_Cycle1);	
		else if(refresh_index == 1)
			TIM_SetCompare4(TIM5, duty_Cycle1);
		else if(refresh_index == 2)
			TIM_SetCompare3(TIM3, duty_Cycle1);
		else if(refresh_index == 3)
			TIM_SetCompare4(TIM3, duty_Cycle1);
		else if(refresh_index == 4)
			TIM_SetCompare1(TIM3, duty_Cycle1);	
	}
#endif
#if ASIX_MINI 
	else if(Modbus.mini_type == MINI_VAV)
	{
		adc1 = output_raw[0];
		adc2 = output_raw[1];
		
		duty_Cycle1 = adc1;
		if(duty_Cycle1 > Modbus.start_adc[10]) duty_Cycle1 = Modbus.start_adc[10];
		temp1 = (U32_T)255 * (1000 - duty_Cycle1) / 1000;
		temp1 = temp1 * 256 + ccap_low;

		duty_Cycle2 = adc2;
		if(duty_Cycle2 > Modbus.start_adc[10]) duty_Cycle2 = Modbus.start_adc[10];
		temp2 = (U32_T)255 * (1000 - duty_Cycle2) / 1000;
		temp2 = temp2 * 256 + ccap_low;

		PCA_ModuleSetup(PCA_MODULE2,PCA_8BIT_PWM,PCA_CCF_ENB,(U16_T)temp1);	
		PCA_ModuleSetup(PCA_MODULE3,PCA_8BIT_PWM,PCA_CCF_ENB,(U16_T)temp2);		
	}	
	CHSEL3 = 0;

#endif	
	

}

#endif
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
#if (ARM_MINI || ASIX_MINI)
	Initial_PWM();
#endif

#if ARM_CM5
	Output_IO_Init();
#endif	
	sTaskCreate( Refresh_Output, "OutputTask1", OutputSTACK_SIZE, NULL, uxPriority, &xHandler_Output );

	relay_value.word = 0;
}


//#ifdef VAV
// level range is 0 - 100
void SET_VAV(U8_T level)
{
	uart_init_send_com(1);
	uart_send_byte(0xff,1);
	uart_send_byte(0x00,1);
	uart_send_byte(level,1);
	uart_send_byte((U8_T)(0xff + 0x00 + level),1);
}

void READ_VAV(U8_T *level)
{
	U8_T length;
	uart_init_send_com(1);
	uart_send_byte(0xff,1);
	uart_send_byte(0x01,1);
	uart_send_byte(0x00,1);
	uart_send_byte((U8_T)(0xff + 0x01 + 0),1);
	set_subnet_parameters(RECEIVE,4,1);
	if(length = wait_subnet_response(500,1))
	{
		if(subnet_response_buf[0] == 0xff && subnet_response_buf[1] == 0x01 &&  
			subnet_response_buf[3] == (U8_T)(subnet_response_buf[0] + subnet_response_buf[1] + subnet_response_buf[2]))
		{
			*level = subnet_response_buf[2];
		}
	}
	else
	{
		
	}
		
}

//#endif
U8_T flag_read_switch;

void Refresh_Output(void)
{
	portTickType xDelayPeriod = ( portTickType ) 50 / portTICK_RATE_MS;
	U8_T i = 0;	
	UN_RELAY temp1;
	U16_T far pwm_count[12];
	U8_T DO_change_count;
	U8_T flag_DO_changed;
	U8_T auto_output_count;
	task_test.enable[5] = 1;
	DO_change_count = 0;
	for(i = 0;i < 12;i++)
		pwm_count[i] = 0;
	auto_output_count = 0;
	flag_read_switch = 0;
	for (;;)
	{
		vTaskDelay( 50 / portTICK_RATE_MS);	
		task_test.count[5]++;
		current_task = 5;
		
		// two reasion to put check trenlog in outputtask, it is important!!! Task priority is not able to changed. 
		// 1. this task has higher priority than task dealing with commuinciton with top board(this requirment is only for LB, SPI bus is share in LB)
		// 2. this task has lower priorty than programm task.( TIME funcition care about timing, and reading SD takes long time,
		//	so have to set lower pri then programming)
		check_trendlog_1s(20);
		
		// wait reading switch_status
#if (ARM_MINI || ASIX_MINI)
		if(flag_read_switch == 1) 
		{			
			if((Modbus.mini_type == MINI_BIG) || (Modbus.mini_type == MINI_BIG_ARM))
			{		
			//	U8_T temp2;
				temp1.word = relay_value_auto;
				
				for(i = 0;i < 12;i++)
				{							
						if(outputs[i].digital_analog == 0)  // digital
						{
							if(output_raw[i] >= 512)						
								temp1.word |= (0x01 << i);
							else
								temp1.word &= ~(BIT0 << i); 
						}
						else  // analog  , PWM mode
						{
							if(pwm_count[i] < (U32_T)outputs[i].pwm_period * 20)
							{
								pwm_count[i]++;
							}
							else
								pwm_count[i] = 0;
							
							if(pwm_count[i] < (U32_T)outputs[i].pwm_period * 20 * output_raw[i] / 1023)
							{
								temp1.word |= (0x01 << i);
							}
							else 
							{
								temp1.word &= ~(BIT0 << i); 
							}
						}
											
				}			
					
				
				if(temp1.byte[0] != relay_value.byte[0])
				{
					flag_DO_changed = 1;
					DO_change_count = 0;
					relay_value.byte[0]  = temp1.byte[0];	
	#if ARM_MINI
					push_cmd_to_picstack(SET_RELAY_LOW,relay_value.byte[0]); 
	#endif
					
	#if ASIX_MINI
					push_cmd_to_picstack(SET_RELAY_HI,relay_value.byte[0]);
	#endif		
				}
				
				if(temp1.byte[1] != relay_value.byte[1])
				{
					flag_DO_changed = 1;
					DO_change_count = 0;
					relay_value.byte[1]  = temp1.byte[1];	
	#if ARM_MINI
					push_cmd_to_picstack(SET_RELAY_HI,relay_value.byte[1]);
	#endif
					
	#if ASIX_MINI
					push_cmd_to_picstack(SET_RELAY_LOW,relay_value.byte[1]); 
	#endif					
				}

				if(flag_DO_changed) 
				{
					DO_change_count++;
					if(DO_change_count > 200)  // keep stable for 500ms second,send command to pic again
					{
						flag_DO_changed = 0;
						DO_change_count = 0;
	#if ARM_MINI
					push_cmd_to_picstack(SET_RELAY_LOW,relay_value.byte[0]); 
					push_cmd_to_picstack(SET_RELAY_HI,relay_value.byte[1]);
	#endif
					
	#if ASIX_MINI
					push_cmd_to_picstack(SET_RELAY_HI,relay_value.byte[0]);
					push_cmd_to_picstack(SET_RELAY_LOW,relay_value.byte[1]); 
	#endif				
					}
					
				}
				
	// check 12 AO, whether analog output is changed
				for(i = 0;i < 12;i++)
				{
					if(output_raw[i + 12] != output_raw_back[i + 12])
					{				
						output_raw_back[i + 12] = output_raw[i + 12];
						AO_auto[i + 12] = conver_ADC(output_raw[i + 12]);
						flag_output_changed |= (0x01 << i);
						AO_auto_count[i] = 0;
					}

					if(AO_auto_count[i] > 20)
						// generate alarm
					{
						AO_auto_count[i] = 0;
						flag_output = 0;
					// generate a alarm
						generate_common_alarm(ALARM_AO_FB);
					}
				}			

				auto_output_count++;
				//if(auto_output_count % 5 == 0)  // 250ms
				{					
					Calucation_PWM_IO(AnalogOutput_refresh_index);
						
					if(AnalogOutput_refresh_index < 5)	  
						AnalogOutput_refresh_index++;
					else 	
						AnalogOutput_refresh_index = 0;						
				}
			} 
			else if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM))
			{		
				
				for(i = 0;i < 6;i++)
				{	
					if(outputs[i].digital_analog == 0)  // digital
					{
						if(output_raw[i] >= 512)						
								temp1.word |= (0x01 << i);
						else
							temp1.word &= ~(BIT0 << i);
					}	
					else  // analog  , PWM mode
					{
						if(pwm_count[i] < (U32_T)outputs[i].pwm_period * 20)
						{
							pwm_count[i]++;
						}
						else
							pwm_count[i] = 0;
						
						if(pwm_count[i] < (U32_T)outputs[i].pwm_period * 20 * output_raw[i] / 1023)
						{
							temp1.word |= (0x01 << i);
						}
						else 
						{
							temp1.word &= ~(BIT0 << i); 
						}
					}	
				}

				// check 4 AO, whether analog output is changed
				for(i = 0;i < 4;i++)
				{
					if(output_raw[i + 6] != output_raw_back[i + 6])
					{				
						output_raw_back[i + 6] = output_raw[i + 6];
						AO_auto[i + 6] = conver_ADC(output_raw[i + 6]);
						flag_output_changed |= (0x01 << i);
						AO_auto_count[i] = 0;
					}
					
	//				if(AO_auto_count[i] > 50)
	//					// generate alarm
	//				{
	//					AO_auto_count[i] = 0;
	////					flag_output = 0;
	//	// generate a alarm
	//					generate_common_alarm(ALARM_AO_FB);
	//				}
					
				}	
				
				if(temp1.word != relay_value.word)
				{
					flag_DO_changed = 1;
					DO_change_count = 0;
					
					relay_value.word = temp1.word;
	#if ARM_MINI
					if(relay_value.byte[0] & 0x01) 					LB_REALY1 = 1;				else					LB_REALY1 = 0;
					if(relay_value.byte[0] & 0x02) 					LB_REALY2 = 1;				else					LB_REALY2 = 0;
					if(relay_value.byte[0] & 0x04) 					LB_REALY3 = 1;				else					LB_REALY3 = 0;
					if(relay_value.byte[0] & 0x08) 					LB_REALY4 = 1;				else					LB_REALY4 = 0;
					if(relay_value.byte[0] & 0x10) 					LB_REALY5 = 1;				else					LB_REALY5 = 0;
					if(relay_value.byte[0] & 0x20) 					LB_REALY6 = 1;				else					LB_REALY6 = 0;				
	#endif
					
	#if ASIX_MINI
					push_cmd_to_picstack(SET_RELAY_LOW,relay_value.byte[1]); 
	#endif	
				}
				
				if(flag_DO_changed) 
				{
					DO_change_count++;
					if(DO_change_count > 200)  // keep stable for 500ms second,send command to pic again
					{
						flag_DO_changed = 0;
						DO_change_count = 0;
					
	#if ARM_MINI
					if(relay_value.byte[0] & 0x01) 					LB_REALY1 = 1;				else					LB_REALY1 = 0;
					if(relay_value.byte[0] & 0x02) 					LB_REALY2 = 1;				else					LB_REALY2 = 0;
					if(relay_value.byte[0] & 0x04) 					LB_REALY3 = 1;				else					LB_REALY3 = 0;
					if(relay_value.byte[0] & 0x08) 					LB_REALY4 = 1;				else					LB_REALY4 = 0;
					if(relay_value.byte[0] & 0x10) 					LB_REALY5 = 1;				else					LB_REALY5 = 0;
					if(relay_value.byte[0] & 0x20) 					LB_REALY6 = 1;				else					LB_REALY6 = 0;				
	#endif
					
	#if ASIX_MINI
					push_cmd_to_picstack(SET_RELAY_LOW,relay_value.byte[1]); 
	#endif	
					}
				}
				
	#if ARM_MINI
				scan_AO_FB();	
	#endif			
				auto_output_count++;
				if(auto_output_count % 5 == 0)  // 250ms
				{
					Calucation_PWM_IO(AnalogOutput_refresh_index);	
			
					if(AnalogOutput_refresh_index < 3)	  
						AnalogOutput_refresh_index++;
					else 	
						AnalogOutput_refresh_index = 0;			
				}

			}
				
	#if ASIX_MINI
			else if(Modbus.mini_type == MINI_TINY)
			{
				
				for(i = 0;i < 6;i++)
				{	
					// check range of OUT5 & OUT6, they are abled to config to AO or DO
					if( ((i == 4) || (i == 5)) && (outputs[i].digital_analog == 1)) //  analog
					{
						temp1.word &= ~(BIT0 << i); 		
					}
					else
					{
//						if(outputs[i].switch_status == SW_AUTO)
//						{						
							if(outputs[i].digital_analog == 0)  // digital
							{
								if(output_raw[i] >= 512)						
										temp1.word |= (0x01 << i);
								else
									temp1.word &= ~(BIT0 << i);
							}	
							else  // analog  , PWM mode
							{
								if(pwm_count[i] < (U32_T)outputs[i].pwm_period * 20)
								{
									pwm_count[i]++;
								}
								else
									pwm_count[i] = 0;
								
								if(pwm_count[i] < (U32_T)outputs[i].pwm_period * 20 * output_raw[i] / 1023)
								{
									temp1.word |= (0x01 << i);
								}
								else 
								{
									temp1.word &= ~(BIT0 << i); 
								}
							}
//						}
//						else if (outputs[i].switch_status == SW_OFF )			
//							temp1.word &= ~(BIT0 << i); 		
//						else if (outputs[i].switch_status == SW_HAND )	
//							temp1.word |= (BIT0 << i);
					}
				}		
				
							// check 4 AO, whether analog output is changed
				for(i = 0;i < 4;i++)
				{
					if(output_raw[i + 4] != output_raw_back[i + 4])
					{				
						output_raw_back[i + 4] = output_raw[i + 4];
						AO_auto[i + 4] = conver_ADC(output_raw[i + 4]);
						flag_output_changed |= (0x01 << i);
						AO_auto_count[i] = 0;
						
					}

					if(AO_auto_count[i] > 20)
						// generate alarm
					{
						AO_auto_count[i] = 0;
						flag_output = 0;		
		// generate a alarm
						generate_common_alarm(ALARM_AO_FB);
					}
				}	
				
				
				if(temp1.word != relay_value.word)
				{
					relay_value.word = temp1.word;
					flag_DO_changed = 1;
					DO_change_count = 0;
#if ASIX_MINI
					push_cmd_to_picstack(SET_RELAY_LOW,relay_value.byte[1]); 
#endif	
				}
				
				if(flag_DO_changed) 
				{
					DO_change_count++;
					if(DO_change_count > 200)  // keep stable for 500ms second,send command to pic again
					{
						flag_DO_changed = 0;
						DO_change_count = 0;
					
#if ASIX_MINI
						push_cmd_to_picstack(SET_RELAY_LOW,relay_value.byte[1]); 
#endif	
					}
				}
				
				Calucation_PWM_IO(AnalogOutput_refresh_index);	
								
				if(AnalogOutput_refresh_index < 3)	  
					AnalogOutput_refresh_index++;
				else 	
					AnalogOutput_refresh_index = 0;	
				
			}
	#endif 
	#if ARM_MINI
			else if((Modbus.mini_type == MINI_NEW_TINY) || (Modbus.mini_type == MINI_TINY_ARM) )
			{
				
				for(i = 0;i < 8;i++)
				{	
					// check range of OUT7 & OUT8, they are abled to config to AO or DO
	//				if( ((i == 6) || (i == 7)) && (outputs[i].digital_analog == 1)) //  analog
	//				{
	//					temp1.word &= ~(BIT0 << i); 		
	//				}
	//				else
	//				{
//						if(outputs[i].switch_status == SW_AUTO)
//						{						
							if(outputs[i].digital_analog == 0)  // digital
							{
								if(output_raw[i] >= 512)						
										temp1.word |= (0x01 << i);
								else
									temp1.word &= ~(BIT0 << i);
							}	
							else  // analog  , PWM mode
							{
								if(pwm_count[i] < (U32_T)outputs[i].pwm_period * 20)
								{
									pwm_count[i]++;
								}
								else
									pwm_count[i] = 0;
								
								if(pwm_count[i] < (U32_T)outputs[i].pwm_period * 20 * output_raw[i] / 1023)
								{
									temp1.word |= (0x01 << i);
								}
								else 
								{
									temp1.word &= ~(BIT0 << i); 
								}
							}
//						}
//						else if (outputs[i].switch_status == SW_OFF )			
//							temp1.word &= ~(BIT0 << i); 		
//						else if (outputs[i].switch_status == SW_HAND )	
//							temp1.word |= (BIT0 << i);
					}
//			}		
			
//						// check 4 AO, whether analog output is changed
//			for(i = 0;i < 8;i++)
//			{
//				if(output_raw[i + 6] != output_raw_back[i + 6])
//				{				
//					output_raw_back[i + 6] = output_raw[i + 6];
//					AO_auto[i + 6] = conver_ADC(output_raw[i + 6]);
//					flag_output_changed |= (0x01 << i);
//					AO_auto_count[i] = 0;
//					
//				}

//				if(AO_auto_count[i] > 20)
//					// generate alarm
//				{
//					AO_auto_count[i] = 0;
//					flag_output = 0;					
//	// generate a alarm
//					generate_common_alarm(ALARM_AO_FB);
//				}
//			}	
				
				
				if(temp1.word != relay_value.word)
				{
					flag_DO_changed = 1;
					DO_change_count = 0;
					Test[10]++;
					relay_value.word = temp1.word;
					if(relay_value.byte[0] & 0x01) 					TB_REALY1 = 1;				else					TB_REALY1 = 0;
					if(relay_value.byte[0] & 0x02) 					TB_REALY2 = 1;				else					TB_REALY2 = 0;
					if(relay_value.byte[0] & 0x04) 					TB_REALY3 = 1;				else					TB_REALY3 = 0;
					if(relay_value.byte[0] & 0x08) 					TB_REALY4 = 1;				else					TB_REALY4 = 0;
					if(relay_value.byte[0] & 0x10) 					TB_REALY5 = 1;				else					TB_REALY5 = 0;
					if(relay_value.byte[0] & 0x20) 					TB_REALY6 = 1;				else					TB_REALY6 = 0;
					if(outputs[6].digital_analog == 0)
					{
						if(relay_value.byte[0] & 0x40) 					TB_REALY7 = 1;				else					TB_REALY7 = 0;
					}
					if(outputs[7].digital_analog == 0)
					{
						if(relay_value.byte[0] & 0x80) 					TB_REALY8 = 1;				else			 		TB_REALY8 = 0;
					}			
				}
				
				if(flag_DO_changed) 
				{
					DO_change_count++;
					if(DO_change_count > 200)  // keep stable for 500ms second,send command to pic again
					{Test[11]++;
						flag_DO_changed = 0;
						DO_change_count = 0;
					
						if(relay_value.byte[0] & 0x01) 					TB_REALY1 = 1;				else					TB_REALY1 = 0;
						if(relay_value.byte[0] & 0x02) 					TB_REALY2 = 1;				else					TB_REALY2 = 0;
						if(relay_value.byte[0] & 0x04) 					TB_REALY3 = 1;				else					TB_REALY3 = 0;
						if(relay_value.byte[0] & 0x08) 					TB_REALY4 = 1;				else					TB_REALY4 = 0;
						if(relay_value.byte[0] & 0x10) 					TB_REALY5 = 1;				else					TB_REALY5 = 0;
						if(relay_value.byte[0] & 0x20) 					TB_REALY6 = 1;				else					TB_REALY6 = 0;
						if(outputs[6].digital_analog == 0) {if(relay_value.byte[0] & 0x40) 					TB_REALY7 = 1;				else					TB_REALY7 = 0;}
						if(outputs[7].digital_analog == 0) {if(relay_value.byte[0] & 0x80) 			  	TB_REALY8 = 1;				else					TB_REALY8 = 0;	}
					}
				}
				
				Calucation_PWM_IO(AnalogOutput_refresh_index);	
								
				if(AnalogOutput_refresh_index < 6)	  
					AnalogOutput_refresh_index++;
				else 	
					AnalogOutput_refresh_index = 0;	
				
			}	
			else if(Modbus.mini_type == MINI_TINY_11I)
			{// 5AO6DO				
				for(i = 0;i < 6;i++)
				{		
					if(outputs[i].digital_analog == 0)  // digital
					{
						if(output_raw[i] >= 512)						
								temp1.word |= (0x01 << i);
						else
							temp1.word &= ~(BIT0 << i);
					}	
					else  // analog  , PWM mode
					{
						if(pwm_count[i] < (U32_T)outputs[i].pwm_period * 20)
						{
							pwm_count[i]++;
						}
						else
							pwm_count[i] = 0;
						
						if(pwm_count[i] < (U32_T)outputs[i].pwm_period * 20 * output_raw[i] / 1023)
						{
							temp1.word |= (0x01 << i);
						}
						else 
						{
							temp1.word &= ~(BIT0 << i); 
						}
					}
				}
			
				
				if(temp1.word != relay_value.word)
				{
					flag_DO_changed = 1;
					DO_change_count = 0;
					
					relay_value.word = temp1.word;
					if(relay_value.byte[0] & 0x01) 					TB_REALY1 = 1;				else					TB_REALY1 = 0;
					if(relay_value.byte[0] & 0x02) 					TB_REALY2 = 1;				else					TB_REALY2 = 0;
					if(relay_value.byte[0] & 0x04) 					TB_REALY3 = 1;				else					TB_REALY3 = 0;
					if(relay_value.byte[0] & 0x08) 					TB_REALY4 = 1;				else					TB_REALY4 = 0;
					if(relay_value.byte[0] & 0x10) 					TB_REALY8 = 1;				else					TB_REALY8 = 0;
					if(relay_value.byte[0] & 0x20) 					TB_REALY6 = 1;				else					TB_REALY6 = 0;
						
				}
				
				if(flag_DO_changed) 
				{
					DO_change_count++;
					if(DO_change_count > 200)  // keep stable for 500ms second,send command to pic again
					{
						flag_DO_changed = 0;
						DO_change_count = 0;
					
						if(relay_value.byte[0] & 0x01) 					TB_REALY1 = 1;				else					TB_REALY1 = 0;
						if(relay_value.byte[0] & 0x02) 					TB_REALY2 = 1;				else					TB_REALY2 = 0;
						if(relay_value.byte[0] & 0x04) 					TB_REALY3 = 1;				else					TB_REALY3 = 0;
						if(relay_value.byte[0] & 0x08) 					TB_REALY4 = 1;				else					TB_REALY4 = 0;
						if(relay_value.byte[0] & 0x10) 					TB_REALY8 = 1;				else					TB_REALY8 = 0;
						if(relay_value.byte[0] & 0x20) 					TB_REALY6 = 1;				else					TB_REALY6 = 0;						
					}
				}
				
				Calucation_PWM_IO(AnalogOutput_refresh_index);	
								
				if(AnalogOutput_refresh_index < 5)	  
					AnalogOutput_refresh_index++;
				else 	
					AnalogOutput_refresh_index = 0;	
				
			}	
	#endif
			
	}
#endif
		
#if ARM_CM5		
				for(i = 0;i < 10;i++)
				{						
					if(outputs[i].digital_analog == 0)  // digital
					{
						if(output_raw[i] >= 512)						
								temp1.word |= (0x01 << i);
						else
							temp1.word &= ~(BIT0 << i);
					}	
					else  // analog  , PWM mode
					{
						if(pwm_count[i] < (U32_T)outputs[i].pwm_period * 20)
						{
							pwm_count[i]++;
						}
						else
							pwm_count[i] = 0;
						
						if(pwm_count[i] < (U32_T)outputs[i].pwm_period * 20 * output_raw[i] / 1023)
						{
							temp1.word |= (0x01 << i);
						}
						else 
						{
							temp1.word &= ~(BIT0 << i); 
						}
					}
				}

				
				if(temp1.word != relay_value.word)
				{
					flag_DO_changed = 1;
					DO_change_count = 0;
					
					relay_value.word = temp1.word;
					if(relay_value.byte[0] & 0x01) 					CM5_REALY1 = 1;				else					CM5_REALY1 = 0;
					if(relay_value.byte[0] & 0x02) 					CM5_REALY2 = 1;				else					CM5_REALY2 = 0;
					if(relay_value.byte[0] & 0x04) 					CM5_REALY3 = 1;				else					CM5_REALY3 = 0;
					if(relay_value.byte[0] & 0x08) 					CM5_REALY4 = 1;				else					CM5_REALY4 = 0;
					if(relay_value.byte[0] & 0x10) 					CM5_REALY5 = 1;				else					CM5_REALY5 = 0;
					if(relay_value.byte[0] & 0x20) 					CM5_REALY6 = 1;				else					CM5_REALY6 = 0;
					if(relay_value.byte[0] & 0x40) 					CM5_REALY7 = 1;				else					CM5_REALY7 = 0;
					if(relay_value.byte[0] & 0x80) 					CM5_REALY8 = 1;				else			 		CM5_REALY8 = 0;
					
					if(relay_value.byte[1] & 0x01) 					CM5_REALY9 = 1;				else					CM5_REALY9 = 0;
					if(relay_value.byte[1] & 0x02) 					CM5_REALY10 = 1;			else			 		CM5_REALY10 = 0;
				}			
			
				if(flag_DO_changed) 
				{
					DO_change_count++;
					if(DO_change_count > 200)  // keep stable for 500ms second,send command to pic again
					{
						flag_DO_changed = 0;
						DO_change_count = 0;
					
						if(relay_value.byte[0] & 0x01) 					CM5_REALY1 = 1;				else					CM5_REALY1 = 0;
						if(relay_value.byte[0] & 0x02) 					CM5_REALY2 = 1;				else					CM5_REALY2 = 0;
						if(relay_value.byte[0] & 0x04) 					CM5_REALY3 = 1;				else					CM5_REALY3 = 0;
						if(relay_value.byte[0] & 0x08) 					CM5_REALY4 = 1;				else					CM5_REALY4 = 0;
						if(relay_value.byte[0] & 0x10) 					CM5_REALY5 = 1;				else					CM5_REALY5 = 0;
						if(relay_value.byte[0] & 0x20) 					CM5_REALY6 = 1;				else					CM5_REALY6 = 0;
						if(relay_value.byte[0] & 0x40) 					CM5_REALY7 = 1;				else					CM5_REALY7 = 0;
						if(relay_value.byte[0] & 0x80) 					CM5_REALY8 = 1;				else			 		CM5_REALY8 = 0;
						
						if(relay_value.byte[1] & 0x01) 					CM5_REALY9 = 1;				else					CM5_REALY9 = 0;
						if(relay_value.byte[1] & 0x02) 					CM5_REALY10 = 1;			else			 		CM5_REALY10 = 0;	
					}
				}
		
#endif
	}

} 

#endif


