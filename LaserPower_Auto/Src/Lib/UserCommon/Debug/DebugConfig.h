/*
    DebugConfig.h

    Configuration File for Debug Module
*/

/* Copyright 2019 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 16Jul19, Karl Created
*/

#ifndef __DEBUG_CONFIG_H__
#define __DEBUG_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

/* Includes */
#include "Config.h"

/* Configuration defines */
#ifndef DEBUG_ENABLE
#define DEBUG_ENABLE                (0)
#endif
#ifndef DEBUG_ENABLE_UART
#define DEBUG_ENABLE_UART           (0)
#endif
#ifndef DEBUG_ENABLE_TCP
#define DEBUG_ENABLE_TCP            (0)
#endif
#ifndef DEBUG_ENABLE_UDP
#define DEBUG_ENABLE_UDP            (0)
#endif
#ifndef DEBUG_RTOS
#define DEBUG_RTOS                  (1)
#endif
#ifndef DEBUG_DEBUG
#define DEBUG_DEBUG                 (0)
#endif
#ifndef DEBUG_TEST
#define DEBUG_TEST                  (0)
#endif
#ifndef DEBUG_ASSERT
#define DEBUG_ASSERT                (0)
#endif
#ifndef DEBUG_BUF_SIZE
#define DEBUG_BUF_SIZE              (128)
#endif
#ifndef DEBUG_UART_ENABLE_MSP
#define DEBUG_UART_ENABLE_MSP       (0)
#endif
#ifndef DEBUG_UART_ENABLE_PRINTF
#define DEBUG_UART_ENABLE_PRINTF    (0)
#endif
#ifndef DEBUG_UART_SEND_TIMEOUT
#define DEBUG_UART_SEND_TIMEOUT     (1000) /* Ms */
#endif
#ifndef DEBUG_UDP_ENABLE_STAMP
#define DEBUG_UDP_ENABLE_STAMP      (0)
#endif
#ifndef DEBUG_UDP_PKT_HEAD
#define DEBUG_UDP_PKT_HEAD          (0x7E7E)
#endif
#ifndef DEBUG_UDP_PKT_TAIL
#define DEBUG_UDP_PKT_TAIL          (0x0D0A)
#endif
#ifndef DEBUG_UDP_SVR_ADDR
#define DEBUG_UDP_SVR_ADDR          "192.168.1.255"
#endif
#ifndef DEBUG_UDP_SVR_PORT
#define DEBUG_UDP_SVR_PORT          (6000)
#endif
#ifndef DEBUG_UDP_CLI_ADDR
#define DEBUG_UDP_CLI_ADDR          IPADDR_ANY
#endif
#ifndef DEBUG_UDP_CLI_PORT
#define DEBUG_UDP_CLI_PORT          (8000)
#endif

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif /*__DEBUG_CONFIG_H__*/
