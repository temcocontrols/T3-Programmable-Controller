#ifndef __LCD_H__
#define __LCD_H__

#include "types.h"

//  display mode
#define 	NORMAL 		1
#define 	INVERSE 	0

//void delay(char num);
void Lcd_Write_Byte(U8_T num); //from high to low
void Lcd_Write_Command(U8_T command); //from high to low
void Lcd_Write_Char(U8_T row,U8_T line,U8_T num,U8_T mode); // row: 0 - 4 , line: 0 - 15
void Lcd_Set_Fuction(U8_T mode);
void Lcd_Set_Y_Addr(U8_T page);// 0 1 0 0        page 0 - 9
void Lcd_Set_X_Addr(U8_T line);// 1 1 1 0/1        page 0 - 129
void Lcd_Initial(void);
//void Lcd_Show_String(char pos_x,char pos_y,char* str,unsigned char mode,unsigned char length);	// pos_x: 0 - 4 , pos_y: 0 - 15
//void Lcd_Show_String(U8_T pos_x, U8_T pos_y, S8_T* str,U8_T mode,U8_T format);
void Lcd_Show_Data(U8_T pos_x,U8_T pos_y,U16_T number,U8_T dot,U8_T mode);
void Lcd_All_Off(void);

void scrolling_message(void);
void start_scrolling(void);
void stop_scrolling(void);
void update_message_context(void);

#define MAX_INT		32767
#define MAX_BYTE	255
#define MAX_UINT	65535

#define FALSE 0
#define TRUE  1

#define NONE 0
// define command 
#define CMD_WRITE_DATA                 // A0 = 1

// A0 = 0
//[H0 :H1] = [0 : 0]
#define CMD_SET_V0Range_LOW     0x04
#define CMD_SET_V0Range_HIGH    0x05
#define CMC_END			0X06
#define CMD_DISPLAT_ALLOFF      0X08
#define CMD_DISPLAT_NORMAL      0X0C
#define CMD_DISPLAT_ALLON       0X09
#define CMD_DISPLAY_INVERSE     0X0D


//[H0 :H1] = [0 : 1]
#define CMD_DISPLAY_CONFIG      0X08// DO = 1
#define CMD_SET_BIAS            0x11  //  1/10
#define CMD_SET_V0              0xae  //Set VOP value:AB=Vop=10.8V  =0aeh


//[H0 :H1] = [1 : 0]
#define CMD_SET_SHOWMODE        0x04   //full display 1/80 duty
//#define CMD_SET_SHOWPART 
//#define CMC_SET_STARTLINE

//[H0 :H1] = [1 : 1]
#define CMD_RESET               0x03
#define CMD_HIGH_POWER          0xb4
#define CMD_FRE_50              0x08
#define CMD_FRE_68		0x09
#define CMD_FRE_70		0x0a
#define CMD_FRE_73		0x0b
#define CMD_FRE_75		0x0c
#define CMD_FRE_78		0x0d
#define CMD_FRE_81		0x0e
#define CMD_FRE_150		0x0f




// FOR NEW LCD


typedef struct
{
	U8_T x;
	U8_T y;
	S8_T *str;
	U8_T mode;
	U8_T len;
	U16_T dcolor;
	U16_T bgcolor;
} xLCDMessage;



//void vLCDTask( void ) reentrant;
U8_T Check_Lcd(void);
void Lcd_Show_String(U16_T pos_x, U16_T pos_y, char* str,U8_T mode,U8_T format,U16_T dcolor,U16_T bgcolor);
//void ClearScreen(unsigned int bColor);
//void disp_str(uint8 form, uint16 x,uint16 y,uint8 *str,uint16 dcolor,uint16 bgcolor);


#endif




