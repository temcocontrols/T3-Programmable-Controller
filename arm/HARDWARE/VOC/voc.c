#include "sgp30.h"
#include "sensirion_i2c.h"
#include "types.h"
#include "delay.h"
#include "define.h"
#include "e2prom.h"

#if ARM_TSTAT_WIFI
uint8 voc_ok;
uint16 voc_value = 0;
uint32 iaq_baseline;
uint8 voc_ini_baseline;
extern uint16 Test[50];

void VOC_IIC_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//Ê¹ÄÜSCL
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_3);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA,  GPIO_Pin_2);
}

uint8_t voc_baseline[4];
void VOC_Init(void)
{
		int16_t err;
		
		uint32_t iaq_baseline;
		uint16_t ethanol_raw_signal, h2_raw_signal;
		uint32 temp;
		uint8 temp1[4];
		voc_ini_baseline = 1;
		//VOC_IIC_Init();
		voc_ok = 1;
    temp = 0;
		while(sgp30_probe() != STATUS_OK) {
			temp++;
			if(temp>20)
			{
				voc_ok = 0;
				break;
			}
			delay_ms(100);
    }
		temp = 0;

		if(voc_ok)
		{
			uint16_t feature_set_version;
			uint8_t product_type;
			err = sgp30_get_feature_set_version(&feature_set_version, &product_type);

			uint64_t serial_id;
			err = sgp30_get_serial_id(&serial_id);
			err = sgp30_measure_raw_blocking_read(&ethanol_raw_signal, &h2_raw_signal); 
			err = sgp30_iaq_init();
//			E2prom_Read_Byte(EEP_VOC_BASELINE1,&voc_baseline[0]);
//			E2prom_Read_Byte(EEP_VOC_BASELINE2,&voc_baseline[1]);
//			E2prom_Read_Byte(EEP_VOC_BASELINE3,&voc_baseline[2]);
//			E2prom_Read_Byte(EEP_VOC_BASELINE4,&voc_baseline[3]);
			
//		Test[16] = voc_baseline[0];
//		Test[17] = voc_baseline[1];
//		Test[18] = voc_baseline[2];
//		Test[19] = voc_baseline[3];
//			if((voc_baseline[0] == 0xff) && (voc_baseline[1] == 0xff))
//			{
//				voc_baseline[0] = 0;
//				voc_baseline[1] = 0;
//				voc_baseline[2] = 0;
//				voc_baseline[3] = 0;
//			}
//			
//			iaq_baseline = 
//				voc_baseline[0] + (U16_T)(voc_baseline[1] << 8) + ((U32_T)voc_baseline[2] << 16) + ((U32_T)voc_baseline[3] << 24);

//			if(iaq_baseline == 0xffffffff)
//			{
//			err = sgp30_set_iaq_baseline(iaq_baseline);
//			if(err == STATUS_OK)
//				;//printf(" set baseline: OK");
//			delay_ms(100);
//			}		
		}
}


uint16 voc_buf[5];

void Check_Voc(void)
{
	static uint8 voc_cnt;
	uint16_t tvoc_ppb, co2_eq_ppm;
	uint32 temp;
	uint8 i;
	int16_t err;
	static	uint16 baseline_time = 0; 

	if(voc_ok)
	{
		if(sgp30_measure_tvoc() == STATUS_OK)
		{
			delay_ms(100);
			if( sgp30_read_tvoc(&tvoc_ppb) == STATUS_OK)
			{
				voc_buf[voc_cnt] = tvoc_ppb;
				if(voc_cnt < 4)
					voc_cnt++;
				else
				{
					temp = 0;
					for(i = 0;i < 5;i++)
					{
						temp += voc_buf[i];
					}

					voc_value = temp/5;
					if(Modbus.mini_type == MINI_T10P)
						inputs[HI_COMMON_CHANNEL + 1].value = voc_value * 1000;
					else
						inputs[COMMON_CHANNEL + 1].value = voc_value * 1000;
				
					voc_cnt = 0;
				}				
			}
		}

		// Persist the current baseline every hour 
		if(++baseline_time % 3600 == 3599) 
		{
			err = sgp30_get_iaq_baseline(&iaq_baseline);
			if (err == STATUS_OK) 
			{
				//printf("baseline: %u\n", iaq_baseline);
//				E2prom_Write_Byte(EEP_VOC_BASELINE1, iaq_baseline & 0XFF);  // IMPLEMENT: store baseline to presistent storage 
//				E2prom_Write_Byte(EEP_VOC_BASELINE2, (iaq_baseline>>8) & 0XFF);
//				E2prom_Write_Byte(EEP_VOC_BASELINE3, (iaq_baseline>>16) & 0XFF);
//				E2prom_Write_Byte(EEP_VOC_BASELINE4, (iaq_baseline>>24) & 0XFF);
//				voc_baseline[0] = iaq_baseline & 0XFF;
//				voc_baseline[1] = (iaq_baseline>>8)& 0XFF;
//				voc_baseline[2] = (iaq_baseline>>16) & 0XFF;
//				voc_baseline[3] = (iaq_baseline>>24) & 0XFF;
				
			}
		}
			
		if(voc_ini_baseline == 1)
		{
			err = sgp30_iaq_init();
			if (err == STATUS_OK) {
				voc_ini_baseline = 0;
				//printf("sgp30_iaq_init done\n");
			} else 
					;//printf("sgp30_iaq_init failed!\n");			
		}
	}  					
}

#endif
