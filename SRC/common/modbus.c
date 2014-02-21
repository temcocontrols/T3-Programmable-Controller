#include "main.h"    
#include "serial.h"
//#include "schedule.h" 
#include "stdlib.h" 




STR_MODBUS  far Modbus;
UN_Time RTC;
bit Mac_Address_write_enable = FALSE;

U16_T far input_raw[32];
U16_T far output_raw[24];

xTaskHandle xdata Handle_MainSerial;
//xTaskHandle xdata Handle_SubSerial;
//xQueueHandle qSubSerial_uart0;
//xQueueHandle qSubSerial_uart2;
xSemaphoreHandle sem_subnet_tx_uart0;
xSemaphoreHandle sem_subnet_tx_uart2;
xSemaphoreHandle sem_subnet_tx_uart1;

U8_T subnet_response_buf[255];
U8_T subnet_rec_package_size; 


U16_T main_rece_count;
U16_T sub1_rece_count;
U16_T zig_rece_count;
U8_T main_rece_size = 0;
U16_T sub1_rece_size = 0;
U16_T zig_rece_size = 0;
U8_T main_dealwithTag;
U8_T sub1_dealwithTag;
U8_T zig_dealwithTag;
U8_T far main_serial_receive_timeout_count, sub1_serial_receive_timeout_count;
U8_T main_transmit_finished, sub1_transmit_finished,zig_transmit_finished;
U8_T far main_data_buffer[MAIN_BUF_LEN];
U8_T far sub1_data_buffer[SUB_BUF_LEN];
U8_T far zig_data_buffer[SUB_BUF_LEN];


U8_T SERIAL_RECEIVE_TIMEOUT;
U8_T SNWriteflag; 
U8_T far rand_read_ten_count;
unsigned short TransID = 1;


//U8_T datalen = 0;
//U8_T  address = 0;

void main_dealwithData( void );
//void subdealwithData( void );


void vStartMainSerialTasks( U8_T uxPriority)
{
//	memset(&Modbus,0,sizeof(STR_MODBUS));
	sTaskCreate( main_dealwithData, "mainserailtask", MainSerialSTACK_SIZE, NULL, uxPriority, &Handle_MainSerial );
}


void initSerial(void)
{
#if defined(CM5)
	MAIN_TXEN = RECEIVE;
#endif
	main_rece_count = 0;
	main_dealwithTag = 0;

//	qSubSerial_uart0 = xQueueCreate(SUB_BUF_LEN, 1);
	vSemaphoreCreateBinary(sem_subnet_tx_uart0);

#if  defined(MINI)
//	qSubSerial_uart2 = xQueueCreate(SUB_BUF_LEN, 1);
	vSemaphoreCreateBinary(sem_subnet_tx_uart2);
	vSemaphoreCreateBinary(sem_subnet_tx_uart1);
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
U8_T SubCRChi = 0xff;
U8_T SubCRClo = 0xff;

U8_T EthernetCRChi = 0xff;
U8_T EthernetCRClo = 0xff;

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
	if(port == UART_MAIN && Modbus.zigbee == 0)
	{
		UART_Init(port);
	   	main_transmit_finished = 0;
		#if (defined(CM5))
		MAIN_TXEN = SEND;
		#endif
	}
	else if(port == UART_SUB1)
	{
		UART_Init(port);
		sub1_transmit_finished = 0;
		SUB1_TXEN = SEND;
	}
#if  defined(MINI)
	else if(port == UART_SUB2)
	{
		UART_Init(port);
		SUB2_TXEN = SEND;
	}
	else if(port == UART_ZIG && Modbus.zigbee == 1)
	{
		UART_Init(port);
	//	SUB2_TXEN = SEND;
	}
#endif
}

void uart_send_byte(U8_T buffer,U8_T port)
{
	U16_T count = 0;

	if(port == UART_SUB1)
	{
		SUB1_UART_SBUF = buffer;
		sub1_transmit_finished = 0;
		while(!sub1_transmit_finished && count < 5000) 
		{
			count++;
		}
	}
#if  defined(MINI)
	else if(port == UART_MAIN && Modbus.zigbee == 0)
	{
		MAIN_UART_SBUF = buffer;
		main_transmit_finished = 0;
		while(!main_transmit_finished && count < 5000) 
		{
			count++;
		}	
	}
	else if(port == UART_SUB2)
	{
	   	HSUR_PutChar(buffer);
	}
	else if(port == UART_ZIG && Modbus.zigbee == 1)
	{
		ZIG_UART_SBUF = buffer;
		zig_transmit_finished = 0;
		while(!zig_transmit_finished && count < 5000) 
		{
			count++;
		}
	}
#endif

}


void uart_send_string(U8_T *p, U16_T length,U8_T port)
{
	U16_T i;

	for(i = 0; i < length; i++)
	{
		uart_send_byte(p[i],port);
	}
#if  defined(MINI)
	if(port == UART_SUB2) 
	{
		flagLED_uart2_tx = 1; 
		if(length > 100)	vTaskDelay(80 / portTICK_RATE_MS); // 19200	
		else vTaskDelay(5 / portTICK_RATE_MS);
	}
#endif
}


void set_subnet_parameters(U8_T io, U8_T length,U8_T port)
{
	U16_T temp = 0;
	subnet_rec_package_size = length;  
	if(port == UART_SUB1)
	{	
		sub1_rece_count = 0;
		memset(sub1_data_buffer,0,subnet_rec_package_size);
		SUB1_TXEN = io;	
	
	}  
#if  defined(MINI)

	else if(port == UART_SUB2)
	{	
		SUB2_TXEN = io;
//		if(io == RECEIVE)	
//		{
//			while(hsurRxCount < length && temp < 1000)
//			{
//			vTaskDelay(1);
//			temp++;
//			}
//		}
	}
	else if(port == UART_ZIG && Modbus.zigbee == 1)
	{
		zig_rece_count = 0;
		memset(zig_data_buffer,0,subnet_rec_package_size);
		
	}
#endif
}
  



U8_T wait_subnet_response(U16_T nDoubleTick,U8_T port)
{
	U16_T i, length;
	U8_T cTaskWokenByPost = FALSE;
	port = UART_SUB1;
	if(port == UART_SUB1)
	{ 		
		for(i = 0; i < nDoubleTick; i++)
		{
			if((length = sub1_rece_count) >= subnet_rec_package_size)
			{	
				return length;
			}			
			vTaskDelay(1);
		}
		
	} 
#if defined(MINI)

	else if(port == UART_SUB2)
	{
		for(i = 0; i < nDoubleTick; i++)
		{	
			if(length > 0)	
			{	flagLED_uart2_rx = 1;	
				uart2_heartbeat = 0;
			}
			if((length = hsurRxCount) >= subnet_rec_package_size)
			{			
				return length;
			}
			vTaskDelay(1);
		}
	}
	else if(port == UART_ZIG)
	{  // ZIGBEE need more delay
	 	for(i = 0; i < nDoubleTick * 5; i++)
		{	
			if((length = zig_rece_count) >= subnet_rec_package_size)
			{	
				return length;
			}
			vTaskDelay(1);
		}
	}
#endif
	return 0;
}


void mstp_int_handler(void);

#if defined(MINI)
void zigbee_uart_int_hander(void)
{
	if(ZIG_UART_RI == 1)
	{	
		Test[41]++;
		flagLED_uart1_rx = 1; 
		uart1_heartbeat = 0;
		ZIG_UART_RI = 0;
		zig_data_buffer[zig_rece_count++] = ZIG_UART_SBUF;	 		
	}
	else if(ZIG_UART_TI == 1)
	{
		Test[40]++;
		ZIG_UART_TI = 0;
		zig_transmit_finished = 1;
		flagLED_uart1_tx = 1; 
	}
	return;
}
#endif

void modubs_main_uart_int_hander(void)
{
	if(MAIN_UART_RI == 1)
	{
		flagLED_uart1_rx = 1; 
	//	uart1_heartbeat = 0;
		MAIN_UART_RI = 0;		
		if(main_rece_count < MAIN_BUF_LEN)
			main_data_buffer[main_rece_count++] = MAIN_UART_SBUF;
		else
			main_serial_restart();
		
	//	uart1_serial_receive_timeout_count = 2;//SERIAL_RECEIVE_TIMEOUT;
		if(main_rece_count == 1)
		{
			main_rece_size = 8;
	//		uart1_serial_receive_timeout_count = 2;//SERIAL_RECEIVE_TIMEOUT;
		}
		else if(main_rece_count == 4)
		{
			if((((U16_T)(main_data_buffer[2] << 8) + main_data_buffer[3]) == 0x0a) && (main_data_buffer[1] == WRITE_VARIABLES))
			{
				main_rece_size = DATABUFLEN_SCAN;
	//			uart1_serial_receive_timeout_count = 2;//SERIAL_RECEIVE_TIMEOUT;	
			}
		}
		else if(main_rece_count == 7)
		{
			if(main_data_buffer[1] == MULTIPLE_WRITE)
			{
				main_rece_size = main_data_buffer[6] + 9;
	//			uart1_serial_receive_timeout_count = main_data_buffer[6] + 8;//from 4 to 8 to see if can resolve multiple problem
			}
		}
		else if(main_rece_count >= main_rece_size)
		{	 
	//		uart1_serial_receive_timeout_count = 0;
			main_dealwithTag = VALID_PACKET;
		}
	}
	else if(MAIN_UART_TI == 1)
	{
		MAIN_UART_TI = 0;
		main_transmit_finished = 1;
		flagLED_uart1_tx = 1; 
	}

	return;
}
//------------------------serialport ----------------------------------
//	serial port interrupt , deal with all action about serial port. include receive data and 
//		send data and deal with interal.


void handle_main_RX(void) interrupt MAIN_UART_VECTOR
{
#if defined(MINI)

	if(Modbus.zigbee == 1)	 // add jumper for zigbee
	{
		zigbee_uart_int_hander();
	}
	else   // for main port 232, test port
#endif
		modubs_main_uart_int_hander();
}


void modubs_sub1_uart_int_hander(void)
{
//	U16_T crc_val;
//	U8_T cTaskWokenByPost = FALSE;
//	U8_T buf;

	if(SUB1_UART_RI == 1)
	{
		flagLED_uart0_rx = 1; 
		uart0_heartbeat = 0;
		SUB1_UART_RI = 0;
				
		sub1_data_buffer[sub1_rece_count++] = SUB1_UART_SBUF;

		if(sub1_data_buffer[0] == 0)
		{
		   	sub1_rece_count--;
		}
//		cQueueSendFromISR(qSubSerial_uart0, (void *)&buf, cTaskWokenByPost);
//		if(cTaskWokenByPost != pdFALSE)
//	        taskYIELD();

  	}
	else if(SUB1_UART_TI == 1)
	{
		SUB1_UART_TI = 0;
		sub1_transmit_finished = 1;
		flagLED_uart0_tx = 1; 
	}
	return;

}



void handle_sub1_RX(void) interrupt SUB1_UART_VECTOR
{
	if(Modbus.protocal == BAC_MSTP)
	{
		mstp_int_handler();
	}
	else
		modubs_sub1_uart_int_hander();
}


	
/*********************************************\
*	MAIN NET CODE START
\*********************************************/
void main_serial_restart(void)
{
#if (defined(CM5))
	MAIN_TXEN = RECEIVE;
#endif
	main_rece_count = 0;
	main_dealwithTag = 0;
}


void main_init_send_com(void)
{
	main_transmit_finished = 0;
	#if (defined(CM5))
	MAIN_TXEN = SEND;
	#endif
}




// ------------------------dealwithdata -------------------------
// the routine dealwith data ,it has three steps.
// the 3 step is : 1 prepare to send data and init crc for next Tim
//				   2 dealwith interal
//                 3 organize the data of sending, and send data.

void main_dealwithData(void)
{
	U16_T far address;
//	U8_T frametype;
	portTickType xDelayPeriod = ( portTickType ) 50 / portTICK_RATE_MS;
	for( ; ;)
	{ 	
		vTaskDelay(xDelayPeriod);

		if(main_dealwithTag == VALID_PACKET)
		{					
			address = ((U16_T)main_data_buffer[2] << 8) + main_data_buffer[3];
			if (checkData(address))
			{			
				
				if(main_data_buffer[0] ==  Modbus.address || main_data_buffer[0] == 255)
				{
					main_init_send_com();
		
					main_init_crc16();
		
					main_responseData(address);
				}				
				else
				{  			   
					Response_MAIN_To_SUB(main_data_buffer,main_rece_size - 2,Modbus.sub_port);

				} 
			}  
							
		// Restart the serial receive.
			main_serial_restart(); 		
		}
		else
		{
			main_serial_restart(); 		
		/*	
			//if(main_serial_receive_timeout_count > 10) 	main_serial_receive_timeout_count = 5;
			//if(main_serial_receive_timeout_count > 0)
			{
				main_serial_receive_timeout_count--;
				if(main_serial_receive_timeout_count == 0)
				{
					main_serial_restart();
				//	main_net_init();
				}
			} 
		*/
		} 
		
	}

}


//---------------------checkdata ----------------------
//This function calculates and verifies the checksum
U8_T checkData(U8_T address)
{
	U16_T crc_val;
	U8_T minaddr,maxaddr, variable_delay;
	U8_T randval;

	U8_T i;

//	if(main_data_buffer[0] != 255 && main_data_buffer[0] !=  address && main_data_buffer[0] != 0)
//		return FALSE;

	if(main_data_buffer[1] == CHECKONLINE)
	{
		crc_val = crc16(main_data_buffer, 4);
		if(crc_val != (main_data_buffer[4] << 8) + main_data_buffer[5])
			return FALSE;

		minaddr = (main_data_buffer[2] >= main_data_buffer[3]) ? main_data_buffer[3] : main_data_buffer[2];	
		maxaddr = (main_data_buffer[2] >= main_data_buffer[3]) ? main_data_buffer[2] : main_data_buffer[3];	
		if( address < minaddr ||  address > maxaddr)
			return FALSE;
		else
		{
			variable_delay = rand() % 20;
			for (i = 0; i < variable_delay; i++);				
			return TRUE;
		}
	}
	
	if((main_data_buffer[1] != READ_VARIABLES) && (main_data_buffer[1] != WRITE_VARIABLES) && (main_data_buffer[1] != MULTIPLE_WRITE))
		return FALSE;
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
	

	crc_val = crc16(main_data_buffer, main_rece_size - 2);

	if(crc_val == ((U16_T)main_data_buffer[main_rece_size - 2] << 8) | main_data_buffer[main_rece_size - 1])
		return TRUE;
	else
		return FALSE;
}


void main_responseData(U16_T address)
{
	responseCmd(MODBUS,main_data_buffer,NULL);
}


/* reponse READ command*/
void responseCmd(U8_T type,U8_T* pData,HTTP_SERVER_CONN * pHttpConn)  
{	
    U16_T far StartAdd;  
	U16_T far RegNum;
	U8_T far HeadLen; // Modbus head is 0, Tcp head is 6
	U16_T far loop;
	U8_T far sendbuf[200];
	U8_T far cmd;
//	U8_T far temp;
//	U8_T far tempbit;
	U8_T far temp_number;
	U8_T far temp_address;
	U8_T far send_buffer;
//	U8_T far tempbuf[100];  /* convert schedule structure */
	U8_T far update_flash;

	if(type == SERIAL || type == USB)  // modbus packet
	{
		HeadLen = 0;		
	}
	else   // TCP packet
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
				sendbuf[HeadLen + 3 + loop * 2 + 1] = SW_REV & 0xff;
			}
			else if(StartAdd + loop == MODBUS_FIRMWARE_VERSION_NUMBER_HI)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = SW_REV >> 8;			
			}
			else if(StartAdd + loop == MODBUS_HARDWARE_REV)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
			#if defined(CM5)
				sendbuf[HeadLen + 3 + loop * 2 + 1] = HW_REV;
			#else if  defined(MINI)
			if(Modbus.mini_type == MINI_BIG)
				sendbuf[HeadLen + 3 + loop * 2 + 1] = MINI_BIG_HW;
			else
				sendbuf[HeadLen + 3 + loop * 2 + 1] = MINI_SMALL_HW;
			#endif 
			}
			else if(StartAdd + loop ==  MODBUS_ADDRESS)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Modbus.address;		
			}
			else if(StartAdd + loop == MODBUS_PRODUCT_MODEL)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
			#if defined(CM5)	
				sendbuf[HeadLen + 3 + loop * 2 + 1] = PRODUCT_CM5;
			#else if  defined(MINI)
			if(Modbus.mini_type == MINI_BIG)
				sendbuf[HeadLen + 3 + loop * 2 + 1] = PRODUCT_MINI_BIG;
			else
				sendbuf[HeadLen + 3 + loop * 2 + 1] = PRODUCT_MINI_SMALL;
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
				#if defined(CM5)	
				sendbuf[HeadLen + 3 + loop * 2 + 1] = CM5_PIC_REV;
				#else if  defined(MINI)
				sendbuf[HeadLen + 3 + loop * 2 + 1] = MINI_PIC_REV;
				#endif
			}
			else if(StartAdd + loop == MODBUS_BAUDRATE)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Modbus.baudrate;
			}
			else if(StartAdd + loop == MODBUS_PROTOCAL)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Modbus.protocal;
			}
			#if  defined(MINI)
			else if(StartAdd + loop == MODBUS_MINI_TYPE)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Modbus.mini_type;
			}
			#endif
//			else if(StartAdd + loop == MODBUS_RESET_PARAMETER)
//			{
//				sendbuf[HeadLen + 3 + loop * 2] = 0;
//				sendbuf[HeadLen + 3 + loop * 2 + 1] =  protocal;
//			}	
			else if(StartAdd + loop == MODBUS_INSTANCE)
			{
				sendbuf[HeadLen + 3 + loop * 2] = Instance >> 8;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Instance;
			}
			else if(StartAdd + loop == MODBUS_STATION_NUM)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Station_NUM;
			}
//			else if(StartAdd + loop == MODBUS_SUB_PORT)
//			{
//				sendbuf[HeadLen + 3 + loop * 2] = 0;
//				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Modbus.sub_port;
//			}
			else if(StartAdd + loop == MODBUS_ZIGBEE_EN)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Modbus.zigbee;
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
			else if(StartAdd + loop == MODBUS_TOTAL_NO)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  sub_no;

			}
			else if(StartAdd + loop >= MODBUS_SUBADDR_FIRST && StartAdd + loop <= MODBUS_SUBADDR_LAST)
			{  // 18 - 25
		
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  sub_addr[StartAdd + loop  - MODBUS_SUBADDR_FIRST];
			}

//  sub infomation block		
//			else if(StartAdd + loop == MODBUS_OUT_NUM)
//			{
//				sendbuf[HeadLen + 3 + loop * 2] = 0;
//				sendbuf[HeadLen + 3 + loop * 2 + 1] = max_out;
//			}
//			else if(StartAdd + loop == MODBUS_IN_NUM)
//			{
//				sendbuf[HeadLen + 3 + loop * 2] = 0;
//				sendbuf[HeadLen + 3 + loop * 2 + 1] = max_in;
//			}
//			else if(StartAdd + loop == MODBUS_VAR_NUM)
//			{
//				sendbuf[HeadLen + 3 + loop * 2] = 0;
//				sendbuf[HeadLen + 3 + loop * 2 + 1] = max_var;
//			}
//			else if(StartAdd + loop == MODBUS_CON_NUM)
//			{
//				sendbuf[HeadLen + 3 + loop * 2] = 0;
//				sendbuf[HeadLen + 3 + loop * 2 + 1] = max_con;
//			}
//			else if(StartAdd + loop == MODBUS_WR_NUM)
//			{
//				sendbuf[HeadLen + 3 + loop * 2] = 0;
//				sendbuf[HeadLen + 3 + loop * 2 + 1] = max_wr;
//			}
//			else if(StartAdd + loop == MODBUS_AR_NUM)
//			{
//				sendbuf[HeadLen + 3 + loop * 2] = 0;
//				sendbuf[HeadLen + 3 + loop * 2 + 1] = max_ar;
//			}
//			else if(StartAdd + loop == MODBUS_PRG_NUM)
//			{
//				sendbuf[HeadLen + 3 + loop * 2] = 0;
//				sendbuf[HeadLen + 3 + loop * 2 + 1] = max_prg;
//			}
//			else if(StartAdd + loop == MODBUS_TBL_NUM)
//			{
//				sendbuf[HeadLen + 3 + loop * 2] = 0;
//				sendbuf[HeadLen + 3 + loop * 2 + 1] = max_tbl;
//			}
//			else if(StartAdd + loop == MODBUS_IN_NUM)
//			{
//				sendbuf[HeadLen + 3 + loop * 2] = 0;
//				sendbuf[HeadLen + 3 + loop * 2 + 1] = max_in;
//			}
//			else if(StartAdd + loop == MODBUS_IN_NUM)
//			{
//				sendbuf[HeadLen + 3 + loop * 2] = 0;
//				sendbuf[HeadLen + 3 + loop * 2 + 1] = max_in;
//			}
//			else if(StartAdd + loop == MODBUS_IN_NUM)
//			{
//				sendbuf[HeadLen + 3 + loop * 2] = 0;
//				sendbuf[HeadLen + 3 + loop * 2 + 1] = max_in;
//			}
			else if(StartAdd + loop >= MODBUS_SUB_INFO_FIRST && StartAdd + loop <= MODBUS_SUB_INFO_LAST)
			{
				U8_T index;
				U8_T reg;
				index = (StartAdd + loop - MODBUS_SUB_INFO_FIRST) / Tst_reg_num;
				reg = (StartAdd + loop - MODBUS_SUB_INFO_FIRST) % Tst_reg_num;
				if(reg == TST_PRODUCT_MODEL)
				{
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = tst_info[index].product_model;	
				}
				else	if(reg == TST_OCCUPIED)
				{
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = tst_info[index].occupied;
				} 
				else	if(reg == TST_COOL_SETPOINT)
				{
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = tst_info[index].cool_setpoint;
				}
				else	if(reg == TST_HEAT_SETPOINT)
				{
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = tst_info[index].heat_setpoint;
				}
				else	if(reg == TST_ROOM_SETPOINT)
				{
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = tst_info[index].setpoint;
				}
				else	if(reg == TST_ROOM_TEM)
				{
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = tst_info[index].temperature;
				}
				else	if(reg == TST_MODE)
				{
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = tst_info[index].mode;
				}
				else	if(reg == TST_OUTPUT_STATE)
				{
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = tst_info[index].output_state;
				}
				else	if(reg == TST_NIGHT_HEAT_DB)
				{
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = tst_info[index].night_heat_db;
				}
				else	if(reg == TST_NIGHT_COOL_DB)
				{
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = tst_info[index].night_cool_db;
				}
				else	if(reg == TST_NIGHT_HEAT_SP)
				{
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = tst_info[index].night_heat_sp;
				}
				else	if(reg == TST_NIGHT_COOL_SP)
				{
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = tst_info[index].night_cool_sp;
				}
				else if(reg == TST_ADDRESS)
				{
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = scan_db[index].id;//tst_info[index].serial_number[0];
				}
				else if(reg == TST_PORT)
				{
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = scan_db[index].port;//tst_info[index].serial_number[0];
				}
//				else	if(reg == TST_OVER_RIDE)
//				{
//					sendbuf[HeadLen + 3 + loop * 2] = 0;
//					sendbuf[HeadLen + 3 + loop * 2 + 1] = tst_info[index].over_ride;
//				}
				else	if(reg == TST_SERIAL_NUM_0)
				{
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)scan_db[index].sn;//tst_info[index].serial_number[0];
				}
				else	if(reg == TST_SERIAL_NUM_1)
				{
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)(scan_db[index].sn >> 8);//tst_info[index].serial_number[1];
				}
				else	if(reg == TST_SERIAL_NUM_2)
				{
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)(scan_db[index].sn >> 16);//tst_info[index].serial_number[2];
				}
				else	if(reg == TST_SERIAL_NUM_3)
				{
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)(scan_db[index].sn >> 24);//tst_info[index].serial_number[3];
				}
			}
			
/*  END SUB TSTAT */
/******************************end for tstst resigter ************************************************/
		   //  input block
			else if(StartAdd + loop >= MODBUS_INPUT_FIRST && StartAdd + loop <= MODBUS_INPUT_LAST)
			{
			 	U8_T index;
				U8_T reg;
				index = (StartAdd + loop - MODBUS_INPUT_FIRST) / MAX_INS;
				reg = (StartAdd + loop - MODBUS_INPUT_FIRST) % MAX_INS;
			   	if(index < e_in_label)
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = inputs[reg].description[index - e_in_description];
				}
				else if(index < e_in_value)
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = inputs[reg].label[index - e_in_description];
				}
				else if(index < e_in_filter)
				{
					if(index < e_in_filter + 2)
					{
						sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(inputs[reg].value >> 24);
						sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)(inputs[reg].value >> 16);
					}
					else
					{
						sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(inputs[reg].value >> 8);
						sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)inputs[reg].value;
					}
				}
				else if(index == e_in_filter)
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = inputs[reg].filter;
				}
				else if(index == e_in_decom)
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = inputs[reg].decom;
				}
				else if(index == e_in_sen_on)				
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = inputs[reg].sen_on;
				}
				else if(index == e_in_sen_off)				
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = inputs[reg].sen_off;
				}
				else if(index == e_in_control)				
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = inputs[reg].control;
				}
				else if(index == e_in_auto_manua)				
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = inputs[reg].auto_manual;
				}
				else if(index == e_in_digital_analog)				
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = inputs[reg].digital_analog;
				}
				else if(index == e_in_calibration_sign)				
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = inputs[reg].calibration_sign;
				}
				else if(index == e_in_calibration_increment)				
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = inputs[reg].calibration_increment;
				}
				else if(index == e_in_unused)				
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = inputs[reg].unused;
				}
				else if(index == e_in_calibration)				
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = inputs[reg].calibration;
				}
				else if(index == e_in_range)				
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = inputs[reg].range;
				}

			}
			else if(StartAdd + loop >= MODBUS_OUTPUT_FIRST && StartAdd + loop <= MODBUS_OUTPUT_LAST)
			{
			  	U8_T index;
				U8_T reg;
				index = (StartAdd + loop - MODBUS_OUTPUT_FIRST) / MAX_OUTS;
				reg = (StartAdd + loop - MODBUS_OUTPUT_FIRST) % MAX_OUTS;
			   	if(index < e_out_label)
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = outputs[reg].description[index - e_in_description];
				}
				else if(index < e_out_value)
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = outputs[reg].label[index - e_in_description];
				}
				else if(index < e_out_auto_manual)
				{
					if(index < e_out_auto_manual + 2)
					{
						sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(outputs[reg].value >> 24);
						sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)(outputs[reg].value >> 16);
					}
					else
					{
						sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(outputs[reg].value >> 8);
						sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)outputs[reg].value;
					}
				}
				else if(index == e_out_auto_manual)
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = outputs[reg].auto_manual;
				}
				else if(index == e_out_digital_analog)
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = outputs[reg].digital_analog;
				}
				else if(index == e_out_switch_status)				
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = outputs[reg].switch_status;
				}
				else if(index == e_out_control)				
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = outputs[reg].control;
				}
				else if(index == e_out_digital_control)				
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = outputs[reg].digital_control;
				}
				else if(index == e_out_decom)				
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = outputs[reg].decom;
				}
				else if(index == e_out_range)				
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = outputs[reg].range;
				}
				else if(index == e_out_m_del_low)				
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = outputs[reg].m_del_low;
				}
				else if(index == e_out_s_del_high)				
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = outputs[reg].s_del_high;
				}
				else if(index == e_out_delay_timer)				
				{
						sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(outputs[reg].delay_timer >> 8);
						sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)outputs[reg].delay_timer;
				}
			}
			else if(StartAdd + loop >= MODBUS_WR_FIRST && StartAdd + loop <= MODBUS_WR_LAST)
			{
			  	U8_T index;
				U8_T reg;
				index = (StartAdd + loop - MODBUS_WR_FIRST) / MAX_WR;
				reg = (StartAdd + loop - MODBUS_WR_FIRST) % MAX_WR;
			   	if(index < e_week_label)
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = weekly_routines[reg].description[index - e_in_description];
				}
				else if(index < e_week_value)
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = weekly_routines[reg].label[index - e_in_description];
				}
				else if(index < e_week_auto_manual)
				{
					if(index < e_out_auto_manual + 2)
					{
						sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(weekly_routines[reg].value >> 24);
						sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)(weekly_routines[reg].value >> 16);
					}
					else
					{
						sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(weekly_routines[reg].value >> 8);
						sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)weekly_routines[reg].value;
					}
				}
				else if(index == e_week_auto_manual)
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = weekly_routines[reg].auto_manual;
				}
				else if(index == e_week_override_1_value)
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = weekly_routines[reg].override_1_value;
				}
				else if(index == e_week_override_2_value)				
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = weekly_routines[reg].override_2_value;
				}
				else if(index == e_week_off)				
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = weekly_routines[reg].off;
				}
//				else if(index == e_week_unused)				
//				{
//						sendbuf[HeadLen + 3 + loop * 2] = 0;
//						sendbuf[HeadLen + 3 + loop * 2 + 1] = outputs[reg].unused;
//				}
//				else if(index == e_week_override_1)	// 3bytes			
//				{
//						sendbuf[HeadLen + 3 + loop * 2] = 0;
//						sendbuf[HeadLen + 3 + loop * 2 + 1] = weekly_routines[reg].decom;
//				}
//				else if(index == e_week_override_2)	// 3bytes			
//				{
//						sendbuf[HeadLen + 3 + loop * 2] = 0;
//						sendbuf[HeadLen + 3 + loop * 2 + 1] = weekly_routines[reg].range;
//				}
			}
			else if(StartAdd + loop >= MODBUS_AR_FIRST && StartAdd + loop <= MODBUS_AR_LAST)
			{
			  	U8_T index;
				U8_T reg;
				index = (StartAdd + loop - MODBUS_AR_FIRST) / MAX_AR;
				reg = (StartAdd + loop - MODBUS_AR_FIRST) % MAX_AR;
			   	if(index < e_ar_label)
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = annual_routines[reg].description[index - e_in_description];
				}
				else if(index < e_ar_value)
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = annual_routines[reg].label[index - e_in_description];
				}
				else if(index < e_ar_auto_manual)
				{
					if(index < e_ar_auto_manual + 2)
					{
						sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(annual_routines[reg].value >> 24);
						sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)(annual_routines[reg].value >> 16);
					}
					else
					{
						sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(annual_routines[reg].value >> 8);
						sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)annual_routines[reg].value;
					}
				}
				else if(index == e_ar_auto_manual)
				{
						sendbuf[HeadLen + 3 + loop * 2] = 0;
						sendbuf[HeadLen + 3 + loop * 2 + 1] = annual_routines[reg].auto_manual;
				}
			}
			else if(StartAdd + loop >= MODBUS_WR_TIME_FIRST && StartAdd + loop < MODBUS_WR_TIME_LAST)
			{	
				U8_T index;
				U8_T reg;
				U8_T len;
				U8_T loop;
				U8_T seg;
				index = (StartAdd + loop - MODBUS_WR_TIME_FIRST) / MAX_WR;
				reg = (StartAdd + loop - MODBUS_WR_TIME_FIRST) % MAX_WR; 
				seg = reg / MAX_SCHEDULES_PER_WEEK / 2;
				if(seg % 2 == 0)
				{
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] =  wr_times[index][reg / 16].time[seg / 8].hours;
				}
				else
				{
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] =  wr_times[index][reg / 16].time[seg / 8].minutes;
				}
			}
			else if(StartAdd + loop >= MODBUS_AR_TIME_FIRST && StartAdd + loop < MODBUS_AR_TIME_LAST)
			{	
				U8_T index;
				U8_T reg;
				index = (StartAdd + loop - MODBUS_AR_TIME_FIRST) / MAX_AR;
				reg = (StartAdd + loop - MODBUS_AR_TIME_FIRST) % MAX_AR; 
			
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  ar_dates[index][reg];
				
			}
			else if(StartAdd + loop >= MODBUS_TIMER_ADDRESS && StartAdd + loop < MODBUS_TIMER_ADDRESS + 8)
			{	 
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  RTC.all[StartAdd + loop - MODBUS_TIMER_ADDRESS];
			}			
	/******************end of names of inputs and outputs ******************/
			else if( StartAdd + loop >= MODBUS_TEST && StartAdd + loop <= MODBUS_TEST_50 )
			{
				sendbuf[HeadLen + 3 + loop * 2] = (Test[StartAdd + loop - MODBUS_TEST] >> 8) & 0xFF;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = Test[StartAdd + loop - MODBUS_TEST] & 0xFF;
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
			U8_T i;

			crc_val = crc16(sendbuf,RegNum * 2 + 3);
			sendbuf[RegNum * 2 + 3]	= crc_val >> 8;
			sendbuf[RegNum * 2 + 4]	= (U8_T)crc_val;

			if(type == SERIAL)
			{
				uart_init_send_com(UART_MAIN);
				for(i = 0;i < RegNum * 2 + 5;i++)
				{
					uart_send_byte(sendbuf[i],UART_MAIN);
				}
			}
			else
			{
				memcpy(UpBuf,sendbuf,RegNum * 2 + 5); 
				UpIndex = 0;
				UpCtr = RegNum * 2 + 5;
				ENDP2_NEED_UP_FLAG = 1;	
			}
		}		
		else // TCP
		{
			TransID =  ((U16_T)pData[0] << 8) | pData[1];
			sendbuf[0] = TransID >> 8;			//	TransID
			sendbuf[1] = (U8_T)TransID;	
			sendbuf[2] = 0;			//	ProtoID
			sendbuf[3] = 0;
			sendbuf[4] = (6 + RegNum * 2) >> 8;	//	Len
			sendbuf[5] = (U8_T)(6 + RegNum * 2) ;
		    flagLED_ether_tx = 1;
			TCPIP_TcpSend(pHttpConn->TcpSocket, sendbuf, RegNum * 2 + HeadLen + 3, TCPIP_SEND_NOT_FINAL); 
		}
	}
	else if(cmd == WRITE_VARIABLES)
	{		
		if(type == SERIAL || type == USB)
		{ 	
			U8_T i;				
			if(type == SERIAL)
			{		
				uart_init_send_com(UART_MAIN);
				for(i = 0;i < 8;i++)
				{
					uart_send_byte(pData[i],UART_MAIN);
				}
			}
			else
			{
				memcpy(UpBuf,pData,8); 
				UpIndex = 0;
				ENDP2_NEED_UP_FLAG = 1;	
				UpCtr = 8;	
			}
		}
		else // TCP   dont have CRC 
		{
		//	SetTransactionId(6 + UIP_HEAD);
			sendbuf[0] = pData[0];//0;			//	TransID
			sendbuf[1] = pData[1];//TransID++;	
			sendbuf[2] = 0;			//	ProtoID
			sendbuf[3] = 0;
			sendbuf[4] = (3 + UIP_HEAD) >> 8;	//	Len
			sendbuf[5] = (U8_T)(3 + UIP_HEAD) ;

			for (loop = 0;loop < 6;loop++)
			{
				sendbuf[HeadLen + loop] = pData[HeadLen + loop];	
			}
			flagLED_ether_tx = 1;
			TCPIP_TcpSend(pHttpConn->TcpSocket, sendbuf, 6 + UIP_HEAD, TCPIP_SEND_NOT_FINAL); 
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
		else if(StartAdd == MODBUS_ADDRESS )
		{
			Modbus.address = pData[HeadLen + 5]+ (pData[HeadLen + 4]<<8);
			E2prom_Write_Byte(EEP_ADDRESS, Modbus.address);
		}
		else if(StartAdd == MODBUS_BAUDRATE )
		{
			Modbus.baudrate = pData[HeadLen + 5];
			E2prom_Write_Byte(EEP_BAUDRATE, pData[HeadLen + 5] );
		
			if(pData[HeadLen + 5] == 1)
			{
				PCON  |= 0XC0 ;
				//SERIAL_RECEIVE_TIMEOUT = 3;
			}
			else
			{
		 		PCON  = 0X00 ;
			//	SERIAL_RECEIVE_TIMEOUT = 6;
			}
		}
		else if(StartAdd == MODBUS_PROTOCAL)
		{
			 Modbus.protocal = pData[HeadLen + 5];
		  	 E2prom_Write_Byte(EEP_PROTOCAL, Modbus.protocal);
		}
		#if defined(MINI)
		
		else if(StartAdd == MODBUS_MINI_TYPE)
		{
			Modbus.mini_type = pData[HeadLen + 5]+ (pData[HeadLen + 4]<<8);
			E2prom_Write_Byte(EEP_MINI_TYPE, Modbus.mini_type);
			Start_Comm_Top();
		}
		#endif
		else if(StartAdd == MODBUS_INSTANCE)
		{
			Instance = pData[HeadLen + 5]+ (pData[HeadLen + 4]<<8);
			E2prom_Write_Byte(EEP_INSTANCE_HIGH,  pData[HeadLen + 4] );
			E2prom_Write_Byte(EEP_INSTANCE_LOW,  pData[HeadLen + 5] );
		}
		else if(StartAdd == MODBUS_STATION_NUM)
		{
			Station_NUM = pData[HeadLen + 5];
			E2prom_Write_Byte(EEP_STATION_NUM,  Station_NUM );
		} 
//		else if(StartAdd == MODBUS_SUB_PORT)
//		{
//			Modbus.sub_port = pData[HeadLen + 5];
//		}
		else if(StartAdd == MODBUS_ZIGBEE_EN)
		{
			Modbus.zigbee = pData[HeadLen + 5];
			E2prom_Write_Byte(EEP_ZIGBEE_EN,  Modbus.zigbee );
		}
		else if(StartAdd == MODBUS_CLEAR_SCAN_DB)
		{
			reset_scan_db_flag = 1;
		}
		else if(StartAdd == MODBUS_ENABLE_WRITE_MAC)
		{
			Mac_Address_write_enable = 1;
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
		}
		else if(StartAdd >= MODBUS_SUBNET_1 && StartAdd <= MODBUS_SUBNET_4)
		{
			 Modbus.subnet[StartAdd - MODBUS_SUBNET_1] = pData[HeadLen + 5];
			 E2prom_Write_Byte(EEP_SUBNET + 3 - ( StartAdd - MODBUS_SUBNET_1), Modbus.subnet[StartAdd - MODBUS_SUBNET_1]);
		}
		else if(StartAdd >= MODBUS_GETWAY_1 && StartAdd <= MODBUS_GETWAY_4)
		{
			 Modbus.getway[StartAdd - MODBUS_GETWAY_1] = pData[HeadLen + 5];
		//	E2prom_Write_Byte(EEP_Modbus.GETWAY + StartAdd - MODBUS_Modbus.GETWAY_1, Modbus.GETWAY[StartAdd - MODBUS_Modbus.GETWAY_1]);
		}
		else if(StartAdd == MODBUS_TCP_LISTEN_PORT)
		{
			Modbus.tcp_port =  pData[HeadLen + 5]+ (pData[HeadLen + 4]<<8);
			E2prom_Write_Byte(EEP_PORT_LOW,pData[HeadLen + 5]);
			E2prom_Write_Byte(EEP_PORT_HIGH,pData[HeadLen + 4]);
		}
		
		else if(StartAdd == MODBUS_RESET_PARAMETER )
		{
		//	write_parameters_to_nodes(1,Tst_Register[TST_OCCUPIED][0],0);
			if(pData[HeadLen + 5] == 55)
			{
			client_ip[0] = 202;
			client_ip[1] = 0;
			client_ip[2] = 168;
			client_ip[3] = 192;
			newsocket = TCPIP_UdpNew(2, 3, ((U32_T)client_ip[3] << 24) + ((U32_T)client_ip[2] << 16) + (U16_T)(client_ip[1] << 8) + client_ip[0], 0, 47808);
			Test[36] = newsocket;
			}
			#if  defined(MINI)

			if(pData[HeadLen + 5] == 22)
			{  // reset top board
				Rsest_Top();
			}
			#endif
			if(pData[HeadLen + 5] == 100)
			{	IntFlashErase(ERA_RUN,0x70000);
				set_default_parameters();
			}
		}
		else if(StartAdd >= MODBUS_TIMER_ADDRESS && StartAdd <= MODBUS_TIMER_ADDRESS + 7)
		{
			 RTC.all[StartAdd - MODBUS_TIMER_ADDRESS] = pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4]<<8);
		//	E2prom_Write_Byte(EEP_SEC + StartAdd - MODBUS_SEC, Time.all[StartAdd - MODBUS_SEC]);
			Set_Clock(PCF_SEC + StartAdd - MODBUS_TIMER_ADDRESS, RTC.all[StartAdd - MODBUS_TIMER_ADDRESS]);
		}		

  // SUB INFO BLOCK
  		else if(StartAdd  >= MODBUS_SUB_INFO_FIRST && StartAdd  <= MODBUS_SUB_INFO_LAST)
		{
			U8_T index;
			U8_T reg;
			index = (StartAdd - MODBUS_SUB_INFO_FIRST) / Tst_reg_num;
			reg = (StartAdd - MODBUS_SUB_INFO_FIRST) % Tst_reg_num;
			if(reg == TST_OCCUPIED)
			{
				U8_T reg_id;
				reg_id = Tst_Register[reg][tst_info[index].type];
				write_parameters_to_nodes(sub_addr[index],reg_id,pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4]<<8));
				tst_info[index].occupied = pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4]<<8);
			} 
			else	if(reg == TST_COOL_SETPOINT)
			{
				U8_T reg_id;
				reg_id = Tst_Register[reg][tst_info[index].type];
				write_parameters_to_nodes(sub_addr[index],reg_id,pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4]<<8));
				tst_info[index].cool_setpoint = pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4]<<8);
			}
			else	if(reg == TST_HEAT_SETPOINT)
			{
				U8_T reg_id;
				reg_id = Tst_Register[reg][tst_info[index].type];
				write_parameters_to_nodes(sub_addr[index],reg_id,pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4]<<8));
				tst_info[index].heat_setpoint = pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4]<<8);
			}
			else	if(reg == TST_ROOM_SETPOINT)
			{
				U8_T reg_id;
				reg_id = Tst_Register[reg][tst_info[index].type];
				write_parameters_to_nodes(sub_addr[index],reg_id,pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4]<<8));
				tst_info[index].setpoint = pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4]<<8);
			}
			else	if(reg == TST_NIGHT_HEAT_DB)
			{
				U8_T reg_id;
				reg_id = Tst_Register[reg][tst_info[index].type];
				write_parameters_to_nodes(sub_addr[index],reg_id,pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4]<<8));
				tst_info[index].night_heat_db = pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4]<<8);
			}
			else	if(reg == TST_NIGHT_COOL_DB)
			{
				U8_T reg_id;
				reg_id = Tst_Register[reg][tst_info[index].type];
				write_parameters_to_nodes(sub_addr[index],reg_id,pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4]<<8));
				tst_info[index].night_cool_db = pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4]<<8);
			}
			else	if(reg == TST_NIGHT_HEAT_SP)
			{
				U8_T reg_id;
				reg_id = Tst_Register[reg][tst_info[index].type];
				write_parameters_to_nodes(sub_addr[index],reg_id,pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4]<<8));
				tst_info[index].night_heat_sp = pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4]<<8);
			}
			else	if(reg == TST_NIGHT_COOL_SP)
			{
				U8_T reg_id;
				reg_id = Tst_Register[reg][tst_info[index].type];
				write_parameters_to_nodes(sub_addr[index],reg_id,pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4]<<8));
				tst_info[index].night_cool_db = pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4]<<8);
			}		
		}		
	}
	else if(cmd == MULTIPLE_WRITE)
	{
		if(type == SERIAL || type == USB)   // TBD: need change
		{	
			U16_T crc_val;
			U8_T i;
			U8_T temp_buf[8];
			memcpy(temp_buf,pData,6);
			crc_val = crc16(temp_buf,6);
			temp_buf[6]	= crc_val >> 8;
			temp_buf[7]	= (U8_T)crc_val;
			if(type == SERIAL)
			{
				uart_init_send_com(UART_MAIN);				
				for(i = 0;i < 8;i++)
				{
					uart_send_byte(sendbuf[i],UART_MAIN);
				}
			}
			else
			{
				 memcpy(UpBuf,sendbuf,8); 
				 UpIndex = 0;
				 ENDP2_NEED_UP_FLAG = 1;
				 UpCtr = 8;			
			}

		}
		else
		{
		//	SetTransactionId(6 + UIP_HEAD);
		
			sendbuf[0] = pData[0];			//	TransID
			sendbuf[1] = pData[1];	
			sendbuf[2] = 0;			//	ProtoID
			sendbuf[3] = 0;
			sendbuf[4] = (3 + UIP_HEAD) >> 8;	//	Len
			sendbuf[5] = (U8_T)(3 + UIP_HEAD) ;

			for (loop = 0;loop < 6;loop++)
			{
				sendbuf[HeadLen + loop] = pData[HeadLen + loop];	
			}
			flagLED_ether_tx = 1;
			TCPIP_TcpSend(pHttpConn->TcpSocket, sendbuf, 6 + UIP_HEAD, TCPIP_SEND_NOT_FINAL);  
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
		else if(StartAdd  >= MODBUS_OUTPUT_FIRST && StartAdd  <= MODBUS_OUTPUT_LAST)
		{
			if((StartAdd - MODBUS_OUTPUT_FIRST) % sizeof(Str_out_point) == 0)
		 	{
				U8_T i;
			 	i = (StartAdd - MODBUS_OUTPUT_FIRST) / sizeof(Str_out_point);
				memcpy(&outputs[i],&pData[HeadLen + 7],sizeof(Str_out_point)); 	
			}
		}
		else if(StartAdd  >= MODBUS_INPUT_FIRST && StartAdd  <= MODBUS_INPUT_LAST)
		{
			if((StartAdd - MODBUS_INPUT_FIRST) % sizeof(Str_in_point) == 0)
		 	{
				U8_T i;
			 	i = (StartAdd - MODBUS_INPUT_FIRST) / sizeof(Str_in_point);
				memcpy(&inputs[i],&pData[HeadLen + 7],sizeof(Str_in_point)); 	
			}
		}
		else if(StartAdd  >= MODBUS_WR_FIRST && StartAdd  <= MODBUS_WR_LAST)
		{
			if((StartAdd - MODBUS_WR_FIRST) % sizeof(Str_weekly_routine_point) == 0)
		 	{
				U8_T i;
			 	i = (StartAdd - MODBUS_WR_FIRST) / sizeof(Str_weekly_routine_point);
				memcpy(&weekly_routines[i],&pData[HeadLen + 7],sizeof(Str_weekly_routine_point)); 	
			}
		}
		else if(StartAdd  >= MODBUS_AR_FIRST && StartAdd  <= MODBUS_AR_LAST)
		{
		 	if((StartAdd - MODBUS_AR_FIRST) % sizeof(Str_annual_routine_point) == 0)
		 	{
				U8_T i;
			 	i = (StartAdd - MODBUS_AR_FIRST) / sizeof(Str_annual_routine_point);
				memcpy(&annual_routines[i],&pData[HeadLen + 7],sizeof(Str_annual_routine_point)); 	
			}
		}
		else if(StartAdd  >= MODBUS_WR_TIME_FIRST && StartAdd  <= MODBUS_WR_TIME_LAST)
		{
			if((StartAdd - MODBUS_WR_TIME_FIRST) % (sizeof(Wr_one_day) * MAX_SCHEDULES_PER_WEEK) == 0)
		 	{
				U8_T i;
			 	i = (StartAdd - MODBUS_WR_TIME_FIRST) / (sizeof(Wr_one_day) * MAX_SCHEDULES_PER_WEEK);
				memcpy(&wr_times[i],&pData[HeadLen + 7],(sizeof(Wr_one_day) * MAX_SCHEDULES_PER_WEEK)); 	
			}
		}
		else if(StartAdd  >= MODBUS_AR_TIME_FIRST && StartAdd  <= MODBUS_AR_TIME_LAST)
		{
			if((StartAdd - MODBUS_AR_TIME_FIRST) % AR_DATES_SIZE == 0)
		 	{
				U8_T i;
			 	i = ((StartAdd - MODBUS_AR_TIME_FIRST) / AR_DATES_SIZE);
				memcpy(&ar_dates[i],&pData[HeadLen + 7],AR_DATES_SIZE); 	
			}
		}
  	


	}
	else if(cmd == CHECKONLINE)
	{
		if(type == SERIAL || type == USB)
		{
			U8_T temp_buf[9];
			U8_T crc_val;


			temp_buf[0] = sub1_data_buffer[0];
			temp_buf[1] = sub1_data_buffer[1];
			temp_buf[2] = Modbus.address;
			temp_buf[3] = Modbus.serialNum[0];
			temp_buf[4] = Modbus.serialNum[1];
			temp_buf[5] = Modbus.serialNum[2];
			temp_buf[6] = Modbus.serialNum[3];			
			
			crc_val = crc16(temp_buf,7);

			temp_buf[7] = crc_val >> 8;
			temp_buf[8] = (U8_T)crc_val;

			if(type == SERIAL)
			{
				uart_init_send_com(UART_MAIN);
				uart_send_string(temp_buf,9,UART_MAIN);
			}
			else
			{
				memcpy(UpBuf,sendbuf,9); 
				UpIndex = 0;
				ENDP2_NEED_UP_FLAG = 1;
				UpCtr = 9;		
			}

		}
		else
		{
			U16_T far crc_val;
			U8_T tempbuf[6];
			U8_T i;
			TcpSocket_ME = pHttpConn->TcpSocket;

		    transaction_id = ((U16_T)pData[0] << 8) | pData[1];
			transaction_num	= 2 * pData[UIP_HEAD + 5] + 3;

		//	TcpIp_Scan = 1;
		    //uart1_init_crc16();		 

 			for(i = 0;i < 4;i++) 
            { 
		        tempbuf[i] = pData[UIP_HEAD+i];									   
            }
			crc_val = crc16(tempbuf,4);		
			tempbuf[4] = crc_val >> 8;
			tempbuf[5] = (U8_T)crc_val;

//			uart_init_send_com(UART1);
//
//			for(i = 0;i < 6;i++)
//			{
//				uart0_send_byte(tempbuf[i]); 
//			}
//			sub_rece_size = 10;
//			uart2_send_string(tempbuf,6);	 // UART2
		}
	}
}
