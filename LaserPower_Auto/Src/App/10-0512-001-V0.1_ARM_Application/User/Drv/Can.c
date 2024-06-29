/*
    Can.c

    Implementation File for Can Module
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 15Nov23, Karl Created
*/

/* Includes */
#include "Include.h"

/* Debug config */
#if CAN_DEBUG
    #undef TRACE
    #define TRACE(...)  DebugPrintf(__VA_ARGS__)
#else
    #undef TRACE
    #define TRACE(...)
#endif /* CAN_DEBUG */
#if CAN_ASSERT
    #undef ASSERT
    #define ASSERT(a)   while(!(a)){DebugPrintf("ASSERT failed: %s %d\n", __FILE__, __LINE__);}
#else
    #undef ASSERT
    #define ASSERT(...)
#endif /* CAN_ASSERT */

/* Local defines */

/* Local types */

/* Forward declarations */

/* Local variables */
static CAN_HandleTypeDef s_hCan;
static CanTxMsgTypeDef   s_xTxMsg;
static CanRxMsgTypeDef   s_xRxMsg;

/* Functions */
Status_t DrvCanInit(void)
{
    s_hCan.Instance       = CAN1;
    s_hCan.pRxMsg         = &s_xRxMsg;
    s_hCan.pTxMsg         = &s_xTxMsg;
    
    s_hCan.Init.Prescaler = 36;
    s_hCan.Init.Mode      = CAN_MODE_NORMAL;
    s_hCan.Init.SJW       = CAN_SJW_1TQ;
    s_hCan.Init.BS1       = CAN_BS1_6TQ;
    s_hCan.Init.BS2       = CAN_BS2_1TQ;
    s_hCan.Init.TTCM      = DISABLE;
    s_hCan.Init.ABOM      = DISABLE;
    s_hCan.Init.AWUM      = DISABLE;
    s_hCan.Init.NART      = DISABLE;
    s_hCan.Init.RFLM      = DISABLE;
    s_hCan.Init.TXFP      = DISABLE;
    HAL_CAN_Init(&s_hCan);
    
    CAN_FilterConfTypeDef  xFilterConfig;
    xFilterConfig.FilterNumber         = 0;
    xFilterConfig.FilterMode           = CAN_FILTERMODE_IDMASK;
    xFilterConfig.FilterScale          = CAN_FILTERSCALE_32BIT;
    xFilterConfig.FilterIdHigh         = 0x0000;
    xFilterConfig.FilterIdLow          = 0x0000;
    xFilterConfig.FilterMaskIdHigh     = 0x0000;
    xFilterConfig.FilterMaskIdLow      = 0x0000;
    xFilterConfig.FilterFIFOAssignment = 0;
    xFilterConfig.FilterActivation     = ENABLE;
    xFilterConfig.BankNumber           = 14;
    HAL_CAN_ConfigFilter(&s_hCan, &xFilterConfig);
    
    HAL_CAN_Receive_IT(&s_hCan, CAN_FIFO0);
    
    return STATUS_OK;
}

Status_t DrvCanTerm(void)
{
    /* Do nothing */
    return STATUS_OK;
}

Status_t CanSend(IN CanMsgTx_t *pxMsg, uint16_t usWaitMs)
{
    s_hCan.pTxMsg->StdId = pxMsg->StdId;
    s_hCan.pTxMsg->ExtId = pxMsg->ExtId;
    s_hCan.pTxMsg->RTR   = pxMsg->RTR;
    s_hCan.pTxMsg->IDE   = pxMsg->IDE;
    s_hCan.pTxMsg->DLC   = pxMsg->DLC;
    if (s_hCan.pTxMsg->DLC > 8) {
        s_hCan.pTxMsg->DLC = 8;
    }
    for (uint8_t n = 0; n < s_hCan.pTxMsg->DLC; n++) {
        s_hCan.pTxMsg->Data[n] = pxMsg->Data[n];
    }
    
    return HAL_CAN_Transmit(&s_hCan, usWaitMs);
}

Status_t CanRead(OUT CanMsgRx_t *pxMsg, uint16_t usWaitMs)
{
    /* TODO: CanRead not implemented! */
    return STATUS_OK;
}

__weak void CanRxNotify(uint32_t ulPwr2Addr, CanMsgRx_t *pxMsg)
{
    /* Do nothing */
}

void HAL_CAN_MspInit(CAN_HandleTypeDef* pxCan)
{
    GPIO_InitTypeDef xInit;
    if (pxCan->Instance == CAN1) {
        /* Peripheral clock enable */
        __HAL_RCC_CAN1_CLK_ENABLE();
        /* Gpio configuration */
        xInit.Pin   = GPIO_PIN_11;
        xInit.Mode  = GPIO_MODE_INPUT;
        xInit.Pull  = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &xInit);
        xInit.Pin   = GPIO_PIN_12;
        xInit.Mode  = GPIO_MODE_AF_PP;
        xInit.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &xInit);
        /* Interrupt init */
        HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
    }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* pxCan)
{
    if (pxCan->Instance == CAN1) {
        /* Peripheral clock disable */
        __HAL_RCC_CAN1_CLK_DISABLE();
        /* Gpio configuration */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);
        /* Interrupt deinit */
        HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
    }
}

void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef *pxCan)
{
    CanRxNotify(PWR2_M2_ADDR, pxCan->pRxMsg);
    CanRxNotify(PWR2_M1_ADDR, pxCan->pRxMsg);
    CanRxNotify(PWR2_M3_ADDR, pxCan->pRxMsg);
    /* Restart receive */
#if 0
    HAL_CAN_Receive_IT(&s_hCan, CAN_FIFO0);
#else
    __HAL_CAN_ENABLE_IT(&s_hCan, CAN_IT_FMP0);
#endif
}

void CAN1_RX0_IRQHandler(void)
{
    HAL_CAN_IRQHandler(&s_hCan);
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

static uint32_t prvCvtHexToUint32(const char* pcStr)
{
    uint32_t ulData = 0;
    while (0 != *pcStr) {
        char cTmp = *pcStr++;
        if ((cTmp >= 'a') && (cTmp <= 'f')) {
            ulData *= 16;
            ulData += 10 + (cTmp - 'a');
        }
        if ((cTmp >= 'A') && (cTmp <= 'F')) {
            ulData *= 16;
            ulData += 10 + (cTmp - 'A');
        }
        if ((cTmp >= '0') && (cTmp <= '9')) {
            ulData *= 16;
            ulData += cTmp - '0';
        }
    }
    return ulData;
}

static void prvCliCmdCanSend(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    if (argc < 4) {
        cliprintf("can_send TYPE(0-STD, 1-EXT) ID(HEX) LEN(DEC) DATA0 ... DATA8(HEX)\n");
        return;
    }

    int type = atoi(argv[1]);
    int id   = prvCvtHexToUint32(argv[2]);
    int len  = atoi(argv[3]);
    
    CanMsgTx_t m;    
    m.StdId = id;
    m.ExtId = id;
    m.IDE   = (type == 0) ? CAN_ID_STD : CAN_ID_EXT;
    m.RTR   = CAN_RTR_DATA;
    m.DLC   = len;
    if (len && (argc >= 5)) {
        uint8_t s = 0;
        uint8_t *d = prvParseHexStr(argv[4], &s);
        for (uint8_t n = 0; n < 8; n++) {
            m.Data[n] = d[n];
        }
    }
    
    CanSend(&m, 5);
    
    return;
}
CLI_CMD_EXPORT(can_send, send can data, prvCliCmdCanSend)
