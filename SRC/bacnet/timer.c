/**************************************************************************
*
* Copyright (C) 2007 Steve Karg <skarg@users.sourceforge.net>
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*********************************************************************/
#include <stdint.h>
#include "main.h"
//#include "hardware.h"

/* This module is a 1 millisecond timer */
#if 0
/* Prescaling: 1, 8, 64, 256, 1024 */
#define TIMER_PRESCALER 64
/* Count: Timer0 counts up to 0xFF and then signals overflow */
#define TIMER_TICKS (F_CPU/TIMER_PRESCALER/1000)
#if (TIMER_TICKS > 0xFF)
#error Timer Prescaler value is too small
#endif
#define TIMER_COUNT (0xFF-TIMER_TICKS)

#endif
/* Global variable millisecond timer - used by main.c for timers task */
//volatile uint16_t Timer_Milliseconds = 0;
/* MS/TP Silence Timer */
volatile uint16_t SilenceTime;

/* Configure the Timer */
void Timer_Initialize(   void)
{
  
}


/* Public access to the Silence Timer */
uint16_t Timer_Silence(void)
{ 
    return SilenceTime;
}

/* Public reset of the Silence Timer */
void Timer_Silence_Reset( void)
{
	SilenceTime = 0;
}						  


