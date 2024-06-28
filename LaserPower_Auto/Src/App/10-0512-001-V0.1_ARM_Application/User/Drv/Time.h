/*
    Time.h

    Head File for Time Module
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 17Nov23, Karl Created
*/

#ifndef ___TIME_H__
#define ___TIME_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Functions */
Status_t DrvTimeInit(void);
Status_t DrvTimeTerm(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ___TIME_H__ */
