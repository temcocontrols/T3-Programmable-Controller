
#ifndef _FATFS_H_

#define _FATFS_H_

#include "types.h"


#define IsUpper(c)	(((c)>='A')&&((c)<='Z'))
#define IsLower(c)	(((c)>='a')&&((c)<='z'))
#define IsDigit(c)	(((c)>='0')&&((c)<='9'))

extern U8_T sd_status;

void vStartFatfsTask(U8_T uxPriority);


#endif