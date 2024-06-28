/*
    Rbuf.h

    Head File for Rbuf Module
*/

/* Copyright 2018 Shanghai BiNY Inc. */

/*
    modification history
    --------------------
    01a, 28Nov18, Karl Created
    01b, 04Dec18, Karl Added RbufReadDirect
    01c, 12Jul19, Karl Added RbufCreate and RbufDestroy
    01d, 12Jul19, Karl Reconstructured Rbuf library
*/

#ifndef __RBUF_H__
#define __RBUF_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

/* Includes */
#include <stdint.h>
#include "Include/Include.h"
#include "Rbuf/RbufConfig.h"

/* Types */
typedef uint32_t RbufHandle_t;

typedef enum {
    RBUF_STATUS_OK,
    RBUF_STATUS_ERR,
    RBUF_STATUS_ERR_INIT,
    RBUF_STATUS_ERR_REINIT,
    RBUF_STATUS_ERR_UNINIT,
    RBUF_STATUS_ERR_MSGQ,
}RbufStatus_t;

/* Functions */
RbufStatus_t    RbufInit(void);
RbufStatus_t    RbufTerm(void);

RbufHandle_t    RbufCreate(void);
RbufStatus_t    RbufDelete(RbufHandle_t xHandle);
RbufStatus_t    RbufConfig(RbufHandle_t xHandle, void* pvBuffer, uint32_t ulBufSize, uint32_t ulMsgQueueSize, uint32_t ulMsgQueueWaitMs);

uint32_t        RbufWrite(RbufHandle_t xHandle, const void* pvData, uint32_t ulCount);
uint32_t        RbufRead(RbufHandle_t xHandle, void* pvData, uint32_t ulCount);
uint32_t        RbufReadDirect(RbufHandle_t xHandle, void* pvData, uint32_t ulCount);
uint32_t        RbufGetFree(RbufHandle_t xHandle);
uint32_t        RbufGetFull(RbufHandle_t xHandle);
RbufStatus_t    RbufShowStatus(RbufHandle_t xHandle);

#if RBUF_TEST
Status_t        RbufTest(void);
#endif /* RBUF_TEST */

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif /* __RBUF_H__ */
