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
#define Common_STACK_SIZE	((unsigned portSHORT)100)
#define Schedule_STACK_SIZE	((unsigned portSHORT)512)
#define UpdateAISTACK_SIZE	((unsigned portSHORT)200)
#define SampleDISTACK_SIZE	((unsigned portSHORT)200)
#define UpdateDISTACK_SIZE	((unsigned portSHORT)200)
//#define ScanReslut_STACK_SIZE	((unsigned portSHORT)1024)
#define BACnet_STACK_SIZE	((unsigned portSHORT)4000)


/*
U16_T xdata test_buf[500];
U16_T xdata test_buf1[500];
U16_T xdata test_buf2[200];
U16_T xdata test_buf3[200];
U16_T xdata test_buf4[200];
*/

xTaskHandle xdata xHandleCommon;
xTaskHandle xdata xSoftWatchTask;
xTaskHandle xdata xHandleTcp;
xTaskHandle xdata xHandleSchedule;
xTaskHandle xdata Handle_UpdateAI; 
xTaskHandle xdata Handle_SampleDI;
xTaskHandle xdata Handle_UpdateDI;
xTaskHandle xdata xHandleScanReslut;
xTaskHandle xdata xHandleMSTP;  
xTaskHandle xdata xHandleBacCon;

U16_T far Test[50] _at_ 0x10050;
U8_T far ChangeFlash = 0;
U8_T far WriteFlash = 0;

void Bacnet_Control(void) reentrant;

/*
	put E2prom data to buffer when start-up 
*/
void Read_ALL_Data(void)
{
	U8_T loop;
	U8_T  far temp[64];

	/* base infomation */
	for(loop = 0;loop < 4;loop++)	
	E2prom_Read_Byte(EEP_SERIALNUMBER_LOWORD + loop,& serialNum[loop]);

//	E2prom_Read_Byte(EEP_ADDRESS,& Modbusaddress);
	E2prom_Read_Byte(EEP_PROTOCAL,&protocal); //protocal = TCP_IP;

	E2prom_Read_Byte(EEP_BAUDRATE,&baudrate);
	 baudrate = 1;
	if( baudrate == 0) // 9600
	{
		PCON = 0;
	}
	else   // 19200
	{
		PCON |= 0xc0;
	}

	E2prom_Read_Byte(EEP_INSTANCE_LOW,&temp[0]);
	E2prom_Read_Byte(EEP_INSTANCE_HIGH,&temp[1]);

	Instance = temp[1] * 256 + temp[0];

	
		

	E2prom_Read_Byte(EEP_UPDTE_STATUS,& update_status);
	E2prom_Read_Byte(EEP_BASE_ADDRESS,& BASE_ADRESS);
	E2prom_Read_Byte(EEP_TCP_TYPE,& TCP_TYPE);
//	TCP_TYPE = 0; // for test 
	//if( TCP_TYPE == 0)  // static ip, read ip address fromm E2prom
	{
		for(loop = 0;loop < 4;loop++)
		{
			E2prom_Read_Byte(EEP_IP + loop,&IP_Addr[3 - loop]);	 	
			E2prom_Read_Byte(EEP_SUBNET + loop,&SUBNET[3 - loop]);	
		}
		for(loop = 0;loop < 6;loop++)
		{
			E2prom_Read_Byte(EEP_MAC + loop,&Mac_Addr[5 - loop]);
		}
		
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
		E2prom_Read_Byte(EEP_INPUT1_CAL + loop * 2,&temp[0]);
		E2prom_Read_Byte(EEP_INPUT1_CAL + loop * 2 + 1,&temp[1]);
		Input_CAL[loop] = 	temp[1] * 256 + temp[0];
	}
	
	for(loop = 0;loop < 8;loop++) 
		E2prom_Read_Byte(EEP_DI_TYPE1 + loop,&DI_Type[loop]);

	E2prom_Read_Byte(EEP_OUTPUT_LOW,&temp[0]);
	E2prom_Read_Byte(EEP_OUTPUT_HIGH,&temp[1]);
	DO_Value = 	temp[1] * 256 + temp[0];

	E2prom_Read_Byte(EEP_SWITCH,&DO_SoftSwitch);
	E2prom_Read_Byte(EEP_PRIORTITY,&Priority);
//	Master = read_eeprom(EEP_MASTER);
													 
	E2prom_Read_Byte(EEP_DI_ENABLE_LOW,&temp[0]);
	E2prom_Read_Byte(EEP_DI_ENABLE_HIGH,&temp[1]);
	DI_Enable = 	temp[1] * 256 + temp[0];
	E2prom_Read_Byte(EEP_AI_ENABLE_LOW,&temp[0]);
	E2prom_Read_Byte(EEP_AI_ENABLE_HIGH,&temp[1]);
	AI_Enable = 	temp[1] * 256 + temp[0];
	E2prom_Read_Byte(EEP_DINPUT_AM_LOW,&temp[0]);
	E2prom_Read_Byte(EEP_DINPUT_AM_HIGH,&temp[1]);
	DInputAM = 	temp[1] * 256 + temp[0];
	E2prom_Read_Byte(EEP_OUTPUT_AM_LOW,&temp[0]);
	E2prom_Read_Byte(EEP_OUTPUT_AM_HIGH,&temp[1]);
	OuputAM = 	temp[1] * 256 + temp[0];
/*	E2prom_Read_Byte(EEP_AINPUT_AM_LOW,&temp[0]);
	E2prom_Read_Byte(EEP_AINPUT_AM_HIGH,&temp[1]);
	AInputAM = 	temp[1] * 256 + temp[0];*/
	AInputAM = 0;

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

	E2prom_Read_Byte(EEP_PORT_LOW,&temp[0]);
	E2prom_Read_Byte(EEP_PORT_HIGH,&temp[1]);

	TCP_PORT = temp[1] * 256 + temp[0];
}

void Read_Info_From_Flash(void)
{
	

//	Flash_Read_Schedule();
	Flash_Read_Mass();
}


void set_default_parameters(void)
{ 
	U8_T i;
	E2prom_Write_Byte(EEP_ADDRESS, 254);	
//	E2prom_Write_Byte(EEP_PRODUCT_MODEL, 50);	
//	E2prom_Write_Byte(EEP_HARDWARE_REV, 4);	
	E2prom_Write_Byte(EEP_UNIT, 0);
//	E2prom_Write_Byte(EEP_SERIALNUMBER_WRITE_FLAG, 0);
	E2prom_Write_Byte(EEP_PROTOCAL, 1/*TCP_IP*/);
    E2prom_Write_Byte(EEP_TCP_TYPE,0);

	E2prom_Write_Byte(EEP_IP, 174);
	E2prom_Write_Byte(EEP_IP + 1, 0);
	E2prom_Write_Byte(EEP_IP + 2, 168);
	E2prom_Write_Byte(EEP_IP + 3, 192);

	E2prom_Write_Byte(EEP_SUBNET, 0);
	E2prom_Write_Byte(EEP_SUBNET + 1, 255);
	E2prom_Write_Byte(EEP_SUBNET + 2, 255);
	E2prom_Write_Byte(EEP_SUBNET + 3, 255);

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
//	E2prom_Write_Byte(EEP_AINPUT_AM_LOW, 0);
//	E2prom_Write_Byte(EEP_AINPUT_AM_HIGH, 0);

//	E2prom_Write_Byte(EEP_FIRST_TIME, 0xaa);
	E2prom_Write_Byte(EEP_PORT_LOW,6001);
	E2prom_Write_Byte(EEP_PORT_HIGH,6001 >> 8);

	E2prom_Write_Byte(EEP_DIS_TEMP_NUM,1);
	E2prom_Write_Byte(EEP_DIS_TEMP_INTERVAL,5);
	for(i = 0;i < 10;i++)
		E2prom_Write_Byte(EEP_DIS_TEMP_SEQ_FIRST + i,0); 
   	memset(WR_Roution,0,sizeof(STR_WR) * MAX_WR1);
	memset(AR_Roution,0,sizeof(STR_AR) * MAX_AR1);
	memset(ID_Config,0,sizeof(UN_ID) * MAX_ID);

	memset(menu_name,0,MAX_NAME * NAME_SIZE);	// 10 output 26 input
	 // for test
//	 for(i = 0;i < 36;i++)
//	 	memcpy(menu_name[i],"         ",NAME_SIZE);
	IntFlashErase(ERA_RUN,0x70000);	
//	Flash_Write_Schedule();
// 	Flash_Write_Mass();

}


 void bip_Init(U16_T localPort);
void UpdateIpSettings(U32_T ip);

void Output_Count_Priority_Task(void);
extern U16_T far old_output,new_output;
extern bit flag_bip_active;	 	
U8_T flag_lcd_pic = 0;

void Common_task(void) reentrant
{
	static U8_T count = 0;
//	static U8_T relay_value = 0;
	char text[20];
	static U16_T refresh_flash_timer = 0;
	static U16_T count_bip_active = 0;
	portTickType xDelayPeriod = ( portTickType ) 500 / portTICK_RATE_MS;
	for (;;)
	{
		vTaskDelay(xDelayPeriod);
		if(protocal == BAC_IP)
		{
			if(flag_bip_active)
			{					
				flag_bip_active = 0;
				count_bip_active = 0;			
			}
	
			if(!flag_bip_active)
			{
			   	count_bip_active++;
				if(count_bip_active > 50)   // inactive over 20s, reset eth
				{
					ETH_Init();
				    bip_Init(0xbac0);
				    UpdateIpSettings(0);
					HTTP_Init();
				    ETH_Start();
					count_bip_active = 0;
					Test[2]++;
	
				}
			}
		}
		Updata_Clock();	

		count++; 
		if(count == 50)
		{	
		   Lcd_Initial(); 			   
           count = 0;

//		   		sprintf(text, "IP:   %u.%u.%u.%u", (uint16)IP_Addr[0], (uint16)IP_Addr[1], (uint16)IP_Addr[2], (uint16)IP_Addr[3]);
//				Lcd_Show_String(0, 0, text, NORMAL, 21);
//				
//				// subnet mask address		
//				sprintf(text, "MASK: %u.%u.%u.%u", (uint16)SUBNET[0], (uint16)SUBNET[1], (uint16)SUBNET[2], (uint16)SUBNET[3]);
//				Lcd_Show_String(1, 0, text, NORMAL, 21);
//				// tcp port
//				sprintf(text, "GATE: %u.%u.%u.%u", (uint16)GETWAY[0], (uint16)GETWAY[1], (uint16)GETWAY[2], (uint16)GETWAY[3]);
//				Lcd_Show_String(2, 0, text, NORMAL, 21);
//				// tcp port
//				sprintf(text, "PORT: %u", (uint16)HTTP_SERVER_PORT);
//				Lcd_Show_String(3, 0, text, NORMAL, 21);
//				// MAC address
//				sprintf(text, "MAC:%02X:%02X:%02X:%02X:%02X:%02X", (uint16)Mac_Addr[0], (uint16)Mac_Addr[1], (uint16)Mac_Addr[2], (uint16)Mac_Addr[3], (uint16)Mac_Addr[4], (uint16)Mac_Addr[5]);
//				Lcd_Show_String(4, 0, text, NORMAL, 21); 	   	
		}
//		else 
//		if(protocal > TCP_IP)
//		{
//			if(flag_lcd_pic == 0)
//			{
//				flag_lcd_pic = 1;
//	//		// ip address
//
//
//			 }
//			 else if(flag_lcd_pic == 1)
//			 {					
//			 	Update_AI();
//
//			 	Test[2] = 100;
//				flag_lcd_pic = 0;
//
//			 }
//		}
//		Lcd_Show_Data (4,1,count,0,1); 
//	//	Lcd_Show_Data (4,1,Test[0],0,1);  		
//		Lcd_Show_Data (4,5,Test[1],0,1);
//		Lcd_Show_Data (4,9,Test[2],0,1);  		
//		Lcd_Show_Data (4,13,Test[3],0,1);
//		Lcd_Show_Data (4,17,Test[4],0,1); 
//		Lcd_Show_Data (3,1,Test[5],0,1);  		
//		Lcd_Show_Data (3,5,Test[6],0,1);
//		Lcd_Show_Data (3,9,Test[7],0,1);  		
//		Lcd_Show_Data (3,13,Test[8],0,1);
//		Lcd_Show_Data (3,17,Test[9],0,1); 
//		Lcd_Show_Data (2,1,Test[10],0,1);
//		Lcd_Show_Data (2,5,Test[11],0,1);
//		Lcd_Show_Data (2,9,Test[12],0,1);
//		Lcd_Show_Data (2,13,Test[13],0,1);
//		Lcd_Show_Data (2,17,Test[14],0,1);
		#if 1
		//if(protocal <= TCP_IP)
		{
			if(ChangeFlash == 1)
			{  
				ChangeFlash = 0;
				refresh_flash_timer = 20;			
			}
	
			if(refresh_flash_timer)
			{
				refresh_flash_timer--;
				if(refresh_flash_timer == 0)
				{
					WriteFlash = 1;
					RELAY1_8 = (U8_T)new_output;
					DELAY_Us(5);
				  	RELAY_LATCH = 0; 
					RELAY_LATCH = 1;  		
					DI2_LATCH = 1;
					KEY_LATCH = 1;
					DI1_LATCH = 1;
				
					if( new_output & 0x100)
						RELAY_9 = 0;
					else 
						RELAY_9 = 1;
					
					/* OUTPUT10  */
					if( new_output & 0x0200)
						RELAY_10 = 0;
					else 
						RELAY_10 = 1;  				
	
					Flash_Write_Mass();	
					WriteFlash = 0;
				}
			}
	
			#endif
		}

	//	taskYIELD();
	}
}




void watchdog(void)
{
	/*software watchdog */
	#if AX_WATCHDOG_ENB
		TA = 0xAA;
		TA = 0x55;
		RWT=1;
	//	Test[0]++;
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



//#if (defined(BACDL_MSTP) || (defined(BACDL_IP)))

uint8_t far Temp_Buf[MAX_APDU] _at_ 0x42000;

uint8_t far PDUBuffer[MAX_APDU] _at_ 0x43000;
unsigned Analog_Value_Instance_To_Index(
        uint32_t object_instance);
unsigned Binary_Value_Instance_To_Index(
    uint32_t object_instance);
void Bacnet_Initial_Data(void);
void control_logic(void);


/* global variables used in this file */
static int32_t Target_Router_Network = 0;
static BACNET_ADDRESS Target_Router_Address;


void Master_Node_task(void) reentrant
{
	portTickType xDelayPeriod  = ( portTickType ) 5 / portTICK_RATE_MS; // 1000
	uint16_t pdu_len = 0;  
	uint8_t	 flash_store = 0;
	BACNET_ADDRESS far src; /* source address */

#if defined(BACDL_MSTP)
//	IntFlashReadByte(0x7fff0,&flash_store);
//	if(flash_store != 0x55)
	{
//		init_panel(); // for test now
	}
    RS485_Set_Baud_Rate(19200);
//	dlmstp_set_mac_address(255);
    dlmstp_set_max_master(127);
    dlmstp_set_max_info_frames(1);
	/* initialize datalink layer */
    dlmstp_init(NULL);
    /* initialize objects */
    Device_Init();
	/* set up our confirmed service unrecognized service handler - required! */
    apdu_set_unrecognized_service_handler_handler
        (handler_unrecognized_service);
    /* we need to handle who-is to support dynamic device binding */
	
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_IS, handler_who_is);
//   apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_HAS, handler_who_has);
    /* Set the handlers for any confirmed services that we support. */
    /* We must implement read property - it's required! */
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROPERTY,
        handler_read_property);
 //   apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROP_MULTIPLE,
  //      handler_read_property_multiple);
//    apdu_set_confirmed_handler(SERVICE_CONFIRMED_REINITIALIZE_DEVICE,
 //       handler_reinitialize_device);
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_WRITE_PROPERTY,
        handler_write_property);
    /* handle communication so we can shutup when asked */
//    apdu_set_confirmed_handler(SERVICE_CONFIRMED_DEVICE_COMMUNICATION_CONTROL,	     // dcc
 //       handler_device_communication_control);
    /* start the cyclic 1 second timer for DCC */
//    timer_interval_start_seconds(&DCC_Timer, DCC_CYCLE_SECONDS);
    /* Hello World! */
#endif     

	Send_I_Am(&Handler_Transmit_Buffer[0]);	   
	Device_Set_Object_Instance_Number(Instance);  
//	Analog_Value_Instance_To_Index(4);		
//	Binary_Value_Instance_To_Index(5);	   	
	/* setup my info */
 //   Init_Service_Handlers();
  //  address_init();
 //   dlenv_init();
  //  atexit(datalink_cleanup);
    /* configure the timeout values */
   // last_seconds = time(NULL);
  //  timeout_seconds = apdu_timeout() / 1000;
    /* send the request */
   // Send_Who_Is_Router_To_Network(&Target_Router_Address,
   //     Target_Router_Network);
	
	/* add read temperature */
	for (;;)
    {
		vTaskDelay(xDelayPeriod);
		//if(MODBUS_OR_BACNET_FUNCTION_SELECT_ON_UART0)
		{
			pdu_len = datalink_receive(&src, &PDUBuffer[0], sizeof(PDUBuffer), 0);
			
	        if (pdu_len) {
	          //  LED_NPDU_ON();
//				if(PDUBuffer[16] == 0x67 && PDUBuffer[17] == 0x00 && PDUBuffer[18] == 0x03)
//				{
//				char teststr[] = "\r\n 3: \r\n";
//				sub_send_string(teststr,10,UART0);
//			  	sub_send_string(PDUBuffer,pdu_len,UART0);
//				}
			//	if()
			//	printf("PDUBuffer: %s",PDUBuffer);	
	            npdu_handler(&src, &PDUBuffer[0], pdu_len);
	          //  LED_NPDU_OFF();
	        }
		}
    }		

}

									  
//#endif
void Read_ALL_Data(void);
void vStartDisplayTasks(U8_T uxPriority);
void Display_Initial_Data(void);


void main( void )
{
	U8_T                regisp;
	U8_T set_para;
	U8_T loop;
	U8_T flag_store;


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
	UART0_TXEN = 1;
	UART1_TXEN = 1;
//	printf("Test uart1\r\n");
	Key_Inital();
	Lcd_Initial();
	Display_Initial_Data();	
	E2prom_Initial();
	initSerial();
   	memset(WR_Roution,'\0',sizeof(STR_WR) * MAX_WR1);
	memset(AR_Roution,'\0',sizeof(STR_AR) * MAX_AR1);
	memset(ID_Config,'\0',sizeof(UN_ID) * MAX_ID);
	Comm_Tstat_Initial_Data();
//	calculate_ID_table();
	Lcd_Show_Data (4,1,1,0,1);	
	// read pic version
	loop = 0;
#if (RUNTIME_CODE_START_ADDRESS == RUNTIME_CODE_START_AT_24kH)
   #if 1
	IntFlashReadByte(0x4000,&regisp);
	IntFlashReadByte(0x5fff,& IspVer);
	IntFlashReadByte(0x70000,& set_para);
	//if(set_para == 0xff)
	{ /* initial defautl parameters*/
	//	Eeprom_Write_Cpu_Config();
	//	set_default_parameters();
	//	set_para = 0xaa;
	//	IntFlashWriteByte(0x7fff1,set_para);
	}
	#endif
#endif		

	Read_ALL_Data();   	
	protocal = 3;
	Flash_Inital();
	IntFlashReadByte(0x7fff0,&flag_store);
	if(protocal > TCP_IP)
	{	 
		BACnet_Port = UART1;
		Test[5] = protocal;
//		Station_NUM = 6;
//		Instance = 1000;
		Panel_Info.reg.instance = mGetPointWord2(Instance);
//		Panel_Info.reg.mac[0] = Station_NUM;
		Panel_Info.reg.mac[0] = IP_Addr[0];
		Panel_Info.reg.mac[1] = IP_Addr[1];
		Panel_Info.reg.mac[2] = IP_Addr[2];
		Panel_Info.reg.mac[3] = IP_Addr[3];
		Panel_Info.reg.mac[4] = 0xBA;
		Panel_Info.reg.mac[5] = 0xc0;

		Panel_Info.reg.serial_num[0] = serialNum[0];
		Panel_Info.reg.serial_num[1] = serialNum[1];
		Panel_Info.reg.serial_num[2] = serialNum[2];
		Panel_Info.reg.serial_num[3] = serialNum[3];
	
		Panel_Info.reg.modbus_addr = Modbus_address;
		Panel_Info.reg.product_type = PRODUCT_CM5;	
		if(protocal == BAC_IP)	Panel_Info.reg.panel_number = IP_Addr[3];
		else if(protocal == BAC_MSTP)
			Panel_Info.reg.panel_number	= Station_NUM;
		Bacnet_Initial_Data();
		if(flag_store == 0x55)
		{		
			Test[13] = 55;
			Flash_Read_Mass();	
		}  
	}
	initial_input_value(); 
	sTaskCreate(TCPIP_Task,"TCPIP_task",Tcpip_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, (xTaskHandle *)&xHandleTcp);
	sTaskCreate(Common_task,"Common_task",Common_STACK_SIZE, NULL, tskIDLE_PRIORITY + 10, (xTaskHandle *)&xHandleCommon);
	if(protocal <= TCP_IP)
	{	 
		sTaskCreate(Schedule_task,"schedule_task",Schedule_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, (xTaskHandle *)&xHandleSchedule);
		vStartMainSerialTasks(tskIDLE_PRIORITY + 9);
		sTaskCreate( Sampel_DI_Task, "SampleDItask", SampleDISTACK_SIZE, NULL, tskIDLE_PRIORITY + 5, &Handle_SampleDI ); 
		sTaskCreate( Update_DI_Task, "UpdateDItask", UpdateDISTACK_SIZE, NULL, tskIDLE_PRIORITY + 3, &Handle_UpdateDI ); 

	}  		
	vStartDisplayTasks(tskIDLE_PRIORITY + 3); 		
	vStartScanTask(tskIDLE_PRIORITY + 4); 	
	vStartUpdateOutputTasks(tskIDLE_PRIORITY + 5); 
	vStartKeyTasks(tskIDLE_PRIORITY + 7); 

	if(protocal > TCP_IP)
	{ 
		sTaskCreate(Master_Node_task, (const signed portCHAR * const)"Master_Node_task",
			BACnet_STACK_SIZE, NULL, tskIDLE_PRIORITY + 9, (xTaskHandle *)&xHandleMSTP);
		sTaskCreate(Bacnet_Control, (const signed portCHAR * const)"BAC_Control_task",
			portMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 5, (xTaskHandle *)&xHandleBacCon);			
	}
	/* Finally kick off the scheduler.  This function should never return. */
	vTaskStartScheduler( portUSE_PREEMPTION );

	/* Should never reach here now under control of the scheduler. */

}
/*-----------------------------------------------------------*/


