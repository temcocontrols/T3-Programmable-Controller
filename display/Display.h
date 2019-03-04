#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "types.h"

void vStartDisplayTasks(U8_T uxPriority);
void Display_Check_Task(void) reentrant;
void Display_Process(void) reentrant;


void Display_Refresh(void);
void Display_Clear_Space(void);
void Display_Clear_Screen(void);
void Display_String(U8_T line, U8_T pos, S8_T* str,U8_T mode,U8_T format);

void Display_Initial_Data(void);
void Refreash_Display_Task(void);
void Display_IP(void);


void Display_Str(char line,char pos,char* str,char len);
void Display_Idle(unsigned char index);
void Display_Menu(unsigned char index);
void Display_Output(unsigned char index);
void Display_Input(unsigned char index);
void Display_Sensor(unsigned char index);
void Display_Tstat_Temp(unsigned char index);
void Display_Tstat_Setpoint(unsigned char index);
void Display_Tstat_Mode(unsigned char index);
void Display_Set_Mode(void);
void Display_Set_Pri(void);


#endif



