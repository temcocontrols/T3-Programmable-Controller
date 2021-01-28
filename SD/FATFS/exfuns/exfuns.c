#include "product.h"



#include <string.h>
//#include "exfuns.h"
#include "fattester.h"	
#include<stdlib.h>
//#include "usart.h"
#include "user_data.h"
#include "main.h"

#if (ASIX_MINI || ASIX_CM5)
extern xSemaphoreHandle sem_SPI;
#endif


U8_T far SD_exist;  // 1- inexist 2- exist 3 - abnormal SD
U8_T far pre_open_file_index;

U8_T count_sd_seek_error;

typedef struct 
{
	U32_T time;
	U32_T seg;
}TIME_SD;
TIME_SD far seg_in_time_txt[10];
U8_T	far	index_in_time_txt;

#if STORE_TO_SD

void SD_HardWareInit(void);


#define SD_NO_ERR 0

 //??????
//const char *FILE_TYPE_TBL[6][13] =
//{
//	{"BIN"},			//BIN??
//	{"LRC"},			//LRC??
//	{"NES"},			//NES??
//	{"TXT","C","H"},	//????
//	{"MP1","MP2","MP3","MP4","M4A","3GP","3G2","OGG","ACC","WMA","WAV","MID","FLAC"},//????
//	{"BMP","JPG","JPEG","GIF"},//????
//};



///////////////////////////////?????,??malloc???////////////////////////////////////////////
//FATFS far *fs;  		//???????.	 
//FIL far *file;	  		//??1
FIL far *ftemp;	  		//??2.
UINT br,bw;			//????
FILINFO far fileinfo;	//????
DIR far dir;  			//??


FATFS far fs;
FIL far file[48]; 

#define READ_SD_BUFFER_SIZE 512

U8_T flag_f_mount;
//u8 far *fatbuf;			//SD??????

U8_T far rw_sd_buf[READ_SD_BUFFER_SIZE];

//SD_Error SD_Init(void);
#if (ASIX_MINI || ASIX_CM5)
extern u8 SD_Type;
#endif

void check_SD_PnP(void)
{
	if((Modbus.mini_type == MINI_BIG) ||(Modbus.mini_type == MINI_BIG_ARM)
			|| (Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM) 
			|| (Modbus.mini_type == MINI_NEW_TINY) || (Modbus.mini_type == MINI_TINY_ARM)	|| (Modbus.mini_type == MINI_TINY_11I)
			|| (Modbus.mini_type == MINI_TINY)
			|| (Modbus.mini_type == MINI_NANO) 
			)
		{
			if(SD_exist == 1) // check whehter SD exist
			{
#if (SD_BUS == SPI_BUS_TYPE)				
#if ARM_MINI
				if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM))
				{
					vTaskSuspend(xHandler_SPI);
					SPI1_Init(1);
				}
#endif
#endif
				check_SD_exist();
#if (SD_BUS == SPI_BUS_TYPE)
#if ARM_MINI
				if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM))
				{
					SPI1_Init(0);
					vTaskResume(xHandler_SPI);
				}
#endif
#endif
			}
		}
}


void check_SD_exist(void)
{
	U8_T ret;
	U16_T loop = 20;
	SD_exist = 1;

#if (ASIX_MINI || ASIX_CM5)	
	do
	{
		ret = SD_Initialize();
	} while((ret != SD_NO_ERR) && (--loop));
#else
	ret = SD_Initialize();
#endif	
	
	if(ret == 0 || ret == 42)  // successfully
	{
		if(SD_Type > 0)
			SD_exist = 2;		
	flag_f_mount = f_mount(0, &fs);
	count_sd_seek_error = 0;
	pre_open_file_index = 255;
	
#if ARM_UART_DEBUG
	uart1_init(115200);
	DEBUG_EN = 1;
	printf(" sd type %u mount %u\r\n",SD_Type,flag_f_mount);
#endif
	Test[41] = SD_exist;
	index_in_time_txt = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////////
//?exfuns????
//???:0,??
//1,??
//u8 exfuns_init(void)
//{
//	fs = (FATFS*)malloc(sizeof(FATFS));	//???0???????	
//	fs[1] = (FATFS*)malloc(sizeof(FATFS));	//???1???????
//	file = (FIL*)malloc(sizeof(FIL));			//?file????
//	ftemp = (FIL*)malloc(sizeof(FIL));		//?ftemp????
//	fatbuf = (u8*)malloc(512);				//?fatbuf????
//	if(fs && file && ftemp && fatbuf)
//		return 0;										//???????,???.
//	else
//		return 1;	
//}

// 0:/xxxyyz.bin
// xxx file_num
// yy - index
// z - A or D
U8_T open_file(U16_T file_num,U8_T index)
{
	U8_T ret;
	S8_T filename[20];

	if(index >= 48)  // picture
	{
//		memcpy(filename, "0:/       .bin", 20);
//		filename[3] = 'G';
//		filename[4] = 'R';
//		filename[5] = 'P';
//		filename[6] = '_';
//		if((index - 48) / 2 + 1 >= 10)
//		{
//			filename[7] = ((index - 48) / 2 + 1) / 10 + '0';
//			filename[8] = ((index - 48) / 2 + 1) % 10 + '0';
//		}
//		else
//		{
//			filename[7] = '0';
//			filename[8] = (index - 48) / 2 + 1 + '0';
//		}
		return 1;
	}
//	if(pre_open_file_index != cur_open_file_index)
	{		
		if(index < 24)
		{	
#if (SD_BUS == SPI_BUS_TYPE)			
			memcpy(filename, "0:/       .bin", 20);
#else
			memcpy(filename, "0:test.txt", 20);
#endif
			if(index / 2 + 1 >= 10)
			{
				filename[7] = (index / 2 + 1) / 10 + '0';
				filename[8] = (index / 2 + 1) % 10 + '0';
			}
			else
			{
				filename[7] = '0';
				filename[8] = index / 2 + 1 + '0';
			}
			
			if(index % 2 == 0)  // analog
			{					
					filename[9] = 'A';						
			}
			else  // digital
			{					
					filename[9] = 'D';
			}			

			filename[3] = file_num / 1000 + '0';
			filename[4] = file_num % 1000 / 100 + '0';
			filename[5] = file_num % 100 / 10 + '0';
			filename[6] = file_num % 10 + '0';
		}
		else if(index < 48)  // from 24 to 47, they are time_xx(A or D).txt
		{
			memcpy(filename, "0:/       .bin", 20);
			if((index - 24) / 2 + 1 >= 10)
			{
				filename[7] = ((index - 24) / 2 + 1) / 10 + '0';
				filename[8] = ((index - 24) / 2 + 1) % 10 + '0';
			}
			else
			{
				filename[7] = '0';
				filename[8] = (index - 24) / 2 + 1 + '0';
			}
			
			if(index % 2 == 0)  // analog
			{					
					filename[9] = 'A';						
			}
			else  // digital
			{					
					filename[9] = 'D';
			}			

			filename[3] = 'T';
			filename[4] = 'I';
			filename[5] = 'M';
			filename[6] = '_';
		}
		
		ret = f_open(&file[index],filename, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
		
		if(ret == FR_OK)
			pre_open_file_index = index;
		return ret;
	}
//	else
//		return FR_OK;
}




//extern xTaskHandle xHandler_SPI;
// file_no -> range is  0-255
// index   -> which monitor, range is 0 - 11
// ana_dig -> range is 0 - 1
U8_T Read_SD(U16_T file_no,U8_T index,U8_T ana_dig,U32_T star_pos)
{	
//	U16_T loop = 50;
	uint8 result;
	uint8 ret;
	uint8_t file_index;
	if(SD_exist != 2)
	{
		return 1;
	}		
	if(flag_f_mount != FR_OK) {  return 0;}

#if (SD_BUS == SPI_BUS_TYPE)
	
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
	if(cSemaphoreTake(sem_SPI, 200) == pdFALSE)
	{
		return 0;
	}
#if ARM_MINI || ARM_CM5
	if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM))
	{
		vTaskSuspend(xHandler_SPI);
		SPI1_Init(1);
	}
#endif
#endif
	
#if (ASIX_MINI || ASIX_CM5)		
	if(cSemaphoreTake(sem_SPI, 200) == pdFALSE)
			return 0;
	vTaskSuspend(xHandler_SPI);
	SD_HardWareInit();	
#endif

#endif
	
	if(ana_dig == 1)
	{
		file_index = index * 2;		
	}
	else
	{
		file_index = index * 2 + 1;		
	}
	//if(open_file_flag[file_index][file_no / 8] & (0x01 << (file_no % 8)) == 0)
	if(pre_open_file_index != file_index)
	{
		ret = open_file(file_no,file_index);
	}
	else
	{
		ret = FR_OK;
	}
	if(ret == FR_OK)
	{
		
		ret = f_lseek(&file[file_index], star_pos);

		if(ret != FR_OK)
		{
			// if seek fail, open file again.
			ret = open_file(file_no,file_index);
			ret = f_lseek(&file[file_index], star_pos);
			
			count_sd_seek_error++;
			
			if(ret != FR_OK)
				Test[16]++;
			else
				count_sd_seek_error = 0;
			
			result = 0;
			if(count_sd_seek_error > 5)
			{
				// generate SD card erro alarm
				count_sd_seek_error = 0;
				generate_common_alarm(ALARM_ABNORMAL_SD);
			}
		}
		
		if(ret == FR_OK)
		{
			count_sd_seek_error = 0;
			ret = f_read(&file[file_index], rw_sd_buf, MAX_MON_POINT * sizeof(Str_mon_element), &br);
			if(ret != FR_OK)
			{
				result = 0;
			}
			memcpy(&read_mon_point_buf, rw_sd_buf, MAX_MON_POINT * sizeof(Str_mon_element));
			if(star_pos == 255L * MAX_MON_POINT * sizeof(Str_mon_element))  // last packet
			{
				f_close(&file[file_index]);
			}
		}
		result = 1;
	}
	else
	{
		result = 0;
	}
#if (SD_BUS == SPI_BUS_TYPE)
	
#if (ASIX_MINI || ASIX_CM5)		
	SPI_Setup(SPI_SSO_ENB|SPI_MST_SEL|SPI_SS_AUTO|SPI_ENB, SPI_STCFIE, 10, SLAVE_SEL_1); // 25M
	vTaskResume(xHandler_SPI);
	cSemaphoreGive(sem_SPI);
#endif	
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
#if ARM_MINI || ARM_CM5
	if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM))
	{
		SPI1_Init(0);
		vTaskResume(xHandler_SPI);
	}
#endif
	cSemaphoreGive(sem_SPI);

#endif
#endif	

	
	return result;
}


u8 Get_start_end_packet_by_time(u8 file_index,u32 start_time,u32 end_time, u32 * start_seg, u32 *total_seg,u32 block_no)
{
	uint8 result;
	uint8 ret;
	s32 i,j;
	static uint32 k = 0; // temp pos, if the file is too big.
	u8 f_start = 0,f_end = 0;
	u32 end_seg;
	u32 old_time;
	u32 temp_start_seg;
	u32 temp_end_seg;	
	static u32 t1 = 0;
	static u32 t2 = 0;
	f_start = 0;
	f_end = 0;
	
	if(SD_exist != 2) 
	{
		*start_seg = 0;
		*total_seg = 1;		
		return 0;
	}
	
	if(file_index >= 24 )  // 0-24
		return 0;
	
	if(start_time > end_time) 
		return 0;
	
	if(SD_exist != 2)
	{
		return 1;
	}		
	

	if(flag_f_mount != FR_OK) return 0;
#if (SD_BUS == SPI_BUS_TYPE)
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
	if(cSemaphoreTake(sem_SPI, 200) == pdFALSE)
	{
			return 0;
	}
#if ARM_MINI || ARM_CM5
	if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM))
	{
		vTaskSuspend(xHandler_SPI);
		SPI1_Init(1);
	}
#endif
#endif
	
#if (ASIX_MINI || ASIX_CM5)		
	if(cSemaphoreTake(sem_SPI, 200) == pdFALSE)
			return 0;
	vTaskSuspend(xHandler_SPI);
	SD_HardWareInit();	
#endif
#endif
	
	ret = open_file(0,file_index + 24);
	*start_seg = 0;
	*total_seg = 0;
	memset(&rw_sd_buf,0,READ_SD_BUFFER_SIZE);
	if(ret == FR_OK)
	{
		u32 time;
// max packet is 16 * 65536
// buffer lenght is 512, time is 4 bytes,  so read 128 packets per time	
// max count = 16*65536 / 128 = 8192		
		
		i = block_no / 128;
		j = block_no % 128;
//#if ARM_UART_DEBUG
//		uart1_init(115200);
//		DEBUG_EN = 1;
//		printf(" get seg start = %u, end = %u, \r\n",start_time,end_time);
//#endif
		

		if(i >= 8192) 
		{		
#if (SD_BUS == SPI_BUS_TYPE)			
#if (ASIX_MINI || ASIX_CM5)		
	SPI_Setup(SPI_SSO_ENB|SPI_MST_SEL|SPI_SS_AUTO|SPI_ENB, SPI_STCFIE, 10, SLAVE_SEL_1); // 25M
	vTaskResume(xHandler_SPI);
	cSemaphoreGive(sem_SPI);
#endif				

#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
#if ARM_MINI || ARM_CM5
	if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM))
	{
		SPI1_Init(0);
		vTaskResume(xHandler_SPI);
	}
#endif
	cSemaphoreGive(sem_SPI);
#endif
#endif
			return 0;
		}
		for(;i >= 0;i--)
		{
			ret = f_lseek(&file[file_index + 24], i * READ_SD_BUFFER_SIZE);		// buffer size is 512	
		
			if(ret != FR_OK)
			{
				result = 0;
			}
			else
			{				
				ret = f_read(&file[file_index + 24], rw_sd_buf, READ_SD_BUFFER_SIZE, &br);
				
				if(ret != FR_OK)
				{
					result = 0;
				}
				else
				{
					// check start_time and end_time
					for(j = 127;j >= 0;j--)
					{
						if(i * 128 + j >= block_no - 1)
						{
							continue;
						}
						if((i * 128 + j) == 0)
						{
							if(f_start == 0 && f_end == 0)  // do not find start_time and end_time
							{
								*start_seg = 0;
								*total_seg = 0;
							}
							else if(f_end == 1)  // do not find start time 
							{
								*start_seg = 0;	
								*total_seg  = end_seg + 1;
								
//								if(end_time >= get_current_time() - 300)  // if ask recent time 
//								{ // recent time
//									 // start time and end time are not in range, and they are recent time
//									*total_seg = 1;
//									
//								}
//								else
//								{ // old time
//									if(*start_seg != 0)
//									{ //????????????????
//										*total_seg = 0;
//									}
//								}
							}
							i = -1;	  // end search
							j = -1;
							k = 0;
							break;
						}
						
						memcpy(&time,&rw_sd_buf[j*4],4);
					
						if(time != 0)
						{
							if((time < 1514736000) || (time > 1893427200))  //from 2018/1/1 to 2030/1/1
							{// error
#if ARM_UART_DEBUG
		uart1_init(115200);
		DEBUG_EN = 1;
		printf("find error time seg = %u %u, time = %u \r\n",i, j,time);
#endif	
				
									if(i * 128 + j - 1 > 0)
										*start_seg = i * 128 + j - 2;
									*total_seg = 1;								
								
								j = -1;
								i = -1;  // end search

								f_start = 1; // found start
								f_end = 1; // found end
								k = 0;
								break;
							}		
							
							if(time < end_time && f_end == 0) 
							{
								end_seg = i * 128 + j + 1;
								f_end = 1; // found end
								k = 0;	
#if ARM_UART_DEBUG
		uart1_init(115200);
		DEBUG_EN = 1;
		printf("find end seg = %u, time = %u \r\n",i * 128 + j,time);
#endif	
							}	
							
							if((time < start_time) && (f_start == 0)) 
							{
								*start_seg = i * 128 + j;
								f_start = 1; // found start
									
								if(f_end == 1) // already find end point
									*total_seg = end_seg - *start_seg;
								k = 0;	
#if ARM_UART_DEBUG
		uart1_init(115200);
		DEBUG_EN = 1;
		printf("find start seg = %u, time = %u \r\n",i * 128 + j,time);
#endif	
							}
							
							if((f_start == 1 && f_end == 1) || (time == 0))
							{  // find start and end, end searching
								j = -1;
								i = -1;  // end search
								k = 0;
							}
						}		
						else
						{
//							j = -1;
//							i = -1;  // end search
//							k = 0;
//							Test[19]++;
						}
					}
				}				
			}
			
		}		
		if(k == 0)
		{	
			result = 1;
			f_close(&file[file_index + 24]);
		}
			
	}
	else
	{
		result = 0;
	}
	
	
#if (SD_BUS == SPI_BUS_TYPE)
	
#if (ASIX_MINI || ASIX_CM5)		
	SPI_Setup(SPI_SSO_ENB|SPI_MST_SEL|SPI_SS_AUTO|SPI_ENB, SPI_STCFIE, 10, SLAVE_SEL_1); // 25M
	vTaskResume(xHandler_SPI);
	cSemaphoreGive(sem_SPI);
#endif	
	
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
#if ARM_MINI || ARM_CM5
	if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM))
	{
		SPI1_Init(0);
		vTaskResume(xHandler_SPI);
	}
#endif
	cSemaphoreGive(sem_SPI);

#endif

#endif	
	return result;
	
}

extern U8_T reading_sd;

// file_no -> range is  0-255
// index   -> which monitor, range is 1-12
// ana_dig -> range is 0 - 1
U8_T Write_SD(U16_T file_no,U8_T index,U8_T ana_dig,U32_T star_pos)
{
	uint8 ret;//, loop = 50;
	uint8 result;	
	uint8 file_index;  // range is  0 - 23

	if(SD_exist != 2) 
	{
		return 1;
	}
#if (SD_BUS == SPI_BUS_TYPE)
	// wait reading
#if ARM_MINI || ARM_CM5
	if(cSemaphoreTake(sem_SPI, 100) == pdFALSE)
	{
		return 0;
	}
#if ARM_MINI || ARM_CM5
	if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM))
	{
		vTaskSuspend(xHandler_SPI);
		SPI1_Init(1);
	}
#endif
#endif
	
#if (ASIX_MINI || ASIX_CM5)	
	if(cSemaphoreTake(sem_SPI, 200) == pdFALSE)
			return 0;
	vTaskSuspend(xHandler_SPI);	
	SD_HardWareInit();	
	
#endif
	
#endif
	if(ana_dig == 1)  // analog
	{
		file_index = index * 2;		
	}		
	else  // digital
	{
		file_index = index * 2 + 1;
	}
	result = 0;
	//if(star_pos == 0) // first packet, record start time
	{
		U32_T time;
		ret = open_file(0,file_index + 24);
		if(ret == FR_OK)
		{
			time = get_current_time();//RTC_GetCounter();
			// check time is valid or not
			if((time > 1514736000) && (time < 1893427200))  //from 2018/1/1 to 2030/1/1
			{
				ret = f_lseek(&file[file_index + 24] , (U32_T)SD_block_num[file_index] * 4);
//				if(ana_dig == 1)
//						memcpy(rw_sd_buf,&time,4);
//				else
				if(ret != FR_OK)
				{
					
				}
				else
				{
					memcpy(rw_sd_buf,&time,4);
					
					ret = f_write(&file[file_index + 24], &rw_sd_buf , 4, &bw);
					if(ret != FR_OK)
					{			
					}
					else
					{
						result = 1;
					}
				}
#if ARM_UART_DEBUG
	uart1_init(115200);
	DEBUG_EN = 1;
	printf("index %u,store time %lu,block %u \r\n",file_index,time,SD_block_num[file_index]);
#endif
				
			}	
			else
				;
			
			f_close(&file[file_index + 24]);	
		}
		else
		{
			if(ret == 13)  // FILE SYSTEM is not correct
			{
				SD_exist = 3;
				Setting_Info.reg.sd_exist = 3;
			}
		}
		
		
	}
	
	if(result == 1)  // write time_txt succusfully
	{
		result = 0;
		//if(pre_open_file_index != file_index)
		{
			ret = open_file(file_no,file_index);
		}
		if(ret == FR_OK)
		{
			ret = f_lseek(&file[file_index] , star_pos);
			if(ret != FR_OK)
			{
				ret = open_file(file_no,file_index);
				count_sd_seek_error++;
				
				if(count_sd_seek_error > 5)
				{
					// generate SD card erro alarm
					count_sd_seek_error = 0;
					generate_common_alarm(ALARM_ABNORMAL_SD);
				}
			}
			else
			{
				count_sd_seek_error = 0;
				if(ana_dig == 1)
						memcpy(rw_sd_buf,&write_mon_point_buf[index * 2], MAX_MON_POINT * sizeof(Str_mon_element));
				else
						memcpy(rw_sd_buf,&write_mon_point_buf[index * 2 + 1], MAX_MON_POINT * sizeof(Str_mon_element));

				ret = f_write(&file[file_index], &rw_sd_buf , MAX_MON_POINT * sizeof(Str_mon_element), &bw);
				if(ret != FR_OK)
				{			

				}
				// if it is last packet, open file
				if(star_pos == 255L * MAX_MON_POINT * sizeof(Str_mon_element))  // last packet
				{
					f_close(&file[file_index]);	
					
				}
				else
				{				
					f_sync(&file[file_index]);
				}	
				result = 1;  // write data succussfully
			}
		}
//		else
//		{
//			Test[29]++;
//		}
	}
#if (SD_BUS == SPI_BUS_TYPE)
	
#if (ASIX_MINI || ASIX_CM5)	
	SPI_Setup(SPI_SSO_ENB|SPI_MST_SEL|SPI_SS_AUTO|SPI_ENB, SPI_STCFIE, 10, SLAVE_SEL_1); // 25M
	vTaskResume(xHandler_SPI);
	cSemaphoreGive(sem_SPI);
#endif

#if ARM_MINI || ARM_CM5
	if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM))
	{
		SPI1_Init(0);
		vTaskResume(xHandler_SPI);
	}
	cSemaphoreGive(sem_SPI);

#endif

#endif	
	return result;
}

U8_T Write_ALARM_TO_SD(Alarm_point alarm,U32_T star_pos)
{

//	uint8 str[100];  

//	uint8 ret;//, loop = 50;
//	if(SD_exist != 2) return 0;
//#if (ASIX_MINI || ASIX_CM5)	
//	if(cSemaphoreTake(sem_SPI, 50) == pdFALSE)
//		return 0;
//	
//	vTaskSuspend(xHandler_SPI);
//	
//	
//#endif

//	ret = f_mount(0, &fs);
//	if(FR_OK == ret)
//	{
//			ret = open_file(0,32);
//	}
//	else
//	{

//	}

//	
//	f_lseek((FIL *)&file, star_pos);
//	
//// store point
//	memset(str,0,100);
//#if (ASIX_MINI || ASIX_CM5)
//	sprintf((char *)str, "%s", time);
//#endif
//	sprintf((char *)&str[20], alarm.alarm_message, sizeof(alarm.alarm_message));
//	str[98] = 0x0d;
//	str[99] = 0x0a;
//	f_write((FIL *)&file, str ,100, &bw);
//	f_close((FIL *)&file);
//	
//#if (ASIX_MINI || ASIX_CM5)	
//	SPI_Setup(SPI_SSO_ENB|SPI_MST_SEL|SPI_SS_AUTO|SPI_ENB, SPI_STCFIE, 10, SLAVE_SEL_1); // 25M	


//	vTaskResume(xHandler_SPI);	
//	cSemaphoreGive(sem_SPI);
//#endif
	return 1;	
}

#if 0
U8_T Read_Picture_from_SD(U8_T fileindex,U16_T index)
{
  //U16_T loop = 50;
	uint8 result;
	uint8 ret;
	
	if(SD_exist != 2)
	{
		return 1;
	}		
	if(flag_f_mount != FR_OK) { return 0;}
#if (SD_BUS == SPI_BUS_TYPE)
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
	if(cSemaphoreTake(sem_SPI, 200) == pdFALSE)
	{
		return 0;
	}
#if ARM_MINI || ARM_CM5
	if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM))
	{
		vTaskSuspend(xHandler_SPI);
		SPI1_Init(1);
	}
#endif
#endif
	
#if (ASIX_MINI || ASIX_CM5)		
	if(cSemaphoreTake(sem_SPI, 200) == pdFALSE)
			return 0;
	vTaskSuspend(xHandler_SPI);
	SD_HardWareInit();	
#endif
#endif	
	
	ret = open_file(0,48 + fileindex);	

	if(ret == FR_OK)
	{
		result = 1;
		ret = f_lseek((FIL *)&file, (U32_T)index * PIC_PACKET_LEN);
		if(ret != FR_OK)
		{
			result = 0;
		}
		else
		{
			ret = f_read((FIL *)&file, rw_sd_buf, PIC_PACKET_LEN, &br);
			if(ret != FR_OK)
			{
				result = 0;
				return 0;
			}

			memcpy(&read_mon_point_buf, rw_sd_buf, PIC_PACKET_LEN);

			// for test
			
			
			f_close((FIL *)&file);	
		}
		
	}
	else
	{
		result = 0;
	}
#if (SD_BUS == SPI_BUS_TYPE)	
#if (ASIX_MINI || ASIX_CM5)		
	SPI_Setup(SPI_SSO_ENB|SPI_MST_SEL|SPI_SS_AUTO|SPI_ENB, SPI_STCFIE, 10, SLAVE_SEL_1); // 25M
	vTaskResume(xHandler_SPI);
	cSemaphoreGive(sem_SPI);
#endif	
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
#if ARM_MINI || ARM_CM5
	if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM))
	{
		SPI1_Init(0);
		vTaskResume(xHandler_SPI);
	}
#endif
	cSemaphoreGive(sem_SPI);

#endif
#endif
	return result;	
}


U8_T Write_Picture(U8_T fileindex,U8_T * buf,U16_T index)
{
	uint8 ret;//, loop = 50;
		
	if(SD_exist != 2)
	{
		return 1;
	}		
	if(flag_f_mount != FR_OK) { return 0;}
#if (SD_BUS == SPI_BUS_TYPE)
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
	if(cSemaphoreTake(sem_SPI, 200) == pdFALSE)
	{
		return 0;
	}
#if ARM_MINI || ARM_CM5
	if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM))
	{
		vTaskSuspend(xHandler_SPI);
		SPI1_Init(1);
	}
#endif
#endif
	
#if (ASIX_MINI || ASIX_CM5)		
	if(cSemaphoreTake(sem_SPI, 200) == pdFALSE)
			return 0;
	vTaskSuspend(xHandler_SPI);
	SD_HardWareInit();	
#endif
#endif	
	ret = f_mount(0, &fs);
	if(FR_OK == ret)
	{
			ret = open_file(0,48 + fileindex);
	}
	else
	{

	}

	ret = f_lseek((FIL *)&file, (U32_T)index * PIC_PACKET_LEN);	

	ret = f_write((FIL *)&file, buf , PIC_PACKET_LEN, &bw);

	f_close((FIL *)&file);
#if (SD_BUS == SPI_BUS_TYPE)	
#if (ASIX_MINI || ASIX_CM5)		
	SPI_Setup(SPI_SSO_ENB|SPI_MST_SEL|SPI_SS_AUTO|SPI_ENB, SPI_STCFIE, 10, SLAVE_SEL_1); // 25M
	vTaskResume(xHandler_SPI);
	cSemaphoreGive(sem_SPI);
#endif	
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )
#if ARM_MINI || ARM_CM5
	if((Modbus.mini_type == MINI_SMALL) || (Modbus.mini_type == MINI_SMALL_ARM))
	{
		SPI1_Init(0);
		vTaskResume(xHandler_SPI);
	}
#endif
	cSemaphoreGive(sem_SPI);

#endif
#endif	
	return 1;		
}
#endif


//???????????,?????,?????.
//u8 char_upper(u8 c)
//{
//	if(c < 'A')
//		return c;			//??,????.
//	
//	if(c >= 'a')
//		return c - 0x20;	//????.
//	else
//		return c;			//??,????
//}

//???????
//fname:???
//???:0XFF,?????????????.
//		 ??,?????????,?????????.
//u8 f_typetell(u8 *fname)
//{
//	u8 tbuf[5];
//	u8 *attr = '\0';//???
//	u8 i = 0, j;
//	
//	while(i < 250)
//	{
//		i++;
//		if(*fname == '\0')
//			break;	//???????.
//		
//		fname++;
//	}
//	
//	if(i == 250)
//		return 0XFF;//??????.
//	
// 	for(i = 0; i < 5; i++)//?????
//	{
//		fname--;
//		if(*fname == '.')
//		{
//			fname++;
//			attr = fname;
//			break;
//		}
//  	}
//	
//	strcpy((char *)tbuf, (const char*)attr);//copy
// 	for(i = 0; i < 4; i++)
//		tbuf[i] = char_upper(tbuf[i]);		//?????? 
//	
//	for(i = 0; i < 6; i++)
//	{
//		for(j = 0; j < 13; j++)
//		{
//			if(*FILE_TYPE_TBL[i][j] == 0)
//				break;						//?????????????.
//			
//			if(strcmp((const char *)FILE_TYPE_TBL[i][j], (const char *)tbuf) == 0)//???
//			{
//				return (i << 4) | j;
//			}
//		}
//	}
//	
//	return 0XFF;//???		 			   
//}	 

//????????
//drv:????("0:"/"1:")
//total:???	 (??KB)
//free:????	 (??KB)
//???:0,??.??,????
//u8 exf_getfree(u8 *drv, u32 *total, u32 *free)
//{
//	FATFS *fs1;
//	u8 res;
//    u32 fre_clust = 0, fre_sect = 0, tot_sect = 0;
//    //????????????
//    res = f_getfree((const TCHAR*)drv, (void *)&fre_clust, &fs1);
//    if(res == 0)
//	{											   
//	    tot_sect = (fs1->n_fatent - 2) * fs1->csize;	//??????
//	    fre_sect = fre_clust * fs1->csize;				//???????	   
//		
//#if _MAX_SS != 512						  				//??????512??,????512??
//		tot_sect *= fs1->ssize / 512;
//		fre_sect *= fs1->ssize / 512;
//#endif
//		
//		*total = tot_sect >> 1;							//???KB
//		*free = fre_sect >> 1;							//???KB 
// 	}
//	
//	return res;
//}

#endif