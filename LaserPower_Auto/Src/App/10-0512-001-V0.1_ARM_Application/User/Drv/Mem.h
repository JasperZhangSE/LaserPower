/*
    Mem.h

    Head File for Mem Module
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 17Nov23, Karl Created
    01b, 27Nov23, Karl Added MemFlashRead and MemFlashWrite
*/

#ifndef ___MEM_H__
#define ___MEM_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Defines */
#ifndef MEM_FLASH_SPI
#define MEM_FLASH_SPI       SPI3
#endif /* MEM_FRAM_SPI */
#ifndef MEM_FLASH_CAPACITY
#define MEM_FLASH_CAPACITY  8*1024*1024 /*8MB*/
#endif /* MEM_FLASH_CAPACITY */

/* Functions */
Status_t DrvMemInit(void);
Status_t DrvMemTerm(void);

Status_t MemFlashRead(uint32_t ulAddr, uint32_t ulLength, OUT uint8_t *pucData);
Status_t MemFlashWrite(uint32_t ulAddr, uint32_t ulLength, IN uint8_t *pucData);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ___MEM_H__ */
