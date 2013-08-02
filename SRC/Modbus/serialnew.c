#include "main.h"    
#include "serial.h"
#include "schedule.h" 
#include "stdlib.h" 


extern xQueueHandle	xSubRevQueue;
extern U8_T flag_send_scan_table;
extern bit b_Master_Slave_TCP;

void responseCmd(U8_T type,U8_T* pData,HTTP_SERVER_CONN * pHttpConn); 

	U8_T far protocal = TCP_IP;
	U8_T far demo_enable = 0;
	U8_T far IspVer;
	U8_T far serialNum[4];
//	U8_T firmwareVer[2];
	U8_T far Modbus_address;
	U8_T far product_model;
	U8_T far hardRev;
	U8_T far baudrate;
//	U8_T PICversion;
//	U8_T update;
	U8_T far unit;
	U8_T far sub_addr[8];
	U8_T far switch_tstat_val;
//	U8_T UPDATE_STATUS;
	U8_T far BASE_ADRESS;
	U8_T far TCP_TYPE;   /* 0 -- DHCP, 1-- STATIC */
	U8_T far Mac_Addr[6];
	U8_T far IP_Addr[4];
	U8_T far SUBNET[4];
	U8_T far GETWAY[4];


	U16_T  far AI_Value[10] = {0};	
	U8_T far Input_Range[10];
	U8_T far Input_Filter[10];	
	U16_T far Input_CAL[10];

	S8_T far menu_name[36][14];
	U8_T far dis_temp_num;
	U8_T far dis_temp_interval;
	U8_T far dis_temp_seq[10];

	U8_T far DI1_Value;
	U8_T far DI2_Value;
	U8_T far DI_Type[8]; 		/* choose DI1 or DI2 control DO, 0 -- DI1, 1 -- DI2 */

	U16_T far DO_Value;  		// control relay
	U8_T far DO_SoftSwitch;  	// software switch logic
	U8_T far Priority;			// Zone1 has priority,
	U16_T far count_priority;   // count priority timer 
//	U8_T Master;
	U16_T far DI_Enable;
	U16_T far AI_Enable;
	U16_T far DInputAM;  // digital input 
	U16_T far OuputAM;
	U16_T far AInputAM;  // input 1 - 10 sensor
    U8_T far sn_write_flag;
	UN_Time RTC;
	U8_T far update_status;

	U8_T far sub_no = 0;
	U8_T far switch_sub_no = 0;
	U8_T far switch_sub_bit = 0;
	U8_T far heat_no = 0;
	U8_T far cool_no = 0;

//STR_MODBUS  far Modbus;	
extern U16_T far Test[50];

//Added by andy sun
#define DATA_TO_FIRMWARE  1000
U8_T temp_state = NONE_ID;

bit mac_address_write_enable = FALSE;
U8_T flag_update;  

//extern U8_T pic_type; // chelsea
signed int RangeConverter(U8_T function, signed int para,U8_T i,U16_T cal);

xTaskHandle far Handle_MainSerial;
xTaskHandle far Handle_SubSerial;


U16_T main_rece_count, sub_rece_count;
U8_T main_rece_size = 0;
U16_T sub_rece_size = 0;
U8_T main_dealwithTag;
U8_T sub_dealwithTag;
U8_T far main_serial_receive_timeout_count, sub_serial_receive_timeout_count;
U8_T main_transmit_finished, sub_transmit_finished;
U8_T far main_data_buffer[MAIN_BUF_LEN];
U8_T far sub_data_buffer[SUB_BUF_LEN];

U16_T far wait = 0;


U8_T far sub_net_state = SUB_NET_SPARE;
U8_T far sub_response_in = FALSE;


U8_T SERIAL_RECEIVE_TIMEOUT;

bit flag_transimit_from_serial = 0;  // flag for CM5 or Tstat, if CM5 b_Master_Slave is 0, if Tstat b_Master_Slave is 1
U8_T SNWriteflag;
bit flag_control_by_button = 0;  // control Tstat using T3000 or CM5's button

U8_T far rand_read_ten_count;

unsigned short TransID = 1;

U8_T far flag_uart = 0;


U8_T comm_tstat = SPARE;	     // 0: T3000 control TSTAT, 1: CM5 control TSTAT directly
U8_T datalen = 0;
//U8_T  address = 0;

void main_dealwithData( void );
void subdealwithData( void );
//void subdealwithData( void );


void vStartMainSerialTasks( U8_T uxPriority)
{
	sub_no = 0;
	memset(sub_addr,0,8);
//	sub_addr[0] = 199;
//	sub_addr[1] = 93;
//	memset(menu_name,0,MAX_NAME * NAME_SIZE);
	initSerial();
//	memset(&Modbus,0,sizeof(STR_MODBUS));
	sTaskCreate( main_dealwithData, "mainserailtask", MainSerialSTACK_SIZE, NULL, uxPriority, &Handle_MainSerial );
}


void initSerial(void)
{
	UART1_TXEN = RECEIVE;
	main_rece_count = 0;
	main_dealwithTag = 0;
// 	check if during ISP mode if the address has been changed
 	E2prom_Read_Byte(EEP_ADDRESS,& Modbus_address);

	// if it has not been changed, check the flash memory
	if( ( Modbus_address == 255) || ( Modbus_address == 0) )
	{
	//	if(E2prom_Read_Byte(EEP_ADDRESS, & address) )
		{
			 Modbus_address = 254;
			E2prom_Write_Byte(EEP_ADDRESS,  Modbus_address);
		}

	}
	else
    {
		E2prom_Write_Byte(EEP_ADDRESS,  Modbus_address);

	}

	// if data is blank, means first Time programming, thus put as default
	// Added by RL 02/11/04
	if( Modbus_address == 0 ||  Modbus_address == 255 ) 
		 Modbus_address = 254;



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

void sub_send_byte(U8_T buffer, U8_T crc);	

//init crc 
void main_init_crc16(void)
{
	MainCRClo = 0xff;
	MainCRChi = 0xff;
}

void sub_init_crc16(void)
{
	SubCRClo = 0xff;
	SubCRChi = 0xff;
}

void ethernet_init_crc16(void)
{
	EthernetCRClo = 0xff;
	EthernetCRChi = 0xff;
}

//calculate crc with one byte
void main_crc16_byte(U8_T ch)
{
	U8_T uIndex;
	uIndex = MainCRChi^ch; // calculate the CRC 
	MainCRChi = MainCRClo^auchCRCHi[uIndex];
	MainCRClo = auchCRCLo[uIndex];
}

void sub_crc16_byte(U8_T ch)
{
	U8_T uIndex;
	uIndex = SubCRChi^ch; // calculate the CRC 
	SubCRChi = SubCRClo^auchCRCHi[uIndex];
	SubCRClo = auchCRCLo[uIndex];
}

void ethernet_crc16_byte(U8_T ch)
{
	U8_T uIndex;
	uIndex = EthernetCRChi^ch; // calculate the CRC 
	EthernetCRChi = EthernetCRClo^auchCRCHi[uIndex];
	EthernetCRClo = auchCRCLo[uIndex];
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







#if 1
U8_T wait_SubSerial(U16_T delay)
{
//	wait = 0;
	/* delay is to avoid dead cycle */
	while((sub_dealwithTag == 0) && (wait < delay)) 
	{
		wait++;
		vTaskDelay(1);					
	}
//	cQueueSend(xSubRevQueue, &sub_data_buffer, 0); 
	if(sub_dealwithTag == VALID_PACKET) return 1;   
	else return 0;
}

#endif

void sub_serial_restart(void)
{
	UART0_TXEN = RECEIVE;
//	DELAY_Us(10);
	sub_rece_count = 0;
	sub_dealwithTag = 0;
//	sub_rece_size = 8;	
}

void sub_net_init(void)
{
	UART_Init(0);
	sub_serial_restart();
//	sub_respond_state = SPARE;
}

void sub_init_send_com(void)
{
	UART_Init(0);
	sub_transmit_finished = 0;
	UART0_TXEN = SEND;
	DELAY_Us(10);
}

void sub_send_byte(U8_T buffer, U8_T crc)
{
	U8_T count = 0;
	SBUF0 = buffer;
	sub_transmit_finished = 0;
	while(!sub_transmit_finished && count < 20)
	{
		DELAY_Us(1);
		count++;
	}
	if(crc == CRC_NO)
	{
		sub_crc16_byte(buffer);
	}
}

void sub_send_string(U8_T *bufs,char len)
{
	char data i;
	U16_T crc_val;
	U8_T far buf[20];
	U16_T count;

	crc_val = crc16(bufs,len);
	memcpy(buf,bufs,len);
	buf[len] =  crc_val >> 8;
	buf[len + 1] = 	crc_val;

	for(i = 0;i < len + 2;i++)
	{
		SBUF0 = buf[i];
		sub_transmit_finished = 0;
		count = 0;
		while (!sub_transmit_finished && count < 20)
		{
			DELAY_Us(1);
			count++;
		}
		if(sub_transmit_finished == 0)	  Test[43]++;
	
	}
	
}
//------------------------serialport ----------------------------------
//	serial port interrupt , deal with all action about serial port. include receive data and 
//		send data and deal with interal.


void handle_Modbus_RX0(void) interrupt 4
{
	U16_T crc_val;

	if(RI0 == 1)
	{
		RI0 = 0;

		if(sub_rece_count < SUB_BUF_LEN)
			sub_data_buffer[sub_rece_count++] = SBUF0;

		sub_serial_receive_timeout_count = 5;

		if(sub_data_buffer[0] == 0)  {	sub_rece_count--; }
	//	Test[41]= sub_rece_count;
		if(sub_rece_count >= sub_rece_size)
		{
			sub_serial_receive_timeout_count = 0;
		//	sub_rece_count = 0;
		}
  	}
	else if(TI0 == 1)
	{
		TI0 = 0;
		sub_transmit_finished = 1;
	}
	return;

}




	
/*********************************************\
*	MAIN NET CODE START
\*********************************************/
void main_serial_restart(void)
{
	UART1_TXEN = RECEIVE;
	main_rece_count = 0;
	main_dealwithTag = 0;
}

void main_net_init(void)
{
//	UART_Init(1);
	SERIAL_RECEIVE_TIMEOUT = 2;
	main_serial_restart();
}

void main_init_send_com(void)
{
	main_transmit_finished = 0;
	UART1_TXEN = SEND;
//	DELAY_Us(100);
}

void main_send_byte(U8_T buffer, U8_T crc)
{
	SBUF1 = buffer;
	main_transmit_finished = 0;
	while(!main_transmit_finished);
	if(crc == CRC_NO)
	{
		main_crc16_byte(buffer);
	}
}

void mstp_int_handler(void);

void modbus_int_handler(void)
{
	if(RI1 == 1)
	{
		RI1 = 0;		
		if(main_rece_count < MAIN_BUF_LEN)
			main_data_buffer[main_rece_count++] = SBUF1;
		else
			main_serial_restart();
		
		main_serial_receive_timeout_count = 2;//SERIAL_RECEIVE_TIMEOUT;

		if(main_rece_count == 1)
		{
			main_rece_size = 8;
			main_serial_receive_timeout_count = 2;//SERIAL_RECEIVE_TIMEOUT;
		}
		else if(main_rece_count == 4)
		{
			if((((U16_T)(main_data_buffer[2] << 8) + main_data_buffer[3]) == 0x0a) && (main_data_buffer[1] == WRITE_VARIABLES))
			{
				main_rece_size = DATABUFLEN_SCAN;
				main_serial_receive_timeout_count = 2;//SERIAL_RECEIVE_TIMEOUT;	
			}
		}
		else if(main_rece_count == 7)
		{
			if(main_data_buffer[1] == MULTIPLE_WRITE)
			{
				main_rece_size = main_data_buffer[6] + 9;
				main_serial_receive_timeout_count = main_data_buffer[6] + 8;//from 4 to 8 to see if can resolve multiple problem
			}
		}
//		else if(((main_rece_count == main_rece_size) && ((main_data_buffer[1] == READ_VARIABLES) || (main_data_buffer[1] == WRITE_VARIABLES) || (main_data_buffer[1] == MULTIPLE_WRITE))) || ((main_rece_count == 6) && (main_data_buffer[1] == CHECKONLINE)))
		else if(main_rece_count == main_rece_size)
		{	 
		
			main_serial_receive_timeout_count = 0;
			main_dealwithTag = VALID_PACKET;
		}
	}
	else if(TI1 == 1)
	{
		TI1 = 0;
		main_transmit_finished = 1;
	}

	return;
}
#if 1
void handle_Modbus_RX1(void) interrupt 6
{
	if(protocal == BAC_MSTP)
	{
		mstp_int_handler();
	}
	else
	{
		modbus_int_handler();
	}

}
#endif

// ------------------------dealwithdata -------------------------
// the routine dealwith data ,it has three steps.
// the 3 step is : 1 prepare to send data and init crc for next Tim
//				   2 dealwith interal
//                 3 organize the data of sending, and send data.

void main_dealwithData(void)
{
	U16_T far address;
	U8_T frametype;
	portTickType xDelayPeriod = ( portTickType ) 50 / portTICK_RATE_MS;
	main_net_init();
	for( ; ;)
	{ 	
		vTaskDelay(xDelayPeriod);


		if(main_dealwithTag == VALID_PACKET)
		{					
			address = ((U16_T)main_data_buffer[2] << 8) + main_data_buffer[3];
			if (checkData(address))
			{	
				U16_T i;
			   	flag_transimit_from_serial = 0; 
								
				for(i = 0;i <  sub_no && !flag_transimit_from_serial;i++)
				{
					if(main_data_buffer[0] == sub_addr[i])	
						flag_transimit_from_serial = 1;  // Tstat is sub device
					else
						flag_transimit_from_serial = 0;  // the current device is CM5
				} 
				
				if(!flag_transimit_from_serial)	
				{
					if(main_data_buffer[0] ==  Modbus_address || main_data_buffer[0] == 255)
					{
						main_init_send_com();
			
						main_init_crc16();
			
						main_responseData(address);
					}
				}
				else
				{  
					sub_init_send_com();
					if(main_data_buffer[1] == READ_VARIABLES) 
					{
						sub_rece_size = main_data_buffer[5] * 2 + 5;

						for(i = 0;i < 8;i++)
						{
							sub_send_byte(main_data_buffer[i],CRC_YES);
						}			
						
					}
					else if(main_data_buffer[1] == WRITE_VARIABLES) 
					{
						sub_rece_size = 8;
						for(i = 0;i < 8;i++)
						{
							sub_send_byte(main_data_buffer[i],CRC_YES);
						}
					}
					else if(main_data_buffer[1] == MULTIPLE_WRITE)  // MULTIPLE_WRITE
					{				
						sub_rece_size = 8;
						for(i = 0;i < 137;i++)
						{
							sub_send_byte(main_data_buffer[i],CRC_YES);
						}	
					}
					
					sub_serial_restart();  

					wait_SubSerial(50);
					//if(cQueueReceive( xSubRevQueue, &sub_data_buffer, 0))
					{
						main_init_send_com();	 // enable COM1 send 
						// send data to MBPOLL or T3000 
						for(i = 0;i < sub_rece_size;i++)					
							main_send_byte(sub_data_buffer[i],CRC_YES);
					} 
				} 
			}  
							
		// Restart the serial receive.
			main_serial_restart(); 		
		}
		#if 0
		if(main_dealwithTag == VALID_PACKET)
		{					
			address = ((U16_T)main_data_buffer[2] << 8) + main_data_buffer[3];
			if(checkData(address))
			{	
				U16_T i; 			
				
				if(main_data_buffer[0] == Modbus_address || main_data_buffer[0] == 255)
				{
					flag_transimit_from_serial = 0;
					main_init_send_com();
		
					main_init_crc16();
		
					main_responseData(address);

					continue;
				}
				else 
				{  
					for(i = 0;i < sub_no;i++)
					{
						if(main_data_buffer[0] == sub_addr[i])
						{
							flag_transimit_from_serial = 1;
							sub_init_send_com();

							if(main_data_buffer[1] == READ_VARIABLES) 
							{
								sub_rece_size = main_data_buffer[5] * 2 + 5;
		
								for(i = 0;i < 8;i++)
								{
									sub_send_byte(main_data_buffer[i],CRC_YES);
								}			
								
							}
							else if(main_data_buffer[1] == WRITE_VARIABLES) 
							{
								sub_rece_size = 8;
								for(i = 0;i < 8;i++)
								{
									sub_send_byte(main_data_buffer[i],CRC_YES);
								}
							}
							else if(main_data_buffer[1] == MULTIPLE_WRITE)  // MULTIPLE_WRITE
							{				
								sub_rece_size = 8;
								for(i = 0;i < 137;i++)
								{
									sub_send_byte(main_data_buffer[i],CRC_YES);
								}	
							}							
							sub_serial_restart();  
						//	source = PC_SERIAL;
						//	flag_transimit_from_serial = 1;
						//	Test[47]++;
						    wait_SubSerial(100);
							main_init_send_com();	 // enable COM1 send 
							// send data to MBPOLL or T3000 
							for(i = 0;i < sub_rece_size;i++)					
								main_send_byte(sub_data_buffer[i],CRC_YES);	
						
							flag_transimit_from_serial = 0;	

							continue;
						}
					}
				} 
			}  
							
		// Restart the serial receive.
			main_serial_restart(); 		
		}
		#endif 
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
			if(main_data_buffer[6] != serialNum[0]) 
			return 0;
			if(main_data_buffer[7] != serialNum[1]) 
			return 0;
			if(main_data_buffer[8] != serialNum[2])  
			return 0;
			if(main_data_buffer[9] != serialNum[3]) 
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
	U8_T far temp;
	U8_T far tempbit;
	U8_T far temp_number;
	U8_T far temp_address;
	U8_T far send_buffer;
	U8_T far tempbuf[100];  /* convert schedule structure */
	U8_T far update_flash;

	if(type == MODBUS)  // modbus packet
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
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  serialNum[StartAdd + loop - MODBUS_SERIALNUMBER_LOWORD];
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
				sendbuf[HeadLen + 3 + loop * 2 + 1] = HW_REV;
			}
			else if(StartAdd + loop ==  MODBUS_ADDRESS)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Modbus_address;		
			}
			else if(StartAdd + loop == MODBUS_PRODUCT_MODEL)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] = PRODUCT_CM5;
			}
			else if(StartAdd + loop == MODBUS_ISP_VER)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  IspVer;
			}
			else if(StartAdd + loop == MODBUS_PIC)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] = PIC_REV;
			}
			else if(StartAdd + loop == MODBUS_BAUDRATE)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  baudrate;
			}
			else if(StartAdd + loop == MODBUS_UPDATE_STATUS)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  update_status;
			}
			else if(StartAdd + loop == MODBUS_DEMO_ENABLE)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  demo_enable;
			}
			else if(StartAdd + loop == MODBUS_PROTOCAL)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  protocal;
			}
			/*else if(StartAdd + loop == MODBUS_RESET_PARAMETER)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  protocal;
			}*/			
			else if(StartAdd + loop >= MODBUS_MAC_1 && StartAdd + loop <= MODBUS_MAC_6)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Mac_Addr[StartAdd + loop - MODBUS_MAC_1];
			}
			else if(StartAdd + loop == MODBUS_TCP_TYPE)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  TCP_TYPE;
			}
			else if(StartAdd + loop >= MODBUS_IP_1 && StartAdd + loop <= MODBUS_IP_4)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  IP_Addr[StartAdd + loop - MODBUS_IP_1];
			}
			else if(StartAdd + loop >= MODBUS_SUBNET_1 && StartAdd + loop <= MODBUS_SUBNET_4)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  SUBNET[StartAdd + loop - MODBUS_SUBNET_1];
			}
			else if(StartAdd + loop >= MODBUS_GETWAY_1 && StartAdd + loop <= MODBUS_GETWAY_4)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  GETWAY[StartAdd + loop - MODBUS_GETWAY_1];
			} 
			else if(StartAdd + loop == MODBUS_TCP_LISTEN_PORT)
			{
				sendbuf[HeadLen + 3 + loop * 2] = HTTP_SERVER_PORT >> 8;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  (U8_T)HTTP_SERVER_PORT;
			}
		
#if 1		
			/* the following is for sub-tstat 
			register 5670 - 5758
			register 160 - 200
			*/
/* START SUB TSTAT */

			else if(StartAdd + loop >= MODBUS_SUBADDR_FIRST && StartAdd + loop <= MODBUS_SUBADDR_LAST)
			{  // 18 - 25
		
				sendbuf[HeadLen + 3 + loop * 2] = 0;	
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  sub_addr[StartAdd + loop  - MODBUS_SUBADDR_FIRST];
			
			}
			else if(StartAdd + loop == MODBUS_OCCUPIED)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = tstat_occupied;
			}
			else if(StartAdd + loop >= MODBUS_ROOM_SETPOINT_FIRST && StartAdd + loop <= MODBUS_ROOM_SETPOINT_LAST)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				
				if(StartAdd + loop  - MODBUS_ROOM_SETPOINT_FIRST <  sub_no)	
					sendbuf[HeadLen + 3 + loop * 2 + 1] = tstat_setpoint[StartAdd + loop  - MODBUS_ROOM_SETPOINT_FIRST];
				else 
					sendbuf[HeadLen + 3 + loop * 2 + 1] = 0;
			}
			else if(StartAdd + loop >= MODBUS_COOL_SETPOINT_FIRST && StartAdd + loop <= MODBUS_COOL_SETPOINT_LAST)
			{
				
				if(StartAdd + loop  - MODBUS_COOL_SETPOINT_FIRST <  sub_no)	
				{
					sendbuf[HeadLen + 3 + loop * 2] = tstat_cool_setpoint[StartAdd + loop  - MODBUS_COOL_SETPOINT_FIRST] >> 8;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)tstat_cool_setpoint[StartAdd + loop  - MODBUS_COOL_SETPOINT_FIRST];
				}
				else 
				{
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = 0;
				}
			}
			else if(StartAdd + loop >= MODBUS_HEAT_SETPOINT_FIRST && StartAdd + loop <= MODBUS_HEAT_SETPOINT_LAST)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				if(StartAdd + loop  - MODBUS_HEAT_SETPOINT_FIRST <  sub_no)	
					sendbuf[HeadLen + 3 + loop * 2 + 1] = tstat_heat_setpoint[StartAdd + loop  - MODBUS_HEAT_SETPOINT_FIRST];
				else 
					sendbuf[HeadLen + 3 + loop * 2 + 1] = 0;
			}
			else if(StartAdd + loop >= MODBUS_ROOM_TEM_FIRST && StartAdd + loop <= MODBUS_ROOM_TEM_LAST)
			{
				
				if(StartAdd + loop  - MODBUS_ROOM_TEM_FIRST <  sub_no)
				{	
					sendbuf[HeadLen + 3 + loop * 2] = tstat_temperature[StartAdd + loop  - MODBUS_ROOM_TEM_FIRST] >> 8;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)tstat_temperature[StartAdd + loop  - MODBUS_ROOM_TEM_FIRST];
				}
				else 
				{
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = 0;
				}
			}
			else if(StartAdd + loop >= MODBUS_MODE_FIRST && StartAdd + loop <= MODBUS_MODE_LAST)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0; 				
				if(StartAdd + loop  - MODBUS_MODE_FIRST <  sub_no)	
					sendbuf[HeadLen + 3 + loop * 2 + 1] = tstat_mode[StartAdd + loop  - MODBUS_MODE_FIRST];
				else 
					sendbuf[HeadLen + 3 + loop * 2 + 1] = 0;
			}
			else if(StartAdd + loop >= MODBUS_OUTPUT_STATE_FIRST && StartAdd + loop <= MODBUS_OUTPUT_STATE_LAST)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;				
				if(StartAdd + loop  - MODBUS_OUTPUT_STATE_FIRST <  sub_no)	
					sendbuf[HeadLen + 3 + loop * 2 + 1] = tstat_output_state[StartAdd + loop  - MODBUS_OUTPUT_STATE_FIRST];
				else 
					sendbuf[HeadLen + 3 + loop * 2 + 1] = 0;
			}
			else if(StartAdd + loop >= MODBUS_NIGHT_HEAT_DB_FIRST && StartAdd + loop <= MODBUS_NIGHT_HEAT_DB_LAST)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				if(StartAdd + loop  - MODBUS_NIGHT_HEAT_DB_FIRST <  sub_no)	
					sendbuf[HeadLen + 3 + loop * 2 + 1] = tstat_night_heat_db[StartAdd + loop  - MODBUS_NIGHT_HEAT_DB_FIRST];
				else 
					sendbuf[HeadLen + 3 + loop * 2 + 1] = 0;
			}
			else if(StartAdd + loop >= MODBUS_NIGHT_COOL_DB_FIRST && StartAdd + loop <= MODBUS_NIGHT_COOL_DB_LAST)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				if(StartAdd + loop  - MODBUS_NIGHT_COOL_DB_FIRST <  sub_no)	
					sendbuf[HeadLen + 3 + loop * 2 + 1] = tstat_night_cool_db[StartAdd + loop  - MODBUS_NIGHT_COOL_DB_FIRST];
				else 
					sendbuf[HeadLen + 3 + loop * 2 + 1] = 0;
			}
			else if(StartAdd + loop >= MODBUS_NIGHT_HEAT_SP_FIRST && StartAdd + loop <= MODBUS_NIGHT_HEAT_SP_LAST)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				if(StartAdd + loop  - MODBUS_NIGHT_HEAT_SP_FIRST <  sub_no)	
					sendbuf[HeadLen + 3 + loop * 2 + 1] = tstat_night_heat_sp[StartAdd + loop  - MODBUS_NIGHT_HEAT_SP_FIRST];
				else 
					sendbuf[HeadLen + 3 + loop * 2 + 1] = 0;
			}
			else if(StartAdd + loop >= MODBUS_NIGHT_COOL_SP_FIRST && StartAdd + loop <= MODBUS_NIGHT_COOL_SP_LAST)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				if(StartAdd + loop  - MODBUS_NIGHT_COOL_SP_FIRST <  sub_no)	
					sendbuf[HeadLen + 3 + loop * 2 + 1] = tstat_night_cool_sp[StartAdd + loop  - MODBUS_NIGHT_COOL_SP_FIRST];
				else 
					sendbuf[HeadLen + 3 + loop * 2 + 1] = 0;
			}
			else if(StartAdd + loop >= MODBUS_PRODUCT_MODEL_FIRST && StartAdd + loop <= MODBUS_PRODUCT_MODEL_LAST)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				if(StartAdd + loop  - MODBUS_PRODUCT_MODEL_FIRST <  sub_no)	
					sendbuf[HeadLen + 3 + loop * 2 + 1] = tstat_product_model[StartAdd + loop  - MODBUS_PRODUCT_MODEL_FIRST];
				else 
					sendbuf[HeadLen + 3 + loop * 2 + 1] = 0;
			}
			else if(StartAdd + loop >= MODBUS_OVER_RIDE_FIRST && StartAdd + loop <= MODBUS_OVER_RIDE_LAST)
			{	
				if(StartAdd + loop  - MODBUS_OVER_RIDE_FIRST <  sub_no)	
				{			
					sendbuf[HeadLen + 3 + loop * 2] = tstat_over_ride[StartAdd + loop  - MODBUS_OVER_RIDE_FIRST] >> 8;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)tstat_over_ride[StartAdd + loop  - MODBUS_OVER_RIDE_FIRST];		
				}
				else 
				{
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = 0;
				}
			}
			else if(StartAdd + loop >= MODBUS_SERIAL_NUM_FIRST && StartAdd + loop <= MODBUS_SERIAL_NUM_LAST)  // have  4 * 8= 32 bytes 
			{
				
				if((StartAdd + loop  - MODBUS_SERIAL_NUM_FIRST) / 4 <  sub_no)	
				{	
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = tstat_serial_number[(StartAdd + loop  - MODBUS_SERIAL_NUM_FIRST)/4][(StartAdd + loop  - MODBUS_SERIAL_NUM_FIRST)%4];
				}
				else
				{	
					sendbuf[HeadLen + 3 + loop * 2] = 0;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = 0;
				}
			}
			
/*  END SUB TSTAT */
			else if(StartAdd + loop == MODBUS_TOTAL_NO)	
			{	
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  sub_no + switch_sub_no;
			}
			else if(StartAdd + loop == MODBUS_TOTAL_HEAT)	
			{	
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  0;//heat_no;
			}
			else if(StartAdd + loop == MODBUS_TOTAL_COOL)	
			{	
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  0;//cool_no;
			}
		/*	else if(StartAdd + loop == MODBUS_REFRESH_STATUS)
			{	
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = REFRESH_STATUS;
			} */
#endif
/******************************end for tstst resigter ************************************************/

			
			else if(StartAdd + loop >= MODBUS_AINPUT1 && StartAdd + loop <= MODBUS_AINPUT10)
			{
				U16_T tempValue;
				tempValue = AI_Value[StartAdd + loop - MODBUS_AINPUT1];

			//	if( AInputAM & (0x01 << (StartAdd + loop - MODBUS_AINPUT1)) == 0)  // auto
				{
				
				sendbuf[HeadLen + 3 + loop * 2] = (RangeConverter(Input_Range[StartAdd + loop - MODBUS_AINPUT1],
							tempValue, StartAdd + loop - MODBUS_AINPUT1, Input_CAL[StartAdd + loop - MODBUS_AINPUT1])>> 8) & 0xFF;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = RangeConverter(Input_Range[StartAdd + loop - MODBUS_AINPUT1],
							tempValue, StartAdd + loop - MODBUS_AINPUT1, Input_CAL[StartAdd + loop - MODBUS_AINPUT1]) & 0xFF;
				}
			/*	else
				{
					sendbuf[HeadLen + 3 + loop * 2] = tempValue >> 8;
					sendbuf[HeadLen + 3 + loop * 2 + 1] = tempValue;
				} */
			}
			else if(StartAdd + loop >= MODBUS_INPUT1_RANGE && StartAdd + loop <= MODBUS_INPUT10_RANGE)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Input_Range[StartAdd + loop - MODBUS_INPUT1_RANGE];
			}
			
			else if(StartAdd + loop >= MODBUS_INPUT1_FILTER && StartAdd + loop <= MODBUS_INPUT10_FILTER)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Input_Filter[StartAdd + loop - MODBUS_INPUT1_FILTER];
			}
			else if(StartAdd + loop >= MODBUS_INPUT1_CAL && StartAdd + loop <= MODBUS_INPUT10_CAL)
			{
				sendbuf[HeadLen + 3 + loop * 2] = (U8_T)( Input_CAL[StartAdd + loop - MODBUS_INPUT1_CAL] >> 8);
				sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)( Input_CAL[StartAdd + loop - MODBUS_INPUT1_CAL]);
			}
			else if(StartAdd + loop >= MODBUS_DI_TYPE1 && StartAdd + loop <= MODBUS_DI_TYPE8)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  DI_Type[StartAdd + loop - MODBUS_DI_TYPE1];
			}
			else if(StartAdd + loop == MODBUS_DI1)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  DI1_Value;
			}
			else if(StartAdd + loop == MODBUS_DI2)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  DI2_Value;
			}
			else if(StartAdd + loop == MODBUS_DOUTPUT)
			{
				sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(DO_Value >> 8);
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  (U8_T)DO_Value;
			}
			else if(StartAdd + loop == MODBUS_SWITCH)   /* switch logic */
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  DO_SoftSwitch;
			}
			else if(StartAdd + loop == MODBUS_PRIORTITY)   /* switch logic */
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  Priority;
			}
			else if(StartAdd + loop == MODBUS_COUNT_PRI)   /* count priority timer,count down */
			{
				sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(( Priority * 60 -  count_priority) >> 8);
				sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)( Priority * 60 -  count_priority);
			}
			else if(StartAdd + loop == MODBUS_DI_ENABLE)
			{
				sendbuf[HeadLen + 3 + loop * 2] = (U8_T)( DI_Enable >> 8);
				sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)( DI_Enable);
			}
			else if(StartAdd + loop == MODBUS_AI_ENABLE)
			{
				sendbuf[HeadLen + 3 + loop * 2] = (U8_T)( AI_Enable >> 8);
				sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)( AI_Enable);
			}
			else if(StartAdd + loop == MODBUS_DINPUT_AM)
			{
				sendbuf[HeadLen + 3 + loop * 2] = (U8_T)( DInputAM >> 8);
				sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)( DInputAM);
			}
			else if(StartAdd + loop == MODBUS_OUTPUT_AM)
			{
				sendbuf[HeadLen + 3 + loop * 2] = (U8_T)( OuputAM >> 8);
				sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)( OuputAM);
			}
			else if(StartAdd + loop == MODBUS_AINPUT_AM)
			{
				sendbuf[HeadLen + 3 + loop * 2] = (U8_T)( AInputAM >> 8);
				sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)( AInputAM);
			}
			else if(StartAdd + loop >= MODBUS_TIMER_ADDRESS && StartAdd + loop < MODBUS_TIMER_ADDRESS + 8)
			{	 
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] =  RTC.all[StartAdd + loop - MODBUS_TIMER_ADDRESS];
			}			
			else if( StartAdd + loop >= MODBUS_WR_DESCRIP_FIRST && StartAdd + loop < MODBUS_WR_DESCRIP_LAST)
			{
				temp_number = (StartAdd + loop - MODBUS_WR_DESCRIP_FIRST) / WR_DESCRIPTION_SIZE;
				temp_address = (StartAdd + loop - MODBUS_WR_DESCRIP_FIRST) % WR_DESCRIPTION_SIZE;

				send_buffer = WR_Roution[temp_number].UN.all[temp_address];
			    if(temp_address == WR_DESCRIPTION_SIZE - 1 && send_buffer != 0xff)
				{
					if((send_buffer & 0x80) == 0)
					{
						if(GetBit(temp_number,wr_state_index))
						send_buffer |= 0x40;
						else
						send_buffer &= 0xbf;
					}
					if(GetBit(temp_number,holiday1_state_index))
					send_buffer |= 0x20;
					else
					send_buffer &= 0xdf;
					if(GetBit(temp_number,holiday2_state_index))
					send_buffer |= 0x10;
					else
					send_buffer &= 0xef;
				}

				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = send_buffer;	
				
			}
			else if( StartAdd + loop >= MODBUS_AR_DESCRIP_FIRST && StartAdd + loop < MODBUS_AR_DESCRIP_LAST)
			{
				temp_number = (StartAdd + loop - MODBUS_AR_DESCRIP_FIRST) / AR_DESCRIPTION_SIZE;    // line
				temp_address = (StartAdd + loop - MODBUS_AR_DESCRIP_FIRST) % AR_DESCRIPTION_SIZE;
				send_buffer = AR_Roution[temp_number].UN.all[temp_address];

				if(temp_address == AR_DESCRIPTION_SIZE - 1)
				{
					if((send_buffer & 0x80) == 0)
					{
						if(GetBit(temp_number,ar_state_index))
						send_buffer |= 0x40;
						else
						send_buffer &= 0xbf;
					}
				}

				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = send_buffer;
				
			}
			else if( StartAdd + loop >= MODBUS_ID_FIRST && StartAdd + loop < MODBUS_ID_LAST)
			{
				temp_number = (StartAdd + loop - MODBUS_ID_FIRST) / ID_SIZE;
				temp_address = (StartAdd + loop - MODBUS_ID_FIRST) % ID_SIZE;
				send_buffer = ID_Config[temp_number].all[temp_address];
		

				// --- send first byte -------------			
			//	Send_Byte(send_buffer,CRC_NO);	
				
				// --- send second byte ------------ 			 
		
				if(temp_address == ID_SIZE - 1 && send_buffer != 0xff)
				{
					if((send_buffer & 0x80) == 0)
					{
						if(GetBit(temp_number,output_state_index))
						send_buffer |= 0x40;
						else
						send_buffer &= 0xbf;
					}
					if(GetBit(temp_number,schedual1_state_index))
					send_buffer |= 0x20;
					else
					send_buffer &= 0xdf;
					if(GetBit(temp_number,schedual2_state_index))
					send_buffer |= 0x10;
					else
					send_buffer &= 0xef;
				}

				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = send_buffer;	
			}
			else if( StartAdd + loop >= MODBUS_AR_TIME_FIRST && StartAdd + loop < MODBUS_WR_ONTIME_FIRST )
			{
				temp_number = (StartAdd + loop - MODBUS_AR_TIME_FIRST) / AR_TIME_SIZE;
				temp_address = (StartAdd + loop-MODBUS_AR_TIME_FIRST)%AR_TIME_SIZE;

				send_buffer = AR_Roution[temp_number].Time[temp_address];

				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = send_buffer;	
			}
			else if( StartAdd + loop >= MODBUS_WR_ONTIME_FIRST && StartAdd + loop < MODBUS_WR_OFFTIME_FIRST )
			{
				temp_number = 	(StartAdd + loop - MODBUS_WR_ONTIME_FIRST) / WR_TIME_SIZE;		
				temp_address = (StartAdd + loop - MODBUS_WR_ONTIME_FIRST) % WR_TIME_SIZE;
				send_buffer = WR_Roution[temp_number].OnTime[temp_address];

				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = send_buffer;
			}
			else if( StartAdd + loop >= MODBUS_WR_OFFTIME_FIRST && StartAdd + loop <= MODBUS_WR_OFFTIME_LAST )
			{
				// --- send first byte -------------			
				
				temp_number = (StartAdd + loop - MODBUS_WR_OFFTIME_FIRST) / WR_TIME_SIZE;
				temp_address = (StartAdd + loop - MODBUS_WR_OFFTIME_FIRST) % WR_TIME_SIZE;

				send_buffer = WR_Roution[temp_number].OffTime[temp_address];

				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = send_buffer;			
			}	
			/*else if(StartAdd + loop >= MODBUS_TSTAT_OFFTIME_FIRST && StartAdd + loop <= MODBUS_TSTAT_OFFTIME_LAST )	
			{
				sendbuf[HeadLen + 3 + loop * 2] = (U8_T) (time_tstat_off[StartAdd + loop - MODBUS_TSTAT_OFFTIME_FIRST] >> 8);
				sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)time_tstat_off[StartAdd + loop - MODBUS_TSTAT_OFFTIME_FIRST];
			}	
			else if(StartAdd + loop >= MODBUS_TSTAT_ONTIME_FIRST && StartAdd + loop <= MODBUS_TSTAT_ONTIME_LAST )	
			{
				sendbuf[HeadLen + 3 + loop * 2] = (U8_T)(time_tstat_on[StartAdd + loop - MODBUS_TSTAT_ONTIME_FIRST] >> 8);
				sendbuf[HeadLen + 3 + loop * 2 + 1] = (U8_T)time_tstat_on[StartAdd + loop - MODBUS_TSTAT_ONTIME_FIRST];
			}*/
		   /**************** start of names inputs and output *********************/ 
			else if(StartAdd + loop >= MODBUS_NAME_FIRST && StartAdd + loop <= MODBUS_NAME_LAST)
			{
				unsigned char temp = (StartAdd + loop - MODBUS_NAME_FIRST);
				sendbuf[HeadLen + 3 + loop * 2] = menu_name[temp / (NAME_SIZE / 2)][(temp % (NAME_SIZE / 2)) * 2 ];
				sendbuf[HeadLen + 3 + loop * 2 + 1] = menu_name[temp / (NAME_SIZE / 2)][(temp % (NAME_SIZE / 2)) * 2 + 1];			
			}
			 /******************end of names of inputs and outputs ******************/
			else if(StartAdd + loop == MODBUS_DISPLAY_TEMP_NUM)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = dis_temp_num;
			}	
			else if(StartAdd + loop == MODBUS_DISPLAY_TMEP_INTERVAL)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = dis_temp_interval;
			}	
			else if(StartAdd + loop >= MODBUS_DISPLAY_TEMP_SEQ_FIRST && StartAdd + loop <= MODBUS_DISPLAY_TEMP_SEQ_LAST)
			{
				sendbuf[HeadLen + 3 + loop * 2] = 0;
				sendbuf[HeadLen + 3 + loop * 2 + 1] = dis_temp_seq[StartAdd + loop - MODBUS_DISPLAY_TEMP_SEQ_FIRST];
			}	
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

		if(type == MODBUS)
		{
			main_init_send_com();
			for(loop = 0;loop < RegNum * 2 + 3;loop++)
				main_send_byte(sendbuf[loop],CRC_NO);

			main_send_byte(MainCRChi,CRC_YES);
			main_send_byte(MainCRClo,CRC_YES);
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

			TCPIP_TcpSend(pHttpConn->TcpSocket, sendbuf, RegNum * 2 + HeadLen + 3, TCPIP_SEND_NOT_FINAL); 
		}
	}
	else if(cmd == WRITE_VARIABLES)
	{		
		
		if(type == MODBUS)
		{
			main_init_send_com();
			/* send data back */
			for (loop = 0;loop < 8;loop++)
			{
				main_send_byte(pData[loop],CRC_YES);	
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
			TCPIP_TcpSend(pHttpConn->TcpSocket, sendbuf, 6 + UIP_HEAD, TCPIP_SEND_NOT_FINAL); 
		}


		/* dealwith write command */
		if (StartAdd >= MODBUS_SERIALNUMBER_LOWORD && StartAdd <= MODBUS_SERIALNUMBER_LOWORD + 3 )
		{
			if((StartAdd == MODBUS_SERIALNUMBER_LOWORD) && (SNWriteflag & 0x01) == 0)
			{
				 serialNum[0] = pData[HeadLen + 5];
				 serialNum[1] = pData[HeadLen + 4];
				E2prom_Write_Byte(EEP_SERIALNUMBER_LOWORD , serialNum[0]);
				E2prom_Write_Byte(EEP_SERIALNUMBER_LOWORD + 1, serialNum[1]);

				SNWriteflag |= 0x01;
				if(SNWriteflag & 0x02)
					update_flash = 0;
			}
			else if((StartAdd == MODBUS_SERIALNUMBER_HIWORD) && (SNWriteflag & 0x02) == 0)
			{
				 serialNum[2] = pData[HeadLen + 5];
				 serialNum[3] = pData[HeadLen + 4];
				E2prom_Write_Byte(EEP_SERIALNUMBER_HIWORD , serialNum[2]);
				E2prom_Write_Byte(EEP_SERIALNUMBER_HIWORD + 1, serialNum[3]);
				SNWriteflag |= 0x02;
				if(SNWriteflag & 0x01)
					update_flash = 0;
			}
		}
		else if(StartAdd == MODBUS_HARDWARE_REV)
		{	
			 hardRev = pData[HeadLen + 5]+ (pData[HeadLen + 4]<<8);
			E2prom_Write_Byte(EEP_HARDWARE_REV, pData[HeadLen + 5]+ (pData[HeadLen + 4]<<8) );

			Lcd_Initial();
			//SNWriteflag |= 0x04;
			//E2prom_Write_Int(EEP_SERINALNUMBER_WRITE_FLAG, SNWriteflag );
			//hardware_rev = pData[HeadLen + 5];
		}
		else if(StartAdd ==  MODBUS_ADDRESS )
		{
			 Modbus_address = pData[HeadLen + 5]+ (pData[HeadLen + 4]<<8);
			E2prom_Write_Byte(EEP_ADDRESS, Modbus_address);
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

					 serialNum[0] = 0;
					 serialNum[1] = 0;
					 serialNum[2] = 0;
					 serialNum[3] = 0;
				}
			}
		}
		/*else if(StartAdd >= MODBUS_MINADDR && StartAdd <= MODBUS_MAXADDR)		 
		{ 								
			E2prom_Write_Byte(EEP_MINADDR + StartAdd - MODBUS_MINADDR, pData[HeadLen + 5] );
			 address = pData[HeadLen + 5] ;	
		//	iap_program_data_byte(0,DATA_TO_FIRMWARE + EEP_ADDRESS);	// will reset the address to 254 if in ISP mode
			IntFlashWriteByte(0x70000 + DATA_TO_FIRMWARE + StartAdd - MODBUS_MINADDR,0);
		}*/
		else if(StartAdd == MODBUS_BAUDRATE )
		{
			 baudrate = pData[HeadLen + 5];
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
	/*	else if( StartAdd ==  addressESS_PLUG_N_PLAY)
		{
			if(randval == RESPONSERANDVALUE)
			{
				if(pData[HeadLen + 5] >=  MinAddr && pData[HeadLen + 5] <=  MaxAddr && pData[HeadLen + 5] != 0 && pData[HeadLen + 5] != 255)
				{
					E2prom_Write_Byte( EEP_ADDRESS , pData[HeadLen + 5] ) ;
					 address = pData[HeadLen + 5];	
					IntFlashWriteByte(0x70000 + DATA_TO_FIRMWARE + StartAdd - MODBUS_MINADDR,0);
				}
			}
		}*/	
		else if(StartAdd == MODBUS_UNIT)
		{
			 unit = pData[HeadLen + 5];
			E2prom_Write_Byte(EEP_UNIT, unit);
		}
		else if(StartAdd == MODBUS_DEMO_ENABLE)
		{
			 demo_enable = pData[HeadLen + 5];
		//	E2prom_Write_Byte(EEP_UNIT, unit);
		}
		else if(StartAdd == MODBUS_PROTOCAL)
		{
			 protocal = pData[HeadLen + 5];
		  	 E2prom_Write_Byte(EEP_PROTOCAL, protocal);
		}	
			
		else if(StartAdd == MODBUS_TCP_TYPE)
		{
			TCP_TYPE = pData[HeadLen + 5]+ (pData[HeadLen + 4]<<8);
			E2prom_Write_Byte(EEP_TCP_TYPE,  TCP_TYPE );
		}
		else if(StartAdd >= MODBUS_IP_1 && StartAdd <= MODBUS_IP_4)
		{
			 IP_Addr[StartAdd - MODBUS_IP_1] = pData[HeadLen + 5];

			E2prom_Write_Byte(EEP_IP + 3 - (StartAdd - MODBUS_IP_1), IP_Addr[StartAdd - MODBUS_IP_1]);
		}
		else if(StartAdd >= MODBUS_SUBNET_1 && StartAdd <= MODBUS_SUBNET_4)
		{
			 SUBNET[StartAdd - MODBUS_SUBNET_1] = pData[HeadLen + 5];
			E2prom_Write_Byte(EEP_SUBNET + 3 - ( StartAdd - MODBUS_SUBNET_1), SUBNET[StartAdd - MODBUS_SUBNET_1]);
		}
		else if(StartAdd >= MODBUS_GETWAY_1 && StartAdd <= MODBUS_GETWAY_4)
		{
			 GETWAY[StartAdd - MODBUS_GETWAY_1] = pData[HeadLen + 5];
		//	E2prom_Write_Byte(EEP_GETWAY + StartAdd - MODBUS_GETWAY_1, GETWAY[StartAdd - MODBUS_GETWAY_1]);
		}
		else if(StartAdd >= MODBUS_AINPUT1 && StartAdd <= MODBUS_AINPUT10 )
		{  // if auto/manual feature is Manual (0=auto, 1=manual)
			if( AInputAM & (0x01 << (StartAdd - MODBUS_AINPUT1)))  // manual	
			{
				 AI_Value[StartAdd - MODBUS_AINPUT1] = pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4]<<8);
			}
		}
		else if(StartAdd >= MODBUS_INPUT1_RANGE && StartAdd <= MODBUS_INPUT10_RANGE)
		{
			Input_Range[StartAdd - MODBUS_INPUT1_RANGE] = pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4]<<8);
			if( Input_Range[StartAdd - MODBUS_INPUT1_RANGE] > 5 ||  Input_Range[StartAdd - MODBUS_INPUT1_RANGE] < 0)
				 Input_Range[StartAdd - MODBUS_INPUT1_RANGE] = 0;
			E2prom_Write_Byte(EEP_INPUT1_RANGE + StartAdd - MODBUS_INPUT1_RANGE, Input_Range[StartAdd - MODBUS_INPUT1_RANGE]);
		}
		else if(StartAdd >= MODBUS_INPUT1_FILTER && StartAdd <= MODBUS_INPUT10_FILTER)
		{
			 Input_Filter[StartAdd - MODBUS_INPUT1_FILTER] = pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4]<<8);
			E2prom_Write_Byte(EEP_INPUT1_FILTER + StartAdd - MODBUS_INPUT1_FILTER, Input_Filter[StartAdd - MODBUS_INPUT1_FILTER]);
		}
		else if(StartAdd >= MODBUS_INPUT1_CAL && StartAdd <= MODBUS_INPUT10_CAL)
		{
			Input_CAL[StartAdd - MODBUS_INPUT1_CAL] = pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4]<<8);
			E2prom_Write_Byte(EEP_INPUT1_CAL + 2 * (StartAdd - MODBUS_INPUT1_CAL),pData[HeadLen + 5]);
			E2prom_Write_Byte(EEP_INPUT1_CAL + 2 * (StartAdd - MODBUS_INPUT1_CAL) + 1,pData[HeadLen + 4]);
		}
		else if(StartAdd >= MODBUS_DI_TYPE1 && StartAdd <= MODBUS_DI_TYPE8)
		{
			DI_Type[StartAdd - MODBUS_DI_TYPE1] = pData[HeadLen + 5];
			E2prom_Write_Byte(EEP_DI_TYPE1 + StartAdd - MODBUS_DI_TYPE1, DI_Type[StartAdd - MODBUS_DI_TYPE1]);
		}
		/* for test OUTPUT 1-8 */
		else if(StartAdd == MODBUS_DI1)
		{
			U8_T loop;
			U8_T temp1 = 0;
			U8_T temp2 = 0;
			U8_T temp3 = 0;
			for(loop = 0;loop < 8;loop++)
			{
				temp1 = pData[HeadLen + 5] & (0x01 << loop);
				temp2 = DInputAM & (0x01 << loop);
				temp3 |= (temp1 & temp2);
					
			}
			DI1_Value = temp3;		
		}
		else if(StartAdd == MODBUS_DI2)
		{	
			U8_T loop;
			U8_T temp1 = 0;
			U8_T temp2 = 0;
			U8_T temp3 = 0;
			for(loop = 0;loop < 8;loop++)
			{
				temp1 = pData[HeadLen + 5] & (0x01 << loop);
				temp2 = (DInputAM >> 8) & (0x01 << loop);
				temp3 |= (temp1 & temp2);
					
			}
			DI2_Value = temp3;
		}
		else if(StartAdd == MODBUS_DOUTPUT)
		{  /* only manual type, it can be changed */
			/*U8_T loop;
			U16_T temp = pData[HeadLen + 5] + (U16_T)(pData[HeadLen + 4]<<8);
			for(loop = 0;loop < 10;loop++)
			{
				if(OuputAM & (0x01 << loop)) // manual
				{
					if(temp & (0x01 << loop))
					{
					   	DO_Value |= (0x01 << loop);
					}
					else
					{
						DO_Value &= ~(0x01 << loop);	
					}
				}
			}*/
			
			DO_Value = (pData[HeadLen + 5] + (U16_T)(pData[HeadLen + 4]<<8)) & OuputAM;
			E2prom_Write_Byte(EEP_OUTPUT_LOW,(U8_T)DO_Value);
			E2prom_Write_Byte(EEP_OUTPUT_HIGH,(U8_T)(DO_Value >> 8));
		}		
		else if(StartAdd == MODBUS_SWITCH )
		{
			 DO_SoftSwitch = pData[HeadLen + 5] + (U16_T)(pData[HeadLen + 4]<<8);
			if( DO_SoftSwitch >2) 	 DO_SoftSwitch = 0;
			E2prom_Write_Byte(EEP_SWITCH, DO_SoftSwitch);
		}
		else if(StartAdd == MODBUS_PRIORTITY )
		{
			 Priority = pData[HeadLen + 5] + (U16_T)(pData[HeadLen + 4]<<8);
			 E2prom_Write_Byte(EEP_PRIORTITY, Priority);
			 count_priority = 0;
		}
		else if(StartAdd == MODBUS_RESET_PARAMETER )
		{
		//	if(pData[HeadLen + 5] + (U16_T)(pData[HeadLen + 4]) == 55)
			 set_default_parameters();
		//	 ChangeFlash = 1;
		} 
		else if(StartAdd == MODBUS_DI_ENABLE)
		{
			 DI_Enable = pData[HeadLen + 5] + (U16_T)(pData[HeadLen + 4]<<8);
			E2prom_Write_Byte(EEP_DI_ENABLE_LOW,pData[HeadLen + 5]);
			E2prom_Write_Byte(EEP_DI_ENABLE_HIGH,pData[HeadLen + 4]);
		}
		else if(StartAdd == MODBUS_AI_ENABLE)
		{
			 AI_Enable = pData[HeadLen + 5] + (U16_T)(pData[HeadLen + 4]<<8);
			E2prom_Write_Byte(EEP_AI_ENABLE_LOW,pData[HeadLen + 5]);
			E2prom_Write_Byte(EEP_AI_ENABLE_HIGH,pData[HeadLen + 4]);
		}
		else if(StartAdd == MODBUS_DINPUT_AM)
		{
			DInputAM = pData[HeadLen + 5] + (U16_T)(pData[HeadLen + 4]<<8);
			E2prom_Write_Byte(EEP_DINPUT_AM_LOW,pData[HeadLen + 5]);
			E2prom_Write_Byte(EEP_DINPUT_AM_HIGH,pData[HeadLen + 4]);
		}
		else if(StartAdd == MODBUS_OUTPUT_AM)
		{ 
			 OuputAM = pData[HeadLen + 5] + (U16_T)(pData[HeadLen + 4]<<8);
			 E2prom_Write_Byte(EEP_OUTPUT_AM_LOW,pData[HeadLen + 5]);
			 E2prom_Write_Byte(EEP_OUTPUT_AM_HIGH,pData[HeadLen + 4]);
		}
		else if(StartAdd == MODBUS_AINPUT_AM)
		{
			AInputAM = pData[HeadLen + 5] + (U16_T)(pData[HeadLen + 4]<<8);
//			E2prom_Write_Byte(EEP_AINPUT_AM_LOW,pData[HeadLen + 5]);
//			E2prom_Write_Byte(EEP_AINPUT_AM_HIGH,pData[HeadLen + 4]);
		}	
		else if(StartAdd == MODBUS_ENABLE_WRITE_MAC)
		{
			mac_address_write_enable = 1;
		}
	/*	else if(StartAdd == MODBUS_MASTER )
		{
			 Master = pData[HeadLen + 5];+ (U16_T)(pData[HeadLen + 4]<<8);
			E2prom_Write_Byte(EEP_MASTER, Master);
		}*/
		else if(StartAdd >= MODBUS_TIMER_ADDRESS && StartAdd <= MODBUS_TIMER_ADDRESS + 7)
		{
			 RTC.all[StartAdd - MODBUS_TIMER_ADDRESS] = pData[HeadLen + 5]+ (U16_T)(pData[HeadLen + 4]<<8);
		//	E2prom_Write_Byte(EEP_SEC + StartAdd - MODBUS_SEC, Time.all[StartAdd - MODBUS_SEC]);
			Set_Clock(PCF_SEC + 7 - StartAdd + MODBUS_TIMER_ADDRESS, RTC.all[StartAdd - MODBUS_TIMER_ADDRESS]);
		}
		else if(StartAdd == MODBUS_DISPLAY_TEMP_NUM)
		{
			dis_temp_num = pData[HeadLen + 5];
			E2prom_Write_Byte(EEP_DIS_TEMP_NUM,pData[HeadLen + 5]);
		}
		else if(StartAdd == MODBUS_DISPLAY_TMEP_INTERVAL)
		{
			dis_temp_interval = pData[HeadLen + 5];
			E2prom_Write_Byte(EEP_DIS_TEMP_INTERVAL,pData[HeadLen + 5]);
		}	
		else if(StartAdd >= MODBUS_DISPLAY_TEMP_SEQ_FIRST && StartAdd <= MODBUS_DISPLAY_TEMP_SEQ_FIRST + 9)
		{
			dis_temp_seq[StartAdd - MODBUS_DISPLAY_TEMP_SEQ_FIRST] = pData[HeadLen + 5];
			E2prom_Write_Byte(EEP_DIS_TEMP_SEQ_FIRST + StartAdd - MODBUS_DISPLAY_TEMP_SEQ_FIRST,pData[HeadLen + 5]);
		}

	}
	else if(cmd == MULTIPLE_WRITE)
	{
		if(type == MODBUS)   // TBD: need change
		{	
			main_init_send_com();
			// --- response to a multiple write function ---
			// the 6 first bits are the same and then send the crc bits
			for (loop = 0; loop < 6; loop++)
				main_send_byte(pData[loop],CRC_NO);	
			main_send_byte(MainCRChi,CRC_YES); //send the two last CRC bits
			main_send_byte(MainCRClo,CRC_YES);
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
			TCPIP_TcpSend(pHttpConn->TcpSocket, sendbuf, 6 + UIP_HEAD, TCPIP_SEND_NOT_FINAL);  
		}

		// the following are schedule registers 
		//ChangeFlash = 1;

		if(StartAdd == MODBUS_MAC_1)
		{ 			
			if(mac_address_write_enable)
			{
				if(pData[HeadLen + 5] == 12)
				{
					Mac_Addr[0] = pData[HeadLen + 7];
					Mac_Addr[1] = pData[HeadLen + 9];
					Mac_Addr[2] = pData[HeadLen + 11];
					Mac_Addr[3] = pData[HeadLen + 13];
					Mac_Addr[4] = pData[HeadLen + 15];
					Mac_Addr[5] = pData[HeadLen + 17];
				}
				mac_address_write_enable = 0;
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
				calibrated_time = 1;
			 
			 }
		 }
		
		 if(StartAdd < MODBUS_WR_DESCRIP_LAST && StartAdd >= MODBUS_WR_DESCRIP_FIRST)
		 {
		 	if((StartAdd - MODBUS_WR_DESCRIP_FIRST) % WR_DESCRIPTION_SIZE == 0)
		 	{
				U8_T i;
		 		i = (StartAdd - MODBUS_WR_DESCRIP_FIRST) / WR_DESCRIPTION_SIZE;
				
				memcpy(WR_Roution[i].UN.all,&pData[HeadLen + 7],WR_DESCRIPTION_SIZE);
		
		 	}
		 }
		 else if(StartAdd < MODBUS_AR_DESCRIP_LAST && StartAdd >= MODBUS_AR_DESCRIP_FIRST)
		 {
		 	if((StartAdd - MODBUS_AR_DESCRIP_FIRST) % AR_DESCRIPTION_SIZE == 0)
		 	{
				U8_T i;
		 		i = (StartAdd - MODBUS_AR_DESCRIP_FIRST) / AR_DESCRIPTION_SIZE;
		 
				memcpy(AR_Roution[i].UN.all,&pData[HeadLen + 7],AR_DESCRIPTION_SIZE);
		 	
		 	}
		 }
		 else if(StartAdd < MODBUS_ID_LAST && StartAdd >= MODBUS_ID_FIRST)
		 {
		 	if((StartAdd - MODBUS_ID_FIRST) % ID_SIZE == 0)
		 	{
				U8_T i;
		 		i = (StartAdd - MODBUS_ID_FIRST) / ID_SIZE;
		
				memcpy(ID_Config[i].all,&pData[HeadLen + 7],ID_SIZE);
		
		 	}
		 }
		 else if(StartAdd < MODBUS_AR_TIME_LAST && StartAdd >= MODBUS_AR_TIME_FIRST)
		 {
		 	if((StartAdd - MODBUS_AR_TIME_FIRST)%AR_TIME_SIZE == 0)
		 	{
				U8_T i;
		 		i = (StartAdd - MODBUS_AR_TIME_FIRST) / AR_TIME_SIZE;
		
				memcpy(AR_Roution[i].Time,&pData[HeadLen + 7],AR_TIME_SIZE);		 	}
		 }
		 else if(StartAdd < MODBUS_WR_ONTIME_LAST && StartAdd >= MODBUS_WR_ONTIME_FIRST)
		 {
		 	if((StartAdd - MODBUS_WR_ONTIME_FIRST) % WR_TIME_SIZE == 0)
		 	{
				U8_T i;
		 		i = (StartAdd - MODBUS_WR_ONTIME_FIRST) / WR_TIME_SIZE;
		
				memcpy(WR_Roution[i].OnTime,&pData[HeadLen + 7],WR_TIME_SIZE);
		 	}
		 }
		 else if(StartAdd < MODBUS_WR_OFFTIME_LAST && StartAdd >= MODBUS_WR_OFFTIME_FIRST)
		 {
		 	if((StartAdd-MODBUS_WR_OFFTIME_FIRST) % WR_TIME_SIZE == 0)
		 	{
				U8_T i;
		 		i = (StartAdd - MODBUS_WR_OFFTIME_FIRST) / WR_TIME_SIZE;
		
				memcpy(WR_Roution[i].OffTime,&pData[HeadLen + 7],WR_TIME_SIZE);		 
			}
		 }

		 else if(StartAdd < MODBUS_NAME_LAST && StartAdd >= MODBUS_NAME_FIRST)
		 {
		 	if((StartAdd - MODBUS_NAME_FIRST) % (NAME_SIZE / 2) == 0)
		 	{
				U8_T i;
			 	i = (StartAdd - MODBUS_NAME_FIRST) / (NAME_SIZE / 2);
	
				memcpy(menu_name[i],&pData[HeadLen + 7],NAME_SIZE); 	
			//	if(i == MAX_NAME - 1)	  // write flash after receive the last name
			//		ChangeFlash = 1;
				ChangeFlash = 1;			
			}
		 }

	}
	else if(cmd == CHECKONLINE)
	{
		if(type == MODBUS)
		{
			main_init_send_com();
			main_send_byte(main_data_buffer[0],CRC_NO);
			main_send_byte(main_data_buffer[1],CRC_NO);
			main_send_byte( Modbus_address,CRC_NO);				// send address of device	
	
			main_send_byte( serialNum[0],CRC_NO);
			main_send_byte( serialNum[1],CRC_NO);
			main_send_byte( serialNum[2],CRC_NO);
			main_send_byte( serialNum[3],CRC_NO);	
	
			main_send_byte(MainCRChi,CRC_YES);		// send the two last CRC bits
			main_send_byte(MainCRClo,CRC_YES);
		}
		else
		{
			
		}
	}
}
