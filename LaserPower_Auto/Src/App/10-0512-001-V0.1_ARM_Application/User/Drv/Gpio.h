/*
    Gpio.h

    Head File for Gpio Module
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 17Nov23, Karl Created
    01b, 22Nov23, Karl Implemented di and do command
*/

#ifndef ___GPIO_H__
#define ___GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Functions */
Status_t DrvGpioInit(void);
Status_t DrvGpioTerm(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ___GPIO_H__ */
