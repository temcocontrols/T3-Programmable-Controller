/*************************************************************************
用定时器 T0 或 T1 模拟串行口程序。
最高波特率(12 clock):
    本程序收、发波特率相同。
    11.059MHz -- 最高波特率 收: 9600,  最低波特率：300   
    30.000MHz -- 最高波特率 收: 28800  最低波特率：300   
    40.000MHz -- 最高波特率 收: 38400  最低波特率：300   
       ...
使用说明:
    1. 本程序使用一个定时器和任意 2 个 I/O 口模拟一个串行口。
    2. 1位起始位，8位数据位，1位停止位。发数据位时先发低位。
    3. 支持半双工通讯。收、发波特率相同。
    4. 应把定时器中断优先级设置为最高级。
    5. 本程序每接收一个字节后就把它放到一个队列缓冲区中(也可使用环行缓冲区),
       待缓冲区满后，将缓冲区中的内容原样发回。这是为了测试多字节连续收发的
       能力和简化程序。实际应用中应防止缓冲区溢出。  
    6. 由接收转换到发送时要先调用  soft_send_enable ()；
       由发送转换到接收时要先调用  soft_receive_enable ()。
    7. 发送最后一个字节后如果要立刻转为接收，必须等待最后一个字节后发送完毕
           while ( rs_f_TI == 0)  ;  // 等待最后一个字节发送完毕
**************************************************************************
编程说明:
----------------
发送:
    由接收转换到发送时要先调用 soft_send_enable (), 它为发送做初始化的工作。
以后就可以调用 rs_send_byte () 启动发送一个字节的过程。
    发送口平时为高电平，rs_send_byte ()函数使发送口变为低电平开始发送起始位; 
同时设置和启动定时器，为发送数据位在预定的时刻产生定时器中断。发送数据位和
停止位都在定时器的中断服务程序中进行。
    中断服务程序中处理 4 种情况：发送数据位、发送停止位、发送完毕、处理错误。
----------------
接收：
    由发送转换到接收时要先调用 soft_receive_enable (), 它为接收做初始化的工
作。定时器以 3 到 4 倍波特率的频率产生中断（参见 rs_TEST0 的定义）检测 PC
机发送的起始位。一旦检测到起始位，立刻把定时器产生中断的频率调整到与波特率
相同，准备在下一个定时器中断中接收第 1 个数据位。
    中断服务程序中处理以下情况：
    1. 收到的是 PC 机发送的起始位: 调整定时器产生中断的频率与波特率相同。
    2. 收到第 8 位数据位: 存储接收到的字节。
    3. 收到第 1--7 位数据位: 存储到收、发移位暂存器。
    4. 收到停止位: 调用 soft_receive_enable()，检测 PC 机发出的下一个起始位。 
    5. 处理出错的情况。
**************************************************************************/
#include	"reg80390.h"
#include   "types.h"
#include 	"stdio.h"

sfr16 DPTR = 0x82;

#define YES   1
#define NO    0
//定义使用哪个定时器, 只可定义一个   
//#define TIMER_0
#define TIMER_1
//定义串口收、发送管脚。  
sbit rs_TXD = P1^6;
sbit rs_RXD = P1^7;
//根据定时器确定参数  
#ifdef TIMER_0
    #define TMOD_AND_WORD   0xF0;
    #define TMOD_TIME_MODE  0x01;
    #define TMOD_COUNT_MODE 0x05;      //设置计数模式位  
    sbit  TCON_ENABLE_TIMER = TCON^4;
    sbit  TCON_TFx = TCON^5;           //中断标志位  
    sbit  IE_ETx = IE^1;               //中断允许位为 ET0  
    sbit  IP_PTx = IP^1;               //中断优先级  
    sfr rs_timerL = 0x8A;              //TL0 
    sfr rs_timerH = 0x8C;              //TH0 
#endif
#ifdef TIMER_1
    #define TMOD_AND_WORD   0x0F;
    #define TMOD_TIME_MODE  0x10;
    #define TMOD_COUNT_MODE 0x50;      //设置计数模式位  
    sbit  TCON_ENABLE_TIMER = TCON^6;  //
    sbit  TCON_TFx = TCON^7;           //中断标志位  
    sbit  IE_ETx = IE^3;               //中断允许位为 ET1   
    sbit  IP_PTx = IP^4;               //中断优先级  
    sfr rs_timerL = 0x8B;              //TL1
    sfr rs_timerH = 0x8D;              //TH1  
#endif
U8_T   bdata rs_BUF;                  //串行收、发时用的移位暂存器。   
sbit rs_BUF_bit7 = rs_BUF^7;        //移位暂存器的最高位。 
U8_T   rs_shift_count;                //移位计数器。  
U8_T bdata rsFlags;
sbit rs_f_TI        = rsFlags^0;    //0:正在发送; 1: 一个字符完毕  
sbit rs_f_RI_enable = rsFlags^1;   //0:禁止接收; 1:允许接收  
sbit rs_f_TI_enable = rsFlags^2;   //0:禁止发送; 1:允许发送  
//选择以下一个晶体频率
//#define Fosc 6000000                 //6MHz 
#define Fosc 11059200                  //11.059MHz 
//#define Fosc 12000000
//#define Fosc 18432000
//#define Fosc 20000000
//#define Fosc 24000000
//#define Fosc 30000000
//#define Fosc 40000000
//选择以下一个波特率:
//#efine Baud 300                      //11.059MHz时，baud 最低为 300   
//#define Baud 1200
//#define Baud 2400
//#define Baud 4800
#define Baud 9600
//#define Baud 14400
//#define Baud 19200
//#define Baud 28800
//#define Baud 38400
//#define Baud 57600
//收、发一位所需定时器计数   
#define rs_FULL_BIT0 2000 //((Fosc/12) / Baud)
#define rs_FULL_BIT (65536 - rs_FULL_BIT0)
#define rs_FULL_BIT_H rs_FULL_BIT >> 8        //收、发一位所需定时器计数高位   
#define rs_FULL_BIT_L (rs_FULL_BIT & 0x00FF)  //收、发一位所需定时器计数低位   
//检测起始位的时间间隔所需定时器计数    
#define rs_TEST0 rs_FULL_BIT0 / 4             //波特率较低时可以除以 3 或除以 2    
#define rs_TEST ((~rs_TEST0))
#define rs_TEST_H rs_TEST >> 8                //高位  
#define rs_TEST_L rs_TEST & 0x00FF            //低位  
//发送起始位所需定时器总计数   
#define rs_START_BIT 0xFFFF - 2000/*(Fosc/12/Baud)*/ + 0x28
#define rs_START_BIT_H rs_START_BIT >> 8      //发送起始位所需定时器计数高位    
#define rs_START_BIT_L rs_START_BIT & 0x00FF  //发送起始位所需定时器计数低位   
#define rs_RECEIVE_MAX   128                  //最大接收长度  
U8_T  rs232buffer[rs_RECEIVE_MAX];      //收、发缓冲区
U16_T ReceivePoint;                       //接收数据存储指针  
void soft_rs232_interrupt( void );
#ifdef TIMER_0
    void timer0 (void) interrupt 1 using 3
    {
        if (rs_RXD == 0 | rs_shift_count > 0)
        { soft_rs232_interrupt(); }
        else
        {
            rs_timerH = rs_TEST_H;
            rs_timerL = rs_TEST_L;
        }
    }
#endif
#ifdef TIMER_1
    void timer1 (void) interrupt 3 using 3
    {
        if (rs_RXD == 0 | rs_shift_count > 0)
        { soft_rs232_interrupt(); }
        else
        {
            rs_timerH = rs_TEST_H;
            rs_timerL = rs_TEST_L;
        }
    }
#endif
/***************************************/
void soft_rs232_init (void)            //串口初始化  
{
    TCON_ENABLE_TIMER = 0;             //停止定时器  
    TMOD &= TMOD_AND_WORD;
    TMOD |= TMOD_TIME_MODE;
    rs_RXD = 1;                        //接收脚置成高电平  
    rs_TXD = 1;                        //发射脚置成高电平  
    IP_PTx = 1;                        //置中断优先级为高  
    IE_ETx = 1;                        //允许定时器中断    
}
void soft_receive_init()               //监测起始位  
{
    TCON_ENABLE_TIMER = 0;             //停止定时器  
    rs_timerH = rs_TEST_H;
    rs_timerL = rs_TEST_L;
    rs_shift_count = 0;
    TCON_ENABLE_TIMER = 1;             //启动定时器  
}

void soft_receive_enable()             //允许接收  
{
    rs_f_RI_enable = 1;                //允许接收  
    rs_f_TI_enable = 0;                //禁止发送   
    soft_receive_init();               //监测起始位, RXD 下降沿触发接收字节过程.     
}
void soft_send_enable (void)        //允许发送  
{
    TCON_ENABLE_TIMER = 0;             //停止定时器  
    rs_f_TI_enable = 1;                //允许发送  
    rs_f_RI_enable = 0;                //禁止接收  
    rs_shift_count = 0;                //清移位计数器  
    rs_f_TI   = 1;                     //发送一个字符完毕标志  
    TCON_ENABLE_TIMER = 1;             //启动定时器
}
void soft_rs232_interrupt( void )
{
    /************************ 接收 ****************************/
    if (rs_f_RI_enable == 1)
    {
        if (rs_shift_count == 0)        //移位计数器==0, 表示检测到起始位的起点   
        {
            if ( rs_RXD == 1 )
            {
                soft_receive_enable (); //起始位错, 从新开始   
            }
            else
            {
                //下次中断在数据位或停止位中的某时刻发生    
                rs_timerL += rs_FULL_BIT_L + 0x10; 
                rs_timerH = rs_FULL_BIT_H;
                rs_shift_count++;              
                rs_BUF = 0;             //清移位缓冲变量   
            }
        }
        else
        {
            rs_timerL += rs_FULL_BIT_L; //下次中断在数据位或停止位中发生    
            rs_timerH = rs_FULL_BIT_H;
                                       
            rs_shift_count++;           //2--9:数据位 10:停止位 
                                       
            if ( rs_shift_count == 9)
            {
                rs_BUF = rs_BUF >> 1;   //接收第8位   
                rs_BUF_bit7 = rs_RXD;
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
                    rs_BUF_bit7 = rs_RXD;
                }
                else
                {   //收到停止位，继续检测 PC 机发出的下一个起始位    
                    soft_receive_init(); 
                }
            }
        }
        TCON_TFx = 0;                  //清定时器中断标志   
    }
    else
    {
        /************************ 发送 ****************************/  
        if (rs_f_TI_enable == 1)
        {
            rs_timerL += rs_FULL_BIT_L;//下次中断在数据位的末尾时刻   
            rs_timerH = rs_FULL_BIT_H;
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
                rs_TXD = CY;
                rs_BUF = ACC;
            }
            else
            {
                if (rs_shift_count == 0) //0:停止位末尾时刻到  
                {
                    rs_TXD = 1;
                    rs_f_TI = 1;       //已发送完毕一个字节  
                }
                else
                {
                    rs_TXD = 1;        //1:发送停止位  
                }
            }
        }
    }
}
//由收转到发时，要先调用 soft_send_enable ()  
void rs_send_byte(U8_T SendByte)      //发送一个字节  
{
    while ( rs_f_TI == 0);             //等待发送完毕前一个字节  
    rs_TXD = 1;
    rs_timerL = rs_START_BIT_L;        //下次中断在起始位的末尾时刻   
    rs_timerH = rs_START_BIT_H;
    rs_BUF = SendByte;
    rs_shift_count = 10;
    rs_TXD = 0;                        //发送起始位  
    rs_f_TI = 0;                       //清已发送完毕一个字节的标志   
}
void initiate_MCU (void)               //系统初始化  
{
    soft_rs232_init();                 //串口初始化  
    EA = 1;                            //开中断  
}
void main (void)
{
//首先发送 128 个字节 00H--7FH, 然后等待 PC 机发送的数据。当收到 128
//个字节后，立刻将收到的 128 个数据回发送给 PC 机，然后继续等待下一个
//数据块。
  
    U8_T i;
    initiate_MCU();                    //系统初始化  
    soft_send_enable ();               //允许发送，禁止接收  
    for (i=0; i < rs_RECEIVE_MAX; i++ )
    {
        rs_send_byte(i);
    }
    while ( rs_f_TI == 0)  ;           // 等待最后一个字节发送完毕   
    while(1)
    {
        soft_receive_enable ();        //启动并开始接收，禁止发送  
        while (ReceivePoint < rs_RECEIVE_MAX); // 等待接收缓冲区满  
        soft_send_enable ();           //允许发送，禁止接收  
        for (i=0; i < rs_RECEIVE_MAX; i++ )
        {
            rs_send_byte(rs232buffer[i]);
        }
        while ( rs_f_TI == 0)  ;       //等待最后一个字节发送完毕
        ReceivePoint = 0;
    }
}

