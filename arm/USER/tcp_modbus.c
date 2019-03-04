#include "main.h"
//#include "define.h"
//#include "tcpip.h"

#define UIP_HEAD 6

u8 tcp_server_databuf[300];   	//发送数据缓存	  
u8 tcp_server_sta;				//服务端状态
//[7]:0,无连接;1,已经连接;
//[6]:0,无数据;1,收到客户端数据
//[5]:0,无数据;1,有数据需要发送
u8 update_firmware;

u8 tcp_server_sendbuf[300];
u16 tcp_server_sendlen;

void Set_transaction_ID(U8_T *str, U16_T id, U16_T num)
{
	str[0] = (U8_T)(id >> 8);		//transaction id
	str[1] = (U8_T)id;

	str[2] = 0;						//protocol id, modbus protocol = 0
	str[3] = 0;

	str[4] = (U8_T)(num >> 8);
	str[5] = (U8_T)num;
}

//这是一个TCP 服务器应用回调函数。
//该函数通过UIP_APPCALL(tcp_demo_appcall)调用,实现Web Server的功能.
//当uip事件发生时，UIP_APPCALL函数会被调用,根据所属端口(1200),确定是否执行该函数。
//例如 : 当一个TCP连接被创建时、有新的数据到达、数据已经被应答、数据需要重发等事件
void tcp_server_appcall(struct uip_conn * conn)
{
	u8 send_flag = 0;
	u8 pos;
 	struct tcp_demo_appstate *s = (struct tcp_demo_appstate *)&uip_conn->appstate;

	if(uip_aborted()){tcp_server_aborted();	}	//连接终止
 	if(uip_timedout()){tcp_server_timedout();}	//连接超时   
	if(uip_closed()) {tcp_server_closed();}		//连接关闭	   
 	if(uip_connected()){tcp_server_connected(); } 	//连接成功	    
	if(uip_acked())tcp_server_acked();			//发送的数据成功送达 
	if(uip_newdata())//收到客户端发过来的数据
	{
//		net_rx_count  = 2 ;
		memcpy(&tcp_server_databuf[0], uip_appdata,uip_len);		
		// check modbus data
		if( (tcp_server_databuf[0] == 0xee) && (tcp_server_databuf[1] == 0x10) &&
		(tcp_server_databuf[2] == 0x00) && (tcp_server_databuf[3] == 0x00) &&
		(tcp_server_databuf[4] == 0x00) && (tcp_server_databuf[5] == 0x00) &&
		(tcp_server_databuf[6] == 0x00) && (tcp_server_databuf[7] == 0x00) )
		{		
//			Udtcp_server_databuf(0);
			send_flag = 1;
			update_firmware = 1;

		}
		else if(tcp_server_databuf[6] == Modbus.address 
		|| ((tcp_server_databuf[6] == 255) && (tcp_server_databuf[7] != 0x19))
		)
		{	
//			net_tx_count  = 2 ;
			send_flag = 1;
			tcp_server_sendlen = 0;
			responseCmd(1, tcp_server_databuf);
		}
		else
		{
			// transfer data to sub ,TCP TO RS485
			U8_T header[6];	
			U8_T i;
			
			if((tcp_server_databuf[UIP_HEAD] == 0x00) || 
			((tcp_server_databuf[UIP_HEAD + 1] != READ_VARIABLES) 
			&& (tcp_server_databuf[UIP_HEAD + 1] != WRITE_VARIABLES) 
			&& (tcp_server_databuf[UIP_HEAD + 1] != MULTIPLE_WRITE) 
			&&(tcp_server_databuf[UIP_HEAD + 1] != CHECKONLINE)
			&& (tcp_server_databuf[UIP_HEAD + 1] != READ_COIL)
			&& (tcp_server_databuf[UIP_HEAD + 1] != READ_DIS_INPUT)
			&& (tcp_server_databuf[UIP_HEAD + 1] != READ_INPUT)
			&& (tcp_server_databuf[UIP_HEAD + 1] != WRITE_COIL)
			&& (tcp_server_databuf[UIP_HEAD + 1] != WRITE_MULTI_COIL)
			&& (tcp_server_databuf[UIP_HEAD + 1] != CHECKONLINE_WIHTCOM)))
			{
				return;
			}
			if((tcp_server_databuf[UIP_HEAD + 1] == MULTIPLE_WRITE) && ((uip_len - UIP_HEAD) != (tcp_server_databuf[UIP_HEAD + 6] + 7)))
			{
				return;
			}	

			if(Modbus.com_config[2] == MODBUS_MASTER)
				Modbus.sub_port = 2;
			else if(Modbus.com_config[0] == MODBUS_MASTER)
				Modbus.sub_port = 0;
			else if(Modbus.com_config[1] == MODBUS_MASTER)
				Modbus.sub_port = 1;
			else
			{
				return;
			}

			for(i = 0;i <  sub_no ;i++)
			{
				if(tcp_server_databuf[UIP_HEAD] == uart2_sub_addr[i])
				{
					Modbus.sub_port = 2;
					continue;
				}
				else if(tcp_server_databuf[UIP_HEAD] == uart0_sub_addr[i])
				{
					 Modbus.sub_port = 0;
					 continue;
				}
				else if(tcp_server_databuf[UIP_HEAD] == uart1_sub_addr[i])
				{	
					Modbus.sub_port = 1;
					continue;
				}
			}		


			
			if(Modbus.mini_type == MINI_VAV)	
				Modbus.sub_port = 0;			
			
			if(flag_resume_rs485 == 0 || flag_resume_rs485 == 2)			
			{
				vTaskSuspend(Handle_Scan); 
			}				
			vTaskSuspend(xHandler_Output);	
			vTaskSuspend(xHandleCommon);		
//			vTaskSuspend(xHandleBacnetControl); 
			vTaskSuspend(xHandleMornitor_task);			
//#if MSTP
//			vTaskSuspend(xHandleMSTP);			
//#endif 
			
#if ARM_MINI	
			
			if((Modbus.mini_type == MINI_BIG) ||	(Modbus.mini_type == MINI_BIG_ARM) 
				|| (Modbus.mini_type == MINI_SMALL)  || (Modbus.mini_type == MINI_SMALL_ARM) 
			|| (Modbus.mini_type == MINI_TINY)			
			)
				vTaskSuspend(xHandler_SPI);	
#endif

			task_test.inactive_count[0] = 0;

//			TcpSocket_ME = pMODBUSTCPConn->TcpSocket;
			send_flag = 1;
			tcp_server_sendlen = 0;
			Set_transaction_ID(header, ((U16_T)tcp_server_databuf[0] << 8) | tcp_server_databuf[1], 2 * tcp_server_databuf[UIP_HEAD + 5] + 3);
			
			Response_TCPIP_To_SUB(tcp_server_databuf + UIP_HEAD,uip_len - UIP_HEAD,Modbus.sub_port,header);
//			memcpy(tcp_server_sendbuf,tcp_server_databuf,2 * tcp_server_databuf[UIP_HEAD + 5] + 3);
//      tcp_server_sendlen = 2 * tcp_server_databuf[UIP_HEAD + 5] + 3;
			
			flag_resume_rs485 = 1;	// suspend rs485 task, resume it later, make the communication smoothly	
			resume_rs485_count = 0;
			
			vTaskResume(xHandler_Output); 
			vTaskResume(xHandleCommon);
//			vTaskResume(xHandleBacnetControl);
			vTaskResume(xHandleMornitor_task);
//#if MSTP
//			vTaskResume(xHandleMSTP);			
//#endif 

#if ARM_MINI	

			if((Modbus.mini_type == MINI_BIG) ||	(Modbus.mini_type == MINI_BIG_ARM) 
				|| (Modbus.mini_type == MINI_SMALL)  || (Modbus.mini_type == MINI_SMALL_ARM) 
			|| (Modbus.mini_type == MINI_TINY)			
			)
			vTaskResume(xHandler_SPI);

#endif	
		
		}
	}
	

//	/*else*/ if(tcp_server_sta & (1 << 5))			//有数据需要发送
//	{
//		s->textptr = tcp_server_databuf;
//		s->textlen = strlen((const char*)tcp_server_databuf);
//		tcp_server_sta &= ~(1 << 5);			//清除标记
//	}
	
	//当需要重发、新数据到达、数据包送达、连接建立时，通知uip发送数据 
	if(uip_rexmit() || uip_newdata() || uip_acked() || uip_connected() || uip_poll())
	{
		if(send_flag == 1)
		{
			s->textptr = tcp_server_sendbuf;
			s->textlen = tcp_server_sendlen;
		}
		tcp_server_senddata();
		
	}
}

//终止连接				    
void tcp_server_aborted(void)
{
	tcp_server_sta &= ~(1 << 7);				//标志没有连接
//	uip_log("tcp_server aborted!\r\n");			//打印log
}

//连接超时
void tcp_server_timedout(void)
{
	tcp_server_sta &= ~(1 << 7);				//标志没有连接
//	uip_log("tcp_server timeout!\r\n");			//打印log
}

//连接关闭
void tcp_server_closed(void)
{
	tcp_server_sta &= ~(1 << 7);				//标志没有连接
//	uip_log("tcp_server closed!\r\n");			//打印log
}

//连接建立
void tcp_server_connected(void)
{								  
	struct tcp_demo_appstate *s = (struct tcp_demo_appstate *)&uip_conn->appstate;
	//uip_conn结构体有一个"appstate"字段指向应用程序自定义的结构体。
	//声明一个s指针，是为了便于使用。
 	//不需要再单独为每个uip_conn分配内存，这个已经在uip中分配好了。
	//在uip.c 中 的相关代码如下：
	//		struct uip_conn *uip_conn;
	//		struct uip_conn uip_conns[UIP_CONNS]; //UIP_CONNS缺省=10
	//定义了1个连接的数组，支持同时创建几个连接。
	//uip_conn是一个全局的指针，指向当前的tcp或udp连接。
	tcp_server_sta |= 1 << 7;					//标志连接成功
//  	uip_log("tcp_server connected!\r\n");		//打印log
	s->state = STATE_CMD; 						//指令状态
	s->textlen = 0;
//	s->textptr = "Connect to STM32 Board Successfully!\r\n";
//	s->textlen = strlen((char *)s->textptr);
}

//发送的数据成功送达
void tcp_server_acked(void)
{						    	 
	struct tcp_demo_appstate *s = (struct tcp_demo_appstate *)&uip_conn->appstate;
	s->textlen = 0;								//发送清零
//	uip_log("tcp_server acked!\r\n");			//表示成功发送		 
}

//发送数据给客户端
void tcp_server_senddata(void)
{
	struct tcp_demo_appstate *s = (struct tcp_demo_appstate *)&uip_conn->appstate;
	//s->textptr : 发送的数据包缓冲区指针
	//s->textlen ：数据包的大小（单位字节）		   
	if(s->textlen > 0)
		uip_send(s->textptr, s->textlen);//发送TCP数据包	 
}


