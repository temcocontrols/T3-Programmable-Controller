#include "main.h"
#include "define.h"
#include "LCD_TSTAT.h"
#include "menu.h"
#include "Constcode.h"


extern uint8_t item_to_adjust;



uint16 set_value;
uint8 blink_count;
uint8 flag_blink;
void MenuSet_init(void)
{
	item_to_adjust = 0;
//	clear_line(1);
//	clear_line(0);
	set_value = Modbus.address;
	clear_lines();
	start_menu();
	flag_blink = 1;
	blink_count = 0;
}
 

void MenuSet_display(void)
{
	if(blink_count < 10)	blink_count++;
	if(blink_count % 2 == 0)
		display_value(0,set_value, ' ');
	else
		clear_line(0);
}

void MenuSet_keycope(uint16 key_value)
{
	switch(key_value& KEY_SPEED_MASK)
	{
		case 0:
			// do nothing
			break;
		case KEY_UP_MASK:
			// do nothing
		if(set_value < menu[item_to_adjust].max)	set_value++; 
		blink_count = 0;
			break;
		case KEY_DOWN_MASK:
			// do nothing
			if(set_value > menu[item_to_adjust].min)	set_value--; 
		blink_count = 0;
			break;
		case KEY_LEFT_MASK:
			update_menu_state(MenuMain);
			break;
		case KEY_RIGHT_MASK:
			// confirm setting,save value
			break;
	}
}

void Save_Parmeter(item_to_adjust)
{
	switch(item_to_adjust)
	{
		case 0: // address
			break;
		default:
			break;
	}
}


