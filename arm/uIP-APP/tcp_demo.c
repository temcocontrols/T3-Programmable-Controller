#include "stm32f10x.h"
#include "usart.h"	 		   
#include "uip.h"	    
#include "enc28j60.h"
#include "httpd.h"
#include "tcp_demo.h"

//TCP应用接口函数(UIP_APPCALL)
//完成TCP服务(包括server和client)和HTTP服务


//打印日志用
void uip_log(char *m)
{
	//printf("uIP log:%s\r\n", m);
}
