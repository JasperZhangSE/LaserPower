/*
    DebugUart.c

    Implementation File for Debug Uart Module
*/

/* Copyright 2019 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 10Sep18, Karl Created
    01b, 15Nov18, Karl Modified
    01c, 15Jul19, Karl Reconstructured Debug module
*/

/* Includes */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "Debug/DebugUart.h"

#if (DEBUG_ENABLE && DEBUG_ENABLE_UART)

/* Pragmas */
#pragma diag_suppress 550   /* warning: #550-D: variable was set but never used */

/* Debug config */
#if DEBUG_DEBUG
    #undef TRACE
    #define TRACE(...)
#else
    #undef TRACE
    #define TRACE(...)
#endif /* DEBUG_DEBUG */
#if DEBUG_ASSERT
    #undef ASSERT
    #define ASSERT(a)   while(!(a));
#else
    #undef ASSERT
    #define ASSERT(...)
#endif /* DEBUG_ASSERT */

/* Local variables */
static UART_HandleTypeDef   *s_pxUart = NULL;
static UART_HandleTypeDef   s_xUart;
static Bool_t               s_bConfig = FALSE;

/* Forward declaration */
#ifdef __GNUC__
    #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
    #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/* Functions */
Status_t DebugUartInit(void)
{
    /* Do nothing */
    return STATUS_OK;
}

Status_t DebugUartTerm(void)
{
    /* Do nothing */
    return STATUS_OK;
}

Status_t DebugUartConfig(USART_TypeDef *pxUart, uint32_t usBaudRate)
{
    ASSERT(pxUart != NULL);
    
    /* Config test uart channel */
    s_xUart.Instance          = pxUart;
    s_xUart.Init.BaudRate     = usBaudRate;
    s_xUart.Init.WordLength   = UART_WORDLENGTH_8B;
    s_xUart.Init.StopBits     = UART_STOPBITS_1;
    s_xUart.Init.Parity       = UART_PARITY_NONE;
    s_xUart.Init.Mode         = UART_MODE_TX_RX;
    s_xUart.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    s_xUart.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&s_xUart) != HAL_OK) {
        /* We should never get here! */
        ASSERT(0);
    }
    
    s_pxUart   = &s_xUart;
    s_bConfig  = TRUE;
    return STATUS_OK;
}

Status_t DebugUartPrintf(char *cFormat, ...)
{
    static char cBuf[DEBUG_BUF_SIZE];
    int nVspCnt = 0;
    
    ASSERT(NULL != s_pxUart);
    ASSERT(TRUE == s_bConfig);
    va_list va;
    va_start(va, cFormat);
    nVspCnt = vsprintf(cBuf, cFormat, va);
    if (nVspCnt > 0) {
        while (HAL_UART_GetState(s_pxUart) == HAL_UART_STATE_BUSY_TX);
        HAL_UART_Transmit(s_pxUart, (uint8_t*)cBuf, strlen(cBuf), DEBUG_UART_SEND_TIMEOUT);
        va_end(va); 
        return STATUS_OK;
    }
    else {
        va_end(va);
        return STATUS_ERR;
    }
}

Status_t DebugUartPrintfDirect(char *cStr, uint16_t usLength)
{
    ASSERT(NULL != s_pxUart);
    ASSERT(TRUE == s_bConfig);
    while(HAL_UART_GetState(s_pxUart) == HAL_UART_STATE_BUSY_TX);
    HAL_UART_Transmit(s_pxUart, (uint8_t*)cStr, usLength, DEBUG_UART_SEND_TIMEOUT);
    return STATUS_OK;
}

#if DEBUG_UART_ENABLE_PRINTF

PUTCHAR_PROTOTYPE
{
    while (HAL_UART_GetState(s_pxUart) == HAL_UART_STATE_BUSY_TX);
    HAL_UART_Transmit(s_pxUart, (uint8_t*)&ch, 1, DEBUG_UART_SEND_TIMEOUT);
    return STATUS_OK;
}

#endif /* DEBUG_UART_ENABLE_PRINTF */

#if DEBUG_UART_ENABLE_MSP
    
/* XXX: UartDebug.c -> This HAL_UART_MspInit applies to "BY-SCGATE101-V1.1-STM32F107VCT6" board! */
void HAL_UART_MspInit(UART_HandleTypeDef* pxUartHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    if (pxUartHandle->Instance==USART1) {
        /* Peripheral clock enable */
        __HAL_RCC_USART1_CLK_ENABLE();
        /*
         * PB6 -> USART1_TX
         * PB7 -> USART1_RX
         */
        GPIO_InitStruct.Pin     = GPIO_PIN_6;
        GPIO_InitStruct.Mode    = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed   = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        GPIO_InitStruct.Pin     = GPIO_PIN_7;
        GPIO_InitStruct.Mode    = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull    = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        __HAL_AFIO_REMAP_USART1_ENABLE();
    }
    else if (pxUartHandle->Instance==USART2) {
        /* Peripheral clock enable */
        __HAL_RCC_USART2_CLK_ENABLE();
        /*
         * PD5 -> USART2_TX
         * PD6 -> USART2_RX 
         */
        GPIO_InitStruct.Pin     = GPIO_PIN_5;
        GPIO_InitStruct.Mode    = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed   = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
        GPIO_InitStruct.Pin     = GPIO_PIN_6;
        GPIO_InitStruct.Mode    = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull    = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
        __HAL_AFIO_REMAP_USART2_ENABLE();
    }
    else if (pxUartHandle->Instance==USART3) {
        /* Peripheral clock enable */
        __HAL_RCC_USART3_CLK_ENABLE();
        /*
         * PD8 -> USART3_TX
         * PD9 -> USART3_RX 
         */
        GPIO_InitStruct.Pin     = GPIO_PIN_8;
        GPIO_InitStruct.Mode    = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed   = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
        GPIO_InitStruct.Pin     = GPIO_PIN_9;
        GPIO_InitStruct.Mode    = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull    = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
        __HAL_AFIO_REMAP_USART3_ENABLE();
    }
}

/* XXX: UartDebug.c -> This HAL_UART_MspDeInit applies to "BY-SCGATE101-V1.1-STM32F107VCT6" board! */
void HAL_UART_MspDeInit(UART_HandleTypeDef* pxUartHandle)
{
    if (pxUartHandle->Instance==USART1) {
        /* Peripheral clock disable */
        __HAL_RCC_USART1_CLK_DISABLE();
        /*
         * PB6 -> USART1_TX
         * PB7 -> USART1_RX 
         */
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6|GPIO_PIN_7);
        /* Peripheral DMA DeInit*/
        HAL_DMA_DeInit(pxUartHandle->hdmarx);
        HAL_DMA_DeInit(pxUartHandle->hdmatx);
    }
    else if (pxUartHandle->Instance==USART2) {
        /* Peripheral clock disable */
        __HAL_RCC_USART2_CLK_DISABLE();
        /*
         * PD5 -> USART2_TX
         * PD6 -> USART2_RX 
         */
        HAL_GPIO_DeInit(GPIOD, GPIO_PIN_5|GPIO_PIN_6);
        /* Peripheral DMA DeInit*/
        HAL_DMA_DeInit(pxUartHandle->hdmarx);
        HAL_DMA_DeInit(pxUartHandle->hdmatx);
    }
    else if (pxUartHandle->Instance==USART3) {
        __HAL_RCC_USART3_CLK_DISABLE();
        /*
         * PD8 -> USART3_TX
         * PD9 -> USART3_RX
         */
        HAL_GPIO_DeInit(GPIOD, GPIO_PIN_8|GPIO_PIN_9);
        HAL_DMA_DeInit(pxUartHandle->hdmarx);
        HAL_DMA_DeInit(pxUartHandle->hdmatx);
    }
}

#endif /* DEBUG_UART_ENABLE_MSP */

#endif /* (DEBUG_ENABLE && DEBUG_ENABLE_UART) */
