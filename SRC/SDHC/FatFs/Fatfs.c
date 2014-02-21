
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ff.h"
#include "task.h"
#include "queue.h"
#include "FatFs.h"
#include "../sd/sddriver.h"
#include "../LCD/lcd.h"
#include "../modbus/serial.h"
#include "../modbus/define.h"
#include "../debug/printd.h"
#include "../program/hex.h"
#include "../program/program.h"

#define	FatFsSTACK_SIZE		512
xTaskHandle xHandle_FatFs;

DWORD AccSize;				/* Work register for fs command */
WORD AccFiles, AccDirs;
FILINFO Finfo;

#if _USE_LFN
char Lfname[_MAX_LFN+1];
#endif

FATFS fs;
FIL file; 
FIL dst_file;

U8_T sd_status = 0;

#define READ_SIZE	512
#define WRITE_SIZE	512
U16_T br, bw;
U8_T read_buf[READ_SIZE + 1];
U8_T write_buf[READ_SIZE + 1];
uint32 read_ptr = 0;
uint32 file_size = 0;
uint8 hex_buf[100];

uint8 *token = "\n\r";
uint8 *getstr;
uint8 valid_length = 0;
uint8 hex_bytes = 0;

HEX_FORMAT hexline;
uint8 file_end_flag = FALSE;
uint32 max_data_length;
uint16 max_package;

/* parameter: Pointer to the working buffer with start path */
static FRESULT scan_files (char* path) reentrant
{
	DIR dirs;
	FRESULT res;
	int i;
	char *fn;

	res = f_opendir(&dirs, path);
	if(res == FR_OK)
	{
		sub_send_string("f_opendir...success\n", 20);
		i = strlen(path);
		while(((res = f_readdir(&dirs, &Finfo)) == FR_OK) && Finfo.fname[0])
		{
			if(_FS_RPATH && Finfo.fname[0] == '.') continue;
#if _USE_LFN
			fn = *Finfo.lfname ? Finfo.lfname : Finfo.fname;
#else
			fn = Finfo.fname;
#endif
			if(Finfo.fattrib & AM_DIR)
			{
				AccDirs++;
				path[i] = '/';
				strcpy(path+i+1, fn);
				res = scan_files(path);
				path[i] = '\0';
				if(res != FR_OK) break;
			}
			else
			{
//				xprintf(PSTR("%s/%s\n"), path, fn);
//				Lcd_Show_String(line++, 0, DISP_NOR, fn);
//				sub_send_string(fn, strlen(fn));
				AccFiles++;
				AccSize += Finfo.fsize;
			}
		}
	}
	
	return res;
}

uint8 str_to_hex_uint8(uint8 *str)
{
	uint8 high_4bits, low_4bits;

	if(IsDigit(*str))
	{
		high_4bits = *str - '0';
	}
	else if(IsLower(*str))
	{
		high_4bits = *str - 'a' + 10;
	}
	else // upper character
	{
		high_4bits = *str - 'A' + 10;
	}

	str++;

	if(IsDigit(*str))
	{
		low_4bits = *str - '0';
	}
	else if(IsLower(*str))
	{
		low_4bits = *str - 'a' + 10;
	}
	else // upper character
	{
		low_4bits = *str - 'A' + 10;
	}

	return 	((high_4bits << 4) | low_4bits);
}

uint16 str_to_hex_uint16(uint8 *str)
{
	uint8 high_byte_high_4bits, high_byte_low_4bits, low_byte_high_4bits, low_byte_low_4bits;

	if(IsDigit(*str))
	{
		high_byte_high_4bits = *str - '0';
	}
	else if(IsLower(*str))
	{
		high_byte_high_4bits = *str - 'a' + 10;
	}
	else // upper character
	{
		high_byte_high_4bits = *str - 'A' + 10;
	}

	str++;

	if(IsDigit(*str))
	{
		high_byte_low_4bits = *str - '0';
	}
	else if(IsLower(*str))
	{
		high_byte_low_4bits = *str - 'a' + 10;
	}
	else // upper character
	{
		high_byte_low_4bits = *str - 'A' + 10;
	}

	str++;

	if(IsDigit(*str))
	{
		low_byte_high_4bits = *str - '0';
	}
	else if(IsLower(*str))
	{
		low_byte_high_4bits = *str - 'a' + 10;
	}
	else // upper character
	{
		low_byte_high_4bits = *str - 'A' + 10;
	}

	str++;

	if(IsDigit(*str))
	{
		low_byte_low_4bits = *str - '0';
	}
	else if(IsLower(*str))
	{
		low_byte_low_4bits = *str - 'a' + 10;
	}
	else // upper character
	{
		low_byte_low_4bits = *str - 'A' + 10;
	}

	return 	(((uint16)high_byte_high_4bits << 12) | ((uint16)high_byte_low_4bits << 8) | (low_byte_high_4bits << 4) | low_byte_low_4bits);
}

uint8 parse(uint8 *src, uint8 *dst, uint8 *dst_len, uint8 *counter)
{
	uint8 check, i;
	uint8 line_length = strlen(src);
	
	*counter = 0;

	if(*src != ':')				// line header
		return FALSE;

	src++;
	hexline.length = str_to_hex_uint8(src); // data length

	if(line_length != (11 + 2*hexline.length))
		return FALSE;

	src += 2;
	hexline.address = str_to_hex_uint16(src); // data address

	src += 4;
	hexline.type = str_to_hex_uint8(src); // data type

	src += 2;
	for(i = 0; i < hexline.length; i++)
	{
		*dst = str_to_hex_uint8(src);
		src += 2;
		dst++;
	}
	*dst = '\0';

	check = str_to_hex_uint8(src);

	*dst_len = hexline.length;
	*counter = line_length;
	return TRUE;
}

void FatFs_task(void)
{
	portTickType xDelayPeriod = (portTickType)500 / portTICK_RATE_MS;
	sub_net_init();
	while(1)
	{
	 	vTaskDelay(xDelayPeriod);
		if(sd_status == 1)  // parse hex file, and read it to RAM
		{
			f_mount(0, &fs);
//			scan_files("0:/");
		    f_open(&file, "0:/64K.hex", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
			read_ptr = 0;

//			file_size = f_size(&file);
//			printd("\r\nfile_size = %lu", file_size);

			file_end_flag = FALSE;
			memset(hex_ram_0, 0xff, HEX_BANK_SIZE);
			memset(hex_ram_1, 0xff, HEX_BANK_SIZE);

			max_data_length = 0;
			max_package = 0;

			while(1)
			{
				if(FR_OK == f_lseek(&file, read_ptr))
				{
					if((FR_OK == f_read(&file, read_buf, READ_SIZE, &br)) && (br != 0))
					{
						read_buf[br] = '\0';
						if(getstr = strtok(read_buf, token))
						{
							do
							{
								if(parse(getstr, hex_buf, &hex_bytes, &valid_length) == TRUE)
								{
									if(hexline.type == 0)
									{
										uint8 i;
										for(i = 0; i < hex_bytes; i++)
										{
											if(hexline.length < HEX_BANK_SIZE)
											{
												hex_ram_0[hexline.address + i] = hex_buf[i];
											}
											else
											{
												hex_ram_1[hexline.address + i - HEX_BANK_SIZE] = hex_buf[i];
											} 
										}
									}
									else if(hexline.type == 1)
									{
										file_end_flag = TRUE;
										break;
									}
									else
									{

									}

									read_ptr += valid_length;
									
									if(max_data_length < (hexline.address + hexline.length))
									{
										max_data_length = hexline.address + hexline.length;
									}
								}
							} while(getstr = strtok(NULL, token));
						}

//						printd("\r\nread_ptr = %lu", read_ptr);
					}
					else
					{
						printd("\r\nf_read() failed..");
					}
				}
				else
				{
					printd("\r\nf_lseek() failed..");
				}
				
				if(file_end_flag == TRUE)
				{
					max_package = (max_data_length >> 7);
					if(max_data_length & 0x0000007f)
						max_package++;
					break;
				}

				watchdog_reset();
				vTaskDelay(5);
			}

			f_close(&file);

			sd_status = 2;
		}
		// for test the hex file parse
		else if(sd_status == 3)	// write the hex data from RAM to a temporary file
		{
			uint16 i;
			f_open(&dst_file, "0:/write.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
			for(i = 0; i < max_package; i++)
			{
				if(i < 256)
				{
//					sub_send_string(hex_ram_0 + ((uint16)i << 7), 128);
					f_write(&dst_file, hex_ram_0 + ((uint16)i << 7), 128, &bw);
				}
				else
				{
//					sub_send_string(hex_ram_1 + ((uint16)(i - 256) << 7), 128);
					f_write(&dst_file, hex_ram_1 + ((uint16)(i - 256) << 7), 128, &bw);
				}
				vTaskDelay(5);
			}
			f_close(&dst_file);
			sd_status = 4;
		}
		else if(sd_status == 5) // for test, read the temporary hex file out
		{
			uint16 i;
			f_open(&dst_file, "0:/write.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
			f_lseek(&dst_file, 0);
			for(i = 0; i < max_package; i++)
			{
				f_read(&dst_file, read_buf, 128, &br);
				sub_send_string(read_buf, 128);
				vTaskDelay(5);
			}
			f_close(&dst_file);
			sd_status = 6;
		}
		else if(sd_status == 7) // program tstat
		{
			uint8 retry = 0;
			uint8 update_end = FALSE;
			isp_status = PROG_INIT;
			do
			{
				switch(isp_status)
				{
//					case SAVE_BAUDRATE:
//						
//						break;
//					case SET_BAUDRATE:
//						
//						break;
					case PROG_INIT:
						if(isp_init() == TRUE)
						{
							isp_status = PROG_ERASE_FLASH;
						}
						else
						{
							retry++;
							if(retry >= 5)
							{
								update_end = TRUE;
								retry = 0;
							}
							else
							{
								vTaskDelay(100);
							}
						}
						break;
					case PROG_ERASE_FLASH:
						if(isp_erase_flash() == TRUE)
						{
							isp_status = PROG_START;
						}
						else
						{
							retry++;
							if(retry >= 3)
							{
								update_end = TRUE;
								retry = 0;
							}
							else
							{
								vTaskDelay(100);
							}
						}
						break;
					case PROG_START:
						if(isp_start() == TRUE)
						{
							isp_status = PROG_PROCESS;
						}
						else
						{
							retry++;
							if(retry >= 3)
							{
								update_end = TRUE;
								retry = 0;
							}
							else
							{
								vTaskDelay(100);
							}
						}
						break;
					case PROG_PROCESS:
						if(isp_process() == TRUE)
							isp_status = PROG_PROCESS;
						else
							update_end = TRUE;
						break;
					case PROG_END:
						isp_end();
						update_end = TRUE;
						break;
//					case RESTORE_BAUDRATE:
//						
//						break;
					default:
						break;
				}
			}while(update_end == FALSE);

			sd_status = 8;
		}
		else if(sd_status == 9) // test it works with the SDHC card
		{
			uint8 ret, i = 50;
			do
			{
				ret = SD_Initialize();
			} while((ret != SD_NO_ERR) && (--i));
			if(ret != SD_NO_ERR)
				printd("\r\nSD_Initialize() failed ..");

			memset(write_buf, 'c', 512);
			memset(read_buf, 'k', 512);
			ret = SD_WriteBlock(0, write_buf);
			if(ret != SD_NO_ERR)
				printd("\r\nSD_WriteBlock() failed ..");

			ret = SD_ReadBlock(0, read_buf);
			if(ret != SD_NO_ERR)
				printd("\r\nSD_ReadBlock() failed ..");
			else
			{
			 	read_buf[512] = 0;
				printd("\r\n%s", read_buf);
			}
			sd_status = 10;
		}
		else if(sd_status == 11)
		{
			uint8 *str = "Hello, world!";
			FRESULT ret;
			ret = f_mount(0, &fs);
			if(FR_OK == ret)
			{
//				printd("\r\nfmount() OK!");
				ret = f_open(&file, "0:/test.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
//		    	printd("\r\n->f_open() returns %bd", ret);
			}
			f_lseek(&file, 0);
			f_write(&file, str, strlen(str), &bw);
			f_lseek(&file, 0);
			f_read(&file, read_buf, 50, &br);
			read_buf[br] = '\0';
			printd("\r\n%s", read_buf);
			f_close(&file);
			sd_status = 12;
		}
	}
}

void vStartFatfsTask(U8_T uxPriority)
{
	sTaskCreate(FatFs_task, (const signed portCHAR * const)"FatFs_task", FatFsSTACK_SIZE, NULL, uxPriority, (xTaskHandle *)&xHandle_FatFs);
}