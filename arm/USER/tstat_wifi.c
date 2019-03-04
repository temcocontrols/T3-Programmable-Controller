#include "main.h"

#if ARM_WIFI
#include "controls.h"
#include "constcode.h"
// only for tstat-wifi
/*
-> output
-> input 

*/


#define SEL1_IN   PAout(6)
#define SEL2_IN   PAout(7)
#define SEL3_IN   PCout(4)

#define RANGE_SET0			PCout(2)
#define RANGE_SET1			PFout(3)



u8_t flag_ready_to_scan = 0;
U8_T table_pwm[8];
U8_T slop[10];
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
#ifdef LIJUN 
	 RANGE_SET0 = 0 ;
	 RANGE_SET1 = 1 ;
#endif
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


void Input_IO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC |RCC_APB2Periph_GPIOF, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_5; 	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_SetBits(GPIOC, GPIO_Pin_2);
	GPIO_ResetBits(GPIOC, GPIO_Pin_5);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10; 	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOF, GPIO_Pin_6 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10);	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7; 	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA, GPIO_Pin_6 | GPIO_Pin_7);	
	GPIO_SetBits(GPIOA, GPIO_Pin_5);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5; 	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOC, GPIO_Pin_4);
	
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
	u32 temp;
	temp  = 0;
	for(i = 0;i < 10;i++)
	{
		temp += ADC_getChannal(ADC1,ADC_Channel_11,1);
	}
	
	input_raw[channel_count] = temp / 40;		
//	Test[10 + channel_count] = input_raw[channel_count];
	channel_count++;
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

	
	// AO1 AO2
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	TIM_TimeBaseStructure.TIM_Period = 1000;
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
	
	TIM_OC3Init(TIM2, &TIM_OCInitStructure);  // AO1 PB10 	TIM2_CH3
	TIM_OC4Init(TIM2, &TIM_OCInitStructure);  // AO2 PB11 	TIM2_CH4
	
}

extern U16_T test_adc;
extern U8_T test_adc_flag;
void Calucation_PWM_IO(U8_T refresh_index)
{
	U32_T duty_Cycle1,duty_Cycle2;
	U32_T temp1,temp2;
//	U8_T loop;
	U32_T far adc1;
	U32_T far adc2; /* adc1 is for the first 4051, adc2 is for the second 4051 */

	
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
		
		if(outputs[refresh_index + 6].range == 0)		
			adc1 = 0;

		duty_Cycle1 = adc1;

		if(refresh_index == 0)		
			TIM_SetCompare3(TIM2, duty_Cycle1);			
		else if(refresh_index == 1)
			TIM_SetCompare4(TIM2, duty_Cycle1);

}

extern U8_T flag_read_switch;
void Check_Pulse_Counter(void);
void Update_Led(void);
UN_RELAY relay_value;
void refresh_Input_Output_Task(void) reentrant;
xTaskHandle xdata xHandler_Output;
void vStartOutputTasks( unsigned char uxPriority)
{

	Output_IO_Init();
	sTaskCreate( refresh_Input_Output_Task, "OutputTask1", 200, NULL, uxPriority, &xHandler_Output );

	relay_value.word = 0;
}

void refresh_Input_Output_Task(void) reentrant
{	
	portTickType xDelayPeriod = ( portTickType ) 1000 / portTICK_RATE_MS;
	U8_T loop,i;
	UN_RELAY temp1;
	U16_T far pwm_count[2];
	U8_T DO_change_count;
	U8_T flag_DO_changed;
	U8_T auto_output_count;
	
	Input_IO_Init();
	inputs_adc_init();
	for(;;)
	{		
		vTaskDelay( 200 / portTICK_RATE_MS);	
		inpust_scan();
		if(loop < 5)	loop++;
		else
			flag_ready_to_scan = 1;
		
// OUTPUT
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
		}
		
	}
}



/****************************** display ************************************/

uint8 update_flag = 0;
//PTBDSTR KEY_TEMP;
uint8 menu_id;
uint8 pre_menu_id = 0;

xTaskHandle xDisplyTask;


void Display_process(void)
{
	uint8 menu_buf[9],i;
	static uint8 top_refresh = 0;
	static uint8 rs485_warning = 0;
	uint8 bk_blink = 0;
	 
  icon.unit = 1;
	icon.setpoint = 1;
	icon.fan = 1;
	icon.sysmode = 1;
	icon.occ_unocc = 1;
	icon.fanspeed = 1;
	LCDtest();
	
  scroll = &scroll_ram[0][0];
//	fanspeedbuf = fan_speed_user;
		
	disp_null_icon(240, 36, 0, 0,TIME_POS,TSTAT8_CH_COLOR, TSTAT8_MENU_COLOR2);
	disp_icon(14, 14, degree_o, UNIT_POS - 14,56 ,TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
	
	draw_tangle(102,105);
	draw_tangle(102,148);
	draw_tangle(102,191);
	
	disp_str(FORM15X30, SCH_XPOS,  SETPOINT_POS,  "SET",SCH_COLOR,TSTAT8_BACK_COLOR);//TSTAT8_BACK_COLOR
	disp_str(FORM15X30, SCH_XPOS,  FAN_MODE_POS,"FAN",SCH_COLOR,TSTAT8_BACK_COLOR);
	disp_str(FORM15X30, SCH_XPOS,  SYS_MODE_POS,  "SYS",SCH_COLOR,TSTAT8_BACK_COLOR);

	Top_area_display(TOP_AREA_DISP_ITEM_TEMPERATURE, 258, TOP_AREA_DISP_UNIT_C);
	
	for(;;)
		{		
			display_scroll();
			
			display_SP(123);
			display_fanspeed(3);
			display_mode();	
			
			top_refresh++;
			if(top_refresh > 5)
				top_refresh = 0;
			
			if(update_flag == 1)
			{
				ClearScreen(TSTAT8_BACK_COLOR); 
				disp_str(FORM15X30, 30,  30,  "Updating",TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
//				serial_restart();
				SoftReset();
			}
			else if(update_flag == 2)
			{
				update_flag = 0;
				ClearScreen(TSTAT8_BACK_COLOR); 
				disp_str(FORM15X30, 30,  30,  "Restart",TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
//				initialize_eeprom( ) ; // set default parameters
//				DisRestart( );
			}
			else if(update_flag == 4)//switch baudrate
			{
				update_flag = 0;
//			if(uart0_baudrate == UART_19200)
//				{
//				uart1_init(19200);
//				modbus.baudrate = 19200;
//				}
//			else if(uart0_baudrate == UART_9600)
//				{
//				uart1_init(9600);
//				modbus.baudrate = 9600;
//				}

//			else if(uart0_baudrate == UART_38400)
//				{
//				uart1_init(38400);
//				modbus.baudrate = 38400;
//				}
//			else if(uart0_baudrate == UART_57600)
//				{
//				uart1_init(57600);
//				modbus.baudrate = 57600;
//				}
//			else if(uart0_baudrate == UART_115200)
//				{
//				uart1_init(115200);
//				modbus.baudrate = 115200;
//				}
//			else if(uart0_baudrate == UART_76800)
//				{
//				uart1_init(76800);
//				modbus.baudrate = 76800;
//				}	
//			else if(uart0_baudrate == UART_1200)
//				{
//				uart1_init(1200);
//				modbus.baudrate = 1200;
//				}
//			else if(uart0_baudrate == UART_4800)
//				{
//				uart1_init(4800);
//				modbus.baudrate = 4800;
//				}
//			else if(uart0_baudrate == UART_14400)
//				{
//				uart1_init(14400);
//				modbus.baudrate = 14400;
//				}
//			if(modbus.com_config[0] == MODBUS)	
//				serial_restart();				
			}
			
			else if((update_flag == 5)||(update_flag == 7))
			{
//				Set_night_setpoint(((setpoint_buf >> 8)&0xff), (setpoint_buf & 0xff));
//				refresh_setpoint(NIGHT_MODE);
			  update_flag = 0;
			}
			else if((update_flag == 6)||(update_flag == 8))
			{
//				Set_day_setpoint(((setpoint_buf >> 8)&0xff), (setpoint_buf & 0xff));
//				refresh_setpoint(DAY_MODE);
			  update_flag = 0;
			}			

			else if(update_flag == 11)
			{
//				mass_flash_init();
				update_flag = 0;
			}
			
			else if(update_flag == 15)
			{
				disp_str(FORM15X30, SCH_XPOS,  SETPOINT_POS,  "SET",SCH_COLOR,TSTAT8_BACK_COLOR);
				disp_str(FORM15X30, SCH_XPOS,	 FAN_MODE_POS,  "FAN",SCH_COLOR,TSTAT8_BACK_COLOR);
				disp_str(FORM15X30, SCH_XPOS,  SYS_MODE_POS,  "SYS",SCH_COLOR,TSTAT8_BACK_COLOR);
				update_flag = 0;
			}			
			
			else
			{
//					blink_parameter = !blink_parameter;
//					if(menu_mode == 1)
//					{				
//						if((blink_parameter == 0)&&((item_to_adjust<EEP_CLOCK_YEAR) || (item_to_adjust>EEP_PROTOCOL_SEL/*EEP_SCHEDULE_WEEKEND_NIGHTTIME_MINUTE*/)))
//								clear_line(2);
//						else 
//								show_parameter(); 
//					}
//					else
//					{	
//						if(testlcd)
//						{
//							;
//						}
//						else
//						{
//							if(exist_menu)
//							{
//								clear_lines();
//								icon.setpoint = 1;
//								icon.fan = 1;
//								icon.sysmode = 1;
////								icon.heatcool = 1;
//								icon.occ_unocc = 1;

//								draw_tangle(102,105);
//								draw_tangle(102,148);
//								draw_tangle(102,191);
//								
//								disp_str(FORM15X30, SCH_XPOS,  SETPOINT_POS,  "SET",SCH_COLOR,TSTAT8_BACK_COLOR);//TSTAT8_BACK_COLOR
//								disp_str(FORM15X30, SCH_XPOS,FAN_MODE_POS,"FAN",SCH_COLOR,TSTAT8_BACK_COLOR);
//								disp_str(FORM15X30, SCH_XPOS,  SYS_MODE_POS,  "SYS",SCH_COLOR,TSTAT8_BACK_COLOR);
//								exist_menu = 0;
//							}
//							blink_flag = !blink_flag;
//							display_flag = 1;
//							if(top_refresh == 5)
//								//Top_area_display(TOP_AREA_DISP_ITEM_TEMPERATURE, -250, TOP_AREA_DISP_UNIT_C);
//								Top_area_display(TOP_AREA_DISP_ITEM_TEMPERATURE, temperature, TOP_AREA_DISP_UNIT_C);
//													
//							if(Show_ID_Enable)
//							{
//								if(showidbit == 0)
//								{
//								clear_lines();
//								showidbit = 1;
//								}						
//								disp_str(FORM15X30, 0,  MENU_ITEM1,  "MODBUS",TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
//								disp_str(FORM15X30, 0,  MENU_ITEM2,  "ID",TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
//								display_value(0,laddress, ' ');
//							}	
//							else							
//							{
//								if(HardwareVersion > HW_VERSION)
//									{
//										if(voltage_overshoot == 1)//rs485 lines have high voltage 
//										{
//											clear_lines();//disp_str(FORM15X30, 0,  0,  "testing",TSTAT8_CH_COLOR,TSTAT8_BACK_COLOR);
//											//103,TSTAT8_CH_COLOR
//											disp_str(FORM15X30, 0,  103,  "High",0xf800,TSTAT8_BACK_COLOR);
//											disp_str(FORM15X30, 0,  103+BTN_OFFSET,  "Voltage",0xf800,TSTAT8_BACK_COLOR);							
//											disp_str(FORM15X30, 0,  103+BTN_OFFSET+BTN_OFFSET,  "on RS485",0xf800,TSTAT8_BACK_COLOR);

//											rs485_warning = 1;
//										}
//										else if(rs485_warning == 1 && voltage_overshoot == 0)//overshoot voltage has gone, refresh normal display
//										{
//											rs485_warning = 0;
//											clear_lines();
//											icon.setpoint = 1;
//											icon.fan = 1;
//											icon.sysmode = 1;
//											icon.heatcool = 1;
//											icon.occ_unocc = 1;

//											draw_tangle(102,105);
//											draw_tangle(102,148);
//											draw_tangle(102,191);
//											disp_null_icon(239, 45, 0, 0,TIME_POS,TSTAT8_CH_COLOR, TSTAT8_MENU_COLOR2);
//										}	
//										else
//										{
//										display_SP(loop_setpoint[0]);
//										display_fanspeed(fanspeedbuf);
//										display_mode();	
//										}										
//									}
//									
//									else
//									{
//										display_SP(loop_setpoint[0]);
//										display_fanspeed(fanspeedbuf);
//										display_mode();	
//									}
//								}	
//									
//									
//								for(i=0;i<9;i++)
//									icon_control_output(i);
//							
//								display_icon();
//								//refresh_icon_output();
//								display_fan();
//							if(HardwareVersion > HW_VERSION)
//							{
//								if(voltage_overshoot == 1)
//									scroll_warning(0);
//								else
//								{
//									if(bacnet_detect)
//										scroll_warning(1);
//									else
//										display_scroll();	
//								}
//							}
//							else
//							{
//								if(bacnet_detect)
//									scroll_warning(1);
//								else								
//									display_scroll();
//							}
//							if(icon.cmnct_send > 0)
//								disp_icon(13, 26, cmnct_send, 	0,	0, TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
//							else
//								disp_null_icon(13, 26, 0, 0,0,TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);//(26, 26, cmnct_icon, 	0,	0, TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);

//							if(icon.cmnct_rcv > 0)
//								disp_icon(13, 26, cmnct_rcv, 	13,	0, TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
//							else
//								disp_null_icon(13, 26, 0, 13,0,TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);//(26, 26, cmnct_icon, 	0,	0, TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
//	
//							#ifndef TSTAT7_ARM
//							if(icon.cmnct_send > 0)
//								icon.cmnct_send--;
//							if(icon.cmnct_rcv > 0)
//								icon.cmnct_rcv--;
//							#endif	

//							
//							#ifdef TSTAT8_HUNTER
//							if((icon.cmnct_send > 0) || (icon.cmnct_rcv > 0))//uart is active 
//								RS485_LED_ON
//							else
//								RS485_LED_OFF
//							#endif //TSTAT8_HUNTER
//							
//						}
//					}					
//					DealWithKey(menu_id);										
			}
			display_icon();
			display_fan();
			delay_ms(300);
		}
}
		
//void DealWithKey(uint8 key_id)
//{



//	switch(	menu_id	)
//	{
//		case INCREASE_COOLING_SETPOINT :
//	 		  
//			increase_cooling_setpoint() ;
//		break ;

//		case DECREASE_COOLING_SETPOINT :
//		 		  
//			decrease_cooling_setpoint() ;
//		break ;

//		case INCREASE_FAN_SPEED	:
////		 	if(EEP_SpecialMenuLock != 5)	  
//				increase_fan_speed() ;
//		break;
//		
//		case T8_DECREASE_FAN_SPEED	:
////		 	if(EEP_SpecialMenuLock != 5)	  
//				decrease_fan_speed() ;
//		break;
//		
//		case SELECT_ITEM_UP:
//			  Selectitem_up();
//		break;

//		case SELECT_ITEM_DOWN:
//			  Selectitem_down();
//		break;		
//		
//		case INCREASE_SYS_MODE://DECREASE_FAN_SPEED:
//			if(EEP_SpecialMenuLock != 5)	
//				increase_sysmode() ;  
//			//decrease_fan_speed();
//		break ;

//		case DECREASE_SYS_MODE://DECREASE_FAN_SPEED:
//			if(EEP_SpecialMenuLock != 5)	
//				decrease_sysmode() ;  
//			//decrease_fan_speed();
//		break ;			
//			
//		case START_MENU_MODE :
//			clear_lines();
//				
//			start_menu_mode();
//			if(EEP_SpecialMenuLock == 0 || EEP_SpecialMenuLock == 4)   // USER UNLOCK
//			{
//				part_lock = 0;
//			  start_menu_mode();
//			}
//			else if(EEP_SpecialMenuLock == 2 || EEP_SpecialMenuLock == 3 || EEP_SpecialMenuLock == 5)
//			{
//				start_menu_mode();
//				part_lock = 1 ;
//			}
//			
//		break ;

////		case EXIT_MENU_MODE :
////		  
////			exit_menu_mode() ;
////		break ;

//		case START_NEXT_MENU :
//		  
//			start_next_menu() ;
//		break ;

//		case START_PREVIOUS_MENU :
//		  
//			start_previous_menu() ;
//		break ;

//		case INCREASE_PARAMETER	:
//		  
//			increase_parameter() ;
//		break ;

//		case DECREASE_PARAMETER	:
//		  
//			decrease_parameter() ;
//		break ;

//		case ACCEPT_PARAMETER :
//		  
//			accept_parameter() ; 

//	    break ;

//		case ACCEPT_EXIT_PARAMETER :
//		  
//			accept_parameter() ;
////			exit_menu_mode() ;
//		
//		break ;

//  	default:
//			break;

//	}	// end switch
//	menu_id = 0;
//}	

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


void vStartDisplayTasks( unsigned char uxPriority)
{	
	sTaskCreate(Display_process, (const signed portCHAR * const)"dispaly_task",100, NULL, uxPriority, (xTaskHandle *)&xDisplyTask);
}

#endif