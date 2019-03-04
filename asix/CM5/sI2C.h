#ifndef __SI2C_H__
#define __SI2C_H__

#include "types.h"

void sI2C_start();
void sI2C_stop();
U8_T sI2C_ACK( void );
void sI2C_write( U8_T ch );
U8_T sI2C_read( void );


#endif
