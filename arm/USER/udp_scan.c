#include "tcpip.h"
#include "define.h"
#include "rtc.h"
#include "serial.h"
#include "main.h"


#define GUDPBC_MAX_CONNS			2
#define GUDPBC_NO_NEW_CONN			0xFF

#define GUDPBC_STATE_FREE			0
#define	GUDPBC_STATE_WAIT			1
#define	GUDPBC_STATE_CONNECTED		2


STR_SCAN_CMD Scan_Infor;

//u8	InformationStr[60];
U8_T 	state=1;
U8_T 	scanstart=0;

U8_T flag_response_scan = 0;
U32_T response_scan_ip = 0;
U32_T response_scan_port = 0;
STR_SCAN_CMD Infor[20];

U8_T IP_Change = 0;

U8_T state;

extern U8_T far flag_scan_sub;
extern U8_T far count_scan_sub_by_hand;

void udp_scan_init(void)
{
	struct uip_udp_conn *conn;
	uip_ipaddr_t addr;
	
	// udp server
	uip_listen(HTONS(UDP_SCAN_LPORT));
	
	uip_ipaddr_copy(addr, 0xffffffff);
	conn = uip_udp_new(&addr, HTONS(UDP_SCAN_LPORT)); // des port
	if(conn != NULL) 
	{ 
		uip_udp_bind(conn,HTONS(UDP_SCAN_LPORT));  // src port					
	}
	//uip_udp_bind(&uip_udp_conns[0], HTONS(UDP_SCAN_LPORT));
	
}

U8_T network_scan[5];
U8_T flag_newtork_scan;
struct uip_udp_conn * Send_NET_scan_conn;


void UdpData(U8_T type)
{
	// header 2 bytes
	memset(&Scan_Infor,0,sizeof(STR_SCAN_CMD));
	if(type == 0)
		Scan_Infor.cmd = 0x0065;
	else if(type == 1)
		Scan_Infor.cmd = 0x0067;

	Scan_Infor.len = 0x001d;
	
	//serialnumber 4 bytes
	Scan_Infor.own_sn[0] = (U16_T)Modbus.serialNum[0];
	Scan_Infor.own_sn[1] = (U16_T)Modbus.serialNum[1];
	Scan_Infor.own_sn[2] = (U16_T)Modbus.serialNum[2];
	Scan_Infor.own_sn[3] = (U16_T)Modbus.serialNum[3];
	
	//nc 
	
	if(Modbus.mini_type == MINI_CM5)
		Scan_Infor.product = (U16_T)PRODUCT_CM5;
	else if((Modbus.mini_type >= MINI_BIG_ARM) && (Modbus.mini_type <= MINI_TINY_ARM))
		Scan_Infor.product = (U16_T)PRODUCT_MINI_ARM;
	else
		Scan_Infor.product = (U16_T)PRODUCT_MINI_BIG;

	//modbus address
	Scan_Infor.address = (U16_T)Modbus.address;
	
	//Ip
	Scan_Infor.ipaddr[0] = (U16_T)Modbus.ip_addr[0];
	Scan_Infor.ipaddr[1] = (U16_T)Modbus.ip_addr[1];
	Scan_Infor.ipaddr[2] = (U16_T)Modbus.ip_addr[2];
	Scan_Infor.ipaddr[3] = (U16_T)Modbus.ip_addr[3];
	
	//port
	Scan_Infor.modbus_port = swap_word(Modbus.tcp_port);

	// software rev
	Scan_Infor.firmwarerev = swap_word(SW_REV / 10 + SW_REV % 10);
	// hardware rev
	Scan_Infor.hardwarerev = swap_word(Modbus.hardRev);
	
	Scan_Infor.instance_low = HTONS(Instance); // hight byte first
	Scan_Infor.panel_number = panel_number; //  36	
	Scan_Infor.instance_hi = HTONS(Instance >> 16); // hight byte first
	
	Scan_Infor.bootloader = 0;  // 0 - app, 1 - bootloader, 2 - wrong bootloader
	Scan_Infor.BAC_port = Modbus.Bip_port;//((Modbus.Bip_port & 0x00ff) << 8) + (Modbus.Bip_port >> 8);  // 
	Scan_Infor.zigbee_exist = zigbee_exist; // 0 - inexsit, 1 - exist
	Scan_Infor.subnet_protocal = 0;
	state = 1;
	scanstart = 0;

}
 
static u8 scan_temp[70];
u8 rec_scan_index;
void private_scan(void)
{	
	u8 len;
	u8 n = 0;
	u8 i;
	   
	len = uip_len; 
	if(len <= sizeof(STR_SCAN_CMD))
			memcpy(scan_temp, uip_appdata, len);
		
	if(scan_temp[0] == 0x64)
		{
			state = 1;
			for(n = 0;n < (u8)len / 4;n++)
			{       
				if((scan_temp[4*n+1] == Modbus.ip_addr[0]) && (scan_temp[4*n+2] == Modbus.ip_addr[1])
					 &&(scan_temp[4*n+3] == Modbus.ip_addr[2]) && (scan_temp[4*n+4] == Modbus.ip_addr[3]))
				{ 
					 state=0;
				}
			}

		
			
			if(state)
			{            
				//use broadcast when scan			
				UdpData(0);
				//serialnumber 4 bytes
				Scan_Infor.master_sn[0] = 0;
				Scan_Infor.master_sn[1] = 0;
				Scan_Infor.master_sn[2] = 0;
				Scan_Infor.master_sn[3] = 0;
				memcpy(&Scan_Infor.panelname,panelname,20);			
				uip_send((char *)&Scan_Infor, sizeof(STR_SCAN_CMD));
				rec_scan_index = 0;

			// for MODBUS device
				for(i = 0;i < sub_no;i++)
				{	 
					if((scan_db[i].product_model >= CUSTOMER_PRODUCT) || (current_online[scan_db[i].id / 8] & (1 << (scan_db[i].id % 8))))	  	 // in database but not on_line
					{
						if(scan_db[i].product_model != PRODUCT_MINI_BIG)
						{
						Scan_Infor.own_sn[0] = (U16_T)scan_db[i].sn;
						Scan_Infor.own_sn[1] = (U16_T)(scan_db[i].sn >> 8) & 0x00ff;
						Scan_Infor.own_sn[2] = (U16_T)(scan_db[i].sn >> 16) & 0x00ff;
						Scan_Infor.own_sn[3] = (U16_T)(scan_db[i].sn >> 24) & 0x00ff;					

						Scan_Infor.product = (U16_T)scan_db[i].product_model;
						Scan_Infor.address = (U16_T)scan_db[i].id;
				
						Scan_Infor.instance_low = 0;
						Scan_Infor.instance_hi = 0;
						Scan_Infor.subnet_protocal = 1;  // modbus device
						
						Scan_Infor.master_sn[0] = Modbus.serialNum[0];
						Scan_Infor.master_sn[1] = Modbus.serialNum[1];
						Scan_Infor.master_sn[2] = Modbus.serialNum[2];
						Scan_Infor.master_sn[3] = Modbus.serialNum[3];
						
						memcpy(&Scan_Infor.panelname,tstat_name[i],20);
							//uip_send((char *)InformationStr, 60);
							 if(rec_scan_index < 19)
								 memcpy(&Infor[rec_scan_index++],&Scan_Infor, sizeof(STR_SCAN_CMD));
						}	
					}						
				}
				
// if id confict, send it to T3000
			if(conflict_num == 1)
			{
				Scan_Infor.own_sn[0] = (U16_T)conflict_sn_old;
				Scan_Infor.own_sn[1] = (U16_T)(conflict_sn_old >> 8);
				Scan_Infor.own_sn[2] = (U16_T)(conflict_sn_old >> 16);
				Scan_Infor.own_sn[3] = (U16_T)(conflict_sn_old >> 24);					

				Scan_Infor.product = 0x08;//(U16_T)scan_db[i].product_model << 8;
				Scan_Infor.address = (U16_T)conflict_id;
		
				
				Scan_Infor.master_sn[0] = Modbus.serialNum[0];
				Scan_Infor.master_sn[1] = Modbus.serialNum[1];
				Scan_Infor.master_sn[2] = Modbus.serialNum[2];
				Scan_Infor.master_sn[3] = Modbus.serialNum[3];
				
				if(rec_scan_index < 19)
					memcpy(&Infor[rec_scan_index++],&Scan_Infor, sizeof(STR_SCAN_CMD));
				
				Scan_Infor.own_sn[0] = (U16_T)conflict_sn_new;
				Scan_Infor.own_sn[1] = (U16_T)(conflict_sn_new >> 8);
				Scan_Infor.own_sn[2] = (U16_T)(conflict_sn_new >> 16);
				Scan_Infor.own_sn[3] = (U16_T)(conflict_sn_new >> 24);		
				

				//uip_send((char *)InformationStr, 60);
				if(rec_scan_index < 19)
				 memcpy(&Infor[rec_scan_index++],&Scan_Infor,  sizeof(STR_SCAN_CMD));
			}

				
	// for MSTP device		
				for(i = 0;i < remote_panel_num;i++)
				{	 
					
					if(remote_panel_db[i].protocal == BAC_MSTP)
					{
//						BACNET_ADDRESS dest = { 0 };
//						uint16 max_apdu = 0;
//						bool status = false;

						/* is the device bound? */
						//status = address_get_by_device(remote_panel_db[i].device_id, &max_apdu, &dest);
						
//						if(status > 0)
						{
							char temp_name[20];
							Scan_Infor.own_sn[0] = (U16_T)remote_panel_db[i].device_id;
							Scan_Infor.own_sn[1] = (U16_T)(remote_panel_db[i].device_id >> 8);
							Scan_Infor.own_sn[2] = (U16_T)(remote_panel_db[i].device_id >> 16);
							Scan_Infor.own_sn[3] = (U16_T)(remote_panel_db[i].device_id >> 24);
							
							
							Scan_Infor.product = remote_panel_db[i].product_model;
							Scan_Infor.address = remote_panel_db[i].sub_id;			
							
							Scan_Infor.master_sn[0] = Modbus.serialNum[0];
							Scan_Infor.master_sn[1] = Modbus.serialNum[1];
							Scan_Infor.master_sn[2] = Modbus.serialNum[2];
							Scan_Infor.master_sn[3] = Modbus.serialNum[3];	
						
							
							Scan_Infor.instance_low = HTONS(remote_panel_db[i].device_id); // hight byte first
							Scan_Infor.panel_number = remote_panel_db[i].panel;	
							Scan_Infor.instance_hi = HTONS(remote_panel_db[i].device_id >> 16); // hight byte first
							
							memset(temp_name,0,20);
//							strcmp(temp_name, "panel:"/*, remote_panel_db[i].sub_id*/);
							temp_name[0] = 'M';
							temp_name[1] = 'S';
							temp_name[2] = 'T';
							temp_name[3] = 'P';
							temp_name[4] = ':';
//							temp_name[5] = Scan_Infor.station_num / 10 + '0';
//							temp_name[6] = Scan_Infor.station_num % 10 + '0';							
							
							temp_name[19] = '\0';
							memcpy(&Scan_Infor.panelname,temp_name,20);
							
							Scan_Infor.subnet_protocal = 12;  // MSTP device
							
							if(rec_scan_index < 19)
							memcpy(&Infor[rec_scan_index++],&Scan_Infor, sizeof(STR_SCAN_CMD));		
						}						
					}				
				}				
			}
		}
   else if((scan_temp[0] == 0x66) && (scan_temp[1] == Modbus.ip_addr[0]) && (scan_temp[2] == Modbus.ip_addr[1]) && (scan_temp[3] == Modbus.ip_addr[2]) && (scan_temp[4] == Modbus.ip_addr[3]))
   { 
		 // cmd(1 byte) + changed ip(4 bytes) + new ip(4 bytes) + new subnet(4 bytes) + new getway(4)  --- old protocal
		 // + sn(4 bytes)  -- new protocal, used to change conflict ip
    	 
		 if(((scan_temp[17] == Modbus.serialNum[0]) && (scan_temp[18] == Modbus.serialNum[1]) && (scan_temp[19] == Modbus.serialNum[2]) && (scan_temp[20] == Modbus.serialNum[3]))
			 || ((scan_temp[17] == 0) && (scan_temp[18] == 0) && (scan_temp[19] == 0) && (scan_temp[20] == 0)))
		 {
				n = 5;
				UdpData(1);
				uip_send((char *)&Scan_Infor,  sizeof(STR_SCAN_CMD));
				Modbus.tcp_type = 0;
			 
				Modbus.ip_addr[0] = scan_temp[n++];
				Modbus.ip_addr[1] = scan_temp[n++];
				Modbus.ip_addr[2] = scan_temp[n++];
				Modbus.ip_addr[3] = scan_temp[n++];
			 
			 	if(Modbus.com_config[2] == BACNET_SLAVE || Modbus.com_config[2] == BACNET_MASTER)
				{
					Send_I_Am_Flag = 1;
				}
		//	  Modbus.ghost_ip_addr[0] = Modbus.ip_addr[0];
		//	  Modbus.ghost_ip_addr[1] = Modbus.ip_addr[1];
		//	  Modbus.ghost_ip_addr[2] = Modbus.ip_addr[2];
		//	  Modbus.ghost_ip_addr[3] = Modbus.ip_addr[3];
			 
			//printf("%u,%u,%u,%u,%u,%u,%u,%u,",  Modbus.ip_addr[0], Modbus.ip_addr[1], Modbus.ip_addr[2], Modbus.ip_addr[3],temp[5],temp[6],temp[7],temp[8]);
				Modbus.subnet[0] = scan_temp[n++];
				Modbus.subnet[1] = scan_temp[n++];
				Modbus.subnet[2] = scan_temp[n++];
				Modbus.subnet[3] = scan_temp[n++];

		//		Modbus.ghost_mask_addr[0] = Modbus.mask_addr[0] ;
		//		Modbus.ghost_mask_addr[1] = Modbus.mask_addr[1] ;
		//		Modbus.ghost_mask_addr[2] = Modbus.mask_addr[2] ;
		//		Modbus.ghost_mask_addr[3] = Modbus.mask_addr[3] ;
			 
				Modbus.getway[0] = scan_temp[n++];
				Modbus.getway[1] = scan_temp[n++];
				Modbus.getway[2] = scan_temp[n++];
				Modbus.getway[3] = scan_temp[n++];
				
		//	 Modbus.ghost_gate_addr[0] = Modbus.gate_addr[0] ;
		//	 Modbus.ghost_gate_addr[1] = Modbus.gate_addr[1] ;
		//	 Modbus.ghost_gate_addr[2] = Modbus.gate_addr[2] ;
		//	 Modbus.ghost_gate_addr[3] = Modbus.gate_addr[3] ;
			 
			

				if((Modbus.ip_addr[0] != 0)  && (Modbus.ip_addr[1] != 0)  && (Modbus.ip_addr[3] != 0) )
				{		
		#if ASIX			
					E2prom_Write_Byte(EEP_IP, Modbus.ip_addr[3]);
					E2prom_Write_Byte(EEP_IP + 1, Modbus.ip_addr[2]);
					E2prom_Write_Byte(EEP_IP + 2, Modbus.ip_addr[1]);
					E2prom_Write_Byte(EEP_IP + 3, Modbus.ip_addr[0]);
					
					
					E2prom_Write_Byte(EEP_SUBNET, Modbus.subnet[3]);
					E2prom_Write_Byte(EEP_SUBNET + 1, Modbus.subnet[2]);
					E2prom_Write_Byte(EEP_SUBNET + 2, Modbus.subnet[1]);
					E2prom_Write_Byte(EEP_SUBNET + 3, Modbus.subnet[0]);
		#endif

		#if (ARM_MINI || ARM_CM5 || ARM_WIFI)			
					E2prom_Write_Byte(EEP_IP, Modbus.ip_addr[0]);
					E2prom_Write_Byte(EEP_IP + 1, Modbus.ip_addr[1]);
					E2prom_Write_Byte(EEP_IP + 2, Modbus.ip_addr[2]);
					E2prom_Write_Byte(EEP_IP + 3, Modbus.ip_addr[3]);
					
					
					E2prom_Write_Byte(EEP_SUBNET, Modbus.subnet[0]);
					E2prom_Write_Byte(EEP_SUBNET + 1, Modbus.subnet[1]);
					E2prom_Write_Byte(EEP_SUBNET + 2, Modbus.subnet[2]);
					E2prom_Write_Byte(EEP_SUBNET + 3, Modbus.subnet[3]);
					
					E2prom_Write_Byte(EEP_GETWAY, Modbus.getway[0]);
					E2prom_Write_Byte(EEP_GETWAY + 1, Modbus.getway[1]);
					E2prom_Write_Byte(EEP_GETWAY + 2, Modbus.getway[2]);
					E2prom_Write_Byte(EEP_GETWAY + 3, Modbus.subnet[3]);
		#endif

					IP_Change = 1 ;
		//		delay_ms(100);
		//		SoftReset();
		//		tapdev_init() ;
						 
					}
				}
		}
		else if(scan_temp[0] == 0x69)  // time sync and password
		{
			// receive scan info of sub minipanel
			for(i = 0;i < 10;i++)  // time
			{
				 Rtc.all[i] = scan_temp[1+i];
			}			

			//Rtc_Set(Rtc.Clk.year,Rtc.Clk.mon,Rtc.Clk.day,Rtc.Clk.hour,Rtc.Clk.min,Rtc.Clk.sec,0);
			flag_Updata_Clock = 1;
			// update password	
#if 	USER_SYNC		
			if(pData[11] == 0x55)  // enable user and pass
			memcpy((char *)passwords,&pData[11 + i],sizeof(Password_point) * MAX_PASSW);
#endif			
		}
#if 0//TIME_SYNC		
		else if(scan_temp[0] == 0x6b)  // heart beat 
		{			
			check_lost_master = 0;
			// receive other heart beat of sub minipanel
			if(panel_number > scan_temp[1])  // receive small panel number,  
			{
				flag_master_panel = 0;
			}
		}
#endif	
    else if(scan_temp[0] == 0x6c)  // alarm sync
		{		
				if(scan_temp[1] == 0)  // add
				{	
					if(scan_temp[3] != panel_number) 	
					{						
//						generatealarm(&scan_temp[4],255, pData[3], VIRTUAL_ALARM, alarm_at_all, ind_alarm_panel, alarm_panel, 0);		
					}
				}
		
		}	
		else if(scan_temp[0] == 0x65)
		{  // receive scan command
			U32_T device_id;
			BACNET_ADDRESS src;
			STR_SCAN_CMD * ptr;
			
			ptr = (STR_SCAN_CMD *)(&scan_temp[0]);
			
			if(ptr->product != 0)
			{		
#if (ARM_MINI || ARM_CM5 || ARM_WIFI)		
				
//		device_id = ptr->instance_low * 65536L + ptr->instance_hi;
		device_id = HTONS(ptr->instance_low) + HTONS(ptr->instance_hi) * 65536L ;
		
		src.mac[4] = ptr->BAC_port;
		src.mac[5] = ptr->BAC_port >> 8;
		src.mac[0] = ptr->ipaddr[0];
		src.mac[1] = ptr->ipaddr[1];
		src.mac[2] = ptr->ipaddr[2];
		src.mac[3] = ptr->ipaddr[3];
#else
		device_id = swap_word(ptr->instance_hi) * 65536L + swap_word(ptr->instance_low);

		src.mac[4] = ptr->BAC_port >> 8;
		src.mac[5] = ptr->BAC_port;
		src.mac[0] = ptr->ipaddr[0] >> 8;
		src.mac[1] = ptr->ipaddr[1] >> 8;
		src.mac[2] = ptr->ipaddr[2] >> 8;
		src.mac[3] = ptr->ipaddr[3] >> 8;
#endif
				
			src.mac_len = 6;
		
			src.net = 0;
      src.len = 0;		

			address_add(device_id, 480, &src);
			add_remote_panel_db(device_id,&src,ptr->panel_number,NULL,0,BAC_IP,1);
			}
		}
		
		if(rec_scan_index > 0)
		{
		  memcpy(&response_scan_ip ,uip_udp_conn->ripaddr,4);
			response_scan_port = HTONS(uip_udp_conn->rport);
			Response_Scan_Start();
		}
}



void UDP_SCAN_APP(void)
{
//	struct uip_udp_conn *conn;
//	uip_ipaddr_t addr;
	

	
	static u32 t1 = 0;
	static u32 t2 = 0;

	if(uip_poll()) 
	{	
		// auto send
		t1 = uip_timer;
		if(t1 - t2 >= 100)
		{			
			if(flag_newtork_scan)
			{
				uip_send((char *)(network_scan), 5);	
				flag_newtork_scan = 0;
			}
			t2 = uip_timer;
		}
		
	}
		
	
	if(uip_newdata())
  {  // receive data

		private_scan();
		
	}
}
struct uip_udp_conn * Scan_Rec_conn;
static u8 send_scan_index;
void Response_Scan_Start(void)
{
	uip_ipaddr_t addr;
	
	if(Scan_Rec_conn != NULL) 
	{
		uip_udp_remove(Scan_Rec_conn);
	}

	uip_ipaddr(addr, (U8_T)(response_scan_ip), (U8_T)(response_scan_ip >> 8), (U8_T)(response_scan_ip >> 16), (U8_T)(response_scan_ip >> 24));	
	Scan_Rec_conn = uip_udp_new(&addr, HTONS(response_scan_port));
	if(Scan_Rec_conn != NULL) { 
		uip_udp_bind(Scan_Rec_conn, HTONS(UDP_SCAN_LPORT));
		send_scan_index = 0;
			
  }

}

void Response_Scan_appcall(void)
{
	static u32 t1 = 0;
	static u32 t2 = 0;

	if(uip_poll()) 
	{	
		// auto send
		t1 = uip_timer;
		if(t1 - t2 >= 100)
		{			
			if(send_scan_index < rec_scan_index)
			{
				uip_send(&Infor[send_scan_index++], sizeof(STR_SCAN_CMD));	
			}			
			t2 = uip_timer;
		}
		
	}
}

//uint8_t flag_update_tstat;
U8_T TimeSync_Tstat(void)
{
	U8_T buf[30];
	U16_T crc_val;
	U16_T length;
	U16_T crc_check;
	U8_T port;
	STR_Sch_To_T8 * str;
	static U8_T retry = 0;
	static U8_T j = 0;
	
	do
	{
		str = &Sch_To_T8[j];
		if(str->f_time_sync == 1) 
			break;
	}
	while(j++ < 100);
	if(j >= 100) { j = 0; return 0;}
	
	retry++;
	if(retry > 10) 
	{// generate a alarm, sync time error		
		retry = 0;
		return 0;
	}
	
	port = get_port_by_id(str->schedule_id);
	if(port == 0) 
	{  // wrong id
	    return 0;
	}
//	if(f_time_sync == 0) return;
// send broadcase command to change time of tstat

// It is broadcast command, sycn three ports	
	port = port - 1;
	
	buf[0] = str->schedule_id;;
	buf[1] = 0x10;
	buf[2] = 0x01;
	buf[3] = 0x9a;
	buf[4] = 0x00;
	buf[5] = 0x07;
	buf[6] = 0x0e;  // byte count
	
	buf[7] = 0x00;
	buf[8] = Rtc.Clk.year;
	buf[9] = 0x00;
	buf[10] = Rtc.Clk.mon;
	buf[11] = 0x00;
	buf[12] = Rtc.Clk.week;
	buf[13] = 0x00;
	buf[14] = Rtc.Clk.day;
	buf[15] = 0x00;
	buf[16] = Rtc.Clk.hour;
	buf[17] = 0x00;
	buf[18] = Rtc.Clk.min;
	buf[19] = 0x00;
	buf[20] = Rtc.Clk.sec;
	
	crc_val = crc16(buf, 21);
	buf[21] = HIGH_BYTE(crc_val);
	buf[22] = LOW_BYTE(crc_val);

	if(port == 2) Test[0]++;
	uart_init_send_com(port);
	
	uart_send_string(buf,23,port);
	
	set_subnet_parameters(RECEIVE, 8,port);
	// send successful if receive the reply

	if(length = wait_subnet_response(500,port))
	{
		crc_check = crc16(subnet_response_buf, length - 2);
		if(crc_check == subnet_response_buf[length - 2] * 256 + subnet_response_buf[length - 1])
		{  
			
			//flag_update_tstat = 0;
			
		}
		else 
			return 0;
	}
	else
		 return 0;
	set_subnet_parameters(SEND, 0,port);

	str->f_time_sync = 0;
	retry = 0;
	return 1;
}


// @param 	id: aim id
// @parma 	schedule_index : which schedule

#define SCHEDUEL_TIME_REG_START 813
//U8_T flag_sync_schedule;
//U8_T	schedule_id;
//U8_T schedule_index;
int Send_Ptransfer_to_Sub(U8_T *p, U16_T length,U8_T port);
U8_T Schedule_Sync_Tstat(U8_T protocal)
{
	U8_T buf[40];
	U16_T crc_val;
	U16_T length;
	U16_T crc_check;
	static U8_T i = 0;
	U16_T multi_write_addr;
	U16_T write_addr;
	U8_T port;
	static U8_T retry = 0;	
	static U8_T j = 0;
	STR_Sch_To_T8 * str;
	
	
	do
	{
		str = &Sch_To_T8[j];
		if(str->f_schedule_sync == 1) 
			break;
	}
	while(j++ < 100);
	if(j >= 100) { j = 0; return 0;	}	
	
	if(str->count_send_schedule++ < 10)  
		return 0;
	
	retry++;
	if(retry > 10) 
	{// generate a alarm, sync schedule error
		
		retry = 0;
		return 0;
	}
	
	port = get_port_by_id(str->schedule_id);
	if(port == 0) 
	{  // wrong id
	    return 0;
	}

	if(str->schedule_index <= 0)
		return 0;
	
	port = port - 1;
// multi-write weekly roution time	
	for(;i < 8;i++)
	{
		buf[0] = str->schedule_id;
		buf[1] = 0x10;
		multi_write_addr = 813 + i * 12;
		buf[2] = multi_write_addr >> 8;
		buf[3] = multi_write_addr;
		buf[4] = 0x00;
		buf[5] = 0x0c;
		buf[6] = 0x18;  // byte count

		buf[7] = 0x00;		
		buf[8] = wr_times[str->schedule_index - 1][i].time[0].hours;		
		buf[9] = 0x00;
		buf[10] = wr_times[str->schedule_index - 1][i].time[0].minutes;
		buf[11] = 0x00;
		buf[12] = wr_times[str->schedule_index - 1][i].time[1].hours;
		buf[13] = 0x00;
		buf[14] = wr_times[str->schedule_index - 1][i].time[1].minutes;
		buf[15] = 0x00;
		buf[16] = wr_times[str->schedule_index - 1][i].time[2].hours;
		buf[17] = 0x00;
		buf[18] = wr_times[str->schedule_index - 1][i].time[2].minutes;
		buf[19] = 0x00;
		buf[20] = wr_times[str->schedule_index - 1][i].time[3].hours;
		buf[21] = 0x00;
		buf[22] = wr_times[str->schedule_index - 1][i].time[3].minutes;
		buf[23] = 0x00;
		buf[24] = wr_times[str->schedule_index - 1][i].time[4].hours;
		buf[25] = 0x00;
		buf[26] = wr_times[str->schedule_index - 1][i].time[4].minutes;
		buf[27] = 0x00;
		buf[28] = wr_times[str->schedule_index - 1][i].time[5].hours;
		buf[29] = 0x00;
		buf[30] = wr_times[str->schedule_index - 1][i].time[5].minutes;
		
		crc_val = crc16(buf, 31);
		buf[31] = HIGH_BYTE(crc_val);
		buf[32] = LOW_BYTE(crc_val);
		
		if(protocal == 0)  // MODBUS
		{
			uart_init_send_com(port);

			uart_send_string(buf,33,port);
			
			set_subnet_parameters(RECEIVE, 8,port);
			// send successful if receive the reply
			if(length = wait_subnet_response(200,port))
			{
				crc_check = crc16(subnet_response_buf, length - 2);
				if(crc_check == subnet_response_buf[length - 2] * 256 + subnet_response_buf[length - 1])
				{  
				}
				else
				{
					 return 0;
				}
			}
			else
			{
				 return 0;
			}
			set_subnet_parameters(SEND, 0,port);
		}
		else 
		{
			return Send_Ptransfer_to_Sub(buf,33,port);
		}
		delay_ms(100);
	}
	if(i < 8) return 0;
	// signal write flag of weely time
	for(;i < 16;i++)
	{
		buf[0] = str->schedule_id;
		buf[1] = 0x06;
		write_addr = 909 + 2 * (i - 8);
		buf[2] = write_addr >> 8;
		buf[3] = write_addr;
		buf[4] = 0x11;
		buf[5] = 0x11;
		crc_val = crc16(buf, 6);
		buf[6] = HIGH_BYTE(crc_val);
		buf[7] = LOW_BYTE(crc_val);
		
		if(protocal == 0)
		{
			uart_init_send_com(port);
			
			uart_send_string(buf,8,port);
			set_subnet_parameters(RECEIVE, 8,port);
			// send successful if receive the reply

			if(length = wait_subnet_response(100,port))
			{
				crc_check = crc16(subnet_response_buf, length - 2);
				if(crc_check == subnet_response_buf[length - 2] * 256 + subnet_response_buf[length - 1])
				{
					
				}
				else
				{
					 return 0;
				}
			}
			else
			{
				return Send_Ptransfer_to_Sub(buf,8,port);
			}
		}
		else
		{
			 return 0;
		}
		delay_ms(500);
		
		buf[0] = str->schedule_id;
		buf[1] = 0x06;
		write_addr = 909 + 2 * (i - 8) + 1;
		buf[2] = write_addr >> 8;
		buf[3] = write_addr;
		buf[4] = 0;
		buf[5] = 0x11;
		crc_val = crc16(buf, 6);
		buf[6] = HIGH_BYTE(crc_val);
		buf[7] = LOW_BYTE(crc_val);
		
		if(protocal == 0)
		{
			uart_init_send_com(port);
			uart_send_string(buf,8,port);
			
			set_subnet_parameters(RECEIVE, 8,port);
			// send successful if receive the reply

			if(length = wait_subnet_response(100,port))
			{
				crc_check = crc16(subnet_response_buf, length - 2);
				if(crc_check == subnet_response_buf[length - 2] * 256 + subnet_response_buf[length - 1])
				{  
				}
				else
				{
					 return 0;
				}
			}
			else
			{
				return Send_Ptransfer_to_Sub(buf,8,port);
			}
		}
		else
		{
			 return 0;
		}
	}
	if(i >= 16) i = 0;
	else
		return 0;
	str->f_schedule_sync = 0;
	retry = 0;
	return 1;
}


/* End of gudpbc.c */
