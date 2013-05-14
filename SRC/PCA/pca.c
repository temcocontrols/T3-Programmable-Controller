/*
 *********************************************************************************
 *     Copyright (c) 2005   ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 *********************************************************************************
 */
/*================================================================================
 * Module Name : pca.c
 * Purpose     : This file include PCA interrupt service routine and
 *               PCA driver.
 * Author      : Robin Lee
 * Date        : 2006-06-23
 * Notes       :
 * $Log: pca.c,v $
 * Revision 1.0  2009-01-05 14:03:53+08  wong
 * Initial revision
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include	"reg80390.h"
#include	"types.h"
#include	"pca.h"
#include	"pca_cfg.h"

#if 1
/* STATIC VARIABLE DECLARATIONS */
static U8_T		pcaIntrFlag = 0;
static U16_T	pcaCapVal[5] = {0};
static U16_T	pcaCexVal[5] = {0};


/* LOCAL SUBPROGRAM DECLARATIONS */
static void pca_ISR(void);
#if PCA_CAP_POS_ON_MODULE_0
	static void pca_CapturePosOnModule0(void);
#endif
#if PCA_CAP_POS_ON_MODULE_1
	static void pca_CapturePosOnModule1(void);
#endif
#if PCA_CAP_POS_ON_MODULE_2
	static void pca_CapturePosOnModule2(void);
#endif
#if PCA_CAP_POS_ON_MODULE_3
	static void pca_CapturePosOnModule3(void);
#endif
#if PCA_CAP_POS_ON_MODULE_4
	static void pca_CapturePosOnModule4(void);
#endif

#if PCA_CAP_NEG_ON_MODULE_0
	static void pca_CaptureNegOnModule0(void);
#endif
#if PCA_CAP_NEG_ON_MODULE_1
	static void pca_CaptureNegOnModule1(void);
#endif
#if PCA_CAP_NEG_ON_MODULE_2
	static void pca_CaptureNegOnModule2(void);
#endif
#if PCA_CAP_NEG_ON_MODULE_3
	static void pca_CaptureNegOnModule3(void);
#endif
#if PCA_CAP_NEG_ON_MODULE_4
	static void pca_CaptureNegOnModule4(void);
#endif

#if PCA_CAP_POS_NEG_ON_MODULE_0
	static void pca_CapturePosNegOnModule0(void);
#endif
#if PCA_CAP_POS_NEG_ON_MODULE_1
	static void pca_CapturePosNegOnModule1(void);
#endif
#if PCA_CAP_POS_NEG_ON_MODULE_2
	static void pca_CapturePosNegOnModule2(void);
#endif
#if PCA_CAP_POS_NEG_ON_MODULE_3
	static void pca_CapturePosNegOnModule3(void);
#endif
#if PCA_CAP_POS_NEG_ON_MODULE_4
	static void pca_CapturePosNegOnModule4(void);
#endif

#if PCA_SW_TIMER_ON_MODULE_0
	static void pca_SoftwareTimerOnModule0(void);
#endif
#if PCA_SW_TIMER_ON_MODULE_1
	static void pca_SoftwareTimerOnModule1(void);
#endif
#if PCA_SW_TIMER_ON_MODULE_2
	static void pca_SoftwareTimerOnModule2(void);
#endif
#if PCA_SW_TIMER_ON_MODULE_3
	static void pca_SoftwareTimerOnModule3(void);
#endif
#if PCA_SW_TIMER_ON_MODULE_4
	static void pca_SoftwareTimerOnModule4(void);
#endif

#if PCA_HI_SPEED_OUT_ON_MODULE_0
	static void pca_HighSpeedOutputOnModule0(void);
#endif
#if PCA_HI_SPEED_OUT_ON_MODULE_1
	static void pca_HighSpeedOutputOnModule1(void);
#endif
#if PCA_HI_SPEED_OUT_ON_MODULE_2
	static void pca_HighSpeedOutputOnModule2(void);
#endif
#if PCA_HI_SPEED_OUT_ON_MODULE_3
	static void pca_HighSpeedOutputOnModule3(void);
#endif
#if PCA_HI_SPEED_OUT_ON_MODULE_4
	static void pca_HighSpeedOutputOnModule4(void);
#endif

#if PCA_8BIT_PWM_ON_MODULE_0
	static void pca_8bitPwmOnModule0(void);
#endif
#if PCA_8BIT_PWM_ON_MODULE_1
	static void pca_8bitPwmOnModule1(void);
#endif
#if PCA_8BIT_PWM_ON_MODULE_2
	static void pca_8bitPwmOnModule2(void);
#endif
#if PCA_8BIT_PWM_ON_MODULE_3
	static void pca_8bitPwmOnModule3(void);
#endif
#if PCA_8BIT_PWM_ON_MODULE_4
	static void pca_8bitPwmOnModule4(void);
#endif


/* LOCAL SUBPROGRAM BODIES */

/*
 *--------------------------------------------------------------------------------
 * static void PcaISR(void)
 * Purpose : 
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
static void pca_ISR(void) interrupt 8 //use interrupt 3 (0x43)
{
	pcaIntrFlag = CCON & (PCA_CCF0|PCA_CCF1|PCA_CCF2|PCA_CCF3|PCA_CCF4|PCA_CF);

#if PCA_CAP_POS_ON_MODULE_0
	pca_CapturePosOnModule0();
#endif
#if PCA_CAP_POS_ON_MODULE_1
	pca_CapturePosOnModule1();
#endif
#if PCA_CAP_POS_ON_MODULE_2
	pca_CapturePosOnModule2();
#endif
#if PCA_CAP_POS_ON_MODULE_3
	pca_CapturePosOnModule3();
#endif
#if PCA_CAP_POS_ON_MODULE_4
	pca_CapturePosOnModule4();
#endif

#if PCA_CAP_NEG_ON_MODULE_0
	pca_CaptureNegOnModule0();
#endif
#if PCA_CAP_NEG_ON_MODULE_1
	pca_CaptureNegOnModule1();
#endif
#if PCA_CAP_NEG_ON_MODULE_2
	pca_CaptureNegOnModule2();
#endif
#if PCA_CAP_NEG_ON_MODULE_3
	pca_CaptureNegOnModule3();
#endif
#if PCA_CAP_NEG_ON_MODULE_4
	pca_CaptureNegOnModule4();
#endif

#if PCA_CAP_POS_NEG_ON_MODULE_0
	pca_CapturePosNegOnModule0();
#endif
#if PCA_CAP_POS_NEG_ON_MODULE_1
	pca_CapturePosNegOnModule1();
#endif
#if PCA_CAP_POS_NEG_ON_MODULE_2
	pca_CapturePosNegOnModule2();
#endif
#if PCA_CAP_POS_NEG_ON_MODULE_3
	pca_CapturePosNegOnModule3();
#endif
#if PCA_CAP_POS_NEG_ON_MODULE_4
	pca_CapturePosNegOnModule4();
#endif

#if PCA_SW_TIMER_ON_MODULE_0
	pca_SoftwareTimerOnModule0();
#endif
#if PCA_SW_TIMER_ON_MODULE_1
	pca_SoftwareTimerOnModule1();
#endif
#if PCA_SW_TIMER_ON_MODULE_2
	pca_SoftwareTimerOnModule2();
#endif
#if PCA_SW_TIMER_ON_MODULE_3
	pca_SoftwareTimerOnModule3();
#endif
#if PCA_SW_TIMER_ON_MODULE_4
	pca_SoftwareTimerOnModule4();
#endif

#if PCA_HI_SPEED_OUT_ON_MODULE_0
	pca_HighSpeedOutputOnModule0();
#endif
#if PCA_HI_SPEED_OUT_ON_MODULE_1
	pca_HighSpeedOutputOnModule1();
#endif
#if PCA_HI_SPEED_OUT_ON_MODULE_2
	pca_HighSpeedOutputOnModule2();
#endif
#if PCA_HI_SPEED_OUT_ON_MODULE_3
	pca_HighSpeedOutputOnModule3();
#endif
#if PCA_HI_SPEED_OUT_ON_MODULE_4
	pca_HighSpeedOutputOnModule4();
#endif

#if PCA_8BIT_PWM_ON_MODULE_0
	pca_8bitPwmOnModule0();
#endif
#if PCA_8BIT_PWM_ON_MODULE_1
	pca_8bitPwmOnModule1();
#endif
#if PCA_8BIT_PWM_ON_MODULE_2
	pca_8bitPwmOnModule2();
#endif
#if PCA_8BIT_PWM_ON_MODULE_3
	pca_8bitPwmOnModule3();
#endif
#if PCA_8BIT_PWM_ON_MODULE_4
	pca_8bitPwmOnModule4();
#endif
}

/*
 *--------------------------------------------------------------------------------
 * static void pca_CapturePosOnModule0(void)
 * Purpose : 
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if PCA_CAP_POS_ON_MODULE_0
static void pca_CapturePosOnModule0(void)
{
	if (pcaIntrFlag & PCA_CCF0)
	{
		pcaIntrFlag &= ~PCA_CCF0;
		
		pcaCapVal[PCA_MODULE0] = (U16_T)CCAPL0 | ((U16_T)CCAPH0 << 8);
	}
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void pca_CapturePosOnModule1(void)
 * Purpose : 
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if PCA_CAP_POS_ON_MODULE_1
static void pca_CapturePosOnModule1(void)
{
	if (pcaIntrFlag & PCA_CCF1)
	{
		pcaIntrFlag &= ~PCA_CCF1;
		
		pcaCapVal[PCA_MODULE1] = (U16_T)CCAPL1 | ((U16_T)CCAPH1 << 8);
	}
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void pca_CapturePosOnModule2(void)
 * Purpose : 
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if PCA_CAP_POS_ON_MODULE_2
static void pca_CapturePosOnModule2(void)
{
	if (pcaIntrFlag & PCA_CCF2)
	{
		pcaIntrFlag &= ~PCA_CCF2;
		
		pcaCapVal[PCA_MODULE2] = (U16_T)CCAPL2 | ((U16_T)CCAPH2 << 8);
	}
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void pca_CapturePosOnModule3(void)
 * Purpose : 
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if PCA_CAP_POS_ON_MODULE_3
static void pca_CapturePosOnModule3(void)
{
	if (pcaIntrFlag & PCA_CCF3)
	{
		pcaIntrFlag &= ~PCA_CCF3;
		
		pcaCapVal[PCA_MODULE3] = (U16_T)CCAPL3 | ((U16_T)CCAPH3 << 8);
	}
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void pca_CapturePosOnModule4(void)
 * Purpose : 
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if PCA_CAP_POS_ON_MODULE_4
static void pca_CapturePosOnModule4(void)
{
	if (pcaIntrFlag & PCA_CCF4)
	{
		pcaIntrFlag &= ~PCA_CCF4;
		
		pcaCapVal[PCA_MODULE4] = (U16_T)CCAPL4 | ((U16_T)CCAPH4 << 8);
	}
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void pca_CaptureNegOnModule0(void)
 * Purpose : 
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if PCA_CAP_NEG_ON_MODULE_0
static void pca_CaptureNegOnModule0(void)
{
	if (pcaIntrFlag & PCA_CCF0)
	{
		pcaIntrFlag &= ~PCA_CCF0;

		pcaCapVal[PCA_MODULE0] = (U16_T)CCAPL0 | ((U16_T)CCAPH0 << 8);
	}
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void pca_CaptureNegOnModule1(void)
 * Purpose : 
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if PCA_CAP_NEG_ON_MODULE_1
static void pca_CaptureNegOnModule1(void)
{
	if (pcaIntrFlag & PCA_CCF1)
	{
		pcaIntrFlag &= ~PCA_CCF1;

		pcaCapVal[PCA_MODULE1] = (U16_T)CCAPL1 | ((U16_T)CCAPH1 << 8);
	}
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void pca_CaptureNegOnModule2(void)
 * Purpose : 
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if PCA_CAP_NEG_ON_MODULE_2
static void pca_CaptureNegOnModule2(void)
{
	if (pcaIntrFlag & PCA_CCF2)
	{
		pcaIntrFlag &= ~PCA_CCF2;

		pcaCapVal[PCA_MODULE2] = (U16_T)CCAPL2 | ((U16_T)CCAPH2 << 8);
	}
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void pca_CaptureNegOnModule3(void)
 * Purpose : 
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if PCA_CAP_NEG_ON_MODULE_3
static void pca_CaptureNegOnModule3(void)
{
	if (pcaIntrFlag & PCA_CCF3)
	{
		pcaIntrFlag &= ~PCA_CCF3;

		pcaCapVal[PCA_MODULE3] = (U16_T)CCAPL3 | ((U16_T)CCAPH3 << 8);
	}
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void pca_CaptureNegOnModule4(void)
 * Purpose : 
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if PCA_CAP_NEG_ON_MODULE_4
static void pca_CaptureNegOnModule4(void)
{
	if (pcaIntrFlag & PCA_CCF4)
	{
		pcaIntrFlag &= ~PCA_CCF4;

		pcaCapVal[PCA_MODULE4] = (U16_T)CCAPL4 | ((U16_T)CCAPH4 << 8);
	}
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void pca_CapturePosNegOnModule0(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if PCA_CAP_POS_NEG_ON_MODULE_0
static void pca_CapturePosNegOnModule0(void)
{
	if (pcaIntrFlag & PCA_CCF0)
	{
		pcaIntrFlag &= ~PCA_CCF0;

		pcaCapVal[PCA_MODULE0] = (U16_T)CCAPL0 | ((U16_T)CCAPH0 << 8);
	}
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void pca_CapturePosNegOnModule1(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if PCA_CAP_POS_NEG_ON_MODULE_1
static void pca_CapturePosNegOnModule1(void)
{
	if (pcaIntrFlag & PCA_CCF1)
	{
		pcaIntrFlag &= ~PCA_CCF1;

		pcaCapVal[PCA_MODULE1] = (U16_T)CCAPL1 | ((U16_T)CCAPH1 << 8);
	}
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void pca_CapturePosNegOnModule2(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if PCA_CAP_POS_NEG_ON_MODULE_2
static void pca_CapturePosNegOnModule2(void)
{
	if (pcaIntrFlag & PCA_CCF2)
	{
		pcaIntrFlag &= ~PCA_CCF2;

		pcaCapVal[PCA_MODULE2] = (U16_T)CCAPL2 | ((U16_T)CCAPH2 << 8);
	}
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void pca_CapturePosNegOnModule3(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if PCA_CAP_POS_NEG_ON_MODULE_3
static void pca_CapturePosNegOnModule3(void)
{
	if (pcaIntrFlag & PCA_CCF3)
	{
		pcaIntrFlag &= ~PCA_CCF3;

		pcaCapVal[PCA_MODULE3] = (U16_T)CCAPL3 | ((U16_T)CCAPH3 << 8);
	}
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void pca_CapturePosNegOnModule4(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if PCA_CAP_POS_NEG_ON_MODULE_4
static void pca_CapturePosNegOnModule4(void)
{
	if (pcaIntrFlag & PCA_CCF4)
	{
		pcaIntrFlag &= ~PCA_CCF4;

		pcaCapVal[PCA_MODULE4] = (U16_T)CCAPL4 | ((U16_T)CCAPH4 << 8);
	}
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void pca_SoftwareTimerOnModule0(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if PCA_SW_TIMER_ON_MODULE_0
static void pca_SoftwareTimerOnModule0(void)
{
	U16_T	tCount = 0;

	if (pcaIntrFlag & PCA_CCF0)
	{
		EINT3 = 0;
		pcaIntrFlag &= ~PCA_CCF0;

		tCount = (U16_T)CCAPL0 | ((U16_T)CCAPH0 << 8);
		tCount += PCA_SW_TIMER_TRIG_COUNT;

		CCAPL0 = (U8_T)tCount;
		CCAPH0 = (U8_T)(tCount >> 8);
		EINT3 = 1;
	}
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void pca_SoftwareTimerOnModule1(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if PCA_SW_TIMER_ON_MODULE_1
static void pca_SoftwareTimerOnModule1(void)
{
	U16_T	tCount = 0;

	if (pcaIntrFlag & PCA_CCF1)
	{
		EINT3 = 0;
		pcaIntrFlag &= ~PCA_CCF1;

		tCount = (U16_T)CCAPL1 | ((U16_T)CCAPH1 << 8);
		tCount += PCA_SW_TIMER_TRIG_COUNT;

		CCAPL1 = (U8_T)tCount;
		CCAPH1 = (U8_T)(tCount >> 8);
		EINT3 = 1;
	}
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void pca_SoftwareTimerOnModule2(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if PCA_SW_TIMER_ON_MODULE_2
static void pca_SoftwareTimerOnModule2(void)
{
	U16_T	tCount = 0;

	if (pcaIntrFlag & PCA_CCF2)
	{
		EINT3 = 0;
		pcaIntrFlag &= ~PCA_CCF2;

		tCount = (U16_T)CCAPL2 | ((U16_T)CCAPH2 << 8);
		tCount += PCA_SW_TIMER_TRIG_COUNT;

		CCAPL2 = (U8_T)tCount;
		CCAPH2 = (U8_T)(tCount >> 8);
		EINT3 = 1;
	}
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void pca_SoftwareTimerOnModule3(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if PCA_SW_TIMER_ON_MODULE_3
static void pca_SoftwareTimerOnModule3(void)
{
	U16_T	tCount = 0;

	if (pcaIntrFlag & PCA_CCF3)
	{
		EINT3 = 0;
		pcaIntrFlag &= ~PCA_CCF3;

		tCount = (U16_T)CCAPL3 | ((U16_T)CCAPH3 << 8);
		tCount += PCA_SW_TIMER_TRIG_COUNT;

		CCAPL3 = (U8_T)tCount;
		CCAPH3 = (U8_T)(tCount >> 8);
		EINT3 = 1;
	}
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void pca_SoftwareTimerOnModule4(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if PCA_SW_TIMER_ON_MODULE_4
static void pca_SoftwareTimerOnModule4(void)
{
	U16_T	tCount = 0;

	if (pcaIntrFlag & PCA_CCF4)
	{
		EINT3 = 0;
		pcaIntrFlag &= ~PCA_CCF4;

		tCount = (U16_T)CCAPL4 | ((U16_T)CCAPH4 << 8);
		tCount += PCA_SW_TIMER_TRIG_COUNT;

		CCAPL4 = (U8_T)tCount;
		CCAPH4 = (U8_T)(tCount >> 8);
		EINT3 = 1;
	}
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void pca_HighSpeedOutputOnModule0(void)
 * Purpose : 
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if PCA_HI_SPEED_OUT_ON_MODULE_0
static void pca_HighSpeedOutputOnModule0(void)
{
	U16_T	count;

	if (pcaIntrFlag & PCA_CCF0)
	{
		pcaIntrFlag &= ~PCA_CCF0;
		count = (U16_T)CCAPH0 << 8 | (U16_T)CCAPL0 ;
		count += PCA_HIGH_SPEED_OUT_TRIG_COUNT;
		CCAPL0 = (U8_T)count;
		CCAPH0 = (U8_T)(count >> 8);
	}
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void pca_HighSpeedOutputOnModule1(void)
 * Purpose : 
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if PCA_HI_SPEED_OUT_ON_MODULE_1
static void pca_HighSpeedOutputOnModule1(void)
{
	U16_T	count;

	if (pcaIntrFlag & PCA_CCF1)
	{
		pcaIntrFlag &= ~PCA_CCF1;
		count = (U16_T)CCAPH1 << 8 | (U16_T)CCAPL1 ;
		count += PCA_HIGH_SPEED_OUT_TRIG_COUNT;
		CCAPL1 = (U8_T)count;
		CCAPH1 = (U8_T)(count >> 8);
	}
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void pca_HighSpeedOutputOnModule2(void)
 * Purpose : 
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if PCA_HI_SPEED_OUT_ON_MODULE_2
static void pca_HighSpeedOutputOnModule2(void)
{
	U16_T	count;

	if (pcaIntrFlag & PCA_CCF2)
	{
		pcaIntrFlag &= ~PCA_CCF2;
		count = (U16_T)CCAPH2 << 8 | (U16_T)CCAPL2 ;
		count += PCA_HIGH_SPEED_OUT_TRIG_COUNT;
		CCAPL2 = (U8_T)count;
		CCAPH2 = (U8_T)(count >> 8);
	}
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void pca_HighSpeedOutputOnModule3(void)
 * Purpose : 
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if PCA_HI_SPEED_OUT_ON_MODULE_3
static void pca_HighSpeedOutputOnModule3(void)
{
	U16_T	count;

	if (pcaIntrFlag & PCA_CCF3)
	{
		pcaIntrFlag &= ~PCA_CCF3;
		count = (U16_T)CCAPH3 << 8 | (U16_T)CCAPL3 ;
		count += PCA_HIGH_SPEED_OUT_TRIG_COUNT;
		CCAPL3 = (U8_T)count;
		CCAPH3 = (U8_T)(count >> 8);
	}
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void pca_HighSpeedOutputOnModule4(void)
 * Purpose : 
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if PCA_HI_SPEED_OUT_ON_MODULE_4
static void pca_HighSpeedOutputOnModule4(void)
{
	U16_T	count;

	if (pcaIntrFlag & PCA_CCF4)
	{
		pcaIntrFlag &= ~PCA_CCF4;
		count = (U16_T)CCAPH4 << 8 | (U16_T)CCAPL4 ;
		count += PCA_HIGH_SPEED_OUT_TRIG_COUNT;
		CCAPL4 = (U8_T)count;
		CCAPH4 = (U8_T)(count >> 8);
	}
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void pca_8bitPwmOnModule0(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if PCA_8BIT_PWM_ON_MODULE_0
static void pca_8bitPwmOnModule0(void)
{
	if (pcaIntrFlag & PCA_CCF0)
	{
		pcaIntrFlag &= ~PCA_CCF0;

		pcaCexVal[PCA_MODULE0] = (U16_T)CCAPL0 | ((U16_T)CCAPH0 << 8);
	}
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void pca_8bitPwmOnModule1(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if PCA_8BIT_PWM_ON_MODULE_1
static void pca_8bitPwmOnModule1(void)
{
	if (pcaIntrFlag & PCA_CCF1)
	{
		pcaIntrFlag &= ~PCA_CCF1;

		pcaCexVal[PCA_MODULE1] = (U16_T)CCAPL1 | ((U16_T)CCAPH1 << 8);
	}
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void pca_8bitPwmOnModule2(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if PCA_8BIT_PWM_ON_MODULE_2
static void pca_8bitPwmOnModule2(void)
{
	if (pcaIntrFlag & PCA_CCF2)
	{
		pcaIntrFlag &= ~PCA_CCF2;

		pcaCexVal[PCA_MODULE2] = (U16_T)CCAPL2 | ((U16_T)CCAPH2 << 8);
	}
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void pca_8bitPwmOnModule3(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if PCA_8BIT_PWM_ON_MODULE_3
static void pca_8bitPwmOnModule3(void)
{
	if (pcaIntrFlag & PCA_CCF3)
	{
		pcaIntrFlag &= ~PCA_CCF3;

		pcaCexVal[PCA_MODULE3] = (U16_T)CCAPL3 | ((U16_T)CCAPH3 << 8);
	}
}
#endif

/*
 *--------------------------------------------------------------------------------
 * static void pca_8bitPwmOnModule4(void)
 * Purpose :
 * Params  :
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
#if PCA_8BIT_PWM_ON_MODULE_4
static void pca_8bitPwmOnModule4(void)
{
	if (pcaIntrFlag & PCA_CCF4)
	{
		pcaIntrFlag &= ~PCA_CCF4;

		pcaCexVal[PCA_MODULE4] = (U16_T)CCAPL4 | ((U16_T)CCAPH4 << 8);
	}
}
#endif


/* EXPORTED SUBPROGRAM BODIES */

/*
 *--------------------------------------------------------------------------------
 * void PCA_ValueInit(void)
 * Purpose : Initial all global values in PCA module.
 * Params  : none
 * Returns : none
 * Note    :
 *--------------------------------------------------------------------------------
 */
void PCA_ValueInit(void)
{
	U8_T	i;

	for (i=0 ; i<5 ; i++)
	{
		pcaCapVal[i] = 0;
		pcaCexVal[i] = 0;
	}
}

/*
 *--------------------------------------------------------------------------------
 * void PCA_ModeSetup(U8_T modeVal, U8_T ctrlVal)
 * Purpose : AX11000 PCA mode configuration and initial global values.
 * Params  : modeVal - CMOD register for counter overflow interrupt and clock frequency.
 *           ctrlVal - control register to start counter and clear interrupt flag.
 * Returns : none.
 * Note    :
 *--------------------------------------------------------------------------------
 */
void PCA_ModeSetup(U8_T modeVal, U8_T ctrlVal)
{
	CMOD = modeVal;
	CL = 0;
	CH = 0;
	CCON = ctrlVal;
	PCA_ValueInit();
}

/*
 *--------------------------------------------------------------------------------
 * void PCA_ModuleSetup(U8_T moduleNum, U8_T modeType,
 *      U8_T intrEnb, U16_T timerVal)
 * Purpose : Configure one module type, interrupt and compare/capture registers.
 * Params  : moduleNum - a module number (0 ~ 4)
 *           modeType - the type in the module
 *           intrEnb - interrupt enable if need
 *           timerVal - compare/capture register value if need
 * Returns :
 * Note    :
 *--------------------------------------------------------------------------------
 */
void PCA_ModuleSetup(U8_T moduleNum, U8_T modeType, U8_T intrEnb, U16_T timerVal)
{
	switch (moduleNum)
	{
		case PCA_MODULE0 :
			CCAPL0 = (U8_T)(timerVal & 0x00FF);
			CCAPH0 = (U8_T)((timerVal & 0xFF00) >> 8);
			CCAPM0 = modeType | intrEnb;
			break;
		case PCA_MODULE1 :
			CCAPL1 = (U8_T)(timerVal & 0x00FF);
			CCAPH1 = (U8_T)((timerVal & 0xFF00) >> 8);
			CCAPM1 = modeType | intrEnb;
			break;
		case PCA_MODULE2 :
			CCAPL2 = (U8_T)(timerVal & 0x00FF);
			CCAPH2 = (U8_T)((timerVal & 0xFF00) >> 8);
			CCAPM2 = modeType | intrEnb;
			break;
		case PCA_MODULE3 :
			CCAPL3 = (U8_T)(timerVal & 0x00FF);
			CCAPH3 = (U8_T)((timerVal & 0xFF00) >> 8);
			CCAPM3 = modeType | intrEnb;
			break;
		case PCA_MODULE4 :
			CCAPL4 = (U8_T)(timerVal & 0x00FF);
			CCAPH4 = (U8_T)((timerVal & 0xFF00) >> 8);
			CCAPM4 = modeType | intrEnb;
			break;
	}
}

/*
 *--------------------------------------------------------------------------------
 * void PCA_ModuleTimer(U8_T moduleNum, U16_T newTimerVal)
 * Purpose : Set the timeout value into a module which is configured in compare mode
 * Params  : moduleNum - module number (0 ~ 4)
 *           newTimerVal - the new value in the compare register.
 * Returns : none
 * Note    :
 *--------------------------------------------------------------------------------
 */
void PCA_ModuleTimer(U8_T moduleNum, U16_T newTimerVal)
{
	switch (moduleNum)
	{
		case PCA_MODULE0 :
			CCAPL0 = (U8_T)(newTimerVal & 0x00FF);
			CCAPH0 = (U8_T)((newTimerVal & 0xFF00) >> 8);
			break;
		case PCA_MODULE1 :
			CCAPL1 = (U8_T)(newTimerVal & 0x00FF);
			CCAPH1 = (U8_T)((newTimerVal & 0xFF00) >> 8);
			break;
		case PCA_MODULE2 :
			CCAPL2 = (U8_T)(newTimerVal & 0x00FF);
			CCAPH2 = (U8_T)((newTimerVal & 0xFF00) >> 8);
			break;
		case PCA_MODULE3 :
			CCAPL3 = (U8_T)(newTimerVal & 0x00FF);
			CCAPH3 = (U8_T)((newTimerVal & 0xFF00) >> 8);
			break;
		case PCA_MODULE4 :
			CCAPL4 = (U8_T)(newTimerVal & 0x00FF);
			CCAPH4 = (U8_T)((newTimerVal & 0xFF00) >> 8);
			break;
	}
}

/*
 *--------------------------------------------------------------------------------
 * U16_T PCA_GetModuleCaptureValue(U8_T moduleNum)
 * Purpose : get the captured values of the specified PCA module in the capture mode.
 * Params  : moduleNum - module number (0 ~ 4)
 * Returns : valCapture - the captured value of the specified PCA module.
 * Note    :
 *--------------------------------------------------------------------------------
 */
U16_T PCA_GetModuleCaptureValue(U8_T moduleNum)
{
	U16_T	valCapture = 0;

	switch (moduleNum)
	{
		case PCA_MODULE0 :
			valCapture = pcaCapVal[PCA_MODULE0];
			break;
		case PCA_MODULE1 :
			valCapture = pcaCapVal[PCA_MODULE1];
			break;
		case PCA_MODULE2 :
			valCapture = pcaCapVal[PCA_MODULE2];
			break;
		case PCA_MODULE3 :
			valCapture = pcaCapVal[PCA_MODULE3];
			break;
		case PCA_MODULE4 :
			valCapture = pcaCapVal[PCA_MODULE4];
			break;
	}
	return valCapture;
}


/* End of pca.c */

#endif
