/*
    Debug.c

    Implementation File for Debug Module
*/

/* Copyright 2019 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 12Sep18, Karl Created
    01b, 15Nov18, Karl Modified
    01c, 15Jul19, Karl Reconstructured Debug module
    01d, 26Aug19, Karl Modified include files
*/

/* Includes */
#include <stdio.h>
#include <stdarg.h>
#include <cmsis_os.h>
#include "Debug/Debug.h"

#if DEBUG_ENABLE

/* Pragmas */
#pragma diag_suppress 550 /* warning: #550-D: variable was set but never used */

/* Debug config */
#if DEBUG_DEBUG
    #undef TRACE
    #define TRACE(...)
#else
    #undef TRACE
    #define TRACE(...)
#endif /* DEBUG_DEBUG */
#if DEBUG_ASSERT
    #undef ASSERT
    #define ASSERT(a)   while(!(a));
#else
    #undef ASSERT
    #define ASSERT(...)
#endif /* DEBUG_ASSERT */

/* Local variables */
static DebugChan_t  s_xDebugChan = DEBUG_CHAN_NULL;
static osMutexId    s_xDebugMutex = NULL;
static Bool_t       s_bInit = FALSE;

/* Forward declaration */
static int      prvIsInIsr(void);
static void     prvDebugLock(void);
static void     prvDebugUnlock(void);

/* Functions */
Status_t DebugInit(void)
{
    osMutexDef(DebugMutex);
    s_xDebugMutex = osMutexCreate(osMutex(DebugMutex));
    s_bInit = TRUE;
    return STATUS_OK;
}

Status_t DebugTerm(void)
{
    return STATUS_OK;
}

Status_t DebugChanSet(DebugChan_t xDebugChan)
{
    s_xDebugChan = xDebugChan;
    return STATUS_OK;
}

DebugChan_t DebugChanGet(void)
{
    return s_xDebugChan;
}

Status_t DebugPrintf(char *cFormat, ...)
{
    uint16_t    usLength = 0;
    Status_t    xRet = STATUS_OK;
    static char cBuf[DEBUG_BUF_SIZE];

    ASSERT(s_bInit);
    /* "TCP & UDP" does not support printf function in isr */
    if (((DEBUG_CHAN_TCP == s_xDebugChan)|| (DEBUG_CHAN_UDP == s_xDebugChan)) && prvIsInIsr()) {
        return STATUS_ERR;
    }
    
    /* Enter critical section */
    prvDebugLock();
    
    va_list va;
    va_start(va, cFormat);
    usLength = vsprintf(cBuf, cFormat, va);
    switch(s_xDebugChan) {
#if DEBUG_ENABLE_UART
    case DEBUG_CHAN_UART:
        xRet = DebugUartPrintfDirect(cBuf, usLength);
        break;
#endif /* DEBUG_ENABLE_UART */
#if DEBUG_ENABLE_TCP
    case DEBUG_CHAN_TCP:
        xRet = DebugTcpPrintfDirect(cBuf, usLength);
        break;
#endif /* DEBUG_ENABLE_TCP */
#if DEBUG_ENABLE_UDP
    case DEBUG_CHAN_UDP:
        xRet = DebugUdpPrintfDirect(cBuf, usLength);
        break;
#endif /* DEBUG_ENABLE_UDP */
    default: 
        /* Do nothing! */
        break;
    }
    va_end(va);
    
    /* Leave critical section */
    prvDebugUnlock();
    
    return xRet;
}

static int prvIsInIsr(void)
{
    return __get_IPSR();
}

static void prvDebugLock(void)
{
    ASSERT(s_bInit);
    if (prvIsInIsr() != 0) {
        taskDISABLE_INTERRUPTS();
    }
    else {
    #if 0
        taskENTER_CRITICAL();
    #else
        ASSERT(NULL != s_xDebugMutex);
        osMutexWait(s_xDebugMutex, osWaitForever);
    #endif
    }
    return;
}

static void prvDebugUnlock(void)
{
    ASSERT(s_bInit);
    if (prvIsInIsr() != 0) {
        taskENABLE_INTERRUPTS();
    }
    else {
    #if 0
        taskEXIT_CRITICAL();
    #else
        ASSERT(NULL != s_xDebugMutex);
        osMutexRelease(s_xDebugMutex);
    #endif
    }
    return;
}

#endif /* DEBUG_ENABLE */
