 /*================================================================================
 * Module Name : key.c
 * Purpose     : key dirver and deal with back light and buzzer
 * Author      : Chelsea
 * Date        : 2008/11/10
 * Notes       : 
 * Revision	   : 
 *	rev1.0
 *================================================================================
 */


#define KEY
#ifdef KEY

#include "main.h"
#include "key.h"


/* CONSTANT DECLARATIONS */

#define C_MAX_TIME       200
#define C_TRUE_TIME      1
#define C_HOLD_TIME_S    15
#define C_HOLD_TIME_M    30
#define C_HOLD_TIME_L 	 50



#define C_BEEP			 1        	/* last time when the buzzer is enabled */
#define	C_BACK		     1200		/* last time when the backlit is enabled */

#define 	KEY_QUEUE_SIZE 		3


/* GLOBAL VARIABLE DECLARATIONS */
U16_T w_Key_Count;
U8_T by_Key_Buffer;
U8_T b_PressKey;
U16_T w_Beep_Count;
U16_T w_Backlit_Count;
U8_T by_Key;			/* the value of button */
static U8_T by_shake_Count = 0;


xTaskHandle xKeyTask;
xQueueHandle xKeyQueue;

#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)

// ARM_CM5
#if ARM_CM5
#define  KEY_PRO 	   PEin(12)
#define  KEY_SEL 	   PEin(13)
#define  KEY_DOWN 	 PEin(14)
#define  KEY_UP 		 PEin(15)
#endif

//#if ARM_TSTAT_WIFI
//#define  KEY_PRO 	   PAin(12)
//#define  KEY_SEL 	   PAin(13)
//#define  KEY_DOWN 	 PAin(14)
//#define  KEY_UP 		 PAin(15)

//#endif
void KEY_IO_config(void)
{
#if ARM_CM5
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOE, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE,&GPIO_InitStructure);
#endif
	
#if ARM_TSTAT_WIFI
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
#endif
}

#endif


/*
 *--------------------------------------------------------------------------------
 * void Key_Inital(void)
 * Purpose : initial I/O port and datas of Key
 * Params  : none
 * Returns : none
 * Note    : 
 *--------------------------------------------------------------------------------
 */

// <summary>
//  Key_Inital: Fuction for initialing I/O port and datas of Key
//  </summary>
void Key_Inital(void)
{
#if ASIX_MINI || ASIX_CM5 
	KEY_PRO = 1;
	KEY_SEL = 1;
	KEY_DOWN = 1;
	KEY_UP = 1;	
//	KEY_COM = 0;
#endif	
	w_Key_Count = 0;
	w_Beep_Count = 0;
	w_Backlit_Count = 0;
//	BEEPER = BEEP_OFF;
	BACKLIT = BACK_ON;
	w_Backlit_Count = C_BACK;  // start up, light back board for 1min
}


/*
 *--------------------------------------------------------------------------------
 * void vStartKeyTasks( unsigned char uxPriority)
 * Purpose : start the KEY_TASK and create a queue for key event
 * Params  : none
 * Returns : none
 * Note    : 
 *--------------------------------------------------------------------------------
 */

// <summary>
//  vStartKeyTasks : Function for starting the KEY_TASK and creating a queue for key event
//  </summary>
//  <param name="uxPriority"> the priority of this KEY_TASK </param>

void vStartKeyTasks( unsigned char uxPriority)
{	

	sTaskCreate(Key_Process, (const signed portCHAR * const)"key_task",100, NULL, uxPriority, (xTaskHandle *)&xKeyTask);
	xKeyQueue = xQueueCreate(KEY_QUEUE_SIZE,sizeof(U8_T));
}



#if ! ARM_TSTAT_WIFI
void Key_Process(void) reentrant
{
	U8_T by_Dat = K_NONE;
   	U8_T by_Out = K_NONE;
	U8_T by_Key_Buffer = 0;
	portTickType xDelayPeriod = ( portTickType ) 50 / portTICK_RATE_MS;	  
//	portTickType xLastWakeTime = xTaskGetTickCount();
#if ARM_CM5 || ARM_TSTAT_WIFI
		KEY_IO_config();
#endif
	for (;;)
	{
		by_Dat = K_NONE;
		by_Out = K_NONE;
		
		vTaskDelay(xDelayPeriod);
		
#if (ASIX_MINI || ASIX_CM5)
		if(Modbus.mini_type == MINI_BIG)
			by_Key_Buffer = P3 & 0xf0;  

		if(Modbus.mini_type == MINI_CM5)
			by_Key_Buffer = P1 & 0x0f; 		

#endif
		if(by_shake_Count < 2)		by_shake_Count++;
		else   
		{
			by_shake_Count = 0;
			b_PressKey = 1;
		}
		if(!b_PressKey) 
		{
			by_Key = K_NONE; 
#if (ASIX_MINI || ASIX_CM5)
			if(xKeyQueue != 0)
			{
				cQueueSend( xKeyQueue, ( void * )&by_Key, 10);				
			}
#endif			
			continue;	 
		}
   		b_PressKey = 0;
   	/*  check the P2 to get the value of the key */
#if (ASIX_MINI || ASIX_CM5)

		if(Modbus.mini_type == MINI_BIG)
		{
			if(!(by_Key_Buffer & 0x10)) 		by_Dat = K_DOWN1;
			if(!(by_Key_Buffer & 0x20)) 		by_Dat = K_UP1;
			if(!(by_Key_Buffer & 0x40))		    by_Dat = K_SELECT1;
			if(!(by_Key_Buffer & 0x80)) 		by_Dat = K_PROGRAM1;
		}

		if(Modbus.mini_type == MINI_CM5)
		{
			if(!(by_Key_Buffer & 0x01)) 		by_Dat = K_DOWN1;
			if(!(by_Key_Buffer & 0x02)) 		by_Dat = K_UP1;
			if(!(by_Key_Buffer & 0x04))		  by_Dat = K_SELECT1;
			if(!(by_Key_Buffer & 0x08)) 		by_Dat = K_PROGRAM1;
		}
#endif
		
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
		if(KEY_DOWN == 0) 		by_Dat = K_DOWN1;
		if(KEY_UP == 0) 			by_Dat = K_UP1;
		if(KEY_SEL == 0)		  by_Dat = K_SELECT1;
		if(KEY_PRO == 0) 			by_Dat = K_PROGRAM1;
		
		Test[9] = by_Dat;
#endif
		
			if(by_Dat == K_NONE)
	   	{
		    w_Key_Count = 0;
			by_Key = K_NONE;
#if (ASIX_MINI || ASIX_CM5)
			if(xKeyQueue != 0)
			{
				cQueueSend( xKeyQueue, ( void * )&by_Key, 10);
			}  	//  aviod shakeing
#endif
			continue;
	   	}
	
		if(w_Key_Count < C_MAX_TIME)	++w_Key_Count;
	
	   	if(w_Key_Count == C_TRUE_TIME)
	   	{
	//   press any button, 1 beep  100ms			

			w_Beep_Count = C_BEEP;
			w_Backlit_Count = C_BACK;
//			BEEPER = BEEP_ON;
			BACKLIT = BACK_ON;	

			by_Out = by_Dat;

	   	}
	//  add press up and down longer, +/- 1
		if(w_Key_Count > C_HOLD_TIME_S)
	   	{
		    if(by_Dat == K_DOWN1 || by_Dat == K_UP1)
		    {
		        if((w_Key_Count % C_TRUE_TIME) == 0)
		        {
		           	by_Out = by_Dat;
		        }
		    }
	   	}
	
		if(w_Key_Count == C_HOLD_TIME_M)
	   	{
	      	if(by_Dat == K_SELECT1 || by_Dat == K_SEL_PRO1||by_Dat == K_PROGRAM1 ||by_Dat == K_UP_SEL1 || by_Dat == K_DOWN_SEL1 )
	      	{
	        	by_Out = by_Dat + 0x80;
	      	}
	   	}
		/* K_RESET hold 5s */
		if(w_Key_Count == C_HOLD_TIME_L)  
		{
			if(by_Dat == K_UP_DOWN1)
	      	{
	        	by_Out = by_Dat + 0x80;
	      	}
	   	}
	
		by_Key = by_Out;
#if (ASIX_MINI || ASIX_CM5)
		if(xKeyQueue != 0)
		{
			cQueueSend( xKeyQueue, ( void * )&by_Key, 10 );
		} 
#endif
	  continue;
		
	}
}

#else



xQueueHandle qKey;
u8 global_key = K_NONE;
u16 pre_key = K_NONE;



u8 KEY_Scan(void)
{	 
	u16 key_1st, key_2nd;
	u16 key_val = K_NONE;
	 
	key_1st = ~GPIO_ReadInputData(GPIOA) & 0xf000; // PC0-3
	delay_ms(20);
	key_2nd = ~GPIO_ReadInputData(GPIOA) & 0xf000; // PC0-3
	
	if(key_1st & key_2nd & K_DOWN)
		key_val |= K_DOWN;
	
	if(key_1st & key_2nd & K_UP)
		key_val |= K_UP;
	
	if(key_1st & key_2nd & K_LEFT)
		key_val |= K_LEFT;
	
	if(key_1st & key_2nd & K_RIGHT)
		key_val |= K_RIGHT;
	
	return  (key_val >> 12);
}
 
extern void watchdog(void);
void Key_Process(void ) reentrant
{
	u16 key_temp;
	
	static U8_T long_press_key_start = 0;
	qKey = xQueueCreate(5, 2);

 	KEY_IO_config();
//	print("Key Task\r\n");
	delay_ms(100);
	
	for( ;; )
	{
		if((key_temp = KEY_Scan()) != pre_key)
		{
			if(pre_key == 0) // 避免单键和组合键粘连
				xQueueSend(qKey, &key_temp, 0);
			pre_key = key_temp;
			long_press_key_start = 0;
			count_lcd_time_off_delay = 0;
			BACKLIT = 1;
		}
		else
		{
			if(key_temp != K_NONE)
			{		
				
//				if(long_press_key_start >= LONG_PRESS_TIMER_SPEED_100)
//					key_temp |= KEY_SPEED_100;
//				else if(long_press_key_start >= LONG_PRESS_TIMER_SPEED_50)
//					key_temp |= KEY_SPEED_50;
//				else 
				if(long_press_key_start >= LONG_PRESS_TIMER_SPEED_10)
					key_temp |= KEY_SPEED_10;
				else if(long_press_key_start >= LONG_PRESS_TIMER_SPEED_1)
					key_temp |= KEY_SPEED_1;

				if(long_press_key_start >= LONG_PRESS_TIMER_SPEED_1)
				{
					xQueueSend(qKey, &key_temp, 0);
				}

				if(long_press_key_start < LONG_PRESS_TIMER_SPEED_100)
					long_press_key_start++;
				
			}
		} 
//		watchdog();
		vTaskDelay(100 / portTICK_RATE_MS);
//		stack_detect(&test[8]);
    }
}


#endif

#endif







