
#ifndef	CRCXX_H

#define	CRCXX_H

#include "../CPU/types.h"

extern U8_T MainCRChi;
extern U8_T MainCRClo;

extern U8_T SubCRChi;
extern U8_T SubCRClo;

extern U8_T EthernetCRChi;
extern U8_T EthernetCRClo;

void main_init_crc16(void);
void sub_init_crc16(void);
void ethernet_init_crc16(void);
void main_crc16_byte(U8_T ch);
void sub_crc16_byte(U8_T ch);
void ethernet_crc16_byte(U8_T ch);
U16_T crc16(U8_T *p, U8_T length);

#endif

