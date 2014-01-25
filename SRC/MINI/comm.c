 /*================================================================================
 * Module Name : comm.c
 * Purpose     : communicate with the slaver (C8051F023) with SPI bus
 * Author      : Chelsea
 * Date        : 2010/02/03
 * Notes       : 
 * Revision	   : 
 *	rev1.0
 *   
 *================================================================================
 */


#include "e2prom.h"
#include "comm.h"
#include "projdefs.h"
#include "portable.h"
#include "errors.h"
#include "task.h"
#include "list.h"
#include "queue.h"
#include "spi.h"
#include "spiapi.h"
#include "stdio.h"
#include "define.h"
#include "main.h"


xTaskHandle xHandler_SPI;

#define SpiSTACK_SIZE		( ( unsigned portSHORT ) 256 )
unsigned int Filter(unsigned char channel,signed int input);


U16_T far Test[50];
U8_T re_send_led_in = 0;
U8_T re_send_led_out = 0;
//bit flag_EthPort = 0;
//bit flagLED_uart0 = 0;
//bit flagLED_uart1 = 0;
//bit flagLED_uart2 = 0;
//bit flagLED_usb = 0;

bit flagLED_ether_tx = 0;
bit flagLED_ether_rx = 0;
bit flagLED_uart0_rx = 0;
bit flagLED_uart0_tx = 0;
bit flagLED_uart1_rx = 0; 
bit flagLED_uart1_tx = 0;
bit flagLED_uart2_rx = 0;
bit flagLED_uart2_tx = 0;
bit flagLED_usb_rx = 0;
bit flagLED_usb_tx = 0;


U8_T uart0_heartbeat = 0;
U8_T uart1_heartbeat = 0;
U8_T uart2_heartbeat = 0;
U8_T etr_heartbeat = 0;
U8_T usb_heartbeat = 0;	

U8_T flag_led_out_changed = 0;	  
U8_T flag_led_in_changed = 0;
U8_T flag_led_comm_changed = 0;
U8_T flag_high_spd_changed = 0;
U8_T CommLed[2];
U8_T InputLed[32];
U8_T OutputLed[24];


void Updata_Comm_Led(void);

#if defined(MINI)


void vStartCommToTopTasks( unsigned char uxPriority)
{
	memset(InputLed,5,32);
	memset(OutputLed,5,24);

	CommLed[0] = CommLed[1] = 0;
	sTaskCreate( SPI_Roution, "SPITask", SpiSTACK_SIZE, NULL, uxPriority, &xHandler_SPI );
}

void Update_Led(void)
{
	U8_T loop;
//	U8_T index,shift;
//	U32_T tempvalue;
	U16_T pre_in[32];
	U8_T error_in; // error of input raw value	
	U8_T pre_status;
	U8_T max_in,max_out,max_digout;
	/*    check input led status */	
	if(Modbus.mini_type == MINI_BIG)
	{
		max_in = 32;
		max_out = 24;
		max_digout = 12;
	}
	else
	{
		max_in = 16;
		max_out = 10;
		max_digout = 6;
	}

	for(loop = 0;loop < max_in;loop++)
	{	
//		InputLed[loop] = loop / 5;
		pre_status = InputLed[loop];

		if(input_raw[loop] > pre_in[loop])
			error_in = input_raw[loop] - pre_in[loop];
		else
			error_in = pre_in[loop] - input_raw[loop];

		if(input_raw[loop]  < 20) 	InputLed[loop] = 0;
		else  if(input_raw[loop]  < 200) 	InputLed[loop] = 1;
		else  if(input_raw[loop]  < 400) 	InputLed[loop] = 2;
		else  if(input_raw[loop]  < 600) 	InputLed[loop] = 3;
		else  if(input_raw[loop]  < 800) 	InputLed[loop] = 4;
		else
			InputLed[loop] = 5;	
		if(pre_status != InputLed[loop] && error_in > 20)
		{  //  error is larger than 20, led of input is changed
			flag_led_in_changed = 1;   
			re_send_led_in = 0;
		}
		pre_in[loop] = input_raw[loop];
	} 	
	
	/*    check output led status */	
	for(loop = 0;loop < max_out;loop++)
	{	
//		OutputLed[loop] = loop / 4;
		pre_status = OutputLed[loop];

		if(outputs[loop].switch_status == SW_OFF)			 OutputLed[loop] = 0;
		else if(outputs[loop].switch_status == SW_HAND)		 OutputLed[loop] = 5;
		else   // AUTO 
		{
			if(loop < max_digout)	  // digital
			{
				if(output_raw[loop] < 512 ) 	OutputLed[loop] = 0;
				else
					OutputLed[loop] = 5;
			}
			else
			{
				if(output_raw[loop] >= 0 && output_raw[loop] < 50 )	   			OutputLed[loop] = 0;
				else if(output_raw[loop] >= 50 && output_raw[loop] < 200 )		OutputLed[loop] = 1;
				else if(output_raw[loop] >= 200 && output_raw[loop] < 400 )		OutputLed[loop] = 2;
				else if(output_raw[loop] >= 400 && output_raw[loop] < 600 )		OutputLed[loop] = 3;
				else if(output_raw[loop] >= 600 && output_raw[loop] < 800 )		OutputLed[loop] = 4;
				else if(output_raw[loop] >= 800 && output_raw[loop] < 1023 )		OutputLed[loop] = 5;
			}
		} 
		
		if(pre_status != OutputLed[loop])
		{
			flag_led_out_changed = 1;  
			re_send_led_out = 0;
		}
	}

	/*    check communication led status */

	Updata_Comm_Led();
}


void Updata_Comm_Led(void)
{
	U8_T temp1 = 0;
	U8_T temp2 = 0;
	U8_T pre_status1 = CommLed[0];
	U8_T pre_status2 = CommLed[1];

	if(Modbus.mini_type == MINI_BIG)
	{
	if(flagLED_uart0_rx)	{ 	temp1 |= 0x01;	 	flagLED_uart0_rx = 0;}
	if(flagLED_uart0_tx)	{	temp1 |= 0x02;		flagLED_uart0_tx = 0;}	
	if(flagLED_uart2_rx)	{	temp1 |= 0x04;		flagLED_uart2_rx = 0;}
	if(flagLED_uart2_tx)	{	temp1 |= 0x08;		flagLED_uart2_tx = 0;}
	if(flagLED_ether_rx)	{ 	temp1 |= 0x10;		flagLED_ether_rx = 0;}
	if(flagLED_ether_tx)	{	temp1 |= 0x20;		flagLED_ether_tx = 0;}
	if(flagLED_usb_rx)		{	temp1 |= 0x40;	 	flagLED_usb_rx = 0;}
	if(flagLED_usb_tx)		{	temp1 |= 0x80;		flagLED_usb_tx = 0;} 

	if(flagLED_uart1_rx)	{	temp2 |= 0x01;	 	flagLED_uart1_rx = 0;}
	if(flagLED_uart1_tx)	{	temp2 |= 0x02;		flagLED_uart1_tx = 0;} 
	}
	else
	{
	if(flagLED_uart2_rx)	{ 	temp1 |= 0x02;	 	flagLED_uart2_rx = 0;}
	if(flagLED_uart2_tx)	{	temp1 |= 0x01;		flagLED_uart2_tx = 0;}	
	if(flagLED_ether_rx)	{	temp1 |= 0x04;		flagLED_ether_rx = 0;}
	if(flagLED_ether_tx)	{	temp1 |= 0x08;		flagLED_ether_tx = 0;}
	if(flagLED_uart0_rx)	{ 	temp1 |= 0x10;		flagLED_uart0_rx = 0;}
	if(flagLED_uart0_tx)	{	temp1 |= 0x20;		flagLED_uart0_tx = 0;}
	if(flagLED_usb_rx)		{	temp1 |= 0x40;	 	flagLED_usb_rx = 0;}
	if(flagLED_usb_tx)		{	temp1 |= 0x80;		flagLED_usb_tx = 0;} 

	if(flagLED_uart1_tx)	{	temp2 |= 0x01;		flagLED_uart1_tx = 0;} 
	if(flagLED_uart1_rx)	{	temp2 |= 0x02;	 	flagLED_uart1_rx = 0;}

	}
	CommLed[0] = temp1;
	if(pre_status1 != CommLed[0])
		flag_led_comm_changed = 1;

	CommLed[1] = temp2;
	if(pre_status2 != CommLed[1])
		flag_led_comm_changed = 1;
}

void SPI_Send(U8_T cmd,U8_T* buf,U8_T len)
{	 
	U8_T i;
   	SPI_ByteWrite(cmd);
	for(i = 0; i < len; i++) 
	{	
		SPI_ByteWrite(buf[i]);
	}
	// add crc
	SPI_ByteWrite(0x55);
	SPI_ByteWrite(0xaa);

}

void SPI_Get(U8_T cmd,U8_T len)
{
	U8_T i;
	bit error = 1;
	U8_T count_error = 0;
	U8_T tmpbuf[64];
	U8_T crc[2];
	/*send the first byte, command type */	
	SPI_ByteWrite(cmd);
	SPI_ByteWrite(0xff);
	for(i = 0; i < len; i++) 
	{			
		SPI_ByteWrite(0xff);		
		SPI_GetData(&tmpbuf[i]);
		if(tmpbuf[i] != 0xff)  error = 0;
	}
	SPI_ByteWrite(0xff);		
	SPI_GetData(&crc[0]);
	SPI_ByteWrite(0xff);		
	SPI_GetData(&crc[1]);

	if((crc[0]!= 0x55) ||(crc[1] != 0xaa))	 error = 1;

	if(cmd == G_SWTICH_STATUS)
	{
		if((tmpbuf[0] == 0x55) && (tmpbuf[1] == 0x55) && (tmpbuf[2] == 0x55) && (tmpbuf[3] == 0x55) && (tmpbuf[4] == 0x55))
		{	// top board dont not get mini type
			Start_Comm_Top();
			error = 1;
		}
	}
	if(!error) // no error
	{
		count_error = 0;
		Test[47]++;
		if(cmd == G_SWTICH_STATUS)
		{
		   	for(i = 0;i < len;i++)
			{
				outputs[i].switch_status = tmpbuf[i];
			}
		   				
		}
		else if(cmd == G_INPUT_VALUE)
		{
		   	for(i = 0;i < len / 2;i++)
			{				   
				input_raw[i] = (U16_T)(tmpbuf[i * 2 + 1] + tmpbuf[i * 2] * 256);//Filter(i,(U16_T)(tmpbuf[i * 2 + 1] + tmpbuf[i * 2] * 256));
			//	Test[30 + i] = input_raw[i];
			}
		}
	}
	else
	{	// 
		count_error++;
		if(count_error > 10)
		{			
			Test[48]++;
//			Rsest_Top();
//			RESET_8051 = 0;
//			vTaskDelay(10);
//			RESET_8051 = 1;	
//			Start_Comm_Top();
			count_error = 0;
		}
	}
}

void SPI_Roution(void)
{
	static U8_T index = 0;
	portTickType xDelayPeriod = ( portTickType ) 50 / portTICK_RATE_MS;
	for (;;)
	{
		vTaskDelay(xDelayPeriod);

		Update_Led();		
			
		if(flag_led_out_changed)	
		{
			Test[32]++;
			if(re_send_led_out < 10)
			{	
				SPI_Send(S_OUTPUT_LED,OutputLed,24);
				re_send_led_out++;
			}
			else
			{
				re_send_led_out = 0;
				flag_led_out_changed = 0;
			}
		}
		else if(flag_led_in_changed)	
		{	
			Test[33]++;
			if(re_send_led_in < 10)
			{	
				SPI_Send(S_INPUT_LED,InputLed,32);
				re_send_led_in++;
			}
			else
			{
				re_send_led_in = 0;
				flag_led_in_changed = 0;
			}
		}
//		else if(flag_high_spd_changed)
//		{	
//			SPI_Send(S_HI_SP_FLAG,&high_spd_status,1);
//			flag_high_spd_changed = 0;
//		} 
//		else if(flag_led_comm_changed)	
//		{
//			SPI_Send(S_COMM_LED,&CommLed[0],2);
//			flag_led_comm_changed = 0;
//		}
		else  
		{	
		//	vTaskDelay(500 / portTICK_RATE_MS);	
			if(index == 0 || index == 2 || index == 4 || index == 6)	 
				SPI_Send(S_COMM_LED,CommLed,2);
			else if(index == 1)	 SPI_Send(S_OUTPUT_LED,OutputLed,24);
			else if(index == 3)	 SPI_Send(S_INPUT_LED,InputLed,32);
//			else if(index == 3)	 SPI_Send(S_HI_SP_FLAG,&high_spd_status,1);
			else 
			if(index == 5)	 
			{			
				SPI_Get(G_SWTICH_STATUS,24);
			}
			else if(index == 7)	 
			{
				SPI_Get(G_INPUT_VALUE,64);				
			}
//			else if((index == 6) && high_spd_status != 0)	 
//				;//SPI_Get(G_SPEED_COUNTER,high_spd_counter,24);
//	
			if(index < 7)	
				index++;
			else 
				index = 0; 
		
		}
	}
}




void Start_Comm_Top(void)
{
	SPI_ByteWrite(C_MINITYPE);
	SPI_ByteWrite(Modbus.mini_type);
} 


void Rsest_Top(void)
{
//	RESET_8051 = 0;
//	vTaskDelay(100);
//	RESET_8051 = 1;	
}
// send ack to confirm receive correct cmd
//void Send_ACK(U8_T cmd)
//{
//	SPI_ByteWrite(cmd);
//	SPI_ByteWrite(0xff);		
//	SPI_GetData(&crc[0]);
//}
//
//
//U8_T Get_ACK(void)
//{
//	U8_T ack;
//	SPI_ByteWrite(0xff);		
//	SPI_GetData(&ack);
//	retrun ack;
//}
/*
void Start_Comm_Top(void)
{
	 SPI_ByteWrite(C_START);
}


void Stop_Comm_Top(void)
{
	 SPI_ByteWrite(C_STOP);
}
*/
#endif


