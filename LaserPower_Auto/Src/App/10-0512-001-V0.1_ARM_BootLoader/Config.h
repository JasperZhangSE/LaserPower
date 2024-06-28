/*
    Config.h

    Configuration File for User Application
*/

/* Copyright 2019 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 20Aug19, Karl Created
*/

#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */
    
/* User Common Library Configuration */
/* Cli module */
#define CLI_ENABLE                  (0)
#define CLI_ENABLE_UART             (0)
#define CLI_ENABLE_UDP              (0)
#define CLI_ENABLE_CUSTOM           (1)
#define CLI_RTOS                    (1)
#define CLI_DEBUG                   (0)
#define CLI_TEST                    (0)
#define CLI_ASSERT                  (0)

/* Debug module */
#define DEBUG_ENABLE                (1)
#define DEBUG_ENABLE_UART           (0)
#define DEBUG_RTOS                  (1)
#define DEBUG_DEBUG                 (0)
#define DEBUG_TEST                  (0)
#define DEBUG_ASSERT                (0)
#define DEBUG_BUF_SIZE              (128)
#define DEBUG_UART_ENABLE_MSP       (0)
#define DEBUG_UART_ENABLE_PRINTF    (0)
#define DEBUG_UART_SEND_TIMEOUT     (1000) /* Ms */

/* Gpio module */
#define GPIO_ENABLE                 (0)
#define GPIO_RTOS                   (1)
#define GPIO_DEBUG                  (0)
#define GPIO_TEST                   (0)
#define GPIO_ASSERT                 (0)
#define GPIO_DESP                   (0)

/* Mem module */
#define MEM_ENABLE                  (1)
#define MEM_ENABLE_FRAM             (0)
#define MEM_ENABLE_SPIFLASH         (1)
#define MEM_ENABLE_STM32FLASH       (1)
#define MEM_RTOS                    (0)
#define MEM_DEBUGx                  (0)
#define MEM_FRAM_ADDR_MASK          (0x3FFFF)
#define MEM_FRAM_ADDR_WIDTH         (18)
#define MEM_TEST                    (0)
#define MEM_ASSERT                  (0)
#define MEM_SPIFLASH_ENABLE_MSP     (0)
#define MEM_FRAM_ENABLE_MSP         (0)

/* Mqtt module */
#define MQTT_ENABLE                 (0)
#define MQTT_RTOS                   (1)
#define MQTT_DEBUG                  (0)
#define MQTT_ASSERT                 (0)
#define MQTT_TEST                   (0)

/* Net module */
#define NET_ENABLE                  (0)
#define NET_ENABLE_LWIP             (1)
#define NET_ENABLE_SUITE            (1)
#define NET_RTOS                    (1)
#define NET_DEBUG                   (0)
#define NET_TEST                    (0)
#define NET_ASSERT                  (0)

/* Prot module */
#define PROT_ENABLE                 (0)
#define PROT_RTOS                   (1)
#define PROT_DEBUG                  (0)
#define PROT_TEST                   (0)
#define PROT_ASSERT                 (0)
#define PROT_MAX_MARK_SIZE          (4)
#define PROT_SHOW_CONT              (0)

/* Rbuf module */
#define RBUF_ENABLE                 (0)
#define RBUF_RTOS                   (1)
#define RBUF_DEBUG                  (0)
#define RBUF_ASSERT                 (0)
#define RBUF_TEST                   (0)
#define RBUF_MSGQ_SIZE              (10)
#define RBUF_MSGQ_WAITMS            (500)

/* Rtc module */
#define RTC_ENABLE                  (0)
#define RTC_STM32_ENABLE            (1)
#define RTC_DS1302_ENABLE           (1)
#define RTC_STDC_TIME               (0)     /* 0: No use; 1: STM32; 2: DS1302 */
#define RTC_RTOS                    (1)
#define RTC_DEBUG                   (0)
#define RTC_SHOW_TIME               (0)
#define RTC_ASSERT                  (0)
#define RTC_TEST                    (0)
#define RTC_ENABLE_MSP              (0)

/* Rtos module */
#define RTOS_ENABLE                 (1)
#define RTOS_ENABLE_CHECK           (1)
#define RTOS_ENABLE_SYNC            (0)
#define RTOS_RTOS                   (1)
#define RTOS_DEBUG                  (0)
#define RTOS_TEST                   (0)
#define RTOS_ASSERT                 (0)

/* Uart module */
#define UART_ENABLE                 (1)
#define UART_RTOS                   (1)
#define UART_DEBUG                  (0)
#define UART_ASSERT                 (0)
#define UART_TEST                   (0)
#define UART_ENABLE_MSP             (0)
#define UART_TXBUF_SIZE             (256)
#define UART_RXBUF_SIZE             (256)

/* Wdog module */
#define WDOG_ENABLE                 (1)
#define WDOG_RTOS                   (1)
#define WDOG_DEBUG                  (0)
#define WDOG_ASSERT                 (0)
#define WDOG_TEST                   (0)

/* User Application Configuration */
#define APP_BOOT_ENABLE_WDOG        (1)
#define BOOT_UART_ENABLE            (1)
#define BOOT_UART_FLASH_PAGE_SIZE   (0x800)
#define BOOT_UART_APP_ADDR          (0x08010000)
#define BOOT_UART_APP_SIZE          (0x30000)

/* XXX: Just for compatibility */
#define TickInit()
#define TickDelay(d)                HAL_Delay(d)

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif/* __APP_CONFIG_H__ */
