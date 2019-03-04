#include "constcode.h"
#include "types.h"
//#include "LibDisplay.h"


//#include "constcode.h"
//DISPTEMP const fan_speed_4c[5] = {
//	{0xC3,"OFF"},
//	{0xC3,"HEAT"},
//	{0xC3,"COOL"},
//	{0xC3,"GAS"},
//	{0xC3,"AUTO"}, 
//} ;

//uint8 const poll_co2_cmd[3][8] = 
//{
//	{0xff, 0x03, 0x00, 0x6b, 0x00, 0x01, 0xe0, 0x08},	//reg 107:  temperature
//	{0xff, 0x03, 0x00, 0x6c, 0x00, 0x01, 0x31, 0xc9},	//reg 108:humidity
//	{0xff, 0x03, 0x00, 0x6d, 0x00, 0x01, 0x00, 0x09}	//reg 109:co2	
//};
//#define BAUDRATE_9600						0
//#define BAUDRATE_19200					1
//#define BAUDRATE_38400          2
//#define BAUDRATE_57600					3
//#define BAUDRATE_115200					4
//#define BAUDRATE_76800					5
//#define BAUDRATE_1200						6
//#define BAUDRATE_4800						7
//#define BAUDRATE_14400					8
unsigned int short const baudrate[9]={96,192,384,576,1152,768,12,48,144};

uint8 const poll_co2_cmd[9]={0xFF, 0x01, 0x86, 0x00,0x00,0x00,0x00,0x00,0X79};

uint8 const code tem_unit[2] = {'C','F'};


uint16  const code table1_custom_default[ ] =
   {150,85,60,45,35,25,15,5,65531,65521,65486};

uint16  const code table2_custom_default[ ] =
   {0,300,600,900,1200,1500,1800,2100,2400,2700,3000};

uint8 const code default_valve_table[21] =    // [mode_operation]
{
	// 8bit

	//XX XX XX XX
	//v4 v3 v2 v1

//XX	00		01				10				11
//		0%		0-100%			50-100%			100%
//
//  coast, 	cool1,	cool2,	cool3,	heat1,	heat2,	heat3
	0x00,	0x11,	0x33,	0x33,	0x14,	0x3c,	0x3c,   // PID1 ON ANALOG OUTPUT_1TO5 TABLE
	0x00,	0x11,	0x33,	0x33,	0x14,	0x3c,	0x3c,   // PID2 ON ANALOG OUTPUT_1TO5 TABLE
	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00    // PID1 OFF ANALOG OUTPUT_1TO5 TABLE

} ;

uint8 const code default_pwm_table[7] =    // [mode_operation]
{
//  coast, 	cool1,	cool2,	cool3,	heat1,	heat2,	heat3
	0,		0X40,		0X30,		0X10,	0X04,		0X03,		0X01
} ;

uint8 code  fan_table[2][4] = {{0,2,2,4},{1,3,3,4}};

//uint8 const code def_tab[11] =
//			{					 // 10k termistor GREYSTONE -40 to 120 Deg.C or -40 to 248 Deg.F 
//			 192, 209, 206, 187, 161, 131, 103, 79, 61, 45, 155
//			};
//MHF:12-30-05,Added 4 values to make the tstat can measure minus degree
//uint8 const code def_tab_pic[19] =
//			{					 // 10k termistor GREYSTONE -40 to 120 Deg.C or -40 to 248 Deg.F 
//			 25, 39, 61, 83, 102, 113, 112, 101, 85, 67, 51, 38, 28, 21, 15,11, 8, 6, 23
// 			};

uint8 const code def_tab_type3[19] = //10K thernistor type 3
			{ 
			 27, 42, 62, 86, 96, 104, 103,  94, 80,  64, 51, 40, 31, 23, 17,13,10, 8, 30
			};
//uint8 const code def_tab_type3_old[15] = //10K thernistor type 3
//			{ 
//			 29, 45, 63, 81, 96, 104, 103,  94, 80, 65, 51, 40, 30, 23, 77
//			};

//uint16 const code def_tab_type50K[10] = //NTC 50K for Chen
//			{ 
//				12,13,15,14,18,18,19,24,19,791
//			 //57,57,55,52,49,45,40,36,31,27,24,21,18,15,13,11,10,8,57
//			};



//uint8 const code PWM_def_tab[51] =
//			{				
//				5,4,5,5,5,6,6,6,7,7,8,8,8,9,10,10,11,11,13,13,15,16,17,18,21,26,30,35,35,36,41,43,46,47,47,43,40,36,33,29,27,25,23,21,19,19,17,16,15,15,14
//			};
#ifndef TSTAT7_ARM
uint8 const code LCD_LINE1[15][9]=
{
	"        ",
	"  TEMP  ",
	"SETPOINT",
	" INPUT1 ",
	" INPUT2 ",
	" INPUT3 ",
	" INPUT4 ",
	" INPUT5 ",
	" INPUT6 ",
	" INPUT7 ",
	" INPUT8 ",
	" MODE   ",
	" USER   ",
	"CO2  ppm",
	"HUMIDITY"

};

DISPTEMP const code ion[4] = {
{0xC3,"LOF"},
{0xC3,"LON"},
{0xC3,"POF"},
{0xC3,"PON"},
};


// Display fan speed stages on LCD
DISPTEMP const code clock_month[12] =
{
{0xC3,"Jan"}, 											
{0xC3,"Feb"}, 											
{0xC3,"Mar"},											
{0xC3,"Apr"}, 											
{0xC3,"May"},											
{0xC3,"Jun"},										
{0xC3,"Jul"},											
{0xC3,"Aug"}, 											
{0xC3,"Sep"},											
{0xC3,"Oct"},
{0xC3,"Nov"},											
{0xC3,"Dec"},
};

DISPTEMP const code clock_week[7] =
{
{0xC3,"Mon "}, 											
{0xC3,"Tues"}, 											
{0xC3,"Wed "},											
{0xC3,"Thur"}, 											
{0xC3,"Fri "},											
{0xC3,"Sat "},										
{0xC3,"Sun "},											
};
DISPTEMP const code fan_speed[6] = {
	{0xC3,"OFF"},
	{0xC3,"-1-"},
	{0xC3,"-2-"},
	{0xC3,"-3-"},
	{0xC3,"AUTO"},
	{0xC3,"ON"},
 
} ;
//Menu information on LCD display

MENUNAME const menu[MAX_MENU_NUM] = {//Added setpoint and fan mode to menu
{"Modbus","Address", 254 , 0},
{"IP","",255,0},
{"Temperatur","Calibrate", 500,-500},		//00 CAL single point calibration of temperature
{"Temperatur","Select",500,-500},		//01 Temperature Sensor Select
{"Temperatur","Filter", 500,-500},		//02 Temperature filter
{"Analog","Input1",500,-500},		//03 Analog input 1
{"Analog","Input2",500,-500},		//04 Analog input 2
//{0x83,"dI1"},		//05 Digital input 1  //cc not digital input any more
//{"DigitalOut","Calibrate"},		//07 Digital to analog output calibration
{"Baudrate","Select ",9,0},		//08 Baudrate 

{"Degree","C/F",1,0},		//17 Degree C or degree F
{"NightHeat","Deadband",500,-500},		//19 Night heating deadband

{"NightCool","Deadband",500,-500},		//20 Night cooling deadband
{"NightHeat","Setpoint",500,-500},		//21 Night heating setpoint
{"NightCool","Setpoint",500,-500},		//22 Night cooling setpoint	//NOT USE

{"PowerUp","Setpoint",500,-500},		//24 Power up cooling setpoint
{"PowerUp","On/Off",1,0},		//25 Power up on or off


{"MenuLock","mode",1,0},		//32 Menu lock mode
{"MODBUS","BACNET",1,0},

//{"",""},
//{"",""},
//{"",""},
//{"",""},
//{"",""},
//{"",""},
//{"",""},

//{"","Workday"},
//{"","Workday"},
//{"","Workday"},
//{"","Workday"},
//{"","Weekend"},
//{"","Weekend"},
//{"","Weekend"},
//{"Protocol","Select"},

//{"Save as","Default"},		//35 Store current set as factory default
//{"Factory","Default"},		//36 Factory default


//{"Modbus","Address"},		//37 Addrerss
////when LOCK=3,the user have to into menu to adjust temperature setpoiont and fan mode
//{"Setpoint"," "},		//56 Setpoint

//{"Fan","Speed"},		//57 Fan speed
}; 


//DISPTEMP  const code menu[TOTAL_MENU_PARAMETERS + 1 + 2 + 1] = {//Added setpoint and fan mode to menu
//{0x83,"CAL"},		//00 CAL single point calibration of temperature
//{0x83,"tSS"},		//01 Temperature Sensor Select
//{0x83,"FIL"},		//02 Temperature filter
//{0x83,"AI1"},		//03 Analog input 1
//{0x83,"AI2"},		//04 Analog input 2
////{0x83,"dI1"},		//05 Digital input 1  //cc not digital input any more
//{0x83,"Ort"},		//06 Override timer
//{0x83,"dAC"},		//07 Digital to analog output calibration
//{0x83,"bAU"},		//08 Baudrate 
//{0x83,"dSC"},		//09 Short cycle delay
//{0x83,"dCH"},		//10 Changeover delay
//{0x83,"PPr"},		//11 Proportional term
//{0x83,"PIn"},		//12 Integral term
//{0x83,"SOP"},		//13 Different sequence of operations

//{0x83,"HC"},		//14 Heating and cooling config
//{0x83,"Cdb"},		//15 Cooling deadband
//{0x83,"Hdb"},		//16 Heating deadband
//{0x83,"C_F"},		//17 Degree C or degree F
//{0x83,"FAn"},		//19 Select fan speed number
//{0x83,"nHd"},		//19 Night heating deadband

//{0x83,"nCd"},		//20 Night cooling deadband
//{0x83,"nSp"},		//21 Night heating setpoint
//{0x83,"nHSH"},		//21 Night heating setpoint	//NOT USE
//{0x83,"nCS"},		//22 Night cooling setpoint	//NOT USE
//{0x83,"nCSH"},		//22 Night cooling setpoint	//NOT USE

//{0x83,"APP"},		//23 Application mode ,hotel or office
//{0x83,"POS"},		//24 Power up cooling setpoint
//{0x83,"POn"},		//25 Power up on or off

//{0x83,"PAd"},		//26 Keypad select
//{0x83,"AUt"},		//27 Allow user to set auto fan or not
//{0x83,"OU1"},		//28 Calibration analog output channel 1
//{0x83,"OU2"},		//29 Calibration analog output channel 2
//{0x83,"SHI"},		//30 Max setpoint
//{0x83,"SLO"},		//31 Min setpoint

//{0x83,"LOC"},		//32 Menu lock mode
//{0x83,"dIS"},		//33 Led display mode
//{0x83,"Vtt"},		//34 Full valve travel time
//{0X82,"485/ZGB"},
//{0x83,"DEMAND"},

//{0x83,""},
//{0x82,""},
//{0x83,""},
//{0x83,""},
//{0x83,""},
//{0x82,""},
//{0x82,""},

//{0x81,"Workday"},
//{0x81,"Workday"},
//{0x81,"Workday"},
//{0x81,"Workday"},
//{0x81,"Weekend"},
//{0x81,"Weekend"},
//{0x81,"Weekend"},
//{0x81,"Protocol"},

//{0x83,"DEF"},		//35 Store current set as factory default
//{0x83,"FAC"},		//36 Factory default


//{0x83,"Add"},		//37 Addrerss
////when LOCK=3,the user have to into menu to adjust temperature setpoiont and fan mode
//{0x83,"SEt"},		//38 Setpoint

//{0x83,"FAS"},		//39 Fan speed
// 

//}; 

uint8 const code SP_ITEM[3][8]=
{
" TEM_SP ",
" CO2_SP ",
" HUM_SP "

};


uint8 const code LCD_LINE2[9][9]=
{ 
	"        ",
	"      C ",
	"      F ",
	"      % ",
	"      RH",
	"     AMP",
	"      KW",
	"      KA",
	"     ppm",
};
uint8 const code MODE[8][9]=
{ 
	"COASTING",
	"COOLING",
	"HEATING",
	"FAN OFF",
	"FAN -1-",
	"FAN -2-",
	"FAN -3-",
	"FAN AUTO",
};

#endif //TSTAT7_ARM



#ifdef SANJAY
uint8 const code extern_operation_sop1[ ] = 
//		Coast Cool1 Cool2 Cool3 Heat1 Heat2 Heat3
	{	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 , //Fan0
		0x00, 0x05, 0x05, 0x05, 0x00, 0x00, 0x00 , //Fan1  //cooling output + fan_low on
		0x00, 0x09, 0x09, 0x09, 0x00, 0x00, 0x00 , //Fan2  //cooling output + fan_med on
		0x00, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00 , //Fan3  //cooling output + fan_high on
		0x00, 0x41, 0xca, 0xcc, 0x41, 0xd2, 0xd4 , //Fan4
		0x00, 0x41, 0xca, 0xcc, 0x41, 0xd2, 0xd4   //only fan auto ,use for universal PID
    };
#else
uint8 const code extern_operation_sop1[ ] = 
//		Coast Cool1 Cool2 Cool3 Heat1 Heat2 Heat3
	{	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 , //Fan0
		0x01, 0x01, 0x09, 0x09, 0x01, 0x01, 0x11 , //Fan1
		0x02, 0x02, 0x0a, 0x0a, 0x02, 0x02, 0x12 , //Fan2
		0x04, 0x04, 0x0c, 0x0c, 0x04, 0x04, 0x14 , //Fan3
		0x01, 0x41, 0xca, 0xcc, 0x41, 0xd2, 0xd4 , //Fan4
		0x00, 0x41, 0xca, 0xcc, 0x41, 0xd2, 0xd4   //only fan auto ,use for universal PID
    };
#endif
uint8 const code extern_operation_customer[ ] = 
//		Coast Cool1 Cool2 Heat1 Heat2 Spare Spare
	{	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 , //Fan0
		0x00, 0x11, 0x31, 0x03, 0x07, 0x00, 0x00 , //Fan1
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 , //Fan2
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 , //Fan3
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 , //Fan4
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00   //only fan auto ,use for universal PID
    };

uint8 const code extern_operation_default[ ] = 	 //standar 
//		Coast Cool1 Cool2 Heat1 Heat2 Spare Spare
	{	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 , //Fan0
		0x00, 0x09, 0x19, 0x03, 0x07, 0x00, 0x00 , //Fan1
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 , //Fan2
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 , //Fan3
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 , //Fan4
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00   //only fan auto ,use for universal PID
    };


uint8 const code extern_operation_sop4[ ] = 	 //heat	pump
//		Coast Cool1 Cool2 Heat1 Heat2 Spare Spare
	{	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 , //Fan0
		0x00, 0x09, 0x19, 0x05, 0x07, 0x00, 0x00 , //Fan1
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 , //Fan2
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 , //Fan3
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 , //Fan4
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00   //only fan auto ,use for universal PID
    };

uint8 const code extern_operation_sop5[ ] = 	//cool pump
//		Coast Cool1 Cool2 Heat1 Heat2 Spare Spare
	{	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 , //Fan0
		0x00, 0x0d, 0x1d, 0x01, 0x03, 0x00, 0x00 , //Fan1
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 , //Fan2
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 , //Fan3
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 , //Fan4
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00   //only fan auto ,use for universal PID
    };


uint8 const code OUTPUT_DELAY_TABLE[2][7]= 
{
  {0,0,0,0,0,0,0},			      // 1 = 0.1s
  {0,0,0,0,0,0,0}
};


//////////////////////////////////////////////////////////////////////////////////////////////
//uint16 const code tbl_hummity_freq[10][2]={
//		{HUMCOUNT1_H,HUMRH1_H},
//		{HUMCOUNT2_H,HUMRH2_H},
//		{HUMCOUNT3_H,HUMRH3_H},
//		{HUMCOUNT4_H,HUMRH4_H},
//		{HUMCOUNT5_H,HUMRH5_H},
//		{HUMCOUNT6_H,HUMRH6_H},
//		{HUMCOUNT7_H,HUMRH7_H},
//		{HUMCOUNT8_H,HUMRH8_H},	
//		{HUMCOUNT9_H,HUMRH9_H},
//		{HUMCOUNT10_H,HUMRH10_H}
//};
///////////////////////////////////////////////////////////////////////////////////////////////
//uint8 const code user_message[2][8] = {
//	{0x20,0x20,0x55,0x73,0x65,0x72,0x20,0x20},	//==User
//	{0x20,0x4D,0X65,0X73,0X73,0X61,0x67,0x65}	//==Message
//};



