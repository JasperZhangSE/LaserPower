/*
    GpioPin.h

    Head File for Gpio Pin Module
*/

/* Copyright 2018 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 20Sep18, Karl Created
    01b, 13Jul19, Karl Reconstructured Gpio lib
*/

#ifndef __GPIO_PIN_H__
#define __GPIO_PIN_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

/* Includes */
#include "Gpio/Gpio.h"

/* Defines */
#define PIN(a)      (a##_Pin)
#define PORT(a)     (uint32_t)(a##_Pin_Port)

/* ATTENTION: User should supply GPIO pin port macro definitions */
/*
 * Define demo:
 * #define DEMO_Pin                 GPIO_PIN_1
 * #define DEMO_Pin_Port            GPIOA
 */

/* ATTENTION: User should supply GPIO pin sequence enum definitions */
/* Types */
/*
 * Gpio sequence type demo:
 * typedef enum {
 *     iGPIO1,
 *     iGPIO2,
 *     ......
 *     iGPIOn,
 *     GPIO_IN_SIZE,
 * }GpioIn_t;
 *
 * typedef enum {
 *     oGPIO1,
 *     oGPIO2,
 *     ......
 *     oGPIOn,
 *     GPIO_OUT_SIZE,
 * }GpioOut_t;
 */

/* Functions */
Status_t GpioPinInit(void);
Status_t GpioPinTerm(void);

__weak GpioCtrl_t* GpioGetOutCtrl(void);
__weak GpioCtrl_t* GpioGetInCtrl(void);

__weak uint16_t GpioGetOutCtrlNum(void);
__weak uint16_t GpioGetInCtrlNum(void);

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif /* __GPIO_PIN_H__ */
