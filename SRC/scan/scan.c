#include "main.h"

/* record every tstat on and off time, to check whether the tstat is off and have same ID */
/*U16_T time_tstat_off[254];     // record off line time for every ID 
U16_T time_tstat_on[254];		
U8_T error_rate[254];
*/


#if 1

SCAN_DB far scan_db[8];
SCAN_DB far temp_scan_db;
U8_T far tempaddr[8];
U8_T far temp_sub_no;

extern xQueueHandle	xSubRevQueue;

uint8 const code scan_id_table[1+2+4+8+16+32+64+128+256][2] = 
{
	// 1
	{1, 254},
	// 2
	{1, 127},	{128, 254},
	// 4
	{1, 63},	{64, 127},	{128, 191},	{192, 254},
	// 8
	{1, 31},	{32, 63},	{64, 95},	{96, 127},	{128, 159},	{160, 191},	{192, 223},	{224, 254},
	// 16
	{1, 15},	{16, 31},	{32, 47},	{48, 63},	{64, 79},	{80, 95},	{96, 111},	{112, 127},
	{128, 143},	{144, 159},	{160, 175},	{176, 191},	{192, 207},	{208, 223},	{224, 239},	{240, 254},
	// 32
	{1, 7},		{8, 15},	{16, 23},	{24, 31},	{32, 39},	{40, 47},	{48, 55},	{56, 63},	
	{64, 71},	{72, 79},	{80, 87},	{88, 95},	{96, 103},	{104, 111},	{112, 119},	{120, 127},
	{128, 135},	{136, 143},	{144, 151},	{152, 159},	{160, 167},	{168, 175},	{176, 183},	{184, 191},	
	{192, 199},	{200, 207},	{208, 215}, {216, 223},	{224, 231},	{232, 239},	{240, 247},	{248, 254},
	// 64
	{1, 3},		{4, 7},		{8, 11},	{12, 15},	{16, 19},	{20, 23},	{24, 27},	{28, 31},	
	{32, 35},	{36, 39},	{40, 43},	{44, 47},	{48, 51},	{52, 55},	{56, 59},	{60, 63},	
	{64, 67},	{68, 71},	{72, 75},	{76, 79},	{80, 83},	{84, 87},	{88, 91},	{92, 95},	
	{96, 99},	{100, 103},	{104, 107},	{108, 111},	{112, 115},	{116, 119},	{120, 123},	{124, 127},
	{128, 131},	{132, 135},	{136, 139},	{140, 143},	{144, 147},	{148, 151},	{152, 155},	{156, 159},	
	{160, 163},	{164, 167},	{168, 171},	{172, 175},	{176, 179},	{180, 183},	{184, 187},	{188, 191},	
	{192, 195},	{196, 199},	{200, 203},	{204, 207},	{208, 211},	{212, 215}, {216, 219},	{220, 223},	
	{224, 227},	{228, 231},	{232, 235}, {236, 239},	{240, 243},	{244, 247},	{248, 251},	{252, 254},
	// 128
	{1, 1},		{2, 3},		{4, 5},		{6, 7},		{8, 9},		{10, 11},	{12, 13},	{14, 15},	
	{16, 17},	{18, 19},	{20, 21},	{22, 23},	{24, 25},	{26, 27},	{28, 29},	{30, 31},	
	{32, 33},	{34, 35},	{36, 37},	{38, 39},	{40, 41},	{42, 43},	{44, 45},	{46, 47},	
	{48, 49},	{50, 51},	{52, 53},	{54, 55},	{56, 57},	{58, 59},	{60, 61},	{62, 63},	
	{64, 65},	{66, 67},	{68, 69},	{70, 71},	{72, 73},	{74, 75},	{76, 77},	{78, 79},	
	{80, 81},	{82, 83},	{84, 85},	{86, 87},	{88, 89},	{90, 91},	{92, 93},	{94, 95},	
	{96, 97},	{98, 99},	{100, 101},	{102, 103},	{104, 105},	{106, 107},	{108, 109},	{110, 111},	
	{112, 113},	{114, 115},	{116, 117},	{118, 119},	{120, 121},	{122, 123},	{124, 125},	{126, 127},
	{128, 129},	{130, 131},	{132, 133},	{134, 135},	{136, 137},	{138, 139},	{140, 141},	{142, 143},	
	{144, 145},	{146, 147},	{148, 149},	{150, 151},	{152, 153},	{154, 155},	{156, 157},	{158, 159},	
	{160, 161},	{162, 163},	{164, 165},	{166, 167},	{168, 169},	{170, 171},	{172, 173},	{174, 175},	
	{176, 177},	{178, 179},	{180, 181}, {182, 183},	{184, 185},	{186, 187},	{188, 189},	{190, 191},	
	{192, 193},	{194, 195},	{196, 197}, {198, 199},	{200, 201},	{202, 203},	{204, 205},	{206, 207},	
	{208, 209},	{210, 211},	{212, 213},	{214, 215}, {216, 217},	{218, 219},	{220, 221},	{222, 223},	
	{224, 225},	{226, 227},	{228, 229},	{230, 231},	{232, 233},	{234, 235}, {236, 237},	{238, 239},	
	{240, 241},	{242, 243},	{244, 245},	{246, 247},	{248, 249},	{250, 251},	{252, 253},	{254, 254},
	// 256
	{1, 1},		{1, 1},		{2, 2},		{3, 3},		{4, 4},		{5, 5},		{6, 6},		{7, 7},
	{8, 8},		{9, 9},		{10, 10},	{11, 11},	{12, 12},	{13, 13},	{14, 14},	{15, 15},
	{16, 16},	{17, 17},	{18, 18},	{19, 19},	{20, 20},	{21, 21},	{22, 22},	{23, 23},
	{24, 24},	{25, 25},	{26, 26},	{27, 27},	{28, 28},	{29, 29},	{30, 30},	{31, 31},
	{32, 32},	{33, 33},	{34, 34},	{35, 35},	{36, 36},	{37, 37},	{38, 38},	{39, 39},
	{40, 40},	{41, 41},	{42, 42},	{43, 43},	{44, 44},	{45, 45},	{46, 46},	{47, 47},
	{48, 48},	{49, 49},	{50, 50},	{51, 51},	{52, 52},	{53, 53},	{54, 54},	{55, 55},
	{56, 56},	{57, 57},	{58, 58},	{59, 59},	{60, 60},	{61, 61},	{62, 62},	{63, 63},
	{64, 64},	{65, 65},	{66, 66},	{67, 67},	{68, 68},	{69, 69},	{70, 70},	{71, 71},
	{72, 72},	{73, 73},	{74, 74},	{75, 75},	{76, 76},	{77, 77},	{78, 78},	{79, 79},
	{80, 80},	{81, 81},	{82, 82},	{83, 83},	{84, 84},	{85, 85},	{86, 86},	{87, 87},
	{88, 88},	{89, 89},	{90, 90},	{91, 91},	{92, 92},	{93, 93},	{94, 94},	{95, 95},
	{96, 96},	{97, 97},	{98, 98},	{99, 99},	{100, 100},	{101, 101},	{102, 102},	{103, 103},
	{104, 104},	{105, 105},	{106, 106},	{107, 107},	{108, 108},	{109, 109},	{110, 110},	{111, 111},
	{112, 112},	{113, 113},	{114, 114},	{115, 115},	{116, 116},	{117, 117},	{118, 118},	{119, 119},
	{120, 120},	{121, 121},	{122, 122},	{123, 123},	{124, 124},	{125, 125},	{126, 126},	{127, 127},
	{128, 128},	{129, 129},	{130, 130},	{131, 131},	{132, 132},	{133, 133},	{134, 134},	{135, 135},
	{136, 136},	{137, 137},	{138, 138},	{139, 139},	{140, 140},	{141, 141},	{142, 142},	{143, 143},
	{144, 144},	{145, 145},	{146, 146},	{147, 147},	{148, 148},	{149, 149},	{150, 150},	{151, 151},
	{152, 152},	{153, 153},	{154, 154},	{155, 155},	{156, 156},	{157, 157},	{158, 158},	{159, 159},
	{160, 160},	{161, 161},	{162, 162},	{163, 163},	{164, 164},	{165, 165},	{166, 166},	{167, 167},
	{168, 168},	{169, 169},	{170, 170},	{171, 171},	{172, 172},	{173, 173},	{174, 174},	{175, 175},
	{176, 176},	{177, 177},	{178, 178},	{179, 179},	{180, 180},	{181, 181},	{182, 182},	{183, 183},
	{184, 184},	{185, 185},	{186, 186},	{187, 187},	{188, 188},	{189, 189},	{190, 190},	{191, 191},
	{192, 192},	{193, 193},	{194, 194},	{195, 195},	{196, 196},	{197, 197},	{198, 198},	{199, 199},
	{200, 200},	{201, 201},	{202, 202},	{203, 203},	{204, 204},	{205, 205},	{206, 206},	{207, 207},
	{208, 208},	{209, 209},	{210, 210},	{211, 211},	{212, 212},	{213, 213},	{214, 214},	{215, 215},
	{216, 216},	{217, 217},	{218, 218},	{219, 219},	{220, 220},	{221, 221},	{222, 222},	{223, 223},
	{224, 224},	{225, 225},	{226, 226},	{227, 227},	{228, 228},	{229, 229},	{230, 230},	{231, 231},
	{232, 232},	{233, 233},	{234, 234},	{235, 235},	{236, 236},	{237, 237},	{238, 238},	{239, 239},
	{240, 240},	{241, 241},	{242, 242},	{243, 243},	{244, 244},	{245, 245},	{246, 246},	{247, 247},
	{248, 248},	{249, 249},	{250, 250},	{251, 251},	{252, 252},	{253, 253},	{254, 254},	{254, 254},	
};


void calculate_ID_table(void)
{
	U16_T i;
		binsearch_Table[0].valid = 1;

//	memset(binsearch_Table,1,sizeof(BinSearch) * 511);	 	

/*	memset(time_tstat_off,0,2 * 254);
	memset(time_tstat_on,0,2 * 254);
	memset(error_rate,0,254);
*/
/*	binsearch_Table[0].max = 254;
	binsearch_Table[0].min = 1; 


	for(i = 0;i < 255;)
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
			
	} */
}


void Send_Test_Tstat(U8_T addr,U8_T start,U8_T len)
{ 
	U8_T far buf[8];
	U16_T far crc_val;
	U8_T far i;

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
	U8_T far i; 
	U8_T far temp;

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

	U8_T far buf[8];
	U16_T far crc_val;
	U8_T far i;

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

	U8_T far retry = 0; // no reply, retry 3 time
	U8_T far retry1 = 0;
	U8_T far emptypos,loop1,loop2;
	bit flag_retry = 0;  // whether retry
	U8_T far  i,j; 
	U8_T far test_register_len;


	U16_T crc_val = 0;
	bit old_tstat = 0;
	U16_T loop0;
	U8_T tempID;
	U8_T ret = 0;


	memset(tempaddr,'0',8);
	memcpy(tempaddr,sub_addr,8);
	temp_sub_no = sub_no;
//	Test[17] = tstat_id;
	if(tstat_id <= 0 || tstat_id >= 255)  // make sure correct id	
		return;
	
	if(tstat_id == Modbus_address)
	{// if current tstat ID is same as CM5's ID, Must change it
		tempID = Get_Idle_ID(tempaddr,temp_sub_no);	 // get new id	
		Test[41]++;
		if(tstat_id != tempID)
		{	
			assignment_id_with_sn(tstat_id,tempID,temp_scan_db.sn);
			DELAY_Us(10);	
			tstat_id = tempID;	
			
		}  		
	}
	/* get new id, update tstat id list */

	if(temp_sub_no == 0)
	{/* if check tstat on line first time or no tstat in tstat list*/
		tempaddr[0] = tstat_id;	 /* add new one */
		scan_db[0].id = tstat_id;
		scan_db[0].sn = temp_scan_db.sn;
		temp_sub_no = 1;
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
				//	Test[42]++;			
					old_tstat = 1;
					if(tstat_id != tempID)
					{	
						old_tstat = 0;
						assignment_id_with_sn(tstat_id,tempID,temp_scan_db.sn);	
						assignment_id_with_sn(tstat_id,tempID,temp_scan_db.sn);	
						assignment_id_with_sn(tstat_id,tempID,temp_scan_db.sn);	
						DELAY_Us(10);
						tstat_id = tempID;	
					}  				
				}
			}
		}

		/* if not in list, it is a new one, add it */
		if(!old_tstat && tstat_id != 0)	
		{	
			if(switch_sub_bit & (0x01 << temp_sub_no))  // 
			{	
				tempaddr[sub_no] = tstat_id; /* add new one */
				scan_db[sub_no].id = tstat_id;
				scan_db[sub_no].sn = temp_scan_db.sn; 
				temp_sub_no++;
			}
			else
			{
			tempaddr[temp_sub_no] = tstat_id; /* add new one */
			scan_db[temp_sub_no].id = tstat_id;
			scan_db[temp_sub_no].sn = temp_scan_db.sn;
			/* new tstat , increase temp_sub_no */
			temp_sub_no++;
			}	
		} 
	}
	
	
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
	U8_T far ret = 0;
	U8_T far retry = 0; // no reply, retry 3 time
	U8_T far retry1 = 0;
	U8_T far emptypos,loop1,loop2;
	bit flag_retry = 0;  // whether retry
	U16_T far crc_val = 0;

	U8_T i,j;  
	U8_T test_register_len;

	memset(tempaddr,'0',8);
	memcpy(tempaddr,sub_addr,8);
	temp_sub_no = sub_no;
	#if 1
	retry = 0;
	Test[27] = temp_sub_no;		
	for(loop1 = 0;loop1 < temp_sub_no /*&& retry < 10*/;loop1++)
	{		
		flag_retry = 0;
												  
		test_register_len = 1;
		Test[28] = tempaddr[loop1];
		
	/*	Send_Test_Tstat(tempaddr[loop1],101,test_register_len);		
	//	Com_Tstat(READ_ADDRESS,tempaddr[loop1]); 
		ret = wait_SubSerial(10);
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
		}*/	
		
	/*	if(retry == 10)
		{	
			loop1++;
			retry = 0;

			emptypos = 0;
			for(loop2 = 0;loop2 < temp_sub_no && emptypos == 0;loop2++)
			{
				if(tempaddr[loop1] == tempaddr[loop2]) // find the old position in address table
				{
					emptypos = loop2; 
				}
			}
			for(loop2 = emptypos;loop2 < temp_sub_no;loop2++)
			{ // fill the empty postion
				tempaddr[loop2] = tempaddr[loop2 + 1];
			}
			tempaddr[loop2] = 0;
			temp_sub_no--;
			
		} */
					
	}
	#endif
	sub_no = temp_sub_no;
	if(protocal <= TCP_IP)
	{	
		for(i = 0;i < 8;i++)
		{			
			if(DI_Type[i] == DI_SWITCH)
			{
				if(tempaddr[i] != 0)   // position is not free
				{
					tempaddr[sub_no++] = tempaddr[i];
				}
				if(switch_tstat_val & (0x01 << i))
					tempaddr[i] = 1;
				else
					tempaddr[i] = 0;						
			}	
		}
	}
	else 
	{		
	/*	tbd: t3000_bacnet should add the following property
		for( i=0; i< 8; i++ )
		{
			if(inputs[i].unused == DI_SWITCH)
			{
				if(tempaddr[i] != 0)   // position is not free
				{
					tempaddr[sub_no++] = tempaddr[i];
				}
				if(switch_tstat_val & (0x01 << i))
					tempaddr[i] = 1;
				else
					tempaddr[i] = 0;						
			}	
		} */
	}
	memcpy(sub_addr,tempaddr,8);
}




void Scan_Sub_ID(void)
{
	static char tt = 35;

/*	while(binsearch_Table[scan_index].valid == 0 && scan_index < 511)
	{
		if(2 * (scan_index + 1) < 511)
		{
			binsearch_Table[2 * scan_index + 1].valid = 0;
			binsearch_Table[2 * (scan_index + 1)].valid = 0;
		}
		scan_index++;

	} */


	if(scan_index < 511)
	{
		if(binsearch_Table[scan_index].valid == 0)
		{// if the current range is invalid, the following sub range is invalid, too	
			//every range have two sub range 
			if(2 * (scan_index + 1) < 511)
			{
				binsearch_Table[2 * scan_index + 1].valid = 0;
				binsearch_Table[2 * (scan_index + 1)].valid = 0;
			}
		} 
		else 
		{
			Send_Scan_Cmd(scan_id_table[scan_index][1],scan_id_table[scan_index][0]/*binsearch_Table[scan_index].max,binsearch_Table[scan_index].min*/);
		 //   Test[tt++] = scan_index;
			scan_index++;
		 //   Test[15]++;
		} 
		while(binsearch_Table[scan_index].valid == 0)
		{
			scan_index++;
		}

	//	Test[17] = scan_index + 1;

	}
	else 
	{
		scan_index = 0;
	//	tt = 35;
	//	memset(&Test[tt],0,15);
	//	binsearch_Table[0].max = 254;
	//	binsearch_Table[0].min = 1; 
		binsearch_Table[0].valid = 1; 
	}
}



#endif