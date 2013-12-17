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
//U8_T by_Key_Buffer;
U8_T b_PressKey;
U16_T w_Beep_Count;
U16_T w_Backlit_Count;
U8_T by_Key;			/* the value of button */
static U8_T by_shake_Count = 0;

xTaskHandle xdata xKeyTask;
xQueueHandle xdata xKeyQueue;




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
	KEY_PRO = 1;
	KEY_SEL = 0;
	KEY_DOWN = 1;
	KEY_UP = 0;	
//	KEY_LATCH = 0;
	
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
	sTaskCreate(Key_Process, (const signed portCHAR * const)"key_task",portMINIMAL_STACK_SIZE, NULL, uxPriority, (xTaskHandle *)&xKeyTask);
	xKeyQueue = xQueueCreate(KEY_QUEUE_SIZE,sizeof(U8_T));
}



/*
 *--------------------------------------------------------------------------------
 * void Key_Process(void)
 * Purpose : key task
 *			1. check key event,send the value of the key to another task
 *			2. control the back light and buzzer
 * Params  : none
 * Returns : none
 * Note    : 
 *--------------------------------------------------------------------------------
 */

// <summary>
//  Key_Process : Function for dealing with key task
//  1. check key event,send the value of the key to another task
// 	2. control the back light and buzzer
//  </summary>
extern xTaskHandle xdata Handle_PWMoutput;

void Key_Process(void) reentrant
{
	U8_T by_Dat = K_NONE;
   	U8_T by_Out = K_NONE;
	U8_T by_Key_Buffer = 0;
	portTickType xDelayPeriod = ( portTickType ) 50 / portTICK_RATE_MS;	  
//	portTickType xLastWakeTime = xTaskGetTickCount();

	for (;;)
	{
		by_Dat = K_NONE;
		by_Out = K_NONE;
		vTaskDelay(xDelayPeriod);
	//	Test[10]++;
	//	EA = 0;
		
	
		DI1_LATCH = 1; 
		DI2_LATCH = 1; 
		RELAY_LATCH = 0;
		KEY_LATCH = 0;
		P0 = 0xff;
		DELAY_Us(10); 
		by_Key_Buffer = P0 & 0xcc;  // KEYPAND P0 
		KEY_LATCH = 1;
//		EA = 1;	 
		 
	
	#if 0
		by_Key = K_NONE; 

		if(!(by_Key_Buffer & 0x04)) 		by_Key = K_PROGRAM;
		if(!(by_Key_Buffer & 0x08))		    by_Key = K_SELECT;
		if(!(by_Key_Buffer & 0x40)) 		by_Key = K_UP;
		if(!(by_Key_Buffer & 0x80)) 		by_Key = K_DOWN;
	#endif
#if 1
		// by_shake_Count, avoid shakeing
		if(by_shake_Count < 2)		by_shake_Count++;
		else   
		{
			by_shake_Count = 0;
			b_PressKey = 1;
		}
		if(!b_PressKey) 
		{
			by_Key = K_NONE; 
			if(xKeyQueue != 0)
			{
				cQueueSend( xKeyQueue, ( void * )&by_Key, 10);				
			} 
			continue;	 
		}
   		b_PressKey = 0;
	
   	/*  check the P2 to get the value of the key */
//		if(!(by_Key_Buffer & 0x04)) 		{by_Dat = K_PROGRAM; Test[35]++;}
//		if(!(by_Key_Buffer & 0x08))		    {by_Dat = K_SELECT; Test[36]++;}
//		if(!(by_Key_Buffer & 0x40)) 		{by_Dat = K_UP;	 Test[37]++;}
//		if(!(by_Key_Buffer & 0x80)) 		{by_Dat = K_DOWN;Test[38]++;}  
		if(by_Key_Buffer == 200) 		{by_Dat = K_PROGRAM; }
		if(by_Key_Buffer == 196)		{by_Dat = K_SELECT;}
		if(by_Key_Buffer == 140) 		{by_Dat = K_UP;	}
		if(by_Key_Buffer == 76) 		{by_Dat = K_DOWN;}
		/* the following are combine keys*/ 
		if(!(by_Key_Buffer & 0x80) && !(by_Key_Buffer & 0x40)) { by_Dat = K_UP_DOWN;	}
		if(!(by_Key_Buffer & 0x04) && !(by_Key_Buffer & 0x08)) { by_Dat = K_SEL_PRO;  }

	   	if(by_Dat == K_NONE)
	   	{
		    w_Key_Count = 0;
			by_Key = K_NONE;
			if(xKeyQueue != 0)
			{
				cQueueSend( xKeyQueue, ( void * )&by_Key, 10);
			}  	//  aviod shakeing	 			
			continue;
	   	}

		if(w_Key_Count < C_MAX_TIME)	++w_Key_Count;
	
	   	if(w_Key_Count == C_TRUE_TIME)
	   	{
	//   press any button, 1 beep  100ms			

			w_Beep_Count = C_BEEP;
			w_Backlit_Count = C_BACK;
			BACKLIT = BACK_ON;	
			by_Out = by_Dat;
			

	   	}
	//  add press up and down longer, +/- 1
		if(w_Key_Count > C_HOLD_TIME_S)
	   	{
		    if(by_Dat == K_DOWN || by_Dat == K_UP)
		    {
		        if((w_Key_Count % C_TRUE_TIME) == 0)
		        {
		           	by_Out = by_Dat;
		        }
		    }
	   	}
		#if 1
		if(w_Key_Count == C_HOLD_TIME_M)
	   	{
	      	if(by_Dat == K_SELECT || by_Dat == K_SEL_PRO||by_Dat == K_PROGRAM ||by_Dat == K_UP_SEL || by_Dat == K_DOWN_SEL )
	      	{
	        	by_Out = by_Dat + 0x80;
				Test[41]++;
	      	}
	   	}
		/* K_RESET hold 5s */
		if(w_Key_Count == C_HOLD_TIME_L)  
		{
			if(by_Dat == K_UP_DOWN)
	      	{
	        	by_Out = by_Dat + 0x80;
				Test[42]++;
	      	}
	   	}
		 #endif
		by_Key = by_Out;   

		if(xKeyQueue != 0)
		{
			cQueueSend( xKeyQueue, ( void * )&by_Key, 10 );
		} 
	  continue;

#endif
	  //taskYIELD();
	}
}



#endif




