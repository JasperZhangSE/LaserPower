/*
    Sys.c

    Implementation File for App Sys Module
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

/* Includes */
#include "Include.h"

#undef SYS_DEBUG
#define SYS_DEBUG       1 /* XXX: SYS_DEBUG */

#define PWM_DAC_AD_MODE 1

/* Debug config */
#if SYS_DEBUG
#undef TRACE
#define TRACE(...)                                                                                                     \
    if (th_SysDebug)                                                                                                   \
    DebugPrintf(__VA_ARGS__)
#else
#undef TRACE
#define TRACE(...)
#endif /* SYS_DEBUG */
#if SYS_ASSERT
#undef ASSERT
#define ASSERT(a)                                                                                                      \
    while (!(a)) {                                                                                                     \
        DebugPrintf("ASSERT failed: %s %d\n", __FILE__, __LINE__);                                                     \
    }
#else
#undef ASSERT
#define ASSERT(...)
#endif /* SYS_ASSERT */

/* Local defines */
#define SYS_SAVELASTSTATES()  pxState->xLastState = pxState->xState;
#define SYS_TICK_GET()        osKernelSysTick()
#define LED_ALARM_R           LED1
#define LED_ALARM_G           LED2
#define LED_ACTIVE_R          LED3
#define LED_ACTIVE_G          LED4
#define LED_POWER_R           LED5
#define LED_POWER_G           LED6
#define LED_ON                1
#define LED_OFF               0
#define MPWR_DC_OK            1
#define MPWR_AC_OK            1
#define MPWR_ON               1
#define MPWR_OFF              0
#define APWR_OK               1
#define APWR_ON               1
#define APWR_OFF              0
#define LED_CTRL_ON           1
#define LED_CTRL_OFF          0
#define QBH_ON_ON             0
#define QBH_ON_OFF            1
#define WATER_PRESS_ON        0
#define WATER_PRESS_OFF       1
#define WATER_CHILLER_ON      0
#define WATER_CHILLER_OFF     1
#define BEEP_DELAY            70

/* Forward declaration */
static void prvSysTask        (void *pvPara);
static void prvDaemonTask     (void *pvPara);
static void prvManualTask     (void *pvPara);

/* State machine */
static void prvProc           (void);
static void prvFsmStart       (State_t *pxState);
static void prvFsmInitMpwr    (State_t *pxState);
static void prvFsmReady       (State_t *pxState);
static void prvFsmIdle        (State_t *pxState);
static void prvFsmLaserSInit  (State_t *pxState);
static void prvFsmLaserSRun   (State_t *pxState);
static void prvFsmLaserSDone  (State_t *pxState);
static void prvFsmLaserMInit  (State_t *pxState);
static void prvFsmLaserMRun   (State_t *pxState);
static void prvFsmLaserMDone  (State_t *pxState);
static void prvFsmInfraredInit(State_t *pxState);
static void prvFsmInfraredRun (State_t *pxState);
static void prvFsmInfraredDone(State_t *pxState);
static void prvFsmError       (State_t *pxState);

/* Help functions */
static bool prvInitChkMPwr    (void);
static bool prvChkPwr         (void);
static bool prvChkMPwr        (void);
static bool prvChkAPwr        (void);
static void prvEnterFsm       (void);
static void prvProcManualCtrl (void);
static void prvProcPanelLed   (void);

/* Global variables */
State_t    *g_pxState         = NULL;
uint8_t     g_ucVerChk        = 0;
uint16_t    g_usSwInfo        = 0x0000;
SysStatus_t g_xSysStatus;

/* Local variables */
static State_t  s_xState;
static bool     s_bProc         = true;
static bool     s_bManualCtrl   = true;
static uint32_t s_ulCurrent     = 0; /* 0.1A */
static uint8_t  s_ucAPwrCtrl1   = APWR_OFF;
static uint8_t  s_ucAPwrCtrl2   = APWR_OFF;
static uint8_t  s_ucAPwrCtrl3   = APWR_OFF;
static uint8_t  aim_mutex_onoff = 1;
static uint8_t  laser_on_pd_err = 1;
static uint8_t  manual_ctrl_err = 1;

/* Functions */
Status_t AppSysInit(void)
{
    /* 9.5KW */
    if (th_PwrType == 1) {
        DrvPwr1Enable();
    }
    /* 18KW */
    if (th_PwrType == 2) {
        DrvPwr2Enable();
    }

    DrvPwrInit();

    SetAinLightCur(th_AimLightDuty);

    memset(&s_xState, 0, sizeof(s_xState));
    g_pxState        = &s_xState;
    s_xState.xState  = FSM_START;
    g_xSysStatus.all = 0;
    xTaskCreate(prvSysTask, "tSys", 256, NULL, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(prvDaemonTask, "tDaemon", 256, NULL, tskIDLE_PRIORITY + 0, NULL);
    xTaskCreate(prvManualTask, "tManual", 256, NULL, tskIDLE_PRIORITY + 2, NULL);

    /* Control mode selection  */
    /* Read EX_CTRL_EN  Level */
//    th_CtrlMode = GpioGetInput(EX_CTRL_EN) == 0 ? 3 : th_CtrlMode;

    switch (th_CtrlMode) {
    /* DAC Mode */
    case 1:
        GpioSetOutput(MOD_EN, 0);
        GpioSetOutput(EX_AD_EN, 0);
        SetADuty(100);
        ToggleCcsStatus(1);
        break;

    /* PWM Mode */
    case 2:
        GpioSetOutput(MOD_EN, 0);
        GpioSetOutput(EX_AD_EN, 0);
        DacSet(APWR1_CTRL, CUR_TO_DAC(450 / 10.f));
        SetADuty(50);
        ToggleCcsStatus(0);
        break;

    /* AD Mode */
    case 3:
        GpioSetOutput(MOD_EN, 1);
        GpioSetOutput(EX_AD_EN, 1);
        ToggleCcsStatus(0);
        break;
    }

    return STATUS_OK;
}

Status_t AppSysTerm(void)
{
    /* Do nothing */
    return STATUS_OK;
}

Fsm_t SysGetFsm(void)
{
    return s_xState.xState;
}

Status_t LaserOn(uint32_t ulCurrent, uint32_t ulSelect)
{
    State_t *pxState = &s_xState;
    switch (th_CtrlMode) {
    case 1:
    case 2:
        if ((pxState->xState == FSM_IDLE) && prvChkMPwr() && ulSelect && (ulCurrent <= th_WorkCur)) {
            if (th_ModEn.CHAN_CTRL) {
                (ulSelect & 0x01) ? (s_ucAPwrCtrl1 = APWR_ON) : 0;
                (ulSelect & 0x02) ? (s_ucAPwrCtrl2 = APWR_ON) : 0;
                (ulSelect & 0x04) ? (s_ucAPwrCtrl3 = APWR_ON) : 0;
            }
            else {
                s_ucAPwrCtrl1 = APWR_ON;
                s_ucAPwrCtrl2 = APWR_ON;
                s_ucAPwrCtrl3 = APWR_ON;
            }
            s_ulCurrent        = ulCurrent * (1000 + th_CompRate) / 1000;
            pxState->xState    = FSM_LASERs_INIT;
            pxState->ulCounter = 0;
            TRACE("[%6d] Idle         -> LaserSInit\n", SYS_TICK_GET());
            TRACE("[%6d]     Cur %.1f\n", SYS_TICK_GET(), ulCurrent / 10.);
            TRACE("[%6d]     Sel %d\n", SYS_TICK_GET(), ulSelect);
            return STATUS_OK;
        }
        else if ((pxState->xState == FSM_LASERs_INIT) && prvChkMPwr() && ulSelect && (ulCurrent <= th_WorkCur)) {
            s_ulCurrent        = ulCurrent * (1000 + th_CompRate) / 1000;
            pxState->xState    = FSM_LASERs_INIT;
            pxState->ulCounter = 0;
            TRACE("[%6d] LaserSInit   -> LaserSInit\n", SYS_TICK_GET());
            TRACE("[%6d]     Cur %.1f\n", SYS_TICK_GET(), ulCurrent / 10.);
            TRACE("[%6d]     Sel %d\n", SYS_TICK_GET(), ulSelect);
            return STATUS_OK;
        }
        else if ((pxState->xState == FSM_LASERs_RUN) && prvChkMPwr() && ulSelect && (ulCurrent <= th_WorkCur)) {
            s_ulCurrent        = ulCurrent * (1000 + th_CompRate) / 1000;
            pxState->xState    = FSM_LASERs_INIT;
            pxState->ulCounter = 0;
            TRACE("[%6d] LaserSRun    -> LaserSInit\n", SYS_TICK_GET());
            TRACE("[%6d]     Cur %.1f\n", SYS_TICK_GET(), ulCurrent / 10.);
            TRACE("[%6d]     Sel %d\n", SYS_TICK_GET(), ulSelect);
            return STATUS_OK;
        }
        else {
            return STATUS_ERR;
        }

        break;
    case 3:
//        TRACE("ulCurrent = %d\n", ulCurrent);
        if ((pxState->xState == FSM_IDLE) && prvChkMPwr() && (ulCurrent <= th_WorkCur)) {
            pxState->xState    = FSM_LASERm_INIT;
            pxState->ulCounter = 0;
            TRACE("[%6d] Idle         -> LaserMInit\n", SYS_TICK_GET());
            TRACE("[%6d]     Cur %.1f\n", SYS_TICK_GET(), ulCurrent / 10.);
        }
        else {
            return STATUS_ERR;
        }
        break;
    }

    return STATUS_OK;
}

Status_t LaserOff(uint32_t ulSelect)
{
    State_t *pxState = &s_xState;
    switch (th_CtrlMode) {
    /* DAC Mode */
    case 1:
        if ((pxState->xState == FSM_LASERs_RUN) && ulSelect) {
            if (th_ModEn.CHAN_CTRL) {
                (ulSelect & 0x01) ? (s_ucAPwrCtrl1 = APWR_OFF) : 0;
                (ulSelect & 0x02) ? (s_ucAPwrCtrl2 = APWR_OFF) : 0;
                (ulSelect & 0x04) ? (s_ucAPwrCtrl3 = APWR_OFF) : 0;
            }
            else {
                s_ucAPwrCtrl1 = APWR_OFF;
                s_ucAPwrCtrl2 = APWR_OFF;
                s_ucAPwrCtrl3 = APWR_OFF;
            }

            if ((s_ucAPwrCtrl1 == APWR_OFF) && (s_ucAPwrCtrl2 == APWR_OFF) && (s_ucAPwrCtrl3 == APWR_OFF)) {
                pxState->xState    = FSM_LASERs_DONE;
                pxState->ulCounter = 0;
                TRACE("[%6d] LaserSRun    -> LaserSDone\n", SYS_TICK_GET());
            }
            
            GpioSetOutput(APWR1_EN, s_ucAPwrCtrl1);
            GpioSetOutput(APWR2_EN, s_ucAPwrCtrl2);
            GpioSetOutput(APWR3_EN, s_ucAPwrCtrl3);

            if (s_ucAPwrCtrl1 == APWR_OFF) {
                DacSet(APWR1_CTRL, CUR_TO_DAC(0));
            }
            if (s_ucAPwrCtrl2 == APWR_OFF) {
                DacSet(APWR2_CTRL, CUR_TO_DAC(0));
            }
            if (s_ucAPwrCtrl3 == APWR_OFF) {
                DacSet(APWR3_CTRL, CUR_TO_DAC(0));
            }

            TRACE("[%6d]     Set APWR1_EN %d, APWR1_CTRL %.1f A (%4d mV)\n", SYS_TICK_GET(), s_ucAPwrCtrl1, DAC_TO_CUR(DacGet(APWR1_CTRL)), DAC_TO_MVOL(DacGet(APWR1_CTRL)));
            TRACE("[%6d]     Set APWR2_EN %d, APWR2_CTRL %.1f A (%4d mV)\n", SYS_TICK_GET(), s_ucAPwrCtrl2, DAC_TO_CUR(DacGet(APWR2_CTRL)), DAC_TO_MVOL(DacGet(APWR2_CTRL)));
            TRACE("[%6d]     Set APWR3_EN %d, APWR3_CTRL %.1f A (%4d mV)\n", SYS_TICK_GET(), s_ucAPwrCtrl3, DAC_TO_CUR(DacGet(APWR3_CTRL)), DAC_TO_MVOL(DacGet(APWR3_CTRL)));

            return STATUS_OK;
        }
        else {
            return STATUS_ERR;
        }
        break;

    /* PWM Mode */
    case 2:
        if ((pxState->xState == FSM_LASERs_RUN) && ulSelect) {
            if (th_ModEn.CHAN_CTRL) {
                (ulSelect & 0x01) ? (s_ucAPwrCtrl1 = APWR_OFF) : 0;
                (ulSelect & 0x02) ? (s_ucAPwrCtrl2 = APWR_OFF) : 0;
                (ulSelect & 0x04) ? (s_ucAPwrCtrl3 = APWR_OFF) : 0;
            }
            else {
                s_ucAPwrCtrl1 = APWR_OFF;
                s_ucAPwrCtrl2 = APWR_OFF;
                s_ucAPwrCtrl3 = APWR_OFF;
            }
            
            if ((s_ucAPwrCtrl1 == APWR_OFF) && (s_ucAPwrCtrl2 == APWR_OFF) && (s_ucAPwrCtrl3 == APWR_OFF)) {
                pxState->xState    = FSM_LASERs_DONE;
                pxState->ulCounter = 0;
                TRACE("[%6d] LaserSRun    -> LaserSDone\n", SYS_TICK_GET());
            }
            
            GpioSetOutput(APWR1_EN, s_ucAPwrCtrl1);
            GpioSetOutput(APWR2_EN, s_ucAPwrCtrl2);
            GpioSetOutput(APWR3_EN, s_ucAPwrCtrl3);
            
            if (s_ucAPwrCtrl1 == APWR_OFF) {
                SetAFreq(CUR_TO_FREQ((0)));
                SetADuty(0);
//                TRACE("CUR_TO_FREQ : %.1f\n", CUR_TO_FREQ((0)));
//                TRACE("TIM4->PSC   : %d\n",  TIM4->PSC);
            }
            if (s_ucAPwrCtrl2 == APWR_OFF) {
                SetAFreq(CUR_TO_FREQ((0)));
                SetADuty(0);
            }
            if (s_ucAPwrCtrl3 == APWR_OFF) {
                SetAFreq(CUR_TO_FREQ((0)));
                SetADuty(0);
            }
            
            ToggleCcsStatus(0);
            
            TRACE("[%6d]     Set APWR1_EN %d, APWR1_CTRL %.1f A (%d Hz), Duty( %3d %% )\n", SYS_TICK_GET(), s_ucAPwrCtrl1, PSC_TO_CUR(TIM4->PSC), (int)PSC_TO_FREQ(TIM4->PSC), (uint32_t)(TIM4->CCR3 * 100 / (TIM4->ARR + 1)));
            TRACE("[%6d]     Set APWR2_EN %d, APWR2_CTRL %.1f A (%d Hz), Duty( %3d %% )\n", SYS_TICK_GET(), s_ucAPwrCtrl2, PSC_TO_CUR(TIM4->PSC), (int)PSC_TO_FREQ(TIM4->PSC), (uint32_t)(TIM4->CCR3 * 100 / (TIM4->ARR + 1)));
            TRACE("[%6d]     Set APWR3_EN %d, APWR3_CTRL %.1f A (%d Hz), Duty( %3d %% )\n", SYS_TICK_GET(), s_ucAPwrCtrl3, PSC_TO_CUR(TIM4->PSC), (int)PSC_TO_FREQ(TIM4->PSC), (uint32_t)(TIM4->CCR3 * 100 / (TIM4->ARR + 1)));

            return STATUS_OK;
        }
        else {
            return STATUS_ERR;
        }
        break;

    /* AD Mode */
    case 3:
        pxState->xState    = FSM_LASERm_DONE;
        pxState->ulCounter = 0;
        TRACE("[%6d] LaserMRun    -> LaserMDone\n", SYS_TICK_GET());
        break;
    }
    return STATUS_OK;
}

Status_t InfraredOn(void)
{
    State_t *pxState = &s_xState;
    if (pxState->xState == FSM_IDLE && th_AimMutex == 1) {
            pxState->xState    = FSM_INFRARED_INIT;
            GpioSetOutput(MOD_EN, 0);
            pxState->ulCounter = 0;
            TRACE("[%6d] Idle         -> InfraredInit\n", SYS_TICK_GET());
            return STATUS_OK;
        }
        else if (th_AimMutex == 0) {
            if (aim_mutex_onoff == 1) {
                ToggleAimLight(1);
                TRACE("[%6d] InfraredOff  -> InfraredOn (No Fsm)\n", SYS_TICK_GET());
                aim_mutex_onoff = 0;
            }
            return STATUS_OK;
        }
        else {
            return STATUS_ERR;
        }

}

Status_t InfraredOff(void)
{
    State_t *pxState = &s_xState;

    if ((pxState->xState == FSM_INFRARED_RUN) && th_AimMutex == 1) {
        pxState->xState    = FSM_INFRARED_DONE;
        GpioSetOutput(MOD_EN, 1);
        pxState->ulCounter = 0;
        TRACE("[%6d] InfraredRun  -> InfraredDone\n", SYS_TICK_GET());

        return STATUS_OK;
    }
    else if (th_AimMutex == 0) {
        if (aim_mutex_onoff == 0) {
            ToggleAimLight(0);
            TRACE("[%6d] InfraredOn -> InfraredOff (No Fsm)\n", SYS_TICK_GET());
            aim_mutex_onoff = 1;
        }
        return STATUS_OK;
    }
    else {
        return STATUS_ERR;
    }
}

Status_t ManualInfraredCtrl(void)
{
    uint8_t AimEn = GpioGetInput(AIM_EN);
    if (AimEn == 0){
        InfraredOn();
    }
    else {
        InfraredOff();
    }
    return STATUS_OK;
}

#if 0
Status_t ManualLaserCtrl(void)
{
    State_t *pxState = &s_xState;
    
    /* 通道电流检测 */
    uint16_t usApwr1Cur = AdcGet(APWR1_CUR);
    uint16_t usApwr2Cur = AdcGet(APWR2_CUR);
    uint16_t usApwr3Cur = AdcGet(APWR3_CUR);
    uint16_t ExAdVolCur = DAC_TO_CUR(AdcGet(ADC_CHAN_7));
    
    if ((pxState->xState == FSM_IDLE) && ((usApwr1Cur > 410 && usApwr1Cur != 4095)|| (usApwr2Cur > 410 && usApwr2Cur != 4095) || (usApwr3Cur > 410 && usApwr3Cur != 4095))) {
        TRACE("usApwr1Cur = %d\n", usApwr1Cur);
        TRACE("[%6d] Manual ctrl laser on\n", SYS_TICK_GET());
        LaserOn(ExAdVolCur, 0x07);
    }
    else if ((pxState->xState == FSM_LASERm_RUN) && ((usApwr1Cur < 410 && usApwr2Cur < 410 && usApwr3Cur < 410) || (usApwr1Cur == 4095 && usApwr2Cur == 4095 && usApwr3Cur == 4095))) {
        TRACE("usApwr1Cur = %d\n", usApwr1Cur);
        TRACE("[%6d] Manual ctrl laser off\n", SYS_TICK_GET());
        LaserOff(0x07);
    }
    return STATUS_OK;
}
#endif

Status_t ManualLaserCtrl(void)
{
    State_t *pxState = &s_xState;
    
    /* 通道检测 */
    uint16_t ExModEn = GpioGetInput(EX_MOD_EN);
    uint16_t ExAdVolCur = DAC_TO_CUR(AdcGet(ADC_CHAN_7));
    
    if ((pxState->xState == FSM_IDLE) && (ExModEn == 0)) {
        TRACE("[%6d] Manual ctrl laser on\n", SYS_TICK_GET());
        if ( LaserOn(ExAdVolCur, 0x07) == STATUS_ERR) {
            TRACE("Manual ctrl laser on to error.\n");
            TRACE("MPWR is not ready. Please make sure DC_OK\n");
            manual_ctrl_err = 0;
            pxState->xState = FSM_ERROR;
        }
    }
    else if ((pxState->xState == FSM_LASERm_RUN) && (ExModEn == 1)) {
        TRACE("[%6d] Manual ctrl laser off\n", SYS_TICK_GET());
        LaserOff(0x07);
    }
    return STATUS_OK;
}

Status_t EnableManualCtrl(uint8_t ucEnable)
{
    if (!th_ModEn.MANUAL) {
        return STATUS_ERR;
    }
    (ucEnable == 0xA5) ? (s_bManualCtrl = true) : (s_bManualCtrl = false);
    return STATUS_OK;
}

static void prvSysTask(void *pvPara)
{
    /* Start with beep on 2s */
    GpioSetOutput(BEEP_SW, 1);
    osDelay(2000);
    GpioSetOutput(BEEP_SW, 0);
    
    WdogInit();

    /* Check if software can be used! */
    if (0 == th_Trial) {
        th_VerChk = 1;
        th_SwInfo = 0x8000;
    }
    else {
        Time_t   xTm           = RtcReadTime(RTC_TYPE_DS1338);
        uint32_t ulCurrentTime = mktime(&xTm);
        uint32_t ulExpiredTime = th_TrialStTime + th_TrialDays * 86400;
        if (ulCurrentTime < ulExpiredTime) {
            th_VerChk = 1;
            th_SwInfo = (ulExpiredTime - ulCurrentTime) / 86400 + (((ulExpiredTime - ulCurrentTime) % 86400) ? 1 : 0);
        }
        else {
            th_VerChk = 0;
            th_SwInfo = 0;
        }
    }

    while (1) {
        if (s_bProc) {
            prvProc();
        }
        WdogFeed();
        osDelay(SYS_TASK_DELAY);
    }
}

static void prvManualTask(void *pvPara)
{
    while (1) {
        if (th_CtrlMode == 3) {
            ManualLaserCtrl();
            ManualInfraredCtrl();
        }
        osDelay(MANUAL_TASK_DELAY);
    }
    
}

static void prvDaemonTask(void *pvPara)
{
    while (1) {
        prvProcManualCtrl();
        prvProcPanelLed();
        osDelay(LED_TASK_DELAY);
    }
}

static void prvProc(void)
{
    State_t *pxState = &s_xState;

    /* Process state machine */
    switch (pxState->xState) {
    case FSM_START:
        prvFsmStart(pxState);
        break;

    case FSM_INIT_MPWR:
        prvFsmInitMpwr(pxState);
        break;

    case FSM_READY:
        prvFsmReady(pxState);
        break;

    case FSM_IDLE:
        prvFsmIdle(pxState);
        break;

    case FSM_LASERs_INIT:
        prvFsmLaserSInit(pxState);
        break;
    case FSM_LASERs_RUN:
        prvFsmLaserSRun(pxState);
        break;
    case FSM_LASERs_DONE:
        prvFsmLaserSDone(pxState);
        break;

    case FSM_LASERm_INIT:
        prvFsmLaserMInit(pxState);
        break;
    case FSM_LASERm_RUN:
        prvFsmLaserMRun(pxState);
        break;
    case FSM_LASERm_DONE:
        prvFsmLaserMDone(pxState);
        break;

    case FSM_INFRARED_INIT:
        prvFsmInfraredInit(pxState);
        break;
    case FSM_INFRARED_RUN:
        prvFsmInfraredRun(pxState);
        break;
    case FSM_INFRARED_DONE:
        prvFsmInfraredDone(pxState);
        break;

    case FSM_ERROR:
        prvFsmError(pxState);
        break;

    default:
        /* We should never get here! */
        break;
    }

    pxState->ulCounter++;

    return;
}

static void prvFsmStart(State_t *pxState)
{
    SYS_SAVELASTSTATES();

    pxState->xState    = FSM_INIT_MPWR;
    pxState->ulCounter = 0;
    TRACE("[%6d] Start        -> InitMpwr\n", SYS_TICK_GET());
} 

static void prvFsmInitMpwr(State_t *pxState)
{
    SYS_SAVELASTSTATES();

    if (prvInitChkMPwr()) {
        pxState->xState    = FSM_READY;
        pxState->ulCounter = 0;
        TRACE("[%6d] InitMPwr     -> Ready\n", SYS_TICK_GET());
    }
}

static void prvFsmReady(State_t *pxState)
{
    SYS_SAVELASTSTATES();

    if (pxState->ulCounter >= FSM_READY_WAIT) {
        pxState->xState    = FSM_IDLE;
        pxState->ulCounter = 0;
        TRACE("[%6d] Ready        -> Idle\n", SYS_TICK_GET());
    }
}

static void prvFsmIdle(State_t *pxState)
{
    SYS_SAVELASTSTATES();

    if (!prvChkAPwr()) {
        /* 内控模式 */
        pxState->xState         = FSM_ERROR;
        pxState->ulCounter      = 0;
        th_SysStatus.WORK_LASER = 0;
        prvEnterFsm();
        TRACE("[%6d] Idle    -> Error\n", SYS_TICK_GET());
    }

}

static void prvFsmLaserSInit(State_t *pxState)
{
    SYS_SAVELASTSTATES();
    switch (th_CtrlMode) {
    /* DAC Mode */
    case 1:
        if (prvChkAPwr()) {
            GpioSetOutput(APWR1_EN, s_ucAPwrCtrl1);
            GpioSetOutput(APWR2_EN, s_ucAPwrCtrl2);
            GpioSetOutput(APWR3_EN, s_ucAPwrCtrl3);
            if (s_ucAPwrCtrl1 == APWR_ON) {
                DacSet(APWR1_CTRL, CUR_TO_DAC(s_ulCurrent / 10.f));
            }
            if (s_ucAPwrCtrl2 == APWR_ON) {
                DacSet(APWR2_CTRL, CUR_TO_DAC(s_ulCurrent / 10.f));
            }
            if (s_ucAPwrCtrl3 == APWR_ON) {
                DacSet(APWR3_CTRL, CUR_TO_DAC(s_ulCurrent / 10.f));
            }
            if (pxState->ulCounter >= 1) {
                pxState->xState         = FSM_LASERs_RUN;
                pxState->ulCounter      = 0;
                th_SysStatus.WORK_LASER = 1;
                TRACE("[%6d] LaserSInit   -> LaserSRun\n", SYS_TICK_GET());
                TRACE("[%6d]     Set APWR1_EN %d, APWR1_CTRL %.1f A (%4d mV)\n", SYS_TICK_GET(), s_ucAPwrCtrl1, DAC_TO_CUR(DacGet(APWR1_CTRL)), DAC_TO_MVOL(DacGet(APWR1_CTRL)));
                TRACE("[%6d]     Set APWR2_EN %d, APWR2_CTRL %.1f A (%4d mV)\n", SYS_TICK_GET(), s_ucAPwrCtrl2, DAC_TO_CUR(DacGet(APWR2_CTRL)), DAC_TO_MVOL(DacGet(APWR2_CTRL)));
                TRACE("[%6d]     Set APWR3_EN %d, APWR3_CTRL %.1f A (%4d mV)\n", SYS_TICK_GET(), s_ucAPwrCtrl3, DAC_TO_CUR(DacGet(APWR3_CTRL)), DAC_TO_MVOL(DacGet(APWR3_CTRL)));
#if 0
                    for (uint8_t n = 0; n < 5; n++) {
                        Status_t r = PwrOutput(1);
                        TRACE("[%6d]     Set MPWR output on, result = %d\n", SYS_TICK_GET(), r);
                        if (r == STATUS_OK) {
                            break;
                        }
                    }
#endif
            }
        }
        else {
            pxState->xState    = FSM_ERROR;
            pxState->ulCounter = 0;
            prvEnterFsm();
            TRACE("[%6d] LaserSInit   -> Error\n", SYS_TICK_GET());
        }
        break;

    /* PWM Mode */
    case 2:
        if (prvChkAPwr()) {
            GpioSetOutput(APWR1_EN, s_ucAPwrCtrl1);
            GpioSetOutput(APWR2_EN, s_ucAPwrCtrl2);
            GpioSetOutput(APWR3_EN, s_ucAPwrCtrl3);
            if (s_ucAPwrCtrl1 == APWR_ON) {
                SetAFreq(CUR_TO_FREQ((s_ulCurrent)));
                SetADuty(50);
//                TRACE("CUR_TO_FREQ : %.1f\n", CUR_TO_FREQ((s_ulCurrent)));
//                TRACE("TIM4->PSC   : %d\n",  TIM4->PSC);
            }
            if (s_ucAPwrCtrl2 == APWR_ON) {
                SetAFreq(CUR_TO_FREQ((s_ulCurrent)));
                SetADuty(50);
            }
            if (s_ucAPwrCtrl3 == APWR_ON) {
                SetAFreq(CUR_TO_FREQ((s_ulCurrent)));
                SetADuty(50);
            }

            ToggleCcsStatus(1);

//            TRACE("cur     = %d A\n", ((int)(s_ulCurrent / 10.f)));
//            TRACE("f       = %.1f Hz\n", CUR_TO_FREQ((s_ulCurrent / 10.f)));

            if (pxState->ulCounter >= 1) {
                pxState->xState         = FSM_LASERs_RUN;
                pxState->ulCounter      = 0;
                th_SysStatus.WORK_LASER = 1;
          
                TRACE("[%6d] LaserSInit   -> LaserSRun\n", SYS_TICK_GET());
                TRACE("[%6d]     Set APWR1_EN %d, APWR1_CTRL %.1f A (%d Hz), Duty( %3d %% )\n", SYS_TICK_GET(), s_ucAPwrCtrl1, PSC_TO_CUR(TIM4->PSC), (int)PSC_TO_FREQ(TIM4->PSC), (uint32_t)(TIM4->CCR3 * 100 / (TIM4->ARR + 1)));
                TRACE("[%6d]     Set APWR2_EN %d, APWR2_CTRL %.1f A (%d Hz), Duty( %3d %% )\n", SYS_TICK_GET(), s_ucAPwrCtrl2, PSC_TO_CUR(TIM4->PSC), (int)PSC_TO_FREQ(TIM4->PSC), (uint32_t)(TIM4->CCR3 * 100 / (TIM4->ARR + 1)));
                TRACE("[%6d]     Set APWR3_EN %d, APWR3_CTRL %.1f A (%d Hz), Duty( %3d %% )\n", SYS_TICK_GET(), s_ucAPwrCtrl3, PSC_TO_CUR(TIM4->PSC), (int)PSC_TO_FREQ(TIM4->PSC), (uint32_t)(TIM4->CCR3 * 100 / (TIM4->ARR + 1)));

#if 0
                    for (uint8_t n = 0; n < 5; n++) {
                        Status_t r = PwrOutput(1);
                        TRACE("[%6d]     Set MPWR output on, result = %d\n", SYS_TICK_GET(), r);
                        if (r == STATUS_OK) {
                            break;
                        }
                    }
#endif
            }
        }
        else {
            pxState->xState    = FSM_ERROR;
            pxState->ulCounter = 0;
            prvEnterFsm();
            TRACE("[%6d] LaserSInit   -> Error\n", SYS_TICK_GET());
        }
        break;

    /* AD Mode */
    case 3:

        break;
    }
}

static void prvFsmLaserSRun(State_t *pxState)
{
    SYS_SAVELASTSTATES();

    if (!prvChkPwr()) {
        pxState->xState         = FSM_ERROR;
        pxState->ulCounter      = 0;
        th_SysStatus.WORK_LASER = 0;
        prvEnterFsm();
        TRACE("[%6d] LaserSRun    -> Error\n", SYS_TICK_GET());
    }
}

static void prvFsmLaserSDone(State_t *pxState)
{
    SYS_SAVELASTSTATES();

    pxState->xState         = FSM_IDLE;
    pxState->ulCounter      = 0;
    th_SysStatus.WORK_LASER = 0;
    TRACE("[%6d] LaserSDone   -> Idle\n", SYS_TICK_GET());
#if 0
    for (uint8_t n = 0; n < 5; n++) {
        Status_t r = PwrOutput(0);
        TRACE("[%6d]     Set MPWR output on, result = %d\n", SYS_TICK_GET(), r);
        if (r == STATUS_OK) {
            break;
        }
    }
#endif
}

static void prvFsmLaserMInit(State_t *pxState)
{
    SYS_SAVELASTSTATES();
    
    uint16_t ExAdVol = AdcGet(ADC_CHAN_7);
    
    pxState->xState         = FSM_LASERm_RUN;
    pxState->ulCounter      = 0;
    th_SysStatus.WORK_LASER = 1;
    
    if (prvChkAPwr()) {
        TRACE("[%6d] LaserMInit   -> LaserMRun\n", SYS_TICK_GET());
        TRACE("[%6d]     Set APWR1_EN 1, APWR1_CTRL %.1f A (%4d mV)\n", SYS_TICK_GET(), DAC_TO_CUR(ExAdVol), DAC_TO_MVOL(ExAdVol));
        TRACE("[%6d]     Set APWR2_EN 1, APWR2_CTRL %.1f A (%4d mV)\n", SYS_TICK_GET(), DAC_TO_CUR(ExAdVol), DAC_TO_MVOL(ExAdVol));
        TRACE("[%6d]     Set APWR3_EN 1, APWR3_CTRL %.1f A (%4d mV)\n", SYS_TICK_GET(), DAC_TO_CUR(ExAdVol), DAC_TO_MVOL(ExAdVol));
    }
    else {
        pxState->xState    = FSM_ERROR;
        pxState->ulCounter = 0;
        prvEnterFsm();
        TRACE("[%6d] LaserMInit   -> Error\n", SYS_TICK_GET());
    }
}

static void prvFsmLaserMRun(State_t *pxState)
{
    SYS_SAVELASTSTATES();
//    TRACE("[%6d] LaserMRun without error\n", SYS_TICK_GET());
    if (!prvChkPwr()) {
        pxState->xState         = FSM_ERROR;
        pxState->ulCounter      = 0;
        th_SysStatus.WORK_LASER = 0;
        prvEnterFsm();
        TRACE("[%6d] LaserMRun    -> Error\n", SYS_TICK_GET());
    }
} 

static void prvFsmLaserMDone(State_t *pxState)
{
    SYS_SAVELASTSTATES();

    pxState->xState         = FSM_IDLE;
    pxState->ulCounter      = 0;
    th_SysStatus.WORK_LASER = 0;
    TRACE("[%6d] LaserMDone   -> Idle\n", SYS_TICK_GET());
}

static void prvFsmInfraredInit(State_t *pxState)
{
    SYS_SAVELASTSTATES();

    pxState->xState            = FSM_INFRARED_RUN;
    pxState->ulCounter         = 0;
    th_SysStatus.WORK_INFRARED = 1;
    ToggleAimLight(1);
    TRACE("[%6d] InfraredInit -> InfraredRun\n", SYS_TICK_GET());
    TRACE("[%6d]     Set LED_CTRL on\n", SYS_TICK_GET());
}

static void prvFsmInfraredRun(State_t *pxState)
{
    SYS_SAVELASTSTATES();
    if (!prvChkAPwr()) {
        pxState->xState         = FSM_ERROR;
        pxState->ulCounter      = 0;
        th_SysStatus.WORK_LASER = 0;
        TRACE("[%6d] InfraredRun    -> Error\n", SYS_TICK_GET());
    }
}

static void prvFsmInfraredDone(State_t *pxState)
{
    SYS_SAVELASTSTATES();

    pxState->xState            = FSM_IDLE;
    pxState->ulCounter         = 0;
    th_SysStatus.WORK_INFRARED = 0;
    ToggleAimLight(0);
    TRACE("[%6d] InfraredDone -> Idle\n", SYS_TICK_GET());
}

static void prvFsmError(State_t *pxState)
{
//    SYS_SAVELASTSTATES();
    
    GpioSetOutput(FAULT, 1);

    GpioSetOutput(BEEP_SW, 1);
    osDelay(BEEP_DELAY);
    GpioSetOutput(BEEP_SW, 0);
    osDelay(BEEP_DELAY);

    
    if (prvChkPwr() && laser_on_pd_err == 1) {

        GpioSetOutput(BEEP_SW, 0);
        
        /* 错误状态下 指示光变化处理 */
        if (th_CtrlMode == 3) {
            GpioSetOutput(MOD_EN, 1);
            
            GpioSetOutput(FAULT, 0);
            
            uint8_t AimEn = GpioGetInput(AIM_EN);
            if (AimEn == 0){
                pxState->xState    = FSM_INFRARED_RUN;
                pxState->ulCounter = 0;
                ToggleAimLight(1);
                TRACE("[%6d] Error        -> InfraredRun\n", SYS_TICK_GET());
            }
            else {
                pxState->xState    = FSM_IDLE;
                pxState->ulCounter = 0;
                ToggleAimLight(0);
                TRACE("[%6d] Error        -> Idle\n", SYS_TICK_GET());
            }
        }
        
        
    }
    
    if (pxState->xLastState == FSM_IDLE && manual_ctrl_err == 1) {
        if (prvChkAPwr()) {
            pxState->xState    = FSM_IDLE;
            pxState->ulCounter = 0;
            TRACE("[%6d] Error        -> Idle\n", SYS_TICK_GET());
        }
    }
    
    uint16_t ModEnStatus = GpioGetInput(EX_MOD_EN);
    if (manual_ctrl_err == 0 && ModEnStatus == 1) {
        pxState->xState    = FSM_IDLE;
        pxState->ulCounter = 0;
        manual_ctrl_err = 1;
        TRACE("[%6d] Error        -> Idle\n", SYS_TICK_GET());
    }
    else if (manual_ctrl_err == 0 && ModEnStatus == 0) {
        TRACE("[%6d] Please make sure DC_OK.\n", SYS_TICK_GET());
    }
}

static bool prvInitChkMPwr(void)
{
    if (th_CVS == 0)
    {
        return true;
    }
    
    uint16_t s1 = GpioGetInput(MPWR_STAT_AC);
    uint16_t s2 = GpioGetInput(MPWR_STAT_DC);
    bool     b  = PwrIsOk();
    if ((s1 == MPWR_AC_OK) && (GpioGetOutput(MPWR_EN) != MPWR_ON)) {
        GpioSetOutput(MPWR_EN, MPWR_ON);
        TRACE("[%6d]     Set MPWR_EN on\n", SYS_TICK_GET());
    }
    return ((s1 == MPWR_AC_OK) && (s2 == MPWR_DC_OK) && b) ? true : false;
}

static bool prvChkPwr(void)
{
    return (prvChkMPwr() && prvChkAPwr());
}

static bool prvChkMPwr(void)
{
    if (th_CVS == 0) {
        /* Don't check cvs */
        return true;
    }

    int32_t  v  = PwrDataGet(PWR2_M1_ADDR, PWR_OUTPUT_VOL);
    uint16_t s1 = GpioGetInput(MPWR_STAT_AC);
    uint16_t s2 = GpioGetInput(MPWR_STAT_DC);
    return ((v >= 650 /*0.1V*/) && (s1 == MPWR_AC_OK) && (s2 == MPWR_DC_OK)) ? true : false;
}

static bool prvChkAPwr(void)
{
    if (th_CCS == 0) {
        /* Don't check ccs */
        return true;
    }

    if (s_xState.xState != FSM_ERROR) {
        uint16_t s1 = GpioGetInput(APWR1_STAT);
        uint16_t s2 = GpioGetInput(APWR2_STAT);
        uint16_t s3 = GpioGetInput(APWR3_STAT);
        if (((s_ucAPwrCtrl1 == APWR_ON) && (s1 != APWR_OK)) || ((s_ucAPwrCtrl2 == APWR_ON) && (s2 != APWR_OK)) ||
            (((s_ucAPwrCtrl3 == APWR_ON)) && (s3 != APWR_OK))) {
            TRACE("[%6d]     APWRn_STAT error\n", SYS_TICK_GET());
            return false;
        }
    }

    /* 温度组1检测 */
    int16_t sTemp1 = StcGetTempHFrom(0, th_TempNum - 1);
    if (th_ModEn.TEMP1) {
        if (sTemp1 <= th_OtCutTh) {
            /* 切断温度 */
            TRACE("[%6d]     Group1 over cut temperature\n", SYS_TICK_GET());
            return false;
        }
        else if (sTemp1 <= th_OtWarnTh) {
            /* 告警温度 */
            /* TRACE("[%6d]     Group 1 over warning temperature\n", SYS_TICK_GET()); */
        }
    }

    /* 温度组2检测 */
    int16_t sTemp2 = StcGetTempHFrom(th_TempNum, 12 - 1);
    if (th_ModEn.TEMP2) {
        if (sTemp2 <= th_OtCutTh) {
            /* 切断温度 */
            TRACE("[%6d]     Group2 over cut temperature\n", SYS_TICK_GET());
            return false;
        }
        else if (sTemp2 <= th_OtWarnTh) {
            /* 告警温度 */
            /* TRACE("[%6d]     Group2 over warning temperature\n", SYS_TICK_GET()); */
        }
    }
    
    /* PD漏光检测 */
    int16_t pdVol = StcGetPdHFrom();
    if (th_ModEn.PD && ((pdVol >= th_PdWarnL1))) {
        TRACE("[%6d]     PD_VS is over threshold\n", SYS_TICK_GET());
        return false;
    }
    
    /* PD开激光不出光检测 */
    int16_t pdLight = StcGetPdLight();
    State_t *pxState = &s_xState;
    if (th_PdLightEn && ((pxState->xState == FSM_LASERs_RUN) || (pxState->xState == FSM_LASERm_RUN)) && ((pdLight <= th_PdLight))) {
        laser_on_pd_err = 0;
        TRACE("[%6d]     PD Light is lower threshold\n", SYS_TICK_GET());
        return false;
    }

    /* QBH旋钮到位检测 */
    if (th_ModEn.QBH && (QBH_ON_OFF == GpioGetInput(QBH_ON))) {
        TRACE("[%6d]     QBH_ON is off\n", SYS_TICK_GET());
        return false;
    }

    /* 水冷机检测 */
    if (th_ModEn.WATER_CHILLER && (WATER_CHILLER_OFF == GpioGetInput(WATER_CHILLER))) {
        TRACE("[%6d]     WATER_CHILLER is off\n", SYS_TICK_GET());
        return false;
    }

    /* 水压检测 */
    if (th_ModEn.WATER_PRESS && (WATER_PRESS_OFF == GpioGetInput(WATER_PRESS))) {
        TRACE("[%6d]     WATER_PRESS is off\n", SYS_TICK_GET());
        return false;
    }

    /* 通道1电流检测 */
    uint16_t usChan1Cur = AdcGet(APWR1_CUR);
    if (th_ModEn.CHAN1_CUR && (usChan1Cur > th_MaxCurAd)) {
        TRACE("[%6d]     APWR1_CUR is over threshold\n", SYS_TICK_GET());
        return false;
    }

    /* 通道2电流检测 */
    uint16_t usChan2Cur = AdcGet(APWR2_CUR);
    if (th_ModEn.CHAN2_CUR && (usChan2Cur > th_MaxCurAd)) {
        TRACE("[%6d]     APWR2_CUR is over threshold\n", SYS_TICK_GET());
        return false;
    }

    /* 通道3电流检测 */
    uint16_t usChan3Cur = AdcGet(APWR3_CUR);
    if (th_ModEn.CHAN3_CUR && (usChan3Cur > th_MaxCurAd)) {
        TRACE("[%6d]     APWR3_CUR is over threshold\n", SYS_TICK_GET());
        return false;
    }

    /* LASER_EN检测 */
    uint16_t usLaserEn = GpioGetInput(LASER_EN);
    if (usLaserEn == 1) {
        TRACE("[%6d]     LASER_EN is off\n", SYS_TICK_GET());
        return false;
    }
    
    /* 互锁检测 */
    uint16_t usSafeLock = GpioGetInput(SAFE_LOCK);
    if (usSafeLock == 1) {
        TRACE("[%6d]     SAFE_INTER_LOCK is off\n", SYS_TICK_GET());
        return false;
    }

    return true;
}

static void prvEnterFsm(void)
{
    switch (th_CtrlMode)
    {
        case 1:
        case 2:
            TRACE("[%6d]     Set APWRx_EN off\n", SYS_TICK_GET());
            GpioSetOutput(APWR1_EN, APWR_OFF);
            GpioSetOutput(APWR2_EN, APWR_OFF);
            GpioSetOutput(APWR3_EN, APWR_OFF);
            break;
        case 3:
            TRACE("[%6d]     Set MOD_EN off\n", SYS_TICK_GET());
            GpioSetOutput(MOD_EN, 0);
            break;
    }
    
}

static void prvProcManualCtrl(void)
{
    static uint8_t on  = 0;
    static uint8_t off = 0;

    if (!s_bManualCtrl || !th_ModEn.MANUAL) {
        return;
    }

    if (GpioGetInput(EX_CTRL_EN) == 0) {
        on++;
        if (on >= 3) {
            on = 3;
        }
    }
    else {
        on = 0;
    }

    if (GpioGetInput(EX_CTRL_EN) == 1) {
        off++;
        if (off >= 3) {
            off = 3;
        }
    }
    else {
        off = 0;
    }

    /* Manual control enable: on */
    if (on == 3) {
    }

    /* Manual control enable: off */
    if (off == 3) {
    }
}

static void prvProcPanelLed(void)
{
    uint8_t  s_MPWR_STAT_AC   = GpioGetInput(MPWR_STAT_AC);
    uint8_t  s_MPWR_STAT_DC   = GpioGetInput(MPWR_STAT_DC);
    uint8_t  s_APWR1_STAT     = GpioGetInput(APWR1_STAT);
    uint8_t  s_APWR2_STAT     = GpioGetInput(APWR2_STAT);
    uint8_t  s_APWR3_STAT     = GpioGetInput(APWR3_STAT);
    uint8_t  s_QBH_ON         = GpioGetInput(QBH_ON);
    uint8_t  s_WATER_PRESS    = GpioGetInput(WATER_PRESS);
    uint8_t  s_WATER_CHILLER  = GpioGetInput(WATER_CHILLER);
    int16_t  s_MAX_TEMP1      = StcGetTempHFrom(0, th_TempNum - 1);
    int16_t  s_MAX_TEMP2      = StcGetTempHFrom(th_TempNum, 10 - 1);
    int32_t  s_PWR_OUTPUT_VOL = PwrDataGet(PWR2_M1_ADDR, PWR_OUTPUT_VOL);
    int32_t  s_PWR_INPUT_VOL  = PwrDataGet(PWR2_M1_ADDR, PWR_INPUT_VOL);
    uint16_t s_PD             = StcGetPdHFrom();
    uint16_t s_CHAN1_CUR      = AdcGet(APWR1_CUR);
    uint16_t s_CHAN2_CUR      = AdcGet(APWR2_CUR);
    uint16_t s_CHAN3_CUR      = AdcGet(APWR3_CUR);
    uint16_t s_LASER_EN       = GpioGetInput(LASER_EN);
    uint16_t s_MPWR_EN        = GpioGetInput(MPWR_EN);
    int16_t  s_pdLight        = StcGetPdLight();
    
//    TRACE("s_PD = %d\n", s_PD);

    /* Power */
    /* G: PWR_OUTPUT_VOL >= 65V */
    /* R: PWR_OUTPUT_VOL <  65V */
    static uint8_t s1 = 0xFF;
    if (s_PWR_OUTPUT_VOL >= 650 /*0.1V*/) {
        if (s1 != 1) {
            GpioSetOutput(LED_POWER_G, LED_ON);
            GpioSetOutput(LED_POWER_R, LED_OFF);
            s1 = 1;
        }
    }
    else {
        if (s1 != 0) {
            GpioSetOutput(LED_POWER_G, LED_OFF);
            GpioSetOutput(LED_POWER_R, LED_ON);
            s1 = 0;
        }
    }

    /* Active */
    /* G: ((APWR1_EN && APWR1_CUR) || (APWR2_EN && APWR2_CUR) || (APWR3_EN && APWR3_CUR)) */
    /* R: !G */
    static uint8_t s2         = 0xFF;
    uint16_t       usApwr1Cur = AdcGet(APWR1_CUR);
    uint16_t       usApwr2Cur = AdcGet(APWR2_CUR);
    uint16_t       usApwr3Cur = AdcGet(APWR3_CUR);
    if (((s_ucAPwrCtrl1 == APWR_ON) && (usApwr1Cur > 124 /*2A*/)) ||
        ((s_ucAPwrCtrl2 == APWR_ON) && (usApwr2Cur > 124 /*2A*/)) ||
        ((s_ucAPwrCtrl3 == APWR_ON) && (usApwr3Cur > 124 /*2A*/))) {
        if (s2 != 1) {
            GpioSetOutput(LED_ACTIVE_G, LED_ON);
            GpioSetOutput(LED_ACTIVE_R, LED_OFF);
            th_SysStatus.RUN = 1;
            s2               = 1;
        }
    }
    else {
        if (s2 != 0) {
            GpioSetOutput(LED_ACTIVE_G, LED_OFF);
            GpioSetOutput(LED_ACTIVE_R, LED_ON);
            th_SysStatus.RUN = 0;
            s2               = 0;
        }
    }

    State_t *pxState = &s_xState;
    /* Alarm */
    uint8_t s3 = 0xFF;
    /* R: QBH_ON 1 */
    /* R: WATER_PRESS 1 */
    /* R: WATER_CHILLER 1 */
    /* R: Max temperature > over temperature cut threshold */
    /* R: Channel current > maximum current */
    /* R: TODO: MPWR CAN */
    /* R: PD */
    if ((th_ModEn.QBH && (s_QBH_ON == QBH_ON_OFF)) || (th_ModEn.WATER_PRESS && (s_WATER_PRESS == WATER_PRESS_OFF)) ||
        (th_ModEn.WATER_CHILLER && (s_WATER_CHILLER == WATER_CHILLER_OFF)) ||
        (th_ModEn.TEMP1 && (s_MAX_TEMP1 <= th_OtCutTh)) || (th_ModEn.TEMP2 && (s_MAX_TEMP2 <= th_OtCutTh)) ||
        (th_ModEn.CHAN1_CUR && (s_CHAN1_CUR > th_MaxCurAd)) || (th_ModEn.CHAN2_CUR && (s_CHAN2_CUR > th_MaxCurAd)) ||
        (th_ModEn.CHAN3_CUR && (s_CHAN3_CUR > th_MaxCurAd)) || (th_ModEn.PD && (s_PD >= th_PdWarnL1)) ||
        (s_LASER_EN == 1) || 
        (th_PdLightEn && ((pxState->xState == FSM_LASERs_RUN) || (pxState->xState == FSM_LASERm_RUN)) && ((s_pdLight <= th_PdLight))) || 
        (laser_on_pd_err == 0)) {
        if (s3 != 0) {
            GpioSetOutput(LED_ALARM_R, LED_ON);
            GpioSetOutput(LED_ALARM_G, LED_OFF);
            s3 = 0;
        }
    }
    /* Y: Max temperature > over temperature warn threshold */
    /* Y: PD */
    /* Y: TODO: CAN */
    else if ((th_ModEn.TEMP1 && (s_MAX_TEMP1 <= th_OtWarnTh)) || (th_ModEn.TEMP2 && (s_MAX_TEMP2 <= th_OtWarnTh)) ||
             (th_ModEn.PD && (s_PD >= th_PdWarnL2))) {
        if (s3 != 1) {
            GpioSetOutput(LED_ALARM_R, LED_ON);
            GpioSetOutput(LED_ALARM_G, LED_ON);
            s3 = 1;
        }
    }
    /* G: Otherwise */
    else {
        if (s3 != 2) {
            GpioSetOutput(LED_ALARM_R, LED_OFF);
            GpioSetOutput(LED_ALARM_G, LED_ON);
            s3 = 2;
        }
    }
    
    /* Power led */
    /* Led ac ok */
//    TRACE("PWR_LED_RUN \n");
    if ( s_PWR_INPUT_VOL >= 1000 /* 0.1V */){
        GpioSetOutput(LED_AC_OK, LED_ON);
        GpioSetOutput(LED_AC_FAIL, LED_OFF);
    }
    else {
        GpioSetOutput(LED_AC_OK, LED_OFF);
        GpioSetOutput(LED_AC_FAIL, LED_ON);
    }
    
    /* Led dc ok */
    if (s_PWR_OUTPUT_VOL >= 650){
        GpioSetOutput(LED_DC_OK, LED_ON);
        GpioSetOutput(LED_DC_FAIL, LED_OFF);
    }
    else {
        GpioSetOutput(LED_DC_OK, LED_OFF);
        GpioSetOutput(LED_DC_FAIL, LED_ON);
    }
    
    /* Led can */
    if (s_PWR_INPUT_VOL >= 1000 /*0.1V*/) {
        GpioSetOutput(LED_CAN_OK, LED_ON);
        GpioSetOutput(LED_CAN_FAIL, LED_OFF);
    }
    else {
        GpioSetOutput(LED_CAN_OK, LED_OFF);
        GpioSetOutput(LED_CAN_FAIL, LED_ON);
    }

    /* System status update */
    /* Main power */
    if ((s_PWR_OUTPUT_VOL >= 650 /*0.1V*/) && (s_MPWR_STAT_AC == MPWR_AC_OK) && (s_MPWR_STAT_DC == MPWR_AC_OK)) {
        th_SysStatus.STATUS_MPWR = 1;
    }
    else {
        th_SysStatus.STATUS_MPWR = 0;
    }
    /* Emergent stop */
    static uint8_t s_last_MPWR_STAT_AC = 0;
    if ((s_last_MPWR_STAT_AC == 0) && (s_MPWR_STAT_AC == 1)) {
        th_SysStatus.ALARM_EMCY_STOP = 0;
    }
    if ((s_last_MPWR_STAT_AC == 1) && (s_MPWR_STAT_AC == 0)) {
        th_SysStatus.ALARM_EMCY_STOP = 1;
    }
    s_last_MPWR_STAT_AC = s_MPWR_STAT_AC;
    /* Water chiller */
    if (th_ModEn.WATER_CHILLER && (s_WATER_CHILLER == WATER_CHILLER_OFF)) {
        th_SysStatus.ALARM_WATER_CHILLER = 1;
    }
    else {
        th_SysStatus.ALARM_WATER_CHILLER = 0;
    }
    /* Water press */
    if (th_ModEn.WATER_PRESS && (s_WATER_PRESS == WATER_PRESS_OFF)) {
        th_SysStatus.ALARM_WATER_PRESS = 1;
    }
    else {
        th_SysStatus.ALARM_WATER_PRESS = 0;
    }
    /* QBH */
    if (th_ModEn.QBH && (s_QBH_ON == QBH_ON_OFF)) {
        th_SysStatus.ALARM_QBH = 1;
    }
    else {
        th_SysStatus.ALARM_QBH = 0;
    }
    /* OT */
    if ((th_ModEn.TEMP1 && (s_MAX_TEMP1 <= th_OtCutTh)) || (th_ModEn.TEMP2 && (s_MAX_TEMP2 <= th_OtCutTh))) {
        th_SysStatus.ALARM_OT = 1;
        th_SysStatus.WARN_OT  = 0;
    }
    else if ((th_ModEn.TEMP1 && (s_MAX_TEMP1 <= th_OtWarnTh)) || (th_ModEn.TEMP2 && (s_MAX_TEMP2 <= th_OtWarnTh))) {
        th_SysStatus.ALARM_OT = 0;
        th_SysStatus.WARN_OT  = 1;
    }
    else {
        th_SysStatus.ALARM_OT = 0;
        th_SysStatus.WARN_OT  = 0;
    }
    /* PD */
    if (th_ModEn.PD && (s_PD >= th_PdWarnL1)) {
        th_SysStatus.ALARM_PD = 1;
        th_SysStatus.WARN_PD  = 0;
    }
    else if (th_ModEn.PD && (s_PD >= th_PdWarnL2)) {
        th_SysStatus.ALARM_PD = 0;
        th_SysStatus.WARN_PD  = 1;
    }
    else {
        th_SysStatus.ALARM_PD = 0;
        th_SysStatus.WARN_PD  = 0;
    }
    
    
}

static void prvCliCmdSysFsm(cli_printf cliprintf, int argc, char **argv)
{
    CHECK_CLI();

    if (argc != 2) {
        cliprintf("sys_fsm ON_OFF\n");
        return;
    }

    int lOnOff = atoi(argv[1]);
    lOnOff ? (s_bProc = true) : (s_bProc = false);
}
CLI_CMD_EXPORT(sys_fsm, start or stop system fsm process, prvCliCmdSysFsm)

static void prvCliCmdEnableManualCtrl(cli_printf cliprintf, int argc, char **argv)
{
    CHECK_CLI();

    if (argc != 2) {
        cliprintf("enable_manual_ctrl ON_OFF\n");
        return;
    }

    int lOnOff = atoi(argv[1]);
    lOnOff ? (s_bManualCtrl = true) : (s_bManualCtrl = false);
}
CLI_CMD_EXPORT(enable_manual_ctrl, enable manual ctrl, prvCliCmdEnableManualCtrl)

static void prvCliCmdShowSysStatus(cli_printf cliprintf, int argc, char **argv)
{
    CHECK_CLI();

    cliprintf("STATUS_MPWR        : %d\n", th_SysStatus.STATUS_MPWR);
    cliprintf("ALARM_EMCY_STOP    : %d\n", th_SysStatus.ALARM_EMCY_STOP);
    cliprintf("ALARM_WATER_CHILLER: %d\n", th_SysStatus.ALARM_WATER_CHILLER);
    cliprintf("ALARM_WATER_PRESS  : %d\n", th_SysStatus.ALARM_WATER_PRESS);
    cliprintf("ALARM_QBH          : %d\n", th_SysStatus.ALARM_QBH);
    cliprintf("ALARM_OT           : %d\n", th_SysStatus.ALARM_OT);
    cliprintf("WARN_OT            : %d\n", th_SysStatus.WARN_OT);
    cliprintf("ALARM_PD           : %d\n", th_SysStatus.ALARM_PD);
    cliprintf("WARN_PD            : %d\n", th_SysStatus.WARN_PD);
    cliprintf("ALARM_WET          : %d\n", th_SysStatus.ALARM_WET);
    cliprintf("WORK_INFRARED      : %d\n", th_SysStatus.WORK_INFRARED);
    cliprintf("WORK_LASER         : %d\n", th_SysStatus.WORK_LASER);
    cliprintf("RUN                : %d\n", th_SysStatus.RUN);
}
CLI_CMD_EXPORT(show_sys_status, show system status, prvCliCmdShowSysStatus)

static void prvCliCmdFsmTest(cli_printf cliprintf, int argc, char **argv)
{
    CHECK_CLI();

    if (argc != 3) {
        cliprintf("fsm_goto ID Cur\n");
        return;
    }

    int   Step = atoi(argv[1]);
    float Cur  = atof(argv[2]);

    switch (Step) {
    case 1:
        if (LaserOn(Cur * 10, 0x07) == STATUS_ERR) {
            TRACE("LaserOn Error\n");
        }
        break;

    case 2:
        if (LaserOff(0x07) == STATUS_ERR) {
            TRACE("LaserOff Error\n");
        }
        break;

    case 3:
        if (InfraredOn() == STATUS_ERR) {
            TRACE("InfraredOn Error\n");
        }
        break;

    case 4:
        if (InfraredOff() == STATUS_ERR) {
            TRACE("InfraredOff Error\n");
        }
        break;

    default:
        TRACE("fsm_goto ID\n");
        TRACE("    1 - LaserOn;\n");
        TRACE("    2 - LaserOff;\n");
        TRACE("    3 - InfraredOn;\n");
        TRACE("    4 - InfraredOff;\n");
        break;
    }
}
CLI_CMD_EXPORT(sys_fsm_goto, test system fsm process, prvCliCmdFsmTest)

static void prvCliCmdPwmSet(cli_printf cliprintf, int argc, char **argv)
{
    CHECK_CLI();

    if (argc != 3) {
        cliprintf("pwm_set f d\n");
        return;
    }

    int f = atoi(argv[1]);
    int d = atoi(argv[2]);

    SetAFreq(f);
    SetADuty(50);

    cliprintf("pwm info set ok\n");
}
CLI_CMD_EXPORT(pwm_set, pwm set f d, prvCliCmdPwmSet)

static void prvCliCmdPwmGet(cli_printf cliprintf, int argc, char **argv)
{
    CHECK_CLI();

    if (argc != 1) {
        cliprintf("pwm_get\n");
        return;
    }

    int ccr = TIM4->CCR3;
    int psc = TIM4->PSC;
    
    int ccr_aim = TIM2->CCR1;
    int psc_aim = TIM2->PSC;

    cliprintf("ccr = %d\n", ccr);
    cliprintf("psc = %d\n\n", psc);
    
    cliprintf("ccr_aim = %d\n", ccr_aim);
    cliprintf("psc_aim = %d\n", psc_aim);
}
CLI_CMD_EXPORT(pwm_get, pwm get f d, prvCliCmdPwmGet)

//static void prvCliCmdShowFsmStatus(cli_printf cliprintf, int argc, char **argv)
//{
//    CHECK_CLI();

//    if (argc != 1) {
//        cliprintf("show_fsm_status\n");
//        return;
//    }
//    
//    State_t *pxState = &s_xState;
//    cliprintf("FSM_State = %d\n", pxState->xState);
//}
//CLI_CMD_EXPORT(show_fsm_status, show fsm status, prvCliCmdShowFsmStatus)

//static void prvCliCmdCtrlStatus(cli_printf cliprintf, int argc, char **argv)
//{
//    CHECK_CLI();

//    if (argc != 1) {
//        cliprintf("show_fsm_status\n");
//        return;
//    }
//    uint16_t ModEnStatus = GpioGetInput(MOD_EN);
//    
//    cliprintf("MOD_EN  :  %d\n", ModEnStatus);
//    
//}
//CLI_CMD_EXPORT(show_ctrl_status, show fsm status, prvCliCmdCtrlStatus)

static void prvCliCmdSysDiag(cli_printf cliprintf, int argc, char **argv)
{
    CHECK_CLI();

    if (argc != 1) {
        cliprintf("sys_diag\n");
        return;
    }
    
    
    uint16_t ModEnStatus = GpioGetInput(EX_MOD_EN);
    uint16_t AimEnStatus = GpioGetInput(AIM_EN);
    uint16_t LaserEnStatus = GpioGetInput(LASER_EN);
    uint16_t SafeInterLOck = GpioGetInput(SAFE_LOCK);
    
    uint16_t ModEn         = GpioGetOutput(MOD_EN);
    uint16_t ExAdEn        = GpioGetOutput(EX_AD_EN);
    
    int Tim4Ccr3 = TIM4->CCR3;
    int Tim4Psc  = TIM4->PSC;
    bool IsPwmStart = (TIM4->CR1 & TIM_CR1_CEN) && (TIM4->CCER & TIM_CCER_CC3E);

    State_t *pxState = &s_xState;
    switch ((int)(pxState->xState)) {
        case 0:
            cliprintf("FSM_STATUS  :  FSM_START\n");
        break;
        
        case 1:
            cliprintf("FSM_STATUS  :  FSM_INIT_MPWR\n");
        break;
    
        case 2:
            cliprintf("FSM_STATUS  :  FSM_READY\n");
        break;
        
        case 3:
            cliprintf("FSM_STATUS  :  FSM_IDLE\n");
        break;
        
        case 4:
            cliprintf("FSM_STATUS  :  FSM_LASERs_INIT\n");
        break;
        
        case 5:
            cliprintf("FSM_STATUS  :  FSM_LASERs_RUN\n");
        break;
        
        case 6:
            cliprintf("FSM_STATUS  :  FSM_LASERs_DONE\n");
        break;
        
        case 7:
            cliprintf("FSM_STATUS  :  FSM_LASERm_INIT\n");
        break;
        
        case 8:
            cliprintf("FSM_STATUS  :  FSM_LASERm_RUN\n");
        break;
        
        case 9:
            cliprintf("FSM_STATUS  :  FSM_LASERm_DONE\n");
        break;
        
        case 10:
            cliprintf("FSM_STATUS  :  FSM_INFRARED_INIT\n");
        break;
        
        case 11:
            cliprintf("FSM_STATUS  :  FSM_INFRARED_RUN\n");
        break;
        
        case 12:
            cliprintf("FSM_STATUS  :  FSM_INFRARED_DONE\n");
        break;
    }
    cliprintf("\n");
    
    cliprintf("Status: \n");
    cliprintf("LaserEn          :   %d\n", LaserEnStatus == 0 ? 1 : 0);
    cliprintf("ExModEn          :   %d\n", ModEnStatus == 0 ? 1 : 0);
    cliprintf("AimEn            :   %d\n", AimEnStatus == 0 ? 1 : 0);
    
    cliprintf("ModEn            :   %d\n", ModEn == 0 ? 0 : 1);
    cliprintf("ExAdEn           :   %d\n", ExAdEn == 0 ? 0 : 1);
    
    cliprintf("IsPwmStart       :   %d\n", IsPwmStart);
    cliprintf("Tim4Ccr3         :   %d\n", Tim4Ccr3);
    cliprintf("Tim4Psc          :   %d\n", Tim4Psc);
    cliprintf("\n");
    
    cliprintf("Warnning:\n");
    cliprintf("SafeINterLOck      : %d\n", SafeInterLOck == 0 ? 0 : 1);
    cliprintf("STATUS_MPWR        : %d\n", th_SysStatus.STATUS_MPWR);
    cliprintf("ALARM_EMCY_STOP    : %d\n", th_SysStatus.ALARM_EMCY_STOP);
    cliprintf("ALARM_WATER_CHILLER: %d\n", th_SysStatus.ALARM_WATER_CHILLER);
    cliprintf("ALARM_WATER_PRESS  : %d\n", th_SysStatus.ALARM_WATER_PRESS);
    cliprintf("ALARM_QBH          : %d\n", th_SysStatus.ALARM_QBH);
    cliprintf("ALARM_OT           : %d\n", th_SysStatus.ALARM_OT);
    cliprintf("WARN_OT            : %d\n", th_SysStatus.WARN_OT);
    cliprintf("ALARM_PD           : %d\n", th_SysStatus.ALARM_PD);
    cliprintf("WARN_PD            : %d\n", th_SysStatus.WARN_PD);
    cliprintf("ALARM_WET          : %d\n", th_SysStatus.ALARM_WET);
    cliprintf("WORK_INFRARED      : %d\n", th_SysStatus.WORK_INFRARED);
    cliprintf("WORK_LASER         : %d\n", th_SysStatus.WORK_LASER);
    cliprintf("RUN                : %d\n", th_SysStatus.RUN);
    
}
CLI_CMD_EXPORT(sys_diag, show sys diag, prvCliCmdSysDiag)
