/*
    Data.c

    Implementation File for App Data Module
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

/* Includes */
#include "Include.h"

/* Debug config */
#if DATA_DEBUG
    #undef TRACE
    #define TRACE(...)  DebugPrintf(__VA_ARGS__)
#else
    #undef TRACE
    #define TRACE(...)
#endif /* DATA_DEBUG */
#if DATA_ASSERT
    #undef ASSERT
    #define ASSERT(a)   while(!(a)){DebugPrintf("ASSERT failed: %s %d\n", __FILE__, __LINE__);}
#else
    #undef ASSERT
    #define ASSERT(...)
#endif /* DATA_ASSERT */

/* Forward declaration */
static Bool_t prvChkCrc(uint8_t *pucData, uint16_t usLength);
static uint8_t prvCalcCrc(uint8_t *pucData, uint16_t usLength);

/* Global variables */
Data_t g_xData;
uint16_t g_usMaxCurAd;

/* Functions */
Status_t AppDataInit(void)
{
    TRACE("AppDataInit\r\n");
    DataLoad(&g_xData);
    th_MaxCurAd = CUR_TO_ADC(th_MaxCur * 0.1);
    return STATUS_OK;
}

Status_t AppDataTerm(void)
{
    /* Do nothing */
    return STATUS_OK;
}

Status_t DataLoad(OUT Data_t *pxData)
{
    Data_t xData;
    
    /* Read from page1 */
    MemFlashRead(FLASH_SAVE_PAGE1, sizeof(Data_t), (uint8_t*)&xData);
    TRACE("DataLoad = %d\r\n",(0x00 == xData.ucHead));
    if ((FLASH_DATA_HEAD == xData.ucHead) && (prvChkCrc((uint8_t*)&xData, sizeof(xData)))) {
        if (pxData) {
            *pxData = xData;
        }
        
        return STATUS_OK;
    }

    /* Read from page2 */
    MemFlashRead(FLASH_SAVE_PAGE2, sizeof(Data_t), (uint8_t*)&xData);
    if ((FLASH_DATA_HEAD == xData.ucHead) && (prvChkCrc((uint8_t*)&xData, sizeof(xData)))) {
        if (pxData) {
            *pxData = xData;
        }
        return STATUS_OK;
    }

    /* Init FLASH data */
    Data_t xDataInit = APP_DATA_INIT;
    xDataInit.ucCrc = prvCalcCrc((uint8_t*)&xDataInit, sizeof(Data_t) - 1);
    MemFlashWrite(FLASH_SAVE_PAGE1, sizeof(Data_t), (uint8_t*)&xDataInit);
    MemFlashWrite(FLASH_SAVE_PAGE2, sizeof(Data_t), (uint8_t*)&xDataInit);
    if (pxData) {
        *pxData = xDataInit;
    }

    return STATUS_OK;
}

Status_t DataSave(IN Data_t *pxData)
{
    
    /* Init FLASH data */
    Data_t xData;
    pxData->ucHead = FLASH_DATA_HEAD;
    memcpy(&xData, pxData, sizeof(Data_t));
    xData.ucCrc = prvCalcCrc((uint8_t*)&xData, sizeof(Data_t) - 1);
    MemFlashWrite(FLASH_SAVE_PAGE1, sizeof(Data_t), (uint8_t*)&xData);
    MemFlashWrite(FLASH_SAVE_PAGE2, sizeof(Data_t), (uint8_t*)&xData);
    
    return STATUS_OK;
}

Status_t DataSaveDirect(void)
{
    DataSave(&g_xData);
    return STATUS_OK;
}

static Bool_t prvChkCrc(uint8_t *pucData, uint16_t usLength)
{
    /* CRC */
    uint8_t tmp = prvCalcCrc(pucData, usLength - 1);
    return (pucData[usLength - 1] == tmp) ? TRUE : FALSE;
}

static uint8_t prvCalcCrc(uint8_t *pucData, uint16_t usLength) 
{
    /* REFERENCE: https://www.devcoons.com/crc8/ */
    /* CRC-8/MAXIM */
    
    uint8_t ucCrc = 0x00;
    uint8_t ucTmp, ucSum;
    
    for(uint16_t n = 0; n < usLength; n++) {
        ucTmp = pucData[n];
        for (uint8_t m = 0; m < 8; m++) {
            ucSum = (ucCrc ^ ucTmp) & 0x01;
            ucCrc >>= 1;
            if (ucSum) {
                ucCrc ^= 0x8C;
            }
            ucTmp >>= 1;
        }
    }
    
    return ucCrc;
}

static void prvCliCmdCfgShow(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    Time_t *pxTm = localtime(&(th_TrialStTime));
    if (th_VerChk) {
        cliprintf("Professional version\n");
    }
    else {
        cliprintf("Trial version\n");
    }
    
    cliprintf("PwrId    : %s\n", th_PwrId);
    cliprintf("OrderNum : %d\n", th_OrderNum);
    cliprintf("OtWarn   : %.1f ¡æ [%d]\n", AdcToTemp(th_OtWarnTh) * 0.1, th_OtWarnTh);
    cliprintf("OtCut    : %.1f ¡æ [%d]\n", AdcToTemp(th_OtCutTh) * 0.1, th_OtCutTh);
    cliprintf("MaxCur   : %.1f A [%d]\n", th_MaxCur * 0.1, th_MaxCur);
    cliprintf("WorkCur  : %.1f A [%d]\n", th_WorkCur * 0.1, th_WorkCur);
    cliprintf("CompRate : %d¡ë [%d]\n", th_CompRate, th_CompRate);
    cliprintf("PdWarnL1 : %d mV [%d]\n", th_PdWarnL1 * 3300 / 4096, th_PdWarnL1);
    cliprintf("PdWarnL2 : %d mV [%d]\n", th_PdWarnL2 * 3300 / 4096, th_PdWarnL2);
    cliprintf("AdVolPara: %.3f [%d]\n", th_AdVolPara * 0.001, th_AdVolPara);
    cliprintf("PwrType  : %d [%d]\n", th_PwrType, th_PwrType);
    cliprintf("TempNum  : %d [%d]\n", th_TempNum, th_TempNum);
    cliprintf("SysDebug : %d [%d]\n", th_SysDebug, th_SysDebug);
    cliprintf("TrialEn  : %d [%d]\n", th_TrialEn, th_TrialEn);
    cliprintf("Trial    : %d [%d]\n", th_Trial, th_Trial);
    cliprintf("TrialDays: %d [%d]\n", th_TrialDays, th_TrialDays);
    cliprintf("TrialTime: %d %s",  th_TrialStTime, asctime(pxTm));
    cliprintf("ModEn    : \n");
    cliprintf("         : MANUAL        - %d\n", th_ModEn.MANUAL);
    cliprintf("         : QBH           - %d\n", th_ModEn.QBH);
    cliprintf("         : PD            - %d\n", th_ModEn.PD);
    cliprintf("         : TEMP1         - %d\n", th_ModEn.TEMP1);
    cliprintf("         : TEMP2         - %d\n", th_ModEn.TEMP2);
    cliprintf("         : CHAN1_CUR     - %d\n", th_ModEn.CHAN1_CUR);
    cliprintf("         : CHAN2_CUR     - %d\n", th_ModEn.CHAN2_CUR);
    cliprintf("         : CHAN3_CUR     - %d\n", th_ModEn.CHAN3_CUR);
    cliprintf("         : WATER_PRESS   - %d\n", th_ModEn.WATER_PRESS);
    cliprintf("         : WATER_CHILLER - %d\n", th_ModEn.WATER_CHILLER);
    cliprintf("         : CHAN_CTRL     - %d\n", th_ModEn.CHAN_CTRL);
    cliprintf("Dhcp     : %d\n", th_Dhcp);
    cliprintf("DhcpCnt  : %d\n", th_DhcpTimeout);
    cliprintf("Ip       : %s\n", ipaddr_ntoa((const ip_addr_t*)&(th_LocalIp)));
    cliprintf("NetMask  : %s\n", ipaddr_ntoa((const ip_addr_t*)&(th_LocalNetMask)));
    cliprintf("GwAddr   : %s\n", ipaddr_ntoa((const ip_addr_t*)&(th_LocalGwAddr)));
    cliprintf("PWMDuty  : %d\n", th_PwmDuty);
    cliprintf("CtrlMode : %d\n", th_CtrlMode);

}
CLI_CMD_EXPORT(cfg_show, show config parameters, prvCliCmdCfgShow)

static void prvCliCmdCfgSetOtW(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    if (argc != 2) {
        cliprintf("cfg_set_ot_w AD_VALUE\n");
        return;
    }
    
    th_OtWarnTh = (uint16_t)atoi(argv[1]);
    DataSaveDirect();
    cliprintf("ok, recheck the config by cfg_show command\n");
}
CLI_CMD_EXPORT(cfg_set_ot_w, set over temperature warn threshold, prvCliCmdCfgSetOtW)

static void prvCliCmdCfgSetOtC(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    if (argc != 2) {
        cliprintf("cfg_set_ot_c AD_VALUE\n");
        return;
    }
    
    th_OtCutTh = (uint16_t)atoi(argv[1]);
    DataSaveDirect();
    cliprintf("ok, recheck the config by cfg_show command\n");
}
CLI_CMD_EXPORT(cfg_set_ot_c, set over temperature cut threshold, prvCliCmdCfgSetOtC)

static void prvCliCmdCfgSetMaxCur(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    if (argc != 2) {
        cliprintf("cfg_set_max_cur CUR\n");
        return;
    }
    
    th_MaxCur = (uint32_t)(atof(argv[1]) * 10);
    th_MaxCurAd = CUR_TO_ADC(th_MaxCur * 0.1);
    DataSaveDirect();
    cliprintf("ok, recheck the config by cfg_show command\n");
}
CLI_CMD_EXPORT(cfg_set_max_cur, set maximum current, prvCliCmdCfgSetMaxCur)

static void prvCliCmdCfgSetWorkCur(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    if (argc != 2) {
        cliprintf("cfg_set_work_cur CUR\n");
        return;
    }
    
    th_WorkCur = (uint32_t)(atof(argv[1]) * 10);
    DataSaveDirect();
    cliprintf("ok, recheck the config by cfg_show command\n");
}
CLI_CMD_EXPORT(cfg_set_work_cur, set work current, prvCliCmdCfgSetWorkCur)

static void prvCliCmdCfgSetCompRate(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    if (argc != 2) {
        cliprintf("cfg_set_comp_rate COMP_RATE\n");
        return;
    }
    
    th_CompRate = (int32_t)(atoi(argv[1]));
    DataSaveDirect();
    cliprintf("ok, recheck the config by cfg_show command\n");
    
}
CLI_CMD_EXPORT(cfg_set_comp_rate, set compensation rate, prvCliCmdCfgSetCompRate)

static void prvCliCmdCfgSetPdWarnL1(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    if (argc != 2) {
        cliprintf("cfg_set_pd_warn_l1 VOL_IN_MV\n");
        return;
    }
    
    th_PdWarnL1 = (uint16_t)(atoi(argv[1]) * 4096 / 3300);
    DataSaveDirect();
    cliprintf("ok, recheck the config by cfg_show command\n");
    
}
CLI_CMD_EXPORT(cfg_set_pd_warn_l1, set pd warn level 1 threshold, prvCliCmdCfgSetPdWarnL1)

static void prvCliCmdCfgSetPdWarnL2(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    if (argc != 2) {
        cliprintf("cfg_set_pd_warn_l2 VOL_IN_MV\n");
        return;
    }
    
    th_PdWarnL2 = (uint16_t)(atoi(argv[1]) * 4096 / 3300);
    DataSaveDirect();
    cliprintf("ok, recheck the config by cfg_show command\n");
    
}
CLI_CMD_EXPORT(cfg_set_pd_warn_l2, set pd warn level 2 threshold, prvCliCmdCfgSetPdWarnL2)

static void prvCliCmdCfgSetAdcVolPara(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    if (argc != 2) {
        cliprintf("cfg_set_adc_vol_para PARA\n");
        return;
    }
    
    float fPara = atof(argv[1]);
    th_AdVolPara = (uint32_t)(fPara * 1000);
    DataSaveDirect();
    cliprintf("ok, recheck the config by cfg_show command\n");
}
CLI_CMD_EXPORT(cfg_set_adc_vol_para, set adc voltage parameter, prvCliCmdCfgSetAdcVolPara)

static void prvCliCmdCfgSetPwrType(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    if (argc != 2) {
        cliprintf("cfg_set_pwr_type TYPE\n");
        return;
    }
    
    int lType = atoi(argv[1]);
    if ((lType != 1) && (lType != 2)) {
        cliprintf("wrong type, 1 for 9.5KW, 2 for 18KW\n");
        return;
    }
    
    th_PwrType = (uint8_t)lType;
    DataSaveDirect();
    cliprintf("ok, recheck the config by cfg_show command\n");
}
CLI_CMD_EXPORT(cfg_set_pwr_type, set power type, prvCliCmdCfgSetPwrType)

static void prvCliCmdCfgSetTempChanNum(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    if (argc != 2) {
        cliprintf("cfg_set_temp_chan_num NUM\n");
        return;
    }
    
    int lNum = atoi(argv[1]);
    if ((lNum < 1) || (lNum > 12)) {
        cliprintf("wrong number, 1 ~ 12\n");
        return;
    }
    
    th_TempNum = (uint8_t)lNum;
    DataSaveDirect();
    cliprintf("ok, recheck the config by cfg_show command\n");
}
CLI_CMD_EXPORT(cfg_set_temp_chan_num, set temperature channal number, prvCliCmdCfgSetTempChanNum)

static void prvCliCmdCfgSetTrialInfo(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    if (argc != 3) {
        cliprintf("cfg_set_trial_info ENABLE DAYS\n");
        return;
    }
    
    int lEnable = atoi(argv[1]);
    int lDays = atoi(argv[2]);
    
    Time_t xTm = RtcReadTime(RTC_TYPE_DS1338);
    th_Trial = (lEnable > 0) ? 1 : 0;
    th_TrialDays = lDays;
    th_TrialStTime = mktime(&xTm);
    
    DataSaveDirect();
    cliprintf("ok, recheck the config by cfg_show command\n");
}
CLI_CMD_EXPORT(cfg_set_trial_info, set software trial version info, prvCliCmdCfgSetTrialInfo)

static void prvCliCmdCfgSetTrialEn(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    if (argc != 2) {
        cliprintf("cfg_set_trial_info ENABLE DAYS\n");
        return;
    }
    
    uint8_t usEnable = (uint8_t)atoi(argv[1]);
    
    th_TrialEn = usEnable;
    
    DataSaveDirect();
    cliprintf("ok, recheck the config by cfg_show command\n");
}
CLI_CMD_EXPORT(cfg_set_trial_en, set software trial version info, prvCliCmdCfgSetTrialEn)

static void prvCliCmdCfgSetSysDebug(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    if (argc != 2) {
        cliprintf("cfg_set_sys_debug ENABLE\n");
        return;
    }
    
    int lEnable = atoi(argv[1]);
    
    th_SysDebug = (lEnable > 0) ? 1 : 0;
    DataSaveDirect();
    cliprintf("ok, recheck the config by cfg_show command\n");
}
CLI_CMD_EXPORT(cfg_set_sys_debug, set system debug, prvCliCmdCfgSetSysDebug)

static void prvCliCmdCfgSetModEn(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    if (argc != 3) {
        cliprintf("cfg_set_mod_en TYPE ENABLE\n");
        return;
    }
    
    int lType = atoi(argv[1]);
    int lEnable = atoi(argv[2]);
    if (lType > 10) {
        cliprintf("wrong type, 0 ~ 10\n");
        return;
    }
    
    switch (lType) {
    case 0:
        th_ModEn.MANUAL        = lEnable ? 1 : 0;
        cliprintf("set MANUAL %d\n", th_ModEn.MANUAL);
        break;
    case 1:
        th_ModEn.QBH           = lEnable ? 1 : 0;
        cliprintf("set QBH %d\n", th_ModEn.QBH);
        break;
    case 2:
        th_ModEn.PD            = lEnable ? 1 : 0;
        cliprintf("set PD %d\n", th_ModEn.PD);
        break;
    case 3:
        th_ModEn.TEMP1         = lEnable ? 1 : 0;
        cliprintf("set TEMP1 %d\n", th_ModEn.TEMP1);
        break;
    case 4:
        th_ModEn.TEMP2         = lEnable ? 1 : 0;
        cliprintf("set TEMP2 %d\n", th_ModEn.TEMP2);
        break;
    case 5:
        th_ModEn.CHAN1_CUR     = lEnable ? 1 : 0;
        cliprintf("set CHAN1_CUR %d\n", th_ModEn.CHAN1_CUR);
        break;
    case 6:
        th_ModEn.CHAN2_CUR     = lEnable ? 1 : 0;
        cliprintf("set CHAN2_CUR %d\n", th_ModEn.CHAN2_CUR);
        break;
    case 7:
        th_ModEn.CHAN3_CUR     = lEnable ? 1 : 0;
        cliprintf("set CHAN3_CUR %d\n", th_ModEn.CHAN3_CUR);
        break;
    case 8:
        th_ModEn.WATER_PRESS   = lEnable ? 1 : 0;
        cliprintf("set WATER_PRESS %d\n", th_ModEn.WATER_PRESS);
        break;
    case 9:
        th_ModEn.WATER_CHILLER = lEnable ? 1 : 0;
        cliprintf("set WATER_CHILLER %d\n", th_ModEn.WATER_CHILLER);
        break;
    case 10:
        th_ModEn.CHAN_CTRL     = lEnable ? 1 : 0;
        cliprintf("set CHAN_CTRL %d\n", th_ModEn.CHAN_CTRL);
        break;
    }
    
    DataSaveDirect();
    cliprintf("ok, recheck the config by cfg_show command\n");
}
CLI_CMD_EXPORT(cfg_set_mod_en, set module enable, prvCliCmdCfgSetModEn)

static void prvCliCmdCfgSetNet(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    if (argc != 6) {
        cliprintf("cfg_set_net DHCP DHCP_TIMEOUT IP_ADDR SUB_NET_MASK GW_ADDR\n");
        return;
    }
    
    th_Dhcp         = (atoi(argv[1]) > 0) ? 1 : 0;
    th_DhcpTimeout  = atoi(argv[2]);
    th_LocalIp      = inet_addr(argv[3]);
    th_LocalNetMask = inet_addr(argv[4]);
    th_LocalGwAddr  = inet_addr(argv[5]);
    
    DataSaveDirect();
    cliprintf("ok, recheck the config by cfg_show command\n");
}
CLI_CMD_EXPORT(cfg_set_net, set net parameters, prvCliCmdCfgSetNet)

static void prvCliCmdPwmCtrl(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
     if (argc != 2) {
        cliprintf("cfg_set_pwm_duty DUTY\n");
        return;
    }
    
    int duty = 0;
    
    duty = atoi(argv[1]);
    if (duty >= 0 && duty <= 100)
    {
        th_PwmDuty = (uint16_t)duty;
        DataSaveDirect();
        //Set_AimLight_Cur(th_PwmDuty);
        cliprintf("ok, recheck the config by cfg_show command\n");
    }
    else
    {
        cliprintf("\r\n");
        cliprintf("    duty: 0 - 100\n");
    }
}
CLI_CMD_EXPORT(cfg_set_pwm_duty, set pwm duty, prvCliCmdPwmCtrl)

static void prvCliCmdCfgSetPwrId(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    if (argc != 2) {
        cliprintf("cfg_set_pwr_id ENABLE\n");
        return;
    }
    
    strcpy(th_PwrId,argv[1]);
    
    DataSaveDirect();
    cliprintf("ok, recheck the config by cfg_show command\n");
}
CLI_CMD_EXPORT(cfg_set_pwr_id, set pwr id, prvCliCmdCfgSetPwrId)

static void prvCliCmdCfgSetOrderNum(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    if (argc != 2) {
        cliprintf("cfg_set_order_num ORDER_NUM\n");
        return;
    }
    
    th_OrderNum = atoi(argv[1]);
    
    DataSaveDirect();
    cliprintf("ok, recheck the config by cfg_show command\n");
}
CLI_CMD_EXPORT(cfg_set_order_num, set order number, prvCliCmdCfgSetOrderNum)

static void prvCliCmdCfgSetCtrlMode(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    if (argc != 2) {
        cliprintf("cfg_set_ctrl_mode CTRL_MODE(1:DAC; 2:PWM; 3:AD)\n");
        return;
    }
    
    uint16_t mode = atoi(argv[1]);
    
    th_CtrlMode = (mode >= 1 && mode <= 3) ? mode : 1;
    
    DataSaveDirect();
    cliprintf("ok, recheck the config by cfg_show command\n");
}
CLI_CMD_EXPORT(cfg_set_ctrl_mode, set ctrl mode, prvCliCmdCfgSetCtrlMode)

static void Data_test(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    AppDataInit();
    
    DataSaveDirect();
    
}
CLI_CMD_EXPORT(data_test, set order number, Data_test)
