/*
    Uart.h

    Head File for Uart Module
*/

/* Copyright 2018 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 20Sep18, Karl Created
    01b, 24Dec18, Karl Modified, Added UartStartIt & UartStopIt
    01c, 14Jul19, Karl Reconstructured Uart lib
    01d, 28Aug19, Karl Added UartIsrCb support for Uart isr without DMA
    01e, 03Dec19, Karl Added UartConfigComEx
*/

#ifndef __UART_H__
#define __UART_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

/* Includes */
#include <stdint.h>
#include <stm32f1xx_hal.h>
#include "Include/Include.h"
#include "Uart/UartConfig.h"

/* Types */
typedef uint32_t UartHandle_t;

typedef struct {
  uint32_t ulWordLength;
  uint32_t ulStopBits;
  uint32_t ulParity;
  uint32_t ulHwFlowCtl;
}UartPortPara_t;

typedef void (*UartIsrFunc_t)(void*);

typedef Status_t (*UartProcRxFunc_t)(uint8_t*, uint16_t, void*);

/* Functions */
Status_t    UartInit(void);
Status_t    UartTerm(void);

UartHandle_t    UartCreate(void);
Status_t        UartDelete(UartHandle_t xHandle);

Status_t    UartConfigCb(UartHandle_t xHandle, UartProcRxFunc_t CbRxProc, UartIsrFunc_t CbUartIsr, UartIsrFunc_t CbDmaRxIsr, UartIsrFunc_t CbDmaTxIsr, void* pIsrPara);
Status_t    UartConfigCom(UartHandle_t xHandle, USART_TypeDef *pxInstance, uint32_t ulBaudRate, IRQn_Type xIrq);
Status_t    UartConfigComEx(UartHandle_t xHandle, USART_TypeDef *pxInstance, uint32_t ulBaudRate, IRQn_Type xIrq, UartPortPara_t xPara);
Status_t    UartConfigRxDma(UartHandle_t xHandle, DMA_Channel_TypeDef *pDmaChan, IRQn_Type Irq);
Status_t    UartConfigTxDma(UartHandle_t xHandle, DMA_Channel_TypeDef *pDmaChan, IRQn_Type Irq);

Status_t    UartStartIt(UartHandle_t xHandle);
Status_t    UartStopIt(UartHandle_t xHandle);

/* XXX: UartDmaSend should not be called in isr! */
Status_t    UartDmaSend(UartHandle_t xHandle, uint8_t* pucBuf, uint16_t usLength, uint16_t usWaitMs);
Status_t    UartBlkSend(UartHandle_t xHandle, uint8_t* pucBuf, uint16_t usLength, uint16_t usWaitMs);
Status_t    UartBlkRead(UartHandle_t xHandle, uint8_t* pucBuf, uint16_t *pusLength, uint16_t usWaitMs);

void        UartIsrCb(void *p);
void        UartDmaRxIsrCb(void *p);
void        UartDmaTxIsrCb(void *p);
void        UartIsr(UartHandle_t xHandle);
void        UartDmaRxIsr(UartHandle_t xHandle);
void        UartDmaTxIsr(UartHandle_t xHandle);

#if UART_TEST
Status_t    UartTest(void);
#endif /* UART_TEST */

#if UART_DEBUG
Status_t    UartShowStatus(UartHandle_t xHandle);
#endif /* UART_DEBUG */

#if UART_ENABLE_MSP
void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle);
void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle);
#endif /* UART_ENABLE_MSP */

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif /* __UART_H__ */
