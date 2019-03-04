
#include "main.h"
#include "ch375usb.h"


#if USB_DEVICE || USB_HOST

#if USB_DEVICE
#include "ch375inc.h"
#include "descriptor_com.h"

mREQUEST_PACKET far request;
mUART_PARA far uartpara;

bit CH375FLAGERR;							// 错误标志
bit CH375CONFLAG;							// 设备是否已经配置标志
bit ENDP2_NEED_UP_FLAG = 0;					// 端点2有数据需要上传标志
bit ENDP2_UP_SUC_FLAG = 1;					// 端点2本次数据上传成功标志
bit SET_LINE_CODING_Flag;					// 类请求SET_LINE_CODING标志

U8_T VarUsbAddress;							// 设备地址
U8_T mVarSetupRequest;						// USB请求码
U16_T mVarSetupLength;						// 后续数据长度
U8_T *VarSetupDescr;

U8_T UpIndex = 0;
U8_T far DownBuf[300];
U8_T far UpBuf[300];
U8_T far USB_timeout = USB_TIMEOUT;
U16_T far DownCtr = 0;
U16_T xdata UpCtr = 0;


extern U8_T usb_heartbeat;

/* 延时2us */
static void Delay10us(void)
{
	DELAY_1_US();
	DELAY_1_US();
	DELAY_1_US();
	DELAY_1_US();
	DELAY_1_US();
	DELAY_1_US();
	DELAY_1_US();
	DELAY_1_US();
	DELAY_1_US();
	DELAY_1_US();
}

static void Delay5ms(void)
{
	U16_T i;
	for(i = 0; i < 5000; i++)
	{
		DELAY_1_US();
	}	
}

void USB_enable(void)
{
	CH375_WR_CMD_PORT(CMD_SET_USB_MODE);				// 设置USB工作模式, 必要操作
	CH375_WR_DAT_PORT(1);  								// 设置为使用外部固件的USB设备方式
}

void init_virtual_com(void)
{
//	uartpara.uart.bBaudRate1 = 0x80;			// baudrate = 0X00002580 ,即9600(默认)
//	uartpara.uart.bBaudRate2 = 0x25;
//	uartpara.uart.bBaudRate3 = 0x00;
//	uartpara.uart.bBaudRate4 = 0x00;
	uartpara.uart.bBaudRate1 = 0x00;			// baudrate = 0X00004B00 ,即19200
	uartpara.uart.bBaudRate2 = 0x4B;
	uartpara.uart.bBaudRate3 = 0x00;
	uartpara.uart.bBaudRate4 = 0x00;
	uartpara.uart.bStopBit = 0x00;				// Stop bit: 1
	uartpara.uart.bParityBit = 0x00;			// Parity bit: None
	uartpara.uart.bDataBits  = 0x08;			// Data bits: 8
}



/* CH375初始化子程序 */
U8_T CH375_Device_Init(void)
{
    U8_T ret = FALSE;
	U8_T i = 0;
	while(1)
	{
		USB_enable();

		CH375_WR_CMD_PORT(CMD_CHECK_EXIST);
		CH375_WR_DAT_PORT(0x55);
		
		DELAY_Ms(50);
		
		if((i = CH375_RD_DAT_PORT()) != 0xaa)
		{		
			return ret;
		}
		else
		{  
			break;
		}
	}
	
	init_virtual_com();
	
	USB_enable();
	for(i = 0; i < 20; i++)   							// 等待操作成功,通常需要等待10uS-20uS 
  {
		if(CH375_RD_DAT_PORT() == CMD_RET_SUCCESS)
		{
			ret = TRUE;
			break;
		}
	}

	return ret;
}


void USB_device_Initial(void)
{
	UpIndex = 0;
	ENDP2_NEED_UP_FLAG = 0;					// 端点2有数据需要上传标志
	ENDP2_UP_SUC_FLAG = 1;					// 端点2本次数据上传成功标志
	USB_timeout = USB_TIMEOUT;
	DownCtr = 0;
	UpCtr = 0;
	CH375_Device_Init();
}

/*端点0数据上传*/
void mCh375Ep0Up(void)
{
	U8_T i, len;
	if(mVarSetupLength)										// 长度不为0传输具体长度的数据		
    {
		if(mVarSetupLength <= CH375_EP0_SIZE)				// 长度小于8则长输要求的长度, 端点0最大数据包为8bytes
        {
			len = mVarSetupLength;
			mVarSetupLength = 0;
        }	 
		else												// 长度大于8则传输8个，切总长度减8
        { 
			len = CH375_EP0_SIZE;
			mVarSetupLength -= CH375_EP0_SIZE;
		}
									                 
	    CH375_WR_CMD_PORT(CMD_WR_USB_DATA3);				// 发出写端点0的命令
       	CH375_WR_DAT_PORT(len);								// 写入长度
    	for(i = 0; i < len; i++)
            CH375_WR_DAT_PORT(request.buffer[i]);			// 循环写入数据

		#ifdef	USB_DEBUG
		usb_debug_print_hex(&request.buffer[0], len);
		#endif

		Delay5ms();
    }
	else
    {
		CH375_WR_CMD_PORT(CMD_WR_USB_DATA3);			// 发出写端点0的命令
		CH375_WR_DAT_PORT(0);							// 上传0长度数据，这是一个状态阶段

		#ifdef	USB_DEBUG
		Uart0_Tx("ZLP\n\r", 5);
		#endif
		Delay5ms();
	}
}

/* 复制描述符以便上传 */
void mCh375DesUp(void)
{
	U8_T k; 
	for(k = 0; k < CH375_EP0_SIZE; k++) 
    {
         request.buffer[k] = *VarSetupDescr;  			  // 依次复制8个描述符
         VarSetupDescr++;
    }
}

void mCH375Interrupt_device(void) 
{
	U8_T InterruptStatus;
	U8_T length, c1, len;

	CH375_WR_CMD_PORT(CMD_GET_STATUS);								// 获取中断状态并取消中断请求 
	InterruptStatus = CH375_RD_DAT_PORT();  						// 获取中断状态
	switch(InterruptStatus)   										// 解析中断源
    {
		case USB_INT_EP2_OUT: 
			#ifdef	USB_DEBUG
			Uart0_Tx("EP2_OUT:", 9);
			#endif 
			CH375_WR_CMD_PORT(CMD_RD_USB_DATA);						// 发出读数据命令
			if(length = CH375_RD_DAT_PORT())						// 首先读出的是长度
			{
				for(len = 0; len < length; len++)
				{
					DownBuf[DownCtr++] = CH375_RD_DAT_PORT();
				}
				#ifdef	USB_DEBUG
				usb_debug_print_hex(DownBuf + DownCtr - length, length);
				#endif
				USB_timeout = USB_TIMEOUT;
			}
			break;
		case USB_INT_EP2_IN:
			#ifdef	USB_DEBUG
			Uart0_Tx("EP2_IN\n\r", 8);
			#endif

			ENDP2_UP_SUC_FLAG = 1;									// 置本次上传成功标志 
			CH375_WR_CMD_PORT(CMD_UNLOCK_USB);						// 释放缓冲区
			break;
		case USB_INT_EP1_IN: 									// 中断端点上传成功，未处理
			#ifdef	USB_DEBUG
			Uart0_Tx("EP1_IN\n\r", 8);
			#endif
			CH375_WR_CMD_PORT(CMD_UNLOCK_USB);						// 释放缓冲区
			break;
		case USB_INT_EP1_OUT:	  									// 中断端点下传成功，未处理
			#ifdef	USB_DEBUG
			Uart0_Tx("EP1_OUT\n\r", 9);
			#endif
			CH375_WR_CMD_PORT(CMD_RD_USB_DATA);						// 发出读数据命令
			if(length = CH375_RD_DAT_PORT())						// 首先读出的是长度
			{
				for(len = 0; len < length; len++)
					CH375_RD_DAT_PORT();
			}
			break;
		case USB_INT_EP0_SETUP:	
											// 控制端点建立成功
	    	CH375_WR_CMD_PORT(CMD_RD_USB_DATA);						// 读取数据缓冲器
			length = CH375_RD_DAT_PORT();	
									// 获得数据长度
			for(len = 0; len < length; len++)
            {            
                request.buffer[len] = CH375_RD_DAT_PORT();			// 取出输出端点0的数据包
            }

			#ifdef	USB_DEBUG
			Uart0_Tx("USB_SETUP->Request: ", 20);
			usb_debug_print_hex(&request.buffer[0], length);
			#endif
			if(length == 0x08)	// request
            {	 
			    mVarSetupLength = (request.buffer[7] << 8) | request.buffer[6];	// 控制传输请求的数据长度

				if((c1 = request.r.bmReuestType) & 0x40)         		        // 厂商请求,未进行处理
                {
                	// NO DEAL..............
					#ifdef	USB_DEBUG
					Uart0_Tx("->Vendor request\n\r", 18);
					#endif
				}

				if((c1 = request.r.bmReuestType) & 0x20)					// 类请求,进行相应的处理
                {
					#ifdef	USB_DEBUG
					Uart0_Tx("->Class request\n\r", 17);
					#endif
					mVarSetupRequest = request.r.bRequest;	
									// 暂存类请求码 
                    switch(mVarSetupRequest)                                // 分析类请求码,并进行处理
                    {
						case SET_LINE_CODING:                               // SET_LINE_CODING
							#ifdef	USB_DEBUG
							Uart0_Tx("[COM] SET_LINE_CODING\n\r", 23);
							#endif                   
                            SET_LINE_CODING_Flag = 1; 			            // 置SET_LINE_CODING命令标志				
                            mVarSetupLength = 0;
							break;
						case GET_LINE_CODING:                               // GET_LINE_CODING
							#ifdef	USB_DEBUG
							Uart0_Tx("[COM] GET_LINE_CODING\n\r", 23);
							#endif
                            for(c1 = 0; c1 < 7; c1++)
                            {   
								request.buffer[c1] = uartpara.uart_para_buf[c1];
                            }
							mVarSetupLength = 7;
							break;
                        case SET_CONTROL_LINE_STATE:                        // SET_CONTROL_LINE_STATE 
							#ifdef	USB_DEBUG
							Uart0_Tx("[COM] SET_CONTROL_LINE_STATE\n\r", 30);
							#endif
							mVarSetupLength = 0;
							break;
                        default:
                            CH375FLAGERR = 1;								    // 不支持的类命令码
							break;
                    } 
				}
				else if(!((c1 = request.r.bmReuestType) & 0x60)) 	         	// 标准请求,进行相应的处理
                {
					#ifdef	USB_DEBUG
					Uart0_Tx("->Standard request", 18);
					#endif

					mVarSetupRequest = request.r.bRequest;	
					switch(request.r.bRequest)                              	// 分析标准请求
                    {
						case DEF_USB_CLR_FEATURE:								// 清除特性
							#ifdef	USB_DEBUG
							Uart0_Tx("->Clear feature\n\r", 17);
							#endif
							if((c1 = request.r.bmReuestType & 0x1F) == 0X02)	// 不是端点不支持
                            {
								switch(request.buffer[4])	//wIndex
                                {
									case 0x82:
										CH375_WR_CMD_PORT(CMD_SET_ENDP7);		// 清除端点2上传
										CH375_WR_DAT_PORT(0x8E);            	// 发命令清除端点
										break;
									case 0x02:
										CH375_WR_CMD_PORT(CMD_SET_ENDP6);
										CH375_WR_DAT_PORT(0x80);				// 清除端点2下传
										break;
									case 0x81:
										CH375_WR_CMD_PORT(CMD_SET_ENDP5);		// 清除端点1上传
										CH375_WR_DAT_PORT(0x8E);
										break;
									case 0x01:
										CH375_WR_CMD_PORT(CMD_SET_ENDP4);		// 清除端点1下传
										CH375_WR_DAT_PORT(0x80);
										break;
									default:
										break;
								}
							}
							else
                            {
								CH375FLAGERR = 1;								// 不支持的清除特性,置错误标志
							}
							break;
						case DEF_USB_GET_STATUS:								// 获得状态
							#ifdef	USB_DEBUG
							Uart0_Tx("->Get status\n\r", 14);
							#endif
							request.buffer[0] = 0;								// 上传两个字节的状态
							request.buffer[1] = 0;
							mVarSetupLength = 2;
							break;
						case DEF_USB_SET_ADDRESS:								// 设置地址
							#ifdef	USB_DEBUG
							Uart0_Tx("->Set address\n\r", 15);
							#endif
							VarUsbAddress = request.buffer[2];					// 暂存USB主机发来的地址
							break;
						case DEF_USB_GET_DESCR: 
							Test[6]++;								// 获得描述符
							if(request.buffer[3] == 1)							// 设备描述符上传
							{
								VarSetupDescr = DevDes;
								if(mVarSetupLength > DevDes[0])
									mVarSetupLength = DevDes[0];				// 如果要求长度大于实际长度,则取实际长度
							}
							else if(request.buffer[3] == 2)		 				// 配置描述符上传
                            {
								VarSetupDescr = ConDes;
                                if(mVarSetupLength >= 0x43)
									mVarSetupLength = 0x43;						// 如果要求长度大于实际长度,则取实际长度
                            }
 							else if(request.buffer[3] == 3)						// 获得字符串描述符
                            {
								#ifdef	USB_DEBUG
								Uart0_Tx("->string", 8);
								#endif
								switch(request.buffer[2])
								{
									case 0:										// 获取语言ID
										#ifdef	USB_DEBUG
										Uart0_Tx("->language\n\r", 12);
										#endif
										VarSetupDescr = LangDes;
										if(mVarSetupLength > LangDes[0])
											mVarSetupLength = LangDes[0];		// 如果要求长度大于实际长度,则取实际长度
										break;
									case 1:										// 获取厂商字符串
										#ifdef	USB_DEBUG
										Uart0_Tx("->manufacturer\n\r", 16);
										#endif
										VarSetupDescr = MANUFACTURER_Des;
										if(mVarSetupLength > MANUFACTURER_Des[0])
											mVarSetupLength = MANUFACTURER_Des[0];// 如果要求长度大于实际长度,则取实际长度
										break;
									case 2:										// 获取产品字符串
										#ifdef	USB_DEBUG
										Uart0_Tx("->product\n\r", 11);
										#endif
										VarSetupDescr = PRODUCER_Des;
										if(mVarSetupLength > PRODUCER_Des[0])
											mVarSetupLength = PRODUCER_Des[0];	// 如果要求长度大于实际长度,则取实际长度
										break;
									case 3:										// 获取产品序列号
										#ifdef	USB_DEBUG
										Uart0_Tx("->serial number\n\r", 17);
										#endif
										VarSetupDescr = PRODUCER_SN_Des;
										if(mVarSetupLength > PRODUCER_SN_Des[0])
											mVarSetupLength = PRODUCER_SN_Des[0];			// 如果要求长度大于实际长度,则取实际长度
										break;
								}
							}

							mCh375DesUp();
							break;
						case DEF_USB_GET_CONFIG:								// 获得配置
// #if (DEBUG_UART1)
// 	uart_init_send_com(UART_SUB1);
//	sprintf(debug_str," ->Get config \r\n");
//	uart_send_string(debug_str,strlen(debug_str),UART_SUB1);
//#endif
							request.buffer[0] = 0;								// 没有配置则传0
							if(CH375CONFLAG) request.buffer[0] = 1;				// 已经配置则传1
							mVarSetupLength = 1;
							break;
						case DEF_USB_SET_CONFIG:                 		    	// 设置配置
// #if (DEBUG_UART1)
// 	uart_init_send_com(UART_SUB1);
//	sprintf(debug_str," ->Set config \r\n");
//	uart_send_string(debug_str,strlen(debug_str),UART_SUB1);
//#endif
							CH375CONFLAG = 0;
							if(request.buffer[2] != 0) CH375CONFLAG = 1;		// 设置配置标志
							break;
						case DEF_USB_GET_INTERF:								// 得到接口
//#if (DEBUG_UART1)
//	uart_init_send_com(UART_SUB1);
//	sprintf(debug_str," Get interface \r\n");
//	uart_send_string(debug_str,strlen(debug_str),UART_SUB1);
//#endif	
							request.buffer[0] = 1;								// 上传接口数，本事例只支持一个接口
							mVarSetupLength = 1;
							break;
						default:
							#ifdef	USB_DEBUG
							Uart0_Tx("->Other standard requests\n\r", 27);
							#endif
							CH375FLAGERR = 1;									// 不支持的标准请求
							break;
					}
				}
			}
			else                                                            	// 不支持的控制传输，不是8字节的控制传输
            {
				CH375FLAGERR = 1;
				#ifdef	USB_DEBUG
				Uart0_Tx("USB_SETUP ERROR\n\r", 17);
				#endif
			}

			if(!CH375FLAGERR)
			{
				mCh375Ep0Up();													// 没有错误,调用数据上传,若长度为0则上传为状态 3
			}
			else 
            {
				CH375_WR_CMD_PORT(CMD_SET_ENDP3);								// 有错误,则设置端点0为STALL，指示一个错误
				CH375_WR_DAT_PORT(0x0F);
				#ifdef	USB_DEBUG
				Uart0_Tx("USB STALL!\n\r", 12);
				#endif
			}
			break;
		case USB_INT_EP0_IN:
			#ifdef	USB_DEBUG
			Uart0_Tx("EP0_IN\n\r", 8);
			#endif								// 控制端点上传成功
			if(mVarSetupRequest == DEF_USB_SET_ADDRESS)		// 设置地址
            {	
				#ifdef	USB_DEBUG
				Uart0_Tx("EP0_IN\n\r", 8);
				#endif
				CH375_WR_CMD_PORT(CMD_SET_USB_ADDR);
				CH375_WR_DAT_PORT(VarUsbAddress);
//#if (DEBUG_UART1)
// 	uart_init_send_com(UART_SUB1);
//	sprintf(debug_str," Set address as\r\n");
//	uart_send_string(debug_str,strlen(debug_str),UART_SUB1);
//#endif

				#ifdef	USB_DEBUG
				Uart0_Tx("Set address as: ", 16);								// 设置USB地址,设置下次事务的USB地址
				usb_debug_print_hex(&VarUsbAddress, 1);
				#endif
			}
			else
			{
				#ifdef	USB_DEBUG
				Uart0_Tx("Other EP0 IN\n\r", 14);
				#endif
				mCh375DesUp();
				mCh375Ep0Up();
			}
			
			CH375_WR_CMD_PORT(CMD_UNLOCK_USB);									// 释放缓冲区
			CH375_WR_CMD_PORT(CMD_UNLOCK_USB);
			CH375_WR_CMD_PORT(CMD_UNLOCK_USB);
			CH375_WR_CMD_PORT(CMD_UNLOCK_USB);
			CH375_WR_CMD_PORT(CMD_UNLOCK_USB);
			CH375_WR_CMD_PORT(CMD_UNLOCK_USB);
			CH375_WR_CMD_PORT(CMD_UNLOCK_USB);
			CH375_WR_CMD_PORT(CMD_UNLOCK_USB);
			CH375_WR_CMD_PORT(CMD_UNLOCK_USB);
			CH375_WR_CMD_PORT(CMD_UNLOCK_USB);
			CH375_WR_CMD_PORT(CMD_UNLOCK_USB);
			CH375_WR_CMD_PORT(CMD_UNLOCK_USB);
			CH375_WR_CMD_PORT(CMD_UNLOCK_USB);

			break;
		case USB_INT_EP0_OUT:													// 控制端点下传成功
			#ifdef	USB_DEBUG
			Uart0_Tx("EP0_OUT\n\r", 9);
			#endif
			CH375_WR_CMD_PORT(CMD_RD_USB_DATA);									// 发出读数据命令
			length = CH375_RD_DAT_PORT();										// 首先读出的是长度
			if(length == 7)														// SET LINE CODING
			{
				for(len = 0; len < length; len++)
				{
					uartpara.uart_para_buf[len] = CH375_RD_DAT_PORT();
				}

				#ifdef	USB_DEBUG
				Uart0_Tx("[COM] SET_LINE_CODING\n\r", 23);
				usb_debug_print_hex(&uartpara.uart_para_buf[0], length);
				#endif

				SET_LINE_CODING_Flag = 1;
			}

			if(SET_LINE_CODING_Flag == 1)
            {
                SET_LINE_CODING_Flag = 0;									// 类命令SET_LINE_CODING标志清0
				//......
				for(c1 = 0; c1 < 20; c1++)
					Delay10us();

				mVarSetupLength = 0;
				mCh375Ep0Up();
            }
			break;
		default:
			if((InterruptStatus & 0x03) == 0x03)								// 总线复位
            {	
				CH375FLAGERR = 0;											    // 错误清0
				CH375CONFLAG = 0;											    // 配置清0
				mVarSetupLength = 0;
				#ifdef	USB_DEBUG
				Uart0_Tx("USB_RESET\n\r", 11);
				#endif
			}
			else
			{	
				#ifdef	USB_DEBUG
				Uart0_Tx("Other USB interrupt\n\r", 21);
				#endif
			}
			CH375_WR_CMD_PORT(CMD_UNLOCK_USB);										// 释放缓冲区
			CH375_RD_DAT_PORT();
			break;
	}
}


void USB_responseData(void)
{
	responseCmd(USB,DownBuf,NULL);
}
#endif


#if USB_HOST


U8_T far gsm_str[MAX_GSM_BUF];	 // send buf
U8_T flag_sms_start;
U16_T sms_len;
U8_T far usb_buf[512];			 // recv buf
U8_T GSM_Inital_Step;
U8_T usb_status;
U8_T send_len;
U8_T far apnstr[MAX_GSM_APN];
U8_T far ipstr[MAX_GSM_IP];
U8_T far apnlen;
U8_T far iplen;
U8_T far flag_ini_host;
U8_T far flag_sever_or_client;

U8_T flag_reinit_APN;
U8_T flag_clear_send_buf;
U8_T flag_receive_AT_CMD;
U8_T flag_response_AT_CMD;
U8_T flag_open_windows;
U8_T flag_close_windows;



enum{
	USB_WARTING_DEVICE = 0,
	USB_RESET,
 	USB_GET_DEVICE_DESC,
	USB_SET_ADDRESS,
	USB_GET_CONFIG_DESC,
	USB_SET_CONFIG,
	USB_INITIAL_GSM,
	USB_REC_MSM,
	USB_SEND_DATA,
	USB_RECV_DATA,
	USB_ERROR,
};

U8_T GSM_remote_IP[4];
U16_T GSM_remote_tcpport;
U16_T GSM_remote_tcpport1;
U8_T GSM_remote_tcp_link_id;

//char far GSM_APN[12];
//U8_T GSM_remote_udp_link_id;

#define  GSM_POWER_ON_OFF  P1_1


void USB_HOST_initial(void)
{
	U8_T re_init = 0;
	usb_status = USB_WARTING_DEVICE;
	endp_out_addr = 0x05;		  // Change the endpoint to which endpoint you want to send data
	endp_out_size = 0x200;
	endp_in_addr = 0x06;		  // Change the endpoint to which endpoint you want to receive data
	mDeviceOnline = 0;		  // Once a USB device pluging in, mDeviceOnline will be 1
	flag_ini_host = 0;

// reboot GSM moudle	
//	GSM_Power_Off();

host_init:
	re_init++;
	if(CH375Host_Init(5) == USB_INT_SUCCESS)
	{
		Test[40] = 30;
		flag_ini_host = 1;
	}
	else
	{
		Test[40] = 100;
		GSM_Power_On(); 
		if(re_init < 10)
			goto host_init;
	}
	
	if(re_init >= 10)
	{
		Test[40] = 50;
	}

	GSM_Inital_Step = IPINIT;
//		GSM_remote_IP[0] = 114;
//		GSM_remote_IP[1] = 93;
//		GSM_remote_IP[2] = 6;
//		GSM_remote_IP[3] = 112;
//		GSM_remote_tcpport = 34321;
	GSM_remote_tcpport1 = 11002;
	GSM_remote_tcp_link_id = 1;
	flag_sms_start = 0;
	sms_len = 0;
	flag_reinit_APN = 0;
	flag_clear_send_buf = 0;
	flag_receive_AT_CMD = 0;
	flag_open_windows = 0;
	flag_close_windows = 0;
	
	
#if (DEBUG_UART1)
	uart_init_send_com(UART_SUB1);
	sprintf(debug_str,"apn %d %s \r\n", (U16_T)apnlen ,apnstr);
	uart_send_string(debug_str,strlen(debug_str),UART_SUB1);
	
	uart_init_send_com(UART_SUB1);
	sprintf(debug_str,"ip %d %s \r\n", (U16_T)iplen ,ipstr);
	uart_send_string(debug_str,strlen(debug_str),UART_SUB1);
#endif



}


void GSM_Power_On(void)
{
	GSM_POWER_ON_OFF = 1;
	vTaskDelay(1000 / portTICK_RATE_MS);//DELAY_Ms(1000);
	GSM_POWER_ON_OFF = 0;
} 


void GSM_Power_Off(void)
{
	GSM_POWER_ON_OFF = 1;
	vTaskDelay(5000 / portTICK_RATE_MS);//DELAY_Ms(5000);
	GSM_POWER_ON_OFF = 0;
}




#if 0

void GSM_ipinit(U8_T port)
{
	if(port == 0)
		sprintf(gsm_str,"AT^IPINIT=\"3GNET\"\r\n");
	else
		sprintf(gsm_str,"AT^IPINIT=\"CMNET\"\r\n");

	USB_send_data(strlen(gsm_str),gsm_str);
//	wait_usb_response(20);
}


void GSM_ipopen(U8_T link_id,U8_T type,U32_T ip,U16_T remote_port,U16_T local_port)
{
	sprintf(gsm_str,"AT^IPOPEN=%u,\"TCP\",\"%u.%u.%u.%u\",%u,%u\r\n",(U16_T)GSM_remote_tcp_link_id,(U16_T)GSM_remote_IP[0],(U16_T)GSM_remote_IP[1],(U16_T)GSM_remote_IP[2],(U16_T)GSM_remote_IP[3],(U16_T)GSM_remote_tcpport,(U16_T)GSM_remote_tcpport1);
	USB_send_data(strlen(gsm_str),gsm_str);
}


void GSM_Test(void)
{

	sprintf(gsm_str,"AT\r\n");
	USB_send_data(strlen(gsm_str),gsm_str);
//	wait_usb_response(5);


}


void GSM_SET_TRANSFER(U8_T port)
{
	sprintf(gsm_str,"AT^IPENTRANS=%d\r\n",(U16_T)GSM_remote_tcp_link_id);
	USB_send_data(strlen(gsm_str),gsm_str);
}



void GSM_IPSEND(U8_T port)
{
	sprintf(gsm_str,"AT^IPSEND=%u,\"TCP SEND TEST\"\r\n",(U16_T)GSM_remote_tcp_link_id);
	USB_send_data(strlen(gsm_str),gsm_str);
}

void GSM_END_TRANSER(void)
{
	sprintf(gsm_str,"+++");
	USB_send_data(strlen(gsm_str),gsm_str);
//	wait_usb_response(5);
}
#endif




void USB_Send_GSM(U8_T * str,U16_T len)
{
	U16_T gsmlen;
//	U16_T datalen;
	U8_T seg;
	U16_T i;

	seg = 0;	

	gsmlen = 16;	
	memcpy(&gsm_str[0],"AT^IPSENDEX=",12);
	gsm_str[12] = '0' + Test[49];//GSM_remote_tcp_link_id;
	gsm_str[13] = ',';
	gsm_str[14] = '1';
	gsm_str[15] = ',';
	gsm_str[gsmlen++] = '\"';

// trsnfer str to asic
	Test[48] = len;
	for(i = 0;i < len;i++)
	{
		if(str[i] / 16 < 10)
			gsm_str[gsmlen++] = '0' + str[i] / 16;
		else
			gsm_str[gsmlen++] = 'A' + str[i] / 16 - 10;
	
		if(gsmlen == 64)  // 64 = CH375 WRITE BUF LEN 
		{
			USB_send_data(gsmlen,gsm_str);
			gsmlen = 0;
		}

		if(str[i] % 16 < 10)
			gsm_str[gsmlen++] = '0' + str[i] % 16;
		else
			gsm_str[gsmlen++] = 'A' + str[i] % 16 - 10;

		if(gsmlen == 64)
		{
			USB_send_data(gsmlen,gsm_str);
			gsmlen = 0;
		}
	}
	gsm_str[gsmlen++] = '\"';
	if(gsmlen == 64)
	{
		USB_send_data(gsmlen,gsm_str);
		gsmlen = 0;
	}
	gsm_str[gsmlen++] = '\r';
	if(gsmlen == 64)
	{
		USB_send_data(gsmlen,gsm_str);
		gsmlen = 0;
	}
	gsm_str[gsmlen++] = '\n';		
	USB_send_data(gsmlen,gsm_str); 
}




U8_T GSM_Initial(U8_T start_step)
{
	
	U8_T retry = 0;
	U8_T length;	


	if(start_step == IPINIT) goto IP_INIT;
	else
		if(start_step == IPOPEN) goto IP_OPEN_TCPLINK;	

IP_INIT:

	USB_send_data(apnlen,apnstr);
	retry = 50;			
	while(retry--)
	{	
		vTaskDelay(100 / portTICK_RATE_MS);
		memset(usb_buf,0, 512);
		length = USB_recv_data(usb_buf);
		if((length == 20) && (usb_buf[0] == 20) && (usb_buf[1] == 20))
		{
 // rec error
		}
		else
		if(length > 0)
		{	
							
			if((strncmp(usb_buf,"\r\n+CME ERROR: The network has been opened already\r\n",69) == 0)
			|| (strncmp(usb_buf,"\r\nOK\r\n",6) == 0)
			)  
			{
//#if (DEBUG_UART1)
//uart_init_send_com(UART_SUB1);
//	sprintf(debug_str,"IP INITIAL OK \r\n");
//	uart_send_string(debug_str,strlen(debug_str),UART_SUB1);
//#endif 
				break;
			}
			// +CME ERROR: SIM failure
			
		}

		if(retry == 1)
			return IPINIT;
	}																																			 
	
IP_OPEN_TCPLINK:


 	if(flag_sever_or_client == 0)
//	sprintf(gsm_str,"AT^IPOPEN=%u,\"TCP\",\"%d.%u.%u.%u\",%u,%u\r\n",(U16_T)GSM_remote_tcp_link_id,(U16_T)GSM_remote_IP[0],(U16_T)GSM_remote_IP[1],(U16_T)GSM_remote_IP[2],(U16_T)GSM_remote_IP[3],(U16_T)GSM_remote_tcpport,(U16_T)GSM_remote_tcpport1);
		USB_send_data(iplen,ipstr);		
	else
	{	
// if have static IP, gsm work as sever
//AT^IPLISTEN="UDP",47808
		sprintf(&gsm_str,"AT^IPLISTEN=\"UDP\",47808\r\n");
		USB_send_data(strlen(gsm_str),gsm_str);	
	}
			
	retry = 5;			
	while(retry--)
	{
		vTaskDelay(100 / portTICK_RATE_MS);
		memset(usb_buf,0, 512);
		length = USB_recv_data(usb_buf);
		if((length == 20) && (usb_buf[0] == 20) && (usb_buf[1] == 20))
		{
 // rec error
		}
		else
		if(length > 0)
		{
			if(flag_sever_or_client == 0)
			{
				if((strncmp(usb_buf,"\r\n+CME ERROR: The link has been established already\r\n",53) == 0)
				||(strncmp(usb_buf,"\r\nOK\r\n",6) == 0)
				)  
				{  
					return GSM_INIT_OK;
				}
			}
			else
			{
				if((strncmp(usb_buf,"\r\n+CME ERROR: The server has been established already\r\n",50) == 0)
				|| (strncmp(usb_buf,"\r\nOK\r\n",6) == 0)
				)
				{  
					return GSM_INIT_OK;
				}
			}
			
		}
		
		
		if(retry == 1)
			return IPOPEN;
   	}
	return 0;//start_step;

}



void Get_Remote_TCP_PORT(char* poststr)
{
	U8_T dotpos[6];
	U8_T i;
	U8_T j = 0;
	U8_T start;

	for(i = 0;i < strlen(poststr);i++)
	{
		if((poststr[i] == '.') || (poststr[i] == ':') || (poststr[i] == '"'))
		{
			dotpos[j++] = i;
		}	
	}
	for(j = 0; j < 5;j++)
	{		
		start = dotpos[j] + 1;
		if(j < 4)
		{
			if(dotpos[j + 1] - dotpos[j] == 4)
			{
				GSM_remote_IP[j] = (poststr[start] - '0') * 100 + (poststr[start + 1] - '0') * 10 + poststr[start + 2] - '0';	
			}
			else if(dotpos[j + 1] - dotpos[j] == 3)
			{
				GSM_remote_IP[j] = (poststr[start] - '0') * 10 + (poststr[start + 1] - '0');	
			}
			else if(dotpos[j + 1] - dotpos[j] == 2)
			{
				GSM_remote_IP[j] = (poststr[start] - '0'); 	
			}
			
			E2prom_Write_Byte(EEP_GSM_IP1 + j, GSM_remote_IP[j]);
		}
		else  // j == 4
		{
			if(dotpos[j + 1] - dotpos[j] == 6)
			{
				GSM_remote_tcpport = (poststr[start] - '0') * 10000 + (poststr[start + 1] - '0') * 1000 
							+ (poststr[start + 2] - '0') * 100 + (poststr[start + 3] - '0') * 10 
							+ (poststr[start + 4] - '0');		
			}
			else if(dotpos[j + 1] - dotpos[j] == 5)
			{
				GSM_remote_tcpport = (poststr[start] - '0') * 1000 + (poststr[start + 1] - '0') * 100 
							+ (poststr[start + 2] - '0') * 10 + poststr[start + 3] - '0';		
			}
			else if(dotpos[j + 1] - dotpos[j] == 4)
			{
				GSM_remote_tcpport = (poststr[start] - '0') * 100 + (poststr[start + 1] - '0') * 10 + poststr[start + 2] - '0';		
			}
			else if(dotpos[j + 1] - dotpos[j] == 3)
			{
				GSM_remote_tcpport = (poststr[start] - '0') * 10 + poststr[start + 1] - '0';		
			}
			else if(dotpos[j + 1] - dotpos[j] == 2)
			{
				GSM_remote_tcpport = poststr[start] - '0';		
			}
		}
//		E2prom_Write_Byte(EEP_GSM_TCP_PORT_LO,(U8_T)GSM_remote_tcpport);
//		E2prom_Write_Byte(EEP_GSM_TCP_PORT_HI,(U8_T)(GSM_remote_tcpport >> 8));
	}
	
//   // GET APN
//   if(dotpos[7] - dotpos[6] - 1 <= 12)
//   {
//	   for(i = 0;i < dotpos[7] - dotpos[6] - 1;i++)
//	   {
//	 		GSM_APN[i] = poststr[dotpos[6] + 1];
//			E2prom_Write_Byte(EEP_APN_FIRST + i,GSM_APN[i]);
//	   }
//   }
}

U8_T GSM_Receive_SMS(U8_T start_step)
{
	U8_T retry;
	U16_T length;

	if(start_step == CMGF) goto SET_CMGF;
	else
		if(start_step == CMGL) goto LIST_SMS;	

SET_CMGF:	
	// check SM
//	Test[15]++;
	sprintf(gsm_str,"AT+CMGF=1\r\n");	// SET TEXT Format
	USB_send_data(strlen(gsm_str),gsm_str);
	retry = 5;			
	while(retry--)
	{	
		vTaskDelay(200 / portTICK_RATE_MS);
		memset(usb_buf,0, 512);
		length = USB_recv_data(usb_buf);
		if((length == 20) && (usb_buf[0] == 20) && (usb_buf[1] == 20))
		{
 // rec error
		}
		else
		if(length > 0)
		{	
		    Test[16]++;
			if(strncmp(usb_buf,"\r\nOK\r\n",6) == 0)			  
			{
//	#if (DEBUG_UART1)
//	sprintf(debug_str,"GMGF OK \r\n");
//	uart_send_string(debug_str,strlen(debug_str),UART_SUB1);
//	#endif				

				break;
			}
		}

		if(retry == 1)	// ERROR
			return CMGF;		

	}	 

LIST_SMS:			   
	sprintf(gsm_str,"AT+CMGL=\"REC UNREAD\"\r\n");	// list unread
	USB_send_data(strlen(gsm_str),gsm_str);
	retry = 50;			
	while(retry--)
	{	
		vTaskDelay(100 / portTICK_RATE_MS);
		memset(usb_buf,0, 512);
		length = USB_recv_data(usb_buf);
		if((length == 20) && (usb_buf[0] == 20) && (usb_buf[1] == 20))
		{
 // rec error
		}
		else
		if(length > 0)
		{

			char *poststr = NULL;

		    if(flag_sms_start == 1)
			{
				flag_sms_start = 0;
				if((sms_len + length) < MAX_GSM_BUF)
				{	
					memcpy(&gsm_str[sms_len],usb_buf,length);
					Get_Remote_TCP_PORT(gsm_str);
					sprintf(gsm_str,"AT^IPOPEN=%u,\"TCP\",\"%d.%u.%u.%u\",%u,%u\r\n",(U16_T)GSM_remote_tcp_link_id,(U16_T)GSM_remote_IP[0],(U16_T)GSM_remote_IP[1],(U16_T)GSM_remote_IP[2],(U16_T)GSM_remote_IP[3],(U16_T)GSM_remote_tcpport,(U16_T)GSM_remote_tcpport1);
					memcpy(ipstr,gsm_str,strlen(gsm_str));
//#if (DEBUG_UART1)
//	uart_init_send_com(UART_SUB1);
//	sprintf(debug_str,"ip1 %s \r\n",ipstr);
//	uart_send_string(debug_str,strlen(debug_str),UART_SUB1);
//#endif 	

					return GET_SMS_OK;		
				}
			}
			if((poststr = strstr(usb_buf,"gsmport:")) != 0) 
			{

				flag_sms_start = 1;									 
				memcpy(&gsm_str[0],poststr,strlen(poststr));
				sms_len = strlen(poststr);
				Test[6] = sms_len;
			}
			if(strncmp(usb_buf,"\r\nOK\r\n",6) == 0)			  
			{
				if(flag_sms_start == 0)
				{
//#if (DEBUG_UART1)
//	uart_init_send_com(UART_SUB1);
//	sprintf(debug_str," no new msm \r\n",usb_buf);
//	uart_send_string(debug_str,strlen(debug_str),UART_SUB1);
//#endif 			 
					return NO_NEW_MSM;
				}
				else
				{
//#if (DEBUG_UART1)
//	uart_init_send_com(UART_SUB1);
//	sprintf(debug_str," get new msm \r\n",usb_buf);
//	uart_send_string(debug_str,strlen(debug_str),UART_SUB1);
//#endif 	
					return GET_SMS_OK;
				}	
			}
			 					
		}

		if(retry == 1)	// ERROR
			return CMGL;
	}

   	return 0;
}
							   
#endif

void USB_task(void)
{
	U8_T len, length;
//	U16_T StartAdd;
	portTickType xDelayPeriod = (portTickType)20 / portTICK_RATE_MS;

	U16_T GSM_Intial_count = 0;
	U16_T usb_retry = 0;
//	
	U16_T GSM_disconnect_timer = 0;
	U8_T temp_sub[100];

#if USB_DEVICE
//	usb_mode = 1;
	if(Modbus.usb_mode == 0)
	{
		USB_device_Initial();
	}
#endif

#if USB_HOST
//	usb_mode = 0;
	if(Modbus.usb_mode == 1)
	{ 	
		USB_HOST_initial();
	}

#endif
	while(1)
	{   
		vTaskDelay(xDelayPeriod);
#if USB_HOST
		if(Modbus.usb_mode == 1)
		{			
			//if(usb_status >= USB_INITIAL_GSM)
			//{
				if(flag_open_windows)
				{ 	
					Test[35]++;				
					if(flag_clear_send_buf == 1)
					{
						Test[20]++;
						memset(usb_buf,0, 512);
						memset(temp_sub, 0, 100);  
						length = 0;
						send_len = 0; 
						flag_clear_send_buf = 0;
					}
					else if(flag_receive_AT_CMD == 1)
					{
						Test[21]++;
						if(USB_send_data(strlen(&gsm_str[1]),&gsm_str[1]) == 1)
						{
							if((strstr(&gsm_str[1],"AT^IPINIT=")) != 0)
							{
								Test[41]++;
								Test[42] = strlen(&gsm_str[1]); 
								apnlen = strlen(&gsm_str[1]);
								memcpy(apnstr,&gsm_str[1],apnlen);	// AT^IPINIT="3GNET" 
								flag_reinit_APN = 1;
							}
							if((strstr(&gsm_str[1],"AT^IPOPEN=")) != 0)	// AT^IPOPEN=2,"TCP","114.93.6.170",31234,12340
							if((strstr(&gsm_str[1],"\"TCP\",")) != 0)
							{
								Test[40]++;
								Test[42] = strlen(&gsm_str[1]); 
								iplen = strlen(&gsm_str[1]);
								memcpy(ipstr,&gsm_str[1],iplen);	 
								flag_reinit_APN = 1;

							}
#if (DEBUG_UART1)
	uart_init_send_com(UART_SUB1);
	sprintf(debug_str,"send %s \r\n",&gsm_str[1]);
	uart_send_string(debug_str,strlen(debug_str),UART_SUB1);
#endif 	
							memset(usb_buf,0, 512);
							memset(temp_sub, 0, 100);  
							length = 0;
							send_len = 0;
	
							flag_receive_AT_CMD = 0;
						}	
						else
						{
							vTaskDelay(100 / portTICK_RATE_MS);	
						}
						
					// stroe important info, APN  							
							
					}
					else
					{
				//		Test[5]++;
						// if usb intial fail, note customer to inital it again
						if(flag_ini_host == 0)  
						{
							memcpy(usb_buf," -> USB INITIAL HOST FAIL,PLEASE CHECK GSM MOULDE\r\n", 50);
//							flag_ini_host = 2;
						}
						else
						{
							memset(temp_sub, 0, 100);  
							length = 0;
							vTaskDelay(500 / portTICK_RATE_MS);					
							length = USB_recv_data(temp_sub);			    
							if((length == 20) && (temp_sub[0] == 20) && (temp_sub[1] == 20))
							{
					 // rec error
							}
							else
							if(length > 0) 
							{
								Test[35] = send_len;			
	#if (DEBUG_UART1)
		uart_init_send_com(UART_SUB1);
		sprintf(debug_str,"rec %s \r\n",temp_sub);
		uart_send_string(debug_str,strlen(debug_str),UART_SUB1);
	#endif 	
								if(send_len > 100)
								{ 
									send_len = 0;
									length = 0;
								}
								else
									memcpy(&usb_buf[send_len],temp_sub, length);
									
								send_len += length;	
					 	
							}
							else  if(length != 20)
							{
							}
						}						
	
					}	
				}
				else if(flag_close_windows == 1)
				{
					Test[36]++;
				//  check whether reinit it 
					if( flag_reinit_APN == 1 && usb_status >= USB_INITIAL_GSM)  // t3000 reinitial APN				
						usb_status = USB_INITIAL_GSM;
					flag_close_windows = 0;
					flag_open_windows = 0;
//					if(flag_ini_host == 2)
//						flag_ini_host = 0;	
				}
			//}
			else  //flag_open_windows = 0  &&  flag_close_windows == 0
			{ 
				//Test[37]++;
				//Test[47] = usb_status;
				switch(usb_status)
				{
					case USB_WARTING_DEVICE:						
						if(mDeviceOnline == 1)
						{	Test[31]++;
							usb_status = USB_RESET;
						}
						else
						{	
							if(usb_poll() == TRUE)
							{	Test[33]++;
								mCH375Interrupt_host();
							}
							else
							{  	Test[34]++;
								usb_retry++;
								if(usb_retry > 1000)
								{  
									Test[35]++;
//									USB_HOST_initial();
									usb_retry = 0;
									Modbus.usb_mode = 0;
								}
							}								
						}
						break;
					case USB_RESET:
					//	Delay50ms();
						mDeviceOnline = 0;
						CH375Host_Init(7);
						CH375Host_Init(6);
						usb_status = USB_GET_DEVICE_DESC;	
						break;
					case USB_GET_DEVICE_DESC:						
					//	Delay50ms();					
						len = mReadCH375Data( usb_buf );  /* 读取设备描述符数据 */
						usb_status = USB_SET_ADDRESS;
						break;
					case USB_SET_ADDRESS:
					//	Delay50ms();
						mCtrlSetAddress(5);
						usb_status = USB_GET_CONFIG_DESC;	
						break;
					case USB_GET_CONFIG_DESC:
					//	Delay50ms();
						mCtrlGetDescr(2);	/* 获取配置描述符 */
						length = mReadCH375Data( usb_buf );  /* 读取配置描述符数据 */
						usb_status = USB_SET_CONFIG;
						break;
					case USB_SET_CONFIG:
					//	Delay50ms();						
						mCtrlSetConfig(1);
						usb_status = USB_INITIAL_GSM;
						GSM_Intial_count = 0;
						GSM_Inital_Step = IPINIT;
						break;		 
	
					case USB_INITIAL_GSM:
						//Test[4]++;
		 				GSM_Intial_count++;
						GSM_Inital_Step = GSM_Initial(GSM_Inital_Step);
						if(GSM_Inital_Step == GSM_INIT_OK)
						{
							//Test[5]++;
							usb_status = USB_RECV_DATA;
						}
						if((GSM_Intial_count > 10) && (GSM_Inital_Step == IPOPEN))  // INITAIAL FIAL
						{
							if(flag_sever_or_client == 0)
							{  // client
								usb_status = USB_REC_MSM;
								GSM_Inital_Step = CMGF;
							}
							else
							{  // for sever
								
							}
						} 						
						break;
					case USB_REC_MSM:
						GSM_Inital_Step = GSM_Receive_SMS(GSM_Inital_Step);
						if((GSM_Inital_Step == GET_SMS_OK) || (GSM_Inital_Step == NO_NEW_MSM))
						{
							usb_status = USB_INITIAL_GSM;
							GSM_Intial_count = 0;
							GSM_Inital_Step =  IPINIT;//IPOPEN;	 ?????????
						}
						break;
					case USB_RECV_DATA:
					{
						GSM_Inital_Step = RECV_DATA_FROM_T3000;
						vTaskDelay(50 / portTICK_RATE_MS);
						GSM_disconnect_timer++;
						Test[9] = GSM_disconnect_timer;
						if(GSM_disconnect_timer > 300)	   // 1min
						{
							Test[8]++;
							GSM_disconnect_timer = 0;
							usb_status = USB_INITIAL_GSM;
							GSM_Intial_count = 0;
							GSM_Inital_Step = IPINIT;
						}
						memset(usb_buf,0, 512);
						length = 0;
						length = USB_recv_data(usb_buf);
						if((length == 20) && (usb_buf[0] == 20) && (usb_buf[1] == 20))
						{
				 // rec error
						}
						else
						if(length > 0)
						{		
							Test[45]++;	
							if(strncmp(usb_buf,"\r\n^IPSTATE: 1,0,0",16) == 0)   // linkid is 1,the link is closed 
							{
								GSM_disconnect_timer = 0;
								usb_status = USB_INITIAL_GSM;
								GSM_Intial_count = 0;
								GSM_Inital_Step = IPINIT;
								Test[46]++;	
							}
							else if(strncmp(usb_buf,"\r\n^IPDATA:",10) == 0)  
							{
								uint16_t pdu_len = 0;  
								BACNET_ADDRESS far src; /* source address */
								
								Test[10] = usb_buf[10];	
								Test[11] = usb_buf[11];
								GSM_disconnect_timer = 0;
								length = (usb_buf[13] - '0') * 10 + (usb_buf[14] - '0'); 
								Test[12] = length; 
								bip_Data = &usb_buf[16];
								memcpy(bip_Data,&usb_buf[16],length);
								bip_len = length;
			
//								if(Modbus.protocal == 0)
//								{
//									Modbus.protocal = BAC_GSM;
									pdu_len = datalink_receive(&src, &usb_buf[16], length, 0,BAC_GSM);
								    { 
										
									    if(pdu_len) 
										{

								            npdu_handler(&src, &usb_buf[16], pdu_len,BAC_GSM);	
								        }
										
									}
//									Modbus.protocal = 0;
//								}					 
							}						
								
			   			}
					
					}
						break;
					default:
						break;
				}
			}
		}
#endif

#if USB_DEVICE
		if(Modbus.usb_mode == 0) // DEVICE
		{
			if(usb_poll() == TRUE)
			{
				mCH375Interrupt_device();
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
							USB_responseData();
	
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
#endif
	}
}


#endif






