/**
 * \addtogroup timer
 * @{
 */

/**
 * \file
 * Timer library implementation.
 * \author
 * Adam Dunkels <adam@sics.se>
 */

/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: timer.c,v 1.2 2006/06/12 08:00:30 adam Exp $
 */

#include "clock.h"
#include "timer.h"

/*---------------------------------------------------------------------------*/
/**
 * Set a timer.
 *
 * This function is used to set a timer for a time sometime in the
 * future. The function timer_expired() will evaluate to true after
 * the timer has expired.
 *
 * \param t A pointer to the timer
 * \param interval The interval before the timer expires.
 *
 */
void
timer_set(struct timer *t, clock_time_t interval)
{
  t->interval = interval;
  t->start = clock_time();
}
/*---------------------------------------------------------------------------*/
/**
 * Reset the timer with the same interval.
 *
 * This function resets the timer with the same interval that was
 * given to the timer_set() function. The start point of the interval
 * is the exact time that the timer last expired. Therefore, this
 * function will cause the timer to be stable over time, unlike the
 * timer_rester() function.
 *
 * \param t A pointer to the timer.
 *
 * \sa timer_restart()
 */
void
timer_reset(struct timer *t)
{
  t->start += t->interval;
}
/*---------------------------------------------------------------------------*/
/**
 * Restart the timer from the current point in time
 *
 * This function restarts a timer with the same interval that was
 * given to the timer_set() function. The timer will start at the
 * current time.
 *
 * \note A periodic timer will drift if this function is used to reset
 * it. For preioric timers, use the timer_reset() function instead.
 *
 * \param t A pointer to the timer.
 *
 * \sa timer_reset()
 */
void
timer_restart(struct timer *t)
{
  t->start = clock_time();
}
/*---------------------------------------------------------------------------*/
/**
 * Check if a timer has expired.
 *
 * This function tests if a timer has expired and returns true or
 * false depending on its status.
 *
 * \param t A pointer to the timer
 *
 * \return Non-zero if the timer has expired, zero otherwise.
 *
 */
int
timer_expired(struct timer *t)
{
  return (clock_time_t)(clock_time() - t->start) >= (clock_time_t)t->interval;
}
/*---------------------------------------------------------------------------*/

#if 0
uint32 timer_elapsed_time(
    struct timer *t)
{
 //   uint32_t now = timer_milliseconds();
    uint32 delta = 0;

    if (t) 
		{
        delta = clock_time() - t->start;
    }

    return delta;
}



static struct timer Silence_Timer;

void timer_elapsed_start_Silence_Timer(void )
{
    timer_restart(&Silence_Timer);
}


/*************************************************************************
* Description: Reset the silence on the wire timer.
* Returns: nothing
* Notes: none
**************************************************************************/
void rs485_silence_reset(
    void)
{
    //timer_elapsed_start(&Silence_Timer);
	timer_restart(&Silence_Timer);
}

/*************************************************************************
* Description: Determine the amount of silence on the wire from the timer.
* Returns: true if the amount of time has elapsed
* Notes: none
**************************************************************************/
BOOL rs485_silence_elapsed(
    uint32 interval)
{
    return (timer_elapsed_time(&Silence_Timer) >= interval);//timer_elapsed_milliseconds(&Silence_Timer, interval);
}


extern U8_T uart2_baudrate;

/*************************************************************************
* Description: Baud rate determines turnaround time.
* Returns: amount of milliseconds
* Notes: none
**************************************************************************/
static uint16 rs485_turnaround_time(
    void)
{
    /* delay after reception before transmitting - per MS/TP spec */
    /* wait a minimum  40 bit times since reception */
    /* at least 2 ms for errors: rounding, clock tick */
		uint16 turnaround_time;
		if(uart2_baudrate == 5) //UART_9600
		{
			turnaround_time = 6;
		}
		else if(uart2_baudrate == 6/*UART_19200*/)
		{
		    turnaround_time = 4;
		}
		else if(uart2_baudrate == 7/*UART_38400*/)
		{
		    turnaround_time = 3;
		}
		else
			turnaround_time = 2;
    //return (2 + ((40UL * 1000UL) / Baud_Rate));
		return turnaround_time;
}

/*************************************************************************
* Description: Use the silence timer to determine turnaround time.
* Returns: true if turnaround time has expired.
* Notes: none
**************************************************************************/
BOOL rs485_turnaround_elapsed(
    void)
{
    //return timer_elapsed_milliseconds(&Silence_Timer, rs485_turnaround_time());
		return (timer_elapsed_time(&Silence_Timer) >= rs485_turnaround_time());
}

#endif