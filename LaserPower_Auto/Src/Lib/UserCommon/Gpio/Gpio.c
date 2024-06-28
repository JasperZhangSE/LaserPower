/*
    Gpio.c

    Implementation File for Gpio Module
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

/* Includes */
#include <stm32f1xx_hal.h>
#include "Gpio/Gpio.h"
#include "Gpio/GpioPin.h"

#if GPIO_ENABLE

/* Pragmas */
#pragma diag_suppress 550 /* warning: #550-D: variable was set but never used */

/* Debug config */
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

/* Local variables */
static Bool_t       s_bInit         = FALSE;
static uint16_t     s_usGpioOutSize = 0;
static uint16_t     s_usGpioInSize  = 0;
static GpioCtrl_t*  s_pxGpioOutCtrl = NULL;
static GpioCtrl_t*  s_pxGpioInCtrl  = NULL;

/* Forward declaration */
static uint16_t prvGpioGet(GpioCtrl_t *pxSw);
static Status_t prvGpioCtrl(GpioCtrl_t *pxSw, uint16_t usSw);
static Status_t prvGpioToggle(GpioCtrl_t *pxSw);

/* Functions */
Status_t GpioInit(void)
{
    /* Init local variables */
    /* ATTENTION: "GetGpioInCtrl, GetGpioOutCtrl, GetGpioInCtrlNum, GetGpioOutCtrlNum" should supplied by user! */
    s_pxGpioInCtrl  = GpioGetInCtrl();
    s_pxGpioOutCtrl = GpioGetOutCtrl();
    s_usGpioInSize  = GpioGetInCtrlNum();
    s_usGpioOutSize = GpioGetOutCtrlNum();
    
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    
    /* Get GPIO definition table and config GPIO pins */
    GpioPinInit();
    
    /* Set module init mark */
    s_bInit = TRUE;
    
    return STATUS_OK;
}

Status_t GpioTerm(void)
{
    /* Do nothing */
    return STATUS_OK;
}

Status_t GpioPortConfig(uint32_t ulPort, uint32_t ulPin, uint32_t ulInitValue, uint32_t ulMode, uint32_t ulPullUp, uint32_t ulSpeed)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin((GPIO_TypeDef*)ulPort, ulPin, (GPIO_PinState)ulInitValue);

    /*Configure GPIO pins */
    GPIO_InitStruct.Pin     = ulPin;
    GPIO_InitStruct.Mode    = ulMode;
    GPIO_InitStruct.Pull    = ulPullUp;
    GPIO_InitStruct.Speed   = ulSpeed;
    HAL_GPIO_Init((GPIO_TypeDef*)ulPort, &GPIO_InitStruct);

    return STATUS_OK;
}

Status_t GpioSetOutput(uint16_t usSeq, uint16_t usSw)
{
    ASSERT(s_bInit);
    ASSERT(usSeq < s_usGpioOutSize);
    if (usSeq < s_usGpioOutSize) {
        prvGpioCtrl(&s_pxGpioOutCtrl[usSeq], usSw);
        return STATUS_OK;
    }
    else {
        return STATUS_ERR;
    }
}

uint16_t GpioGetOutput(uint16_t usSeq)
{
    ASSERT(s_bInit);
    ASSERT(usSeq < s_usGpioOutSize);
    if (usSeq < s_usGpioOutSize) {
        uint16_t ret;
        ret = prvGpioGet(&s_pxGpioOutCtrl[usSeq]);
        (ret == 0) ? (ret = 0) : (ret = 1);
        return ret;
    }
    else {
        return GPIO_INVALID;
    }
}

Status_t GpioToggleOutput(uint16_t usSeq)
{
    ASSERT(s_bInit);
    ASSERT(usSeq < s_usGpioOutSize);
    if (usSeq < s_usGpioOutSize) {
        prvGpioToggle(&s_pxGpioOutCtrl[usSeq]);
        return STATUS_OK;
    }
    else {
        return STATUS_ERR;
    }
}

uint16_t GpioGetInput(uint16_t usSeq)
{
    ASSERT(s_bInit);
    ASSERT(usSeq < s_usGpioInSize);
    if (usSeq < s_usGpioInSize) {
        uint16_t ret;
        ret = prvGpioGet(&s_pxGpioInCtrl[usSeq]);
        (ret == 0) ? (ret = 0) : (ret = 1);
        return ret;
    }
    else {
        return GPIO_INVALID;
    }
}

#if GPIO_DEBUG
    
Status_t GpioShowStatus(void)
{
    uint16_t n = 0;
    uint16_t ret = 0;
    
    /* Display current GPIO input status */
    TRACE("GPIO Input\n");
    for (n = 0; n < s_usGpioInSize; n++) {
        ret = GpioGetInput(n);
    #if GPIO_DESP
        TRACE("    GPIO-%02d: %d, %s\n", n+1, ret, s_pxGpioOutCtrl[n].cDesp);
    #else
        TRACE("    GPIO-%02d: %d\n", n+1, ret);
    #endif /* GPIO_DESP */
    }
    
    /* Display current GPIO output status */
    TRACE("GPIO Output\n");
    for (n = 0; n < s_usGpioOutSize; n++) {
        ret = GpioGetOutput(n);
    #if GPIO_DESP
        TRACE("    GPIO-%02d: %d, %s\n", n+1, ret, s_pxGpioOutCtrl[n].cDesp);
    #else
        TRACE("    GPIO-%02d: %d\n", n+1, ret);
    #endif /* GPIO_DESP */
    }
    
    return STATUS_OK;
}

#else

Status_t GpioShowStatus(void)
{
    return STATUS_OK;
}

#endif /* GPIO_DEBUG */

static uint16_t prvGpioGet(GpioCtrl_t *pxSw)
{
    uint16_t data = 0;
    data = (uint16_t)HAL_GPIO_ReadPin((GPIO_TypeDef*)(pxSw->ulPort), pxSw->usPin);
    return data;
}

static Status_t prvGpioCtrl(GpioCtrl_t *pxSw, uint16_t usSw)
{
    if (usSw >= GPIO_ON) {
        HAL_GPIO_WritePin((GPIO_TypeDef*)(pxSw->ulPort), pxSw->usPin, GPIO_PIN_SET);
    }
    else {
        HAL_GPIO_WritePin((GPIO_TypeDef*)(pxSw->ulPort), pxSw->usPin, GPIO_PIN_RESET);
    }
#if GPIO_DESP
    TRACE("prvGpioCtrl: Seq-%02d Ctrl-%02d Desp-%s\n", pxSw->usSeq, usSw, pxSw->cDesp);
#else
    TRACE("prvGpioCtrl: Seq-%02d Ctrl-%02d\n", pxSw->usSeq, usSw);
#endif /* GPIO_DESP */

    return STATUS_OK;
}

static Status_t prvGpioToggle(GpioCtrl_t *pxSw)
{
    HAL_GPIO_TogglePin((GPIO_TypeDef*)(pxSw->ulPort), pxSw->usPin);
#if GPIO_DESP
    TRACE("prvGpioToggle: Seq-%02d Desp-%s\n", pxSw->usSeq, pxSw->cDesp);
#else
    TRACE("prvGpioToggle: Seq-%02d\n", pxSw->usSeq);
#endif /* GPIO_DESP */

    return STATUS_OK;
}

#endif /* GPIO_ENABLE */
