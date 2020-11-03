#include "main.h"



#include "dyndns.h"
#include "dyndns_app.h"
#include <string.h>
//#include "dnsc.h"
#include "resolv.h"

//U8_T far test_dyndns_state = 0;
//U8_T far test_dyndns_AppId = 0;
//U8_T far test_dyndns_ConnId = 0xff;
//U8_T far test_tcpip_connect_status = 0;
//U16_T far test_dyndns_rev_len = 0;
//U8_T far test_rev_buf[20] = {0};

extern U16_T far Test[50];

#define DNSC_QUERY_OK 1

//U8_T far dyndns_enable = TRUE;
U16_T far dyndns_update_time;
U8_T far dyndns_provider = CONF_DDNS_DYNDNS;
U8_T far dyndns_domain_name[MAX_DOMAIN_SIZE];// = "temcocontrols.dyndns.org";
U8_T far dyndns_username[MAX_USERNAME_SIZE];// = "temcocontrols";
U8_T far dyndns_password[MAX_PASSWORD_SIZE];// = "travel";
U8_T far check_ip_every;
U8_T far force_update_every;


U8_T far update_from_dyndns = TRUE;
U16_T far dyndns_update_counter;
U16_T far dyndns_update_period;

U32_T dyndns_server_ip;

/*_STR_DYNDNS_SERVER*/U8_T const code dyndns_server_array[3] = 
{
	{
		CONF_DDNS_3322, 	// www.3322.org
//		0x76b8b00f//0x3DA0EF19,			// 61.160.239.25
	},
	{
		CONF_DDNS_DYNDNS	// www.dyndns.com
//		0xCC0DF875,			// 204.13.248.117
	},
	{
		CONF_DDNS_NOIP		// www.no-ip.com
//		0x0817E06E,			// 8.23.224.110
	}
//	{
//		CONF_DDNS_ODS,		//update.ods.com
//		0x081E5919,			//8.30.89.19
//	}

};



void init_dyndns_data(void)
{
	memset(dyndns_domain_name, 0,MAX_DOMAIN_SIZE);
	memset(dyndns_username, 0,MAX_USERNAME_SIZE);
	memset(dyndns_password, 0,MAX_PASSWORD_SIZE);
	

	DynDNS_Init();	
}



void dyndns_select_domain(U8_T provider)
{
	U8_T ip[4];	
	U16_T *ptr_ip = NULL;
	if(provider == 0)
		ptr_ip = resolv_lookup("www.3322.org");		
	else if(provider == 1)
		ptr_ip = resolv_lookup("www.dyndns.com");		
	else if(provider == 2)
		ptr_ip = resolv_lookup("www.no-ip.com");
	else if(provider == 3)
	{	
		ptr_ip = resolv_lookup("newfirmware.com");}
	if(ptr_ip != NULL)
	{
	memcpy(&ip,ptr_ip,4);
	dyndns_server_ip = ((U32_T)ip[0] << 24) + ((U32_T)ip[1] << 16) + ((U32_T)ip[2] << 8) + ip[3];
//	if(dyndns_server_ip == 0x88660020)
//		dyndns_server_ip = 0;
//#if ARM_UART_DEBUG
//		UART_Init(0);
//		DEBUG_EN = 1;
//		printf("dyndsn ip:%u,%u,%u,%u\r\n",(U8_T)(dyndns_server_ip>>24),(U8_T)(dyndns_server_ip>>16),(U8_T)(dyndns_server_ip>>8),(U8_T)dyndns_server_ip);

//#endif
	}
}



void init_dyndns(void)
{
//	dyndns_enable = TRUE;
//	memcpy(dyndns_domain_name, 0,MAX_DOMAIN_SIZE);
//	memcpy(dyndns_username, 0,MAX_USERNAME_SIZE);
//	memcpy(dyndns_password, 0,MAX_PASSWORD_SIZE);	
	
//	dyndns_provider = CONF_DDNS_DYNDNS;

//	memcpy(dyndns_domain_name, "temcocontrols2.dyndns.org",25);
//	memcpy(dyndns_username, "temcocontrols",13);
//	memcpy(dyndns_password, "Travel123",9);
	
	DynDNS_Init();	
	dyndns_select_domain(dyndns_provider);
	update_from_dyndns = TRUE;
}

void do_dyndns(void)
{

	if(dyndns_provider == 3) // temco server
	{
		return;
	}
	else
	{
		
		if(update_from_dyndns == TRUE)
		{
			//if(dyndns_get_serverip(dyndns_provider,&dyndns_server_ip) == DNSC_QUERY_OK)
			//if(dyndns_server_ip != 0)
			{
				if(dyndns_provider == 0)
					dyndns_server_ip = 0x76b8b00f;
				else if(dyndns_provider == 1)
					dyndns_server_ip = 0xcc0df875;
				else if(dyndns_provider == 2)
					dyndns_server_ip = 0x0817e06e;
				DynDNS_DoUpdateDynDns(dyndns_server_array[dyndns_provider],
				dyndns_server_ip,
				dyndns_domain_name, 
				dyndns_username, 
				dyndns_password);
			}			
			
			if(DynDNS_GetUpdateState() == TRUE)
			{
				update_from_dyndns = FALSE;
			}
			//}
		}
	}
}

void dyndns_reply(void)
{
	
}

