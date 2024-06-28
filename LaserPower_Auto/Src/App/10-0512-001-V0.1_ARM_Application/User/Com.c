/*
    Com.c

    Implementation File for App Com Module
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

/* Includes */
#include "Include.h"

/* Debug config */
#if COM_DEBUG
    #undef TRACE
    #define TRACE(...)  DebugPrintf(__VA_ARGS__)
#else
    #undef TRACE
    #define TRACE(...)
#endif /* COM_DEBUG */
#if COM_ASSERT
    #undef ASSERT
    #define ASSERT(a)   while(!(a)){DebugPrintf("ASSERT failed: %s %d\n", __FILE__, __LINE__);}
#else
    #undef ASSERT
    #define ASSERT(...)
#endif /* COM_ASSERT */

/* Local defines */
#define DEV_ID                      1
#define ENABLE_RS485_TEST           1
#define MAX_MSG_SIZE                256 /*64*/
#define GET_SEND_BUFFER()           (s_ucSendBuffer)
#define GET_HEAD_BUFFER()           ((Head_t*)(s_ucSendBuffer))
#define GET_CONT_BUFFER()           (s_ucSendBuffer + sizeof(Head_t))
#define GET_TAIL_BUFFER(pxHead)     ((Tail_t*)(s_ucSendBuffer + pxHead->ucLength - sizeof(Tail_t)))
#define RS485_RD()                  GpioSetOutput(RS485b_EN, 1)
#define RS485_WT()                  GpioSetOutput(RS485b_EN, 0)

#define STR_LENGTH 20
#define NUM_PAIRS 10
#define HEX_PAIR_LENGTH 2
#define DECIMAL_LENGTH 21
#define DECIMAL_CHAR_LENGTH 2

#define SET_COM_SEND_PTL            1    /* 1: Karl版本协议，适用老版控制板; 2: Jasper版本协议，适用新版控制板 */

/* Local types */
#pragma pack(push)
#pragma pack(1)
enum {
    CHAN_COM,
    CHAN_NET
};

#if SET_COM_SEND_PTL == 2
enum {
    iCmdQueryInfo      = 0x01,
    iCmdSysReset       = 0x02,
    iCmdParaConfig     = 0x03,
    iCmdModuleCtrl     = 0x04,
    iCmdUpgradeEnable  = 0x05,
    iCmdCli            = 0x06,
    iCmdEncrypt        = 0x07,
    rCmdReply          = 0x81,
    rCmdStatusInfo     = 0x82,
    rCmdDiagInfo       = 0x83,
    rCmdCli            = 0x84,
    rCmdSysPara        = 0x85,
};

enum {
    TYPE_OT_WARN       = 0x1111,
    TYPE_OT_CUT        = 0x2222,
    TYPE_MAX_CUR       = 0x3333,
    TYPE_WORK_CUR      = 0x4444,
    TYPE_PWR_COMP      = 0x5555,
    TYPE_MPWR_VOL      = 0x6666,
    TYPE_TEMP_NUM      = 0x7777,
    TYPE_PD_VOL        = 0x8888,
    TYPE_MODULE_ENABLE = 0x9999,
};

enum {
    TYPE_EX_CTRL_EN      = 0x1111,
    TYPE_LASER_OFF     = 0x2222,
    TYPE_INFRARED_ON   = 0x3333,
    TYPE_INFRARED_OFF  = 0x4444,
    TYPE_MANUAL_CTRL   = 0x5555,
};

typedef struct {
    uint16_t usStart;
    uint8_t  ucLength;
    uint8_t  ucCmd;
    uint8_t  ucSrcAddr;
    uint8_t  ucDstAddr;
}Head_t;

typedef struct {
    uint8_t  ucCheck;
    uint16_t usEnd;
}Tail_t;

typedef struct {
    uint8_t ucRespCmd;
}ICmdQueryInfo_t;

typedef struct {
    uint32_t ulMark;
}ICmdSysReset_t;

typedef struct {
    uint16_t usType;
    uint32_t ulPara1;
    uint32_t ulPara2;
}ICmdParaConfig_t;

typedef struct {
    uint16_t usType;
    uint32_t ulPara1;
    uint32_t ulPara2;
}ICmdModuleCtrl_t;

typedef struct {
    uint32_t ulMark;
    uint8_t  ucEnable;
}ICmdUpgradeEnable_t;

typedef struct {
    char cCode[20];
}ICmdEncrypt_t;
char decimalArray[NUM_PAIRS * DECIMAL_CHAR_LENGTH];

typedef struct {
    uint8_t ucStatus;
}RCmdReply_t;

typedef struct {
    uint8_t  ucFsm;
    struct {
        uint16_t _MPWR_T_STAT_AC:1;
        uint16_t _MPWR_T_STAT_DC:1;
        uint16_t _MPWR_M_STAT_AC:1;
        uint16_t _MPWR_M_STAT_DC:1;
        uint16_t _MPWR_B_STAT_AC:1;
        uint16_t _MPWR_B_STAT_DC:1;
        
        uint16_t _APWR1_STAT:1;
        uint16_t _APWR2_STAT:1;
        uint16_t _APWR3_STAT:1;
        uint16_t _QBH_ON:1;
        uint16_t _EX_CTRL_EN:1;
        uint16_t _WATER_PRESS:1;
        uint16_t _WATER_CHILLER:1;
    }xDi;
    struct {
        uint16_t _MPWR_T_EN:1;
        uint16_t _MPWR_M_EN:1;
        uint16_t _MPWR_B_EN:1;
        
        uint16_t _APWR1_EN:1;
        uint16_t _APWR2_EN:1;
        uint16_t _APWR3_EN:1;
        uint16_t _LED_CTRL:1;
        uint16_t _LED1:1;
        uint16_t _LED2:1;
        uint16_t _LED3:1;
        uint16_t _LED4:1;
        uint16_t _LED5:1;
        uint16_t _LED6:1;
    }xDo;
    uint16_t usTempH;
    uint16_t usAdc[7];
    uint16_t usDac[3];
    uint16_t usTMPwrVol;
    uint16_t usMMPwrVol;
    uint16_t usBMPwrVol;
    uint16_t usTemp[3][8];
    uint16_t usPd[6];
    uint16_t usSysStatus;
    
    uint32_t ulTPwrStatus;
    uint32_t ulMPwrStatus;
    uint32_t ulBPwrStatus;
    
    uint16_t usSwInfo;
}RCmdStatusInfo_t;

typedef struct {
    uint32_t ulSwVer;
    uint32_t ulRunTime;
}RCmdDiagInfo_t;

typedef struct {
    int16_t  sOtWarnTh;
    int16_t  sOtCutTh;
    uint32_t ulMaxCur;
    uint32_t ulWorkCur;
    int32_t  lCompRate;
    uint16_t usPdWarnL1;
    uint16_t usPdWarnL2;
    uint32_t ulAdVolPara;
    uint8_t  ucPrType;
    uint8_t  ucTempNum;
    uint16_t usModEn;
}RCmdSysPara_t;

enum {
    REPLY_OK,
    REPLY_ERR
};
#pragma pack(pop)

/* Forward declaration */
static void     prvComTask         (void* pvPara);
static void     prvNetTask         (void* pvPara);
static void     prvCmdQueryInfo    (uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo);
static void     prvCmdSysReset     (uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo);
static void     prvCmdParaConfig   (uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo);
static void     prvCmdModuleCtrl   (uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo);
static void     prvCmdUpgradeEnable(uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo);
static void     prvCmdCli          (uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo);
static void     prvCmdEncrypt      (uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo);
static void     prvSend            (uint8_t *pucData, uint8_t ucDataSize, uint8_t ucCmd, void *pvInfo);
static void     prvSendReply       (uint8_t ucStatus, void *pvInfo);
static void     prvSendStatusInfo  (void *pvInfo);
static void     prvSendDiagInfo    (void *pvInfo);
static void     prvSendSysPara     (void *pvInfo);
static Status_t prvProtPktProc     (const void *pvHead, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo);
static Bool_t   prvProtPktChk      (const void *pvStart, uint32_t ulLength);
static Status_t prvUartRecv        (uint8_t *pucBuf, uint16_t usLength, void* pvIsrPara);
static void     prvCliUartPrintf   (const char* cFormat, ...);
#if ENABLE_RS485_TEST
static Status_t prvUartRecvRs485   (uint8_t *pucBuf, uint16_t usLength, void* pvIsrPara);
#endif /* ENABLE_RS485_TEST */

/* Local variables */
static RbufHandle_t s_xRbuf = NULL;
static UartHandle_t s_xUart = NULL;
static ProtHandle_t s_xProt = NULL;
static uint8_t      s_ucBuffer[256];
static uint8_t      s_ucSendBuffer[MAX_MSG_SIZE];
static SOCKET       s_xSvrSock = -1;
static SOCKET       s_xCliSock = -1;
#if ENABLE_RS485_TEST
static UartHandle_t s_xUartRs485 = NULL;
static Bool_t       s_bUartRs485Recv = FALSE;
#endif /* ENABLE_RS485_TEST */

/* Functions */
Status_t AppComInit(void)
{
    
    uint8_t ucHeadMark[] = {0x7E, 0x7E};
    uint8_t ucTailMark[] = {0x0A, 0x0D};
    
    s_xRbuf = NULL;
    s_xUart = NULL;
    s_xProt = NULL;
    
    RbufInit();
    s_xRbuf = RbufCreate();
    RbufConfig(s_xRbuf, s_ucBuffer, sizeof(s_ucBuffer), 3/*Rbuf msg queue size*/, 5/*Rbuf msg queue wait ms*/);
    
    ProtInit();
    s_xProt = ProtCreate();
    ProtConfigHead(s_xProt, ucHeadMark, sizeof(ucHeadMark), sizeof(Head_t));
    ProtConfigTail(s_xProt, ucTailMark, sizeof(ucTailMark), sizeof(Tail_t));
    ProtConfigMisc(s_xProt, MAX_MSG_SIZE, 0/* Msg length offset */, PROT_LENGTH_UINT8, 2/* Head mark size */);
    ProtConfigCb(s_xProt, prvProtPktProc, prvProtPktChk);
    ProtConfig(s_xProt);
    
    UartInit();
    s_xUart = UartCreate();
    UartConfigCb(s_xUart, prvUartRecv, UartIsrCb, UartDmaRxIsrCb, UartDmaTxIsrCb, NULL);
    UartConfigRxDma(s_xUart, DMA1_Channel5, DMA1_Channel5_IRQn);
    UartConfigTxDma(s_xUart, DMA1_Channel4, DMA1_Channel4_IRQn);
    UartConfigCom(s_xUart, USART1, 115200, USART1_IRQn);
#if ENABLE_RS485_TEST
    s_xUartRs485 = UartCreate();
    UartConfigCb(s_xUartRs485, prvUartRecvRs485, UartIsrCb, NULL, NULL, NULL);
    UartConfigCom(s_xUartRs485, UART5, 115200, UART5_IRQn);
    RS485_RD();
#endif /* ENABLE_RS485_TEST */
    
    Bool_t   bDhcp          = th_Dhcp;
    uint32_t ulDhcpTimeout  = th_DhcpTimeout;
    uint32_t ulLocalIp      = th_LocalIp;
    uint32_t ulLocalNetMask = th_LocalNetMask;
    uint32_t ulLocalGwAddr  = th_LocalGwAddr;
    char     *cServerName   = "192.168.1.11"; /* No use here! */
    uint16_t usServerPort   = 6000; /* No use here! */
    void     *pxNetIf       = NULL;
    pxNetIf = LwIPGetNetIf();
    NetConfigEth(bDhcp, ulDhcpTimeout, ulLocalIp, ulLocalNetMask, ulLocalGwAddr, cServerName, usServerPort, pxNetIf);
    //DrvNetInit();
    
    xTaskCreate(prvComTask, "tCom", 256, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(prvNetTask, "tNet", 256, NULL, tskIDLE_PRIORITY, NULL);
    
    return STATUS_OK;
}

Status_t AppComTerm(void)
{
    /* Do nothing */
    return STATUS_OK;
}

static void prvComTask(void* pvPara)
{
    while (1) {
        static uint16_t s_usRecvIndex = 0;
        static uint8_t s_ucProcBuf[MAX_MSG_SIZE];
        static uint8_t s_ucRecvBuf[MAX_MSG_SIZE];
        uint16_t usRecvd = (uint16_t)RbufRead(s_xRbuf, s_ucRecvBuf, MAX_MSG_SIZE);
        if (usRecvd) {
            ProtProc(s_xProt, s_ucRecvBuf, usRecvd, &s_usRecvIndex, s_ucProcBuf, (void*)CHAN_COM);
        }
    }
}

static void prvNetTask(void* pvPara)
{
    struct sockaddr_in xSvrAddr, xCliAddr;

    /* Socket */
    s_xSvrSock = socket(AF_INET, SOCK_STREAM, 0);
    if (s_xSvrSock == -1) {
        TRACE("socket failed\n");
    }

    /* Bind */
    xSvrAddr.sin_family      = AF_INET;
    xSvrAddr.sin_addr.s_addr = INADDR_ANY;
    xSvrAddr.sin_port        = htons(6000);
    if (bind(s_xSvrSock, (struct sockaddr *)&xSvrAddr, sizeof(xSvrAddr)) == -1) {
        TRACE("bind failed\n");
    }

    /* Listen */
    if (listen(s_xSvrSock, 5) == -1) {
        TRACE("listen failed\n");
    }

    while (1) {
        socklen_t xCliAddrLength = sizeof(xCliAddr);

        /* Accept */
        s_xCliSock = accept(s_xSvrSock, (struct sockaddr *)&xCliAddr, &xCliAddrLength);
        if (s_xCliSock == -1) {
            TRACE("accept failed");
        }
        else {
            TRACE("Accepted %s %d\n", inet_ntoa(xCliAddr.sin_addr), ntohs(xCliAddr.sin_port));
        }

       while (1) {
            static uint16_t s_usRecvIndex = 0;
            static uint8_t s_ucProcBuf[MAX_MSG_SIZE];
            static uint8_t s_ucRecvBuf[MAX_MSG_SIZE];
            
            /* Receive */
            ssize_t lRead = recv(s_xCliSock, s_ucRecvBuf, MAX_MSG_SIZE, 0);
            if (lRead <= 0) {
                close(s_xCliSock);
                s_xCliSock = -1;
                break;
            }
            
            /* Process */
            ProtProc(s_xProt, s_ucRecvBuf, (uint16_t)lRead, &s_usRecvIndex, s_ucProcBuf, (void*)CHAN_NET);
       }
    }
}

static void prvCmdQueryInfo(uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo)
{
    TRACE("iCmdQueryInfo\n");
    
    if (ulLength != sizeof(ICmdQueryInfo_t)) {
        TRACE("    Wrong length\n");
        return;
    }
    
    const ICmdQueryInfo_t *pxData = (const ICmdQueryInfo_t*)pucCont;
    
    switch (pxData->ucRespCmd) {
    case rCmdStatusInfo:
        prvSendStatusInfo(pvInfo);
        break;
    case rCmdDiagInfo:
        prvSendDiagInfo(pvInfo);
        break;
    case rCmdSysPara:
        prvSendSysPara(pvInfo);
        break;
    default:
        prvSendReply(REPLY_ERR, pvInfo);
        break;
    }
}

static void prvCmdSysReset(uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo)
{
    TRACE("iCmdSysReset\n");
    
    if (ulLength != sizeof(ICmdSysReset_t)) {
        TRACE("    Wrong length\n");
        return;
    }
    
    const ICmdSysReset_t *pxData = (const ICmdSysReset_t*)pucCont;
    
    if (pxData->ulMark == 0x1234ABCD) {
        prvSendReply(REPLY_OK, pvInfo);
        NVIC_SystemReset();
    }
    else {
        prvSendReply(REPLY_ERR, pvInfo);
    }
}

static void prvCmdParaConfig(uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo)
{
    TRACE("iCmdParaConfig\n");
    
    if (!th_VerChk) {
        prvSendReply(REPLY_ERR, pvInfo);
        return;
    }
    
    if (ulLength != sizeof(ICmdParaConfig_t)) {
        TRACE("    Wrong length\n");
        return;
    }
    
    const ICmdParaConfig_t *pxData = (const ICmdParaConfig_t*)pucCont;
    
    Status_t xRet = STATUS_ERR;
    switch (pxData->usType) {
    case TYPE_OT_WARN:
        if (pxData->ulPara1 >= th_OtCutTh) {
            xRet = STATUS_OK;
            th_OtWarnTh = pxData->ulPara1;
            DataSaveDirect();
        }
        break;
    case TYPE_OT_CUT:
        if (pxData->ulPara1 <= th_OtWarnTh) {
            xRet = STATUS_OK;
            th_OtCutTh = pxData->ulPara1;
            DataSaveDirect();
        }
        break;
    case TYPE_MAX_CUR:
        if (pxData->ulPara1 >= th_WorkCur) {
            xRet = STATUS_OK;
            th_MaxCur = pxData->ulPara1;
            th_MaxCurAd = CUR_TO_ADC(th_MaxCur * 0.1);
            DataSaveDirect();
        }
        break;
    case TYPE_WORK_CUR:
        if (pxData->ulPara1 <= th_MaxCur) {
            xRet = STATUS_OK;
            th_WorkCur = pxData->ulPara1;
            DataSaveDirect();
        }
        break;
    case TYPE_PWR_COMP:
        if (pxData->ulPara1 <= 100) {
            xRet = STATUS_OK;
            th_CompRate = (int32_t)pxData->ulPara1;
            DataSaveDirect();
        }
        break;
    case TYPE_MPWR_VOL:
        PwrSetVolDef(pxData->ulPara1 * 0.1);
        break;
    case TYPE_TEMP_NUM:
        if ((pxData->ulPara1 >= 1) && (pxData->ulPara1 <= 10)) {
            xRet = STATUS_OK;
            th_TempNum = (uint8_t)pxData->ulPara1;
            DataSaveDirect();
        }
        break;
    case TYPE_PD_VOL:
        th_PdWarnL1 = (uint16_t)(pxData->ulPara1/*mV*/ * 4096 / 3300);
        DataSaveDirect();
        break;
    case TYPE_MODULE_ENABLE:
        xRet = STATUS_OK;
        switch (pxData->ulPara1) {
        case 0:  th_ModEn.MANUAL        = pxData->ulPara2 ? 1 : 0; break;
        case 1:  th_ModEn.QBH           = pxData->ulPara2 ? 1 : 0; break;
        case 2:  th_ModEn.PD            = pxData->ulPara2 ? 1 : 0; break;
        case 3:  th_ModEn.TEMP1         = pxData->ulPara2 ? 1 : 0; break;
        case 4:  th_ModEn.TEMP2         = pxData->ulPara2 ? 1 : 0; break;
        case 5:  th_ModEn.CHAN1_CUR     = pxData->ulPara2 ? 1 : 0; break;
        case 6:  th_ModEn.CHAN2_CUR     = pxData->ulPara2 ? 1 : 0; break;
        case 7:  th_ModEn.CHAN3_CUR     = pxData->ulPara2 ? 1 : 0; break;
        case 8:  th_ModEn.WATER_PRESS   = pxData->ulPara2 ? 1 : 0; break;
        case 9:  th_ModEn.WATER_CHILLER = pxData->ulPara2 ? 1 : 0; break;
        case 10: th_ModEn.CHAN_CTRL     = pxData->ulPara2 ? 1 : 0; break;
        }
        DataSaveDirect();
        break;
    default:
        break;
    }
    
    if (xRet == STATUS_OK) {
        prvSendReply(REPLY_OK, pvInfo);
    }
    else {
        prvSendReply(REPLY_ERR, pvInfo);
    }
}

static void prvCmdModuleCtrl(uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo)
{
    TRACE("iCmdModuleCtrl\n");
    
    if (!th_VerChk) {
        prvSendReply(REPLY_ERR, pvInfo);
        return;
    }
    
    if (ulLength != sizeof(ICmdModuleCtrl_t)) {
        TRACE("    Wrong length\n");
        return;
    }
    
    const ICmdModuleCtrl_t *pxData = (const ICmdModuleCtrl_t*)pucCont;
    
    Status_t xRet = STATUS_ERR;
    switch (pxData->usType) {
    case TYPE_EX_CTRL_EN:
        xRet = LaserOn(pxData->ulPara1, pxData->ulPara2);
        break;
    case TYPE_LASER_OFF:
        xRet = LaserOff(pxData->ulPara2);
        break;
    case TYPE_INFRARED_ON:
        xRet = InfraredOn();
        break;
    case TYPE_INFRARED_OFF:
        xRet = InfraredOff();
        break;
    case TYPE_MANUAL_CTRL:
        xRet = EnableManualCtrl((uint8_t)pxData->ulPara1);
    default:
        break;
    }
    
    if (xRet == STATUS_OK) {
        prvSendReply(REPLY_OK, pvInfo);
    }
    else {
        prvSendReply(REPLY_ERR, pvInfo);
    }
}

static void prvCmdUpgradeEnable(uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo)
{
    TRACE("iCmdUpgradeEnable\n");
    
    if (ulLength != sizeof(ICmdUpgradeEnable_t)) {
        TRACE("    Wrong length\n");
        return;
    }
    
    const ICmdUpgradeEnable_t *pxData = (const ICmdUpgradeEnable_t*)pucCont;
    
    if ((pxData->ulMark == 0x1234ABCD) && (pxData->ucEnable)) {
        uint32_t ulMark = 0x1234ABCD;
        MemFlashWrite(0, 4, (uint8_t*)&ulMark);
        prvSendReply(REPLY_OK, pvInfo);
    }
    else {
        prvSendReply(REPLY_ERR, pvInfo);
    }
}

static void prvCmdCli(uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo)
{
    TRACE("iCmdCli\n");
    
    for (uint32_t n = 0; n < ulLength; n++) {
        CliCustomInput(prvCliUartPrintf, *(char*)(pucCont + n));
    }
}

#if 1
/* 拆分十六进制字符串 */
void splitString(const char* str, char pairs[NUM_PAIRS][HEX_PAIR_LENGTH + 1]) {
    for (int i = 0; i < NUM_PAIRS; i++) {
        strncpy(pairs[i], str + i * HEX_PAIR_LENGTH, HEX_PAIR_LENGTH);
        pairs[i][HEX_PAIR_LENGTH] = '\0';
    }
}

/* 十六进制字符串转十进制字符串 */
void hexToDecimal(const char* hex, char* decimalArray) {
    unsigned long decValue = strtoul(hex, NULL, 16);
    int tensDigit = decValue / 10;
    int onesDigit = decValue % 10;
    decimalArray[0] = '0' + tensDigit;
    decimalArray[1] = '0' + onesDigit;
}

void prvHexstrToDecstr(const ICmdEncrypt_t* pxData) {
    char pairs[NUM_PAIRS][HEX_PAIR_LENGTH + 1];
    splitString(pxData->cCode, pairs);
    /* Convert each pair from hexadecimal to decimal and store each digit separately */
    for (int i = 0; i < NUM_PAIRS; i++) {
        hexToDecimal(pairs[i], &decimalArray[i * DECIMAL_CHAR_LENGTH]);
    }
}
#endif

static void prvCmdEncrypt(uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo)
{
    TRACE("iCmdEncrypt\n");
    
    if (ulLength != sizeof(ICmdEncrypt_t)) {
        TRACE("    Wrong length\n");
        return;
    }
    const ICmdEncrypt_t *pxData = (const ICmdEncrypt_t*)pucCont;
    prvHexstrToDecstr(pxData);
    /* 0       :   使能位
     * 5 - 3   :   天数
     * 9 - 6   :   序列号 
     * 13 - 10 :   订单标识码 
     * 17 - 14 :   客户标识码  
     * 上面写反了，不想改，凑合看
    */
    if (((decimalArray[6] - '0') * 1000 + (decimalArray[7] - '0') * 100 + (decimalArray[8] - '0') * 10 + (decimalArray[9] - '0')) != th_OrderNum)
        {
            TRACE("    Order Number is ERROR\n");
            return;
        }
    if ((decimalArray[19] - '0') == 1)
    {
        Time_t xTm = RtcReadTime(RTC_TYPE_DS1338);
        th_Trial = ((decimalArray[19] - '0') == 1) ? 0 : 1;
        th_TrialDays = (decimalArray[16] - '0') + (decimalArray[15] - '0') * 10 + (decimalArray[14] - '0') * 100;
        th_TrialStTime = mktime(&xTm);
        th_TrialEn = 0;
        DataSaveDirect();
    }
    else
    {
        if (!th_TrialEn)
        {
            TRACE("    Don't double-license\n");
            return;
        }
        Time_t xTm = RtcReadTime(RTC_TYPE_DS1338);
        th_Trial = ((decimalArray[19] - '0') == 1) ? 0 : 1;
        th_TrialDays = (decimalArray[16] - '0') + (decimalArray[15] - '0') * 10 + (decimalArray[14] - '0') * 100;
        th_TrialStTime = mktime(&xTm);
        th_TrialEn = 0;
        DataSaveDirect();
    }
}

static void prvSend(uint8_t *pucData, uint8_t ucDataSize, uint8_t ucCmd, void *pvInfo)
{
    uint8_t *pucBuffer = GET_SEND_BUFFER();
    Head_t *pxHead     = GET_HEAD_BUFFER();
    pxHead->usStart    = 0x7E7E;
    pxHead->ucLength   = sizeof(Head_t) + ucDataSize + sizeof(Tail_t);
    pxHead->ucCmd      = ucCmd;
    pxHead->ucSrcAddr  = 1;
    pxHead->ucDstAddr  = 0;
    Tail_t *pxTail     = (Tail_t*)GET_TAIL_BUFFER(pxHead);
    pxTail->ucCheck    = 0;
    pxTail->usEnd      = 0x0D0A;
    
    uint8_t ucChk = 0;
    for (uint32_t n = 0; n < pxHead->ucLength; n++) {
        ucChk += *(pucBuffer + n);
    }
    ucChk              = ~ucChk;
    pxTail->ucCheck    = ucChk;
    
    if (CHAN_COM == (uint32_t)pvInfo) {
        UartBlkSend(s_xUart, pucBuffer, pxHead->ucLength, 10);
    }
    else if (CHAN_NET == (uint32_t)pvInfo) {
        send(s_xCliSock, pucBuffer, pxHead->ucLength, 0);
    }
}

static void prvSendReply(uint8_t ucStatus, void *pvInfo)
{
    uint8_t ucDataSize = sizeof(RCmdReply_t);
    RCmdReply_t *pxData = (RCmdReply_t*)GET_CONT_BUFFER();
    pxData->ucStatus = ucStatus;
    prvSend(GET_CONT_BUFFER(), ucDataSize, rCmdReply, pvInfo);
}

static void prvSendStatusInfo(void *pvInfo)
{
    uint8_t ucDataSize = sizeof(RCmdStatusInfo_t);
    RCmdStatusInfo_t *pxData = (RCmdStatusInfo_t*)GET_CONT_BUFFER();
    
    pxData->ucFsm              = (uint8_t)th_Fsm;
    
    pxData->xDi._MPWR_T_STAT_AC  = GpioGetInput(MPWR_T_STAT_AC);
    pxData->xDi._MPWR_T_STAT_DC  = GpioGetInput(MPWR_T_STAT_DC);
    pxData->xDi._MPWR_M_STAT_AC  = GpioGetInput(MPWR_M_STAT_AC);
    pxData->xDi._MPWR_M_STAT_DC  = GpioGetInput(MPWR_M_STAT_DC);
    pxData->xDi._MPWR_B_STAT_AC  = GpioGetInput(MPWR_B_STAT_AC);
    pxData->xDi._MPWR_B_STAT_DC  = GpioGetInput(MPWR_B_STAT_DC);
    
    pxData->xDi._APWR1_STAT      = GpioGetInput(APWR1_STAT);
    pxData->xDi._APWR2_STAT      = GpioGetInput(APWR2_STAT);
    pxData->xDi._APWR3_STAT      = GpioGetInput(APWR3_STAT);
    pxData->xDi._QBH_ON          = GpioGetInput(QBH_ON);
    pxData->xDi._EX_CTRL_EN        = GpioGetInput(EX_CTRL_EN);
    pxData->xDi._WATER_PRESS     = GpioGetInput(WATER_PRESS);
    pxData->xDi._WATER_CHILLER   = GpioGetInput(WATER_CHILLER);

    pxData->xDo._MPWR_T_EN       = GpioGetOutput(MPWR_T_EN);
    pxData->xDo._MPWR_M_EN       = GpioGetOutput(MPWR_M_EN);
    pxData->xDo._MPWR_B_EN       = GpioGetOutput(MPWR_B_EN);
    
    pxData->xDo._APWR1_EN        = GpioGetOutput(APWR1_EN);
    pxData->xDo._APWR2_EN        = GpioGetOutput(APWR2_EN);
    pxData->xDo._APWR3_EN        = GpioGetOutput(APWR3_EN);
    //pxData->xDo._LED_CTRL        = GpioGetOutput(LED_CTRL);
    pxData->xDo._LED_CTRL        = (TIM2->CCR1) ? 1 : 0;
    pxData->xDo._LED1            = GpioGetOutput(LED1);
    pxData->xDo._LED2            = GpioGetOutput(LED2);
    pxData->xDo._LED3            = GpioGetOutput(LED3);
    pxData->xDo._LED4            = GpioGetOutput(LED4);
    pxData->xDo._LED5            = GpioGetOutput(LED5);
    pxData->xDo._LED6            = GpioGetOutput(LED6);
    
    pxData->usTempH              = StcGetTempH();
    
    pxData->usAdc[0]             = AdcGet(ADC_CHAN_1);    /* 恒流源1电流 */
    pxData->usAdc[1]             = AdcGet(ADC_CHAN_2);    /* 恒流源1电压 */
    pxData->usAdc[2]             = AdcGet(ADC_CHAN_3);    /* 恒流源2电流 */
    pxData->usAdc[3]             = AdcGet(ADC_CHAN_4);    /* 恒流源2电压 */
    pxData->usAdc[4]             = AdcGet(ADC_CHAN_5);    /* 恒流源3电流 */
    pxData->usAdc[5]             = AdcGet(ADC_CHAN_6);    /* 恒流源3电压 */
    pxData->usAdc[6]             = AdcGet(ADC_CHAN_8);    /* AimLight Cur */
    
#if 0
    pxData->usAdc[6]             = AdcGet(ADC_CHAN_7);    /* 漏光检测1 */
    pxData->usAdc[7]             = AdcGet(ADC_CHAN_7);    /* 漏光检测2 */
    pxData->usAdc[8]             = AdcGet(ADC_CHAN_7);    /* 漏光检测3 */
#endif

    pxData->usDac[0]             = DacGet(DAC_CHAN_1);
    pxData->usDac[1]             = DacGet(DAC_CHAN_2);
    pxData->usDac[2]             = DacGet(DAC_CHAN_3);
    
    pxData->usAdc[0]             = (pxData->usAdc[0] >= 3200) ? 0 : pxData->usAdc[0];
    pxData->usAdc[2]             = (pxData->usAdc[2] >= 3200) ? 0 : pxData->usAdc[2];
    pxData->usAdc[4]             = (pxData->usAdc[4] >= 3200) ? 0 : pxData->usAdc[4];
    
    pxData->usTMPwrVol           = PwrDataGet(PWR_T_ADDR, PWR_OUTPUT_VOL);
    pxData->usMMPwrVol           = PwrDataGet(PWR_M_ADDR, PWR_OUTPUT_VOL);
    pxData->usBMPwrVol           = PwrDataGet(PWR_B_ADDR, PWR_OUTPUT_VOL);
    
    pxData->usTemp[0][0]         = StcGetTemp(STC_DEV_1, STC_TEMP_NODE_1);
    pxData->usTemp[0][1]         = StcGetTemp(STC_DEV_1, STC_TEMP_NODE_2);
    pxData->usTemp[0][2]         = StcGetTemp(STC_DEV_1, STC_TEMP_NODE_3);
    pxData->usTemp[0][3]         = StcGetTemp(STC_DEV_1, STC_TEMP_NODE_4);
    pxData->usTemp[0][4]         = StcGetTemp(STC_DEV_1, STC_TEMP_NODE_5);
    pxData->usTemp[0][5]         = StcGetTemp(STC_DEV_1, STC_TEMP_NODE_6);
    pxData->usTemp[0][6]         = StcGetTemp(STC_DEV_1, STC_TEMP_NODE_7);
    pxData->usTemp[0][7]         = StcGetTemp(STC_DEV_1, STC_TEMP_NODE_8);
    
    pxData->usTemp[1][0]         = StcGetTemp(STC_DEV_1, STC_TEMP_NODE_9);
    pxData->usTemp[1][1]         = StcGetTemp(STC_DEV_1, STC_TEMP_NODE_10);
    pxData->usTemp[1][2]         = StcGetTemp(STC_DEV_2, STC_TEMP_NODE_1);
    pxData->usTemp[1][3]         = StcGetTemp(STC_DEV_2, STC_TEMP_NODE_2);
    pxData->usTemp[1][4]         = StcGetTemp(STC_DEV_2, STC_TEMP_NODE_3);
    pxData->usTemp[1][5]         = StcGetTemp(STC_DEV_2, STC_TEMP_NODE_4);
    pxData->usTemp[1][6]         = StcGetTemp(STC_DEV_2, STC_TEMP_NODE_5);
    pxData->usTemp[1][7]         = StcGetTemp(STC_DEV_2, STC_TEMP_NODE_6);
    
    pxData->usTemp[2][0]         = StcGetTemp(STC_DEV_2, STC_TEMP_NODE_7);
    pxData->usTemp[2][1]         = StcGetTemp(STC_DEV_2, STC_TEMP_NODE_8);
    pxData->usTemp[2][2]         = StcGetTemp(STC_DEV_2, STC_TEMP_NODE_9);
    pxData->usTemp[2][3]         = StcGetTemp(STC_DEV_2, STC_TEMP_NODE_10);
    pxData->usTemp[2][4]         = StcGetTemp(STC_DEV_3, STC_TEMP_NODE_1);
    pxData->usTemp[2][5]         = StcGetTemp(STC_DEV_3, STC_TEMP_NODE_2);
    pxData->usTemp[2][6]         = StcGetTemp(STC_DEV_3, STC_TEMP_NODE_3);
    pxData->usTemp[2][7]         = StcGetTemp(STC_DEV_3, STC_TEMP_NODE_4);

#if 0
    pxData->usPd[0]              = StcGetTemp(STC_DEV_3, STC_TEMP_NODE_5);
    pxData->usPd[1]              = StcGetTemp(STC_DEV_3, STC_TEMP_NODE_6);
    pxData->usPd[2]              = StcGetTemp(STC_DEV_3, STC_TEMP_NODE_7);
    pxData->usPd[3]              = StcGetTemp(STC_DEV_3, STC_TEMP_NODE_8);
    pxData->usPd[4]              = StcGetTemp(STC_DEV_3, STC_TEMP_NODE_9);
    pxData->usPd[5]              = StcGetTemp(STC_DEV_3, STC_TEMP_NODE_10);
#endif
    
    pxData->usPd[0]              = StcGetTemp(STC_DEV_3, STC_TEMP_NODE_5);
    pxData->usPd[1]              = StcGetTemp(STC_DEV_3, STC_TEMP_NODE_6);
    pxData->usPd[2]              = StcGetTemp(STC_DEV_3, STC_TEMP_NODE_10);
    pxData->usPd[3]              = StcGetTemp(STC_DEV_3, STC_TEMP_NODE_9);
    pxData->usPd[4]              = StcGetTemp(STC_DEV_3, STC_TEMP_NODE_8);
    pxData->usPd[5]              = StcGetTemp(STC_DEV_3, STC_TEMP_NODE_7);

    pxData->usSysStatus          = th_SysStatusAll;
    
    pxData->ulTPwrStatus         = PwrDataGet(PWR_T_ADDR, PWR_STATUS);
    pxData->ulMPwrStatus         = PwrDataGet(PWR_M_ADDR, PWR_STATUS);
    pxData->ulBPwrStatus         = PwrDataGet(PWR_B_ADDR, PWR_STATUS);
    
    pxData->usSwInfo             = th_SwInfo;
    
    prvSend(GET_CONT_BUFFER(), ucDataSize, rCmdStatusInfo, pvInfo);
}

static void prvSendDiagInfo(void *pvInfo)
{
    uint8_t ucDataSize = sizeof(RCmdDiagInfo_t);
    RCmdDiagInfo_t *pxData = (RCmdDiagInfo_t*)GET_CONT_BUFFER();
    pxData->ulSwVer   = SW_VER;
    pxData->ulRunTime = th_RunTime;
    prvSend(GET_CONT_BUFFER(), ucDataSize, rCmdDiagInfo, pvInfo);
}

static void prvSendSysPara(void *pvInfo)
{
    uint8_t ucDataSize = sizeof(RCmdSysPara_t);
    RCmdSysPara_t *pxData = (RCmdSysPara_t*)GET_CONT_BUFFER();
    pxData->sOtWarnTh   = AdcToTemp(th_OtWarnTh);
    pxData->sOtCutTh    = AdcToTemp(th_OtCutTh);
    pxData->ulMaxCur    = th_MaxCur;
    pxData->ulWorkCur   = th_WorkCur;
    pxData->lCompRate   = th_CompRate;
    pxData->usPdWarnL1  = th_PdWarnL1 * 3300 / 4096;
    pxData->usPdWarnL2  = th_PdWarnL2 * 3300 / 4096;
    pxData->ulAdVolPara = th_AdVolPara;
    pxData->ucPrType    = th_PwrType;
    pxData->ucTempNum   = th_TempNum;
    pxData->usModEn     = th_ModEnAll;
    prvSend(GET_CONT_BUFFER(), ucDataSize, rCmdSysPara, pvInfo);
}

static Status_t prvProtPktProc(const void *pvHead, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo)
{
    Head_t *p = (Head_t*)pvHead;
    
    /* Check source address */
    if (p->ucSrcAddr != 0) {
        TRACE("Com err: src addr\n");
        return STATUS_ERR;
    }
    
    /* Check destination address */  
    if (p->ucDstAddr != 1) {
        TRACE("Com err: dst addr\n");
        return STATUS_ERR;
    }
    
    /* Process command */
    switch (p->ucCmd) {
    case iCmdQueryInfo:
        prvCmdQueryInfo(p->ucSrcAddr, pucCont, ulLength, pvInfo);
        break;
    case iCmdSysReset:
        prvCmdSysReset(p->ucSrcAddr, pucCont, ulLength, pvInfo);
        break;
    case iCmdParaConfig:
        prvCmdParaConfig(p->ucSrcAddr, pucCont, ulLength, pvInfo);
        break;
    case iCmdModuleCtrl:
        prvCmdModuleCtrl(p->ucSrcAddr, pucCont, ulLength, pvInfo);
        break;
    case iCmdUpgradeEnable:
        prvCmdUpgradeEnable(p->ucSrcAddr, pucCont, ulLength, pvInfo);
        break;
    case iCmdCli:
        prvCmdCli(p->ucSrcAddr, pucCont, ulLength, pvInfo);
        break;
    case iCmdEncrypt:
        prvCmdEncrypt(p->ucSrcAddr, pucCont, ulLength, pvInfo);
        break;
    default:
        break;
    }
    
    return STATUS_OK;
}

static Bool_t prvProtPktChk(const void *pvStart, uint32_t ulLength)
{
    uint8_t ucSum = 0;
    
    for (uint32_t n = 0; n < ulLength; n++) {
        ucSum += *((uint8_t*)pvStart + n);
    }
    
    return (0xFF == ucSum) ? TRUE : FALSE;
}

static Status_t prvUartRecv(uint8_t *pucBuf, uint16_t usLength, void* pvIsrPara)
{
    return (usLength == RbufWrite(s_xRbuf, pucBuf, usLength)) ? STATUS_OK : STATUS_ERR;
}

static void prvCliUartPrintf(const char* cFormat, ...)
{
    va_list va;
    va_start(va, cFormat);
    uint16_t usLength = vsprintf((char*)GET_CONT_BUFFER(), cFormat, va);
    if (usLength) {
        prvSend(GET_CONT_BUFFER(), (uint8_t)usLength, rCmdCli, (void*)CHAN_COM);
    }
}

void USART1_IRQHandler(void)
{
    if (s_xUart) {
        UartIsr(s_xUart);
    }
}

void DMA1_Channel5_IRQHandler(void)
{
    if (s_xUart){
        UartDmaRxIsr(s_xUart);
    }
}

void DMA1_Channel4_IRQHandler(void)
{
    if (s_xUart) {
        UartDmaTxIsr(s_xUart);
    }
}

#if ENABLE_RS485_TEST
static Status_t prvUartRecvRs485(uint8_t *pucBuf, uint16_t usLength, void* pvIsrPara)
{
    if (s_bUartRs485Recv) {
        /* Enable RS485 write */
        RS485_WT();
        UartBlkSend(s_xUartRs485, pucBuf, usLength, 10);
        /* Enable RS485 read */
        RS485_RD();
    }
    return STATUS_OK;
}

void UART5_IRQHandler(void)
{
    if (s_xUartRs485) {
        UartIsr(s_xUartRs485);
    }
}

static uint8_t* prvParseHexStr(const char* pcStr, uint8_t *pucLength)
{
    static uint8_t ucBuffer[80];
    uint8_t ucData = 0;
    uint8_t ucCnt = 0;
    uint8_t ucIndex = 0;
    Bool_t bProc = FALSE;
    while ((0 != *pcStr) && (ucIndex < 80)) {
        char cTmp;
        cTmp = *pcStr++;
        if ((cTmp >= 'a') && (cTmp <= 'f')) {
            ucData *= 16;
            ucData += 10 + (cTmp - 'a');
            ucCnt++;
            bProc = TRUE;
        }
        if ((cTmp >= 'A') && (cTmp <= 'F')) {
            ucData *= 16; 
            ucData += 10 + (cTmp - 'A');
            ucCnt++;
            bProc = TRUE;
        }
        if ((cTmp >= '0') && (cTmp <= '9')) {
            ucData *= 16;
            ucData += cTmp - '0';
            ucCnt++;
            bProc = TRUE;
        }
        if ((TRUE == bProc) && ((' ' == cTmp) || (',' == cTmp) || ('|' == cTmp) || (2 == ucCnt) || (0 == *pcStr))) {
            ucBuffer[ucIndex++] = ucData;
            ucData = 0;
            ucCnt = 0;
            bProc = FALSE;
        }
    }
    *pucLength = ucIndex;
    return ucBuffer;
}

static void prvCliCmdRs485Send(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    if (argc < 2) {
        cliprintf("rs485_send DATA(HEX)\n");
        return;
    }
    
    uint8_t ucLength = 0;
    uint8_t *pucBuffer = prvParseHexStr(argv[1], &ucLength);
    
    /* Enable RS485 write */
    RS485_WT();
    
    UartBlkSend(s_xUartRs485, pucBuffer, ucLength, 10);
    
    /* Enable RS485 read */
    RS485_RD();
    
    s_bUartRs485Recv = TRUE;
    
    return;
}
CLI_CMD_EXPORT(rs485_send, rs485 send, prvCliCmdRs485Send)
#endif /* ENABLE_RS485_TEST */



#elif SET_COM_SEND_PTL == 1
enum {
    iCmdQueryInfo      = 0x01,
    iCmdSysReset       = 0x02,
    iCmdParaConfig     = 0x03,
    iCmdModuleCtrl     = 0x04,
    iCmdUpgradeEnable  = 0x05,
    iCmdCli            = 0x06,
    iCmdEncrypt        = 0x07,
    rCmdReply          = 0x81,
    rCmdStatusInfo     = 0x82,
    rCmdDiagInfo       = 0x83,
    rCmdCli            = 0x84,
    rCmdSysPara        = 0x85,
};

enum {
    TYPE_OT_WARN       = 0x1111,
    TYPE_OT_CUT        = 0x2222,
    TYPE_MAX_CUR       = 0x3333,
    TYPE_WORK_CUR      = 0x4444,
    TYPE_PWR_COMP      = 0x5555,
    TYPE_MPWR_VOL      = 0x6666,
    TYPE_TEMP_NUM      = 0x7777,
    TYPE_PD_VOL        = 0x8888,
    TYPE_MODULE_ENABLE = 0x9999,
};

enum {
    TYPE_EX_CTRL_EN      = 0x1111,
    TYPE_LASER_OFF     = 0x2222,
    TYPE_INFRARED_ON   = 0x3333,
    TYPE_INFRARED_OFF  = 0x4444,
    TYPE_MANUAL_CTRL   = 0x5555,
};

typedef struct {
    uint16_t usStart;
    uint8_t  ucLength;
    uint8_t  ucCmd;
    uint8_t  ucSrcAddr;
    uint8_t  ucDstAddr;
}Head_t;

typedef struct {
    uint8_t  ucCheck;
    uint16_t usEnd;
}Tail_t;

typedef struct {
    uint8_t ucRespCmd;
}ICmdQueryInfo_t;

typedef struct {
    uint32_t ulMark;
}ICmdSysReset_t;

typedef struct {
    uint16_t usType;
    uint32_t ulPara1;
    uint32_t ulPara2;
}ICmdParaConfig_t;

typedef struct {
    uint16_t usType;
    uint32_t ulPara1;
    uint32_t ulPara2;
}ICmdModuleCtrl_t;

typedef struct {
    uint32_t ulMark;
    uint8_t  ucEnable;
}ICmdUpgradeEnable_t;

typedef struct {
    char cCode[20];
}ICmdEncrypt_t;
char decimalArray[NUM_PAIRS * DECIMAL_CHAR_LENGTH];

typedef struct {
    uint8_t ucStatus;
}RCmdReply_t;

typedef struct {
    uint8_t  ucFsm;
    struct {
        uint16_t _MPWR_STAT_AC:1;
        uint16_t _MPWR_STAT_DC:1;
        uint16_t _APWR1_STAT:1;
        uint16_t _APWR2_STAT:1;
        uint16_t _APWR3_STAT:1;
        uint16_t _QBH_ON:1;
        uint16_t _EX_CTRL_EN:1;
        uint16_t _WATER_PRESS:1;
        uint16_t _WATER_CHILLER:1;
    }xDi;
    struct {
        uint16_t _MPWR_EN:1;
        uint16_t _APWR1_EN:1;
        uint16_t _APWR2_EN:1;
        uint16_t _APWR3_EN:1;
        uint16_t _LED_CTRL:1;
        uint16_t _LED1:1;
        uint16_t _LED2:1;
        uint16_t _LED3:1;
        uint16_t _LED4:1;
        uint16_t _LED5:1;
        uint16_t _LED6:1;
    }xDo;
    uint16_t usTempH;
    uint16_t usAdc[10];
    uint16_t usDac[3];
    uint16_t usMPwrVol;
    uint16_t usTemp[10];
    uint16_t usSysStatus;
    uint32_t ulPwrStatus;
    uint16_t usSwInfo;
}RCmdStatusInfo_t;

typedef struct {
    uint32_t ulSwVer;
    uint32_t ulRunTime;
}RCmdDiagInfo_t;

typedef struct {
    int16_t  sOtWarnTh;
    int16_t  sOtCutTh;
    uint32_t ulMaxCur;
    uint32_t ulWorkCur;
    int32_t  lCompRate;
    uint16_t usPdWarnL1;
    uint16_t usPdWarnL2;
    uint32_t ulAdVolPara;
    uint8_t  ucPrType;
    uint8_t  ucTempNum;
    uint16_t usModEn;
}RCmdSysPara_t;

enum {
    REPLY_OK,
    REPLY_ERR
};
#pragma pack(pop)

/* Forward declaration */
static void     prvComTask         (void* pvPara);
static void     prvNetTask         (void* pvPara);
static void     prvCmdQueryInfo    (uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo);
static void     prvCmdSysReset     (uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo);
static void     prvCmdParaConfig   (uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo);
static void     prvCmdModuleCtrl   (uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo);
static void     prvCmdUpgradeEnable(uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo);
static void     prvCmdCli          (uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo);
static void     prvCmdEncrypt      (uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo);
static void     prvSend            (uint8_t *pucData, uint8_t ucDataSize, uint8_t ucCmd, void *pvInfo);
static void     prvSendReply       (uint8_t ucStatus, void *pvInfo);
static void     prvSendStatusInfo  (void *pvInfo);
static void     prvSendDiagInfo    (void *pvInfo);
static void     prvSendSysPara     (void *pvInfo);
static Status_t prvProtPktProc     (const void *pvHead, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo);
static Bool_t   prvProtPktChk      (const void *pvStart, uint32_t ulLength);
static Status_t prvUartRecv        (uint8_t *pucBuf, uint16_t usLength, void* pvIsrPara);
static void     prvCliUartPrintf   (const char* cFormat, ...);
#if ENABLE_RS485_TEST
static Status_t prvUartRecvRs485   (uint8_t *pucBuf, uint16_t usLength, void* pvIsrPara);
#endif /* ENABLE_RS485_TEST */

/* Local variables */
static RbufHandle_t s_xRbuf = NULL;
static UartHandle_t s_xUart = NULL;
static ProtHandle_t s_xProt = NULL;
static uint8_t      s_ucBuffer[256];
static uint8_t      s_ucSendBuffer[MAX_MSG_SIZE];
static SOCKET       s_xSvrSock = -1;
static SOCKET       s_xCliSock = -1;
#if ENABLE_RS485_TEST
static UartHandle_t s_xUartRs485 = NULL;
static Bool_t       s_bUartRs485Recv = FALSE;
#endif /* ENABLE_RS485_TEST */

/* Functions */
Status_t AppComInit(void)
{
    uint8_t ucHeadMark[] = {0x7E, 0x7E};
    uint8_t ucTailMark[] = {0x0A, 0x0D};
    
    s_xRbuf = NULL;
    s_xUart = NULL;
    s_xProt = NULL;
    
    RbufInit();
    s_xRbuf = RbufCreate();
    RbufConfig(s_xRbuf, s_ucBuffer, sizeof(s_ucBuffer), 3/*Rbuf msg queue size*/, 5/*Rbuf msg queue wait ms*/);
    
    ProtInit();
    s_xProt = ProtCreate();
    ProtConfigHead(s_xProt, ucHeadMark, sizeof(ucHeadMark), sizeof(Head_t));
    ProtConfigTail(s_xProt, ucTailMark, sizeof(ucTailMark), sizeof(Tail_t));
    ProtConfigMisc(s_xProt, MAX_MSG_SIZE, 0/* Msg length offset */, PROT_LENGTH_UINT8, 2/* Head mark size */);
    ProtConfigCb(s_xProt, prvProtPktProc, prvProtPktChk);
    ProtConfig(s_xProt);
    
    UartInit();
    s_xUart = UartCreate();
    UartConfigCb(s_xUart, prvUartRecv, UartIsrCb, UartDmaRxIsrCb, UartDmaTxIsrCb, NULL);
    UartConfigRxDma(s_xUart, DMA1_Channel5, DMA1_Channel5_IRQn);
    UartConfigTxDma(s_xUart, DMA1_Channel4, DMA1_Channel4_IRQn);
    UartConfigCom(s_xUart, USART1, 115200, USART1_IRQn);
#if ENABLE_RS485_TEST
    s_xUartRs485 = UartCreate();
    UartConfigCb(s_xUartRs485, prvUartRecvRs485, UartIsrCb, NULL, NULL, NULL);
    UartConfigCom(s_xUartRs485, UART5, 115200, UART5_IRQn);
    RS485_RD();
#endif /* ENABLE_RS485_TEST */
    
    Bool_t   bDhcp          = th_Dhcp;
    uint32_t ulDhcpTimeout  = th_DhcpTimeout;
    uint32_t ulLocalIp      = th_LocalIp;
    uint32_t ulLocalNetMask = th_LocalNetMask;
    uint32_t ulLocalGwAddr  = th_LocalGwAddr;
    char     *cServerName   = "192.168.1.11"; /* No use here! */
    uint16_t usServerPort   = 6000; /* No use here! */
    void     *pxNetIf       = NULL;
    pxNetIf = LwIPGetNetIf();
    NetConfigEth(bDhcp, ulDhcpTimeout, ulLocalIp, ulLocalNetMask, ulLocalGwAddr, cServerName, usServerPort, pxNetIf);
    //DrvNetInit();
    
    xTaskCreate(prvComTask, "tCom", 256, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(prvNetTask, "tNet", 256, NULL, tskIDLE_PRIORITY, NULL);
    
    return STATUS_OK;
}

Status_t AppComTerm(void)
{
    /* Do nothing */
    return STATUS_OK;
}

static void prvComTask(void* pvPara)
{
    while (1) {
        static uint16_t s_usRecvIndex = 0;
        static uint8_t s_ucProcBuf[MAX_MSG_SIZE];
        static uint8_t s_ucRecvBuf[MAX_MSG_SIZE];
        uint16_t usRecvd = (uint16_t)RbufRead(s_xRbuf, s_ucRecvBuf, MAX_MSG_SIZE);
        if (usRecvd) {
            ProtProc(s_xProt, s_ucRecvBuf, usRecvd, &s_usRecvIndex, s_ucProcBuf, (void*)CHAN_COM);
        }
    }
}

static void prvNetTask(void* pvPara)
{
    struct sockaddr_in xSvrAddr, xCliAddr;

    /* Socket */
    s_xSvrSock = socket(AF_INET, SOCK_STREAM, 0);
    if (s_xSvrSock == -1) {
        TRACE("socket failed\n");
    }

    /* Bind */
    xSvrAddr.sin_family      = AF_INET;
    xSvrAddr.sin_addr.s_addr = INADDR_ANY;
    xSvrAddr.sin_port        = htons(6000);
    if (bind(s_xSvrSock, (struct sockaddr *)&xSvrAddr, sizeof(xSvrAddr)) == -1) {
        TRACE("bind failed\n");
    }

    /* Listen */
    if (listen(s_xSvrSock, 5) == -1) {
        TRACE("listen failed\n");
    }

    while (1) {
        socklen_t xCliAddrLength = sizeof(xCliAddr);

        /* Accept */
        s_xCliSock = accept(s_xSvrSock, (struct sockaddr *)&xCliAddr, &xCliAddrLength);
        if (s_xCliSock == -1) {
            TRACE("accept failed");
        }
        else {
            TRACE("Accepted %s %d\n", inet_ntoa(xCliAddr.sin_addr), ntohs(xCliAddr.sin_port));
        }

       while (1) {
            static uint16_t s_usRecvIndex = 0;
            static uint8_t s_ucProcBuf[MAX_MSG_SIZE];
            static uint8_t s_ucRecvBuf[MAX_MSG_SIZE];
            
            /* Receive */
            ssize_t lRead = recv(s_xCliSock, s_ucRecvBuf, MAX_MSG_SIZE, 0);
            if (lRead <= 0) {
                close(s_xCliSock);
                s_xCliSock = -1;
                break;
            }
            
            /* Process */
            ProtProc(s_xProt, s_ucRecvBuf, (uint16_t)lRead, &s_usRecvIndex, s_ucProcBuf, (void*)CHAN_NET);
       }
    }
}

static void prvCmdQueryInfo(uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo)
{
    TRACE("iCmdQueryInfo\n");
    
    if (ulLength != sizeof(ICmdQueryInfo_t)) {
        TRACE("    Wrong length\n");
        return;
    }
    
    const ICmdQueryInfo_t *pxData = (const ICmdQueryInfo_t*)pucCont;
    
    switch (pxData->ucRespCmd) {
    case rCmdStatusInfo:
        prvSendStatusInfo(pvInfo);
        break;
    case rCmdDiagInfo:
        prvSendDiagInfo(pvInfo);
        break;
    case rCmdSysPara:
        prvSendSysPara(pvInfo);
        break;
    default:
        prvSendReply(REPLY_ERR, pvInfo);
        break;
    }
}

static void prvCmdSysReset(uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo)
{
    TRACE("iCmdSysReset\n");
    
    if (ulLength != sizeof(ICmdSysReset_t)) {
        TRACE("    Wrong length\n");
        return;
    }
    
    const ICmdSysReset_t *pxData = (const ICmdSysReset_t*)pucCont;
    
    if (pxData->ulMark == 0x1234ABCD) {
        prvSendReply(REPLY_OK, pvInfo);
        NVIC_SystemReset();
    }
    else {
        prvSendReply(REPLY_ERR, pvInfo);
    }
}

static void prvCmdParaConfig(uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo)
{
    TRACE("iCmdParaConfig\n");
    
    if (!th_VerChk) {
        prvSendReply(REPLY_ERR, pvInfo);
        return;
    }
    
    if (ulLength != sizeof(ICmdParaConfig_t)) {
        TRACE("    Wrong length\n");
        return;
    }
    
    const ICmdParaConfig_t *pxData = (const ICmdParaConfig_t*)pucCont;
    
    Status_t xRet = STATUS_ERR;
    switch (pxData->usType) {
    case TYPE_OT_WARN:
        if (pxData->ulPara1 >= th_OtCutTh) {
            xRet = STATUS_OK;
            th_OtWarnTh = pxData->ulPara1;
            DataSaveDirect();
        }
        break;
    case TYPE_OT_CUT:
        if (pxData->ulPara1 <= th_OtWarnTh) {
            xRet = STATUS_OK;
            th_OtCutTh = pxData->ulPara1;
            DataSaveDirect();
        }
        break;
    case TYPE_MAX_CUR:
        if (pxData->ulPara1 >= th_WorkCur) {
            xRet = STATUS_OK;
            th_MaxCur = pxData->ulPara1;
            th_MaxCurAd = CUR_TO_ADC(th_MaxCur * 0.1);
            DataSaveDirect();
        }
        break;
    case TYPE_WORK_CUR:
        if (pxData->ulPara1 <= th_MaxCur) {
            xRet = STATUS_OK;
            th_WorkCur = pxData->ulPara1;
            DataSaveDirect();
        }
        break;
    case TYPE_PWR_COMP:
        if (pxData->ulPara1 <= 100) {
            xRet = STATUS_OK;
            th_CompRate = (int32_t)pxData->ulPara1;
            DataSaveDirect();
        }
        break;
    case TYPE_MPWR_VOL:
        PwrSetVolDef(pxData->ulPara1 * 0.1);
        break;
    case TYPE_TEMP_NUM:
        if ((pxData->ulPara1 >= 1) && (pxData->ulPara1 <= 10)) {
            xRet = STATUS_OK;
            th_TempNum = (uint8_t)pxData->ulPara1;
            DataSaveDirect();
        }
        break;
    case TYPE_PD_VOL:
        th_PdWarnL1 = (uint16_t)(pxData->ulPara1/*mV*/ * 4096 / 3300);
        DataSaveDirect();
        break;
    case TYPE_MODULE_ENABLE:
        xRet = STATUS_OK;
        switch (pxData->ulPara1) {
        case 0:  th_ModEn.MANUAL        = pxData->ulPara2 ? 1 : 0; break;
        case 1:  th_ModEn.QBH           = pxData->ulPara2 ? 1 : 0; break;
        case 2:  th_ModEn.PD            = pxData->ulPara2 ? 1 : 0; break;
        case 3:  th_ModEn.TEMP1         = pxData->ulPara2 ? 1 : 0; break;
        case 4:  th_ModEn.TEMP2         = pxData->ulPara2 ? 1 : 0; break;
        case 5:  th_ModEn.CHAN1_CUR     = pxData->ulPara2 ? 1 : 0; break;
        case 6:  th_ModEn.CHAN2_CUR     = pxData->ulPara2 ? 1 : 0; break;
        case 7:  th_ModEn.CHAN3_CUR     = pxData->ulPara2 ? 1 : 0; break;
        case 8:  th_ModEn.WATER_PRESS   = pxData->ulPara2 ? 1 : 0; break;
        case 9:  th_ModEn.WATER_CHILLER = pxData->ulPara2 ? 1 : 0; break;
        case 10: th_ModEn.CHAN_CTRL     = pxData->ulPara2 ? 1 : 0; break;
        }
        DataSaveDirect();
        break;
    default:
        break;
    }
    
    if (xRet == STATUS_OK) {
        prvSendReply(REPLY_OK, pvInfo);
    }
    else {
        prvSendReply(REPLY_ERR, pvInfo);
    }
}

static void prvCmdModuleCtrl(uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo)
{
    TRACE("iCmdModuleCtrl\n");
    
    if (!th_VerChk) {
        prvSendReply(REPLY_ERR, pvInfo);
        return;
    }
    
    if (ulLength != sizeof(ICmdModuleCtrl_t)) {
        TRACE("    Wrong length\n");
        return;
    }
    
    const ICmdModuleCtrl_t *pxData = (const ICmdModuleCtrl_t*)pucCont;
    
    Status_t xRet = STATUS_ERR;
    switch (pxData->usType) {
    case TYPE_EX_CTRL_EN:
        xRet = LaserOn(pxData->ulPara1, pxData->ulPara2);
        break;
    case TYPE_LASER_OFF:
        xRet = LaserOff(pxData->ulPara2);
        break;
    case TYPE_INFRARED_ON:
        xRet = InfraredOn();
        break;
    case TYPE_INFRARED_OFF:
        xRet = InfraredOff();
        break;
    case TYPE_MANUAL_CTRL:
        xRet = EnableManualCtrl((uint8_t)pxData->ulPara1);
    default:
        break;
    }
    
    if (xRet == STATUS_OK) {
        prvSendReply(REPLY_OK, pvInfo);
    }
    else {
        prvSendReply(REPLY_ERR, pvInfo);
    }
}

static void prvCmdUpgradeEnable(uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo)
{
    TRACE("iCmdUpgradeEnable\n");
    
    if (ulLength != sizeof(ICmdUpgradeEnable_t)) {
        TRACE("    Wrong length\n");
        return;
    }
    
    const ICmdUpgradeEnable_t *pxData = (const ICmdUpgradeEnable_t*)pucCont;
    
    if ((pxData->ulMark == 0x1234ABCD) && (pxData->ucEnable)) {
        uint32_t ulMark = 0x1234ABCD;
        MemFlashWrite(0, 4, (uint8_t*)&ulMark);
        prvSendReply(REPLY_OK, pvInfo);
    }
    else {
        prvSendReply(REPLY_ERR, pvInfo);
    }
}

static void prvCmdCli(uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo)
{
    TRACE("iCmdCli\n");
    
    for (uint32_t n = 0; n < ulLength; n++) {
        CliCustomInput(prvCliUartPrintf, *(char*)(pucCont + n));
    }
}


#if 1
/* 拆分十六进制字符串 */
void splitString(const char* str, char pairs[NUM_PAIRS][HEX_PAIR_LENGTH + 1]) {
    for (int i = 0; i < NUM_PAIRS; i++) {
        strncpy(pairs[i], str + i * HEX_PAIR_LENGTH, HEX_PAIR_LENGTH);
        pairs[i][HEX_PAIR_LENGTH] = '\0';
    }
}

/* 十六进制字符串转十进制字符串 */
void hexToDecimal(const char* hex, char* decimalArray) {
    unsigned long decValue = strtoul(hex, NULL, 16);
    int tensDigit = decValue / 10;
    int onesDigit = decValue % 10;
    decimalArray[0] = '0' + tensDigit;
    decimalArray[1] = '0' + onesDigit;
}

void prvHexstrToDecstr(const ICmdEncrypt_t* pxData) {
    char pairs[NUM_PAIRS][HEX_PAIR_LENGTH + 1];
    

    splitString(pxData->cCode, pairs);

    // Convert each pair from hexadecimal to decimal and store each digit separately
    for (int i = 0; i < NUM_PAIRS; i++) {
        hexToDecimal(pairs[i], &decimalArray[i * DECIMAL_CHAR_LENGTH]);
    }
}
#endif

static void prvCmdEncrypt(uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo)
{
    TRACE("iCmdEncrypt\n");
    
    if (ulLength != sizeof(ICmdEncrypt_t)) {
        TRACE("    Wrong length\n");
        return;
    }
    
    const ICmdEncrypt_t *pxData = (const ICmdEncrypt_t*)pucCont;
    prvHexstrToDecstr(pxData);
    
    /* 0       :   使能位
     * 5 - 3   :   天数
     * 9 - 6   :   序列号 
     * 13 - 10 :   订单标识码 
     * 17 - 14 :   客户标识码  
    */
    
    if (((decimalArray[6] - '0') * 1000 + (decimalArray[7] - '0') * 100 + (decimalArray[8] - '0') * 10 + (decimalArray[9] - '0')) != th_OrderNum)
    {
            TRACE("    Order Number is ERROR\n");
            return;
    }
    
    if ((decimalArray[19] - '0') == 1)
    {
        Time_t xTm = RtcReadTime(RTC_TYPE_DS1338);
        th_Trial = ((decimalArray[19] - '0') == 1) ? 0 : 1;
        th_TrialDays = (decimalArray[16] - '0') + (decimalArray[15] - '0') * 10 + (decimalArray[14] - '0') * 100;
        th_TrialStTime = mktime(&xTm);
        th_TrialEn = 0;
        DataSaveDirect();
    }
    else
    {

        if (!th_TrialEn)
        {
            TRACE("    Don't double-license\n");
            return;
        }
        Time_t xTm = RtcReadTime(RTC_TYPE_DS1338);
        th_Trial = ((decimalArray[19] - '0') == 1) ? 0 : 1;
        th_TrialDays = (decimalArray[16] - '0') + (decimalArray[15] - '0') * 10 + (decimalArray[14] - '0') * 100;
        th_TrialStTime = mktime(&xTm);
        th_TrialEn = 0;
        DataSaveDirect();
    }
}

static void prvSend(uint8_t *pucData, uint8_t ucDataSize, uint8_t ucCmd, void *pvInfo)
{
    uint8_t *pucBuffer = GET_SEND_BUFFER();
    Head_t *pxHead     = GET_HEAD_BUFFER();
    pxHead->usStart    = 0x7E7E;
    pxHead->ucLength   = sizeof(Head_t) + ucDataSize + sizeof(Tail_t);
    pxHead->ucCmd      = ucCmd;
    pxHead->ucSrcAddr  = 1;
    pxHead->ucDstAddr  = 0;
    Tail_t *pxTail     = (Tail_t*)GET_TAIL_BUFFER(pxHead);
    pxTail->ucCheck    = 0;
    pxTail->usEnd      = 0x0D0A;
    
    uint8_t ucChk = 0;
    for (uint32_t n = 0; n < pxHead->ucLength; n++) {
        ucChk += *(pucBuffer + n);
    }
    ucChk              = ~ucChk;
    pxTail->ucCheck    = ucChk;
    
    if (CHAN_COM == (uint32_t)pvInfo) {
        UartBlkSend(s_xUart, pucBuffer, pxHead->ucLength, 10);
    }
    else if (CHAN_NET == (uint32_t)pvInfo) {
        send(s_xCliSock, pucBuffer, pxHead->ucLength, 0);
    }
}

static void prvSendReply(uint8_t ucStatus, void *pvInfo)
{
    uint8_t ucDataSize = sizeof(RCmdReply_t);
    RCmdReply_t *pxData = (RCmdReply_t*)GET_CONT_BUFFER();
    pxData->ucStatus = ucStatus;
    prvSend(GET_CONT_BUFFER(), ucDataSize, rCmdReply, pvInfo);
}

static void prvSendStatusInfo(void *pvInfo)
{
    uint8_t ucDataSize = sizeof(RCmdStatusInfo_t);
    RCmdStatusInfo_t *pxData = (RCmdStatusInfo_t*)GET_CONT_BUFFER();
    
    pxData->ucFsm              = (uint8_t)th_Fsm;
    
    pxData->xDi._MPWR_STAT_AC  = GpioGetInput(MPWR_M_STAT_AC);
    pxData->xDi._MPWR_STAT_DC  = GpioGetInput(MPWR_M_STAT_DC);
    pxData->xDi._APWR1_STAT    = GpioGetInput(APWR1_STAT);
    pxData->xDi._APWR2_STAT    = GpioGetInput(APWR2_STAT);
    pxData->xDi._APWR3_STAT    = GpioGetInput(APWR3_STAT);
    pxData->xDi._QBH_ON        = GpioGetInput(QBH_ON);
    pxData->xDi._EX_CTRL_EN      = GpioGetInput(EX_CTRL_EN);
    pxData->xDi._WATER_PRESS   = GpioGetInput(WATER_PRESS);
    pxData->xDi._WATER_CHILLER = GpioGetInput(WATER_CHILLER);

    pxData->xDo._MPWR_EN       = GpioGetOutput(MPWR_M_EN);
    pxData->xDo._APWR1_EN      = GpioGetOutput(APWR1_EN);
    pxData->xDo._APWR2_EN      = GpioGetOutput(APWR2_EN);
    pxData->xDo._APWR3_EN      = GpioGetOutput(APWR3_EN);
    //pxData->xDo._LED_CTRL      = GpioGetOutput(LED_CTRL);
    pxData->xDo._LED_CTRL      = 0;
    pxData->xDo._LED1          = GpioGetOutput(LED1);
    pxData->xDo._LED2          = GpioGetOutput(LED2);
    pxData->xDo._LED3          = GpioGetOutput(LED3);
    pxData->xDo._LED4          = GpioGetOutput(LED4);
    pxData->xDo._LED5          = GpioGetOutput(LED5);
    pxData->xDo._LED6          = GpioGetOutput(LED6);
    
    pxData->usTempH            = StcGetTempH();
    
    pxData->usAdc[0]           = AdcGet(ADC_CHAN_1);
    pxData->usAdc[1]           = AdcGet(ADC_CHAN_2);
    pxData->usAdc[2]           = AdcGet(ADC_CHAN_3);
    pxData->usAdc[3]           = AdcGet(ADC_CHAN_4);
    pxData->usAdc[4]           = AdcGet(ADC_CHAN_5);
    pxData->usAdc[5]           = AdcGet(ADC_CHAN_6);
    pxData->usAdc[6]           = AdcGet(ADC_CHAN_7);    /* 漏光检测1 */
    pxData->usAdc[7]           = AdcGet(ADC_CHAN_7);    /* 漏光检测2 */
    pxData->usAdc[8]           = AdcGet(ADC_CHAN_7);    /* 漏光检测3 */
    pxData->usAdc[9]           = AdcGet(ADC_CHAN_8);
    
    pxData->usDac[0]           = DacGet(DAC_CHAN_1);
    pxData->usDac[1]           = DacGet(DAC_CHAN_2);
    pxData->usDac[2]           = DacGet(DAC_CHAN_3);
    
    pxData->usAdc[0]           = (pxData->usAdc[0] >= 3200) ? 0 : pxData->usAdc[0];
    pxData->usAdc[2]           = (pxData->usAdc[2] >= 3200) ? 0 : pxData->usAdc[2];
    pxData->usAdc[4]           = (pxData->usAdc[4] >= 3200) ? 0 : pxData->usAdc[4];
    
    pxData->usMPwrVol          = PwrDataGet(PWR_M_ADDR, PWR_OUTPUT_VOL);
    
    pxData->usTemp[0]          = StcGetTemp(STC_DEV_1, STC_TEMP_NODE_1);
    pxData->usTemp[1]          = StcGetTemp(STC_DEV_1, STC_TEMP_NODE_2);
    pxData->usTemp[2]          = StcGetTemp(STC_DEV_1, STC_TEMP_NODE_3);
    pxData->usTemp[3]          = StcGetTemp(STC_DEV_1, STC_TEMP_NODE_4);
    pxData->usTemp[4]          = StcGetTemp(STC_DEV_1, STC_TEMP_NODE_5);
    pxData->usTemp[5]          = StcGetTemp(STC_DEV_1, STC_TEMP_NODE_6);
    pxData->usTemp[6]          = StcGetTemp(STC_DEV_1, STC_TEMP_NODE_7);
    pxData->usTemp[7]          = StcGetTemp(STC_DEV_1, STC_TEMP_NODE_8);
    pxData->usTemp[8]          = StcGetTemp(STC_DEV_1, STC_TEMP_NODE_9);
    pxData->usTemp[9]          = StcGetTemp(STC_DEV_1, STC_TEMP_NODE_10);
    
    pxData->usSysStatus        = th_SysStatusAll;
    pxData->ulPwrStatus        = PwrDataGet(PWR_M_ADDR, PWR_STATUS);
    pxData->usSwInfo           = th_SwInfo;
    
    prvSend(GET_CONT_BUFFER(), ucDataSize, rCmdStatusInfo, pvInfo);
}

static void prvSendDiagInfo(void *pvInfo)
{
    uint8_t ucDataSize = sizeof(RCmdDiagInfo_t);
    RCmdDiagInfo_t *pxData = (RCmdDiagInfo_t*)GET_CONT_BUFFER();
    pxData->ulSwVer   = SW_VER;
    pxData->ulRunTime = th_RunTime;
    prvSend(GET_CONT_BUFFER(), ucDataSize, rCmdDiagInfo, pvInfo);
}

static void prvSendSysPara(void *pvInfo)
{
    uint8_t ucDataSize = sizeof(RCmdSysPara_t);
    RCmdSysPara_t *pxData = (RCmdSysPara_t*)GET_CONT_BUFFER();
    pxData->sOtWarnTh   = AdcToTemp(th_OtWarnTh);
    pxData->sOtCutTh    = AdcToTemp(th_OtCutTh);
    pxData->ulMaxCur    = th_MaxCur;
    pxData->ulWorkCur   = th_WorkCur;
    pxData->lCompRate   = th_CompRate;
    pxData->usPdWarnL1  = th_PdWarnL1 * 3300 / 4096;
    pxData->usPdWarnL2  = th_PdWarnL2 * 3300 / 4096;
    pxData->ulAdVolPara = th_AdVolPara;
    pxData->ucPrType    = th_PwrType;
    pxData->ucTempNum   = th_TempNum;
    pxData->usModEn     = th_ModEnAll;
    prvSend(GET_CONT_BUFFER(), ucDataSize, rCmdSysPara, pvInfo);
}

static Status_t prvProtPktProc(const void *pvHead, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo)
{
    Head_t *p = (Head_t*)pvHead;
    
    /* Check source address */
    if (p->ucSrcAddr != 0) {
        TRACE("Com err: src addr\n");
        return STATUS_ERR;
    }
    
    /* Check destination address */
    if (p->ucDstAddr != 1) {
        TRACE("Com err: dst addr\n");
        return STATUS_ERR;
    }
    
    /* Process command */
    switch (p->ucCmd) {
    case iCmdQueryInfo:
        prvCmdQueryInfo(p->ucSrcAddr, pucCont, ulLength, pvInfo);
        break;
    case iCmdSysReset:
        prvCmdSysReset(p->ucSrcAddr, pucCont, ulLength, pvInfo);
        break;
    case iCmdParaConfig:
        prvCmdParaConfig(p->ucSrcAddr, pucCont, ulLength, pvInfo);
        break;
    case iCmdModuleCtrl:
        prvCmdModuleCtrl(p->ucSrcAddr, pucCont, ulLength, pvInfo);
        break;
    case iCmdUpgradeEnable:
        prvCmdUpgradeEnable(p->ucSrcAddr, pucCont, ulLength, pvInfo);
        break;
    case iCmdCli:
        prvCmdCli(p->ucSrcAddr, pucCont, ulLength, pvInfo);
        break;
    case iCmdEncrypt:
        prvCmdEncrypt(p->ucSrcAddr, pucCont, ulLength, pvInfo);
        break;
    default:
        break;
    }
    
    return STATUS_OK;
}

static Bool_t prvProtPktChk(const void *pvStart, uint32_t ulLength)
{
    uint8_t ucSum = 0;
    
    for (uint32_t n = 0; n < ulLength; n++) {
        ucSum += *((uint8_t*)pvStart + n);
    }
    
    return (0xFF == ucSum) ? TRUE : FALSE;
}

static Status_t prvUartRecv(uint8_t *pucBuf, uint16_t usLength, void* pvIsrPara)
{
    return (usLength == RbufWrite(s_xRbuf, pucBuf, usLength)) ? STATUS_OK : STATUS_ERR;
}

static void prvCliUartPrintf(const char* cFormat, ...)
{
    va_list va;
    va_start(va, cFormat);
    uint16_t usLength = vsprintf((char*)GET_CONT_BUFFER(), cFormat, va);
    if (usLength) {
        prvSend(GET_CONT_BUFFER(), (uint8_t)usLength, rCmdCli, (void*)CHAN_COM);
    }
}

void USART1_IRQHandler(void)
{
    if (s_xUart) {
        UartIsr(s_xUart);
    }
}

void DMA1_Channel5_IRQHandler(void)
{
    if (s_xUart){
        UartDmaRxIsr(s_xUart);
    }
}

void DMA1_Channel4_IRQHandler(void)
{
    if (s_xUart) {
        UartDmaTxIsr(s_xUart);
    }
}

#if ENABLE_RS485_TEST
static Status_t prvUartRecvRs485(uint8_t *pucBuf, uint16_t usLength, void* pvIsrPara)
{
    if (s_bUartRs485Recv) {
        /* Enable RS485 write */
        RS485_WT();
        UartBlkSend(s_xUartRs485, pucBuf, usLength, 10);
        /* Enable RS485 read */
        RS485_RD();
    }
    return STATUS_OK;
}
void UART5_IRQHandler(void)
{
    if (s_xUartRs485) {
        UartIsr(s_xUartRs485);
    }
}

static uint8_t* prvParseHexStr(const char* pcStr, uint8_t *pucLength)
{
    static uint8_t ucBuffer[80];
    uint8_t ucData = 0;
    uint8_t ucCnt = 0;
    uint8_t ucIndex = 0;
    Bool_t bProc = FALSE;
    while ((0 != *pcStr) && (ucIndex < 80)) {
        char cTmp;
        cTmp = *pcStr++;
        if ((cTmp >= 'a') && (cTmp <= 'f')) {
            ucData *= 16;
            ucData += 10 + (cTmp - 'a');
            ucCnt++;
            bProc = TRUE;
        }
        if ((cTmp >= 'A') && (cTmp <= 'F')) {
            ucData *= 16; 
            ucData += 10 + (cTmp - 'A');
            ucCnt++;
            bProc = TRUE;
        }
        if ((cTmp >= '0') && (cTmp <= '9')) {
            ucData *= 16;
            ucData += cTmp - '0';
            ucCnt++;
            bProc = TRUE;
        }
        if ((TRUE == bProc) && ((' ' == cTmp) || (',' == cTmp) || ('|' == cTmp) || (2 == ucCnt) || (0 == *pcStr))) {
            ucBuffer[ucIndex++] = ucData;
            ucData = 0;
            ucCnt = 0;
            bProc = FALSE;
        }
    }
    *pucLength = ucIndex;
    return ucBuffer;
}

static void prvCliCmdRs485Send(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    if (argc < 2) {
        cliprintf("rs485_send DATA(HEX)\n");
        return;
    }
    
    uint8_t ucLength = 0;
    uint8_t *pucBuffer = prvParseHexStr(argv[1], &ucLength);
    
    /* Enable RS485 write */
    RS485_WT();
    
    UartBlkSend(s_xUartRs485, pucBuffer, ucLength, 10);
    
    /* Enable RS485 read */
    RS485_RD();
    
    s_bUartRs485Recv = TRUE;
    
    return;
}
CLI_CMD_EXPORT(rs485_send, rs485 send, prvCliCmdRs485Send)
#endif /* ENABLE_RS485_TEST */

#endif /* SET_COM_SEND_PTL */
