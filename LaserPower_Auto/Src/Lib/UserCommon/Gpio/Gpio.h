/*
    Gpio.h

    Head File for Gpio Module
*/

/* Copyright 2018 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 20Sep18, Karl Created
    01b, 13Jul19, Karl Reconstructured Gpio lib
    01c, 26Aug19, Karl Added GpioToggleOutput function
    01d, 26Aug19, Karl Added array access boundary protection
*/

#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

/* Includes */
#include <stdint.h>
#include "Include/Include.h"
#include "Gpio/GpioConfig.h"

/* Defines */
#define GPIO_ON         (1)     /* GPIO output 1 */
#define GPIO_OFF        (0)     /* GPIO output 0 */
#define GPIO_DIR_OUT    (1)     /* GPIO direction output */
#define GPIO_DIR_IN     (0)     /* GPIO direction input */
#define GPIO_INVALID    (0xFFFF)

/* Types */
typedef struct {
    uint16_t usSeq;
    uint32_t ulPort;
    uint16_t usPin;
    uint32_t ulInitValue;
    uint32_t ulMode;
    uint32_t ulPullUp;
    uint32_t ulSpeed;
#if GPIO_DESP
    char     cDesp[8];  
#endif /* GPIO_DESP */
}GpioCtrl_t;

/* Functions */
Status_t    GpioInit(void);
Status_t    GpioTerm(void);

Status_t    GpioPortConfig(uint32_t ulPort, uint32_t ulPin, uint32_t ulInitValue, uint32_t ulMode, uint32_t ulPullUp, uint32_t ulSpeed);
Status_t    GpioSetOutput(uint16_t usSeq, uint16_t usSw);
uint16_t    GpioGetOutput(uint16_t usSeq);
Status_t    GpioToggleOutput(uint16_t usSeq);
uint16_t    GpioGetInput(uint16_t usSeq);

Status_t    GpioShowStatus(void);

#if GPIO_TEST
Status_t    GpioTest(void);
#endif /* GPIO_TEST */

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif /* __GPIO_H__ */
