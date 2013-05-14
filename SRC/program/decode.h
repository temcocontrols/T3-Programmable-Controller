#ifndef DECODE_H
#define DECODE_H


#define MAX_PRGS  10
#define PROGRAMS_POOL_SIZE	0x2800


typedef struct
{
	unsigned char number;
	unsigned char point_type;
}	Point;


typedef struct
{

	char description[21]; 	      	  /* (21 bytes; string)*/
	char label[9];			  /* (9 bytes; string)*/

	unsigned long bytes;		/* (2 bytes; size in bytes of program)*/

	unsigned char on_off;
	unsigned char auto_manual;
	unsigned char errcode;
	unsigned char unused;

#if 0
	unsigned on_off		      : 1; /* (1 bit; 0=off; 1=on)*/
	unsigned auto_manual	  : 1; /* (1 bit; 0=auto; 1=manual)*/
	unsigned com_prg		    : 1; /* (6 bits; 0=normal use, 1=com program)*/
	unsigned errcode	      : 5; /* (6 bits; 0=normal end, 1=too long in program)*/
	unsigned unused         : 8;
#endif
}	Str_program_point;	  /* 21+9+2+3+2 = 37 bytes*/

void Test_program(void);
void control_logic(void);

int exec_program(int current_prg, unsigned char *prog_code);
void push(long value);
long pop(void);
void pushlong(unsigned long value);
unsigned long poplong(void);
long veval_exp(unsigned char *local);
long operand(char **buf,unsigned char *local);
int isdelimit(char c);
long localvalue(unsigned char *p, unsigned char *local);
int get_point_value( Point *point, long *val_ptr );


#endif


