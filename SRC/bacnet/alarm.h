#ifndef ALARM_H
#define ALARM_H


#define ALARM_MESSAGE_SIZE    58

#define VIRTUAL_ALARM 0
#define TEMPERATURE   1
#define GENERAL_ALARM 2
#define PRINTER_ALARM 10

#define ALARM_VERYLOW 1
#define ALARM_LOW     2
#define ALARM_HI      3
#define ALARM_VERYHI  4



int putmessage(char *mes, int prg, int type, char alarmatall,char indalarmpanel,char *alarmpanel, int j);
int checkforalarm(char *mes, int prg, int panel, int id, int *free_entry);
int generatealarm(char *mes, int prg, int panel, int type, char alarmatall,char indalarmpanel,char *alarmpanel,char printalarm);
void dalarmrestore(char *mes, int prg, int panel);





#endif