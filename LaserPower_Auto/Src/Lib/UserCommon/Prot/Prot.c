/*
    Prot.c

    Implementation File for Prot Module
*/

/* Copyright 2019 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 22Nov18, Karl Created
    01b, 13Jul19, Karl Reconstructured Prot library
*/

/* Includes */
#include <stdlib.h>
#include <string.h>
#include "Prot/Prot.h"

#if PROT_ENABLE

/* Pragmas */
#pragma diag_suppress 111 /* warning: #111-D: statement is unreachable */

/* Debug config */
#if PROT_DEBUG
    #undef TRACE
    #define TRACE(...)  DebugPrintf(__VA_ARGS__)
#else
    #undef TRACE
    #define TRACE(...)
#endif /* PROT_DEBUG */
#if PROT_ASSERT
    #undef ASSERT
    #define ASSERT(a)   while(!(a)){DebugPrintf("ASSERT failed: %s %d\n", __FILE__, __LINE__);}
#else
    #undef ASSERT
    #define ASSERT(...)
#endif /* PROT_ASSERT */

/* Local defines */
#define PROT_GET_CTRL(handle)       ((ProtCtrl_t*)(handle))
#define PROT_GET_LENGTH(type)       ((uint16_t)(type))

/* Local types */
typedef struct {
    Bool_t          bInit;                              /* Module init mark */
    
    uint8_t         bInitHead;                          /* Head init mark */
    uint8_t         ucHeadMarkSize;                     /* Head mark size */
    uint8_t         ucHeadSize;                         /* Total head size in byte */
    uint8_t         ucHeadMark[PROT_MAX_MARK_SIZE];     /* Head mark */
    
    Bool_t          bInitTail;                          /* Tail init mark */
    uint8_t         ucTailMarkSize;                     /* Tail mark size */
    uint8_t         ucTailSize;                         /* Total tail size in byte */
    uint8_t         ucTailMark[PROT_MAX_MARK_SIZE];     /* Tail mark */
    
    uint8_t         ucHeadTailSize;                     /* Head tail size in byte */
    
    Bool_t          bInitMisc;                          /* Misc init mark */
    uint32_t        ulMaxMsgSize;                       /* Max message size in byte */
    ProtLength_t    xLengthType;                        /* Length field type */
    uint8_t         ucLengthOffset;                     /* Length field offset */
    uint32_t        ulMsgLengthOffset;                  /* Msg length offset */
    
    Bool_t           bInitCb;                           /* Callback function init mark */
    ProtProcFunc_t   pxPktProc;                         /* Packet process callback function */
    ProtCheckFunc_t  pxPktChk;                          /* Packet check callback function */

#if PROT_TEST
    uint32_t        ulRxPktNum;                         /* Total recvd packet number */
    uint32_t        ulChkErrNum;                        /* Check code error number */
    uint32_t        ulLenErrNum;                        /* Length or tail code error number */
#endif /* PROT_TEST */
}ProtCtrl_t;

/* Forward declaration */
static uint32_t prvGetLength(IN uint8_t* pucBuf, ProtLength_t xLengthType, uint8_t ucLengthOffset);

/* Functions */
Status_t ProtInit(void)
{
    /* Do nothing */
    return STATUS_OK;
}

Status_t ProtTerm(void)
{
    /* Do nothing */
    return STATUS_OK;
}

ProtHandle_t ProtCreate(void)
{
    ProtCtrl_t *pxCtrl = NULL;
    pxCtrl = (ProtCtrl_t*)malloc(sizeof(ProtCtrl_t));
    memset(pxCtrl, 0, sizeof(ProtCtrl_t));
    return (ProtHandle_t)pxCtrl;
}

Status_t ProtDelete(ProtHandle_t xHandle)
{
    if(xHandle) {
        free((void*)xHandle);
    }
    return STATUS_OK;
}

Status_t ProtConfigHead(ProtHandle_t xHandle, const uint8_t* ucHeadMark, uint8_t ucHeadMarkSize, uint8_t ucHeadSize)
{
    ProtCtrl_t *pxCtrl = PROT_GET_CTRL(xHandle);
    
    ASSERT(NULL != pxCtrl);
    ASSERT(ucHeadMarkSize <= PROT_MAX_MARK_SIZE);
    memcpy(pxCtrl->ucHeadMark, ucHeadMark, ucHeadMarkSize);
    pxCtrl->ucHeadMarkSize = ucHeadMarkSize;
    pxCtrl->ucHeadSize     = ucHeadSize;
    pxCtrl->bInitHead      = TRUE;
    
    return STATUS_OK;
}

Status_t ProtConfigTail(ProtHandle_t xHandle, const uint8_t* pucTailMark, uint8_t ucTailMarkSize, uint8_t ucTailSize)
{
    ProtCtrl_t *pxCtrl = PROT_GET_CTRL(xHandle);
    
    ASSERT(NULL != pxCtrl);
    ASSERT(ucTailMarkSize <= PROT_MAX_MARK_SIZE);
    memcpy(pxCtrl->ucTailMark, pucTailMark, ucTailMarkSize);
    pxCtrl->ucTailMarkSize = ucTailMarkSize;
    pxCtrl->ucTailSize     = ucTailSize;
    pxCtrl->bInitTail      = TRUE;
    
    return STATUS_OK;
}

Status_t ProtConfigMisc(ProtHandle_t xHandle, uint32_t ulMaxMsgSize, uint32_t ulMsgLengthOffset, ProtLength_t xLengthType, uint8_t ucLengthOffset)
{
    ProtCtrl_t *pxCtrl = PROT_GET_CTRL(xHandle);
    
    ASSERT(NULL != pxCtrl);
    pxCtrl->ulMaxMsgSize      = ulMaxMsgSize;
    pxCtrl->xLengthType       = xLengthType;
    pxCtrl->ucLengthOffset    = ucLengthOffset;
    pxCtrl->ulMsgLengthOffset = ulMsgLengthOffset;
    pxCtrl->bInitMisc         = TRUE;
    
    return STATUS_OK;
}

Status_t ProtConfigCb(ProtHandle_t xHandle, ProtProcFunc_t pxPktProc, ProtCheckFunc_t pxPktChk)
{
    ProtCtrl_t *pxCtrl = PROT_GET_CTRL(xHandle);
    
    ASSERT(NULL != pxCtrl);
    pxCtrl->pxPktProc = pxPktProc;
    pxCtrl->pxPktChk  = pxPktChk;
    pxCtrl->bInitCb   = TRUE;
    
    return STATUS_OK;
}

Status_t ProtConfig(ProtHandle_t xHandle)
{
    ProtCtrl_t *pxCtrl = PROT_GET_CTRL(xHandle);
    
    ASSERT(NULL != pxCtrl);
    if (pxCtrl->bInitHead && pxCtrl->bInitTail && pxCtrl->bInitMisc && pxCtrl->bInitCb) {
        pxCtrl->ucHeadTailSize = pxCtrl->ucHeadSize + pxCtrl->ucTailSize;
        pxCtrl->bInit          = TRUE;
        return STATUS_OK;
    }
    else {
        pxCtrl->bInit = FALSE;
        return STATUS_ERR;
    }
}

Status_t ProtProc(ProtHandle_t xHandle, IN uint8_t* pucRecvBuf, IN uint16_t usRecvd, INOUT uint16_t *pusRecvIndex, INOUT uint8_t* pucProcBuf, IN void* pvPara)
{
    uint16_t n = 0;
    uint8_t* pucBuf = NULL;
    uint32_t ulLength = 0;
    uint16_t ucRecvIndex = 0;
    ProtCtrl_t* pxCtrl = PROT_GET_CTRL(xHandle);
    
    ASSERT(NULL != pxCtrl);
    TRACE("Enter ProtProc - %08X\n", xHandle);
    if(!pxCtrl->bInit) {
        TRACE("    ProtCtrl_t hasn't been initialised\n");
        return STATUS_ERR;
    }
    
    pucBuf = pucRecvBuf;
    ucRecvIndex = *pusRecvIndex;
    
    /* Find the user defined net packet and process it */
    for (n = 0; n < usRecvd; n++) {
    #if PROT_SHOW_CONT
        TRACE("0x%02X\n", (*(pucBuf+n))&0xFF);
    #endif /* PROT_SHOW_CONT */
        
        /* Check start mark */
        if (ucRecvIndex < pxCtrl->ucHeadMarkSize) {
            if (pxCtrl->ucHeadMark[ucRecvIndex] == (*(pucBuf+n))) {
                TRACE("    Find the %02X\n", *(pucBuf+n)&0xFF);
                pucProcBuf[ucRecvIndex++] = *(pucBuf+n);
                continue;
            }
            else {
                TRACE("    ERROR: head parse failed\n");
                ucRecvIndex = 0;
                continue;
            }
        }
        
        /* Accumulate the proc data buf */
        pucProcBuf[ucRecvIndex++] = *(pucBuf+n);
        
        /* Get the pkt data length */
        ASSERT(pxCtrl->xLengthType < PROT_LENGTH_TYPE_SIZE);
        Bool_t bGetLength = FALSE;
        if (ucRecvIndex >= (pxCtrl->ucLengthOffset+PROT_GET_LENGTH(pxCtrl->xLengthType))) {
            ulLength   = prvGetLength(pucProcBuf, pxCtrl->xLengthType, pxCtrl->ucLengthOffset);
            ulLength   += pxCtrl->ulMsgLengthOffset;
            bGetLength = TRUE;
            /* Verify the ulLength */
            if (ulLength > pxCtrl->ulMaxMsgSize) {
                /* The pkt is not correct, reset the receiving process! */
                ucRecvIndex = 0;
            #if PROT_TEST
                pxCtrl->ulLenErrNum++;
            #endif /* PROT_TEST */
                TRACE("    ERROR: data length is greater than %d\n", pxCtrl->ulMaxMsgSize);
                continue;
            }
        }
        
        if (bGetLength) {
            /* Check the tail code */
            if ((ucRecvIndex == ulLength) && (0 == memcmp(pucProcBuf + ucRecvIndex-pxCtrl->ucTailMarkSize, pxCtrl->ucTailMark, pxCtrl->ucTailMarkSize))) {
                /* Finally, we got the data we want, now we can process the data immediately! */
                TRACE("    Begin to process the data\n");
                
                if(pxCtrl->pxPktProc != NULL) {
                    uint8_t bChk = TRUE;
                    if(pxCtrl->pxPktChk != NULL) {
                        bChk = (pxCtrl->pxPktChk)(pucProcBuf, ulLength);
                    }
                    
                    if(bChk) {
                    #if PROT_TEST
                        pxCtrl->ulRxPktNum++;                
                    #endif /* PROT_TEST */
                        /* Parse and execute the command */
                        (pxCtrl->pxPktProc)(pucProcBuf,                                         /* Head */
                                            (const uint8_t*)(pucProcBuf+pxCtrl->ucHeadSize),    /* Content */
                                            ulLength-pxCtrl->ucHeadTailSize,                    /* Valid data length */
                                            pvPara                                              /* Other para */);  
                    }
                    else {
                    #if PROT_TEST
                        pxCtrl->ulChkErrNum++;
                    #endif /* PROT_TEST */
                        TRACE("    ERROR: calc check wrong\n");
                    }
                }
                
                ucRecvIndex = 0;
                continue;
            }
            else if (ucRecvIndex >= ulLength) {
            #if PROT_TEST
                pxCtrl->ulLenErrNum++;
            #endif /* PROT_TEST */
                TRACE("    ERROR: tail code or data length wrong\n");
                ucRecvIndex = 0;
                continue;
            }            
        }
    }
    
    *pusRecvIndex = ucRecvIndex;
    TRACE("Leave ProtProc - %08X\n", xHandle);
    
    return STATUS_OK;
}

static uint32_t prvGetLength(IN uint8_t* pucBuf, ProtLength_t xLengthType, uint8_t ucLengthOffset)
{
    uint32_t ulLength;
    
    switch(xLengthType) {
        case PROT_LENGTH_UINT8:     ulLength = *(uint8_t*)(pucBuf+ucLengthOffset);     break;
        case PROT_LENGTH_UINT16:    ulLength = *(uint16_t*)(pucBuf+ucLengthOffset);    break;
        case PROT_LENGTH_UINT32:    ulLength = *(uint32_t*)(pucBuf+ucLengthOffset);    break;
        default:                    ASSERT(0);                                          break;  /* We shoule never get here! */
    }
    
    return ulLength;
}

#if PROT_DEBUG

Status_t ProtShowPara(ProtHandle_t xHandle)
{
    uint16_t n = 0;
    ProtCtrl_t *pxCtrl = PROT_GET_CTRL(xHandle);
    
    ASSERT(NULL != pxCtrl);
    TRACE("ProtHandle %08X\n", xHandle);
    
    /********************************************************/
    TRACE("    bInit:             %d\n", pxCtrl->bInit);
    
    /********************************************************/
    TRACE("    bInitHead:         %d\n", pxCtrl->bInitHead);
    TRACE("    ucHeadMarkSize:    %d\n", pxCtrl->ucHeadMarkSize);
    TRACE("    ucHeadMark:        ");
    for(n = 0; n < pxCtrl->ucHeadMarkSize; n++)
    {
        TRACE("%02X ", pxCtrl->ucHeadMark[n] & 0xFF);   
    }
    TRACE("\n");
    TRACE("    ucHeadSize:        %d\n", pxCtrl->ucHeadSize);
    
    /********************************************************/
    TRACE("    bInitTail:         %d\n", pxCtrl->bInitTail);
    TRACE("    ucTailMarkSize:    %d\n", pxCtrl->ucTailMarkSize);
    TRACE("    ucTailMark:        ");
    for(n = 0; n < pxCtrl->ucTailMarkSize; n++)
    {
        TRACE("%02X ", pxCtrl->ucTailMark[n]);
    }
    TRACE("\n");
    TRACE("    ucTailSize:        %d\n", pxCtrl->ucTailSize);
    
    /********************************************************/
    TRACE("    ucHeadTailSize:    %d\n", pxCtrl->ucHeadTailSize);
    
    /********************************************************/
    TRACE("    bInitMisc:         %d\n", pxCtrl->bInitMisc);
    TRACE("    ulMaxMsgSize:      %d\n", pxCtrl->ulMaxMsgSize);
    TRACE("    xLengthType:       %d\n", pxCtrl->xLengthType);
    TRACE("    ucLengthOffset:    %d\n", pxCtrl->ucLengthOffset);
    TRACE("    ulMsgLengthOffset: %d\n", pxCtrl->ulMsgLengthOffset);
    
    /********************************************************/
    TRACE("    bInitCb:           %d\n", pxCtrl->bInitCb);
    TRACE("    pxPktProc:         %08X\n", pxCtrl->pxPktProc);
    TRACE("    pxPktChk:          %08X\n", pxCtrl->pxPktChk);
    
    /********************************************************/
#if PROT_TEST
    TRACE("    ulRxPktNum:        %d\n", pxCtrl->ulRxPktNum);
    TRACE("    ulChkErrNum:       %d\n", pxCtrl->ulChkErrNum);
    TRACE("    ulLenErrNum:       %d\n", pxCtrl->ulLenErrNum);
#endif /* PROT_TEST */

    return STATUS_OK;
}

#else
    
Status_t ProtShowPara(ProtHandle_t xHandle)
{
    return STATUS_OK;
}

#endif /* PROT_DEBUG */

#endif /* PROT_ENABLE */
