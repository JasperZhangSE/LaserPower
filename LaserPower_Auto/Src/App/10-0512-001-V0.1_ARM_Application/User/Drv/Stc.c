/*
    Stc.c

    Implementation File for Stc Module
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 10Nov23, Karl Created
    01b, 15Nov23, Karl Added StcGetTemp
    01c, 15Nov23, Karl Optimized prvStcTask
    01d, 23Nov23, Karl Added prvGetTemp
    01e, 30Nov23, Karl Added AdcToTemp
    01f, 04Dec23, Karl Added StcGetTempH and StcGetTempL
    01g, 17Jan24, Karl Added StcGetTempHFrom
*/

/* Includes */
#include "Include.h"

/* Pragmas */
#pragma diag_suppress 177 /* warning: #177-D: function "FUNC" was set but never used */

/* Debug config */
#if STC_DEBUG
#undef TRACE
#define TRACE(...) DebugPrintf(__VA_ARGS__)
#else
#undef TRACE
#define TRACE(...)
#endif /* STC_DEBUG */
#if STC_ASSERT
#undef ASSERT
#define ASSERT(a)                                                                                                      \
    while (!(a)) {                                                                                                     \
        DebugPrintf("ASSERT failed: %s %d\n", __FILE__, __LINE__);                                                     \
    }
#else
#undef ASSERT
#define ASSERT(...)
#endif /* STC_ASSERT */

/* Local defines */
#define PD_REVERSE   1
#define DEV_NUM      3
#define MAX_MSG_SIZE 128
#define RS485_RD()   GpioSetOutput(RS485a_EN, 1)
#define RS485_WT()   GpioSetOutput(RS485a_EN, 0)
static int16_t s_xStcValue[30] = {0};

/* Local types */
#pragma pack(push)
#pragma pack(1)
enum {
    iCmdQueryInfo = 0x01,
    iCmdSysReset  = 0x02,
    rCmdTempInfo  = 0x81,
    rCmdDiagInfo  = 0x82,
};

typedef struct {
    uint16_t usStart;
    uint8_t  ucLength;
    uint8_t  ucCmd;
    uint8_t  ucSrcAddr;
    uint8_t  ucDstAddr;
} Head_t;

typedef struct {
    uint8_t  ucCheck;
    uint16_t usEnd;
} Tail_t;

typedef struct {
    int16_t sTemp[10]; /* Raw AD value */
} TempInfo_t;

typedef struct {
    uint32_t ulSwVer;
    uint32_t ulRunTime;
    uint32_t ulAdcErrCnt;
} DiagInfo_t;
#pragma pack(pop)

/* Forward declarations */
static void     prvStcTask(void *pvPara);
static void     prvSendCmdQueryInfo(uint8_t ucAddr, uint8_t ucCmd);
static void     prvSendCmdSysReset(uint8_t ucAddr);
static void     prvCmdTempInfo(uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength);
static void     prvCmdDiagInfo(uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength);
static Status_t prvProtPktProc(const void *pvHead, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo);
static Bool_t   prvProtPktChk(const void *pvStart, uint32_t ulLength);
static Status_t prvUartRecv(uint8_t *pucBuf, uint16_t usLength, void *pvIsrPara);
static int16_t  prvGetTemp(uint16_t usAdc);

/* Local variables */
static UartHandle_t s_xUart = NULL;
static ProtHandle_t s_xProt = NULL;
static TempInfo_t   s_xTemp[DEV_NUM];
static DiagInfo_t   s_xDiag[DEV_NUM];
static Bool_t       s_bQueryDiagInfo = FALSE;

/* Functions */
Status_t DrvStcInit(void) {
    uint8_t ucHeadMark[] = {0x7E, 0x7E};
    uint8_t ucTailMark[] = {0x0A, 0x0D};

    s_xUart              = NULL;
    s_xProt              = NULL;
    memset(s_xTemp, 0, sizeof(s_xTemp));
    memset(s_xDiag, 0, sizeof(s_xDiag));

    ProtInit();
    s_xProt = ProtCreate();
    ProtConfigHead(s_xProt, ucHeadMark, sizeof(ucHeadMark), sizeof(Head_t));
    ProtConfigTail(s_xProt, ucTailMark, sizeof(ucTailMark), sizeof(Tail_t));
    ProtConfigMisc(s_xProt, MAX_MSG_SIZE, 0 /* Msg length offset */, PROT_LENGTH_UINT8, 2 /* Head mark size */);
    ProtConfigCb(s_xProt, prvProtPktProc, prvProtPktChk);
    ProtConfig(s_xProt);

    UartInit();
    s_xUart = UartCreate();
    UartConfigCb(s_xUart, prvUartRecv, UartIsrCb, UartDmaRxIsrCb, UartDmaTxIsrCb, NULL);
    UartConfigRxDma(s_xUart, DMA1_Channel6, DMA1_Channel6_IRQn);
    UartConfigTxDma(s_xUart, DMA1_Channel7, DMA1_Channel7_IRQn);
    UartConfigCom(s_xUart, USART2, 115200, USART2_IRQn);

    xTaskCreate(prvStcTask, "tStc", 256, NULL, tskIDLE_PRIORITY, NULL);

    RS485_RD();

    return STATUS_OK;
}

Status_t DrvStcTerm(void) {
    /* Do nothing */
    return STATUS_OK;
}

int16_t StcGetTemp(StcDev_t xDev, StcTempNode_t xTempNode) {
    return s_xTemp[xDev].sTemp[xTempNode];
}

int16_t StcGetTempH(void) {
    int16_t sTempH = 687;

#if STC_EN_DEV1
    for (uint8_t n = 0; n < 10; n++) {
        if (sTempH > s_xTemp[0].sTemp[n]) {
            sTempH = s_xTemp[0].sTemp[n];
        }
    }
#endif /* STC_EN_DEV1 */

#if STC_EN_DEV2
    for (uint8_t n = 0; n < 2; n++) {
        if (sTempH > s_xTemp[1].sTemp[n]) {
            sTempH = s_xTemp[1].sTemp[n];
        }
    }
#endif /* STC_EN_DEV2 */

#if STC_EN_DEV3
    for (uint8_t n = 0; n < 4; n++) {
        if (sTempH > s_xTemp[2].sTemp[n]) {
            sTempH = s_xTemp[2].sTemp[n];
        }
    }
#endif /* STC_EN_DEV3 */

    return sTempH;
}

int16_t StcGetTempL(void) {
    int16_t sTempL = 2;

#if STC_EN_DEV1
    for (uint8_t n = 0; n < 10; n++) {
        if (sTempL < s_xTemp[0].sTemp[n]) {
            sTempL = s_xTemp[0].sTemp[n];
        }
    }
#endif /* STC_EN_DEV1 */

#if STC_EN_DEV2
    for (uint8_t n = 0; n < 10; n++) {
        if (sTempL < s_xTemp[1].sTemp[n]) {
            sTempL = s_xTemp[1].sTemp[n];
        }
    }
#endif /* STC_EN_DEV2 */

#if STC_EN_DEV3
    for (uint8_t n = 0; n < 4; n++) {
        if (sTempL < s_xTemp[2].sTemp[n]) {
            sTempL = s_xTemp[2].sTemp[n];
        }
    }
#endif /* STC_EN_DEV3 */

    return sTempL;
}

int16_t StcGetTempHFrom(uint8_t st, uint8_t ed) {
    int16_t sTempH = 687;
    uint8_t index  = 0;
#if 1
    for (uint8_t i = 0; i <= 2; i++) {
        for (uint8_t n = 0; n <= 9; n++) {
            s_xStcValue[index++] = s_xTemp[i].sTemp[n];
        }
    }
#endif

    for (index = st; index <= ed; index++) {
        if (sTempH > s_xStcValue[index]) {
            sTempH = s_xStcValue[index];
        }
    }

    return sTempH;
}

int16_t StcGetPdHFrom(void) {
    int16_t sPdH = 0;

    for (uint8_t index = 12; index <= 13; index++) {
        if (sPdH < s_xStcValue[index]) {
            sPdH = s_xStcValue[index];
        }
    }

    return sPdH;
}

int16_t StcGetPdLight(void) {
    int16_t sPdH = 0;
    sPdH = s_xStcValue[14];
    return sPdH;
}

int16_t AdcToTemp(uint16_t usAdc) {
    return prvGetTemp(usAdc);
}

static void prvStcTask(void *pvPara) {
#define INTERVAL 10
    uint16_t n = 0;
    while (1) {
        uint16_t offset = 0;
        uint16_t delay  = STC_QUERY_TASK_DELAY;

        /* Query temp info */
        if ((n % (STC_QUERY_TEMP_PRD / STC_QUERY_TASK_DELAY)) == 0) {
#if STC_EN_DEV1
            prvSendCmdQueryInfo(1, rCmdTempInfo);
            osDelay(INTERVAL);
            offset += INTERVAL;
#endif /* STC_EN_DEV1 */
#if STC_EN_DEV2
            prvSendCmdQueryInfo(2, rCmdTempInfo);
            osDelay(INTERVAL);
            offset += INTERVAL;
#endif /* STC_EN_DEV2 */
#if STC_EN_DEV3
            prvSendCmdQueryInfo(3, rCmdTempInfo);
            osDelay(INTERVAL);
            offset += INTERVAL;
#endif /* STC_EN_DEV3 */
        }

        /* Query diag info */
#if 0 /* Periodic query */
        if ((n % (STC_QUERY_DIAG_PRD / STC_QUERY_TASK_DELAY)) == 0) {
#if STC_EN_DEV1
            prvSendCmdQueryInfo(1, rCmdDiagInfo);
            osDelay(INTERVAL);
            offset += INTERVAL;
#endif /* STC_EN_DEV1 */
#if STC_EN_DEV2
            prvSendCmdQueryInfo(2, rCmdDiagInfo);
            osDelay(INTERVAL);
            offset += INTERVAL;
#endif /* STC_EN_DEV2 */
#if STC_EN_DEV3
            prvSendCmdQueryInfo(3, rCmdDiagInfo);
            osDelay(INTERVAL);
            offset += INTERVAL;
#endif /* STC_EN_DEV3 */
        }
#endif
#if 1 /* Oneshot query */
        if (s_bQueryDiagInfo) {
#if STC_EN_DEV1
            prvSendCmdQueryInfo(1, rCmdDiagInfo);
            osDelay(INTERVAL);
            offset += INTERVAL;
#endif /* STC_EN_DEV1 */
#if STC_EN_DEV2
            prvSendCmdQueryInfo(2, rCmdDiagInfo);
            osDelay(INTERVAL);
            offset += INTERVAL;
#endif /* STC_EN_DEV2 */
#if STC_EN_DEV3
            prvSendCmdQueryInfo(3, rCmdDiagInfo);
            osDelay(INTERVAL);
            offset += INTERVAL;
#endif /* STC_EN_DEV3 */
            s_bQueryDiagInfo = FALSE;
        }
#endif
        n++;
        (offset > STC_QUERY_TASK_DELAY) ? (delay = 0) : (delay -= offset);
        osDelay(delay);
    }
}

static void prvSendCmdQueryInfo(uint8_t ucAddr, uint8_t ucCmd) {
#define LEN1 10
    static uint8_t c[LEN1];
    uint8_t        i = 0, n = 0, sum = 0;

    /* Enable RS485 write */
    RS485_WT();

    /* Head */
    c[i++] = 0x7E;
    c[i++] = 0x7E;
    c[i++] = LEN1;
    c[i++] = iCmdQueryInfo;
    c[i++] = 0;
    c[i++] = ucAddr;
    /* Content */
    c[i++] = ucCmd;
    /* Tail */
    c[i++] = 0;
    c[i++] = 0x0A;
    c[i++] = 0x0D;
    for (n = 0; n < LEN1; n++) {
        sum += c[n];
    }
    sum  = 0xFF - sum;
    c[7] = sum;

    UartBlkSend(s_xUart, c, LEN1, 50);

    /* Enable RS485 read */
    RS485_RD();
}

static void prvSendCmdSysReset(uint8_t ucAddr) {
#define LEN2 13
    static uint8_t c[LEN2];
    uint8_t        i = 0, n = 0, sum = 0;

    /* Enable RS485 write */
    RS485_WT();

    /* Head */
    c[i++] = 0x7E;
    c[i++] = 0x7E;
    c[i++] = LEN2;
    c[i++] = iCmdSysReset;
    c[i++] = 0;
    c[i++] = ucAddr;
    /* Content */
    c[i++] = 0xCD;
    c[i++] = 0xAB;
    c[i++] = 0x34;
    c[i++] = 0x12;
    /* Tail */
    c[i++] = 0;
    c[i++] = 0x0A;
    c[i++] = 0x0D;
    for (n = 0; n < LEN2; n++) {
        sum += c[n];
    }
    sum   = 0xFF - sum;
    c[10] = sum;

    UartBlkSend(s_xUart, c, LEN2, 50);

    /* Enable RS485 read */
    RS485_RD();
}

static void prvCmdTempInfo(uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength) {
    TRACE("rCmdTempInfo\n");

    if (ulLength != sizeof(TempInfo_t)) {
        TRACE("    Wrong length\n");
        return;
    }

    memcpy(&s_xTemp[ucSrcAddr - 1], pucCont, ulLength);
}

static void prvCmdDiagInfo(uint8_t ucSrcAddr, const uint8_t *pucCont, uint32_t ulLength) {
    TRACE("rCmdDiagInfo\n");

    if (ulLength != sizeof(DiagInfo_t)) {
        TRACE("    Wrong length\n");
        return;
    }

    memcpy(&s_xDiag[ucSrcAddr - 1], pucCont, ulLength);
}

static Status_t prvProtPktProc(const void *pvHead, const uint8_t *pucCont, uint32_t ulLength, void *pvInfo) {
    Head_t *p = (Head_t *)pvHead;

    /* Check source address */
    if ((p->ucSrcAddr < 1) || (3 < p->ucSrcAddr)) {
        TRACE("Com err: src addr\n");
        return STATUS_ERR;
    }

    /* Check destination address */
    if (p->ucDstAddr != 0) {
        TRACE("Com err: dst addr\n");
        return STATUS_ERR;
    }

    /* Process command */
    switch (p->ucCmd) {
    case rCmdTempInfo:
        prvCmdTempInfo(p->ucSrcAddr, pucCont, ulLength);
        break;
    case rCmdDiagInfo:
        prvCmdDiagInfo(p->ucSrcAddr, pucCont, ulLength);
        break;
    default:
        break;
    }

    return STATUS_OK;
}

static Bool_t prvProtPktChk(const void *pvStart, uint32_t ulLength) {
    uint8_t ucSum = 0;

    for (uint32_t n = 0; n < ulLength; n++) {
        ucSum += *((uint8_t *)pvStart + n);
    }

    return (0xFF == ucSum) ? TRUE : FALSE;
}

static Status_t prvUartRecv(uint8_t *pucBuf, uint16_t usLength, void *pvIsrPara) {
    static uint16_t s_usRecvIndex = 0;
    static uint8_t  s_ucProcBuf[MAX_MSG_SIZE];
    return ProtProc(s_xProt, pucBuf, usLength, &s_usRecvIndex, s_ucProcBuf, NULL);
}

void USART2_IRQHandler(void) {
    if (s_xUart) {
        UartIsr(s_xUart);
    }
}

void DMA1_Channel6_IRQHandler(void) {
    if (s_xUart) {
        UartDmaRxIsr(s_xUart);
    }
}

void DMA1_Channel7_IRQHandler(void) {
    if (s_xUart) {
        UartDmaTxIsr(s_xUart);
    }
}

#if 0
static void prvCliCmdStcTemp(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    int i = 0xFF;
    
    if (argc >= 2) {
        i = atoi(argv[1]);
    }
    
    if (i == 1)
    {
        cliprintf("TEMP-G1:\n");
        cliprintf("    T1  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[0]));
        cliprintf("    T2  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[1]));
        cliprintf("    T3  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[2]));
        cliprintf("    T4  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[3]));
        cliprintf("    T5  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[4]));
        cliprintf("    T6  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[5]));
        cliprintf("    T7  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[6]));
        cliprintf("    T8  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[7]));
        
        cliprintf("TEMP-G2:\n");
        cliprintf("    T9  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[8]));
        cliprintf("    T10 : %4d\n", prvGetTemp(s_xTemp[0].sTemp[9]));
        cliprintf("    T11 : %4d\n", prvGetTemp(s_xTemp[1].sTemp[0]));
        cliprintf("    T12 : %4d\n", prvGetTemp(s_xTemp[1].sTemp[1]));
        cliprintf("    T13 : %4d\n", prvGetTemp(s_xTemp[1].sTemp[2]));
        cliprintf("    T14 : %4d\n", prvGetTemp(s_xTemp[1].sTemp[3]));
        cliprintf("    T15 : %4d\n", prvGetTemp(s_xTemp[1].sTemp[4]));
        cliprintf("    T16 : %4d\n", prvGetTemp(s_xTemp[1].sTemp[5]));
        
        cliprintf("TEMP-G3:\n");
        cliprintf("    T17 : %4d\n", prvGetTemp(s_xTemp[1].sTemp[6]));
        cliprintf("    T18 : %4d\n", prvGetTemp(s_xTemp[1].sTemp[7]));
        cliprintf("    T19 : %4d\n", prvGetTemp(s_xTemp[1].sTemp[8]));
        cliprintf("    T20 : %4d\n", prvGetTemp(s_xTemp[1].sTemp[9]));
        cliprintf("    T21 : %4d\n", prvGetTemp(s_xTemp[2].sTemp[0]));
        cliprintf("    T22 : %4d\n", prvGetTemp(s_xTemp[2].sTemp[1]));
        cliprintf("    T23 : %4d\n", prvGetTemp(s_xTemp[2].sTemp[2]));
        cliprintf("    T24 : %4d\n", prvGetTemp(s_xTemp[2].sTemp[3]));
    }
    else if (i == 2)
    {
#if PD_REVERSE
        cliprintf("PD-G1:\n");
        cliprintf("    PD1 : %4d\n", s_xTemp[2].sTemp[4]);
        cliprintf("    PD2 : %4d\n", s_xTemp[2].sTemp[5]);
        cliprintf("    PD3 : %4d\n", s_xTemp[2].sTemp[9]);
        cliprintf("    PD4 : %4d\n", s_xTemp[2].sTemp[8]);
        cliprintf("    PD5 : %4d\n", s_xTemp[2].sTemp[7]);
        cliprintf("    PD6 : %4d\n", s_xTemp[2].sTemp[6]);
#else
        cliprintf("PD-G1:\n");
        cliprintf("    PD1 : %4d\n", s_xTemp[2].sTemp[4]);
        cliprintf("    PD2 : %4d\n", s_xTemp[2].sTemp[5]);
        cliprintf("    PD3 : %4d\n", s_xTemp[2].sTemp[6]);
        cliprintf("    PD4 : %4d\n", s_xTemp[2].sTemp[7]);
        cliprintf("    PD5 : %4d\n", s_xTemp[2].sTemp[8]);
        cliprintf("    PD6 : %4d\n", s_xTemp[2].sTemp[9]);
#endif
    }
    else
    {
        cliprintf("TEMP-G1:\n");
        cliprintf("    T1  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[0]));
        cliprintf("    T2  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[1]));
        cliprintf("    T3  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[2]));
        cliprintf("    T4  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[3]));
        cliprintf("    T5  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[4]));
        cliprintf("    T6  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[5]));
        cliprintf("    T7  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[6]));
        cliprintf("    T8  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[7]));
        
        cliprintf("TEMP-G2:\n");
        cliprintf("    T9  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[8]));
        cliprintf("    T10 : %4d\n", prvGetTemp(s_xTemp[0].sTemp[9]));
        cliprintf("    T11 : %4d\n", prvGetTemp(s_xTemp[1].sTemp[0]));
        cliprintf("    T12 : %4d\n", prvGetTemp(s_xTemp[1].sTemp[1]));
        cliprintf("    T13 : %4d\n", prvGetTemp(s_xTemp[1].sTemp[2]));
        cliprintf("    T14 : %4d\n", prvGetTemp(s_xTemp[1].sTemp[3]));
        cliprintf("    T15 : %4d\n", prvGetTemp(s_xTemp[1].sTemp[4]));
        cliprintf("    T16 : %4d\n", prvGetTemp(s_xTemp[1].sTemp[5]));
        
        cliprintf("TEMP-G3:\n");
        cliprintf("    T17 : %4d\n", prvGetTemp(s_xTemp[1].sTemp[6]));
        cliprintf("    T18 : %4d\n", prvGetTemp(s_xTemp[1].sTemp[7]));
        cliprintf("    T19 : %4d\n", prvGetTemp(s_xTemp[1].sTemp[8]));
        cliprintf("    T20 : %4d\n", prvGetTemp(s_xTemp[1].sTemp[9]));
        cliprintf("    T21 : %4d\n", prvGetTemp(s_xTemp[2].sTemp[0]));
        cliprintf("    T22 : %4d\n", prvGetTemp(s_xTemp[2].sTemp[1]));
        cliprintf("    T23 : %4d\n", prvGetTemp(s_xTemp[2].sTemp[2]));
        cliprintf("    T24 : %4d\n", prvGetTemp(s_xTemp[2].sTemp[3]));
        cliprintf("\n");
        
        cliprintf("PD-G1:\n");
#if PD_REVERSE
        cliprintf("PD-G1:\n");
        cliprintf("    PD1 : %4d\n", s_xTemp[2].sTemp[4]);
        cliprintf("    PD2 : %4d\n", s_xTemp[2].sTemp[5]);
        cliprintf("    PD3 : %4d\n", s_xTemp[2].sTemp[9]);
        cliprintf("    PD4 : %4d\n", s_xTemp[2].sTemp[8]);
        cliprintf("    PD5 : %4d\n", s_xTemp[2].sTemp[7]);
        cliprintf("    PD6 : %4d\n", s_xTemp[2].sTemp[6]);
#else
        cliprintf("PD-G1:\n");
        cliprintf("    PD1 : %4d\n", s_xTemp[2].sTemp[4]);
        cliprintf("    PD2 : %4d\n", s_xTemp[2].sTemp[5]);
        cliprintf("    PD3 : %4d\n", s_xTemp[2].sTemp[6]);
        cliprintf("    PD4 : %4d\n", s_xTemp[2].sTemp[7]);
        cliprintf("    PD5 : %4d\n", s_xTemp[2].sTemp[8]);
        cliprintf("    PD6 : %4d\n", s_xTemp[2].sTemp[9]);
#endif
    }

#if 0
    if ((i >= 1) && (i <= DEV_NUM)) {
        i -= 1;
        cliprintf("STC-%d:\n", i + 1);
        cliprintf("    T1 : %4d\n", prvGetTemp(s_xTemp[i].sTemp[0]));
        cliprintf("    T2 : %4d\n", prvGetTemp(s_xTemp[i].sTemp[1]));
        cliprintf("    T3 : %4d\n", prvGetTemp(s_xTemp[i].sTemp[2]));
        cliprintf("    T4 : %4d\n", prvGetTemp(s_xTemp[i].sTemp[3]));
        cliprintf("    T5 : %4d\n", prvGetTemp(s_xTemp[i].sTemp[4]));
        cliprintf("    T6 : %4d\n", prvGetTemp(s_xTemp[i].sTemp[5]));
        cliprintf("    T7 : %4d\n", prvGetTemp(s_xTemp[i].sTemp[6]));
        cliprintf("    T8 : %4d\n", prvGetTemp(s_xTemp[i].sTemp[7]));
        cliprintf("    T9 : %4d\n", prvGetTemp(s_xTemp[i].sTemp[8]));
        cliprintf("    T10: %4d\n", prvGetTemp(s_xTemp[i].sTemp[9]));
    }
    else {
        for (uint8_t n = 0; n < DEV_NUM; n++) {
            cliprintf("STC-%d:\n", n + 1);
            cliprintf("    T1 : %4d\n", prvGetTemp(s_xTemp[n].sTemp[0]));
            cliprintf("    T2 : %4d\n", prvGetTemp(s_xTemp[n].sTemp[1]));
            cliprintf("    T3 : %4d\n", prvGetTemp(s_xTemp[n].sTemp[2]));
            cliprintf("    T4 : %4d\n", prvGetTemp(s_xTemp[n].sTemp[3]));
            cliprintf("    T5 : %4d\n", prvGetTemp(s_xTemp[n].sTemp[4]));
            cliprintf("    T6 : %4d\n", prvGetTemp(s_xTemp[n].sTemp[5]));
            cliprintf("    T7 : %4d\n", prvGetTemp(s_xTemp[n].sTemp[6]));
            cliprintf("    T8 : %4d\n", prvGetTemp(s_xTemp[n].sTemp[7]));
            cliprintf("    T9 : %4d\n", prvGetTemp(s_xTemp[n].sTemp[8]));
            cliprintf("    T10: %4d\n", prvGetTemp(s_xTemp[n].sTemp[9]));
            cliprintf("\n");
        }
    }
#endif
    return;
}
CLI_CMD_EXPORT(stc_get_info, show stc sampled temperatures, prvCliCmdStcTemp)

#endif

static void prvCliCmdStcTemp(cli_printf cliprintf, int argc, char **argv) {
    CHECK_CLI();

    int i = 0xFF;

    if (argc >= 2) {
        i = atoi(argv[1]);
    }

    if (i == 1) {
        cliprintf("TEMP:\n");
        cliprintf("    T1  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[0]));
        cliprintf("    T2  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[1]));
        cliprintf("    T3  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[2]));
        cliprintf("    T4  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[3]));
        cliprintf("    T5  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[4]));
        cliprintf("    T6  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[5]));
        cliprintf("    T7  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[6]));
        cliprintf("    T8  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[7]));
        cliprintf("    T9  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[8]));
        cliprintf("    T10 : %4d\n", prvGetTemp(s_xTemp[0].sTemp[9]));
        cliprintf("    T11 : %4d\n", prvGetTemp(s_xTemp[1].sTemp[0]));
        cliprintf("    T12 : %4d\n", prvGetTemp(s_xTemp[1].sTemp[1]));
    }
    else if (i == 2) {
        cliprintf("PD:\n");
        cliprintf("    PD1 : %4d\n", prvGetTemp(s_xTemp[1].sTemp[2]));
        cliprintf("    PD2 : %4d\n", prvGetTemp(s_xTemp[1].sTemp[3]));
        cliprintf("    PD3 : %4d\n", prvGetTemp(s_xTemp[1].sTemp[4]));
    }
    else {
        cliprintf("TEMP:\n");
        cliprintf("    T1  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[0]));
        cliprintf("    T2  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[1]));
        cliprintf("    T3  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[2]));
        cliprintf("    T4  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[3]));
        cliprintf("    T5  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[4]));
        cliprintf("    T6  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[5]));
        cliprintf("    T7  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[6]));
        cliprintf("    T8  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[7]));
        cliprintf("    T9  : %4d\n", prvGetTemp(s_xTemp[0].sTemp[8]));
        cliprintf("    T10 : %4d\n", prvGetTemp(s_xTemp[0].sTemp[9]));
        cliprintf("    T11 : %4d\n", prvGetTemp(s_xTemp[1].sTemp[0]));
        cliprintf("    T12 : %4d\n", prvGetTemp(s_xTemp[1].sTemp[1]));
        cliprintf("\n");

        cliprintf("PD:\n");
        cliprintf("    PD1 : %4d\n", s_xTemp[1].sTemp[2]);
        cliprintf("    PD2 : %4d\n", s_xTemp[1].sTemp[3]);
        cliprintf("    PD3 : %4d\n", s_xTemp[1].sTemp[4]);
    }

    return;
}
CLI_CMD_EXPORT(stc_get_info, show stc sampled temperatures, prvCliCmdStcTemp)

static void prvCliCmdStcDiag(cli_printf cliprintf, int argc, char **argv) {
    CHECK_CLI();

    int i = 0xFF;

    if (argc >= 2) {
        i = atoi(argv[1]);
    }

    /* Enable oneshot query */
    s_bQueryDiagInfo = TRUE;
    osDelay(1000);

    if ((i >= 1) && (i <= DEV_NUM)) {
        i -= 1;
        cliprintf("STC-%d DIAG:\n", i + 1);
        cliprintf("    SwVer   : %d.%d.%d\n", (s_xDiag[i].ulSwVer >> 16) & 0xFF, (s_xDiag[i].ulSwVer >> 8) & 0xFF,
                  (s_xDiag[i].ulSwVer >> 0) & 0xFF);
        cliprintf("    RunTime : %ld\n", s_xDiag[i].ulRunTime);
        cliprintf("    AdErrCnt: %ld\n", s_xDiag[i].ulAdcErrCnt);
    }
    else {
        for (uint8_t n = 0; n < DEV_NUM; n++) {
            cliprintf("STC-%d DIAG:\n", n + 1);
            cliprintf("    SwVer   : %d.%d.%d\n", (s_xDiag[n].ulSwVer >> 16) & 0xFF, (s_xDiag[n].ulSwVer >> 8) & 0xFF,
                      (s_xDiag[n].ulSwVer >> 0) & 0xFF);
            cliprintf("    RunTime : %ld\n", s_xDiag[n].ulRunTime);
            cliprintf("    AdErrCnt: %ld\n", s_xDiag[n].ulAdcErrCnt);
            cliprintf("\n");
        }
    }

    return;
}
CLI_CMD_EXPORT(stc_diag, show stc diagnostic information, prvCliCmdStcDiag)

#define TEMP_TBL_SIZE 241
#define TEMP_TBL_CONT                                                                                                  \
    {                                                                                                                  \
        {687, -400}, {674, -390}, {660, -380}, {647, -370}, {633, -360}, {619, -350}, {605, -340}, {592, -330},        \
        {578, -320}, {564, -310}, {550, -300}, {537, -290}, {523, -280}, {510, -270}, {496, -260}, {483, -250},        \
        {470, -240}, {457, -230}, {444, -220}, {432, -210}, {419, -200}, {407, -190}, {395, -180}, {384, -170},        \
        {372, -160}, {361, -150}, {350, -140}, {339, -130}, {328, -120}, {318, -110}, {308, -100}, {298, -90},         \
        {288, -80},  {279, -70},  {270, -60},  {261, -50},  {253, -40},  {244, -30},  {236, -20},  {228, -10},         \
        {221, 0},    {213, 10},   {206, 20},   {199, 30},   {192, 40},   {186, 50},   {180, 60},   {173, 70},          \
        {168, 80},   {162, 90},   {156, 100},  {151, 110},  {146, 120},  {141, 130},  {136, 140},  {131, 150},         \
        {127, 160},  {123, 170},  {118, 180},  {114, 190},  {111, 200},  {107, 210},  {103, 220},  {100, 230},         \
        {96, 240},   {93, 250},   {90, 260},   {87, 270},   {84, 280},   {81, 290},   {79, 300},   {76, 310},          \
        {74, 320},   {71, 330},   {69, 340},   {67, 350},   {64, 360},   {62, 370},   {60, 380},   {58, 390},          \
        {56, 400},   {55, 410},   {53, 420},   {51, 430},   {50, 440},   {48, 450},   {46, 460},   {45, 470},          \
        {44, 480},   {42, 490},   {41, 500},   {40, 510},   {38, 520},   {37, 530},   {36, 540},   {35, 550},          \
        {34, 560},   {33, 570},   {32, 580},   {31, 590},   {30, 600},   {29, 610},   {28, 620},   {28, 630},          \
        {27, 640},   {26, 650},   {25, 660},   {24, 670},   {24, 680},   {23, 690},   {22, 700},   {22, 710},          \
        {21, 720},   {21, 730},   {20, 740},   {19, 750},   {19, 760},   {18, 770},   {18, 780},   {17, 790},          \
        {17, 800},   {16, 810},   {16, 820},   {16, 830},   {15, 840},   {15, 850},   {14, 860},   {14, 870},          \
        {14, 880},   {13, 890},   {13, 900},   {13, 910},   {12, 920},   {12, 930},   {12, 940},   {11, 950},          \
        {11, 960},   {11, 970},   {10, 980},   {10, 990},   {10, 1000},  {10, 1010},  {9, 1020},   {9, 1030},          \
        {9, 1040},   {9, 1050},   {9, 1060},   {8, 1070},   {8, 1080},   {8, 1090},   {8, 1100},   {8, 1110},          \
        {7, 1120},   {7, 1130},   {7, 1140},   {7, 1150},   {7, 1160},   {7, 1170},   {6, 1180},   {6, 1190},          \
        {6, 1200},   {6, 1210},   {6, 1220},   {6, 1230},   {6, 1240},   {6, 1250},   {5, 1260},   {5, 1270},          \
        {5, 1280},   {5, 1290},   {5, 1300},   {5, 1310},   {5, 1320},   {5, 1330},   {5, 1340},   {4, 1350},          \
        {4, 1360},   {4, 1370},   {4, 1380},   {4, 1390},   {4, 1400},   {4, 1410},   {4, 1420},   {4, 1430},          \
        {4, 1440},   {4, 1450},   {4, 1460},   {4, 1470},   {4, 1480},   {3, 1490},   {3, 1500},   {3, 1510},          \
        {3, 1520},   {3, 1530},   {3, 1540},   {3, 1550},   {3, 1560},   {3, 1570},   {3, 1580},   {3, 1590},          \
        {3, 1600},   {3, 1610},   {3, 1620},   {3, 1630},   {3, 1640},   {3, 1650},   {3, 1660},   {3, 1670},          \
        {3, 1680},   {3, 1690},   {2, 1700},   {2, 1710},   {2, 1720},   {2, 1730},   {2, 1740},   {2, 1750},          \
        {2, 1760},   {2, 1770},   {2, 1780},   {2, 1790},   {2, 1800},   {2, 1810},   {2, 1820},   {2, 1830},          \
        {2, 1840},   {2, 1850},   {2, 1860},   {2, 1870},   {2, 1880},   {2, 1890},   {2, 1900},   {2, 1910},          \
        {2, 1920},   {2, 1930},   {2, 1940},   {2, 1950},   {2, 1960},   {2, 1970},   {2, 1980},   {2, 1990},          \
        {2, 2000},                                                                                                     \
    }

typedef struct {
    uint16_t adc;
    int16_t  temp;
} TempTblItem_t;

static int16_t prvGetTemp(uint16_t adc) {
    static TempTblItem_t T[TEMP_TBL_SIZE] = TEMP_TBL_CONT;

    /* values */
    uint8_t  ok = 0;
    uint16_t st = 0;
    uint16_t ed = 0;
    uint16_t n  = 0;
    int16_t  temp;

    if ((T[0].adc > adc) && (adc > T[TEMP_TBL_SIZE - 1].adc)) {
        for (n = 0; n < (TEMP_TBL_SIZE - 1); n++) {
            if (adc >= T[n + 1].adc) {
                ok = 1;
                st = n;
                ed = n + 1;
                break;
            }
        }
    }

    if (ok) {
        /* Linear interpolation */
        /* y = (x - x1) * (y2 - y1) / (x2 - x1) + y1 */
        temp = (adc - T[st].adc) * (T[ed].temp - T[st].temp) / (T[ed].adc - T[st].adc) + T[st].temp;
    }
    else {
        if (adc > T[0].adc) {
            TRACE("Temp: under range\n");
            temp = T[0].temp;
        }
        else if (adc < T[TEMP_TBL_SIZE - 1].adc) {
            TRACE("Temp: over range\n");
            temp = T[TEMP_TBL_SIZE - 1].temp;
        }
    }

    return temp;
}
