#include "24cxx.h" 
#include "types.h"
#include "usart.h"
#include "define.h"
#include "delay.h"
#include "spi.h"




void E2prom_Initial(void)
{
		AT24CXX_Init();
}


U8_T E2prom_Read_Byte(U16_T addr,U8_T *value)
{	
		*value = AT24CXX_ReadOneByte(addr);
		return 1;
}


U8_T E2prom_Read_Int(U16_T addr,U16_T *value)
{
		*value = AT24CXX_ReadLenByte(addr,2);
		return 1;
	
}

U8_T E2prom_Write_Byte(U16_T addr,U16_T dat)
{	
		AT24CXX_WriteOneByte(addr,dat);
		return 1;
}

void DELAY_Us(U32_T delay)
{
		delay_us(delay);
}

void DELAY_Ms(U32_T delay)
{
	delay_ms(delay);
}

void SPI_ByteWrite(u8 TxData)
{
	SPI1_ReadWriteByte(TxData);		
}



S8_T far time[20];
void get_time_text(void)
{
	time[0] = '2';
	time[1] = '0';
	time[2] =  Rtc.Clk.year / 10 + '0';
	time[3] =  Rtc.Clk.year % 10 + '0';
	time[4] = '-';
	time[5] =  Rtc.Clk.mon / 10 + '0';
	time[6] =  Rtc.Clk.mon % 10 + '0';
	time[7] = '-';
	time[8] =  Rtc.Clk.day / 10 + '0';
	time[9] =  Rtc.Clk.day % 10 + '0';
	time[10] = ' ';
	time[11] =  Rtc.Clk.hour / 10 + '0';
	time[12] =  Rtc.Clk.hour % 10 + '0';
	time[13] = ':';
	time[14] =  Rtc.Clk.min / 10 + '0';
	time[15] =  Rtc.Clk.min % 10 + '0';
	time[16] = ':';
	time[17] =  Rtc.Clk.sec / 10 + '0';
	time[18] =  Rtc.Clk.sec % 10 + '0';

}

void UART_Init(U8_T port)
{
	U8_T temp;

	if(port == PORT_RS485_MAIN)  // main 
	{
		
		if(flag_temp_baut[2] == 1)
		{
			temp = temp_baut[2];
			flag_temp_baut[2] = 0;
		}
		else
		{
			temp = uart2_baudrate;
		}

		switch(temp)
		{
			case UART_1200:	uart3_init(1200); break;
			case UART_2400:	uart3_init(2400); break;
			case UART_3600:	uart3_init(3600); break;
			case UART_4800:	uart3_init(4800); break;
			case UART_7200:	uart3_init(7200); break;
			case UART_9600:	uart3_init(9600); break;
			case UART_19200:	uart3_init(19200); break;
			case UART_38400:	uart3_init(38400); break;
			case UART_57600:	uart3_init(57600); break;		
			case UART_76800:	uart3_init(76800); break;						
			case UART_115200:	uart3_init(115200); break;
			case UART_921600:	uart3_init(921600); break;
			default:
				break;
		}
	}
	else if(port == PORT_RS485_SUB)
	{
		if(flag_temp_baut[0] == 1)
		{
			temp = temp_baut[0];
			flag_temp_baut[0] = 0;
		}
		else
		{
			temp = uart0_baudrate;
		}
		switch(temp)
		{
			case UART_1200:	uart1_init(1200); break;
			case UART_2400:	uart1_init(2400); break;
			case UART_3600:	uart1_init(3600); break;
			case UART_4800:	uart1_init(4800); break;
			case UART_7200:	uart1_init(7200); break;
			case UART_9600:	uart1_init(9600); break;
			case UART_19200:	uart1_init(19200); break;
			case UART_38400:	uart1_init(38400); break;
			case UART_57600:	uart1_init(57600); break;
			case UART_76800:	uart1_init(76800); break;	
			case UART_115200:	uart1_init(115200); break;
			case UART_921600:	uart1_init(921600); break;
			default:
				break;
		}
	}
	else if(port == PORT_ZIGBEE)
	{	
		if((Modbus.mini_type == MINI_NEW_TINY) || (Modbus.mini_type == MINI_TINY_ARM))  // new arm tiny, dont have zigbee port
				return;
		switch(uart1_baudrate)
		{
			case UART_1200:	uart2_init(1200); break;
			case UART_2400:	uart2_init(2400); break;
			case UART_3600:	uart2_init(3600); break;
			case UART_4800:	uart2_init(4800); break;
			case UART_7200:	uart2_init(7200); break;
			case UART_9600:	uart2_init(9600); break;
			case UART_19200:	uart2_init(19200); break;
			case UART_38400:	uart2_init(38400); break;
			case UART_57600:	uart2_init(57600); break;	
			case UART_76800:	uart2_init(76800); break;	
			case UART_115200:	uart2_init(115200); break;
			case UART_921600:	uart2_init(921600); break;
			default:
				break;
		}
	}
	else
		return;
}


// only for mstp
#if 0
extern bit flagLED_uart2_tx;
void MSPT_SendData(uint8_t tx_byte)
{
	flagLED_uart2_tx = 1; 
	com_tx[2]++;
	USART_SendData(USART3, tx_byte);
}
/*************************************************************************
* Description: Determines if a byte in the USART has been shifted from
*   register
* Returns: true if the USART register is empty
* Notes: none
**************************************************************************/
bool rs485_byte_sent(
    void)
{
    return USART_GetFlagStatus(USART3, USART_FLAG_TXE);
}

/*************************************************************************
* Description: Determines if the entire frame is sent from USART FIFO
* Returns: true if the USART FIFO is empty
* Notes: none
**************************************************************************/
bool rs485_frame_sent(
    void)
{
    return USART_GetFlagStatus(USART3, USART_FLAG_TC);
}


#endif