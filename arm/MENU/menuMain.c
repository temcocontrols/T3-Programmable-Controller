#include "main.h"
#include "define.h"
#include "LCD_TSTAT.h"
#include "menu.h"
#include "Constcode.h"

const char c_strBaudate[UART_BAUDRATE_MAX][7] =
{
    "1200",   //0
    "2400",   //1
    "3600",   //2
    "4800",   //3
    "7200",   //4
    "9600",   //5
    "19200",  //6
    "38400",  //7
    "76800",  //8
    "115200", //9
    "921600", //10
    "57600"   //11
};

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

	if(item == 0) 		
        return Modbus.address;
    else if (item == 1)
    {
        return  Setting_Info.reg.com_baudrate[0];//      Modbus.baudrate;
        ;//return (uint32)(Modbus.ip_addr[0] << 24) + (uint32)(Modbus.ip_addr[1] << 16) + (uint16)(Modbus.ip_addr[2] << 8) + Modbus.ip_addr[3];
    }
    else if (item == 2)
    {
        return Modbus.protocal;
    }
	else	//if(item == 3)			
		return 0;
	
}


void show_parameter(void)
{
    char temp_buffer[10];
    memset(temp_buffer, 0, 0);
    switch (item_to_adjust)
    {
    case 0:
        display_value(0, get_display_value(item_to_adjust), ' ');
        break;
    case 1:
    {
        //vars[108].value = Setting_Info.reg.com_baudrate[0];
        switch (Setting_Info.reg.com_baudrate[0])
        {
            //case UART_1200:	    //Fandu  这一批波特率暂时不让设置
            //case UART_2400:	   
            //case UART_3600:	   
            //case UART_4800:	   
            //case UART_7200:	   
            //case UART_921600:	

        case UART_9600:
        case UART_19200:
        case UART_38400:
        case UART_57600:
        case UART_76800:
        case UART_115200:
            memcpy(temp_buffer, c_strBaudate[Setting_Info.reg.com_baudrate[0]], 7);
            break;
        default:
            sprintf(temp_buffer, "val %d", Setting_Info.reg.com_baudrate[0]);
            break;

        }
        disp_str(FORM15X30, 0, MENU_VALUE_POS, temp_buffer, TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
    }
    break;
    case 2:
    {
        if ((Setting_Info.reg.com_config[0] == BACNET_SLAVE) ||
            (Setting_Info.reg.com_config[0] == BACNET_MASTER))
            disp_str(FORM15X30, 0, MENU_VALUE_POS, "BACNET", TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
        else
            disp_str(FORM15X30, 0, MENU_VALUE_POS, "MODBUS", TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
    }
        //if (Setting_Info.reg.com_config[0] == 0)
        //    disp_str(FORM15X30, 0, MENU_VALUE_POS, "BACNET", TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
        //else
        //    disp_str(FORM15X30, 0, MENU_VALUE_POS, "MODBUS", TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
        break;
    default:
        break;
    }
}

void MenuMain_display(void)
{
//	if(item_to_adjust == 1)
//	{
//		char text[20];
//		sprintf(text, "%u.%u.%u.%u", (uint16)Modbus.ip_addr[0], (uint16)Modbus.ip_addr[1], (uint16)Modbus.ip_addr[2], (uint16)Modbus.ip_addr[3]);
//		disp_str(FORM15X30, 0, SYS_MODE_POS,  text,  TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
//	}
//	else	
	{		
        show_parameter();
		//display_value(0,get_display_value(item_to_adjust), ' '); 	
	}
}

void MenuMain_keycope(uint16 key_value)
{
	switch(key_value & KEY_SPEED_MASK)
	{
		case 0:
			// do nothing
			break;
		case KEY_UP_MASK:
		if(item_to_adjust < MAX_MENU_NUM)	item_to_adjust++; start_menu();
			break;
		case KEY_DOWN_MASK:
		if(item_to_adjust > 0)	item_to_adjust--; start_menu();
			break;
		case KEY_LEFT_MASK:
			update_menu_state(MenuIdle);
			break;
		case KEY_RIGHT_MASK:
			// go into main menu
			// change value
			update_menu_state(MenuSet);
			break;
	}
}




