#include "main.h"
#include "pic.h"
#include "sI2C.h"


#if 1
unsigned char pic_type=0;	// 0 - no PIC
						// 1 - original PIC
						// 2 - PIC with interrupt, and new start condition
						// 3 - PIC with 8 AD
unsigned char pic_version;

//extern bit pic_exists;


//signed int Filter(unsigned char channel,signed input);
unsigned int Filter(unsigned char channel,signed int input);


unsigned int data_buf[2]=0;
//unsigned int checksum;

extern U8_T flag_protect_lcd; 


bit read_pic( char channel)
{
	i2c_pic_start();
	i2c_pic_write(READ_CHAN0 + channel);
	DELAY_Us(10);
	if (GET_ACK())
	{	 
		sI2C_stop();
		Test[38]++;
		return 0;
	}
	DELAY_Us(10); 
	data_buf[0] = i2c_pic_read();
	GIVE_PIC_ACK();
	DELAY_Us(10);
	data_buf[1] = i2c_pic_read();
//	GIVE_PIC_ACK();
	DELAY_Us(10);
	sI2C_stop();
	//checksum = (data_buf[0] + data_buf[1] + data_buf[2] + data_buf[3])& 0xFFFF;
	if (data_buf[1] == /*checksum*/250)
	{
		//if(protocal <= TCP_IP)
		  Test[39]++;
		
		
			AI_Value[channel] = Filter(channel,data_buf[0]);//Filter(channel,data_buf[0] / 3);
//		else 			
//			inputs[channel].value = Filter(channel,data_buf[0]);//Filter(channel,data_buf[0] / 3);
		
			
		return 1;
	}
	else
	{
		return 0;
	}


}

#if 0
// MDF 12/01/04
// Reads version from the PIC chip.  
bit read_pic_version( void)
{

	unsigned int temp_version, temp_check;

	pic_version = 2;

//	EA = 0;
	ET2=0;
	i2c_pic_start();
	
	i2c_pic_write( READ_VERSION );


	if (GET_ACK())
	{
		sI2C_stop();
//		EA = 1;
//		ET2=1;
		pic_version = 0;
 
		return 0;
	}
	DELAY_Us(10);


	temp_version = i2c_pic_read();

	GIVE_PIC_ACK();
	DELAY_Us(10);
	temp_check = i2c_pic_read();

	sI2C_stop();

//	EA = 1;
//	ET2=1;
	
	if (temp_check == C_VER_CHECKSUM)
	{
		pic_version = temp_version & 0x0ff;
		//E2prom_Write_Byte( EEP_PIC_VERSION,pic_version);
 		//printf("version = %d\r\n",(int)pic_version);
		return 1;
	}
	else
	{
		pic_version = 0;
		//E2prom_Write_Byte( EEP_PIC_VERSION,pic_version);
		//printf("version = %d\r\n",(int)pic_version);
		return 0;
	}

}

#endif
/*;***********************************/
/*;i2c_startup sequence of 24Cxx*/
void i2c_pic_start()
{
	// PIC chip requires a special double start condition in order
	// to initiate communication.  This is to insure the PIC does not 
	// read any false starts.
	// MDF 12/01/04
	
	// 1st start condition
	I2C_SDA = 0;
	DELAY_Us(30);
	I2C_SCL = 0;
	DELAY_Us(15);
	// reset bus
	I2C_SDA = 1;
	I2C_SCL = 1;
	DELAY_Us(15);

	// 2nd start condition
	I2C_SDA = 0;
	DELAY_Us(30);
	I2C_SCL = 0;
//	DELAY_Us(20);
}




void GIVE_PIC_ACK( void )
{
 	int j=0;
	// Wait until the data signal goes high
	while (!I2C_SDA){
		j++;
		// If no clock, exit i2c_read routine
		if (j == 500)
			return;
	}
	// Bring the data line low
    I2C_SDA=0;
	// Pulse the clock
    I2C_SCL=1;
    DELAY_Us (1);
    I2C_SCL=0;
	// Bring the data line back high
    I2C_SDA=1;
	DELAY_Us (1);
}

/*;************************************/
/*;send a 8-bit data to 12F675 */
void i2c_pic_write( unsigned char ch )
{
	unsigned char i = 8;
	do
	{    
		DELAY_Us(50);
		I2C_SDA = ( ch & 0x80 );
		I2C_SCL=1;
		DELAY_Us(50);
		if(i > 1) I2C_SCL=0;
		ch<<=1;

	} while( --i != 0 );	
	I2C_SDA=1;
	//I2C_SCL=0;
}


/*;**************************************/
/*;receive a 8-bit data from 12F675 */
int i2c_pic_read( void )
{
	unsigned char i;
	int data1 = 0;

	for( i=0; i<16; i++ )
	{
		I2C_SCL = 1;
		DELAY_Us(1);
		data1 = ( data1 << 1 ) | I2C_SDA;
		I2C_SCL = 0;
		DELAY_Us(1);
	}

	return data1;
}


/*;**************************************/
/*;detect the ACK signal to 24Cxx*/	
// returns a 1 for no ACK.  returns a 0 for successful ACK		
unsigned int GET_ACK( void )
{
    //unsigned char c=0;
    unsigned char i;
 

	//if (pic_version) // only newer versions of the pic have the 2nd start condition.
	{
	    // MDF 12/01/04
	    // Wait for data line to be pulled low.
	    for (i=0; i<100; i++)
	    {
			I2C_SDA = 1;
			//c=I2C_SDA;
			if (I2C_SDA == 0){
			// if data line is low, pulse the clock.
			    I2C_SCL=1;
				DELAY_Us(10);
			    I2C_SCL=0;
				return 0;
			}		
	    }
	    I2C_SCL=0;
	    return 1;
	}

	/*else
	{
	    I2C_SDA=1;
	    I2C_SCL=1;
	    for (i=0; i<10; i++)
	    {
			c=I2C_SDA;
			if (c == 0){
			    I2C_SCL=0;
				return 0;
			}		
	    }
	    I2C_SCL=0;
	    return 1;
	}*/
}


#endif

