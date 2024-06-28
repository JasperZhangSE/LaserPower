/*
    Include.h

    Include File for User Application
*/

/* Copyright 2019 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 21Aug19, Karl Created
*/

#ifndef __APP_INCLUDE_H__
#define __APP_INCLUDE_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

/* Includes */

/* Std C */
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

/* Lwip */
#include "lwip/sockets.h"
#include "lwip/dhcp.h"
 
/* RTOS */
#include <cmsis_os.h>
#include <portable.h>

/* HAL */
#include <stm32f1xx_hal.h>

/* Type & config */
#include "Config.h"

/* User common library */
#include "Include/Include.h"
#include "Debug/Debug.h"
#include "Mem/Mem.h"
#include "Mem/MemFram.h"
#include "Net/Lwip.h"
#include "Wdog/Wdog.h" 

/* Bsp library */
#include "BspSpi.h"

/* User application */
#include "User/_Config.h"
#include "User/_Include.h"

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __APP_INCLUDE_H__ */
