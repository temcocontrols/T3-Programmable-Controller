#include "main.h"
#include "define.h"
#include "LCD_TSTAT.h"
#include "menu.h"
#include "Constcode.h"


extern uint8_t item_to_adjust;

void MenuMain_init(void)
{
	item_to_adjust = 0;
//	clear_line(1);
//	clear_line(0);
	clear_lines();
	start_menu();
}
 
uint32 get_display_value(uint8 item)
{

	if(item == 0) 		return Modbus.address;
	else	if(item == 1)			;//return (uint32)(Modbus.ip_addr[0] << 24) + (uint32)(Modbus.ip_addr[1] << 16) + (uint16)(Modbus.ip_addr[2] << 8) + Modbus.ip_addr[3];
	else	if(item == 2)			return 10;
	else	//if(item == 3)			
		return 100;
	
}


void MenuMain_display(void)
{
	if(item_to_adjust == 1)
	{
		char text[20];
		sprintf(text, "%u.%u.%u.%u", (uint16)Modbus.ip_addr[0], (uint16)Modbus.ip_addr[1], (uint16)Modbus.ip_addr[2], (uint16)Modbus.ip_addr[3]);
		disp_str(FORM15X30, 0, SYS_MODE_POS,  text,  TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
	}
	else	
	{		
		display_value(0,get_display_value(item_to_adjust), ' '); 	
	}
}

void MenuMain_keycope(uint16 key_value)
{
	switch(key_value& KEY_SPEED_MASK)
	{
		case 0:
			// do nothing
			break;
		case KEY_UP_MASK:
			// do nothing
		if(item_to_adjust < MAX_MENU_NUM)	item_to_adjust++; start_menu();
			break;
		case KEY_DOWN_MASK:
			// do nothing
		if(item_to_adjust > 0)	item_to_adjust--; start_menu();
			break;
		case KEY_LEFT_MASK:
			// do nothing
			update_menu_state(MenuIdle);
			break;
		case KEY_RIGHT_MASK:
			// go into main menu
			// change value
			update_menu_state(MenuSet);
			break;
	}
}




