#include "udp_demo.h"
#include "uip.h"
#include "resolv.h"
#include <stdio.h>
#include <string.h>


#define UDP_SERVER_LPORT	7777
#define UDP_CLIENT_LPORT	6666
#define UDP_CLIENT_RPORT	5555

void udp_app_init(void)
{
	struct uip_udp_conn *conn;
	uip_ipaddr_t addr;
	
	// udp server
	uip_listen(HTONS(UDP_SERVER_LPORT));
	uip_udp_bind(&uip_udp_conns[0], HTONS(UDP_SERVER_LPORT));
	
	// udp client
	uip_ipaddr(addr, 192, 168, 0, 146);
	conn = uip_udp_new(&addr, HTONS(UDP_CLIENT_RPORT));
	if(conn != NULL)
	{
		printf("uip_udp_new OK\r\n");
		uip_udp_bind(conn, HTONS(UDP_CLIENT_LPORT));
	}
}

void UDP_SERVER_APP(void)
{
	/* check the status */
	if(uip_poll())
	{
		char *tmp_dat = "udp server: the auto send!\r\n";
		uip_send((char *)tmp_dat, strlen(tmp_dat));
	}
	
	if(uip_newdata())
	{
		char *tmp_dat = "udp server: receive the data\r\n";
		/* new data comes in */
		printf("receive %dbytes: ", uip_len);
		printf("%s\r\n", (char *)uip_appdata);
		uip_send((char *)tmp_dat, strlen(tmp_dat));
	}
}

void UDP_CLIENT_APP(void)
{
	/* check the status */
	if(uip_poll())
	{
		char *tmp_dat = "udp client: the auto send!\r\n";
		uip_send((char *)tmp_dat, strlen(tmp_dat));
	}
	
	if(uip_newdata())
	{
		char *tmp_dat = "udp client: receive the data\r\n";
		/* new data comes in */
		printf("receive %dbytes: ", uip_len);
		printf("%s\r\n", (char *)uip_appdata);
		uip_send((char *)tmp_dat, strlen(tmp_dat));
	}
}

//void udp_demo_appcall(void)
//{
//// udp server
//	switch(uip_udp_conn->lport)
//    {
//		case HTONS(UDP_SERVER_LPORT):
//			UDP_SERVER_APP();
//			break;
////		case HTONS(UDP_CLIENT_LPORT):
////			UDP_CLIENT_APP();
////			break;
//    }

//// udp client	
//    switch(uip_udp_conn->rport)
//    {
//		case HTONS(67):
//			dhcpc_appcall();
//			break;
//		case HTONS(68):
//			dhcpc_appcall();
//			break;
//		case HTONS(53):
//			resolv_appcall();
//			break;
//		case HTONS(UDP_CLIENT_RPORT):
//			UDP_CLIENT_APP();
//			break;
//    }
//}

void dhcpc_configured(const struct dhcpc_state *s)
{
	uip_init();							//uIP初始化
	
	uip_sethostaddr(s->ipaddr);
	uip_setnetmask(s->netmask);
	uip_setdraddr(s->default_router);
	resolv_conf((u16_t *)s->dnsaddr);
	
	uip_listen(HTONS(1200));			//监听1200端口,用于TCP Server
	uip_listen(HTONS(80));				//监听80端口,用于Web Server
	tcp_client_reconnect();	   		    //尝试连接到TCP Server端,用于TCP Client
//	udp_app_init();
}
