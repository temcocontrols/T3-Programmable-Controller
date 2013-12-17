/**************************************************************************
*
* Copyright (C) 2007 Steve Karg <skarg@users.sourceforge.net>
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*********************************************************************/

/* The module handles sending data out the RS-485 port */
/* and handles receiving data from the RS-485 port. */
/* Customize this file for your specific hardware */
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
/*#include "mstp.h" */
#include "uart.h"
#include <main.h>
#include <fifo.h>

//uint8_t Rec_Mstp_Byte;
//uint8_t Rec_Mstp_Err;

/* This file has been customized for use with ATMEGA168 */
//#include "hardware.h"
#include "timer.h"


#define Tturnaround  (40UL)

/* Timers for turning off the TX,RX LED indications */
/*static uint8_t LED1_Off_Timer;
static uint8_t LED3_Off_Timer; */

/* baud rate */
static uint32_t RS485_Baud = 9600;

/*unsigned char xdata gucReceiveCount;
unsigned char xdata gucReceive_Index;
unsigned char xdata gucTake_Index;
bit  gbReceiveError;*/
bit MSTP_Transmit_Finished;
/* buffer for storing received bytes - size must be power of two */
static uint8_t Receive_Buffer_Data[512];
static FIFO_BUFFER Receive_Buffer;
/****************************************************************************
* DESCRIPTION: Initializes the RS485 hardware and variables, and starts in
*              receive mode.
* RETURN:      none
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
void RS485_Initialize(
    void)
{
	if(BACnet_Port == UART0)
		UART_Init(0); 
	else
		UART_Init(1); 
	FIFO_Init(&Receive_Buffer, &Receive_Buffer_Data[0],
        (unsigned) sizeof(Receive_Buffer_Data));
	
/*	gbReceiveError = false;
	gucTake_Index = 0;
	gucReceive_Index = 0;
	gucReceiveCount = 0;   */

    return;
}

/****************************************************************************
* DESCRIPTION: Returns the baud rate that we are currently running at
* RETURN:      none
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
uint32_t RS485_Get_Baud_Rate(
    void)
{
    return RS485_Baud;
}

/****************************************************************************
* DESCRIPTION: Sets the baud rate for the chip USART
* RETURN:      true if valid baud rate
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
bool RS485_Set_Baud_Rate(
    uint32_t baud)
{
    bool valid = true;
	RS485_Initialize();	
 #if 0
    switch (baud) {
        case 9600:
        case 19200:
        case 38400:
        case 57600:
        case 76800:
        case 115200:
            RS485_Baud = baud;
            /* 2x speed mode */
            BIT_SET(UCSR0A, U2X0);
            /* configure baud rate */
            UBRR0 = (F_CPU / (8UL * RS485_Baud)) - 1;
            /* FIXME: store the baud rate */
            break;
        default:
            valid = false;
            break;
    }
#endif
    return valid;
}

/****************************************************************************
* DESCRIPTION: Enable or disable the transmitter
* RETURN:      none
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
void RS485_Transmitter_Enable(
    bool enable)
{
	if(BACnet_Port == UART0)
	{
		if(enable)
			UART0_TXEN = 1;
		else
			UART0_TXEN = 0;
	}
	else
	{
		if(enable)
			UART1_TXEN = 1;
		else
			UART1_TXEN = 0;
	}
}

/****************************************************************************
* DESCRIPTION: Waits on the SilenceTimer for 40 bits.
* RETURN:      none
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
void RS485_Turnaround_Delay(
    void)
{
	uint16_t turnaround_time;
	
	/* delay after reception before trasmitting - per MS/TP spec */
	/* wait a minimum  40 bit times since reception */
	/* at least 1 ms for errors: rounding, clock tick */
	// if baud is 9600, turnaround_time is 4
	// if baud if 19200, turnaround time is 2
	turnaround_time = 2;//1 + ((Tturnaround * 1000UL) / RS485_Baud);
	while (Timer_Silence() < turnaround_time) {
	/* do nothing - wait for timer to increment */
	};
}




/****************************************************************************
* DESCRIPTION: Send some data and wait until it is sent
* RETURN:      none
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
void RS485_Send_Data(
    uint8_t * buffer,   /* data to send */
    uint16_t nbytes)
{       /* number of bytes of data */
    /* send all the bytes */
	uint16_t count = 0;

    while (nbytes) 
	{
        if(BACnet_Port == UART0)
			SBUF0 = *buffer;
		else
			SBUF1 = *buffer;
        MSTP_Transmit_Finished = 0;
		count = 0;
        while (!MSTP_Transmit_Finished && count < 2500) {
		count++;
            /* do nothing - wait until Tx buffer is empty */
        }
		
		if(count >= 2500)	
	{
		Test[17]++;
	}

        buffer++;
        nbytes--;
    }
    /* per MSTP spec */
    Timer_Silence_Reset();
}

/****************************************************************************
* DESCRIPTION: Return true if a framing or overrun error is present
* RETURN:      true if error
* ALGORITHM:   autobaud - if there are a lot of errors, switch baud rate
* NOTES:       Clears any error flags.
*****************************************************************************/
bool RS485_ReceiveError(
    void)
{
  //  bool ReceiveError = false;
 
    return false;//ReceiveError;
}

/****************************************************************************
* DESCRIPTION: Return true if data is available
* RETURN:      true if data is available, with the data in the parameter set
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
bool RS485_DataAvailable(
    uint8_t * data_register)
{
   bool DataAvailable = false;
   
    if (!FIFO_Empty(&Receive_Buffer)) {
        if (data_register) {
            *data_register = FIFO_Get(&Receive_Buffer);
        }
     //   timer_elapsed_start(&Silence_Timer);
        DataAvailable = true;
    }
 
  /*  if (gucReceiveCount) {
        *Dat = Receive_Buffer[gucTake_Index];
        if (gucTake_Index >= RECEIVE_BUFFER - 1)
            gucTake_Index = 0;
        else
            gucTake_Index++;
        // FIXME: disable interrupts around Rx_Bytes 
        gucReceiveCount--;
		DataAvailable = true;
	}
   */ 
    return DataAvailable;
}


void mstp_int_handler(void)
{
	uint8_t data_byte;
	if(BACnet_Port == UART0)
	{
	   if (RI0 == 1) {
	        /* we received a byte */
			data_byte = SBUF0;
			FIFO_Put(&Receive_Buffer, data_byte);
			
	        RI0 = 0;
	    } else if (TI0 == 1) {
	        /* we finished trasmitting a byte */
	        MSTP_Transmit_Finished = 1;
	        TI0 = 0;
	    }
	}
	else
	{
	    if (RI1 == 1) {
	        /* we received a byte */
			data_byte = SBUF1;
			FIFO_Put(&Receive_Buffer, data_byte);
			
	        RI1 = 0;
	    } else if (TI1 == 1) {
	        /* we finished trasmitting a byte */
	        MSTP_Transmit_Finished = 1;
	        TI1 = 0;
	    }
	}
    return;

}
#if 0
void MSTP_Interrupt(void) interrupt 6
{
	uint8_t data_byte;
    if (RI1 == 1) {
        /* we received a byte */
		data_byte = SBUF1;
		FIFO_Put(&Receive_Buffer, data_byte);
		
        RI1 = 0;
    } else if (TI1 == 1) {
        /* we finished trasmitting a byte */
        MSTP_Transmit_Finished = 1;
        TI1 = 0;
    }

    return;
}
#endif

#ifdef TEST_RS485
int main(
    void)
{
    unsigned i = 0;
    uint8_t DataRegister;

    RS485_Set_Baud_Rate(38400);
    RS485_Initialize();
    /* receive task */
    for (;;) {
        if (RS485_ReceiveError()) {
            fprintf(stderr, "ERROR ");
        } else if (RS485_DataAvailable(&DataRegister)) {
            fprintf(stderr, "%02X ", DataRegister);
        }
    }
}
#endif /* TEST_RS485 */
