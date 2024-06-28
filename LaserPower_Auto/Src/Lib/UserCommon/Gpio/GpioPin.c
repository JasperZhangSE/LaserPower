/*
    GpioPin.c

    Implementation File for Gpio Pin Module
*/

/* Copyright 2018 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 20Sep18, Karl Created
    01b, 13Jul19, Karl Reconstructured Gpio lib
*/

/* Includes */
#include "Gpio/Gpio.h"
#include "Gpio/GpioPin.h"

#if GPIO_ENABLE

/* Debug Config */
#if GPIO_DEBUG
    #undef TRACE
    #define TRACE(...)  DebugPrintf(__VA_ARGS__)
#else
    #undef TRACE
    #define TRACE(...)
#endif /* GPIO_DEBUG */
#if GPIO_ASSERT
    #undef ASSERT
    #define ASSERT(a)   while(!(a)){DebugPrintf("ASSERT failed: %s %d\n", __FILE__, __LINE__);}
#else
    #undef ASSERT
    #define ASSERT(...)
#endif /* GPIO_ASSERT */

/* Pragmas */
#pragma diag_suppress 1296 /* warning: #1296-D: extended constant initialiser used */

/* Global variables */
/* User should define the table somewhere, this is only an example */
#if GPIO_DESP
/*
 * GpioCtrl_t gGpioOut[] = {
 *      usSeq,          ulPort,             usPin               ulInitValue,    ulMode,                 ulPullUp,       ulSpeed,                cDesp
 *      {DEMO_oGPIO,    PORT(DEMO_oGPIO),   PIN(DEMO_oGPIO),    GPIO_OFF,       GPIO_MODE_OUTPUT_PP,    GPIO_PULLUP,    GPIO_SPEED_FREQ_LOW,    "DEMOo"},
 *
 * };
 *
 * GpioCtrl_t gGpioIn[] = {
 *      usSeq,          ulPort,             usPin                ulInitValue,   ulMode,                 ulPullUp,       ulSpeed,                cDesp
 *      {DEMO_iGPIO,    PORT(DEMO_iGPIO),   PIN(DEMO_iGPIO),    GPIO_OFF,       GPIO_MODE_INPUT,        GPIO_PULLUP,    GPIO_SPEED_FREQ_LOW,    "DEMOi"},
 *
 * };
 */
#else
/*
 * GpioCtrl_t gGpioOut[] = {
 *      usSeq,          ulPort,             usPin               ulInitValue,    ulMode,                 ulPullUp,       ulSpeed,           
 *      {DEMO_oGPIO,    PORT(DEMO_oGPIO),   PIN(DEMO_oGPIO),    GPIO_OFF,       GPIO_MODE_OUTPUT_PP,    GPIO_PULLUP,    GPIO_SPEED_FREQ_LOW},
 *
 * };
 *
 * GpioCtrl_t gGpioIn[] = {
 *      usSeq,          ulPort,             usPin                ulInitValue,   ulMode,                 ulPullUp,       ulSpeed,           
 *      {DEMO_iGPIO,    PORT(DEMO_iGPIO),   PIN(DEMO_iGPIO),    GPIO_OFF,       GPIO_MODE_INPUT,        GPIO_PULLUP,    GPIO_SPEED_FREQ_LOW},
 *
 * };
 */    
#endif /* GPIO_DESP */

Status_t GpioPinInit(void)
{
    uint16_t    n            = 0;
    uint16_t    usGpioInNum  = 0;
    uint16_t    usGpioOutNum = 0;
    GpioCtrl_t* pxGpioIn     = NULL;
    GpioCtrl_t* pxGpioOut    = NULL;
    
    pxGpioIn     = GpioGetInCtrl();
    pxGpioOut    = GpioGetOutCtrl();
    usGpioInNum  = GpioGetInCtrlNum();
    usGpioOutNum = GpioGetOutCtrlNum();
    
    /* Init gpio out */
    if ((NULL != pxGpioOut) && (0 != usGpioOutNum)) {
        for (n = 0; n < usGpioOutNum; n++) {
            GpioPortConfig(pxGpioOut[n].ulPort, pxGpioOut[n].usPin, pxGpioOut[n].ulInitValue, pxGpioOut[n].ulMode, pxGpioOut[n].ulPullUp, pxGpioOut[n].ulSpeed);
        }
    }

    /* Init gpio in */
    if ((NULL != pxGpioIn) && (0 != usGpioInNum)) {
        for(n = 0; n < usGpioInNum; n++) {
            GpioPortConfig(pxGpioIn[n].ulPort, pxGpioIn[n].usPin, pxGpioIn[n].ulInitValue, pxGpioIn[n].ulMode, pxGpioIn[n].ulPullUp, pxGpioIn[n].ulSpeed);
        }
    }
        
    return STATUS_OK;
}

Status_t GpioPinTerm(void)
{
    /* Do nothing */
    return STATUS_OK;
}

__weak GpioCtrl_t* GpioGetOutCtrl(void)
{
    /* TODO: GpioPin.c -> GetGpioOutCtrl() should be implemented by module user! */
    return NULL;
}

__weak GpioCtrl_t* GpioGetInCtrl(void)
{
    /* TODO: GpioPin.c -> GetGpioInCtrl() should be implemented by module user! */
    return NULL;
}

__weak uint16_t GpioGetOutCtrlNum(void)
{
    /* TODO: GpioPin.c -> GetGpioOutCtrlNum() should be implemented by module user! */
    return 0;
}

__weak uint16_t GpioGetInCtrlNum(void)
{
    /* TODO: GpioPin.c -> GetGpioInCtrlNum() should be implemented by module user! */
    return 0;
}

#endif /* GPIO_ENABLE */
