#ifndef POINT_H
#define POINT_H

#include "ud_str.h"
#include "types.h"


//uint16_t search_point( U8_T *type, U8_T *number, S8_T *buff, Search_type order );
void update_alarm_tbl( Alarm_point *block, S16_T max_points_bank );
S16_T put_net_point_value( Point_Net *point, S32_T *val_ptr, S16_T aux, S16_T prog_op , U8_T mode );
S16_T put_point_value( Point *point, S32_T *val_ptr, S16_T aux, S16_T prog_op );
//void put_remote_point_value( S16_T index, S32_T *val_ptr, S16_T prog_op );
//S16_T put_point_info( Point_info *info );
S16_T get_point_value( Point *point, S32_T *val_ptr );
S16_T get_remote_point_value( S16_T index, S32_T *val_ptr );
S16_T insert_remote_point( Point_Net *point, S16_T index );
S16_T delete_remote_point( Point_Net *point, S16_T index );
//S16_T get_point_info( Point_info *ptr );
//S16_T update_grp_element( Str_grp_element *ptr );
/*	int putmessage( void ); *//* ind alarm_set */
//S16_T writepropertyvalue( BACnetObjectIdentifier *obj, S32_T lvalue );
//S16_T writepropertyauto( BACnetObjectIdentifier *obj, S16_T auto_manual );
//void move_groups( S8_T *dest, S8_T *source, S16_T length,S16_T no_elem, Str_grp_element *address );
S32_T localvalue(U8_T *p, U8_T *local);
S16_T find_remote_poS16_T( Point_Net *point );
S16_T get_net_point_value( Point_Net *p, S32_T *val_ptr , U8_T mode ,U8_T flag);
//int GetPrivateData(Point_Net point,U32_T deviceid);
//void WritePrivateData(Point_Net point,S32_T value);
//S8_T put_timer_value(U8_T pro_index,U8_T type,U32_T time,char *messgae); 
//U32_T check_pro_timer_count(U8_T index,U32_T time,U8_T type,U8_T time_sign);
void initial_graphic_point(void);
//void clear_pro_timer(U8_T prg_index,U32_T time,U8_T type);

int GetRemotePoint(uint8_t object_type,uint32_t object_instance,uint8_t panel,uint8_t sub,uint8_t protocal);
int WriteRemotePoint(uint8_t object_type,uint32_t object_instance,uint8_t panel,uint8_t sub,float value,uint8_t protocal);


void Check_Net_Point_Table(void);

S8_T get_point_info_by_instacne(Point_Net * point);
U8_T check_point_type(Point_Net * point);


#endif

