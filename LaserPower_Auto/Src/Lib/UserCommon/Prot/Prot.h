/*
    Prot.h

    Head File for Prot Module
*/

/* Copyright 2019 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 22Nov18, Karl Created
    01b, 13Jul19, Karl Reconstructured Prot library
*/

#ifndef __PROT_H__
#define __PROT_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

/* Includes */
#include <stdint.h>
#include "Include/Include.h"
#include "Prot/ProtConfig.h"

/* Types */
typedef uint32_t ProtHandle_t;

/* Packet head length type */
typedef enum {
    PROT_LENGTH_UINT8   = 1,
    PROT_LENGTH_UINT16  = 2,
    PROT_LENGTH_UINT32  = 4,
    PROT_LENGTH_TYPE_SIZE,
}ProtLength_t;

/* Packet process callback function */
typedef uint8_t (*ProtCheckFunc_t)(const void*, uint32_t);
typedef Status_t (*ProtProcFunc_t)(const void*, const uint8_t*, uint32_t, void*);

/* Functions */
Status_t        ProtInit(void);
Status_t        ProtTerm(void);

ProtHandle_t    ProtCreate(void);
Status_t        ProtDelete(ProtHandle_t xHandle);

Status_t        ProtConfigHead(ProtHandle_t xHandle, const uint8_t* pucHeadMark, uint8_t ucHeadMarkSize, uint8_t ucHeadSize);
Status_t        ProtConfigTail(ProtHandle_t xHandle, const uint8_t* pucTailMark, uint8_t ucTailMarkSize, uint8_t ucTailSize);
Status_t        ProtConfigMisc(ProtHandle_t xHandle, uint32_t ulMaxMsgSize, uint32_t ulMsgLengthOffset, ProtLength_t xLengthType, uint8_t ucLengthOffset);
Status_t        ProtConfigCb(ProtHandle_t xHandle, ProtProcFunc_t pxPktProc, ProtCheckFunc_t pxPktChk);
Status_t        ProtConfig(ProtHandle_t xHandle);

Status_t        ProtProc(ProtHandle_t xHandle, IN uint8_t* pucRecvBuf, IN uint16_t usRecvd, INOUT uint16_t *pusRecvIndex, INOUT uint8_t* pucProcBuf, IN void* pvPara);

#if PROT_TEST
Status_t        ProtTest(void);
#endif /* PROT_TEST */

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif /* __PROT_H__ */
