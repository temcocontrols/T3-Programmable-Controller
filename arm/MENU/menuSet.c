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
	//item_to_adjust = 0;
//	clear_line(1);
//	clear_line(0);
	//set_value = Modbus.address;
    if (item_to_adjust == 0) // 初始化ID；
    {
        set_value = Setting_Info.reg.modbus_id;
    }
    else if (item_to_adjust == 1)
    {
        set_value = Setting_Info.reg.com_baudrate[0];
    }
    else if (item_to_adjust == 2)
    { 
        if ((Setting_Info.reg.com_config[0] == BACNET_MASTER) ||
            (Setting_Info.reg.com_config[0] == BACNET_SLAVE))
            set_value = 0;
        else
            set_value = 1;
    }
	clear_lines();
	start_menu();
	flag_blink = 1;
	//blink_count = 0;
}
 
extern const char c_strBaudate[UART_BAUDRATE_MAX][7];
void MenuSet_display(void)
{
    char temp_buffer[10];
    memset(temp_buffer, 0, 0);

    blink_count = (++blink_count) % 2;

    if (blink_count %2  == 0)
    {
        if(item_to_adjust == 0) //如果是调整modbusID
            display_value(0, set_value, ' ');
        else if (item_to_adjust == 1) //如果是调整波特率
        {
            if (set_value < 5) //限制 波特率的范围 其他波特率 菜单不显示;
                set_value = 5;
            if (set_value > 9)
                set_value = 9;
            memcpy(temp_buffer, c_strBaudate[set_value], 7);
						
            disp_str(FORM15X30, 0, MENU_VALUE_POS, temp_buffer, TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
        }
        else if (item_to_adjust == 2) //调整协议
        {
            if (set_value == 0)
                disp_str(FORM15X30, 0, MENU_VALUE_POS, "BACNET", TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
            else
                disp_str(FORM15X30, 0, MENU_VALUE_POS, "MODBUS", TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
        }
    }
    else
        clear_line(0);

	//if(blink_count % 2 == 0)
	//	display_value(0,set_value, ' ');
	//else
	//	clear_line(0);
}

void Save_Parmeter(item_to_adjust)
{
    switch (item_to_adjust)
    {
    case 0: // address
        Modbus.address = set_value;
        E2prom_Write_Byte(EEP_ADDRESS, Modbus.address);
        Station_NUM = Modbus.address;
        Setting_Info.reg.MSTP_ID = Station_NUM;
        Setting_Info.reg.modbus_id = Station_NUM;
        if (Modbus.protocal == 0)
            dlmstp_init(NULL);
        break;
    case 1:  //修改 波特率
    {        
        uart0_baudrate = set_value;
        Setting_Info.reg.com_baudrate[0] = uart0_baudrate;
        E2prom_Write_Byte(EEP_UART0_BAUDRATE, uart0_baudrate);
        UART_Init(0);
    }

    break;
    case 2:  //修改 协议
    {
        if (set_value == 0)         //这里只定义了两种协议 所以用0 和1 来表示
            set_value = BACNET_MASTER;
        else
            set_value = MODBUS_SLAVE;

        if (set_value == BACNET_MASTER)
        {
            Modbus.com_config[0] = BACNET_MASTER;
            Setting_Info.reg.com_config[0] = BACNET_MASTER;
        }
        else
        {
            Modbus.com_config[0] = MODBUS_SLAVE;
            Setting_Info.reg.com_config[0] = MODBUS_SLAVE;
        }
        if ((Modbus.com_config[0] == MODBUS_SLAVE) || (Modbus.com_config[0] == 0))
            uart_serial_restart(0);
        if (Modbus.com_config[0] == BACNET_SLAVE || Modbus.com_config[0] == BACNET_MASTER)
        {
            Recievebuf_Initialize(0);
            dlmstp_init(NULL);
        }Test[9] = 15;
        Count_com_config();
        E2prom_Write_Byte(EEP_COM0_CONFIG, Modbus.com_config[0]);

    }
        break;
    default:
        break;
    }
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
			break;
		case KEY_DOWN_MASK:
			// do nothing
			if(set_value > menu[item_to_adjust].min)	set_value--; 
			break;
		case KEY_LEFT_MASK:
			update_menu_state(MenuMain);
			break;
		case KEY_RIGHT_MASK:
            Save_Parmeter(item_to_adjust);
            update_menu_state(MenuIdle);
			// confirm setting,save value
			break;
	}
}



