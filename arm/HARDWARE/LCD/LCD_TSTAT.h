
#ifndef	__LCD_TSTAT_H__

#define	__LCD_TSTAT_H__



#include "bitmap.h"
#include "define.h"



#ifndef	TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif


//#define FORM48X120		0
#define FORM32X64 		0
#define FORM15X30			1


#define CH_HEIGHT													36

#define THERM_METER_POS										5

#define SETPOINT_POS											108
#define FAN_MODE_POS											SETPOINT_POS+CH_HEIGHT+7
#define SYS_MODE_POS											FAN_MODE_POS+CH_HEIGHT+7

#define MENU_ITEM1      SETPOINT_POS+0
#define MENU_ITEM2      FAN_MODE_POS+0

#define TIME_POS										      SYS_MODE_POS + CH_HEIGHT + 7
#define MENU_ITEM_POS											130
#define MENU_VALUE_POS										SYS_MODE_POS//MENU_ITEM_POS + CH_HEIGHT
#define ICON_POS													274
#define ICON_XPOS 												2

#define FIRST_ICON_POS									  ICON_XPOS
#define SECOND_ICON_POS                   FIRST_ICON_POS + 60
#define THIRD_ICON_POS                    SECOND_ICON_POS + 60
#define FOURTH_ICON_POS                   THIRD_ICON_POS + 60
#define FIFTH_ICON_POS                    FOURTH_ICON_POS + 40

#define ICON_XDOTS						55
#define ICON_YDOTS						45

#define FANBLADE_XDOTS						40
#define FANBLADE_YDOTS						45

#define FANSPEED_XDOTS						15
#define FANSPEED_YDOTS						45

#define THERM_METER_XPOS									39
#define TEMP_FIRST_BLANK						      0//30  //+= blank width
#define FIRST_CH_POS											TEMP_FIRST_BLANK + THERM_METER_XPOS
#define SECOND_CH_POS											FIRST_CH_POS+48
#define THIRD_CH_POS											SECOND_CH_POS+48+16
#define UNIT_POS													THIRD_CH_POS + 48+ 15
#define BUTTON_DARK_COLOR   							0X0BA7
#define BTN_OFFSET												CH_HEIGHT+7


#define TOP_AREA_DISP_ITEM_TEMPERATURE   	0
#define TOP_AREA_DISP_ITEM_HUM					 	1
#define TOP_AREA_DISP_ITEM_CO2				   	2

#define TOP_AREA_DISP_UNIT_C   					 	0
#define TOP_AREA_DISP_UNIT_F					 	 	1
#define TOP_AREA_DISP_UNIT_PPM				   	2
#define TOP_AREA_DISP_UNIT_PERCENT			 	3

#define TSTAT8_CH_COLOR   	0xffff //0xd6e0
#define TSTAT8_MENU_COLOR   0x7e17//0x3bef//0x43f2//0x14a9


#define SCH_COLOR  0xffff//0XB73F
#define SCH_BACK_COLOR  0x3bef//0x43f2//0x14E9

#define TSTAT8_BACK_COLOR1  0x3cef//0x7E19
#define TSTAT8_BACK_COLOR   0x7E19//
#define TSTAT8_MENU_COLOR2  0x7e17
#define TANGLE_COLOR        0xbe9c

#define FAN_OFF 	0
#define FAN_AUTO 	4
#define FAN_ON		1
#define FAN_SPEED1 1
#define FAN_SPEED2 2
#define FAN_SPEED3 3

#define SCH_XPOS  10

#define HC_CFG_AUTO			0
#define HC_CFG_COOL			1
#define HC_CFG_HEAT			2


#define	LONG_PRESS_TIMER_SPEED_100	200
#define	LONG_PRESS_TIMER_SPEED_50	100
#define	LONG_PRESS_TIMER_SPEED_10	50
#define	LONG_PRESS_TIMER_SPEED_1	20


#define KEY_SPEED_1			(0x0000)
#define KEY_SPEED_10		(0x0100)
#define KEY_SPEED_50		(0x0200)
#define KEY_SPEED_100		(0x0300)
#define KEY_SPEED_MASK		(0x00ff)
#define KEY_FUNCTION_MASK	(0xff00)

#define	KEY_UP_MASK			2//(1 << 1)
#define	KEY_DOWN_MASK		4//(1 << 2)
#define	KEY_LEFT_MASK		8//(1 << 3)
#define	KEY_RIGHT_MASK		1//(1 << 0)
#define	KEY_LEFT_RIGHT_MASK		9//(1 << 0)


void vStartKeyTasks( unsigned char uxPriority);
void vStartMenuTask(unsigned char uxPriority);


#ifndef TSTAT7_ARM

#define MAX_SCOROLL 16

extern uint8 *scroll;
extern uint8 scroll_ram[5][MAX_SCOROLL];
extern uint8 fan_flag;
extern uint8 display_flag;
extern uint8 schedule_hour_minute; //indicate current display item is "hour" or "minute"
extern uint8 blink_parameter;
extern uint8 clock_blink_flag;
void LCDtest(void);
extern uint8 const chlib[];
extern uint8 const chlibsmall[];
extern uint16 const athome[];
extern uint16 const offhome[];
extern uint16 const sunicon[];
extern uint16 const moonicon[];
extern uint16 const heaticon[]; 
extern uint16 const coolicon[];
extern uint16 const fanspeed0a[];
extern uint16 const fanspeed1a[];
extern uint16 const fanspeed2a[];
extern uint16 const fanspeed3a[];
extern uint16 const fanbladeA[];
extern uint16 const fanbladeB[];

extern uint16 const degree_o[];
//extern uint16 const therm_meter[];
extern uint16 const leftup[];
extern uint16 const leftdown[];
extern uint16 const rightdown[];
extern uint16 const rightup[];
extern uint16 const cmnct_send[]; 
extern uint16 const cmnct_rcv[]; 
extern uint16 const wifinocnnct[];
extern uint16 const wificonnect[];

typedef struct   
{
 uint8 unit;
 uint8 setpoint;
 uint8 fan;
 uint8 sysmode;
 uint8 occ_unocc;
 uint8 heatcool;
 uint8 fanspeed;
 uint8 cmnct_send;
 uint8 cmnct_rcv; 	
} DISP_CHANGE; 
extern DISP_CHANGE icon;
//extern uint16 const angle[];
void draw_tangle(uint8 xpos, uint16 ypos);
void ClearScreen(unsigned int bColor);
void disp_ch(uint8 form, uint16 x, uint16 y,uint8 value,uint16 dcolor,uint16 bgcolor);		
void disp_icon(uint16 cp, uint16 pp, uint16 const *icon_name, uint16 x,uint16 y,uint16 dcolor, uint16 bgcolor);
void disp_null_icon(uint16 cp, uint16 pp, uint16 const *icon_name, uint16 x,uint16 y,uint16 dcolor, uint16 bgcolor);
void disp_str(uint8 form, uint16 x,uint16 y,uint8 *str,uint16 dcolor,uint16 bgcolor);	
void display_SP(int16 setpoint);
void display_screen_value(uint8 type);
void display_fanspeed(int16 speed);
void display_mode(uint8 heat_cool_user);
void display_fan(void);
void display_icon(void);
void display_value(uint16 pos,int16 disp_value, uint8 disp_unit);
//void display_menu(uint16 pos, uint8 *item);
void display_menu (uint8 *item1, uint8 *item2);
void clear_line(uint8 linenum);
void clear_lines(void);
//void display_clock_date(int8 item, int16 value);
//void display_clock_time(int8 item, int16 value);
void display_scroll(void);
void scroll_warning(uint8 item);
void display_schedule_time(int8 schedule_time_sel, uint8 hour_minute);
void Top_area_display(uint8 item, int16 value, uint8 unit);

#endif //TSTAT7_ARM

#endif

