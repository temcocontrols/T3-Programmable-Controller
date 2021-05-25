/* Standard includes. */

#include "main.h"



#define Control_STACK_SIZE	((unsigned portSHORT)2048)
#define BACnet_STACK_SIZE	((unsigned portSHORT)2048)
#define TCPIP_STACK_SIZE	((unsigned portSHORT)2028)
//#define USB_STACK_SIZE	   ((unsigned portSHORT)1024)
#define COMMON_STACK_SIZE	  ((unsigned portSHORT)256)
//#define GSM_STACK_SIZE	  ((unsigned portSHORT)512)
#define SampleDISTACK_SIZE  ((unsigned portSHORT)128)
#define SampleAISTACK_SIZE  ((unsigned portSHORT)128)
#define Monitor_STACK_SIZE	((unsigned portSHORT)1000)

void init_panel(void);
void control_logic(void);
void Bacnet_Initial_Data(void);

extern U16_T PT1K_para;


U8_T cpu_type;
U8_T current_task;
STR_Task_Test far task_test;

xTaskHandle far Handle_SampleDI;
xTaskHandle far Handle_SampleAI;
xTaskHandle far xHandleMSTP;
xTaskHandle xHandleCommon;
xTaskHandle xSoftWatchTask;
xTaskHandle xHandleTcp;
//xTaskHandle far xHandleSchedule;
xTaskHandle far xHandleBacnetControl;
xTaskHandle far xHandleUSB;
xTaskHandle far xHandleMornitor_task;
xTaskHandle far xHandleGSM;
xTaskHandle far xHandleLCD_task;
xTaskHandle far xHandleLedRefresh;

extern  uint8_t SendBuff[SENDBUFF_SIZE];  //用于DMA 串口发送
//xQueueHandle xLCDQueue;
xLCDMessage xMessage;

uint16 count_refresh_all = 0;
static uint8 backup_sub_no = 0;
static uint8 backup_current_online_ctr = 0;
static uint32 backup_ether_rx_packet = 0;
static uint32 backup_ether_tx_packet = 0;

#if (ASIX_MINI || ASIX_CM5)

#if (DEBUG_UART1)
U8_T far debug_str[200];
#endif
#endif

#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
U32_T ether_rx_packet;	 
U32_T ether_tx_packet;
extern U8_T flag_output;
extern u8 IP_Change;
#endif
//void test_alarm(uint8_t test);
void check_alarm(void);

U8_T etr_reboot;
//uint16_t pdu_len = 0;  
//BACNET_ADDRESS far src;

U8_T far ChangeFlash = 0;
//U8_T far WriteFlash = 0;
U16_T count_flash = 0;
U16_T count_write_E2 = 0;
U8_T flag_Updata_Clock;



U8_T flag_resume_rs485 = 0;  // 0 - intial , 1 - suspend rs485 task  2 - resume rs485 task
U8_T resume_rs485_count = 0;


//const unsigned int code SW_REV = 3401;
U16_T far default_pwm[10];


void Bacnet_Control(void) reentrant;

/*
	put E2prom data to buffer when start-up 
*/	

void check_flash_changed(void)
{
	if(ChangeFlash == 1)
	{  
		ChangeFlash = 0;
		
#if (ASIX_MINI || ASIX_CM5)
		if(count_flash == 0)
			count_flash = Modbus.refresh_flash_timer * 60;
		
		if(count_flash < 60) 
			count_flash = 60;  // for asix, at least 60 second 
#endif
		
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
		if(count_flash < 5) 
			count_flash = 5; // for arm, at least 5 second 
#endif
		
	} 
	
	
	if(ChangeFlash == 2)	// write into flash now
	{	
#if (ARM_MINI || ARM_TSTAT_WIFI)
		Store_Pulse_Counter(1);
#endif
		ChangeFlash = 0;	
		Flash_Write_Mass();	
#if (ASIX_MINI || ASIX_CM5)
		flag_Updata_Clock = 1;
		//Updata_Clock(0);
		// ??????????, from rev44.3
		// when wirte flash, sem_SPI is not correct?????
#if (ARM_MINI || ASIX_MINI)
		vSemaphoreCreateBinary(sem_SPI);
#endif		
		
#endif


		count_flash = 0;	
		
	}
	if(count_flash > 0)
	{
		count_flash--;
		if(count_flash == 0)
		{	
			Flash_Write_Mass();
#if (ASIX_MINI || ASIX_CM5)
			flag_Updata_Clock = 1;
			//Updata_Clock(0);
#endif
		}
	}

}



void Read_ALL_Data(void)
{	
	U8_T  temp[48];
	U8_T  loop;
//	U16_T value;
	/* base infomation */  

	memset((uint8 *)(&Modbus),0,sizeof(STR_MODBUS));

#if (ASIX_MINI || ASIX_CM5)
	IntFlashReadByte(0x5fff,&Modbus.IspVer);
#endif
	
	for(loop = 0;loop < 4;loop++)	
	E2prom_Read_Byte(EEP_SERIALNUMBER_LOWORD + loop,&Modbus.serialNum[loop]);
	
	for(loop = 0;loop < 4;loop++)
	{
		E2prom_Read_Byte(EEP_INSTANCE1 + loop,&temp[loop]);
	}

	Instance = temp[0] + (U16_T)(temp[1] << 8) + ((U32_T)temp[2] << 16) + ((U32_T)temp[3] << 24);
	if(Instance == 0xffffffff || Instance > 0x3fffff)
	{
		Instance =  Modbus.serialNum[0] + (U16_T)(Modbus.serialNum[1] << 8) + ((U32_T)Modbus.serialNum[2] << 16) + ((U32_T)Modbus.serialNum[3] << 24);
	}

	E2prom_Read_Byte(EEP_HARDWARE_REV,&Modbus.hardRev);
	
	for(loop = 0;loop < 4;loop++)
		E2prom_Read_Byte(EEP_REMOTE_SERVER1 + loop,&temp[loop]);
	
#if (ASIX_MINI || ASIX_CM5)
#if REM_CONNECTION
	RM_Conns.ServerIp = temp[0] + (U16_T)(temp[1] << 8) + ((U32_T)temp[2] << 16) + ((U32_T)temp[3] << 24);
#endif
#endif
	
	E2prom_Read_Byte(EEP_ADDRESS,&Modbus.address);
	// if it has not been changed, check the flash memory
	if(( Modbus.address == 255) || ( Modbus.address == 0) )
	{
			Modbus.address = 1;
			E2prom_Write_Byte(EEP_ADDRESS,  Modbus.address);
	}
	
	Station_NUM = Modbus.address;
//	 E2prom_Read_Byte(EEP_STATION_NUM,&Station_NUM);
//	if(Station_NUM == 0 || Station_NUM == 255)
//	{
//		Station_NUM = 1;
//		E2prom_Write_Byte(EEP_STATION_NUM,Station_NUM);
//	}


	
	E2prom_Read_Byte(EEP_EN_NODE_PLUG_N_PLAY,&Modbus.external_nodes_plug_and_play);
	if(Modbus.external_nodes_plug_and_play > 1)
		Modbus.external_nodes_plug_and_play = 0;
#if !(ARM_TSTAT_WIFI)
	E2prom_Read_Byte(EEP_OUTPUT_MODE,&flag_output);
	if(flag_output > 1)
	{
		flag_output = 0; 
		E2prom_Write_Byte(EEP_OUTPUT_MODE,0);
	}
#endif
	E2prom_Read_Byte(EEP_TCP_TYPE,&Modbus.tcp_type);
	if(Modbus.tcp_type >= 2)
	{
		Modbus.tcp_type = 0;
		E2prom_Write_Byte(EEP_TCP_TYPE, 0);	
	}
#if ARM_MINI	
	E2prom_Read_Byte(EEP_CPU_TYPE,&cpu_type);
#endif
	E2prom_Read_Byte(EEP_MINI_TYPE,&Modbus.mini_type);
	if(Modbus.mini_type == 0xff && Modbus.mini_type == 0x00)
	{
		Modbus.mini_type = 1;
	}
	

	if(Modbus.mini_type > MAX_MINI_TYPE)
		Modbus.mini_type = 1;

#if ASIX_MINI
	// ?panel 25??, ? panel 7 ??????? , ????????3.4v
	if((Modbus.mini_type == MINI_BIG) && (Modbus.hardRev >= 25)
		|| (Modbus.mini_type == MINI_SMALL) && (Modbus.hardRev >= 7) )
	{
		default_pwm[0] = 120;
		default_pwm[1] = 225;
		default_pwm[2] = 330;
		default_pwm[3] = 400;
		default_pwm[4] = 465;
		default_pwm[5] = 545;
		default_pwm[6] = 630;
		default_pwm[7] = 715;
		default_pwm[8] = 810;
		default_pwm[9] = 1000;
		
	}
	else 
	{
		default_pwm[0] = 110;
		default_pwm[1] = 220;
		default_pwm[2] = 320;
		default_pwm[3] = 410;
		default_pwm[4] = 480;
		default_pwm[5] = 560;
		default_pwm[6] = 650;
		default_pwm[7] = 740;
		default_pwm[8] = 840;
		default_pwm[9] = 980;
	}
#endif

#if ARM_MINI || ARM_TSTAT_WIFI
		default_pwm[0] = 100;
		default_pwm[1] = 200;
		default_pwm[2] = 300;
		default_pwm[3] = 400;
		default_pwm[4] = 500;
		default_pwm[5] = 600;
		default_pwm[6] = 700;
		default_pwm[7] = 800;
		default_pwm[8] = 900;
		default_pwm[9] = 1000;

#endif	
	
	
#if ARM_MINI || ARM_TSTAT_WIFI

	E2prom_Read_Byte(EEP_ISP_REV,&Modbus.IspVer);
	if(Modbus.IspVer >= 49)  // ARM BOARD,change product id to 74(new arm)if it is old product id 35(old asix)
	{
		if(Modbus.mini_type == MINI_BIG)
		{
			Modbus.mini_type = MINI_BIG_ARM;
		}
		else if(Modbus.mini_type == MINI_SMALL)
		{
			Modbus.mini_type = MINI_SMALL_ARM;
		}
		else if(Modbus.mini_type == MINI_NEW_TINY)
		{
			Modbus.mini_type = MINI_TINY_ARM;
		}			
	}
		

#endif
	
#if ARM_CM5
		E2prom_Read_Byte(EEP_ISP_REV,&Modbus.IspVer);
		Modbus.mini_type = MINI_CM5;
#endif
	

	
	// get number of DO 
	if((Modbus.mini_type == MINI_BIG) || (Modbus.mini_type == MINI_BIG_ARM))	 {	max_dos = BIG_MAX_DOS; max_aos = BIG_MAX_AOS; }
	else if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM))		{	max_dos = SMALL_MAX_DOS; max_aos = SMALL_MAX_AOS; }
	else if(Modbus.mini_type == MINI_TINY)		
	{	//max_dos = TINY_MAX_DOS;	max_aos = TINY_MAX_AOS;
		max_dos = 4;
		max_aos = 2;
		if(outputs[4].digital_analog == 0)
			max_dos++;
		else
			max_aos++;
		if(outputs[5].digital_analog == 0)
			max_dos++;	
		else
			max_aos++;		
	}
	else if(Modbus.mini_type == MINI_VAV)		{		max_dos = VAV_MAX_DOS;	max_aos = VAV_MAX_AOS; }
	else if(Modbus.mini_type == MINI_CM5)		{		max_dos = CM5_MAX_DOS;	max_aos = CM5_MAX_AOS; }
	else if((Modbus.mini_type == MINI_NEW_TINY) || (Modbus.mini_type == MINI_TINY_ARM))		{		max_dos = NEW_TINY_MAX_DOS;	max_aos = NEW_TINY_MAX_AOS; }
	else if(Modbus.mini_type == MINI_TINY_11I)		{		max_dos = TINY_11I_MAX_DOS;	max_aos = TINY_11I_MAX_AOS; }
	else if(Modbus.mini_type == MINI_TSTAT10)			{		max_dos = TSTAT10_MAX_DOS;	max_aos = TSTAT10_MAX_AOS;}
	else if(Modbus.mini_type == MINI_T10P)			{		max_dos = T10P_MAX_DOS;	max_aos = T10P_MAX_AOS;}
	else 	{	max_aos = 0; max_dos = 0;	}
	
	if(Modbus.tcp_type != 0 && Modbus.tcp_type != 1)
		Modbus.tcp_type = 0;
	//if( Modbus.tcp_type == 0)  // static ip, read ip address fromm E2prom
	{ 
		for(loop = 0;loop < 4;loop++)
		{
#if (ASIX_MINI || ASIX_CM5)
			E2prom_Read_Byte(EEP_IP + loop,&Modbus.ip_addr[3 - loop]);
			E2prom_Read_Byte(EEP_SUBNET + loop,&Modbus.subnet[3 - loop]);
			E2prom_Read_Byte(EEP_GETWAY + loop,&Modbus.getway[3 - loop]);
#endif
			
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
			E2prom_Read_Byte(EEP_IP + loop,&Modbus.ip_addr[loop]);
			E2prom_Read_Byte(EEP_SUBNET + loop,&Modbus.subnet[loop]);
			E2prom_Read_Byte(EEP_GETWAY + loop,&Modbus.getway[loop]);
			
//			own_ip[loop] = Modbus.ip_addr[loop];

#endif
		}
		

		
//printf("mini1 %u %u %u %u\r\n",Modbus.ip_addr[0],Modbus.ip_addr[1],Modbus.ip_addr[2],Modbus.ip_addr[3]);
		for(loop = 0;loop < 6;loop++)
		{
#if (ASIX_MINI || ASIX_CM5)
			E2prom_Read_Byte(EEP_MAC + loop,&Modbus.mac_addr[5 - loop]);
#endif
			
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
			E2prom_Read_Byte(EEP_MAC + loop,&Modbus.mac_addr[loop]);
#endif			
		}
		
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
			if(Modbus.mac_addr[0] != 0)
			{// inverse mac address
				char i;
				for(i = 0;i < 6;i++)
				{
					E2prom_Write_Byte(EEP_MAC, Modbus.mac_addr[5]);
					E2prom_Write_Byte(EEP_MAC + 1, Modbus.mac_addr[4]);
					E2prom_Write_Byte(EEP_MAC + 2, Modbus.mac_addr[3]);
					E2prom_Write_Byte(EEP_MAC + 3, Modbus.mac_addr[2]);
					E2prom_Write_Byte(EEP_MAC + 4, Modbus.mac_addr[1]);
					E2prom_Write_Byte(EEP_MAC + 5, Modbus.mac_addr[0]);
				}
			}
#endif
			
		if((Modbus.ip_addr[0] == 0)  && (Modbus.ip_addr[1] == 0)  && (Modbus.ip_addr[2] == 0) && (Modbus.ip_addr[3] == 0) )
		{		
			Modbus.ip_addr[0] = 192;
			Modbus.ip_addr[1] = 168;
			Modbus.ip_addr[2] = 0;
			Modbus.ip_addr[3] = 3;
		}
		
	}
	
	if(Modbus.com_config[2] == BACNET_SLAVE || Modbus.com_config[2] == BACNET_MASTER
		|| Modbus.com_config[0] == BACNET_SLAVE || Modbus.com_config[0] == BACNET_MASTER)
	{
		Send_I_Am_Flag = 1;
	}
	

	E2prom_Read_Byte(EEP_PORT_LOW,&temp[0]);
	E2prom_Read_Byte(EEP_PORT_HIGH,&temp[1]);

	Modbus.tcp_port = temp[1] * 256 + temp[0];	
	
	if(Modbus.tcp_port == 0xffff || Modbus.tcp_port == 0 )
	{
		Modbus.tcp_port = 502;
		E2prom_Write_Byte(EEP_PORT_LOW,Modbus.tcp_port);
		E2prom_Write_Byte(EEP_PORT_HIGH,Modbus.tcp_port >> 8);
	}

	E2prom_Read_Byte(EEP_BACNET_PORT_LO,&temp[0]);
	E2prom_Read_Byte(EEP_BACNET_PORT_HI,&temp[1]);
	
	Modbus.uart_parity[1] = 0;
	E2prom_Read_Byte(EEP_UART0_PARITY,&Modbus.uart_parity[0]);
	E2prom_Read_Byte(EEP_UART2_PARITY,&Modbus.uart_parity[2]);
	
	if(Modbus.uart_parity[0] > 2)
	{	
		Modbus.uart_parity[0] = 0;
		E2prom_Write_Byte(EEP_UART0_PARITY,0);
	}
	if(Modbus.uart_parity[2] > 2)
	{	
		Modbus.uart_parity[2] = 0;
		E2prom_Write_Byte(EEP_UART2_PARITY,0);
	}
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
	Modbus.uart_stopbit[1] = 0;
	E2prom_Read_Byte(EEP_UART0_STOPBIT,&Modbus.uart_stopbit[0]);
	E2prom_Read_Byte(EEP_UART2_STOPBIT,&Modbus.uart_stopbit[2]);
	if(Modbus.uart_stopbit[0] > 2)
	{	
		Modbus.uart_stopbit[0] = 0;
		E2prom_Write_Byte(EEP_UART0_STOPBIT,0);
	}
	if(Modbus.uart_stopbit[2] > 2)
	{	
		Modbus.uart_stopbit[2] = 0;
		E2prom_Write_Byte(EEP_UART2_STOPBIT,0);
	}
//	E2prom_Read_Byte(EEP_UART0_NETWORK,&Modbus.network_ID[0]);
//	E2prom_Read_Byte(EEP_UART1_NETWORK,&Modbus.network_ID[1]);
//	E2prom_Read_Byte(EEP_UART2_NETWORK,&Modbus.network_ID[2]);
#endif	

	Modbus.Bip_port = temp[1] * 256 + temp[0];
	if(Modbus.Bip_port == 0xffff || Modbus.Bip_port == 0 || Modbus.Bip_port == 255)
	{
		Modbus.Bip_port = 0XBAC0;
		E2prom_Write_Byte(EEP_BACNET_PORT_LO,Modbus.Bip_port);
		E2prom_Write_Byte(EEP_BACNET_PORT_HI,Modbus.Bip_port >> 8);
	}
#if OUTPUT_DEATMASTER	
	E2prom_Read_Byte(EEP_DEAD_MASTER,&Modbus.dead_master);
	if(Modbus.dead_master == 255)
	{
		Modbus.dead_master = 0;
	}
#endif
	
#if ARM_TSTAT_WIFI
	E2prom_Read_Byte(EEP_DISABLE_T10_DIS,&Modbus.disable_tstat10_display);
	if(Modbus.disable_tstat10_display == 255)
	{
		Modbus.disable_tstat10_display = 0;
	}	
#endif
	
	E2prom_Read_Byte(EEP_COM0_CONFIG,&Modbus.com_config[0]);
	if((Modbus.com_config[0] != NOUSE) 
		&& (Modbus.com_config[0] != MODBUS_SLAVE) 
		&& (Modbus.com_config[0] != MODBUS_MASTER)
		&& (Modbus.com_config[0] != BACNET_SLAVE) 
		&& (Modbus.com_config[0] != BACNET_MASTER))
	{
		Modbus.com_config[0] = NOUSE;
		E2prom_Write_Byte(EEP_COM0_CONFIG,0);
	}

	E2prom_Read_Byte(EEP_COM1_CONFIG,&Modbus.com_config[1]);
	if((Modbus.com_config[1] != NOUSE) && (Modbus.com_config[1] != MODBUS_SLAVE) && (Modbus.com_config[1] != MODBUS_MASTER) && (Modbus.com_config[1] != RS232_METER))
	{
		Modbus.com_config[1] = NOUSE;
		E2prom_Write_Byte(EEP_COM1_CONFIG,0);
	}
	E2prom_Read_Byte(EEP_COM2_CONFIG,&Modbus.com_config[2]);
	if((Modbus.com_config[2] != NOUSE) && (Modbus.com_config[2] != MODBUS_SLAVE) && (Modbus.com_config[2] != MODBUS_MASTER) 
		&& (Modbus.com_config[2] != BACNET_SLAVE) && (Modbus.com_config[2] != BACNET_MASTER))
	{
		Modbus.com_config[2] = NOUSE;
		E2prom_Write_Byte(EEP_COM2_CONFIG,0);
	}

	E2prom_Read_Byte(EEP_REFRESH_FLASH,&Modbus.refresh_flash_timer);

	if(Modbus.refresh_flash_timer == 255)
		Modbus.refresh_flash_timer = 5;


	if(Modbus.mini_type == MINI_CM5)
	{
		//Modbus.com_config[2] = MODBUS_MASTER;
		Modbus.com_config[1] = NOUSE;
		if((Modbus.com_config[0] != NOUSE) && (Modbus.com_config[0] != MODBUS_SLAVE) && (Modbus.com_config[0] != BACNET_SLAVE))
		{
			Modbus.com_config[0] = NOUSE;
			E2prom_Write_Byte(EEP_COM2_CONFIG,0);
		}

		if((Modbus.com_config[2] != NOUSE) && (Modbus.com_config[2] != MODBUS_SLAVE) && (Modbus.com_config[2] != MODBUS_MASTER) 
		&& (Modbus.com_config[2] != BACNET_SLAVE) && (Modbus.com_config[2] != BACNET_MASTER))
		{
			Modbus.com_config[2] = NOUSE;
			E2prom_Write_Byte(EEP_COM2_CONFIG,0);
		}
		
//		Modbus.main_port = 2;
//		Modbus.sub_port = 0;

		uart0_baudrate = UART_19200;
		uart1_baudrate = 0;
		E2prom_Read_Byte(EEP_UART2_BAUDRATE,&uart2_baudrate);
		if(uart2_baudrate == 255)
		{
			uart2_baudrate = UART_19200;
			E2prom_Write_Byte(EEP_UART2_BAUDRATE,uart2_baudrate);	
		}
		
		UART_Init(0);
		UART_Init(2);
	}
	else
	{
		if(Modbus.com_config[0] == 255)
			Modbus.com_config[0] = MODBUS_SLAVE;
		if(Modbus.com_config[1] == 255)
			Modbus.com_config[1] = NOUSE;
		if(Modbus.com_config[2] == 255)
			Modbus.com_config[2] = MODBUS_MASTER;
		 
		if(Modbus.com_config[1] == MODBUS_MASTER)
			count_send_id_to_zigbee = NOUSE;

#if ARM_MINI
	if((Modbus.mini_type == MINI_BIG_ARM) || (Modbus.mini_type == MINI_SMALL_ARM))
	{
		if((Modbus.com_config[1] == MODBUS_MASTER) || (Modbus.com_config[1] == NOUSE))
			UART1_SW = 1;
		else
			UART1_SW = 0;
	}
#endif
	
#if ARM_TSTAT_WIFI
			E2prom_Read_Byte(EEP_COM0_CONFIG,&Modbus.com_config[0]);
			if(Modbus.com_config[0] == 0xff || Modbus.com_config[0] == 0)
				Modbus.com_config[0] = MODBUS_SLAVE;
			
			if((Modbus.com_config[0] != NOUSE) 
				&& (Modbus.com_config[0] != MODBUS_SLAVE) 
				&& (Modbus.com_config[0] != MODBUS_MASTER)
				&& (Modbus.com_config[0] != BACNET_SLAVE) 
				&& (Modbus.com_config[0] != BACNET_MASTER))
			{
				Modbus.com_config[0] = NOUSE;
				E2prom_Write_Byte(EEP_COM0_CONFIG,0);
			}
			
			E2prom_Read_Byte(EEP_UART0_BAUDRATE,&uart0_baudrate);  
			if(uart0_baudrate == 255)
			{
				uart0_baudrate = UART_115200;
				E2prom_Write_Byte(EEP_UART0_BAUDRATE,uart0_baudrate);	
			}
//			Modbus.com_config[0] = MODBUS_SLAVE;
			Modbus.com_config[1] = NOUSE;
			Modbus.com_config[2] = NOUSE;
	
			//uart0_baudrate = UART_115200;
			uart1_baudrate = 0;
			uart2_baudrate = 0;
			

#endif		
	}	

	
#if (ARM_MINI || ASIX_MINI || ARM_TSTAT_WIFI)
	Modbus.start_adc[0] = 0;
  for(loop = 0;loop <= 9;loop++)
	{
		
#if (ARM_MINI || ARM_TSTAT_WIFI)
		E2prom_Read_Byte(EEP_OUT_1V + loop * 2,&temp[loop * 2]);
		E2prom_Read_Byte(EEP_OUT_1V + loop * 2 + 1,&temp[loop * 2 + 1]);
		
		Modbus.start_adc[1 + loop] = (U16_T)temp[loop * 2] * 256 + temp[loop * 2 + 1]; 
		
		if((Modbus.start_adc[1 + loop] == 0xffff) || (Modbus.start_adc[1 + loop] == 0))
		{
			E2prom_Write_Byte(EEP_OUT_1V + loop * 2,default_pwm[loop] / 256);
			E2prom_Write_Byte(EEP_OUT_1V + loop * 2 + 1,default_pwm[loop] % 256);
			Modbus.start_adc[1 + loop] = default_pwm[loop];
		}
#endif
		
#if ASIX_MINI
		E2prom_Read_Byte(EEP_OUT_1V + loop,&temp[loop]);
		Modbus.start_adc[1 + loop] = temp[loop] * 10; 
		if(temp[loop] == 255)
		{
			E2prom_Write_Byte(EEP_OUT_1V + loop,default_pwm[loop] / 10);
			Modbus.start_adc[1 + loop] = default_pwm[loop];
		}
#endif
	}

	cal_slop();
#endif

	E2prom_Read_Byte(EEP_USER_NAME,&Modbus.en_username);
	E2prom_Read_Byte(EEP_CUS_UNIT,&Modbus.cus_unit);

	Modbus.usb_mode = 0;

#if !(ARM_TSTAT_WIFI)	
	E2prom_Read_Byte(EEP_EN_DYNDNS,&Modbus.en_dyndns);
	if(Modbus.en_dyndns == 255)
	{
		Modbus.en_dyndns = 1; 
		E2prom_Write_Byte(EEP_EN_DYNDNS,1);	
	}

	E2prom_Read_Byte(EEP_DYNDNS_PROVIDER,&dyndns_provider);
	if(dyndns_provider > 3)
	{
		dyndns_provider = 3; 
		E2prom_Write_Byte(EEP_DYNDNS_PROVIDER,3);	
	}

	E2prom_Read_Byte(EEP_DYNDNS_UPDATE_LO,&temp[0]);
	E2prom_Read_Byte(EEP_DYNDNS_UPDATE_HI,&temp[1]);
	dyndns_update_time = temp[1] * 256 + temp[0];
	if(dyndns_update_time == 65535)
	{
		dyndns_update_time = 10; 
		E2prom_Write_Byte(EEP_DYNDNS_UPDATE_LO,10);	
		E2prom_Write_Byte(EEP_DYNDNS_UPDATE_HI,0);	
	}
	
	E2prom_Read_Byte(EEP_UART2_BAUDRATE,&uart2_baudrate);
	if(uart2_baudrate == 255)
	{
		uart2_baudrate = UART_19200;
		E2prom_Write_Byte(EEP_UART2_BAUDRATE,uart2_baudrate);	
	}
	E2prom_Read_Byte(EEP_UART0_BAUDRATE,&uart0_baudrate);  
	if(uart0_baudrate == 255)
	{
		uart0_baudrate = UART_19200;
		E2prom_Write_Byte(EEP_UART0_BAUDRATE,uart0_baudrate);	
	}	
	E2prom_Read_Byte(EEP_UART1_BAUDRATE,&uart1_baudrate);
	if(uart1_baudrate == 255)
	{
		uart1_baudrate = UART_19200;
		E2prom_Write_Byte(EEP_UART1_BAUDRATE,uart1_baudrate);	
	} 	
	UART_Init(1);
	UART_Init(2);

	
#endif
	
#if !(ARM_UART_DEBUG)
	UART_Init(0);
#endif



	if(Modbus.mini_type == MINI_VAV)
	{
			UART_Init(1);   // control VAV board by UART1
		//Modbus.com_config[0] = 2;
		Modbus.com_config[1] = RS232_METER;
		//Modbus.com_config[2] = 2;
	}
	
#if (ASIX_MINI || ASIX_CM5)	
	PS0 = 1;
	PS1 = 1;
	PINT3 = 1;


#endif
	
//#if ARM_CM5
//	Modbus.com_config[0] = 2;
//	Modbus.com_config[1] = 0;
//	Modbus.com_config[2] = 7;
//#endif
	for(loop = 0;loop < 12;loop++)
	{
		E2prom_Read_Byte(EEP_SD_BLOCK_HI1 + loop,&temp[24 + loop]);
		if(temp[24 + loop] == 255)
		{
			E2prom_Write_Byte(EEP_SD_BLOCK_HI1 + loop,0);
			temp[24 + loop] = 0;
		}
	}
	
	for(loop = 0;loop < 12;loop++)
	{
		E2prom_Read_Byte(EEP_SD_BLOCK_A1 + loop * 2,&temp[0]);
		E2prom_Read_Byte(EEP_SD_BLOCK_A1 + loop * 2 + 1,&temp[1]);
		
		
		if(temp[0] * 256 + temp[1] == 65535)
		{
			SD_block_num[loop * 2] = 0;
			E2prom_Write_Byte(EEP_SD_BLOCK_A1 + loop * 2,0);
			E2prom_Write_Byte(EEP_SD_BLOCK_A1 + loop * 2 + 1,0);
		}
		else
			SD_block_num[loop * 2] = temp[0] * 256 + temp[1];
		
		SD_block_num[loop * 2] += 65536L * (temp[24 + loop] & 0x0f);
		
	}
	for(loop = 0;loop < 12;loop++)
	{
		E2prom_Read_Byte(EEP_SD_BLOCK_D1 + loop * 2,&temp[0]);
		E2prom_Read_Byte(EEP_SD_BLOCK_D1 + loop * 2 + 1,&temp[1]);
		SD_block_num[loop * 2 + 1] = temp[0] * 256 + temp[1];
		
		if(temp[0] * 256 + temp[1] == 65535)
		{
			SD_block_num[loop * 2 + 1] = 0;
			E2prom_Write_Byte(EEP_SD_BLOCK_D1 + loop * 2,0);
			E2prom_Write_Byte(EEP_SD_BLOCK_D1 + loop * 2 + 1,0);
		}
		else
			SD_block_num[loop * 2 + 1] = temp[0] * 256 + temp[1];
		
		SD_block_num[loop * 2 + 1] += 65536L * (temp[24 + loop] >> 4);
	}
	



	E2prom_Read_Byte(EEP_TIME_ZONE_LO,&temp[0]);
	E2prom_Read_Byte(EEP_TIME_ZONE_HI,&temp[1]);
	if(temp[0] * 256 + temp[1] == 65535)
	{		
		timezone = 800;
		E2prom_Write_Byte(EEP_TIME_ZONE_LO,timezone);
		E2prom_Write_Byte(EEP_TIME_ZONE_HI,timezone >> 8);
	}
	
	E2prom_Read_Byte(EEP_DAYLIGHT_SAVING_TIME,&Daylight_Saving_Time);
	if(Daylight_Saving_Time > 1)
	{
		Daylight_Saving_Time = 0;
		E2prom_Write_Byte(EEP_DAYLIGHT_SAVING_TIME,0);	
	}
	
 	E2prom_Read_Byte(EEP_EN_SNTP,&Modbus.en_sntp);	
	if(Modbus.en_sntp == 255)
	{
		Modbus.en_sntp = 1; 
		E2prom_Write_Byte(EEP_EN_SNTP,1);	
	}	
//	Modbus.en_sntp = 2;	
//	timezone = 800;	//202.120.2.101 24.56.178.101


	if(Modbus.en_sntp > 5)  
	{  // 5 - time server defined by customer 
			Modbus.en_sntp = 2;
	}
		
 
	
	 E2prom_Read_Byte(EEP_PANEL_NUMBER,&panel_number);
	if(panel_number == 0 || panel_number == 255)
	{
#if !(ARM_TSTAT_WIFI)	
		if(Modbus.ip_addr[3] != 0)
		{
			panel_number = Modbus.ip_addr[3];
		}
		else
#endif
			panel_number = 1;
		
		E2prom_Write_Byte(EEP_PANEL_NUMBER,panel_number);
	}
 
//  E2prom_Read_Byte(EEP_NETWORK,&Modbus.network_number);
//	if(Modbus.network_number == 0 || Modbus.network_number > 32)
//	{
//		Modbus.network_number = 1;
//		E2prom_Write_Byte(EEP_NETWORK,Modbus.network_number);
//	}	
	
	E2prom_Read_Byte(EEP_MSTP_NETWORK_LO,&temp[0]);
	E2prom_Read_Byte(EEP_MSTP_NETWORK_HI,&temp[1]);
	mstp_network = temp[1] * 256 + temp[0];
	if(mstp_network == 0 || mstp_network == 0xffff)
	{
		mstp_network = 1;
		E2prom_Write_Byte(EEP_MSTP_NETWORK_LO,mstp_network);
		E2prom_Write_Byte(EEP_MSTP_NETWORK_HI,mstp_network >> 8);
	}	
	
	
	E2prom_Read_Byte(EEP_BBMD_EN,&bbmd_en);
	if(bbmd_en == 255)
	{
		bbmd_en = 0;
		E2prom_Write_Byte(EEP_BBMD_EN,bbmd_en);
	}	
	

	
	E2prom_Read_Byte(EEP_LCD_TIME_OFF_DELAY,&Modbus.LCD_time_off_delay);
//	if(Modbus.LCD_time_off_delay == 255)  
//	{
//		Modbus.LCD_time_off_delay = 30;
//		E2prom_Write_Byte(EEP_LCD_TIME_OFF_DELAY,Modbus.LCD_time_off_delay);
//	}	
	
	E2prom_Read_Byte(EEP_EN_TIME_SYNC_PC,&Modbus.en_time_sync_with_pc);
	if(Modbus.en_time_sync_with_pc == 255)
	{
		Modbus.en_time_sync_with_pc = 0;
		E2prom_Write_Byte(EEP_EN_TIME_SYNC_PC,Modbus.en_time_sync_with_pc);
	}		
	
//	E2prom_Read_Byte(EEP_NETWORK_MASTER,&Modbus.network_master);
//	if(Modbus.network_master == 255)
//	{
//		Modbus.network_master = 0;
//		E2prom_Write_Byte(EEP_NETWORK_MASTER,Modbus.network_master);
//	}
	
	for(loop = 0;loop < 4;loop++)
	{
		E2prom_Read_Byte(EEP_SNTP_TIME1 + loop,&temp[loop]);
	}
	update_sntp_last_time = temp[0] + (U16_T)(temp[1] << 8) + ((U32_T)temp[2] << 16) + ((U32_T)temp[3] << 24);

#if ARM_MINI	
	E2prom_Read_Byte(EEP_BAC_VENDOR_ID_LO,&temp[0]);
	E2prom_Read_Byte(EEP_BAC_VENDOR_ID_HI,&temp[1]);
	Bacnet_Vendor_ID = temp[0] + temp[1] * 256;
#endif	
	
	E2prom_Read_Byte(EEP_VCC_ADC_LO,&temp[0]);
	E2prom_Read_Byte(EEP_VCC_ADC_HI,&temp[1]);
	Modbus.vcc_adc = temp[0] + (U16_T)(temp[1] << 8);
	if(Modbus.vcc_adc == 0xffff || Modbus.vcc_adc == 0)
	{		
		Modbus.vcc_adc = 1023;
		E2prom_Write_Byte(EEP_VCC_ADC_LO,Modbus.vcc_adc);
		E2prom_Write_Byte(EEP_VCC_ADC_HI,Modbus.vcc_adc >> 8);
	}
#if ARM_MINI	
	E2prom_Read_Byte(EEP_PT1K_PARA_LO,&temp[0]);
	E2prom_Read_Byte(EEP_PT1K_PARA_HI,&temp[1]);
	PT1K_para = temp[0] + (U16_T)(temp[1] << 8);
	if(PT1K_para == 0xffff)
	{		
		PT1K_para = 10000;
		E2prom_Write_Byte(EEP_PT1K_PARA_LO,PT1K_para);
		E2prom_Write_Byte(EEP_PT1K_PARA_HI,PT1K_para >> 8);
	}
#endif	
	E2prom_Read_Byte(EEP_FIX_COM_CONFIG,&Modbus.fix_com_config);
	if(Modbus.fix_com_config == 0xff)
	{
		Modbus.fix_com_config = 0;
		E2prom_Write_Byte(EEP_FIX_COM_CONFIG,Modbus.fix_com_config);
	}
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
	E2prom_Read_Byte(EEP_MAX_MASTER,&MAX_MASTER);
	if(MAX_MASTER == 0xff || MAX_MASTER <= 1)
	{
		MAX_MASTER = 254;
		E2prom_Write_Byte(EEP_MAX_MASTER,MAX_MASTER);
	}
	
	E2prom_Read_Byte(EEP_EX_MOUDLE_EN,&ex_moudle.enable);
	if((ex_moudle.enable >= 0x55) && (ex_moudle.enable <= 0x65))
	{		
		E2prom_Read_Byte(EEP_EX_MOUDLE_FLAG1,&temp[0]);
		E2prom_Read_Byte(EEP_EX_MOUDLE_FLAG2,&temp[1]);
		E2prom_Read_Byte(EEP_EX_MOUDLE_FLAG3,&temp[2]);
		E2prom_Read_Byte(EEP_EX_MOUDLE_FLAG4,&temp[3]);
		
		ex_moudle.flag = temp[0] + (U16_T)(temp[1] << 8) + ((U32_T)temp[2] << 16) + ((U32_T)temp[3] << 24);

		if(ex_moudle.flag == 0xffff)
		{
			ex_moudle.flag = 0;
			E2prom_Write_Byte(EEP_EX_MOUDLE_FLAG1,0);
			E2prom_Write_Byte(EEP_EX_MOUDLE_FLAG2,0);
			E2prom_Write_Byte(EEP_EX_MOUDLE_FLAG3,0);
			E2prom_Write_Byte(EEP_EX_MOUDLE_FLAG4,0);
			
		}
	}
    
#endif
	
	
	E2prom_Read_Byte(EEP_VRESION_HI,&temp[0]);
	E2prom_Read_Byte(EEP_VERSION_LO,&temp[1]);

	if(temp[1] + (U16_T)(temp[0] * 100) != SW_REV)
	{
		E2prom_Write_Byte(EEP_VRESION_HI,SW_REV / 100);
		E2prom_Write_Byte(EEP_VERSION_LO,SW_REV % 100);
	}
#if ARM_MINI	
	E2prom_Read_Byte(EEP_DLS_START_MON,&Modbus.start_month);
	if(Modbus.start_month == 0xff || Modbus.start_month == 0x00)
	{
		Modbus.start_month = 3;
	}
	E2prom_Read_Byte(EEP_DLS_START_DAY,&Modbus.start_day);
	if(Modbus.start_day == 0xff || Modbus.start_day == 0x00)
	{
		Modbus.start_day = 14;
	}
	E2prom_Read_Byte(EEP_DLS_END_MON,&Modbus.end_month);
	if(Modbus.end_month == 0xff || Modbus.start_month == 0x00)
	{
		Modbus.end_month = 11;
	}
	E2prom_Read_Byte(EEP_DLS_END_DAY,&Modbus.end_day);
	if(Modbus.end_day == 0xff || Modbus.end_day == 0x00)
	{
		Modbus.end_day = 7;
	}
	Calculate_DSL_Time();
#endif
	
	E2prom_Read_Byte(EEP_ETR_REBOOT,&temp[0]);
	Test[49] = temp[0];
}


void set_default_parameters(void)
{
	char loop;
	E2prom_Write_Byte(EEP_PORT_LOW,502);
	E2prom_Write_Byte(EEP_PORT_HIGH,502 >> 8);

	E2prom_Write_Byte(EEP_ADDRESS,1);
	E2prom_Write_Byte(EEP_EN_NODE_PLUG_N_PLAY,1);

	E2prom_Write_Byte(EEP_UART0_PARITY,0);
	E2prom_Write_Byte(EEP_UART2_PARITY,0);
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
	E2prom_Write_Byte(EEP_UART0_STOPBIT,0);
	E2prom_Write_Byte(EEP_UART2_STOPBIT,0);
#endif
	
#if (ARM_MINI || ASIX_MINI)
//	E2prom_Write_Byte(EEP_COM0_CONFIG, MODBUS_MASTER);
//	E2prom_Write_Byte(EEP_COM1_CONFIG, 0 );
//	E2prom_Write_Byte(EEP_COM2_CONFIG, SUB_MODBUS);

//	for(loop = 0;loop < 10;loop++)
//	{
//#if (ASIX_MINI || ASIX_CM5)
//		E2prom_Write_Byte(EEP_OUT_1V + loop,default_pwm[loop] / 10);
//#endif
//		
//#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
//		E2prom_Write_Byte(EEP_OUT_1V + loop * 2,default_pwm[loop] / 256);
//		E2prom_Write_Byte(EEP_OUT_1V + loop * 2 + 1,default_pwm[loop] % 256);
//#endif
//	}
	
	E2prom_Write_Byte(EEP_USER_NAME,0);
	E2prom_Write_Byte(EEP_CUS_UNIT,0);
//	E2prom_Write_Byte(EEP_USB_MODE,0);

// clear SD_NUMBER
	for(loop = 0;loop < 12;loop++)
	{
		E2prom_Write_Byte(EEP_SD_BLOCK_HI1 + loop,0);
		E2prom_Write_Byte(EEP_SD_BLOCK_A1 + loop * 2,0);
		E2prom_Write_Byte(EEP_SD_BLOCK_A1 + loop * 2 + 1,0);
		E2prom_Write_Byte(EEP_SD_BLOCK_D1 + loop * 2,0);
		E2prom_Write_Byte(EEP_SD_BLOCK_D1 + loop * 2 + 1,0);
	}
	
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
	E2prom_Write_Byte(EEP_REFRESH_FLASH, 0 );	  // 5min
#endif
	
#if ARM_TSTAT_WIFI
	E2prom_Write_Byte(EEP_DISABLE_T10_DIS, 0 );	
#endif
	
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
	E2prom_Write_Byte(EEP_MAX_MASTER,254);
	E2prom_Write_Byte(EEP_DEAD_MASTER,0);
#endif	
	
#endif

	
#if (ASIX_MINI || ASIX_CM5)
	
	E2prom_Write_Byte(EEP_REFRESH_FLASH, 5 );	  // 5min
	
//	E2prom_Write_Byte(EEP_NO_USED80,++Test[41]);
	IntFlashErase(ERA_RUN,0x70000);	
	IntFlashWriteByte(0x4001,0);					
	AX11000_SoftReboot();  
//	Test[41] = 4;
#endif 
	
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
// erase all flash for user	
	Bacnet_Initial_Data();	
	
	__disable_irq();
	STMFLASH_Unlock();
					
	for(loop = 0;loop < 64;loop++)
	{
		STMFLASH_ErasePage(0x8060000 + 2048 * loop);	
	}
	
	STMFLASH_Lock();
	__enable_irq();

    for (loop = 0; loop < 26; loop++)
    {
        write_page_en[loop] = 1;
    }
    ChangeFlash = 2;

  flag_reboot = 1;
	
#endif

}



uint8_t far PDUBuffer[MAX_APDU];
void Trend_Log_Init(void);
//void Schedule_Init(void);

static void Init_Service_Handlers(
    void)
{
    //Device_Init(NULL);
    /* we need to handle who-is
       to support dynamic device binding to us */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_IS, handler_who_is);
    /* handle i-am to support binding to other devices */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_I_AM, handler_i_am_add);
    /* set the handler for all the services we don't implement
       It is required to send the proper reject message... */
    apdu_set_unrecognized_service_handler_handler
        (handler_unrecognized_service);
    /* we must implement read property - it's required! */
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROPERTY,
        handler_read_property);
		apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROP_MULTIPLE,
					handler_read_property_multiple);
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_WRITE_PROPERTY,
        handler_write_property);
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_WRITE_PROP_MULTIPLE,
        handler_write_property_multiple);
//		apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_RANGE,
//        handler_read_range);
//		Test[10] = 100;
}


void Inital_Bacnet_Server(void)
{
	if(((panelname[0] == 0) && (panelname[1] == 0) && (panelname[2] == 0))  || 
		((panelname[0] == 255) && (panelname[1] == 255) && (panelname[2] == 255)) )
	{
#if ARM_MINI || ASIX_MINI
		if((Modbus.mini_type == MINI_BIG) || (Modbus.mini_type == MINI_BIG_ARM))
			Set_Object_Name("T3-BB");
		else if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM))
			Set_Object_Name("T3-LB");
		if((Modbus.mini_type == MINI_TINY) || (Modbus.mini_type == MINI_TINY_ARM)|| (Modbus.mini_type == MINI_TINY_ARM)) 
			Set_Object_Name("T3-TB");
		if(Modbus.mini_type == MINI_NANO) 
			Set_Object_Name("T3-NB");
#endif
		
#if ARM_CM5
		Set_Object_Name("BC Controller");
#endif
		
#if  ARM_TSTAT_WIFI
		if(Modbus.mini_type == MINI_TSTAT10) 
			Set_Object_Name("Tstat10");
		else if(Modbus.mini_type == MINI_T10P) 
			Set_Object_Name("T3-OEM");
			
#endif

	}
	else
		Set_Object_Name(panelname);
	Device_Init();
	Init_Service_Handlers();
	Device_Set_Object_Instance_Number(Instance);  

	address_init();
#if !(ARM_TSTAT_WIFI )
	bip_set_broadcast_addr(0xffffffff);
#endif

	if(bbmd_en == 1)
		bvlc_intial();
#if  BAC_COMMON   



#if BAC_SCHEDULE
	SCHEDULES = 8;
#endif
	
	
#if BAC_CALENDAR
	CALENDARS = 4;
#endif
	
#if BAC_TRENDLOG
	TRENDLOGS = 8;
	Trend_Log_Init();
#endif

#if BAC_PROPRIETARY
	TemcoVars = 5;
	// add initial code
#endif

#if BAC_MSV
#if 1//ARM_TSTAT_WIFI
		MSVS = 3;
#endif
#endif	

	Count_IN_Object_Number();
	Count_OUT_Object_Number();
	Count_VAR_Object_Number();
	
#endif	
}

#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
//uint16_t count_send_whois = 0;
extern uint8_t count_hold_on_bip_to_mstp;
bool dcc_communication_initial_disabled(void);
int Send_private_scan(U8_T index);
uint8_t flag_suspend_mstp;
uint16_t count_suspend_mstp;
void Master_Node_task(void) reentrant
{
	portTickType xDelayPeriod  = ( portTickType ) 10 / portTICK_RATE_MS; // 1000
	uint16_t pdu_len = 0;  
	BACNET_ADDRESS far src; /* source address */
	static uint16_t count_start_task = 0;
//	U8_T remote_bacnet_index;
	uint16_t Max_count_Mstp = 3000;
	U8_T i;
	int invoke;
	
	/* initialize datalink layer */	   	
	task_test.enable[7] = 1;
  dlmstp_init(NULL);
//	dcc_communication_initial_disabled();
	remote_bacnet_index = 0;
	pdu_len = 0;
	if(Modbus.com_config[2] == BACNET_SLAVE || Modbus.com_config[2] == BACNET_MASTER)
	{
		Recievebuf_Initialize(2);
		Send_I_Am_Flag = 1;
	}
	else if(Modbus.com_config[0] == BACNET_SLAVE || Modbus.com_config[0] == BACNET_MASTER)
	{
		Recievebuf_Initialize(0);
		Send_I_Am_Flag = 1;
	}
	
	count_start_task = 0;
//	count_send_whois = 0;
	if(Modbus.com_config[2] == BACNET_MASTER || Modbus.com_config[0] == BACNET_MASTER)
		Send_Whois_Flag = 1;	
	flag_receive_rmbp = 0;
	
	count_hold_on_bip_to_mstp = 0;
// UART2 have higher priority
	flag_suspend_mstp = 0;
	count_suspend_mstp = 0;
	for(;;)
	{
		task_test.count[7]++; 
		current_task = 7;
		if(Modbus.com_config[0] == BACNET_MASTER || Modbus.com_config[0] == BACNET_SLAVE || Modbus.com_config[2] == BACNET_MASTER || Modbus.com_config[2] == BACNET_SLAVE)
		{
//			count_send_whois++;
			vTaskDelay(5 / portTICK_RATE_MS);

			if(count_start_task % 12000 == 0)	// 1 min
			{
				if(Modbus.com_config[2] == BACNET_MASTER || Modbus.com_config[0] == BACNET_MASTER)
					Send_Whois_Flag = 1;	
				count_start_task = 0;
			}
			else
			{  // whether exist remote mstp point
				if(Modbus.com_config[2] == BACNET_MASTER || Modbus.com_config[0] == BACNET_MASTER)
				{
					if(count_start_task % 200 == 0) // 1.5s
					{
						// check whether the device is online or offline
						if(flag_receive_rmbp == 1)
						{
							U8_T remote_panel_index;
							if(Get_rmp_index_by_panel(remote_points_list_bacnet[remote_bacnet_index].point.panel,
							remote_points_list_bacnet[remote_bacnet_index].point.sub_id, 
							&remote_panel_index,
							BAC_MSTP) != -1)
							{
								remote_panel_db[remote_panel_index].time_to_live = RMP_TIME_TO_LIVE;
							}
							remote_points_list_bacnet[remote_bacnet_index].lose_count = 0;
							remote_points_list_bacnet[remote_bacnet_index].decomisioned = 1;	
						}
						else
						{
							remote_points_list_bacnet[remote_bacnet_index].lose_count++;
							if(remote_points_list_bacnet[remote_bacnet_index].lose_count > 10)
							{
								remote_points_list_bacnet[remote_bacnet_index].lose_count = 0;
								remote_points_list_bacnet[remote_bacnet_index].decomisioned = 0;	
							}
						}					
						
					// read remote mstp points
						if(remote_bacnet_index < number_of_remote_points_bacnet)
						{							
							remote_bacnet_index++;	
						}
						else
						{
							remote_bacnet_index = 0;							
						}	
						
						if(remote_bacnet_index == number_of_remote_points_bacnet)
						{  // read private modbus from Temco product
#if (ARM_MINI || ARM_CM5)
							static char j = 0;
							char count;
							
							if(j < remote_panel_num)//for(j = 0;j < remote_panel_num;j++)
							{
								if(remote_panel_db[j].protocal == BAC_MSTP 
									&& remote_panel_db[j].sn == 0)
								{
									remote_panel_db[j].retry_reading_panel++;
									flag_receive_rmbp = 0;									
									invoke = Send_private_scan(j);
									remote_mstp_panel_index = j;
									while((flag_receive_rmbp == 0) && count++ < 20)
										delay_ms(200);									
								}
								if(remote_panel_db[j].retry_reading_panel > 5)
								{
									remote_panel_db[j].sn = remote_panel_db[j].device_id;
									remote_panel_db[j].retry_reading_panel = 0;
									remote_panel_db[j].product_model = 0;
								}								
							}
							j++;
							
							if(j > remote_panel_num) 
								j = 0;	
							
							
#endif
						}
						else
						{
							if(number_of_remote_points_bacnet > 0)
							{
								// read remote bacnet point
								if(Modbus.com_config[2] == BACNET_MASTER || Modbus.com_config[0] == BACNET_MASTER)
								{
									flag_receive_rmbp = 0;
									invoke	= GetRemotePoint(remote_points_list_bacnet[remote_bacnet_index].tb.RP_bacnet.object,
												remote_points_list_bacnet[remote_bacnet_index].tb.RP_bacnet.instance,
												panel_number,/*Modbus.network_ID[2],*/
												remote_points_list_bacnet[remote_bacnet_index].tb.RP_bacnet.panel ,
												BAC_MSTP);
									// check whether the device is online or offline	
									
									if(invoke >= 0)
									{
										remote_points_list_bacnet[remote_bacnet_index].invoked_id	= invoke;
									}
									else
									{
										remote_points_list_bacnet[remote_bacnet_index].lose_count++;								
									}
								}
							}
						}
					}
				}					
			}		
			
			count_start_task++;
			if(flag_suspend_mstp == 0)
			{			
				pdu_len = datalink_receive(&src, &PDUBuffer[0], sizeof(PDUBuffer), 0,BAC_MSTP);
				{ 								
					if(pdu_len) 
					{
						npdu_handler(&src, &PDUBuffer[0], pdu_len,BAC_MSTP);	
					} 					
				}			
			}
		}
		else
		{
			delay_ms(5000);
		}				
  }	
}

#endif

void TCP_IP_Init(void);
	
void store_buffer_to_SD(void);
//void RM_send_information(void);


U32_T count_sntp = 0;
U8_T flag_Update_Sntp;
U8_T Update_Sntp_Retry;
U8_T flag_Update_Dyndns;
U8_T Update_Dyndns_Retry;

#define MAX_DDNS_RETRY_COUNT 5
void Common_task(void) reentrant
{
	static U8_T count = 0;
//	char text[20];		
	portTickType xDelayPeriod = ( portTickType ) 250 / portTICK_RATE_MS;
	U8_T i = 0;
	
	backup_sub_no = 0;
  backup_current_online_ctr = 0;
	backup_ether_rx_packet = 0;
	backup_ether_tx_packet = 0;
	count_refresh_all = 0;
	flag_Update_Sntp = 0;
	Update_Sntp_Retry = 0;
	count_sntp = 0;

//	portTickType xLastWakeTime = xTaskGetTickCount();
	task_test.enable[1] = 1;
#if (ASIX_MINI || ASIX_CM5)	
	count_write_E2 = 0;
#endif
	count_flash = 0;
	ChangeFlash = 0;
	for (;;)
	{
		vTaskDelay(250 / portTICK_RATE_MS);
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
		if(task_test.count[0] == 0)  // if tcptask is not running, need watchdog in lower task
			IWDG_ReloadCounter(); 
#endif
		Test[10]++;
		task_test.count[1]++;
		current_task = 1;
#if 0
#if STORE_TO_SD		

		store_buffer_to_SD();
#endif		

#endif	


	
		if(count % 4 == 0)  // 1 second
		{	

#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)	//!(ARM_TSTAT_WIFI  )
			update_sntp();
#if (ARM_MINI || ASIX_MINI || ASIX_CM5)
			PIC_refresh();	
			
			if((Modbus.mini_type == MINI_BIG) || (Modbus.mini_type == MINI_BIG_ARM))
			{								
// Check LCD				
				Check_Lcd();				
			}
#endif
#endif	
			
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)	
			RTC_Get();
#endif
			
#if (ASIX_MINI || ASIX_CM5)	
			
#if (ARM_MINI || ASIX_MINI)
			if(Modbus.mini_type == MINI_VAV)
				LED_BEAT = ~LED_BEAT;	
#endif			
			
#endif
			
			
		}	
		count++; 
		
#if (ARM_CM5 || ASIX_CM5)
		if(count % 4 == 0)  // 10 second
		{	
			Check_Lcd();				
		}
#endif

#if ARM_TSTAT_WIFI 		
		IWDG_ReloadCounter(); 
#endif
	}
}



#if (ASIX_MINI || ASIX_CM5)
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
#endif




U8_T flag_reset_default;   
U8_T flag_reboot;
U8_T ether_tx_inactive_count;
U8_T check_input_alarm_count;
void SET_VAV(U8_T level);
#if (ARM_MINI || ARM_CM5)
void tcpip_intial(void);
#endif
void Check_Program_Output_Pri_valid(void);
void Check_Send_bip(void);
void Check_Remote_Panel_Table(void);
void check_whether_suspend_mstp(void);
void Check_LCD_time_off(void);
void Check_Whether_TCP_STUCK(void);
void Check_TCP_UDP_Socket(void);

// 每秒钟执行一次
void check_override_timer_1s(void);

void Email_Task(void);
void ESP8266_Rst( void );
void Monitor_Task_task(void) reentrant
{	
	portTickType xDelayPeriod = ( portTickType ) 1000 / portTICK_RATE_MS;
	U8_T loop;

	static U16_T check_sd = 0;
	static U8_T DYNDNS_TIMER = 0; // time  

	
	ether_tx_inactive_count = 0;
	flag_reset_default = 0;
	flag_reboot = 0;
#if (ASIX_MINI || ASIX_CM5)
	flag_udp_scan = 1;	
	flag_Updata_Clock = 1;	
	//Updata_Clock(0);
#endif
	flag_Update_Dyndns = 0;
	Update_Dyndns_Retry = 0;
	task_test.enable[14] = 1;
	flag_resume_rs485 = 0;
	resume_rs485_count = 0;
	check_sd = 0;
	Device_Set_Object_Instance_Number(Instance);
	
	for(;;)
	{				
		vTaskDelay(1000 / portTICK_RATE_MS);
		task_test.count[14]++;	
		current_task = 14;
		Check_Program_Output_Pri_valid();
		Test[8] = ether_rx_packet;
		Test[9] = ether_tx_packet;
	
		Check_LCD_time_off();	
		
#if ARM_TSTAT_WIFI		
		memcpy(&Test[0],Setting_Info.reg.display_lcd.lcddisplay,sizeof(lcdconfig));
		check_override_timer_1s();
#endif
		
		if(run_time % 3600 == 0)
		{
#if ARM_MINI || ARM_CM5
			tcpip_intial();
#endif
			
#if (ASIX_MINI || ASIX_CM5)
			TCP_IP_Init();
#endif 
		}
		
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
#if (ARM_MINI || ARM_CM5)		
		Check_Whether_TCP_STUCK();
		Check_TCP_UDP_Socket();
#if REBOOT_PER_WEEK
		if((Rtc.Clk.day == 0) && (Rtc.Clk.hour == 2) && (Rtc.Clk.min == 0) && (run_time > 3600))
		{
			QuickSoftReset();			
		}
#endif
#endif
		Check_whether_clear_conflict_id();

		
#endif
		
#if !(ARM_TSTAT_WIFI)	
#if SMTP
		Email_Task();
#endif
#endif

		
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)	
#if (ARM_MINI || ARM_CM5)		
		Check_Send_bip();
#endif		
		check_whether_suspend_mstp();

#if OUTPUT_DEATMASTER
		output_dead_master();
#endif
//		dcc_timer_seconds(1);
		if(count_hold_on_bip_to_mstp > 0)
			count_hold_on_bip_to_mstp--;
		Check_Remote_Panel_Table();
		
#if (ARM_MINI || ASIX_MINI)				
		
#if 1
		if((Modbus.mini_type == MINI_BIG) ||(Modbus.mini_type == MINI_BIG_ARM)
			|| (Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM) 
			|| (Modbus.mini_type == MINI_NEW_TINY) || (Modbus.mini_type == MINI_TINY_ARM)	|| (Modbus.mini_type == MINI_TINY_ARM)
			|| (Modbus.mini_type == MINI_TINY)
			|| (Modbus.mini_type == MINI_NANO) 
			)
		{ // Only for LB, can not detect it 
			if((check_sd < 1200) && (SD_exist == 1))
			{	
				check_sd++;
				
				if(check_sd % 10 == 0)  //check SD per 5 second 
				{
					if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM))
					{
						vTaskSuspend(xHandler_SPI);
						SPI1_Init(1);
					}
					check_SD_exist();

					if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM))
					{
						SPI1_Init(0);
						vTaskResume(xHandler_SPI);
					}
				}
			}
		}
#endif
		
#endif
		
		
#endif		
		Check_Net_Point_Table();
		if(flag_reboot == 1)
		{			
#if (ASIX_MINI || ASIX_CM5)
			Flash_Write_Mass();
			IntFlashWriteByte(0x4001,0);
			AX11000_SoftReboot();
#else  // ARM
			SoftReset();

#endif			

		}

#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)		
		if(flag_Updata_Clock == 1)
		{
			Rtc_Set(Rtc.Clk.year,Rtc.Clk.mon,Rtc.Clk.day,Rtc.Clk.hour,Rtc.Clk.min,Rtc.Clk.sec,0);
			RTC_Get();
			flag_Updata_Clock = 0;
		}
#endif
		
#if (ASIX_MINI || ASIX_CM5)		
		if(flag_Updata_Clock == 1)
		{
			Updata_Clock(0);
			flag_Updata_Clock = 0;
		}
#endif
		
// check if time is expired
// if current panel is #1
#if TIME_SYNC	
#if (ASIX_MINI || ASIX_CM5)		
		check_time_sync();
#endif
#endif	

		
#if (ASIX_MINI || ASIX_CM5)
		if((Modbus.en_dyndns == 2) && (dyndns_provider == 3))
//			 // if cant connect temco_server, retry 3 time
//			// 0xc0a80359
		{
//			RM_Start();
		}
#endif
		
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
		if((Modbus.en_dyndns == 2) && (dyndns_provider == 3))
//			 // if cant connect temco_server, retry 3 time
//			// 0xc0a80359
		{			
//				RM_Start();
		}
#endif
	
// check count of read tstat name
		check_read_tstat_name();			

	

		
#if !(ARM_TSTAT_WIFI)		
		if(Modbus.en_dyndns == 2)
		{
			DYNDNS_TIMER++;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
			do_dyndns();
#endif

			if(flag_Update_Dyndns == 0)
			{
				if(DynDNS_GetState() == DYNDNS_STATE_UPDATE_OK)
				{
					flag_Update_Dyndns = 1;
					
					memcpy(update_dyndns_time.all,Rtc.all,sizeof(UN_Time));
		
					Update_Dyndns_Retry = 0;
		
				}
				else
				{
					if(Update_Dyndns_Retry < MAX_DDNS_RETRY_COUNT)
					{
						if(DYNDNS_TIMER % 10 == 0)  // retry 1 time at a interval of 5 seconds
						{
							init_dyndns();
							Update_Dyndns_Retry++;
						}
					}
					else
					{
						Update_Dyndns_Retry = 0;
						flag_Update_Dyndns = 1;
					}
				}
			}
			
			if(DYNDNS_TIMER > dyndns_update_time * 60)  // 10 min
			{
				init_dyndns();
				DYNDNS_TIMER = 0;
				flag_Update_Dyndns = 0;
				Update_Dyndns_Retry = 0;
			}
		}
#endif
		
#if (ASIX_MINI || ASIX_CM5)	
		Check_whether_reiniTCP();

#if TIME_SYNC				
		flag_stop_timesync = 0;
#endif

		flag_udp_scan = 1;	
		
#endif	
		
	
// step3 ok	
		if(flag_resume_rs485 == 1)  // suspend rs485 task
		{
			resume_rs485_count++;
			if(resume_rs485_count > 5)
			{			
				vTaskResume(Handle_Scan);	
				flag_resume_rs485 = 2;  // resume rs485 task
				resume_rs485_count = 0;
			}
		}

  
		
// check input alarm	
#if ALARM_SYNC
		if(check_input_alarm_count < 60)  // 1min
		{
			check_input_alarm_count++;
		}
		else
		{
			check_input_alarm_count = 0;
			check_input_alarm();
		}
#endif		


// step2 ok		
		if(flag_reset_default == 1)
		{
			set_default_parameters();

			flag_reset_default = 0;
		}


		// step1 ok
// check task		

		for(loop = 0;loop < 15;loop++)	
		{				
			if(task_test.enable[loop] == 1)
			{
			  if(task_test.count[loop] != task_test.old_count[loop])
				{
					task_test.old_count[loop] = task_test.count[loop];
					task_test.inactive_count[loop] = 0;
				}
				else
					task_test.inactive_count[loop]++;
			}
			
			if(task_test.inactive_count[0] > 20)	
			{ 					
				task_test.inactive_count[0] = 0;

				flag_reboot = 1;

			}	
		} 		
		check_flash_changed();
	}
}






void Read_ALL_Data(void);
u8 flag_tcpip_initial;
u8 retry_tcpip_intial;
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
void Check_Whether_TCP_STUCK(void)
{
	static U8_T count = 0;
//	static U32_T TX = 0;
	static U32_T RX = 0;
	static u32 t1 = 0;
	static u32 t2 = 0;

	if(RX != ether_rx_packet)
	{
		RX = ether_rx_packet;
		retry_tcpip_intial = 0;
		count = 0;
	}
	else
	{
		count++;
		if(count >= 5)  // no tx rx , for 5s
		{
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
			flag_tcpip_initial = 1;
#else
			TCP_IP_Init();
#endif
			count = 0;
		}
	}	
	
	if(flag_tcpip_initial == 1)
	{Test[42]++;
		flag_tcpip_initial = 0;
#if (ARM_MINI || ARM_CM5)
		tcpip_intial();
		
#elif (ASIX_MINI || ASIX_CM5)
		TCP_IP_Init();
#endif
		
		t1 = uip_timer;
		if(t1 - t2 > 60000)  // 1min
		{
			t2 = uip_timer;
			retry_tcpip_intial++;
			if(retry_tcpip_intial > 5)
			// reboot
			{
				Test[43]++;
//				if(ether_rx_packet > 0) 
//					flag_reboot = 1;
			}
			
		}
	}
	
}
#endif

#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)

void refresh_led_switch_Task(void) reentrant;
void Scan_network_modbus_Task(void) reentrant;
void Scan_network_bacnet_Task(void) reentrant;
void Handler_COV_Task(void)	reentrant;


void watchdog_init(void)
{
		/* Enable write access to IWDG_PR and IWDG_RLR registers */ 
		IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
		/* IWDG counter clock: 40KHz(LSI) / 4 = 10 KHz */ 
		IWDG_SetPrescaler(IWDG_Prescaler_128); 
		/* Set counter reload value to 10000 = 1s */ 
		IWDG_SetReload(4000); 
		IWDG_ReloadCounter(); // reload the value
		IWDG_Enable();  			//enable the watchdog

}

static void debug_config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable/*GPIO_Remap_SWJ_JTAGDisable*/, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;			
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  						
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);										
	GPIO_ResetBits(GPIOA, GPIO_Pin_13 | GPIO_Pin_14);
}

void RESET_TOP_IO_config(void)
{
		    //GPIO????
  GPIO_InitTypeDef GPIO_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF , ENABLE);	//??USART1,GPIOA??

	//   PD6 PD8 PD9 PD10 PG6 PF11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;				
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;			//??????
	GPIO_Init(GPIOF, &GPIO_InitStructure);			
}

#endif


void inputs_adc_init(void);
void Input_IO_Init(void);
void inpust_scan(void);


#define  DMASIZE 1024

#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)	
#define USART1_DR_Base  0x40013804		// 0x40013800 + 0x04 = 0x40013804

u8 sendbuf[1024];
u8 receivebuf[1024];
static void _uart1_dma_configuration()
{
    DMA_InitTypeDef DMA_InitStructure;

    /* DMA1 Channel6 (triggered by USART1 Rx event) Config */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2,
        ENABLE);

    /* DMA1 Channel5 (triggered by USART1 Rx event) Config */
    DMA_DeInit(DMA1_Channel5);
    DMA_InitStructure.DMA_PeripheralBaseAddr = USART1_DR_Base;// 初始化外设地址，相当于“哪家快递”
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)receivebuf;// 内存地址，相当于几号柜
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//外设作为数据来源，即为收快递
    DMA_InitStructure.DMA_BufferSize = DMASIZE;// 缓存容量，即柜子大小
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; // 外设地址不递增，即柜子对应的快递不变
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;// 内存递增
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //外设字节宽度，即快递运输快件大小度量（按重量算，还是按体积算）
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;// 内存字节宽度，即店主封装快递的度量(按重量，还是按体质进行封装)
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;// 正常模式，即满了就不在接收了，而不是循环存储
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;// 优先级很高，对应快递就是加急
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; // 内存与外设通信，而非内存到内存
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);// 把参数初始化，即拟好与快递公司的协议

    DMA_Cmd(DMA1_Channel5, ENABLE);// 启动DMA，即与快递公司签订合同，正式生效

                                   /* DMA1 Channel4 (triggered by USART1 Tx event) Config */
    DMA_DeInit(DMA1_Channel4);
    DMA_InitStructure.DMA_PeripheralBaseAddr = USART1_DR_Base;  // 外设地址，串口1， 即发件的快递
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)sendbuf;// 发送内存地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;// 外设为传送数据目的地，即发送数据，即快递是发件
    DMA_InitStructure.DMA_BufferSize = 0;  //发送长度为0，即未有快递需要发送
    DMA_Init(DMA1_Channel4, &DMA_InitStructure);//初始化

    USART_ITConfig(USART1, USART_IT_TC, ENABLE);// 使能串口发送完成中断
    USART_DMACmd(USART1, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);// 使能DMA串口发送和接受请求
}
#endif



U8_T flag_reboot_ontime;
void main( void )
{
//	U8_T regisp;
	U8_T set_para;
	U16_T loop;
	U8_T flag_store;
	run_time = 0;
	
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)			

	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x8008000);
	debug_config();	
	//ram_test = 0 ;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD , ENABLE);
 	delay_init(72);
	
	E2prom_Initial(); 	
	for(loop = 0;loop < 50;loop++)	Test[loop] = 0;	

#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
	Output_IO_Init();//测试
	uart1_init(115200);
// 如果上一次跳转是默认自己
#if REBOOT_PER_WEEK
	if(AT24CXX_ReadOneByte(EEP_APP2BOOT_TYPE) == 0x55)
	{
		U8_T temp[4];
		E2prom_Read_Byte(EEP_RUNNING_TIME1, &temp[0]);
		E2prom_Read_Byte(EEP_RUNNING_TIME2, &temp[1]);
		E2prom_Read_Byte(EEP_RUNNING_TIME3, &temp[2]);
		E2prom_Read_Byte(EEP_RUNNING_TIME4, &temp[3]);
		
		E2prom_Read_Byte(EEP_ISP_REV,&Modbus.IspVer);
		
		run_time_last = temp[0] + (U16_T)(temp[1] << 8) + ((U32_T)temp[2] << 16) + ((U32_T)temp[3] << 24);
//		flag_reboot_ontime = 1;
		if(Modbus.IspVer < 66)
			run_time_last += 10;
		else
			run_time_last += 3;
	}
	else
#endif
	{
		E2prom_Write_Byte(EEP_RUNNING_TIME1, 0);
		E2prom_Write_Byte(EEP_RUNNING_TIME2, 0);
		E2prom_Write_Byte(EEP_RUNNING_TIME3, 0);
		E2prom_Write_Byte(EEP_RUNNING_TIME4, 0);
		run_time_last = 0;
//		flag_reboot_ontime = 0;
	}
	
	AT24CXX_WriteOneByte(EEP_APP2BOOT_TYPE, 0);
	
	E2prom_Read_Byte(EEP_CPU_TYPE,&cpu_type);
	E2prom_Read_Byte(EEP_MINI_TYPE,&Modbus.mini_type);
#if (ARM_MINI || ASIX_MINI)
	if((Modbus.mini_type == MINI_NEW_TINY) || (Modbus.mini_type == MINI_TINY_ARM) || (Modbus.mini_type == MINI_TINY_ARM)|| (Modbus.mini_type == MINI_TINY_11I) ||
		(Modbus.mini_type == MINI_NANO))
	{	
		UART0_TXEN_TINY = 1;
		LED_IO_Init();
	}
	else
	{
		UART0_TXEN_BIG = 1;		
		RESET_TOP_IO_config();

	}
	if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM))
	{
		TOP_CS = 1;
		SD_CS_SMALL = 0;
		
	}
	printf("mini app\r\n");
#endif
#if (ARM_TSTAT_WIFI)
	UART0_TXEN_BIG = 1;
	UART2_TXEN_BIG = 1;
	printf("tstat app\r\n");
#endif	
	
#endif
	
#if ARM_TSTAT_WIFI_DEBUG
	printf("tstat8 app\r\n");
#endif
#if 0//ARM_UART_DEBUG
	uart1_init(115200);
	DEBUG_EN = 1;
	printf("if mini type is %u E2 is ok. \r\n",Modbus.mini_type);
#endif

#if ARM_UART_DEBUG
	printf("intial RTC...\r\n");
#endif
	
	RTC_Init();
	
#if ARM_UART_DEBUG
	printf("intial RTC OK\r\n");
#endif
#endif

		
	
#if (ASIX_MINI || ASIX_CM5)
	AX11000_Init();
	ExecuteRuntimeFlag = 1;

#if AX_WATCHDOG_ENB
	AX11000_WatchDogSetting(0, 1, 0, WD_INTERVAL_67M);  /* time out, reset cpu */
	sTaskCreate(SoftwareWatchdog_task, (const signed portCHAR * const)"softwatch_task",portMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 11 , (xTaskHandle *)&xSoftWatchTask);
#endif
	
	DELAY_Init();
#endif


//	xLCDQueue = xQueueCreate( 5, sizeof(xLCDMessage));
	memset(&task_test,0,sizeof(STR_Task_Test));	
#if (ASIX_MINI || ASIX_CM5)
	IntFlashReadByte(0x6fff1,& set_para);
	if(set_para == 0xff)
	{ /* initial defautl parameters*/
//		Eeprom_Write_Cpu_Config();
//		Test[48]++;
		etr_reboot = 0;
		set_para = 0xaa;
		IntFlashWriteByte(0x6fff1,set_para);
	}	
	else
	{
		etr_reboot = 1;
	}
		
	E2prom_Initial(); 
	
#endif
	Bacnet_Initial_Data();
	Read_ALL_Data();  
#if !(ARM_TSTAT_WIFI)	
	if((Modbus.mini_type <= MINI_BIG) || (Modbus.mini_type == MINI_BIG_ARM))
	{
		Lcd_Initial();
		Display_Initial_Data();
		Key_Inital();
		Lcd_Show_String(0, 0, "step1: LCD & KEY ", NORMAL, 21,0x0000,0xffff);
		Lcd_Show_String(1, 0, "step2: E2 OK ", NORMAL, 21,0x0000,0xffff);		
		
	}
#endif
	
#if (ARM_MINI || ASIX_MINI)

	if((Modbus.mini_type == MINI_BIG) || (Modbus.mini_type == MINI_BIG_ARM))
	{
		RESET_TOP = 0;  // RESET c8051f023 
		DELAY_Us(500);
		RESET_TOP = 1; 	
	}
	else if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM))
	{ // ARM REV dont have reset pin
		RESET_TOP = 0;  // RESET c8051f023 
		DELAY_Us(500);
		RESET_TOP = 1; 	
	}
	else if(Modbus.mini_type == MINI_TINY)
	{		
		if(Modbus.hardRev >= STM_TINY_REV)
		{			
			RESET_TOP = 0;  // RESET stm32
			DELAY_Us(500);
			RESET_TOP = 1; 	
		}
		else
		{  // old version, top board is sm5r16		
			RESET_TOP = 1;  // RESET sm5r16 
			DELAY_Us(500);
			RESET_TOP = 0;
		}
	}	
#endif	
	
	Comm_Tstat_Initial_Data();
	init_scan_db();
	Flash_Inital();	
#if (ASIX_MINI || ASIX_CM5)
	init_dyndns_data();
	IntFlashReadByte(0x7fff0,&flag_store);
//	Test[49] = flag_store;
	if(flag_store == 0x55)
	{		
		if((Modbus.mini_type <= MINI_BIG) || (Modbus.mini_type == MINI_BIG_ARM))
		Lcd_Show_String(2, 0, "step3: READ FLASH", NORMAL, 21,0x0000,0xffff);	
		Flash_Read_Mass();	
		Get_Tst_DB_From_Flash(); 
	}
	else
	{
		if((Modbus.mini_type <= MINI_BIG) || (Modbus.mini_type == MINI_BIG_ARM))
		Lcd_Show_String(2, 0, "step3: EMPTY FLASH", NORMAL, 21,0x0000,0xffff);	
	}
#endif

#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)	
	Flash_Read_Mass();	
	Get_Tst_DB_From_Flash(); 
  TIM6_Int_Init(100, 719);
#endif
	
	SD_exist = 1;  // inexist
	
#if STORE_TO_SD	
	
//#if (ASIX_MINI || ASIX_CM5)
//	if((Modbus.mini_type == MINI_BIG) ||(Modbus.mini_type == MINI_BIG_ARM)
//	 || (Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM) 
//	|| (Modbus.mini_type == MINI_NEW_TINY) || (Modbus.mini_type == MINI_TINY_ARM)	
//	|| (Modbus.mini_type == MINI_TINY) 
//	)
		check_SD_exist();
//#endif
#if (ARM_MINI || ASIX_MINI)	
	if(SD_exist == 2)	
	{
		if((Modbus.mini_type <= MINI_BIG) || (Modbus.mini_type == MINI_BIG_ARM))
			Lcd_Show_String(3, 0, "step4: SD EXIST", NORMAL, 21,0x0000,0xffff);	
	}
	else
	{
		if((Modbus.mini_type <= MINI_BIG) || (Modbus.mini_type == MINI_BIG_ARM))
			Lcd_Show_String(3, 0, "step4: SD INEXIST", NORMAL, 21,0x0000,0xffff);	
	}
#endif
	
#endif
	initial_graphic_point();
 	monitor_init();

#if (DEBUG_UART1)
	UART_Init(UART_SUB1);

	if(UART_SUB1 == 0)	UART0_TXEN_TINY = SEND;  // MINI
	if(UART_SUB1 == 2)	UART2_TXEN_BIG = SEND;  // MINI

	
	sprintf(debug_str,"intial\r\n");
	uart_send_string(debug_str,strlen(debug_str),UART_SUB1);
	
#endif	
	Initial_Panel_Info(); // read panel name, must read flash first
	Sync_Panel_Info();
	initSerial();
	current_online_ctr = 0;
#if (ARM_MINI || ASIX_MINI || ASIX_CM5)
	PIC_initial_data();
#endif

#if ARM_UART_DEBUG
	uart1_init(115200);
	DEBUG_EN = 1;
	printf("APP intial ok\r\n");
#endif
#if (DEBUG_UART1)
	uart_init_send_com(UART_SUB1);	// for test		
	sprintf(debug_str," \r\n\ intial ok");
	uart_send_string(debug_str,strlen(debug_str),UART_SUB1);
#endif 
#if (ARM_MINI || ARM_TSTAT_WIFI)
	vStartWifiTasks(tskIDLE_PRIORITY + 14); 		
#endif

#if ARM_TSTAT_WIFI
	vStartKeyTasks(tskIDLE_PRIORITY + 5);
	//vStartDisplayTasks(tskIDLE_PRIORITY + 4);
	vStartMenuTask(tskIDLE_PRIORITY + 4);
#endif	
	sTaskCreate(Common_task,/* (const signed portCHAR * const)*/"Common_task",
		COMMON_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, (xTaskHandle *)&xHandleCommon);

	Inital_Bacnet_Server();
	
	vStartMainSerialTasks(tskIDLE_PRIORITY + 12);	 // main uart, rs485 or zigbee
#if !(ARM_TSTAT_WIFI)
	if((Modbus.mini_type <= MINI_BIG) || (Modbus.mini_type == MINI_BIG_ARM))
		Lcd_Show_String(4, 0, "step5: INITIAL OK", NORMAL, 21,0x0000,0xffff);	

	Display_IP();	
	
	sTaskCreate(TCPIP_Task, /*(const signed portCHAR * const)*/"TCPIP_task",
		TCPIP_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, (xTaskHandle *)&xHandleTcp); 

#if (ARM_MINI || ASIX_MINI)		
	/* slave select output enable, SPI master, SSO auto, SPI enable, SPI_STCFIE enable, baudrate, slave select */
		if((Modbus.mini_type == MINI_BIG) ||(Modbus.mini_type == MINI_BIG_ARM)
	|| (Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM) 
	|| (Modbus.mini_type == MINI_NEW_TINY) || (Modbus.mini_type == MINI_TINY_ARM)	|| (Modbus.mini_type == MINI_TINY_ARM)
	|| (Modbus.mini_type == MINI_TINY)) 
	{
#if ASIX_MINI
		SPI_Setup(SPI_SSO_ENB|SPI_MST_SEL|SPI_SS_AUTO|SPI_ENB, SPI_STCFIE, 10, SLAVE_SEL_1); // 25M
		vStartCommToTopTasks(tskIDLE_PRIORITY + 5);
#endif	
	}
		
#if ARM_MINI
		if((Modbus.mini_type == MINI_BIG) || (Modbus.mini_type == MINI_BIG_ARM)
			|| (Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM))
			vStartCommToTopTasks(tskIDLE_PRIORITY + 7);		

		else if((Modbus.mini_type == MINI_NEW_TINY) || (Modbus.mini_type == MINI_TINY_ARM) || (Modbus.mini_type == MINI_TINY_11I))
		{
			sTaskCreate(refresh_led_switch_Task, "refresh_led_task", 500, NULL, tskIDLE_PRIORITY + 3, (xTaskHandle *)&xHandleLedRefresh);
		}		
#if COV		
		sTaskCreate(Handler_COV_Task, (const signed portCHAR * const)"Handler_COV_Task", 200, NULL, tskIDLE_PRIORITY + 1, (xTaskHandle *)&Handle_COV);
#endif
		sTaskCreate(Scan_network_bacnet_Task, (const signed portCHAR * const)"Scan_network_bacnet_Task", 512, NULL, tskIDLE_PRIORITY + 1, (xTaskHandle *)&Handle_Scan_net);

#if NETWORK_MODBUS		
		sTaskCreate(Scan_network_modbus_Task, (const signed portCHAR * const)"Scan_network_modbus_Task", 256, NULL, tskIDLE_PRIORITY + 1, (xTaskHandle *)&Handle_Scan_net);
#endif
		
#endif
		

	
#endif


	
	


#if (ARM_CM5 || ASIX_CM5)		
#if ARM_CM5
		sTaskCreate(Scan_network_bacnet_Task, (const signed portCHAR * const)"Scan_network_bacnet_Task", 512, NULL, tskIDLE_PRIORITY + 1, (xTaskHandle *)&Handle_Scan_net);
#endif
		vStartKeyTasks(tskIDLE_PRIORITY + 5);
		sTaskCreate( Sampel_AI_Task, "SampleAItask", SampleAISTACK_SIZE, NULL, tskIDLE_PRIORITY + 5, &Handle_SampleAI ); 
		sTaskCreate( Sampel_DI_Task, "SampleDItask", SampleDISTACK_SIZE, NULL, tskIDLE_PRIORITY + 5, &Handle_SampleDI ); 
#endif
	
#endif
	
	vStartScanTask(tskIDLE_PRIORITY + 3);
#if MSTP
	sTaskCreate(Master_Node_task, (const signed portCHAR * const)"Master_Node_task", BACnet_STACK_SIZE, NULL, tskIDLE_PRIORITY + 13, (xTaskHandle *)&xHandleMSTP);
#endif
	
	sTaskCreate(Bacnet_Control,/*(const signed portCHAR * const)*/"BAC_Control_task",
		Control_STACK_SIZE, NULL, tskIDLE_PRIORITY + 8,(xTaskHandle *)&xHandleBacnetControl);	

	sTaskCreate(Monitor_Task_task, /*(const signed portCHAR * const)*/"monitor_task",
		Monitor_STACK_SIZE, NULL, tskIDLE_PRIORITY + 12,(xTaskHandle *)&xHandleMornitor_task);
	
#if (ARM_MINI || ASIX_MINI || ARM_CM5 || ARM_TSTAT_WIFI)	
	vStartOutputTasks(tskIDLE_PRIORITY + 5);
	
#endif
	

//	if(Modbus.mini_type <= MINI_BIG)
//	{
//		sTaskCreate( vLCDTask, /*( signed portCHAR * )*/ "LCD", 500, NULL, tskIDLE_PRIORITY + 1, (xTaskHandle *)&xHandleLCD_task );
//	}
	/* Finally kick off the scheduler.  This function should never return. */


#if (ASIX_MINI || ASIX_CM5)
	vTaskStartScheduler( portUSE_PREEMPTION );
#else
	vTaskStartScheduler();
#endif

	/* Should never reach here now under control of the scheduler. */

}


/*-----------------------------------------------------------*/
#if !(ARM_TSTAT_WIFI)
U8_T Check_Lcd(void)
{
	char far text[21];
	if(count_refresh_all < 3600)  // 1hour
	{
		count_refresh_all++;
		if((backup_sub_no != sub_no) || (backup_current_online_ctr != current_online_ctr))
		{
			sprintf(text, "DB: %u ON: %u", (uint16)sub_no,(uint16)current_online_ctr);
			xMessage.x = 2;
			xMessage.y = 0;
			xMessage.str = text;
			xMessage.mode = NORMAL;
			xMessage.len = 21;
			xMessage.dcolor = 0;
			xMessage.bgcolor = 0xffff;
			Lcd_Show_String(xMessage.x, xMessage.y,xMessage.str, NORMAL, 21,xMessage.dcolor,xMessage .bgcolor);

			backup_sub_no = sub_no;
			backup_current_online_ctr = current_online_ctr;
			return 1;
		}
		if((backup_ether_rx_packet != ether_rx_packet) || (backup_ether_tx_packet != ether_tx_packet))
		{
			backup_ether_rx_packet = ether_rx_packet;
			backup_ether_tx_packet = ether_tx_packet;
			sprintf(text, "RX:%lu TX:%lu",ether_rx_packet,ether_tx_packet);
			xMessage.x = 3;
			xMessage.y = 0;
			xMessage.str = text;
			xMessage.mode = NORMAL;
			xMessage.len = 21;
			xMessage.dcolor = 0;
			xMessage.bgcolor = 0xffff;

			Lcd_Show_String(xMessage.x, xMessage.y,xMessage.str, NORMAL, 21,xMessage.dcolor,xMessage .bgcolor);

			//return 1;
		}	
		//else
		{
		// update time
			get_time_text();
			sprintf(text, "%s", time);
			xMessage.x = 4;
			xMessage.y = 0;
			xMessage.str = text;
			xMessage.mode = NORMAL;
			xMessage.len = 21;
			xMessage.dcolor = 0x0000;
			xMessage.bgcolor = 0xffff;	
		}
		Lcd_Show_String(xMessage.x, xMessage.y,xMessage.str, NORMAL, 21,xMessage.dcolor,xMessage .bgcolor);

	}
	else
	{
		Lcd_Initial();
		Display_IP();
		count_refresh_all = 0;
	}
	return 0;
}
#endif

#if 0
void vLCDTask( void ) reentrant
{
//	xLCDMessage xMessage1;
	uint16_t count;
	/* Initialise the LCD and display a startup message. */
//	prvConfigureLCD();
//	LCD_DrawMonoPict( ( unsigned portLONG * ) pcBitmap );

	task_test.enable[6] = 1;
	for( ;; )
	{
		/* Wait for a message to arrive that requires displaying. */
		task_test.count[6]++;
		count = 0;
//		while((cQueueReceive( xLCDQueue, &xMessage, portMAX_DELAY ) != pdPASS) && (count++ < 10000)) Test[30]++;
	 vTaskDelay( 500 / portTICK_RATE_MS);	
		/* Display the message.  Print each message to a different position. */
		//printf( ( portCHAR const * ) xMessage.pcMessage );
//		Lcd_Initial();
		Lcd_Show_String(xMessage.x, xMessage.y,xMessage.str, NORMAL, 21,xMessage.dcolor,xMessage .bgcolor);
	}
}
#endif

#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
void _ttywrch(int ch)
{
ch = ch;
}
#endif

