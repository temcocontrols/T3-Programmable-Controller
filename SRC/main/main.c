/*
 *********************************************************************************
 *     Copyright (c) 2005   ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 *********************************************************************************
 */

/* Standard includes. */

#include "main.h"

#define Tcpip_STACK_SIZE	((unsigned portSHORT)1024)
#define Common_STACK_SIZE	((unsigned portSHORT)1024)
#define Schedule_STACK_SIZE	((unsigned portSHORT)1024)
#define UpdateAISTACK_SIZE	((unsigned portSHORT)1024)
#define SampleDISTACK_SIZE	((unsigned portSHORT)1024)
#define UpdateDISTACK_SIZE	((unsigned portSHORT)1024)
#define ScanReslut_STACK_SIZE	((unsigned portSHORT)1024)





xTaskHandle xHandleCommon;
xTaskHandle xSoftWatchTask;
xTaskHandle xHandleTcp;
xTaskHandle xHandleSchedule;
xTaskHandle Handle_UpdateAI; 
xTaskHandle Handle_SampleDI;
xTaskHandle Handle_UpdateDI;
xTaskHandle xHandleScanReslut;

U16_T far Test[50];
U8_T ChangeFlash = 0;
U8_T WriteFlash = 0;


/*
	put E2prom data to buffer when start-up 
*/
void Read_ALL_Data(void)
{
	U8_T loop;
	U8_T  temp[64];

	/* base infomation */
//	for(loop = 0;loop < 4;loop++)	
//	E2prom_Read_Byte(EEP_SERIALNUMBER_LOWORD + loop,& serialNum[loop]);

//	E2prom_Read_Byte(EEP_ADDRESS,& address);
	E2prom_Read_Byte(EEP_BAUDRATE,& baudrate);
	 baudrate = 1;
	if( baudrate == 0) // 9600
	{
		PCON = 0;
	}
	else   // 19200
	{
		PCON |= 0xc0;
	}
	E2prom_Read_Byte(EEP_UPDTE_STATUS,& update_status);
	E2prom_Read_Byte(EEP_BASE_ADDRESS,& BASE_ADRESS);
	E2prom_Read_Byte(EEP_TCP_TYPE,& TCP_TYPE);
//	TCP_TYPE = 0; // for test 
	if( TCP_TYPE == 0)  // static ip, read ip address fromm E2prom
	{
		for(loop = 0;loop < 4;loop++)
		{
			E2prom_Read_Byte(EEP_IP + loop,&IP_Addr[loop]);
		//	 IP_Addr[loop] = temp[loop];
	
		//	E2prom_Read_Byte(EEP_SUBNET + loop,&SUBNET[loop]);
		//	 SUBNET[loop] = temp[loop];
	
		}
		SUBNET[0] = 255;
		SUBNET[1] = 255;
		SUBNET[2] = 255;
		SUBNET[3] = 0;

		GETWAY[0] = 192;
		GETWAY[1] = 168;
		GETWAY[2] = 0;
		GETWAY[3] = 1;
	}	
	
 	for(loop = 0;loop < 4;loop++) 
		E2prom_Read_Byte(EEP_SERIALNUMBER_LOWORD + loop,&serialNum[loop]);
//	Modbus_Data.baudrate = read_eeprom(EEP_BAUDRATE);

//	SNWriteflag = 0;//read_eeprom(EEP_SERIALNUMBER_WRITE_FLAG);

	E2prom_Read_Byte(EEP_UNIT,&unit);


	for(loop = 0;loop < 10;loop++) 
	{
		E2prom_Read_Byte(EEP_INPUT1_RANGE + loop,&Input_Range[loop]);
		if(Input_Range[loop] > 5 || Input_Range[loop] < 0)
				Input_Range[loop] = 0;
		E2prom_Read_Byte(EEP_INPUT1_FILTER + loop,&Input_Filter[loop]);
		E2prom_Read_Int(EEP_INPUT1_CAL + loop * 2,&Input_CAL[loop]);		
	}
	
	for(loop = 0;loop < 8;loop++) 
		E2prom_Read_Byte(EEP_DI_TYPE1 + loop,&DI_Type[loop]);

	E2prom_Read_Int(EEP_OUTPUT_LOW,&DO_Value);
	E2prom_Read_Byte(EEP_SWITCH,&DO_SoftSwitch);
	E2prom_Read_Byte(EEP_PRIORTITY,&Priority);
//	Master = read_eeprom(EEP_MASTER);
	
	E2prom_Read_Int(EEP_DI_ENABLE_LOW,&DI_Enable);
	E2prom_Read_Int(EEP_AI_ENABLE_LOW,&AI_Enable);
	E2prom_Read_Int(EEP_DINPUT_AM_LOW,&DInputAM);
	E2prom_Read_Int(EEP_OUTPUT_AM_LOW,&OuputAM);
	E2prom_Read_Int(EEP_AINPUT_AM_LOW,&AInputAM);


//	for(loop = 0;loop < 7;loop++) 
//		Modbus_Data.Time.all[loop] = read_eeprom(EEP_SEC + loop);
	
	E2prom_Read_Byte(EEP_DAYLIGHT_STATUS,&daylight_flag);
	if(daylight_flag > 2)
	{
		daylight_enable = 0;
		E2prom_Write_Byte(EEP_DAYLIGHT_ENABLE,daylight_enable);
		daylight_flag  = 0;
		E2prom_Write_Byte(EEP_DAYLIGHT_STATUS,daylight_flag);
	}
	else
	{
		E2prom_Read_Byte(EEP_DAYLIGHT_ENABLE,&daylight_enable);
	}

	E2prom_Read_Byte(EEP_DIS_TEMP_NUM,&dis_temp_num);
	if(dis_temp_num > 9)   dis_temp_num = 0;
	E2prom_Read_Byte(EEP_DIS_TEMP_INTERVAL,&dis_temp_interval);
	for(loop = 0;loop < 10;loop++)
	{
		E2prom_Read_Byte(EEP_DIS_TEMP_SEQ_FIRST + loop,&dis_temp_seq[loop]);
		if(dis_temp_seq[loop] > 9)   dis_temp_num = 0;
	}
	
}

void Read_Info_From_Flash(void)
{
	

	Flash_Read_Schedule();

}


void set_default_parameters(void)
{ 
	U8_T i;
	E2prom_Write_Byte(EEP_ADDRESS, 254);	
//	E2prom_Write_Byte(EEP_PRODUCT_MODEL, 50);	
//	E2prom_Write_Byte(EEP_HARDWARE_REV, 4);	
	E2prom_Write_Byte(EEP_UNIT, 0);
//	E2prom_Write_Byte(EEP_SERIALNUMBER_WRITE_FLAG, 0);
	for(i = 0;i < 8;i++)
	{	
		E2prom_Write_Byte(EEP_SUBADDR1 + i, 0);
		E2prom_Write_Byte(EEP_DI_TYPE1 + i, DI_TSTAT);
	}	
	for(i = 0;i < 10;i++)
	{	
		E2prom_Write_Byte(EEP_INPUT1_RANGE + i, 0);
		E2prom_Write_Byte(EEP_INPUT1_FILTER + i, 2);
		E2prom_Write_Byte(EEP_INPUT1_CAL + 2 * i, 150);	 // default value is 150
		E2prom_Write_Byte(EEP_INPUT1_CAL + 2 * i + 1, 0);
	}

	E2prom_Write_Byte(EEP_OUTPUT_LOW, 0);
	E2prom_Write_Byte(EEP_OUTPUT_HIGH, 0);
	E2prom_Write_Byte(EEP_SWITCH, 0);
	E2prom_Write_Byte(EEP_PRIORTITY, 0);

	E2prom_Write_Byte(EEP_DI_ENABLE_LOW, 0);
	E2prom_Write_Byte(EEP_DI_ENABLE_HIGH, 0);
	E2prom_Write_Byte(EEP_AI_ENABLE_LOW, 0);
	E2prom_Write_Byte(EEP_AI_ENABLE_HIGH, 0);
	E2prom_Write_Byte(EEP_DINPUT_AM_LOW, 0);
	E2prom_Write_Byte(EEP_DINPUT_AM_HIGH, 0);
	E2prom_Write_Byte(EEP_OUTPUT_AM_LOW, 0);
	E2prom_Write_Byte(EEP_OUTPUT_AM_HIGH, 0);
	E2prom_Write_Byte(EEP_AINPUT_AM_LOW, 0);
	E2prom_Write_Byte(EEP_AINPUT_AM_HIGH, 0);

//	E2prom_Write_Byte(EEP_FIRST_TIME, 0xaa);
	E2prom_Write_Byte(EEP_TCP_TYPE,0);

	E2prom_Write_Byte(EEP_IP, 192);
	E2prom_Write_Byte(EEP_IP + 1, 168);
	E2prom_Write_Byte(EEP_IP + 2, 0);
	E2prom_Write_Byte(EEP_IP + 3, 178);

	E2prom_Write_Byte(EEP_SUBNET, 255);
	E2prom_Write_Byte(EEP_SUBNET + 1, 255);
	E2prom_Write_Byte(EEP_SUBNET + 2, 255);
	E2prom_Write_Byte(EEP_SUBNET + 3, 0);
	
	E2prom_Write_Byte(EEP_DIS_TEMP_NUM,1);
	E2prom_Write_Byte(EEP_DIS_TEMP_INTERVAL,5);
	for(i = 0;i < 8;i++)
		E2prom_Write_Byte(EEP_DIS_TEMP_SEQ_FIRST + i,0); 

   	memset(WR_Roution,'\0',sizeof(STR_WR) * MAX_WR);
	memset(AR_Roution,'\0',sizeof(STR_AR) * MAX_AR);
	memset(ID_Config,'\0',sizeof(UN_ID) * MAX_ID);

	memset(menu_name,'\0',MAX_NAME * NAME_SIZE);	// 10 output 26 input
  
	 // for test
//	 for(i = 0;i < 36;i++)
//	 	memcpy(menu_name[i],"         ",NAME_SIZE);
//	IntFlashErase(ERA_RUN,0x70000);	
	Flash_Write_Schedule();


}


void Output_Count_Priority_Task(void);

void Common_task(void) reentrant
{
	static U8_T count = 0;
//	static U8_T relay_value = 0;
	static U16_T refresh_flash_timer = 0;
	portTickType xDelayPeriod = ( portTickType ) 200 / portTICK_RATE_MS;
	for (;;)
	{
		vTaskDelay(xDelayPeriod);
		Test[2]++;
	//	count++;
	/*{
		Test[2]++;
	//	Flash_Write_Schedule();
	//	Lcd_Show_Data(3,15,Test[2],0,1);
		}
		else

		{
			Test[2] = 0;
		//	Lcd_Initial();	
		}  */
	 //   P0 = ~P0;
	 //	control_logic();
		Updata_Clock();	
		Output_Count_Priority_Task();
	

		if(ChangeFlash == 1)
		{  
			ChangeFlash = 0;
			refresh_flash_timer = 25;			
		}

		if(refresh_flash_timer)
		{
			refresh_flash_timer--;
			if(refresh_flash_timer == 0)
			{
				WriteFlash = 1;
				Flash_Write_Schedule();
				WriteFlash = 0;
			}
		}

	
	}
}




void watchdog(void)
{
	/*software watchdog */
	#if AX_WATCHDOG_ENB
		TA = 0xAA;
		TA = 0x55;
		RWT=1;
	#endif
}





void SoftwareWatchdog_task(void) reentrant
{	
	portTickType xDelayPeriod = ( portTickType ) 100 / portTICK_RATE_MS;
	for (;;)
	{
		vTaskDelay(xDelayPeriod);
		/* clear watch dog */
		watchdog();
	}
}




void Read_ALL_Data(void);
void vStartDisplayTasks(U8_T uxPriority);
void Display_Initial_Data(void);


void main( void )
{
	U8_T                regisp;
	U8_T loop;
	U8_T flag_store_schedule;


	AX11000_Init();

	#if (RUNTIME_CODE_START_ADDRESS == RUNTIME_CODE_START_AT_24kH)
		ExecuteRuntimeFlag = 1;
	#else
		ExecuteRuntimeFlag = 0;
	#endif

#if AX_WATCHDOG_ENB
	AX11000_WatchDogSetting(0, 1, 0, WD_INTERVAL_67M);  /* time out, reset cpu */
	sTaskCreate(SoftwareWatchdog_task, (const signed portCHAR * const)"softwatch_task",portMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 5 , (xTaskHandle *)&xSoftWatchTask);
#endif
	for(loop = 0;loop < 50;loop++)
		Test[loop] = 0;

	DELAY_Init();
	UART_Init(0);
	UART_Init(1);
	Key_Inital();
	Lcd_Initial();
//	Test_program();
	Display_Initial_Data();	
	E2prom_Initial();
	initSerial();
	Flash_Inital();
	
	Comm_Tstat_Initial_Data();
	calculate_ID_table();		

	// read pic version

	loop = 0;

#if (RUNTIME_CODE_START_ADDRESS == RUNTIME_CODE_START_AT_24kH)

	IntFlashReadByte(0x60001,&regisp);
	IntFlashReadByte(0x6ffff,& IspVer);
	if(regisp == 0xff && regisp != 0xaa)
	{ /* initial defautl parameters*/
	//	Eeprom_Write_Cpu_Config();
		set_default_parameters();
		regisp = 0xaa;
		IntFlashWriteByte(0x60001,regisp); 		 
	}
#endif
	IntFlashReadByte(0x7fff0, &flag_store_schedule);
	if(flag_store_schedule == 0x55)	
	{
		//Flash_Read_Schedule(); // read scheduel data from flash
		Read_Info_From_Flash();
 	}
	Read_ALL_Data(); 
	initial_input_value();

	sTaskCreate(TCPIP_Task,"TCPIP_task",Tcpip_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, (xTaskHandle *)&xHandleTcp);
	sTaskCreate(Schedule_task,"schedule_task",Schedule_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, (xTaskHandle *)&xHandleSchedule);
	sTaskCreate(Common_task,"Common_task",Common_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, (xTaskHandle *)&xHandleCommon);
	
	vStartCommSubTasks(tskIDLE_PRIORITY + 8);
//	vStartSubSerialTasks(tskIDLE_PRIORITY + 12);
//	sTaskCreate( Update_AI_Task, "UpdateAItask", UpdateAISTACK_SIZE, NULL, tskIDLE_PRIORITY + 6, &Handle_UpdateAI );
	sTaskCreate( Sampel_DI_Task, "SampleDItask", SampleDISTACK_SIZE, NULL, tskIDLE_PRIORITY + 5, &Handle_SampleDI );
	sTaskCreate( Update_DI_Task, "UpdateDItask", UpdateDISTACK_SIZE, NULL, tskIDLE_PRIORITY + 7, &Handle_UpdateDI );
	vStartUpdateOutputTasks(tskIDLE_PRIORITY + 4); 
	vStartDisplayTasks(tskIDLE_PRIORITY + 9); 	 
	vStartKeyTasks(tskIDLE_PRIORITY + 11);
	vStartMainSerialTasks(tskIDLE_PRIORITY + 13);	    
	
	/* Finally kick off the scheduler.  This function should never return. */
	vTaskStartScheduler( portUSE_PREEMPTION );

	/* Should never reach here now under control of the scheduler. */

}
/*-----------------------------------------------------------*/


