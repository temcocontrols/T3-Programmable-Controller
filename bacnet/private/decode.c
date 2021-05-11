#include "bacnet.h"
#include "point.h"
#include <string.h>
#include "basic.h"
#include <stdlib.h>
#include "user_data.h"
#include "alarm.h"
#include "define.h"
#include <math.h>

#if BAC_PRIVATE

S32_T veval_exp(U8_T *local);
void put_local_var(U8_T *p, S32_T value, U8_T *local);
S16_T put_local_array(U8_T *p, S32_T value, S32_T v1, S32_T v2, U8_T *local );
S32_T operand(S8_T **buf,U8_T *local);
void push(S32_T value);
S32_T pop(void);
void pushlong(unsigned long value);
unsigned long poplong(void);

extern UN_Time Rtc;

#if (ARM_MINI || ARM_CM5 )
extern uint8_t flag_start_scan_network;
extern uint8_t start_scan_network_count;
extern uint16_t scan_network_bacnet_count;
#endif

S16_T exec_program(S16_T current_prg, U8_T *prog_code);


S8_T                         just_load = 0;
U32_T  far                     miliseclast_cur = 0;
U32_T  far                     miliseclast = 0;
S16_T isdelimit(S8_T c)
{
	static U16_T count = 0;
	if (strchr( "\x1\xFF\xFE" , c) )
	{	
		count = 0;
		return 1;
	}
	else 
	{
		count++;		
		if(count > 2000)	
		{
			count = 0;  
			return 1; // avoid dead 
		}
		else
			return 0; 
	}
}


#if 1

/*
1. float和unsigned long具有相同的数据结构长度
*/

/*
将浮点数转化为4个字节数据放在byte
*/
		

void Float_to_Byte(float f, unsigned char *mybyte,  unsigned char ntype)
{
		unsigned char transfer_byte1[4] = { 0 };
		FloatLongType fl1;

		fl1.fdata = f;
    transfer_byte1[3] = (unsigned char)fl1.ldata;
    transfer_byte1[2] = (unsigned char)(fl1.ldata >> 8);
    transfer_byte1[1] = (unsigned char)(fl1.ldata >> 16);
    transfer_byte1[0] = (unsigned char)(fl1.ldata >> 24);


    if (ntype == FLOAT_TYPE_ABCD)
    {
       mybyte[0] = transfer_byte1[0];
       mybyte[1] = transfer_byte1[1];
       mybyte[2] = transfer_byte1[2];
       mybyte[3] = transfer_byte1[3];
    }
    else if (ntype == FLOAT_TYPE_CDAB)
    {
        mybyte[2] = transfer_byte1[0];
        mybyte[3] = transfer_byte1[1];
        mybyte[0] = transfer_byte1[2];
        mybyte[1] = transfer_byte1[3];
    }
    else if (ntype == FLOAT_TYPE_BADC)
    {
         mybyte[1] = transfer_byte1[0];
         mybyte[0] = transfer_byte1[1];
         mybyte[3] = transfer_byte1[2];
         mybyte[2] = transfer_byte1[3];
    }
    else if (ntype == FLOAT_TYPE_DCBA)
    {
         mybyte[3] = transfer_byte1[0];
         mybyte[2] = transfer_byte1[1];
         mybyte[1] = transfer_byte1[2];
         mybyte[0] = transfer_byte1[3];
    }
}


/*
将4个字节byte[4]转化为浮点数存放在f中
*/
void Byte_to_Float(float *f, S32_T val_ptr,unsigned char ntype)
{
	FloatLongType fl;
	unsigned char transfer_byte[4] = { 0 };
    if (ntype == FLOAT_TYPE_DCBA)
    {
        transfer_byte[0] = (U8_T)(val_ptr);
        transfer_byte[1] = (U8_T)(val_ptr >> 8);
        transfer_byte[2] = (U8_T)(val_ptr >> 16);
        transfer_byte[3] = (U8_T)(val_ptr >> 24);
    }
    else if (ntype == FLOAT_TYPE_BADC)
    {
        transfer_byte[0] = (U8_T)(val_ptr >> 16);
        transfer_byte[1] = (U8_T)(val_ptr >> 24);
        transfer_byte[2] = (U8_T)(val_ptr);
        transfer_byte[3] = (U8_T)(val_ptr >> 8);
    }
    else if (ntype == FLOAT_TYPE_CDAB)
    {
        transfer_byte[0] = (U8_T)(val_ptr >> 8);
        transfer_byte[1] = (U8_T)(val_ptr);
        transfer_byte[2] = (U8_T)(val_ptr >> 24);
        transfer_byte[3] = (U8_T)(val_ptr >> 16);
    }
    else if (ntype == FLOAT_TYPE_ABCD)
    {
        transfer_byte[0] = (U8_T)(val_ptr >> 24);
        transfer_byte[1] = (U8_T)(val_ptr >> 16);
        transfer_byte[2] = (U8_T)(val_ptr >> 8);
        transfer_byte[3] = (U8_T)(val_ptr);
    }

    fl.ldata = 0;
    fl.ldata = (U8_T)transfer_byte[0];
    fl.ldata = (U16_T)(fl.ldata << 8) | (U8_T)transfer_byte[1];
    fl.ldata = (U32_T)(fl.ldata << 8) | (U8_T)transfer_byte[2];
    fl.ldata = (U32_T)(fl.ldata << 8) | (U8_T)transfer_byte[3];
    *f = fl.fdata;
}


#endif

U16_T convert_pointer_to_word( U8_T *iAddr ) //	 mGetPointWord
{ 
  	return( iAddr[1] | (U16_T)iAddr[0] << 8 );
}


//U16_T swap_word( U16_T dat ) 	//	  swap_word
//{ 
//	U8_T far temp1,temp2;
//	temp1 = (U8_T)dat;
//	temp2 = (U8_T)(dat >> 8);
//  	return( temp2 | (U16_T)temp1 << 8 );
//}


U32_T convert_pointer_to_double( U8_T *iAddr )	  // DoulbemGetPointWord
{
	return( iAddr[0] | (U16_T)iAddr[1] << 8 | (U32_T)iAddr[2] << 16 |  (U32_T)iAddr[3] << 24);
}


//U32_T swap_double( U32_T dat ) 	 //swap_double
//{ 
//	U8_T temp1,temp2,temp3,temp4;
//	temp1 = (U8_T)dat;
//	temp2 = (U8_T)(dat >> 8);
//	temp3 = (U8_T)(dat >> 16);
//	temp4 = (U8_T)(dat >> 24);
//	return( temp4 | (U16_T)temp3 << 8 | (U32_T)temp2 << 16 |  (U32_T)temp1 << 24);
//}

extern u32 uip_timer;
S16_T exec_program(S16_T current_prg, U8_T *prog_code)
{
	Point p_var;
	Point_Net point_net;
	S32_T val1, val2, step;
	S16_T nbytes;
	// S32_T *i_stack, *idecl_stack;
	// S16_T  ind, i, j;
	S16_T i;
	S16_T id, len, /*nprog, ndeclare ,*/nitem, lvar;
	S8_T then_else;
	S8_T ana_dig;
	S8_T *return_pointer, /**decl_prog,*/ *p_buf, *p, *q;
	U32_T r;
	U8_T type_var;
	S32_T value, v1, v2;
	U8_T *local;
	static U16_T temp = 0;
	
	u32 t1,t2;
	// S16_T r_ind_remote;
//	Program_remote_points /**r_remote,*/ *remote_local_list;
//	S16_T ind_remote_local_list;

	S32_T tempval = 0;

	then_else = alarm_flag /*= error_flag*/ = 0;
	prog = (U8_T *)prog_code;
	
/*	if (called_program)
	{
		if (*(prog+2+3)!=DECLARE)		 return 1;
	}
	else */
	 	index_stack = stack;
	
	memcpy(&nbytes, prog, 2);	
	nbytes = swap_word(nbytes);  // add by chelsea	

	if(nbytes == 0) return 0;

	prog += nbytes+2+3;

	memcpy(&nbytes, prog, 2);       /*LOCAL VARIABLES*/	
	nbytes = swap_word(nbytes);  // add by chelsea	
	local = (prog + 2);
	prog += 2 + nbytes;

	if(nbytes > 100)	{	nbytes = 5;	}// TBD?????????????????????????


	memcpy(&nbytes, prog, 2);
	
	nbytes = swap_word(nbytes);	// add by chelsea 
	
	prog += 2;	
	p_buf = (S8_T*)prog + nbytes;
	
	time_buf = (S8_T*)prog;

//	memcpy(&ind_remote_local_list,prog+nbytes,2);
//	ind_remote_local_list = swap_word(ind_remote_local_list);

//	remote_local_list = (Program_remote_points*)(prog+nbytes+2);
/*	memcpy(remote_local_list,prog+nbytes+2,ind_remote_local_list*sizeof(Remote_local_list));*/

	temp = 0;
	while( ((S8_T*)prog < p_buf) && (temp++ < 2000))
	{
		cond = (int)veval_exp( local );
		pn = (S32_T *)(prog + 1);
		if(cond)
		{
			if(*prog++)
			{
				if(just_load) *pn = 0;
				(*pn) += miliseclast_cur;
			}
			else
			{
			 	*(prog-1) = 1;
			 	*pn = 0;
			}
		}
		else if(*prog++)
		{	
		 	*(prog-1) = 0;
		 	*pn = 0;
		}
		else 
		{
			if(just_load) *pn = 0;
			(*pn) += miliseclast_cur;
		}	
	 	prog += 4;
 	}
	if(temp >= 2000) 
	{
// generate a alarm		
		generate_program_alarm(0,current_prg + 1);
		return 0;
	}
	
	prog = (U8_T *)prog_code;
	p_buf = (S8_T*)prog;
	memcpy(&nbytes, prog, 2);
	nbytes = swap_word(nbytes);	// add by chelsea
//	g_ind_remote_local_list = ind_remote_local_list;
//	g_remote_local_list = remote_local_list;

	p_buf += 2;
	prog += 2;
	// *(prog + nbytes + 1) -> low byte
	// *(prog + nbytes + 2) -> high byte
	// these are attached with 0xfe, if need WAIT, jumpe to end
	// fixed it by chelsea
	if(*(prog + nbytes + 1) + *(prog + nbytes + 2) * 256 > 2000) 
	// offset is too bigger, it is wrong
		return 0;
	prog = prog + *(prog + nbytes + 1) + *(prog + nbytes + 2) * 256;
//	alarm_at_all = OFF;
//	ind_alarm_panel = 0;
//	timeout = 0;
#if 1
	temp = 0;

#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)
	t1 = uip_timer;
#else	// tbd: for asix
	t1 = (U16_T)SWTIMER_Tick();
#endif
	while((*prog != 0xfe) && (temp++ < 2000))
	{
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)		
		t2 = uip_timer;		
		if(t2 - t1 > 100)	// avoid dead cycle		
		{		
			return 0;
		}
#else
		t2 = (U16_T)SWTIMER_Tick();
		//Test[20] = t2 - t1;
#endif		
		

		lvar = 0;
		if(!then_else)
	 	{	 
			if (*prog!=0x01)
			{	
			/*			printf("ERROR!!!!!!Virtual!!!!!!!!!!!!!!\n"); */
			/*			exit(1);*/
				return -1;
			}
			prog++;			/* step over 01*/
			/*		 memcpy(&cur_line, prog, 2);*/
			prog += 2;
	 	}
	 	else if (*prog==0x01)
		{	
			then_else = 0;
			continue;
		}
		switch (*prog++) 
		{	
		case ASSIGN:  
		case ASSIGNAR:
		case ASSIGNARRAY_1:
		case ASSIGNARRAY_2:
		case STARTPRG:
		case OPEN:
		case ENABLEX:
		case STOP:
		case CLOSE:
		case DISABLEX:
			id = *(prog-1);
			if (id == ASSIGN || id == ASSIGNAR)
					 ana_dig=ANALOG;
			else
					 ana_dig=DIGITAL;

			if(*prog >= LOCAL_VARIABLE && *prog <= STRING_TYPE_ARRAY)
			{	 
				type_var = LOCAL;
				p = prog;
				prog++;
				prog += 2;
			}
			else if (*prog == LOCAL_POINT_PRG)
			{	
				  prog++;
				  type_var = LOCAL_POINT_PRG;
				  p_var = *((Point *)prog);
				  prog += sizeof(Point);
			}
			else
			{
				if (*prog == REMOTE_POINT_PRG)
				{
					prog++;
					type_var = REMOTE_POINT_PRG;
					point_net = *((Point_Net *)prog);
					prog += sizeof(Point_Net);
				}
			}
			if ( id==OPEN )
			{	
//				 if (type_var == LOCAL_POINT_PRG)
//				 {
//					if ( p_var.point_type - 1 == GRP )
//					{
//						*((Point *)&localopenscreen) = p_var;
//						localopenscreen.panel = Station_NUM-1;
//						localopenscreen.network_number = 0xFFFF;      /*NetworkAddress;*/
//						break;
//					}
//				 }
//				 if (type_var == REMOTE_POINT_PRG)
//				 {
//					if ( point_net.point_type - 1 == GRP )
//					{
//						localopenscreen = point_net;
//				    break;
//					}
//				 }
			}
			if (id==STARTPRG || id==OPEN || id==ENABLEX) {	value = 1000L;	}
			if (id==STOP || id==CLOSE || id==DISABLEX) {	value = 0L;	}
			if (id==ASSIGN)
			{
				 value = veval_exp(local);
				 if (type_var == LOCAL)
				 {	

					put_local_var(p,value,local);
				 }
						
			}
			else if (id==ASSIGNARRAY_1)
			{
				v2 = 0;
				v1 = 1;
				v2 = veval_exp(local);
				value=veval_exp(local);
				put_local_array(p,value,v1,v2/1000L,local);
			}
			else if (id==ASSIGNARRAY_2)
			{
				v2 = 0;
				v1 = veval_exp(local);
				v2 = veval_exp(local);
				value=veval_exp(local);
				put_local_array(p,value,v1/1000L,v2/1000L,local);
			}
			else
			{
				if( id==ASSIGNAR )
				{
					ana_dig = (int)(veval_exp(local)/1000)-1;
					value=veval_exp(local);
				}
				else
				{
					if (type_var == LOCAL)
						put_local_var(p,value,local);
				}
			}
			if (type_var == LOCAL_POINT_PRG)
			{		
					put_point_value( &p_var, &value, ana_dig, PROGR );
			}
			// added for remote point by chelsa
			else if(type_var == REMOTE_POINT_PRG)
			{  
#if (ARM_MINI || ARM_CM5)
			flag_start_scan_network = 1;			
			start_scan_network_count = 0;
#endif				
				put_net_point_value(&point_net,&value,ana_dig,PROGR,0);
			}
			break;
//		case PHONE:
//								len = *prog++;
//								i=0;
//								while(*prog!='\x1' && i<len) message[i++] = *prog++;
//								message[i]=0;
//								phone(message,i);
//								break;
		case REM:
		case DIM:
		case INTEGER_TYPE:
		case BYTE_TYPE:
		case STRING_TYPE:
		case LONG_TYPE:
		case FLOAT_TYPE:
								len = *prog++;
								prog += len;
								break;
		case PRINT:
								break;
		case CLEARX:              /* clear all local variables to zero*/
	/*
								for(ind=0;ind<MAX_VAR;ind++)
										local[ind]=0;
	*/
								break;
		case CLEARPORT:
						 /*if( port >= 0 )
						 {
							Port_parameters[port].Length = Port_parameters[port].Index;
						 }*/
						 break;
		case ENDPRG:	return 1;      /* end program*/
		case RETURN:	          r = poplong();
								prog = (S8_T *)r;
								break;
		case HANGUP:
//								handup();      /* end phone call*/
								break;
		case SET_PRINTER:
								break;
		case RUN_MACRO:
								break;
		case ON:
								nitem = veval_exp(local);
								if (nitem < 1 || nitem > *(prog+1))
										{
										 while(*prog!='\x1') prog++;
										 break;
										}
								if (*prog==GOSUB)   /*gosub*/
									 {
										return_pointer =  (S8_T *)prog + 2 + *(prog+1)*2;
										pushlong((S32_T)return_pointer);
									 }
								memcpy(&i, prog + 2 + (nitem-1)*2, 2);
								i = swap_word(i);
								prog = (U8_T *)p_buf + i - 2;
								break;
		case GOSUB:
	
								return_pointer =  (S8_T*)prog + 2 ;
								memcpy(&i, prog, 2);
								i = swap_word(i);
								prog = (U8_T *)p_buf + i - 2;
								pushlong((S32_T)return_pointer);
								break;
	
		case ON_ALARM:
					
								if (alarm_flag)
								{
									memcpy(&i, prog, 2);
									i = swap_word(i);
									prog = (U8_T *)p_buf + i - 2;
									alarm_flag=0;
								}
								else
								 prog += 2;
								break;
//		case ON_ERROR:
//								if (error_flag)
//								{
//									memcpy(&i, prog, 2);
//									i = swap_word(i);
//									prog = (U8_T *)p_buf + i - 2;
//									error_flag=0;
//								}
//								else
//								 	prog += 2;
//								break;
		case GOTOIF:
		case GOTO:
								memcpy(&i, prog, 2);
								i = swap_word(i);
								prog = (U8_T *)p_buf + i - 2;
								
								break;
		case Alarm:		
			           if((p=(char *)memchr(prog,LT,30)) != NULL)
										i = LT;
									else
										if((p=(char *)memchr(prog,GT,30)) != NULL)
											i = GT;
									*p = 0xFF;
									v1 = veval_exp(local);
									v2 = veval_exp(local);
									value = veval_exp(local);
									len = *prog++;
									memcpy(message, prog, len);
									message[len] = 0;
									prog += len;

									{
										if (i == LT)
											if ( v1 < v2 )
											{
												//*(prog - 1) = 1;
												generatealarm(message, current_prg+1, Station_NUM, VIRTUAL_ALARM, alarm_at_all, ind_alarm_panel, alarm_panel, 0);
												alarm_flag = 1;
												//alarm();
											}
										if (i == GT)
											if ( v1 > v2 )
											{
												//*(prog - 1) = 1;
												alarm_flag = 1;
												generatealarm(message, current_prg+1, Station_NUM, VIRTUAL_ALARM, alarm_at_all, ind_alarm_panel, alarm_panel, 0);
												//alarm();
											}
									 }
									*p=i;
									break;
								break;
		case ALARM_AT:			
								if (*prog==0xFF)
								{
									 alarm_at_all = ON;
									 prog++;
								}
								else
								{
									 while(*prog)
										 alarm_panel[ind_alarm_panel++]=*prog++;
									 prog++;
								}
								break;
//		case PRINT_AT:
//								if (*prog==0xFF)
//								{
////									alarm_at_all = ON;
//									prog++;
//								}
//								else
//								{
////									while(*prog)
////										 print_panel[ind_print_panel++]=*prog++;
//									prog++;
//								}
//								break;
//								break;
//		case CALLB:
//								break;
		case DALARM:
							 {	
							 	U8_T i;
								alarm_flag = 0;
								cond = veval_exp(local);  /* condition  */								
								
								memcpy(&value,prog,4);    /* delay time */
								value = swap_double(value);	 

								prog += 4;
								len = *prog++;								
								if (cond)         /* test condition*/
									{
										memcpy(message, prog, len);
										message[len]=0;
										prog += len;
										if(just_load)
											memcpy(prog,&value,4);
										memcpy(&v1,prog,4);
										if( v1>0 )
										{
											v1 -= miliseclast_cur;
											v1 = swap_double(v1);  // for asix
											memcpy(prog, &v1, 4);
										}
										if (v1<=0)      /* delayed time elapsed */
										{
											 i = generatealarm(message, current_prg+1, Station_NUM, VIRTUAL_ALARM, alarm_at_all, ind_alarm_panel, alarm_panel, 0);
											 if ( i > 0 )    /* new alarm message*/
											 {
												 alarm_flag = 1;
											 }
										}
									}
									else
									{      /* condition is false*/
										memcpy(&v1,prog+len,4);
										if (v1<=0)   /* test for restore*/
										{
										 memcpy(message, prog, len);
										 message[len]=0;
										 dalarmrestore(message,current_prg+1,Station_NUM);
										 new_alarm_flag |= 0x01;  /* send the alarm to the destination panels*/
										 //resume(ALARMTASK);
										}
										prog += len;
										value = swap_double(value);  // for asix
									  memcpy(prog,&value,4);
									
									}
									
								prog += 4;
						 }
						 break;
//		case DECLARE:
//								break;
//		case REMOTE_GET:
//							 {
//							 }
//							break;
//	
//		case REMOTE_SET:
//								break;
		case FOR:
								p = prog;
								prog += 3;
								val1 = veval_exp(local);
								val2 = veval_exp(local);
								step = veval_exp(local);
								if(val2>=val1)
								{
								 put_local_var(p,val1,local);
								 prog += 2;
	/*											interpret(); */
								}
								else
								{
								 memcpy(&lvar, prog, 2);
								 lvar = swap_word(lvar);
								 prog = p_buf + lvar - 2;
								}
								break;
		case NEXT:
							 {
								memcpy(&lvar, prog, 2);
								lvar = swap_word(lvar);
								prog = p_buf + lvar - 2 + 4;
	          		p = prog;
								prog += 3;
								val1 = veval_exp(local);
								val2 = veval_exp(local);
								step = veval_exp(local);
								q = prog;
								prog = p;
								value=operand(NULL,local);    /*	veval_exp(local);*/
								value += step;
								
								put_local_var(p,value,local);
								prog = q;
								if(value<=val2)
								{									
									prog += 2;
								}
								else
								{
								 	memcpy(&lvar, prog, 2);
									lvar = swap_word(lvar);
								 	prog = p_buf + lvar - 2;
								}
							 }
							 break;
		case IF:				

								then_else = 1;

								cond = veval_exp(local);
								if(cond)
								{	
									prog++; 
									prog++;
								}
								else
								{	
									prog = (U8_T *)p_buf + swap_word(*((S16_T *)prog)) -2;
									if( *prog == 0x01 || *prog == 0xFE)      /*TEST DACA EXISTA ELSE*/
										then_else = 0;
								}
								break;
		case IFP:
								cond = veval_exp(local);
								if (cond)
								 if (!*prog++)
								 {
								 	
									*(prog-1) = 1;
									prog++; prog++;
								 }
								else
								 {
									prog = (U8_T *)p_buf + swap_word(*((S16_T *)prog)) -2;
								 }
								else
								 {
									*prog++ = 0;
									prog = (U8_T *)p_buf + swap_word(*((S16_T *)prog)) -2;
								 }
	
								then_else = 1;
								if( *prog == 0x01 || *prog == 0xFE)      /*TEST DACA EXISTA ELSE*/
									then_else = 0;
								break;
		case IFM:
								cond = veval_exp(local);
								if (!cond)
								 if (*prog++)
								 {
									*(prog-1) = 0;
									prog++; prog++;
								 }
								else
								 {
									prog = (U8_T *)p_buf + swap_word(*((S16_T *)prog)) -2;
								 }
								else
								 {
									*prog++ = 1;
									prog = (U8_T *)p_buf + swap_word(*((S16_T *)prog)) -2;
								 }
								then_else = 1;
								if( *prog == 0x01 || *prog == 0xFE)      /*TEST DACA EXISTA ELSE*/
									then_else = 0;
								break;
		case ELSE:
								/*prog++;
								prog = (U8_T *)p_buf + *((S16_T *)prog) -2;*/
								prog++;								
								prog = (U8_T *)p_buf + swap_word(*((S16_T *)prog)) -2;
								break;
		case WAIT:
								return_pointer = (S8_T *)prog - 4;
								if (*prog == 0xA1)
								{
									memcpy(&r,++prog,4);
									r = swap_double(r);
									r = 1000 * r; // T3000 send raw data, need 1000x
									prog += 4;
								}
								else
								{
									r = (U32_T)veval_exp(local);									
								}
								
								memcpy(&value,prog,4);
								value = swap_double(value);
								value += miliseclast_cur;
							
								if (value > r)
								{
									memset(prog,0,4);
									*((S16_T *)(p_buf + nbytes + 1)) = 0;
								}
								else
								{
									value = swap_double(value);
									memcpy(prog,&value,4);
								 *((S16_T *)(p_buf + nbytes + 1)) = swap_word(return_pointer - p_buf);

									return 1;
								}
								prog += 4;
								break;
		default :  
			break;
	 	}
		 
	}
	if(temp >= 2000) 
	{
// generate a alarm		
		generate_program_alarm(1,current_prg + 1);
	}
	
#endif
}


//void check_totalizers( void )
//{
//	U8_T i;
//	Str_totalizer_point *ptr;
//	S32_T ctime;
//	S32_T l;
//	ptr = totalizers;
//	for( i=0; i<MAX_TOTALIZERS; i++, ptr++ )
//	{
//		if( ptr->point.point_type )
//		{
//			if( GetByteBit(ptr->flag,totalizer_reset,1)/*ptr->reset*/ )
//			{
//				ptr->start_time = ctime;
//			//	ptr->reset = 0;
//				SetByteBit(&ptr->flag,0,totalizer_reset,1);
//				ptr->value_timeon = 0;
//				ptr->ratecount_OFF_ON_transitions = 0;
//				ptr->time_of_last_check = ctime;
//				ptr->value1 = 0;
//				ptr->count  = 0;
//			}
//			if( GetByteBit(ptr->flag,totalizer_active,1)/*ptr->active*/ )
//			{
//				if( !(GetByteBit(ptr->flag,totalizer_digital_analog,1))/*ptr->digital_analog*/ )  /* digital*/
//				if( GetByteBit(ptr->flag,totalizer_last_state,1)/*ptr->last_state*/ )
//				{
//					ptr->value_timeon += ( ctime - ptr->time_of_last_check );
//				}
//        		get_point_value( &ptr->point, &value );
//				if( GetByteBit(ptr->flag,totalizer_digital_analog,1)/*ptr->digital_analog*/ )
//				{                        /* analog*/
//					if( !(GetByteBit(ptr->flag,totalizer_rate,2))/*ptr->rate*/ ) /*sec*/
//					{
//						ptr->value_timeon += value;
//						ptr->ratecount_OFF_ON_transitions++;
//					}
//					else
//					{
//						ptr->value1 += value;
//						ptr->count++;
//						if( GetByteBit(ptr->flag,totalizer_rate,2) == 1 ) l = 60L;     /* min*/
//						if( GetByteBit(ptr->flag,totalizer_rate,2) == 2 ) l = 3600L;   /* hour*/
//						if( GetByteBit(ptr->flag,totalizer_rate,2) == 3 ) l = 86400L;  /* day*/
//
//						if(	ctime >= ptr->time_of_last_check+l )
//						{
//							ptr->count *= 1000L;
//							ptr->value_timeon += ((ptr->value1/ptr->count)*1000L)+
//																	((ptr->value1%ptr->count)*1000L)/ptr->count;
//							ptr->ratecount_OFF_ON_transitions++;
//							ptr->count  = 0;
//							ptr->value1 = 0;
//							ptr->time_of_last_check = ctime;
//						}
//					}
//				}
//				else
//				{                        /* digital*/
//				 if( value )
//				 {
//					if( !(GetByteBit(ptr->flag,totalizer_last_state,1))/*ptr->last_state*/ )
//						ptr->ratecount_OFF_ON_transitions++;
//					//ptr->last_state = 1;
//					SetByteBit(&ptr.totalizer_last_state,1);
//				 }
//				 else
//					//ptr->last_state = 0;
//					SetByteBit(&ptr->flag,0,totalizer_last_state,1);
//				 	ptr->time_of_last_check = ctime;
//				}
//			}
//		}
//	}
//}


S32_T veval_exp(U8_T *local)
{
	S16_T i, m;
//	S8_T *p;
	S32_T n;
	U8_T time_sign; 
	
	/* S32_T timer;*/
	U32_T temp1,temp2;
	Str_points_ptr sptr;
	if(*prog >= LOCAL_VARIABLE && *prog <= REMOTE_POINT_PRG )
	{
		push((operand(0,local)));
	}
	
	while( !isdelimit(*prog))         /* && code < )*/
	{	
		switch (*prog++) {	
		case PLUS:			
							op1 = pop();
							op2 = pop();
							op1 = swap_double(op1);
							op2 = swap_double(op2);
							push(swap_double(op1 + op2));
							 break;
		case MINUS:
							op1 = pop();
							op2 = pop();
							op1 = swap_double(op1);
							op2 = swap_double(op2);
							
							 push(swap_double(op2 - op1));
							 break;
		case MINUSUNAR:
							op1 = pop();
							op1 = swap_double(op1);
							push(swap_double(-op1));
							 break;
		case POW:
							 op2 = pop(); op1 = pop();
							 op1 = swap_double(op1);
							 op2 = swap_double(op2); 
							 m = op2/1000L;
							 n = op1;
							 if(m > 1)
							 {
								for(i=0;i<m-1;i++)
								 n = (n/1000L)*op1 + (n%1000L)*op1/1000L;
               }
							 push(swap_double( n ));
							 break;
		case MUL:
							 op2 = pop(); 
							 op1 = pop();
							 op1 = swap_double(op1);
							 op2 = swap_double(op2); 

							 push( swap_double((op1/1000L)*op2 + (op1%1000L)*op2/1000L ));					
							 break;
		case DIV:
							 op2 = pop(); op1 = pop();
							 op1 = swap_double(op1);
							 op2 = swap_double(op2); 
							 if(op2==0)
									push(swap_double(1000L));
							 else
								 //push( swap_double((op1/op2)*1000L + ((op1%op2)*1000L)/op2 ));
									push( swap_double((op1/op2)*1000L + ((float)(op1%op2)/op2 *1000L)));
							 break;
		case INTDIV:
								op2 = pop(); op1 = pop();
							 op1 = swap_double(op1);
							 op2 = swap_double(op2); 
							 if(op2==0)
									push(swap_double(1000L));
							 else
									push( swap_double(op1/op2)*1000L);
							 break;
		case MOD:
							 op2 = pop(); op1 = pop();
							 op1 = swap_double(op1);
							 op2 = swap_double(op2); 
							 if(op2==0)
									push(swap_double(1000L));
							 else
							 {
								 op1 = op1%op2;
								 op1 /=1000L;
								 push( swap_double(op1*1000L ));
							 }
							 break;
		case XOR:	
							 op2 = pop();
							 op1 = pop();
							 op1 = swap_double(op1);
							 op2 = swap_double(op2); 
							 op2 = op2/1000L; op1 = op1/1000L;
							 push(swap_double((op2 ^ op1)*1000L));
							 
							 break;
		case OR:
							 op1 = pop(); op2 = pop();
							 op1 = swap_double(op1);
							 op2 = swap_double(op2); 
               if( op1 || op2 )
								 push(swap_double( 1000L) );
               else
								 push(swap_double( 0 ));
							 break;
		case AND:
							 op1 = pop(); op2 = pop();
							 op1 = swap_double(op1);
							 op2 = swap_double(op2); 
			         if( op1 && op2 )
								 push(swap_double( 1000L) );
               else
								 push(swap_double( 0 ));
							 break;
		case BITOR:
							 op1 = pop(); op2 = pop();
							 op1 = swap_double(op1) / 1000;
							 op2 = swap_double(op2) / 1000; 							 
							 push(swap_double( (op1 | op2) * 1000));
							 break;
		case BITAND:
							 op1 = pop(); op2 = pop();
							 op1 = swap_double(op1) / 1000;
							 op2 = swap_double(op2) / 1000;
			         push( swap_double((op1 & op2) * 1000));
							 break;
		case NOT:
							 op1 = pop();
							 op1 = swap_double(op1);
							 op1 = !op1;
							 push(swap_double(op1));
							 break;
		case GT:
							 op2 = pop(); op1 = pop();
							 op1 = swap_double(op1);
							 op2 = swap_double(op2); 
							 push(swap_double(op1 > op2));
							 break;
		case GE:
							op2 = pop(); op1 = pop();
							op1 = swap_double(op1);
							op2 = swap_double(op2); 
							 
							 push(swap_double(op1 >= op2));
							 break;
		case LT:
							 op2 = pop(); op1 = pop();
							 op1 = swap_double(op1);
							 op2 = swap_double(op2); 
							 push(swap_double(op1 < op2));
							 break;
		case LE:
							 op2 = pop(); op1 = pop();
							 op1 = swap_double(op1);
							 op2 = swap_double(op2); 
							 push(swap_double(op1 <= op2));
							 break;
		case EQ:
							 op2 = pop(); op1 = pop();
							 op1 = swap_double(op1);
							 op2 = swap_double(op2); 
							 push(swap_double( op1 == op2 ));
							 break;
		case NE:
							 op2 = pop(); op1 = pop();
							 op1 = swap_double(op1);
							 op2 = swap_double(op2); 
							 push( swap_double(op1 != op2 ));
							 break;
		case ABS:
							op1 = pop();
							op1 = swap_double(op1); 				
							 push(swap_double(labs(op1)));
							 break;
		case LN:
/*
							 push((float)log(pop()));
*/
							 break;
		case LN_1:
/*
							 push((float)exp(pop()));
*/
							 break;
		case SQR:
							 
							 op1 = swap_double(pop());
							 op1 = (float)sqrt(op1 / 1000.0) * 1000;
							 push(swap_double(op1));
							 //push((float)sqrt(pop()));		
							 break;
		case INT:							 
							 op1 = swap_double(pop());
		           op1 = op1 / 1000l * 1000l;
							 push( swap_double(op1));
							 break;
		case SENSOR_ON:
							{
							 sptr.pin = &inputs[*(prog-3)];
							 pop();
//							 push(( GetByteBit(sptr.pin->in_sen_on,1) ) ? 1000L : 0L );  
							}
							 break;
		case SENSOR_OFF:
							{
							 sptr.pin = &inputs[*(prog-3)];
							 pop();
//							 push( ( GetByteBit(sptr.pin.in_sen_off,1)) ? 1000L : 0L );
							}
							 break;
		case INTERVAL:
							if(just_load)
							{
								n = swap_double((U32_T)pop());
								push(0);
							}
							else
							{
							 memcpy(&n,prog,4);
							 n -= miliseclast_cur;
		
							 if(n > 0)
							 {
									pop();
									push(0);
							 }
							 else
							 {
									 if(n + miliseclast_cur == LONGTIMETEST )
									 {
										 n = swap_double((U32_T)pop());
										 push(0);
									 }
									 else
									 {
										n = swap_double((U32_T)pop());
										
										push(1);
									 }
								 }
							}
							memcpy(prog,&n,4);
							prog += 4;						

							break;
	 	case TIME_ON:  
							pn = (S32_T *)(time_buf + swap_word((*(S16_T *)prog)));
							if( *((S8_T *)pn - 1) )
							 push(swap_double(*pn));
							else
							 push(0);
							prog += 2;
							break;
	 	case TIME_OFF:
							pn = (S32_T *)(time_buf + swap_word((*(S16_T *)prog)));
							if( *((S8_T *)pn - 1) )
							 push(0);
							else
							 push(swap_double(*pn));
							prog += 2;
						break;
	 	case TIME_FORMAT:

							 break;
//		case RUNTIME:
//				i = (int)pop()/1000L;
//				if(!i)
//				{
//					push(((S32_T)miliseclast_cur)*1000l);
//				}
//				else
//					push(0l);
//				break;
//	 	case Status:
//				 i = (int)pop()/1000L;
//				// if(i==Station_NUM)
//				 {
//					//	 if( Port_parameters[0].PTP_reception_state==Station_NUM )  /*OS==Station_NUM?*/
//						 {
//							push(0l);
//						 }
//					//	 else
//						 {
//							push(1000L);
//						 }
//				 }
//				 else
//				 {
//    				memcpy(&n,&panel_net_info.active_panels[0],4);
//					if( n&(1l<<(i-1)) )
//						push(1000L);
//					else
//						push(0);
//				 }
//				 break;
	 case AVG:	 			
				m = *prog++;
				value = 0;
				for(i = 0;i < m;i++)
				{
					op1 = pop();
	 				op1 = swap_double(op1); 	
					value += op1;
				}				
				push(swap_double(value / m));	
				break;
#if (ARM_MINI || ASIX_MINI)
		case COM1:	 			
				m = *prog++;
				value = 0;

				for(i = 0;i < m;i++)
				{
					op1 = pop();
	 				op1 = swap_double(op1); 	
					value += op1;
					
					if(i == 0) rs232_cmd.cmd[0] = op1 / 1000;
					if(i == 1) rs232_cmd.type = op1 / 1000;
					if(i == 2) rs232_cmd.baut = op1 / 1000;
				}	
				rs232_cmd.len = m - 2;
				send_rs232_command();
				push(swap_double(rs232_cmd.res));	
				break;
#endif
	 case MAX:
				m = *prog++;
				value = swap_double(pop());
				for(i = 0;i < m - 1;i++)
				{
					op1 = pop();
	 				op1 = swap_double(op1); 	
					if((v = op1) > value) 
						value = v;
				}
				push(swap_double(value));
				break;
	 case MIN:
				m = *prog++;
				value = swap_double(pop());
				for(i = 0;i < m - 1;i++)
				{
					op1 = pop();
	 				op1 = swap_double(op1); 
					if((v = op1) < value) 
						value = v;
				}
				push(swap_double(value));
				break;
	 case PIDPROP:
				m = swap_double(pop()) / 1000;
				i = swap_double(pop()) / 1000;

				if(m > 4000) m = 4000;
				if(m < 0) m = 0;
				n = m;
				if(controllers[i - 1].auto_manual == AUTO)
				{
					controllers[i - 1].proportional = (char)n;
					controllers[i - 1].prop_high = (char)(n / 256);
				}
				push(swap_double(n * 1000));
				break;
	 case PIDDERIV: // dtem
				value = swap_double(pop()) / 10;
				i = swap_double(pop()) / 1000;
				
 
				if(value < 0) 
					value = 0;
				//							 if(value>0.2) value=0.2;
				
				if(value > 200L) 
					value = 200L;
				//							 n=(int)(value*100);
				n = value;

				if (controllers[i - 1].auto_manual == AUTO)
					controllers[i - 1].rate = (char)n;
				//							 push(((float)n)/100, port);
				push(swap_double(n * 10));
				break;
	 case PIDINT:  // item
				 m = swap_double(pop()) / 1000;
				 i = swap_double(pop()) / 1000;
				 if(m > 255) 
					 m = 255;
				 if(m < 0) 
					 m = 0;
				 n = m;
				 if (controllers[i - 1].auto_manual == AUTO)
					 controllers[i - 1].reset = (char)n;
				 push(swap_double(n * 1000));
				 break;
	 case Tbl:
				 push(1);
				 break;
	 case WR_ON:
	 case WR_OFF:
				 op1 = pop();
				 op1 = swap_double(op1);
				 op2 = pop();
				 op2 = swap_double(op2);
				 i = op1 / 1000 - 1;  
				 m = Rtc.Clk.week - 1;
				 if (m < 0) m = 6;
			//	 p =(S8_T *)wr_times[(op2/1000)-1][m].time;
					
				if(*(prog-1)==WR_ON)
				    i = i * 2;
				else
					i = i * 2 + 1;	
				
				if(((op2 / 1000) <= 0)		||					
				((i < 0) || (i >= MAX_SCHEDULES_PER_WEEK )) ||
					(m <= 0))
				{
					push(0);
					break;
				}
				value = (S32_T)wr_times[(op2/1000)-1][m - 1].time[i].hours * 3600L + (S32_T)wr_times[(op2/1000)-1][m - 1].time[i].minutes * 60L;
				
				push(swap_double(value*1000L));				 
				 
				 break;
           
		case DOM:
				value = (Rtc.Clk.day)*1000L;
				push(swap_double(value));
            	break;
		case DOW:
				value = Rtc.Clk.week*1000L;
				push(swap_double(value));
            	break;
		case DOY:
				value = (Rtc.Clk.day_of_year) * 1000L;
				push(swap_double(value));
            	break;
		case MOY:
				value = (Rtc.Clk.mon)*1000L;
				push(swap_double(value));
            	break;
/*
					 {
						 if (*(prog-1) == DOM)
								value = (ora_current.day)*1000L;
						 if (*(prog-1) == DOW)
								value = ora_current.dayofweek*1000L;
						 if (*(prog-1) == DOY)
								value = (ora_current.day_of_year+1)*1000L;
						 push(value);
						 break;
						}
*/
		case POWER_LOSS: 
		{
			static char power_loss = 0;
			if(power_loss == 0)
			{
				power_loss = 1;
				push(swap_double(1));	
			}
			else
				push(swap_double(0));	
		}			
		break;
		case SCANS:	 push(swap_double(1));  break;  /* nr scanari pe secunda*/						 
		case SUN:		 push(swap_double(0));						 break;
		case MON:		 push(swap_double(1000));					 break;
		case TUE:		 push(swap_double(2000));					 break;
		case WED:		 push(swap_double(3000));					 break;
		case THU:		 push(swap_double(4000));					 break;
		case FRI:		 push(swap_double(5000));					 break;
		case SAT:		 push(swap_double(6000));					 break;
		case JAN:  	 push(swap_double(1000));					 break;
		case FEB:		 push(swap_double(2000));					 break;
		case MAR:		 push(swap_double(3000));					 break;
		case APR:		 push(swap_double(4000));					 break;
		case MAYM:	 push(swap_double(5000));					 break;
		case JUN:		 push(swap_double(6000));					 break;
		case JUL:		 push(swap_double(7000));					 break;
		case AUG:		 push(swap_double(8000));					 break;
		case SEP:		 push(swap_double(9000));					 break;
		case OCT:		 push(swap_double(10000));					 break;
		case NOV:		 push(swap_double(11000));					 break;
		case DEC:		 push(swap_double(12000));					 break;
		case TIME:		// hour * 100 + (60 * min + sec) / 36
//						if(Daylight_Saving_Time)  // timezone : +8 ---> 800
//						{
//							if((Rtc.Clk.day_of_year >= 105) && (Rtc.Clk.day_of_year <= 260))
//								value =  3600000L * Rtc.Clk.hour + 60000L * Rtc.Clk.min + 1000L * Rtc.Clk.sec + 3600000;
//							else
//								value =  3600000L * Rtc.Clk.hour + 60000L * Rtc.Clk.min + 1000L * Rtc.Clk.sec;
//						}
//						else
							value =  3600000L * Rtc.Clk.hour + 60000L * Rtc.Clk.min + 1000L * Rtc.Clk.sec;// - (S16_T)timezone * 36000;
								
						push(swap_double(value));
						break;
		case USER_A:	 // ethernet
//			if()
			push(swap_double(1));						 
			break;
		case USER_B:	 // mstp
			push(0);						 
			break;
		case UNACK:		 
		{
			char i;
			for(i = 0;i < MAX_ALARMS;i++)
			{
				if((alarms[i].alarm == 1) && (alarms[i].acknowledged == 0))
				{
					push(swap_double(1));
					return 1;
				}
			}
			push(0);		
		}
		break;
		case INKEYD:
					#if 1
						 i = pop()/1000;   /* offset last S8_T */
						 m = (int)(pop()/1000);   /* nr. of S8_Ts */
						 n = pop()/1000;          /* offset */
	/*
						 if( n >= i )
							 push( -1000l );
						 else
							 push( inkey( local + n, m, i-n, port)*1000l );
	*/
						 push( -1000l );
					#endif
					
						 break;
		case OUTPUTD:	
	 	//	push( outputd( local + (pop()/1000), pop()/1000, port)*1000l );				 
			break;
		case ASSIGNARRAY: /* local var */
						if (*prog >= LOCAL_VARIABLE && *prog <= STRING_TYPE_ARRAY)   
							push( ((S32_T)*((S16_T *)(prog+1)))*1000L );
						//	push( swap_double(((S32_T)*((S16_T *)(prog+1)))*1000L ));
						 prog += 3;
						 break;
		case ASSIGNARRAY_1:
//						push(getvalelem(1, pop()/1000L, local));
						 break;
		case ASSIGNARRAY_2:
//						 push(getvalelem(pop()/1000L, pop()/1000L, local));
						 break;
		default:
								prog--;
				push(operand(0,local));
	}
 }	 

	if (*prog==0xFF) 	{ prog++;  }
	temp1 = pop();
	temp2 = swap_double(temp1);
	
	return (temp2);
}


/*
 * ----------------------------------------------------------------------------
 * Function Name: operand
 * Purpose: 
 * Params:
 * Returns:
 * Note: this function is called in veval_exp() routions
 * ----------------------------------------------------------------------------
 */
S32_T operand(S8_T **buf,U8_T *local)
{
	S8_T *p;
	S16_T num;
	value = 0;
	if (*prog >= LOCAL_VARIABLE && *prog <= BYTE_TYPE)    /* local var */
	{
		prog += 3;
		if(buf)			*buf=0;
		return localvalue(prog-3, local);
	}
	
	if (*prog == LOCAL_POINT_PRG)
	{
		/*if( (((Point *)(prog+1))->point_type)-1 == ARRAY )
		{
			++prog;
			get_ay_elem(&value, local);
		}
		else */
		{	
			get_point_value( ( (Point *)(++prog) ), &value );
			prog += sizeof(Point);

		}
		return value;              /* = read point */
	}
	if (*prog == REMOTE_POINT_PRG)
	{	
		if( (((Point_Net *)(prog+1))->point_type)-1 == ARRAY)
		{
			++prog;
			p = prog;
			prog += sizeof(Point_Net);
			num = veval_exp(local) / 1000L - 1;
			get_net_point_value( (Point_Net *)p, &value,0,0 );
		}
		else
		{	// if exist remote point, try to scan network
#if (ARM_MINI || ARM_CM5 )
			flag_start_scan_network = 1;
			start_scan_network_count = 0;
#endif
			get_net_point_value( ( (Point_Net *)(++prog) ), &value,0,0 );
			prog += sizeof( Point_Net );
		}	
	/*	get_remote_point_value(*((Point_Net *)(++prog)), &value, buf);
	prog += sizeof(Point_Net);*/
		return value;              /* = read point */
	}

	if (*prog == CONST_VALUE_PRG)
	{	  
		prog += 5;
		if(buf)		*buf=0;
		return *((S32_T *)(prog-4));
	}
	return 0;
}


void push(S32_T value)
{
 	*index_stack++ = value;
}


S32_T pop(void)
{
 	return (*(--index_stack));
}

void pushlong(unsigned long value)
{
 memcpy(index_stack++, &value, 4);
}


unsigned long poplong(void)
{
 unsigned long value;
 memcpy( &value, --index_stack, 4);
 return (value);
}



//S16_T phone(S8_T *message,S16_T i)    /* phone call*/
//{
//	message[i] = '\r';
//	message[i+1] = 0;
//	memmove(message+4, message, i+1);
//	memcpy(message, "ATDT", 4);
//	//outputd( message, i+5, COM2);
//}
//
//
//S16_T print(S8_T *message)  /* print to printer*/
//{
///*
//	set_semaphore(&print_sem);
//	if( print_message_pool.put( message, strlen(message)+1 ) )
//	{
//	action=1;
//	print_flag=1;
//	if( tasks[MISCELLANEOUS].status == SUSPENDED )
//		 resume(MISCELLANEOUS);
//	}
//	clear_semaphore(&prS16_T_sem);
//	return 0;
//*/
//}
//
//S16_T	handup()    	/* end phone call*/
//{
////	STR_PTP *ps;
////	ps = &Port_parameters[COM2];
////	outputd( "ATH0\r", 5, COM2);
///*	reset_tx_port( ps->port );*/
//}


/*
 * ----------------------------------------------------------------------------
 * Function Name: put_local_array
 * Purpose: put the value to the array
 * Params:
 * Returns:
 * Note: it is used in excu_program(ASSIGNARRAY_1,ASSIGNARRAY_2)
 * ----------------------------------------------------------------------------
 */
S16_T put_local_array(U8_T *p, S32_T value, S32_T v1, S32_T v2, U8_T *local )
{
	S16_T k, i, j;

  	j = *((S16_T *)(p+1));
	if( *((S16_T *)&local[ j - 4]) )
	{
		if ( v1<=0 || v1 > *((S16_T *)&local[ j - 4]) || v2<=0 || v2 > *((S16_T *)&local[ j - 2]))
			return 1;
	}
	else
	{
	 	if ( v2<=0 || v2 > *((S16_T *)&local[ j - 2]))
			return 1;
	}
	switch(*p)
	{
		case FLOAT_TYPE_ARRAY:
		case LONG_TYPE_ARRAY:
				k=4;
				break;
		case INTEGER_TYPE_ARRAY:
				k=2;
				break;
		case BYTE_TYPE_ARRAY:
/*		case STRING_TYPE_ARRAY: */
				k=1;
				break;
	}

	i =  *((S16_T *)&local[((j)-2)]);
	i *= (v1-1);
	i += v2 - 1;
	i *= k;
	k = j + i;

	switch(*p)
	{
		case FLOAT_TYPE_ARRAY:
		case LONG_TYPE_ARRAY:
			*((S32_T *)&local[k])=value;							break;
		case INTEGER_TYPE_ARRAY:
			*((S16_T *)&local[k])=(S16_T)(value/1000L);				break;
		case BYTE_TYPE_ARRAY:
/*		case STRING_TYPE_ARRAY: */
			local[k]=(S8_T)(value/1000L);							break;
	}
	return 0;
}


/*
 * ----------------------------------------------------------------------------
 * Function Name: localvalue
 * Purpose: get the local value when *p is (float long, integer,byte)
 * Params:
 * Returns:
 * Note: it is used in operand()
 * ----------------------------------------------------------------------------
 */
S32_T localvalue(U8_T *p, U8_T *local)
{
	S32_T l;
	S16_T i;
	i = *((S16_T *)(p+1));
	switch(*p)
	{
		case FLOAT_TYPE:
		case LONG_TYPE:
				l = *((S32_T *)&local[i]);
				break;
		case INTEGER_TYPE:
				l = (S32_T)(*((S16_T *)&local[i]))*1000L;
				break;
		case BYTE_TYPE:
				l = (S32_T)((S8_T)local[i])*1000L;
				break;
	}
	return l;
}



/*
 * ----------------------------------------------------------------------------
 * Function Name: put_local_var
 * Purpose: put the local value when *p is (float long, integer,byte)
 * Params:
 * Returns:
 * Note: it is used in operand()
 * ----------------------------------------------------------------------------
 */
void put_local_var(U8_T *p, S32_T value, U8_T *local)
{
	S16_T i;
	i = *((S16_T *)(p+1));
	
	switch(*p)
	{
		case FLOAT_TYPE:
		case LONG_TYPE:
				*((S32_T *)&local[i])=value;
				break;
		case INTEGER_TYPE: 
				*((S16_T *)&local[i])=(S16_T)(value/1000L);
			
				break;
		case BYTE_TYPE:
				local[i]=(S8_T)(value/1000L);
				break;
	}
}

#endif