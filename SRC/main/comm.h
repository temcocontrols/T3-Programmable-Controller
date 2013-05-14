#include "types.h"

/* communication task */
/*
	from master to slaver
	1. 	SEND output led status
	2.	SEND input type : 0-3.3v / therimsiter / 0 - 10v / 0-20ma
	3. 	GET	input value
    4. 	GET output switch status

*/
/* define command for communication with slaver */

typedef  enum 
{
	/* 	send	*/
	C_INITIAL = 0,

	S_OUTPUT_LED = 0x10, /* 0x10 + 24 bytes */
	S_INPUT_TYPE = 0x11, /* 0x11 + 32 bytes */

	G_SWTICH_STATUS = 0x20,	/* 0x12 + 24 bytes */
	G_INPUT_VALUE = 0x21,	/* 0x13 + 32 bytes */
	
	C_END = 255

};

/* command for communication , pls refer to detailed protocal */

typedef struct
{
	U8_T type;		/* type for current command */
	U8_T len; 		/* data number for sending or receiving */
	U8_T buf[32];  /* buffer for sending or receiveing */
	U8_T index; 	/* index for current bytes of sending or receiving */
	U8_T flag;    	/* flag for whether finish the current sending or receiving */
}STR_CMD;




void SPI_task(void);
void vStartSPITasks( unsigned char uxPriority);

