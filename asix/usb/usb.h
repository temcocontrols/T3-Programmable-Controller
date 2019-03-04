#ifndef USB_H
#define USB_H

#include 	"types.h"

#define	USB_TIMEOUT		50




extern U8_T UpIndex;
extern U8_T far DownBuf[300];
extern U8_T far UpBuf[300];
extern U8_T far USB_timeout;
extern U16_T far DownCtr;
extern U16_T xdata UpCtr;

extern bit CH375FLAGERR;							// 错误标志
extern bit CH375CONFLAG;							// 设备是否已经配置标志
extern bit ENDP2_NEED_UP_FLAG ;					// 端点2有数据需要上传标志
extern bit ENDP2_UP_SUC_FLAG;					// 端点2本次数据上传成功标志
extern bit SET_LINE_CODING_Flag;


unsigned char mCH375Interrupt_host(void);
void mCH375Interrupt_device(void);


#define MAX_GSM_BUF 600


#define IPINIT  	2
#define IPOPEN		3
#define GSM_INIT_OK	 4
#define CMGF  	5
#define CMGL  	6
#define GET_SMS_OK	7
#define NO_NEW_MSM	8
#define RECV_DATA_FROM_T3000 9
#define UDP_SEVER 10

#define MAX_GSM_APN 100
#define MAX_GSM_IP 100


extern U8_T GSM_Inital_Step;


extern U8_T flag_reinit_APN;
extern U8_T flag_clear_send_buf;
extern U8_T flag_receive_AT_CMD;
extern U8_T flag_response_AT_CMD;
extern U8_T flag_open_windows;
extern U8_T flag_close_windows;
extern U8_T far gsm_str[MAX_GSM_BUF];	 // send buf
extern U8_T far usb_buf[512];			 // recv buf
extern U8_T far apnstr[MAX_GSM_APN];
extern U8_T far ipstr[MAX_GSM_IP];
extern U8_T far apnlen;
extern U8_T far iplen;
extern U8_T far flag_sever_or_client;

void USB_task(void);
void USB_device_Initial();
void USB_HOST_initial();
void GSM_Power_On(void);
void GSM_Power_Off(void);



#endif