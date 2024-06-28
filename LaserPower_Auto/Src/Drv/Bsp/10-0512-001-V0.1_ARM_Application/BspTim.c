/*
    BspTim.c

    Implementation File for STM32 TIM Module
*/

/* Copyright 2019 Shanghai Master Inc. */

/*
    Modification History
    --------------------
    01a 19Aug19 Ziv Created
    01b 12Mar20 Karl Fixed incorrect __HAL_AFIO_REMAP_TIMx_ENABLE
*/

/* Includes */
#include "BspTim.h"

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* pxTim)
{
    if (TIM1 == pxTim->Instance) {
        __HAL_RCC_TIM1_CLK_ENABLE();
    }
    else if (TIM2 == pxTim->Instance) {
        __HAL_RCC_TIM2_CLK_ENABLE();
    }
    else if (TIM3 == pxTim->Instance) {
        __HAL_RCC_TIM3_CLK_ENABLE();
    }
    else if (TIM4 == pxTim->Instance) {
        __HAL_RCC_TIM4_CLK_ENABLE();
        
        HAL_NVIC_SetPriority(TIM4_IRQn, 8, 0);
        HAL_NVIC_EnableIRQ(TIM4_IRQn);
    }
    
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* pxTim)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    if (TIM1 == pxTim->Instance) {
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    #ifdef TIM1_CH1_PWM_ENABLE_PE9
        GPIO_InitStruct.Pin = GPIO_PIN_9;
        HAL_GPIO_Init(GPIOE,&GPIO_InitStruct);
    #endif
    #ifdef TIM1_CH2_PWM_ENABLE_PE11
        GPIO_InitStruct.Pin = GPIO_PIN_11;
        HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    #endif
    #ifdef TIM1_CH3_PWM_ENABLE_PE13
        GPIO_InitStruct.Pin = GPIO_PIN_13;
        HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    #endif
    #ifdef TIM1_CH4_PWM_ENABLE_PE14
        GPIO_InitStruct.Pin = GPIO_PIN_14;
        HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    #endif
    #ifdef TIM1_CH1_PWM_ENABLE_PA8
        GPIO_InitStruct.Pin = GPIO_PIN_8;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    #endif
    #ifdef TIM1_CH2_PWM_ENABLE_PA9
        GPIO_InitStruct.Pin = GPIO_PIN_9;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    #endif
    #ifdef TIM1_CH3_PWM_ENABLE_PA10
        GPIO_InitStruct.Pin = GPIO_PIN_10;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    #endif
    #ifdef TIM1_CH4_PWM_ENABLE_PA11
        GPIO_InitStruct.Pin = GPIO_PIN_11;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    #endif
    #ifdef TIM1_AFIO_REMAP
        TIM1_AFIO_REMAP();
    #endif
    }
    else if (TIM2 == pxTim->Instance) {  
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    #ifdef TIM2_CH1_PWM_ENABLE_PA15
        GPIO_InitStruct.Pin = GPIO_PIN_15;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    #endif
    #ifdef TIM2_CH2_PWM_ENABLE_PB3
        GPIO_InitStruct.Pin = GPIO_PIN_3;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    #endif
    #ifdef TIM2_CH3_PWM_ENABLE_PB10
        GPIO_InitStruct.Pin = GPIO_PIN_10;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    #endif
    #ifdef TIM2_CH4_PWM_ENABLE_PB11
        GPIO_InitStruct.Pin = GPIO_PIN_11;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    #endif
    #ifdef TIM2_CH1_PWM_ENABLE_PA0
        GPIO_InitStruct.Pin = GPIO_PIN_0;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    #endif
    #ifdef TIM2_CH2_PWM_ENABLE_PA1
        GPIO_InitStruct.Pin = GPIO_PIN_1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    #endif
    #ifdef TIM2_CH3_PWM_ENABLE_PA2
        GPIO_InitStruct.Pin = GPIO_PIN_2;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    #endif
    #ifdef TIM2_CH4_PWM_ENABLE_PA3
        GPIO_InitStruct.Pin = GPIO_PIN_3;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    #endif
    #ifdef TIM2_AFIO_REMAP
        TIM2_AFIO_REMAP();
    #endif
    }
    else if (TIM3 == pxTim->Instance) { 
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    #ifdef TIM3_CH1_PWM_ENABLE_PC6
        GPIO_InitStruct.Pin = GPIO_PIN_6;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    #endif
    #ifdef TIM3_CH2_PWM_ENABLE_PC7
        GPIO_InitStruct.Pin = GPIO_PIN_7;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    #endif
    #ifdef TIM3_CH3_PWM_ENABLE_PC8
        GPIO_InitStruct.Pin = GPIO_PIN_8;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    #endif
    #ifdef TIM3_CH4_PWM_ENABLE_PC9
        GPIO_InitStruct.Pin = GPIO_PIN_9;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    #endif
    #ifdef TIM3_CH1_PWM_ENABLE_PB4
        GPIO_InitStruct.Pin = GPIO_PIN_4;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    #endif
    #ifdef TIM3_CH2_PWM_ENABLE_PB5
        GPIO_InitStruct.Pin = GPIO_PIN_5;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    #endif
    #ifdef TIM3_CH3_PWM_ENABLE_PB0
        GPIO_InitStruct.Pin = GPIO_PIN_0;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    #endif
    #ifdef TIM3_CH4_PWM_ENABLE_PB1
        GPIO_InitStruct.Pin = GPIO_PIN_1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    #endif
    #ifdef TIM3_CH1_PWM_ENABLE_PA6
        GPIO_InitStruct.Pin = GPIO_PIN_6;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    #endif
    #ifdef TIM3_CH2_PWM_ENABLE_PA7
        GPIO_InitStruct.Pin = GPIO_PIN_7;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    #endif
    #ifdef TIM3_AFIO_REMAP
        TIM3_AFIO_REMAP();
    #endif
    }
    else if (TIM4 == pxTim->Instance) { 
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    #ifdef TIM4_CH1_PWM_ENABLE_PD12
        GPIO_InitStruct.Pin = GPIO_PIN_12;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    #endif
    #ifdef TIM4_CH2_PWM_ENABLE_PD13
        GPIO_InitStruct.Pin = GPIO_PIN_13;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    #endif
    #ifdef TIM4_CH3_PWM_ENABLE_PD14
        GPIO_InitStruct.Pin = GPIO_PIN_14;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    #endif
    #ifdef TIM4_CH4_PWM_ENABLE_PD15
        GPIO_InitStruct.Pin = GPIO_PIN_15;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    #endif
    #ifdef TIM4_CH1_PWM_ENABLE_PB6
        GPIO_InitStruct.Pin = GPIO_PIN_6;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    #endif
    #ifdef TIM4_CH2_PWM_ENABLE_PB7
        GPIO_InitStruct.Pin = GPIO_PIN_7;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    #endif
    #ifdef TIM4_CH3_PWM_ENABLE_PB8
        GPIO_InitStruct.Pin = GPIO_PIN_8;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    #endif
    #ifdef TIM4_CH4_PWM_ENABLE_PB9
        GPIO_InitStruct.Pin = GPIO_PIN_9;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    #endif
    #ifdef TIM4_AFIO_REMAP
        TIM4_AFIO_REMAP();
    #endif
    }
}


void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef* pxTim)
{
    if (TIM1 == pxTim->Instance) {
        __HAL_RCC_TIM1_CLK_DISABLE();
    }
    else if (TIM2 == pxTim->Instance) {
        __HAL_RCC_TIM2_CLK_DISABLE();
    }
    else if (TIM3 == pxTim->Instance) {
        __HAL_RCC_TIM3_CLK_DISABLE();
    }
    else if (TIM4 == pxTim->Instance) {
        __HAL_RCC_TIM4_CLK_DISABLE();
        HAL_NVIC_DisableIRQ(TIM4_IRQn);
    }
} 
