/*
    Rtc.h

    Head File for Rtc Module
*/

/* Copyright 2019 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 15Jul19, Karl Created
    01b, 26Aug19, Karl Modified include files
*/

#ifndef __RTC_H__
#define __RTC_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

/* Includes */
#include <stdint.h>
#include "Include/Include.h"
#include "Rtc/RtcConfig.h"
#include "Rtc/RtcDs1302.h"
#include "Rtc/RtcDs1338.h"

/* Types */
typedef enum {
    RTC_TYPE_STM32,
    RTC_TYPE_DS1302,
    RTC_TYPE_DS1338,
    RTC_TYPE_SIZE
}RtcType_t;

/* Functions */
Status_t    RtcInit(void);
Status_t    RtcTerm(void);
Status_t    RtcConfig(RtcType_t xRtcType, const void *pvConfig, uint16_t usConfigSize);

Time_t      RtcReadTime(RtcType_t xRtcType);
Status_t    RtcWriteTime(RtcType_t xRtcType, Time_t xTm);

#if RTC_TEST
Status_t    RtcTest(uint8_t ucSel);
#endif /* RTC_TEST */

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif /* __RTC_H__ */
