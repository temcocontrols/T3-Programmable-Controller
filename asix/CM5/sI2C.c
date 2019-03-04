#include "main.h"
#include "sI2C.h"

#if (defined(CM5) && (defined(OLDPIC)))

/*;***********************************/
/*;i2c_startup sequence of 24Cxx*/
void sI2C_start()
{
	I2C_SCL=0;
  	I2C_SDA = 1;
	I2C_SCL = 1;
	I2C_SDA = 0;
	I2C_SCL=0;	 
}

/*;************************************/
/*;i2c_stop sequence of 24Cxx*/
void sI2C_stop()
{
 	I2C_SDA = 0;
	I2C_SCL = 1; 
	I2C_SDA = 1;  

}

/*;**************************************/
/*;detect the ACK signal to 24Cxx*/
U8_T sI2C_ACK( void )
{
   U8_T c;
   I2C_SDA=1;
   I2C_SCL=1;
   c=I2C_SDA;
   I2C_SCL=0;	
   return c;   
}



/*;************************************/
/*;send a 8-U8_T data to 24Cxx*/
void sI2C_write( U8_T ch )
{
	U8_T i = 8;
	do
	{    
		I2C_SDA = ( ch & 0x80 );
		I2C_SCL=1;		
		I2C_SCL=0;
		ch<<=1;

	} while( --i != 0 );  
}


/*;**************************************/
/*;receive a 8-U8_T data from 24Cxx*/
U8_T sI2C_read( void )
{
	U8_T i, data1 = 0;

	for( i=0; i<8; i++ )
	{
		I2C_SCL = 1;
		data1 = ( data1 << 1 ) | I2C_SDA;
		I2C_SCL = 0;
		DELAY_Us(1);  // this is to slow down all i2c communication so that the pic chip 
					  // is able to run better. MDF 12/01/04
	}  
	return data1;
}

#endif
