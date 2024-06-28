/*
    BspUart.h

    Head File for STM32 BSP UART Module
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
   Modification History
   --------------------
   01a 13Nov23 Ziv Created
*/

#ifndef __BSP_UART_H__
#define __BSP_UART_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes */
#include <stm32f1xx_hal.h>

/* Define */
#define USART1_PA9_PA10_ENABLE    (1)
#define USART2_PD5_PD6_ENABLE     (1)
#define UART4_PC10_PC11_ENABLE    (1)
#define UART5_PC12_PD2_ENABLE     (1)

/* Functions */
void HAL_UART_MspInit(UART_HandleTypeDef* pxUart);
void HAL_UART_MspDeInit(UART_HandleTypeDef* pxUart);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BSP_UART_H__ */
