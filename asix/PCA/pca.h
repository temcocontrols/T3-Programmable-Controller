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
 * Module Name : pca.h
 * Purpose     : 
 * Author      : Robin Lee
 * Date        : 2006-06-23
 * Notes       :
 * $Log: pca.h,v $
 * Revision 1.0  2009-01-05 14:03:53+08  wong
 * Initial revision
 *
 *================================================================================
 */
#ifndef PCA_H
#define PCA_H

/* INCLUDE FILE DECLARATIONS */
#include	"pca_cfg.h"


/* NAMING CONSTANT DECLARATIONS */
#define		PCA_MODULE0			0
#define		PCA_MODULE1			1
#define		PCA_MODULE2			2
#define		PCA_MODULE3			3
#define		PCA_MODULE4			4
#define		PCA_CLK_1			0
#define		PCA_CLK_1_33		BIT1
#define		PCA_CLK_3_03		BIT2
#define		PCA_CLK_4			BIT1+BIT2
#define		PCA_CCF_ENB			ECCFs
#define		PCA_CAP_POS			CAPPs
#define		PCA_CAP_NEG			CAPNs
#define		PCA_CAP_POS_NEG		CAPPs|CAPNs
#define		PCA_SW_TIMER		ECOMs|MATs
#define		PCA_HI_SPEED_OUT	ECOMs|MATs|TOGs
#define		PCA_8BIT_PWM		ECOMs|PWMs


/* MACRO DECLARATIONS */


/* TYPE DECLARATIONS */


/* GLOBAL VARIABLES */


/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void	PCA_ValueInit(void);
void	PCA_ModeSetup(U8_T modeVal, U8_T ctrlVal);
void	PCA_ModuleSetup(U8_T moduleNum, U8_T modeType, U8_T intrEnb, U16_T timerVal);
void	PCA_ModuleTimer(U8_T moduleNum, U16_T newTimerVal);
U16_T	PCA_GetModuleCaptureValue(U8_T moduleNum);


#endif /* End of PCA_H */
