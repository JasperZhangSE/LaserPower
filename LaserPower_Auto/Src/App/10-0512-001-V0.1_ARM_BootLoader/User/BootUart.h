/*
    BootUart.h

    Head File for Boot Uart Module
*/

/* Copyright 2022 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 06Apr22, Karl Created
    01b, 10Apr22, Karl Added image verification utility
*/

#ifndef __BOOT_UART_H__
#define __BOOT_UART_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes */
#include "Include/Include.h"
#include "Uart/Uart.h"
#include "BootUartConfig.h"

/* Types */
typedef void (*BootUartDone_t)(void);

typedef struct {
    GPIO_TypeDef*  pxRxPinPort;
    uint32_t       ulRxPin;
    uint32_t       ulRxPinAf;   /* It's not used here! */
    
    GPIO_TypeDef*  pxTxPinPort;
    uint32_t       ulTxPin;
    uint32_t       ulTxPinAf;   /* It's not used here! */
    
    USART_TypeDef* pxInstance;
    uint32_t       ulWordLength;
    uint32_t       ulStopBits;
    uint32_t       ulParity;
    uint32_t       ulBaudRate;
    
    BootUartDone_t pxDoneFunc;
}BootUartConfig_t;

/* Functions */
Status_t BootUartInit(void);
Status_t BootUartTerm(void);

Status_t BootUartConfig(const BootUartConfig_t *pxConfig);
Status_t BootUartRun(void);

#if BOOT_UART_TEST
Status_t BootUartTest(void);
#endif /* BOOT_UART_TEST */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOOT_UART_H__ */
