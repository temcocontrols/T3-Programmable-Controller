/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2009 Steve Karg

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to:
 The Free Software Foundation, Inc.
 59 Temple Place - Suite 330
 Boston, MA  02111-1307, USA.

 As a special exception, if other files instantiate templates or
 use macros or inline functions from this file, or you compile
 this file and link it with other works to produce a work based
 on this file, this file does not by itself cause the resulting
 work to be covered by the GNU General Public License. However
 the source code for this file must still be made available in
 accordance with section (3) of the GNU General Public License.

 This exception does not invalidate any other reasons why a work
 based on this file might be covered by the GNU General Public
 License.
 -------------------------------------------
####COPYRIGHTEND####*/
#include <stdint.h>
#include "bacenum.h"
#include "bacdcode.h"
#include "bacdef.h"
#include "ptransfer.h"
#include "client.h"
#include "device.h"
#include "datalink.h"
#include "string.h"
#include "point.h"
#include "monitor.h"

// for temoc private application
#include "user_data.h"
#include "define.h"
//#include "lcd.h"
#include "clock.h"

/** @file ptransfer.c  Encode/Decode Private Transfer data */
/* 
	handler roution for private transfer
	created by chelsea
*/


uint8_t invoke_id;
uint16_t transfer_len;
extern U8_T far ChangeFlash;
uint8_t header_len;



void handler_private_transfer( 	
	uint8_t * apdu,
    unsigned apdu_len,
	BACNET_ADDRESS * src)/*,
    BACNET_PRIVATE_TRANSFER_DATA * private_data */ 
{
    BACNET_APPLICATION_DATA_VALUE data_value = { 0 };
    BACNET_APPLICATION_DATA_VALUE rec_data_value = { 0 };
    BACNET_PRIVATE_TRANSFER_DATA private_data = { 0 };
    BACNET_PRIVATE_TRANSFER_DATA rec_data = { 0 };
	Str_user_data_header	private_header;	
	BACNET_OCTET_STRING Temp_CS;

	uint8_t* ptr = NULL;
	uint8_t test_value[480] = { 0 };
    int len = 0;
	int private_data_len = 0;
    int property_len = 0;
//    BACNET_NPDU_DATA npdu_data;
    int bytes_sent = 0;
	bool status = false;
	uint8_t temp[480] = {0};
	uint8_t command = 0;

	U8_T j;

	 int iLen;   /* Index to current location in data */
     int tag_len;
	 uint8_t tag_number;
	 uint32_t len_value_type;
//   decode ptransfer
	len = ptransfer_decode_apdu(&apdu[0], apdu_len, &invoke_id, &rec_data);
	
	iLen = 0;

    /* Error code is returned for read and write operations */

    tag_len =
        decode_tag_number_and_value(&rec_data.serviceParameters[iLen],
        &tag_number, &len_value_type);
    iLen += tag_len;
	if(tag_number == BACNET_APPLICATION_TAG_OCTET_STRING)
	{
		decode_octet_string(&rec_data.serviceParameters[iLen], len_value_type,&Temp_CS);
	}
	private_data.vendorID =  rec_data.vendorID;
	private_data.serviceNumber = rec_data.serviceNumber;

   //bacapp_decode_application_data(rec_data.serviceParameters,
   //    rec_data.serviceParametersLen, &rec_data_value);	
	 command = Temp_CS.value[2];;

	if( command  == READMONITORDATA_T3000 || command  == UPDATEMEMMONITOR_T3000)
	{
		 header_len = 18;

		 Graphi_data->command = Temp_CS.value[2];
		 Graphi_data->index = Temp_CS.value[3];	  // monitor table index
		 Graphi_data->sample_type = Temp_CS.value[4];

		 memcpy(Graphi_data->comm_arg.string,&Temp_CS.value[5],4);  // size oldest_time most_recent_time
		 memcpy(Graphi_data->comm_arg.string + 4,&Temp_CS.value[9],4);
		 memcpy(Graphi_data->comm_arg.string + 8,&Temp_CS.value[13],4);

		 Graphi_data->comm_arg.monupdate.size = DoulbemGetPointWord2(Graphi_data->comm_arg.monupdate.size);
		 Graphi_data->comm_arg.monupdate.most_recent_time = DoulbemGetPointWord2(Graphi_data->comm_arg.monupdate.most_recent_time);
		 Graphi_data->comm_arg.monupdate.oldest_time = DoulbemGetPointWord2(Graphi_data->comm_arg.monupdate.oldest_time);
   
		 Graphi_data->special = Temp_CS.value[17];	  // 0 - scan  1 - send monitor frame 

	}
	else
	{
		private_header.total_length = Temp_CS.value[1] * 256 + Temp_CS.value[0];
		private_header.command = Temp_CS.value[2];
		private_header.point_start_instance = Temp_CS.value[3];
		private_header.point_end_instance = Temp_CS.value[4];
		private_header.entitysize = Temp_CS.value[6] * 256 + Temp_CS.value[5];

		header_len = USER_DATA_HEADER_LEN;			 

	}

	if(command > 100)   // write command
	{
		if(command ==  WRITEPRGFLASH_COMMAND)   // other commad
		{
			//Flash_Write_Mass();
			ChangeFlash = 1;
		} 		
		else
		{
		// TBD: add more write command
			switch(command)
			{
				case WRITEINPUT_T3000:
					ptr = (char *)(&inputs[private_header.point_start_instance]);				
					break;	
				case WRITEOUTPUT_T3000:
					ptr = (char *)(&outputs[private_header.point_start_instance]);
					break;
				case WRITEVARIABLE_T3000:        /* write variables  */
					ptr = (char *)(&vars[private_header.point_start_instance]);
					break;
			 	case WRITEWEEKLYROUTINE_T3000:         /* write weekly routines*/
					ptr = (char *)(&weekly_routines[private_header.point_start_instance]);
					break;
			 	case WRITEANNUALROUTINE_T3000:         /* write annual routines*/
					ptr = (char *)(&annual_routines[private_header.point_start_instance]);
					break;
			 	case WRITEPROGRAM_T3000:
					ptr = (char *)(&programs[private_header.point_start_instance]);
					break;
	
				case WRITEPROGRAMCODE_T3000:
					ptr = (char *)(prg_code[private_header.point_start_instance]);
					break;
				case WRITETIMESCHEDULE_T3000:
					ptr = (char *)(wr_times[private_header.point_start_instance]);
					break;
				case WRITEANNUALSCHEDULE_T3000:
					ptr = (char *)(ar_dates[private_header.point_start_instance]);
					break;
				case RESTARTMINI_COMMAND:
					ptr = (char *)(RTC.all);

					break;
				case WRITECONTROLLER_T3000:
					ptr = (char *)&controllers[private_header.point_start_instance];
//					for( j=0; j<MAX_CONS; j++ )
//					{
//						get_point_value( (Point*)&controllers[j].input, &controllers[j].input_value );
//						get_point_value( (Point*)&controllers[j].setpoint, &controllers[j].setpoint_value );
//					}
					break;
				case WRITEMONITOR_T3000 :
					ptr = (char *)&monitors[private_header.point_start_instance];
					break;
			 	case WRITESCREEN_T3000  :   //CONTROL_GROUP
					ptr = (char *)&control_groups[private_header.point_start_instance];
					break;
	//			case WRITEGROUPELEMENTS_T3000:
	//				ptr = (char *)(&group_data[private_header.point_start_instance]);
					break;

				default:
					break;	
					
			} 
			if(ptr != NULL)	
			{
				if(private_header.total_length  == private_header.entitysize * (private_header.point_end_instance - private_header.point_start_instance + 1) + header_len)
				{	// check is length is correct 
					if(command == WRITEVARIABLE_T3000)
					{
						if(private_header.point_start_instance == 0)
						{
//						char teststr[] = "\r\n 4: \r\n";
//						sub_send_string(teststr,10,UART0);
//						sub_send_string(Temp_CS.value,private_header.total_length,UART0);
						}
						
					}
				   	memcpy(ptr,&Temp_CS.value[header_len],private_header.total_length - header_len);
					if(command == WRITEPROGRAMCODE_T3000)
					{
					//	U8_T j = 0;
						for(j = private_header.point_start_instance;j <= private_header.point_end_instance;j++)
						{  						
							programs[j].bytes = mGetPointWord2(prg_code[j][1]* 256 + prg_code[j][0]);
						}

						/* recount code lenght once update program code */
	
						Code_total_length = 0;
						for(j = 0;j < MAX_PRGS;j++)
						{							
							Code_total_length += mGetPointWord2(programs[j].bytes);
						}
					}
					else if(command == WRITEMONITOR_T3000)
					{ 
						dealwithMonitor(private_header);
					}
					else if(command == RESTARTMINI_COMMAND)
					{ 
						Set_Clock(0,0);
						Set_Clock(1,0);
						Set_Clock(PCF_SEC,RTC.Clk.sec);
						Set_Clock(PCF_MIN,RTC.Clk.min);
						Set_Clock(PCF_HOUR,RTC.Clk.hour);
						Set_Clock(PCF_DAY,RTC.Clk.day);
						Set_Clock(PCF_WEEK,RTC.Clk.week);
						Set_Clock(PCF_MON,RTC.Clk.mon);
						Set_Clock(PCF_YEAR,RTC.Clk.year - 2000);
					}
				}
			}
		}
	}
	else  // read
	{	 
		if( Temp_CS.value[2]  == READMONITORDATA_T3000 || Temp_CS.value[2]  == UPDATEMEMMONITOR_T3000)
		{
			temp[0] = 0;
			temp[1] = 0;
			temp[2] = Graphi_data->command;
			temp[3] = Graphi_data->index;
			temp[4] = Graphi_data->sample_type;
			memcpy(&temp[5],Graphi_data->comm_arg.string,12);
			temp[17] = Graphi_data->special;
		}
		else
		{
			transfer_len = private_header.entitysize * (private_header.point_end_instance - private_header.point_start_instance + 1);
			if(transfer_len >= 0)
			{
				temp[0] = (uint8_t)(transfer_len >> 8);
				temp[1] = (uint8_t)transfer_len;
				temp[2] = private_header.command;
				temp[3] = private_header.point_start_instance;
				temp[4] = private_header.point_end_instance;
				temp[5] = (uint8_t)private_header.entitysize ; 
				temp[6] = (uint8_t)(private_header.entitysize >> 8); 	  
			}
		}

		switch(command)
		{
			case READOUTPUT_T3000:
				ptr = (char *)(&outputs[private_header.point_start_instance]);
				break;
			case READINPUT_T3000:
				ptr = (char *)(&inputs[private_header.point_start_instance]);
				break;
			case READVARIABLE_T3000:
				ptr = (char *)(&vars[private_header.point_start_instance]);
				break;
			case READWEEKLYROUTINE_T3000:
				ptr = (char *)(&weekly_routines[private_header.point_start_instance]);
				break;
			case READANNUALROUTINE_T3000:
				ptr = (char *)(&annual_routines[private_header.point_start_instance]);
				break;
			case READPROGRAM_T3000:
				ptr = (char *)(&programs[private_header.point_start_instance]);
				break;
			case READPROGRAMCODE_T3000:	
				for(j = private_header.point_start_instance;j <= private_header.point_end_instance;j++)
				{  // SWAP hi byte and low byte
					prg_code[j][0] = (U8_T)(programs[j].bytes >> 8);
					prg_code[j][1] = (U8_T)(programs[j].bytes);
				}
				ptr = (char *)prg_code[private_header.point_start_instance];
				break;
			case READTIMESCHEDULE_T3000:   /* read time schedule  */
				ptr = (char *)&wr_times[private_header.point_start_instance];
				break;

		 	case READANNUALSCHEDULE_T3000:    /* read annual schedule*/
				ptr = (char *)&ar_dates[private_header.point_start_instance];				
				break;

			case TIME_COMMAND:
				ptr = (char *)(RTC.all);
				break;
			case READCONTROLLER_T3000:
				
				ptr = (char *)(&controllers[private_header.point_start_instance]);
				for( j=0; j<MAX_CONS; j++ )
				{
					get_point_value( (Point*)&controllers[j].input, &controllers[j].input_value );
					get_point_value( (Point*)&controllers[j].setpoint, &controllers[j].setpoint_value );
				}
				break;

			case READMONITOR_T3000 :
				ptr = (char *)(&monitors[private_header.point_start_instance]);
				break;
	 		case READSCREEN_T3000 :
				ptr = (char *)(&control_groups[private_header.point_start_instance]);
				break;
			case READGROUPELEMENTS_T3000:
				ptr = (char *)(&group_data[private_header.point_start_instance]);
				break;	 		
			case READMONITORDATA_T3000:
				ReadMonitor(Graphi_data);				
				ptr = (char *)(Graphi_data->asdu);
				break;			
			case UPDATEMEMMONITOR_T3000:
				UpdateMonitor(Graphi_data);
				ptr = (char *)(Graphi_data->asdu);
				break;
			case GET_PANEL_INFO:   // other commad
		
				ptr = (char *)(Panel_Info.all);	
				break;

			default:
				break;
		}

		

		memcpy(&temp[header_len],ptr,transfer_len);
			
		if(command == READMONITOR_T3000)
		{	
			dealwithMonitor(private_header);	
		}	

		status = bacapp_parse_application_data(BACNET_APPLICATION_TAG_OCTET_STRING,	temp, &data_value);
		if(status == false)	Test[1]++;
	} 
 
    if(status == true)
	{	
	   	memset(temp,0,480);
		
	    private_data_len =
	        bacapp_encode_application_data(&temp[0],&data_value);
		
	    private_data.serviceParameters = &temp[0];
	    private_data.serviceParametersLen = private_data_len; 		

	    len = uptransfer_encode_apdu(&apdu[0], &private_data);
	}
	Send_UnconfirmedPrivateTransfer(src,&private_data);

    return;

}

int ptransfer_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    uint8_t * invoke_id,
    BACNET_PRIVATE_TRANSFER_DATA * private_data)
{
    int len = 0;
    unsigned offset = 0;

    if (!apdu)
        return -1;
    /* optional checking - most likely was already done prior to this call */
    if (apdu[0] != PDU_TYPE_CONFIRMED_SERVICE_REQUEST)
        return -1;
    /*  apdu[1] = encode_max_segs_max_apdu(0, MAX_APDU); */
    /* invoke id - filled in by net layer */
    *invoke_id = apdu[2];
    if (apdu[3] != SERVICE_CONFIRMED_PRIVATE_TRANSFER)
        return -1;
    offset = 4;

    if (apdu_len > offset) {
        len =
            ptransfer_decode_service_request(&apdu[offset], apdu_len - offset,
            private_data);
    }

    return len;
}

int uptransfer_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_PRIVATE_TRANSFER_DATA * private_data)
{
    int len = 0;
    unsigned offset = 0;

    if (!apdu) {
        return -1;
    }
    /* optional checking - most likely was already done prior to this call */
    if (apdu[0] != PDU_TYPE_UNCONFIRMED_SERVICE_REQUEST) {
        return -1;
    }
    if (apdu[1] != SERVICE_UNCONFIRMED_PRIVATE_TRANSFER) {
        return -1;
    }
    offset = 2;
    if (apdu_len > offset) {
        len =
		  ptransfer_decode_service_request(&apdu[offset], apdu_len - offset,
            private_data);
    }
    return len;
}





/* encode service */
static int pt_encode_apdu(
    uint8_t * apdu,
    uint16_t max_apdu,
    BACNET_PRIVATE_TRANSFER_DATA * private_data)
{
    int len = 0;        /* length of each encoding */
    int apdu_len = 0;   /* total length of the apdu, return value */
/*
        Unconfirmed/ConfirmedPrivateTransfer-Request ::= SEQUENCE {
        vendorID               [0] Unsigned,
        serviceNumber          [1] Unsigned,
        serviceParameters      [2] ABSTRACT-SYNTAX.&Type OPTIONAL
    }
*/
    /* unused parameter */
    max_apdu = max_apdu;
    if (apdu) {
        len =
            encode_context_unsigned(&apdu[apdu_len], 0,
            private_data->vendorID);
        apdu_len += len;
        len =
            encode_context_unsigned(&apdu[apdu_len], 1,
            private_data->serviceNumber);
        apdu_len += len;
        len = encode_opening_tag(&apdu[apdu_len], 2);
        apdu_len += len;
        for (len = 0; len < private_data->serviceParametersLen; len++) {
            apdu[apdu_len] = private_data->serviceParameters[len];
            apdu_len++;
        }
        len = encode_closing_tag(&apdu[apdu_len], 2);
        apdu_len += len;
    }

    return apdu_len;
}

int ptransfer_encode_apdu(
    uint8_t * apdu,
    uint8_t invoke_id,
    BACNET_PRIVATE_TRANSFER_DATA * private_data)
{
    int apdu_len = 0;   /* total length of the apdu, return value */
    int len = 0;

    if (apdu) {
        apdu[0] = PDU_TYPE_CONFIRMED_SERVICE_REQUEST;
        apdu[1] = encode_max_segs_max_apdu(0, MAX_APDU);
        apdu[2] = invoke_id;
        apdu[3] = SERVICE_CONFIRMED_PRIVATE_TRANSFER;
        apdu_len = 4;
        len =
            pt_encode_apdu(&apdu[apdu_len], (uint16_t) (MAX_APDU - apdu_len),
            private_data);
        apdu_len += len;
    }

    return apdu_len;
}

int uptransfer_encode_apdu(
    uint8_t * apdu,
    BACNET_PRIVATE_TRANSFER_DATA * private_data)
{
    int apdu_len = 0;   /* total length of the apdu, return value */
    int len = 0;

    if (apdu) {
        apdu[0] = PDU_TYPE_UNCONFIRMED_SERVICE_REQUEST;
        apdu[1] = SERVICE_UNCONFIRMED_PRIVATE_TRANSFER;
        apdu_len = 2;
        len =
            pt_encode_apdu(&apdu[apdu_len], (uint16_t) (MAX_APDU - apdu_len),
            private_data);
        apdu_len += len;
    }

    return apdu_len;
}

/* decode the service request only */
int ptransfer_decode_service_request(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_PRIVATE_TRANSFER_DATA * private_data)
{
    int len = 0;        /* return value */
    int decode_len = 0; /* return value */
    uint32_t unsigned_value = 0;

    /* check for value pointers */
    if (apdu_len && private_data) {
        /* Tag 0: vendorID */
        decode_len = decode_context_unsigned(&apdu[len], 0, &unsigned_value);
        if (decode_len < 0) {
            return -1;
        }
        len = decode_len;
        private_data->vendorID = (uint16_t) unsigned_value;
        /* Tag 1: serviceNumber */
        decode_len = decode_context_unsigned(&apdu[len], 1, &unsigned_value);
        if (decode_len < 0) {
            return -1;
        }
        len += decode_len;
        private_data->serviceNumber = unsigned_value;
        /* Tag 2: serviceParameters */
        if (decode_is_opening_tag_number(&apdu[len], 2)) {
            /* a tag number of 2 is not extended so only one octet */
            len++;
            /* don't decode the serviceParameters here */
            private_data->serviceParameters = &apdu[len];
            private_data->serviceParametersLen =
                (int) apdu_len - len - 1 /*closing tag */ ;
            /* len includes the data and the closing tag */
            len = (int) apdu_len;
        } else {
            return -1;
        }
    }

    return len;
}

int ptransfer_error_encode_apdu(
    uint8_t * apdu,
    uint8_t invoke_id,
    BACNET_ERROR_CLASS error_class,
    BACNET_ERROR_CODE error_code,
    BACNET_PRIVATE_TRANSFER_DATA * private_data)
{
    int apdu_len = 0;   /* total length of the apdu, return value */
    int len = 0;        /* length of the part of the encoding */

    if (apdu) {
        apdu[0] = PDU_TYPE_ERROR;
        apdu[1] = invoke_id;
        apdu[2] = SERVICE_CONFIRMED_PRIVATE_TRANSFER;
        apdu_len = 3;
        /* service parameters */
/*
        ConfirmedPrivateTransfer-Error ::= SEQUENCE {
        errorType       [0] Error,
        vendorID        [1] Unsigned,
        serviceNumber [2] Unsigned,
        errorParameters [3] ABSTRACT-SYNTAX.&Type OPTIONAL
    }
*/
        len = encode_opening_tag(&apdu[apdu_len], 0);
        apdu_len += len;
        len = encode_application_enumerated(&apdu[apdu_len], error_class);
        apdu_len += len;
        len = encode_application_enumerated(&apdu[apdu_len], error_code);
        apdu_len += len;
        len = encode_closing_tag(&apdu[apdu_len], 0);
        apdu_len += len;
        len =
            encode_context_unsigned(&apdu[apdu_len], 1,
            private_data->vendorID);
        apdu_len += len;
        len =
            encode_context_unsigned(&apdu[apdu_len], 2,
            private_data->serviceNumber);
        apdu_len += len;
        len = encode_opening_tag(&apdu[apdu_len], 3);
        apdu_len += len;
        for (len = 0; len < private_data->serviceParametersLen; len++) {
            apdu[apdu_len] = private_data->serviceParameters[len];
            apdu_len++;
        }
        len = encode_closing_tag(&apdu[apdu_len], 3);
        apdu_len += len;
    }

    return apdu_len;
}

/* decode the service request only */
int ptransfer_error_decode_service_request(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_ERROR_CLASS * error_class,
    BACNET_ERROR_CODE * error_code,
    BACNET_PRIVATE_TRANSFER_DATA * private_data)
{
    int len = 0;        /* return value */
    int decode_len = 0; /* return value */
    uint8_t tag_number = 0;
    uint32_t len_value_type = 0;
    uint32_t unsigned_value = 0;

    /* check for value pointers */
    if (apdu_len && private_data) {
        /* Tag 0: Error */
        if (decode_is_opening_tag_number(&apdu[len], 0)) {
            /* a tag number of 0 is not extended so only one octet */
            len++;
            /* error class */
            decode_len =
                decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value_type);
            len += decode_len;
            if (tag_number != BACNET_APPLICATION_TAG_ENUMERATED) {
                return 0;
            }
            decode_len =
                decode_enumerated(&apdu[len], len_value_type, &unsigned_value);
            len += decode_len;
            if (error_class) {
                *error_class = (BACNET_ERROR_CLASS) unsigned_value;
            }
            /* error code */
            decode_len =
                decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value_type);
            len += decode_len;
            if (tag_number != BACNET_APPLICATION_TAG_ENUMERATED) {
                return 0;
            }
            decode_len =
                decode_enumerated(&apdu[len], len_value_type, &unsigned_value);
            len += decode_len;
            if (error_code) {
                *error_code = (BACNET_ERROR_CODE) unsigned_value;
            }
            if (decode_is_closing_tag_number(&apdu[len], 0)) {
                /* a tag number of 0 is not extended so only one octet */
                len++;
            } else {
                return 0;
            }
        }
        /* Tag 1: vendorID */
        decode_len = decode_context_unsigned(&apdu[len], 1, &unsigned_value);
        if (decode_len < 0) {
            return -1;
        }
        len += decode_len;
        private_data->vendorID = (uint16_t) unsigned_value;
        /* Tag 2: serviceNumber */
        decode_len = decode_context_unsigned(&apdu[len], 2, &unsigned_value);
        if (decode_len < 0) {
            return -1;
        }
        len += decode_len;
        private_data->serviceNumber = unsigned_value;
        /* Tag 3: serviceParameters */
        if (decode_is_opening_tag_number(&apdu[len], 3)) {
            /* a tag number of 2 is not extended so only one octet */
            len++;
            /* don't decode the serviceParameters here */
            private_data->serviceParameters = &apdu[len];
            private_data->serviceParametersLen =
                (int) apdu_len - len - 1 /*closing tag */ ;
        } else {
            return -1;
        }
        /* we could check for a closing tag of 3 */
    }

    return len;
}

int ptransfer_ack_encode_apdu(
    uint8_t * apdu,
    uint8_t invoke_id,
    BACNET_PRIVATE_TRANSFER_DATA * private_data)
{
    int len = 0;        /* length of each encoding */
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu) {
        apdu[0] = PDU_TYPE_COMPLEX_ACK; /* complex ACK service */
        apdu[1] = invoke_id;    /* original invoke id from request */
        apdu[2] = SERVICE_CONFIRMED_PRIVATE_TRANSFER;   /* service choice */
        apdu_len = 3;
        /* service ack follows */
/*
        ConfirmedPrivateTransfer-ACK ::= SEQUENCE {
        vendorID               [0] Unsigned,
        serviceNumber          [1] Unsigned,
        resultBlock            [2] ABSTRACT-SYNTAX.&Type OPTIONAL
    }
*/
        len =
            encode_context_unsigned(&apdu[apdu_len], 0,
            private_data->vendorID);
        apdu_len += len;
        len =
            encode_context_unsigned(&apdu[apdu_len], 1,
            private_data->serviceNumber);
        apdu_len += len;
        len = encode_opening_tag(&apdu[apdu_len], 2);
        apdu_len += len;
        for (len = 0; len < private_data->serviceParametersLen; len++) {
            apdu[apdu_len] = private_data->serviceParameters[len];
            apdu_len++;
        }
        len = encode_closing_tag(&apdu[apdu_len], 2);
        apdu_len += len;
    }

    return apdu_len;
}

/* ptransfer_ack_decode_service_request() is the same as
       ptransfer_decode_service_request */


#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"
#include "bacapp.h"

int ptransfer_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    uint8_t * invoke_id,
    BACNET_PRIVATE_TRANSFER_DATA * private_data)
{
    int len = 0;
    unsigned offset = 0;

    if (!apdu)
        return -1;
    /* optional checking - most likely was already done prior to this call */
    if (apdu[0] != PDU_TYPE_CONFIRMED_SERVICE_REQUEST)
        return -1;
    /*  apdu[1] = encode_max_segs_max_apdu(0, MAX_APDU); */
    /* invoke id - filled in by net layer */
    *invoke_id = apdu[2];
    if (apdu[3] != SERVICE_CONFIRMED_PRIVATE_TRANSFER)
        return -1;
    offset = 4;

    if (apdu_len > offset) {
        len =
            ptransfer_decode_service_request(&apdu[offset], apdu_len - offset,
            private_data);
    }

    return len;
}

int uptransfer_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_PRIVATE_TRANSFER_DATA * private_data)
{
    int len = 0;
    unsigned offset = 0;

    if (!apdu) {
        return -1;
    }
    /* optional checking - most likely was already done prior to this call */
    if (apdu[0] != PDU_TYPE_UNCONFIRMED_SERVICE_REQUEST) {
        return -1;
    }
    if (apdu[1] != SERVICE_UNCONFIRMED_PRIVATE_TRANSFER) {
        return -1;
    }
    offset = 2;
    if (apdu_len > offset) {
        len =
            ptransfer_decode_service_request(&apdu[offset], apdu_len - offset,
            private_data);
    }

    return len;
}

int ptransfer_ack_decode_apdu(
    uint8_t * apdu,
    int apdu_len,       /* total length of the apdu */
    uint8_t * invoke_id,
    BACNET_PRIVATE_TRANSFER_DATA * private_data)
{
    int len = 0;
    int offset = 0;

    if (!apdu)
        return -1;
    /* optional checking - most likely was already done prior to this call */
    if (apdu[0] != PDU_TYPE_COMPLEX_ACK)
        return -1;
    *invoke_id = apdu[1];
    if (apdu[2] != SERVICE_CONFIRMED_PRIVATE_TRANSFER)
        return -1;
    offset = 3;
    if (apdu_len > offset) {
        len =
            ptransfer_decode_service_request(&apdu[offset], apdu_len - offset,
            private_data);
    }

    return len;
}

int ptransfer_error_decode_apdu(
    uint8_t * apdu,
    int apdu_len,       /* total length of the apdu */
    uint8_t * invoke_id,
    BACNET_ERROR_CLASS * error_class,
    BACNET_ERROR_CODE * error_code,
    BACNET_PRIVATE_TRANSFER_DATA * private_data)
{
    int len = 0;
    int offset = 0;

    if (!apdu)
        return -1;
    /* optional checking - most likely was already done prior to this call */
    if (apdu[0] != PDU_TYPE_ERROR)
        return -1;
    *invoke_id = apdu[1];
    if (apdu[2] != SERVICE_CONFIRMED_PRIVATE_TRANSFER)
        return -1;
    offset = 3;
    if (apdu_len > offset) {
        len =
            ptransfer_error_decode_service_request(&apdu[offset],
            apdu_len - offset, error_class, error_code, private_data);
    }

    return len;
}

void test_Private_Transfer_Ack(
    Test * pTest)
{
    uint8_t apdu[480] = { 0 };
    int len = 0;
    int apdu_len = 0;
    uint8_t invoke_id = 128;
    uint8_t test_invoke_id = 0;
    BACNET_PRIVATE_TRANSFER_DATA private_data;
    BACNET_PRIVATE_TRANSFER_DATA test_data;
    uint8_t test_value[480] = { 0 };
    int private_data_len = 0;
    char private_data_chunk[33] = { "00112233445566778899AABBCCDDEEFF" };
    BACNET_APPLICATION_DATA_VALUE data_value;
    BACNET_APPLICATION_DATA_VALUE test_data_value;
    bool status = false;

    private_data.vendorID = BACNET_VENDOR_ID;
    private_data.serviceNumber = 1;

    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_OCTET_STRING,
        &private_data_chunk[0], &data_value);
    ct_test(pTest, status == true);
    private_data_len =
        bacapp_encode_application_data(&test_value[0], &data_value);

    private_data.serviceParameters = &test_value[0];
    private_data.serviceParametersLen = private_data_len;


    len = ptransfer_ack_encode_apdu(&apdu[0], invoke_id, &private_data);
    ct_test(pTest, len != 0);
    ct_test(pTest, len != -1);
    apdu_len = len;
    len =
        ptransfer_ack_decode_apdu(&apdu[0], apdu_len, &test_invoke_id,
        &test_data);
    ct_test(pTest, len != -1);
    ct_test(pTest, test_invoke_id == invoke_id);
    ct_test(pTest, test_data.vendorID == private_data.vendorID);
    ct_test(pTest, test_data.serviceNumber == private_data.serviceNumber);
    ct_test(pTest,
        test_data.serviceParametersLen == private_data.serviceParametersLen);
    len =
        bacapp_decode_application_data(test_data.serviceParameters,
        test_data.serviceParametersLen, &test_data_value);
    ct_test(pTest, bacapp_same_value(&data_value, &test_data_value) == true);
}

void test_Private_Transfer_Error(
    Test * pTest)
{
    uint8_t apdu[480] = { 0 };
    int len = 0;
    int apdu_len = 0;
    uint8_t invoke_id = 128;
    uint8_t test_invoke_id = 0;
    BACNET_ERROR_CLASS error_class = ERROR_CLASS_RESOURCES;
    BACNET_ERROR_CODE error_code = ERROR_CODE_OPERATIONAL_PROBLEM;
    BACNET_ERROR_CLASS test_error_class = 0;
    BACNET_ERROR_CODE test_error_code = 0;
    BACNET_PRIVATE_TRANSFER_DATA private_data;
    BACNET_PRIVATE_TRANSFER_DATA test_data;
    uint8_t test_value[480] = { 0 };
    int private_data_len = 0;
    char private_data_chunk[33] = { "00112233445566778899AABBCCDDEEFF" };
    BACNET_APPLICATION_DATA_VALUE data_value;
    BACNET_APPLICATION_DATA_VALUE test_data_value;
    bool status = false;

    private_data.vendorID = BACNET_VENDOR_ID;
    private_data.serviceNumber = 1;

    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_OCTET_STRING,
        &private_data_chunk[0], &data_value);
    ct_test(pTest, status == true);
    private_data_len =
        bacapp_encode_application_data(&test_value[0], &data_value);
    private_data.serviceParameters = &test_value[0];
    private_data.serviceParametersLen = private_data_len;

    len =
        ptransfer_error_encode_apdu(&apdu[0], invoke_id, error_class,
        error_code, &private_data);
    ct_test(pTest, len != 0);
    ct_test(pTest, len != -1);
    apdu_len = len;
    len =
        ptransfer_error_decode_apdu(&apdu[0], apdu_len, &test_invoke_id,
        &test_error_class, &test_error_code, &test_data);
    ct_test(pTest, len != -1);
    ct_test(pTest, test_invoke_id == invoke_id);
    ct_test(pTest, test_data.vendorID == private_data.vendorID);
    ct_test(pTest, test_data.serviceNumber == private_data.serviceNumber);
    ct_test(pTest, test_error_class == error_class);
    ct_test(pTest, test_error_code == error_code);
    ct_test(pTest,
        test_data.serviceParametersLen == private_data.serviceParametersLen);
    len =
        bacapp_decode_application_data(test_data.serviceParameters,
        test_data.serviceParametersLen, &test_data_value);
    ct_test(pTest, bacapp_same_value(&data_value, &test_data_value) == true);
}

void test_Private_Transfer_Request(
    Test * pTest)
{
    uint8_t apdu[480] = { 0 };
    uint8_t test_value[480] = { 0 };
    int len = 0;
    int apdu_len = 0;
    uint8_t invoke_id = 128;
    uint8_t test_invoke_id = 0;
    int private_data_len = 0;
    char private_data_chunk[33] = { "00112233445566778899AABBCCDDEEFF" };
    BACNET_APPLICATION_DATA_VALUE data_value = { 0 };
    BACNET_APPLICATION_DATA_VALUE test_data_value = { 0 };
    BACNET_PRIVATE_TRANSFER_DATA private_data = { 0 };
    BACNET_PRIVATE_TRANSFER_DATA test_data = { 0 };
    bool status = false;

    private_data.vendorID = BACNET_VENDOR_ID;
    private_data.serviceNumber = 1;

    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_OCTET_STRING,
        &private_data_chunk[0], &data_value);
    ct_test(pTest, status == true);
    private_data_len =
        bacapp_encode_application_data(&test_value[0], &data_value);

    private_data.serviceParameters = &test_value[0];
    private_data.serviceParametersLen = private_data_len;

    len = ptransfer_encode_apdu(&apdu[0], invoke_id, &private_data);
    ct_test(pTest, len != 0);
    apdu_len = len;
    len =
        ptransfer_decode_apdu(&apdu[0], apdu_len, &test_invoke_id, &test_data);
    ct_test(pTest, len != -1);
    ct_test(pTest, test_data.vendorID == private_data.vendorID);
    ct_test(pTest, test_data.serviceNumber == private_data.serviceNumber);
    ct_test(pTest,
        test_data.serviceParametersLen == private_data.serviceParametersLen);
    len =
        bacapp_decode_application_data(test_data.serviceParameters,
        test_data.serviceParametersLen, &test_data_value);
    ct_test(pTest, bacapp_same_value(&data_value, &test_data_value) == true);

    return;
}

void test_Unconfirmed_Private_Transfer_Request(
    Test * pTest)
{
    uint8_t apdu[480] = { 0 };
    uint8_t test_value[480] = { 0 };
    int len = 0;
    int apdu_len = 0;
    int private_data_len = 0;
    char private_data_chunk[32] = { "I Love You, Patricia!" };
    BACNET_APPLICATION_DATA_VALUE data_value;
    BACNET_APPLICATION_DATA_VALUE test_data_value;
    BACNET_PRIVATE_TRANSFER_DATA private_data;
    BACNET_PRIVATE_TRANSFER_DATA test_data;
    bool status = false;

    private_data.vendorID = BACNET_VENDOR_ID;
    private_data.serviceNumber = 1;

    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_CHARACTER_STRING,
        &private_data_chunk[0], &data_value);
    ct_test(pTest, status == true);
    private_data_len =
        bacapp_encode_application_data(&test_value[0], &data_value);
    private_data.serviceParameters = &test_value[0];
    private_data.serviceParametersLen = private_data_len;

    len = uptransfer_encode_apdu(&apdu[0], &private_data);
    ct_test(pTest, len != 0);
    apdu_len = len;
    len = uptransfer_decode_apdu(&apdu[0], apdu_len, &test_data);
    ct_test(pTest, len != -1);
    ct_test(pTest, test_data.vendorID == private_data.vendorID);
    ct_test(pTest, test_data.serviceNumber == private_data.serviceNumber);
    ct_test(pTest,
        test_data.serviceParametersLen == private_data.serviceParametersLen);
    len =
        bacapp_decode_application_data(test_data.serviceParameters,
        test_data.serviceParametersLen, &test_data_value);
    ct_test(pTest, bacapp_same_value(&data_value, &test_data_value) == true);

    return;
}

#ifdef TEST_PRIVATE_TRANSFER
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet PrivateTransfer", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, test_Private_Transfer_Request);
    assert(rc);
    rc = ct_addTestFunction(pTest, test_Private_Transfer_Ack);
    assert(rc);
    rc = ct_addTestFunction(pTest, test_Private_Transfer_Error);
    assert(rc);
    rc = ct_addTestFunction(pTest, test_Unconfirmed_Private_Transfer_Request);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_READ_PROPERTY */
#endif /* TEST */
