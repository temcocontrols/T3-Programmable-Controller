/*-----------------pic.h---------------------*/
 
#define C_VER_CHECKSUM  0x69
#define READING_THRESHOLD	150

// PIC commands protocol
#define PIC16_PULSE    	 		0xB1
#define PIC16_VERSION			0xB2

#define MAX_FILTER 12

//    define command 
#define READ_VERSION    0xb2
enum
{
    READ_CHAN0 = 0XF0,
    READ_CHAN2,READ_CHAN3,READ_CHAN4,READ_CHAN5,READ_CHAN6,READ_CHAN7,READ_CHAN8,READ_CHAN9,READ_CHAN10,READ_CHAN11
};
//   end define


void pic_detect(void);
void vStartPicTasks( unsigned char uxPriority);
void Initial_Range(void);
void write_pic( unsigned char addr, unsigned char value );
unsigned int GET_ACK( void );
void GIVE_PIC_ACK( void );
void i2c_pic_start();
void i2c_pic_write( unsigned char ch );
int i2c_pic_read( void );
void i2c_stop(void);
//unsigned char  ReadPicVersion(void);
bit read_pic_version( void);
bit read_pic( char channel);














