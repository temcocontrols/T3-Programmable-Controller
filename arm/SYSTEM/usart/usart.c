
#include "usart.h"

#include "main.h"

//////////////////////////////////////////////////////////////////////////////////
//V1.3修改说明 
//支持适应不同频率下的串口波特率设置.
//加入了对printf的支持
//增加了串口接收命令功能.
//修正了printf第一个字符丢失的bug
//V1.4修改说明
//1,修改串口初始化IO的bug
//2,修改了USART_RX_STA,使得串口最大接收字节数为2的14次方
//3,增加了USART_REC_LEN,用于定义串口最大允许接收的字节数(不大于2的14次方)
//4,修改了EN_USART1_RX的使能方式
//V1.5修改说明
////////////////////////////////////////////////////////////////////////////////// 	  
 

//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 
FILE __stdout;
       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
}

//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
//	while((USART1->SR & 0X40) == 0);//循环发送,直到发送完毕   
//	USART1->DR = (u8)ch;
//	TXEN = SEND;
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
	USART_SendData(USART1, ch);
//	TXEN = RECEIVE;
	return ch;
}
#endif 

/*使用microLib的方法*/
 /* 
int fputc(int ch, FILE *f)
{
	USART_SendData(USART1, (uint8_t) ch);

	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}	
   
    return ch;
}

int GetKey (void)  { 

    while (!(USART1->SR & USART_FLAG_RXNE));

    return ((int)(USART1->DR & 0x1FF));
}
*/
 
#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	



//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA = 0;       //接收状态标记	  

//初始化IO 串口1 
//bound:波特率
// SUB
void uart1_init(u32 bound)
{
    //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);	//使能USART1，GPIOA时钟
 	USART_DeInit(USART1);  //复位串口1
	//USART1_TX   PA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				//PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//初始化PA9
 
	//USART1_RX	  PA.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;				//PA.10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//初始化PA10

#if (ARM_MINI || ARM_CM5)
	//RS485_TXEN	PC.2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;				//PA.2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//普通推挽输出
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//初始化PA2
//	GPIO_SetBits(GPIOC, GPIO_Pin_2);
#endif

#if ARM_WIFI
	//RS485_TXEN	PA.8
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				//PA.8
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//普通推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//初始化PA2
	GPIO_SetBits(GPIOA, GPIO_Pin_8);
#endif
// FOR TINY
	//RS485_TXEN	PD.8
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				//PD.8
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//普通推挽输出
	GPIO_Init(GPIOD, &GPIO_InitStructure);					//初始化PD8
//	GPIO_SetBits(GPIOC, GPIO_Pin_2);
	
	//Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;	//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);								//根据指定的参数初始化VIC寄存器
  
	//USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;					//波特率设置
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		//一个停止位
	
	if(Modbus.uart_parity[0] == 2)
	{
		USART_InitStructure.USART_Parity = USART_Parity_Even;
		USART_InitStructure.USART_WordLength = USART_WordLength_9b;	//字长为9位数据格式
	}
	else if(Modbus.uart_parity[0] == 1)
	{
		USART_InitStructure.USART_Parity = USART_Parity_Odd;
		USART_InitStructure.USART_WordLength = USART_WordLength_9b;	//字长为9位数据格式
	}
	else
	{
		USART_InitStructure.USART_Parity = USART_Parity_No;			//无奇偶校验位	
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//字长为8位数据格式
	}
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//收发模式
	USART_Init(USART1, &USART_InitStructure); 					//初始化串口

	USART_ITConfig(USART1, USART_IT_RXNE/*|USART_IT_TC*/, ENABLE);				//开启中断
	USART_Cmd(USART1, ENABLE);                    				
}

// MAIN
void uart3_init(u32 bound)
{
    //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE); 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOE, ENABLE);	//使能USART3，GPIOA时钟
 	USART_DeInit(USART3);  //复位串口1
	//USART3_TX   PB.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;				//PB.10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			//复用推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//初始化PB10
 
	//USART3_RX	  PB.11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;				//PB.11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//浮空输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//初始化PB11
	
	//RS485_TXEN	PE.11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;				//PE.11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//普通推挽输出
	GPIO_Init(GPIOE, &GPIO_InitStructure);				
//	GPIO_SetBits(GPIOE, GPIO_Pin_11);
	
	//Usart3 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;			//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);								//根据指定的参数初始化VIC寄存器
  
	//USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;					//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		//一个停止位

	if(Modbus.uart_parity[2] == 2)
	{
		USART_InitStructure.USART_Parity = USART_Parity_Even;
		USART_InitStructure.USART_WordLength = USART_WordLength_9b;	//字长为9位数据格式
	}
	else if(Modbus.uart_parity[2] == 1)
	{
		USART_InitStructure.USART_Parity = USART_Parity_Odd;
		USART_InitStructure.USART_WordLength = USART_WordLength_9b;	//字长为9位数据格式
	}
	else
	{
		USART_InitStructure.USART_Parity = USART_Parity_No;			//无奇偶校验位	
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//字长为8位数据格式
	}
	
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//收发模式
	USART_Init(USART3, &USART_InitStructure); 					//初始化串口

	USART_ITConfig(USART3, USART_IT_RXNE/*|USART_IT_TC*/, ENABLE);				//开启中断
	USART_Cmd(USART3, ENABLE);                    				
}

// ZIGBEE
void uart2_init(u32 bound)
{
    //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE); 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);	//使能USART2，GPIOA时钟
 	USART_DeInit(USART2);  //复位串口2
	//USART1_TX   PA.2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;				//PA.2
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//初始化PA9
 
	//USART1_RX	  PA.3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;				//PA.3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//初始化PA10

// SELECT RS232 or ZIGBEE by PC3	
	if((Modbus.mini_type == MINI_BIG) || (Modbus.mini_type == MINI_BIG_ARM))
	{
		// if T3_BB
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;				//PE.11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//普通推挽输出
	GPIO_Init(GPIOC, &GPIO_InitStructure);		
	}
	
	//Usart2 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;	//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);								//根据指定的参数初始化VIC寄存器
  
	//USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;					//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;			//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//收发模式
	USART_Init(USART2, &USART_InitStructure); 					//初始化串口

	USART_ITConfig(USART2, USART_IT_RXNE/*|USART_IT_TC*/, ENABLE);				//开启中断
	USART_Cmd(USART2, ENABLE);                    				
}



#endif
