/*
    BspGpio.c

    Implementation File for STM32 BSP GPIO Module
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
   Modification History
   --------------------
   01a 13Nov23 Ziv Created
*/

/* Includes */
#include "BspGpio.h"

/* Debug Config */
#if GPIO_DEBUG
#undef TRACE
#define TRACE(...) DebugPrinf(__VA_ARGS__)
#else
#undef TRACE
#define TRACE(...)
#endif /* GPIO_DEBUG */
#if GPIO_ASSERT
#undef ASSERT
#define ASSERT(a)                                                                                                      \
    while (!(a)) {                                                                                                     \
        DebugPrintf("ASSERT failed: %s %d\n", __FILE__, __LINE__);                                                     \
    }
#else
#undef ASSERT
#define ASSERT(...)
#endif /* GPIO_ASSERT */

/* Pragmas */
#pragma diag_suppress 1296 /* warning: #1296-D: extended constant initialiser used */

/* Global variables */
static GpioCtrl_t s_xGpioOut[] = {  
    {LED1,          PORT(LED1),         PIN(LED1),          GPIO_OFF,   GPIO_MODE_OUTPUT_PP,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},
    {LED2,          PORT(LED2),         PIN(LED2),          GPIO_OFF,   GPIO_MODE_OUTPUT_PP,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},
    {LED3,          PORT(LED3),         PIN(LED3),          GPIO_OFF,   GPIO_MODE_OUTPUT_PP,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},
    {LED4,          PORT(LED4),         PIN(LED4),          GPIO_OFF,   GPIO_MODE_OUTPUT_PP,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},
    {LED5,          PORT(LED5),         PIN(LED5),          GPIO_OFF,   GPIO_MODE_OUTPUT_PP,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},
    {LED6,          PORT(LED6),         PIN(LED6),          GPIO_OFF,   GPIO_MODE_OUTPUT_PP,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},

    {LED_CAN_OK,    PORT(LED_CAN_OK),   PIN(LED_CAN_OK),    GPIO_OFF,   GPIO_MODE_OUTPUT_PP,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},
    {LED_CAN_FAIL,  PORT(LED_CAN_FAIL), PIN(LED_CAN_FAIL),  GPIO_OFF,   GPIO_MODE_OUTPUT_PP,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},
    {LED_AC_OK,     PORT(LED_AC_OK),    PIN(LED_AC_OK),     GPIO_OFF,   GPIO_MODE_OUTPUT_PP,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},
    {LED_AC_FAIL,   PORT(LED_AC_FAIL),  PIN(LED_AC_FAIL),   GPIO_OFF,   GPIO_MODE_OUTPUT_PP,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},
    {LED_DC_OK,     PORT(LED_DC_OK),    PIN(LED_DC_OK),     GPIO_OFF,   GPIO_MODE_OUTPUT_PP,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},
    {LED_DC_FAIL,   PORT(LED_DC_FAIL),  PIN(LED_DC_FAIL),   GPIO_OFF,   GPIO_MODE_OUTPUT_PP,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},
    
    {RS485a_EN,     PORT(RS485a_EN),    PIN(RS485a_EN),     GPIO_ON,    GPIO_MODE_OUTPUT_PP,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},

    {ETH_RST,       PORT(ETH_RST),      PIN(ETH_RST),       GPIO_ON,    GPIO_MODE_OUTPUT_PP,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},
//    {ETH_EN,        PORT(ETH_EN),       PIN(ETH_EN),        GPIO_ON,    GPIO_MODE_OUTPUT_PP,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},

    {MPWR_EN,       PORT(MPWR_EN),      PIN(MPWR_EN),       GPIO_OFF,   GPIO_MODE_OUTPUT_PP,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},

    {APWR1_EN,      PORT(APWR1_EN),     PIN(APWR1_EN),      GPIO_OFF,   GPIO_MODE_OUTPUT_PP,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},
    {APWR2_EN,      PORT(APWR2_EN),     PIN(APWR2_EN),      GPIO_OFF,   GPIO_MODE_OUTPUT_PP,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},
    {APWR3_EN,      PORT(APWR3_EN),     PIN(APWR3_EN),      GPIO_OFF,   GPIO_MODE_OUTPUT_PP,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},

    {I2C_SCL,       PORT(I2C_SCL),      PIN(I2C_SCL),       GPIO_OFF,   GPIO_MODE_OUTPUT_OD,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},
    {I2C_SDA,       PORT(I2C_SDA),      PIN(I2C_SDA),       GPIO_OFF,   GPIO_MODE_OUTPUT_OD,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},

    {FAULT,         PORT(FAULT),        PIN(FAULT),         GPIO_OFF,   GPIO_MODE_OUTPUT_PP,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},
    {EX_AD_EN,      PORT(EX_AD_EN),     PIN(EX_AD_EN),      GPIO_OFF,   GPIO_MODE_OUTPUT_PP,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},
    {MOD_EN,        PORT(MOD_EN),       PIN(MOD_EN),        GPIO_OFF,   GPIO_MODE_OUTPUT_PP,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},
    {BEEP_SW,       PORT(BEEP_SW),      PIN(BEEP_SW),       GPIO_OFF,   GPIO_MODE_OUTPUT_PP,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},
};

static GpioCtrl_t s_xGpioIn[] = {
    {MPWR_STAT_AC,  PORT(MPWR_STAT_AC),     PIN(MPWR_STAT_AC),  GPIO_OFF,   GPIO_MODE_INPUT,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},
    {MPWR_STAT_DC,  PORT(MPWR_STAT_DC),     PIN(MPWR_STAT_DC),  GPIO_OFF,   GPIO_MODE_INPUT,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},

    {APWR1_STAT,    PORT(APWR1_STAT),       PIN(APWR1_STAT),    GPIO_OFF,   GPIO_MODE_INPUT,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},
    {APWR2_STAT,    PORT(APWR2_STAT),       PIN(APWR2_STAT),    GPIO_OFF,   GPIO_MODE_INPUT,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},
    {APWR3_STAT,    PORT(APWR3_STAT),       PIN(APWR3_STAT),    GPIO_OFF,   GPIO_MODE_INPUT,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},

    {QBH_ON,        PORT(QBH_ON),           PIN(QBH_ON),        GPIO_OFF,   GPIO_MODE_INPUT,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},
    {EX_CTRL_EN,    PORT(EX_CTRL_EN),       PIN(EX_CTRL_EN),    GPIO_OFF,   GPIO_MODE_INPUT,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},
    {WATER_PRESS,   PORT(WATER_PRESS),      PIN(WATER_PRESS),   GPIO_OFF,   GPIO_MODE_INPUT,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},
    {WATER_CHILLER, PORT(WATER_CHILLER),    PIN(WATER_CHILLER), GPIO_OFF,   GPIO_MODE_INPUT,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},
    {SAFE_LOCK,     PORT(SAFE_LOCK),        PIN(SAFE_LOCK),     GPIO_OFF,   GPIO_MODE_INPUT,    GPIO_NOPULL,    GPIO_SPEED_FREQ_HIGH},

    {AIM_EN,        PORT(AIM_EN),           PIN(AIM_EN),        GPIO_OFF,   GPIO_MODE_INPUT,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},
    {LASER_EN,      PORT(LASER_EN),         PIN(LASER_EN),      GPIO_OFF,   GPIO_MODE_INPUT,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},
    {EX_MOD_EN,     PORT(EX_MOD_EN),        PIN(EX_MOD_EN),     GPIO_OFF,   GPIO_MODE_INPUT,    GPIO_PULLUP,    GPIO_SPEED_FREQ_HIGH},
};

/* Function */
GpioCtrl_t *GpioGetOutCtrl(void) {
    return s_xGpioOut;
}

GpioCtrl_t *GpioGetInCtrl(void) {
    return s_xGpioIn;
}

uint16_t GpioGetOutCtrlNum(void) {
    return sizeof(s_xGpioOut) / sizeof(s_xGpioOut[0]);
}

uint16_t GpioGetInCtrlNum(void) {
    return sizeof(s_xGpioIn) / sizeof(s_xGpioIn[0]);
}
