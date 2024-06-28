/*
    Rbuf.c

    Implementation File for Rbuf Module
*/

/* Copyright 2018 Shanghai BiNY Inc. */

/*
    modification history
    --------------------
    01a, 28Nov18, Karl Modified
    01b, 04Dec18, Karl Added RbufReadDirect
    01c, 12Jul19, Karl Added RbufCreate and RbufDestroy
    01d, 12Jul19, Karl Reconstructured Rbuf library
*/

/* Includes */
#include <string.h>
#include <cmsis_os.h>
#include "Rbuf/Rbuf.h"
#include "Rbuf/RbufInternal.h"

#if RBUF_ENABLE

/* Debug config */
#if RBUF_DEBUG
    #undef TRACE
    #define TRACE(...)  DebugPrintf(__VA_ARGS__)
#else
    #undef TRACE
    #define TRACE(...)
#endif /* RBUF_DEBUG */
#if RBUF_ASSERT
    #undef ASSERT
    #define ASSERT(a)   while(!(a)){DebugPrintf("ASSERT failed: %s %d\n", __FILE__, __LINE__);}
#else
    #undef ASSERT
    #define ASSERT(...)
#endif /* RBUF_ASSERT */

/* Local defines */
#if RBUF_RTOS
    #define RBUF_LOCK       taskENTER_CRITICAL
    #define RBUF_UNLOCK     taskEXIT_CRITICAL
#else
    #define RBUF_LOCK
    #define RBUF_UNLOCK
#endif /* RBUF_RTOS */

#define RBUF_GET_CTRL(handle)   ((RbufCtrl_t*)(handle))

/* Local types */
typedef struct {
    uint8_t         bInit;
    BUFFER_t        xRbuf;
#if RBUF_RTOS
    osMessageQId    xMsgQueue;
    uint32_t        ulMsgQueueWaitMs;
#endif /* RBUF_RTOS */
#if RBUF_TEST
    uint32_t        ulTotalWriteCnt;
    uint32_t        ulTotalReadCnt;
    uint32_t        ulBufFullErrCnt;
    uint32_t        ulBufMsgErrCnt;
#endif /* RBUF_TEST */
}RbufCtrl_t;

/* Forward declaration */
static int inHandlerMode (void);

/* Functions */
RbufStatus_t RbufInit(void)
{
    /* Do nothing */
    return RBUF_STATUS_OK;
}

RbufStatus_t RbufTerm(void)
{
    /* Do nothing */
    return RBUF_STATUS_OK;
}

RbufHandle_t RbufCreate(void)
{
    RbufCtrl_t *pxCtrl = NULL;
    pxCtrl = (RbufCtrl_t*)malloc(sizeof(RbufCtrl_t));
    memset(pxCtrl, 0, sizeof(RbufCtrl_t));
    return (RbufHandle_t)pxCtrl;
}

RbufStatus_t RbufDelete(RbufHandle_t xHandle)
{
    if (xHandle) {
        free((void*)xHandle);
    }
    return RBUF_STATUS_OK;
}

RbufStatus_t RbufConfig(RbufHandle_t xHandle, void* pvBuffer, uint32_t ulBufSize, uint32_t ulMsgQueueSize, uint32_t ulMsgQueueWaitMs)
{
    RbufCtrl_t *pxCtrl = RBUF_GET_CTRL(xHandle);
    
    ASSERT(NULL != pxCtrl);
    if(pxCtrl->bInit)
    {
        TRACE("RbufConfig has been inited\n");
        return RBUF_STATUS_ERR_REINIT;
    }
    
    if(BUFFER_Init(&(pxCtrl->xRbuf), ulBufSize, pvBuffer))
    {
        pxCtrl->bInit = FALSE;
        TRACE("RbufConfig BUFFER_Init failed\n");
        return RBUF_STATUS_ERR_INIT;
    }
    
#if RBUF_RTOS
    osMessageQDef(RbufMsgQ, ulMsgQueueSize, uint32_t);
    pxCtrl->xMsgQueue = osMessageCreate(osMessageQ(RbufMsgQ), NULL);
    pxCtrl->ulMsgQueueWaitMs = ulMsgQueueWaitMs;
    if(NULL == pxCtrl->xMsgQueue)
    {
        pxCtrl->bInit = FALSE;
        TRACE("RbufConfig osMessageCreate failed\n");
        return RBUF_STATUS_ERR_MSGQ;
    }
#endif /* RBUF_RTOS */

#if RBUF_TEST
    pxCtrl->ulTotalWriteCnt = 0;
    pxCtrl->ulTotalReadCnt  = 0;
    pxCtrl->ulBufFullErrCnt = 0;
    pxCtrl->ulBufMsgErrCnt  = 0;
#endif /* RBUF_TEST */
    pxCtrl->bInit           = TRUE;
    
    return RBUF_STATUS_OK;   
}

uint32_t RbufWrite(RbufHandle_t xHandle, const void* pvData, uint32_t ulCount)
{
    uint32_t ulFree = 0;
    uint32_t ulSent = 0;
    RbufCtrl_t *pxCtrl = RBUF_GET_CTRL(xHandle);
#if RBUF_RTOS
    int isr;
#endif /* RBUF_RTOS */

    ASSERT((NULL != pxCtrl) && pxCtrl->bInit);
    ulFree = BUFFER_GetFree(&(pxCtrl->xRbuf));
    if(ulFree < ulCount)
    {
    #if RBUF_TEST
        pxCtrl->ulBufFullErrCnt++;
    #endif /* RBUF_TEST */
        /* Notify the client to read the data immediately */
        if(osOK != osMessagePut(pxCtrl->xMsgQueue, 1, 0))
        {
        #if RBUF_TEST
            pxCtrl->ulBufMsgErrCnt++;
        #endif /* RBUF_TEST */
            TRACE("RbufWrite osMessagePut failed\n");
        }
        TRACE("RbufWrite failed\n");
        return 0;
    }
    
#if RBUF_RTOS
    isr = inHandlerMode();
    if(!isr)
    {
        RBUF_LOCK();
    }
#endif /* RBUF_RTOS */
    ulSent = BUFFER_Write(&(pxCtrl->xRbuf), pvData, ulCount);
#if RBUF_TEST
    pxCtrl->ulTotalWriteCnt += ulSent;
#endif /* RBUF_TEST */
#if RBUF_RTOS
    if(!isr)
    {
        RBUF_UNLOCK();
    }
#endif /* RBUF_RTOS */
    
#if RBUF_RTOS
    if(ulSent)
    {
        /* Notify the client to read the data */
        if(osOK != osMessagePut(pxCtrl->xMsgQueue, 1, 0))
        {
        #if RBUF_TEST
            pxCtrl->ulBufMsgErrCnt++;
        #endif /* RBUF_TEST */
            TRACE("RbufWrite osMessagePut failed\n");
        }
    }
#endif /* RBUF_RTOS */

    return ulSent;
}

uint32_t RbufRead(RbufHandle_t xHandle, void* pvData, uint32_t ulCount)
{
    osEvent ev;
    uint32_t ulUsed = 0;
    uint32_t ulRead = 0;
    RbufCtrl_t *pxCtrl = RBUF_GET_CTRL(xHandle);
#if RBUF_RTOS
    int isr;
#endif /* RBUF_RTOS */

    ASSERT((NULL != pxCtrl) && pxCtrl->bInit);
#if RBUF_RTOS
    ev = osMessageGet(pxCtrl->xMsgQueue, pxCtrl->ulMsgQueueWaitMs);
    ulUsed = BUFFER_GetFull(&(pxCtrl->xRbuf));  /* In case of the msg facility is something wrong! */
    if((osEventMessage != ev.status) && (0 == ulUsed))
    {
        TRACE("RbufRead osMessageGet failed and the buffer is empty\n");
        return 0;
    }
#endif /* RBUF_RTOS */
    
#if RBUF_RTOS
    isr = inHandlerMode();
    if(!isr)
    {
        RBUF_LOCK();
    }
#endif /* RBUF_RTOS */
    ulUsed = BUFFER_GetFull(&(pxCtrl->xRbuf));
    if(ulUsed > ulCount)
    {
        ulUsed = ulCount;
    }
    if(ulUsed)
    {
        ulRead = BUFFER_Read(&(pxCtrl->xRbuf), pvData, ulUsed);
    #if RBUF_TEST
        pxCtrl->ulTotalReadCnt += ulRead;
    #endif /* RBUF_TEST */
    }
#if RBUF_RTOS
    if(!isr)
    {
        RBUF_UNLOCK();
    }
#endif /* RBUF_RTOS */

    return ulRead;
}

uint32_t RbufReadDirect(RbufHandle_t xHandle, void* pvData, uint32_t ulCount)
{
    uint32_t ulUsed = 0;
    uint32_t ulRead = 0;
    RbufCtrl_t *pxCtrl = RBUF_GET_CTRL(xHandle);
#if RBUF_RTOS
    int isr;
#endif /* RBUF_RTOS */
    
    ASSERT((NULL != pxCtrl) && pxCtrl->bInit);
#if RBUF_RTOS
    isr = inHandlerMode();
    if(!isr)
    {
        RBUF_LOCK();
    }
#endif /* RBUF_RTOS */
    ulUsed = BUFFER_GetFull(&(pxCtrl->xRbuf));
    if(ulUsed > ulCount)
    {
        ulUsed = ulCount;
    }
    if(ulUsed)
    {
        ulRead = BUFFER_Read(&(pxCtrl->xRbuf), pvData, ulUsed);
    #if RBUF_TEST
        pxCtrl->ulTotalReadCnt += ulRead;
    #endif /* RBUF_TEST */
    }
#if RBUF_RTOS
    if(!isr)
    {
        RBUF_UNLOCK();
    }
#endif /* RBUF_RTOS */

    return ulRead;
}

uint32_t RbufGetFree(RbufHandle_t xHandle)
{
    RbufCtrl_t *pxCtrl = RBUF_GET_CTRL(xHandle);
    ASSERT((NULL != pxCtrl) && pxCtrl->bInit);
    return BUFFER_GetFree(&(pxCtrl->xRbuf));
}

uint32_t RbufGetFull(RbufHandle_t xHandle)
{
    RbufCtrl_t *pxCtrl = RBUF_GET_CTRL(xHandle);
    ASSERT((NULL != pxCtrl) && pxCtrl->bInit);
    return BUFFER_GetFull(&(pxCtrl->xRbuf));
}

#if RBUF_DEBUG

RbufStatus_t RbufShowStatus(RbufHandle_t xHandle)
{
    RbufCtrl_t *pxCtrl = RBUF_GET_CTRL(xHandle);
    ASSERT(NULL != pxCtrl);
    TRACE("RbufShowStatus\n");
    TRACE("    TotalWriteCnt: %d\n", pxCtrl->ulTotalWriteCnt);
    TRACE("    TotalReadCnt : %d\n", pxCtrl->ulTotalReadCnt);
    TRACE("    BufFullErrCnt: %d\n", pxCtrl->ulBufFullErrCnt);
    TRACE("    BufMsgErrCnt : %d\n", pxCtrl->ulBufMsgErrCnt);
    return RBUF_STATUS_OK;
}

#else

RbufStatus_t RbufShowStatus(RbufHandle_t xHandle)
{
    return RBUF_STATUS_OK;
}

#endif /* RBUF_DEBUG */

static int inHandlerMode (void)
{
    return __get_IPSR() != 0;
}

#endif /* RBUF_ENABLE */
