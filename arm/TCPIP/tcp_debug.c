#include "tcpip.h"
#include "tcp_debug.h"
#include "product.h"


extern  S8_T smtpc_Buf[256];
	#ifdef ETHERNET_DEBUG
			unsigned char tcpdebug_flag = 0;
			unsigned char tcpdebug_length = 0;
			void EthernetDebug_appcall(void)
			{
				struct tcp_demo_appstate *s = (struct tcp_demo_appstate *)&uip_conn->appstate;
			 static unsigned int n_test_value = 0;
				//char temp_char[20];
				
				if(uip_poll()) 
				{
					if((tcpdebug_flag == TCP_PRINT_STR) ||
						(tcpdebug_flag == TCP_PRINT_HEX))
					{
						if(tcpdebug_length < 256)
							TcpDebug_SendMessage(smtpc_Buf,tcpdebug_length);	
						tcpdebug_flag = TCP_PRINT_IDLE;
					}
					 //sprintf(smtpc_Buf,"test%d",++n_test_value);
						

				}
			}


			void tcp_printf_str(char *str)
			{
				int nstr_length = 0;
				nstr_length = strlen(str);
				if(nstr_length >= 256)
					return;
				strcpy(smtpc_Buf,str);
				tcpdebug_flag = TCP_PRINT_STR;
				tcpdebug_length = nstr_length;
			}
			
			void tcp_printf_hex(char *str,int nlength,char *des_str ) //最多将84个转换为hex  buffer为256;
			{
				int i=0;
        unsigned char des_length = 0;
				
				if(des_str != 0)
				{
					des_length = strlen(des_str);
					memcpy(smtpc_Buf,des_str,des_length);
				}
				
				if((nlength + des_length) >= 84)
					return;
				

				
				for(i=0;i<nlength;i++)
				{
					sprintf(&smtpc_Buf[i*3 + des_length ],"%02x ",str[i]);
				}
				smtpc_Buf[3*nlength ]= 0;
				tcpdebug_flag = TCP_PRINT_HEX;
				tcpdebug_length = strlen(smtpc_Buf);
			}
#endif
			
			
#ifndef ETHERNET_DEBUG
			void tcp_printf_str(char *str)
			{
				;
			}
			
       void tcp_printf_hex(char *str,int nlength,char *des_str  )
			{
				;
			}
#endif