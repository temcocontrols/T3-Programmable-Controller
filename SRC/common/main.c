/* Standard includes. */

#include "main.h"

#define BACnet_STACK_SIZE	((unsigned portSHORT)1024)
#define Schedule_STACK_SIZE	((unsigned portSHORT)512)
#define TCPIP_STACK_SIZE	((unsigned portSHORT)2048)
#define USB_STACK_SIZE	((unsigned portSHORT)512)
#define COMMON_STACK_SIZE	  ((unsigned portSHORT)512)


xTaskHandle far xHandleBACnetComm; 
xTaskHandle xHandleCommon;
xTaskHandle xSoftWatchTask;
xTaskHandle xHandleTcp;
xTaskHandle far xHandleSchedule;
xTaskHandle far xHandleBacnetControl;
xTaskHandle far xHandleUSB;


U8_T far ChangeFlash = 0;
U8_T far WriteFlash = 0;

void Bacnet_Control(void) reentrant;
/*
	put E2prom data to buffer when start-up 
*/	
U8_T xdata temp[48];

void Read_ALL_Data(void)
{
	U8_T loop;
	U16_T value;
	/* base infomation */
	for(loop = 0;loop < 4;loop++)	
	E2prom_Read_Byte(EEP_SERIALNUMBER_LOWORD + loop,&Modbus.serialNum[loop]);
	E2prom_Read_Byte(EEP_HARDWARE_REV,&Modbus.hardRev);
	E2prom_Read_Byte(EEP_ADDRESS,&Modbus.address);
	E2prom_Read_Byte(EEP_BAUDRATE,&Modbus.baudrate);
	Modbus.baudrate = 1;
	if(Modbus.baudrate == 0) // 9600
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

	E2prom_Read_Byte(EEP_STATION_NUM,&Station_NUM);

//	E2prom_Read_Byte(EEP_UPDTE_STATUS,&Modbus.UPDATE_STATUS);
//	E2prom_Read_Byte(EEP_BASE_ADDRESS,&Modbus.BASE_ADRESS);
	E2prom_Read_Byte(EEP_PROTOCAL,&Modbus.protocal);
	Test[35] = Modbus.protocal;
	E2prom_Read_Byte(EEP_TCP_TYPE,&Modbus.tcp_type);

#if defined(MINI)
	E2prom_Read_Byte(EEP_MINI_TYPE,&Modbus.mini_type);

	if(Modbus.mini_type == 0xff && Modbus.mini_type == 0x00)
	{
		Modbus.mini_type = 1;
	}
	E2prom_Read_Byte(EEP_ZIGBEE_EN,&Modbus.zigbee);
#endif
//	Modbus.TCP_TYPE = 0; // for test 
	if( Modbus.tcp_type == 0)  // static ip, read ip address fromm E2prom
	{
		for(loop = 0;loop < 4;loop++)
		{
			E2prom_Read_Byte(EEP_IP + loop,&Modbus.ip_addr[3 - loop]);
			E2prom_Read_Byte(EEP_SUBNET + loop,&Modbus.subnet[3 - loop]);
		}

		for(loop = 0;loop < 6;loop++)
		{
			E2prom_Read_Byte(EEP_MAC + loop,&Modbus.mac_addr[5 - loop]);
		}
		Modbus.getway[0] = 192;
		Modbus.getway[1] = 168;
		Modbus.getway[2] = 0;
		Modbus.getway[3] = 1;	
	}
	E2prom_Read_Byte(EEP_PORT_LOW,&temp[0]);
	E2prom_Read_Byte(EEP_PORT_HIGH,&temp[1]);

	Modbus.tcp_port = 6001;//temp[1] * 256 + temp[0];

	if(Modbus.tcp_port == 0xffff)
	{
		Modbus.tcp_port = 6001;
		E2prom_Write_Byte(EEP_PORT_LOW,6001);
		E2prom_Write_Byte(EEP_PORT_HIGH,6001 >> 8);
	}	
//  	E2prom_Write_Byte(EEP_INSTANCE_LOW,1001);
//	E2prom_Write_Byte(EEP_INSTANCE_HIGH,1000 >> 8);

//	E2prom_Read_Byte(EEP_RELAY_LOW,&temp[0]);
//	E2prom_Read_Byte(EEP_RELAY_HIGH,&temp[1]); 

//	relay_value_auto = temp[1] * 256 + temp[0];
//
//	for(loop = 0;loop < 48;loop++)
//		E2prom_Read_Byte(EEP_OUTPUT1 + loop,&temp[loop]);
//	
//	for(loop = 0;loop < 12;loop++)
//	{
//		Modbus.AOUTPUT[loop] = temp[loop * 2 + 1] * 256  + (temp[2 * loop]);
//	}
//
//	E2prom_Read_Byte(EEP_AI_EN_1,&temp[0]);
//	E2prom_Read_Byte(EEP_AI_EN_2,&temp[1]); 
//	E2prom_Read_Byte(EEP_AI_EN_3,&temp[2]);
//	E2prom_Read_Byte(EEP_AI_EN_4,&temp[3]); 
//
//	AI_Enable = (temp[3] << 24) + (temp[2] << 16) + (temp[1] << 8) + temp[0];
//
//
//	E2prom_Read_Byte(EEP_DO_EN_LOW,&temp[0]);
//	E2prom_Read_Byte(EEP_DO_EN_HIGH,&temp[1]); 
//
//	DO_Enable = temp[1] * 256 + temp[0];
//
//	E2prom_Read_Byte(EEP_AO_EN_LOW,&temp[0]);
//	E2prom_Read_Byte(EEP_AO_EN_HIGH,&temp[1]); 
//
//	AO_Enable = temp[1] * 256 + temp[0];

//	for(loop = 0;loop < 34;loop++)
//	E2prom_Read_Byte(EEP_INPUT1_RANGE + loop,&Modbus.Input_Range[loop]);
	
//	for(loop = 0;loop < 7;loop++)
//	E2prom_Read_Int(EEP_SEC + loop,&Modbus.Time.all[loop]);
//	E2prom_Read_Byte(EEP_MINADDR,&Modbus.MinAddr);
//	E2prom_Read_Byte(EEP_MAXADDR,&Modbus.MaxAddr);

//	Flash_Read_Mass();
	
}


void set_default_parameters(void)
{
	U8_T loop;

//	for(loop = 0;loop < 48;loop++)
//	E2prom_Write_Byte(EEP_OUTPUT1 + loop,0);

/*	for(loop = 0;loop < 34;loop++)
	E2prom_Write_Byte(EEP_INPUT1_RANGE + loop,0);*/

	E2prom_Write_Byte(EEP_PROTOCAL,3);

	E2prom_Write_Byte(EEP_TCP_TYPE,0);

	E2prom_Write_Byte(EEP_IP, 173);
	E2prom_Write_Byte(EEP_IP + 1, 0);
	E2prom_Write_Byte(EEP_IP + 2, 168);
	E2prom_Write_Byte(EEP_IP + 3, 192);

	E2prom_Write_Byte(EEP_SUBNET, 0);
	E2prom_Write_Byte(EEP_SUBNET + 1, 255);
	E2prom_Write_Byte(EEP_SUBNET + 2, 255);
	E2prom_Write_Byte(EEP_SUBNET + 3, 255);

	E2prom_Write_Byte(EEP_PORT_LOW,6001);
	E2prom_Write_Byte(EEP_PORT_HIGH,6001 >> 8);

	E2prom_Write_Byte(EEP_INSTANCE_LOW,1005);
	E2prom_Write_Byte(EEP_INSTANCE_HIGH,1000 >> 8);

	E2prom_Write_Byte(EEP_ADDRESS,254);

//	Flash_Write_Mass();
//    IntFlashErase(ERA_RUN,0x70000);	
}



uint8_t far Temp_Buf[MAX_APDU]_at_ 0x42000; 
static uint8_t far PDUBuffer[MAX_APDU] _at_ 0x43000;
unsigned Analog_Value_Instance_To_Index( uint32_t object_instance);
unsigned Binary_Value_Instance_To_Index( uint32_t object_instance);
void init_panel(void);
void control_logic(void);
void Bacnet_Initial_Data(void);

extern uint8_t This_Station;
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
////	if(flash_store != 0x55)
//	{
//		init_panel(); // for test now
//	}
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
//	apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_I_AM, handler_i_am_bind);
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
	Analog_Value_Instance_To_Index(4);		
	Binary_Value_Instance_To_Index(5);	   	
	/* setup my info */
 //   Init_Service_Handlers();
  //  address_init();
   // dlenv_init();
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
		//	Test[17] = pdu_len;
	        if (pdu_len) {
	          //  LED_NPDU_ON();
	            npdu_handler(&src, &PDUBuffer[0], pdu_len);
	          //  LED_NPDU_OFF();
	        }
		}
    }		

}

void PIC_refresh(void);
extern char time[];

void Common_task(void) reentrant
{
	static U8_T count = 0;
	char text[20];
	portTickType xDelayPeriod = ( portTickType ) 50 / portTICK_RATE_MS;
	static U16_T refresh_flash_timer = 0;
	for (;;)
	{
		vTaskDelay(xDelayPeriod);
		Test[1]++;	
	//	BACKLIT = ~BACKLIT;		
		count++; 
		if(count == 10)
		{
			Updata_Clock();	 
			get_time_text();
			sprintf(text, "%s", time);
			Lcd_Show_String(4, 0, text, NORMAL, 21); 
			if(TcpIp_to_sub)  // resum scan task
			{
				if(count_resume_scan < 20)
				{
				   	count_resume_scan++;
				}
				else
				{
					TcpIp_to_sub = 0;
					Test[37]++;
					vTaskResume(Handle_Scan); 
					vTaskResume(Handle_ParameterOperation); 
				}
			} 
			   
           count = 0; 	   	
		}	 
		
//	Lcd_Show_Data (4,0,Test[40],0,1);
//	Lcd_Show_Data (4,5,Test[41],0,1);
//	Lcd_Show_Data (4,10,Test[42],0,1);
//	Lcd_Show_Data (4,15,Test[43],0,1);
//	Lcd_Show_Data (3,0,Test[44],0,1);
//	Lcd_Show_Data (3,5,Test[45],0,1);
//	Lcd_Show_Data (3,10,Test[46],0,1);
//	Lcd_Show_Data (1,0,Test[23],0,1);
//	Lcd_Show_Data (1,5,Test[24],0,1);
//	Lcd_Show_Data (1,10,Test[25],0,1);
//	Lcd_Show_Data (1,15,Test[26],0,1); 
//	Lcd_Show_Data (1,18,Test[27],0,1); 	   	

#if defined(MINI)
	
		PIC_refresh(); 		
#else if(defined(CM5))
		Update_AI();

#endif	
		if(ChangeFlash == 1)
		{  
			ChangeFlash = 0;
			refresh_flash_timer = 500;			
		} 
		if(refresh_flash_timer)
		{
			refresh_flash_timer--;
			if(refresh_flash_timer == 0)
			{
				WriteFlash = 1;
				Test[21]++;
				Flash_Write_Mass();	
				WriteFlash = 0;
			}
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
	#endif
}





void SoftwareWatchdog_task(void) reentrant
{	
	portTickType xDelayPeriod = ( portTickType ) 100 / portTICK_RATE_MS;
	for(;;)
	{
		vTaskDelay(xDelayPeriod);
		/* clear watch dog */
		watchdog();
	}
}



//void DELAY_Us(U16_T loop);

void Read_ALL_Data(void);




//void SD_test(void);

void main( void )
{
	U8_T regisp;
	U8_T set_para;
	U8_T loop;
	U8_T flag_store;

	U16_T baudRateDiv = 0;

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
	UART_Init(0);
	UART_Init(1);
#if  defined(MINI)
	UART_Init(2);
#endif	
	Lcd_Initial();	 
	Key_Inital();
	Display_Initial_Data();
	Comm_Tstat_Initial_Data();
	E2prom_Initial(); 
#if (RUNTIME_CODE_START_ADDRESS == RUNTIME_CODE_START_AT_24kH)
//	IntFlashReadByte(0x4000,&regisp);
	IntFlashReadByte(0x5fff,&Modbus.IspVer);
	IntFlashReadByte(0x7fff1,& set_para);
	//if(set_para == 0xff)
	{ /* initial defautl parameters*/
	//	Eeprom_Write_Cpu_Config();
		set_default_parameters();
//		set_para = 0xaa;
//		IntFlashWriteByte(0x7fff1,set_para);
	}
#endif
	Read_ALL_Data();  
	DELAY_Init();	
	Flash_Inital();	
	IntFlashReadByte(0x7fff0,&flag_store);
	if(Modbus.protocal > TCP_IP)
	{
	//		Station_NUM = 6;
	//		Instance = 1000;
		Panel_Info.reg.instance = mGetPointWord2(Instance);
	//		Panel_Info.reg.mac[0] = Station_NUM;
		Panel_Info.reg.mac[0] = Modbus.ip_addr[0];
		Panel_Info.reg.mac[1] = Modbus.ip_addr[1];
		Panel_Info.reg.mac[2] = Modbus.ip_addr[2];
		Panel_Info.reg.mac[3] = Modbus.ip_addr[3];
		Panel_Info.reg.mac[4] = 0xBA;
		Panel_Info.reg.mac[5] = 0xc0;
	
		Panel_Info.reg.serial_num[0] = Modbus.serialNum[0];
		Panel_Info.reg.serial_num[1] = Modbus.serialNum[1];
		Panel_Info.reg.serial_num[2] = Modbus.serialNum[2];
		Panel_Info.reg.serial_num[3] = Modbus.serialNum[3];
	
		Panel_Info.reg.modbus_addr = Modbus.address;
		if(Modbus.protocal == BAC_IP)	Panel_Info.reg.panel_number = Modbus.ip_addr[3];
		else if(Modbus.protocal == BAC_MSTP)
		Panel_Info.reg.panel_number	= Station_NUM;
	}
#if  defined(MINI)
	if(Modbus.mini_type == MINI_BIG)
		Panel_Info.reg.product_type = PRODUCT_MINI_BIG;	
	else
		Panel_Info.reg.product_type = PRODUCT_MINI_SMALL;
#else 
	Panel_Info.reg.product_type = PRODUCT_CM5;
#endif
	
	Bacnet_Initial_Data();
	if(flag_store == 0x55)
	{		
		Test[13] = 66;
		Flash_Read_Mass();	
	}  
	Display_IP();
	init_scan_db();	 
//	Get_Tst_DB_From_Flash();
// send mini type to top board
//	protocal = TCP_IP;
//	Modbus.zigbee = 1;
	sTaskCreate(TCPIP_Task, (const signed portCHAR * const)"TCPIP_task",
		TCPIP_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, (xTaskHandle *)&xHandleTcp);  

	sTaskCreate(Common_task, (const signed portCHAR * const)"Common_task",
		COMMON_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, (xTaskHandle *)&xHandleCommon);
	
	initSerial();		
//	vStartMainSerialTasks(tskIDLE_PRIORITY + 6);	 // main uart, rs485 or zigbee
	vStartScanTask(tskIDLE_PRIORITY + 3);
	vStartOutputTasks(tskIDLE_PRIORITY + 4);

	if(Modbus.protocal > TCP_IP)
	{  
		sTaskCreate(Master_Node_task, (const signed portCHAR * const)"Master_Node_task",
			BACnet_STACK_SIZE, NULL, tskIDLE_PRIORITY + 9, (xTaskHandle *)&xHandleBACnetComm);		  

		sTaskCreate(Bacnet_Control, (const signed portCHAR * const)"BAC_Control_task",
			portMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 8, (xTaskHandle *)&xHandleBacnetControl);			
	} 

#if  defined(MINI)
	Initial_PWM();

	/* slave select output enable, SPI master, SSO auto, SPI enable, SPI_STCFIE enable, baudrate, slave select */
	SPI_Setup(SPI_SSO_ENB|SPI_MST_SEL|SPI_SS_AUTO|SPI_ENB, SPI_STCFIE, 0x20/*0x80*/, SLAVE_SEL_1); // 25M
	Start_Comm_Top(); 

    vStartCommToTopTasks(tskIDLE_PRIORITY + 8);
	if(Modbus.mini_type == BIG)
	{
	//	vStartDisplayTasks(tskIDLE_PRIORITY + 4);
	//	vStartKeyTasks(tskIDLE_PRIORITY + 5);
	}
	sTaskCreate(USB_task, (const signed portCHAR * const)"USB_task",USB_STACK_SIZE, NULL, tskIDLE_PRIORITY + 11, (xTaskHandle *)&xHandleUSB);//8

#endif

#if defined(CM5)
//	sTaskCreate( Sampel_DI_Task, "SampleDItask", SampleDISTACK_SIZE, NULL, tskIDLE_PRIORITY + 5, &Handle_SampleDI ); 
//	sTaskCreate( Update_DI_Task, "UpdateDItask", UpdateDISTACK_SIZE, NULL, tskIDLE_PRIORITY + 3, &Handle_UpdateDI ); 
//	vStartDisplayTasks(tskIDLE_PRIORITY + 4);
//	vStartKeyTasks(tskIDLE_PRIORITY + 5);
#endif


	
	/* Finally kick off the scheduler.  This function should never return. */
	vTaskStartScheduler( portUSE_PREEMPTION );

	/* Should never reach here now under control of the scheduler. */

}
/*-----------------------------------------------------------*/


