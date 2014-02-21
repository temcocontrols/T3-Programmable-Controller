#include "LCD.h"
#include "font.h"
#include "string.h"
#include "display.h"
#include "main.h"
#include <ctype.h> 
#include "font.h"	  
#include "key.h"
#include "serial.h"


#define DisProcess_STACK_SIZE	((unsigned portSHORT)200)
#define UpdateSta_STACK_SIZE	((unsigned portSHORT)200)


#define 	MENU_NUM		8

#define 	IDLE_LEN		2
#define 	MAIN_MENU_LEN 	3
#define 	MAX_OUT_LEN 	24
#define 	MAX_IN_LEN 	    46
//#define 	SEN_MENU_LEN 	10
#define 	TST_MENU_LEN	13
#define 	MAX_SUB_NUM		8    
//xTaskHandle xKeyTask;
extern xQueueHandle xKeyQueue;
//extern unsigned int far temperature[10];
extern bit flag_control_by_button;

xTaskHandle xDisplayTask;		/* handle for display task */
xTaskHandle xDisplayCheckTask;  /* handle for check display status task */

xSemaphoreHandle xSemaphore_Display;  /* protect "Display_Check_Status" roution */
xQueueHandle xBtnQueue;				  /* trasmit button value when displaying ITEM */
xSemaphoreHandle xSemaphore_LCD;	  /* protect "LCD" roution */

static U16_T count_status = 0; /* count for going back to idle mode, 1 min */
//U8_T far LcdStr[5][21];	/* buffer for display */
U8_T data by_Status = 0;
unsigned char data by_tstat_index = 0;
unsigned char far by_Idle_index = 0;
unsigned char far by_menu_index = 0;
unsigned char far by_submenu_index = 0;
unsigned char far by_set_index = 0;

unsigned char far sub_menu_len = 0;
unsigned char far Value_Range[36] = 0;
unsigned char far Set_Value = 0;
unsigned char far by_Cur_sub = 0;
char* menu = NULL;

//extern unsigned int data temperature[10];
		// control Tstat using T3000 or CM5's button


void Display_Save_Value(unsigned char sub,unsigned char index);
void Display_SubMenu(unsigned char sub,unsigned char index);


const char* Main_Menu[MAIN_MENU_LEN] = 
{
	" Output",
	" Input",
	" Tstat info"
};

char far In_Menu[MAX_IN_LEN][14];
char far Out_Menu[MAX_OUT_LEN][14];


typedef enum 
{
	E_MODBUS_ID,
	E_TEMPER,
	E_MODE,
	E_SET_POINT,
	E_COOL_SP,
	E_HEAT_SP,
	E_OCC,
	E_OUT_STATUS,
	E_NIGHT_HEAT_DB,
	E_NIGHT_COOL_DB,
	E_NIGHT_HEAT_SP,
	E_NIGHT_COOL_SP,
	E_OVER_RIDE_TIME,	
}Tst_Menu_index;

const char TST_Menu[TST_MENU_LEN][15] =
{
	"Modbus ID Addr",
	"Temperature   ",
	"Mode          ",
	"Set Point     ",
	"Cool Set Point",
	"Heat Set Point",
	"Occupied      ",
	"Output Status ",
	"Night Heat DB ",
	"Night Cool DB ",
	"Night Heat SP ",
	"Night Cool SP ",
//	"Product Model ",
	"Over Ride Time",

}; 

typedef enum
{
	/*D_START = 0,*/D_IDLE = 0,D_MENU,D_SUBMENU/*,D_ITEMD_OUTPUT,D_INPUT,D_SENSOR,D_TSTAT_TEMP,D_TSTAT_MODE,D_TSTAT_SETPOINT,D_SET_MODE,D_SET_PRI,*/
};




void vStartDisplayTasks(U8_T uxPriority)
{
	sTaskCreate(Display_Process, (const signed portCHAR * const)"display_task",DisProcess_STACK_SIZE, NULL, uxPriority, (xTaskHandle *)&xDisplayTask);
	sTaskCreate(Display_Check_Task, (const signed portCHAR * const)"display_check",UpdateSta_STACK_SIZE, NULL, uxPriority + 1, (xTaskHandle *)&xDisplayCheckTask);
	vSemaphoreCreateBinary(xSemaphore_Display);
	xBtnQueue = xQueueCreate(3,sizeof(U8_T));
	vSemaphoreCreateBinary(xSemaphore_LCD);
}


void Display_Initial_Data(void)
{ 
	by_Status = D_IDLE;
	
	Display_Clear_Screen();

//	Lcd_Show_String(1,1,"CM5",NORMAL,3);
//  	Lcd_Show_String(2,1,"TEMCO LTD.,",NORMAL,10);
	by_submenu_index = 0;
//	if(protocal <= TCP_IP)
//	{
//		memset(menu_name,'\0',MAX_NAME * NAME_SIZE); 
//		menu = *menu_name;
//	}

//	DELAY_Us(5000);		DELAY_Us(5000);	DELAY_Us(5000);	DELAY_Us(5000);	
}

extern char time[];

void Display_IP(void)
{  	
	char text[20];

	sprintf(text, "IP:%u.%u.%u.%u:%u", (uint16)Modbus.ip_addr[0], (uint16)Modbus.ip_addr[1], (uint16)Modbus.ip_addr[2], (uint16)Modbus.ip_addr[3],(uint16)HTTP_SERVER_PORT);
	Lcd_Show_String(0, 0, text, NORMAL, 21);
	
	// subnet mask address		
	sprintf(text, "MASK: %u.%u.%u.%u", (uint16)Modbus.subnet[0], (uint16)Modbus.subnet[1], (uint16)Modbus.subnet[2], (uint16)Modbus.subnet[3]);
	Lcd_Show_String(1, 0, text, NORMAL, 21);
	// tcp port
	sprintf(text, "GATE: %u.%u.%u.%u", (uint16)Modbus.getway[0], (uint16)Modbus.getway[1], (uint16)Modbus.getway[2], (uint16)Modbus.getway[3]);
	Lcd_Show_String(2, 0, text, NORMAL, 21);
//	// tcp port
//	sprintf(text, "PORT: %u", (uint16)HTTP_SERVER_PORT);
//	Lcd_Show_String(3, 0, text, NORMAL, 21);
	// MAC address
	sprintf(text, "MAC:%02X:%02X:%02X:%02X:%02X:%02X", (uint16)Modbus.mac_addr[0], (uint16)Modbus.mac_addr[1], (uint16)Modbus.mac_addr[2], (uint16)Modbus.mac_addr[3], (uint16)Modbus.mac_addr[4], (uint16)Modbus.mac_addr[5]);
	Lcd_Show_String(3, 0, text, NORMAL, 21); 
//	get_time_text();
//	sprintf(text, "%s", time);
//	Lcd_Show_String(4, 0, text, NORMAL, 21); 
}

/*
 *--------------------------------------------------------------------------------
 * void Display_Clear_Screen(void)
 * Purpose : clear the display buffer
 * Params  : none
 * Returns : none
 * Note    :
 *--------------------------------------------------------------------------------
 */	 
void Display_Clear_Screen(void)
{
/*	U8_T i,j;
	for(i = 0;i < 5;i++)		
		for(j = 0;j < 21;j++)
			LcdStr[i][j] = ' ';	*/ 
//	for( i = 0;i < 5;i++)	ptrLcdStr[i] = &LcdStr[i];	
	Display_Clear_Space();
//	Display_Refresh();
	Lcd_All_Off();
}



/*
 *--------------------------------------------------------------------------------
 * void Display_Clear_Space(void)
 * Purpose : clear the left space 
 * Params  : 
 * Returns : 
 * Note    : none
 *--------------------------------------------------------------------------------
 */	
void Display_Clear_Space(void)
{
	U8_T loop,line;
	/* the lcd have a left frame, clear it to avoid srambled display */
	for(line = 0; line < 5;line++)
	{
		Lcd_Set_X_Addr(0);
		Lcd_Set_Y_Addr(line * 2); 
		for(loop = 0;loop < 6;loop++) Lcd_Write_Byte(0); 
		Lcd_Set_X_Addr(0);
		Lcd_Set_Y_Addr(line * 2+1);
		for(loop = 0;loop < 6;loop++) Lcd_Write_Byte(0); 
	}
}

#if 0
/*
 *--------------------------------------------------------------------------------
 * char* Display_Format(U16_T number)
 * Purpose : tranfer num to string
 * Params  : number - the source data  (range:0 - 9999)
 * Returns : return string
 * Note    : none
 *--------------------------------------------------------------------------------
 */	 
char* Display_Format(U16_T number)
{
	char loop = 0;
	char num[5];
	char length;
	//number = number / div;
	for(loop = 0;loop < 5;loop++)	num[loop] = 0;

	if(number >= 1000)	length = 4;
	else if(number >= 100)	length = 3;
	else if(number >= 10)	length = 2;
	else length = 1;

	/* using num[] buffer to store the every num */
	
	num[0] = number / 1000; 		number = number % 1000;
	num[1] = number / 100; 		number = number % 100;
	num[2] = number / 10; 		number = number % 10;
	num[3] = number;

	/* check num[], put correct character to every posion */

	if(num[0] > 0)  /* if number is bigger than 999 */
	{
		for(loop = 1;loop < 4;loop++)	num[loop] = num[loop] + 0x30;
	}
	else if(num[1] > 0)  /* if number is bigger than 99 */
	{
		num[0] = ' ';
		for(loop = 1;loop < 4;loop++)	num[loop] = num[loop] + 0x30;		
	}
	else if(num[2] > 0) 	 /* if number is bigger than 9 */
	{
		num[0] = ' ';
		num[1] = ' ';
		for(loop = 2;loop < 4;loop++)	num[loop] = num[loop] + 0x30;
	}
	else if(num[3] > 0)   /* if number is less than 10 */
	{
		num[0] = ' ';
		num[1] = ' ';
		num[2] = ' ';
		for(loop = 3;loop < 4;loop++)	num[loop] = num[loop] + 0x30;
	}
	num[4] = '\0';
	return num;
}

#endif

void Display_Check_Status(void)
{
	U8_T loop;
	if(by_Key != 0)	count_status = 0; /* if press any key,count_Status is 0 */
	if(by_Status == D_IDLE) 
	{	
		if(by_Key == K_PROGRAM)		{by_Status = D_MENU; by_Idle_index = 0;stop_scrolling();}
		if(by_Key == K_SELECT)		
		{	
			if(by_Idle_index < IDLE_LEN - 1)  /* idle menu have 2 pages now , press this button to enter next page */
				by_Idle_index++;
			else by_Idle_index = 0;	
			
			if(by_Idle_index == IDLE_LEN - 1)	 stop_scrolling();
			else
				start_scrolling();
				 
		}
	}
	else if(by_Status == D_MENU) 
	{
		if(by_Key == K_PROGRAM)		{	by_Status = D_IDLE; by_menu_index = 0;Lcd_All_Off(); start_scrolling();}
		if(by_Key == K_SELECT)	   	/* enter sub-menu */
		{	
			by_Status = D_SUBMENU; 
			by_Cur_sub = 0;
			by_submenu_index = 0;
		}
		if(by_Key == K_UP)	{	if(by_menu_index < MAIN_MENU_LEN - 1)		by_menu_index++; else by_menu_index = 0; }
		if(by_Key == K_DOWN)	{	if(by_menu_index > 0)		by_menu_index--; else by_menu_index = MAIN_MENU_LEN - 1; }
	}
	else if(by_Status == D_SUBMENU) 
	{
		if(by_Key == K_PROGRAM)		{	by_Status = D_MENU; by_submenu_index = 0;Display_Save_Value(by_menu_index,by_submenu_index);}	// tbd:
		if(by_Key == K_SELECT)	   	/* enter sub-menu */
		{	
			if(sub_menu_len > 0)
			{
				if(by_submenu_index < sub_menu_len - 1)		by_submenu_index++; 
				else by_submenu_index = 0;
			}
		}
		if(by_Key == K_UP && (Value_Range[by_submenu_index] > 0))	
		{
		}
		if(by_Key == K_DOWN && (Value_Range[by_submenu_index] > 0))	
		{ 
		}
	}
}
	


void Display_Check_Task(void) reentrant
{
	portTickType xDelayPeriod = ( portTickType ) 50 / portTICK_RATE_MS; //  100 
	for (;;)
	{
		Test[9]++;
		vTaskDelay(xDelayPeriod);	

		#if 1

		while( ucQueueMessagesWaiting( xKeyQueue ) )
		{
			
			if( cQueueReceive( xKeyQueue, &by_Key, ( portTickType ) 0 ) == pdPASS )
			{ /* any button is pressed */
				if(cSemaphoreTake( xSemaphore_Display, ( portTickType ) 10 ) == pdTRUE)
				{	
					Display_Check_Status();	
					cSemaphoreGive( xSemaphore_Display );
				}
			}
			
		} 

		#endif

	}
}




void Display_Process(void) reentrant
{
	portTickType xDelayPeriod = ( portTickType ) 100 / portTICK_RATE_MS; //  100 
//    portTickType xLastWakeTime = xTaskGetTickCount();
	U8_T first_Status,second_Status;

	for (;;)
	{	
		char loop;
	//	static char pre_Status = 0;
		vTaskDelay(xDelayPeriod);
			Test[5]++;
		#if 1
			second_Status = by_Status;
			/* if change status, clear screen and initial lcd again to avoid messed display */
			if(first_Status != second_Status)	
			{
				Lcd_Initial();	
				Display_Clear_Screen();	 //  clear display
				first_Status = second_Status;
				count_status = 0;
			}
			else  
			{/* keep status for 1 min, go to idle mode */
			
				count_status++;
				if(count_status >= 300) // 1min
				{	
					BACKLIT = BACK_OFF;	
				/* if current display is IDLE, initial lcd to avoid messed display */
					if(by_Status == D_IDLE)	{/*Lcd_Initial();Lcd_All_Off();*/}
					else if(by_Status == D_SUBMENU || by_Status == D_MENU)
					{
						by_Status = D_IDLE;
						by_Idle_index = 0; 					
					} 
					start_scrolling();
					count_status = 0;
				}
			}
		update_message_context();
		scrolling_message();  
		
		

		if(by_Status == D_IDLE)		Display_Idle(by_Idle_index);
		else if(by_Status == D_MENU)		Display_Menu(by_menu_index);
		else if(by_Status == D_SUBMENU)		//Lcd_Show_String(3,1,"sub menu ,",NORMAL,10); 
			Display_SubMenu(by_menu_index,by_submenu_index); 
		
		#endif		
	
	}

}



void Display_Idle(U8_T index)
{
	static char pre_status = 0;
	static char count = 0;
	static char i = 0;
	char text[20];

	if(pre_status != index)
	{	
		Display_Clear_Screen();
		pre_status = index;
	}
/*	if(index == 0)
	{
		if(count < dis_temp_interval) 
			count++;
		else 
		{
			count = 0;
			Lcd_Show_String(0,0,menu_name[dis_temp_seq[i] + 26] ,NORMAL,10);
			if(Modbus.unit == 0)	
			{
				Lcd_Show_String(1,18,"~",NORMAL,3);
				print_number(temperature[dis_temp_seq[i]], 1);
			}
			else//(Modbus_Data.unit == UNIT_F)
			{
				Lcd_Show_String(1,18,"|",NORMAL,3);
				print_number(temperature[dis_temp_seq[i]] * 9 / 5 + 320, 1);
			}

			if(i < dis_temp_num - 1)			
				i++;
			else 
				i = 0;	
		
		}

	}

	else if(index == 1)
	{
		Lcd_Show_String(0,5,"CM5 DEMO  ",NORMAL,21);
		Lcd_Show_String(1,1,"MOD  ",NORMAL,5);	
		if(DO_SoftSwitch == OFF)
			Lcd_Show_String(1,5,"OFF  ",NORMAL,5);	//Display_Str(0,4,"OFF ",4);
		else if(DO_SoftSwitch == ON)
			Lcd_Show_String(1,5,"ON  ",NORMAL,5);//Display_Str(0,4,"ON  ",4);
		else if(DO_SoftSwitch == TIMER)
			Lcd_Show_String(1,5,"TIMER  ",NORMAL,5);//Display_Str(0,4,"TIME",4);

		Lcd_Show_String(2,1,"PRI  ",NORMAL,5); 
		Lcd_Show_Data (2,6,Priority,0,1);
		
		 
		Lcd_Show_String(3,1,"TIME      :",NORMAL,12); 
		Lcd_Show_Data (3,6,(Priority * 60 - count_priority) / 60,0,1);
		Lcd_Show_Data (3,12,(Priority * 60 - count_priority) % 60,0,1);
	}
	else if(index == 1) // relay value  
	{	 
		

	}
	else if(index == 2)	 // di input value
	{
		Lcd_Show_String(0,1,"DI Input  ",NORMAL,20);

	}
	else if(index == 3)	 // ip info	*/
	if(index == 1)
	{
		// ip address
		sprintf(text, "IP:   %u.%u.%u.%u", (U16_T)Modbus.ip_addr[0], (U16_T)Modbus.ip_addr[1], (U16_T)Modbus.ip_addr[2], (U16_T)Modbus.ip_addr[3]);
		Lcd_Show_String(0, 0, text, NORMAL, 21);
		// subnet mask address
		sprintf(text, "MASK: %u.%u.%u.%u", (U16_T)Modbus.subnet[0], (U16_T)Modbus.subnet[1], (U16_T)Modbus.subnet[2], (U16_T)Modbus.subnet[3]);
		Lcd_Show_String(1, 0, text, NORMAL, 21);
		// tcp port
		sprintf(text, "GATE: %u.%u.%u.%u", (U16_T)Modbus.getway[0], (U16_T)Modbus.getway[1], (U16_T)Modbus.getway[2], (U16_T)Modbus.getway[3]);
		Lcd_Show_String(2, 0, text, NORMAL, 21);
		// tcp port
		sprintf(text, "PORT: %u", (U16_T)HTTP_SERVER_PORT);
		Lcd_Show_String(3, 0, text, NORMAL, 21);
		// MAC address
		sprintf(text, "MAC:%02X:%02X:%02X:%02X:%02X:%02X", (U16_T)Modbus.mac_addr[0], (U16_T)Modbus.mac_addr[1], (U16_T)Modbus.mac_addr[2], (U16_T)Modbus.mac_addr[3], (U16_T)Modbus.mac_addr[4], (U16_T)Modbus.mac_addr[5]);
		Lcd_Show_String(4, 0, text, NORMAL, 21);
		
	}
}




void Display_Menu(unsigned char index)
{
	unsigned char  flag_high_light;	
	unsigned char  start_line;
	unsigned char  loop1;

	Lcd_Show_String(0,0,"MAIN BOARD MENU  ",NORMAL,21);
//	if(index < 4)
//	{
//		start_line = 0;
//		flag_high_light = index;
//	}
//	else
//	{
//		start_line = index - 3;
//		flag_high_light = 3;
//	}
//	for(loop1= 0;loop1 < 4;loop1++)
//	{
//		if(loop1 == flag_high_light)
//			Lcd_Show_String(loop1 + 1,0,*(Main_Menu + start_line+ loop1),INVERSE,21);
//		else  
//			Lcd_Show_String(loop1 + 1,0,*(Main_Menu + start_line + loop1),NORMAL,21);	
//	}
//	Lcd_Show_String(4,0,"                 ",NORMAL,21);

} 


void Display_SubMenu(unsigned char sub,unsigned char index)
{
//	unsigned int far PTRtemp[26];
//
//	unsigned char  flag_high_light;	
//	unsigned char  start_line;
//	unsigned char  loop1;
//	unsigned int tempValue;
//	unsigned char str_len;
//	unsigned char i;
//
//	Lcd_Show_String(0,0,"MAIN BOARD MENU  ",NORMAL,21);
//	if(sub == 2 && sub_no == 0) 
//	{
//		Lcd_Show_String(1,0,"                    ",NORMAL,21);
//		Lcd_Show_String(2,0,"                    ",NORMAL,21);
//		Lcd_Show_String(3,0," No TSTAT CONNECTED ",NORMAL,21);
//		Lcd_Show_String(4,0,"                    ",NORMAL,21);
//		return;
//	}
//	
//	if(sub == 0)
//	{	
//		U8_T loop = 0;		
//		str_len = 14;
//		sub_menu_len = 0;
//		
//		loop = 0; 
//		for(i = 0;i < 12;i++)
//			if(DO_Enable & (0x01 << i))	
//			{
//				PTRtemp[sub_menu_len] =  (relay_value.word & (0x01 << i)) >> i;
//				Value_Range[sub_menu_len] = 0;
//
////				memcpy(In_Menu[sub_menu_len],menu_name[i],14); 
//				sub_menu_len++;	
//				loop++;
//			}
//
//		loop = 0; 	
//		for(i = 0;i < 12;i++)
//			if(AO_Enable & (0x01 << i))	
//			{
////				PTRtemp[sub_menu_len] =  Modbus.AOUTPUT[i];
//				Value_Range[sub_menu_len] = 1;
//
//				memcpy(In_Menu[sub_menu_len],menu_name[i + 12],14); 
//				sub_menu_len++;	
//				loop++;
//			}
//		menu = *In_Menu; 
//
//	}	
//	else if(sub == 1)
//	{
//		U8_T loop = 0;
//		sub_menu_len = 0;
//		str_len = 14;
// 
//		for(i = 0;i < sub_no;i++)	
//		{
//			PTRtemp[sub_menu_len] = sub_addr[i];
//			Value_Range[sub_menu_len] = 0;
//
////			memcpy(In_Menu[sub_menu_len],menu_name[i + 10],14);
//			sub_menu_len++;
//			loop++;
//
//		}
//		
//		loop = 0; 
//		for(i = 0;i < 32;i++)
//			if(AI_Enable & (0x01 << i))	
//			{
//				PTRtemp[sub_menu_len] = Input[i];
//				Value_Range[sub_menu_len] = 0;
//
////				memcpy(In_Menu[sub_menu_len],menu_name[i + 26],14); 
//				sub_menu_len++;	
//				loop++;
//			}
//
//		if(sub_menu_len > 0)
//			menu = *In_Menu;
////		Test[30] = sub_menu_len;	
//	
//	}
//	else if(sub == 2)
//	{	
//		menu = *TST_Menu; 
//		sub_menu_len = 13;  
//		str_len = 15;
////		PTRtemp[0] = sub_addr[by_Cur_sub];	  Value_Range[0] = sub_no - 1;
////		PTRtemp[1] = tstat_temperature[by_Cur_sub];	   Value_Range[1] = 0;
////		PTRtemp[2] = tstat_mode[by_Cur_sub];		   Value_Range[2] = 1;
////		PTRtemp[3] = tstat_setpoint[by_Cur_sub];	   Value_Range[3] = 1;
////		PTRtemp[4] = tstat_cool_setpoint[by_Cur_sub];  Value_Range[4] = 1;
////		PTRtemp[5] = tstat_heat_setpoint[by_Cur_sub];  Value_Range[5] = 1;
////		PTRtemp[6] = (tstat_occupied & (0x01 << by_Cur_sub)) >> by_Cur_sub;   Value_Range[6] = 1;
////		PTRtemp[7] = tstat_output_state[by_Cur_sub];   Value_Range[7] = 0;
////		PTRtemp[8] = tstat_night_heat_db[by_Cur_sub];  Value_Range[8] = 1;
////		PTRtemp[9] = tstat_night_cool_db[by_Cur_sub];  Value_Range[9] = 1;
////		PTRtemp[10] = tstat_night_heat_sp[by_Cur_sub]; Value_Range[10] = 1;
////		PTRtemp[11] = tstat_night_cool_sp[by_Cur_sub]; Value_Range[11] = 1;
////		PTRtemp[12] = tstat_over_ride[by_Cur_sub];	   Value_Range[12] = 1;
//	}
//
//	if(Value_Range[index] > 0)
//	{
//		if(sub == 2 && index == 0)
//			Set_Value = by_Cur_sub;
//		else
//			Set_Value = PTRtemp[index];//tempValue;
//	} 
//
//	if(index < 4)
//	{
//		start_line = 0;
//		flag_high_light = index;
//	}
//	else
//	{
//		start_line = index - 3;
//		flag_high_light = 3;
//	}
//
//	if(sub_menu_len == 0 && sub == 1)
//	{
//		Lcd_Show_String(2,0,"NO INPUT",NORMAL,21);
//		return;	
//	}
//	else
//	{
//		U8_T max_row;
//		if(sub_menu_len < 4)
//		{
//			max_row = sub_menu_len;
//			for(loop1 = sub_menu_len;loop1 < 4;loop1++)
//				Lcd_Show_String(loop1 + 1,0,"                     ",NORMAL,21);
//		}
//		else 
//			max_row = 4;
//		for(loop1 = 0;loop1 < max_row;loop1++)
//		{
//			//if(sub_menu_len > loop1)	
//			//{
//		
//
//				Lcd_Show_String(loop1 + 1,0,&(menu + str_len *(start_line + loop1)),NORMAL,10);
//				
//				if(loop1 == flag_high_light)
//				{
//					if(sub == 0)	 // output menu
//					{
//						if(Value_Range[start_line+ loop1] == 0)
//						{
//							if(PTRtemp[start_line+ loop1] == 1)
//								Lcd_Show_String(loop1 + 1,16,"ON",INVERSE,5);
//							else
//								Lcd_Show_String(loop1 + 1,16,"OFF",INVERSE,5);
//						}
//						else
//						{
//							Lcd_Show_Data (loop1 + 1,16,PTRtemp[start_line+ loop1],0,INVERSE);
//						}
//						
//					}
//					else  if(sub == 1)	  // input menu
//					{
//					   	if(Value_Range[start_line+ loop1] == 0)	 
//							 Lcd_Show_Data (loop1 + 1,16,PTRtemp[start_line+ loop1],0,INVERSE);
//						else if(Value_Range[index] > 0) // DI
//						{
//							if(PTRtemp[start_line+ loop1] == 1)
//								Lcd_Show_String(loop1 + 1,16,"ON",INVERSE,5);
//							else
//								Lcd_Show_String(loop1 + 1,16,"OFF",INVERSE,5);
//						}	
//					}
//					else  if(sub == 2)	   // tst menu
//						Lcd_Show_Data (loop1 + 1,16,PTRtemp[start_line+ loop1],0,INVERSE);
//				}
//				else 
//				{ 
//					if(sub == 0)
//					{
//						if(Value_Range[start_line+ loop1] == 0)
//						{
//							if(PTRtemp[start_line+ loop1] == 1)
//								Lcd_Show_String(loop1 + 1,16,"ON",NORMAL,5);
//							else
//								Lcd_Show_String(loop1 + 1,16,"OFF",NORMAL,5);
//						}
//						else
//						{
//							Lcd_Show_Data (loop1 + 1,16,PTRtemp[start_line+ loop1],0,NORMAL);
//						}
//					}
//					else  if(sub == 1)
//					{
//					   	if(Value_Range[start_line+ loop1] == 0)	  // read only , sub_tst and temperature
//							 Lcd_Show_Data (loop1 + 1,16,PTRtemp[start_line+ loop1],0,NORMAL);
//						else if(Value_Range[start_line+ loop1] > 0) // DI
//						{
//							if(PTRtemp[start_line+ loop1] == 1)
//								Lcd_Show_String(loop1 + 1,16,"ON",NORMAL,5);
//							else
//								Lcd_Show_String(loop1 + 1,16,"OFF",NORMAL,5);
//						}	
//					}
//					else  if(sub == 2)	
//						Lcd_Show_Data (loop1 + 1,16,PTRtemp[start_line+ loop1],0,NORMAL);
//				}	
//			//} 
//		} 
//	}
} 


void Display_Save_Value(unsigned char sub,unsigned char index)
{
//	U8_T WRT_Tst_Reg;
//	switch(sub) 
//	{
//		case 0:  
//		/*	if(Set_Value)	
//				DO_Value |= (0x01 << index);
//			else	 
//				DO_Value &= ~(0x01 << index); */
//			break;
//		case 1:	 
//		/* input menu - sub DI DI*/
//		//	DI2_Value |= Set_Value << (index - sub_no);
//			break;
//		case 2:	 
//			if(index == E_MODBUS_ID)	  
//				by_Cur_sub = Set_Value;
//			else if(index == E_SET_POINT)
//			{
//				tst_info[by_Cur_sub].setpoint = Set_Value;				
//				WRT_Tst_Reg = Tst_Register[TST_ROOM_SETPOINT][tst_info[by_Cur_sub].type];
//				write_parameters_to_nodes(by_Cur_sub,WRT_Tst_Reg,Set_Value);
//			}
//			else if(index == E_COOL_SP)
//			{
//				tst_info[by_Cur_sub].cool_setpoint = Set_Value;
//				WRT_Tst_Reg = Tst_Register[TST_COOL_SETPOINT][tst_info[by_Cur_sub].type];
//				write_parameters_to_nodes(by_Cur_sub,WRT_Tst_Reg,Set_Value);
//			}
//			else if(index == E_HEAT_SP)
//			{
//				tst_info[by_Cur_sub].heat_setpoint = Set_Value;
//				WRT_Tst_Reg = Tst_Register[TST_HEAT_SETPOINT][tst_info[by_Cur_sub].type];
//				write_parameters_to_nodes(by_Cur_sub,WRT_Tst_Reg,Set_Value);
//			} 		
//			else if(index == E_NIGHT_HEAT_DB)
//			{
//				tst_info[by_Cur_sub].night_heat_db = Set_Value;
//				WRT_Tst_Reg = Tst_Register[TST_NIGHT_HEAT_DB][tst_info[by_Cur_sub].type];
//				write_parameters_to_nodes(by_Cur_sub,WRT_Tst_Reg,Set_Value);
//			}
//			else if(index == E_NIGHT_COOL_DB)
//			{
//				tst_info[by_Cur_sub].night_cool_db = Set_Value;
//				WRT_Tst_Reg = Tst_Register[TST_NIGHT_COOL_DB][tst_info[by_Cur_sub].type];
//				write_parameters_to_nodes(by_Cur_sub,WRT_Tst_Reg,Set_Value);
//			}
//			else if(index == E_NIGHT_HEAT_SP) 
//			{
//				tst_info[by_Cur_sub].night_heat_sp = Set_Value;				
//				WRT_Tst_Reg = Tst_Register[TST_NIGHT_HEAT_SP][tst_info[by_Cur_sub].type];
//				write_parameters_to_nodes(by_Cur_sub,WRT_Tst_Reg,Set_Value);
//			}
//			else if(index == E_NIGHT_COOL_SP)
//			{
//				tst_info[by_Cur_sub].night_cool_sp = Set_Value;
//				WRT_Tst_Reg = Tst_Register[TST_NIGHT_COOL_DB][tst_info[by_Cur_sub].type];
//				write_parameters_to_nodes(by_Cur_sub,WRT_Tst_Reg,Set_Value);
//			}
////			else if(index == E_OVER_RIDE_TIME)
////			{
////				tst_info[by_Cur_sub].over_ride = Set_Value;
////				write_parameters_to_nodes(by_Cur_sub,WRT_Tst_Reg,Set_Value);			
////			}
//			break;
//		default: break;
//	}
//	#endif

}

