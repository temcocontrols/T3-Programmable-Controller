#ifndef	_FLASH_SCHEDULE_H
#define	_FLASH_SCHEDULE_H

typedef enum
{
	T_WEEK_DES = 0,
	T_WEEK_ONTIME,
	T_WEEK_OFFTIME,
	T_ANNUAL_DES,
	T_ANNUAL_TIME,
	T_ID,

	T_NAME, // ONLY FOR CM5
	T_END_OLD
};

/* MACRO DECLARATIONS */
typedef struct
{
	U16_T addr;
	U16_T len;
}STR_Flash_POS;


typedef struct
{
	U8_T table;
	U16_T index;
	U8_T flag;
	U32_T len;
	U8_T dat[500];
}STR_flag_flash;


void Flash_Inital(void);
void Flash_Write_Mass(void);
void Flash_Read_Mass(void);


extern STR_flag_flash 	far bac_flash;

#endif

