/*
    UartConfig.h

    Configuration File for Uart Module
*/

/* Copyright 2019 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 14Jul19, Karl Created
*/

#ifndef __UART_CONFIG_H__
#define __UART_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

/* Includes */
#include "Config.h"

/* Defines */
#ifndef UART_ENABLE
#define UART_ENABLE             (0)
#endif
#ifndef UART_RTOS
#define UART_RTOS               (1)
#endif
#ifndef UART_DEBUG
#define UART_DEBUG              (0)
#endif
#ifndef UART_ASSERT
#define UART_ASSERT             (0)
#endif
#ifndef UART_TEST
#define UART_TEST               (0)
#endif
#ifndef UART_ENABLE_MSP
#define UART_ENABLE_MSP         (0)
#endif
#ifndef UART_TXBUF_SIZE
#define UART_TXBUF_SIZE         (128)
#endif
#ifndef UART_RXBUF_SIZE
#define UART_RXBUF_SIZE         (128)
#endif

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif /* __UART_CONFIG_H__ */
