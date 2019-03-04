#ifndef USB_CH375_H
#define USB_CH375_H

/* INCLUDES*/
#include "ch375inc.h"
#include "reg80390.h"	   //  write your chip header file
#include <intrins.h>
#include <absacc.h>


/* CONSTANTS */
#define UNKNOWN_USB_DEVICE  0xf1
#define USB_MEM_ERROR       0xf2

#ifndef	TRUE
#define TRUE				1
#endif

#ifndef	FALSE
#define FALSE				0
#endif


#define  CH375_INT_WIRE 	P2_7    

/* MACROS */
#define DELAY_1_US() { 	_nop_(); _nop_(); _nop_(); _nop_(); _nop_();  \
	_nop_(); _nop_(); _nop_(); _nop_(); _nop_();  \
	_nop_(); _nop_(); _nop_(); _nop_(); _nop_();   \
	_nop_(); _nop_(); _nop_(); _nop_(); _nop_();  \
	_nop_(); _nop_(); _nop_(); _nop_(); _nop_(); \				
}										  // Write your 1 us delay


#define CH375_CMD_PORT	FVAR(unsigned char,0x188001)
#define CH375_DAT_PORT 	FVAR(unsigned char,0x188000)

#define CH375_WR_CMD_PORT(cmd)  CH375_CMD_PORT = cmd;

#define CH375_WR_DAT_PORT(dat)  CH375_DAT_PORT = dat;

#define CH375_RD_DAT_PORT() 	CH375_DAT_PORT 
								  


/* TYPEDEFS */
typedef unsigned char BOOL1;

/* VARIABLES */

extern unsigned char endp_out_addr;		  // Change the endpoint to which endpoint you want to send data
extern unsigned short endp_out_size;
extern unsigned char endp_in_addr;		  // Change the endpoint to which endpoint you want to receive data
extern unsigned char mDeviceOnline;		  // Once a USB device pluging in, mDeviceOnline will be 1

/* FUNCTIONS */

/* @brief    Detect	interrupt
   @return   TRUE, interrupt exist
   			 FALSE, no interrupt
 */
unsigned char usb_poll(void);

/* @brief    Initialize ch375 to host mode
   @param	 host_mode=5 set ch375 to host mode without SOF pocket
   			 host_mode=6 set ch375 to host mode with SOF pocket
			 host_mode=7 set ch375 to host mode and reset USB bus
   @return   USB_INT_SUCCESS
   			 USB_INT_DISK_ERR
 */
unsigned char CH375Host_Init(unsigned char host_mode);

/* @brief    To get device description and config description
			 use mReadCH375Data to put the descriptions in buffer
   @param	 type=1 get device description
   			 type=2 get config description		
   @return   mCH375Interrupt
 */
unsigned char mCtrlGetDescr( unsigned char type);

/* @brief    Set device address
   @param	 addr=1-127		
   @return   mCH375Interrupt
 */
unsigned char mCtrlSetAddress( unsigned char addr );

/* @brief    Set config value
   @param	 config description's bConfigurationValue		
   @return   mCH375Interrupt
 */
unsigned char mCtrlSetConfig( unsigned char value );

/* @brief    Read the continuuous data from ch375, 
   @param	 Point to the buffer where put data in		
   @return   Length of the data
 */
unsigned char mReadCH375Data( unsigned char *buf );

/* @brief    Send data to endp_out_addr, 
   @param	 len - length of the data
             buf - the data buffer	
 */
void USB_send_data(unsigned char len, unsigned char *buf);

/* @brief    Read the data from endp_in_addr, 
   @param	 Point to the buffer where put data in		
   @return   Length of the data
 */
unsigned char USB_recv_data( unsigned char *buf);


/* @brief    Write command to CH375, 
   @param	 CMD_GET_DESCR and so on		
   @return   none
 */
//void CH375_WR_CMD_PORT(unsigned char cmd);

/* @brief    Write data to CH375, 
   @param	 a byte		
   @return   none
 */
//void CH375_WR_DAT_PORT(unsigned char dat);

/* @brief    Read data from CH375, 
   @param	 none		
   @return   a byte
 */
//unsigned char CH375_RD_DAT_PORT(void);


/* @brief    Interrupt function, write by yourself		
   @return   USB_INT_SUCCESS
   			 USB_INT_CONNECT
			 USB_INT_DISCONNECT
			 USB_INT_BUF_OVER
			 USB_INT_DISK_READ
			 USB_INT_DISK_WRITE
			 USB_INT_DISK_ERR
 */	

//unsigned char CH375_Device_Init(void);

unsigned char mCH375Interrupt_host(void);

#endif