#ifndef TIMESYNC_H
#define TIMESYNC_H

#include "types.h"

extern U8_T flag_stop_timesync;
extern U16_T timesync_period;
extern U8_T flag_update_tstat;
//extern uint8_t flag_update_panel;
extern U8_T flag_master_panel;
extern U8_T check_lost_master;

void SYNC_Init(void);
void TimeSync_Tstat(U8_T port);
void check_time_sync();
void TimeSync_Panel(void);
U8_T TimeSync_Heart(void);
U8_T TimeSync(void);
U8_T TimeSync_Scan(void);

#endif