/*
    Uart.c

    Implementation File for STM32 Uart Module
*/

/* Copyright 2019 Shanghai Master Inc. */

/*
    Modification History
    --------------------
    01a 19Aug19 Ziv Created
    01b 11Mar20 Karl Modified tx pin from GPIO_NOPULL to GPIO_PULLUP
*/

/* Includes */
#include "BspUart.h"

/* Define */
#ifndef USART1_PA9_PA10_ENABLE
#define USART1_PA9_PA10_ENABLE     (0)
#endif
#ifndef USART1_PB6_PB7_ENABLE
#define USART1_PB6_PB7_ENABLE      (0)
#endif
#ifndef USART2_PA2_PA3_ENABLE
#define USART2_PA2_PA3_ENABLE      (0)
#endif
#ifndef USART2_PD5_PD6_ENABLE
#define USART2_PD5_PD6_ENABLE      (0)
#endif
#ifndef USART3_PB10_PB11_ENABLE
#define USART3_PB10_PB11_ENABLE    (0)
#endif
#ifndef USART3_PD8_PD9_ENABLE
#define USART3_PD8_PD9_ENABLE      (0)
#endif
#ifndef UART4_PC10_PC11_ENABLE
#define UART4_PC10_PC11_ENABLE     (0)
#endif
#ifndef UART5_PC12_PD2_ENABLE
#define UART5_PC12_PD2_ENABLE      (0)
#endif

/* Function */
void HAL_UART_MspInit(UART_HandleTypeDef* pxUart)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    if (USART1 == pxUart->Instance) { 
    #if USART1_PA9_PA10_ENABLE
        __HAL_RCC_USART1_CLK_ENABLE();
        GPIO_InitStruct.Pin   = GPIO_PIN_9;
        GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        GPIO_InitStruct.Pin   = GPIO_PIN_10;
        GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull  = GPIO_PULLUP;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    #endif /* USART1_PA9_PA10_ENABLE*/
    #if USART1_PB6_PB7_ENABLE
        __HAL_RCC_USART1_CLK_ENABLE();
        GPIO_InitStruct.Pin   = GPIO_PIN_6;
        GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        GPIO_InitStruct.Pin   = GPIO_PIN_7;
        GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull  = GPIO_PULLUP;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        __HAL_AFIO_REMAP_USART1_ENABLE();
    #endif /* USART1_PB6_PB7_ENABLE*/
    }
    else if (USART2 == pxUart->Instance) { 
    #if USART2_PA2_PA3_ENABLE
        __HAL_RCC_USART2_CLK_ENABLE();
        GPIO_InitStruct.Pin   = GPIO_PIN_2;
        GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        GPIO_InitStruct.Pin   = GPIO_PIN_3;
        GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull  = GPIO_PULLUP;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    #endif /* USART2_PA2_PA3_ENABLE*/
    #if USART2_PD5_PD6_ENABLE
        __HAL_RCC_USART2_CLK_ENABLE();
        GPIO_InitStruct.Pin   = GPIO_PIN_5;
        GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
        GPIO_InitStruct.Pin   = GPIO_PIN_6;
        GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull  = GPIO_PULLUP;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
        __HAL_AFIO_REMAP_USART2_ENABLE();
    #endif /* USART2_PD5_PD6_ENABLE*/
    }
    else if (USART3 == pxUart->Instance) { 
    #if USART3_PB10_PB11_ENABLE
        __HAL_RCC_USART3_CLK_ENABLE();
        GPIO_InitStruct.Pin   = GPIO_PIN_10;
        GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        GPIO_InitStruct.Pin   = GPIO_PIN_11;
        GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull  = GPIO_PULLUP;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    #endif /* USART3_PB10_PB11_ENABLE*/
    #if USART3_PD8_PD9_ENABLE
        __HAL_RCC_USART3_CLK_ENABLE();
        GPIO_InitStruct.Pin   = GPIO_PIN_8;
        GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
        GPIO_InitStruct.Pin   = GPIO_PIN_9;
        GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull  = GPIO_PULLUP;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
        __HAL_AFIO_REMAP_USART3_ENABLE();
    #endif /* USART3_PD8_PD9_ENABLE*/
    }
    else if (UART4 == pxUart->Instance) { 
    #if UART4_PC10_PC11_ENABLE
        __HAL_RCC_UART4_CLK_ENABLE();
        GPIO_InitStruct.Pin   = GPIO_PIN_10;
        GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
        GPIO_InitStruct.Pin   = GPIO_PIN_11;
        GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull  = GPIO_PULLUP;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    #endif /* UART4_PC10_PC11_ENABLE*/
    }
    else if(UART5 == pxUart->Instance) { 
    #if UART5_PC12_PD2_ENABLE
         __HAL_RCC_UART5_CLK_ENABLE();
        GPIO_InitStruct.Pin   = GPIO_PIN_12;
        GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
        GPIO_InitStruct.Pin   = GPIO_PIN_2;
        GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull  = GPIO_PULLUP;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    #endif /* UART5_PC12_PD2_ENABLE*/
    }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* pxUart)
{
    if (USART1 == pxUart->Instance) {
    #if USART1_PA9_PA10_ENABLE
        __HAL_RCC_USART1_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);
    #endif /* USART1_PA9_PA10_ENABLE */
    #if USART1_PB6_PB7_ENABLE
        __HAL_RCC_USART1_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6 | GPIO_PIN_7);
    #endif /* USART1_PB6_PB7_ENABLE */
    }
    else if (USART2 == pxUart->Instance) {
    #if USART2_PA2_PA3_ENABLE
        __HAL_RCC_USART2_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2 | GPIO_PIN_3);
    #endif /* USART2_PA2_PA3_ENABLE */
    #if USART2_PD5_PD6_ENABLE
        __HAL_RCC_USART2_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOD, GPIO_PIN_5 | GPIO_PIN_6);
    #endif /* USART2_PD5_PD6_ENABLE */
    }
    else if (USART3 == pxUart->Instance) {
    #if USART3_PB10_PB11_ENABLE
        __HAL_RCC_USART3_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10 | GPIO_PIN_11);
    #endif /* USART3_PB10_PB11_ENABLE */
    #if USART3_PD8_PD9_ENABLE
        __HAL_RCC_USART3_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOD, GPIO_PIN_8 | GPIO_PIN_9);
    #endif /* USART3_PD8_PD9_ENABLE */
    }
    else if (UART4 == pxUart -> Instance) {
    #if UART4_PC10_PC11_ENABLE
        __HAL_RCC_UART4_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_10 | GPIO_PIN_11);
    #endif /* UART4_PC10_PC11_ENABLE */
    }
    else if (UART5 == pxUart->Instance) {
    #if UART5_PC12_PD2_ENABLE
        __HAL_RCC_UART5_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_12);
        HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);
    #endif /* UART5_PC12_PD2_ENABLE */
    }
}
