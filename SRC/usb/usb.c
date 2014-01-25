#include "main.h"

U8_T UpIndex = 0;


void USB_responseData(U16_T address)
{
	responseCmd(USB,DownBuf,NULL);
}


void USB_task(void)
{
	U8_T len, length;
//	U16_T StartAdd;
	portTickType xDelayPeriod = (portTickType)20 / portTICK_RATE_MS;
	CH375_Init();
	while(1)
	{   
		vTaskDelay(xDelayPeriod);
		if(usb_poll() == TRUE)
		{
			mCH375Interrupt();
		}
		else
		{
			if(USB_timeout == 0)
			{
				if(DownCtr > 0)
				{
				//   LED = Usb_OK; 					
					flagLED_usb_rx = TRUE;
					usb_heartbeat = 0;
					if((DownBuf[0] == Modbus.address) || (DownBuf[0] == 0xff))	// Address of NetControl 
					{ 							
						USB_responseData(((U16_T)DownBuf[2] << 8) + DownBuf[3]);
					/*	if(DownBuf[1] == 0x19) 		//scan Tsnet
						{
							Sever_Order = SERVER_USB;		//USB
							Sever_id = DownBuf[0];
							Tx_To_Tstat(DownBuf, DownCtr);			   												
						}			25		
						else if(DownBuf[1] == 0x1a)	//scan NC
						{ 
							UpCtr = 0;
							UpBuf[UpCtr++] = DownBuf[0];
							UpBuf[UpCtr++] = 0x19;
							UpBuf[UpCtr++] = Para[13];						 
							UpBuf[UpCtr++] = Para[1];
							UpBuf[UpCtr++] = Para[3];
							UpBuf[UpCtr++] = Para[5];
							UpBuf[UpCtr++] = Para[7];
							InitCRC16();
							for(len = 0; len < UpCtr; len++)
								CRC16_Tstat(UpBuf[len]);
							UpBuf[UpCtr++] = CRChi;
							UpBuf[UpCtr++] = CRClo;
							UpIndex = 0;
							ENDP2_NEED_UP_FLAG = 1;				         
						} 
						else if(DownBuf[1] == READ_VARIABLES) 
						{
							UpBuf[0] = DownBuf[0];
							UpBuf[1] = DownBuf[1];
							length = (DownBuf[4] << 8) | DownBuf[5];
							UpBuf[2] = 2 * length;
							UpCtr = 3 + UpBuf[2];
							StartAdd = (DownBuf[2] << 8) | DownBuf[3];						                             
							for(len = 0; len < length; len++)
							{
								if(StartAdd < 200)
								{
									UpBuf[3 + 2 * len] = Para[2 * (StartAdd + len)];  
									UpBuf[4 + 2 * len] = Para[2 * (StartAdd + len) + 1];
								}
								else if((StartAdd + len) >= MODBUS_TIMER_ADDRESS) 
								{
									if(StartAdd + len < MODBUS_TIMER_ADDRESS + 8)
									{	 
									//  YEAR  MONTH  WEEK DATE  HOUR  MINUTE  SECOND 
										UpBuf[3 + 2 * len] = 0;
										switch(StartAdd + len - MODBUS_TIMER_ADDRESS)
										{
											case 7:
												UpBuf[4 + 2 * len] = Time.UN.Current.sec;
												break;	
											case 6:
												UpBuf[4 + 2 * len] = Time.UN.Current.min;
												break;
											case 5:
												UpBuf[4 + 2 * len] = Time.UN.Current.hour;
												break;
											case 4:
												UpBuf[4 + 2 * len] = Time.UN.Current.day;
												break;
											case 3:
												UpBuf[4 + 2 * len] = Time.UN.Current.dayofweek;
												break;
											case 2:
												UpBuf[4 + 2 * len] = Time.UN.Current.month;
												break;
											case 1:
												UpBuf[4 + 2 * len] = Time.UN.Current.year;
												break;
											case 0:
												UpBuf[4 + 2 * len] = Time.UN.Current.centary;
												break;
										}		
									}
									else if(StartAdd + len >= MODBUS_WR_DESCRIP_FIRST && StartAdd + len < MODBUS_WR_DESCRIP_LAST)
									{
										U8_T temp_number = (StartAdd + len - MODBUS_WR_DESCRIP_FIRST) / WR_DESCRIPTION_SIZE;
										U8_T temp_address = (StartAdd + len - MODBUS_WR_DESCRIP_FIRST) % WR_DESCRIPTION_SIZE;
										U8_T send_buffer = WR_Roution[temp_number].UN.all[temp_address];
									    if((temp_address == (WR_DESCRIPTION_SIZE - 1)) && (send_buffer != 0xff))
										{
											if((send_buffer & 0x80) == 0)
											{
												if(GetBit(temp_number, wr_state_index))
													send_buffer |= 0x40;
												else
													send_buffer &= 0xbf;
											}

											if(GetBit(temp_number, holiday1_state_index))
												send_buffer |= 0x20;
											else
												send_buffer &= 0xdf;

											if(GetBit(temp_number, holiday2_state_index))
												send_buffer |= 0x10;
											else
												send_buffer &= 0xef;
										}
										UpBuf[3 + 2 * len] = 0;	
										UpBuf[4 + 2 * len] = send_buffer;	
									}
									else if(StartAdd + len >= MODBUS_AR_DESCRIP_FIRST && StartAdd + len < MODBUS_AR_DESCRIP_LAST)
									{
										U8_T temp_number = (StartAdd + len - MODBUS_AR_DESCRIP_FIRST) / AR_DESCRIPTION_SIZE;    // line
										U8_T temp_address = (StartAdd + len - MODBUS_AR_DESCRIP_FIRST) % AR_DESCRIPTION_SIZE;
										U8_T send_buffer = AR_Roution[temp_number].UN.all[temp_address];
				
										if(temp_address == (AR_DESCRIPTION_SIZE - 1))
										{
											if((send_buffer & 0x80) == 0)
											{
												if(GetBit(temp_number, ar_state_index))
													send_buffer |= 0x40;
												else
													send_buffer &= 0xbf;
											}
										}
										UpBuf[3 + 2 * len] = 0;	
										UpBuf[4 + 2 * len] = send_buffer;	
									}
									else if(StartAdd + len >= MODBUS_ID_FIRST && StartAdd + len < MODBUS_ID_LAST)
									{
										U8_T temp_number = (StartAdd + len - MODBUS_ID_FIRST) / ID_SIZE;
										U8_T temp_address = (StartAdd + len - MODBUS_ID_FIRST) % ID_SIZE;
										U8_T send_buffer = ID_Config[temp_number].all[temp_address];
						
										if((temp_address == (ID_SIZE - 1)) && (send_buffer != 0xff))
										{
											if((send_buffer & 0x80) == 0)
											{
												if(GetBit(temp_number, output_state_index))
													send_buffer |= 0x40;
												else
													send_buffer &= 0xbf;
											}

											if(GetBit(temp_number, schedual1_state_index))
												send_buffer |= 0x20;
											else
												send_buffer &= 0xdf;

											if(GetBit(temp_number, schedual2_state_index))
												send_buffer |= 0x10;
											else
												send_buffer &= 0xef;
										}
										UpBuf[3 + 2 * len] = 0;	
										UpBuf[4 + 2 * len] = send_buffer;	
									}
									else if(StartAdd + len >= MODBUS_AR_TIME_FIRST && StartAdd + len < MODBUS_WR_ONTIME_FIRST)
									{
										U8_T temp_number = (StartAdd + len - MODBUS_AR_TIME_FIRST) / AR_TIME_SIZE;
										U8_T temp_address = (StartAdd + len - MODBUS_AR_TIME_FIRST) % AR_TIME_SIZE;
										U8_T send_buffer = AR_Roution[temp_number].Time[temp_address];
						
										UpBuf[3 + 2 * len] = 0;	
										UpBuf[4 + 2 * len] = send_buffer;
									}
									else if(StartAdd + len >= MODBUS_WR_ONTIME_FIRST && StartAdd + len < MODBUS_WR_OFFTIME_FIRST)
									{
										U8_T temp_number = (StartAdd + len - MODBUS_WR_ONTIME_FIRST) / WR_TIME_SIZE;		
										U8_T temp_address = (StartAdd + len - MODBUS_WR_ONTIME_FIRST) % WR_TIME_SIZE;
										U8_T send_buffer = WR_Roution[temp_number].OnTime[temp_address];
										UpBuf[3 + 2 * len] = 0;	
										UpBuf[4 + 2 * len] = send_buffer;
									}
									else if(StartAdd + len >= MODBUS_WR_OFFTIME_FIRST && StartAdd + len < MODBUS_TOTAL_PARAMETERS)
									{
										U8_T temp_number = (StartAdd + len - MODBUS_WR_OFFTIME_FIRST) / WR_TIME_SIZE;
										U8_T temp_address = (StartAdd + len - MODBUS_WR_OFFTIME_FIRST) % WR_TIME_SIZE;
										U8_T send_buffer = WR_Roution[temp_number].OffTime[temp_address];
										UpBuf[3 + 2 * len] = 0;	
										UpBuf[4 + 2 * len] = send_buffer;			
									}
									else
									{
										UpBuf[3 + 2 * len] = 0;	
										UpBuf[4 + 2 * len] = 1;
									}						
								} 
							}

							InitCRC16();
							for(len = 0; len < UpCtr; len++)
								CRC16_Tstat(UpBuf[len]);

							UpBuf[UpCtr++] = CRChi;                        
							UpBuf[UpCtr++] = CRClo;

							UpIndex = 0;
							ENDP2_NEED_UP_FLAG = 1;							  					
						}
						else if(DownBuf[1] == WRITE_VARIABLES) 
						{
							if(((DownBuf[2] << 8) | DownBuf[3]) < 200)
							{
								if(((DownBuf[2] << 8) | DownBuf[3]) == 106)
								{
									if((((Para[212] << 8) | Para[213]) == 0) && (((DownBuf[4] << 8) | DownBuf[5]) != 0))
									{
										Para[212] = DownBuf[4];
										Para[213] = DownBuf[5];
										ChangeIP = 1;
										ChangeFlash = 2;
									}
								}
								else if((((DownBuf[2] << 8) | DownBuf[3]) >= 107) && (((DownBuf[2] << 8) | DownBuf[3]) <= 120)) //IP change ,reset cpu
								{
									if(((Para[212] << 8) | Para[213]) == 0)
									{
										Para[2*DownBuf[3]] = DownBuf[4];		//write to bufffer array high bit
										Para[2*DownBuf[3] + 1] = DownBuf[5];	//write to bufffer array low bit
										ChangeFlash = 2;
										ChangeIP = 1;
									}
								}
								else
								{
									Para[2*DownBuf[3]] = DownBuf[4]; 			//write to bufffer array high bit
									Para[2*DownBuf[3] + 1] = DownBuf[5];		//write to bufffer array low bit
									ChangeFlash = 2;
								}
							}

							for(len = 0; len < DownCtr; len++)
								UpBuf[len] = DownBuf[len];

							UpCtr = len;
							UpIndex = 0;
							ENDP2_NEED_UP_FLAG = 1;
						}*/
					}
					else
					{
					//	Sever_Order = SERVER_USB;		//USB
					//	Sever_id = DownBuf[0];				  
					//	Tx_To_Tstat(DownBuf, DownCtr);								 			          			 
					} 
					
					DownCtr = 0;
				}
			}
			if((ENDP2_NEED_UP_FLAG == 1) && (ENDP2_UP_SUC_FLAG == 1) && UpCtr)
			{
				flagLED_usb_tx = TRUE;
				ENDP2_UP_SUC_FLAG = 0;
				if(UpCtr > BULK_IN_ENDP_MAX_SIZE)
				{
					length = BULK_IN_ENDP_MAX_SIZE;
					UpCtr -= BULK_IN_ENDP_MAX_SIZE;
				}
				else
				{
					length = UpCtr;
					UpCtr = 0;
					ENDP2_NEED_UP_FLAG = 0;
				}

				CH375_WR_CMD_PORT(CMD_WR_USB_DATA7);			// 发出写上传端点2命令
				CH375_WR_DAT_PORT(length);
				for(len = 0; len < length; len++)
					CH375_WR_DAT_PORT(UpBuf[UpIndex++]);
			}
		}
	}
}