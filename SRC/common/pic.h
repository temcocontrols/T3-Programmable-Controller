#ifndef PIC_H
#define PIC_H
 
#include "main.h"

#if defined(CM5)

#define C_VER_CHECKSUM  0x69
#define READING_THRESHOLD	150

// PIC commands protocol
#define PIC16_PULSE    	 		0xB1
#define PIC16_VERSION			0xB2

#define MAX_FILTER 12

//    define command 
#define READ_VERSION    0xb2
enum
{
    READ_CHAN0 = 0XF0,
    READ_CHAN2,READ_CHAN3,READ_CHAN4,READ_CHAN5,READ_CHAN6,READ_CHAN7,READ_CHAN8,READ_CHAN9,READ_CHAN10,READ_CHAN11
};
//   end define


void pic_detect(void);
void vStartPicTasks( unsigned char uxPriority);
void Initial_Range(void);
void write_pic( unsigned char addr, unsigned char value );
unsigned int GET_ACK( void );
void GIVE_PIC_ACK( void );
void i2c_pic_start();
void i2c_pic_write( unsigned char ch );
int i2c_pic_read( void );
void i2c_stop(void);
//unsigned char  ReadPicVersion(void);
bit read_pic_version( void);
bit read_pic( char channel);




#else if defined(MINI)

#define 	C_VER_CHECKSUM  0x69
#define READING_THRESHOLD	150

// PIC commands protocol
#define PIC16_PULSE    	 		0xB1
#define PIC16_VERSION			0xB2

#define MAX_FILTER 12

//    define command 
#define READ_VERSION    0xb2
enum
{
	GET_VERSION = 0,
	READ_AO1_FEEDBACK_L,READ_AO1_FEEDBACK_H,
	READ_AO2_FEEDBACK_L,READ_AO2_FEEDBACK_H,
	READ_AO3_FEEDBACK_L,READ_AO3_FEEDBACK_H,
	READ_AO4_FEEDBACK_L,READ_AO4_FEEDBACK_H,
	READ_AO5_FEEDBACK_L,READ_AO5_FEEDBACK_H,
	READ_AO6_FEEDBACK_L,READ_AO6_FEEDBACK_H,
	READ_AO7_FEEDBACK_L,READ_AO7_FEEDBACK_H,
	READ_AO8_FEEDBACK_L,READ_AO8_FEEDBACK_H,
	READ_AO9_FEEDBACK_L,READ_AO9_FEEDBACK_H,
	READ_AO10_FEEDBACK_L,READ_AO10_FEEDBACK_H,
	READ_AO11_FEEDBACK_L,READ_AO11_FEEDBACK_H,
	READ_AO12_FEEDBACK_L,READ_AO12_FEEDBACK_H,

	SET_RELAY_LOW,
	SET_RELAY_HI,
	
};
//   end define


void vStartPicTasks( unsigned char uxPriority);
void Initial_Range(void);

#endif



#endif














