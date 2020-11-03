#ifndef ALARM_H
#define ALARM_H

#include "types.h"


#define CUS_ALARM  0
// it is generated in progrom which is writen by customer
// structure : panel + sn + CUS_ALRAM + " message ... "


#define IN_ALARM  1
// input short or open
// structure : panel + sn + CUS_ALRAM + " message ... "


#define ID_ALARM  2
// ID of sub device are conflict


//typedef struct {

//	U8_T 	panel;
//	U8_T 	cmd	; // 0- cusmtomer 1- IN OPEN OR SHOTR 2 - ID CONFILICT
//	U8_T  index;
//	U8_T	del;
//	
//	U32_T 	alarm_time;
//	S8_T 	  alarm_count;
//	S8_T    alarm_message[ALARM_MESSAGE_SIZE+1];
//  U8_T reserved [10];
//} Alarm_point;


#define VIRTUAL_ALARM 0
#define TEMPERATURE   1
#define GENERAL_ALARM 2
#define PRINTER_ALARM 10

//#define ALARM_VERYLOW 1
//#define ALARM_LOW     2
//#define ALARM_HI      3
//#define ALARM_VERYHI  4

#define ALARM_LOST_TOP 1
#define ALARM_AO_FB     2
#define ALARM_SNTP_FAIL  3
#define ALARM_LOST_PIC   4
#define ALARM_ABNORMAL_SD 5
#define ALARM_PROGRAM		6
#define DNS_FAIL			7


S16_T putmessage(S8_T *mes, S16_T prg, S16_T panel, S16_T type, S8_T alarmatall,S8_T indalarmpanel,S8_T *alarmpanel, S16_T j);
S16_T checkforalarm(S8_T *mes, S16_T prg, S16_T panel, S16_T id, S16_T *free_entry);
S16_T generatealarm(S8_T *mes, S16_T prg, S16_T panel, S16_T type, S8_T alarmatall,S8_T indalarmpanel,S8_T *alarmpanel,S8_T printalarm);
void dalarmrestore(S8_T *mes, S16_T prg, S16_T panel);
void check_input_alarm(void);
//void generate_alarm_lost_top(void);
//void generate_read_AO_FeedBack(void);

void generate_common_alarm(U8_T index);
void generate_program_alarm(U8_T type,U8_T prg);


#endif

