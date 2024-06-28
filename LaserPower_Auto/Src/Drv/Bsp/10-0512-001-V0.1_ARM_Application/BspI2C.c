/*
    BspI2C.c

    Implementation File for STM32 I2C Module
*/

/* Copyright 2024 JangSu AXLON Inc. */

/*
    Modification History
    --------------------
    01a 27Feb24 Jasper Created
*/

/* Includes */

#include "include.h"

/* Debug config */
#if I2C_DEBUG
    #undef TRACE
    #define TRACE(...)  DebugPrintf(__VA_ARGS__)
#else
    #undef TRACE
    #define TRACE(...)
#endif /* TIME_DEBUG */
#if I2C_ASSERT
    #undef ASSERT
    #define ASSERT(a)   while(!(a)){DebugPrintf("ASSERT failed: %s %d\n", __FILE__, __LINE__);}
#else
    #undef ASSERT
    #define ASSERT(...)
#endif /* TIME_ASSERT */
    

/* Define the start and stop conditions of I2C */
void I2C_Start(void) {
    /*    __________
     *SCL           \________
     *    ______
     *SDA       \_____________
     */
    I2C_SDA_HIGH;
    I2C_SCL_HIGH;
    osDelay(I2C_DELAY);
    I2C_SDA_LOW;
    osDelay(I2C_DELAY);
    I2C_SCL_LOW;
    osDelay(I2C_DELAY);
}

void I2C_Stop(void) {
    /*          ____________
     *SCL _____/
     *               _______
     *SDA __________/
     */
    I2C_SCL_LOW;
    I2C_SDA_LOW;
    osDelay(I2C_DELAY);
    I2C_SCL_HIGH;
    osDelay(I2C_DELAY);
    I2C_SDA_HIGH;
    osDelay(I2C_DELAY);
}

/* Define data write and read for I2C */
uint8_t I2C_WriteByte(uint8_t byte) {
    uint8_t i;
    for (i = 0; i < 8; i++) {
        if (byte & 0x80) {
            I2C_SDA_HIGH;
        } else {
            I2C_SDA_LOW;
        }
        osDelay(I2C_DELAY); 
        I2C_SCL_HIGH;
        osDelay(I2C_DELAY); 
        I2C_SCL_LOW;
        byte <<= 1;
    }
    /* Read the acknowledge bit */
    I2C_SDA_HIGH;
    osDelay(I2C_DELAY); 
    I2C_SCL_HIGH;
    osDelay(I2C_DELAY); 
    uint8_t usAck = I2C_SDA_READ; /* Read the acknowledge bit, 0 indicates successful data reception by the device */
    I2C_SCL_LOW;
    return usAck;
}

uint8_t I2C_ReadByte(uint8_t ack) {
    uint8_t i, byte = 0;
    for (i = 0; i < 8; i++) {
        byte <<= 1;
        I2C_SCL_HIGH;
        osDelay(I2C_DELAY); 
        if (I2C_SDA_READ) {
            byte |= 0x01;
        }
        I2C_SCL_LOW;
        osDelay(I2C_DELAY); 
    }
    /* Send the acknowledge bit */
    if (ack) {
        I2C_SDA_LOW;
    } else {
        I2C_SDA_HIGH;
    }
    osDelay(I2C_DELAY); 
    I2C_SCL_HIGH;
    osDelay(I2C_DELAY); 
    I2C_SCL_LOW;
    return byte;
}

/* Initialize the I2C bus */
void I2C_Init(void) {
    /* Initialize GPIO pins */
    GPIO_InitTypeDef GPIO_InitStruct;
    __HAL_RCC_GPIOD_CLK_ENABLE();
    GPIO_InitStruct.Pin = I2C_SCL_PIN | I2C_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD; /* Open-drain output mode */
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(I2C_GPIO_PORT, &GPIO_InitStruct);

    /* Initialize the I2C bus */
    I2C_Stop(); /* Send stop condition */
}

uint8_t I2C_WriteData(uint8_t address, uint8_t reg, uint8_t* data, uint16_t size) {
    I2C_Start(); /* Send start condition */
    
    uint8_t usAck = I2C_WriteByte(address << 1); /* Send device address and write flag */
    if (usAck) {
        I2C_Stop(); /* Send stop condition */
        return usAck; /* Return error */
    }
    
    /* Send register address */
    usAck = I2C_WriteByte(reg);
    if (usAck) {
        I2C_Stop(); /* Send stop condition */
        return usAck; /* Return error */
    }
    
    for (uint16_t i = 0; i < size; i++) { /* Loop to send data */
        usAck = I2C_WriteByte(data[i]); /* Send data */

        if (usAck) {
            I2C_Stop(); /* Send stop condition */
            return usAck; /* Return error */
        }
    }
    
    I2C_Stop(); /* Send stop condition */
    return 0; /* Return no error */
}

uint8_t I2C_ReadData(uint8_t address, uint8_t reg, uint8_t* data, uint16_t size) {
    I2C_Start(); /* Send start condition */
    
    uint8_t usAck = I2C_WriteByte(address << 1); /* Send device address and write flag */
    if (usAck) {
        I2C_Stop(); /* Send stop condition */
        return usAck; /* Return error */
    }
    
    /* Send register address */
    usAck = I2C_WriteByte(reg);
    if (usAck) {
        I2C_Stop(); /* Send stop condition */
        return usAck; /* Return error */
    }
    
    I2C_Start(); /* Send start condition again */
    /* Send device address and read flag again */
    usAck = I2C_WriteByte((address << 1) | 0x01);
    if (usAck) {
        I2C_Stop(); /* Send stop condition */
        return usAck; /* Return error */
    }
    
    for (uint16_t i = 0; i < size; i++) { /* Loop to read data */
        data[i] = I2C_ReadByte(i < size - 1); /* Read data */
        
        /* Check if it's the last byte, if so, exit the loop */
        if (i == size - 1) {
            break;
        }
    }
    
    I2C_Stop(); /* Send stop condition */
    return 0; /* Return no error */
}


