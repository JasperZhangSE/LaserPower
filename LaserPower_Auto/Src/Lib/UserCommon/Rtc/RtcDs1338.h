/*
    Ds1302.h

    Head File for Ds1302 Module
*/

/* Copyright 2018 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 20Sep18, David Created
    01b, 13Nov18, Karl Modified
*/

#ifndef __RTC_DS1338_H__
#define __RTC_DS1338_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

/* Includes */
#include <stdint.h>
#include <stm32f1xx_hal.h>
#include "Include/Include.h"
#include "Rtc/RtcConfig.h"


// Device address
#define DS1338_ADDR 0x68

/**
If set, in an hour register (DS1338_REG_HOURS, DS1338_REG_A1_HOUR,
DS1338_REG_A2_HOUR, the hour is between 0 and 12, and the
(!AM)/PM bit indicates AM or PM.

If not set, the hour is between 0 and 23.
*/
#define DS1338_HOUR_12          (0x01 << 6)

/**
If DS1338_HOUR_12 is set:
- If set, indicates PM
- If not set, indicates AM
*/
#define DS1338_PM_MASK          (0x01 << 5)

// If set, the oscillator has stopped since the last time
// this bit was cleared.
#define DS1338_OSC_STOP_FLAG    (0x01 << 5)

// Set to disable the oscillator
#define DS1338_OSC_DISABLE      (0x01 << 7)

/**
These options control the behavior of the SQW/(!INTB) pin. 
*/
#define DS1338_SQWE_FLAG        (0x01 << 4)
#define DS1338_SQW_MASK         (0x03)
#define DS1338_SQW_32768HZ      (0x03)
#define DS1338_SQW_8192HZ       (0x02)
#define DS1338_SQW_4096HZ       (0x01)
#define DS1338_SQW_1HZ          (0x00)

// Occurs when the number of I2C bytes available is less than the number requested.
#define READ_ERROR 5

#define decode_bcd(x) ((x >> 4) * 10 + (x & 0x0F))
#define encode_bcd(x) ((((x / 10) & 0x0F) << 4) + (x % 10))


/* Functions */
Time_t RtcDs1338ReadTime(void);
Status_t RtcDs1338WriteTime(Time_t xTm);
Status_t RtcDs1338Init(void);


#if (RTC_STDC_TIME == 2)
time_t time(time_t* timer); 
#endif /* (RTC_STDC_TIME == 2) */

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif /* __RTC_DS1302_H__ */
