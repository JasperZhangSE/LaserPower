/*
    Sys.h

    Head File for App Sys Module
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 22Nov23, Karl Created
    01b, 04Dec23, Karl Added SysGetFsm
    01c, 06Dec23, Karl Added sys_fsm
    01d, 22Dec23, Karl Added current limitation in LaserOn function
    01e, 27Dec23, Karl Fixed s_ucAPwrCtrl1, s_ucAPwrCtrl2 and s_ucAPwrCtrl3 bug
    01f, 27Dec23, Karl Added s_bManualCtrl
    01g, 15Jan24, Karl PD check in prvChkAPwr
    01h, 17Jan24, Karl Modified LaserOn to limit current less than th_WorkCur
    01i, 20Jan24, Karl Added th_CompRate for current output
    01j, 20Jan24, Karl Added th_SysStatus
    01k, 22Jan24, Karl Added show_sys_status
    01l, 23Jan24, Karl Added th_SysDebug
    01m, 24Jan24, Karl Added th_VerChk
    01n, 26Jan24, Karl Added th_SwInfo
    01o, 29Jan24, Karl Added dynamic current adjustment
    01p, 30Jan24, Karl Optimized prvChkMPwr function
*/

#ifndef __SYS_H__
#define __SYS_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

#include "stm32f1xx_hal.h"
#include "Include/Include.h"

/* Defines */
#define SYS_TASK_DELAY  1   /* ms */
#define LED_TASK_DELAY  100 /* ms */
#define FSM_READY_WAIT  (2500 / SYS_TASK_DELAY)
#define th_Fsm          g_pxState->xState
#define th_RunTime      g_pxState->ulRunTime
#define th_SysStatus    g_xSysStatus.bit
#define th_SysStatusAll g_xSysStatus.all
#define th_VerChk       g_ucVerChk
#define th_SwInfo       g_usSwInfo

/* Types */
typedef enum {
    FSM_START,
    
    FSM_INIT_MPWR,
    
    FSM_READY,
    
    FSM_IDLE,
    
    FSM_LASERs_INIT,
    FSM_LASERs_RUN,
    FSM_LASERs_DONE,
    
    FSM_LASERm_INIT,
    FSM_LASERm_RUN,
    FSM_LASERm_DONE,
    
    FSM_INFRARED_INIT,
    FSM_INFRARED_RUN,
    FSM_INFRARED_DONE,
    
    FSM_ERROR
}Fsm_t;

typedef union {
    uint16_t all;
    struct {
        uint16_t STATUS_MPWR:1;         /* 主电源 */
        uint16_t ALARM_EMCY_STOP:1;     /* 急停报警 */
        uint16_t ALARM_WATER_CHILLER:1; /* 水冷机报警 */
        uint16_t ALARM_WATER_PRESS:1;   /* 水压报警 */
        uint16_t ALARM_QBH:1;           /* QBH报警 */
        uint16_t ALARM_OT:1;            /* 过温报警 */
        uint16_t WARN_OT:1;             /* 过温预警 */
        uint16_t ALARM_PD:1;            /* PD漏光报警 */
        uint16_t WARN_PD:1;             /* PD漏光预警 */
        uint16_t ALARM_WET:1;           /* 超湿报警 */
        uint16_t WORK_INFRARED:1;       /* 指示光工作 */
        uint16_t WORK_LASER:1;          /* 激光工作 */
        uint16_t RUN:1;                 /* 程序执行 */
    }bit;
}SysStatus_t;

typedef struct {
    uint32_t ulRunTime;         /* System runtime after reset */
    
    uint32_t ulCounter;         /* General purpose counter */
    
    /* Main FSM */
    Fsm_t xState;               /* Current state of the state machine */
    Fsm_t xLastState;           /* Previous state of the state machine */
    Fsm_t xNextState;           /* Next state of the state machine */
}State_t;

/* Forward declarations */
extern State_t *g_pxState;
extern SysStatus_t g_xSysStatus;
extern uint8_t g_ucVerChk;
extern uint16_t g_usSwInfo;

/* Functions */
Status_t AppSysInit(void);
Status_t AppSysTerm(void);

Fsm_t    SysGetFsm(void);

Status_t LaserOn(uint32_t ulCurrent, uint32_t ulSelect);
Status_t LaserOff(uint32_t ulSelect);

Status_t InfraredOn(void);
Status_t InfraredOff(void);

Status_t EnableManualCtrl(uint8_t ucEnable);

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif /* __SYS_H__ */
