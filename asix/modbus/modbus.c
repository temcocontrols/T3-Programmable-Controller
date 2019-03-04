#include "main.h"    
#include "serial.h"
//#include "schedule.h" 
#include "stdlib.h" 
#include "pic.h"
#include <stdio.h>
#include <string.h>

//void Reset_Zigbee_Moudle(void);
void bip_Init(U16_T localPort);
void TCP_IP_Init(void);
void init_dyndns(void);
//void Write_SD(void);


S16_T far timezone;
U8_T far SntpServer[4];

extern U8_T flag_Update_Dyndns;
extern U8_T Update_Dyndns_Retry;

void write_user_data_by_block(U16_T StartAdd,U8_T HeadLen,U8_T *pData) ;
U16_T read_user_data_by_block(U16_T addr);
void test_alarm(uint8_t test);

U8_T logic_in;
U8_T logic_out;
U16_T logic_count;

void SET_VAV(U8_T level);
bit flag_logic_control;



U8_T pic_watchdog = 0;
U8_T far bbmd_en;
U16_T far mstp_network;
STR_MODBUS far Modbus;// _at_ 0x1000;
UN_Time RTC;
bit Mac_Address_write_enable = FALSE;

U8_T uart0_baudrate;
U8_T uart1_baudrate;
U8_T uart2_baudrate;

U16_T test_adc = 0;
U8_T test_adc_flag;




xTaskHandle xdata Handle_MainSerial;
//xTaskHandle xdata Handle_SubSerial;
//xQueueHandle qSubSerial_uart0;
//xQueueHandle qSubSerial_uart2;
xSemaphoreHandle sem_subnet_tx_uart0;
xSemaphoreHandle sem_subnet_tx_uart2;
xSemaphoreHandle sem_subnet_tx_uart1;


xQueueHandle qSubSerial_uart1;
bit flag_gsm_initial;

//UN_HIGH_COUNT 
U32_T high_spd_counter[HI_COMMON_CHANNEL];
U32_T high_spd_counter_tempbuf[HI_COMMON_CHANNEL];
U8_T high_spd_en[HI_COMMON_CHANNEL];

U8_T subnet_rec_package_size; 



U16_T uart0_rece_count;
U16_T uart1_rece_count;
U16_T uart0_rece_size = 0;
U16_T uart1_rece_size = 0;
U16_T main_rece_size = 0;
U8_T uart0_dealwithTag;
U8_T uart1_dealwithTag;
U8_T main_dealwithTag;
//U8_T far uart_serial_receive_timeout_count, sub1_serial_receive_timeout_count;
U8_T uart0_transmit_finished,uart1_transmit_finished;
U8_T far uart0_data_buffer[MAX_BUF_LEN];// _at_ 0x43100;
U8_T far uart1_data_buffer[MAX_BUF_LEN];// _at_ 0x43200;
U8_T far main_data_buffer[MAX_BUF_LEN];// _at_ 0x43300;
U8_T far subnet_response_buf[MAX_BUF_LEN];// _at_ 0x43400;

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




void vStartMainSerialTasks( U8_T uxPriority)
{
	sTaskCreate( main_dealwithData, "mainserailtask", MainSerialSTACK_SIZE, NULL, uxPriority, &Handle_MainSerial );
}


void initSerial(void)
{
	TransID = 1;
//	fix_tstat_position = 1;
//	add_id_by_hand = 0;
//	memset(tstat_position,0,SUB_NO);
#if CM5
	UART2_TXEN = RECEIVE;
#endif

	if(Modbus.com_config[0] == SUB_MODBUS)
	{		
		uart_serial_restart(0);
	}
	if(Modbus.com_config[1] == SUB_MODBUS)
		uart_serial_restart(1);
	if(Modbus.com_config[2] == SUB_MODBUS)
		uart_serial_restart(2);	
	uart0_rece_count = 0;
	uart0_dealwithTag = 0;
	uart0_rece_size = 0;
	uart1_rece_count = 0;
	uart1_dealwithTag = 0;
	uart1_rece_size = 0;
	main_rece_size = 0;
	main_dealwithTag = 0;

//	qSubSerial_uart0 = xQueueCreate(100, 1);
	vSemaphoreCreateBinary(sem_subnet_tx_uart0);
	vSemaphoreCreateBinary(sem_subnet_tx_uart2);
#if MINI
	if(Modbus.com_config[0] == SUB_MODBUS)
		 UART0_TXEN = RECEIVE;	
	else if(Modbus.com_config[2] == SUB_MODBUS || Modbus.com_config[2] == MAIN_MSTP)
	{
		#if MINI
		if(Modbus.mini_type == MINI_TINY)
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
		
	}
	qSubSerial_uart1 = xQueueCreate(5, MAX_BUF_LEN);
//	qSubSerial_uart2 = xQueueCreate(SUB_BUF_LEN, 1);
	
	vSemaphoreCreateBinary(sem_subnet_tx_uart1);


	logic_in = 0;
	logic_out = 0;
	logic_count = 0;

	flag_logic_control = 0;

#endif



// 	check if during ISP mode if the address has been changed
 	E2prom_Read_Byte(EEP_ADDRESS,& Modbus.address);

	// if it has not been changed, check the flash memory
	if( ( Modbus.address == 255) || ( Modbus.address == 0) )
	{
	//	if(E2prom_Read_Byte(EEP_ADDRESS, & address) )
		{
			 Modbus.address = 254;
			E2prom_Write_Byte(EEP_ADDRESS,  Modbus.address);
		}

	}
	else
    {
		E2prom_Write_Byte(EEP_ADDRESS,  Modbus.address);

	}

	// if data is blank, means first Time programming, thus put as default
	// Added by RL 02/11/04
	if( Modbus.address == 0 ||  Modbus.address == 255 ) 
		 Modbus.address = 254;



}
/****************************************************\
*	CRC CODE START
\****************************************************/
U8_T MainCRChi = 0xff;
U8_T MainCRClo = 0xff;


// Table of CRC values for high¡§Corder byt
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

void uart_init_send_com(U8_T port)
{
	if(port > 2) return;
#if CM5
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

#if MINI 
	UART_Init(port);
	if(port == 1)
	{		
		if(Modbus.com_config[1] == SUB_MODBUS) 		
	   		uart1_transmit_finished = 0;
	}
	else if(port == 0 )
	{
	   uart0_transmit_finished = 0;
	   UART0_TXEN = SEND;
		
	}
	else if(port == 2)
	{

		if(Modbus.mini_type == MINI_TINY)
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


}

void uart_send_byte(U8_T buffer,U8_T port)
{
	U16_T count = 0;
	if(port > 2) return;
#if CM5
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


#if MINI 
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


}


void uart_send_string(U8_T *p, U16_T length,U8_T port)
{
	U16_T i;

	if(port > 2) return;
	for(i = 0; i < length; i++)
	{
		uart_send_byte(p[i],port);
		
	}

	if(port == 2 ) 
	{		
		
// 		delay is important!!!!
		
		if(Modbus.com_config[2] == MASTER_MODBUS)
		{
			if(length > 100)
			{	
				if(uart2_baudrate == UART_9600)		DELAY_Ms(36);	
				else if(uart2_baudrate == UART_19200)	DELAY_Ms(16);	  // 83.2MS
				else if(uart2_baudrate == UART_38400)	DELAY_Ms(8);	  
				else if(uart2_baudrate == UART_57600)	DELAY_Ms(4);	 
				else if(uart2_baudrate == UART_115200)	DELAY_Ms(2);	  // 83.2MS
			}
			else 
			{				
				// to be fixed, different bautrate, delay is differnt, need fix it
				if(uart2_baudrate >= UART_19200)
					//DELAY_Ms(length / 10 + 1);  // for minipanel
				{
//					if((p[1] == 0x10) && (p[2] == 0x02) && (p[3] == 0xCB)) //when mitiple-wirte name, the delay is longer
//					{
//						DELAY_Ms(length / 10 + 1);
//					}
					if(p[1] == 0x10)/* && (p[6] == 46)*/ //when mitiple-wirte 
					{						
						DELAY_Ms(length / 8);
					}
					else // when update firmware for tstat, DELAY_Ms(1), 
						DELAY_Ms(1);  // for minipanel
				}				
				
				else if(uart2_baudrate == UART_9600)
					DELAY_Ms(2);  // for minipanel
			}	
				
		}
		else if(Modbus.com_config[2] == SUB_MODBUS)
		{
			DELAY_Ms(length / 5 + 1);
		}
		else if(Modbus.com_config[2] == MAIN_MSTP)
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
		
	}
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
		UART0_TXEN = io;	
	
	}  
#if MINI	
	if(port == 2 )
	{			
		hsurRxCount = 0;
		memset(hsurRxBuffer,0,hsurRxCount);
		if(Modbus.mini_type == MINI_TINY)
		{			
				UART2_TXEN_TINY = io;
		}
		else
		{
				UART2_TXEN_BIG = io;
		}
	}
	if(port == 1 )
	{
		uart1_rece_count = 0;
		memset(uart1_data_buffer,0,subnet_rec_package_size);
	}
#endif
}
  



U8_T wait_subnet_response(U16_T nDoubleTick,U8_T port)
{
	U16_T i, length;
	U8_T cTaskWokenByPost = FALSE;
	if(port > 2) return;
#if CM5
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

#if MINI 

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
			if((length = hsurRxCount) >= subnet_rec_package_size)
			{					
				memcpy(subnet_response_buf,hsurRxBuffer,length);
				return length;
			}
		}
		if(port == 1)
		{
			if((length = uart1_rece_count) >= subnet_rec_package_size)
			{	 
				memcpy(subnet_response_buf,uart1_data_buffer,length);
				return length;
			}			
		}		
		vTaskDelay(1);		
	} 

#endif
	timeout[port]++;	
	return 0;
}




#if MINI


void zigbee_gsm_modbus_int_hander(void)
{
//	U8_T flag;
	if(RI1 == 1)
	{	
		com_rx[1]++;
		flagLED_uart1_rx = 1; 
		RI1 = 0;	
		//if(uart1_rece_count > )
		uart1_data_buffer[uart1_rece_count++] = SBUF1;

		// check whether ZIGBEE moudle is attatched
		if(strstr(uart1_data_buffer,"zi") != NULL)
		{
			uart1_rece_count = 0;	
			if(Modbus.com_config[1] != MASTER_MODBUS)
			{
				Modbus.com_config[1] = MASTER_MODBUS;
				E2prom_Write_Byte(EEP_COM0_CONFIG + 1, Modbus.com_config[1]);
			}
		}
	}
	else if(TI1 == 1)
	{
		com_tx[1]++;
		TI1 = 0;
		uart1_transmit_finished = 1;
		flagLED_uart1_tx = 1; 
	}
	return;
}
#endif


void modubs_main_uart_int_hander()
{
	if(Modbus.main_port == 0)
	{		
		if(RI0 == 1)
		{	
			RI0 = 0;
			flagLED_uart0_rx = 1;
			com_rx[0]++;
			if(uart0_rece_count < MAX_BUF_LEN)
				uart0_data_buffer[uart0_rece_count++] = SBUF0;
			else
			{	
				uart_serial_restart(0);
			}
			
			if(uart0_rece_count == 1)
			{	
				uart0_rece_size = 8;
			}
			else if(uart0_rece_count == 4)
			{
				if((((U16_T)(uart0_data_buffer[2] << 8) + uart0_data_buffer[3]) == 0x0a) && (uart0_data_buffer[1] == WRITE_VARIABLES))
				{
					uart0_rece_size = DATABUFLEN_SCAN;
		//			uart1_serial_receive_timeout_count = 2;//SERIAL_RECEIVE_TIMEOUT;	
				}
				if((uart0_data_buffer[0] == 0xff) && (uart0_data_buffer[1] == 0x19))
				{  
					uart0_rece_size = 6;	
				}
				if(uart0_data_buffer[1] == 0x12)  // for customer
				{
// id (1 byte ) + function code (1 byte) + len ( 1 byte )
// C0 + DO status ( 2 bytes)     -- fixed
// C4 + len(1 byte) + AO status 				-- fixed
// C6 + len(1 byte PInum * 2) + PI					-- not fixed
// C8 + logic control		-- not fixed
					uart0_rece_size =  uart0_data_buffer[2] + 5;

				}

			}
			else if(uart0_rece_count == 7)
			{	
				if(uart0_data_buffer[1] == MULTIPLE_WRITE)
				{
					uart0_rece_size = uart0_data_buffer[5] * 2 + 9;	
				}
			}
			else if(uart0_rece_count >= uart0_rece_size)
			{	
				uart0_dealwithTag = VALID_PACKET;
			}
		}
		else if(TI0 == 1)
		{				
			uart0_transmit_finished = 1;
			flagLED_uart0_tx = 1; 
			com_tx[0]++;
			TI0 = 0;
		}
	}
	else if(Modbus.main_port == 1)
	{
		if(RI1 == 1)
		{
			com_rx[1]++;
			flagLED_uart1_rx = 1; 
		//	uart1_heartbeat = 0;
			RI1 = 0;		
			if(uart1_rece_count < MAX_BUF_LEN)
				uart1_data_buffer[uart1_rece_count++] = SBUF1;
			else
				uart_serial_restart(1);
	
		//	uart1_serial_receive_timeout_count = 2;//SERIAL_RECEIVE_TIMEOUT;
			if(uart1_rece_count == 1)
			{	
				uart1_rece_size = 8;
		//		uart1_serial_receive_timeout_count = 2;//SERIAL_RECEIVE_TIMEOUT;
			}
			else if(uart1_rece_count == 4)
			{
				if((((U16_T)(uart1_data_buffer[2] << 8) + uart1_data_buffer[3]) == 0x0a) && (uart1_data_buffer[1] == WRITE_VARIABLES))
				{
					uart1_rece_size = DATABUFLEN_SCAN;
		//			uart1_serial_receive_timeout_count = 2;//SERIAL_RECEIVE_TIMEOUT;	
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
		//		uart1_serial_receive_timeout_count = 0;
				uart1_dealwithTag = VALID_PACKET;
			}
		}
		else if(TI1 == 1)
		{	
			TI1 = 0;
			com_tx[1]++;
			uart1_transmit_finished = 1;
			flagLED_uart1_tx = 1; 
		}
	}
	return;
}
//------------------------serialport ----------------------------------
//	serial port interrupt , deal with all action about serial port. include receive data and 
//		send data and deal with interal.

void handle_uart1_RX(void) interrupt 6
{

#if MINI 
	if((Modbus.com_config[1] == MASTER_MODBUS) ||( Modbus.com_config[1] == NOUSE) || (Modbus.com_config[1] == RS232_METER))	
	{
		zigbee_gsm_modbus_int_hander();
	}
	else if(Modbus.com_config[1] == SUB_MODBUS)  // for main port 232, test port
#endif
	{
		Modbus.main_port = 1;
		modubs_main_uart_int_hander();
	}
}


void modubs_sub1_uart_int_hander(void)
{
	U16_T crc_val;
	if(RI0 == 1)
	{	
		flagLED_uart0_rx = 1;
		com_rx[0]++;
		RI0 = 0;
		
		if(uart0_rece_count < MAX_BUF_LEN)
				uart0_data_buffer[uart0_rece_count++] = SBUF0;
		else
		{				
			uart_serial_restart(0);
		}		
		

			if((uart0_rece_count == 6) && (uart0_data_buffer[0] == 0xff) && (uart0_data_buffer[1] == 0x19) \
			|| ( (uart0_rece_count == 8) && (uart0_data_buffer[1] == 0x03) && (uart0_data_buffer[7] != 0))) // receive data
			{							
				crc_val = crc16(uart0_data_buffer, uart0_rece_count - 2);
				if(crc_val == (uart0_data_buffer[uart0_rece_count - 2] << 8) + uart0_data_buffer[uart0_rece_count - 1])
				{
					Modbus.com_config[0] = SUB_MODBUS;
					Count_com_config();
					uart_serial_restart(0);
				}						
			}

  }
	else if(TI0 == 1)
	{
		TI0 = 0;
		uart0_transmit_finished = 1;
		flagLED_uart0_tx = 1; 
		com_tx[0]++;
	}
	return;

}



void handle_uart0_RX(void) interrupt 4
{
	if(Modbus.com_config[0] == SUB_MODBUS )
	{	
		Modbus.main_port = 0;
		modubs_main_uart_int_hander();
	}
	else 
	{
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
	UART0_TXEN = RECEIVE;
	}

#if CM5 
	else if(port == 2)
	{
		UART2_TXEN = RECEIVE;
		
	}
#endif		

#if MINI 
	else if(port == 1)
	{
	uart1_rece_count = 0;
	uart1_dealwithTag = 0;
	}
	else if(port == 2)
	{
		if(Modbus.mini_type == MINI_TINY)
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

}

	




// ------------------------dealwithdata -------------------------
// the routine dealwith data ,it has three steps.
// the 3 step is : 1 prepare to send data and init crc for next Tim
//				   2 dealwith interal
//                 3 organize the data of sending, and send data.
U8_T count_master_connect; 
void main_dealwithData(void)
{
	U16_T far address;
//	U8_T frametype;
//	
	portTickType xDelayPeriod = ( portTickType ) 50 / portTICK_RATE_MS;
	task_test.enable[10] = 1;

	for( ; ;)
	{ 	
		vTaskDelay(xDelayPeriod);
		task_test.count[10]++;

		if(uart2_timeout == 0) 	{ UART_Init(2);}	 
		
		if(Modbus.com_config[0] == SUB_MODBUS || Modbus.com_config[1] == SUB_MODBUS || Modbus.com_config[2] == SUB_MODBUS) 	
		{
			if(Modbus.com_config[2] == SUB_MODBUS)
			{	 	
				uart_serial_restart(2);
				if(hsurRxCount > 0) // receive data
				{		
					if(hsurRxBuffer[0] == 0x55 && hsurRxBuffer[1] == 0xff && hsurRxBuffer[2] == 0x01 && hsurRxBuffer[5] == 0x00 && hsurRxBuffer[6] == 0x00)
					{// mstp
							Modbus.com_config[2] = MAIN_MSTP;
							Recievebuf_Initialize(2);						
					}
					
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
			else	if(Modbus.main_port == 0)	
			{	
				memcpy(main_data_buffer,uart0_data_buffer,MAX_BUF_LEN);
				main_dealwithTag = uart0_dealwithTag;
				main_rece_size = uart0_rece_size;
			}
			else if(Modbus.main_port == 1)	
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
#if MINI									
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
					else
					{			
	
#if CM5
			Modbus.sub_port = 0;
#endif
						
#if MINI	
			
			if(Modbus.mini_type == MINI_VAV)
			{			
					Modbus.sub_port = 0;
			}
			else
			{
				U8_T i;	
				Modbus.sub_port = 255;
							
				if(Modbus.com_config[0] == MASTER_MODBUS)
					Modbus.sub_port = 0;
				else if(Modbus.com_config[1] == MASTER_MODBUS)
					Modbus.sub_port = 1;
				else if(Modbus.com_config[2] == MASTER_MODBUS)
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
							
						if(flag_resume_scan	== 0)
							vTaskSuspend(Handle_Scan); 
						vTaskSuspend(Handle_ParameterOperation);
	//					vTaskSuspend(Handle_MainSerial);
						vTaskSuspend(xHandler_Output); 
						vTaskSuspend(xHandleCommon);
						vTaskSuspend(xHandleBacnetControl);
						vTaskSuspend(xHandleMornitor_task);
			#if MSTP
						vTaskSuspend(xHandleMSTP);			
			#endif 
						
			#if MINI	
#if (USB_HOST || USB_DEVICE)						
						 vTaskSuspend(xHandleUSB); 
#endif
						 vTaskSuspend(xHandler_SPI);
			#endif	
						
				if(Modbus.mini_type <= MINI_BIG)
					vTaskSuspend(xHandleLCD_task);


					Response_MAIN_To_SUB(main_data_buffer,main_rece_size - 2,Modbus.sub_port);

					flag_resume_scan = 1;
					resume_scan_count = 0;	
	//			vTaskResume(Handle_Scan); 
					vTaskResume(Handle_ParameterOperation);
	//			vTaskResume(Handle_MainSerial);
					vTaskResume(xHandler_Output); 
					vTaskResume(xHandleCommon);
					vTaskResume(xHandleBacnetControl);
					vTaskResume(xHandleMornitor_task);
		#if MSTP
					vTaskResume(xHandleMSTP);			
		#endif 


#if MINI	
				
#if (USB_HOST || USB_DEVICE)		
					vTaskResume(xHandleUSB); 
#endif
					vTaskResume(xHandler_SPI);
#endif	   

				if(Modbus.mini_type <= MINI_BIG)
					vTaskResume(xHandleLCD_task);
						}
					}				
				}  
				else
					packet_error[Modbus.main_port]++;
								
			// Restart the serial receive.
				uart_serial_restart(Modbus.main_port);
				main_dealwithTag = 0;
			}
			else
			{	
				uart_serial_restart(Modbus.main_port);	
				main_dealwithTag = 0;  
			}
		}

		//500 ms check whether master is connecting
		count_master_connect++;
		if(count_master_connect > 10)
		{
			count_master_connect = 0;
			if(Modbus.com_config[0] == MASTER_MODBUS || Modbus.com_config[0] == NOUSE)
			{	 
				if(cSemaphoreTake(sem_subnet_tx_uart0, 0) == pdTRUE)
				{				
					uart_serial_restart(0);	
				}
				cSemaphoreGive(sem_subnet_tx_uart0);
			}			
			

			 // if UART2 is not slave		
			if(Modbus.com_config[2] == MASTER_MODBUS || Modbus.com_config[2] == NOUSE)
			{	 
				U16_T crc_val;	
									
				if(cSemaphoreTake(sem_subnet_tx_uart2, 0) == pdTRUE)
				{					
						uart_serial_restart(2);
												
						if((hsurRxCount == 6) && (hsurRxBuffer[0] == 0xff) && (hsurRxBuffer[1] == 0x19) \
							|| ((hsurRxCount == 8) && (hsurRxBuffer[1] == 0x03) && (hsurRxBuffer[7] != 0))) // receive data
					//	 UART2 recieved a redundant byte 0 when hsurRxCount is 8 and the last byte is 0
						{							
							crc_val = crc16(hsurRxBuffer, hsurRxCount - 2);
												
							if(crc_val == (hsurRxBuffer[hsurRxCount - 2] << 8) + hsurRxBuffer[hsurRxCount - 1])
							{
								Modbus.com_config[2] = SUB_MODBUS;							
								Count_com_config();
							}						
						}
						else
							hsurRxCount = 0;
				}
				cSemaphoreGive(sem_subnet_tx_uart2);
			}			
		}
				
	}

}


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
	responseCmd(SERIAL,main_data_buffer,NULL);
}


/* reponse READ command*/
void responseCmd(U8_T type,U8_T* pData,MODBUSTCP_SERVER_CONN * pHttpConn)  
{	
    U16_T far StartAdd;  
	U16_T far RegNum;
	U8_T far HeadLen; // Modbus head is 0, Tcp head is 6
	U16_T far loop;
	U8_T far sendbuf[200];
	U8_T far cmd;
	U8_T far i;
//	U8_T far temp;
//	U8_T far tempbit;
//	U8_T far temp_number;
//	U8_T far temp_address;
//	U8_T far send_buffer;
//	U8_T far tempbuf[100];  /* convert schedule structure */
	U8_T far update_flash;

	if(type == SERIAL || type == USB)  // modbus packet
	{
		HeadLen = 0;		
	}
	else    // TCP packet
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
	
		for(loop = 0 ;loop < RegNum;loop++)
		{
			if ( StartAdd + loop >= MODBUS_SERIALNUMBER_LOWORD && StartAdd + loop <= MODBUS_SERIALNUMBER_LOWORD + 3 )
			{			
				sendbuf[HeadLen + 3 + loop * 2] = 0;
			//	sendbuf[HeadLen + 3 + loop * 2] = (Test[StartAdd + loop - MODBUS_SERIALNUMBER_LOWORD] >> 8) & 0xFF;;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Modbus.serialNum[StartAdd + loop - MODBUS_SERIALNUMBER_LOWORD];
			//	sendbuf[HeadLen + 3 + loop * 2 + 1] = Test[StartAdd + loop - MODBUS_SERIALNUMBER_LOWORD] & 0xFF;
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
#if CM5
				sendbuf[HeadLen + 3 + loop * 2 + 1] = PRODUCT_CM5;
#endif

				
#if MINI
			if(Modbus.mini_type == MINI_VAV)
				sendbuf[HeadLen + 3 + loop * 2 + 1] = PRODUCT_MINI_VAV;
			else
				sendbuf[HeadLen + 3 + loop * 2 + 1] = PRODUCT_MINI_BIG;
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
			else if(StartAdd + loop == MODBUS_STATION_NUM)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = Station_NUM;
			}
//			else if(StartAdd + loop == MODBUS_SUB_PORT)
//			{
//				sendbuf[HeadLen + 3 + loop * 2] = 0;
//				sendbuf[HeadLen + 3 + loop * 2 + 1] = Modbus.sub_port;
//			}  
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
			else if(StartAdd + loop == MODBUS_USB_MODE)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = Modbus.usb_mode;
			}

			else if(StartAdd + loop == MODBUS_EN_DYNDNS)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = Modbus.en_dyndns;
			}
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
			else if(StartAdd + loop == MODBUS_NETWORK)
			{
				sendbuf[HeadLen + 3 + loop * 2] = Modbus.network_number >> 8;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = Modbus.network_number;
			}
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
		
/* START SUB TSTAT */
//			else if(StartAdd + loop == MODBUS_ADD_ID_BY_HAND)
//			{
//				sendbuf[HeadLen + 3 + loop * 2] = 0;	
//				sendbuf[HeadLen + 3 + loop * 2 + 1] =  add_id_by_hand;				
//			}
			else if(StartAdd + loop == MODBUS_TOTAL_NO)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  sub_no;
			}
			else if(StartAdd + loop >= MODBUS_SUBADDR_FIRST && StartAdd + loop <= MODBUS_SUBADDR_LAST)
			{  // 18 - 25
		
//				if(add_id_by_hand == 1)
//				{
//					sendbuf[HeadLen + 3 + loop * 2] = 0;	
//					sendbuf[HeadLen + 3 + loop * 2 + 1] =  tstat_position[StartAdd + loop  - MODBUS_SUBADDR_FIRST];
//
//				}  
//				else
				{
					sendbuf[HeadLen + 3 + loop * 2] = (current_online[scan_db[i].id / 8] & (1 << (scan_db[i].id % 8)));	
					sendbuf[HeadLen + 3 + loop * 2 + 1] =  scan_db[StartAdd + loop  - MODBUS_SUBADDR_FIRST].id;
				}
			}
			else if(StartAdd + loop >= MODBUS_CHIP1_HW && StartAdd + loop <= MODBUS_CHIP4_SW)
			{
			   	sendbuf[HeadLen + 3 + loop * 2] = chip_info[StartAdd + loop - MODBUS_CHIP1_HW] >> 8;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] = chip_info[StartAdd + loop - MODBUS_CHIP1_HW];
			}
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

			else if(StartAdd + loop >= MODBUS_OUTPUT_1V && StartAdd + loop <= MODBUS_OUTPUT_10V)
			{
				sendbuf[HeadLen + 3 + loop * 2] = Modbus.start_adc[StartAdd + loop - MODBUS_OUTPUT_1V + 1] >> 8;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Modbus.start_adc[StartAdd + loop - MODBUS_OUTPUT_1V + 1];
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
#if MINI
			else if(StartAdd + loop == MODBUS_OUTPUT_MODE)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] = flag_output; 	
			}
#endif 
			
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
			
//			else if(StartAdd + loop == MODBUS_GSM_UDPPORT)
//			{
//				sendbuf[HeadLen + 3 + loop * 2] = GSM_remote_tcpport1 >> 8;	
//				sendbuf[HeadLen + 3 + loop * 2 + 1] = GSM_remote_tcpport1; 	
//			}
//			else if(StartAdd + loop == MODBUS_GSM_TCP_LINKID)
//			{
// 				sendbuf[HeadLen + 3 + loop * 2] = 0;	
//				sendbuf[HeadLen + 3 + loop * 2 + 1] = GSM_remote_tcp_link_id; 
//
//			}
//			else if(StartAdd + loop == MODBUS_GSM_UDP_LINKID)
//			{
// 				sendbuf[HeadLen + 3 + loop * 2] = 0;	
//				sendbuf[HeadLen + 3 + loop * 2 + 1] = GSM_remote_udp_link_id; 
//
//			}
#endif
//  sub infomation block		

/***********************av bv ai bi ai**********************************\*/
#if BAC_COMMON 
		    else if(StartAdd + loop == MODBUS_MAX_AV)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = MAX_AVS;
			}
			else if(StartAdd + loop == MODBUS_MAX_AI)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = MAX_AIS;
			}
			else if(StartAdd + loop == MODBUS_MAX_AO)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = MAX_AOS;
			}
			else if(StartAdd + loop == MODBUS_MAX_BI)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = MAX_BIS;
			}
			else if(StartAdd + loop == MODBUS_MAX_BO)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = MAX_BOS;
			}
//			else if(StartAdd + loop >= MODBUS_AV_FIRST && StartAdd + loop <= MODBUS_AV_LAST)
//			{
//				sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(AV_Present_Value[StartAdd + loop - MODBUS_AV_FIRST] >> 8);
//				sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)AV_Present_Value[StartAdd + loop - MODBUS_AV_FIRST];	
//			}
//			else if(StartAdd + loop >= MODBUS_AI_FIRST && StartAdd + loop <= MODBUS_AI_LAST)
//			{
//				sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(AI_Present_Value[StartAdd + loop - MODBUS_AI_FIRST] >> 8);
//				sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)AI_Present_Value[StartAdd + loop - MODBUS_AI_FIRST];	
//			}
//			else if(StartAdd + loop >= MODBUS_AO_FIRST && StartAdd + loop <= MODBUS_AO_LAST)
//			{
//				sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(AO_Present_Value[StartAdd + loop - MODBUS_AO_FIRST] >> 8);
//				sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)AO_Present_Value[StartAdd + loop - MODBUS_AO_FIRST];	
//			}
//			else if(StartAdd + loop >= MODBUS_BI_FIRST && StartAdd + loop <= MODBUS_BI_LAST)
//			{
//				sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(BI_Present_Value[StartAdd + loop - MODBUS_BI_FIRST] >> 8);
//				sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)BI_Present_Value[StartAdd + loop - MODBUS_BI_FIRST];	
//			}
//			else if(StartAdd + loop >= MODBUS_BO_FIRST && StartAdd + loop <= MODBUS_BO_LAST)
//			{
//				sendbuf[HeadLen + 3 + loop * 2] = 0;//(U8_T)(BO_Present_Value[StartAdd + loop - MODBUS_BO_FIRST] >> 8);
//				sendbuf[HeadLen + 3 + loop * 2 + 1] = 0;//(U8_T)BO_Present_Value[StartAdd + loop - MODBUS_BO_FIRST];	
//			}
#endif
/*******************************************************************\*/




			
/*  END SUB TSTAT */
/******************************end for tstst resigter ************************************************/
		   //  input block
		   else if(StartAdd + loop >= MODBUS_OUTPUT_FIRST && StartAdd + loop <= MODBUS_OUTPUT_LAST)
		   {
		   		U8_T index;
				index = (StartAdd + loop - MODBUS_OUTPUT_FIRST);
				if(outputs[index].digital_analog == 0) // digtial
				{
//				sendbuf[HeadLen + 3 + loop * 2] = (U8_T)((swap_double(outputs[index].value)) >> 8);
//				sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)((swap_double(outputs[index].value)));
					if(outputs[index].range >= ON_OFF  && outputs[index].range <= HIGH_LOW)
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
				sendbuf[HeadLen + 3 + loop * 2] = (U8_T)((swap_double(outputs[index].value) / 1000) >> 8);
				sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)((swap_double(outputs[index].value) / 1000));
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
		   else if(StartAdd + loop >= MODBUS_INPUT_FIRST && StartAdd + loop <= MODBUS_INPUT_LAST)
		   {
		   		U8_T index;
				index = (StartAdd + loop - MODBUS_INPUT_FIRST);
				if(inputs[index].digital_analog == 0)  // digital
				{
					if(inputs[index].range >= ON_OFF  && inputs[index].range <= HIGH_LOW)
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
					sendbuf[HeadLen + 3 + loop * 2] = (U8_T)((swap_double(inputs[index].value) / 100) >> 8);
					sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)((swap_double(inputs[index].value) / 100));
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
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = inputs[index].range; 	
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
			 else if(StartAdd + loop >= MODBUS_VAR_FIRST && StartAdd + loop <= MODBUS_VAR_LAST)
			{
				U8_T index;
				index = (StartAdd + loop - MODBUS_VAR_FIRST);
				sendbuf[HeadLen + 3 + loop * 2] = (U8_T)((swap_double(vars[index].value) / 100) >> 8);
				sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)((swap_double(vars[index].value) / 100));				
			} 
			else if(StartAdd + loop >= MODBUS_VAR_AM_FIRST && StartAdd + loop <= MODBUS_VAR_AM_LAST)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = vars[StartAdd + loop - MODBUS_VAR_AM_FIRST].auto_manual;
			}			 
			else if(StartAdd + loop >= MODBUS_TIMER_ADDRESS && StartAdd + loop < MODBUS_TIMER_ADDRESS + 8)
			{	 
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  RTC.all[StartAdd + loop - MODBUS_TIMER_ADDRESS];
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

		if(type == SERIAL || type == USB)
		{
			U16_T crc_val;

			crc_val = crc16(sendbuf,RegNum * 2 + 3);
			sendbuf[RegNum * 2 + 3]	= crc_val >> 8;
			sendbuf[RegNum * 2 + 4]	= (U8_T)crc_val;

			if(type == SERIAL)
			{
		   	uart_init_send_com(Modbus.main_port);
				uart_send_string(sendbuf, RegNum * 2 + 5,Modbus.main_port);

			}
#if USB_DEVICE
			else
			{
				memcpy(UpBuf,sendbuf,RegNum * 2 + 5); 
				UpIndex = 0;
				UpCtr = RegNum * 2 + 5;
				ENDP2_NEED_UP_FLAG = 1;	
			}
#endif
		}		
		else // TCP
		{
			 
			TransID =  ((U16_T)pData[0] << 8) | pData[1];
			sendbuf[0] = TransID >> 8;			//	TransID
			sendbuf[1] = (U8_T)TransID;	
			sendbuf[2] = 0;			//	ProtoID
			sendbuf[3] = 0;
			sendbuf[4] = (3 + RegNum * 2) >> 8;	//	Len
			sendbuf[5] = (U8_T)(3 + RegNum * 2) ;  			

			if(type == TCP)
			{
				if(cSemaphoreTake( xSemaphore_tcp_send, ( portTickType ) 10 ) == pdTRUE)
				{				
					TCPIP_TcpSend(pHttpConn->TcpSocket, sendbuf, RegNum * 2 + HeadLen + 3, TCPIP_SEND_NOT_FINAL); 
					cSemaphoreGive( xSemaphore_tcp_send );
				}
			}
//			else if(type == GSM)
//			{
//				uart_init_send_com(1);
//				uart_send_string(sendbuf, RegNum * 2 + HeadLen + 3,1);
//
//			}

		}
	}
	else if(cmd == WRITE_VARIABLES)
	{		
		if(type == SERIAL || type == USB)
		{ 	
		
			if(type == SERIAL)
			{
				uart_init_send_com(Modbus.main_port);
				uart_send_string(pData, 8,Modbus.main_port);
			}	
#if USB_DEVICE
			else
			{
				memcpy(UpBuf,pData,8); 
				UpIndex = 0;
				ENDP2_NEED_UP_FLAG = 1;	
				UpCtr = 8;	
			}
#endif
		}
		else // TCP   dont have CRC 
		{
		//	SetTransactionId(6 + UIP_HEAD);
			sendbuf[0] = pData[0];//0;			//	TransID
			sendbuf[1] = pData[1];//TransID++;	
			sendbuf[2] = 0;			//	ProtoID
			sendbuf[3] = 0;
			sendbuf[4] = 0;	//	Len
			sendbuf[5] = 6;


			for (loop = 0;loop < 6;loop++)
			{
				sendbuf[HeadLen + loop] = pData[HeadLen + loop];	
			}
			if(cSemaphoreTake( xSemaphore_tcp_send, ( portTickType ) 10 ) == pdTRUE)
			{				
				TCPIP_TcpSend(pHttpConn->TcpSocket, sendbuf, 6 + UIP_HEAD, TCPIP_SEND_NOT_FINAL); 
				cSemaphoreGive( xSemaphore_tcp_send );
			}
		}


		/* dealwith write command */
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
		else if(StartAdd == MODBUS_SNTP_TIMEZONE )
		{
			timezone = pData[HeadLen + 5]+ (pData[HeadLen + 4]<<8);
			E2prom_Write_Byte(EEP_TIME_ZONE_HI, pData[HeadLen + 4] );
			E2prom_Write_Byte(EEP_TIME_ZONE_LO, pData[HeadLen + 5] );
		}
		else if(StartAdd == MODBUS_SNTP_EN )
		{			
			if(pData[HeadLen + 5] <= 6)
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

		else if(StartAdd == MODBUS_UPDATE_STATUS)
		{
		 	update_flash = pData[HeadLen + 5];
			if(update_flash == 0x9F)
			{
				//...
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
		#if MINI
		
		else if(StartAdd == MODBUS_MINI_TYPE)
		{
			Modbus.mini_type = pData[HeadLen + 5]+ (pData[HeadLen + 4]<<8);
			E2prom_Write_Byte(EEP_MINI_TYPE, Modbus.mini_type);
   	//		SPI_Setup(SPI_SSO_ENB|SPI_MST_SEL|SPI_SS_AUTO|SPI_ENB, SPI_STCFIE, 0x20/*0x80*/, SLAVE_SEL_1); // 25M

			Start_Comm_Top();
		}
		#endif
		 
//		else if(StartAdd == MODBUS_CLEAR_SCAN_DB)
//		{
//			reset_scan_db_flag = 1;
//		} 
		else if(StartAdd == MODBUS_STATION_NUM)
		{
			if(Station_NUM <= 32)
			{				
				Station_NUM = pData[HeadLen + 5];
				change_panel_number_in_code(Panel_Info.reg.panel_number,Station_NUM);
				E2prom_Write_Byte(EEP_STATION_NUM,Station_NUM);
				Panel_Info.reg.panel_number	= Station_NUM;
			}
		} 
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
//#if USB_HOST
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
//#endif
		

		else if(StartAdd == MODBUS_EN_DYNDNS)
		{
			Modbus.en_dyndns = pData[HeadLen + 5];	
			if(Modbus.en_dyndns == 2)
			{
				flag_Update_Dyndns = 0;
				Update_Dyndns_Retry = 0;
				Recount_Check_serverip = 0;
			}
			E2prom_Write_Byte(EEP_EN_DYNDNS,Modbus.en_dyndns);
		}
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
		else if(StartAdd == MODBUS_ENABLE_WRITE_MAC)
		{
			Mac_Address_write_enable = 1;
		}
		else if(StartAdd >= MODBUS_COM0_TYPE && StartAdd <= MODBUS_COM2_TYPE)
		{
			i = StartAdd - MODBUS_COM0_TYPE;

			if(Modbus.com_config[i] != pData[HeadLen + 5])
			{
				if(((i == 0) && (Modbus.com_config[i] == 0 || Modbus.com_config[i] == 2 || Modbus.com_config[i] == 7)) ||
				((i == 1) && (Modbus.com_config[i] == 0 || Modbus.com_config[i] == 2 || Modbus.com_config[i] == 7)) ||
				((i == 2) && (Modbus.com_config[i] == 0 || Modbus.com_config[i] == 2 || Modbus.com_config[i] == 7 || Modbus.com_config[i] == 1))
					)
			{
				Modbus.com_config[i] = pData[HeadLen + 5];
				if(Modbus.com_config[i] == SUB_MODBUS)
					uart_serial_restart(i);
				if(Modbus.com_config[i] == MAIN_MSTP)
					Recievebuf_Initialize(i);
				if(Modbus.com_config[i] == MASTER_MODBUS)
				{
					Count_com_config();
					if(i == 1)  // zigbee port
						count_send_id_to_zigbee = 0;
				}
				E2prom_Write_Byte(EEP_COM0_CONFIG + i, Modbus.com_config[i]);
			}
		}
			
		}

		else if(StartAdd == MODBUS_REFRESH_FLASH)
		{
			Modbus.refresh_flash_timer = pData[HeadLen + 5];
			E2prom_Write_Byte(EEP_REFRESH_FLASH, Modbus.refresh_flash_timer );
		}
#if MINI
		else if(StartAdd >= MODBUS_OUTPUT_1V && StartAdd <= MODBUS_OUTPUT_10V)
		{
			Modbus.start_adc[StartAdd - MODBUS_OUTPUT_1V + 1] = pData[HeadLen + 5]+ (pData[HeadLen + 4]<<8);			
			E2prom_Write_Byte(EEP_OUT_1V + StartAdd - MODBUS_OUTPUT_1V, Modbus.start_adc[StartAdd - MODBUS_OUTPUT_1V + 1] /10);
			// caclulate slop
			cal_slop();

		}
		else if(StartAdd == MODBUS_OUTPUT_TEST_VALUE)
		{
			test_adc = pData[HeadLen + 5]+ (pData[HeadLen + 4]<<8);
		}
		else if(StartAdd == MODBUS_OUTPUT_TEST_FLAG)
		{
			test_adc_flag = pData[HeadLen + 5];
		}

		else if(StartAdd == MODBUS_OUTPUT_MODE)
		{
			flag_output = pData[HeadLen + 5];
			E2prom_Write_Byte(EEP_OUTPUT_MODE,pData[HeadLen + 5]);
		}
#endif
		else if(StartAdd == MODBUS_NETWORK)
		{
			Modbus.network_number = pData[HeadLen + 5]+ (pData[HeadLen + 4]<<8);
			E2prom_Write_Byte(EEP_NETWORK,pData[HeadLen + 5]);
			//E2prom_Write_Byte(EEP_PORT_HIGH,pData[HeadLen + 4]);
		}
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
		else if(StartAdd == MODBUS_TCP_TYPE)
		{
			Modbus.tcp_type = pData[HeadLen + 5]+ (pData[HeadLen + 4]<<8);
			E2prom_Write_Byte(EEP_TCP_TYPE,  Modbus.tcp_type );
		}
		else if(StartAdd >= MODBUS_IP_1 && StartAdd <= MODBUS_IP_4)
		{
			 Modbus.ip_addr[StartAdd - MODBUS_IP_1] = pData[HeadLen + 5];
			 E2prom_Write_Byte(EEP_IP + 3 - (StartAdd - MODBUS_IP_1), Modbus.ip_addr[StartAdd - MODBUS_IP_1]);
//			 IntFlashWriteByte(0x4001,0);  ?????????????????????? CM5 have trouble
//			 AX11000_SoftReboot();
		}
		else if(StartAdd >= MODBUS_SUBNET_1 && StartAdd <= MODBUS_SUBNET_4)
		{
			 Modbus.subnet[StartAdd - MODBUS_SUBNET_1] = pData[HeadLen + 5];
			 E2prom_Write_Byte(EEP_SUBNET + 3 - ( StartAdd - MODBUS_SUBNET_1), Modbus.subnet[StartAdd - MODBUS_SUBNET_1]);
		}
		else if(StartAdd >= MODBUS_GETWAY_1 && StartAdd <= MODBUS_GETWAY_4)
		{
			 Modbus.getway[StartAdd - MODBUS_GETWAY_1] = pData[HeadLen + 5];
			 E2prom_Write_Byte(EEP_GETWAY + 3 - (StartAdd - MODBUS_GETWAY_1), Modbus.getway[StartAdd - MODBUS_GETWAY_1]);
		}
		else if(StartAdd == MODBUS_TCP_LISTEN_PORT)
		{
			Modbus.tcp_port =  pData[HeadLen + 5]+ (pData[HeadLen + 4]<<8);
			E2prom_Write_Byte(EEP_PORT_LOW,pData[HeadLen + 5]);
			E2prom_Write_Byte(EEP_PORT_HIGH,pData[HeadLen + 4]);
//			IntFlashWriteByte(0x4001,0);
//			AX11000_SoftReboot();
		}
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
//		else if(StartAdd == MODBUS_GSM_UDP_LINKID)
//		{
//			GSM_remote_udp_link_id =  pData[HeadLen + 5];
//			E2prom_Write_Byte(EEP_GSM_UDP_LINKID,pData[HeadLen + 5]);
//			IntFlashWriteByte(0x4001,0);
////			AX11000_SoftReboot();
//		}
#endif
		else if(StartAdd == MODBUS_TEST_CMD )
		{
//			if(pData[HeadLen + 5] == 77)
//			{
//				IntFlashWriteByte(0x4001,0);
//				AX11000_SoftReboot();
//			}

#if 0//USB_HOST
			if(pData[HeadLen + 5] == 80)
			{ 
				GSM_ipinit(0);
			}
			else if(pData[HeadLen + 5] == 81)
			{ 
				GSM_ipopen(0,0,0,0,0);
			}
			else if(pData[HeadLen + 5] == 82)
			{ 
				GSM_ipopen(1,0,0,0,0);
			}
			else if(pData[HeadLen + 5] == 83)
			{ 
				GSM_IPSEND(0); 
			} 
			else if(pData[HeadLen + 5] == 84)
			{ 
				GSM_IPSEND(1);

			}
			else if(pData[HeadLen + 5] == 85)
			{ 
				GSM_SET_TRANSFER(0);
			} 
			else if(pData[HeadLen + 5] == 86)
			{ 
				GSM_SET_TRANSFER(1); 
			} 
//			else if(pData[HeadLen + 5] == 87)
//			{ 
//				GSM_END_TRANSER();
//			}
//			else if(pData[HeadLen + 5] == 90)
//			{ 
//				GSM_Power_On(); 
//			} 
#endif

#if USB_HOST
			if(pData[HeadLen + 5] == 91)
			{ 
//				vTaskSuspend(xHandleUSB); 
				GSM_Power_Off();
			//	P1_1 = 0;
			} 
			if(pData[HeadLen + 5] == 90)
			{ 
			//	P1_1 = 1;
			//	GSM_Power_On();
					USB_HOST_initial();
//				vTaskResume(xHandleUSB);
			}
			if(pData[HeadLen + 5] == 89)
			{
				Reset_CH375();
			}
#endif 


			if(pData[HeadLen + 5] == 92)
			{ 
				//SNTPC_Start(800, 0x1838b28c);	 	//   24.56.178.101
				// read e2
				E2prom_Read_Byte(EEP_ADDRESS,  &Modbus.address);
			}
			if(pData[HeadLen + 5] == 93)
			{ 
//				SNTPC_Start(800, 0xc0a8007f);
				flag_Updata_Clock = 1;
				//E2prom_Write_Byte(EEP_ADDRESS,  100);
			}
			if(pData[HeadLen + 5] == 94)
			{ 
				//SNTPC_Start(800, 0xd248912d);
				//WhoIs_Start(0xc0a80071);
				//Send_Private_Flag = 1;
//				UART_Init(2);
//				dlmstp_init(NULL);
//				Modbus.protocal = BAC_MSTP;
//				Modbus.protocal = 0;
				
			}
			if(pData[HeadLen + 5] == 97)
			{ 
//				Recievebuf_Initialize(2);
//				Modbus.protocal = BAC_MSTP;
//				Send_WhoIs(-1,-1);
//				Modbus.protocal = 0;
//					Send_Whois_Flag = 1;
				//SNTPC_Start(800, 0x836b0d64);
//				dlenv_bbmd_address_set(0xc0a80044);
//				dlenv_bbmd_port_set(0xbac0);
//				dlenv_bbmd_ttl_set(20000);
//				dlenv_init();
//				WhoIs_Start(0xffffffff);
			}
	
			if(pData[HeadLen + 5] == 95)
			{//int GetPrivateData(uint32_t deviceid,uint8_t command,uint8_t start_instance,uint8_t end_instance,int16_t entitysize)
//				GetPrivateData(1457,50,0,10,100);
				//WhoIs_Start(0xffffffff);
				//TimeSync();
//				flag_update_tstat = 1;
//				Send_cmd_other_panel();
//				dlmstp_init(NULL);
#if MINI	
			 //vTaskSuspend(xHandler_SPI);
#endif
			}
			if(pData[HeadLen + 5] == 96)
			{
//				if(Modbus.en_sntp >= 2)	
//				{
//					flag_Update_Sntp = 0; // for test now
//				}
				vTaskResume(xHandleMornitor_task);
			}			
			if(pData[HeadLen + 5] == 111)
			{	
				IntFlashWriteByte(0x4001,0);
				AX11000_SoftReboot();
			}
//			if(pData[HeadLen + 5] == 112)
//			{	
//				SET_VAV(50);
//			}
			if(pData[HeadLen + 5] == 113)
			{	
				//uart0_rece_count = 0;
			}
			if(pData[HeadLen + 5] == 151)
			{
				TCP_IP_Init();				
			}	
//			if(pData[HeadLen + 5] == 150)
//			{
//				
//#if CM5
//				TCP_IP_Init();		
//#endif				
//				//				IntFlashErase(ERA_RUN,0x70000);	
////				IntFlashWriteByte(0x4001,0);					
////				AX11000_SoftReboot(); 			
//			}					
		}
//		else if(StartAdd == MODBUS_PIC_WATCHDOG )
//		{
//			#if CM5
//		   	if(write_pic_watchdog(pData[HeadLen + 5]))							
//				pic_watchdog = pData[HeadLen + 5];
//			else
//				pic_watchdog = 0;
//			#endif
//		}
/****************av ai ao bi bo **********************************\*/

//		else if(StartAdd == MODBUS_RESERVED1)
//		{
//			//Test[45] = (pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8));
//			vars[0].value = swap_double((U32_T)(pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8)) * 1000);
//		}
//		else if(StartAdd == MODBUS_RESERVED2)
//		{
//			//Test[46] = (pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8));
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
			SD_block_num[2 * (StartAdd - MODBUS_SD_BLOCK_A1)] = pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8);
			E2prom_Write_Byte(EEP_SD_BLOCK_A1 + (StartAdd - MODBUS_SD_BLOCK_A1) * 2,pData[HeadLen + 5]);
			E2prom_Write_Byte(EEP_SD_BLOCK_A1 + (StartAdd - MODBUS_SD_BLOCK_A1) * 2 + 1,pData[HeadLen + 4]);
		}
		else if( StartAdd >= MODBUS_SD_BLOCK_D1 && StartAdd <= MODBUS_SD_BLOCK_D12 )
		{
			SD_block_num[2 * (StartAdd - MODBUS_SD_BLOCK_D1) + 1] = pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8);
			E2prom_Write_Byte(EEP_SD_BLOCK_D1 + (StartAdd - MODBUS_SD_BLOCK_D1) * 2,pData[HeadLen + 5]);
			E2prom_Write_Byte(EEP_SD_BLOCK_D1 + (StartAdd - MODBUS_SD_BLOCK_D1) * 2 + 1,pData[HeadLen + 4]);
		}
		else if(StartAdd >= MODBUS_TIMER_ADDRESS && StartAdd <= MODBUS_TIMER_ADDRESS + 7)
		{
			 RTC.all[StartAdd - MODBUS_TIMER_ADDRESS] = pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4]<<8);
		//	E2prom_Write_Byte(EEP_SEC + StartAdd - MODBUS_SEC, Time.all[StartAdd - MODBUS_SEC]);
			Set_Clock(PCF_SEC + StartAdd - MODBUS_TIMER_ADDRESS, RTC.all[StartAdd - MODBUS_TIMER_ADDRESS]);
		}		
//		else if(StartAdd == MODBUS_POINT_SEQ)
//		{
//			Modbus.point_sequence = pData[HeadLen + 5];
//			E2prom_Write_Byte(EEP_POINT_SEQ,pData[HeadLen + 5]);
//		}
		else if(StartAdd  >= MODBUS_OUTPUT_FIRST && StartAdd  <= MODBUS_OUTPUT_LAST)
		{
		 	i = (StartAdd - MODBUS_OUTPUT_FIRST);
			if(outputs[i].digital_analog == 0)  // digital
			{
				if( outputs[i].range >= OFF_ON && outputs[i].range <= LOW_HIGH )
				{
					if(pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8) == 1)
						outputs[i].control = 1;
					else 
						outputs[i].control = 0;
				}
				if( outputs[i].range >= ON_OFF && outputs[i].range <= HIGH_LOW )
				{
					if(pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8) == 1)
						outputs[i].control = 0;
					else 
						outputs[i].control = 1;
				}
				
				outputs[i].value = swap_double(pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8));

			}			
			else if(outputs[i].digital_analog == 1)
			{
				 outputs[i].value = swap_double(1000l * (pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8)));
			} 
		}
		else if(StartAdd  >= MODBUS_OUTPUT_RANGE_FIRST && StartAdd  <= MODBUS_OUTPUT_RANGE_LAST)
		{
		 	i = (StartAdd - MODBUS_OUTPUT_RANGE_FIRST);
			outputs[i].range = pData[HeadLen + 5];
		}
		else if(StartAdd  >= MODBUS_OUTPUT_AM_FIRST && StartAdd  <= MODBUS_OUTPUT_AM_LAST)
		{
		 	i = (StartAdd - MODBUS_OUTPUT_AM_FIRST);
			if(outputs[i].switch_status == SW_AUTO)
				outputs[i].auto_manual = pData[HeadLen + 5];
		}
		else if(StartAdd  >= MODBUS_OUTPUT_AD_FIRST && StartAdd  <= MODBUS_OUTPUT_AD_LAST)
		{
		 	i = (StartAdd - MODBUS_OUTPUT_AD_FIRST);
			outputs[i].digital_analog = pData[HeadLen + 5];
		}
//		else if(StartAdd  >= MODBUS_INPUT_FIRST && StartAdd  <= MODBUS_INPUT_LAST)
//		{
//		 	i = (StartAdd - MODBUS_INPUT_FIRST);
//			inputs[i].value = swap_double((pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8) * 10));
//		}
		else if(StartAdd  >= MODBUS_INPUT_FILTER_FIRST && StartAdd  <= MODBUS_INPUT_FILTER_LAST)
		{	
		 	i = (StartAdd - MODBUS_INPUT_FILTER_FIRST);
			inputs[i].filter = pData[HeadLen + 5];
		}
		else if(StartAdd  >= MODBUS_INPUT_CAL_FIRST && StartAdd  <= MODBUS_INPUT_CAL_LAST)
		{
		 	i = (StartAdd - MODBUS_INPUT_CAL_FIRST);
			inputs[i].calibration_hi = pData[HeadLen + 4];
			inputs[i].calibration_lo = pData[HeadLen + 5];
		}
		else if(StartAdd  >= MODBUS_INPUT_CAL_SIGN_FIRST && StartAdd  <= MODBUS_INPUT_CAL_SIGN_LAST)
		{
		 	i = (StartAdd - MODBUS_INPUT_CAL_SIGN_FIRST);
			inputs[i].calibration_sign = pData[HeadLen + 5];
		}
		else if(StartAdd  >= MODBUS_INPUT_RANGE_FIRST && StartAdd  <= MODBUS_INPUT_RANGE_LAST)
		{
		 	i = (StartAdd - MODBUS_INPUT_RANGE_FIRST);
			inputs[i].range = pData[HeadLen + 5];
		}
		else if(StartAdd  >= MODBUS_INPUT_HI_SPD_EN_FIRST && StartAdd  <= MODBUS_INPUT_HI_SPD_EN_LAST)
		{
		 	i = (StartAdd - MODBUS_INPUT_HI_SPD_EN_FIRST);
			high_spd_en[i] = pData[HeadLen + 5];  
		} 
		else if(StartAdd  >= MODBUS_INPUT_TYPE_FIRST && StartAdd  <= MODBUS_INPUT_TYPE_LAST)
		{
		 	i = (StartAdd - MODBUS_INPUT_TYPE_FIRST);
			input_type[i] = pData[HeadLen + 5];  
		}
		else if(StartAdd  >= MODBUS_VAR_FIRST && StartAdd  <= MODBUS_VAR_LAST)
		{			
			S16_T tempval; // only for test
		 	i = (StartAdd - MODBUS_VAR_FIRST);
			
			tempval = (U32_T)(pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4] << 8));

			if((tempval >= -500) && (tempval <= 1000))
			{
				vars[i].value = swap_double((U32_T)tempval * 100); 
			}
			 
		} 
		else if(StartAdd  >= MODBUS_VAR_AM_FIRST && StartAdd  <= MODBUS_VAR_AM_LAST)
		{
		 	i = (StartAdd - MODBUS_VAR_AM_FIRST);
			vars[i].auto_manual = pData[HeadLen + 5];  
		} 
		
  // SUB INFO BLOCK

	}
	else if(cmd == MULTIPLE_WRITE)
	{
		if(type == SERIAL || type == USB)   // TBD: need change
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
#if USB_DEVICE
			else
			{
				 memcpy(UpBuf,sendbuf,8); 
				 UpIndex = 0;
				 ENDP2_NEED_UP_FLAG = 1;
				 UpCtr = 8;			
			}
#endif
		}
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
			if(cSemaphoreTake( xSemaphore_tcp_send, ( portTickType ) 10 ) == pdTRUE)
			{				
				TCPIP_TcpSend(pHttpConn->TcpSocket, sendbuf, 6 + UIP_HEAD, TCPIP_SEND_NOT_FINAL);  
				cSemaphoreGive( xSemaphore_tcp_send );
			}
		}

		// the following are schedule registers 
		//ChangeFlash = 1;

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
				E2prom_Write_Byte(EEP_MAC, Modbus.mac_addr[5]);
				E2prom_Write_Byte(EEP_MAC + 1, Modbus.mac_addr[4]);
				E2prom_Write_Byte(EEP_MAC + 2, Modbus.mac_addr[3]);
				E2prom_Write_Byte(EEP_MAC + 3, Modbus.mac_addr[2]);
				E2prom_Write_Byte(EEP_MAC + 4, Modbus.mac_addr[1]);
				E2prom_Write_Byte(EEP_MAC + 5, Modbus.mac_addr[0]);

				Mac_Address_write_enable = 0;
			}
		}
		else if(StartAdd == MODBUS_CUSTOMER_DEVICE)
		{
			
			if(pData[HeadLen + 6] == 32)
			{	 
				// check whether id is in database
				//if(get_port_by_id(pData[HeadLen + 8]) == 0) // if id is not on DB
				if(pData[HeadLen + 10] >= 200)  // it is customer device
				{		
					U32_T sn;
					sn = pData[HeadLen + 14] + (U16_T)(pData[HeadLen + 16] << 8) + (U32_T)(pData[HeadLen + 18] << 16) + (U32_T)(pData[HeadLen + 20] << 24);
					if(check_id_in_database(pData[HeadLen + 8], sn,pData[HeadLen + 12]) == 1)
					{  // add new one
						scan_db[db_ctr - 1].product_model = pData[HeadLen + 10];						
						memcpy(tstat_name[db_ctr - 1],&pData[HeadLen + 21],16);
					}
					else
					{ // repalce old one
						U8_T index;
						if(get_index_by_id(pData[HeadLen + 8],&index) == 1)
						{
							scan_db[index].product_model = pData[HeadLen + 10];						
							memcpy(tstat_name[index],&pData[HeadLen + 21],16);
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
					Set_Clock(PCF_SEC,pData[HeadLen + 14]);  // sec
					Set_Clock(PCF_MIN,pData[HeadLen + 13]);  // min
					Set_Clock(PCF_HOUR,pData[HeadLen + 12]); // hour
					Set_Clock(PCF_DAY,pData[HeadLen + 11]);  // day 
					Set_Clock(PCF_WEEK,pData[HeadLen + 10]); // week
					if(pData[HeadLen + 7] |= 0x80)
						pData[HeadLen + 9]|=0x80;
					else									   
						pData[HeadLen + 9]&=0x7f;	
					Set_Clock(PCF_MON,pData[HeadLen + 9]);   // month
					Set_Clock(PCF_YEAR,pData[HeadLen + 8]);	// year
				}
				else
				{
					Set_Clock(PCF_SEC,pData[HeadLen + 22]);  // sec
					Set_Clock(PCF_MIN,pData[HeadLen + 20]);  // min
					Set_Clock(PCF_HOUR,pData[HeadLen + 18]); // hour
					Set_Clock(PCF_DAY,pData[HeadLen + 16]);  // day 
					Set_Clock(PCF_WEEK,pData[HeadLen + 14]); // week
					if(pData[HeadLen + 8] |= 0x80)
						pData[HeadLen + 12]|=0x80;
					else
						pData[HeadLen + 12]&=0x7f;	
					Set_Clock(PCF_MON,pData[HeadLen + 12]);   		// month
					Set_Clock(PCF_YEAR,pData[HeadLen + 10]);		// year
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
//#if MINI
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
//#if MINI			
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
//#if MINI			
//	//		vTaskResume(xHandleUSB); 
//			vTaskResume(xHandler_SPI);
//#endif			

			}
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
		}
		else
		{
			U16_T far crc_val;
			U8_T tempbuf[6];
			U8_T i;
			TcpSocket_ME = pHttpConn->TcpSocket;

		  transaction_id = ((U16_T)pData[0] << 8) | pData[1];
			transaction_num	= 2 * pData[UIP_HEAD + 5] + 3;

 			for(i = 0;i < 4;i++) 
			{ 
				tempbuf[i] = pData[UIP_HEAD+i];									   
			}
			crc_val = crc16(tempbuf,4);		
			tempbuf[4] = crc_val >> 8;
			tempbuf[5] = (U8_T)crc_val;

		}
	}
}
	
void dealwith_write_setting(Str_Setting_Info * ptr)
{

// compare sn to check whether it is current panel	
	Test[22]++;
	if(ptr->reg.sn == Modbus.serialNum[3] + (U16_T)(Modbus.serialNum[2] << 8)	+ ((U32_T)Modbus.serialNum[1] << 16) + ((U32_T)Modbus.serialNum[0] << 24))
	{		
		Test[20] = 100;
		Test[21]++;
		memcpy(panelname,ptr->reg.panel_name,20);
		panelname[19] = 0;
		Set_Object_Name(panelname);
		if(Modbus.en_sntp != ptr->reg.en_sntp)
		{ 						
			Modbus.en_sntp = ptr->reg.en_sntp;
			
			if(Modbus.en_sntp <= 6)
			{
				E2prom_Write_Byte(EEP_EN_SNTP,Modbus.en_sntp);
				if(Modbus.en_sntp >= 2)
				{
					sntp_select_time_server(Modbus.en_sntp);
					flag_Update_Sntp = 0;
					Update_Sntp_Retry = 0;
				}
			}
		}		
		if(timezone != swap_word(ptr->reg.time_zone))
		{ 	
			timezone = swap_word(ptr->reg.time_zone);
			E2prom_Write_Byte(EEP_TIME_ZONE_HI,(U8_T)(timezone >> 8));
			E2prom_Write_Byte(EEP_TIME_ZONE_LO,(U8_T)timezone);
		}					
		if(Modbus.en_dyndns != ptr->reg.en_dyndns)
		{ 						
			Modbus.en_dyndns = ptr->reg.en_dyndns;
			E2prom_Write_Byte(EEP_EN_DYNDNS,Modbus.en_dyndns);

		}
		
		if(Modbus.address != ptr->reg.modbus_id)
		{ 
			if(ptr->reg.modbus_id > 0 && ptr->reg.modbus_id < 255)
			{
			Modbus.address = ptr->reg.modbus_id;
			E2prom_Write_Byte(EEP_ADDRESS,Modbus.address);
			}
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

		memcpy(Modbus.mac_addr,ptr->reg.mac_addr,6);

		if(Modbus.com_config[0] != ptr->reg.com_config[0])
		{
			Modbus.com_config[0] = ptr->reg.com_config[0];
			if(Modbus.com_config[0] == SUB_MODBUS)
				uart_serial_restart(0);
//							if(Modbus.com_config[0] == MAIN_MSTP)
//								Recievebuf_Initialize(0);
			//if(Modbus.com_config[0] == MASTER_MODBUS)
				Count_com_config();
			E2prom_Write_Byte(EEP_COM0_CONFIG, Modbus.com_config[0]);
		}
		if(Modbus.com_config[1] != ptr->reg.com_config[1])
		{
			Modbus.com_config[1] = ptr->reg.com_config[1];
			if(Modbus.com_config[1] == SUB_MODBUS)
			uart_serial_restart(1);	
			if(Modbus.com_config[1] == MASTER_MODBUS)
			{
				Count_com_config();
				count_send_id_to_zigbee = 0;	
			}
			Count_com_config();
			
			E2prom_Write_Byte(EEP_COM1_CONFIG, Modbus.com_config[1]);
		}
		if(Modbus.com_config[2] != ptr->reg.com_config[2])
		{
			Modbus.com_config[2] = ptr->reg.com_config[2];
			if(Modbus.com_config[2] == SUB_MODBUS)
				uart_serial_restart(2);	
			if(Modbus.com_config[2] == MAIN_MSTP)
				Recievebuf_Initialize(2);
			//if(Modbus.com_config[2] == MASTER_MODBUS)
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
		if((Station_NUM != ptr->reg.panel_number) && (ptr->reg.panel_number != 0))
		{
			Station_NUM = ptr->reg.panel_number;
			change_panel_number_in_code(Panel_Info.reg.panel_number,Station_NUM);
			Panel_Info.reg.panel_number	= Station_NUM;
			E2prom_Write_Byte(EEP_STATION_NUM,Station_NUM);
			
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
		
		if(Modbus.refresh_flash_timer != ptr->reg.refresh_flash_timer)
		{ 
			Modbus.refresh_flash_timer = ptr->reg.refresh_flash_timer;
			E2prom_Write_Byte(EEP_REFRESH_FLASH, Modbus.refresh_flash_timer );
		}
		if(Modbus.external_nodes_plug_and_play != ptr->reg.en_plug_n_play)
		{
			Modbus.external_nodes_plug_and_play = ptr->reg.en_plug_n_play;
			E2prom_Write_Byte(EEP_EN_NODE_PLUG_N_PLAY, Modbus.external_nodes_plug_and_play);
		}
 
		if(uart0_baudrate != ptr->reg.com_baudrate[0]) // com_baudrate[2]??T3000
		{
			uart0_baudrate = ptr->reg.com_baudrate[0];
			if((Modbus.com_config[0] == SUB_MODBUS) || (Modbus.com_config[0] == 0) || (Modbus.com_config[0] == MASTER_MODBUS))
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
			if((Modbus.com_config[2] == SUB_MODBUS) || (Modbus.com_config[2] == MAIN_MSTP) || (Modbus.com_config[2] == 0) || (Modbus.com_config[2] == MASTER_MODBUS))
				E2prom_Write_Byte(EEP_UART2_BAUDRATE, uart2_baudrate);		
		
			UART_Init(2);
		}
								 
		if(ptr->reg.reset_default == 88)	// reset default 
		{
			flag_reset_default = 1;
		}
		if(ptr->reg.reset_default == 150)	 // clear db
		{
			clear_scan_db(); 
		}
		
		if(memcmp(Modbus.ip_addr,ptr->reg.ip_addr,4)
			|| memcmp(Modbus.subnet,ptr->reg.subnet,4)
		|| memcmp(Modbus.getway,ptr->reg.getway,4)
		|| (Modbus.tcp_port != swap_word(ptr->reg.tcp_port) && (Modbus.tcp_port != 0))
		|| (Modbus.tcp_type != ptr->reg.tcp_type)
		)
		{
			
			if(memcmp(Modbus.ip_addr,ptr->reg.ip_addr,4) && (ptr->reg.ip_addr[0] != 0) && (ptr->reg.ip_addr[1] != 0) \
				&& (ptr->reg.ip_addr[2] != 0) && (ptr->reg.ip_addr[3] != 0))
			{	 
				memcpy(Modbus.ip_addr,ptr->reg.ip_addr,4);
				E2prom_Write_Byte(EEP_IP + 3, Modbus.ip_addr[0]);
				E2prom_Write_Byte(EEP_IP + 2, Modbus.ip_addr[1]);
				E2prom_Write_Byte(EEP_IP + 1, Modbus.ip_addr[2]);
				E2prom_Write_Byte(EEP_IP + 0, Modbus.ip_addr[3]);
				flag_reboot = 1;
			}
			if(memcmp(Modbus.subnet,ptr->reg.subnet,4) && (ptr->reg.subnet[0] != 0) && (ptr->reg.subnet[1] != 0) \
				&& (ptr->reg.subnet[2] != 0) && (ptr->reg.subnet[3] != 0))
			{
				memcpy(Modbus.subnet,ptr->reg.subnet,4);
				E2prom_Write_Byte(EEP_SUBNET + 3, Modbus.subnet[0]);
				E2prom_Write_Byte(EEP_SUBNET + 2, Modbus.subnet[1]);
				E2prom_Write_Byte(EEP_SUBNET + 1, Modbus.subnet[2]);
				E2prom_Write_Byte(EEP_SUBNET + 0, Modbus.subnet[3]);	
				flag_reboot = 1;
			}
			if(memcmp(Modbus.getway,ptr->reg.getway,4) && (ptr->reg.getway[0] != 0) && (ptr->reg.getway[1] != 0) \
				&& (ptr->reg.getway[2] != 0) && (ptr->reg.getway[3] != 0))
			{
				memcpy(Modbus.getway,ptr->reg.getway,4);
				E2prom_Write_Byte(EEP_GETWAY + 3, Modbus.getway[0]);
				E2prom_Write_Byte(EEP_GETWAY + 2, Modbus.getway[1]);
				E2prom_Write_Byte(EEP_GETWAY + 1, Modbus.getway[2]);
				E2prom_Write_Byte(EEP_GETWAY + 0, Modbus.getway[3]);
				flag_reboot = 1;
			}
			if(Modbus.tcp_port != swap_word(ptr->reg.tcp_port) && (Modbus.tcp_port != 0))
			{ 						
				Modbus.tcp_port = swap_word(ptr->reg.tcp_port);
				E2prom_Write_Byte(EEP_PORT_LOW,Modbus.tcp_port);
				E2prom_Write_Byte(EEP_PORT_HIGH,Modbus.tcp_port >> 8);
				flag_reboot = 1;
			}
			if(Modbus.tcp_type != ptr->reg.tcp_type)
			{
				Modbus.tcp_type = ptr->reg.tcp_type;			
				E2prom_Write_Byte(EEP_TCP_TYPE,  Modbus.tcp_type );	
				flag_reboot = 1;				
			}
			//
		}
		
		
		
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
	U16_T far *block;			
	if( addr >= MODBUS_SETTING_BLOCK_FIRST && addr <= MODBUS_SETTING_BLOCK_LAST )
	{		
		index = (addr - MODBUS_SETTING_BLOCK_FIRST) / 100;
		block = &Setting_Info.all[index * 200];
		item = (addr - MODBUS_SETTING_BLOCK_FIRST) % 100;			
	}
	else if( addr >= MODBUS_OUTPUT_BLOCK_FIRST && addr <= MODBUS_OUTPUT_BLOCK_LAST )
	{
		index = (addr - MODBUS_OUTPUT_BLOCK_FIRST) / ( (sizeof(Str_out_point) + 1) / 2);
		block = &outputs[index];
		item = (addr - MODBUS_OUTPUT_BLOCK_FIRST) % ((sizeof(Str_out_point) + 1) / 2);
	}
	else if( addr >= MODBUS_INPUT_BLOCK_FIRST && addr <= MODBUS_INPUT_BLOCK_LAST )
	{
		index = (addr - MODBUS_INPUT_BLOCK_FIRST) / ((sizeof(Str_in_point) + 1) / 2);
		block = &inputs[index];
		item = (addr - MODBUS_INPUT_BLOCK_FIRST) % ((sizeof(Str_in_point) + 1) / 2);
	}
	else if( addr >= MODBUS_VAR_BLOCK_FIRST && addr <= MODBUS_VAR_BLOCK_LAST )
	{
		index = (addr - MODBUS_VAR_BLOCK_FIRST) / ((sizeof(Str_variable_point) + 1) / 2);
		block = &inputs[index];
		item = (addr - MODBUS_VAR_BLOCK_FIRST) % ((sizeof(Str_variable_point) + 1) / 2);
	}
	else if( addr >= MODBUS_PRG_BLOCK_FIRST && addr <= MODBUS_PRG_BLOCK_LAST )
	{

		index = (addr - MODBUS_PRG_BLOCK_FIRST) / ((sizeof(Str_program_point) + 1) / 2);
		block = &programs[index];
		item = (addr - MODBUS_PRG_BLOCK_FIRST) % ((sizeof(Str_program_point) + 1) / 2);
	}
	else if( addr >= MODBUS_CODE_BLOCK_FIRST && addr <= MODBUS_CODE_BLOCK_LAST )
	{	
		index = (addr - MODBUS_CODE_BLOCK_FIRST) / 100;
		block = &prg_code[index / (CODE_ELEMENT * MAX_CODE / 200)][CODE_ELEMENT * MAX_CODE % 200];
		item = (addr - MODBUS_CODE_BLOCK_FIRST) % 100;
	}
	else if( addr >= MODBUS_WR_BLOCK_FIRST && addr <= MODBUS_WR_BLOCK_LAST )
	{
		index = (addr - MODBUS_WR_BLOCK_FIRST) / ((sizeof(Str_weekly_routine_point) + 1) / 2);
		block = &weekly_routines[index];
		item = (addr - MODBUS_WR_BLOCK_FIRST) % ((sizeof(Str_weekly_routine_point) + 1) / 2);
	}
	else if( addr >= MODBUS_WR_TIME_FIRST && addr <= MODBUS_WR_TIME_LAST )
	{
		index = (addr - MODBUS_WR_TIME_FIRST) / ((sizeof(Wr_one_day) * MAX_SCHEDULES_PER_WEEK + 1) / 2);
		block = &wr_times[index];
		item = (addr - MODBUS_WR_TIME_FIRST) % ((sizeof(Wr_one_day) * MAX_SCHEDULES_PER_WEEK + 1) / 2);
	}
	else if( addr >= MODBUS_AR_BLOCK_FIRST && addr <= MODBUS_AR_BLOCK_LAST )
	{
		index = (addr - MODBUS_AR_BLOCK_FIRST) / ((sizeof(Str_annual_routine_point) + 1) / 2);
		block = &annual_routines[index];
		item = (addr - MODBUS_AR_BLOCK_FIRST) % ((sizeof(Str_annual_routine_point) + 1) / 2);
		
	}
	else if( addr >= MODBUS_AR_TIME_FIRST && addr <= MODBUS_AR_TIME_LAST )
	{
	
		index = (addr - MODBUS_AR_TIME_FIRST) / (AR_DATES_SIZE / 2);
		block = &ar_dates[index];
		item = (addr - MODBUS_AR_TIME_FIRST) % (AR_DATES_SIZE / 2);
	}
	
	return block[item];
	
}

//void Reset_Zigbee_Moudle(void)
//{
//	U8_T tempbuf[8];
//	U16_T crc_val;
//	uart_init_send_com(1);
//	tempbuf[0] = 0x17;
//	tempbuf[1] = 0x06;
//	tempbuf[2] = 0x00;
//	tempbuf[3] = 0x22;
//	tempbuf[4] = 0x00;
//	tempbuf[5] = 0x01;
//	crc_val = crc16(tempbuf,6);
//	tempbuf[6] = crc_val >> 8;
//	tempbuf[7] = (U8_T)crc_val;
//	uart_send_string(tempbuf, 6 ,1);
//	
//}

#if MINI		

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
	Modbus.com_config[2] = SUB_MODBUS;
	uart_serial_restart(2);
	// response

}


#if MINI

RS232_CMD rs232_cmd;
void send_rs232_command(void)
{
	uint8_t length;
	//Test[33]++;
	if(Modbus.mini_type != MINI_BIG) 
		return;
	Test[30] = rs232_cmd.baut;
	Test[31] = rs232_cmd.type;
	Test[32] = rs232_cmd.cmd[0];
	Test[33] = rs232_cmd.len;
	if( rs232_cmd.baut == 9600)
		uart1_baudrate = UART_9600;
	
	 UART_Init(1);
	
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
				Test[34]++;
				set_subnet_parameters(RECEIVE, 13,1);
				if(length = wait_subnet_response(50,1))	 
				{	Test[35]++;	
					if(subnet_response_buf[11] == 0x0d && subnet_response_buf[12] == 0x0a)
					{					
						// wn0000.00kg
						Test[36]++;
						//memcpy(&Test[20],subnet_response_buf,13);

						if(subnet_response_buf[0] == 'w' && subnet_response_buf[1] == 'n')
						{
							Test[37]++;
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




#endif



