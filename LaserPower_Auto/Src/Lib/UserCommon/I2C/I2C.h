/**
 * \file            lib_i2c.h
 * \brief           Software i2c library
 */

/*
 * Copyright (c) 2024 Jasper
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file contrains all the software i2c functions.
 *
 * Author:          Jasper <JasperZhangSE@gmail.com>
 * Version:         v1.0.0-dev
 */

#ifndef __LIB_I2C_H__
#define __LIB_I2C_H__

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Includes */
#include "Include/Include.h"
#include "Config.h"
#include "BspGpio.h"

#define SORTWARE_I2C_ENABLE     1
#define SW_I2C_RTOS             1

#if SORTWARE_I2C_ENABLE

/* Defines */
#define I2C_SCL_PORT    I2C_SCL_Pin_Port
#define I2C_SCL_PIN     I2C_SCL_Pin
#define I2C_SDA_PORT    I2C_SDA_Pin_Port
#define I2C_SDA_PIN     I2C_SDA_Pin

#define I2C_SCL(state)  HAL_GPIO_WritePin(I2C_SCL_PORT, I2C_SCL_PIN, (state ? GPIO_PIN_SET : GPIO_PIN_RESET))
#define I2C_SDA(state)  HAL_GPIO_WritePin(I2C_SDA_PORT, I2C_SDA_PIN, (state ? GPIO_PIN_SET : GPIO_PIN_RESET))
#define I2C_SDA_READ()  HAL_GPIO_ReadPin(I2C_SDA_PORT, I2C_SDA_PIN)

#define USE_SYS_TICK_DELAY_US 0

	typedef enum
	{
		high = 1,
		low = 0
	} i2c_line_Status_t;

    Status_t I2cInit(void);
	Status_t i2c_start(void);
	Status_t i2c_stop(void);
	Status_t i2c_write_byte(uint8_t data);
	Status_t i2c_read_byte(uint8_t *data, uint8_t ack);
	Status_t i2c_write_data(uint8_t device_address, uint8_t *data, uint16_t length);
	Status_t i2c_read_data(uint8_t device_address, uint8_t *data, uint16_t length, uint8_t ack);
	Status_t i2c_write_register(uint8_t device_address, uint8_t register_address, uint8_t *data, uint8_t length);
	Status_t i2c_read_register(uint8_t device_address, uint8_t register_address, uint8_t *data, uint8_t length);

#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LIB_I2C_H__ */
