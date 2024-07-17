/*
    BspGpio.h

    Head File for STM32 BSP GPIO Module
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
   Modification History
   --------------------
   01a 13Nov23 Ziv Created
*/

#ifndef __BSP_GPIO_H__
#define __BSP_GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes */
#include <stm32f1xx_hal.h>
#include "Include/Include.h "
#include "Gpio/Gpio.h "
#include "Gpio/GpioPin.h "
#include "User/_Config.h" /* XXX: BspGpio.h */

#ifndef REMAP_GPIO_LED_CTRL
#define REMAP_GPIO_LED_CTRL     0
#endif /* REMAP_GPIO_LED_CTRL */

/* Define */
#define DBG_SWDIO_Pin           GPIO_PIN_13
#define DBG_SWDIO_Pin_Port      GPIOA
#define DBG_SWCLK_Pin           GPIO_PIN_14
#define DBG_SWCLK_Pin_Port      GPIOA

#define LED1_Pin                GPIO_PIN_1
#define LED1_Pin_Port           GPIOE
#define LED2_Pin                GPIO_PIN_2
#define LED2_Pin_Port           GPIOE
#define LED3_Pin                GPIO_PIN_3
#define LED3_Pin_Port           GPIOE
#define LED4_Pin                GPIO_PIN_4
#define LED4_Pin_Port           GPIOE
#define LED5_Pin                GPIO_PIN_5
#define LED5_Pin_Port           GPIOE
#define LED6_Pin                GPIO_PIN_6
#define LED6_Pin_Port           GPIOE

#define LED_CAN_OK_Pin          GPIO_PIN_7
#define LED_CAN_OK_Pin_Port     GPIOE
#define LED_CAN_FAIL_Pin        GPIO_PIN_8
#define LED_CAN_FAIL_Pin_Port   GPIOE
#define LED_AC_OK_Pin           GPIO_PIN_9
#define LED_AC_OK_Pin_Port      GPIOE
#define LED_AC_FAIL_Pin         GPIO_PIN_10
#define LED_AC_FAIL_Pin_Port    GPIOE
#define LED_DC_OK_Pin           GPIO_PIN_4
#define LED_DC_OK_Pin_Port      GPIOC
#define LED_DC_FAIL_Pin         GPIO_PIN_10
#define LED_DC_FAIL_Pin_Port    GPIOB

#define FLASH_CS_Pin            GPIO_PIN_7
#define FLASH_CS_Pin_Port       GPIOD
#define FLASH_SCLK_Pin          GPIO_PIN_3
#define FLASH_SCLK_Pin_Port     GPIOB
#define FLASH_MISO_Pin          GPIO_PIN_4
#define FLASH_MISO_Pin_Port     GPIOB
#define FLASH_MOSI_Pin          GPIO_PIN_5
#define FLASH_MOSI_Pin_Port     GPIOB

#define DEBUG_RX_Pin            GPIO_PIN_11
#define DEBUG_RX_Pin_Port       GPIOC
#define DEBUG_TX_Pin            GPIO_PIN_10
#define DEBUG_TX_Pin_Port       GPIOC

#define RS232_RX_Pin            GPIO_PIN_10
#define RS232_RX_Pin_Port       GPIOA
#define RS232_TX_Pin            GPIO_PIN_9
#define RS232_TX_Pin_Port       GPIOA

#define RS485a_EN_Pin           GPIO_PIN_4
#define RS485a_EN_Pin_Port      GPIOD
#define RS485a_RX_Pin           GPIO_PIN_6
#define RS485a_RX_Pin_Port      GPIOD
#define RS485a_TX_Pin           GPIO_PIN_5
#define RS485a_TX_Pin_Port      GPIOD

#define RS485b_EN_Pin           GPIO_PIN_3
#define RS485b_EN_Pin_Port      GPIOD
#define RS485b_RX_Pin           GPIO_PIN_2
#define RS485b_RX_Pin_Port      GPIOD
#define RX485b_TX_Pin           GPIO_PIN_12
#define RX485b_TX_Pin_Port      GPIOD

#define CAN_RX_Pin              GPIO_PIN_11
#define CAN_RX_Pin_Port         GPIOA
#define CAN_TX_Pin              GPIO_PIN_11
#define CAN_TX_Pin_Port         GPIOA

#define ETH_RST_Pin             GPIO_PIN_14
#define ETH_RST_Pin_Port        GPIOB
#define ETH_EN_Pin              GPIO_PIN_15
#define ETH_EN_Pin_Port         GPIOB
#define ETH_OSCIN_Pin           GPIO_PIN_1
#define ETH_OSCIN_Pin_Port      GPIOA
#define ETH_MDIO_Pin            GPIO_PIN_2
#define ETH_MDIO_Pin_Port       GPIOA
#define ETH_MDC_Pin             GPIO_PIN_1
#define ETH_MDC_Pin_Port        GPIOC
#define ETH_CRS_DV_Pin          GPIO_PIN_8
#define ETH_CRS_DV_Pin_Port     GPIOD
#define ETH_RXDN_Pin            GPIO_PIN_9
#define ETH_RXDN_Pin_Port       GPIOD
#define ETH_RXDP_Pin            GPIO_PIN_10
#define ETH_RXDP_Pin_Port       GPIOD
#define ETH_TXEN_Pin            GPIO_PIN_11
#define ETH_TXEN_Pin_Port       GPIOB
#define ETH_TXDN_Pin            GPIO_PIN_12
#define ETH_TXDN_Pin_Port       GPIOB
#define ETH_TXDP_Pin            GPIO_PIN_13
#define ETH_TXDP_Pin_Port       GPIOB

#define MPWR_STAT_AC_Pin        GPIO_PIN_13
#define MPWR_STAT_AC_Pin_Port   GPIOE
#define MPWR_STAT_DC_Pin        GPIO_PIN_12
#define MPWR_STAT_DC_Pin_Port   GPIOE
#define MPWR_EN_Pin             GPIO_PIN_11
#define MPWR_EN_Pin_Port        GPIOE

#define APWR1_STAT_Pin          GPIO_PIN_0
#define APWR1_STAT_Pin_Port     GPIOD
#define APWR1_EN_Pin            GPIO_PIN_9
#define APWR1_EN_Pin_Port       GPIOC
#define APWR1_CTRL_DAC_Pin      GPIO_PIN_4
#define APWR1_CTRL_DAC_Pin_Port GPIOA
#define APWR1_CTRL_PWM_Pin      GPIO_PIN_8
#define APWR1_CTRL_PWM_Pin_Port GPIOB
#define APWR1_CUR_Pin           GPIO_PIN_2
#define APWR1_CUR_Pin_Port      GPIOC
#define APWR1_VOL_Pin           GPIO_PIN_3
#define APWR1_VOL_Pin_Port      GPIOC
#define APWR2_STAT_Pin          GPIO_PIN_1
#define APWR2_STAT_Pin_Port     GPIOD
#define APWR2_EN_Pin            GPIO_PIN_8
#define APWR2_EN_Pin_Port       GPIOA
#define APWR2_CTRL_DAC_Pin      GPIO_PIN_4
#define APWR2_CTRL_DAC_Pin_Port GPIOA
#define APWR2_CTRL_PWM_Pin      GPIO_PIN_8
#define APWR2_CTRL_PWM_Pin_Port GPIOB
#define APWR2_CUR_Pin           GPIO_PIN_3
#define APWR2_CUR_Pin_Port      GPIOA
#define APWR2_VOL_Pin           GPIO_PIN_6
#define APWR2_VOL_Pin_Port      GPIOA
#define APWR3_STAT_Pin          GPIO_PIN_6
#define APWR3_STAT_Pin_Port     GPIOB
#define APWR3_EN_Pin            GPIO_PIN_7
#define APWR3_EN_Pin_Port       GPIOB
#define APWR3_CTRL_DAC_Pin      GPIO_PIN_5
#define APWR3_CTRL_DAC_Pin_Port GPIOA
#define APWR3_CTRL_PWM_Pin      GPIO_PIN_8
#define APWR3_CTRL_PWM_Pin_Port GPIOB
#define APWR3_CUR_Pin           GPIO_PIN_5
#define APWR3_CUR_Pin_Port      GPIOC
#define APWR3_VOL_Pin           GPIO_PIN_0
#define APWR3_VOL_Pin_Port      GPIOB

#define QBH_ON_Pin              GPIO_PIN_9
#define QBH_ON_Pin_Port         GPIOB
#define EX_CTRL_EN_Pin          GPIO_PIN_0
#define EX_CTRL_EN_Pin_Port     GPIOE
#define WATER_PRESS_Pin         GPIO_PIN_7
#define WATER_PRESS_Pin_Port    GPIOC
#define WATER_CHILLER_Pin       GPIO_PIN_8
#define WATER_CHILLER_Pin_Port  GPIOC

#define LED_CUR_Pin             GPIO_PIN_0
#define LED_CUR_Pin_Port        GPIOC

#define I2C_SCL_Pin             GPIO_PIN_12
#define I2C_SCL_Pin_Port        GPIOD
#define I2C_SDA_Pin             GPIO_PIN_13
#define I2C_SDA_Pin_Port        GPIOD

#define FAULT_Pin               GPIO_PIN_15
#define FAULT_Pin_Port          GPIOB
#define EX_AD_EN_Pin            GPIO_PIN_11
#define EX_AD_EN_Pin_Port       GPIOD
#define MOD_EN_Pin              GPIO_PIN_14
#define MOD_EN_Pin_Port         GPIOD
#define AIM_EN_Pin              GPIO_PIN_15
#define AIM_EN_Pin_Port         GPIOD
#define LASER_EN_Pin            GPIO_PIN_6
#define LASER_EN_Pin_Port       GPIOC

#define BEEP_SW_Pin             GPIO_PIN_13
#define BEEP_SW_Pin_Port        GPIOC



/* Types */
typedef enum{
    MPWR_STAT_AC, MPWR_STAT_DC,
    APWR1_STAT, APWR2_STAT, APWR3_STAT, 
    QBH_ON, EX_CTRL_EN, WATER_PRESS, WATER_CHILLER, 
    AIM_EN, LASER_EN,
}GpioIn_t;

typedef enum{
    //LED_SYS, 
    LED1, LED2, LED3, LED4, LED5, LED6, 
    LED_CAN_OK, LED_CAN_FAIL, LED_AC_OK, LED_AC_FAIL, LED_DC_OK, LED_DC_FAIL,
    RS485a_EN,
    ETH_RST, ETH_EN, 
    MPWR_EN,
    APWR1_EN, APWR2_EN, APWR3_EN, 
    I2C_SCL, I2C_SDA,
    FAULT, EX_AD_EN, MOD_EN,
    BEEP_SW,
}GpioOut_t;

/* Functions */
GpioCtrl_t* GpioGetOutCtrl(void);
GpioCtrl_t* GpioGetInCtrl(void);

uint16_t GpioGetOutCtrlNum(void);
uint16_t GpioGetInCtrlNum(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BSPGPIO_H__ */
