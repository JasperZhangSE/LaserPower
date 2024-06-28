/*
    Boot.h
    
    Head File for Application Boot Module
*/

/* Copyright 2022 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 08Apr22, Karl Created
*/

#ifndef __APP_BOOT_H__
#define __APP_BOOT_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

/* Includes */
#include "Include.h"

/* Defines */
#ifndef APP_BOOT_ENABLE_WDOG
#define APP_BOOT_ENABLE_WDOG    (1)
#endif

/* Functions */
Status_t BootInit(void);
Status_t BootTerm(void);

Status_t BootRun(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __APP_BOOT_H__ */
