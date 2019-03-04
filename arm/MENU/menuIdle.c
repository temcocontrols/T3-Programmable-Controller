#include "main.h"
#include "define.h"
#include "LCD_TSTAT.h"
#include "menu.h"

#define	NODES_POLL_PERIOD	30

static uint8 display_around_time_ctr = NODES_POLL_PERIOD;
static uint8 disp_index = 0;
static uint8 warming_state = TRUE;
static uint8 force_refresh = TRUE;

void MenuIdle_init(void)
{

	LCDtest();
	
  scroll = &scroll_ram[0][0];
//	fanspeedbuf = fan_speed_user;
		
	disp_null_icon(240, 36, 0, 0,TIME_POS,TSTAT8_CH_COLOR, TSTAT8_MENU_COLOR2);
	disp_icon(14, 14, degree_o, UNIT_POS - 14,56 ,TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
	
	draw_tangle(102,105);
	draw_tangle(102,148);
	draw_tangle(102,191);
	
	disp_str(FORM15X30, SCH_XPOS,  SETPOINT_POS,  "SET",SCH_COLOR,TSTAT8_BACK_COLOR);//TSTAT8_BACK_COLOR
	disp_str(FORM15X30, SCH_XPOS,  FAN_MODE_POS,"FAN",SCH_COLOR,TSTAT8_BACK_COLOR);
	disp_str(FORM15X30, SCH_XPOS,  SYS_MODE_POS,  "SYS",SCH_COLOR,TSTAT8_BACK_COLOR);

	Top_area_display(TOP_AREA_DISP_ITEM_TEMPERATURE, 258, TOP_AREA_DISP_UNIT_C);
#if ARM_UART_DEBUG
	uart1_init(115200);
	DEBUG_EN = 1;
	printf("IDLE init \r\n");
#endif	
}

 
void get_data_format(u8 loc,float num,char *s)
{
	u8 i,s_len,s_start,buf_start;
	
	if(loc == 0)
		sprintf(s,"%9.0f",num);
	else if(loc == 1)
		sprintf(s,"%9.1f",num);
	else if(loc == 2)
		sprintf(s,"%9.2f",num);
	else if(loc == 3)
		sprintf(s,"%9.3f",num);
	else if(loc == 4)
		sprintf(s,"%9.4f",num);
	else if(loc == 5)
		sprintf(s,"%9.5f",num);
	else if(loc == 6)
		sprintf(s,"%9.6f",num);
	else
		sprintf(s,"%f",num);
	
	for(i=0;i<9;i++)
	{
		if(s[i]!=0x20) break;
	}
	s_len = 9 - i;   					//数据长度
	s_start = i;     					//数据起始位置
	buf_start = i - i / 2; 				//重新排置后的起始位置
	
	for(i=0;i<s_len;i++) 				//数据左移
	{
		s[buf_start + i] = s[s_start + i];
	}
	for(i=buf_start + s_len;i<9;i++ ) 	//补" "
	{
		s[i] = 0x20;
	} 
}
 

void MenuIdle_display(void)
{
		display_scroll();			
		display_SP(123);
		display_fanspeed(3);
		display_mode();
		display_icon();
		display_fan();	

}

void MenuIdle_keycope(uint16 key_value)
{
	switch(key_value& KEY_SPEED_MASK)
	{
		case 0:Test[10]++;
			// do nothing
			break;
		case KEY_UP_MASK:Test[11]++;
			// do nothing
			break;
		case KEY_DOWN_MASK:Test[12]++;
			// do nothing
			break;
		case KEY_LEFT_MASK:Test[13]++;
			// do nothing
			update_menu_state(MenuMain);

			break;
		case KEY_RIGHT_MASK:Test[14]++;
			// go into main menu
			break;
	}
}



