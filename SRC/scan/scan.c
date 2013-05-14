#include "main.h"

/* record every tstat on and off time, to check whether the tstat is off and have same ID */
/*U16_T time_tstat_off[254];     // record off line time for every ID 
U16_T time_tstat_on[254];		
U8_T error_rate[254];
*/

SCAN_DB scan_db[8];
SCAN_DB temp_scan_db;
U8_T tempaddr[8];
U8_T temp_sub_no;

extern xQueueHandle	xSubRevQueue;



void calculate_ID_table(void)
{
	U8_T i;
	

	memset(binsearch_Table,'\0',sizeof(BinSearch) * 129);
/*	memset(time_tstat_off,0,2 * 254);
	memset(time_tstat_on,0,2 * 254);
	memset(error_rate,0,254);
*/
	binsearch_Table[0].max = 254;
	binsearch_Table[0].min = 1; 
	binsearch_Table[0].valid = 1; 


	for(i = 0;i<64;)
	{
		binsearch_Table[2 * i + 1].max = binsearch_Table[i].max;
		binsearch_Table[2 * i + 1].min = (binsearch_Table[i].max + binsearch_Table[i].min) / 2;
		binsearch_Table[2 * i + 1].valid = 0;


		binsearch_Table[2 * (i + 1)].max = (binsearch_Table[i].max + binsearch_Table[i].min) / 2 - 1;
		binsearch_Table[2 * (i + 1)].min =  binsearch_Table[i].min;
		binsearch_Table[2 * (i + 1)].valid = 0;

		if((binsearch_Table[2 * i + 1].max > binsearch_Table[2 * i + 1].min) && 
			(binsearch_Table[2 * (i + 1)].max  > binsearch_Table[2 * (i + 1)].min))
 			i++;	
			
	}
}


void Send_Test_Tstat(U8_T addr,U8_T start,U8_T len)
{ 
	U8_T buf[8];
	U16_T crc_val;
	U8_T i;

//	comm_tstat = CM5_Check_Sub_OnLine;

	sub_init_send_com();
	sub_init_crc16();
	
	buf[0] = addr;
	buf[1] = 0x03;
	buf[2] = 0x00;
	buf[3] = start;
	buf[4] = 0x00;
	buf[5] = len;

	crc_val = crc16(buf,6);
		
	buf[6] = crc_val >> 8;
	buf[7] = (U8_T)crc_val;

	for(i = 0;i < 8;i++)
	{
		sub_send_byte(buf[i],CRC_YES);
	}

	sub_rece_size = 5 + len * 2;;
	sub_serial_restart();
	//for(i = 0;i < 12;i++)
	//	sub_data_buffer[i] = 0;		
}


U8_T Get_Idle_ID(U8_T *subaddr,U8_T subno)
{
	U8_T i; 
	U8_T temp;

	if(subno == 0)	
	{
		do
		{
			temp = rand() % 255;
		}
		while(temp == 255 || temp ==  Modbus_address) ;
	}
	else
	{
		for(i = 0;i < subno;i++)
		{
		   	do
			{
				temp = rand() % 255;
			}
			while(temp == 255 || temp ==  Modbus_address || temp == subaddr[i]);
		}
	}
	return temp;
}

void Send_Scan_Cmd(U8_T max, U8_T min)
{	

	U8_T buf[8];
	U16_T crc_val;
	U8_T i;

	sub_init_send_com();
	sub_init_crc16();
	
	buf[0] = 0xff;
	buf[1] = 0x19;
	buf[2] = max;
	buf[3] = min;

	crc_val = crc16(buf,4);
		
	buf[4] = crc_val >> 8;
	buf[5] = (U8_T)crc_val;

	for(i = 0;i < 6;i++)
	{
		sub_send_byte(buf[i],CRC_YES);
	}

	sub_rece_size = 12;
//	sub_rece_size = 20;
//	scan_response_state = NONE_ID;
	sub_serial_restart();
	for(i = 0;i < 12;i++)
		sub_data_buffer[i] = 0;	
}



/* check tstat id, make sure whether add new tstat list or remove the old one form tstat list */
void update_tstat_list(U8_T tstat_id)
{

	U8_T retry = 0; // no reply, retry 3 time
	U8_T retry1 = 0;
	U8_T emptypos,loop1,loop2;
	bit flag_retry = 0;  // whether retry
	U8_T i,j; 
	U8_T test_register_len;


	U16_T crc_val = 0;
	bit old_tstat = 0;
	U16_T loop0;
	U8_T tempID;
	memset(tempaddr,'0',8);
	memcpy(tempaddr,sub_addr,8);
	temp_sub_no = sub_no;
	if(tstat_id <= 0 || tstat_id >= 255)  // make sure correct id	
		return;
	
	if(tstat_id == Modbus_address)
	{// if current tstat ID is same as CM5's ID, Must change it
		tempID = Get_Idle_ID(tempaddr,temp_sub_no);	 // get new id	
		if(tstat_id != tempID)
		{	
			assignment_id_with_sn(tstat_id,tempID,temp_scan_db.sn);	
			tstat_id = tempID;	
		}  		
	}

	/* get new id, update tstat id list */

	if(temp_sub_no == 0)
	{/* if check tstat on line first time or no tstat in tstat list*/
		/*Com_Tstat(READ_ADDRESS,tstat_id);
		
		if(wait_SubSerial(500))   // get reply
		{
			crc_val = crc16(sub_data_buffer,5);

			if(crc_val == sub_data_buffer[5] * 256 + sub_data_buffer[6])
			{*/
				tempaddr[0] = tstat_id;	 /* add new one */
				scan_db[0].id = tstat_id;
				scan_db[0].sn = temp_scan_db.sn;
				temp_sub_no = 1;	
				
		/*	}
			
		} */
	} 
	else
	{	
		old_tstat = 0;
		/* scan old tstat list , add or remove tstat id */
		//  old_tstat 0: continue compare  1: end compare
		for(loop0 = 0;loop0 < temp_sub_no && !old_tstat;loop0++)
		{
		 /* search whether the current tstat is in old tstat list */		
			if(tstat_id == tempaddr[loop0])	
			{  /* if the tstat id is in tstat list, the current id is old one,end compare	*/
			//	old_tstat = 1;
				if(scan_db[loop0].sn == temp_scan_db.sn)  // compare serail number, whether this one is new one
					old_tstat = 1;
				else 
				{	
					tempID = Get_Idle_ID(tempaddr,temp_sub_no);				
					Test[44]++;
					Test[43] = temp_scan_db.sn;
					old_tstat = 1;
					if(tstat_id != tempID)
					{	
						old_tstat = 0;
						assignment_id_with_sn(tstat_id,tempID,temp_scan_db.sn);	
						tstat_id = tempID;	
					}  				
				}
			}
		}
		/* if not in list, it is a new one, add it */
		if(!old_tstat && tstat_id != 0)	
		{	
			tempaddr[temp_sub_no] = tstat_id; /* add new one */
			scan_db[temp_sub_no].id = tstat_id;
			scan_db[temp_sub_no].sn = temp_scan_db.sn;
			/* new tstat , increase temp_sub_no */
		   	Test[45 + temp_sub_no] =  temp_scan_db.sn;
			temp_sub_no++;			
		}

	#if 0
// the following code is check_on_line roution

		retry = 0;
		
		for(loop1 = 0;loop1 < temp_sub_no && retry < 10;loop1++)
		{		
			flag_retry = 0;
													  
			test_register_len = 1;

			Send_Test_Tstat(tempaddr[loop1],101,test_register_len);
		
			
			if(wait_SubSerial(100))   // get reply
			{		
							
				crc_val = crc16(sub_data_buffer,test_register_len * 2 + 5 - 2);
			
				if(crc_val == sub_data_buffer[test_register_len * 2 + 5 - 2] * 256 + sub_data_buffer[test_register_len * 2 + 5 - 1])
				{
					flag_retry = 0;
					retry = 0;
				}
				else
				{
					loop1--;
					flag_retry = 1;
					retry++;
				}
			
			}			
			else  
			{
				flag_retry = 1;
				loop1--;
				retry++; 				
			}
				
			if(retry == 10)
			{	
				loop1++;
				retry = 0;
				emptypos = 0;
				for(loop2 = 0;loop2 < temp_sub_no && emptypos == 0;loop2++)
				{
					if(tempaddr[loop1] == tempaddr[loop2])  /* find the old position in address table */
					{
						emptypos = loop2; 
					}
				}
				for(loop2 = emptypos;loop2 < temp_sub_no;loop2++)
				{ /* fill the empty postion */
					tempaddr[loop2] = tempaddr[loop2 + 1];
				}
				tempaddr[loop2] = 0;
				temp_sub_no--;
				
			}
		}
	#endif	
	}


/* check whether there are duplicate id */
	
	sub_no = temp_sub_no;
	
	memcpy(sub_addr,tempaddr,8);
}



void read_sn_for_assignment_id(U8_T address)
{ 	
	sub_init_send_com();
	sub_init_crc16();
	sub_send_byte(address,CRC_NO);
	sub_send_byte(READ_VARIABLES,CRC_NO);
	sub_send_byte(0,CRC_NO);
	sub_send_byte(MODBUS_ADDRESS_PLUG_N_PLAY,CRC_NO);
	sub_send_byte(0,CRC_NO);
	sub_send_byte(1,CRC_NO);  // must be 1
	sub_send_byte(SubCRChi,CRC_YES);
	sub_send_byte(SubCRClo,CRC_YES);
//	sub_rece_size = 11;
	sub_serial_restart();
}

void assignment_id_with_sn(U8_T address, U8_T new_address,unsigned long current_sn)
{
//	comm_tstat = CM5_Assign_ID;
	sub_init_send_com();
	sub_init_crc16();
	sub_send_byte(address,CRC_NO);
	sub_send_byte(WRITE_VARIABLES,CRC_NO);
	sub_send_byte(0,CRC_NO);
	sub_send_byte(MODBUS_ADDRESS_PLUG_N_PLAY,CRC_NO);
	sub_send_byte(0x55,CRC_NO);
	sub_send_byte(new_address,CRC_NO);
	sub_send_byte((U8_T)(current_sn >> 24),CRC_NO);
	sub_send_byte((U8_T)(current_sn >> 16),CRC_NO);
	sub_send_byte((U8_T)(current_sn >> 8),CRC_NO);
	sub_send_byte((U8_T)current_sn ,CRC_NO);

	sub_send_byte(SubCRChi,CRC_YES);
	sub_send_byte(SubCRClo,CRC_YES);
//	sub_rece_size = 8;
	sub_serial_restart();
	
}


void check_on_line(void)
{
	U8_T ret = 0;
	U8_T retry = 0; // no reply, retry 3 time
	U8_T retry1 = 0;
	U8_T emptypos,loop1,loop2;
	bit flag_retry = 0;  // whether retry
	U16_T crc_val = 0;

	U8_T i,j;

	U8_T test_register_len;

	memset(tempaddr,'0',8);
	memcpy(tempaddr,sub_addr,8);
	temp_sub_no = sub_no;
	#if 1
	retry = 0;
		
	for(loop1 = 0;loop1 < temp_sub_no && retry < 10;loop1++)
	{		
		flag_retry = 0;
												  
		test_register_len = 1;
		Test[28] = tempaddr[loop1];
		Test[27]++;	
	//	Test[40] = 50;
		Send_Test_Tstat(tempaddr[loop1],101,test_register_len);		
	//	Com_Tstat(READ_ADDRESS,tempaddr[loop1]); 
		ret = wait_SubSerial(10);
	//	Test[40] = 0;	
		if(cQueueReceive( xSubRevQueue, ( void * )&sub_data_buffer, 0))
		{
			if(ret == 1)   // get reply
			{		
					
				crc_val = crc16(sub_data_buffer,test_register_len * 2 + 5 - 2);
			
				if(crc_val == sub_data_buffer[test_register_len * 2 + 5 - 2] * 256 + sub_data_buffer[test_register_len * 2 + 5 - 1])
				{	 
					flag_retry = 0;
					retry = 0;
				}
				else
				{
					loop1--;
					flag_retry = 1;
					retry++;
				}			
				
			}			
			else if(ret == 0) 
			{
				flag_retry = 1;
				loop1--;
				retry++;
				
			} 	 
		}	
		
		if(retry == 10)
		{	
			loop1++;
			retry = 0;

			emptypos = 0;
			for(loop2 = 0;loop2 < temp_sub_no && emptypos == 0;loop2++)
			{
				if(tempaddr[loop1] == tempaddr[loop2])  /* find the old position in address table */
				{
					emptypos = loop2; 
				}
			}
			for(loop2 = emptypos;loop2 < temp_sub_no;loop2++)
			{ /* fill the empty postion */
				tempaddr[loop2] = tempaddr[loop2 + 1];
			}
			tempaddr[loop2] = 0;
			temp_sub_no--;
			
		}
					
	}
	#endif
	sub_no = temp_sub_no;
	
	memcpy(sub_addr,tempaddr,8);
}




char binarySearchforComDevice(unsigned char maxaddr,unsigned char minaddr) //reentrant
{
	U16_T crc_val;
	U8_T ret;
	if(maxaddr > minaddr)
	{
		Send_Scan_Cmd(maxaddr,minaddr);
		ret = wait_SubSerial(10);
		if(cQueueReceive( xSubRevQueue, &sub_data_buffer, 0))
		{
			Test[30] = sub_data_buffer[0];
			Test[31] = sub_data_buffer[1];
			Test[32] = sub_data_buffer[2];
			Test[33] = sub_data_buffer[3];
			Test[34] = sub_data_buffer[4];
			Test[35] = sub_data_buffer[5];
			Test[36] = sub_data_buffer[6];
			Test[37] = sub_data_buffer[7];
			Test[38] = sub_data_buffer[8];
			Test[39] = sub_data_buffer[9];
			Test[40] = sub_data_buffer[10];
			Test[41] = sub_data_buffer[11];
			Test[42] = sub_data_buffer[12];
		//	Test[43] = sub_data_buffer[13];
			if(sub_data_buffer[0] == 0xff && sub_data_buffer[1] == 0x19 )
			{
				if(ret == 1)   // get reply 
				{
					crc_val = crc16(sub_data_buffer,7);
		
					if(crc_val == sub_data_buffer[7] * 256 + sub_data_buffer[8])
					{	
						Test[20]++;
						//if(sub_data_buffer[9] != 0 || sub_data_buffer[10] != 0)
						{	//	ttt[0] = 22;
							return -2;   // continue check, many tstat is connecting
						}	
					}
					else
					{	Test[21]++;			
						return -3;  
					}
				}
				else if(ret == 0)
				{
					crc_val = crc16(sub_data_buffer,7);
		
					if(crc_val == sub_data_buffer[7] * 256 + sub_data_buffer[8])
					{	/* if crc is correct , unique ID */
						/* store id and serial number to Modbus_Data.sub_addr*/	
		    		//  stroe serial number for every tstat 
						Test[22]++;
						temp_scan_db.sn = ((unsigned long)sub_data_buffer[3] << 24) | ((unsigned long)sub_data_buffer[4] << 16) | 
											((unsigned long)sub_data_buffer[5] << 8) | sub_data_buffer[6];								
						temp_scan_db.id = sub_data_buffer[2];
						//Modbus_Data.sub_addr[test_index++] = sub_data_buffer[2];
						return 1;    /* if crc is correct , unique ID in this range */
					}
					else	/* if crc is error,  many tstat have same ID  */
					{
						if(sub_data_buffer[0] == 0xff && sub_data_buffer[1] == 0x19 )
						{
							Test[23]++;
							return -3;
						}
						Test[24]++;
					}
				}
			}
		}
	}
	else 
		return -4;	// end whole search	

	return -1;
}

void Scan_Sub_ID(void)
{
	char result;
	U8_T i;

	binsearch_Table[0].max = 254;
	binsearch_Table[0].min = 1; 
	binsearch_Table[0].valid = 1; 


	for(i = 0;i < 129;i++)
	{
		if(binsearch_Table[i].valid == 0)
		{// if the current range is invalid, the following sub range is invalid, too	
			/* every range have two sub range */
			if(2 * (i + 1) < 129)
			{
				binsearch_Table[2 * i + 1].valid = 0;
				binsearch_Table[2 * (i + 1)].valid = 0;
			}
		} 
		else
		{
			result = binarySearchforComDevice(binsearch_Table[i].max,binsearch_Table[i].min);
			if(result == 1)		// unique ID in this range
			{
			//	Test[20]++;
				binsearch_Table[2 * i + 1].valid = 0;
				binsearch_Table[2 * (i + 1)].valid = 0;
				update_tstat_list(temp_scan_db.id);
			}
			else if(result == -2)		// continue check, many tstat is connecting
			{
			//	Test[21]++;
				binsearch_Table[2 * i + 1].valid = 1;
				binsearch_Table[2 * (i + 1)].valid = 1;	
			}
			else if(result == -3)	// many tstat have same ID 
			{
			//	Test[22]++;
			//	deal_with_sameID(temp_scan_db);
			}
			else if(result == -4)		 // end search
			{
			//	Test[23]++;
				binsearch_Table[2 * i + 1].valid = 0;
				binsearch_Table[2 * (i + 1)].valid = 0;
			}
		
		}
	}
}


