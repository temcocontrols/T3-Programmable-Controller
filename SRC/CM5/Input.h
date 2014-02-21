#ifndef __INPUT_H__
#define __INPUT_H__

#include "types.h"

#define DI_NONE 0
#define DI_TSTAT 1
#define DI_SWITCH 2
//void vStartUpdateInputTasks( U8_T uxPriority);
void initial_input_value(void);
void Update_AI_Task(void);
void Sampel_DI_Task(void);
void Update_DI_Task(void);
void Update_AI(void);




#endif
