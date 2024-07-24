/*
    Uart.c

    Implementation File for Uart Module
*/

/* Copyright 2018 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 10Sep18, Karl Created
    01b, 24Dec18, Karl Modified, added UartStartIt & UartStopIt
    01c, 14Jul19, Karl Reconstructured Uart lib
    01d, 28Aug19, Karl Added UartIsrCb support for Uart isr without DMA
    01e, 03Dec19, Karl Added UartConfigComEx
*/

/* Includes */
#include <stdlib.h>
#include <string.h>
#include <cmsis_os.h>
#include "Uart/Uart.h"
#include "Debug/Debug.h"

#if UART_ENABLE

/* Pragmas */
#pragma diag_suppress 177 /* warning: #177-D: variable was declared but never referenced */
#pragma diag_suppress 188 /* warning: #188-D: enumerated type mixed with another type */

/* Debug config */
#if UART_DEBUG || 1
#undef TRACE
#define TRACE(...) DebugPrintf(__VA_ARGS__)
#else
#undef TRACE
#define TRACE(...)
#endif /* UART_DEBUG */
#if UART_ASSERT
#undef ASSERT
#define ASSERT(a)                                                                                                      \
    while (!(a)) {                                                                                                     \
        DebugPrintf("ASSERT failed: %s %d\n", __FILE__, __LINE__);                                                     \
    }
#else
#undef ASSERT
#define ASSERT(...)
#endif /* UART_ASSERT */

/* Local defines */
#define UART_GET_CTRL(handle) ((UartCtrl_t *)(handle))

/* Local types */
typedef struct {
    UART_HandleTypeDef hUart;
    DMA_HandleTypeDef  hDmaRx;
    DMA_HandleTypeDef  hDmaTx;

    uint8_t            ucTxBuf[UART_TXBUF_SIZE];
    uint8_t            ucRxBuf[UART_RXBUF_SIZE];
    uint16_t           RxLen;
    
    UartIsrFunc_t      pxUartIsrFunc;
    UartIsrFunc_t      pxDmaRxIsrFunc;
    UartIsrFunc_t      pxDmaTxIsrFunc;
    UartProcRxFunc_t   pxProcRxFunc;

    IRQn_Type          xUartIrq;
    IRQn_Type          xDmaRxIrq;
    IRQn_Type          xDmaTxIrq;
    void              *pvIsrPara;

#if UART_TEST
    uint32_t ulUartIsrCnt;
    uint32_t ulUartDmaRxIsrCnt;
    uint32_t ulUartDmaTxIsrCnt;
    uint32_t ulUartDmaRxIsrCbCnt;
    uint32_t ulUartDmaTxIsrCbCnt;
#endif /* UART_TEST */
} UartCtrl_t;

/* Functions */
Status_t UartInit(void) {
    /* Do nothing */
    return STATUS_OK;
}

Status_t UartTerm(void) {
    /* Do nothing */
    return STATUS_OK;
}

UartHandle_t UartCreate(void) {
    UartCtrl_t *pxCtrl = NULL;
    pxCtrl             = (UartCtrl_t *)malloc(sizeof(UartCtrl_t));
    ASSERT(NULL != pxCtrl);
    if (pxCtrl) {
        memset(pxCtrl, 0, sizeof(UartCtrl_t));
    }
    return (UartHandle_t)pxCtrl;
}

Status_t UartDelete(UartHandle_t xHandle) {
    if (xHandle) {
        free((void *)xHandle);
    }
    return STATUS_OK;
}

Status_t UartConfigCb(UartHandle_t xHandle, UartProcRxFunc_t pxProcRxFunc, UartIsrFunc_t pxUartIsrFunc,
    UartIsrFunc_t pxDmaRxIsrFunc, UartIsrFunc_t pxDmaTxIsrFunc, void *pvIsrPara) {
    UartCtrl_t *pxCtrl = UART_GET_CTRL(xHandle);

    ASSERT(NULL != pxCtrl);
    pxCtrl->pxUartIsrFunc  = pxUartIsrFunc;
    pxCtrl->pxDmaRxIsrFunc = pxDmaRxIsrFunc;
    pxCtrl->pxDmaTxIsrFunc = pxDmaTxIsrFunc;
    pxCtrl->pxProcRxFunc   = pxProcRxFunc;
    pxCtrl->pvIsrPara      = pvIsrPara;
        
    pxCtrl->RxLen = 0;
    return STATUS_OK;
}

Status_t UartConfigCom(UartHandle_t xHandle, USART_TypeDef *pxInstance, uint32_t ulBaudRate, IRQn_Type xIrq) {
    UartCtrl_t *pxCtrl = UART_GET_CTRL(xHandle);

    ASSERT(NULL != pxCtrl);
    pxCtrl->hUart.Instance          = pxInstance;
    pxCtrl->hUart.Init.BaudRate     = ulBaudRate;
    pxCtrl->hUart.Init.WordLength   = UART_WORDLENGTH_8B;
    pxCtrl->hUart.Init.StopBits     = UART_STOPBITS_1;
    pxCtrl->hUart.Init.Parity       = UART_PARITY_NONE;
    pxCtrl->hUart.Init.Mode         = UART_MODE_TX_RX;
    pxCtrl->hUart.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    pxCtrl->hUart.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&(pxCtrl->hUart)) != HAL_OK) {
        /* We should never get here! */
        ASSERT(0);
    }

    if (pxCtrl->pxUartIsrFunc) {
        /* IDLE Interrupt Configuration */
//        SET_BIT(pxInstance->CR1, USART_CR1_IDLEIE);
        __HAL_UART_ENABLE_IT(&(pxCtrl->hUart), UART_IT_IDLE);
        HAL_NVIC_SetPriority(xIrq, 5, 0);
        HAL_NVIC_EnableIRQ(xIrq);
    }

    /* Start DMA receive */
    if (pxCtrl->hDmaRx.Instance) {
        HAL_UART_Receive_DMA(&(pxCtrl->hUart), (uint8_t *)(pxCtrl->ucRxBuf), UART_RXBUF_SIZE);
    }
    else {
        
        TRACE("Enable uart it rxne.\n");
        __HAL_UART_ENABLE_IT(&(pxCtrl->hUart), UART_IT_RXNE);
    }

    return STATUS_OK;
}

Status_t UartConfigComEx(UartHandle_t xHandle, USART_TypeDef *pxInstance, uint32_t ulBaudRate, IRQn_Type xIrq,
                         UartPortPara_t xPara) {
    UartCtrl_t *pxCtrl = UART_GET_CTRL(xHandle);

    ASSERT(NULL != pxCtrl);
    pxCtrl->hUart.Instance          = pxInstance;
    pxCtrl->hUart.Init.BaudRate     = ulBaudRate;
    pxCtrl->hUart.Init.WordLength   = xPara.ulWordLength;
    pxCtrl->hUart.Init.StopBits     = xPara.ulStopBits;
    pxCtrl->hUart.Init.Parity       = xPara.ulParity;
    pxCtrl->hUart.Init.Mode         = UART_MODE_TX_RX;
    pxCtrl->hUart.Init.HwFlowCtl    = xPara.ulHwFlowCtl;
    pxCtrl->hUart.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&(pxCtrl->hUart)) != HAL_OK) {
        /* We should never get here! */
        ASSERT(0);
    }

    if (pxCtrl->pxUartIsrFunc) {
        /* IDLE Interrupt Configuration */
        SET_BIT(pxInstance->CR1, USART_CR1_IDLEIE);
        HAL_NVIC_SetPriority(xIrq, 5, 0);
        HAL_NVIC_EnableIRQ(xIrq);
    }

    /* Start DMA receive */
    if (pxCtrl->hDmaRx.Instance) {
        HAL_UART_Receive_DMA(&(pxCtrl->hUart), (uint8_t *)(pxCtrl->ucRxBuf), UART_RXBUF_SIZE);
    }

    return STATUS_OK;
}

Status_t UartConfigRxDma(UartHandle_t xHandle, DMA_Channel_TypeDef *pxDmaChan, IRQn_Type xIrq) {
    UartCtrl_t *pxCtrl = UART_GET_CTRL(xHandle);

    ASSERT(NULL != pxCtrl);
    __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();
    pxCtrl->hDmaRx.Instance                 = pxDmaChan;
    pxCtrl->hDmaRx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    pxCtrl->hDmaRx.Init.PeriphInc           = DMA_PINC_DISABLE;
    pxCtrl->hDmaRx.Init.MemInc              = DMA_MINC_ENABLE;
    pxCtrl->hDmaRx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    pxCtrl->hDmaRx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    pxCtrl->hDmaRx.Init.Mode                = DMA_NORMAL;
    pxCtrl->hDmaRx.Init.Priority            = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&(pxCtrl->hDmaRx)) != HAL_OK) {
        /* We should never get here! */
        ASSERT(0);
    }

    __HAL_LINKDMA(&(pxCtrl->hUart), hdmarx, pxCtrl->hDmaRx);

    if (pxCtrl->pxDmaRxIsrFunc) {
        /* DMA Interrupt Configuration */
        HAL_NVIC_SetPriority(xIrq, 5, 0);
        HAL_NVIC_EnableIRQ(xIrq);
    }

    return STATUS_OK;
}

Status_t UartConfigTxDma(UartHandle_t xHandle, DMA_Channel_TypeDef *pxDmaChan, IRQn_Type xIrq) {
    UartCtrl_t *pxCtrl = UART_GET_CTRL(xHandle);

    ASSERT(NULL != pxCtrl);
    __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();
    pxCtrl->hDmaTx.Instance                 = pxDmaChan;
    pxCtrl->hDmaTx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    pxCtrl->hDmaTx.Init.PeriphInc           = DMA_PINC_DISABLE;
    pxCtrl->hDmaTx.Init.MemInc              = DMA_MINC_ENABLE;
    pxCtrl->hDmaTx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    pxCtrl->hDmaTx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    pxCtrl->hDmaTx.Init.Mode                = DMA_NORMAL;
    pxCtrl->hDmaTx.Init.Priority            = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&(pxCtrl->hDmaTx)) != HAL_OK) {
        /* We should never get here! */
        ASSERT(0);
    }

    __HAL_LINKDMA(&(pxCtrl->hUart), hdmatx, pxCtrl->hDmaTx);

    if (pxCtrl->pxDmaTxIsrFunc) {
        /* DMA Interrupt Configuration */
        HAL_NVIC_SetPriority(xIrq, 5, 0);
        HAL_NVIC_EnableIRQ(xIrq);
    }

    return STATUS_OK;
}

Status_t UartStartIt(UartHandle_t xHandle) {
    UartCtrl_t *pxCtrl = UART_GET_CTRL(xHandle);

    ASSERT(NULL != pxCtrl);
    __HAL_UART_CLEAR_IDLEFLAG(&(pxCtrl->hUart));
    __HAL_UART_ENABLE_IT(&(pxCtrl->hUart), UART_IT_IDLE);
    /* Restart UART DMA receive */
    HAL_UART_Receive_DMA(&(pxCtrl->hUart), (uint8_t *)(pxCtrl->ucRxBuf), UART_RXBUF_SIZE);

    return STATUS_OK;
}

Status_t UartStopIt(UartHandle_t xHandle) {
    UartCtrl_t *pxCtrl = UART_GET_CTRL(xHandle);

    ASSERT(NULL != pxCtrl);
    /* Disable IDLE interrupt */
    __HAL_UART_DISABLE_IT(&(pxCtrl->hUart), UART_IT_IDLE);
    /* Stop DMA transfer */
    HAL_UART_DMAStop(&(pxCtrl->hUart));

    return STATUS_OK;
}

Status_t UartDmaSend(UartHandle_t xHandle, uint8_t *pucBuf, uint16_t usLength, uint16_t usWaitMs) {
    UartCtrl_t *pxCtrl = UART_GET_CTRL(xHandle);

    ASSERT(NULL != pucBuf);
    ASSERT(NULL != pxCtrl);
    HAL_UART_StateTypeDef state = HAL_UART_GetState(&(pxCtrl->hUart)) & HAL_UART_STATE_BUSY_TX;
    while ((state == HAL_UART_STATE_BUSY_TX) && usWaitMs) {
        state = HAL_UART_GetState(&(pxCtrl->hUart)) & HAL_UART_STATE_BUSY_TX;
        usWaitMs--;
        osDelay(1);
    }

    if (state != HAL_UART_STATE_BUSY_TX) {
        if (usLength <= UART_TXBUF_SIZE) {
            memcpy(pxCtrl->ucTxBuf, pucBuf, usLength);
            if (HAL_OK == HAL_UART_Transmit_DMA(&(pxCtrl->hUart), (uint8_t *)(pxCtrl->ucTxBuf), usLength)) {
                return STATUS_OK;
            }
            else {
                TRACE("UartDmaSend: send error\n");
                return STATUS_ERR;
            }
        }
        else {
            TRACE("UartDmaSend: length error\n");
            return STATUS_ERR;
        }
    }
    else {
        TRACE("UartDmaSend: busy error\n");
        return STATUS_ERR;
    }
}

Status_t UartBlkSend(UartHandle_t xHandle, uint8_t *pucBuf, uint16_t usLength, uint16_t usWaitMs) {
    UartCtrl_t *pxCtrl = UART_GET_CTRL(xHandle);

    ASSERT(NULL != pucBuf);
    ASSERT(NULL != pxCtrl);
    HAL_UART_StateTypeDef state = HAL_UART_GetState(&(pxCtrl->hUart)) & HAL_UART_STATE_BUSY_TX;
    while (state == HAL_UART_STATE_BUSY_TX) {
        state = HAL_UART_GetState(&(pxCtrl->hUart)) & HAL_UART_STATE_BUSY_TX;
    }
    if (HAL_OK == HAL_UART_Transmit(&(pxCtrl->hUart), (uint8_t *)pucBuf, usLength, usWaitMs)) {
        return STATUS_OK;
    }
    else {
        return STATUS_ERR;
    }
}

Status_t UartBlkRead(UartHandle_t xHandle, uint8_t *pucBuf, uint16_t *pusLength, uint16_t usWaitMs) {
    uint16_t              n      = 0;
    uint16_t              usCnt  = 0;
    UartCtrl_t           *pxCtrl = UART_GET_CTRL(xHandle);
    HAL_UART_StateTypeDef state;

    ASSERT(NULL != pucBuf);
    ASSERT(NULL != pxCtrl);
    usCnt = *pusLength;
    for (n = 0; n < usCnt; n++) {
        while (HAL_UART_STATE_BUSY_RX == (HAL_UART_GetState(&(pxCtrl->hUart)) & HAL_UART_STATE_BUSY_RX)) {
        };

        state = HAL_UART_Receive(&(pxCtrl->hUart), (uint8_t *)pucBuf + n, 1, usWaitMs);

        if (HAL_TIMEOUT == state) {
            *pusLength = (n + 1);
            return STATUS_ERR;
        }
    }

    *pusLength = n;
    return STATUS_OK;
}

void UartIsrCb(void *p) {
    UartCtrl_t *pxCtrl = UART_GET_CTRL(p);

    ASSERT(NULL != pxCtrl);
    /* IDLE interrupt process */
    if (pxCtrl->hUart.hdmarx) {
        /* DMA process */
        if ((__HAL_UART_GET_FLAG(&(pxCtrl->hUart), UART_FLAG_IDLE) != RESET)) {
            /* Stop UART DMA Rx */
            CLEAR_BIT(pxCtrl->hUart.Instance->CR3, USART_CR3_DMAR);
            HAL_DMA_Abort(pxCtrl->hUart.hdmarx);
            if (pxCtrl->hUart.State == HAL_UART_STATE_BUSY_TX_RX) {
                pxCtrl->hUart.State = HAL_UART_STATE_BUSY_TX;
            }
            else {
                pxCtrl->hUart.State = HAL_UART_STATE_READY;
            }
            /* Receive & process the data */
            {
                uint32_t ulRecvd;
                ulRecvd = UART_RXBUF_SIZE - pxCtrl->hUart.hdmarx->Instance->CNDTR;
                if (pxCtrl->pxProcRxFunc) {
                    (pxCtrl->pxProcRxFunc)((uint8_t *)pxCtrl->ucRxBuf, (uint16_t)ulRecvd, pxCtrl->pvIsrPara);
                }
            }
            /* Restart UART DMA receive */
            HAL_UART_Receive_DMA(&(pxCtrl->hUart), (uint8_t *)(pxCtrl->ucRxBuf), UART_RXBUF_SIZE);
            /* Clear interrupt flag */
            __HAL_UART_CLEAR_IDLEFLAG(&(pxCtrl->hUart));
        }
    }
    else {
        /* Read DR register directly */
        if ((__HAL_UART_GET_FLAG(&(pxCtrl->hUart), UART_FLAG_IDLE) != RESET)) {
            if (pxCtrl->hUart.State == HAL_UART_STATE_BUSY_TX_RX) {
                pxCtrl->hUart.State = HAL_UART_STATE_BUSY_TX;
            }
            else {
                pxCtrl->hUart.State = HAL_UART_STATE_READY;
            }
//            /* Receive & process the data */
//            {
//                pxCtrl->ucRxBuf[0] = pxCtrl->hUart.Instance->DR;
//                if (pxCtrl->pxProcRxFunc) {
//                    (pxCtrl->pxProcRxFunc)((uint8_t *)pxCtrl->ucRxBuf, 1, pxCtrl->pvIsrPara);
//                }
//            }
            
            /* Receive & process the data */
            {
                if (pxCtrl->pxProcRxFunc) {
                    (pxCtrl->pxProcRxFunc)((uint8_t *)pxCtrl->ucRxBuf, pxCtrl->RxLen, pxCtrl->pvIsrPara);
                    memset(pxCtrl->ucRxBuf, 0, UART_RXBUF_SIZE);
                    pxCtrl->RxLen = 0;
                }
            }
            
            /* Clear interrupt flag */
            __HAL_UART_CLEAR_IDLEFLAG(&(pxCtrl->hUart));
        }
        
        if ((__HAL_UART_GET_FLAG(&(pxCtrl->hUart), UART_FLAG_RXNE) != RESET)) {
            
            if (pxCtrl->hUart.State == HAL_UART_STATE_BUSY_TX_RX) {
                pxCtrl->hUart.State = HAL_UART_STATE_BUSY_TX;
            }
            else {
                pxCtrl->hUart.State = HAL_UART_STATE_READY;
            }
            
            /* Receive & process the data */
            {
                pxCtrl->ucRxBuf[pxCtrl->RxLen] = pxCtrl->hUart.Instance->DR;
                pxCtrl->RxLen++;
            }
        }
    }
    return;
}

void UartDmaRxIsrCb(void *p) {
    UartCtrl_t *pxCtrl = UART_GET_CTRL(p);
    /* TODO: UartDmaRxIsrCb */
    ASSERT(NULL != pxCtrl);
#if UART_TEST
    pxCtrl->ulUartDmaRxIsrCbCnt++;
#endif /* UART_TEST */
    return;
}

void UartDmaTxIsrCb(void *p) {
    UartCtrl_t *pxCtrl = UART_GET_CTRL(p);
    /* TODO: UartDmaTxIsrCb */
    ASSERT(NULL != pxCtrl);
#if UART_TEST
    pxCtrl->ulUartDmaTxIsrCbCnt++;
#endif /* UART_TEST */
    return;
}

void UartIsr(UartHandle_t xHandle) {
    UartCtrl_t *pxCtrl = UART_GET_CTRL(xHandle);

    ASSERT(NULL != pxCtrl);
#if UART_TEST
    pxCtrl->ulUartIsrCnt++;
#endif /* UART_TEST */
    (pxCtrl->pxUartIsrFunc)(pxCtrl);
    HAL_UART_IRQHandler(&(pxCtrl->hUart));
    return;
}

void UartDmaRxIsr(UartHandle_t xHandle) {
    UartCtrl_t *pxCtrl = UART_GET_CTRL(xHandle);

    ASSERT(NULL != pxCtrl);
#if UART_TEST
    pxCtrl->ulUartDmaRxIsrCnt++;
#endif /* UART_TEST */
    (pxCtrl->pxDmaRxIsrFunc)(pxCtrl);
    HAL_DMA_IRQHandler(pxCtrl->hUart.hdmarx);
    return;
}

void UartDmaTxIsr(UartHandle_t xHandle) {
    UartCtrl_t *pxCtrl = UART_GET_CTRL(xHandle);

    ASSERT(NULL != pxCtrl);
#if UART_TEST
    pxCtrl->ulUartDmaTxIsrCnt++;
#endif /* UART_TEST */
    (pxCtrl->pxDmaTxIsrFunc)(pxCtrl);
    HAL_DMA_IRQHandler(pxCtrl->hUart.hdmatx);
    return;
}

#if UART_DEBUG
Status_t UartShowStatus(UartHandle_t xHandle) {
    UartCtrl_t *pxCtrl = UART_GET_CTRL(xHandle);

    ASSERT(NULL != pxCtrl);
    TRACE("Uart handle: %08X\n", xHandle);
    TRACE("    hUart               : %08X\n", pxCtrl->hUart);
    TRACE("    hDmaRx              : %08X\n", pxCtrl->hDmaRx);
    TRACE("    hDmaTx              : %08X\n", pxCtrl->hDmaTx);

    TRACE("    pxUartIsrFunc       : %08X\n", pxCtrl->pxUartIsrFunc);
    TRACE("    pxDmaRxIsrFunc      : %08X\n", pxCtrl->pxDmaRxIsrFunc);
    TRACE("    pxDmaTxIsrFunc      : %08X\n", pxCtrl->pxDmaTxIsrFunc);
    TRACE("    pxProcRxFunc        : %08X\n", pxCtrl->pxProcRxFunc);

    TRACE("    xUartIrq            : %d\n", pxCtrl->xUartIrq);
    TRACE("    xDmaRxIrq           : %d\n", pxCtrl->xDmaRxIrq);
    TRACE("    xDmaTxIrq           : %d\n", pxCtrl->xDmaTxIrq);
    TRACE("    pvIsrPara           : %08X\n", pxCtrl->pvIsrPara);

#if UART_TEST
    TRACE("    ulUartIsrCnt        : %d\n", pxCtrl->ulUartIsrCnt);
    TRACE("    ulUartDmaRxIsrCnt   : %d\n", pxCtrl->ulUartDmaRxIsrCnt);
    TRACE("    ulUartDmaTxIsrCnt   : %d\n", pxCtrl->ulUartDmaTxIsrCnt);
    TRACE("    ulUartDmaRxIsrCbCnt : %d\n", pxCtrl->ulUartDmaRxIsrCbCnt);
    TRACE("    ulUartDmaTxIsrCbCnt : %d\n", pxCtrl->ulUartDmaTxIsrCbCnt);
    TRACE("    \n");
#endif /* UART_TEST */

    return STATUS_OK;
}
#endif /* UART_DEBUG */

#if UART_ENABLE_MSP

/* XXX: Uart.c -> This HAL_UART_MspInit applies to "BY-SCGATE101-V1.1-STM32F107VCT6" board! */
void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle) {
    GPIO_InitTypeDef GPIO_InitStruct;
    if (uartHandle->Instance == USART1) {
        /* Peripheral clock enable */
        __HAL_RCC_USART1_CLK_ENABLE();
        /*
         * PB6 -> USART1_TX
         * PB7 -> USART1_RX
         */
        GPIO_InitStruct.Pin   = GPIO_PIN_6;
        GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        GPIO_InitStruct.Pin  = GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        __HAL_AFIO_REMAP_USART1_ENABLE();
    }
    else if (uartHandle->Instance == USART2) {
        /* Peripheral clock enable */
        __HAL_RCC_USART2_CLK_ENABLE();
        /*
         * PD5 -> USART2_TX
         * PD6 -> USART2_RX 
         */
        GPIO_InitStruct.Pin   = GPIO_PIN_5;
        GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
        GPIO_InitStruct.Pin  = GPIO_PIN_6;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
        __HAL_AFIO_REMAP_USART2_ENABLE();
    }
    else if (uartHandle->Instance == USART3) {
        /* Peripheral clock enable */
        __HAL_RCC_USART3_CLK_ENABLE();
        /*
         * PD8 -> USART3_TX
         * PD9 -> USART3_RX 
         */
        GPIO_InitStruct.Pin   = GPIO_PIN_8;
        GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
        GPIO_InitStruct.Pin  = GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
        __HAL_AFIO_REMAP_USART3_ENABLE();
    }
}

/* XXX: Uart.c -> This HAL_UART_MspDeInit applies to "BY-SCGATE101-V1.1-STM32F107VCT6" board! */
void HAL_UART_MspDeInit(UART_HandleTypeDef *uartHandle) {
    if (uartHandle->Instance == USART1) {
        /* Peripheral clock disable */
        __HAL_RCC_USART1_CLK_DISABLE();
        /*
         * PB6 -> USART1_TX
         * PB7 -> USART1_RX 
         */
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6 | GPIO_PIN_7);
        /* Peripheral DMA DeInit*/
        HAL_DMA_DeInit(uartHandle->hdmarx);
        HAL_DMA_DeInit(uartHandle->hdmatx);
    }
    else if (uartHandle->Instance == USART2) {
        /* Peripheral clock disable */
        __HAL_RCC_USART2_CLK_DISABLE();
        /*
         * PD5 -> USART2_TX
         * PD6 -> USART2_RX 
         */
        HAL_GPIO_DeInit(GPIOD, GPIO_PIN_5 | GPIO_PIN_6);
        /* Peripheral DMA DeInit*/
        HAL_DMA_DeInit(uartHandle->hdmarx);
        HAL_DMA_DeInit(uartHandle->hdmatx);
    }
    else if (uartHandle->Instance == USART3) {
        __HAL_RCC_USART3_CLK_DISABLE();
        /*
         * PD8 -> USART3_TX
         * PD9 -> USART3_RX
         */
        HAL_GPIO_DeInit(GPIOD, GPIO_PIN_8 | GPIO_PIN_9);
        HAL_DMA_DeInit(uartHandle->hdmarx);
        HAL_DMA_DeInit(uartHandle->hdmatx);
    }
}

#endif /* UART_ENABLE_MSP */

#endif /* UART_ENABLE */
