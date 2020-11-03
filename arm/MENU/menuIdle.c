#include "main.h"
#include "define.h"
#include "LCD_TSTAT.h"
#include "menu.h"
#include "wifi.h"
#define	NODES_POLL_PERIOD	30

char UI_DIS_LINE1[4]; //对应之前 setpoint  fan 以及 sys
char UI_DIS_LINE2[4];
char UI_DIS_LINE3[4];

static uint8 display_around_time_ctr = NODES_POLL_PERIOD;
static uint8 disp_index = 0;
static uint8 set_msv = 0;
static uint8 warming_state = TRUE;
static uint8 force_refresh = TRUE;
uint8 flag_left_key = 0;
uint8	count_left_key = 0;
void MenuIdle_init(void)
{
	//vars[0].value = uart0_baudrate * 1000;
	//vars[1].value = Station_NUM * 1000;
	//vars[2].value = Modbus.protocal * 1000;
	//vars[3].value = Instance * 1000;
//	vars[4].value = 0;
//	vars[5].value = 0;
//	vars[6].value = 0;
//	vars[7].value = 0;
//	vars[8].value = SW_REV * 1000;
//	vars[9].value = 0;
//	vars[10].value = 0;
//	vars[11].value = 0;
//	vars[12].value = 0;
//	vars[13].value = 0;
//	vars[14].value = 0;
	
	LCDtest();
	
  scroll = &scroll_ram[0][0];
//	fanspeedbuf = fan_speed_user;
		
	disp_null_icon(240, 36, 0, 0,TIME_POS,TSTAT8_CH_COLOR, TSTAT8_MENU_COLOR2);
	disp_icon(14, 14, degree_o, UNIT_POS - 14,56 ,TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
	
	draw_tangle(102,105);
	draw_tangle(102,148);
	draw_tangle(102,191);

	memcpy(UI_DIS_LINE1, vars[0].label, 3);UI_DIS_LINE1[4] = 0;
	memcpy(UI_DIS_LINE2, vars[1].label, 3);UI_DIS_LINE2[4] = 0;
	memcpy(UI_DIS_LINE3, vars[2].label, 3);UI_DIS_LINE3[4] = 0;
	disp_str(FORM15X30, SCH_XPOS,  SETPOINT_POS, UI_DIS_LINE1,SCH_COLOR,TSTAT8_BACK_COLOR);//TSTAT8_BACK_COLOR
	disp_str(FORM15X30, SCH_XPOS,  FAN_MODE_POS, UI_DIS_LINE2,SCH_COLOR,TSTAT8_BACK_COLOR);
	disp_str(FORM15X30, SCH_XPOS,  SYS_MODE_POS, UI_DIS_LINE3,SCH_COLOR,TSTAT8_BACK_COLOR);

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
    //char test_char[3];
    //memset(test_char, 0, 3);
    memcpy(UI_DIS_LINE1, vars[0].label, 3);UI_DIS_LINE1[3] = 0;
    memcpy(UI_DIS_LINE2, vars[1].label, 3);UI_DIS_LINE2[3] = 0;
    memcpy(UI_DIS_LINE3, vars[2].label, 3);UI_DIS_LINE3[3] = 0;
		
//		display_SP(vars[0].value / 100);
//		display_fanspeed(vars[1].value /1000);
//		display_mode(vars[2].value / 1000);
		
    //display_input_value(inputs[0].value);
		//display_value(inputs[0].value);
		display_screen_value( 1); // 分别用var的值显示在原先的 set fan 以及sys 地方
		display_screen_value( 2);
		display_screen_value( 3);

		//display_SP(inputs[0].value / 1000);
		//display_fanspeed(outputs[0].value / 1000);
		//display_mode(vars[0].value / 1000);
		if(Modbus.disable_tstat10_display == 0)
		{
			display_scroll();			
			display_icon();
			display_fan();
		}		
		else
		{
			disp_str(FORM15X30, 0,TIME_POS,"            ",TSTAT8_CH_COLOR,TSTAT8_MENU_COLOR2); 
			disp_null_icon(ICON_XDOTS, ICON_YDOTS, 0, FIRST_ICON_POS ,ICON_POS,TSTAT8_BACK_COLOR, TSTAT8_BACK_COLOR);
			disp_null_icon(ICON_XDOTS, ICON_YDOTS, 0, SECOND_ICON_POS ,ICON_POS,TSTAT8_BACK_COLOR, TSTAT8_BACK_COLOR);
			disp_null_icon(ICON_XDOTS, ICON_YDOTS, 0, THIRD_ICON_POS ,ICON_POS,TSTAT8_BACK_COLOR, TSTAT8_BACK_COLOR);
			disp_null_icon(ICON_XDOTS, ICON_YDOTS, 0, FOURTH_ICON_POS ,ICON_POS,TSTAT8_BACK_COLOR, TSTAT8_BACK_COLOR);
		}
		
		if(Modbus.mini_type == MINI_T10P)
		{
			if((inputs[HI_COMMON_CHANNEL].digital_analog == 1) && inputs[HI_COMMON_CHANNEL].range == R10K_40_250DegF) //如果range选的是10K type2 F 就显示 F
			{	Top_area_display(TOP_AREA_DISP_ITEM_TEMPERATURE, inputs[HI_COMMON_CHANNEL].value / 100, TOP_AREA_DISP_UNIT_F);
				
			}
			else
			{
					Top_area_display(TOP_AREA_DISP_ITEM_TEMPERATURE, inputs[HI_COMMON_CHANNEL].value / 100, TOP_AREA_DISP_UNIT_C);
			}
		}
		else
		{
			if((inputs[COMMON_CHANNEL].digital_analog == 1) && inputs[COMMON_CHANNEL].range == R10K_40_250DegF) //如果range选的是10K type2 F 就显示 F
			{	Top_area_display(TOP_AREA_DISP_ITEM_TEMPERATURE, inputs[COMMON_CHANNEL].value / 100, TOP_AREA_DISP_UNIT_F);
				
			}
			else
			{
					Top_area_display(TOP_AREA_DISP_ITEM_TEMPERATURE, inputs[COMMON_CHANNEL].value / 100, TOP_AREA_DISP_UNIT_C);
			}
		}
		
		if(count_left_key > 5) 
			disp_index = 0;
		else
			count_left_key++;

		if(disp_index == 1)
		{
			disp_str(FORM15X30, SCH_XPOS,  SETPOINT_POS, UI_DIS_LINE1,SCH_COLOR,TSTAT8_BACK_COLOR1);//TSTAT8_BACK_COLOR
			disp_str(FORM15X30, SCH_XPOS,  FAN_MODE_POS, UI_DIS_LINE2,SCH_COLOR,TSTAT8_BACK_COLOR);
			disp_str(FORM15X30, SCH_XPOS,  SYS_MODE_POS, UI_DIS_LINE3,SCH_COLOR,TSTAT8_BACK_COLOR);
		}
		else if(disp_index == 2)
		{
			disp_str(FORM15X30, SCH_XPOS,  SETPOINT_POS, UI_DIS_LINE1,SCH_COLOR,TSTAT8_BACK_COLOR);//TSTAT8_BACK_COLOR
			disp_str(FORM15X30, SCH_XPOS,  FAN_MODE_POS, UI_DIS_LINE2,SCH_COLOR,TSTAT8_BACK_COLOR1);
			disp_str(FORM15X30, SCH_XPOS,  SYS_MODE_POS, UI_DIS_LINE3,SCH_COLOR,TSTAT8_BACK_COLOR);
		}
		else if(disp_index == 3)
		{
			disp_str(FORM15X30, SCH_XPOS,  SETPOINT_POS, UI_DIS_LINE1,SCH_COLOR,TSTAT8_BACK_COLOR);//TSTAT8_BACK_COLOR
			disp_str(FORM15X30, SCH_XPOS,  FAN_MODE_POS, UI_DIS_LINE2,SCH_COLOR,TSTAT8_BACK_COLOR);
			disp_str(FORM15X30, SCH_XPOS,  SYS_MODE_POS, UI_DIS_LINE3,SCH_COLOR,TSTAT8_BACK_COLOR1);
		}
		else 
		{
			disp_str(FORM15X30, SCH_XPOS,  SETPOINT_POS, UI_DIS_LINE1,SCH_COLOR,TSTAT8_BACK_COLOR);//TSTAT8_BACK_COLOR
			disp_str(FORM15X30, SCH_XPOS,  FAN_MODE_POS, UI_DIS_LINE2,SCH_COLOR,TSTAT8_BACK_COLOR);
			disp_str(FORM15X30, SCH_XPOS,  SYS_MODE_POS, UI_DIS_LINE3,SCH_COLOR,TSTAT8_BACK_COLOR);
		}

        //sprintf(test_char, "%d", SSID_Info.IP_Wifi_Status); //测试用，在屏幕左上角 显示 wifi状态的数值;
        //disp_str(FORM15X30, 0, 0, test_char, SCH_COLOR, TSTAT8_BACK_COLOR);

        if (SSID_Info.IP_Wifi_Status == WIFI_NORMAL) //在屏幕右上角显示wifi的状态
        {
            disp_icon(26, 26, wificonnect, 210, 0, TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
        }
         //  
        else if (SSID_Info.IP_Wifi_Status == WIFI_NO_WIFI || SSID_Info.IP_Wifi_Status == WIFI_NONE)
        {
            disp_null_icon(26, 26, 0, 210, 0, TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
        }
        else
            disp_icon(26, 26, wifinocnnct, 210, 0, TSTAT8_CH_COLOR, TSTAT8_BACK_COLOR);
}


extern uint8_t item_to_adjust;
void MenuIdle_keycope(uint16 key_value)
{
    uint8 i;
    uint8 temp_value = 0;
	switch(key_value /*& KEY_SPEED_MASK*/)
	{
		case 0:
			break;
		case KEY_UP_MASK:
			count_left_key = 0;
			
			if((disp_index >= 1) && (disp_index <= 3))
			{
				if ((vars[disp_index - 1].range >= 101) && (vars[disp_index - 1].range <= 103))  // 101 102 103 	MSV range
				{
					if (vars[disp_index - 1].range == 101)  //判断range 是不是多态，是的话 调整多态的值;
					{
						for (i = 0; i < 8; i++)
						{
							if (vars[disp_index - 1].value / 1000 == msv_data[disp_index - 1][i].msv_value)
							{
								temp_value = i;
								break;
							}
						}

						for (i = temp_value; i < 7; i++)
						{
							if(strlen(msv_data[disp_index - 1][i + 1].msv_name) != 0)
							{
								vars[disp_index - 1].value = msv_data[disp_index - 1][i + 1].msv_value * 1000;
								break;
							}
						}
					}
					else
					{
						if(vars[disp_index - 1].value < STR_MSV_MULTIPLE_COUNT * 1000)
							vars[disp_index - 1].value = vars[disp_index - 1].value + 1000;
						else
							vars[disp_index - 1].value = 0;
					}
				}
				else
				{
					if(vars[disp_index - 1].value < 99 * 1000)
							vars[disp_index - 1].value = vars[disp_index - 1].value + 1000;
						else
							vars[disp_index - 1].value = 0;
				}
			}

			write_page_en[VAR] = 1;
			ChangeFlash = 1;
			break;
		case KEY_SPEED_10 | KEY_UP_MASK:	
			break;

		case KEY_DOWN_MASK:
			count_left_key = 0;
			
			if((disp_index >= 1) && (disp_index <= 3))
			{
				if ((vars[disp_index - 1].range >= 101) && (vars[disp_index - 1].range <= 103))  // 101 102 103 	MSV range
				{
					if (vars[disp_index - 1].range == 101)  //判断range 是不是多态，是的话 调整多态的值;
					{
							for (i = 0; i < 8; i++)
							{
									if (vars[disp_index - 1].value / 1000 == msv_data[disp_index - 1][i].msv_value)
									{
											temp_value = i;
											break;
									}
							}

							for (i = temp_value; i > 0; i--)
							{
									if (strlen(msv_data[disp_index - 1][i - 1].msv_name) != 0)
									{
											vars[disp_index - 1].value = msv_data[disp_index - 1][i - 1].msv_value * 1000;
											break;
									}
							}
					}
					else
					{
						if(vars[disp_index - 1].value > 1000)
							vars[disp_index - 1].value = vars[disp_index - 1].value - 1000;
						else
							vars[disp_index - 1].value = STR_MSV_MULTIPLE_COUNT * 1000;
					}
				}
				else
				{
//					if(vars[disp_index - 1].value > 1000)
							vars[disp_index - 1].value = vars[disp_index - 1].value - 1000;
//						else
//							vars[disp_index - 1].value = 99 * 1000;
				}
			}
		
			write_page_en[VAR] = 1;
			ChangeFlash = 1;
			break;		
		case KEY_SPEED_10 | KEY_DOWN_MASK: 

			break;
		
		case KEY_LEFT_MASK:
			// change SETP, FAN , SYS
			if(disp_index < 3) disp_index++;
			else 
				disp_index = 1;
			flag_left_key = 1;
			count_left_key = 0;
			break;
		case KEY_RIGHT_MASK:
			// go into main menu
			//vars[19].value += 1000;
			break;
		case KEY_LEFT_RIGHT_MASK:
			update_menu_state(MenuMain);
			break;
		default:
			break;
	}
}



