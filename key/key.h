 /*================================================================================
 * Module Name : key.h
 * Purpose     : key dirver and deal with back light and buzzer
 * Author      : Chelsea
 * Date        : 2008/11/10
 * Notes       : 
 * Revision	   : 
 *	rev1.0
 *================================================================================
 */


#define KEY
#ifdef KEY

#include "types.h"
/* CONSTANT DECLARATIONS */

/*************************************************************************/
/*  KEY FUCTION DECLARTION */
/*************************************************************************/
#define BEEP_ON 	0
#define BEEP_OFF 	1

#define BACK_ON 	1
#define BACK_OFF 	0

#define K_NONE      0
#define K_SELECT1	  1    //  Select a menu or submenu  / Select a value for adjustment/ Save a change made to the value.
#define K_PROGRAM1   2    //   Menu/Cancel  - toggle between the idle display and the Menu.
#define K_UP1				3  
#define K_DOWN1      4 
#define K_UP_DOWN1	5
#define K_SEL_PRO1   6
#define K_UP_SEL1	7
#define K_DOWN_SEL1  8
//#define K_SELECT	  1    //  Select a menu or submenu  / Select a value for adjustment/ Save a change made to the value.
//#define K_PROGRAM   2    //   Menu/Cancel  - toggle between the idle display and the Menu.
//#define K_UP				3  
//#define K_DOWN      4 
//#define K_UP_DOWN	5
//#define K_SEL_PRO   6
//#define K_UP_SEL	7
//#define K_DOWN_SEL  8

#define K_LOAD      K_SELECT + 0x80
#define K_LONG_PROG K_PROGRAM + 0x80
#define K_RESET		K_UP_DOWN + 0x80
//#define K_RESET_LCD K_SEL_PRO + 0x80
#define K_DOWNLOAD	K_SEL_PRO + 0x80

#define K_AUTO_TEST		K_UP_SEL + 0x80
#define K_MANUAL_TEST 	K_DOWN_SEL + 0x80

#define K_LEFT 0X8000
#define K_DOWN 0X4000
#define K_UP	 0X2000
#define K_RIGHT 0X1000
/*enum
{
  KEY1 = 1,KEY2,KEY3,KEY4,LCD
};
*/
extern U8_T by_Key;
extern U8_T flag_Send;



void Key_Inital(void);
void Key_Process(void) reentrant;   // 2ms
void vStartKeyTasks( unsigned char uxPriority);

/******END KEY***********************************************************/



#endif




