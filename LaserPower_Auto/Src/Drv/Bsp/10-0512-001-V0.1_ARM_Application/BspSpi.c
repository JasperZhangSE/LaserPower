/*
    Spi.c

    Implementation File for STM32 SPI Module
*/

/* Copyright 2019 Shanghai Master Inc. */

/*
    Modification History
    --------------------
    01a 19Aug19 Ziv Created
*/

/* Includes */
#include "BspSpi.h"

/* Define */
#ifndef SPI1_PA5_PA6_PA7_ENABLE
#define SPI1_PA5_PA6_PA7_ENABLE     (0)
#endif
#ifndef SPI1_PB3_PB4_PB5_ENABLE
#define SPI1_PB3_PB4_PB5_ENABLE     (0)
#endif
#ifndef SPI2_PB13_PB14_PB15_ENABLE
#define SPI2_PB13_PB14_PB15_ENABLE  (0)
#endif
#ifndef SPI3_PB3_PB4_PB5_ENABLE
#define SPI3_PB3_PB4_PB5_ENABLE     (0)
#endif
#ifndef SPI3_PC10_PC11_PC12_ENABLE
#define SPI3_PC10_PC11_PC12_ENABLE  (0)
#endif

/* Function */
void HAL_SPI_MspInit(SPI_HandleTypeDef* pxSpi)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    if (SPI1 == pxSpi->Instance) {
    #if SPI1_PA5_PA6_PA7_ENABLE
        __HAL_RCC_SPI1_CLK_ENABLE();
        GPIO_InitStruct.Pin   = GPIO_PIN_7 | GPIO_PIN_5;
        GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        GPIO_InitStruct.Pin   = GPIO_PIN_6;
        GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull  = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    #endif /* SPI1_PA5_PA6_PA7_ENABLE */
    #if SPI1_PB3_PB4_PB5_ENABLE
        __HAL_RCC_SPI1_CLK_ENABLE();
        __HAL_AFIO_REMAP_SPI1_ENABLE();
        GPIO_InitStruct.Pin  = GPIO_PIN_5 | GPIO_PIN_3;
        GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        GPIO_InitStruct.Pin   = GPIO_PIN_4;
        GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull  = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    #endif /* SPI1_PB3_PB4_PB5_ENABLE */
    }
    else if (SPI2 == pxSpi->Instance) {
    #if SPI2_PB13_PB14_PB15_ENABLE
        __HAL_RCC_SPI2_CLK_ENABLE();
        GPIO_InitStruct.Pin   = GPIO_PIN_15 | GPIO_PIN_13;
        GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        GPIO_InitStruct.Pin   = GPIO_PIN_14;
        GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull  = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    #endif /* SPI2_PB13_PB14_PB15_ENABLE */
    }
    else if (SPI3 == pxSpi->Instance) {
    #if SPI3_PB3_PB4_PB5_ENABLE
        __HAL_RCC_SPI3_CLK_ENABLE();
        GPIO_InitStruct.Pin   = GPIO_PIN_5 | GPIO_PIN_3;
        GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        GPIO_InitStruct.Pin   = GPIO_PIN_4;
        GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull  = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    #endif /* SPI3_PB3_PB4_PB5_ENABLE */
    #if SPI3_PC10_PC11_PC12_ENABLE
        __HAL_RCC_SPI3_CLK_ENABLE();
        __HAL_AFIO_REMAP_SPI3_ENABLE();
        GPIO_InitStruct.Pin   = GPIO_PIN_12 | GPIO_PIN_10;
        GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
        GPIO_InitStruct.Pin   = GPIO_PIN_11;
        GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull  = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    #endif /* SPI3_PC10_PC11_PC12_ENABLE */
    }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* pxSpi)
{
    if (SPI1 == pxSpi->Instance) {
    #if SPI1_PA5_PA6_PA7_ENABLE
        __HAL_RCC_SPI1_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_7 | GPIO_PIN_5);
    #endif /* SPI1_PA5_PA6_PA7_ENABLE */
    #if SPI1_PB3_PB4_PB5_ENABLE
        __HAL_RCC_SPI1_CLK_DISABLE();
        __HAL_AFIO_REMAP_SPI1_DISABLE();
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_5 | GPIO_PIN_3);
    #endif /* SPI1_PB3_PB4_PB5_ENABLE */
    }
    else if (SPI2 == pxSpi->Instance) {
    #if SPI2_PB13_PB14_PB15_ENABLE
        __HAL_RCC_SPI2_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_15 | GPIO_PIN_13);
    #endif /* SPI2_PB13_PB14_PB15_ENABLE */
    }
    else if (SPI3 == pxSpi->Instance) {
    #if SPI3_PB3_PB4_PB5_ENABLE
        __HAL_RCC_SPI3_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_5 | GPIO_PIN_3);
    #endif /* SPI3_PB3_PB4_PB5_ENABLE */
    #if SPI3_PC10_PC11_PC12_ENABLE
        __HAL_RCC_SPI3_CLK_DISABLE();
        __HAL_AFIO_REMAP_SPI3_DISABLE();
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_12 | GPIO_PIN_10);
    #endif /* SPI3_PC10_PC11_PC12_ENABLE */
    }
}
