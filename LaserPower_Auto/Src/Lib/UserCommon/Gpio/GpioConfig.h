/*
    GpioConfig.h

    Configuration File for Gpio Module
*/

/* Copyright 2018 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 13Jul19, Karl Created
*/

#ifndef __GPIO_CONFIG_H__
#define __GPIO_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

/* Includes */
#include "Config.h"

/* Configuration defines */
#ifndef GPIO_ENABLE
#define GPIO_ENABLE         (0)
#endif
#ifndef GPIO_RTOS
#define GPIO_RTOS           (1)
#endif
#ifndef GPIO_DEBUG
#define GPIO_DEBUG          (0)
#endif
#ifndef GPIO_TEST
#define GPIO_TEST           (0)
#endif
#ifndef GPIO_ASSERT
#define GPIO_ASSERT         (0)
#endif
#ifndef GPIO_DESP
#define GPIO_DESP           (0)
#endif

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif /* __GPIO_CONFIG_H__ */
