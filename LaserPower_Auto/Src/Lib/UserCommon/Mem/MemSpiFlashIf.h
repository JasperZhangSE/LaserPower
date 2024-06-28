/*
    MemSpiFlashIf.h
    
    Head File for Mem Spi Flash Interface Module
*/

/* Copyright 2019 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 20Sep18, David Created
    01b, 06Oct18, Karl Modified
    01c, 24Jul19, Karl Reconstructured Mem lib
*/

#ifndef __MEM_SPI_FLASH_IF_H__
#define __MEM_SPI_FLASH_IF_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

/* Includes */
#include <stdint.h>
#include <stm32f1xx_hal.h>
#include "Include/Include.h"
#include "Mem/Mem.h"
#include "Mem/MemConfig.h"

/* Functions */
Status_t    SpiFlashIfInit(void);
Status_t    SpiFlashIfTerm(void);
Status_t    SpiFlashIfConfig(MemHandle_t xHandle, MemConfig_t *pxConfig);

Status_t    SpiFlashIfCtrl(MemHandle_t xHandle, MemCtrlOperation_t xCtrlOp, uint32_t ulExtraPara);
Status_t    SpiFlashIfRead(MemHandle_t xHandle, uint32_t ulAddr, uint32_t ulLength, OUT uint8_t *pucData);
Status_t    SpiFlashIfWrite(MemHandle_t xHandle, uint32_t ulAddr, uint32_t ulLength, IN uint8_t *pucData);
Status_t    SpiFlashIfEraseWrite(MemHandle_t xHandle, uint32_t ulAddr, uint32_t ulLength, IN uint8_t *pucData);

#if MEM_SPIFLASH_ENABLE_MSP
void HAL_SPI_MspInit(SPI_HandleTypeDef *pxSpiHandle);
void HAL_SPI_MspDeInit(SPI_HandleTypeDef *pxSpiHandle);
#endif /* MEM_SPIFLASH_ENABLE_MSP */

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif /* __MEM_SPI_FLASH_IF_H__ */
