#include "main.h"

#if ARM_TSTAT_WIFI
#include "controls.h"
#include "constcode.h"
#include "sht3x.h"
// only for tstat-wifi
/*
-> output
-> input 

*/
#define LIGHT_COE    63
#define LIGHT_R1     1

// for MSV
uint8 FAN_MODE[8][9];
U8_T far flag_count_in[HI_COMMON_CHANNEL];
void signal_dealwith(u8 i);

/*
从tstat10_rev4开始硬件引脚变动 */
#define PT1K_SET		PAout(5) 
#define SEL1_IN   	PAout(6)	
#define SEL2_IN   	PAout(7)	

#define PT1K_SET_NEW		PGout(13) 
#define SEL1_IN_NEW   	PGout(14)	
#define SEL2_IN_NEW   	PGout(15)


#define SEL3_IN   PCout(4)

#define RANGE_SET0			PCout(3)
#define RANGE_SET1			PCout(2)

// Tstat10P 新增HSP count和 两路 AO
uint8 Check_sensor_exist(uint8 type);
 /*  PE11 - PG14
 PE11------------>	HSP_INPUT1
 PE12------------>	HSP_INPUT2
 PE13------------>	HSP_INPUT3
 PE14------------>	HSP_INPUT4
*/
#define READ_PULSE1 GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_11)
#define READ_PULSE2 GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_12)
#define READ_PULSE3 GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_13)
#define READ_PULSE4 GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_14)

#define INPUT1 PEin(11)
#define INPUT2 PEin(12)
#define INPUT3 PEin(13)
#define INPUT4 PEin(14)

#define FILTER 100

#define RISE 0
#define FALL 1

U8_T high_spd_flag[HI_COMMON_CHANNEL];
void pulse_set(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// PC6 7 8 9
	
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource14);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource11);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource12);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource13);
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line14 | EXTI_Line11 | EXTI_Line12 | EXTI_Line13; 
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//		if(rise_or_fall == RISE)
//			EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
//		else
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;		
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);	
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	high_spd_counter_tempbuf[COMMON_CHANNEL] = 0;
	high_spd_counter_tempbuf[COMMON_CHANNEL + 1] = 0;
	high_spd_counter_tempbuf[COMMON_CHANNEL + 2] = 0;
	high_spd_counter_tempbuf[COMMON_CHANNEL + 3] = 0;
}



void EXTI15_10_IRQHandler(void)
{

	if(EXTI->PR & (1 << 11))	
	{    
		EXTI_ClearITPendingBit(GPIO_Pin_11);
		//EXTI->PR  = (1 << 6);
		if((inputs[COMMON_CHANNEL].range == HI_spd_count) || (inputs[COMMON_CHANNEL].range == N0_2_32counts)
			|| (inputs[COMMON_CHANNEL].range == RPM))
		{
			high_spd_counter_tempbuf[COMMON_CHANNEL]++;
		}
#if 0		
		else
		{	
		// PWM		
		flag_pulse[0] = 0 ;
		if(INPUT1) // 上升沿
		{
			delay_us(DELAY_CHECK_PO);
			if(INPUT1)
				flag_pulse[0] =  PLUG_OUT;// 上升沿;
		}
		else  // 下降沿
		{Test[39]++;
			delay_us(DELAY_CHECK_PI);
			if(INPUT1 == 0)
				flag_pulse[0] = PLUG_IN;
		}		
		signal_dealwith(0);
	}
#endif
	}
	
	if(EXTI->PR & (1 << 12))	
	{      
		//EXTI->PR  = (1 << 7);	
		EXTI_ClearITPendingBit(GPIO_Pin_12);
		if((inputs[COMMON_CHANNEL + 1].range == HI_spd_count) || (inputs[COMMON_CHANNEL + 1].range == N0_2_32counts)
			|| (inputs[COMMON_CHANNEL + 1].range == RPM))		
		{
			high_spd_counter_tempbuf[COMMON_CHANNEL + 1]++;
		}
#if 0		
		else
		{			
				// PWM		
		flag_pulse[1] = 0 ;
		if(INPUT2) // 上升沿
		{
			delay_us(DELAY_CHECK_PO);
			if(INPUT2)
				flag_pulse[1] =  PLUG_OUT;// 上升沿;
		}
		else  // 下降沿
		{
			delay_us(DELAY_CHECK_PI);
			if(INPUT2 == 0)
				flag_pulse[1] = PLUG_IN;
		}		
		signal_dealwith(1);
	}
#endif
	}
	
	if(EXTI->PR & (1 << 13))	
	{      
		//EXTI->PR  = (1 << 8);	
		EXTI_ClearITPendingBit(GPIO_Pin_13);
		if((inputs[COMMON_CHANNEL + 2].range == HI_spd_count) || (inputs[COMMON_CHANNEL + 2].range == N0_2_32counts)
			|| (inputs[COMMON_CHANNEL + 2].range == RPM))		{
			high_spd_counter_tempbuf[COMMON_CHANNEL + 2]++;
		}
#if 0		
		else
		{				// PWM		
			flag_pulse[2] = 0 ;
			if(INPUT3) // 上升沿
			{
				delay_us(DELAY_CHECK_PO);
				if(INPUT3)
					flag_pulse[2] =  PLUG_OUT;// 上升沿;
			}
			else  // 下降沿
			{
				delay_us(DELAY_CHECK_PI);
				if(INPUT3 == 0)
					flag_pulse[2] = PLUG_IN;
			}		
			signal_dealwith(2);
		}
#endif
	}
	
	if(EXTI->PR & (1 << 14))	
	{      
		//EXTI->PR  = (1 << 9);	
		EXTI_ClearITPendingBit(GPIO_Pin_14);
		if((inputs[COMMON_CHANNEL + 3].range == HI_spd_count) || (inputs[COMMON_CHANNEL + 3].range == N0_2_32counts)
			|| (inputs[COMMON_CHANNEL + 3].range == RPM))
		{
			high_spd_counter_tempbuf[COMMON_CHANNEL + 3]++;
		}
#if 0		
		else
		{				// PWM		
			flag_pulse[3] = 0 ;
			if(INPUT4) // 上升沿
			{
				delay_us(DELAY_CHECK_PO);
				if(INPUT4)
					flag_pulse[3] =  PLUG_OUT;// 上升沿;
			}
			else  // 下降沿
			{
				delay_us(DELAY_CHECK_PI);
				if(INPUT4 == 0)
					flag_pulse[3] = PLUG_IN;
			}		
			signal_dealwith(2);
		}
#endif
	}	
}

 
u8_t flag_ready_to_scan = 0;
U8_T table_pwm[8];
U8_T slop[10];

u8_t hum_sensor_type;
float tem_org = 0;
float hum_org = 0;

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

void range_set_func(u8 range)
{
	if(range == INPUT_PT1K)
	{
		if(Modbus.hardRev < 4)			PT1K_SET = 0;		
		else		PT1K_SET_NEW = 0;	
	}
	else if(range == INPUT_V0_5)
	//if(inputs[channel].range == V0_5 || inputs[channel].range == P0_100_0_5V)
	{
		if(Modbus.hardRev < 4)
		{
			PT1K_SET = 1;
		}
		else
		{
			PT1K_SET_NEW = 1;
		}		
		RANGE_SET0 = 1 ;
		RANGE_SET1 = 0 ;
	}
	else if(range == INPUT_0_10V)
	//else if (inputs[channel].range == V0_10_IN)
	{
		if(Modbus.hardRev < 4)
		{
			PT1K_SET = 1;
		}
		else
		{
			PT1K_SET_NEW = 1;
		}		
		RANGE_SET0 = 0 ;
		RANGE_SET1 = 1 ;
	}
	else if(range == INPUT_I0_20ma)
	//else if (inputs[channel].range == I0_20ma)
	{
		if(Modbus.hardRev < 4)
		{
			PT1K_SET = 1;
		}
		else
		{
			PT1K_SET_NEW = 1;
		}
		RANGE_SET0 = 0 ;
		RANGE_SET1 = 0 ;
	}
	else
	{
		if(Modbus.hardRev < 4)
		{
			PT1K_SET = 1;		
		}
		else
		{
			PT1K_SET_NEW = 1;
		}
		RANGE_SET0 = 1 ;
		RANGE_SET1 = 1 ;
	} 

}

uint8 PirSensorZero;
uint8 Pir_Sensetivity;
uint16 override_timer_time;
uint8 override_timer;

void initial_tstat10_range(void)
{
	char i;
	// PIR SENSOR   OCC
	if(Check_sensor_exist(E_FLAG_OCC))
	{
		PirSensorZero = 160;
		Pir_Sensetivity = 50;
		override_timer = 5;

		if(Modbus.mini_type == MINI_T10P)
		{
			inputs[HI_COMMON_CHANNEL + 3].range = UNOCCUPIED_OCCUPIED;	
			inputs[HI_COMMON_CHANNEL + 3].digital_analog = 0;
			inputs[HI_COMMON_CHANNEL + 3].control = 0;
		}
		else
		{
			inputs[COMMON_CHANNEL + 3].range = UNOCCUPIED_OCCUPIED;	
			inputs[COMMON_CHANNEL + 3].digital_analog = 0;
			inputs[COMMON_CHANNEL + 3].control = 0;
		}
	}
	else
	{
		if(Modbus.mini_type == MINI_T10P)
		{
			inputs[HI_COMMON_CHANNEL + 3].range = 0;	
			inputs[HI_COMMON_CHANNEL + 3].value = 0;	
		}
		else
		{
			inputs[COMMON_CHANNEL + 3].range = 0;	
			inputs[COMMON_CHANNEL + 3].value = 0;	
		}		
	}
		
// humidity & temperature	
	if(Check_sensor_exist(E_FLAG_HUM))
	{
		if(Modbus.mini_type == MINI_T10P)
		{
			inputs[HI_COMMON_CHANNEL + 2].range = 27;  //humidity RANGE	
			inputs[HI_COMMON_CHANNEL + 2].digital_analog = 1;
			inputs[HI_COMMON_CHANNEL + 2].value = 0;
		}
		else
		{
			inputs[COMMON_CHANNEL + 2].range = 27;   //humidity RANGE		
			inputs[COMMON_CHANNEL + 2].digital_analog = 1;
			inputs[COMMON_CHANNEL + 2].value = 0;
		}
	}
	else
	{
		if(Modbus.mini_type == MINI_T10P)
		{
			inputs[HI_COMMON_CHANNEL + 2].range = 0;	
			inputs[HI_COMMON_CHANNEL + 2].value = 0;	
		}
		else
		{
			inputs[COMMON_CHANNEL + 2].range = 0;	
			inputs[COMMON_CHANNEL + 2].value = 0;	
		}		
	}
		

	if(Modbus.mini_type == MINI_T10P)
	{
		if(inputs[HI_COMMON_CHANNEL].range == 0)
		{
			inputs[HI_COMMON_CHANNEL].range = R10K_40_120DegC;	
			inputs[HI_COMMON_CHANNEL].digital_analog = 1;
		}
	}
	else
	{
		if(inputs[COMMON_CHANNEL].range == 0)
		{
			inputs[COMMON_CHANNEL].range = R10K_40_120DegC;	
			inputs[COMMON_CHANNEL].digital_analog = 1;
		}
	}
//	inputs[HI_COMMON_CHANNEL].value = 0;
//	co2
	if(Check_sensor_exist(E_FLAG_CO2))
	{
		if(Modbus.mini_type == MINI_T10P)
		{
			if(inputs[HI_COMMON_CHANNEL + 4].range == 0)
			{
				inputs[HI_COMMON_CHANNEL + 4].range = 28;  //????????????	
				inputs[HI_COMMON_CHANNEL + 4].digital_analog = 1;
				inputs[HI_COMMON_CHANNEL + 4].value = 0;
			}
		}
		else
		{
			if(inputs[COMMON_CHANNEL + 4].range == 0)
			{
				inputs[COMMON_CHANNEL + 4].range = 28;  //????????????	
				inputs[COMMON_CHANNEL + 4].digital_analog = 1;
				inputs[COMMON_CHANNEL + 4].value = 0;
			}
		}
	}
	else
	{
		if(Modbus.mini_type == MINI_T10P)
		{
			inputs[HI_COMMON_CHANNEL + 4].range = 0;	
			inputs[HI_COMMON_CHANNEL + 4].value = 0;	
		}
		else
		{
			inputs[COMMON_CHANNEL + 4].range = 0;	
			inputs[COMMON_CHANNEL + 4].value = 0;	
		}
	}
	
	// spd counter
	if(Modbus.mini_type == MINI_T10P)
	{
		for(i = 0;i < 4;i++)
		{
			if(inputs[COMMON_CHANNEL + i].range == 0)
			{
				inputs[COMMON_CHANNEL + i].range = HI_spd_count;	
				inputs[COMMON_CHANNEL + i].digital_analog = 1;
			}
		}
	}

	if(Check_sensor_exist(E_FLAG_TVOC))
	{
		if(Modbus.mini_type == MINI_T10P)
		{
			inputs[HI_COMMON_CHANNEL + 1].range = PPB;  // tbd: add PPB
			inputs[HI_COMMON_CHANNEL + 1].digital_analog = 1;
			inputs[HI_COMMON_CHANNEL + 1].value = 0;
		}
		else
		{
			inputs[COMMON_CHANNEL + 1].range = PPB;  //  tbd: add PPB	
			inputs[COMMON_CHANNEL + 1].digital_analog = 1;
			inputs[COMMON_CHANNEL + 1].value = 0;
		}
	}
	else
	{
		if(Modbus.mini_type == MINI_T10P)
		{
			inputs[HI_COMMON_CHANNEL + 1].range = 0;	
			inputs[HI_COMMON_CHANNEL + 1].value = 0;	
		}
		else
		{
			inputs[COMMON_CHANNEL + 1].range = 0;	
			inputs[COMMON_CHANNEL + 1].value = 0;	
		}
	}
	
	
	if(Check_sensor_exist(E_FLAG_LIGHT))
	{
		if(Modbus.mini_type == MINI_T10P)
		{
			inputs[HI_COMMON_CHANNEL + 5].range = LUX;  
			inputs[HI_COMMON_CHANNEL + 5].digital_analog = 1;
			inputs[HI_COMMON_CHANNEL + 5].value = 0;
		}
		else
		{
			inputs[COMMON_CHANNEL + 5].range = LUX;  	
			inputs[COMMON_CHANNEL + 5].digital_analog = 1;
			inputs[COMMON_CHANNEL + 5].value = 0;
		}
	}
	else
	{
		if(Modbus.mini_type == MINI_T10P)
		{
			inputs[HI_COMMON_CHANNEL + 5].range = 0;	
			inputs[HI_COMMON_CHANNEL + 5].value = 0;	
		}
		else
		{
			inputs[COMMON_CHANNEL + 5].range = 0;	
			inputs[COMMON_CHANNEL + 5].value = 0;	
		}
	}
	
	
	if(Check_sensor_exist(E_FLAG_VOICE))
	{
		if(Modbus.mini_type == MINI_T10P)
		{
			inputs[HI_COMMON_CHANNEL + 6].range = DB;  // DB
			inputs[HI_COMMON_CHANNEL + 6].digital_analog = 1;
			inputs[HI_COMMON_CHANNEL + 6].value = 0;
		}
		else
		{
			inputs[COMMON_CHANNEL + 6].range = DB;  //  DB
			inputs[COMMON_CHANNEL + 6].digital_analog = 1;
			inputs[COMMON_CHANNEL + 6].value = 0;
		}
	}
	else
	{
		if(Modbus.mini_type == MINI_T10P)
		{
			inputs[HI_COMMON_CHANNEL + 6].range = 0;	
			inputs[HI_COMMON_CHANNEL + 6].value = 0;	
		}
		else
		{
			inputs[COMMON_CHANNEL + 6].range = 0;	
			inputs[COMMON_CHANNEL + 6].value = 0;	
		}
	}
}
// PC1 - ADC123_IN11
// PC5 - internal temperature sensor  ADC12_IN15
// PB0 - ADC12_IN8
// PB1 - ADC12_IN9
// PC0 - ADC123_IN10
void inputs_adc_init(void)
{
	ADC_InitTypeDef ADC_InitStructure;
  char i;
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);		
  /* ADC1 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = 5;
  ADC_Init(ADC1, &ADC_InitStructure);	
		
 	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 1, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 2, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 3, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 4, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 5, ADC_SampleTime_55Cycles5);

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
	
	initial_tstat10_range();
}


void Input_IO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC |RCC_APB2Periph_GPIOF, ENABLE);
	
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_5; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4; 	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);


	if(Modbus.hardRev < 4)
	{
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7; 	
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		GPIO_ResetBits(GPIOA, GPIO_Pin_6 | GPIO_Pin_7);	
		GPIO_SetBits(GPIOA, GPIO_Pin_5);	
	}
	else
	{
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15; 	
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOG, &GPIO_InitStructure);
		GPIO_ResetBits(GPIOG, GPIO_Pin_13 | GPIO_Pin_14);	
		GPIO_SetBits(GPIOG, GPIO_Pin_15);
	}
	
	
	// light sensor & occupied sensor 
	// PB0 & PB1
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
//	
//	// PG 6 7 8 9
//	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE);
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14; 
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOE, &GPIO_InitStructure);
//			
//	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource11);
//	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource12);
//	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource13);
//	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource14);
//	GPIO_EXTILineConfig(GPIO_PortSourceGPIOG,GPIO_PinSource11 | GPIO_PinSource12 | GPIO_PinSource13 | GPIO_PinSource14); 
//	EXTI_InitStructure.EXTI_Line  = EXTI_Line11 | EXTI_Line12 | EXTI_Line13 | EXTI_Line14; 
//	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
////		if(rise_or_fall == RISE)
////			EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
////		else
//	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;		
//	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//	EXTI_Init(&EXTI_InitStructure);	
//	
//	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);
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

// 10ms 一次
// 载波的高低电压0.6~0.8
#define MIC_CARRIER_HI 	1090
#define MIC_CARRIER_LO	820
uint16 voice_table[10][2] = 
{ {60,9},{63,13},{67,36},{70,55},{72,95},
	{74,170},{76,220},{78,270},{80,340},{83,500}
};

u8 check_voice_table(uint16 adc)
{
	char i;
	if(adc < voice_table[0][1]) 
		return 50;
	
	if(adc > voice_table[9][1]) 
		return 90;
	
	for(i = 0;i < 9;i++)
	{		
		if((adc >= voice_table[i][1]) && (adc < voice_table[i + 1][1]))
		{
			return voice_table[i][0] + 
				(voice_table[i + 1][0] - voice_table[i][0]) * (adc - voice_table[i][1]) / (voice_table[i + 1][1] - voice_table[i][1]);
		}
	}
	return 0;
}

// 每秒钟执行一次
void check_override_timer_1s(void)
{
	if(Check_sensor_exist(E_FLAG_OCC))
	{
		if(Modbus.mini_type == MINI_T10P)
		{
				if(inputs[HI_COMMON_CHANNEL + 3].control == 1)  // if current status is occ
				{
				// check whether 
					if(override_timer_time > 0)
					{
						override_timer_time --;	
						if(override_timer_time == 0)//if override timer reduce to 0, go to off mode
						{
							inputs[HI_COMMON_CHANNEL + 3].control = 0;
						}
					}
				}	
		}
		else
		{
			if(inputs[COMMON_CHANNEL + 3].control == 1)  // if current status is occ
			{
			// check whether 
				if(override_timer_time > 0)
				{
					override_timer_time --;	
					if(override_timer_time == 0)//if override timer reduce to 0, go to off mode
					{
						inputs[COMMON_CHANNEL + 3].control = 0;
					}
				}
			}	
		}
	}
}


u16 voltage_occ;
void inpust_scan(void)
{
	static u8 channel_count = 0;
	u8 i;
	float light_temp;
	u32 temp1,temp2,temp3,temp4,temp5;
	u32 mic_sum;
	u8 count;
//	static u8 count_no_mic = 0;
	mic_sum = 0;
	
	temp1 = 0;  // AI0-AI7
	temp2 = 0; // temperature
	temp3 = 0;  // light
	temp4 = 0;  // occ
	temp5 = 0; // mic
	count = 0;
	
	for(i = 0;i < 100;i++)
	{
		temp1 += ADC_getChannal(ADC1,ADC_Channel_11,1);
		temp2 += ADC_getChannal(ADC1,ADC_Channel_15,1);
		temp3 += ADC_getChannal(ADC1,ADC_Channel_8,1);
		temp4 += ADC_getChannal(ADC1,ADC_Channel_9,1);
		
		
		temp5 = ADC_getChannal(ADC1,ADC_Channel_10,1);
		if(temp5 > MIC_CARRIER_HI)
		{
			count++;
			temp5 = temp5 - MIC_CARRIER_HI;
		}
		else if(temp5 < MIC_CARRIER_LO)
		{
			count++;
			temp5 = MIC_CARRIER_LO - temp5;
		}
		else
		{
			temp5 = 0;
		}
		
		mic_sum += temp5;// * temp5;
		delay_ms(1);
	}
	
	temp1 = Filter(channel_count,temp1 / 400);
	if(Modbus.mini_type == MINI_T10P)
	{
		temp2 = Filter(HI_COMMON_CHANNEL,temp2 / 400);
		temp3 = Filter(HI_COMMON_CHANNEL + 5,temp3 / 100);
		temp4 = Filter(HI_COMMON_CHANNEL + 3,temp4 / 100);  // occ
	}
	else
	{
		temp2 = Filter(COMMON_CHANNEL,temp2 / 400);
		temp3 = Filter(COMMON_CHANNEL + 5,temp3 / 100);
		temp4 = Filter(COMMON_CHANNEL + 3,temp4 / 100);
	}
	
	if(Check_sensor_exist(E_FLAG_VOICE))
	{
		if(count >= 0)
		{
			temp5 = check_voice_table(mic_sum /count);
			if(Modbus.mini_type == MINI_T10P)
				inputs[HI_COMMON_CHANNEL + 6].value = temp5 * 1000;
			else
				inputs[COMMON_CHANNEL + 6].value = temp5 * 1000;
		}
		else
		{
			if(Modbus.mini_type == MINI_T10P)
				inputs[HI_COMMON_CHANNEL + 6].value = 40000;
			else
				inputs[COMMON_CHANNEL + 6].value = 40000;
		}
	}
	else
	{
		if(Modbus.mini_type == MINI_T10P)
			inputs[HI_COMMON_CHANNEL + 6].value = 0;
		else
			inputs[COMMON_CHANNEL + 6].value = 0;
	}
	
	input_raw[channel_count] = temp1;	
	
	if(Modbus.mini_type == MINI_T10P)
		input_raw[HI_COMMON_CHANNEL] = temp2;		// temperatue
	else
		input_raw[COMMON_CHANNEL] = temp2;		// temperatue
	
	channel_count++;
	channel_count %= 8;
	if(Check_sensor_exist(E_FLAG_LIGHT))
	{
		if(Modbus.mini_type == MINI_T10P)
		{
			temp3 = temp3 / 10;
			temp3 = 3000 * temp3 / 4095;
			light_temp = temp3 * 100 / (LIGHT_COE * LIGHT_R1);
			inputs[HI_COMMON_CHANNEL + 5].value = light_temp * 1000;				
		}
		else
		{
			light_temp = 3000 * temp3 / 4095;
			inputs[COMMON_CHANNEL + 5].value = light_temp * 100 * 1000/(LIGHT_COE * LIGHT_R1);
		}
	}	
	
	 // detect occ senseor
	if(Check_sensor_exist(E_FLAG_OCC))
	{
		voltage_occ = temp4 * 3000 / 40 / 1023;
		if(Modbus.mini_type == MINI_T10P)
		{
			if(inputs[HI_COMMON_CHANNEL + 3].digital_analog == 0)
			{
				if(abs(temp4 * 3000 / 40 / 1023 - PirSensorZero) > Pir_Sensetivity) //occupied
				{
					inputs[HI_COMMON_CHANNEL + 3].control = 1;  // OCC
					override_timer_time = (uint32)60 * override_timer;
				}				
			}
			else
			{
				inputs[HI_COMMON_CHANNEL + 3].value = temp4 * 3000 / 40 / 1023;
			}
		}
		else
		{
			if(inputs[COMMON_CHANNEL + 3].digital_analog == 0)
			{	
				if(abs(temp4 * 3000 / 40 / 1023 - PirSensorZero) > Pir_Sensetivity) //occupied
				{
					inputs[COMMON_CHANNEL + 3].control = 1;  // OCC
					override_timer_time = (uint32)60 * override_timer;
				}					
			}
			else
			{
				inputs[COMMON_CHANNEL + 3].value = temp4 * 3000 / 40 / 1023;
			}
		}
	}
	
	switch(channel_count)
	{
		case 0: 
			SEL3_IN = 0;
			if(Modbus.hardRev < 4)
			{
				SEL1_IN = 0;	SEL2_IN = 0;	
			}
			else
			{
				SEL1_IN_NEW = 0;	SEL2_IN_NEW = 0;
			}
			break;
		case 1: 
			SEL3_IN = 0;
			if(Modbus.hardRev < 4)
			{
				SEL1_IN = 1;	SEL2_IN = 0;
			}
			else
			{
				SEL1_IN_NEW = 1;	SEL2_IN_NEW = 0;
			}
			break;
		case 2: 
			SEL3_IN = 0;
			if(Modbus.hardRev < 4)
			{
				SEL1_IN = 0;	SEL2_IN = 1;
			}
			else
			{
				SEL1_IN_NEW = 0;	SEL2_IN_NEW = 1;
			}
			break;
		case 3: 
			SEL3_IN = 0;
			if(Modbus.hardRev < 4)
			{
				SEL1_IN = 1;	SEL2_IN = 1;
			}
			else
			{
				SEL1_IN_NEW = 1;	SEL2_IN_NEW = 1;
			}
			break;		
		case 4: 
			SEL3_IN = 1;
			if(Modbus.hardRev < 4)
			{
				SEL1_IN = 0;	SEL2_IN = 0;
			}
			else
			{
				SEL1_IN_NEW = 0;	SEL2_IN_NEW = 0;
			}
			break;	
		case 5: 
			SEL3_IN = 1;
			if(Modbus.hardRev < 4)
			{
				SEL1_IN = 1;	SEL2_IN = 0;
			}
			else
			{
				SEL1_IN_NEW = 1;	SEL2_IN_NEW = 0;
			}
			break;		
		case 6: 
			SEL3_IN = 1;
			if(Modbus.hardRev < 4)
			{
				SEL1_IN = 0;	SEL2_IN = 1;
			}
			else
			{
				SEL1_IN_NEW = 0;	SEL2_IN_NEW = 1;
			}
			break;		
		case 7: 
			SEL3_IN = 1;
			if(Modbus.hardRev < 4)
			{
				SEL1_IN = 1;	SEL2_IN = 1;
			}
			else
			{
				SEL1_IN_NEW = 1;	SEL2_IN_NEW = 1;
			}
			break;		
		default:
			break;	
	}	
	

	range_set_func(input_type[channel_count]);

}

#define TSTAT_REALY1	PEout(0)
#define TSTAT_REALY2	PEout(1)
#define TSTAT_REALY3	PBout(4)
#define TSTAT_REALY4	PBout(5)
#define TSTAT_REALY5	PEout(6)

void Output_IO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;	
	
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOE, ENABLE);
	
//REALY 1 2 5		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_6;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	GPIO_ResetBits(GPIOE, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_6);
//REALY 3 4		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_ResetBits(GPIOB, GPIO_Pin_4 | GPIO_Pin_5);

	
	// AO1 AO2 AO3 AO4
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM4, ENABLE);

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM2, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_8 | GPIO_Pin_9;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	TIM_TimeBaseStructure.TIM_Period = 1000;
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
	
	TIM_OC3Init(TIM2, &TIM_OCInitStructure);  // AO1 PB10 	TIM2_CH3
	TIM_OC4Init(TIM2, &TIM_OCInitStructure);  // AO2 PB11 	TIM2_CH4
	TIM_OC3Init(TIM4, &TIM_OCInitStructure);  // AO3 PB8 	TIM4_CH3
	TIM_OC4Init(TIM4, &TIM_OCInitStructure);  // AO4 PB9 	TIM4_CH4
//	
	TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);
	TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);
	TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);
	TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);
	
	TIM_Cmd(TIM2, ENABLE);
	TIM_Cmd(TIM4, ENABLE);
}

extern U16_T test_adc;
extern U8_T test_adc_flag;
void Calucation_PWM_IO(U8_T refresh_index)
{
	U32_T duty_Cycle1;//,duty_Cycle2;
	U32_T temp1;//,temp2;
//	U8_T loop;
	U32_T far adc1;
//	U32_T far adc2; /* adc1 is for the first 4051, adc2 is for the second 4051 */

	
	if(outputs[refresh_index + 5].digital_analog == 0) // digital
	{
		if(output_raw[refresh_index + 5] >= 512)
			adc1 = Modbus.start_adc[10];//1000;
		else
			adc1 = 0;
	}
	else // analog
	{
		adc1 = output_raw[refresh_index + 5];
		
//		if(flag_output == ADJUST_AUTO)
//		{
//			adc1 = Auto_Calibration_AO(refresh_index,AO_auto[refresh_index + 6]);
//			AO_auto[refresh_index + 6] = adc1;
//		}
//		else
			adc1 = conver_ADC(adc1);
	}
		
// if output is used for a digtal output, do not use feedback to adjust


		if(refresh_index == 0) 
		{
			if(test_adc_flag == 1)	adc1 = test_adc;
			test_adc = adc1;
		}
		
		if(outputs[refresh_index + 5].range == 0)		
			adc1 = 0;

		duty_Cycle1 = adc1;

		if(refresh_index == 0)			
			TIM_SetCompare3(TIM2, duty_Cycle1);			
		else if(refresh_index == 1)
			TIM_SetCompare4(TIM2, duty_Cycle1);
		else if(refresh_index == 2)		
			TIM_SetCompare3(TIM4, duty_Cycle1);			
		else if(refresh_index == 3)
			TIM_SetCompare4(TIM4, duty_Cycle1);

}

extern U8_T flag_read_switch;
void Check_Pulse_Counter(void);
void Update_Led(void);
UN_RELAY relay_value;
void refresh_Input_Task(void) reentrant;
void refresh_Output_Task(void) reentrant;
xTaskHandle xdata xHandler_Input;
xTaskHandle xdata xHandler_Output;
void vStartOutputTasks( unsigned char uxPriority)
{
	Output_IO_Init();
	sTaskCreate( refresh_Input_Task, "InputTask1", 200, NULL, uxPriority, &xHandler_Input );
	sTaskCreate( refresh_Output_Task, "OutputTask1", 200, NULL, uxPriority, &xHandler_Output );
	relay_value.word = 0;
}

void Hum_Initial(void)
{
	SHT3X_Init(0x45); 
	
}

void Read_Humidity(void)
{
	SHT3X_GetTempAndHumi(&tem_org, &hum_org, REPEATAB_HIGH, MODE_POLLING, 50);
	// if no internal temperature sensor, use temperature of humidity
	if(Modbus.mini_type == MINI_T10P)
	{
		if(input_raw[HI_COMMON_CHANNEL] > 1000)
			inputs[HI_COMMON_CHANNEL].value = tem_org * 100;
		inputs[HI_COMMON_CHANNEL + 2].value = hum_org * 100;
	}
	else
	{
		if(input_raw[COMMON_CHANNEL] > 1000)
			inputs[COMMON_CHANNEL].value = tem_org * 100;
		inputs[COMMON_CHANNEL + 2].value = hum_org * 100;
	}
}


void refresh_Input_Task(void) reentrant
{	
	portTickType xDelayPeriod = ( portTickType ) 1000 / portTICK_RATE_MS;
	U8_T loop,i;
	
	//chip_info[2] = 0;
	//Setting_Info.reg.specila_flag &= 0xfe;
	//chip_info[1] = 42;
	
	Input_IO_Init();
#if 1
	//MX_I2S2_Init();
#endif
	inputs_adc_init();
	Hum_Initial();
	VOC_Init();
	if(Modbus.mini_type == MINI_T10P)
	{
		pulse_set();
	}	

	for(;;)
	{			
		//vTaskDelay( 50 / portTICK_RATE_MS);
		if(Check_sensor_exist(E_FLAG_HUM))
			Read_Humidity();
		if(Check_sensor_exist(E_FLAG_TVOC))	
			Check_Voc();

		inpust_scan();
		if(loop < 5)	loop++;
		else
			flag_ready_to_scan = 1;	

		
	}
}



void refresh_Output_Task(void) reentrant
{	
	portTickType xDelayPeriod = ( portTickType ) 1000 / portTICK_RATE_MS;
	U8_T loop,i;
	UN_RELAY temp1;
	U16_T far pwm_count[5];
	U8_T DO_change_count;
	U8_T flag_DO_changed;
	U8_T auto_output_count;
	
	outputs[i].switch_status = SW_AUTO;
	for(;;)
	{			
		vTaskDelay( 50 / portTICK_RATE_MS);
				
// OUTPUT
		temp1.word = 0;
		for(i = 0;i < 5;i++)
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

			if(relay_value.byte[0] & 0x01) 					TSTAT_REALY1 = 1;				else					TSTAT_REALY1 = 0;
			if(relay_value.byte[0] & 0x02) 					TSTAT_REALY2 = 1;				else					TSTAT_REALY2 = 0;
			if(relay_value.byte[0] & 0x04) 					TSTAT_REALY3 = 1;				else					TSTAT_REALY3 = 0;
			if(relay_value.byte[0] & 0x08) 					TSTAT_REALY4 = 1;				else					TSTAT_REALY4 = 0;
			if(relay_value.byte[0] & 0x10) 					TSTAT_REALY5 = 1;				else					TSTAT_REALY5 = 0;

		}
		
		if(flag_DO_changed) 
		{
			DO_change_count++;
			if(DO_change_count > 200)  // keep stable for 500ms second,send command to pic again
			{
				flag_DO_changed = 0;
				DO_change_count = 0;

				if(relay_value.byte[0] & 0x01) 					TSTAT_REALY1 = 1;				else					TSTAT_REALY1 = 0;
				if(relay_value.byte[0] & 0x02) 					TSTAT_REALY2 = 1;				else					TSTAT_REALY2 = 0;
				if(relay_value.byte[0] & 0x04) 					TSTAT_REALY3 = 1;				else					TSTAT_REALY3 = 0;
				if(relay_value.byte[0] & 0x08) 					TSTAT_REALY4 = 1;				else					TSTAT_REALY4 = 0;
				if(relay_value.byte[0] & 0x10) 					TSTAT_REALY5 = 1;				else					TSTAT_REALY5 = 0;

			}
		}

		if(auto_output_count++ % 5 == 0)  // 250ms
		{
			Calucation_PWM_IO(0);	
			Calucation_PWM_IO(1);	
			Calucation_PWM_IO(2);	
			Calucation_PWM_IO(3);
		}
		
	}
}


uint8 Check_sensor_exist(uint8 type)
{
	if((ex_moudle.enable >= 0x55) && (ex_moudle.enable <= 0x65))
	{
//		if(type == E_FLAG_WIFI)
//		{
//			if(ex_moudle.flag && 0x01)
//				return 1;
//			else
//				return 0;
//		}
		if(type == E_FLAG_HUM)
		{
			if(ex_moudle.flag & 0x04)
				return 1;
			else
				return 0;
		}
		if(type == E_FLAG_OCC)
		{
			if(ex_moudle.flag & 0x08)
				return 1;
			else
				return 0;
		}
		if(type == E_FLAG_CO2)
		{
			if(ex_moudle.flag & 0x10)
				return 1;
			else
				return 0;
		}
		if(type == E_FLAG_PRESS)
		{
			if(ex_moudle.flag & 0x20)
				return 1;
			else
				return 0;
		}
		if(type == E_FLAG_TVOC)
		{
			if(ex_moudle.flag & 0x40)
				return 1;
			else
				return 0;
		}
		if(type == E_FLAG_LIGHT)
		{
			if(ex_moudle.flag & 0x80)
				return 1;
			else
				return 0;
		}
		if(type == E_FLAG_VOICE)
		{
			if(ex_moudle.flag & 0x100)
				return 1;
			else
				return 0;
		}
		if(type == E_FLAG_ZIGBEE)
		{
			if(ex_moudle.flag & 0x0200)
				return 1;
			else
				return 0;
		}
		if(type == E_FLAG_PM25)
		{
			if(ex_moudle.flag & 0x0400)
				return 1;
			else
				return 0;
		}
		if(type == E_FLAG_PT1K)
		{
			if(ex_moudle.flag & 0x800)
				return 1;
			else
				return 0;
		}
	}
	else
		return 1;
}

uint8_t item_to_adjust;
typedef enum
{
	D_ADDESS,
};

void start_menu(void)
{
	uint8 i;
	uint8 menu_buf1[10];
	uint8 menu_buf2[10];
//	clear_lines();
 
	for(i=0;i<10;i++)
	{
		menu_buf1[i] = menu[item_to_adjust].first_line[i];//
		menu_buf2[i] = menu[item_to_adjust].second_line[i];//
	}			

	display_menu(menu_buf1,menu_buf2);
}	

// --------------for PWM OF T3-OEM ---------------
#if 0
uint32_t pulse_plug_out[4];
uint32_t pulse_plug_in[4];
u8 flag_pulse[4]; // 0 -none, 1 -> plug in, 2 -> plug out
u8 input_flag[4];
 
#define FILTER 100
#define AC_PULSE_MAX 15
#define AC_PULSE_MIN 7
 
#define PLUG_OUT 2
#define PLUG_IN 1
#define NO_USED 0

#define DELAY_CHECK_PI 500   // US
#define DELAY_CHECK_PO 500
 
#define INPUT1 PEin(11)
#define INPUT2 PEin(12)
#define INPUT3 PEin(13)
#define INPUT4 PEin(14)

void check_signal(u8 channel)
{
	if((inputs[channel].range == V0_5) || (inputs[channel].range == V0_10_IN))//DC range. 0-5V or 0-10V
	{
		if(channel == 0)	
		{	if(INPUT1 == 0)	flag_pulse[channel] = PLUG_IN; else flag_pulse[channel] = PLUG_OUT;}
		else if(channel == 1)	
		{	if(INPUT2 == 0)	flag_pulse[channel] = PLUG_IN; else flag_pulse[channel] = PLUG_OUT;}
		else if(channel == 2)	
		{	if(INPUT3 == 0)	flag_pulse[channel] = PLUG_IN; else flag_pulse[channel] = PLUG_OUT;}
		else if(channel == 3)	
		{	if(INPUT4 == 0)	flag_pulse[channel] = PLUG_IN; else flag_pulse[channel] = PLUG_OUT;}	
	}
}
 
void signal_dealwith(u8 i)
{
	if((inputs[i].range == V0_5) || (inputs[i].range == V0_10_IN))//DC range. 0-5V or 0-10V
	{	// 检查到上升沿或者下降沿，如果脉管少于10ms当做毛刺处理
		if(flag_pulse[i] == PLUG_OUT)  
		{
			if(pulse_plug_in[i] > FILTER)
			{
				pulse_plug_out[i] = 0;
			}
			else
			{
				flag_pulse[i] = PLUG_IN;
			}
		}
		else if(flag_pulse[i] == PLUG_IN)
		{
			if(pulse_plug_out[i] > FILTER)
			{// 拔掉500ms后重新计数
				Test[17]++;
				if(pulse_plug_in[i] > 1000)
				{Test[18]++;
					pulse_plug_in[i] = 0;
				}
			}
			else
			{
				flag_pulse[i] = PLUG_OUT;
			}
		}
	}
	else if(inputs[i].range == AC_PWM)// 24vac
	{
		if(flag_pulse[i] == PLUG_OUT)  
		{				
			if((pulse_plug_in[i] <= AC_PULSE_MAX))
			{					
				input_flag[i] = PLUG_IN;
			}
			else
			{
				input_flag[i] = PLUG_OUT;
			}
		}
		else if(flag_pulse[i] == PLUG_IN)
		{			
			if((pulse_plug_out[i] <= AC_PULSE_MAX))
			{
				input_flag[i] = PLUG_IN;
			}
			else
			{
				input_flag[i] = PLUG_OUT;
			}
		}
		pulse_plug_out[i] = 0;
		pulse_plug_in[i] = 0;
		
	}
	
}
#endif
// ---------END FOR PWM----------------------

#endif