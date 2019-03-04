#include "ch375usb.h"

unsigned char endp_out_addr = 0x05;		  // Change the endpoint to which endpoint you want to send data
unsigned short endp_out_size = 0x200;
unsigned char endp_in_addr = 0x06;		  // Change the endpoint to which endpoint you want to receive data
unsigned char mDeviceOnline = 0;		  // Once a USB device pluging in, mDeviceOnline will be 1


static BOOL1 tog_send, tog_recv;

extern unsigned int far Test[50];

static void toggle_recv( BOOL1 tog );
static void toggle_send( BOOL1 tog );

unsigned char usb_poll(void)
{
  if(CH375_INT_WIRE == 0)
	{
     return TRUE;
	}
  else
	{
     return FALSE;
	}
}


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

/* 延时50ms， 不精确 */
static void Delay50ms(void)
{
	unsigned char i, j;
	for(i = 0; i < 200; i++)
		for(j = 0; j < 250; j++)
			DELAY_1_US();
}

///* CH375写命令端口 */
//void CH375_WR_CMD_PORT(unsigned char cmd)  					// 向CH375的命令端口写入命令,周期不小于4uS
//{
//	Delay10us();
//    CH375_CMD_PORT = cmd;
//	Delay10us();
//	Delay10us();
//}
//
///* CH375写数据端口 */
//void CH375_WR_DAT_PORT(unsigned char dat) 
//{
//	Delay10us();
//	CH375_DAT_PORT = dat;
//	Delay10us();
//	Delay10us();
//}
//
///* CH375读数据端口 */
//unsigned char CH375_RD_DAT_PORT(void)
//{
//	unsigned char ret;
//	Delay10us();
//	ret = CH375_DAT_PORT;
//	return ret;
//}

/* 从CH375的端点缓冲区读取接收到的数据 */
unsigned char	mReadCH375Data( unsigned char *buf )
{
	unsigned char len, i;
	unsigned char *p;
	CH375_WR_CMD_PORT( CMD_RD_USB_DATA );  /* 从CH375的端点缓冲区读取数据块 */
	p = buf;
	len = CH375_RD_DAT_PORT();  /* 数据长度 */
	for ( i=0; i<len; i++ ) *p++ = CH375_RD_DAT_PORT( );  /* 连续读取数据 */
	return( len );
}

/* 向CH375的端点缓冲区写入准备发送的数据 */
void	mWriteCH375Data( unsigned char *buf, unsigned char len )
{
	unsigned char i;
	unsigned char *p;
	CH375_WR_CMD_PORT( CMD_WR_USB_DATA7 );  /* 向CH375的端点缓冲区写入数据块 */
	p = buf;
	CH375_WR_DAT_PORT( len );  /* 数据长度 */
	for ( i=0; i<len; i++ ) CH375_WR_DAT_PORT( *p++ );  /* 连续写入数据 */
}


/* 对目标USB设备执行控制传输: 获取USB描述符 */
unsigned char mCtrlGetDescr( unsigned char type)
{
//	mIntStatus = 0;		/* 清中断状态 */
	CH375_WR_CMD_PORT( CMD_GET_DESCR);	 /* 控制传输-获取描述符 */
	CH375_WR_DAT_PORT( type);	  /* 0:设备描述符, 1:配置描述符 */
//	while( mIntStatus ==0);	   /* 等待操作完成 */
	return( mCH375Interrupt_host( ) );  /* 等待操作完成 */
}

/* 对目标USB设备执行控制传输: 设置USB地址 */
unsigned char	mCtrlSetAddress( unsigned char addr )
{
//	mIntStatus = 0;  /* 清中断状态 */
	unsigned char c;
	CH375_WR_CMD_PORT( CMD_SET_ADDRESS );  /* 控制传输-设置USB地址 */
	CH375_WR_DAT_PORT( addr );  /* 1 - 7eh */
//	while ( mIntStatus == 0 );  /* 等待操作完成 */
//	if ( mIntStatus != USB_INT_SUCCESS ) return;  /* 操作失败 */
	c = mCH375Interrupt_host();
	if( c!= USB_INT_SUCCESS)
	{
//		gsm_debug( "USB set address failed");
		return c;
	}
//	else
//		gsm_debug("USB SET ADDRESS SUCCESS");
/* 当目标USB设备的地址成功修改后,应该同步修改CH375的USB地址,否则CH375将无法与目标设备通讯 */
	CH375_WR_CMD_PORT( CMD_SET_USB_ADDR );  /* 设置CH375的USB地址 */
	CH375_WR_DAT_PORT( addr );  /* 修改CH375的USB设备能够立即完成,不会产生中断通知 */
	return c;
}

/* 对目标USB设备执行控制传输: 设置配置值 */
unsigned char	mCtrlSetConfig( unsigned char value )
{
//	mIntStatus = 0;  /* 清中断状态 */
	tog_send = tog_recv = 0;
	CH375_WR_CMD_PORT( CMD_SET_CONFIG );  /* 控制传输-设置USB配置 */
	CH375_WR_DAT_PORT( value );
	return( mCH375Interrupt_host( ) );
//	while ( mIntStatus == 0 );  /* 等待操作完成 */
}

unsigned char issue_token( unsigned char endp_and_pid ) {  /* 执行USB事务 */
/* 执行完成后, 将产生中断通知单片机, 如果是USB_INT_SUCCESS就说明操作成功 */
	CH375_WR_CMD_PORT( CMD_ISSUE_TOKEN );
	CH375_WR_DAT_PORT( endp_and_pid );  /* 高4位目的端点号, 低4位令牌PID */
	return( mCH375Interrupt_host() );  /* 等待CH375操作完成 */
}

/* 数据同步 */
/* USB的数据同步通过切换DATA0和DATA1实现: 在设备端, USB打印机可以自动切换;
   在主机端, 必须由SET_ENDP6和SET_ENDP7命令控制CH375切换DATA0与DATA1.
   主机端的程序处理方法是为设备端的各个端点分别提供一个全局变量,
   初始值均为DATA0, 每执行一次成功事务后取反, 每执行一次失败事务后将其复位为DATA1 */

static void toggle_recv( BOOL1 tog ) {  /* 主机接收同步控制:0=DATA0,1=DATA1 */
	CH375_WR_CMD_PORT( CMD_SET_ENDP6 );
	CH375_WR_DAT_PORT( tog ? 0xC0 : 0x80 );
	DELAY_1_US();DELAY_1_US();
}

static void toggle_send( BOOL1 tog ) {  /* 主机发送同步控制:0=DATA0,1=DATA1 */
	CH375_WR_CMD_PORT( CMD_SET_ENDP7 );
	CH375_WR_DAT_PORT( tog ? 0xC0 : 0x80 );
	DELAY_1_US();DELAY_1_US();
}

unsigned char clr_stall( unsigned char endp_addr ) {  /* USB通讯失败后,复位设备端的指定端点到DATA0 */
	CH375_WR_CMD_PORT( CMD_CLR_STALL );
	CH375_WR_DAT_PORT( endp_addr );
	return( mCH375Interrupt_host() );
}

/* 数据读写, 单片机读写CH375芯片中的数据缓冲区 */

unsigned char rd_usb_data( unsigned char *buf ) {  /* 从CH37X读出数据块 */
	unsigned char i, len;
	CH375_WR_CMD_PORT( CMD_RD_USB_DATA );  /* 从CH375的端点缓冲区读取接收到的数据 */
	len=CH375_RD_DAT_PORT();  /* 后续数据长度 */
	for ( i=0; i!=len; i++ ) *buf++=CH375_RD_DAT_PORT();
	return( len );
}

void wr_usb_data( unsigned char len, unsigned char *buf ) {  /* 向CH37X写入数据块 */
	CH375_WR_CMD_PORT( CMD_WR_USB_DATA7 );  /* 向CH375的端点缓冲区写入准备发送的数据 */
	CH375_WR_DAT_PORT( len );  /* 后续数据长度, len不能大于64 */
	while( len-- ) CH375_WR_DAT_PORT( *buf++ );
}
extern unsigned int far Test[50];
#define	USB_INT_RET_NAK		0x2A		/* 00101010B,返回NAK */
unsigned char USB_send_data(unsigned char len, unsigned char *buf)
{
	unsigned char ret;
	unsigned char  s;
//	while( len ) {  /* 连续输出数据块给USB打印机 */
//		l = len>endp_out_size?endp_out_size:len;  /* 单次发送不能超过端点尺寸 */
		wr_usb_data( len, buf );  /* 将数据先复制到CH375芯片中 */
		toggle_send( tog_send );  /* 数据同步 */
		s = issue_token( ( endp_out_addr << 4 ) | DEF_USB_PID_OUT );  /* 请求CH375输出数据 */
		if ( s==USB_INT_SUCCESS ) {  /* CH375成功发出数据 */
	//		gsm_debug("Send success!!!");
			Test[15]++;
			tog_send = ~ tog_send;  /* 切换DATA0和DATA1进行数据同步 */
			//len-=l;  /* 计数 */
			//buf+=l;  /* 操作成功 */
			ret = 1;
		}
		else if ( s==USB_INT_RET_NAK ) {  /* USB打印机正忙,如果未执行SET_RETRY命令则CH375自动重试,所以不会返回USB_INT_RET_NAK状态 */
			/* USB打印机正忙,正常情况下应该稍后重试 */
			/* s=get_port_status( );  如果有必要,可以检查是什么原因导致打印机忙 */
	//		gsm_debug("Send:Device busy");
			ret = 0;
		}
		else {  /* 操作失败,正常情况下不会失败 */
	//		gsm_debug("Send failed!");
			Test[17]++;
			clr_stall( endp_out_addr );  /* 清除打印机的数据接收端点,或者 soft_reset_print() */
/*			soft_reset_print();  打印机出现意外错误,软复位 */
			tog_send = 0;  /* 操作失败 */
			ret = 0;
//		}
/* 如果数据量较大,可以定期调用get_port_status()检查打印机状态 */
	}
	return ret;
}

unsigned char USB_recv_data( unsigned char *buf)
{
	unsigned char s;
	toggle_recv( tog_recv);
	s = issue_token( (endp_in_addr << 4) | DEF_USB_PID_IN);
	if ( s == USB_INT_SUCCESS)
	{
//		Test[8]++;
	//	gsm_debug("Receive success!!!");
		tog_recv = ~tog_recv;
		return (rd_usb_data(buf));
	}
	else
	{		
//		Test[9]++;
	//	gsm_debug("Receive failed!");
		clr_stall( endp_in_addr | 0x80);
		tog_recv = 0;
		return 0;
	}
//	return (rd_usb_data(buf));
}


void Reset_CH375(void)
{
	CH375_WR_CMD_PORT( CMD_RESET_ALL);

	CH375_WR_CMD_PORT(CMD_CHECK_EXIST);
	CH375_WR_DAT_PORT(0x55);
	
	Test[42] = CH375_RD_DAT_PORT();
}


/* CH375初始化子程序 */
unsigned char CH375Host_Init(unsigned char host_mode)
{
	unsigned char c,i;

	CH375_WR_CMD_PORT( CMD_SET_USB_MODE);
	CH375_WR_DAT_PORT(host_mode);
	Delay50ms();

	for( i = 0xff; i!= 0; i--){
	 	c = CH375_RD_DAT_PORT();
		if ( c== CMD_RET_SUCCESS)
		{
			Test[40]++;
			break;
		}
		else
        {
          	Test[41]++;
        }
	}
	if( i!=0) 
	{

		return (USB_INT_SUCCESS);
	}
        else
          return USB_INT_DISK_ERR;
}




//unsigned char CH375_Device_Init(void)
//{
//    unsigned char ret = FALSE;
//	unsigned char i = 0;
//
//	CH375_WR_CMD_PORT(CMD_SET_USB_MODE);				// 设置USB工作模式, 必要操作
//	CH375_WR_DAT_PORT(1);  								// 设置为使用外部固件的USB设备方式
//
//	
//	for(i = 0; i < 20; i++)   							// 等待操作成功,通常需要等待10uS-20uS 
//    {
//		if(CH375_RD_DAT_PORT() == CMD_RET_SUCCESS)
//		{
//			ret = TRUE;
//			break;
//		}
//	}
//	return ret;
//}

/* CH375中断服务程序，使用查询方式 */
unsigned char mCH375Interrupt_host(void) 
{
	unsigned char mIntStatus;
	unsigned int count = 0;
	count = 0;
	while( CH375_INT_WIRE)	  /* 查询等待CH375操作完成中断(INT#低电平) */
	{
		count++;
		if(count > 5000)	{ // 50ms
		CH375_WR_CMD_PORT( CMD_ABORT_NAK );
		break; 
		}
		Delay10us();
	//	vTaskDelay(5/ portTICK_RATE_MS);
	}

//	CH375_WR_CMD_PORT( CMD_GET_STATUS );  /* 产生操作完成中断, 获取中断状态 */
	//return( CH375_RD_DAT_PORT() );
	CH375_WR_CMD_PORT(CMD_GET_STATUS);								// 获取中断状态并取消中断请求 
	mIntStatus = CH375_RD_DAT_PORT();  						// 获取中断状态

	if( mIntStatus == USB_INT_DISCONNECT)
	{
		mDeviceOnline = 0;
	}
	else if( mIntStatus == USB_INT_CONNECT)
	{
		mDeviceOnline = 1;
	}
	else
	{
	}
	return mIntStatus;
}



