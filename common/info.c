#include "product.h"


#if ASIX_MINI
unsigned char code pro_info[20] = {'T','e','m','c','o','M','i','n','i','P','a','n','e','l', 0 ,SW_REV % 100, SW_REV / 100,  0, 0, 0}; 
#endif

#if ASIX_CM5
unsigned char code pro_info[20] = {'T','e','m','c','o','C','M','5', 0 , 0 , 0 , 0 , 0 , 0 , 0 ,SW_REV % 100, SW_REV / 100,  0, 0, 0};
#endif

#if ARM_MINI
const uint8 pro_info[20] __attribute__((at(0x08008200))) = {'T','e','m','c','o','M','i','n','i','_','a','r','m',0 , 0 ,(SW_REV)&0xff , (SW_REV>>8)&0xff,  0, 0, 0}; 
#endif

#if ARM_CM5
const uint8 pro_info[20] __attribute__((at(0x08008200))) = {'T','e','m','c','o','C','M','5','_' ,'A','R','M', 0 , 0 , 0 ,SW_REV % 100, SW_REV / 100,  0, 0, 0};
#endif

#if ARM_TSTAT_WIFI
const uint8 pro_info[20] __attribute__((at(0x08008200))) = {'T','e','m','c','o','P','I','D','1','0',0,0,0,0,0,(SW_REV)&0xff , (SW_REV>>8)&0xff,  0, 0, 0}; 
//const uint8 pro_info[20] __attribute__((at(0x08008200))) = {'T','e','m','c','o','T','S','T','A','T','8',0,0,0,0,(SW_REV)&0xff , (SW_REV>>8)&0xff,  0, 0, 0}; 

#endif	

	



	