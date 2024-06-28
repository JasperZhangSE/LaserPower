/*
    BspSpi.h

    Head File for STM32 BSP SPI Module
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
   Modification History
   --------------------
   01a 13Nov23 Ziv Created
*/

#ifndef __BSP_SPI_H__
#define __BSP_SPI_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes */
#include <stm32f1xx_hal.h>

/* Define */
#define SPI3_PB3_PB4_PB5_ENABLE      (1)

/* Functions */
void HAL_SPI_MspInit(SPI_HandleTypeDef* pxSpi);
void HAL_SPI_MspDeInit(SPI_HandleTypeDef* pxSpi);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BSP_SPI_H__ */
