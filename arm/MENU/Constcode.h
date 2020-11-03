

#include "types.h"

#define MAX_MENU_NUM 3


typedef struct
{
	uint8 first_line[10];
	uint8 second_line[10];
	int16 max;
	int16 min;
}MENUNAME;

typedef struct  {
uint8 location;
uint8 name[9];
}DISPTEMP;

#define TOTAL_MENU_PARAMETERS 10

//extern uint8 const poll_co2_cmd[3][8];
extern unsigned int short const baudrate[9];
extern uint8 const poll_co2_cmd[9];
extern  DISPTEMP const code ion[4];

extern  DISPTEMP const code clock_week[7];
extern  DISPTEMP const code clock_month[12];
extern  DISPTEMP const code fan_speed[6];

//extern  DISPTEMP  const code menu[TOTAL_MENU_PARAMETERS + 1 + 2 + 1 ];
extern MENUNAME const menu[MAX_MENU_NUM];
extern unsigned int short  const code table1_custom_default[ ];
extern unsigned int short  const code table2_custom_default[ ];

extern uint8 const code extern_operation_sop1[ ];
extern uint8 const code extern_operation_customer[ ];
extern uint8 const code extern_operation_default[ ];
extern uint8 const code extern_operation_sop4[ ];
extern uint8 const code extern_operation_sop5[ ];
extern uint8 const code default_valve_table[21];
extern uint8 const code default_pwm_table[7];

extern uint8 const code default_valve_table[21];
extern uint8 const code default_pwm_table[7];

extern uint8 code  fan_table[2][4] ;

extern uint8 const code def_tab[11];
extern uint8 const code def_tab_pic[19];
extern uint8 const code def_tab_type3[19] ;
extern unsigned int short const code def_tab_type50K[19];

//extern uint8 const code PWM_def_tab[51] ;
extern uint8 const code SP_ITEM[3][8];
extern uint8 const code LCD_LINE1[15][9];
extern uint8 const code LCD_LINE2[9][9];
extern uint8 const code MODE[8][9];





