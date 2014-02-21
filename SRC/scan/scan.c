
#include <stdlib.h>
#include <string.h>
#include "scan.h"
#include "main.h"
#include "commsub.h"

#define	MAX_EXT_TST					254
#define	SCAN_DB_SIZE				5

#define ScanSTACK_SIZE					1024
#define ParameterOperationSTACK_SIZE	512
xTaskHandle Handle_Scan, Handle_ParameterOperation;

extern xSemaphoreHandle sem_subnet_tx_uart0;
extern xSemaphoreHandle sem_subnet_tx_uart2;
extern xSemaphoreHandle sem_subnet_tx_uart1;
#define STACK_LEN  5
#define MAX_WRITE_RETRY 10
typedef struct
{
	U8_T id;
	U16_T reg;
	U16_T value;
	U8_T flag;
	U8_T retry;
}STR_NODE_OPERATE;
typedef enum { WRITE_OK = 0,WAIT_FOR_WRITE};

STR_NODE_OPERATE xdata node_write[STACK_LEN];

extern U8_T subnet_rec_package_size;
extern U8_T subnet_response_buf[255];

U16_T crc16(U8_T *p, U8_T length);
//void uart_send_string(U8_T *p, U16_T length,U8_T port);

SCAN_DB far scan_db[MAX_ID];
SCAN_DB far current_db;
U8_T db_ctr = 1;
U8_T reset_scan_db_flag = 0;

U8_T db_online[32];	// Should be added by scan
U8_T db_occupy[32]; // Added/subtracted by scan
U8_T current_online[32]; // Added/subtracted by co2 request command
U8_T current_online_ctr = 0;

U8_T get_para[32];

static U8_T scan_db_changed = FALSE;

static U8_T node_operate_index = 0;
static U16_T node_operate_reg = 0;
static U16_T node_operate_value = 0;
static U8_T node_operate_flag = 0;

U8_T external_nodes_plug_and_play = 0;





void recount_sub_addr(void)
{	
	U8_T i;
	U8_T tmp_uart0 = 0;
#if  defined(MINI)
	U8_T tmp_uart1 = 0;
	U8_T tmp_uart2 = 0;
#endif
	for(i = 1;i < db_ctr + 1;i++)
	{
		if(scan_db[i].port == UART_SUB1 + 1)
			uart0_sub_addr[tmp_uart0++]	= scan_db[i].id;
#if  defined(MINI)
		else if(scan_db[i].port == UART_ZIG + 1)
			uart1_sub_addr[tmp_uart1++]	= scan_db[i].id;
		else if(scan_db[i].port == UART_SUB2 + 1)
			uart2_sub_addr[tmp_uart2++]	= scan_db[i].id;
#endif			
	}
	uart0_sub_no = tmp_uart0;
	memcpy(sub_addr,uart0_sub_addr,uart0_sub_no);

#if  defined(MINI)

	uart1_sub_no = tmp_uart1;
	uart2_sub_no = tmp_uart2;

	sub_no = uart0_sub_no + uart1_sub_no + uart2_sub_no;

   	memcpy(sub_addr + uart0_sub_no,uart1_sub_addr,uart1_sub_no);
	memcpy(sub_addr + uart0_sub_no + uart1_sub_no,uart2_sub_addr,uart2_sub_no);

#endif
    for(i = 0;i < sub_no;i++)
	{
		tst_info[i].tst_db.port = scan_db[i].port;
		tst_info[i].tst_db.id = scan_db[i].id;
		tst_info[i].tst_db.sn = scan_db[i].sn;

	}
}


void Get_Tst_DB_From_Flash(void)
{
	U8_T i;
	U8_T tmp_uart0 = 0;
#if  defined(MINI)
	U8_T tmp_uart1 = 0;
	U8_T tmp_uart2 = 0;
#endif
	for(i = 0;i < SUB_NO;i++)
	{
		scan_db[i].port = tst_info[i].tst_db.port;
		scan_db[i].id = tst_info[i].tst_db.id;
		scan_db[i].sn = tst_info[i].tst_db.sn;
		if(scan_db[i].port != 0) 
		{ 					
			db_online[i / 8] |= 1 << (i % 8);
			db_occupy[i / 8] &= ~(1 << (i % 8));
			get_para[db_ctr / 8] |= 1 << (db_ctr % 8);
			db_ctr++;	
		}
	}
	
	for(i = 1;i < db_ctr + 1;i++)
	{
		if(scan_db[i].port == UART_SUB1 + 1)
			uart0_sub_addr[tmp_uart0++]	= scan_db[i].id;
#if  defined(MINI)
		else if(scan_db[i].port == UART_SUB1 + 1)
			uart1_sub_addr[tmp_uart1++]	= scan_db[i].id;
		else if(scan_db[i].port == UART_SUB2 + 1)
			uart2_sub_addr[tmp_uart2++]	= scan_db[i].id;
#endif			
	}
	
	uart0_sub_no = tmp_uart0;
	memcpy(sub_addr,uart0_sub_addr,uart0_sub_no);

#if  defined(MINI)

	uart1_sub_no = tmp_uart1;
	uart2_sub_no = tmp_uart2;

	sub_no = uart0_sub_no + uart1_sub_no + uart2_sub_no;

   	memcpy(sub_addr + uart0_sub_no,uart1_sub_addr,uart1_sub_no);
	memcpy(sub_addr + uart0_sub_no + uart1_sub_no,uart2_sub_addr,uart2_sub_no);

#endif	
}



U8_T receive_scan_reply(U8_T *p, U8_T len)
{
	U16_T crc_check = crc16(p, 7); // crc16
	if((subnet_response_buf[0] = 0xff) && (subnet_response_buf[1] == CHECKONLINE)
		&& (HIGH_BYTE(crc_check) == p[7]) && (LOW_BYTE(crc_check) == p[8]))
	{
		current_db.id = p[2];
		current_db.sn = ((U32_T)p[6] << 24) | ((U32_T)p[5] << 16) | 
						((U32_T)p[4] << 8) | p[3];
		
		if((len == subnet_rec_package_size) /*&& (subnet_response_buf[2] == subnet_response_buf[3])*/)
		{	
			return UNIQUE_ID;
		}
		else
		{
			return UNIQUE_ID_FROM_MULTIPLE;
		}
	}
	else
	{  
		return MULTIPLE_ID;
	}
}

U8_T send_scan_cmd(U8_T max_id, U8_T min_id,U8_T port)
{
	U16_T wCrc16;	
	U8_T buf[6];
	U8_T length;
	U8_T ret;
	xSemaphoreHandle  tempsem;
	if(port == UART_SUB1)	
	{			
		tempsem = sem_subnet_tx_uart0;
	}
#if  defined(MINI)

	else if(port == UART_SUB2)	
	{	
		tempsem = sem_subnet_tx_uart2;
	}
	else if(port == UART_ZIG)	
	{	
		tempsem = sem_subnet_tx_uart1;
	}
#endif

	if(cSemaphoreTake(tempsem, 50) == pdFALSE)
		return SCAN_BUSY;
	uart_init_send_com(port);
	Test[15]++;
	buf[0] = 0xff;
	buf[1] = 0x19;
	buf[2] = max_id;
	buf[3] = min_id;

	wCrc16 = crc16(buf, 4);

	buf[4] = HIGH_BYTE(wCrc16);
	buf[5] = LOW_BYTE(wCrc16);
	uart_send_string(buf, 6,port);
	set_subnet_parameters(RECEIVE,9,port);
	if(length = wait_subnet_response(200,port))
	{	
		if(port == UART_SUB1) 
		{
			memcpy(subnet_response_buf,sub1_data_buffer,length);
		}
#if  defined(MINI)

		else if(port == UART_SUB2) 
		{
			memcpy(subnet_response_buf,hsurRxBuffer,length);
		}
		else if(port == UART_ZIG)
		{
			memcpy(subnet_response_buf,zig_data_buffer,length);
		}
#endif
		ret = receive_scan_reply(subnet_response_buf, length);
	}
	else // NONE_ID || MULTIPLE_ID
	{		
		if(length > subnet_rec_package_size)
		{	
			ret = MULTIPLE_ID;
		}
		else
			ret = NONE_ID;
	}

	set_subnet_parameters(SEND, 0,port);
	cSemaphoreGive(tempsem);
	return ret;
}

U8_T receive_assign_id_reply(U8_T *p, U8_T length)
{
	U16_T crc_check = crc16(p, 10); // crc16
	if((length == subnet_rec_package_size) && (HIGH_BYTE(crc_check) == p[10]) && (LOW_BYTE(crc_check) == p[11]))
		return ASSIGN_ID;
	else
		return NONE_ID;
}

U8_T assignment_id_with_sn(U8_T old_id, U8_T new_id, U32_T current_sn,U8_T port)
{
	U8_T buf[12];
	U16_T wCrc16;
	U8_T length, ret;
	xSemaphoreHandle  tempsem;

	if(port == UART_SUB1)	
	{
		tempsem = sem_subnet_tx_uart0;
	}
#if  defined(MINI)

	else if(port == UART_SUB2)	
	{
		tempsem = sem_subnet_tx_uart2;
	}
	else if(port == UART_ZIG)	
	{
		tempsem = sem_subnet_tx_uart1;
	}
#endif
	if(cSemaphoreTake(tempsem, 10) == pdFALSE)
		return SCAN_BUSY;

	uart_init_send_com(port);

	buf[0] = old_id;
	buf[1] = 0x06;
	buf[2] = 0;
	buf[3] = 0x0a;	//MODBUS_ADDRESS_PLUG_N_PLAY = 10
	buf[4] = 0x55;
	buf[5] = new_id;

	buf[6] = (U8_T)(current_sn);
	buf[7] = (U8_T)(current_sn >> 8);
	buf[8] = (U8_T)(current_sn >> 16);
	buf[9] = (U8_T) (current_sn >> 24);

	wCrc16 = crc16(buf, 10);

	buf[10] = HIGH_BYTE(wCrc16);
	buf[11] = LOW_BYTE(wCrc16);

	uart_send_string(buf, 12,port);
	set_subnet_parameters(RECEIVE, 12,port);

	if(length = wait_subnet_response(10,port))
	{
		if(port == UART_SUB1) 
		{
			memcpy(subnet_response_buf,sub1_data_buffer,length);
		}
#if  defined(MINI)

		else if(port == UART_SUB2) 
		{
			memcpy(subnet_response_buf,hsurRxBuffer,length);
		}
		else if(port == UART_ZIG) 
		{
			memcpy(subnet_response_buf,zig_data_buffer,length);
		}
#endif		
		ret = receive_assign_id_reply(subnet_response_buf, length);
	}

	set_subnet_parameters(SEND, 0,port);
	cSemaphoreGive(tempsem);
	return ret;
}

U8_T get_idle_id(void)
{
	U8_T i;
	for(i = 1; i < MAX_ID; i++)
	{
		if(((db_online[i / 8] & (1 << (i % 8))) == 0)	&& ((db_occupy[i / 8] & (1 << (i % 8))) == 0))
			return i;
	}
	return 0xff;
}

void check_id_in_database(U8_T id, U32_T sn,U8_T port)
{
	if(db_online[id / 8] & (1 << (id % 8))) // in the database
	{
		U8_T i;
		for(i = 0; i < db_ctr; i++)
		{
			if(id == scan_db[i].id) // id already in the database
			{
				if(sn != scan_db[i].sn) // if there exist the same id with defferent sn, push it into the db_occupy list
				{
					if(external_nodes_plug_and_play == 0)
					{
						remove_id_from_db(i);
					}
					else
						db_occupy[id / 8] |= 1 << (id % 8);
				}
				break;
			}
			// if the device is already in the database, return without doing anything
		}
	}
	else
	{
		db_online[id / 8] |= 1 << (id % 8);
		db_occupy[id / 8] &= ~(1 << (id % 8));
		get_para[db_ctr / 8] |= 1 << (db_ctr % 8);
		scan_db[db_ctr].id = id;
		scan_db[db_ctr].sn = sn; 
		scan_db[db_ctr].port = port + 1;

		map_id_port[db_ctr - 1].id = id;
		map_id_port[db_ctr - 1].port = port + 1;

		db_ctr++;
		
		scan_db_changed = TRUE;
	}
}

void bin_search(U8_T min_id, U8_T max_id,U8_T port) reentrant
{
	U8_T scan_status;
	if(min_id > max_id) return;
	

	scan_status = send_scan_cmd(max_id, min_id, port);
	switch(scan_status)	// wait for response from nodes scan command
	{
		case UNIQUE_ID:
			// unique id means it is the only id in the range.
			// if the id is already in the database, set it occupy and then change the id with sn in the dealwith_conflict_id routine.
			check_id_in_database(current_db.id, current_db.sn,port);
			if(min_id != max_id) // to avoid the miss reply some nodes
			{
				bin_search(min_id, (U8_T)(((U16_T)min_id + (U16_T)max_id) / 2),port);
				bin_search((U8_T)(((U16_T)min_id + (U16_T)max_id) / 2) + 1, max_id,port);
			}			
			break;
		case MULTIPLE_ID: 
			// multiple id means there is more than one id in the range.
			// if the min_id == max_id, there is same id, set the id occupy and return
			// if the min_id != max_id, there is multi id in the range, divide the range and do the sub scan
			if(min_id == max_id)
			{
				db_occupy[min_id / 8] |= 1 << (min_id % 8);
				if((db_online[min_id / 8] & (1 << (min_id % 8))) && (external_nodes_plug_and_play == 0))
				{
					U8_T i = 0;
					for(i = 0; i < db_ctr; i++)
						if(scan_db[i].id == min_id)
							break;
					remove_id_from_db(i);
					
				}
			}
			else
			{	
				bin_search(min_id, (U8_T)(((U16_T)min_id + (U16_T)max_id) / 2),port);
				bin_search((U8_T)(((U16_T)min_id + (U16_T)max_id) / 2) + 1, max_id,port);
			}
			break;
		case UNIQUE_ID_FROM_MULTIPLE:  
			// there are multiple ids in the range, but the fisrt reply is good.
			if(min_id == max_id)
			{
				db_occupy[min_id / 8] |= 1 << (min_id % 8);
				if((db_online[min_id / 8] & (1 << (min_id % 8))) && (external_nodes_plug_and_play == 0))
				{
					U8_T i = 0;
					
					for(i = 0; i < db_ctr; i++)
						if(scan_db[i].id == min_id)
							break;
					remove_id_from_db(i);
				
				}
			}
			else
			{
				

				check_id_in_database(current_db.id, current_db.sn,port);
				bin_search(min_id, (U8_T)(((U16_T)min_id + (U16_T)max_id) / 2),port);
				bin_search((U8_T)(((U16_T)min_id + (U16_T)max_id) / 2) + 1, max_id,port);
			}
			break;
		case NONE_ID: 
			// none id means there is not id in the range
			break;
		case SCAN_BUSY:
		default:
			bin_search(min_id, max_id,port);
			break;
	}

	return;
}

void dealwith_conflict_id(U8_T port)
{
	U8_T idle_id;
	U8_T status;
	U8_T occupy_id = 1;

	while(1)
	{
		if(db_occupy[occupy_id / 8] & (1 << (occupy_id % 8)))
		{
			idle_id = get_idle_id();
			if(idle_id == 0xff) break;

			status = send_scan_cmd(occupy_id, occupy_id,port); // get the seperate sn

			if((status == UNIQUE_ID) || (status == UNIQUE_ID_FROM_MULTIPLE))
			{
				if(occupy_id == current_db.id)
					db_occupy[occupy_id / 8] &= ~(1 << (occupy_id % 8));

				check_id_in_database(current_db.id, current_db.sn,port);
				// if still occupy in the database
				if(db_occupy[current_db.id / 8] & (1 << (current_db.id % 8)))
				{
					// assign idle id with sn to this occupy device.
					if(assignment_id_with_sn(current_db.id, idle_id, current_db.sn,port)== ASSIGN_ID)
					{
						db_online[idle_id / 8] |= 1 << (idle_id % 8);
					
						scan_db[db_ctr].id = idle_id;
      					scan_db[db_ctr].sn = current_db.sn;	
						

						map_id_port[db_ctr - 1].id = idle_id;
						map_id_port[db_ctr - 1].port = port + 1;


						db_ctr++;
						scan_db_changed = TRUE;
					}
				}

				if(status == UNIQUE_ID_FROM_MULTIPLE)
					continue;
			}
			else if(status == MULTIPLE_ID)
			{
				continue;
			}
			else // if(status == NONE_ID)
			{
				// maybe the nodes are removed from the subnet, so skip it.
			}
		}
		
		occupy_id++;
		if(occupy_id == 0xff) break;
	}

	return;
}

void scan_sub_nodes(U8_T port)
{
	bin_search(1, 254,port);
	if(external_nodes_plug_and_play == 1)
		dealwith_conflict_id(port);

//	if(scan_db_changed == TRUE)
//	{
////		start_data_save_timer();
//		scan_db_changed = FALSE;
//	}
}

// read from flash to get the init db_online status
void init_scan_db(void)
{
	U8_T i;

	U8_T local_id = Modbus.address;
	U32_T local_sn = ((U32_T)Modbus.serialNum[3] << 24) | ((U32_T)Modbus.serialNum[2] << 16) | \
				((U32_T)Modbus.serialNum[1] << 8) | Modbus.serialNum[0];

	memset(db_online, 0, 32);
	memset(db_occupy, 0, 32);
	memset(get_para, 0, 32);
	memset(current_online, 0, 32);
	memset((uint8 *)(&scan_db[0].id), 0, SCAN_DB_SIZE * MAX_EXT_TST);
	current_online_ctr = 0;

	if((db_ctr == 0) || (db_ctr == 0xff))
		db_ctr = 1;

	for(i = 0; i < db_ctr; i++)
	{
		if(i == 0)
		{
			if((scan_db[i].id != local_id) || (scan_db[i].sn != local_sn))
			{
				scan_db[i].id = local_id;
				scan_db[i].sn = local_sn;
//				start_data_save_timer();
			}
		}

		db_online[scan_db[i].id / 8] |= (1 << (scan_db[i].id % 8));
	}
}

void clear_scan_db(void)
{
	db_ctr = 0;
	init_scan_db();
//	start_data_save_timer();
}

void remove_id_from_db(U8_T index)
{
	U8_T i;
	if((db_ctr > 1) && (index < db_ctr) && (index > 0)) // can not delete the internal sensor
	{
		i = scan_db[index].id;
		db_online[i / 8] &= ~(1 << (i % 8));
		db_occupy[i / 8] &= ~(1 << (i % 8));
		if(current_online[i / 8] & (1 << (i % 8)))
		{
			current_online[i / 8] &= ~(1 << (i % 8));
			current_online_ctr--;
		}

		for(i = index; i < db_ctr - 1; i++)
		{
			scan_db[i].id = scan_db[i + 1].id;
			scan_db[i].sn = scan_db[i + 1].sn;
			scan_db[i].port	= scan_db[i + 1].port;
		}

		for(i = index; i < db_ctr - 1; i++)
		{
			map_id_port[i].id = map_id_port[i + 1].id;
			map_id_port[i].port = map_id_port[i + 1].port;
		}

	
		db_ctr--;
		scan_db_changed = TRUE;
	}
}

U8_T check_master_id_in_database(U8_T set_id, U8_T increase) reentrant
{
	U8_T i;

	if((set_id == 0) && (increase == 0))
		set_id = 254;
	
	if((set_id == 255) && (increase == 1))
		set_id = 1;

	for(i = 1; i < db_ctr; i++)
	{
		if(scan_db[i].id == set_id)
			break;
	}

	if(i >= db_ctr)
		return set_id;
	else
	{
		if(increase == 1)
			return check_master_id_in_database(set_id+1, 1);
		else
			return check_master_id_in_database(set_id-1, 0);
	}
}

void modify_master_id_in_database(U8_T old_id, U8_T set_id)
{
	Modbus.address = set_id;
	scan_db[0].id = set_id;
	E2prom_Write_Byte(EEP_ADDRESS, Modbus.address);

	// modify scan datebase
	db_online[old_id / 8] &= ~(1 << (old_id % 8));
	db_online[set_id / 8] |= 1 << (set_id % 8);

//	start_data_save_timer();
}

void get_parameters(uint8 index, uint8 *p,uint8 reg)
{
	U16_T crc_check = crc16(p, 5); // crc16
	if((HIGH_BYTE(crc_check) == p[5]) && (LOW_BYTE(crc_check) == p[6]))
	{
		switch(reg)
		{  
			case TST_PRODUCT_MODEL:  
				if(p[4] == 6 || p[4] == 7)
					tst_info[index].type = TSTAT_6;
				else if(p[4] == 1 || p[4] == 2 ||p[4] == 3 ||p[4] == 4 ||p[4] == 12||p[4] == 17)
				{	
					tst_info[index].type = TSTAT_5A;				
				}
				else if(p[4] == 16 || p[4] == 19)
					tst_info[index].type = TSTAT_5E;
				tst_info[index].product_model = p[4];
				break;
			case TST_OCCUPIED: tst_info[index].occupied = (U16_T)((p[3] << 8) | p[4]);	  //???????????????????
				break;
			case TST_COOL_SETPOINT:	 tst_info[index].cool_setpoint = (U16_T)((p[3] << 8) | p[4]);
				break;
			case TST_HEAT_SETPOINT:   	tst_info[index].heat_setpoint = (U16_T)((p[3] << 8) | p[4]);
				break;
			case TST_ROOM_SETPOINT:	tst_info[index].setpoint = (U16_T)((p[3] << 8) | p[4]);
				break;
			case TST_ROOM_TEM:  tst_info[index].temperature = (U16_T)((p[3] << 8) | p[4]);
				break;
			case TST_MODE:  tst_info[index].mode = (U16_T)((p[3] << 8) | p[4]);
				break;
			case TST_OUTPUT_STATE:	  tst_info[index].output_state = (U16_T)((p[3] << 8) | p[4]);
				break;
			case TST_NIGHT_HEAT_DB:   tst_info[index].night_cool_db = (U16_T)((p[3] << 8) | p[4]);
				break;
			case TST_NIGHT_COOL_DB:   tst_info[index].night_heat_db = (U16_T)((p[3] << 8) | p[4]);
				break;
			case TST_NIGHT_HEAT_SP:   tst_info[index].night_heat_sp = (U16_T)((p[3] << 8) | p[4]);
				break;
			case TST_NIGHT_COOL_SP:   tst_info[index].night_cool_sp = (U16_T)((p[3] << 8) | p[4]);
				break;
//			case TST_OVER_RIDE:		tst_info[index].over_ride = (U16_T)((p[3] << 8) | p[4]);
//				break;
//			case TST_SERIAL_NUM_0:	tst_info[index].serial_number[0] = (U16_T)((p[3] << 8) | p[4]);
//				break;
//			case TST_SERIAL_NUM_1:	tst_info[index].serial_number[1] = (U16_T)((p[3] << 8) | p[4]);
//				break;
//			case TST_SERIAL_NUM_2:	tst_info[index].serial_number[2] = (U16_T)((p[3] << 8) | p[4]);
//				break;
//			case TST_SERIAL_NUM_3:	tst_info[index].serial_number[3] = (U16_T)((p[3] << 8) | p[4]);
//				break;
//			case TST_ADDRESS:			tst_info[index].address = (U16_T)((p[3] << 8) | p[4]);
//				break;
			break;
		}
		get_para[(index + 1) / 8] &= ~(1 << ((index + 1) % 8));	
	}
}

void get_parameters_from_nodes(U8_T port)
{
	U8_T i, j, length;
	U8_T buf[8];
	U16_T crc_check;
	static U8_T current_index0 = 0;
	static U8_T current_index1 = 0;
	static U8_T current_index2 = 0;
	static U8_T sub_reg0 = READ_PRODUCT_MODLE; 
	static U8_T sub_reg1 = READ_PRODUCT_MODLE;
	static U8_T sub_reg2 = READ_PRODUCT_MODLE;
	U8_T type = 0; 
	U8_T sub_reg;
	U8_T tst_id;
	xSemaphoreHandle  tempsem;

	if(port == UART_SUB1)	
	{			
		if(uart0_sub_no == 0)	return;

		tempsem = sem_subnet_tx_uart0;
		i = current_index0;
		sub_reg = sub_reg0;
		tst_id = uart0_sub_addr[i];
		type = tst_info[i].type;
		j = i;
	}
#if  defined(MINI)
	else if(port == UART_ZIG)	
	{			
		if(uart1_sub_no == 0)	return;

		tempsem = sem_subnet_tx_uart1;
		i = current_index1;
		sub_reg = sub_reg1;
		tst_id = uart1_sub_addr[i];
		type = tst_info[i].type;
		i = current_index1 + uart0_sub_no;
		j = i;
	}
	else if(port == UART_SUB2)	
	{			
		if(uart2_sub_no == 0)	return;
		tempsem = sem_subnet_tx_uart2;
		i = current_index2;
		sub_reg = sub_reg2;
		tst_id = uart2_sub_addr[i];
		type = tst_info[i].type;
		i = current_index2 + uart0_sub_no + uart1_sub_no;
		j = i;
	} 
#endif
	if(type == 0xff)  // 0xff initial type
	{		
		if(sub_reg == READ_PRODUCT_MODLE)
			type = 0;
		else
			return;
	}

	if(cSemaphoreTake(tempsem, 5) == pdFALSE)	
		return ;

	uart_init_send_com(port);

	buf[0] = tst_id;
	buf[1] = READ_VARIABLES;
	buf[2] = HIGH_BYTE(Tst_Register[sub_reg][type]);
	buf[3] = LOW_BYTE(Tst_Register[sub_reg][type]); // start address
	buf[4] = 0;
	buf[5] = 1;

	crc_check = crc16(buf, 6); // crc16
	buf[6] = HIGH_BYTE(crc_check);
	buf[7] = LOW_BYTE(crc_check);

	uart_send_string(buf, 8,port);
	set_subnet_parameters(RECEIVE, 7,port);
	if(length = wait_subnet_response(10,port))
	{
		if(port == UART_SUB1) 
		{
			memcpy(subnet_response_buf,sub1_data_buffer,length);
		}
#if  defined(MINI)
		else if(port == UART_ZIG) 
		{
			memcpy(subnet_response_buf,zig_data_buffer,length);
		}
		else if(port == UART_SUB2) 
		{
			memcpy(subnet_response_buf,hsurRxBuffer,length);
		}
#endif		

		get_parameters(j, subnet_response_buf,sub_reg);

		if(port == UART_SUB1)
		{
			if(current_index0 < uart0_sub_no && sub_reg == TST_REG_END - 1) 
			{ 	
				current_index0++; 
				if(current_index0 == uart0_sub_no) 
					current_index0 = 0;
			}
		}
#if  defined(MINI)

		if(port == UART_ZIG)
		{
			if(current_index1 < uart1_sub_no && sub_reg == TST_REG_END - 1) 
			{ 	
				current_index1++; 
				if(current_index1 == uart1_sub_no) 
					current_index1 = 0;
			}
		}
		else
		{
			if(current_index2 < uart2_sub_no && sub_reg == TST_REG_END - 1) 
			{ 	
				current_index2++; 
				if(current_index2 == uart2_sub_no) 
					current_index2 = 0;
			}
		}
#endif
		if(sub_reg < TST_REG_END - 1) 	 sub_reg++;
		else 
			 sub_reg = READ_PRODUCT_MODLE;

		if(port == UART_SUB1)	sub_reg0 = sub_reg;
#if  defined(MINI)
		else if(port == UART_ZIG)  sub_reg1 = sub_reg;
		else 	   sub_reg2 = sub_reg;
#endif
	}

	set_subnet_parameters(SEND, 0, port);
	cSemaphoreGive(tempsem);
//		return;
}

void write_parameters_to_nodes(uint8 index, uint16 reg, uint16 value)
{
	U8_T i;
	for(i = 0;i < STACK_LEN;i++)
	{
		if(node_write[i].flag == WRITE_OK) 	break;
	}
	if(i == STACK_LEN)		
	{  // stack full
	// tbd
		return;	
	}
	else
	{	
		node_write[i].id = index;
		node_write[i].reg = reg;
		node_write[i].value = value;
		node_write[i].flag = WAIT_FOR_WRITE;
		node_write[i].retry = 0;
	}
}

void check_write_to_nodes(U8_T port)
{
	U8_T buf[8], length;
	U16_T crc_check;
	xSemaphoreHandle  tempsem;
	U8_T i;

	if(port == UART_SUB1)	
	{
		tempsem = sem_subnet_tx_uart0;
	}
#if  defined(MINI)
	else if(port == UART_ZIG)	
	{
		tempsem = sem_subnet_tx_uart1;
	}
	else if(port == UART_SUB2)	
	{
		tempsem = sem_subnet_tx_uart2;
	}
#endif 

	for(i = 0;i < STACK_LEN;i++)
	{
		if(node_write[i].flag == WAIT_FOR_WRITE) //	get current index, 1 -- WAIT_FOR_WRITE, 0 -- WRITE_OK
		{
			if(node_write[i].retry < MAX_WRITE_RETRY)
			{
				node_write[i].retry++;
				break;
			}
			else
			{  	// retry 10 time, give up
				node_write[i].flag = WRITE_OK; 
			}
		}
	}

	if(i == STACK_LEN)		// no WAIT_FOR_WRITE
		return;	

	Test[45]++;

	if(cSemaphoreTake(tempsem, 5) == pdFALSE)
		return;

	uart_init_send_com(port);
	buf[0] = sub_addr[node_operate_index];
	buf[1] = WRITE_VARIABLES;
	buf[2] = HIGH_BYTE(node_operate_reg);
	buf[3] = LOW_BYTE(node_operate_reg); // start address

	buf[4] = HIGH_BYTE(node_operate_value);
	buf[5] = LOW_BYTE(node_operate_value);

	crc_check = crc16(buf, 6); // crc16
	buf[6] = HIGH_BYTE(crc_check);
	buf[7] = LOW_BYTE(crc_check);

	uart_send_string(buf, 8,port);
	set_subnet_parameters(RECEIVE, 8,port);
	Test[46]++;
	// send successful if receive the reply
	if(length = wait_subnet_response(100,port))
	{	 Test[47]++;
		node_write[i].flag = WRITE_OK; // without doing checksum
	}
	set_subnet_parameters(SEND, 0,port);
	cSemaphoreGive(tempsem);
}

void ScanTask(void)
{
	portTickType xDelayPeriod = (portTickType)2500 / portTICK_RATE_MS;
	U8_T port = UART_SUB1;	
	

	while(1)	 
	{
		
#if defined(MINI) 
		vTaskDelay(3000);
		if(port == UART_SUB1)
			port = UART_SUB2;
		else if(port == UART_SUB2)
		{
			if(Modbus.zigbee == 1)  	
				port = UART_ZIG;
			else 
				port = UART_SUB1;
		} 
#endif	

#if defined(CM5)
	   vTaskDelay(5000);

#endif
		scan_sub_nodes(port);
		if(scan_db_changed == TRUE)
		{
		recount_sub_addr();
		ChangeFlash = 1;
		}
		taskYIELD();
	}
}

void ParameterOperationTask(void)
{
	portTickType xDelayPeriod = (portTickType)100 / portTICK_RATE_MS;
	U8_T port = UART_SUB1;
	while(1)
	{
		vTaskDelay(xDelayPeriod);
#if defined(MINI)
		if(port == UART_SUB1)
			port = UART_SUB2;
		else if(port == UART_SUB2)
		{
			if(Modbus.zigbee == 1)  	port = UART_ZIG;
			else port = UART_SUB1;
		} 
#endif		
		get_parameters_from_nodes(port);
		check_write_to_nodes(port);
//		if(reset_scan_db_flag)
//		{
//			reset_scan_db_flag = 0;
//			clear_scan_db();
//		}
		taskYIELD();
	}
}


U8_T Response_TCPIP_To_SUB(U8_T *buf, U16_T len,U8_T port,U8_T *header)
{
	U16_T length;
	U16_T crc_check;
	U8_T size;
	U16_T delay_time;
	U8_T tmp_sendbuf[300];
	xSemaphoreHandle  tempsem;
	U8_T ret = 0;
//	xQueueHandle tempque;
//	port = UART_SUB1;
	if(port == UART_SUB1)	
	{	
		tempsem = sem_subnet_tx_uart0;
//		tempque = qSubSerial_uart0;
	}
#if  defined(MINI)	
	else if(port == UART_SUB2)	
	{	
		tempsem = sem_subnet_tx_uart2;
//		tempque = qSubSerial_uart2;
	}
	else if(port == UART_ZIG)
	{
		tempsem =  sem_subnet_tx_uart1;
	}
#endif
//	if(cSemaphoreTake(tempsem, 50) == pdFALSE)
//		return ;

	if(buf[1] == 0x03) // read
	{		 
		 size = buf[5] * 2 + 5;
		 delay_time = size * 2;
	}
	else if(buf[1] == 0x06 || buf[1] == 0x10)
	{	
		size = 8;
		if(buf[1] == 0x06 && buf[5] == 0x3f)   // erase flash 
			delay_time = 1000;
		else
			delay_time = 500;
	}
	else if(buf[1] == 0x19)
	{  
		size = 12;
		#if defined(MINI)

		if(port == UART_ZIG )		delay_time = 500;
		else
		#endif
			delay_time = 50;
	}
	else 
	{
//		cSemaphoreGive(tempsem);
		return;
	}

	uart_init_send_com(port);
	memcpy(tmp_sendbuf,buf,len);
	crc_check = crc16(tmp_sendbuf, len);
	tmp_sendbuf[len] = HIGH_BYTE(crc_check);
	tmp_sendbuf[len + 1] = LOW_BYTE(crc_check);
	uart_send_string(tmp_sendbuf, len + 2 ,port);
	
	set_subnet_parameters(RECEIVE,size,port);			
	if(buf[1] == 0x19)	  // scan cmd
	{ 	
		U16_T i;  	

		for(i = 0; i < delay_time; i++)
		{
			if(port == UART_SUB1)
				if(sub1_rece_count > 8)   continue;
		#if  defined(MINI)
			else if(port == UART_SUB2)
				if(hsurRxCount > 8)	 continue;
			else if(port == UART_ZIG)
				if(zig_rece_count > 8)   continue;
		#endif
			vTaskDelay(1);
		}
		if(port == UART_SUB1)
		{			
			length = sub1_rece_count;
			memcpy(subnet_response_buf,sub1_data_buffer,sub1_rece_count);
		}
		#if  defined(MINI)
		else if(port == UART_SUB2)
		{	 
			length = hsurRxCount;
			memcpy(subnet_response_buf,hsurRxBuffer,hsurRxCount);
		}
		else if(port == UART_ZIG)
		{	
			length = zig_rece_count;
			memcpy(subnet_response_buf,zig_data_buffer,zig_rece_count);
		}
		#endif
		if(length >= 9)
		{  	
			U8_T id;
			U32_T sn;
			Test[21]++;
			memcpy(tmp_sendbuf,header,6);
			memcpy(&tmp_sendbuf[6],subnet_response_buf,length - 2);	
			flagLED_ether_tx = 1;
			TCPIP_TcpSend(TcpSocket_ME, tmp_sendbuf, length + 4, TCPIP_SEND_NOT_FINAL); 
			if(subnet_response_buf[2] == 253) 
			{ Test[21] = 50; 	 Test[22] = length;  length = 9;}
			if(length == 9)
			{
				crc_check = crc16(subnet_response_buf, length - 2);

				if(crc_check == subnet_response_buf[length - 2] * 256 + subnet_response_buf[length - 1])
				{
					id = subnet_response_buf[2];
					sn = (U32_T)(subnet_response_buf[3] << 24)  + 	(U32_T)(subnet_response_buf[3] << 16) + 
							(U16_T)(subnet_response_buf[3] << 8) + 	subnet_response_buf[3];
					
					check_id_in_database(id,sn,port); 
					recount_sub_addr();
				//	ChangeFlash = 1;
				}
			}
			ret = 1;
		}
	}
	else if(length = wait_subnet_response(delay_time,port))
	{		
		if(port == UART_SUB1)
		{	
			memcpy(subnet_response_buf,sub1_data_buffer,length);
		}
#if  defined(MINI)

		else if(port == UART_SUB2)
		{
			memcpy(subnet_response_buf,hsurRxBuffer,length);
		}
		else if(port == UART_ZIG)
		{
			memcpy(subnet_response_buf,zig_data_buffer,length);
		}
#endif
		crc_check = crc16(subnet_response_buf, length - 2);

		if(crc_check == subnet_response_buf[length - 2] * 256 + subnet_response_buf[length - 1])
		{
			memcpy(tmp_sendbuf,header,6);
			memcpy(&tmp_sendbuf[6],subnet_response_buf,length - 2);	
			flagLED_ether_tx = 1;						
			TCPIP_TcpSend(TcpSocket_ME, tmp_sendbuf, size + 4, TCPIP_SEND_NOT_FINAL); 
			ret = 1;
		}			

	}

	set_subnet_parameters(SEND, 0,port);
//	cSemaphoreGive(tempsem);
//	return ret;
}

void Response_MAIN_To_SUB(U8_T *buf, U16_T len,U8_T port)
{
	U16_T length;
	U16_T crc_check;
	U8_T size;
	U16_T delay_time;
	U8_T tmp_sendbuf[150];
	xSemaphoreHandle  tempsem;
//	xQueueHandle tempque;

	if(port == UART_SUB1)	
	{			
		tempsem = sem_subnet_tx_uart0;
	}
#if  defined(MINI)
	else if(port == UART_SUB2)	
	{	
		tempsem = sem_subnet_tx_uart2;
	}
	else if(port == UART_ZIG)
	{
		tempsem =  sem_subnet_tx_uart1;
	}
#endif

	if(buf[1] == 0x03) // read
	{		 
		 size = tmp_sendbuf[5] * 2 + 5;
		 delay_time = size * 2;
	}
	else if(buf[1] == 0x06 || buf[1] == 0x10)
	{	 
		size = 8;
		if(buf[1] == 0x06 && buf[5] == 0x3f)   // erase flash 
			delay_time = 1000;
		else
			delay_time = 100;
	}
	else if(buf[1] == 0x19)
	{  
		size = 12;
#if defined(MINI)

		if(port == UART_ZIG )		delay_time = 500;
		else
#endif
			delay_time = 50;
	}
	else 
		return;

//	if(cSemaphoreTake(tempsem, 10) == pdFALSE)
//		return ;

	uart_init_send_com(port);

	memcpy(tmp_sendbuf,buf,len);
	crc_check = crc16(tmp_sendbuf, len);
	tmp_sendbuf[len] = HIGH_BYTE(crc_check);
	tmp_sendbuf[len + 1] = LOW_BYTE(crc_check);
	uart_send_string(tmp_sendbuf, len + 2 ,port);
	set_subnet_parameters(RECEIVE,size,port);
		if(buf[1] == 0x19)	  // scan cmd
	{ 	
		U16_T i;
		for(i = 0; i < delay_time; i++)
		{
			if(port == UART_SUB1)
				if(sub1_rece_count > 8)   continue;
		#if  defined(MINI)
			else if(port == UART_SUB2)
				if(hsurRxCount > 8)	 continue;
			else if(port == UART_ZIG)
				if(zig_rece_count > 8)   continue;
		#endif
			vTaskDelay(1);
		}
		if(port == UART_SUB1)
		{			
			length = sub1_rece_count;
			memcpy(subnet_response_buf,sub1_data_buffer,sub1_rece_count);
		}
		#if  defined(MINI)
		else if(port == UART_SUB2)
		{	  	
			length = hsurRxCount;
			memcpy(subnet_response_buf,hsurRxBuffer,hsurRxCount);
		}
		else if(port == UART_ZIG)
		{
			length = zig_rece_count;
			memcpy(subnet_response_buf,zig_data_buffer,zig_rece_count);
		}
		#endif
		if(length >= 9)
		{ 
			main_init_send_com();
			for(i = 0;i < length;i++)
				uart_send_byte(subnet_response_buf[i],UART_MAIN);
		}
	}
	else if(length = wait_subnet_response(delay_time,port))
	{		
		U16_T i;
		if(port == UART_SUB1)
		{ 
			memcpy(subnet_response_buf,sub1_data_buffer,length);
		}
#if  defined(MINI)

		else if(port == UART_SUB2)
		{ 
			memcpy(subnet_response_buf,hsurRxBuffer,length);
		}
		else if(port == UART_ZIG)
		{ 
			memcpy(subnet_response_buf,zig_data_buffer,length);
		}
#endif
		crc_check = crc16(subnet_response_buf, length - 2);

		if(crc_check == subnet_response_buf[length - 2] * 256 + subnet_response_buf[length - 1])
		{	
			main_init_send_com();
			for(i = 0;i < length;i++)
				uart_send_byte(subnet_response_buf[i],UART_MAIN);
		}			

	}
	set_subnet_parameters(SEND, 0,port);
//	cSemaphoreGive(tempsem);
}

void vStartScanTask(unsigned char uxPriority)
{
	memset(node_write,0,sizeof(STR_NODE_OPERATE) * STACK_LEN);
	sTaskCreate(ScanTask, (const signed portCHAR * const)"ScanTask", ScanSTACK_SIZE, NULL, uxPriority, (xTaskHandle *)&Handle_Scan);
	sTaskCreate(ParameterOperationTask, (const signed portCHAR * const)"ParameterOperationTask", ParameterOperationSTACK_SIZE, NULL, uxPriority, (xTaskHandle *)&Handle_ParameterOperation);
}