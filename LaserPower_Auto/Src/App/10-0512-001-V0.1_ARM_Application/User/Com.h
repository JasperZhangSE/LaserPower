/*
    Com.h

    Head File for App Com Module
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 22Nov23, Karl Created
    01b, 03Dec23, Karl Optimized prvCliUartPrintf
    01c, 04Dec23, Karl Implemented iCmdQueryInfo, rCmdStatusInfo and rCmdDiagInfo
    01d, 27Dec23, Karl Modified prvSendStatusInfo to force current adc as 0 when it is greater than 3200
    01e, 27Dec23, Karl Added TYPE_MANUAL_CTRL
    01f, 03Jan24, Karl Added usMPwrVol in RCmdStatusInfo_t
    01g, 15Jan24, Karl Added usTemp in RCmdStatusInfo_t
    01h, 17Jan24, Karl Modified prvCmdParaConfig
    01i, 20Jan24, Karl Added range check for TYPE_PWR_COMP
    01j, 20Jan24, Karl Added rCmdSysPara
    01k, 20Jan24, Karl Added usSysStatus and ulPwrStatus in RCmdStatusInfo_t
    01l, 24Jan24, Karl Added iCmdEncrypt
    01m, 24Jan24, Karl Added version check in prvCmdParaConfig and prvCmdModuleCtrl
    01n, 26Jan24, Karl Added usSwInfo in RCmdStatusInfo_t
    01o, 01Mar24, Karl Added RS485 test
*/

#ifndef __COM_H__
#define __COM_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */


/* Functions */
Status_t AppComInit(void);
Status_t AppComTerm(void);

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif /* __COM_H__ */
