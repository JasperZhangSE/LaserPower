/*
    Data.h

    Head File for App Data Module
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 23Nov23, Karl Created
    01b, 30Nov23, Karl Optimized storage implementation
    01c, 08Jan24, Karl Added ulAdVolPara in Data_t
    01d, 08Jan24, Karl Added cfg_set_adc_vol_para
    01e, 09Jan24, Karl Added ucPwrType in Data_t
    01f, 09Jan24, Karl Added cfg_set_pwr_type
    01g, 17Jan24, Karl Added cfg_set_mod_en
    01h, 17Jan24, Karl Added th_MaxCurAd
    01i, 17Jan24, Karl Added cfg_set_temp_chan_num
    01j, 23Jan24, Karl Added th_SysDebug
    01k, 24Jan24, Karl Added trial version control
    01l, 21Feb24, Karl Added net parameters
*/

#ifndef __DATA_H__
#define __DATA_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

/* Includes */
#include <stm32f1xx_hal.h>
#include "Include/Type.h"
#include "Include/Define.h"

/* Defines */
#define th_Data             g_xData
#define th_OtWarnTh         g_xData.usOtWarnTh
#define th_OtCutTh          g_xData.usOtCutTh
#define th_MaxCur           g_xData.ulMaxCur
#define th_WorkCur          g_xData.ulWorkCur
#define th_CompRate         g_xData.lCompRate
#define th_PdWarnL1         g_xData.usPdWarnL1
#define th_PdWarnL2         g_xData.usPdWarnL2
#define th_AdVolPara        g_xData.ulAdVolPara
#define th_PwrType          g_xData.ucPwrType
#define th_TempNum          g_xData.ucTempNum
#define th_SysDebug         g_xData.ucSysDebug
#define th_Trial            g_xData.ucTrial
#define th_TrialDays        g_xData.usTrialDays
#define th_TrialStTime      g_xData.ulTrialStTime
#define th_ModEn            g_xData.xModEn.bit
#define th_ModEnAll         g_xData.xModEn.all
#define th_Dhcp             g_xData.bDhcp
#define th_DhcpTimeout      g_xData.ulDhcpTimeout
#define th_LocalIp          g_xData.ulLocalIp
#define th_LocalNetMask     g_xData.ulLocalNetMask
#define th_LocalGwAddr      g_xData.ulLocalGwAddr
#define th_MaxCurAd         g_usMaxCurAd
#define th_AimLightDuty     g_xData.usAimLightDuty
#define th_PwrId            g_xData.ucPwrId
#define th_TrialEn          g_xData.usTrialEn
#define th_OrderNum         g_xData.ulOrderNum
#define th_CtrlMode         g_xData.CtrlMode
#define th_APwmDuty         g_xData.APwmDuty
#define th_CVS              g_xData.CVS
#define th_CCS              g_xData.CCS
#define th_AimMutex         g_xData.AimMutex

#define FLASH_DATA_HEAD     (0xA5)
#define FLASH_SAVE_PAGE1    (1024*64)
#define FLASH_SAVE_PAGE2    (1024*128)

#ifndef APP_DATA_INIT
#define APP_DATA_INIT       { \
                                .ucHead = FLASH_DATA_HEAD, \
                                .usOtWarnTh = 26,\
                                .usOtCutTh = 17,\
                                .ulMaxCur = 450,\
                                .ulWorkCur = 400,\
                                .lCompRate = 0/*0.001*/,\
                                .usPdWarnL1 = 1490/*1200mV*/,\
                                .usPdWarnL2 = 993/*800mV*/,\
                                .ulAdVolPara = 5800/*87/15=5.8*/,\
                                .ucPwrType = 1,\
                                .ucTempNum = 18,\
                                .usTrialEn = 1, \
                                .ucTrial = 0, \
                                .usTrialDays = 30, \
                                .ulTrialStTime = 0, \
                                .ucSysDebug = 0,\
                                .xModEn.all = 0xFFFF,\
                                .bDhcp = FALSE, \
                                .ulDhcpTimeout = 5, \
                                .ulLocalIp = MAKE_IPADDR(192, 168, 1, 10), \
                                .ulLocalNetMask = MAKE_IPADDR(255, 255, 255, 0), \
                                .ulLocalGwAddr = MAKE_IPADDR(192, 168, 1, 1), \
                                .usAimLightDuty = 60, \
                                .ucPwrId = "12345678901234", \
                                .ulOrderNum = 0001, \
                                .CtrlMode = 3 /* 1: Internal dac; 2: Pwm; 3: External AD */, \
                                .APwmDuty = 50, \
                                .CVS = 1, \
                                .CCS = 1, \
                                .AimMutex = 1, \
                                .ucCrc  = 0 \
                            }
#endif /* APP_DATA_INIT */

/* Types */
#pragma pack(push)
#pragma pack(1)
typedef union {
    uint16_t all;
    struct {
        uint16_t MANUAL:1;          /* 手动 */
        uint16_t QBH:1;             /* QBH检测 */
        uint16_t PD:1;              /* PD检测 */
        uint16_t TEMP1:1;           /* 温度组1检测 */
        uint16_t TEMP2:1;           /* 温度组2检测 */
        uint16_t CHAN1_CUR:1;       /* 通道1电流检测 */
        uint16_t CHAN2_CUR:1;       /* 通道2电流检测 */
        uint16_t CHAN3_CUR:1;       /* 通道3电流检测 */
        uint16_t WATER_PRESS:1;     /* 水压检测 */
        uint16_t WATER_CHILLER:1;   /* 水冷机检测 */
        uint16_t CHAN_CTRL:1;       /* 通道开关 */
    }bit;
}ModEn_t;

typedef struct {
    uint8_t  ucHead;         /* Head mark */
    uint16_t usOtWarnTh;     /* Over temperature warn threshold (raw AD value) */
    uint16_t usOtCutTh;      /* Over temperature cut threshold (raw AD value) */
    uint32_t ulMaxCur;       /* Maximum current (0.1A) */
    uint32_t ulWorkCur;      /* Work current (0.1A) */
    int32_t  lCompRate;      /* Current compensation rate (0.01) */
    uint16_t usPdWarnL1;     /* PD warn threshold level 1 */
    uint16_t usPdWarnL2;     /* PD warn threshold level 2 */
    uint32_t ulAdVolPara;    /* AD voltage parameter */
    uint8_t  ucPwrType;      /* Power type(1-9.5KW, 2-18KW) */
    uint8_t  ucTempNum;      /* Temperature channel number */
    uint8_t  ucSysDebug;     /* System debug enable mark */
    uint8_t  usTrialEn;
    uint8_t  ucTrial;        /* Trial version mark */
    uint16_t usTrialDays;    /* Trial days */
    uint32_t ulTrialStTime;  /* Trial start time stamp */
    ModEn_t  xModEn;         /* Module enable */
    Bool_t   bDhcp;          /* DHCP mark */
    uint32_t ulDhcpTimeout;  /* DHCP timeout */
    uint32_t ulLocalIp;      /* Local IP address */
    uint32_t ulLocalNetMask; /* Local net mask */
    uint32_t ulLocalGwAddr;  /* Local gateway address */
    uint16_t usAimLightDuty; /* Pwm duty */
    char  ucPwrId[20];       /* Pwr ID */ /* Use uint8_t will warnning */
    uint32_t ulOrderNum;     /* Order Number  */
    uint16_t CtrlMode;       /* Laser control mode */
    uint16_t APwmDuty;       /* Constant flow source pwm duty */
    
    /* 为防止报文出错 没有放在ModEn，后续如果升级可以放到ModEn */
    uint16_t CVS;            /* 恒压源检测 */
    uint16_t CCS;            /* 恒流源检测 */
    
    uint16_t AimMutex;       /* 红光互斥 */
    
    uint8_t  ucCrc;          /* Check code */

}Data_t;
#pragma pack(pop)

/* Forward declarations */
extern Data_t g_xData;
extern uint16_t g_usMaxCurAd;

/* Functions */
Status_t AppDataInit(void);
Status_t AppDataTerm(void);

Status_t DataLoad(OUT Data_t *pxData);
Status_t DataSave(IN Data_t *pxData);
Status_t DataSaveDirect(void);

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif /* __DATA_H__ */
