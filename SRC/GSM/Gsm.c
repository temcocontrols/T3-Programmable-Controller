#include	"reg80390.h"
#include   "types.h"
#include 	"stdio.h"

#define 	SERAIL_IO 	TRUE

#if (SERAIL_IO == TRUE)
/* Software UART */

/* software serial port :  2 General IO and TIME0 */
#define 	GSM_ENABLE 	P3_1

#define 	GSM_TXD		P1_1
#define 	GSM_RXD		P3_0

U8_T   bdata rs_BUF;                  //串行收、发时用的移位暂存器。   `
sbit rs_BUF_bit7 = rs_BUF^7;        //移位暂存器的最高位。 
U8_T   rs_shift_count;                //移位计数器。  
U8_T bdata rsFlags;
sbit rs_f_TI        = rsFlags^0;    //0:正在发送; 1: 一个字符完毕  
sbit rs_f_RI_enable = rsFlags^1;   //0:禁止接收; 1:允许接收  
sbit rs_f_TI_enable = rsFlags^2;   //0:禁止发送; 1:允许发送  



//收、发一位所需定时器计数   
#define rs_FULL_BIT0	 217		// (Fosc/ 2 ) / Baud        1302 ( 9600 )   2604 ( 4800 )
#define rs_FULL_BIT 	(65536 - rs_FULL_BIT0)
#define rs_FULL_BIT_H 	rs_FULL_BIT >> 8        //收、发一位所需定时器计数高位   
#define rs_FULL_BIT_L 	(rs_FULL_BIT & 0x00FF)  //收、发一位所需定时器计数低位   
//检测起始位的时间间隔所需定时器计数    
#define rs_TEST0 rs_FULL_BIT0 / 4             //波特率较低时可以除以 3 或除以 2    
#define rs_TEST ((~rs_TEST0))
#define rs_TEST_H rs_TEST >> 8                //高位  
#define rs_TEST_L rs_TEST & 0x00FF            //低位  
//发送起始位所需定时器总计数   
#define rs_START_BIT 0xFFFF - 217/*(Fosc/12/Baud)*/ + 0x28
#define rs_START_BIT_H rs_START_BIT >> 8      //发送起始位所需定时器计数高位    
#define rs_START_BIT_L rs_START_BIT & 0x00FF  //发送起始位所需定时器计数低位   
#define rs_RECEIVE_MAX   128                  //最大接收长度  
U8_T rs232buffer[rs_RECEIVE_MAX];          //收、发缓冲区
U16_T ReceivePoint;                       //接收数据存储指针  


void soft_rs232_interrupt( void );

void soft_rs232_init (void)            //串口初始化  
{
    TR0 = 0;             //停止定时器  
    TH0 = 0xf8;
    TL0 = 0x06;
	GSM_RXD = 1;
	GSM_TXD = 1;
    TMOD &= 0xf0;		// set timer way
    TMOD |= 0x01;
    PT0 = 1;                        //置中断优先级为高  
    ET0 = 1;                        //允许定时器中断 
	TR0 = 1;             //启动定时器  
}

U32_T Timer0Counter = 0;

void vTimer0ISR( void ) interrupt 1   
{  // 		U8_T	temp;
		ET0 = 0;
	//	TH0 = 0xf8;
    //	TL0 = 0x06;
	//	Timer0Counter++;
		soft_rs232_interrupt();
		
		ET0 = 1;

	//	P3_1 = ~P3_1;
	/*	if (GSM_RXD == 0 | rs_shift_count > 0)
        { soft_rs232_interrupt(); }
        else
        {
            TH0 = rs_TEST_H;
            TL0 = rs_TEST_L;
        }*/
}
 

void soft_receive_init()               //监测起始位  
{
    TR0 = 0;             //停止定时器  
    TH0 = rs_TEST_H;
    TL0 = rs_TEST_L;
    rs_shift_count = 0;
    TR0 = 1;             //启动定时器  
}

void soft_receive_enable()             //允许接收  
{
    rs_f_RI_enable = 1;                //允许接收  
    rs_f_TI_enable = 0;                //禁止发送   
    soft_receive_init();               //监测起始位, RXD 下降沿触发接收字节过程.     
}
void soft_send_enable (void)        //允许发送  
{
    TR0 = 0;             //停止定时器  
    rs_f_TI_enable = 1;                //允许发送  
    rs_f_RI_enable = 0;                //禁止接收  
    rs_shift_count = 0;                //清移位计数器  
    rs_f_TI   = 1;                     //发送一个字符完毕标志  
    TR0 = 1;             //启动定时器
}
void soft_rs232_interrupt( void )
{
    /************************ 接收 ****************************/
    if (rs_f_RI_enable == 1)
    {
        if (rs_shift_count == 0)        //移位计数器==0, 表示检测到起始位的起点   
        {
            if ( GSM_RXD == 1 )
            {
                soft_receive_enable (); //起始位错, 从新开始   
            }
            else
            {
                //下次中断在数据位或停止位中的某时刻发生    
                TL0 += rs_FULL_BIT_L/* + 0x10*/; 
                TH0 = rs_FULL_BIT_H;
                rs_shift_count++;              
                rs_BUF = 0;             //清移位缓冲变量   
            }
        }
        else
        {
            TL0 += rs_FULL_BIT_L; //下次中断在数据位或停止位中发生    
            TH0 = rs_FULL_BIT_H;
                                       
            rs_shift_count++;           //2--9:数据位 10:停止位 
                                       
            if ( rs_shift_count == 9)
            {
                rs_BUF = rs_BUF >> 1;   //接收第8位   
                rs_BUF_bit7 = GSM_RXD;
                if( ReceivePoint < rs_RECEIVE_MAX)
                {                       //保存收到的字节    
                    rs232buffer[ReceivePoint++] = rs_BUF;
                }
                else
                {
                    rs_f_RI_enable = 0; //缓冲区满, 禁止接收   
                }
            }
            else
            {
                if (rs_shift_count < 9 ) //收到的是数据位 1 -- 7  
                {
                    rs_BUF = rs_BUF >> 1;
                    rs_BUF_bit7 = GSM_RXD;
                }
                else
                {   //收到停止位，继续检测 PC 机发出的下一个起始位    
                    soft_receive_init(); 
                }
            }
        }
        TF0 = 0;                  //清定时器中断标志   
    }
    else
    {
        /************************ 发送 ****************************/  
        if (rs_f_TI_enable == 1)
        {
            TL0 += rs_FULL_BIT_L;//下次中断在数据位的末尾时刻   
            TH0 = rs_FULL_BIT_H;
            rs_shift_count--;          //0:停止位末尾时刻到  
                                       //1:发送停止位  
                                       //2--9:发送数据位  
            if (rs_shift_count > 9)    //错误状态  
            {
                rs_shift_count = 9;
                rs_BUF = 0xFF;
            }
            if (rs_shift_count > 1)    //2--9:发送数据位  
            {
                ACC = rs_BUF;
                ACC = ACC >> 1;
                GSM_TXD = CY;
                rs_BUF = ACC;
            }
            else
            {
                if (rs_shift_count == 0) //0:停止位末尾时刻到  
                {
                    GSM_TXD = 1;
                    rs_f_TI = 1;       //已发送完毕一个字节  
                }
                else
                {
                    GSM_TXD = 1;        //1:发送停止位  
                }
            }
        }
    }
}
//由收转到发时，要先调用 soft_send_enable ()  
void rs_send_byte(U8_T SendByte)      //发送一个字节  
{
    while ( rs_f_TI == 0);             //等待发送完毕前一个字节  
    GSM_TXD = 1;
    TL0 = rs_START_BIT_L;        //下次中断在起始位的末尾时刻   
    TH0 = rs_START_BIT_H;
    rs_BUF = SendByte;
    rs_shift_count = 10;
    GSM_TXD = 0;                        //发送起始位  
    rs_f_TI = 0;                       //清已发送完毕一个字节的标志   
}


void initiate_MCU (void)               //系统初始化  
{
    soft_rs232_init();                 //串口初始化
    
    EA = 1;                            //开中断  
}


void DELAY_Us(U16_T loop);
#if 0
void Test_serial(void)
{
//首先发送 128 个字节 00H--7FH, 然后等待 PC 机发送的数据。当收到 128
//个字节后，立刻将收到的 128 个数据回发送给 PC 机，然后继续等待下一个
//数据块。
  
    U8_T i;
    initiate_MCU();                    //系统初始化  
   soft_send_enable ();               //允许发送，禁止接收  
   printf("enter\r\n");
	//while(1)
    for (i=0; i < rs_RECEIVE_MAX; i++ )
    {
	//	printf("send\r\n");
        rs_send_byte(i);
		DELAY_Us(1000);
    }
    while ( rs_f_TI == 0)  ;           // 等待最后一个字节发送完毕   
    while(1)
    {
	//	printf("enter\r\n");
        soft_receive_enable ();        //启动并开始接收，禁止发送  
        while (ReceivePoint < 20); // 等待接收缓冲区满  
	//	printf("rx0 %s\r\n",rs232buffer);
	//	printf("rx1 %d\r\n",(int)rs232buffer[5]);
        soft_send_enable ();           //允许发送，禁止接收  
        for (i=0; i < 20; i++ )
        {
            rs_send_byte(rs232buffer[i]);
        }
        while ( rs_f_TI == 0)  ;       //等待最后一个字节发送完毕*/
        ReceivePoint = 0;
    }
}
#endif
#if 1
const char GSM_Test[] = "AT\r";
void Test_serial(void)
{
	U8_T i;
	initiate_MCU();	 /* initial mcu */
	printf("test");
	/* initial gsm module */
	soft_send_enable();
	while(1)
	{
		/*send "at" */
		printf("at\r");
		rs_send_byte('A');
		rs_send_byte('T');
		rs_send_byte('\r');
		
		/*receive "at" */
		#if 0
		soft_receive_enable ();      
        while (ReceivePoint < 3); 
	//	printf("rx0 %s\r\n",rs232buffer);
		printf("rx1 %c %c %c \r\n",rs232buffer[0],rs232buffer[1],rs232buffer[2]);
     /*   soft_send_enable ();           //允许发送，禁止接收  
        for (i=0; i < 5; i++ )
        {
            rs_send_byte(rs232buffer[i]);
        }
        while ( rs_f_TI == 0)  ;       //等待最后一个字节发送完毕*/
       // ReceivePoint = 0;
		#endif
		//DELAY_Us(50000);DELAY_Us(50000);DELAY_Us(50000);DELAY_Us(50000);DELAY_Us(50000);
		DELAY_Us(50000);DELAY_Us(50000);DELAY_Us(50000);DELAY_Us(50000);DELAY_Us(50000);
	}

}

#endif


#else



/**********************************************************/
//   created by chelsea 2009/10/16

void UART0_PutString(char* str)
{
	U8_T i;
	U8_T len = strlen(str);
	for(i = 0;i < len; i++)
	{
		uart0_PutChar(str[i]);
	}
	uart0_PutChar('\0');
}


/*
char* UART0_GetString(void)
{
	
	U8_T i = 0;
	for(i = 0;i < 255;i++)
	{
	//	printf("str %d %c\r\n",(int)i,uart0_GetKey());
		if(uart0_GetKey() != '\0')
			str[i] = uart0_GetKey();
		else break;
	}
	str[++i] = '\0';
	printf("str %s\r\n",str);
	return str;
}*/


#endif 
