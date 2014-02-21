
//#include "bacnet.h"
#include "ud_str.h"
#include "types.h"

//uint16_t search_point( U8_T *type, U8_T *number, S8_T *buff, Search_type order );
void update_alarm_tbl( Alarm_point *block, S16_T max_points_bank );
S16_T put_net_point_value( Point_Net *point, S32_T *val_ptr, S16_T aux, S16_T prog_op );
S16_T put_point_value( Point *point, S32_T *val_ptr, S16_T aux, S16_T prog_op );
void put_remote_point_value( S16_T index, S32_T *val_ptr, S16_T prog_op );
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
S16_T get_net_point_value( Point_Net *p, S32_T *val_ptr );


