#include "main.h"    
#include "serial.h"
//#include "schedule.h" 
#include "stdlib.h" 
#include <stdio.h>
#include <string.h>


void Sync_timestamp(S16_T newTZ,S16_T oldTZ,S8_T newDLS,S8_T oldDLS);
void Write_ZIGBEE_ID(U16_T newid);
void clear_sd(void);

#if ARM_WIFI

#include "wifi.h"

extern uint8_t modbus_wifi_buf[500];
extern uint16_t modbus_wifi_len;

#endif

#if (ASIX_MINI || ASIX_CM5)
#include "pic.h"

#endif

#include "fifo.h"
#include "tcpip.h"

extern U8_T 	far bacnet_to_modbus[300];

#if ARM_WIFI
bit flagLED_ether_tx;
bit flagLED_ether_rx;
bit flagLED_uart0_rx;
bit flagLED_uart0_tx;
bit flagLED_uart1_rx; 
bit flagLED_uart1_tx;
bit flagLED_uart2_rx;
bit flagLED_uart2_tx;

#endif

#define UIP_HEAD  6

U16_T far uart0_sendbyte_num;
U16_T far uart1_sendbyte_num;
U16_T far uart2_sendbyte_num;

U16_T far uart0_send_count;
U16_T far uart1_send_count;
U16_T far uart2_send_count;

U16_T far uart2_rece_size = 0;
U16_T far uart2_rece_count;
U8_T far uart0_send_buf[MAX_BUF_LEN];
U8_T far uart1_send_buf[MAX_BUF_LEN];
U8_T far uart2_send_buf[MAX_BUF_LEN];
U8_T far uart2_dealwithTag;
U8_T far uart2_data_buffer[MAX_BUF_LEN];
U8_T far uart2_transmit_finished;

#if ARM_MINI || ARM_CM5 || ARM_WIFI
void UART_Init(U8_T port);
u32 Rtc_Set(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec,u8 flag);
void modubs_sub1_uart_int_hander(void);
//extern u8 tcp_server_sendbuf[300];
extern u16 tcp_server_sendlen;
extern uint8_t write_page_en[25];
#endif

//void Reset_Zigbee_Moudle(void);
//void bip_Init(U16_T localPort);
void TCP_IP_Init(void);
void init_dyndns(void);
//void Write_SD(void);

U8_T far flag_sem_response_modbus;

extern void set_output_raw(uint8_t point,uint16_t value);

U8_T far scan_port;
U8_T far scan_baut;

S16_T far timezone;
U8_T far Daylight_Saving_Time;
U8_T far SntpServer[4];
U8_T far zigbee_exist;

extern U8_T flag_Update_Dyndns;
extern U8_T Update_Dyndns_Retry;

extern uint8 flag_response_zigbee;

void write_user_data_by_block(U16_T StartAdd,U8_T HeadLen,U8_T *pData) ;
U16_T read_user_data_by_block(U16_T addr);
void test_alarm(uint8_t test);

#if 0
U8_T logic_in;
U8_T logic_out;
U16_T logic_count;
#endif

void SET_VAV(U8_T level);
bit flag_logic_control;

#if (ARM_MINI || ASIX_MINI)
RS232_CMD rs232_cmd;
#endif

U8_T pic_watchdog = 0;
U8_T far bbmd_en;
U16_T far mstp_network;
STR_MODBUS far Modbus;


bit Mac_Address_write_enable = FALSE;

U8_T uart0_baudrate;
U8_T uart1_baudrate;
U8_T uart2_baudrate;
U8_T temp_baut[3];
U8_T flag_temp_baut[3];

U16_T test_adc = 0;
U8_T test_adc_flag;


uint32_t run_time;
uint8_t tcp_connect_num;
uint8_t udp_connect_num;

xTaskHandle xdata Handle_MainSerial;

xSemaphoreHandle sem_response_modbus;


xSemaphoreHandle sem_subnet_tx_uart0;
xSemaphoreHandle sem_subnet_tx_uart2;
xSemaphoreHandle sem_subnet_tx_uart1;

#if (ASIX_MINI || ASIX_CM5)
//xQueueHandle qSubSerial_uart1;

UN_Time Rtc;

#endif

bit flag_gsm_initial;

//UN_HIGH_COUNT 
U32_T far high_spd_counter[HI_COMMON_CHANNEL];
U32_T far high_spd_counter_tempbuf[HI_COMMON_CHANNEL];
U8_T far high_spd_en[HI_COMMON_CHANNEL];

U8_T subnet_rec_package_size; 



U16_T uart0_rece_count;
U16_T uart1_rece_count;


U16_T uart0_rece_size = 0;
U16_T uart1_rece_size = 0;

U8_T uart0_dealwithTag;
U8_T uart1_dealwithTag;


U8_T main_dealwithTag;
U16_T main_rece_size = 0;

U8_T far uart0_serial_receive_timeout_count, uart1_serial_receive_timeout_count,uart2_serial_receive_timeout_count;
U8_T uart0_transmit_finished,uart1_transmit_finished;//,uart2_transmit_finished;

U8_T far uart0_data_buffer[MAX_BUF_LEN];
U8_T far uart1_data_buffer[MAX_BUF_LEN];

U8_T far main_data_buffer[MAX_BUF_LEN];
U8_T far subnet_response_buf[MAX_BUF_LEN];

U8_T SERIAL_RECEIVE_TIMEOUT;
U8_T SNWriteflag; 
U8_T far rand_read_ten_count;
unsigned short TransID = 1;

//bit  fix_tstat_position = 1;
//U8_T tstat_position[SUB_NO];
//U8_T add_id_by_hand;
//U8_T datalen = 0;
//U8_T  address = 0;

//U8_T GSM_Initial(void);
void GSM_Test();
void GSM_Test1(void);
void GSM_IPSEND(U8_T port);

void TEST_PIC(char type);
void SPI_Send(U8_T cmd,U8_T* buf,U8_T len);
extern U8_T OutputLed[24];

void main_dealwithData( void );
//void subdealwithData( void );
#if ARM_MINI 
void Modbus_IO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM))
	{
		// SUB -> PE.13 P3.14
		// MAIN -> PE.15 PF.6
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF, ENABLE);
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15; 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOE, &GPIO_InitStructure);
		
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOF, &GPIO_InitStructure);
		
		GPIO_SetBits(GPIOE, GPIO_Pin_15);
		GPIO_ResetBits(GPIOF, GPIO_Pin_6);
		GPIO_SetBits(GPIOE, GPIO_Pin_13);
		GPIO_ResetBits(GPIOE, GPIO_Pin_14);
	}
}

#endif


void vStartMainSerialTasks( U8_T uxPriority)
{
#if ARM_MINI
	Modbus_IO_Init();
#endif

	sTaskCreate( main_dealwithData, "mainserailtask", MainSerialSTACK_SIZE, NULL, uxPriority, &Handle_MainSerial );
}




void initSerial(void)
{
	TransID = 1;
//	fix_tstat_position = 1;
//	add_id_by_hand = 0;
//	memset(tstat_position,0,SUB_NO);
	
	if((Modbus.com_config[0] == MODBUS_SLAVE) || (Modbus.com_config[0] == 0))
		uart_serial_restart(0);
	if((Modbus.com_config[1] == MODBUS_SLAVE) || (Modbus.com_config[1] == 0))
		uart_serial_restart(1);
	if((Modbus.com_config[2] == MODBUS_SLAVE) || (Modbus.com_config[2] == 0))
		uart_serial_restart(2);	
	uart0_rece_count = 0;
	uart0_dealwithTag = 0;
	uart0_rece_size = 0;
	uart1_rece_count = 0;
	uart1_dealwithTag = 0;
	uart1_rece_size = 0;
	main_rece_size = 0;
	main_dealwithTag = 0;

#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
	uart2_rece_count = 0;
	uart2_dealwithTag = 0;
	uart2_rece_size = 0;
#endif
	
#if (ASIX_MINI || ASIX_CM5)
	vSemaphoreCreateBinary(sem_subnet_tx_uart0);
	vSemaphoreCreateBinary(sem_subnet_tx_uart1);
	vSemaphoreCreateBinary(sem_subnet_tx_uart2);
	vSemaphoreCreateBinary(sem_response_modbus);
#endif
	
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)

	sem_subnet_tx_uart0 = vSemaphoreCreateBinary(0);
	sem_subnet_tx_uart1 = vSemaphoreCreateBinary(0);
	sem_subnet_tx_uart2 = vSemaphoreCreateBinary(0);
	sem_response_modbus = vSemaphoreCreateBinary(0);
#endif
		
	flag_sem_response_modbus = 0;
#if (ARM_MINI || ASIX_MINI )
	//main_dealwithTag = 1;
	if(Modbus.com_config[0] == MODBUS_SLAVE)
	{
		if((Modbus.mini_type == MINI_TINY) 
			|| (Modbus.mini_type == MINI_NEW_TINY) || (Modbus.mini_type == MINI_TINY_ARM))
				UART0_TXEN_TINY = RECEIVE;
		else
				UART0_TXEN_BIG = RECEIVE;	
		
	}
	else if(Modbus.com_config[2] == MODBUS_SLAVE || Modbus.com_config[2] == BACNET_SLAVE || Modbus.com_config[2] == BACNET_MASTER)
	{
		if((Modbus.mini_type == MINI_TINY) 
			|| (Modbus.mini_type == MINI_NEW_TINY) || (Modbus.mini_type == MINI_TINY_ARM))
		{			
				UART2_TXEN_TINY = RECEIVE;
		}
		else if(Modbus.mini_type == MINI_VAV)
		{			
				UART2_TXEN_VAV = RECEIVE;
		}
		else
		{
				UART2_TXEN_BIG = RECEIVE;
		}
	
	}	
	
#endif

#if ARM_CM5

	if(Modbus.com_config[0] == MODBUS_SLAVE)
	{	
		UART0_TXEN = RECEIVE;			
	}
	else if(Modbus.com_config[2] == MODBUS_SLAVE || Modbus.com_config[2] == BACNET_SLAVE || Modbus.com_config[2] == BACNET_MASTER)
	{
		UART2_TXEN = RECEIVE;
	}	
	
#endif
	
#if ARM_WIFI
	UART0_TXEN_BIG = RECEIVE;
#endif

//// 	check if during ISP mode if the address has been changed
// 	E2prom_Read_Byte(EEP_ADDRESS,& Modbus.address);

//	// if it has not been changed, check the flash memory
//	if( ( Modbus.address == 255) || ( Modbus.address == 0) )
//	{
//	//	if(E2prom_Read_Byte(EEP_ADDRESS, & address) )
//		{
//			 Modbus.address = 254;
//			
//			E2prom_Write_Byte(EEP_ADDRESS,  Modbus.address);
//		}

//	}
//	else
//    {
//		E2prom_Write_Byte(EEP_ADDRESS,  Modbus.address);

//	}

//	// if data is blank, means first Time programming, thus put as default
//	// Added by RL 02/11/04
//	if( Modbus.address == 0 ||  Modbus.address == 255 ) 
//		 Modbus.address = 254;

	SERIAL_RECEIVE_TIMEOUT = 3;
  uart2_serial_receive_timeout_count = SERIAL_RECEIVE_TIMEOUT;
	uart0_serial_receive_timeout_count = SERIAL_RECEIVE_TIMEOUT;	
}
/****************************************************\
*	CRC CODE START
\****************************************************/
U8_T MainCRChi = 0xff;
U8_T MainCRClo = 0xff;


// Table of CRC values for high??order byt
U8_T const code auchCRCHi[256] = {
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};	

// Table of CRC values for low Corder byte
U8_T const code  auchCRCLo[256] = {
	0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04,
	0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8,
	0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
	0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 0x11, 0xD1, 0xD0, 0x10,
	0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
	0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
	0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C,
	0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0,
	0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
	0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
	0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C,
	0xB4, 0x74, 0x75, 0xB5, 0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
	0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54,
	0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98,
	0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
	0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};


//init crc 
void main_init_crc16(void)
{
	MainCRClo = 0xff;
	MainCRChi = 0xff;
}

//calculate crc with one byte
void main_crc16_byte(U8_T ch)
{
	U8_T uIndex;
	uIndex = MainCRChi^ch; // calculate the CRC 
	MainCRChi = MainCRClo^auchCRCHi[uIndex];
	MainCRClo = auchCRCLo[uIndex];
}


//calculate crc 
U16_T crc16(U8_T *p, U8_T length)
{
	U16_T uchCRCHi = 0xff;	// high byte of CRC initialized 
	U8_T uchCRCLo = 0xff;	// low byte of CRC initialized 
	U8_T uIndex;			// will index into CRC lookup table 
	U8_T i = 0;

	while(length--)//pass through message buffer 
	{		
		uIndex = uchCRCHi^p[i++]; // calculate the CRC 
		uchCRCHi = uchCRCLo^auchCRCHi[uIndex] ;
		uchCRCLo = auchCRCLo[uIndex];
	}
	return (uchCRCHi << 8 | uchCRCLo);
}
/****************************************************\
*	CRC CODE END
\****************************************************/

// for mstp

#if (ARM_MINI || ARM_CM5 || ARM_WIFI)

u8 UART_Get_SendCount(void)
{
	return 1;//uart2_send_count;
}

char get_current_mstp_port(void)
{
	if(Modbus.com_config[2] == BACNET_MASTER || Modbus.com_config[2] == BACNET_SLAVE)
		return 2;
	else if(Modbus.com_config[0] == BACNET_MASTER || Modbus.com_config[0] == BACNET_SLAVE)
	{
		return 0;
	}
	else 
		return -1;
}
#endif

void uart_init_send_com(U8_T port)
{
	if(port > 2) return;

	
#if (ARM_CM5 || ASIX_CM5)
	UART_Init(port);
	if(port == 2)
	{ 	
		UART2_TXEN = SEND;
	}
	else if(port == 0)
	{
//		uart0_transmit_finished = 0;
		UART0_TXEN = SEND;
	}
#endif

#if (ARM_MINI || ASIX_MINI) 

	UART_Init(port);
	
	if(port == 1)
	{		
		if(Modbus.com_config[1] == MODBUS_SLAVE) 		
	   		uart1_transmit_finished = 0;
		// ONLY FOR NEW ARM, choose ZIGBEE or RS232 by anaolog switch
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
		if((Modbus.mini_type == MINI_BIG_ARM) || (Modbus.mini_type == MINI_SMALL_ARM))
		{
			if(Modbus.com_config[1] == MODBUS_MASTER || Modbus.com_config[1] == 0)
				UART1_SW = 1;
			else
				UART1_SW = 0;
		}
#endif
	}
	else if(port == 0 )
	{
#if ASIX_MINI
	   uart0_transmit_finished = 0;
#endif
		if((Modbus.mini_type == MINI_TINY) 
			|| (Modbus.mini_type == MINI_NEW_TINY) || (Modbus.mini_type == MINI_TINY_ARM))
			UART0_TXEN_TINY = SEND;		
		else
			UART0_TXEN_BIG = SEND;		
	}
	else if(port == 2)
	{
		if((Modbus.mini_type == MINI_TINY) 
			|| (Modbus.mini_type == MINI_NEW_TINY) || (Modbus.mini_type == MINI_TINY_ARM))
		{			
				UART2_TXEN_TINY = SEND;
		}
		else if(Modbus.mini_type == MINI_VAV)
		{			
				UART2_TXEN_VAV = SEND;
		}
		else
		{
				UART2_TXEN_BIG = SEND;
		}
	}

#endif

#if ARM_WIFI
//	UART_Init(port);
//	if(port == 2)
//	{ 	
//		UART2_TXEN_BIG = SEND;
//	}
//	else if(port == 0)
//	{
////		uart0_transmit_finished = 0;
		UART0_TXEN_BIG = SEND;
//	}
#endif

}

void uart_send_byte(U8_T buffer,U8_T port)
{
	U16_T count = 0;
	if(port > 2) return;

#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
	if(port == PORT_RS485_MAIN)
	{
	 USART_ClearFlag(USART3, USART_FLAG_TC); 
   USART_SendData(USART3,  buffer);
	}
	else	if(port == PORT_RS485_SUB)
	{
	 USART_ClearFlag(USART1, USART_FLAG_TC); 
   USART_SendData(USART1,  buffer);
	}
	else	if(port == PORT_ZIGBEE)
	{
	 USART_ClearFlag(USART2, USART_FLAG_TC); 
   USART_SendData(USART2,  buffer);
	}


#endif	

#if (ASIX_MINI || ASIX_CM5)
#if  ASIX_CM5
	if(port == 0)
	{
		SBUF0 = buffer;
		uart0_transmit_finished = 0;
		while(!uart0_transmit_finished && count < 5000) 
		{
			
			count++;
		}

	}
	if(port == 2)
	{		
		HSUR_PutChar(buffer);
		com_tx[2]++;
		flagLED_uart2_tx = 1; 
	}

#endif

#if ASIX_MINI
	if(port == 0)
	{
		SBUF0 = buffer;
		uart0_transmit_finished = 0;
		count = 0;
		while(!uart0_transmit_finished && count < 500) count++;
		
	}
	else if(port == 1 )
	{
		SBUF1 = buffer;
		uart1_transmit_finished = 0;
		
		while(!uart1_transmit_finished && count < 500) 		
		{					
			count++;
		}
		
	}  
	else if(port == 2)
	{
	  HSUR_PutChar(buffer);
		com_tx[2]++;
		flagLED_uart2_tx = 1; 
	}

#endif

#endif


}


void uart_send_string(U8_T *p, U16_T length,U8_T port)
{
	U16_T i;
	

#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
	U16_T delay;
	U16_T count;
#endif
	
#if !(ARM_WIFI)
	if(port > 2) return;
	
#if (ARM_MINI || ARM_CM5)
	if(port == PORT_RS485_MAIN)
	{		
		memcpy(uart2_send_buf, p, length);
    uart2_sendbyte_num = length;
		USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
		// wait transfer ok
		count = 0;
		delay = uart2_sendbyte_num;
		while((uart2_sendbyte_num != 0) && (count < delay))
		{
			count++;
			vTaskDelay(2);//delay_ms(2);	
		}		
	}
	else	if(port == PORT_RS485_SUB)
	{
		memcpy(uart0_send_buf, p, length);
    uart0_sendbyte_num = length;
//    uart_num = 0 ;
		USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
		// wait transfer ok
		count = 0;
		delay = uart0_sendbyte_num;
		while((uart0_sendbyte_num != 0) && (count < delay))
		{
			count++;
			vTaskDelay(2);
			//delay_ms(1);	
		}
	}
	else	if(port == PORT_ZIGBEE)
	{
		memcpy(uart1_send_buf, p, length);
    uart1_sendbyte_num = length;
//    uart_num = 0 ;
		USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
				// wait transfer ok
		count = 0;
		delay = uart1_sendbyte_num *10;
		while((uart1_sendbyte_num != 0) && (count < delay))
		{
			count++;
			vTaskDelay(2);
			//delay_ms(1);	
		}
	}
#endif
	
#if (ASIX_MINI || ASIX_CM5)
	for(i = 0; i < length; i++)
	{
		uart_send_byte(p[i],port);		
	}

	if(port == 2 ) 
	{		
// 		delay is important!!!!
		if(Modbus.com_config[2] == MODBUS_MASTER)
		{
			if(length > 100)
			{	
				if(uart2_baudrate == UART_9600)		DELAY_Ms(36);	
				else if(uart2_baudrate == UART_19200)	DELAY_Ms(16);	  // 83.2MS
				else if(uart2_baudrate == UART_38400)	DELAY_Ms(8);	  
				else if(uart2_baudrate == UART_57600)	DELAY_Ms(4);	
				else if(uart2_baudrate == UART_76800)	DELAY_Ms(4);				
				else if(uart2_baudrate == UART_115200)	DELAY_Ms(2);	  // 83.2MS
			}
			else 
			{				
				// to be fixed, different bautrate, delay is differnt, need fix it
				if(uart2_baudrate >= UART_19200)
				{
					if(p[1] == 0x10)/* && (p[6] == 46)*/ //when mitiple-wirte 
					{						
						DELAY_Ms(length / 8 + 1);
					}
					else // when update firmware for tstat, DELAY_Ms(1), 
					{
						if((p[1] == 0x06) && (p[5] == 0x0a))  // plug and play
							DELAY_Ms(2);  // for minipanel
						else
							DELAY_Ms(1);  // for minipanel
					}
					
				}				
				
				else if(uart2_baudrate == UART_9600)
					DELAY_Ms(2);  // for minipanel
			}				
		}
		else if(Modbus.com_config[2] == MODBUS_SLAVE)
		{
			DELAY_Ms(length / 5 + 1);
		}
		else if(Modbus.com_config[2] == BACNET_SLAVE || Modbus.com_config[2] == BACNET_MASTER)
		{
			U16_T ttt;				
			if(uart2_baudrate == UART_19200) 
			{		
				DELAY_Ms(length / 5);
//				ttt = 100 * length; //	350 - 400		DELAY_Ms(length / 5);
//				while(ttt--)
//				{
//					DELAY_1_US();	
//				}
			}
			else if(uart2_baudrate == UART_38400)  // 700 -800
			{				
//				if(length > 30) 
//				{ttt = 50 * length;
//					vTaskDelay(ttt / 1000 + 1);
//				}
//				else
//				{
				ttt = 50 * length; //	350 - 400		DELAY_Ms(length / 5);
				while(ttt--)
				{
					DELAY_1_US();	
				}
//				}
					
			}			
			else if(uart2_baudrate == UART_57600)
			{	
				ttt = 25 * length; //	350 - 400		DELAY_Ms(length / 5);
				while(ttt--)
				{
					DELAY_1_US();	
				}	
			}		
			else if(uart2_baudrate == UART_115200)
			{	
				ttt = 12 * length; //	350 - 400		DELAY_Ms(length / 5);
				while(ttt--)
				{
					DELAY_1_US();	
				}	
			}					
		}
		else
		{
			if(uart2_baudrate == UART_9600)
				DELAY_Ms(2);
			else
				DELAY_Ms(1);
		}		
	}
#endif
	
#endif
	
#if ARM_WIFI
	if(port == 0)
	{		
//		memcpy(uart_sendB, p, length);
//	  sendbyte_numB = length;
//	  USART_ITConfig(UART4, USART_IT_TXE, ENABLE);//	
//		count = 0;
//		delay = sendbyte_numB *10;
//		while((sendbyte_numB != 0) && (count < delay))
//		{
//			count++;
//			vTaskDelay(2);
//		}	
		memcpy(uart0_send_buf, p, length);
    uart0_sendbyte_num = length;
//    uart_num = 0 ;
		USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
		// wait transfer ok
		count = 0;
		delay = uart0_sendbyte_num;
		while((uart0_sendbyte_num != 0) && (count < delay))
		{
			count++;
			vTaskDelay(2);
			//delay_ms(1);	
		}		
	}
#endif
}



void set_subnet_parameters(U8_T io, U8_T length,U8_T port)
{
	U16_T temp = 0;
	subnet_rec_package_size = length;  
	if(port > 2) return;
	if(port == 0 )
	{	
		uart0_rece_count = 0;
		memset(uart0_data_buffer,0,subnet_rec_package_size);
#if (ARM_MINI || ASIX_MINI)
		if((Modbus.mini_type == MINI_TINY) 
			|| (Modbus.mini_type == MINI_NEW_TINY) || (Modbus.mini_type == MINI_TINY_ARM))
			UART0_TXEN_TINY = io;	
		else
			UART0_TXEN_BIG = io;	
#endif
		
#if (ARM_CM5 || ASIX_CM5)		
		UART0_TXEN = io;
#endif
	}  
	if(port == 2 )
	{	
#if (ASIX_MINI || ASIX_CM5)		
		hsurRxCount = 0;
		memset(hsurRxBuffer,0,hsurRxCount);
#endif
		
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
		uart2_rece_count = 0;
		memset(uart2_data_buffer,0,subnet_rec_package_size);
#endif
		
#if ARM_MINI || ASIX_MINI
		if((Modbus.mini_type == MINI_TINY) 
			|| (Modbus.mini_type == MINI_NEW_TINY) || (Modbus.mini_type == MINI_TINY_ARM))
		{			
				UART2_TXEN_TINY = io;
		}
		else
		{
				UART2_TXEN_BIG = io;
		}
#endif
		
#if ARM_CM5
		UART2_TXEN = io;
#endif
	}
	if(port == 1 )
	{
		uart1_rece_count = 0;
		memset(uart1_data_buffer,0,subnet_rec_package_size);
	}
}
  



U8_T wait_subnet_response(U16_T nDoubleTick,U8_T port)
{
	U16_T i, length;
	U8_T cTaskWokenByPost = FALSE;
	U16_T far t[3];
	if(port > 2) return 0;

	
#if ASIX_CM5
	if(port == 0)
	{ 	
		for(i = 0; i < nDoubleTick; i++)
		{
			if((length = uart0_rece_count) >= subnet_rec_package_size)
			{	
				memcpy(subnet_response_buf,uart0_data_buffer,length);
				return length;
			}
			vTaskDelay(1);
		}
	} 
#endif 

#if (ARM_MINI || ASIX_MINI || ARM_CM5) 
	for(i = 0; i < nDoubleTick; i++)
	{
		if(port == 0 )
		{			
			if((length = uart0_rece_count) >= subnet_rec_package_size)
			{	
				memcpy(subnet_response_buf,uart0_data_buffer,length);
				return length;
			}		
		}
		if(port == 2 )
		{
#if ASIX_MINI	

				if((length = hsurRxCount) >= subnet_rec_package_size)
				{		
					memcpy(subnet_response_buf,hsurRxBuffer,length);
					return length;
				}
#endif
			
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			if((length = uart2_rece_count) >= subnet_rec_package_size)
			{	
				memcpy(subnet_response_buf,uart2_data_buffer,length);				
				return length;
			}	
#endif			
		}
		if(port == 1)
		{	
			if((length = uart1_rece_count) >= subnet_rec_package_size)
			{	
				memcpy(subnet_response_buf,uart1_data_buffer,length);
				return length;
			}
		}	
		
#if ASIX_MINI
		vTaskDelay(1);
#endif
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
		vTaskDelay(2);
#endif	
		
} 
	t[port] = i;
#endif

	timeout[port]++;	
	return 0;
}




#if (ARM_MINI || ASIX_MINI)


void zigbee_gsm_modbus_int_hander(void)
{
//	U8_T flag;
#if (ASIX_MINI || ASIX_CM5)
	if(RI1 == 1)
#endif
	
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
	if(USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
#endif
		{
		com_rx[1]++;
		flagLED_uart1_rx = 1; 

#if (ASIX_MINI || ASIX_CM5)
		RI1 = 0;	
		if(uart1_rece_count < MAX_BUF_LEN - 1)
			uart1_data_buffer[uart1_rece_count++] = SBUF1;
#endif

#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
		if(uart1_rece_count < MAX_BUF_LEN - 1)
			uart1_data_buffer[uart1_rece_count++] = USART_ReceiveData(USART2);      //??????????Y
#endif
		else
			uart1_rece_count = 0;		

		// check whether ZIGBEE moudle is attatched
		if((strstr(uart1_data_buffer,"zi")) != NULL && (flag_response_zigbee == 0))
		{
			uart1_rece_count = 0;	
			if(Modbus.com_config[1] != MODBUS_MASTER)
			{
				Modbus.com_config[1] = MODBUS_MASTER;
				E2prom_Write_Byte(EEP_COM0_CONFIG + 1, Modbus.com_config[1]);
			}
			flag_response_zigbee = 1;
		}

		// check if data is from zigbee moudle
		if((Modbus.com_config[1] == MODBUS_MASTER) ||( Modbus.com_config[1] == NOUSE))
		{	
			if(uart1_rece_count > 1)
			{
				//if((uart1_data_buffer[uart1_rece_count] == 0xff) && (uart1_data_buffer[uart1_rece_count] == 0x03))
					zigbee_exist = 0x74;
			}
		}			
		
	}
#if (ASIX_MINI || ASIX_CM5)
	else if(TI1 == 1)
#endif
	
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
	else  if( USART_GetITStatus(USART2, USART_IT_TXE) == SET )	
#endif
	{
		com_tx[1]++;
		uart1_transmit_finished = 1;
		flagLED_uart1_tx = 1; 
#if (ASIX_MINI || ASIX_CM5)
		TI1 = 0;
#endif
		
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)	

		USART_SendData(USART2, uart1_send_buf[uart1_send_count++]);

		if(uart1_send_count >= uart1_sendbyte_num)
		{
			while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
			USART_ClearFlag(USART2, USART_FLAG_TC);

			USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
			uart1_send_count = 0;
			uart1_sendbyte_num = 0;
			uart_serial_restart(1);
		}
	
#endif
		
	}
	return;
}
#endif


void modubs_main_uart_int_hander()
{
	if(Modbus.main_port == 0)
	{		
#if (ASIX_MINI || ASIX_CM5)
		if(RI0 == 1)
#endif 
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
		if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET)  
#endif
		{
#if (ASIX_MINI || ASIX_CM5)			
			RI0 = 0;
#endif 
			flagLED_uart0_rx = 1;
			com_rx[0]++;
			if(uart0_rece_count < MAX_BUF_LEN - 1)
#if (ASIX_MINI || ASIX_CM5)	
				uart0_data_buffer[uart0_rece_count++] = SBUF0;
#endif 
		
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			{
				USART_ClearFlag(USART1, USART_FLAG_RXNE);
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
		    uart0_data_buffer[uart0_rece_count++] = USART_ReceiveData(USART1);/*((USART1->DR);*/      //??????????Y
			}
#endif
			
			else
			{	
				uart_serial_restart(0);
			}

			
			
			if(uart0_rece_count == 1)
			{	
				uart0_rece_size = MAX_BUF_LEN;	
				uart0_serial_receive_timeout_count = SERIAL_RECEIVE_TIMEOUT;				
			}
			else if(uart0_rece_count == 3)
			{	
				 if(uart0_data_buffer[1] == CHECKONLINE)
						uart0_rece_size = 6;
			}
			else if(uart0_rece_count == 4)
			{
				if((((U16_T)(uart0_data_buffer[2] << 8) + uart0_data_buffer[3]) == 0x0a) && (uart0_data_buffer[1] == WRITE_VARIABLES))
				{
					uart0_rece_size = DATABUFLEN_SCAN;
					uart0_serial_receive_timeout_count = SERIAL_RECEIVE_TIMEOUT;
				}
//				if((uart0_data_buffer[0] == 0xff) && (uart0_data_buffer[1] == 0x19))
//				{  
//					uart0_rece_size = 6;	
//				}
//				if(uart0_data_buffer[1] == 0x12)  // for customer
//				{
//// id (1 byte ) + function code (1 byte) + len ( 1 byte )
//// C0 + DO status ( 2 bytes)     -- fixed
//// C4 + len(1 byte) + AO status 				-- fixed
//// C6 + len(1 byte PInum * 2) + PI					-- not fixed
//// C8 + logic control		-- not fixed
//					uart0_rece_size =  uart0_data_buffer[2] + 5;

//				}

			}
			else if(uart0_rece_count == 7)
			{
				if((uart0_data_buffer[1] == READ_VARIABLES) || (uart0_data_buffer[1] == WRITE_VARIABLES))
				{
					uart0_rece_size = 8;
					//dealwithTag = 1;
				}
				else if(uart0_data_buffer[1] == MULTIPLE_WRITE)
				{
					uart0_rece_size = uart0_data_buffer[5] * 2 + 9;	
					uart0_serial_receive_timeout_count = main_data_buffer[6] + 8;
				}
				else
					uart0_rece_size = MAX_BUF_LEN;
			}
			else if(uart0_rece_count >= uart0_rece_size)
			{		
//				uart0_serial_receive_timeout_count = 0;
				uart0_dealwithTag = VALID_PACKET;
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
				uart0_rece_count = 0;
#endif
			}

		}
#if (ASIX_MINI || ASIX_CM5)
		else if(TI0 == 1)
#endif
		
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
		else  if( USART_GetITStatus(USART1, USART_IT_TXE) == SET  )
#endif
		{		
			uart0_transmit_finished = 1;
			flagLED_uart0_tx = 1; 
			com_tx[0]++;
#if (ASIX_MINI || ASIX_CM5)
			TI0 = 0;
#endif

#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			USART_SendData(USART1, uart0_send_buf[uart0_send_count++]);
			Timer_Silence_Reset();
			if(uart0_send_count >= uart0_sendbyte_num)
			{
				while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
				//USART_ClearFlag(USART1, USART_FLAG_TC);
				USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
				uart0_send_count = 0;
				uart0_sendbyte_num = 0;				
				uart_serial_restart(0);
			}
#endif	
		}
	}
	else if(Modbus.main_port == 1)
	{
#if (ASIX_MINI || ASIX_CM5)
		if(RI1 == 1)
#endif
		
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
		if(USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
#endif
		{
			com_rx[1]++;
			flagLED_uart1_rx = 1; 
		//	uart1_heartbeat = 0;
#if (ASIX_MINI || ASIX_CM5)
			RI1 = 0;
#endif

#if (ARM_MINI || ARM_CM5 || ARM_WIFI)

#endif			
			if(uart1_rece_count < MAX_BUF_LEN - 1)
#if (ASIX_MINI || ASIX_CM5)
				uart1_data_buffer[uart1_rece_count++] = SBUF1;
#endif
			
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			{
			USART_ClearFlag(USART2, USART_FLAG_RXNE);
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);

			 uart1_data_buffer[uart1_rece_count++] = USART_ReceiveData(USART2);      //??????????Y
			}
#endif
			else
				uart_serial_restart(1);
	
//			uart1_serial_receive_timeout_count = 2;//SERIAL_RECEIVE_TIMEOUT;
			if(uart1_rece_count == 1)
			{	
				uart1_rece_size = 8;
//				uart1_serial_receive_timeout_count = 2;//SERIAL_RECEIVE_TIMEOUT;
			}
			else if(uart1_rece_count == 4)
			{
				if((((U16_T)(uart1_data_buffer[2] << 8) + uart1_data_buffer[3]) == 0x0a) && (uart1_data_buffer[1] == WRITE_VARIABLES))
				{
					uart1_rece_size = DATABUFLEN_SCAN;
//					uart1_serial_receive_timeout_count = 2;//SERIAL_RECEIVE_TIMEOUT;	
				}				
			}
			else if(uart1_rece_count == 7)
			{
				if(uart1_data_buffer[1] == MULTIPLE_WRITE)
				{
					uart1_rece_size = uart1_data_buffer[5] * 2 + 9;
				}
			}
			else if(uart1_rece_count >= uart1_rece_size)
			{	 
//				uart1_serial_receive_timeout_count = 0;
				uart1_dealwithTag = VALID_PACKET;
			}
		}
#if (ASIX_MINI || ASIX_CM5)
		else if(TI1 == 1)
#endif 
		
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
		else  if( USART_GetITStatus(USART2, USART_IT_TXE) == SET  )
#endif
		{

#if (ASIX_MINI || ASIX_CM5)			
			TI1 = 0;
#endif			

			com_tx[1]++;
			uart1_transmit_finished = 1;
			flagLED_uart1_tx = 1; 
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			{
				 USART_SendData(USART2, uart1_send_buf[uart1_send_count++]);
				 
				if(uart1_send_count >= uart1_sendbyte_num)
				{
					
					while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
					USART_ClearFlag(USART2, USART_FLAG_TC);
					
					USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
					uart1_send_count = 0;
					uart1_sendbyte_num = 0;
					uart_serial_restart(1);
				}
			}  
#endif			
		}
	}
#if (ARM_MINI || ARM_CM5)
	else if(Modbus.main_port == 2)
	{
		if(USART_GetITStatus(USART3, USART_IT_RXNE) == SET)   //????D??
		{
			com_rx[2]++;
			flagLED_uart2_rx = 1;
			if(uart2_rece_count < 250)
				uart2_data_buffer[uart2_rece_count++] = USART_ReceiveData(USART3);//(USART1->DR);      //??????????Y
			else
			{				
				uart_serial_restart(PORT_RS485_MAIN);
			}
			USART_ClearFlag(USART1, USART_FLAG_RXNE);
      USART_ClearITPendingBit(USART1, USART_IT_RXNE);

			if(uart2_rece_count == 1)
			{
				 // This starts a timer that will reset communication.  If you do not
				 // receive the full packet, it insures that the next receive will be fresh.
				 // The timeout is roughly 7.5ms.  (3 ticks of the hearbeat)
				 uart2_rece_size = 250;
				 uart2_serial_receive_timeout_count = SERIAL_RECEIVE_TIMEOUT;
			}
			else if(uart2_rece_count == 3 )
			{
				 if(uart2_data_buffer[1] == CHECKONLINE)
						uart2_rece_size = 6;
			}
			else if(uart2_rece_count == 4)
			{
				 //check if it is a scan command
				 if((((vu16)(uart2_data_buffer[2] << 8) + uart2_data_buffer[3]) == 0x0a) && (uart2_data_buffer[1] == WRITE_VARIABLES))
				 {
						uart2_rece_size = DATABUFLEN_SCAN;
						uart2_serial_receive_timeout_count = SERIAL_RECEIVE_TIMEOUT;   
				 }
			}
			else if(uart2_rece_count == 7)
			{
				if((uart2_data_buffer[1] == READ_VARIABLES) || (uart2_data_buffer[1] == WRITE_VARIABLES))
				{
					uart2_rece_size = 8;
					//dealwithTag = 1;
				}
				else if(uart2_data_buffer[1] == MULTIPLE_WRITE)
				{
					uart2_rece_size = uart2_data_buffer[6] + 9;
					uart2_serial_receive_timeout_count = main_data_buffer[6] + 8;
				}
				else
					uart2_rece_size = 250;
			}
//				else if(uart2_data_buffer[0] == 0x55 && uart2_data_buffer[1] == 0xff && uart2_data_buffer[2] == 0x01 && uart2_data_buffer[5] == 0x00 && uart2_data_buffer[6] == 0x00)
//				{//bacnet protocal detected
//						Modbus.com_config[2] = BACNET_SLAVE;
//						E2prom_Write_Byte(EEP_COM2_CONFIG, BACNET_SLAVE);
//						Recievebuf_Initialize(2); 
//						uart_serial_restart(2);						
//				}
			else if(uart2_rece_count == uart2_rece_size)      
			{
				 // full packet received - turn off serial timeout
//					uart2_serial_receive_timeout_count = 0;
				uart2_dealwithTag = 2;      // making this number big to increase delay
				uart2_rece_count = 0;
			}
   }
   else  if( USART_GetITStatus(USART3, USART_IT_TXE) == SET  )
   {
		 	com_tx[2]++;
			uart2_transmit_finished = 1;
			flagLED_uart2_tx = 1; 


			USART_SendData(USART3, uart2_send_buf[uart2_send_count++]);
			Timer_Silence_Reset();

			if(uart2_send_count >= uart2_sendbyte_num)
			{
				while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
				USART_ClearFlag(USART3, USART_FLAG_TC);

				USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
				uart2_send_count = 0;
				uart2_sendbyte_num = 0;
				uart_serial_restart(2);
			}
		}			
	}
#endif	
	return;
}





#if (ARM_MINI || ARM_CM5 || ARM_WIFI)


extern uint8_t Receive_Buffer_Data0[512];
extern FIFO_BUFFER Receive_Buffer0;
uint8_t CRC_Calc_Header(
    uint8_t dataValue,
    uint8_t crcValue);
void mstp_ptp_int_handler(uint8_t port)
{
		uint8_t data_byte;
		uint16_t i;
		
		if(port == 2)
		{
			Timer_Silence_Reset();
			if(USART_GetITStatus(USART3, USART_IT_RXNE) == SET)   //????D??
			{	
				flagLED_uart2_rx = 1;
				
			 /* we received a byte */
				data_byte = USART_ReceiveData(USART3);//(USART1->DR);      //??????????Y   
				
				FIFO_Put(&Receive_Buffer0, data_byte);
				com_rx[2]++;
			}
			
			if( USART_GetITStatus(USART3, USART_IT_TXE) == SET  )		
			{
				Timer_Silence_Reset();
				USART_SendData(USART3, uart2_send_buf[uart2_send_count++]);
				if(uart2_send_count >= uart2_sendbyte_num)
				{
					while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
					USART_ClearFlag(USART3, USART_FLAG_TC);
					USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
					uart2_send_count = 0;
					uart2_sendbyte_num = 0;
					uart_serial_restart(2);
				}

				uart2_transmit_finished = 1;
				flagLED_uart2_tx = 1; 
				com_tx[2]++;
			}
		}
		else if(port == 0)
		{
			Timer_Silence_Reset();
			if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET)   //????D??
			{	
				flagLED_uart0_rx = 1;
				com_rx[0]++;

			 /* we received a byte */
				data_byte = USART_ReceiveData(USART1);//(USART1->DR);      //??????????Y   
				FIFO_Put(&Receive_Buffer0, data_byte);

			}
			
			if( USART_GetITStatus(USART1, USART_IT_TXE) == SET  )		
			{
				Timer_Silence_Reset();
				USART_SendData(USART1, uart0_send_buf[uart0_send_count++]);
				if(uart0_send_count >= uart0_sendbyte_num)
				{
					while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
					USART_ClearFlag(USART1, USART_FLAG_TC);
					USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
					uart0_send_count = 0;
					uart0_sendbyte_num = 0;
					uart_serial_restart(0);
				}

				uart0_transmit_finished = 1;
				flagLED_uart0_tx = 1; 
				com_tx[0]++;
			}
		}
		
		return;

}


void USART3_IRQHandler(void) 
{
	if(Modbus.com_config[2] == MODBUS_SLAVE )
	{	
		Modbus.main_port = 2;
		modubs_main_uart_int_hander();
	}	
	else if(Modbus.com_config[2] == BACNET_SLAVE || Modbus.com_config[2] == BACNET_MASTER)
	{
		mstp_ptp_int_handler(2);
	}	
	else // if(Modbus.com_config[2] == MODBUS_MASTER )
	{
		Modbus.sub_port = 2;
		modubs_sub1_uart_int_hander();
	}	
}
#endif


//------------------------serialport ----------------------------------
//	serial port interrupt , deal with all action about serial port. include receive data and 
//		send data and deal with interal.
#if (ASIX_MINI || ASIX_CM5)
void handle_uart1_RX(void) interrupt 6
#endif

#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
void USART2_IRQHandler(void) 
#endif
{

#if (ARM_MINI || ASIX_MINI) 
	if((Modbus.com_config[1] == MODBUS_MASTER) ||( Modbus.com_config[1] == NOUSE) || (Modbus.com_config[1] == RS232_METER))	
	{
		zigbee_gsm_modbus_int_hander();
	}
	else if(Modbus.com_config[1] == MODBUS_SLAVE)  // for main port 232, test port
#endif
	{
		Modbus.main_port = 1;
		modubs_main_uart_int_hander();
	}
}

void modubs_sub1_uart_int_hander(void)
{
	U16_T crc_val;
	
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
	if(Modbus.sub_port == 0)
	{
#endif 
#if (ASIX_MINI || ASIX_CM5)
	if(RI0 == 1)
#endif
	
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
	if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET)   //????D??
#endif
	{	
		flagLED_uart0_rx = 1;
		com_rx[0]++;
#if (ASIX_MINI || ASIX_CM5)
		RI0 = 0;
#endif
		
		if(uart0_rece_count < MAX_BUF_LEN - 1)
#if (ASIX_MINI || ASIX_CM5)	
			uart0_data_buffer[uart0_rece_count++] = SBUF0;
#endif

#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
		  uart0_data_buffer[uart0_rece_count++] = USART_ReceiveData(USART1);//(USART1->DR);      //??????????Y     
#endif
		else
		{			
			uart_serial_restart(0);
		}	
		
		// if comport config is not fixed, it is changed to MODBUS_SLAVE automatically
		if(Modbus.fix_com_config == 0)
		{
			if(((uart0_rece_count == 6) && (uart0_data_buffer[0] == 0xff) && (uart0_data_buffer[1] == 0x19)) \
			|| ((uart0_rece_count == 8) && (uart0_data_buffer[1] == 0x03) && (uart0_data_buffer[7] != 0))) // receive data
			{							
				crc_val = crc16(uart0_data_buffer, uart0_rece_count - 2);
				if(crc_val == (uart0_data_buffer[uart0_rece_count - 2] << 8) + uart0_data_buffer[uart0_rece_count - 1])
				{
					Modbus.com_config[0] = MODBUS_SLAVE;
					Count_com_config();
					uart_serial_restart(0);
				}						
			}
		}
  }
#if (ASIX_MINI || ASIX_CM5)
	else if(TI0 == 1)
#endif
	
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
		else if(USART_GetITStatus(USART1, USART_IT_TXE) == SET)
#endif		
		{
#if (ASIX_MINI || ASIX_CM5)
			TI0 = 0;
#endif

#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			USART_SendData(USART1, uart0_send_buf[uart0_send_count++]);
			Timer_Silence_Reset();

			if(uart0_send_count >= uart0_sendbyte_num)
			{	
				while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
				USART_ClearFlag(USART1, USART_FLAG_TC);
				
				USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
				uart0_send_count = 0;
				uart0_sendbyte_num = 0;
				uart_serial_restart(0);
			}
#endif
			uart0_transmit_finished = 1;
			flagLED_uart0_tx = 1; 
			com_tx[0]++;
		}
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
	}
	else if(Modbus.sub_port == 2)
	{
		if(USART_GetITStatus(USART3, USART_IT_RXNE) == SET)   //????D??
		{	
			flagLED_uart2_rx = 1;
			com_rx[2]++;
			if(uart2_rece_count < MAX_BUF_LEN)
				uart2_data_buffer[uart2_rece_count++] = USART_ReceiveData(USART3);//(USART1->DR);      //??????????Y   
			else
				uart_serial_restart(2);

					// if comport config is not fixed, it is changed to MODBUS_SLAVE automatically
			if(Modbus.fix_com_config == 0)
			{
				if(((uart2_rece_count == 6) && (uart2_data_buffer[0] == 0xff) && (uart2_data_buffer[1] == 0x19)) \
				|| ( (uart2_rece_count == 8) && (uart2_data_buffer[1] == 0x03) && (uart2_data_buffer[7] != 0))) // receive data
				{		

					crc_val = crc16(uart2_data_buffer, uart2_rece_count - 2);
					if(crc_val == (uart2_data_buffer[uart2_rece_count - 2] << 8) + uart2_data_buffer[uart2_rece_count - 1])
					{
						Modbus.com_config[2] = MODBUS_SLAVE;
						Count_com_config();
						uart_serial_restart(2);
					}						
				}
	//			if(uart2_data_buffer[0] == 0x55 && uart2_data_buffer[1] == 0xff && uart2_data_buffer[2] == 0x01 && uart2_data_buffer[5] == 0x00 && uart2_data_buffer[6] == 0x00)
	//			{//bacnet protocal detected
	//					Modbus.com_config[2] = MAIN_MSTP;
	//					E2prom_Write_Byte(EEP_COM2_CONFIG, MAIN_MSTP);
	//					Recievebuf_Initialize(2); 
	//					uart_serial_restart(2);
	//			}
			}
		}

		if( USART_GetITStatus(USART3, USART_IT_TXE) == SET  )		
		{

			USART_SendData(USART3, uart2_send_buf[uart2_send_count++]);
			Timer_Silence_Reset();

			if(uart2_send_count >= uart2_sendbyte_num)
			{
				while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
				USART_ClearFlag(USART3, USART_FLAG_TC);
				USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
				uart2_send_count = 0;
				uart2_sendbyte_num = 0;
				uart_serial_restart(2);
			}

			uart2_transmit_finished = 1;
			flagLED_uart2_tx = 1; 
			com_tx[2]++;
		}
	}
#endif
	return;

}

#if (ASIX_MINI || ASIX_CM5)
void handle_uart0_RX(void) interrupt 4
#endif

#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
void USART1_IRQHandler(void)
#endif
{
	if(Modbus.com_config[0] == MODBUS_SLAVE )
	{	
		Modbus.main_port = 0;
		modubs_main_uart_int_hander();
	}
#if (ARM_MINI || ARM_WIFI)
	else if(Modbus.com_config[0] == BACNET_SLAVE || Modbus.com_config[0] == BACNET_MASTER)
	{	
		Test[10]++;
		mstp_ptp_int_handler(0);
	}
#endif
	else
	{
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
		Modbus.sub_port = 0;
#endif
		modubs_sub1_uart_int_hander();
	}

}

void uart_serial_restart(U8_T port)
{
	if(port > 2) return;
	if(port == 0)
	{
		uart0_rece_count = 0;
		uart0_dealwithTag = 0;
#if (ARM_MINI || ASIX_MINI) 
		if((Modbus.mini_type == MINI_TINY) 
			|| (Modbus.mini_type == MINI_NEW_TINY) || (Modbus.mini_type == MINI_TINY_ARM))
			UART0_TXEN_TINY = RECEIVE;
		else
			UART0_TXEN_BIG = RECEIVE;
#endif
		
#if (ARM_CM5 || ASIX_CM5) 
		UART0_TXEN = RECEIVE;
#endif
		
#if ARM_WIFI 
		UART0_TXEN_BIG = RECEIVE;
#endif
	}

	else if(port == 1)
	{
	uart1_rece_count = 0;
	uart1_dealwithTag = 0;
	}
	else if(port == 2)
	{
		uart2_rece_count = 0;
		uart2_dealwithTag = 0;
#if (ARM_MINI || ASIX_MINI)
		if((Modbus.mini_type == MINI_TINY) 
			|| (Modbus.mini_type == MINI_NEW_TINY) || (Modbus.mini_type == MINI_TINY_ARM))
		{			
				UART2_TXEN_TINY = RECEIVE;
		}
		else if(Modbus.mini_type == MINI_VAV)
		{			
				UART2_TXEN_VAV = RECEIVE;
		}
		else
		{
				UART2_TXEN_BIG = RECEIVE;
		}

#endif	

#if ARM_CM5
	UART2_TXEN = RECEIVE;
#endif
	}


}


#if !(ARM_WIFI)

// ------------------------dealwithdata -------------------------
// the routine dealwith data ,it has three steps.
// the 3 step is : 1 prepare to send data and init crc for next Tim
//				   2 dealwith interal
//                 3 organize the data of sending, and send data.
U8_T count_master_connect; 
void main_dealwithData(void)
{
	U16_T far address;
	U8_T count_lose_master;
//	U8_T frametype;
//	
	portTickType xDelayPeriod = ( portTickType ) 25 / portTICK_RATE_MS;
	task_test.enable[10] = 1;
	count_lose_master = 0;
	for( ; ;)
	{ 	

		vTaskDelay(xDelayPeriod);
		current_task = 10;
		task_test.count[10]++;
#if (ASIX_MINI || ASIX_CM5)
		if(uart2_timeout == 0) 	{ UART_Init(2);}	 
#endif
		
		if(Modbus.com_config[0] == MODBUS_SLAVE || Modbus.com_config[1] == MODBUS_SLAVE || Modbus.com_config[2] == MODBUS_SLAVE) 	
		{
#if (ASIX_MINI || ASIX_CM5)
			if(Modbus.com_config[2] == MODBUS_SLAVE)
			{					
				uart_serial_restart(2);
				if(hsurRxCount > 0) // receive data
				{		
//					if(hsurRxBuffer[0] == 0x55 && hsurRxBuffer[1] == 0xff && hsurRxBuffer[2] == 0x01 && hsurRxBuffer[5] == 0x00 && hsurRxBuffer[6] == 0x00)
//					{// mstp
//							Modbus.com_config[2] = MAIN_MSTP;
//							Recievebuf_Initialize(2);						
//					}
					
					if((hsurRxBuffer[0] == 0xff && hsurRxBuffer[1] == 0x19 && hsurRxCount == 6)	 // scan cmd
					||((hsurRxBuffer[1] == 0x03 || hsurRxBuffer[1] == 0x06) && hsurRxCount == 8)
					||(hsurRxBuffer[1] == 0x10 && hsurRxCount == hsurRxBuffer[5] * 2 + 9)
					||(hsurRxBuffer[1] == 0x12)	  // for customer
					)  // read or write
					{ 						
						memcpy(main_data_buffer,hsurRxBuffer,hsurRxCount); 						
						main_dealwithTag = VALID_PACKET;
						main_rece_size = hsurRxCount;
						Modbus.main_port = 2;						
					}
					// check whehter this packet is finished
					if(hsurRxBuffer[1] == 0x10 && hsurRxCount < hsurRxBuffer[5] * 2 + 9)
					{
						// miti-write need long time,if not finish wait the second packet
					}						
					else
					{
						hsurRxCount = 0;
						memset(hsurRxBuffer,0,hsurRxCount);
					}
				}
			}
#endif
#if (ARM_MINI || ARM_CM5 )
			if(Modbus.main_port == 2)	
			{				
				memcpy(main_data_buffer,uart2_data_buffer,MAX_BUF_LEN);
				main_dealwithTag = uart2_dealwithTag;
				main_rece_size = uart2_rece_size;
			}
#endif
			if(Modbus.main_port == 0)	
			{	
				memcpy(main_data_buffer,uart0_data_buffer,MAX_BUF_LEN);
				main_dealwithTag = uart0_dealwithTag;
				main_rece_size = uart0_rece_size;
			}
			if(Modbus.main_port == 1)	
			{	
				memcpy(main_data_buffer,uart1_data_buffer,MAX_BUF_LEN);
				main_dealwithTag = uart1_dealwithTag;
				main_rece_size = uart1_rece_size;
			}

			if(main_dealwithTag == VALID_PACKET)
			{	
				address = ((U16_T)main_data_buffer[2] << 8) + main_data_buffer[3];		
				if (checkData(address))
				{
					if(main_data_buffer[0] ==  Modbus.address || main_data_buffer[0] == 255)
					{	
						uart_init_send_com(Modbus.main_port); 
#if 0									
						if(main_data_buffer[1] == 0x12)	// speical commad
						{
							Response_Speical_Logic(main_data_buffer); 	
						}
						else
#endif
						{							
							main_init_crc16(); 							
							main_responseData();
						
						}						
					}	
#if RS232_TO_RS485					
					else
					{			
#if ASIX_CM5
			Modbus.sub_port = 0;
#endif
						
#if (ARM_MINI || ASIX_MINI || ARM_CM5)	
			
			if(Modbus.mini_type == MINI_VAV)
			{			
					Modbus.sub_port = 0;
			}
			else
			{
				U8_T i;	
				Modbus.sub_port = 255;
							
				if(Modbus.com_config[0] == MODBUS_MASTER)
					Modbus.sub_port = 0;
				else if(Modbus.com_config[1] == MODBUS_MASTER)
					Modbus.sub_port = 1;
				else if(Modbus.com_config[2] == MODBUS_MASTER)
					Modbus.sub_port = 2;
				
				for(i = 0;i <  sub_no ;i++)
				{
					if(main_data_buffer[0] == uart0_sub_addr[i])
					{
						Modbus.sub_port = 0;
						continue;
					}
					else if(main_data_buffer[0] == uart2_sub_addr[i])
					{
						Modbus.sub_port = 2;
						continue;
					}
					else if(main_data_buffer[0] == uart1_sub_addr[i])
					{	
						Modbus.sub_port = 1;
						continue;
					}
				}	
			}		 
							
#endif
					if(Modbus.sub_port != 255)
					{
						if(flag_resume_rs485 == 0 || flag_resume_rs485 == 2)
						{
							vTaskSuspend(Handle_Scan); 
						}
#if (ARM_MINI || ASIX_MINI || ARM_CM5)	
						vTaskSuspend(xHandler_Output); 
#endif
						vTaskSuspend(xHandleCommon);
						vTaskSuspend(xHandleBacnetControl);
						vTaskSuspend(xHandleMornitor_task);
#if MSTP
						vTaskSuspend(xHandleMSTP);			
#endif 
						
#if (ARM_MINI || ASIX_MINI)	
				if((Modbus.mini_type == MINI_BIG) || (Modbus.mini_type == MINI_BIG_ARM)
					 || (Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM)
						|| (Modbus.mini_type == MINI_TINY))
					vTaskSuspend(xHandler_SPI);
#endif	
						
					Response_MAIN_To_SUB(main_data_buffer,main_rece_size - 2,Modbus.sub_port);
					flag_resume_rs485 = 1;  // suspend rs485 task, resume it later, make the communication smoothly
					resume_rs485_count = 0;	
#if (ARM_MINI || ASIX_MINI || ARM_CM5)	
					vTaskResume(xHandler_Output); 
#endif
					vTaskResume(xHandleCommon);
					vTaskResume(xHandleBacnetControl);
					vTaskResume(xHandleMornitor_task);
#if MSTP
					vTaskResume(xHandleMSTP);			
#endif 


#if (ARM_MINI || ASIX_MINI)	
				if((Modbus.mini_type == MINI_BIG) || (Modbus.mini_type == MINI_BIG_ARM)
					 || (Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM)
						|| (Modbus.mini_type == MINI_TINY))
					vTaskResume(xHandler_SPI);
#endif	   

						}
					}		
#endif					
				}  
				else
					packet_error[Modbus.main_port]++;
								
			// Restart the serial receive.
				uart_serial_restart(Modbus.main_port); 
				main_dealwithTag = 0;
				count_lose_master = 0;
			}
			else
			{	
				count_lose_master++;					
				if(count_lose_master > 100)
				{// if slave mode, cant always restart uart
					// if lose communication for 5s, reset it
					count_lose_master = 0;
					uart_serial_restart(Modbus.main_port);	
				}
				
				main_dealwithTag = 0;  
			}
			
			if(Modbus.com_config[2] == MODBUS_SLAVE)
			{
				if(uart2_serial_receive_timeout_count>0)  
				{
						uart2_serial_receive_timeout_count -- ; 
						if(uart2_serial_receive_timeout_count == 0)
						{
							uart_serial_restart(2);
						}
				}
			}
			if(Modbus.com_config[0] == MODBUS_SLAVE)
			{
				if(uart0_serial_receive_timeout_count>0)  
				{
						uart0_serial_receive_timeout_count -- ; 
						if(uart0_serial_receive_timeout_count == 0)
						{
							uart_serial_restart(0);
						}
				}
			}
		}

		//500 ms check whether master is connecting
		count_master_connect++;
		if(count_master_connect > 10)
		{
			count_master_connect = 0;
			if(Modbus.com_config[0] == MODBUS_MASTER || Modbus.com_config[0] == NOUSE)
			{	 
				// when it is master or noused, try to receive data in free time
				// if it is using now, cant reset it.
				if(cSemaphoreTake(sem_subnet_tx_uart0, 0) == pdTRUE)
				{
					if(flag_resume_rs485 == 0 || flag_resume_rs485 == 2)
					{
						uart_serial_restart(0);
					}
				}
				cSemaphoreGive(sem_subnet_tx_uart0);
			}		
			
			
			if(Modbus.com_config[1] == MODBUS_MASTER || Modbus.com_config[1] == NOUSE)
			{	 
				if(cSemaphoreTake(sem_subnet_tx_uart1, 0) == pdTRUE)
				{
					if(flag_resume_rs485 == 0 || flag_resume_rs485 == 2)
						uart_serial_restart(1);	
				}		
				cSemaphoreGive(sem_subnet_tx_uart1);				
			}	
			
			 // if UART2 is not slave		
			if(Modbus.com_config[2] == MODBUS_MASTER || Modbus.com_config[2] == NOUSE)
			{	 
				U16_T crc_val;	
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
				if(cSemaphoreTake(sem_subnet_tx_uart2, 0) == pdTRUE)
				{
					if(flag_resume_rs485 == 0 || flag_resume_rs485 == 2)
					{
							uart_serial_restart(2);
					}
				}
				cSemaphoreGive(sem_subnet_tx_uart2);
#endif
				
#if (ASIX_MINI || ASIX_CM5)									
				if(cSemaphoreTake(sem_subnet_tx_uart2, 0) == pdTRUE)
				if(flag_resume_rs485 == 0 || flag_resume_rs485 == 2)
				{					
					uart_serial_restart(2);
						

						if((hsurRxCount == 6) && (hsurRxBuffer[0] == 0xff) && (hsurRxBuffer[1] == 0x19) \
							|| ((hsurRxCount == 8) && (hsurRxBuffer[1] == 0x03) && (hsurRxBuffer[7] != 0))) // receive data
					//	 UART2 recieved a redundant byte 0 when hsurRxCount is 8 and the last byte is 0
						{							
							crc_val = crc16(hsurRxBuffer, hsurRxCount - 2);
												
							if(crc_val == (hsurRxBuffer[hsurRxCount - 2] << 8) + hsurRxBuffer[hsurRxCount - 1])
							{
									Modbus.com_config[2] = MODBUS_SLAVE;							
									Count_com_config();
							}						
						}
						else
							hsurRxCount = 0;
					
				}
				cSemaphoreGive(sem_subnet_tx_uart2);
#endif
			}			
		}				
	}
	
}

#else

// FOR WIFI
U8_T count_master_connect; 
void main_dealwithData(void)
{
	U16_T far address;
	U8_T count_lose_master;
//	U8_T frametype;
//	
	portTickType xDelayPeriod = ( portTickType ) 25 / portTICK_RATE_MS;
	task_test.enable[10] = 1;
	count_lose_master = 0;
	Modbus.main_port = 0;
	for( ; ;)
	{ 	

		vTaskDelay(xDelayPeriod);
		current_task = 10;
		task_test.count[10]++;

		if(Modbus.com_config[0] == MODBUS_SLAVE) 	
		{
			memcpy(main_data_buffer,uart0_data_buffer,MAX_BUF_LEN);
			main_dealwithTag = uart0_dealwithTag;
			main_rece_size = uart0_rece_size;	
			if(main_dealwithTag == VALID_PACKET)
			{	
				address = ((U16_T)main_data_buffer[2] << 8) + main_data_buffer[3];		
				if (checkData(address))
				{
					if(main_data_buffer[0] ==  Modbus.address || main_data_buffer[0] == 255)
					{	
						uart_init_send_com(0);
						main_init_crc16(); 							
						main_responseData();						
					}				
				}  
				else
					packet_error[0]++;
								
			// Restart the serial receive.
				uart_serial_restart(0); 
				main_dealwithTag = 0;
				count_lose_master = 0;
			}
			else
			{	
				count_lose_master++;					
				if(count_lose_master > 100)
				{// if slave mode, cant always restart uart
					// if lose communication for 5s, reset it
					count_lose_master = 0;
					uart_serial_restart(0);	
				}
				
				main_dealwithTag = 0;  
			}
			
			if(Modbus.com_config[0] == MODBUS_SLAVE)
			{
				if(uart0_serial_receive_timeout_count>0)  
				{
					uart0_serial_receive_timeout_count -- ; 
					if(uart0_serial_receive_timeout_count == 0)
					{
						uart_serial_restart(0);
					}
				}
			}
		}

	}
	
}
#endif


//---------------------checkdata ----------------------
//This function calculates and verifies the checksum
U8_T checkData(U8_T address)
{
	U16_T crc_val;
	U8_T minaddr,maxaddr, variable_delay;
//	U8_T randval;

	U8_T i;

	if(main_data_buffer[1] == 0x12)	  // for customer, dont check 
		return TRUE;
//	if(main_data_buffer[0] != 255 && main_data_buffer[0] !=  address && main_data_buffer[0] != 0)
//		return FALSE;
	if(main_data_buffer[1] == CHECKONLINE)
	{
		crc_val = crc16(main_data_buffer, 4);
		if(crc_val != (main_data_buffer[4] << 8) + main_data_buffer[5])
		{
			return FALSE;
		}
		minaddr = (main_data_buffer[2] >= main_data_buffer[3]) ? main_data_buffer[3] : main_data_buffer[2];	
		maxaddr = (main_data_buffer[2] >= main_data_buffer[3]) ? main_data_buffer[2] : main_data_buffer[3];	
		if( address < minaddr ||  address > maxaddr)
		{
			return FALSE;
		}
		else
		{
			variable_delay = rand() % 20;
			for (i = 0; i < variable_delay; i++);
			return TRUE;
		}
	}
	if((main_data_buffer[1] != READ_VARIABLES) && (main_data_buffer[1] != WRITE_VARIABLES) && (main_data_buffer[1] != MULTIPLE_WRITE))
	{	
		return FALSE;  
	}
	#if 0

	dont support plug_n_play fucntion
 /*******************************************************************************************************/
  //copy from tstat code the add PLUG_N_PLAY function
  if(main_data_buffer[2]*256 + main_data_buffer[3] ==  MODBUS_ADDRESS_PLUG_N_PLAY)
	{
		if(main_data_buffer[1] == WRITE_VARIABLES)
		{
			if(main_data_buffer[6] != Modbus.serialNum[0]) 
			return 0;
			if(main_data_buffer[7] != Modbus.serialNum[1]) 
			return 0;
			if(main_data_buffer[8] != Modbus.serialNum[2])  
			return 0;
			if(main_data_buffer[9] != Modbus.serialNum[3]) 
			return 0;
		}
		if (main_data_buffer[1] == READ_VARIABLES)
		{
			randval = rand() % 10 / 2 ;
		}

		if(randval != RESPONSERANDVALUE)
		{
//mhf:12-29-05 if more than 5 times does not response read register 10,reponse manuly.
			rand_read_ten_count++;
			if(rand_read_ten_count%5 == 0)
			{
				rand_read_ten_count = 0;
				randval = RESPONSERANDVALUE;
				variable_delay = rand() % 10;
				for ( i=0; i<variable_delay; i++)
					DELAY_Us(75);
			}
			else
				return 0;
		}
		else
		{		
			// in the TRUE case, we add a random delay such that the Interface can pick up the packets
			rand_read_ten_count = 0;
			variable_delay = rand() % 10;
			for ( i=0; i<variable_delay; i++)
				DELAY_Us(75);				
		}
	}

 /*******************************************************************************************************/
#endif	
	crc_val = crc16(main_data_buffer, main_rece_size - 2);
	if(crc_val == ((U16_T)main_data_buffer[main_rece_size - 2] << 8) | main_data_buffer[main_rece_size - 1])
	{
		return TRUE;
	}
	else
	{		
		return FALSE;
	}
}


void main_responseData(void)
{
	if(flag_sem_response_modbus == 0)
#if (ASIX_MINI || ASIX_CM5)		
		responseCmd(SERIAL,main_data_buffer,NULL);
#endif
	
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
	responseCmd(SERIAL,main_data_buffer);
#endif
}


/* reponse READ command*/
#if (ASIX_MINI || ASIX_CM5)
void responseCmd(U8_T type,U8_T* pData,MODBUSTCP_SERVER_CONN * pHttpConn)  
#endif

#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
void responseCmd(U8_T type,U8_T* pData)
#endif
{	
  U16_T far StartAdd;  
	U16_T far RegNum;
	U8_T far HeadLen; // Modbus head is 0, Tcp head is 6
	U16_T far loop;
	U8_T far sendbuf[300];
	U8_T far cmd;
	U8_T far i;

	U8_T far update_flash;
	U8_T far read_ok;

	flag_sem_response_modbus = 1;
	if(type == SERIAL || type == USB || type == BAC_TO_MODBUS)  // modbus packet
	{
		HeadLen = 0;		
	}
	else    // TCP packet or wifi
	{
		HeadLen = UIP_HEAD;
		for(loop = 0;loop < 6;loop++)
			sendbuf[loop] = 0; 		
	}
	cmd = pData[HeadLen + 1];
	StartAdd = (U16_T)(pData[HeadLen + 2] <<8 ) + pData[HeadLen + 3];
	RegNum = (U8_T)pData[HeadLen + 5];

/* add transaction id */ 	

	if(cmd == READ_VARIABLES)
	{			
		sendbuf[HeadLen] = pData[HeadLen];
		sendbuf[HeadLen + 1] = pData[HeadLen + 1];
		sendbuf[HeadLen + 2] = RegNum * 2;
		memset(&sendbuf[HeadLen + 3],0,RegNum * 2);
		
		read_ok = 1;
		// zigbee map
		if(StartAdd >= MODBUS_ZIGBEE_SUB_NUM && StartAdd <= MODBUS_ZIGBEE_SUB_NUM + 99)
		{			
			if(read_zigbee_map_number(StartAdd,RegNum,&sendbuf[HeadLen + 3]) == 1)  // get zigbee map ok
			{
				
			}
			else
			{
				read_ok = 0;
			}
		}
		else
		{
		
		for(loop = 0 ;loop < RegNum;loop++)
		{
			if ( StartAdd + loop >= MODBUS_SERIALNUMBER_LOWORD && StartAdd + loop <= MODBUS_SERIALNUMBER_LOWORD + 3 )
			{			
				sendbuf[HeadLen + 3 + loop * 2] = 0;
			//	sendbuf[HeadLen + 3 + loop * 2] = (Test[StartAdd + loop - MODBUS_SERIALNUMBER_LOWORD] >> 8) & 0xFF;;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Modbus.serialNum[StartAdd + loop - MODBUS_SERIALNUMBER_LOWORD];
			//	sendbuf[HeadLen + 3 + loop * 2 + 1] = Test[StartAdd + loop - MODBUS_SERIALNUMBER_LOWORD] & 0xFF;
			}
			else if(StartAdd + loop == MODBUS_T3000_PRIVATE)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] = T3000_Private.flag; 	
			}
			else if(StartAdd + loop == MODBUS_FIRMWARE_VERSION_NUMBER_LO)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] = SW_REV % 100;
			}
			else if(StartAdd + loop == MODBUS_FIRMWARE_VERSION_NUMBER_HI)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = SW_REV / 100;			
			}
			else if(StartAdd + loop == MODBUS_HARDWARE_REV)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = Modbus.hardRev;
			}
			else if(StartAdd + loop ==  MODBUS_ADDRESS)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Modbus.address;		
			}
			else if(StartAdd + loop == MODBUS_PRODUCT_MODEL)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
#if (ARM_CM5 || ASIX_CM5)
				sendbuf[HeadLen + 3 + loop * 2 + 1] = PRODUCT_CM5;
#endif
			
#if ARM_WIFI
				sendbuf[HeadLen + 3 + loop * 2 + 1] = 9;  // ????????
#endif
				
#if (ARM_MINI || ASIX_MINI)
			if(Modbus.mini_type == MINI_VAV)
				sendbuf[HeadLen + 3 + loop * 2 + 1] = PRODUCT_MINI_VAV;
			else
			{
				if(Modbus.mini_type >= 1 && Modbus.mini_type <= 4)
					sendbuf[HeadLen + 3 + loop * 2 + 1] = PRODUCT_MINI_BIG;
				else
					sendbuf[HeadLen + 3 + loop * 2 + 1] = PRODUCT_MINI_ARM;
			}
#endif 
			}
			else if(StartAdd + loop == MODBUS_ISP_VER)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Modbus.IspVer;
			}
			else if(StartAdd + loop == MODBUS_PIC)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = Modbus.PicVer;
				
			}
			else if(StartAdd + loop == MODBUS_SNTP_TIMEZONE)
			{
				sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(timezone >> 8);
				sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)timezone;
			}  
			else if(StartAdd + loop == MODBUS_TIMEZONE_SUMMER)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = Daylight_Saving_Time;
			}
			else if(StartAdd + loop == MODBUS_SNTP_SERVER1)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  SntpServer[0];
			}
			else if(StartAdd + loop == MODBUS_SNTP_SERVER2)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  SntpServer[1];
			}
			else if(StartAdd + loop == MODBUS_SNTP_SERVER3)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  SntpServer[2];
			}
			else if(StartAdd + loop == MODBUS_SNTP_SERVER4)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  SntpServer[3];
			}
			else if(StartAdd + loop == MODBUS_SNTP_EN)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Modbus.en_sntp;
			}
			else if(StartAdd + loop == MODBUS_UART0_BAUDRATE)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  uart0_baudrate;
			}
			else if(StartAdd + loop == MODBUS_UART1_BAUDRATE)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  uart1_baudrate;
			}
			else if(StartAdd + loop == MODBUS_UART2_BAUDRATE)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  uart2_baudrate;
			}
			else if(StartAdd + loop == MODBUS_EN_NODES_PLUG_N_PLAY)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Modbus.external_nodes_plug_and_play;
			}
			else if(StartAdd + loop == MODBUS_UPDATE_STATUS)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Modbus.update_status;
			}
//			else if(StartAdd + loop == MODBUS_PROTOCAL)
//			{
//				sendbuf[HeadLen + 3 + loop * 2] = 0;
//				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Modbus.protocal;
//			}
			else if(StartAdd + loop == MODBUS_MINI_TYPE)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Modbus.mini_type;
			}
//			else if(StartAdd + loop == MODBUS_PIC_WATCHDOG)
//			{
//				sendbuf[HeadLen + 3 + loop * 2] = 0;
//				sendbuf[HeadLen + 3 + loop * 2 + 1] =  pic_watchdog;
//			}	
			else if(StartAdd + loop == MODBUS_INSTANCE_LO)
			{
				sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(Instance >> 8);
				sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)Instance;
			}
			else if(StartAdd + loop == MODBUS_INSTANCE_HI)
			{
				sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(Instance >> 24);
				sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)(Instance >> 16);
			}
			else if(StartAdd + loop == MODBUS_PANEL_NUMBER)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = panel_number;
			}
			else if(StartAdd + loop == MODBUS_STATION_NUM)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = Station_NUM;
			}
#if ARM_MINI	
			else if(StartAdd + loop == MODBUS_MSTP_MAX_MASTER)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = MAX_MASTER;
			} 
#endif
			else if(StartAdd + loop == MODBUS_EN_USER)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = Modbus.en_username;
			}
			else if(StartAdd + loop == MODBUS_EN_CUS_UNIT)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = Modbus.cus_unit;
			}
			else if(StartAdd + loop == MODBUS_EN_DYNDNS)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = Modbus.en_dyndns;
			}
#if (ASIX_MINI || ASIX_CM5)
			else if(StartAdd + loop == MODBUS_DYNDNS_PROVIDER)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = dyndns_provider;
			}
			else if(StartAdd + loop == MODBUS_DYNDNS_UPDATE)
			{
				sendbuf[HeadLen + 3 + loop * 2] = dyndns_update_time >> 8;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = dyndns_update_time;
			}
#endif
			else if(StartAdd + loop == MODBUS_RUN_TIME_LO)
			{
				sendbuf[HeadLen + 3 + loop * 2] = run_time >> 8;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = run_time;
			}
			else if(StartAdd + loop == MODBUS_RUN_TIME_HI)
			{
				sendbuf[HeadLen + 3 + loop * 2] = run_time >> 24;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = run_time >> 16;
			}
			else if(StartAdd + loop == MODBUS_TCP_CONNECT_NUM)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = tcp_connect_num;
			}
			else if(StartAdd + loop == MODBUS_UDP_CONNECT_NUM)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = udp_connect_num;
			}
//			else if(StartAdd + loop == MODBUS_NETWORK)
//			{
//				sendbuf[HeadLen + 3 + loop * 2] = Modbus.network_number >> 8;
//				sendbuf[HeadLen + 3 + loop * 2 + 1] = Modbus.network_number;
//			}
			else if(StartAdd + loop == MODBUS_MSTP_NETWORK)
			{
				sendbuf[HeadLen + 3 + loop * 2] = mstp_network >> 8;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = mstp_network;
			}
			else if(StartAdd + loop == MODBUS_BBMD_EN)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = bbmd_en;
			}
			else if(StartAdd + loop == MODBUS_BACKLIGHT)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = Modbus.backlight;
			}
			else if(StartAdd + loop == MODBUS_EN_TIME_SYNC_PC)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = Modbus.en_time_sync_with_pc;
			}
			else if(StartAdd + loop >= MODBUS_MAC_1 && StartAdd + loop <= MODBUS_MAC_6)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Modbus.mac_addr[StartAdd + loop - MODBUS_MAC_1];
			}
			else if(StartAdd + loop == MODBUS_TCP_TYPE)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Modbus.tcp_type;
			}				 
			else if(StartAdd + loop >= MODBUS_IP_1 && StartAdd + loop <= MODBUS_IP_4)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Modbus.ip_addr[StartAdd + loop - MODBUS_IP_1];
			}
			else if(StartAdd + loop >= MODBUS_SUBNET_1 && StartAdd + loop <= MODBUS_SUBNET_4)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Modbus.subnet[StartAdd + loop - MODBUS_SUBNET_1];
			}
			else if(StartAdd + loop >= MODBUS_GETWAY_1 && StartAdd + loop <= MODBUS_GETWAY_4)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Modbus.getway[StartAdd + loop - MODBUS_GETWAY_1];
			} 
			else if(StartAdd + loop == MODBUS_TCP_LISTEN_PORT)
			{
				sendbuf[HeadLen + 3 + loop * 2] = Modbus.tcp_port >> 8;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  (U8_T)Modbus.tcp_port;
			}
			else if(StartAdd + loop == MODBUS_BIP_PORT)
			{
				sendbuf[HeadLen + 3 + loop * 2] = Modbus.Bip_port >> 8;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  (U8_T)Modbus.Bip_port;
			}
			else if(StartAdd + loop == MODBUS_TOTAL_NO)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  sub_no;
			}
			else if(StartAdd + loop >= MODBUS_SUBADDR_FIRST && StartAdd + loop <= MODBUS_SUBADDR_LAST)
			{  
				sendbuf[HeadLen + 3 + loop * 2] = (current_online[scan_db[StartAdd + loop  - MODBUS_SUBADDR_FIRST].id / 8] & (1 << (scan_db[StartAdd + loop  - MODBUS_SUBADDR_FIRST].id % 8))) ? 1 : 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  scan_db[StartAdd + loop  - MODBUS_SUBADDR_FIRST].id;
			}
//			else if(StartAdd + loop >= MODBUS_CHIP1_HW && StartAdd + loop <= MODBUS_CHIP4_SW)
//			{
//			   	sendbuf[HeadLen + 3 + loop * 2] = chip_info[StartAdd + loop - MODBUS_CHIP1_HW] >> 8;	
//				sendbuf[HeadLen + 3 + loop * 2 + 1] = chip_info[StartAdd + loop - MODBUS_CHIP1_HW];
//			}
//			else if(StartAdd + loop >= MODBUS_NETWORK_UART0 && StartAdd + loop <= MODBUS_NETWORK_UART2)
//			{
//			  sendbuf[HeadLen + 3 + loop * 2] = Modbus.network_ID[StartAdd + loop - MODBUS_NETWORK_UART0] >> 8;	
//				sendbuf[HeadLen + 3 + loop * 2 + 1] = Modbus.network_ID[StartAdd + loop - MODBUS_NETWORK_UART0];
//			}
			else if(StartAdd + loop >= MODBUS_COM0_TYPE && StartAdd + loop <= MODBUS_COM2_TYPE)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Modbus.com_config[StartAdd + loop  - MODBUS_COM0_TYPE];
			
			
			}
			else if(StartAdd + loop == MODBUS_REFRESH_FLASH)
			{				
			 	sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Modbus.refresh_flash_timer;
			}
			else if(StartAdd + loop == MODBUS_FLASH_TIME_COUNT)
			{				
			 	sendbuf[HeadLen + 3 + loop * 2] = count_flash >> 8;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  count_flash;
			}
			else if(StartAdd + loop >= MODBUS_OUTPUT_1V && StartAdd + loop <= MODBUS_OUTPUT_10V)
			{
				sendbuf[HeadLen + 3 + loop * 2] = Modbus.start_adc[StartAdd + loop - MODBUS_OUTPUT_1V + 1] >> 8;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Modbus.start_adc[StartAdd + loop - MODBUS_OUTPUT_1V + 1];
			}
#if (ARM_MINI || ASIX_MINI)
			else if(StartAdd + loop >= MODBUS_AO1_FEEDBACK && StartAdd + loop <= MODBUS_AO16_FEEDBACK)
			{
				sendbuf[HeadLen + 3 + loop * 2] = AO_feedback[StartAdd + loop - MODBUS_AO1_FEEDBACK] >> 8;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  AO_feedback[StartAdd + loop - MODBUS_AO1_FEEDBACK];
			}
#endif
			else if(StartAdd + loop == MODBUS_VCC_ADC)
			{
				sendbuf[HeadLen + 3 + loop * 2] = Modbus.vcc_adc >> 8;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] = Modbus.vcc_adc; 	
			}
			else if(StartAdd + loop == MODBUS_OUTPUT_TEST_VALUE)
			{
				sendbuf[HeadLen + 3 + loop * 2] = test_adc >> 8;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] = test_adc; 	
			}
			else if(StartAdd + loop == MODBUS_OUTPUT_TEST_FLAG)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] = test_adc_flag; 	
			}
			else if(StartAdd + loop == Modbus_Bacnet_Vendor_ID)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] = Bacnet_Vendor_ID; 	
			}
			else if(StartAdd + loop == Modbus_Fix_Com_Config)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] = Modbus.fix_com_config; 	
			}
			else if(StartAdd + loop == MODBUS_PANNEL_NAME)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] = 0x56; 	
			}
			else if(StartAdd + loop >= MODBUS_NAME1 && StartAdd + loop <= MODBUS_NAME_END)
			{
				u16 temp = StartAdd + loop - MODBUS_NAME1;  
				sendbuf[HeadLen + 3 + loop * 2] = panelname[temp * 2];	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  panelname[temp * 2 + 1];
			}
			else if(StartAdd + loop == MODBUS_UART0_PARITY)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] = Modbus.uart_parity[0]; 	
			}
			else if(StartAdd + loop == MODBUS_UART1_PARITY)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] = Modbus.uart_parity[1]; 	
			}
			else if(StartAdd + loop == MODBUS_UART2_PARITY)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] = Modbus.uart_parity[2]; 	
			}			
			else if(StartAdd + loop == MODBUS_ZIGBEE_MOUDLE_ID)
			{
				sendbuf[HeadLen + 3 + loop * 2] = Modbus.zigbee_module_id >> 8;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] = Modbus.zigbee_module_id; 	
			}
//			else if(StartAdd + loop == MODBUS_NETWORK_UART0)
//			{
//				sendbuf[HeadLen + 3 + loop * 2] = 0;	
//				sendbuf[HeadLen + 3 + loop * 2 + 1] = Modbus.network_ID[0]; 	
//			}
//			else if(StartAdd + loop == MODBUS_NETWORK_UART1)
//			{
//				sendbuf[HeadLen + 3 + loop * 2] = 0;	
//				sendbuf[HeadLen + 3 + loop * 2 + 1] = Modbus.network_ID[1]; 	
//			}
//			else if(StartAdd + loop == MODBUS_NETWORK_UART2)
//			{
//				sendbuf[HeadLen + 3 + loop * 2] = 0;	
//				sendbuf[HeadLen + 3 + loop * 2 + 1] = Modbus.network_ID[2]; 	
//			}
#if (ARM_MINI || ASIX_MINI)
			else if(StartAdd + loop == MODBUS_OUTPUT_MODE)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] = flag_output; 	
			}
#endif 
			
#if (ASIX_MINI || ASIX_CM5)	
			


		
#if USB_HOST
			else if(StartAdd + loop >= MODBUS_GSM_IP_1 && StartAdd + loop <= MODBUS_GSM_IP_4)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  GSM_remote_IP[StartAdd + loop - MODBUS_GSM_IP_1];
			}
			else if(StartAdd + loop == MODBUS_GSM_TCPPORT)
			{
				sendbuf[HeadLen + 3 + loop * 2] = GSM_remote_tcpport >> 8;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] = GSM_remote_tcpport; 	
			}
			else if(StartAdd + loop == MODBUS_GSM_SEVER_OR_CLIENT)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] = flag_sever_or_client; 	
			}
			
#endif
#endif
//  sub infomation block		

/***********************av bv ai bi ai**********************************\*/
#if BAC_COMMON 
//			else if(StartAdd + loop >= MODBUS_RI_FIRST && StartAdd + loop <= MODBUS_RI_LAST)
//		   {
//		   	U8_T index;
//				index = (StartAdd + loop - MODBUS_RI_FIRST) / 2;
//				if((StartAdd + loop - MODBUS_RI_FIRST) % 2 == 0)
//				{
//					if(inputs[index].digital_analog == 0)  // digital
//					{
//						sendbuf[HeadLen + 3 + loop * 2] = 0;
//						sendbuf[HeadLen + 3 + loop * 2 + 1] = 0;
//					}
//					else
//					{
//						sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(swap_double(inputs[index].value) >> 24);
//						sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)(swap_double(inputs[index].value) >> 16);
//					}
//				}
//				else
//				{
//					if(inputs[index].digital_analog == 0)  // digital
//					{
//						if(inputs[index].range >= ON_OFF  && inputs[index].range <= HIGH_LOW)
//						{  // inverse logic
//							if(inputs[index].control == 1)
//							{
//								sendbuf[HeadLen + 3 + loop * 2] = 0;
//								sendbuf[HeadLen + 3 + loop * 2 + 1] = 0;
//							}
//							else
//							{
//								sendbuf[HeadLen + 3 + loop * 2] = 0;
//								sendbuf[HeadLen + 3 + loop * 2 + 1] = 1;
//							}
//						}	
//						else
//						{
//							if(inputs[index].control == 1)
//							{
//								sendbuf[HeadLen + 3 + loop * 2] = 0;
//								sendbuf[HeadLen + 3 + loop * 2 + 1] = 1;
//							}
//							else
//							{
//								sendbuf[HeadLen + 3 + loop * 2] = 0;
//								sendbuf[HeadLen + 3 + loop * 2 + 1] = 0;
//							}
//						}	
//					}
//					else  // analog
//					{
//						sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(swap_double(inputs[index].value) >> 8);
//						sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)(swap_double(inputs[index].value));
//					}	
//				}
//					
//		   }
//			 else if(StartAdd + loop >= MODBUS_RV_FIRST && StartAdd + loop <= MODBUS_RV_LAST) 
//			{
//				U8_T index;
//				index = (StartAdd + loop - MODBUS_RV_FIRST) / 2;
//				if((StartAdd + loop - MODBUS_RV_FIRST) % 2 == 0)   // high word
//				{
//					if(vars[index].digital_analog == 0)  // digital
//					{
//						sendbuf[HeadLen + 3 + loop * 2] = 0;
//						sendbuf[HeadLen + 3 + loop * 2 + 1] = 0;
//					}
//					else
//					{
//						sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(swap_double(vars[index].value) >> 24);
//						sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)(swap_double(vars[index].value) >> 16);
//					}
//				}
//				else   // low word
//				{
//					if(vars[index].digital_analog == 0)  // digital
//					{
//						if(vars[index].range >= ON_OFF  && vars[index].range <= HIGH_LOW)
//						{  // inverse logic
//							if(vars[index].control == 1)
//							{
//								sendbuf[HeadLen + 3 + loop * 2] = 0;
//								sendbuf[HeadLen + 3 + loop * 2 + 1] = 0;
//							}
//							else
//							{
//								sendbuf[HeadLen + 3 + loop * 2] = 0;
//								sendbuf[HeadLen + 3 + loop * 2 + 1] = 1;
//							}
//						}	
//						else
//						{
//							if(vars[index].control == 1)
//							{
//								sendbuf[HeadLen + 3 + loop * 2] = 0;
//								sendbuf[HeadLen + 3 + loop * 2 + 1] = 1;
//							}
//							else
//							{
//								sendbuf[HeadLen + 3 + loop * 2] = 0;
//								sendbuf[HeadLen + 3 + loop * 2 + 1] = 0;
//							}
//						}	
//					}
//					else  // analog
//					{
//						sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(swap_double(vars[index].value) >> 8);
//						sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)(swap_double(vars[index].value));
//					}	
//				}
//				
//			} 
#endif
/*******************************************************************\*/




			
/*  END SUB TSTAT */
/******************************end for tstst resigter ************************************************/
		   //  input block
		   else if(StartAdd + loop >= MODBUS_OUTPUT_FIRST && StartAdd + loop <= MODBUS_OUTPUT_LAST)
		   {
				U8_T index;			 

				index = (StartAdd + loop - MODBUS_OUTPUT_FIRST) / 2;
				if((StartAdd + loop - MODBUS_OUTPUT_FIRST) % 2 == 0)  // high word
				{
					if(outputs[index].digital_analog == 0) // digtial
					{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = 0;
					}
					else
					{
				    sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(swap_double(outputs[index].value) >> 24);
						sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)(swap_double(outputs[index].value) >> 16);
					}
				}
				else // low word
				{
					if(outputs[index].digital_analog == 0) // digtial
					{
						if((outputs[index].range >= ON_OFF  && outputs[index].range <= HIGH_LOW)
							||(outputs[index].range >= custom_digital1 // customer digital unit
							&& outputs[index].range <= custom_digital6
							&& digi_units[outputs[index].range - custom_digital1].direct == 1))
						{  // inverse logic
							if(outputs[index].control == 1)
							{
								sendbuf[HeadLen + 3 + loop * 2] = 0;
								sendbuf[HeadLen + 3 + loop * 2 + 1] = 0;
							}
							else
							{
								sendbuf[HeadLen + 3 + loop * 2] = 0;
								sendbuf[HeadLen + 3 + loop * 2 + 1] = 1;
							}
						}	
						else
						{
							if(outputs[index].control == 1)
							{
								sendbuf[HeadLen + 3 + loop * 2] = 0;
								sendbuf[HeadLen + 3 + loop * 2 + 1] = 1;
							}
							else
							{
								sendbuf[HeadLen + 3 + loop * 2] = 0;
								sendbuf[HeadLen + 3 + loop * 2 + 1] = 0;
							}
						}	
					}
					else  // analog
					{
						sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(swap_double(outputs[index].value) >> 8);
						sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)(swap_double(outputs[index].value));
					}
				}
				
		   }
		   else if(StartAdd + loop >= MODBUS_OUTPUT_SWICH_FIRST && StartAdd + loop <= MODBUS_OUTPUT_SWICH_LAST)
		   {
		   		U8_T index;
				index = (StartAdd + loop - MODBUS_OUTPUT_SWICH_FIRST); 
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = outputs[index].switch_status;		
		   }
		   else if(StartAdd + loop >= MODBUS_OUTPUT_RANGE_FIRST && StartAdd + loop <= MODBUS_OUTPUT_RANGE_LAST)
		   {
		   		U8_T index;
				index = (StartAdd + loop - MODBUS_OUTPUT_RANGE_FIRST);
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = outputs[index].range;
	
		   }
		   else if(StartAdd + loop >= MODBUS_OUTPUT_AM_FIRST && StartAdd + loop <= MODBUS_OUTPUT_AM_LAST)
		   {
		   		U8_T index;
				index = (StartAdd + loop - MODBUS_OUTPUT_AM_FIRST);
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = outputs[index].auto_manual;
	
		   }
		   else if(StartAdd + loop >= MODBUS_OUTPUT_AD_FIRST && StartAdd + loop <= MODBUS_OUTPUT_AD_LAST)
		   {
		   		U8_T index;
				index = (StartAdd + loop - MODBUS_OUTPUT_AD_FIRST);
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = outputs[index].digital_analog;
	
		   }
// end output
// start input
		   else if((StartAdd + loop >= MODBUS_INPUT_FIRST && StartAdd + loop <= MODBUS_INPUT_LAST) 
				 || (StartAdd + loop >= MODBUS_RI_FIRST && StartAdd + loop <= MODBUS_RI_LAST))
			 {
		   	U8_T index;
				U16_T base; 
					if((StartAdd + loop >= MODBUS_INPUT_FIRST && StartAdd + loop <= MODBUS_INPUT_LAST))
						base = MODBUS_INPUT_FIRST;
					else
						base = MODBUS_RI_FIRST;
					 
				index = (StartAdd + loop - base) / 2;
				if((StartAdd + loop - base) % 2 == 0)
				{
					if(inputs[index].digital_analog == 0)  // digital
					{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = 0;
					}
					else
					{
						sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(swap_double(inputs[index].value) >> 24);
						sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)(swap_double(inputs[index].value) >> 16);
					}
				}
				else
				{
					if(inputs[index].digital_analog == 0)  // digital
					{
						if((inputs[index].range >= ON_OFF  && inputs[index].range <= HIGH_LOW)
							||(inputs[index].range >= custom_digital1 // customer digital unit
							&& inputs[index].range <= custom_digital6
							&& digi_units[inputs[index].range - custom_digital1].direct == 1))
						{  // inverse logic
							if(inputs[index].control == 1)
							{
								sendbuf[HeadLen + 3 + loop * 2] = 0;
								sendbuf[HeadLen + 3 + loop * 2 + 1] = 0;
							}
							else
							{
								sendbuf[HeadLen + 3 + loop * 2] = 0;
								sendbuf[HeadLen + 3 + loop * 2 + 1] = 1;
							}
						}	
						else
						{
							if(inputs[index].control == 1)
							{
								sendbuf[HeadLen + 3 + loop * 2] = 0;
								sendbuf[HeadLen + 3 + loop * 2 + 1] = 1;
							}
							else
							{
								sendbuf[HeadLen + 3 + loop * 2] = 0;
								sendbuf[HeadLen + 3 + loop * 2 + 1] = 0;
							}
						}	
					}
					else  // analog
					{
						sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(swap_double(inputs[index].value) >> 8);
						sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)(swap_double(inputs[index].value));
					}	
				}
					
		   }
		   else if(StartAdd + loop >= MODBUS_INPUT_FILTER_FIRST && StartAdd + loop <= MODBUS_INPUT_FILTER_LAST)
		   {
		   		U8_T index;
					index = (StartAdd + loop - MODBUS_INPUT_FILTER_FIRST);
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = inputs[index].filter;	
		   }
		   else if(StartAdd + loop >= MODBUS_INPUT_CAL_FIRST && StartAdd + loop <= MODBUS_INPUT_CAL_LAST)
		   {
					U8_T index;
					index = (StartAdd + loop - MODBUS_INPUT_CAL_FIRST);
					sendbuf[HeadLen + 3 + loop * 2] = inputs[index].calibration_hi;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = inputs[index].calibration_lo;	
		   }
		   else if(StartAdd + loop >= MODBUS_INPUT_RANGE_FIRST && StartAdd + loop <= MODBUS_INPUT_RANGE_LAST)
		   {
					U8_T index;
					index = (StartAdd + loop - MODBUS_INPUT_RANGE_FIRST);
					sendbuf[HeadLen + 3 + loop * 2] = inputs[index].digital_analog;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = inputs[index].range;
		   }
			 else if(StartAdd + loop >= MODBUS_INPUT_AM_FIRST && StartAdd + loop <= MODBUS_INPUT_AM_LAST)
		   {
					U8_T index;
					index = (StartAdd + loop - MODBUS_INPUT_AM_FIRST);
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = inputs[index].auto_manual; 	
		   }
		   else if(StartAdd + loop >= MODBUS_INPUT_CAL_SIGN_FIRST && StartAdd + loop <= MODBUS_INPUT_CAL_SIGN_LAST)
		   {
					U8_T index;
					index = (StartAdd + loop - MODBUS_INPUT_CAL_SIGN_FIRST);
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = inputs[index].calibration_sign;
	
		   }
		   else if(StartAdd + loop >= MODBUS_INPUT_HI_SPD_COUNTER_FIRST && StartAdd + loop <= MODBUS_INPUT_HI_SPD_COUNTER_LAST)
		   {
					U8_T index;
					index = (StartAdd + loop - MODBUS_INPUT_HI_SPD_COUNTER_FIRST) / 2;
					if((StartAdd + loop - MODBUS_INPUT_HI_SPD_COUNTER_FIRST) % 2 == 0)
					{
						sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(high_spd_counter[index] >> 24);
						sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)(high_spd_counter[index] >> 16);
					}
					else
					{
						sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(high_spd_counter[index] >> 8);
						sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)high_spd_counter[index];
					}				
		   }
		   else if(StartAdd + loop >= MODBUS_INPUT_HI_SPD_EN_FIRST && StartAdd + loop <= MODBUS_INPUT_HI_SPD_EN_LAST)
		   {
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = high_spd_en[StartAdd + loop - MODBUS_INPUT_HI_SPD_EN_FIRST];
		   }
			 else if(StartAdd + loop >= MODBUS_INPUT_TYPE_FIRST && StartAdd + loop <= MODBUS_INPUT_TYPE_LAST)
		   {
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = input_type[StartAdd + loop - MODBUS_INPUT_TYPE_FIRST];
		   }
// end input
// start variable
			 else if((StartAdd + loop >= MODBUS_VAR_FIRST && StartAdd + loop <= MODBUS_VAR_LAST) ||
				(StartAdd + loop >= MODBUS_RV_FIRST && StartAdd + loop <= MODBUS_RV_LAST)) 
			{
				U8_T index;
				U16_T base;
				
			 if(StartAdd + loop >= MODBUS_VAR_FIRST && StartAdd + loop <= MODBUS_VAR_LAST)
				 base = MODBUS_VAR_FIRST;
			 else 
				 base = MODBUS_RV_FIRST;
			 
				index = (StartAdd + loop - base) / 2;
				if((StartAdd + loop - base) % 2 == 0)   // high word
				{
					if(vars[index].digital_analog == 0)  // digital
					{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = 0;
					}
					else
					{
						sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(swap_double(vars[index].value) >> 24);
						sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)(swap_double(vars[index].value) >> 16);
					}
				}
				else   // low word
				{
					if(vars[index].digital_analog == 0)  // digital
					{
						if((vars[index].range >= ON_OFF  && vars[index].range <= HIGH_LOW)
							||(vars[index].range >= custom_digital1 // customer digital unit
							&& vars[index].range <= custom_digital6
							&& digi_units[vars[index].range - custom_digital1].direct == 1))
						{  // inverse logic
							if(vars[index].control == 1)
							{
								sendbuf[HeadLen + 3 + loop * 2] = 0;
								sendbuf[HeadLen + 3 + loop * 2 + 1] = 0;
							}
							else
							{
								sendbuf[HeadLen + 3 + loop * 2] = 0;
								sendbuf[HeadLen + 3 + loop * 2 + 1] = 1;
							}
						}	
						else
						{
							if(vars[index].control == 1)
							{
								sendbuf[HeadLen + 3 + loop * 2] = 0;
								sendbuf[HeadLen + 3 + loop * 2 + 1] = 1;
							}
							else
							{
								sendbuf[HeadLen + 3 + loop * 2] = 0;
								sendbuf[HeadLen + 3 + loop * 2 + 1] = 0;
							}
						}	
					}
					else  // analog
					{
						sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(swap_double(vars[index].value) >> 8);
						sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)(swap_double(vars[index].value));
					}	
				}
				
			} 
			else if(StartAdd + loop >= MODBUS_VAR_AM_FIRST && StartAdd + loop <= MODBUS_VAR_AM_LAST)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = vars[StartAdd + loop - MODBUS_VAR_AM_FIRST].auto_manual;
			}	
// end variable
// start weekly
			else if(StartAdd + loop >= MODBUS_WR_AM_FIRST && StartAdd + loop <= MODBUS_WR_AM_LAST)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = weekly_routines[StartAdd + loop - MODBUS_WR_AM_FIRST].auto_manual;
			}	
			else if(StartAdd + loop >= MODBUS_WR_OUT_FIRST && StartAdd + loop <= MODBUS_WR_OUT_LAST)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = weekly_routines[StartAdd + loop - MODBUS_WR_OUT_FIRST].value;
			}	
			else if(StartAdd + loop >= MODBUS_WR_HOLIDAY1_FIRST && StartAdd + loop <= MODBUS_WR_HOLIDAY1_LAST)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = weekly_routines[StartAdd + loop - MODBUS_WR_HOLIDAY1_FIRST].override_1.number;
			}	
			else if(StartAdd + loop >= MODBUS_WR_STATE1_FIRST && StartAdd + loop <= MODBUS_WR_STATE1_LAST)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = weekly_routines[StartAdd + loop - MODBUS_WR_STATE1_FIRST].override_1_value;
			}	
			else if(StartAdd + loop >= MODBUS_WR_HOLIDAY2_FIRST && StartAdd + loop <= MODBUS_WR_HOLIDAY2_LAST)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = weekly_routines[StartAdd + loop - MODBUS_WR_HOLIDAY2_FIRST].override_2.number;
			}	
			else if(StartAdd + loop >= MODBUS_WR_STATE2_FIRST && StartAdd + loop <= MODBUS_WR_STATE2_LAST)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = weekly_routines[StartAdd + loop - MODBUS_WR_STATE2_FIRST].override_2_value;
			}	
// weekly_time
			else if(StartAdd + loop >= MODBUS_WR_TIME_FIRST && StartAdd + loop <= MODBUS_WR_TIME_LAST)
			{
				U8_T i,j,k; 
				i = (StartAdd + loop - MODBUS_WR_TIME_FIRST) / (MAX_SCHEDULES_PER_WEEK * 8); // week index
				j = (StartAdd + loop - MODBUS_WR_TIME_FIRST) %(MAX_SCHEDULES_PER_WEEK * 8) / 8; // day index
				k = (StartAdd + loop - MODBUS_WR_TIME_FIRST) % (MAX_SCHEDULES_PER_WEEK * 8) % 8;  // seg index
				sendbuf[HeadLen + 3 + loop * 2] = wr_times[i][j].time[k].hours;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = wr_times[i][j].time[k].minutes;
			}	
// end weekly
			
// start annual 
			else if(StartAdd + loop >= MODBUS_AR_AM_FIRST && StartAdd + loop <= MODBUS_AR_AM_LAST)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = annual_routines[StartAdd + loop - MODBUS_AR_AM_FIRST].auto_manual;
			}	
			else if(StartAdd + loop >= MODBUS_AR_OUT_FIRST && StartAdd + loop <= MODBUS_AR_OUT_LAST)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = annual_routines[StartAdd + loop - MODBUS_AR_OUT_FIRST].value;
			}	
			else if(StartAdd + loop >= MODBUS_AR_TIME_FIRST && StartAdd + loop <= MODBUS_AR_TIME_LAST)
			{
				U8_T i,j; 
				i = (StartAdd + loop - MODBUS_AR_TIME_FIRST) / AR_DATES_SIZE;
				j = (StartAdd + loop - MODBUS_AR_TIME_FIRST) % AR_DATES_SIZE;
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = ar_dates[i][j];
			}	
//			else if(StartAdd + loop >= MODBUS_ID_FIRST && StartAdd + loop <= MODBUS_ID_LAST)
//			{
//				sendbuf[HeadLen + 3 + loop * 2] = 0;
//				sendbuf[HeadLen + 3 + loop * 2 + 1] = ID_Config[(StartAdd + loop - MODBUS_ID_FIRST) / ID_SIZE].all[(StartAdd + loop - MODBUS_ID_FIRST) % ID_SIZE];
//			}
			
// end annual
			
			else if(StartAdd + loop >= MODBUS_TIMER_ADDRESS && StartAdd + loop < MODBUS_TIMER_ADDRESS + 8)
			{	 
				sendbuf[HeadLen + 3 + loop * 2] = Rtc.all[StartAdd + loop - MODBUS_TIMER_ADDRESS] >> 8;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Rtc.all[StartAdd + loop - MODBUS_TIMER_ADDRESS];
			}
			else if(StartAdd + loop >= MODBUS_SD_BLOCK_A1 && StartAdd + loop <= MODBUS_SD_BLOCK_A12)
			{
				sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(SD_block_num[(StartAdd + loop - MODBUS_SD_BLOCK_A1) * 2] >> 8);
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  SD_block_num[(StartAdd + loop - MODBUS_SD_BLOCK_A1) * 2];
			}			
			else if(StartAdd + loop >= MODBUS_SD_BLOCK_D1 && StartAdd + loop <= MODBUS_SD_BLOCK_D12)
			{
				sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(SD_block_num[(StartAdd + loop - MODBUS_SD_BLOCK_D1) * 2 + 1] >> 8);
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  SD_block_num[(StartAdd + loop - MODBUS_SD_BLOCK_D1) * 2 + 1];
			}
			else if(StartAdd + loop >= MODBUS_SD_BLOCK_H1 && StartAdd + loop <= MODBUS_SD_BLOCK_H12)
			{
				sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(SD_block_num[(StartAdd + loop - MODBUS_SD_BLOCK_H1) * 2 + 1] >> 16);
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  (U8_T)(SD_block_num[(StartAdd + loop - MODBUS_SD_BLOCK_H1) * 2] >> 16);
			}
			else if( StartAdd + loop >= MODBUS_TEST && StartAdd + loop <= MODBUS_TEST_50 )
			{
				sendbuf[HeadLen + 3 + loop * 2] = (Test[StartAdd + loop - MODBUS_TEST] >> 8) & 0xFF;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = Test[StartAdd + loop - MODBUS_TEST] & 0xFF;
			}
/******************* read IN OUT by block start ******************************************/
			else if( StartAdd + loop >= MODBUS_USER_BLOCK_FIRST && StartAdd + loop <= MODBUS_USER_BLOCK_LAST)
			{
				U16_T far temp;
				temp = read_user_data_by_block(StartAdd + loop);
				
				sendbuf[HeadLen + 3 + loop * 2 + 1] = (temp >> 8) & 0xFF;;
				sendbuf[HeadLen + 3 + loop * 2] = temp & 0xFF;
			}
/*********************read IN OUT by block endf ***************************************/			
			
			
			
			else if(StartAdd + loop >= MODBUS_TASK_TEST && StartAdd + loop <= MODBUS_TASK_TEST + 59)	
			{
				
				U16_T reg = (StartAdd + loop - MODBUS_TASK_TEST) / 4;
				U16_T index = (StartAdd + loop - MODBUS_TASK_TEST) % 4;	 
				if(index == 0)
				{
					sendbuf[HeadLen + 3 + loop * 2] = task_test.count[reg] >> 8;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = task_test.count[reg] & 0xff; 
				}
				else if(index == 1)
				{
					sendbuf[HeadLen + 3 + loop * 2] = task_test.old_count[reg] >> 8;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = task_test.old_count[reg] & 0xff; 
				}
				else if(index == 2)
				{
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = task_test.enable[reg]; 
				}
				else if(index == 3)
				{
					sendbuf[HeadLen + 3 + loop * 2] = 0;											 
					sendbuf[HeadLen + 3 + loop * 2 + 1] = task_test.inactive_count[reg]; 
				}
			}	 
			 
			else
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] = 1;
			}
		}
	}

		if(type == SERIAL || type == USB || type == BAC_TO_MODBUS)
		{
			U16_T crc_val;

			crc_val = crc16(sendbuf,RegNum * 2 + 3);
			sendbuf[RegNum * 2 + 3]	= crc_val >> 8;
			sendbuf[RegNum * 2 + 4]	= (U8_T)crc_val;
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			if(type == BAC_TO_MODBUS)
			{
				memcpy(&bacnet_to_modbus,&sendbuf[3],RegNum * 2);
			}
			else 
#endif
			if(type == SERIAL)
			{
		   	uart_init_send_com(Modbus.main_port);
				uart_send_string(sendbuf, RegNum * 2 + 5,Modbus.main_port);

			}
#if (ASIX_MINI || ASIX_CM5)
#if USB_DEVICE
			else
			{
				memcpy(UpBuf,sendbuf,RegNum * 2 + 5); 
				UpIndex = 0;
				UpCtr = RegNum * 2 + 5;
				ENDP2_NEED_UP_FLAG = 1;	
			}
#endif
#endif
		}		
#if ARM_WIFI
		else if(type == WIFI) // wifi
		{
			TransID =  ((U16_T)pData[0] << 8) | pData[1];
			sendbuf[0] = TransID >> 8;			//	TransID
			sendbuf[1] = (U8_T)TransID;	
			sendbuf[2] = 0;			//	ProtoID
			sendbuf[3] = 0;
			sendbuf[4] = (3 + RegNum * 2) >> 8;	//	Len
			sendbuf[5] = (U8_T)(3 + RegNum * 2) ;  
			
			memcpy(modbus_wifi_buf,sendbuf,RegNum * 2 + 3 + HeadLen);
			modbus_wifi_len = RegNum * 2 + 3 + HeadLen;
//			uart_init_send_com(Modbus.main_port);
//			uart_send_string(sendbuf, RegNum * 2 + 3 + HeadLen,Modbus.main_port);
		}
#endif
#if !(ARM_WIFI)
		else // TCP
		{
			TransID =  ((U16_T)pData[0] << 8) | pData[1];
			sendbuf[0] = TransID >> 8;			//	TransID
			sendbuf[1] = (U8_T)TransID;	
			sendbuf[2] = 0;			//	ProtoID
			sendbuf[3] = 0;
			sendbuf[4] = (3 + RegNum * 2) >> 8;	//	Len
			sendbuf[5] = (U8_T)(3 + RegNum * 2) ;  			

			if(read_ok == 1)
			{
				if(type == TCP)
				{
	#if (ASIX_MINI || ASIX_CM5)
					if(cSemaphoreTake( xSemaphore_tcp_send, ( portTickType ) 10 ) == pdTRUE)
					{				
						TCPIP_TcpSend(pHttpConn->TcpSocket, sendbuf, RegNum * 2 + HeadLen + 3, TCPIP_SEND_NOT_FINAL); 
						cSemaphoreGive( xSemaphore_tcp_send );
					}
	#endif
					
					
	#if (ARM_MINI || ARM_CM5)
					 memcpy(tcp_server_sendbuf,sendbuf,RegNum * 2 + 3 + HeadLen);
					 tcp_server_sendlen = RegNum * 2 + 3 + HeadLen;
					
	#endif

				}
				
			}
		}
#endif
	}
	else if(cmd == WRITE_VARIABLES)
	{		
		if(type == SERIAL || type == USB || type == BAC_TO_MODBUS)
		{ 	
			if(type == SERIAL)
			{
				uart_init_send_com(Modbus.main_port);
				uart_send_string(pData, 8,Modbus.main_port);
			}	
#if (ASIX_MINI || ASIX_CM5)
#if USB_DEVICE
			else
			{
				memcpy(UpBuf,pData,8); 
				UpIndex = 0;
				ENDP2_NEED_UP_FLAG = 1;	
				UpCtr = 8;	
			}
#endif
#endif
		}
#if !(ARM_WIFI)
		else // TCP   dont have CRC 
		{
		//	SetTransactionId(6 + UIP_HEAD);
			sendbuf[0] = pData[0];//0;			//	TransID
			sendbuf[1] = pData[1];//TransID++;	
			sendbuf[2] = 0;			//	ProtoID
			sendbuf[3] = 0;
			sendbuf[4] = 0;	//	Len
			sendbuf[5] = 6;

			if(type == TCP)
			{
				for (loop = 0;loop < 6;loop++)
				{
					sendbuf[HeadLen + loop] = pData[HeadLen + loop];	
				}
#if (ASIX_MINI || ASIX_CM5)
				if(cSemaphoreTake( xSemaphore_tcp_send, ( portTickType ) 10 ) == pdTRUE)
				{				
					TCPIP_TcpSend(pHttpConn->TcpSocket, sendbuf, 6 + UIP_HEAD, TCPIP_SEND_NOT_FINAL); 
					cSemaphoreGive( xSemaphore_tcp_send );
				}
#endif
			
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
				memcpy(tcp_server_sendbuf,sendbuf,6 + UIP_HEAD);
				tcp_server_sendlen = 6 + UIP_HEAD;
			
#endif
			}
			else if(type == WIFI) // wifi
			{
				uart_init_send_com(Modbus.main_port);
				uart_send_string(sendbuf, 6 + UIP_HEAD,Modbus.main_port);
			}
		}
#endif

		/* dealwith write command */
		if(StartAdd >= MODBUS_OUTPUT_FIRST && StartAdd <= MODBUS_AR_OUT_LAST)
		{
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			if(StartAdd >= MODBUS_OUTPUT_FIRST && StartAdd <= MODBUS_OUTPUT_AD_LAST)
			{
				write_page_en[OUT] = 1;	
			}
			if(StartAdd >= MODBUS_INPUT_FIRST && StartAdd <= MODBUS_INPUT_TYPE_LAST)
			{
				write_page_en[IN] = 1;	
			}
			if(StartAdd >= MODBUS_VAR_FIRST && StartAdd <= MODBUS_VAR_AM_LAST)
			{
				write_page_en[VAR] = 1;	
			} 
			if((StartAdd >= MODBUS_WR_AM_FIRST && StartAdd <= MODBUS_WR_STATE2_LAST) || \
			(StartAdd >= MODBUS_WR_TIME_FIRST && StartAdd <= MODBUS_WR_TIME_LAST))
			{
				write_page_en[WRT] = 1;	
			}
			if((StartAdd >= MODBUS_AR_AM_FIRST && StartAdd <= MODBUS_AR_OUT_LAST) || \
				(StartAdd >= MODBUS_AR_TIME_FIRST && StartAdd <= MODBUS_AR_TIME_LAST))
			{
				write_page_en[AR] = 1;	
			}
#endif
			ChangeFlash = 1;
		}
		if (StartAdd >= MODBUS_SERIALNUMBER_LOWORD && StartAdd <= MODBUS_SERIALNUMBER_LOWORD + 3 )
		{
			if((StartAdd == MODBUS_SERIALNUMBER_LOWORD) && (SNWriteflag & 0x01) == 0)
			{
				 Modbus.serialNum[0] = pData[HeadLen + 5];
				 Modbus.serialNum[1] = pData[HeadLen + 4];
				E2prom_Write_Byte(EEP_SERIALNUMBER_LOWORD , Modbus.serialNum[0]);
				E2prom_Write_Byte(EEP_SERIALNUMBER_LOWORD + 1, Modbus.serialNum[1]);

				SNWriteflag |= 0x01;
				if(SNWriteflag & 0x02)
					update_flash = 0;
			}
			else if((StartAdd == MODBUS_SERIALNUMBER_HIWORD) && (SNWriteflag & 0x02) == 0)
			{
				Modbus.serialNum[2] = pData[HeadLen + 5];
				Modbus.serialNum[3] = pData[HeadLen + 4];
				E2prom_Write_Byte(EEP_SERIALNUMBER_HIWORD , Modbus.serialNum[2]);
				E2prom_Write_Byte(EEP_SERIALNUMBER_HIWORD + 1, Modbus.serialNum[3]);
				SNWriteflag |= 0x02;
				if(SNWriteflag & 0x01)
					update_flash = 0;
			}
		}
		
		else if(StartAdd == MODBUS_ADDRESS)
		{
			Modbus.address = pData[HeadLen + 5];
			E2prom_Write_Byte(EEP_ADDRESS, Modbus.address);
			Panel_Info.reg.modbus_addr = Modbus.address;
			Station_NUM = Modbus.address;
			Setting_Info.reg.MSTP_ID = Station_NUM;	
		}
		else if(StartAdd == MODBUS_HARDWARE_REV)
		{
			Modbus.hardRev = pData[HeadLen + 5];
			E2prom_Write_Byte(EEP_HARDWARE_REV, Modbus.hardRev);		
		}
		else if(StartAdd == MODBUS_UART0_BAUDRATE )
		{
			uart0_baudrate = pData[HeadLen + 5];
			E2prom_Write_Byte(EEP_UART0_BAUDRATE, pData[HeadLen + 5] );
			UART_Init(0);
		}
		else if(StartAdd == MODBUS_UART1_BAUDRATE )
		{
			uart1_baudrate = pData[HeadLen + 5];
			E2prom_Write_Byte(EEP_UART1_BAUDRATE, pData[HeadLen + 5] );
			UART_Init(1);
		}
		else if(StartAdd == MODBUS_UART2_BAUDRATE )
		{
			uart2_baudrate = pData[HeadLen + 5];
			E2prom_Write_Byte(EEP_UART2_BAUDRATE, pData[HeadLen + 5] );
			UART_Init(2);		
		}
#if !(ARM_WIFI)
		else if(StartAdd == MODBUS_SNTP_TIMEZONE )
		{
			if(timezone != pData[HeadLen + 5]+ (pData[HeadLen + 4]<<8))
			{			
				Sync_timestamp(pData[HeadLen + 5]+ (pData[HeadLen + 4]<<8),timezone,0,0);
				timezone = pData[HeadLen + 5]+ (pData[HeadLen + 4]<<8);
				E2prom_Write_Byte(EEP_TIME_ZONE_HI, pData[HeadLen + 4] );
				E2prom_Write_Byte(EEP_TIME_ZONE_LO, pData[HeadLen + 5] );
			}
			
		}
		else if(StartAdd == MODBUS_TIMEZONE_SUMMER )
		{
			if(Daylight_Saving_Time != pData[HeadLen + 5])
			{
				Sync_timestamp(0,0,pData[HeadLen + 5],Daylight_Saving_Time);
				Daylight_Saving_Time = pData[HeadLen + 5];
				E2prom_Write_Byte(EEP_DAYLIGHT_SAVING_TIME, pData[HeadLen + 5] );
			}
		}
		else if(StartAdd == MODBUS_SNTP_EN )
		{			
			if(pData[HeadLen + 5] <= 5)
			{
				Modbus.en_sntp = pData[HeadLen + 5];
				sntp_select_time_server(Modbus.en_sntp);
				E2prom_Write_Byte(EEP_EN_SNTP, pData[HeadLen + 5] );
				if(Modbus.en_sntp >= 2)
				{
					flag_Update_Sntp = 0;
					Update_Sntp_Retry = 0;
				}
			}
		}
#endif
		else if(StartAdd == MODBUS_UPDATE_STATUS)
		{
		 	update_flash = pData[HeadLen + 5];
			if(update_flash == 0x7F)
			{
				flag_reboot = 1;
			}		
			else if((update_flash == 0x8E) || (update_flash == 0x8F))
			{
				if(update_flash == 0x8e)
				{
					SNWriteflag = 0x00;
					E2prom_Write_Byte(EEP_SERIALNUMBER_WRITE_FLAG, SNWriteflag);
					
					Modbus.serialNum[0] = 0;
					Modbus.serialNum[1] = 0;
					Modbus.serialNum[2] = 0;
					Modbus.serialNum[3] = 0;
				}
			}
			
		}
		else if(StartAdd == MODBUS_EN_NODES_PLUG_N_PLAY)
		{
			 Modbus.external_nodes_plug_and_play = pData[HeadLen + 5];
		  	 E2prom_Write_Byte(EEP_EN_NODE_PLUG_N_PLAY, Modbus.external_nodes_plug_and_play);
		}
//		else if(StartAdd == MODBUS_PROTOCAL)
//		{
//			 Modbus.protocal = pData[HeadLen + 5];
//		  	 E2prom_Write_Byte(EEP_PROTOCAL, Modbus.protocal);
//		}

		
		else if(StartAdd == MODBUS_MINI_TYPE)
		{
			Modbus.mini_type = pData[HeadLen + 5]+ (pData[HeadLen + 4]<<8);
			E2prom_Write_Byte(EEP_MINI_TYPE, Modbus.mini_type);
   	//		SPI_Setup(SPI_SSO_ENB|SPI_MST_SEL|SPI_SS_AUTO|SPI_ENB, SPI_STCFIE, 0x20/*0x80*/, SLAVE_SEL_1); // 25M
#if (ASIX_MINI || ASIX_CM5)
			Start_Comm_Top();
#endif
		}
		
 
		else if(StartAdd == MODBUS_PANEL_NUMBER)
		{
			if(pData[HeadLen + 5] != 0 && pData[HeadLen + 5] != 255)
			{
				panel_number = pData[HeadLen + 5];
				E2prom_Write_Byte(EEP_PANEL_NUMBER,panel_number);
				change_panel_number_in_code(Setting_Info.reg.panel_number,panel_number);
				Setting_Info.reg.panel_number	= panel_number;
			}
		} 
		else if(StartAdd == MODBUS_STATION_NUM)
		{
			if(pData[HeadLen + 5] != 0 && pData[HeadLen + 5] != 255)
			{
				Station_NUM = pData[HeadLen + 5];
				Modbus.address = Station_NUM;
				//E2prom_Write_Byte(EEP_STATION_NUM,Station_NUM);
				E2prom_Write_Byte(EEP_ADDRESS,Modbus.address);
				Setting_Info.reg.MSTP_ID	= Station_NUM;
			}
		} 
#if ARM_MINI	
		else if(StartAdd == MODBUS_MSTP_MAX_MASTER)
		{
				MAX_MASTER = pData[HeadLen + 5];
				E2prom_Write_Byte(EEP_MAX_MASTER,MAX_MASTER);
		} 
#endif
		else if(StartAdd == MODBUS_INSTANCE_LO)
		{
			U32_T		temp;
			temp = Instance;
			if((U16_T)(Instance >> 16) != pData[HeadLen + 5] + pData[HeadLen + 4] * 256)
			{
				temp = temp & 0xffff0000L;
				temp |= pData[HeadLen + 5] + pData[HeadLen + 4] * 256;
				Instance = temp;
				E2prom_Write_Byte(EEP_INSTANCE1,pData[HeadLen + 5]);
				E2prom_Write_Byte(EEP_INSTANCE2,pData[HeadLen + 4]);
			}
		} 
	  else if(StartAdd == MODBUS_INSTANCE_HI)
		{
			if((U16_T)(Instance >> 16) != pData[HeadLen + 5] + pData[HeadLen + 4] * 256)
			{
				Instance = ((U32_T)pData[HeadLen + 5] << 16) + ((U32_T)pData[HeadLen + 4] << 24) + (U16_T)Instance;	
				E2prom_Write_Byte(EEP_INSTANCE3,pData[HeadLen + 5]);
				E2prom_Write_Byte(EEP_INSTANCE4,pData[HeadLen + 4]);
			}
		} 
		else if(StartAdd == MODBUS_EN_USER)
		{
			Modbus.en_username = pData[HeadLen + 5];
			E2prom_Write_Byte(EEP_USER_NAME,Modbus.en_username);
		}
/*		else if(StartAdd == MODBUS_EN_CUS_UNIT)
		{
			Modbus.cus_unit = pData[HeadLen + 5];
			E2prom_Write_Byte(EEP_CUS_UNIT,Modbus.cus_unit);
		} */
#if 0//ASIX
		else if(StartAdd == MODBUS_USB_MODE)
		{
			Modbus.usb_mode = pData[HeadLen + 5];
			E2prom_Write_Byte(EEP_USB_MODE,Modbus.usb_mode);
			if(Modbus.usb_mode == 0)
			{
#if USB_DEVICE
				USB_device_Initial();
#endif
			}
			else if(Modbus.usb_mode == 1)
			{
#if USB_HOST
				USB_HOST_initial();
#endif
			}
		}
#endif

#if !(ARM_WIFI)
		else if(StartAdd == MODBUS_EN_DYNDNS)
		{
			Modbus.en_dyndns = pData[HeadLen + 5];	
			if(Modbus.en_dyndns == 2)
			{
				flag_Update_Dyndns = 0;
				Update_Dyndns_Retry = 0;
#if (ASIX_MINI || ASIX_CM5)
#if REM_CONNECTION
				Recount_Check_serverip = 0;
#endif
#endif
			}
			E2prom_Write_Byte(EEP_EN_DYNDNS,Modbus.en_dyndns);
		}		
#if (ASIX_MINI || ASIX_CM5)
		else if(StartAdd == MODBUS_DYNDNS_PROVIDER)
		{
			if(pData[HeadLen + 5] <= 3)
			{
				dyndns_provider = pData[HeadLen + 5];
				E2prom_Write_Byte(EEP_DYNDNS_PROVIDER,dyndns_provider);				
			}
		}
		else if(StartAdd == MODBUS_DYNDNS_UPDATE)
		{
			dyndns_update_time = pData[HeadLen + 5] + pData[HeadLen + 4] * 256;
			if(dyndns_update_time < 6)	dyndns_update_time = 6;
			E2prom_Write_Byte(EEP_DYNDNS_UPDATE_HI,pData[HeadLen + 4]);
			E2prom_Write_Byte(EEP_DYNDNS_UPDATE_LO,pData[HeadLen + 5]);
		}
#endif
		else if(StartAdd == MODBUS_ENABLE_WRITE_MAC)
		{

				Mac_Address_write_enable = 1;

		}
#endif
		else if(StartAdd == MODBUS_SCAN_SUB_PORT)
		{
			scan_port = pData[HeadLen + 5];
		}
		else if(StartAdd == MODBUS_SCAN_SUB_BAUT)
		{
			scan_baut = pData[HeadLen + 5];
		}
		else if(StartAdd >= MODBUS_COM0_TYPE && StartAdd <= MODBUS_COM2_TYPE)
		{
			i = StartAdd - MODBUS_COM0_TYPE;

			if(Modbus.com_config[i] != pData[HeadLen + 5])
			{
				if(((i == 0) && (pData[HeadLen + 5] == NOUSE || pData[HeadLen + 5] == MODBUS_SLAVE || pData[HeadLen + 5] == MODBUS_MASTER || pData[HeadLen + 5] == BACNET_SLAVE)) ||
				((i == 1) && (pData[HeadLen + 5] == NOUSE || pData[HeadLen + 5] == MODBUS_SLAVE || pData[HeadLen + 5] == MODBUS_MASTER ||  pData[HeadLen + 5] == RS232_METER)) ||
				((i == 2) && (pData[HeadLen + 5] == NOUSE || pData[HeadLen + 5] == MODBUS_SLAVE || pData[HeadLen + 5] == MODBUS_MASTER || pData[HeadLen + 5] == BACNET_SLAVE || pData[HeadLen + 5] == BACNET_MASTER))
					)
			{
				Modbus.com_config[i] = pData[HeadLen + 5];
				if(Modbus.com_config[i] == MODBUS_SLAVE)
					uart_serial_restart(i);
				if(Modbus.com_config[i] == BACNET_SLAVE || Modbus.com_config[i] == BACNET_MASTER)
					Recievebuf_Initialize(i);
				if(Modbus.com_config[i] == MODBUS_MASTER)
				{
					Count_com_config();
					if(i == 1)  // zigbee port
						count_send_id_to_zigbee = 0;
				}
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
#if (ARM_MINI || ASIX_MINI)
	if((Modbus.mini_type == MINI_BIG_ARM) || (Modbus.mini_type == MINI_SMALL_ARM))
	{
		if(Modbus.com_config[1] == MODBUS_MASTER)
			UART1_SW = 1;
		else
			UART1_SW = 0;
	}
#endif
#endif
				E2prom_Write_Byte(EEP_COM0_CONFIG + i, Modbus.com_config[i]);
			}
		}
			
		}

		else if(StartAdd == MODBUS_REFRESH_FLASH)
		{
			Modbus.refresh_flash_timer = pData[HeadLen + 5];
			E2prom_Write_Byte(EEP_REFRESH_FLASH, Modbus.refresh_flash_timer );
		}
#if (ARM_MINI || ASIX_MINI)
		else if(StartAdd >= MODBUS_OUTPUT_1V && StartAdd <= MODBUS_OUTPUT_10V)
		{
			Modbus.start_adc[StartAdd - MODBUS_OUTPUT_1V + 1] = pData[HeadLen + 5]+ (pData[HeadLen + 4]<<8);		
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)			
			E2prom_Write_Byte(EEP_OUT_1V + (StartAdd - MODBUS_OUTPUT_1V) * 2, pData[HeadLen + 4]);
			E2prom_Write_Byte(EEP_OUT_1V + (StartAdd - MODBUS_OUTPUT_1V) * 2 + 1, pData[HeadLen + 5]);
#endif

#if (ASIX_MINI || ASIX_CM5)
			E2prom_Write_Byte(EEP_OUT_1V + (StartAdd - MODBUS_OUTPUT_1V),Modbus.start_adc[StartAdd - MODBUS_OUTPUT_1V + 1] / 10);
#endif			
			// caclulate slop

			cal_slop();


		}
		else if(StartAdd == MODBUS_VCC_ADC)
		{
			Modbus.vcc_adc = pData[HeadLen + 5]+ (pData[HeadLen + 4]<<8);;
			E2prom_Write_Byte(EEP_VCC_ADC_LO,Modbus.vcc_adc);
			E2prom_Write_Byte(EEP_VCC_ADC_HI,Modbus.vcc_adc >> 8);
		}
		else if(StartAdd == MODBUS_OUTPUT_TEST_VALUE)
		{
			test_adc = pData[HeadLen + 5]+ (pData[HeadLen + 4]<<8);
		}
		else if(StartAdd == MODBUS_OUTPUT_TEST_FLAG)
		{
			test_adc_flag = pData[HeadLen + 5];
		}
		else if(StartAdd == Modbus_Bacnet_Vendor_ID)
		{
			Bacnet_Vendor_ID = pData[HeadLen + 5];
			E2prom_Write_Byte(EEP_BAC_VENDOR_ID,pData[HeadLen + 5]);
		}
		else if(StartAdd == Modbus_Fix_Com_Config)
		{
			Modbus.fix_com_config = pData[HeadLen + 5];
			E2prom_Write_Byte(EEP_FIX_COM_CONFIG,pData[HeadLen + 5]);
			if(Modbus.fix_com_config == 0)
			{
				// intial serial port
			}
		}

		else if(StartAdd == MODBUS_OUTPUT_MODE)
		{
			flag_output = pData[HeadLen + 5];
			E2prom_Write_Byte(EEP_OUTPUT_MODE,pData[HeadLen + 5]);
		}

#endif
//		else if(StartAdd == MODBUS_NETWORK)
//		{
//			Modbus.network_number = pData[HeadLen + 5]+ (pData[HeadLen + 4]<<8);
//			E2prom_Write_Byte(EEP_NETWORK,pData[HeadLen + 5]);
//			//E2prom_Write_Byte(EEP_PORT_HIGH,pData[HeadLen + 4]);
//		}
		else if(StartAdd == MODBUS_MSTP_NETWORK)
		{
			mstp_network = pData[HeadLen + 5]+ (pData[HeadLen + 4]<<8);
			E2prom_Write_Byte(EEP_MSTP_NETWORK_LO,pData[HeadLen + 5]);
			E2prom_Write_Byte(EEP_MSTP_NETWORK_HI,pData[HeadLen + 4]);
		}		
		else if(StartAdd == MODBUS_BBMD_EN)
		{
			bbmd_en = pData[HeadLen + 5];
			E2prom_Write_Byte(EEP_BBMD_EN,pData[HeadLen + 5]);
		}
#if !(ARM_WIFI)
		else if(StartAdd == MODBUS_BACKLIGHT)
		{
			Modbus.backlight = pData[HeadLen + 5];
			E2prom_Write_Byte(EEP_BACKLIGHT,pData[HeadLen + 5]);
			BACKLIT = (Modbus.backlight != 0)? 1 : 0;
		}
#endif
		else if(StartAdd == MODBUS_EN_TIME_SYNC_PC)
		{
			Modbus.en_time_sync_with_pc = pData[HeadLen + 5];
			E2prom_Write_Byte(EEP_EN_TIME_SYNC_PC,pData[HeadLen + 5]);
		}
		else if(StartAdd == MODBUS_TCP_TYPE)
		{
			if(pData[HeadLen + 5] <= 1 )
			{
				Modbus.tcp_type = pData[HeadLen + 5]+ (pData[HeadLen + 4]<<8);			
				E2prom_Write_Byte(EEP_TCP_TYPE,  Modbus.tcp_type );
			}
		}
		else if(StartAdd >= MODBUS_IP_1 && StartAdd <= MODBUS_IP_4)
		{
			 Modbus.ip_addr[StartAdd - MODBUS_IP_1] = pData[HeadLen + 5];	

#if (ASIX_MINI || ASIX_CM5)
			 E2prom_Write_Byte(EEP_IP + 3 - (StartAdd - MODBUS_IP_1), Modbus.ip_addr[StartAdd - MODBUS_IP_1]);
#endif
			
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			E2prom_Write_Byte(EEP_IP + (StartAdd - MODBUS_IP_1), Modbus.ip_addr[StartAdd - MODBUS_IP_1]);
#endif
			
//			 IntFlashWriteByte(0x4001,0);  ?????????????????????? CM5 have trouble
//			 AX11000_SoftReboot();
		}
		else if(StartAdd >= MODBUS_SUBNET_1 && StartAdd <= MODBUS_SUBNET_4)
		{
			 Modbus.subnet[StartAdd - MODBUS_SUBNET_1] = pData[HeadLen + 5];
#if (ASIX_MINI || ASIX_CM5)
			 E2prom_Write_Byte(EEP_SUBNET + 3 - ( StartAdd - MODBUS_SUBNET_1), Modbus.subnet[StartAdd - MODBUS_SUBNET_1]);
#endif
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			 E2prom_Write_Byte(EEP_SUBNET + ( StartAdd - MODBUS_SUBNET_1), Modbus.subnet[StartAdd - MODBUS_SUBNET_1]);
#endif
		}
		else if(StartAdd >= MODBUS_GETWAY_1 && StartAdd <= MODBUS_GETWAY_4)
		{
			 Modbus.getway[StartAdd - MODBUS_GETWAY_1] = pData[HeadLen + 5];
#if (ASIX_MINI || ASIX_CM5)
			 E2prom_Write_Byte(EEP_GETWAY + 3 - (StartAdd - MODBUS_GETWAY_1), Modbus.getway[StartAdd - MODBUS_GETWAY_1]);
#endif
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			 E2prom_Write_Byte(EEP_GETWAY + (StartAdd - MODBUS_GETWAY_1), Modbus.getway[StartAdd - MODBUS_GETWAY_1]);
#endif
		}
		else if(StartAdd == MODBUS_TCP_LISTEN_PORT)
		{
			Modbus.tcp_port =  pData[HeadLen + 5]+ (pData[HeadLen + 4]<<8);
			E2prom_Write_Byte(EEP_PORT_LOW,pData[HeadLen + 5]);
			E2prom_Write_Byte(EEP_PORT_HIGH,pData[HeadLen + 4]);
//			IntFlashWriteByte(0x4001,0);
//			AX11000_SoftReboot();
		}	
		else if(StartAdd == MODBUS_BIP_PORT)
		{
			Modbus.Bip_port =  pData[HeadLen + 5]+ (pData[HeadLen + 4]<<8);
			E2prom_Write_Byte(EEP_BACNET_PORT_LO,pData[HeadLen + 5]);
			E2prom_Write_Byte(EEP_BACNET_PORT_HI,pData[HeadLen + 4]);
//			IntFlashWriteByte(0x4001,0);
//			AX11000_SoftReboot();
		}	
		else if(StartAdd == MODBUS_UART0_PARITY)
		{
			Modbus.uart_parity[0] = pData[HeadLen + 5];
			E2prom_Write_Byte(EEP_UART0_PARITY,pData[HeadLen + 5]);
			UART_Init(0);
		}
//		else if(StartAdd == MODBUS_UART1_PARITY)
//		{
//			Modbus.uart_parity[1] = pData[HeadLen + 5];
//			//E2prom_Write_Byte(EEP_GSM_SEVER_OR_CLIENT,pData[HeadLen + 5]);
//		}
		else if(StartAdd == MODBUS_UART2_PARITY)
		{
			Modbus.uart_parity[2] = pData[HeadLen + 5];
			E2prom_Write_Byte(EEP_UART2_PARITY,pData[HeadLen + 5]);
			UART_Init(2);
		}
		else if(StartAdd == MODBUS_ZIGBEE_MOUDLE_ID)
		{
			Write_ZIGBEE_ID(pData[HeadLen + 5]+ (pData[HeadLen + 4]<<8));
		}
//		else if(StartAdd == MODBUS_NETWORK_UART0)
//		{
//			Modbus.network_ID[0] = pData[HeadLen + 5];
//			E2prom_Write_Byte(EEP_UART0_NETWORK,pData[HeadLen + 5]);
//		}
//		else if(StartAdd == MODBUS_NETWORK_UART1)
//		{
//			Modbus.network_ID[1] = pData[HeadLen + 5];
//			E2prom_Write_Byte(EEP_UART1_NETWORK,pData[HeadLen + 5]);
//		}
//		else if(StartAdd == MODBUS_NETWORK_UART2)
//		{
//			Modbus.network_ID[2] = pData[HeadLen + 5];
//			E2prom_Write_Byte(EEP_UART2_NETWORK,pData[HeadLen + 5]);
//		}
#if (ASIX_MINI || ASIX_CM5)
#if USB_HOST
		else if(StartAdd >= MODBUS_GSM_IP_1 && StartAdd <= MODBUS_GSM_IP_4)
		{
			 GSM_remote_IP[StartAdd - MODBUS_GSM_IP_1] = pData[HeadLen + 5];
			 E2prom_Write_Byte(EEP_GSM_IP1 + (StartAdd - MODBUS_GSM_IP_1), GSM_remote_IP[StartAdd - MODBUS_GSM_IP_1]);
		}
		else if(StartAdd == MODBUS_GSM_TCPPORT)
		{
			GSM_remote_tcpport =  pData[HeadLen + 5]+ (pData[HeadLen + 4]<<8);
			E2prom_Write_Byte(EEP_GSM_TCP_PORT_LO,pData[HeadLen + 5]);
			E2prom_Write_Byte(EEP_GSM_TCP_PORT_HI,pData[HeadLen + 4]);
		}
		else if(StartAdd == MODBUS_GSM_SEVER_OR_CLIENT)
		{
			flag_sever_or_client = pData[HeadLen + 5];
			E2prom_Write_Byte(EEP_GSM_SEVER_OR_CLIENT,pData[HeadLen + 5]);
		}
		
#endif
#endif
		else if(StartAdd == MODBUS_TEST_CMD )
		{

			if(pData[HeadLen + 5] == 111)
			{	
				flag_reboot = 1;
				
//				IntFlashWriteByte(0x4001,0);
//				AX11000_SoftReboot();
			}
#if (ASIX_MINI || ASIX_CM5)
			if(pData[HeadLen + 5] == 151)
			{
				TCP_IP_Init();
			}				
#endif
			
#if (ARM_MINI || ARM_CM5)
			if(pData[HeadLen + 5] == 151)
			{
				//IP_Change = 1;
				//tapdev_init() ;
				tcpip_intial();
			}	
#endif
			
		}

/****************av ai ao bi bo **********************************\*/

//		else if(StartAdd == MODBUS_RESERVED1)
//		{
//			vars[0].value = swap_double((U32_T)(pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8)) * 1000);
//		}
//		else if(StartAdd == MODBUS_RESERVED2)
//		{
//			vars[1].value = swap_double((U32_T)(pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8)) * 1000);
//		}
#if BAC_COMMON 
//		else if( StartAdd >= MODBUS_AV_FIRST && StartAdd <= MODBUS_AV_LAST )
//		{
//			AV_Present_Value[StartAdd - MODBUS_AV_FIRST] = pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8);
//		}
//		else if( StartAdd >= MODBUS_AI_FIRST && StartAdd <= MODBUS_AI_LAST )
//		{
//			AI_Present_Value[StartAdd - MODBUS_AI_FIRST] = pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8);
//		}
//		else if( StartAdd >= MODBUS_AO_FIRST && StartAdd <= MODBUS_AO_LAST )
//		{
//			AO_Present_Value[StartAdd - MODBUS_AO_FIRST] = pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8);
//		}
//		else if( StartAdd >= MODBUS_BI_FIRST && StartAdd <= MODBUS_BI_LAST )
//		{
//			BI_Present_Value[StartAdd - MODBUS_BI_FIRST] = pData[HeadLen + 5];
//		}
//		else if( StartAdd >= MODBUS_BO_FIRST && StartAdd <= MODBUS_BO_LAST )
//		{
//			BO_Present_Value[StartAdd - MODBUS_BO_FIRST] = pData[HeadLen + 5];
//		}
#endif
/********************************************************\*/		
		else if( StartAdd >= MODBUS_TEST && StartAdd <= MODBUS_TEST_50 )
		{			
			Test[StartAdd - MODBUS_TEST] = pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8);
		}
		else if( StartAdd >= MODBUS_SD_BLOCK_A1 && StartAdd <= MODBUS_SD_BLOCK_A12 )
		{
			SD_block_num[2 * (StartAdd - MODBUS_SD_BLOCK_A1)] &= 0xf0000;
			SD_block_num[2 * (StartAdd - MODBUS_SD_BLOCK_A1)] |= pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8);
			E2prom_Write_Byte(EEP_SD_BLOCK_A1 + (StartAdd - MODBUS_SD_BLOCK_A1) * 2,pData[HeadLen + 5]);
			E2prom_Write_Byte(EEP_SD_BLOCK_A1 + (StartAdd - MODBUS_SD_BLOCK_A1) * 2 + 1,pData[HeadLen + 4]);
		}
		else if( StartAdd >= MODBUS_SD_BLOCK_D1 && StartAdd <= MODBUS_SD_BLOCK_D12 )
		{
			SD_block_num[2 * (StartAdd - MODBUS_SD_BLOCK_D1) + 1] &= 0xf0000;
			SD_block_num[2 * (StartAdd - MODBUS_SD_BLOCK_D1) + 1] |= pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8);
			E2prom_Write_Byte(EEP_SD_BLOCK_D1 + (StartAdd - MODBUS_SD_BLOCK_D1) * 2,pData[HeadLen + 5]);
			E2prom_Write_Byte(EEP_SD_BLOCK_D1 + (StartAdd - MODBUS_SD_BLOCK_D1) * 2 + 1,pData[HeadLen + 4]);
		}
		else if( StartAdd >= MODBUS_SD_BLOCK_H1 && StartAdd <= MODBUS_SD_BLOCK_H12 )
		{
			SD_block_num[2 * (StartAdd - MODBUS_SD_BLOCK_H1)] &= 0x0ffff;
			SD_block_num[2 * (StartAdd - MODBUS_SD_BLOCK_H1)] |= (U32_T)(pData[HeadLen + 5] << 16);
			
			SD_block_num[2 * (StartAdd - MODBUS_SD_BLOCK_H1) + 1] &= 0x0ffff;
			SD_block_num[2 * (StartAdd - MODBUS_SD_BLOCK_H1) + 1] |= (U32_T)(pData[HeadLen + 4] << 16);
			
			E2prom_Write_Byte(EEP_SD_BLOCK_HI1 + (StartAdd - MODBUS_SD_BLOCK_H1),((pData[HeadLen + 5] & 0x0f) + (pData[HeadLen + 4] & 0xf0)));
		}
		else if(StartAdd >= MODBUS_TIMER_ADDRESS && StartAdd <= MODBUS_TIMER_ADDRESS + 7)
		{
			 Rtc.all[StartAdd - MODBUS_TIMER_ADDRESS] = pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4]<<8);
		//	E2prom_Write_Byte(EEP_SEC + StartAdd - MODBUS_SEC, Time.all[StartAdd - MODBUS_SEC]);
			if(StartAdd - MODBUS_TIMER_ADDRESS == 0)  // sec
				Rtc_Set(Rtc.Clk.year,Rtc.Clk.mon,Rtc.Clk.day,Rtc.Clk.hour,Rtc.Clk.min,Rtc.all[StartAdd - MODBUS_TIMER_ADDRESS],0);
			else if(StartAdd - MODBUS_TIMER_ADDRESS == 1)  // sec
				Rtc_Set(Rtc.Clk.year,Rtc.Clk.mon,Rtc.Clk.day,Rtc.Clk.hour,Rtc.all[StartAdd - MODBUS_TIMER_ADDRESS],Rtc.Clk.sec,0);
			else if(StartAdd - MODBUS_TIMER_ADDRESS == 2)  // sec
				Rtc_Set(Rtc.Clk.year,Rtc.Clk.mon,Rtc.Clk.day,Rtc.all[StartAdd - MODBUS_TIMER_ADDRESS],Rtc.Clk.min,Rtc.Clk.sec,0);
			else if(StartAdd - MODBUS_TIMER_ADDRESS == 3)  // sec
				Rtc_Set(Rtc.Clk.year,Rtc.Clk.mon,Rtc.all[StartAdd - MODBUS_TIMER_ADDRESS],Rtc.Clk.hour,Rtc.Clk.min,Rtc.Clk.sec,0);
			else if(StartAdd - MODBUS_TIMER_ADDRESS == 5)  // sec
				Rtc_Set(Rtc.Clk.year,Rtc.all[StartAdd - MODBUS_TIMER_ADDRESS],Rtc.Clk.day,Rtc.Clk.hour,Rtc.Clk.min,Rtc.Clk.sec,0);
			else if(StartAdd - MODBUS_TIMER_ADDRESS == 6)  // sec
				Rtc_Set(Rtc.all[StartAdd - MODBUS_TIMER_ADDRESS],Rtc.Clk.mon,Rtc.Clk.day,Rtc.Clk.hour,Rtc.Clk.min,Rtc.Clk.sec,0);
#if (ASIX_MINI || ASIX_CM5)
			flag_Updata_Clock = 1;
#endif
		}		
//		else if(StartAdd == MODBUS_POINT_SEQ)
//		{
//			Modbus.point_sequence = pData[HeadLen + 5];
//			E2prom_Write_Byte(EEP_POINT_SEQ,pData[HeadLen + 5]);
//		}
		else if(StartAdd  >= MODBUS_OUTPUT_FIRST && StartAdd  <= MODBUS_OUTPUT_LAST)
		{
			int32_t tempval; 
		 	i = (StartAdd - MODBUS_OUTPUT_FIRST) / 2;
			tempval = swap_double(outputs[i].value);
			if((StartAdd - MODBUS_OUTPUT_FIRST) % 2 == 0)  // high word
			{
				tempval &= 0x0000ffff;
				tempval += 65536L * (pData[HeadLen + 5] +  256 * pData[HeadLen + 4]);
			}
			else  // low word
			{
				S32_T old_value;
				
				tempval &= 0xffff0000;
				tempval += (pData[HeadLen + 5] + 256 * pData[HeadLen + 4]);
				
				if(outputs[i].digital_analog == 0)  // digital
				{
					if(i < get_max_internal_output())
					{
						if(( outputs[i].range >= ON_OFF && outputs[i].range <= HIGH_LOW )
							||(outputs[i].range >= custom_digital1 // customer digital unit
							&& outputs[i].range <= custom_digital6
							&& digi_units[outputs[i].range - custom_digital1].direct == 1))
						{ // inverse
							output_priority[i][10] = (pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8)) ? 0 : 1;	
							outputs[i].control = Binary_Output_Present_Value(i) ? 0 : 1;	
						}
						else
						{
							output_priority[i][10] = (pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8)) ? 1 : 0;	
							outputs[i].control = Binary_Output_Present_Value(i) ? 1 : 0;	
						}
					}
					else
					{
						old_value = outputs[i].control;
						
						if(( outputs[i].range >= ON_OFF && outputs[i].range <= HIGH_LOW )
							||(outputs[i].range >= custom_digital1 // customer digital unit
							&& outputs[i].range <= custom_digital6
							&& digi_units[outputs[i].range - custom_digital1].direct == 1))
						{// inverse
							outputs[i].control = (pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8)) ? 0 : 1;	
						}	
						else
						{
							outputs[i].control =  (pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8)) ? 1 : 0;	
						}
					}
					if(outputs[i].control) 
						set_output_raw(i,1000);
					else 
						set_output_raw(i,0);		
#if  T3_MAP
				if(i >= get_max_internal_output())
				{					
						if(old_value != outputs[i].control)
						{	
						vTaskSuspend(Handle_Scan);	// dont not read expansion io
#if (ARM_MINI || ASIX_MINI)
						vTaskSuspend(xHandler_Output);  // do not control local io
#endif
						push_expansion_out_stack(&outputs[i],i,0);
#if (ARM_MINI || ASIX_MINI)
							// resume output task
						vTaskResume(xHandler_Output); 
#endif
						vTaskResume(Handle_Scan);		
						}
				}
#endif			
					outputs[i].value = Binary_Output_Present_Value(i) * 1000;
				}			
				else if(outputs[i].digital_analog)
				{
					if(i < get_max_internal_output())
							output_priority[i][10] = (float)(pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8)) / 1000;			
						// if external io
#if  T3_MAP
						if(i >= get_max_internal_output())
						{
							old_value = outputs[i].value; 
							
							output_raw[i] = (float)(pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8));
							if(old_value != (float)(pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8)))
							{
								vTaskSuspend(Handle_Scan);	// dont not read expansion io
#if (ARM_MINI || ASIX_MINI)
								vTaskSuspend(xHandler_Output);  // do not control local io
#endif
								push_expansion_out_stack(&outputs[i],i,0);
#if (ARM_MINI || ASIX_MINI)
									// resume output task
								vTaskResume(xHandler_Output); 
#endif
								vTaskResume(Handle_Scan);									
							}
						}
							
#endif
						outputs[i].value = Analog_Output_Present_Value(i) * 1000;
				} 
			}
			
			check_output_priority_array(i);
			
#if  T3_MAP
			if(i >= get_max_internal_output())
			{
				if( outputs[i].range >= OFF_ON && outputs[i].range <= LOW_HIGH )
				{
					if(pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8) == 1)
							outputs[i].control = 1;
						else 
							outputs[i].control = 0;
				}
				outputs[i].value = swap_double(tempval);		
				push_expansion_out_stack(&outputs[i],i,0);
			}
#endif
			
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			write_page_en[OUT] = 1;	
#endif
			ChangeFlash = 1;
		}
		else if(StartAdd  >= MODBUS_OUTPUT_RANGE_FIRST && StartAdd  <= MODBUS_OUTPUT_RANGE_LAST)
		{
		 	i = (StartAdd - MODBUS_OUTPUT_RANGE_FIRST);
			outputs[i].range = pData[HeadLen + 5];
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			write_page_en[OUT] = 1;	
#endif
			ChangeFlash = 1;

			push_expansion_out_stack(&outputs[i],i,1);
		}
		else if(StartAdd  >= MODBUS_OUTPUT_AM_FIRST && StartAdd  <= MODBUS_OUTPUT_AM_LAST)
		{
		 	i = (StartAdd - MODBUS_OUTPUT_AM_FIRST);
			if(outputs[i].switch_status == SW_AUTO)
				outputs[i].auto_manual = pData[HeadLen + 5];
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			write_page_en[OUT] = 1;	
#endif
			ChangeFlash = 1;

			push_expansion_out_stack(&outputs[i],i,1);
		}
		else if(StartAdd  >= MODBUS_OUTPUT_AD_FIRST && StartAdd  <= MODBUS_OUTPUT_AD_LAST)
		{
		 	i = (StartAdd - MODBUS_OUTPUT_AD_FIRST);
			outputs[i].digital_analog = pData[HeadLen + 5];
			
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			write_page_en[OUT] = 1;	
#endif
			ChangeFlash = 1;
			push_expansion_out_stack(&outputs[i],i,1);
		}
		else if(StartAdd  >= MODBUS_INPUT_FIRST && StartAdd  <= MODBUS_INPUT_LAST)
		{
			int32_t tempval; 
		 	i = (StartAdd - MODBUS_INPUT_FIRST) / 2;
			tempval = swap_double(inputs[i].value);
			if((StartAdd - MODBUS_INPUT_FIRST) % 2 == 0)  // high word
			{
				tempval &= 0x0000ffff;
				tempval += 65536L * (pData[HeadLen + 5] +  256 * pData[HeadLen + 4]);
			}
			else  // low word
			{
				tempval &= 0xffff0000;
				tempval += (pData[HeadLen + 5] + 256 * pData[HeadLen + 4]);
				
				if(inputs[i].digital_analog == 0)  // digital
				{
					if(( inputs[i].range >= ON_OFF && inputs[i].range <= HIGH_LOW )
					||(inputs[i].range >= custom_digital1 // customer digital unit
					&& inputs[i].range <= custom_digital6
					&& digi_units[inputs[i].range - custom_digital1].direct == 1))
					{ // inverse
						if(pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8) == 1)
							inputs[i].control = 0;
						else 
							inputs[i].control = 1;
					}
					else
					{
						if(pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8) == 1)
							inputs[i].control = 1;
						else 
							inputs[i].control = 0;
					}
				}			
				else if(inputs[i].digital_analog == 1)
				{
					 inputs[i].value = swap_double(1000l * (pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8)));
				} 
			}

			inputs[i].value = swap_double(tempval); 
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			write_page_en[IN] = 1;	
#endif
			ChangeFlash = 1;

			push_expansion_in_stack(&inputs[i]);
		}
		else if(StartAdd  >= MODBUS_INPUT_FILTER_FIRST && StartAdd  <= MODBUS_INPUT_FILTER_LAST)
		{	
		 	i = (StartAdd - MODBUS_INPUT_FILTER_FIRST);
			inputs[i].filter = pData[HeadLen + 5];
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			write_page_en[IN] = 1;	
#endif
			ChangeFlash = 1;

			push_expansion_in_stack(&inputs[i]);
		}
		else if(StartAdd  >= MODBUS_INPUT_CAL_FIRST && StartAdd  <= MODBUS_INPUT_CAL_LAST)
		{
		 	i = (StartAdd - MODBUS_INPUT_CAL_FIRST);
			inputs[i].calibration_hi = pData[HeadLen + 4];
			inputs[i].calibration_lo = pData[HeadLen + 5];
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			write_page_en[IN] = 1;	
#endif
			ChangeFlash = 1;

			push_expansion_in_stack(&inputs[i]);
		}
		else if(StartAdd  >= MODBUS_INPUT_CAL_SIGN_FIRST && StartAdd  <= MODBUS_INPUT_CAL_SIGN_LAST)
		{
		 	i = (StartAdd - MODBUS_INPUT_CAL_SIGN_FIRST);
			inputs[i].calibration_sign = pData[HeadLen + 5];
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			write_page_en[IN] = 1;	
#endif
			ChangeFlash = 1;

			push_expansion_in_stack(&inputs[i]);
		}
		else if(StartAdd  >= MODBUS_INPUT_RANGE_FIRST && StartAdd  <= MODBUS_INPUT_RANGE_LAST)
		{
		 	i = (StartAdd - MODBUS_INPUT_RANGE_FIRST);
			inputs[i].digital_analog = pData[HeadLen + 4];
			inputs[i].range = pData[HeadLen + 5];
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			write_page_en[IN] = 1;	
#endif
			ChangeFlash = 1;

			push_expansion_in_stack(&inputs[i]);
		}		
		else if(StartAdd  >= MODBUS_INPUT_AM_FIRST && StartAdd  <= MODBUS_INPUT_AM_LAST)
		{
		 	i = (StartAdd - MODBUS_INPUT_AM_FIRST);
			inputs[i].auto_manual = pData[HeadLen + 5];
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			write_page_en[IN] = 1;	
#endif
			ChangeFlash = 1;

			push_expansion_in_stack(&inputs[i]);
		}		
		else if(StartAdd  >= MODBUS_INPUT_HI_SPD_EN_FIRST && StartAdd  <= MODBUS_INPUT_HI_SPD_EN_LAST)
		{
		 	i = (StartAdd - MODBUS_INPUT_HI_SPD_EN_FIRST);
			high_spd_en[i] = pData[HeadLen + 5];  
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			write_page_en[IN] = 1;	
#endif
			ChangeFlash = 1;
		} 
		else if(StartAdd  >= MODBUS_INPUT_TYPE_FIRST && StartAdd  <= MODBUS_INPUT_TYPE_LAST)
		{
		 	i = (StartAdd - MODBUS_INPUT_TYPE_FIRST);
			input_type[i] = pData[HeadLen + 5];  
			//ChangeFlash = 1;
		}
		else if(StartAdd  >= MODBUS_VAR_FIRST && StartAdd  <= MODBUS_VAR_LAST)
		{			
			int32_t tempval; 
		 	i = (StartAdd - MODBUS_VAR_FIRST) / 2;
			tempval = swap_double(vars[i].value);
			if((StartAdd - MODBUS_VAR_FIRST) % 2 == 0)  // high word
			{
				tempval &= 0x0000ffff;
				tempval += 65536L * (pData[HeadLen + 5] +  256 * pData[HeadLen + 4]);
			}
			else  // low word
			{
				tempval &= 0xffff0000;
				tempval += (pData[HeadLen + 5] + 256 * pData[HeadLen + 4]);
				
				if(vars[i].digital_analog == 0)  // digital
				{
					if(( vars[i].range >= ON_OFF && vars[i].range <= HIGH_LOW )
						||(vars[i].range >= custom_digital1 // customer digital unit
						&& vars[i].range <= custom_digital6
						&& digi_units[vars[i].range - custom_digital1].direct == 1))
					{// inverse
						if(pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8) == 1)
							vars[i].control = 0;
						else 
							vars[i].control = 1;
					}
					else
					{
						if(pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8) == 1)
							vars[i].control = 1;
						else 
							vars[i].control = 0;
					}
					//vars[i].value = vars[i].control ? 1000 : 0;
				}			
//				else if(vars[i].digital_analog == 1)
//				{
//					 vars[i].value = swap_double(1000l * (pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8)));
//				} 
			}

			vars[i].value = swap_double(tempval); 
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			write_page_en[VAR] = 1;	
#endif
			ChangeFlash = 1;
			 
			} 
			else if(StartAdd  >= MODBUS_VAR_AM_FIRST && StartAdd  <= MODBUS_VAR_AM_LAST)
			{
				i = (StartAdd - MODBUS_VAR_AM_FIRST);
				vars[i].auto_manual = pData[HeadLen + 5];  
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			write_page_en[VAR] = 1;	
#endif
				ChangeFlash = 1;
			} 
			else if(StartAdd >= MODBUS_WR_AM_FIRST && StartAdd <= MODBUS_WR_AM_LAST)
			{
				i = (StartAdd - MODBUS_WR_AM_FIRST);
				weekly_routines[i].auto_manual = pData[HeadLen + 5]; 
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			write_page_en[WRT] = 1;	
#endif
				ChangeFlash = 1;				
			}	
			else if(StartAdd >= MODBUS_WR_OUT_FIRST && StartAdd <= MODBUS_WR_OUT_LAST)
			{
				i = (StartAdd - MODBUS_WR_OUT_FIRST);
				weekly_routines[i].value = pData[HeadLen + 5] ? 1 : 0;  
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			write_page_en[WRT] = 1;	
#endif
				ChangeFlash = 1;
			}	
			else if(StartAdd >= MODBUS_WR_HOLIDAY1_FIRST && StartAdd <= MODBUS_WR_HOLIDAY1_LAST)
			{
				i = (StartAdd - MODBUS_WR_HOLIDAY1_FIRST);
				weekly_routines[i].override_1.number = pData[HeadLen + 5];  
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			write_page_en[WRT] = 1;	
#endif
				ChangeFlash = 1;
			}	
			else if(StartAdd >= MODBUS_WR_STATE1_FIRST && StartAdd <= MODBUS_WR_STATE1_LAST)
			{
				i = (StartAdd - MODBUS_WR_STATE1_FIRST);
				weekly_routines[i].override_1_value = pData[HeadLen + 5]; 
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			write_page_en[WRT] = 1;	
#endif
				ChangeFlash = 1;
			}	
			else if(StartAdd >= MODBUS_WR_HOLIDAY2_FIRST && StartAdd <= MODBUS_WR_HOLIDAY2_LAST)
			{
				i = (StartAdd - MODBUS_WR_HOLIDAY2_FIRST);
				weekly_routines[i].override_2.number = pData[HeadLen + 5];  
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			write_page_en[WRT] = 1;	
#endif
				ChangeFlash = 1;
			}	
			else if(StartAdd >= MODBUS_WR_STATE2_FIRST && StartAdd <= MODBUS_WR_STATE2_LAST)
			{
				i = (StartAdd - MODBUS_WR_STATE2_FIRST);
				weekly_routines[i].override_2_value = pData[HeadLen + 5]; 
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			write_page_en[WRT] = 1;	
#endif
				ChangeFlash = 1;
			}	
		
// weekly_time
			else if(StartAdd >= MODBUS_WR_TIME_FIRST && StartAdd <= MODBUS_WR_TIME_LAST)
			{
				U8_T i,j,k; 
				i = (StartAdd - MODBUS_WR_TIME_FIRST) / (MAX_SCHEDULES_PER_WEEK * 8); // week index
				j = (StartAdd - MODBUS_WR_TIME_FIRST) %(MAX_SCHEDULES_PER_WEEK * 8) / 8; // day index
				k = (StartAdd - MODBUS_WR_TIME_FIRST) % (MAX_SCHEDULES_PER_WEEK * 8) % 8;  // seg index
				wr_times[i][j].time[k].hours = pData[HeadLen + 4];
				wr_times[i][j].time[k].minutes = pData[HeadLen + 5];
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			write_page_en[WR_TIME] = 1;	
#endif
				ChangeFlash = 1;
			}	
// end weekly
			
// start annual 
			else if(StartAdd >= MODBUS_AR_AM_FIRST && StartAdd <= MODBUS_AR_AM_LAST)
			{
				i = (StartAdd - MODBUS_AR_AM_FIRST);
				annual_routines[i].auto_manual = pData[HeadLen + 5]; 		
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			write_page_en[AR]	= 1;	
#endif
				ChangeFlash = 1;				
			}	
			else if(StartAdd >= MODBUS_AR_OUT_FIRST && StartAdd <= MODBUS_AR_OUT_LAST)
			{
				i = (StartAdd - MODBUS_AR_OUT_FIRST);
				annual_routines[i].value = pData[HeadLen + 5] ? 1 : 0; 
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			write_page_en[AR]	= 1;	
#endif
				ChangeFlash = 1;
			}	
			else if(StartAdd >= MODBUS_AR_TIME_FIRST && StartAdd <= MODBUS_AR_TIME_LAST)
			{
				U8_T i,j; 
				i = (StartAdd - MODBUS_AR_TIME_FIRST) / AR_DATES_SIZE;
				j = (StartAdd - MODBUS_AR_TIME_FIRST) % AR_DATES_SIZE;
				ar_dates[i][j] = pData[HeadLen + 5];
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			write_page_en[AR_DATA]	= 1;	
#endif
				ChangeFlash = 1;
			}
//			else if(StartAdd >= MODBUS_ID_FIRST && StartAdd <= MODBUS_ID_LAST)
//			{
//				i = (StartAdd - MODBUS_ID_FIRST);
//				ID_Config[i / ID_SIZE].all[i % ID_SIZE] = pData[HeadLen + 5]; 
//#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
//			write_page_en[ID_ROUTION]	= 1;	
//#endif
//				ChangeFlash = 1;
//			}
  // SUB INFO BLOCK

	}
	else if(cmd == MULTIPLE_WRITE)
	{
		if(type == SERIAL || type == USB || type == BAC_TO_MODBUS )   // TBD: need change
		{	
			U16_T far crc_val;
			U8_T far temp_buf[8];
			memcpy(temp_buf,pData,6);
			crc_val = crc16(temp_buf,6);
			temp_buf[6]	= crc_val >> 8;
			temp_buf[7]	= (U8_T)crc_val;
			if(type == SERIAL)
			{	
				uart_init_send_com(Modbus.main_port);
				uart_send_string(temp_buf, 8,Modbus.main_port);				
			}
#if (ASIX_MINI || ASIX_CM5)
#if USB_DEVICE
			else
			{
				 memcpy(UpBuf,sendbuf,8); 
				 UpIndex = 0;
				 ENDP2_NEED_UP_FLAG = 1;
				 UpCtr = 8;			
			}
#endif
#endif
		}
#if ARM_WIFI
		else // wifi
		{
			sendbuf[0] = pData[0];			//	TransID
			sendbuf[1] = pData[1];	
			sendbuf[2] = 0;			//	ProtoID
			sendbuf[3] = 0;
			sendbuf[4] = 0;	//	Len
			sendbuf[5] = 6;
			for (loop = 0;loop < 6;loop++)
			{
				sendbuf[HeadLen + loop] = pData[HeadLen + loop];	
			}
			uart_init_send_com(Modbus.main_port);
			uart_send_string(sendbuf, 6 + UIP_HEAD,Modbus.main_port);			
		}
#endif
#if !(ARM_WIFI)
		else
		{
		//	SetTransactionId(6 + UIP_HEAD);
		
			sendbuf[0] = pData[0];			//	TransID
			sendbuf[1] = pData[1];	
			sendbuf[2] = 0;			//	ProtoID
			sendbuf[3] = 0;
			sendbuf[4] = 0;	//	Len
			sendbuf[5] = 6;

			for (loop = 0;loop < 6;loop++)
			{
				sendbuf[HeadLen + loop] = pData[HeadLen + loop];	
			}
#if (ASIX_MINI || ASIX_CM5)
			if(cSemaphoreTake( xSemaphore_tcp_send, ( portTickType ) 10 ) == pdTRUE)
			{				
				TCPIP_TcpSend(pHttpConn->TcpSocket, sendbuf, 6 + UIP_HEAD, TCPIP_SEND_NOT_FINAL);  
				cSemaphoreGive( xSemaphore_tcp_send );
			}
#endif
			
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			memcpy(tcp_server_sendbuf,sendbuf,6 + UIP_HEAD);
      tcp_server_sendlen = 6 + UIP_HEAD;
#endif
		}
#endif
		// the following are schedule registers 
		////ChangeFlash = 1;
#if !(ARM_WIFI)
		if(StartAdd == MODBUS_MAC_1)
		{ 	
			if(Mac_Address_write_enable)
			{
				if(pData[HeadLen + 6] == 12)
				{	 
					Modbus.mac_addr[0] = pData[HeadLen + 8];
					Modbus.mac_addr[1] = pData[HeadLen + 10];
					Modbus.mac_addr[2] = pData[HeadLen + 12];
					Modbus.mac_addr[3] = pData[HeadLen + 14];
					Modbus.mac_addr[4] = pData[HeadLen + 16];
					Modbus.mac_addr[5] = pData[HeadLen + 18];
				}
#if (ASIX_MINI || ASIX_CM5)
				E2prom_Write_Byte(EEP_MAC, Modbus.mac_addr[5]);
				E2prom_Write_Byte(EEP_MAC + 1, Modbus.mac_addr[4]);
				E2prom_Write_Byte(EEP_MAC + 2, Modbus.mac_addr[3]);
				E2prom_Write_Byte(EEP_MAC + 3, Modbus.mac_addr[2]);
				E2prom_Write_Byte(EEP_MAC + 4, Modbus.mac_addr[1]);
				E2prom_Write_Byte(EEP_MAC + 5, Modbus.mac_addr[0]);
#endif
				
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
				E2prom_Write_Byte(EEP_MAC, Modbus.mac_addr[0]);
				E2prom_Write_Byte(EEP_MAC + 1, Modbus.mac_addr[1]);
				E2prom_Write_Byte(EEP_MAC + 2, Modbus.mac_addr[2]);
				E2prom_Write_Byte(EEP_MAC + 3, Modbus.mac_addr[3]);
				E2prom_Write_Byte(EEP_MAC + 4, Modbus.mac_addr[4]);
				E2prom_Write_Byte(EEP_MAC + 5, Modbus.mac_addr[5]);
				IP_Change = 1 ;
#endif
				Mac_Address_write_enable = 0;
				
			}
		}
		else
#endif
		 if(StartAdd  >= MODBUS_NAME1 && StartAdd <= MODBUS_NAME_END)
	  {
			if(pData[HeadLen +6] <= 20)
			{
				for(i = 0;i < pData[HeadLen + 6];i++)			//	(data_buffer[6]*2)
				{
	 				//AT24CXX_WriteOneByte((EEP_PANEL_NAME1 + i),pData[HeadLen + 7+i]);
 					panelname[i] = pData[HeadLen + 7 + i];
				}
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
				ChangeFlash = 1;
				write_page_en[24] = 1;
#endif			
			}
	  }		
		else if(StartAdd == MODBUS_CUSTOMER_DEVICE)
		{
			
			if(pData[HeadLen + 6] == 32)
			{	 
				// check whether id is in database
				//if(get_port_by_id(pData[HeadLen + 8]) == 0) // if id is not on DB
				if(pData[HeadLen + 10] >= CUSTOMER_PRODUCT)  // it is customer device
				{		
					U32_T sn;
					sn = pData[HeadLen + 14] + (U16_T)(pData[HeadLen + 16] << 8) + (U32_T)(pData[HeadLen + 18] << 16) + (U32_T)(pData[HeadLen + 20] << 24);

					if(check_id_in_database(pData[HeadLen + 8], sn,pData[HeadLen + 12],get_baut_by_port(pData[HeadLen + 12] + 1),0) == 1)
					{  // add new one
						scan_db[db_ctr - 1].product_model = pData[HeadLen + 10];						
						memcpy(tstat_name[db_ctr - 1],&pData[HeadLen + 21],16);
						flag_tstat_name[db_ctr - 1] = 1;
					}
					else
					{ // repalce old one
						U8_T index;
						if(get_index_by_id(pData[HeadLen + 8],&index) == 1)
						{
							scan_db[index].product_model = pData[HeadLen + 10];						
							memcpy(tstat_name[index],&pData[HeadLen + 21],16);
							flag_tstat_name[index] = 1;
						}
					}

				}
			}
		}
		else if(StartAdd == MODBUS_TIMER_ADDRESS)
		{
		 	if(pData[HeadLen + 5] == 0x08)
		 	{
				if(pData[HeadLen + 5] == pData[HeadLen + 6])
				{
//					Set_Clock(PCF_SEC,pData[HeadLen + 14]);  // sec
//					Set_Clock(PCF_MIN,pData[HeadLen + 13]);  // min
//					Set_Clock(PCF_HOUR,pData[HeadLen + 12]); // hour
//					Set_Clock(PCF_DAY,pData[HeadLen + 11]);  // day 
//					Set_Clock(PCF_WEEK,pData[HeadLen + 10]); // week
					if(pData[HeadLen + 7] |= 0x80)
						pData[HeadLen + 9]|=0x80;
					else									   
						pData[HeadLen + 9]&=0x7f;	
//					Set_Clock(PCF_MON,pData[HeadLen + 9]);   // month
//					Set_Clock(PCF_YEAR,pData[HeadLen + 8]);	// year
					Rtc_Set(pData[HeadLen + 8],pData[HeadLen + 9],pData[HeadLen + 11],pData[HeadLen + 12],pData[HeadLen + 13],pData[HeadLen + 14],0);
				}
				else
				{
//					Set_Clock(PCF_SEC,pData[HeadLen + 22]);  // sec
//					Set_Clock(PCF_MIN,pData[HeadLen + 20]);  // min
//					Set_Clock(PCF_HOUR,pData[HeadLen + 18]); // hour
//					Set_Clock(PCF_DAY,pData[HeadLen + 16]);  // day 
//					Set_Clock(PCF_WEEK,pData[HeadLen + 14]); // week
					if(pData[HeadLen + 8] |= 0x80)
						pData[HeadLen + 12]|=0x80;
					else
						pData[HeadLen + 12]&=0x7f;	
//					Set_Clock(PCF_MON,pData[HeadLen + 12]);   		// month
//					Set_Clock(PCF_YEAR,pData[HeadLen + 10]);		// year
						Rtc_Set(pData[HeadLen + 10],pData[HeadLen + 12],pData[HeadLen + 16],pData[HeadLen + 18],pData[HeadLen + 20],pData[HeadLen + 22],0);

				}
			//	calibrated_time = 1;
			 
			 }
		 }
		
/**************************************** read by block ********************************************/
		 else if(StartAdd  >= MODBUS_USER_BLOCK_FIRST && StartAdd  <= MODBUS_USER_BLOCK_LAST)
		 {
			 // dealwith_block
			 write_user_data_by_block(StartAdd,HeadLen,pData);
		 }
/**************************************** read by block ********************************************/  	
		else if(StartAdd == MODBUS_T3000_PRIVATE)
		{
			if(pData[HeadLen + 5] == 0x07)
			{				
				T3000_Private.sn = pData[HeadLen + 10] + (U16_T)(pData[HeadLen + 12] << 8) \
												+ ((U32_T)pData[HeadLen + 14] << 16) + ((U32_T)pData[HeadLen + 16] << 24);
				T3000_Private.oldid = pData[HeadLen + 18];
				T3000_Private.newid = pData[HeadLen + 20];
				if(assignment_id_with_sn(T3000_Private.oldid, T3000_Private.newid, T3000_Private.sn,conflict_port)== ASSIGN_ID)	
				{		
					T3000_Private.flag = 2;	// successfully					
				}					
				else
				{
					T3000_Private.flag = 3; // fail
				}
				clear_conflict_id(); // delete conflict id
			}
			
		}
		else if(StartAdd >= MODBUS_OUTPUT_FIRST && StartAdd <= MODBUS_OUTPUT_LAST)
		{
			if(pData[HeadLen + 5] == 0x02)
			{
				int32_t tempval; 
				i = (StartAdd - MODBUS_OUTPUT_FIRST) / 2;
				tempval = pData[HeadLen + 10] + (U16_T)(pData[HeadLen + 9] << 8) \
					+ ((U32_T)pData[HeadLen + 8] << 16) + ((U32_T)pData[HeadLen + 7] << 24);

					
				if(outputs[i].digital_analog == 0)  // digital
				{
					if(( outputs[i].range >= ON_OFF && outputs[i].range <= HIGH_LOW )
						||(outputs[i].range >= custom_digital1 // customer digital unit
						&& outputs[i].range <= custom_digital6
						&& digi_units[outputs[i].range - custom_digital1].direct == 1))
					{// inverse
						if(tempval == 1)
							outputs[i].control = 0;
						else 
							outputs[i].control = 1;
					}
					else
					{
						if(tempval == 1)
							outputs[i].control = 1;
						else 
							outputs[i].control = 0;
					}
				}	
				
				outputs[i].value = swap_double(tempval); 
			}
				
				
		} 
		else if(StartAdd >= MODBUS_INPUT_FIRST && StartAdd <= MODBUS_INPUT_LAST)
		{
			if(pData[HeadLen + 5] == 0x02)
			{
				int32_t tempval; 
				i = (StartAdd - MODBUS_VAR_FIRST) / 2;
				tempval = pData[HeadLen + 10] + (U16_T)(pData[HeadLen + 9] << 8) \
					+ ((U32_T)pData[HeadLen + 8] << 16) + ((U32_T)pData[HeadLen + 7] << 24);

					
				if(inputs[i].digital_analog == 0)  // digital
				{
					if(( inputs[i].range >= ON_OFF && vars[i].range <= HIGH_LOW )
						||(inputs[i].range >= custom_digital1 // customer digital unit
						&& inputs[i].range <= custom_digital6
						&& digi_units[inputs[i].range - custom_digital1].direct == 1))
					{// inverse
						if(tempval == 1)
							inputs[i].control = 0;
						else 
							inputs[i].control = 1;
					}
					else
					{
						if(tempval == 1)
							inputs[i].control = 1;
						else 
							inputs[i].control = 0;
					}
				}	
				
				inputs[i].value = swap_double(tempval); 
			}
				
				
		} 
		else if(StartAdd >= MODBUS_VAR_FIRST && StartAdd <= MODBUS_VAR_LAST)
		{
			if(pData[HeadLen + 5] == 0x02)
			{
				int32_t tempval; 
				i = (StartAdd - MODBUS_VAR_FIRST) / 2;
				tempval = pData[HeadLen + 10] + (U16_T)(pData[HeadLen + 9] << 8) \
					+ ((U32_T)pData[HeadLen + 8] << 16) + ((U32_T)pData[HeadLen + 7] << 24);

					
				if(vars[i].digital_analog == 0)  // digital
				{
					if(( vars[i].range >= ON_OFF && vars[i].range <= HIGH_LOW )
						||(vars[i].range >= custom_digital1 // customer digital unit
						&& vars[i].range <= custom_digital6
						&& digi_units[vars[i].range - custom_digital1].direct == 1))
					{// inverse
						if(tempval == 1)
							vars[i].control = 0;
						else 
							vars[i].control = 1;
					}
					else
					{
						if(tempval == 1)
							vars[i].control = 1;
						else 
							vars[i].control = 0;
					}
					//vars[i].value = vars[i].control ? 1000 : 0;
				}	
				
				vars[i].value = swap_double(tempval); 
			}
				
				
		} 
#if !(ARM_WIFI)
		else if(StartAdd == MODBUS_OUTPUT_1V)
		{
			if(pData[HeadLen + 6] == 20)
			{
				char i;
				for(i = 1;i <= 10;i++)
				{
					Modbus.start_adc[i] = (U16_T)(pData[HeadLen + i * 2 + 5] << 8) + pData[HeadLen + i * 2 + 6];
				
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)			
			E2prom_Write_Byte(EEP_OUT_1V + i * 2 - 2, pData[HeadLen + i * 2 + 5]);
			E2prom_Write_Byte(EEP_OUT_1V + i * 2 - 1, pData[HeadLen + i * 2 + 6]);
#endif
	
#if (ASIX_MINI || ASIX_CM5)
			E2prom_Write_Byte(EEP_OUT_1V + i - 1,Modbus.start_adc[i] / 10);
#endif					
				}
				
		
			// caclulate slop

			cal_slop();				
			}
		}
#endif
	}
	else if(cmd == CHECKONLINE)
	{
		if(type == SERIAL || type == USB)
		{
			U8_T temp_buf[9];
//			U8_T i;
			U16_T crc_val;
	
			temp_buf[0] = pData[HeadLen + 0];//[0];
			temp_buf[1] = pData[HeadLen + 1];//];
			temp_buf[2] = Modbus.address;
			temp_buf[3] = Modbus.serialNum[0];
			temp_buf[4] = Modbus.serialNum[1];
			temp_buf[5] = Modbus.serialNum[2];
			temp_buf[6] = Modbus.serialNum[3];			
			
			crc_val = crc16(temp_buf,7);

			temp_buf[7] = HIGH_BYTE(crc_val);
			temp_buf[8] = LOW_BYTE(crc_val);
			if(type == SERIAL)
			{
			   // transfer scan commad to tstat
				if((pData[HeadLen + 1] == CHECKONLINE)) // 0x19 UART0
				{
					if((Modbus.address <= pData[HeadLen + 2]) && 
					(Modbus.address >= pData[HeadLen + 3]))
					{
				  		uart_init_send_com(Modbus.main_port);
						  uart_send_string(temp_buf,9,Modbus.main_port);

					}
				}

//				Modbus.sub_port = 0xff;			
//				for(i = 0;i <  sub_no ;i++)
//				{
//					if(pData[UIP_HEAD] == uart0_sub_addr[i])
//					{	
//						Modbus.sub_port = 0;
//					}
//#if (ARM_MINI || ASIX_MINI)
//					else if(pData[UIP_HEAD] == uart2_sub_addr[i])
//					{
//						 Modbus.sub_port = 2;
//					}
//					else if(pData[UIP_HEAD] == uart1_sub_addr[i])
//					{	
//						Modbus.sub_port = 1;
//					}
//#endif
//				}
//				if(Modbus.sub_port == 0xff)
//				{  
//					return;
//				}
//
//				vTaskSuspend(xHandleMornitor_task);
//				vTaskSuspend(Handle_Scan); 
//				vTaskSuspend(Handle_ParameterOperation);
//				vTaskSuspend(xHandler_Output); 
////				vTaskSuspend(xHandleBACnetComm); 
//				vTaskSuspend(xHandleCommon);
//				vTaskSuspend(xHandleBacnetControl);
//#if (ARM_MINI || ASIX_MINI)			
//			//	 vTaskSuspend(xHandleUSB); 
//				 vTaskSuspend(xHandler_SPI);
//#endif	   
//
//
//			Response_MAIN_To_SUB(&pData[HeadLen],4/*main_rece_size - 2*/,Modbus.sub_port);
//	
//			vTaskResume(xHandleMornitor_task); 
//			vTaskResume(Handle_Scan); 
//			vTaskResume(Handle_ParameterOperation);
//	//		vTaskResume(Handle_MainSerial);
//			vTaskResume(xHandler_Output); 
////			vTaskResume(xHandleBACnetComm); 
//			vTaskResume(xHandleCommon);
//			vTaskResume(xHandleBacnetControl);
//#if (ARM_MINI || ASIX_MINI)			
//	//		vTaskResume(xHandleUSB); 
//			vTaskResume(xHandler_SPI);
//#endif			

			}
#if (ASIX_MINI || ASIX_CM5)
#if USB_DEVICE
			else
			{
				if((pData[HeadLen + 1] == CHECKONLINE)) // 0x19 UART0
				{
					if((Modbus.address <= pData[HeadLen + 2]) && 
					(Modbus.address >= pData[HeadLen + 3]))
					{
						memcpy(UpBuf,temp_buf,9); 
						UpIndex = 0;
						ENDP2_NEED_UP_FLAG = 1;
						UpCtr = 9;
					}
				}						
			}
#endif
#endif
		}
		else
		{
			U16_T far crc_val;
			U8_T tempbuf[6];
			U8_T i;
#if (ASIX_MINI || ASIX_CM5)
			TcpSocket_ME = pHttpConn->TcpSocket;

		  transaction_id = ((U16_T)pData[0] << 8) | pData[1];
			transaction_num	= 2 * pData[UIP_HEAD + 5] + 3;
#endif
 			for(i = 0;i < 4;i++) 
			{ 
				tempbuf[i] = pData[UIP_HEAD+i];									   
			}
			crc_val = crc16(tempbuf,4);		
			tempbuf[4] = crc_val >> 8;
			tempbuf[5] = (U8_T)crc_val;

		}
	}
	flag_sem_response_modbus = 0;

}
	
void dealwith_write_setting(Str_Setting_Info * ptr)
{

// compare sn to check whether it is current panel	
	if(ptr->reg.sn == swap_double(Modbus.serialNum[0] + (U16_T)(Modbus.serialNum[1] << 8)	+ ((U32_T)Modbus.serialNum[2] << 16) + ((U32_T)Modbus.serialNum[3] << 24)))
	{		
		if(memcmp(panelname,ptr->reg.panel_name,20))
		{
			ptr->reg.panel_name[19] = 0;
			memcpy(panelname,ptr->reg.panel_name,20);
			Set_Object_Name(panelname);
			
		}		
#if !(ARM_WIFI)		
		if(Modbus.en_time_sync_with_pc != ptr->reg.en_time_sync_with_pc)
		{
			
			flag_Update_Sntp = 0;
			Update_Sntp_Retry = 0;
			count_sntp = 0;
			// start SYNC with PC
			Setting_Info.reg.update_time_sync_pc = 1;
			Modbus.en_time_sync_with_pc = ptr->reg.en_time_sync_with_pc;	
			E2prom_Write_Byte(EEP_EN_TIME_SYNC_PC,Modbus.en_time_sync_with_pc);
		}
		
		if((Modbus.en_sntp != ptr->reg.en_sntp) || ((Modbus.en_sntp == 5) && memcmp(sntp_server,Setting_Info.reg.sntp_server,30)))
		{ 			
			Modbus.en_sntp = ptr->reg.en_sntp;	
			
			if(Modbus.en_sntp <= 5)
			{
				E2prom_Write_Byte(EEP_EN_SNTP,Modbus.en_sntp);
				if(Modbus.en_sntp >= 2)
				{	
					
						if(Modbus.en_sntp == 5)  // defined by customer
						{
							memcpy(sntp_server,Setting_Info.reg.sntp_server,30);
#if (ASIX_MINI || ASIX_CM5)
							DNSC_Start(sntp_server);
#endif
						
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
							resolv_query(sntp_server);
#endif
						}
						sntp_select_time_server(Modbus.en_sntp);
						flag_Update_Sntp = 0;
						Update_Sntp_Retry = 0;
						count_sntp = 0;

						
						SNTPC_Start(timezone, (((U32_T)SntpServer[0]) << 24) | ((U32_T)SntpServer[1] << 16) | ((U32_T)SntpServer[2] << 8) | (SntpServer[3]));
					
				}
			}
		}	
		
		if(timezone != swap_word(ptr->reg.time_zone))
		{ 
			Sync_timestamp(swap_word(ptr->reg.time_zone),timezone,0,0);
			timezone = swap_word(ptr->reg.time_zone);
			E2prom_Write_Byte(EEP_TIME_ZONE_HI,(U8_T)(timezone >> 8));
			E2prom_Write_Byte(EEP_TIME_ZONE_LO,(U8_T)timezone);
			
		}		
		if(Daylight_Saving_Time!= ptr->reg.Daylight_Saving_Time)
		{ 	
			Sync_timestamp(0,0,ptr->reg.Daylight_Saving_Time,Daylight_Saving_Time);
			
			Daylight_Saving_Time = ptr->reg.Daylight_Saving_Time;
//			update_timers();
			
			E2prom_Write_Byte(EEP_DAYLIGHT_SAVING_TIME,(U8_T)Daylight_Saving_Time);

		}		
		if(Modbus.en_dyndns != ptr->reg.en_dyndns)
		{ 						
			Modbus.en_dyndns = ptr->reg.en_dyndns;
			if(Modbus.en_dyndns == 1)
			{	
				dyndns_select_domain(dyndns_provider);
			// reconnect dyndns server again
				flag_Update_Dyndns = 0;
				Update_Dyndns_Retry = 0;
				DynDNS_Init();
			}
			E2prom_Write_Byte(EEP_EN_DYNDNS,Modbus.en_dyndns);
			
		}

		if(dyndns_provider != ptr->reg.dyndns_provider)
		{ 				
			dyndns_provider = ptr->reg.dyndns_provider;
			E2prom_Write_Byte(EEP_DYNDNS_PROVIDER,dyndns_provider);
		}
		if(dyndns_update_time != swap_word(ptr->reg.dyndns_update_time))
		{ 						
			dyndns_update_time = swap_word(ptr->reg.dyndns_update_time);
//							if(dyndns_update_time < 6)	dyndns_update_time = 6;
			E2prom_Write_Byte(EEP_DYNDNS_UPDATE_LO,dyndns_update_time);	
			E2prom_Write_Byte(EEP_DYNDNS_UPDATE_HI,dyndns_update_time >> 8);
		}
		
		memcpy(dyndns_domain_name,ptr->reg.dyndns_domain,MAX_DOMAIN_SIZE);
		memcpy(dyndns_username,ptr->reg.dyndns_user,MAX_USERNAME_SIZE);
		memcpy(dyndns_password,ptr->reg.dyndns_pass,MAX_PASSWORD_SIZE);
#endif
		
		memcpy(Modbus.mac_addr,ptr->reg.mac_addr,6);
		if(Modbus.com_config[0] != ptr->reg.com_config[0])
		{
			Modbus.com_config[0] = ptr->reg.com_config[0];
			if((Modbus.com_config[0] == MODBUS_SLAVE) || (Modbus.com_config[0] == 0))
				uart_serial_restart(0);
//							if(Modbus.com_config[0] == MAIN_MSTP)
//								Recievebuf_Initialize(0);
			//if(Modbus.com_config[0] == MODBUS_MASTER)
				Count_com_config();
			E2prom_Write_Byte(EEP_COM0_CONFIG, Modbus.com_config[0]);
		}
		if(Modbus.com_config[1] != ptr->reg.com_config[1])
		{
			Modbus.com_config[1] = ptr->reg.com_config[1];
			if((Modbus.com_config[1] == MODBUS_SLAVE) || (Modbus.com_config[1] == 0))
				uart_serial_restart(1);	
			if(Modbus.com_config[1] == MODBUS_MASTER)
			{
				Count_com_config();
				count_send_id_to_zigbee = 0;	

			}
			Count_com_config();
			
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
#if (ARM_MINI || ASIX_MINI)
		if((Modbus.mini_type == MINI_BIG_ARM) || (Modbus.mini_type == MINI_SMALL_ARM))
		{
		if(Modbus.com_config[1] == MODBUS_MASTER)
			UART1_SW = 1;
		else
			UART1_SW = 0;
	}
#endif
#endif
			E2prom_Write_Byte(EEP_COM1_CONFIG, Modbus.com_config[1]);
		}
		if(Modbus.com_config[2] != ptr->reg.com_config[2])
		{
			Modbus.com_config[2] = ptr->reg.com_config[2];
			if((Modbus.com_config[2] == MODBUS_SLAVE) || (Modbus.com_config[2] == 0))
				uart_serial_restart(2);	
			if(Modbus.com_config[2] == BACNET_SLAVE || Modbus.com_config[2] == BACNET_MASTER)
				Recievebuf_Initialize(2);
			//if(Modbus.com_config[2] == MODBUS_MASTER)
				Count_com_config();
			
			E2prom_Write_Byte(EEP_COM2_CONFIG, Modbus.com_config[2]);
		}

		if(Modbus.en_username != ptr->reg.en_username)
		{
			Modbus.en_username = ptr->reg.en_username;
			E2prom_Write_Byte(EEP_USER_NAME,Modbus.en_username);
		}
		if(Modbus.cus_unit != ptr->reg.cus_unit)
		{ 						
			Modbus.cus_unit = ptr->reg.cus_unit;
			E2prom_Write_Byte(EEP_USER_NAME,Modbus.cus_unit);
		}
#if (ASIX_MINI || ASIX_CM5)
#if (USB_HOST || USB_DEVICE)
		if(Modbus.usb_mode != ptr->reg.usb_mode)
		{
			Modbus.usb_mode = ptr->reg.usb_mode;
			E2prom_Write_Byte(EEP_USB_MODE,Modbus.usb_mode);
		#if USB_DEVICE
			if(Modbus.usb_mode == 0)
			{
				USB_device_Initial();
			}
		#endif
			
		#if  USB_HOST

			if(Modbus.usb_mode == 1)
			{
				USB_HOST_initial();
			}
		#endif
		}
#endif
#endif
		
		if(Modbus.address != ptr->reg.modbus_id)
		{ 
			if(ptr->reg.modbus_id > 0 && ptr->reg.modbus_id < 255)
			{
				Modbus.address = ptr->reg.modbus_id;
				E2prom_Write_Byte(EEP_ADDRESS,Modbus.address);
				Station_NUM = Modbus.address;
				Setting_Info.reg.MSTP_ID = Station_NUM;	
			}
		}
		if((Station_NUM != ptr->reg.MSTP_ID) && (ptr->reg.MSTP_ID != 0) && (ptr->reg.MSTP_ID != 255))
		{
			Station_NUM = ptr->reg.MSTP_ID;
			//E2prom_Write_Byte(EEP_STATION_NUM,Station_NUM);
			Setting_Info.reg.MSTP_ID = Station_NUM;		
			Modbus.address = Station_NUM;
			E2prom_Write_Byte(EEP_ADDRESS,  Modbus.address);			
		
		}
		
		if(panel_number != ptr->reg.panel_number)
		{ 
			if(ptr->reg.panel_number > 0 && ptr->reg.panel_number < 255)
			{
				panel_number = ptr->reg.panel_number;
				E2prom_Write_Byte(EEP_PANEL_NUMBER,panel_number);
				change_panel_number_in_code(Setting_Info.reg.panel_number,panel_number);
				Setting_Info.reg.panel_number	= panel_number;
			}
		}
		
		if((Instance != swap_double(ptr->reg.instance)) && (ptr->reg.instance != 0))
		{		
			Instance = swap_double(ptr->reg.instance);
			Device_Set_Object_Instance_Number(Instance);
			E2prom_Write_Byte(EEP_INSTANCE1, Instance);
			E2prom_Write_Byte(EEP_INSTANCE2, (U8_T)(Instance >> 8));
			E2prom_Write_Byte(EEP_INSTANCE3, (U8_T)(Instance >> 16));
			E2prom_Write_Byte(EEP_INSTANCE4, (U8_T)(Instance >> 24));
		}		
		
//		if(Modbus.refresh_flash_timer != ptr->reg.refresh_flash_timer)
//		{ 
//			Modbus.refresh_flash_timer = ptr->reg.refresh_flash_timer;
//			E2prom_Write_Byte(EEP_REFRESH_FLASH, Modbus.refresh_flash_timer );
//		}
		if(Modbus.external_nodes_plug_and_play != ptr->reg.en_plug_n_play)
		{
			Modbus.external_nodes_plug_and_play = ptr->reg.en_plug_n_play;
			E2prom_Write_Byte(EEP_EN_NODE_PLUG_N_PLAY, Modbus.external_nodes_plug_and_play);
		}
 
		if(uart0_baudrate != ptr->reg.com_baudrate[0]) // com_baudrate[2]??T3000
		{
			uart0_baudrate = ptr->reg.com_baudrate[0];
			if((Modbus.com_config[0] == MODBUS_SLAVE) || (Modbus.com_config[0] == NOUSE) || (Modbus.com_config[0] == MODBUS_MASTER))
				E2prom_Write_Byte(EEP_UART0_BAUDRATE, uart0_baudrate);
			UART_Init(0);
		}

		if(uart1_baudrate != ptr->reg.com_baudrate[1])
		{
			uart1_baudrate = ptr->reg.com_baudrate[1];
			E2prom_Write_Byte(EEP_UART1_BAUDRATE, uart1_baudrate);
			UART_Init(1);
		}

		if(uart2_baudrate != ptr->reg.com_baudrate[2])  // com_baudrate[0]??T3000
		{
			uart2_baudrate = ptr->reg.com_baudrate[2]; 
			if((Modbus.com_config[2] == MODBUS_SLAVE) || (Modbus.com_config[2] == BACNET_SLAVE) || (Modbus.com_config[2] == BACNET_MASTER) || (Modbus.com_config[2] == NOUSE) || (Modbus.com_config[2] == MODBUS_MASTER))
				E2prom_Write_Byte(EEP_UART2_BAUDRATE, uart2_baudrate);		
		
			UART_Init(2);
		}

//		if(Modbus.network_ID[0] != ptr->reg.network_ID[0])
//		{
//			Modbus.network_ID[0] = ptr->reg.network_ID[0];
//			E2prom_Write_Byte(EEP_UART0_NETWORK, Modbus.network_ID[0]);
//		}
//		if(Modbus.network_ID[1] != ptr->reg.network_ID[1])
//		{
//			Modbus.network_ID[1] = ptr->reg.network_ID[1];
//			E2prom_Write_Byte(EEP_UART1_NETWORK, Modbus.network_ID[1]);
//		}
//		if(Modbus.network_ID[2] != ptr->reg.network_ID[2])
//		{
//			Modbus.network_ID[2] = ptr->reg.network_ID[2];
//			E2prom_Write_Byte(EEP_UART2_NETWORK, Modbus.network_ID[2]);
//		}
		
		if(ptr->reg.reset_default == 88)	// reset default 
		{
			flag_reset_default = 1;
			ptr->reg.reset_default = 0;
		}
		if(ptr->reg.reset_default == 111)	 // reboot
		{
			flag_reboot = 1;
			ptr->reg.reset_default = 0;
		}
		if(ptr->reg.reset_default == 150)	 // clear db
		{
			ptr->reg.reset_default = 0;
			clear_scan_db(); 
		}
#if !(ARM_WIFI)
		if(ptr->reg.reset_default == 99)	 // update sntp right now
		{
			ptr->reg.reset_default = 0;
			ptr->reg.sync_with_ntp_result = 0;
			flag_Update_Sntp = 0;
			Update_Sntp_Retry = 0;
			count_sntp = 0;
			SNTPC_Start(timezone, (((U32_T)SntpServer[0]) << 24) | ((U32_T)SntpServer[1] << 16) | ((U32_T)SntpServer[2] << 8) | (SntpServer[3]));
		}
#if (ARM_MINI || ASIX_MINI || ARM_CM5)
		if(Modbus.backlight != ptr->reg.backlight)
		{
			Modbus.backlight = ptr->reg.backlight;
			E2prom_Write_Byte(EEP_BACKLIGHT,Modbus.backlight);
			BACKLIT = (Modbus.backlight != 0)? 1 : 0;
		}
#endif			
		if((memcmp(Modbus.ip_addr,ptr->reg.ip_addr,4) && !((ptr->reg.ip_addr[0] == 0) && (ptr->reg.ip_addr[1] == 0) \
				&& (ptr->reg.ip_addr[2] == 0) && (ptr->reg.ip_addr[3] == 0)))
			|| (memcmp(Modbus.subnet,ptr->reg.subnet,4) && !((ptr->reg.subnet[0] == 0) && (ptr->reg.subnet[1] == 0) \
				&& (ptr->reg.subnet[2] == 0) && (ptr->reg.subnet[3] == 0)))
		|| (memcmp(Modbus.getway,ptr->reg.getway,4) && !((ptr->reg.getway[0] == 0) && (ptr->reg.getway[1] == 0) \
				&& (ptr->reg.getway[2] == 0) && (ptr->reg.getway[3] == 0)))
		|| (Modbus.tcp_port != swap_word(ptr->reg.tcp_port) && (Modbus.tcp_port != 0))
		|| (Modbus.tcp_type != ptr->reg.tcp_type)
		)
		{
			
			if(memcmp(Modbus.ip_addr,ptr->reg.ip_addr,4) && !((ptr->reg.ip_addr[0] == 0) && (ptr->reg.ip_addr[1] == 0) \
				&& (ptr->reg.ip_addr[2] == 0) && (ptr->reg.ip_addr[3] == 0)))
			{	 
				memcpy(Modbus.ip_addr,ptr->reg.ip_addr,4);
#if (ASIX_MINI || ASIX_CM5)				
				E2prom_Write_Byte(EEP_IP + 3, Modbus.ip_addr[0]);
				E2prom_Write_Byte(EEP_IP + 2, Modbus.ip_addr[1]);
				E2prom_Write_Byte(EEP_IP + 1, Modbus.ip_addr[2]);
				E2prom_Write_Byte(EEP_IP + 0, Modbus.ip_addr[3]);
#endif
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)			
				E2prom_Write_Byte(EEP_IP + 0, Modbus.ip_addr[0]);
				E2prom_Write_Byte(EEP_IP + 1, Modbus.ip_addr[1]);
				E2prom_Write_Byte(EEP_IP + 2, Modbus.ip_addr[2]);
				E2prom_Write_Byte(EEP_IP + 3, Modbus.ip_addr[3]);
#endif				
				//flag_reboot = 1;
				

				if(Modbus.com_config[2] == BACNET_SLAVE || Modbus.com_config[2] == BACNET_MASTER)
				{
					Send_I_Am_Flag = 1;
				}
			}
			if(memcmp(Modbus.subnet,ptr->reg.subnet,4) && !((ptr->reg.subnet[0] == 0) && (ptr->reg.subnet[1] == 0) \
				&& (ptr->reg.subnet[2] == 0) && (ptr->reg.subnet[3] == 0)))
			{
				memcpy(Modbus.subnet,ptr->reg.subnet,4);
#if (ASIX_MINI || ASIX_CM5)		
				E2prom_Write_Byte(EEP_SUBNET + 3, Modbus.subnet[0]);
				E2prom_Write_Byte(EEP_SUBNET + 2, Modbus.subnet[1]);
				E2prom_Write_Byte(EEP_SUBNET + 1, Modbus.subnet[2]);
				E2prom_Write_Byte(EEP_SUBNET + 0, Modbus.subnet[3]);	
#endif

#if (ARM_MINI || ARM_CM5 || ARM_WIFI)		
				E2prom_Write_Byte(EEP_SUBNET + 0, Modbus.subnet[0]);
				E2prom_Write_Byte(EEP_SUBNET + 1, Modbus.subnet[1]);
				E2prom_Write_Byte(EEP_SUBNET + 2, Modbus.subnet[2]);
				E2prom_Write_Byte(EEP_SUBNET + 3, Modbus.subnet[3]);	
#endif
						
				//flag_reboot = 1;
			}
			if(memcmp(Modbus.getway,ptr->reg.getway,4) && !((ptr->reg.getway[0] == 0) && (ptr->reg.getway[1] == 0) \
				&& (ptr->reg.getway[2] == 0) && (ptr->reg.getway[3] == 0)))
			{
				
				memcpy(Modbus.getway,ptr->reg.getway,4);
#if (ASIX_MINI || ASIX_CM5)
				E2prom_Write_Byte(EEP_GETWAY + 3, Modbus.getway[0]);
				E2prom_Write_Byte(EEP_GETWAY + 2, Modbus.getway[1]);
				E2prom_Write_Byte(EEP_GETWAY + 1, Modbus.getway[2]);
				E2prom_Write_Byte(EEP_GETWAY + 0, Modbus.getway[3]);
#endif
				
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
				E2prom_Write_Byte(EEP_GETWAY + 0, Modbus.getway[0]);
				E2prom_Write_Byte(EEP_GETWAY + 1, Modbus.getway[1]);
				E2prom_Write_Byte(EEP_GETWAY + 2, Modbus.getway[2]);
				E2prom_Write_Byte(EEP_GETWAY + 3, Modbus.getway[3]);
#endif			
				//flag_reboot = 1;
			}
			if(Modbus.tcp_port != swap_word(ptr->reg.tcp_port) && (Modbus.tcp_port != 0))
			{ 						
				Modbus.tcp_port = swap_word(ptr->reg.tcp_port);
				E2prom_Write_Byte(EEP_PORT_LOW,Modbus.tcp_port);
				E2prom_Write_Byte(EEP_PORT_HIGH,Modbus.tcp_port >> 8);
				//flag_reboot = 1;
			}
			if(Modbus.tcp_type != ptr->reg.tcp_type)
			{
				if(ptr->reg.tcp_type <= 1)
				{
				Modbus.tcp_type = ptr->reg.tcp_type;			
				E2prom_Write_Byte(EEP_TCP_TYPE,  Modbus.tcp_type );	
				}
				//flag_reboot = 1;				
			}			
			
#if (ASIX_MINI || ASIX_CM5)
			flag_reboot = 1;
#endif
			
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
			IP_Change = 1;
#endif
		}
#endif		
		
		if(Modbus.zigbee_module_id != ptr->reg.zigbee_module_id)
		{
			Write_ZIGBEE_ID(ptr->reg.zigbee_module_id);
			//Modbus.zigbee_module_id = ptr->reg.zigbee_module_id;
		}
#if ARM_MINI		
		if(MAX_MASTER != ptr->reg.MAX_MASTER)
		{
			MAX_MASTER = ptr->reg.MAX_MASTER;
			E2prom_Write_Byte(EEP_MAX_MASTER,MAX_MASTER);	
		}
#endif
	 }
}

void write_user_data_by_block(U16_T StartAdd,U8_T HeadLen,U8_T *pData) 
{
	U8_T far i,j;

	if(StartAdd  >= MODBUS_SETTING_BLOCK_FIRST && StartAdd  <= MODBUS_SETTING_BLOCK_LAST)
	{		
		if((StartAdd - MODBUS_SETTING_BLOCK_FIRST) % 100 == 0)
		{				
			i = (StartAdd - MODBUS_SETTING_BLOCK_FIRST) / 100;
			memcpy(&Setting_Info.all[i * 100],&pData[HeadLen + 7],100); 	
		}
		// dealwith writen setting
		if(i == 1)
			dealwith_write_setting(&Setting_Info);
	}
	else if(StartAdd  >= MODBUS_OUTPUT_BLOCK_FIRST && StartAdd  <= MODBUS_OUTPUT_BLOCK_LAST)
	{
		if((StartAdd - MODBUS_OUTPUT_BLOCK_FIRST) % ((sizeof(Str_out_point) + 1) / 2) == 0)
		{
			i = (StartAdd - MODBUS_OUTPUT_BLOCK_FIRST) / ((sizeof(Str_out_point) + 1) / 2);
			memcpy(&outputs[i],&pData[HeadLen + 7],sizeof(Str_out_point));
		}
	}
	else if(StartAdd  >= MODBUS_INPUT_BLOCK_FIRST && StartAdd  <= MODBUS_INPUT_BLOCK_LAST)
	{
		if((StartAdd - MODBUS_INPUT_BLOCK_FIRST) % ((sizeof(Str_in_point) + 1	) / 2) == 0)
		{
			i = (StartAdd - MODBUS_INPUT_BLOCK_FIRST) / ((sizeof(Str_in_point) + 1) / 2);
			memcpy(&inputs[i],&pData[HeadLen + 7],sizeof(Str_in_point)); 	
		}
	}
	else if(StartAdd  >= MODBUS_VAR_BLOCK_FIRST && StartAdd  <= MODBUS_VAR_BLOCK_LAST)
	{
		if((StartAdd - MODBUS_VAR_BLOCK_FIRST) % ((sizeof(Str_variable_point) + 1	) / 2) == 0)
		{
			i = (StartAdd - MODBUS_VAR_BLOCK_FIRST) / ((sizeof(Str_variable_point) + 1) / 2);
			memcpy(&vars[i],&pData[HeadLen + 7],sizeof(Str_variable_point)); 	
		}
	}
	else if(StartAdd  >= MODBUS_PRG_BLOCK_FIRST && StartAdd  <= MODBUS_PRG_BLOCK_LAST)
	{
		if((StartAdd - MODBUS_PRG_BLOCK_FIRST) % ((sizeof(Str_program_point) + 1	) / 2) == 0)
		{
			i = (StartAdd - MODBUS_PRG_BLOCK_FIRST) / ((sizeof(Str_program_point) + 1) / 2);
			memcpy(&programs[i],&pData[HeadLen + 7],sizeof(Str_program_point)); 	
		}
	}
	else if(StartAdd  >= MODBUS_WR_BLOCK_FIRST && StartAdd  <= MODBUS_WR_BLOCK_LAST)
	{
		if((StartAdd - MODBUS_WR_BLOCK_FIRST) % ((sizeof(Str_weekly_routine_point) + 1	) / 2) == 0)
		{
			i = (StartAdd - MODBUS_VAR_BLOCK_FIRST) / ((sizeof(Str_weekly_routine_point) + 1) / 2);
			memcpy(&weekly_routines[i],&pData[HeadLen + 7],sizeof(Str_weekly_routine_point)); 	
		}
	}
	else if(StartAdd  >= MODBUS_AR_BLOCK_FIRST && StartAdd  <= MODBUS_AR_BLOCK_LAST)
	{
		if((StartAdd - MODBUS_AR_BLOCK_FIRST) % ((sizeof(Str_annual_routine_point) + 1	) / 2) == 0)
		{
			i = (StartAdd - MODBUS_AR_BLOCK_FIRST) / ((sizeof(Str_annual_routine_point) + 1) / 2);
			memcpy(&annual_routines[i],&pData[HeadLen + 7],sizeof(Str_annual_routine_point)); 	
		}
	}
	else if(StartAdd  >= MODBUS_WR_TIME_FIRST && StartAdd  <= MODBUS_WR_TIME_LAST)
	{
		if((StartAdd - MODBUS_WR_TIME_FIRST) % (sizeof(Wr_one_day) * MAX_SCHEDULES_PER_WEEK / 2) == 0)
		{		
			i = (StartAdd - MODBUS_WR_TIME_FIRST) / (sizeof(Wr_one_day) * MAX_SCHEDULES_PER_WEEK / 2);
			memcpy(&wr_times[i],&pData[HeadLen + 7],(sizeof(Wr_one_day) * MAX_SCHEDULES_PER_WEEK)); 	
		}
	}
	else if(StartAdd  >= MODBUS_AR_TIME_FIRST && StartAdd  <= MODBUS_AR_TIME_LAST)
	{
		if((StartAdd - MODBUS_AR_TIME_FIRST) % AR_DATES_SIZE == 0)
		{
			i = ((StartAdd - MODBUS_AR_TIME_FIRST) / AR_DATES_SIZE);
			memcpy(&ar_dates[i],&pData[HeadLen + 7],AR_DATES_SIZE); 	
		}
	}	

}



U16_T read_user_data_by_block(U16_T addr) 
{
	U8_T far index,item;	
	U16_T *block;			
	if( addr >= MODBUS_SETTING_BLOCK_FIRST && addr <= MODBUS_SETTING_BLOCK_LAST )
	{		
		index = (addr - MODBUS_SETTING_BLOCK_FIRST) / 100;
		block = (U16_T *)&Setting_Info.all[index * 200];
		item = (addr - MODBUS_SETTING_BLOCK_FIRST) % 100;			
	}
	else if( addr >= MODBUS_OUTPUT_BLOCK_FIRST && addr <= MODBUS_OUTPUT_BLOCK_LAST )
	{
		index = (addr - MODBUS_OUTPUT_BLOCK_FIRST) / ( (sizeof(Str_out_point) + 1) / 2);
		block = (U16_T *)&outputs[index];
		item = (addr - MODBUS_OUTPUT_BLOCK_FIRST) % ((sizeof(Str_out_point) + 1) / 2);
	}
	else if( addr >= MODBUS_INPUT_BLOCK_FIRST && addr <= MODBUS_INPUT_BLOCK_LAST )
	{
		index = (addr - MODBUS_INPUT_BLOCK_FIRST) / ((sizeof(Str_in_point) + 1) / 2);
		block = (U16_T *)&inputs[index];
		item = (addr - MODBUS_INPUT_BLOCK_FIRST) % ((sizeof(Str_in_point) + 1) / 2);
	}
	else if( addr >= MODBUS_VAR_BLOCK_FIRST && addr <= MODBUS_VAR_BLOCK_LAST )
	{
		index = (addr - MODBUS_VAR_BLOCK_FIRST) / ((sizeof(Str_variable_point) + 1) / 2);
		block = (U16_T *)&inputs[index];
		item = (addr - MODBUS_VAR_BLOCK_FIRST) % ((sizeof(Str_variable_point) + 1) / 2);
	}
	else if( addr >= MODBUS_PRG_BLOCK_FIRST && addr <= MODBUS_PRG_BLOCK_LAST )
	{

		index = (addr - MODBUS_PRG_BLOCK_FIRST) / ((sizeof(Str_program_point) + 1) / 2);
		block = (U16_T *)&programs[index];
		item = (addr - MODBUS_PRG_BLOCK_FIRST) % ((sizeof(Str_program_point) + 1) / 2);
	}
	else if( addr >= MODBUS_CODE_BLOCK_FIRST && addr <= MODBUS_CODE_BLOCK_LAST )
	{	
		index = (addr - MODBUS_CODE_BLOCK_FIRST) / 100;
		block = (U16_T *)&prg_code[index / (CODE_ELEMENT * MAX_CODE / 200)][CODE_ELEMENT * MAX_CODE % 200];
		item = (addr - MODBUS_CODE_BLOCK_FIRST) % 100;
	}
	else if( addr >= MODBUS_WR_BLOCK_FIRST && addr <= MODBUS_WR_BLOCK_LAST )
	{
		index = (addr - MODBUS_WR_BLOCK_FIRST) / ((sizeof(Str_weekly_routine_point) + 1) / 2);
		block = (U16_T *)&weekly_routines[index];
		item = (addr - MODBUS_WR_BLOCK_FIRST) % ((sizeof(Str_weekly_routine_point) + 1) / 2);
	}
	else if( addr >= MODBUS_WR_TIME_FIRST && addr <= MODBUS_WR_TIME_LAST )
	{
		index = (addr - MODBUS_WR_TIME_FIRST) / ((sizeof(Wr_one_day) * MAX_SCHEDULES_PER_WEEK + 1) / 2);
		block = (U16_T *)&wr_times[index];
		item = (addr - MODBUS_WR_TIME_FIRST) % ((sizeof(Wr_one_day) * MAX_SCHEDULES_PER_WEEK + 1) / 2);
	}
	else if( addr >= MODBUS_AR_BLOCK_FIRST && addr <= MODBUS_AR_BLOCK_LAST )
	{
		index = (addr - MODBUS_AR_BLOCK_FIRST) / ((sizeof(Str_annual_routine_point) + 1) / 2);
		block = (U16_T *)&annual_routines[index];
		item = (addr - MODBUS_AR_BLOCK_FIRST) % ((sizeof(Str_annual_routine_point) + 1) / 2);
		
	}
	else if( addr >= MODBUS_AR_TIME_FIRST && addr <= MODBUS_AR_TIME_LAST )
	{
	
		index = (addr - MODBUS_AR_TIME_FIRST) / (AR_DATES_SIZE / 2);
		block = (U16_T *)&ar_dates[index];
		item = (addr - MODBUS_AR_TIME_FIRST) % (AR_DATES_SIZE / 2);
	}
	
	return block[item];
	
}

#if 0		

void Response_Speical_Logic(U8_T *prog_code)
{
	U8_T len; 
	U8_T i;
	U16_T crc_val;

	len = prog_code[2];

	crc_val = crc16(prog_code,len + 3);

	if(crc_val == prog_code[len + 3] * 256 + prog_code[len + 4])
	{
		U8_T len;
		U8_T j;
		i = 3;
		if(prog_code[i] == 0xc0)
		{	// Digital ouput status
			U8_T j;
			U16_T temp_DO = prog_code[i + 1] * 256 + prog_code[i + 2];

			relay_value.word  = temp_DO;

			I2C_ByteWrite(0x60, SET_RELAY_LOW,relay_value.byte[1], I2C_STOP_COND);			
			I2C_ByteWrite(0x60, SET_RELAY_HI,relay_value.byte[0], I2C_STOP_COND);			

			for(j = 0;j < 12;j++)
			{
				if(temp_DO & (0x01 << j))	
					output_raw[j] = 512;
				else
					output_raw[j] = 0;	
			}
		 	i = i + 3;
		}
		if(prog_code[i] == 0xc4)
		{	// analog ouput status
			
			len = prog_code[i + 1];
			for(j = 0;j < len / 2;j++)
			{
				if(Modbus.mini_type == MINI_BIG)
				{
					output_raw[j + 12] =  prog_code[i + 2 + j * 2] * 256 + prog_code[i + 2 + j * 2 + 1];
				}
				else if(Modbus.mini_type == MINI_SMALL)
				{
					output_raw[j + 6] =  prog_code[i + 2 + j * 2] * 256 + prog_code[i + 2 + j * 2 + 1];
				}
				else if(Modbus.mini_type == MINI_TINY)
				{
					output_raw[j + 4] =  prog_code[i + 2 + j * 2] * 256 + prog_code[i + 2 + j * 2 + 1];
				}
			}
		 	i = i + len + 2; 
		}
		if(prog_code[i] == 0xc6)
		{	// PI  C6 04 01 02 01 01 (01 02 is defined to PI)
//			U8_T chanel;
			len = prog_code[i + 1];

			for(j = 0;j < len / 2;j++)
			{
				if(Modbus.mini_type == MINI_BIG)
				{
			   		inputs[26 + prog_code[i + 2 + j] - 1].range = 100;
				}
				else
				{
					inputs[10 + prog_code[i + 2 + j] - 1].range = 100;
				}
				
				if((prog_code[i + 2 + j + len / 2] & 0x0f) == 0)	// stop
					high_spd_en[prog_code[i + 2 + j] - 1] = 1;
				else if((prog_code[i + 2 + j + len / 2] & 0x0f) == 1)	
					high_spd_en[prog_code[i + 2 + j] - 1] = 0;
			}

//			for(j = len / 2;j < len;j++)
//			// the first half byte -initial counter, last half byte -- start / stop count
//			{
//				if(prog_code[i + 2 + j] & 0x0f == 0x01)	
//				{
//					high_spd_en[]	
//				}	
//			}
			


		 	i = i + 2 + len;
		}
		if(prog_code[i] == 0xc8)
		{	// logic program
			len = prog_code[i + 1];
			if(prog_code[i + 2] > 0)
				logic_in = prog_code[i + 2] - 1;
			if(prog_code[i + 3] > 0)
				logic_out = prog_code[i + 3] - 1;
			
			if((prog_code[i + 2] > 0) && (prog_code[i + 3] > 0))
				flag_logic_control = 1;
			logic_count = prog_code[i + 4] * 256 + prog_code[i + 5];

			high_spd_en[logic_in] = 0;
		 	i = i + 2 + len;
		}
			

	}



}
#endif

U8_T RS485_Get_Baudrate(void)
{
	return uart2_baudrate;
}

void switch_to_modbus(void)
{
//	Modbus.com_config[2] = MODBUS_SLAVE;
//	uart_serial_restart(2);
	// response

}


#if (ARM_MINI || ASIX_MINI)

RS232_CMD rs232_cmd;
void send_rs232_command(void)
{
	uint8_t length;
	if((Modbus.mini_type == MINI_BIG) || (Modbus.mini_type == MINI_BIG_ARM)) 
	{  // ONLY T3_BB have RS232 port
		if( rs232_cmd.baut == 9600)
			uart1_baudrate = UART_9600;
		
		UART_Init(1);
		Test[30]++;
		if(uart1_baudrate == UART_9600)
		{
			if(rs232_cmd.type == 1)  // meter
			{
				if(rs232_cmd.len < 10)
					uart_send_string(rs232_cmd.cmd, rs232_cmd.len ,1);
	//			else
	//				error
				// wait response
				if(rs232_cmd.cmd[0] == 'R')		
				{		
					set_subnet_parameters(RECEIVE, 13,1);
					Test[31]++;
					if(length = wait_subnet_response(100,1))	 
					{
						Test[32]++;							
						if(subnet_response_buf[11] == 0x0d && subnet_response_buf[12] == 0x0a)
						{	Test[33]++;			
							// wn0000.00kg
	//						memcpy(&Test[20],subnet_response_buf,10);
							if(subnet_response_buf[0] == 'w' && subnet_response_buf[1] == 'n')
							{Test[34]++;	
								rs232_cmd.res = 1000000l * (subnet_response_buf[2] - '0') + 100000l * (subnet_response_buf[3]- '0') \
								+ 10000l * (subnet_response_buf[4]- '0') + 1000l * (subnet_response_buf[5]- '0') \
							+ 100l * (subnet_response_buf[7]- '0') + 10l * (subnet_response_buf[8]- '0');

							}
						}					
					}
				}
				// or CMD is 'Z' 'T', no response
				
			}
		}	
	}
}




#endif



U8_T Get_Mini_Type(void)
{
	return Modbus.mini_type;
}


#if (ARM_MINI || ARM_CM5 || ARM_WIFI)
void SoftReset(void)
{
  __set_FAULTMASK(1);     
	NVIC_SystemReset();      
}

#endif