/*
    Debug.h

    Head File for Debug Module
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

#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

/* Includes */
#include <stdint.h>
#include "Include/Include.h"
#include "Debug/DebugConfig.h"
#include "Debug/DebugUart.h"

/* Types */
typedef enum {
    DEBUG_CHAN_NULL = 0,
    DEBUG_CHAN_UART = 0x01,
    DEBUG_CHAN_TCP  = 0x02,
    DEBUG_CHAN_UDP  = 0x04,
}DebugChan_t;

/* Functions */
Status_t        DebugInit(void);
Status_t        DebugTerm(void);

Status_t        DebugChanSet(DebugChan_t xDebugChan);
DebugChan_t     DebugChanGet(void);

Status_t        DebugPrintf(char *cFormat, ...);

#if DEBUG_TEST
Status_t        DebugTest(void);
#endif /* DEBUG_TEST */

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif /*__DEBUG_H__*/
