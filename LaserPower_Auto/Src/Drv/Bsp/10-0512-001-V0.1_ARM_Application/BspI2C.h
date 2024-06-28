/*
    BspI2C.c

    Implementation File for STM32 I2C Module
*/

/* Copyright 2024 KunShan AXLON Inc. */

/*
    Modification History
    --------------------
    01a 27Feb24 Jasper Created
*/


#ifndef __BSP_I2C_H__
#define __BSP_I2C_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stm32f1xx_hal.h>

#define I2C_DELAY   1

#define I2C_GPIO_PORT    GPIOD
#define I2C_SCL_PIN      GPIO_PIN_12
#define I2C_SDA_PIN      GPIO_PIN_13

// 定义I2C的时钟和数据线控制宏
#define I2C_SCL_HIGH  HAL_GPIO_WritePin(I2C_GPIO_PORT, I2C_SCL_PIN, GPIO_PIN_SET)
#define I2C_SCL_LOW   HAL_GPIO_WritePin(I2C_GPIO_PORT, I2C_SCL_PIN, GPIO_PIN_RESET)

#define I2C_SDA_HIGH  HAL_GPIO_WritePin(I2C_GPIO_PORT, I2C_SDA_PIN, GPIO_PIN_SET)
#define I2C_SDA_LOW   HAL_GPIO_WritePin(I2C_GPIO_PORT, I2C_SDA_PIN, GPIO_PIN_RESET)
#define I2C_SDA_READ  HAL_GPIO_ReadPin(I2C_GPIO_PORT, I2C_SDA_PIN)

#if 0

#define HIGH (uint32_t)1
#define LOW  (uint32_t)0

#define SCL_OUTPUT(x)  if(x == 0) HAL_GPIO_WritePin(I2C_GPIO_PORT,I2C_SCL_PIN,GPIO_PIN_RESET); else HAL_GPIO_WritePin(I2C_GPIO_PORT,I2C_SCL_PIN,GPIO_PIN_SET);
#define SDA_OUTPUT(x)  if(x == 0) HAL_GPIO_WritePin(I2C_GPIO_PORT,I2C_SDA_PIN,GPIO_PIN_RESET); else HAL_GPIO_WritePin(I2C_GPIO_PORT,I2C_SDA_PIN,GPIO_PIN_SET);
#define SDA_READ()     HAL_GPIO_ReadPin(I2C_GPIO_PORT,I2C_SDA_PIN)

#endif


void I2C_Init(void);
// 写入数据到设备
// 写入数据到设备
uint8_t I2C_WriteData(uint8_t address, uint8_t reg, uint8_t* data, uint16_t size);
// 从设备读取数据
uint8_t I2C_ReadData(uint8_t address, uint8_t reg, uint8_t* data, uint16_t size);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BSP_SPI_H__ */
