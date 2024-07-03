/*
    Include.h

    Include file for Application
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 13Nov23, Karl Created
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
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/* RTOS */
#include <cmsis_os.h>  

/* HAL */
#include <stm32f1xx_hal.h>

/* Type & config */
#include "Config.h"

/* User common library */
#include "Include/Include.h"
#include "Cli/Cli.h"
#include "Debug/Debug.h"
#include "Gpio/Gpio.h"
#include "Mem/Mem.h"
#include "Mem/MemFram.h"
#include "Mqtt/Mqtt.h"
#include "Net/Net.h"
#include "Prot/Prot.h"
#include "Rbuf/Rbuf.h"
#include "I2c/I2c.h"
#include "Aht/Aht30.h"
#include "Rtc/Rtc.h"
#include "Uart/Uart.h"
#include "Wdog/Wdog.h"

/* Bsp library */
#include "BspGpio.h"
#include "BspSpi.h"
#include "BspUart.h"
#include "BspTim.h"

/* User application */
#include "User/_Type.h"
#include "User/_Config.h"
#include "User/_Include.h"
#include "User/Drv/Adc.h"
#include "User/Drv/Can.h"
#include "User/Drv/Dac.h"
#include "User/Drv/Gpio.h"
#include "User/Drv/Mem.h"
#include "User/Drv/Net.h"
#include "User/Drv/Pwr.h"
#include "User/Drv/Pwr/Pwr1Prot.h"
#include "User/Drv/Pwr/Pwr2Prot.h"
#include "User/Drv/Stc.h"
#include "User/Drv/Time.h"
#include "User/Drv/Pwm.h"
#include "User/Cli.h"
#include "User/Com.h"
#include "User/Data.h"
#include "User/Sys.h"

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __APP_INCLUDE_H__ */
