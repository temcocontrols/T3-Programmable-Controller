#include <string.h>
#include "stm32f10x.h"
#include "usart.h"
#include "delay.h"
#include "led.h"
//#include "key.h"
#include "24cxx.h"
#include "spi.h"
#include "lcd.h"
#include "touch.h"
#include "flash.h"
#include "stmflash.h"
//#include "sdcard.h"
#include "mmc_sd.h"
#include "dma.h"
#include "vmalloc.h"
#include "enc28j60.h"
#include "timerx.h"
#include "uip.h"
#include "uip_arp.h"
#include "tapdev.h"
#include "usb_app.h"
//#include "ai.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "modbus.h"
#include "define.h"
#include "inputs.h"
#include "../output/output.h"
#include "dlmstp.h"
//#include "rs485.h"
#include "datalink.h"
#include "config.h"
#include "handlers.h"
#include "device.h"	
#include "registerlist.h"
#include "../filter/filter.h"
#include "../KEY/key.h"
#include "bacnet.h"


 #ifdef T322AI
void vSTORE_EEPTask(void *pvParameters ) ;
#endif
static void vLED0Task( void *pvParameters );
static void vCOMMTask( void *pvParameters );

//static void vUSBTask( void *pvParameters );
static void vINPUTSTask( void *pvParameters );
void vNETTask( void *pvParameters );
#ifdef T38AI8AO6DO
void vKEYTask( void *pvParameters );
void vOUTPUTSTask( void *pvParameters );
#endif
static void vMSTP_TASK(void *pvParameters ) ;
void uip_polling(void);
void EEP_Dat_Init(void) ;
#define	BUF	((struct uip_eth_hdr *)&uip_buf[0])	
	
extern u16 Test[50];

u8 ram_test1  ;
u8 ram_test2  ;

uint8_t  PDUBuffer[MAX_APDU];

u8 global_key = KEY_NON;

static void debug_config(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
}

int main(void)
{
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x8008000);
	debug_config();
	//ram_test = 0 ;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD , ENABLE);
 	delay_init(72);	
//	uart1_init(38400);
//	modbus.baudrate = 38400 ;
	//KEY_Init();
	EEP_Dat_Init();
//	beeper_gpio_init();
//	beeper_on();
	delay_ms(1000);
//	beeper_off();
	//Lcd_Initial();
	SPI1_Init();
	SPI2_Init();
	mem_init(SRAMIN);
//	TIM3_Int_Init(5000, 7199);
//	TIM6_Int_Init(100, 7199);
	
	
	#ifdef  T322AI
	xTaskCreate( vSTORE_EEPTask, ( signed portCHAR * ) "STOREEEP", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL );
	#endif
	xTaskCreate( vLED0Task, ( signed portCHAR * ) "LED0", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL );
 	xTaskCreate( vCOMMTask, ( signed portCHAR * ) "COMM", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL );
	xTaskCreate( vNETTask, ( signed portCHAR * ) "NET",  configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 4, NULL );
//	xTaskCreate( vUSBTask, ( signed portCHAR * ) "USB", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL );
	//xTaskCreate( vUSBTask, ( signed portCHAR * ) "USB", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL );
	/* Start the scheduler. */
	xTaskCreate( vINPUTSTask, ( signed portCHAR * ) "INPUTS", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL );
 	#ifdef T38AI8AO6DO
	xTaskCreate( vOUTPUTSTask, ( signed portCHAR * ) "OUTPUTS", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL );
	xTaskCreate( vKEYTask, ( signed portCHAR * ) "KEY", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL );
	#endif
	xTaskCreate( vMSTP_TASK, ( signed portCHAR * ) "MSTP", configMINIMAL_STACK_SIZE + 1024  , NULL, tskIDLE_PRIORITY + 4, NULL );
	vTaskStartScheduler();
}

//u8 dim_timer[14];

void vLED0Task( void *pvParameters )
{
	static u8 table_led_count =0 ;
	
	LED_Init();
	for( ;; )
	{
		table_led_count++ ;
		if(table_led_count>= 3)
		{
			table_led_count= 0 ;
			tabulate_LED_STATE();
		}
		refresh_led();
		delay_ms(10);
	}
	
//	for(;;)
//	{
//		
//		table_led_count++ ;
//		if(table_led_count>= 10)
//		{
//			table_led_count= 0 ;
//			tabulate_LED_STATE();
//		}	
//		if(count == 0)
//		{
//			//bank2 disable
//			GPIO_SetBits(GPIOE, GPIO_Pin_14);
//			for(i = 0; i < 14; i++)
//			{
//				dim_timer[i] = dim_timer_setting[i];
//			}
//		}
//		else if(count == 8)
//		{
//			//bank1 disable
//			GPIO_SetBits(GPIOE, GPIO_Pin_15);
//			for(i = 0; i < 14; i++)
//			{
//				dim_timer[i] = dim_timer_setting[i+14];
//			}
//		}
//		
//		for(i = 0; i < 14; i++)
//		{
//			if(dim_timer[i])
//			{
//				dim_timer[i]--;
//				GPIO_ResetBits(GPIOE, (GPIO_Pin_0<<i));
//			}
//			else
//			{
//				GPIO_SetBits(GPIOE, (GPIO_Pin_0<<i));
//			}
//		}			
//		// wirte output to GPIO
//		if(count == 0)
//		{
//			//bank2 enable
//			GPIO_ResetBits(GPIOE, GPIO_Pin_15);
//		}
//		else if(count == 8)
//		{
//			//bank1 enable
//			GPIO_ResetBits(GPIOE, GPIO_Pin_14);
//		}
//		
//		count++;
//		count %= 16;		
//		delay_us(1500);
//	}
	
}
void vCOMMTask(void *pvParameters )
{
	modbus_init();
	for( ;; )
	{
		
		if(modbus.protocal == MAIN_MODBUS)
		{
			if(USART_RX_BUF[0] == 0x55 && USART_RX_BUF[1] == 0xff && USART_RX_BUF[2] == 0x01 && USART_RX_BUF[5] == 0x00 && USART_RX_BUF[6] == 0x00)
			{// mstp
				Test[0]++;
				// tbd:  
				
				
				
				
//				modbus.protocal = MAIN_MSTP;
//				Recievebuf_Initialize(0);	
//				serial_restart();
			}
			if (dealwithTag)
			{  
				dealwithTag--;
				if(dealwithTag == 1)//&& !Serial_Master )	
				dealwithData();
			}
			if(serial_receive_timeout_count > 0)  
			{
				serial_receive_timeout_count --; 
				if(serial_receive_timeout_count == 0)
				{
					serial_restart();
				}
			}
		}
		delay_ms(5) ;
	}
	
}

 #ifdef T322AI
void vSTORE_EEPTask(void *pvParameters )
{
	uint8_t  loop ;
	for( ;; )
	{
		for(loop=0; loop<11; loop++)
		{
			if(data_change & (1<<loop))
			{
				AT24CXX_WriteOneByte(EEP_PLUSE0_HI_HI+4*loop, modbus.pulse[loop].quarter[0]);
				AT24CXX_WriteOneByte(EEP_PLUSE0_HI_LO+4*loop, modbus.pulse[loop].quarter[1]);
				AT24CXX_WriteOneByte(EEP_PLUSE0_LO_HI+4*loop, modbus.pulse[loop].quarter[2]);
				AT24CXX_WriteOneByte(EEP_PLUSE0_LO_LO+4*loop, modbus.pulse[loop].quarter[3]);
				data_change &= ~(1<<loop) ;
			}
		}
		delay_ms(1000) ;				//if pulse changes ,store the data
	}
	
}
#endif

void vINPUTSTask( void *pvParameters )
{
	u8 i ;
	#ifndef T3PT12
	for(i=0; i<MAX_AI_CHANNEL; i++)
	{
		AD_Value[i] = 0 ;	
	}
	#endif
	inputs_init();
	for( ;; )
	{
		
		inputs_scan();
		
		delay_ms(20);
	}	
}
#ifdef T38AI8AO6DO
//void TIM3_PWM_Init(u16 arr, u16 psc)
//{
//	GPIO_InitTypeDef GPIO_InitStructure;
//	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
//	TIM_OCInitTypeDef TIM_OCInitStructure;
//	
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
//	GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE); //TIM3_CH1->PC6, TIM3_CH2->PC7, TIM3_CH3->PC8, TIM3_CH4->PC9
//	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;  //TIM3_CH1-4
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOC, &GPIO_InitStructure);
//	
//	TIM_TimeBaseStructure.TIM_Period = arr;
//	TIM_TimeBaseStructure.TIM_Prescaler = psc;
//	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
//	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
//	
//	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
//	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
//	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
//	TIM_OC1Init(TIM3, &TIM_OCInitStructure);
//	TIM_OC2Init(TIM3, &TIM_OCInitStructure);
//	TIM_OC3Init(TIM3, &TIM_OCInitStructure);
//	TIM_OC4Init(TIM3, &TIM_OCInitStructure);
//	
//	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
//	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
//	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
//	TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);
//	
//	TIM_Cmd(TIM3, ENABLE);									  
//} 


void vOUTPUTSTask( void *pvParameters )
{

	output_init();
//	TIM3_PWM_Init(1000, 71);
//	TIM_SetCompare1(TIM3, 800);
//	TIM_SetCompare2(TIM3, 800);
//	TIM_SetCompare3(TIM3, 800);
//	TIM_SetCompare4(TIM3, 800);
	for( ;; )
	{
		output_refresh();
		update_digit_output();
		delay_ms(100);
	}	
}

void vKEYTask( void *pvParameters )
{
	KEY_IO_Init();
	for( ;; )
	{
		KEY_Status_Scan();
		delay_ms(100);
	}	
}

#endif

u32 Instance = 0x0c;
void Inital_Bacnet_Server(void)
{
	
	Device_Init();
	Device_Set_Object_Instance_Number(Instance);  


#if  BAC_COMMON   

	AVS = 10;
	AIS = 22;
	AOS = 4;//BIG_MAX_AVS;
//	BIS = BIG_MAX_DIS;
	BOS = 5;//BIG_MAX_DOS;
//	memset(AV_Present_Value,0,MAX_AVS );
//	memset(AI_Present_Value,0,MAX_AIS );
//	memset(AO_Present_Value,0,MAX_AOS * BACNET_MAX_PRIORITY);
//	memset(BI_Present_Value,0,MAX_BIS);
//	memset(BO_Present_Value,0,MAX_BOS * BACNET_MAX_PRIORITY );
//	Binary_Output_Init();

//	Analog_Output_Init();


#endif	
}
void vMSTP_TASK(void *pvParameters )
{
	uint16_t pdu_len = 0; 
	BACNET_ADDRESS  src;
	Inital_Bacnet_Server();
	dlmstp_init(NULL);
	Recievebuf_Initialize(0);
	for (;;)
    {
			
		if(modbus.protocal == MAIN_MSTP)
		{
					pdu_len = datalink_receive(&src, &PDUBuffer[0], sizeof(PDUBuffer), 0,	BAC_MSTP);
					if(pdu_len) 
						{
				      npdu_handler(&src, &PDUBuffer[0], pdu_len,BAC_MSTP);	
						} 
						
			}
			delay_ms(5);
	}
	
}

extern u32 count_out;
extern u8 buffer_out[64];
void vUSBTask( void *pvParameters )
{
//	u8 i;
	for( ;; )
	{
//		if((count_out != 0) && (bDeviceState == CONFIGURED))
//		{
////			USB_To_USART_Send_Data(&buffer_out[0], count_out);
//			for(i = 0; i < count_out; i++)
//			{

//			}
//			count_out = 0;
//		}
		delay_ms(20);

	}
}

//u8 myMAC[] = {0x00, 0x0e, 0x00, 0x25, 0x36, 0x64}; 
//void vNETTask( void *pvParameters )
//{
//	//uip_ipaddr_t ipaddr;
//	
//	//u8 tcp_server_tsta = 0XFF;
//	//u8 tcp_client_tsta = 0XFF;
//	//printf("Temco ARM Demo\r\n");
//	delay_ms(500);
//	
//	while(tapdev_init())	//初始化ENC28J60错误
//	{								   
//	//	printf("ENC28J60 Init Error!\r\n");
//		delay_ms(200);
//	};		
//	//printf("ENC28J60 Init OK!\r\n");
////	uip_init();							//uIP初始化
////	
////	uip_ipaddr(ipaddr, 192, 168, 0, 163);	//设置本地设置IP地址
////	uip_sethostaddr(ipaddr);					    
////	uip_ipaddr(ipaddr, 192, 168, 0, 4); 	//设置网关IP地址(其实就是你路由器的IP地址)
////	uip_setdraddr(ipaddr);						 
////	uip_ipaddr(ipaddr, 255, 255, 255, 0);	//设置网络掩码
////	uip_setnetmask(ipaddr);

////	printf("IP:192.168.0.163\r\n"); 
////	printf("MASK:255.255.255.0\r\n"); 
////	printf("GATEWAY:192.168.0.4\r\n"); 	
////	
////	uip_listen(HTONS(1200));			//监听1200端口,用于TCP Server
////	uip_listen(HTONS(80));				//监听80端口,用于Web Server
////	tcp_client_reconnect();	   		    //尝试连接到TCP Server端,用于TCP Client
//	
//    for( ;; )
//	{
//		uip_polling();	//处理uip事件，必须插入到用户程序的循环体中 
//		
////		if(tcp_server_tsta != tcp_server_sta)		//TCP Server状态改变
////		{															 
////			if(tcp_server_sta & (1 << 7))
////				printf("TCP Server Connected   \r\n");
////			else
////				printf("TCP Server Disconnected\r\n"); 
////			
//// 			if(tcp_server_sta & (1 << 6))			//收到新数据
////			{
////   			printf("TCP Server RX:%s\r\n", tcp_server_databuf);//打印数据
////				tcp_server_sta &= ~(1 << 6);		//标记数据已经被处理	
////			}
////			tcp_server_tsta = tcp_server_sta;
////		}
////		
////		if(global_key == KEY_1)						//TCP Server 请求发送数据
////		{
////			global_key = KEY_NON;
////			if(tcp_server_sta & (1 << 7))			//连接还存在
////			{
////				sprintf((char*)tcp_server_databuf, "TCP Server OK\r\n");	 
////				tcp_server_sta |= 1 << 5;			//标记有数据需要发送
////			}
////		}
////		
////		if(tcp_client_tsta != tcp_client_sta)		//TCP Client状态改变
////		{
////			if(tcp_client_sta & (1 << 7))
////				printf("TCP Client Connected   \r\n");
////			else
////				printf("TCP Client Disconnected\r\n");
////			
//// 			if(tcp_client_sta & (1 << 6))			//收到新数据
////			{
////    			printf("TCP Client RX:%s\r\n", tcp_client_databuf);//打印数据
////				tcp_client_sta &= ~(1 << 6);		//标记数据已经被处理	
////			}
////			tcp_client_tsta = tcp_client_sta;
////		}
////		
////		if(global_key == KEY_2)						//TCP Client 请求发送数据
////		{
////			global_key = KEY_NON;
////			if(tcp_client_sta & (1 << 7))			//连接还存在
////			{
////				sprintf((char*)tcp_client_databuf, "TCP Client OK\r\n");	 
////				tcp_client_sta |= 1 << 5;			//标记有数据需要发送
////			}
////		}
//		
////		if(global_key == KEY_3)
////		{
////			global_key = KEY_NON;
////			printf("global_key == KEY_3\r\n");
////			mf_mount(0);
////			mf_scan_files("0:");
////			mf_showfree("0:");
////		}
//		
//		delay_ms(5);
//    }
//}



//uip事件处理函数
//必须将该函数插入用户主循环,循环调用.
void uip_polling(void)
{
	u8 i;
	static struct timer periodic_timer, arp_timer;
	static u8 timer_ok = 0;	 
	if(timer_ok == 0)		//仅初始化一次
	{
		timer_ok = 1;
		timer_set(&periodic_timer, CLOCK_SECOND / 2); 	//创建1个0.5秒的定时器 
		timer_set(&arp_timer, CLOCK_SECOND * 10);	   	//创建1个10秒的定时器 
	}
	
	uip_len = tapdev_read();							//从网络设备读取一个IP包,得到数据长度.uip_len在uip.c中定义
	if(uip_len > 0)							 			//有数据
	{   
		//处理IP数据包(只有校验通过的IP包才会被接收) 
		if(BUF->type == htons(UIP_ETHTYPE_IP))			//是否是IP包? 
		{
			uip_arp_ipin();								//去除以太网头结构，更新ARP表
			uip_input();   								//IP包处理
			
			//当上面的函数执行后，如果需要发送数据，则全局变量 uip_len > 0
			//需要发送的数据在uip_buf, 长度是uip_len  (这是2个全局变量)		    
			if(uip_len > 0)								//需要回应数据
			{
				uip_arp_out();							//加以太网头结构，在主动连接时可能要构造ARP请求
				tapdev_send();							//发送数据到以太网
			}
		}
		else if (BUF->type == htons(UIP_ETHTYPE_ARP))	//处理arp报文,是否是ARP请求包?
		{
			uip_arp_arpin();
			
 			//当上面的函数执行后，如果需要发送数据，则全局变量uip_len>0
			//需要发送的数据在uip_buf, 长度是uip_len(这是2个全局变量)
 			if(uip_len > 0)
				tapdev_send();							//需要发送数据,则通过tapdev_send发送	 
		}
	}
	else if(timer_expired(&periodic_timer))				//0.5秒定时器超时
	{
		timer_reset(&periodic_timer);					//复位0.5秒定时器 
		
		//轮流处理每个TCP连接, UIP_CONNS缺省是40个  
		for(i = 0; i < UIP_CONNS; i++)
		{
			//uip_periodic(i);							//处理TCP通信事件
			
	 		//当上面的函数执行后，如果需要发送数据，则全局变量uip_len>0
			//需要发送的数据在uip_buf, 长度是uip_len (这是2个全局变量)
	 		if(uip_len > 0)
			{
				uip_arp_out();							//加以太网头结构，在主动连接时可能要构造ARP请求
				tapdev_send();							//发送数据到以太网
			}
		}
		
#if UIP_UDP	//UIP_UDP 
		//轮流处理每个UDP连接, UIP_UDP_CONNS缺省是10个
		for(i = 0; i < UIP_UDP_CONNS; i++)
		{
			uip_udp_periodic(i);						//处理UDP通信事件
			//当上面的函数执行后，如果需要发送数据，则全局变量uip_len>0
			//需要发送的数据在uip_buf, 长度是uip_len (这是2个全局变量)
			if(uip_len > 0)
			{
				uip_arp_out();							//加以太网头结构，在主动连接时可能要构造ARP请求
				tapdev_send();							//发送数据到以太网
			}
		}
#endif 
		//每隔10秒调用1次ARP定时器函数 用于定期ARP处理,ARP表10秒更新一次，旧的条目会被抛弃
		if(timer_expired(&arp_timer))
		{
			timer_reset(&arp_timer);
			uip_arp_timer();
		}
	}
}


void EEP_Dat_Init(void)
{
		u8 loop ;
		u8 temp1,temp2; 
		AT24CXX_Init();
		modbus.serial_Num[0] = 100;//AT24CXX_ReadOneByte(EEP_SERIALNUMBER_LOWORD);
		modbus.serial_Num[1] = 0;//AT24CXX_ReadOneByte(EEP_SERIALNUMBER_LOWORD+1);
		modbus.serial_Num[2] = 0;//AT24CXX_ReadOneByte(EEP_SERIALNUMBER_HIWORD);
		modbus.serial_Num[3] = 0;//AT24CXX_ReadOneByte(EEP_SERIALNUMBER_HIWORD+1);
		if((modbus.serial_Num[0]==0xff)&&(modbus.serial_Num[1]== 0xff)&&(modbus.serial_Num[2] == 0xff)&&(modbus.serial_Num[3] == 0xff))
		{
					modbus.serial_Num[0] = 0 ;
					modbus.serial_Num[1] = 0 ;
					modbus.serial_Num[2] = 0 ;
					modbus.serial_Num[3] = 0 ;
					AT24CXX_WriteOneByte(EEP_SERIALNUMBER_LOWORD, modbus.serial_Num[0]);
					AT24CXX_WriteOneByte(EEP_SERIALNUMBER_LOWORD+1, modbus.serial_Num[1]);
					AT24CXX_WriteOneByte(EEP_SERIALNUMBER_LOWORD+2, modbus.serial_Num[2]);
					AT24CXX_WriteOneByte(EEP_SERIALNUMBER_LOWORD+3, modbus.serial_Num[3]);
		}
		AT24CXX_WriteOneByte(EEP_VERSION_NUMBER_LO, SOFTREV&0XFF);
		AT24CXX_WriteOneByte(EEP_VERSION_NUMBER_HI, (SOFTREV>>8)&0XFF);
		modbus.address = AT24CXX_ReadOneByte(EEP_ADDRESS);
		if((modbus.address == 255)||(modbus.address == 0))
		{
				modbus.address = 254 ;
				AT24CXX_WriteOneByte(EEP_ADDRESS, modbus.address);
		}
		modbus.product = AT24CXX_ReadOneByte(EEP_PRODUCT_MODEL);
		modbus.product = 43;
		if((modbus.product == 255)||(modbus.product == 0))
		{
			modbus.product = PRODUCT_ID ;
			AT24CXX_WriteOneByte(EEP_PRODUCT_MODEL, modbus.product);
		}
		modbus.hardware_Rev = AT24CXX_ReadOneByte(EEP_HARDWARE_REV);
		if((modbus.hardware_Rev == 255)||(modbus.hardware_Rev == 0))
		{
				modbus.hardware_Rev = HW_VER ;
				AT24CXX_WriteOneByte(EEP_HARDWARE_REV, modbus.hardware_Rev);
		}
		modbus.update = AT24CXX_ReadOneByte(EEP_UPDATE_STATUS);
		modbus.SNWriteflag = AT24CXX_ReadOneByte(EEP_SERIALNUMBER_WRITE_FLAG);
		
		modbus.baud = AT24CXX_ReadOneByte(EEP_BAUDRATE);
		if(modbus.baud > 3) 
		{	
				modbus.baud = 1 ;
				AT24CXX_WriteOneByte(EEP_BAUDRATE, modbus.baud);
		}
		switch(modbus.baud)
				{
					case 0:
						modbus.baudrate = BAUDRATE_9600 ;
						uart1_init(BAUDRATE_9600);
				
						SERIAL_RECEIVE_TIMEOUT = 6;
					break ;
					case 1:
						modbus.baudrate = BAUDRATE_19200 ;
						uart1_init(BAUDRATE_19200);	
						SERIAL_RECEIVE_TIMEOUT = 3;
					break;
					case 2:
						modbus.baudrate = BAUDRATE_38400 ;
						uart1_init(BAUDRATE_38400);
						SERIAL_RECEIVE_TIMEOUT = 2;
					break;
					case 3:
						modbus.baudrate = BAUDRATE_57600 ;
						uart1_init(BAUDRATE_57600);	
						SERIAL_RECEIVE_TIMEOUT = 1;
					break;
					default:
					break ;				
				}
//				modbus.protocal= AT24CXX_ReadOneByte(EEP_MODBUS_COM_CONFIG);
//				if((modbus.protocal == 255)||(modbus.protocal!=MODBUS)||(modbus.protocal!=BAC_MSTP ))
//				{
//					modbus.protocal = MODBUS ;
//					AT24CXX_WriteOneByte(EEP_MODBUS_COM_CONFIG, modbus.protocal);
//				}
//				modbus.protocal= AT24CXX_ReadOneByte(EEP_MODBUS_COM_CONFIG);
//				if(modbus.protocal >2)
//				{
//					modbus.protocal = 0 ;
//					AT24CXX_WriteOneByte(EEP_MODBUS_COM_CONFIG, modbus.protocal);
//				}
				modbus.protocal = 1 ;
		#ifndef T3PT12
		for(loop = 0; loop < MAX_AI_CHANNEL; loop++)
		{
			temp1 = AT24CXX_ReadOneByte(EEP_CUSTOMER_REANGE0_HI+4*loop) ;	
			temp2 = AT24CXX_ReadOneByte(EEP_CUSTOMER_REANGE0_HI+4*loop+1) ;
			if((temp1 == 0xff)&&(temp2 == 0xff))
			{
				modbus.customer_range_hi[loop] = 100 ;
				temp1 = (modbus.customer_range_hi[loop]>>8)&0xff ;
				temp2 = modbus.customer_range_hi[loop]&0xff ;
				AT24CXX_WriteOneByte(EEP_CUSTOMER_REANGE0_HI+4*loop, temp1);
				AT24CXX_WriteOneByte(EEP_CUSTOMER_REANGE0_HI+4*loop+1, temp2);
			}
			else
			{
				modbus.customer_range_hi[loop] = (temp1<<8)|temp2 ;			
			}
			
			temp1 = AT24CXX_ReadOneByte(EEP_CUSTOMER_REANGE0_LO+4*loop) ;	
			temp2 = AT24CXX_ReadOneByte(EEP_CUSTOMER_REANGE0_LO+4*loop+1) ;
			if((temp1 == 0xff)&&(temp2 == 0xff))
			{
				modbus.customer_range_lo[loop] = 0 ;
				temp1 = (modbus.customer_range_lo[loop]>>8)&0xff ;
				temp2 = modbus.customer_range_lo[loop]&0xff ;
				AT24CXX_WriteOneByte(EEP_CUSTOMER_REANGE0_LO+4*loop, temp1);
				AT24CXX_WriteOneByte(EEP_CUSTOMER_REANGE0_LO+4*loop+1, temp2);
			}
			else
			{
				modbus.customer_range_lo[loop] = (temp1<<8)|temp2 ;			
			}
			

		}
#endif		
		#ifdef T322AI
		for(loop = 0; loop < 22; loop++)
		{
				modbus.filter_value[loop] = AT24CXX_ReadOneByte(EEP_AI_FILTER0+loop) ;
				if((modbus.filter_value[loop] == 0xff)||(modbus.filter_value[loop] > 20))
				{
						modbus.filter_value[loop] = 5 ;
						AT24CXX_WriteOneByte(EEP_AI_FILTER0+loop, modbus.filter_value[loop]);
				}
				modbus.range[loop] = AT24CXX_ReadOneByte(EEP_AI_RANGE0+loop) ;
				if(modbus.range[loop]> 20)
				{
						modbus.range[loop] = 0 ;
						AT24CXX_WriteOneByte(EEP_AI_RANGE0+loop, modbus.range[loop]);
				}
				if(loop < 11)
				{
						 pulse_set(loop);
				}
				temp1 = AT24CXX_ReadOneByte(EEP_AI_OFFSET0+2*loop) ;
				temp2 = AT24CXX_ReadOneByte(EEP_AI_OFFSET0+2*loop +1) ;
				if((temp1== 0xff)&&(temp2 == 0xff))
				{
						modbus.offset[loop] = 500 ;
						temp1 = (modbus.offset[loop]>>8)&0xff ;
						temp2 = (modbus.offset[loop])&0xff ;
						AT24CXX_WriteOneByte(EEP_AI_OFFSET0+2*loop, temp1);
						AT24CXX_WriteOneByte(EEP_AI_OFFSET0+2*loop+1, temp2);
				}
				else
				{
						modbus.offset[loop] = (temp1<<8)|temp2;
				}
				
				modbus.pulse[loop].quarter[0] = AT24CXX_ReadOneByte(EEP_PLUSE0_HI_HI+4*loop) ;
				modbus.pulse[loop].quarter[1] = AT24CXX_ReadOneByte(EEP_PLUSE0_HI_LO+4*loop);
				modbus.pulse[loop].quarter[2] = AT24CXX_ReadOneByte(EEP_PLUSE0_LO_HI+4*loop) ;
				modbus.pulse[loop].quarter[3] = AT24CXX_ReadOneByte(EEP_PLUSE0_LO_LO+4*loop);
				if((modbus.pulse[loop].quarter[0] == 0xff)&&(modbus.pulse[loop].quarter[1]== 0xff)&&(modbus.pulse[loop].quarter[2]== 0xff)&& (modbus.pulse[loop].quarter[3] == 0xff))
				{
					modbus.pulse[loop].word = 0 ;
					AT24CXX_WriteOneByte(EEP_PLUSE0_HI_HI+4*loop, modbus.pulse[loop].quarter[0]);
					AT24CXX_WriteOneByte(EEP_PLUSE0_HI_LO+4*loop, modbus.pulse[loop].quarter[1]);
					AT24CXX_WriteOneByte(EEP_PLUSE0_LO_HI+4*loop, modbus.pulse[loop].quarter[2]);
					AT24CXX_WriteOneByte(EEP_PLUSE0_LO_LO+4*loop, modbus.pulse[loop].quarter[3]);
				}
				
				temp1 = AT24CXX_ReadOneByte(EEP_CUSTOMER_RANGE0_ENABLE+loop) ;
				if(temp1 == 0xff)
				{
					modbus.customer_enable[loop] = 0 ; 
					AT24CXX_WriteOneByte(EEP_CUSTOMER_RANGE0_ENABLE+loop, modbus.customer_enable[loop]);
				}
				else
				{
					modbus.customer_enable[loop] = temp1;
				}
		
		}
		#endif
		#ifdef T38AI8AO6DO
		for(loop=0; loop<MAX_AO; loop++)
		{				
				temp1	= AT24CXX_ReadOneByte(EEP_AO_CHANNLE0+2*loop);
				temp2	= AT24CXX_ReadOneByte(EEP_AO_CHANNLE0+1+2*loop);
				if((temp1== 0xff)&&(temp2 == 0xff))
				{
					modbus.analog_output[loop] = 0 ;
					AT24CXX_WriteOneByte(EEP_AO_CHANNLE0+2*loop+1, (modbus.analog_output[loop]>>8)&0xff);
					AT24CXX_WriteOneByte(EEP_AO_CHANNLE0+2*loop, modbus.analog_output[loop]);
				}
				else
				{
						modbus.analog_output[loop] = (temp2<<8)| temp1 ;		
				}
		}
		for(loop=0; loop<MAX_DO; loop++)
		{
			modbus.digit_output[loop] = AT24CXX_ReadOneByte(EEP_DO_CHANNLE0+loop);
			if(modbus.digit_output[loop]> 1) 
			{
					modbus.digit_output[loop] = 1 ;
					AT24CXX_WriteOneByte(EEP_DO_CHANNLE0+loop, modbus.analog_output[loop]);
			}
		}
		for(loop=0; loop<MAX_AI_CHANNEL; loop++)
		{
				modbus.filter_value[loop] = AT24CXX_ReadOneByte(EEP_AI_FILTER0+loop);
				if(modbus.filter_value[loop] == 0xff)
				{
						modbus.filter_value[loop] = 5 ;
						AT24CXX_WriteOneByte(EEP_AI_FILTER0+loop, modbus.filter_value[loop]);
				}
				modbus.range[loop] = AT24CXX_ReadOneByte(EEP_AI_RANGE0+loop) ;
				if(modbus.range[loop] == 0xff)
				{
						modbus.range[loop] = 0 ;
						AT24CXX_WriteOneByte(EEP_AI_RANGE0+loop, modbus.range[loop]);
				}
				
				temp1 = AT24CXX_ReadOneByte(EEP_AI_OFFSET0+2*loop) ;
				temp2 = AT24CXX_ReadOneByte(EEP_AI_OFFSET0+2*loop +1) ;
				if((temp1== 0xff)&&(temp2 == 0xff))
				{
						modbus.offset[loop] = 500 ;
						temp1 = (modbus.offset[loop]>>8)&0xff ;
						temp2 = (modbus.offset[loop])&0xff ;
						AT24CXX_WriteOneByte(EEP_AI_OFFSET0+2*loop, temp1);
						AT24CXX_WriteOneByte(EEP_AI_OFFSET0+2*loop+1, temp2);
				}
				else
				{
						modbus.offset[loop] = (temp1<<8)|temp2;
				}
//				modbus.pulse[loop].quarter[0] = AT24CXX_ReadOneByte(EEP_PLUSE0_HI_HI+4*loop) ;
//				modbus.pulse[loop].quarter[1] = AT24CXX_ReadOneByte(EEP_PLUSE0_HI_LO+4*loop);
//				modbus.pulse[loop].quarter[2] = AT24CXX_ReadOneByte(EEP_PLUSE0_LO_HI+4*loop) ;
//				modbus.pulse[loop].quarter[3] = AT24CXX_ReadOneByte(EEP_PLUSE0_LO_LO+4*loop);
//				if((modbus.pulse[loop].quarter[0] == 0xff)&&(modbus.pulse[loop].quarter[1]== 0xff)&&(modbus.pulse[loop].quarter[2]== 0xff)&& (modbus.pulse[loop].quarter[3] == 0xff))
//				{
//							modbus.pulse[loop].word = 0 ;
//						  AT24CXX_WriteOneByte(EEP_PLUSE0_HI_HI+4*loop, modbus.pulse[loop].quarter[0]);
//							AT24CXX_WriteOneByte(EEP_PLUSE0_HI_LO+4*loop, modbus.pulse[loop].quarter[1]);
//							AT24CXX_WriteOneByte(EEP_PLUSE0_LO_HI+4*loop, modbus.pulse[loop].quarter[2]);
//							AT24CXX_WriteOneByte(EEP_PLUSE0_LO_LO+4*loop, modbus.pulse[loop].quarter[3]);
//				}
				
		}
		#endif
		
}
