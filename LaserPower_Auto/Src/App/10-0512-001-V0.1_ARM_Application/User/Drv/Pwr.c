/*
    Pwr.c

    Implementation File for Pwr Module
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 10Nov23, Karl Created
    01b, 24Nov23, Karl Added pwr1_enable and pwr2_enable
    01c, 24Nov23, Karl Added pwr_a_set_cur
    01d, 30Nov23, Karl Added PwrIsOk function
    01e, 07Dec23, Karl Added PwrOutput function
    01f, 22Dec23, Karl Fixed pwr_a_set_cur bug
    01g, 27Dec23, Karl Added PwrDataGet
    01h, 03Jan24, Karl Modified ADC_TO_VOL definition
    01i, 08Jan24, Karl Added th_AdVolPara in ADC_TO_VOL definition
    01j, 17Jan24, Karl Added PwrSetVolDef
    01k, 20Jan24, Karl Added PWR_STATUS
*/

/* Includes */
#include "Include.h"

/* Debug config */
#if PWR_DEBUG
    #undef TRACE
    #define TRACE(...)  DebugPrintf(__VA_ARGS__)
#else
    #undef TRACE
    #define TRACE(...)
#endif /* PWR_DEBUG */
#if PWR_DEBUG
    #undef ASSERT
    #define ASSERT(a)   while(!(a)){DebugPrintf("ASSERT failed: %s %d\n", __FILE__, __LINE__);}
#else
    #undef ASSERT
    #define ASSERT(...)
#endif /* PWR_DEBUG */

/* Forward declarations */
static void prvPwrTask(void* pvPara);

/* Local variables */
static Bool_t s_bEnPwr1 = FALSE;
static Bool_t s_bEnPwr2 = FALSE;

/* Functions */
Status_t DrvPwr1Enable(void)
{
#if !PWR1_ENABLE
    (void)s_bEnPwr1;
#endif
    s_bEnPwr1 = TRUE;
    return STATUS_OK;
}

Status_t DrvPwr2Enable(void)
{
    s_bEnPwr2 = TRUE;
    return STATUS_OK;
}

Status_t DrvPwrInit(void)
{
#if PWR1_ENABLE
    if (s_bEnPwr1) {
        Pwr1ProtInit();
    }
#endif /* PWR1_ENABLE */

#if PWR2_ENABLE
    if (s_bEnPwr2) {
        Pwr2ProtInit();
    }
#endif /* PWR2_ENABLE */

    xTaskCreate(prvPwrTask, "tPwr", 256, NULL, tskIDLE_PRIORITY, NULL);

    return STATUS_OK;
}

Status_t DrvPwrTerm(void)
{
    /* Do nothing */
    return STATUS_OK;
}

bool PwrIsOk(void)
{
    /* TODO: PwrIsOk */
    return true;
}

Status_t PwrOutput(uint8_t ucOnOff)
{
#if PWR1_ENABLE
    if (s_bEnPwr1) {
        return Pwr1Output(ucOnOff);
    }
#endif /* PWR1_ENABLE */

#if PWR2_ENABLE
    if (s_bEnPwr2) {
        return Pwr2Output(PWR2_M1_ADDR, ucOnOff);
    }
#endif /* PWR2_ENABLE */
    
    return STATUS_ERR;
}

int32_t PwrDataGet(uint32_t ulPwr2Addr, PwrDataType_t xType)
{
#if PWR1_ENABLE
    if (s_bEnPwr1) {
        return Pwr1DataGet(xType);
    }
#endif /* PWR1_ENABLE */

#if PWR2_ENABLE
    if (s_bEnPwr2) {
        switch (ulPwr2Addr)
        {
            case PWR2_M2_ADDR: return Pwr2DataGet(PWR2_M2_ADDR, xType); break;
            case PWR2_M1_ADDR: return Pwr2DataGet(PWR2_M1_ADDR, xType); break;
            case PWR2_M3_ADDR: return Pwr2DataGet(PWR2_M3_ADDR, xType); break;
        }
        
    }
#endif /* PWR2_ENABLE */
    
    return 0;
}

Status_t PwrSetVolDef(float fV)
{
#if PWR1_ENABLE
    if (s_bEnPwr1) {
        return Pwr1SetVolDef(fV);
    }
#endif /* PWR1_ENABLE */

#if PWR2_ENABLE
    if (s_bEnPwr2) {
        return Pwr2SetVolDef(fV);
    }
#endif /* PWR2_ENABLE */
    
    return 0;
}

static void prvPwrTask(void* pvPara)
{
    while (1) {
    #if PWR1_ENABLE
        if (s_bEnPwr1) {
            Pwr1Update();
        }
    #endif /* PWR1_ENABLE */
    
    #if PWR2_ENABLE
        if (s_bEnPwr2) {
            Pwr2Update();
        }
    #endif /* PWR2_ENABLE */
    
        osDelay(1000);
    }
}

/* Overwrite CanRxNotify function in 'Drv\Can.h/c' */
void CanRxNotify(uint32_t ulPwr2Addr, CanMsgRx_t *pxMsg)
{
#if PWR1_ENABLE
    if (s_bEnPwr1) {
        Pwr1CanRxNotify(pxMsg);
    }
#endif /* PWR1_ENABLE */

#if PWR2_ENABLE
    if (s_bEnPwr2) {
        switch (ulPwr2Addr)
        {
            case PWR2_M2_ADDR: Pwr2CanRxNotify(PWR2_M2_ADDR, pxMsg);break;
            case PWR2_M1_ADDR: Pwr2CanRxNotify(PWR2_M1_ADDR, pxMsg);break;
            case PWR2_M3_ADDR: Pwr2CanRxNotify(PWR2_M3_ADDR, pxMsg);break;
        }
        
    }
#endif /* PWR2_ENABLE */
}

static void prvCliCmdPwrMEnable(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    if (argc != 3) {
        cliprintf("pwr_m_enable DEV ONOFF\n");
        return;
    }
    
    int lDev = atoi(argv[1]);
    int lCtrl = atoi(argv[2]);
    switch (lDev)
    {
        case 1: GpioSetOutput(MPWR1_EN, lCtrl); break;
    }
    
}
CLI_CMD_EXPORT(pwr_m_enable, enable main power, prvCliCmdPwrMEnable)

static void prvCliCmdPwrMStatus(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    cliprintf("    MPWR1_STAT_AC [MCU_AC_OK, DI]: %d\n", GpioGetInput(MPWR1_STAT_AC));
    cliprintf("    MPWR1_STAT_DC [MCU_DC_OK, DI]: %d\n", GpioGetInput(MPWR1_STAT_DC));
    cliprintf("    MPWR1_EN      [MCU_SW_OK, DO]: %d\n", GpioGetOutput(MPWR1_EN));
    cliprintf("\n");
}
CLI_CMD_EXPORT(pwr_m_status, show main power status, prvCliCmdPwrMStatus)

static void prvCliCmdPwrAEnable(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    if (argc != 2) {
        cliprintf("pwr_a_enable ONOFF\n");
        return;
    }

    int lCtrl = atoi(argv[1]);
    GpioSetOutput(APWR1_EN, lCtrl);
    GpioSetOutput(APWR2_EN, lCtrl);
    GpioSetOutput(APWR3_EN, lCtrl);
}
CLI_CMD_EXPORT(pwr_a_enable, enable auxiliary power, prvCliCmdPwrAEnable)

static void prvCliCmdPwrAStatus(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    uint16_t usCtrl1 = DacGet(APWR1_CTRL);
    uint16_t usCtrl2 = DacGet(APWR2_CTRL);
    uint16_t usCtrl3 = DacGet(APWR3_CTRL);
    
    uint16_t usCur1 = AdcGet(AUX1_CS);
    uint16_t usCur2 = AdcGet(AUX2_CS);
    uint16_t usCur3 = AdcGet(AUX3_CS);
    
    uint16_t usVol1 = AdcGet(AUX1_VS);
    uint16_t usVol2 = AdcGet(AUX2_VS);
    uint16_t usVol3 = AdcGet(AUX3_VS);
    
    float fVol1 = GpioGetOutput(APWR1_EN) ? ADC_TO_VOL(usVol1, PWR2_M2_ADDR) : 0;
    float fVol2 = GpioGetOutput(APWR2_EN) ? ADC_TO_VOL(usVol2, PWR2_M1_ADDR) : 0;
    float fVol3 = GpioGetOutput(APWR3_EN) ? ADC_TO_VOL(usVol3, PWR2_M3_ADDR) : 0;
    
    cliprintf("Auxiliary power status:\n");
    
    cliprintf("Auxiliary power - 1\n");
    cliprintf("    APWR1_STAT [MCU_AUX1_ERR, DI ]: %d\n", GpioGetInput(APWR1_STAT));
    cliprintf("    APWR1_EN   [MCU_AUX1_OUT, DO ]: %d\n", GpioGetOutput(APWR1_EN));
    cliprintf("    APWR1_CTRL [DAC1        , DAC]: %.1f A (%04d mV)\n", DAC_TO_CUR(usCtrl1), DAC_TO_MVOL(usCtrl1));
    cliprintf("    APWR1_CUR  [AUX1_CS     , ADC]: %.1f A (%04d mV)\n", ADC_TO_CUR(usCur1), ADC_TO_MVOL(usCur1));
    cliprintf("    APWR1_VOL  [AUX1_VS     , ADC]: %.1f V (%04d mV)\n", fVol1, ADC_TO_MVOL(usVol1));
    
    cliprintf("Auxiliary power - 2\n");
    cliprintf("    APWR2_STAT [MCU_AUX2_ERR, DI ]: %d\n", GpioGetInput(APWR2_STAT));
    cliprintf("    APWR2_EN   [MCU_AUX2_OUT, DO ]: %d\n", GpioGetOutput(APWR2_EN));
    cliprintf("    APWR2_CTRL [DAC2        , DAC]: %.1f A (%04d mV)\n", DAC_TO_CUR(usCtrl2), DAC_TO_MVOL(usCtrl2));
    cliprintf("    APWR2_CUR  [AUX2_CS     , ADC]: %.1f A (%04d mV)\n", ADC_TO_CUR(usCur2), ADC_TO_MVOL(usCur2));
    cliprintf("    APWR2_VOL  [AUX2_VS     , ADC]: %.1f V (%04d mV)\n", fVol2, ADC_TO_MVOL(usVol2));
    
    cliprintf("Auxiliary power - 3\n");
    cliprintf("    APWR3_STAT [MCU_AUX3_ERR, DI ]: %d\n", GpioGetInput(APWR3_STAT));
    cliprintf("    APWR3_EN   [MCU_AUX3_OUT, DO ]: %d\n", GpioGetOutput(APWR3_EN));
    cliprintf("    APWR3_CTRL [DAC3        , DAC]: %.1f A (%04d mV)\n", DAC_TO_CUR(usCtrl3), DAC_TO_MVOL(usCtrl3));
    cliprintf("    APWR3_CUR  [AUX3_CS     , ADC]: %.1f A (%04d mV)\n", ADC_TO_CUR(usCur3), ADC_TO_MVOL(usCur3));
    cliprintf("    APWR3_VOL  [AUX3_VS     , ADC]: %.1f V (%04d mV)\n", fVol3, ADC_TO_MVOL(usVol3));
}
CLI_CMD_EXPORT(pwr_a_status, show auxiliary power status, prvCliCmdPwrAStatus)

static void prvCliCmdPwrASetCur(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    if (argc != 3) {
        cliprintf("pwr_a_set_cur CHAN CUR\n");
        return;
    }
    
    int lChan = atoi(argv[1]);
    float fCur = atoi(argv[2]);
    
    if ((lChan < 1) || (lChan > 3)) {
        cliprintf("wrong channel: 1~3\n");
        return;
    }
    
    DacSet(lChan, CUR_TO_DAC(fCur));
}
CLI_CMD_EXPORT(pwr_a_set_cur, set auxiliary output current, prvCliCmdPwrASetCur)

static void prvCliCmdPwr1Enable(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    if (argc != 2) {
        cliprintf("pwr1_enable ONOFF\n");
        return;
    }
    
    int lEnable = atoi(argv[1]);
    if (lEnable) {
#if PWR1_ENABLE
        Pwr1ProtInit();
#endif
    }
    lEnable ? (s_bEnPwr1 = TRUE) : (s_bEnPwr1 = FALSE);
}
CLI_CMD_EXPORT(pwr1_enable, enable power 1, prvCliCmdPwr1Enable)

static void prvCliCmdPwr2Enable(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    if (argc != 2) {
        cliprintf("pwr2_enable ONOFF\n");
        return;
    }
    
    int lEnable = atoi(argv[1]);
    
    if (lEnable) {
        Pwr2ProtInit();
    }
    
    lEnable ? (s_bEnPwr2 = TRUE) : (s_bEnPwr2 = FALSE);
}
CLI_CMD_EXPORT(pwr2_enable, enable power 2, prvCliCmdPwr2Enable)
