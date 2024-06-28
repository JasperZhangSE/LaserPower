/*
    MemFram.h

    Head File for Mem Fram Module
*/

/* Copyright 2019 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 01Jul19, Ziv Created
    01b, 24Jul19, Karl Reconstructured Mem lib
    01c, 09Mar20, Karl Added address width & address mask
    01d, 11May20, Karl Fixed merge errors
*/

#ifndef __MEM_FRAM_H__
#define __MEM_FRAM_H__

#ifdef __cplusplus
    extern "C" {
#endif /*__cplusplus */

/* Includes */
#include <stdint.h>
#include <stm32f1xx_hal.h>
#include "Include/Include.h"
#include "Mem/Mem.h"
#include "Mem/MemConfig.h"

/* Defines */
/* Configuration table */
/* FM25V20(256K): 0x3FFFF, 18 */
/* FM25V10(128K): 0x1FFFF, 17 */
/* FM25L16B(2K) : 0xFFFF,  16 */
#ifndef MEM_FRAM_ADDR_MASK
#define MEM_FRAM_ADDR_MASK      (0x1FFFF)
#endif /* MEM_FRAM_ADDR_MASK */
#ifndef MEM_FRAM_ADDR_WIDTH
#define MEM_FRAM_ADDR_WIDTH     (17)
#endif /* MEM_FRAM_ADDR_WIDTH */

/* Types */
typedef union {
    struct {
        uint8_t Res1:1;     /* Don't care */
        uint8_t WEL:1;      /* Write Enable */
        uint8_t BP:2;       /* Block Protect */
        uint8_t Res2:3;     /* Don't care */
        uint8_t WPEN;       /* Write Protect Enable bit */
    }xBit;
    uint8_t ucData;
}FramStatus_t;

/* Functions */
Status_t    FramInit(void);
Status_t    FramTerm(void);
Status_t    FramConfig(MemHandle_t xHandle, MemConfig_t *pxConfig);

Status_t    FramCtrl(MemHandle_t xHandle, MemCtrlOperation_t xCtrlOp, uint32_t ulExtraPara);
Status_t    FramRead(MemHandle_t xHandle, uint32_t ulAddr, uint32_t ulLength, OUT uint8_t *pucData);
Status_t    FramWrite(MemHandle_t xHandle, uint32_t ulAddr, uint32_t ulLength, IN uint8_t *pucData);

uint8_t     FramReadStatus(MemHandle_t xHandle);
Status_t    FramShowStatus(uint8_t ucStatus);

#if MEM_FRAM_ENABLE_MSP
void HAL_SPI_MspInit(SPI_HandleTypeDef* pxSpiHandle);
void HAL_SPI_MspDeInit(SPI_HandleTypeDef* pxSpiHandle);
#endif /* MEM_FRAM_ENABLE_MSP */

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif /* __MEM_FRAM_H__ */
