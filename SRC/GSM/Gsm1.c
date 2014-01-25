#if 1


void SerialPort() interrupt 4
{
	if(RI0 == 1  )
	{
		// Make sure that you are not in SEND mode and that you do not exceed
		// the length of the data bufer
		if(rece_count < DATABUFLEN)
			data_buffer[rece_count++] = SBUF0;
		RI0 = 0;

		// timeout count adjusted to half a packet length
		serial_receive_timeout_count = SERIAL_RECEIVE_TIMEOUT;

		// need to evaluate the packet size
		// once the sixth byte is received we can then figure out what is packet size
		if(rece_count == 7 )
		{
			if(data_buffer[1] == READ_VARIABLES || data_buffer[1] == WRITE_VARIABLES)
				packet_size = 8;
			else if(data_buffer[1] == MULTIPLE_WRITE && response_receive_finished == 0)
				packet_size = 8;
			else if(data_buffer[1] == MULTIPLE_WRITE)
				// take the quantity amount, and add 9 more for other information needed
				packet_size = data_buffer[6] + 9;
			else
				packet_size = DATABUFLEN;
		}
		// As soon as you receive the final byte, switch to SEND mode
		else if(rece_count == packet_size)		
		{
			// full packet received - turn off serial timeout
			serial_receive_timeout_count = 0;
			DealwithTag = 6;		// making this number big will increase delay
			// if was dealing with a response, must know it is done
			response_receive_finished = 1;
		}
		else if(rece_count == 3 )
		{
			if(data_buffer[1] == CHECKONLINE)
				packet_size = 6;
		}

	}
	else if(TI0 == 1)
	{
		TI0 = 0;
		transmit_finished = 1;
	}

	return;
}


void Test_serial(void)
{

}
#endif
