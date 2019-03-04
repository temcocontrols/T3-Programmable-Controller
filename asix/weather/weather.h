#ifndef	WEATHER_SERVER_H
#define	WEATHER_SERVER_H

#include "types.h"
#include "ud_str.h"

typedef struct
{
	struct
	{
		char Country[15];
		char State[15];
		char City[15];
		char Street[20];
		char Zip[10];
		uint16_t longitude; // high byte is East or West
		uint16_t latitude;  // high byte is North or South
		uint16_t elev;
	}LOC; // location
	
	// outdoor sensor
	Point_Net temper_id;
	uint32_t temper_value;
	
	Point_Net humidity_id;
	uint32_t humidity_value;
	
	// remote weather station services
	struct
	{
		uint8_t enable_service;
		char location[30];
		uint16_t dis_from_building;
		uint16_t last_update;

		uint16_t temper;
		uint16_t hum;
		uint16_t barometer;
		uint16_t wind_spd;
		uint8_t wind_dir;
		
		uint8_t enable_cal;
		uint16_t max_adjust_per_day;
		uint16_t total_currect_offset;
	}Station;
}STR_WEATHER;


extern STR_WEATHER  far weather;

#endif