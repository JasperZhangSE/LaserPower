/*
    Adc.c

    Implementation File for Adc Module
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 15Nov23, Karl Created
    01b, 23Nov23, Karl Added ADC_CHAN_8
*/

/* Includes */
#include "Include.h"

/* Debug config */
#if ADC_DEBUG
    #undef TRACE
    #define TRACE(...)  DebugPrintf(__VA_ARGS__)
#else
    #undef TRACE
    #define TRACE(...)
#endif /* ADC_DEBUG */
#if ADC_ASSERT
    #undef ASSERT
    #define ASSERT(a)   while(!(a)){DebugPrintf("ASSERT failed: %s %d\n", __FILE__, __LINE__);}
#else
    #undef ASSERT
    #define ASSERT(...)
#endif /* ADC_ASSERT */

/* Local variables */
static DMA_HandleTypeDef s_hDma;
static TIM_HandleTypeDef s_hTim;
static ADC_HandleTypeDef s_hAdc;
static uint16_t          s_usData[8];

/* Functions */
Status_t DrvAdcInit(void)
{
    /* DMA clock enable */
    __HAL_RCC_DMA1_CLK_ENABLE();
    /* DMA interrupt init */
#if 0
    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
#endif
    
    /* Common config */
    s_hAdc.Instance                   = ADC1;
    s_hAdc.Init.ScanConvMode          = ADC_SCAN_ENABLE;
    s_hAdc.Init.ContinuousConvMode    = DISABLE;
    s_hAdc.Init.DiscontinuousConvMode = DISABLE;
    s_hAdc.Init.ExternalTrigConv      = ADC_SOFTWARE_START;  /*ADC_EXTERNALTRIGCONV_T3_TRGO*/
    s_hAdc.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    s_hAdc.Init.NbrOfConversion       = 8;
    HAL_ADC_Init(&s_hAdc);
    
    /* Configure Regular Channel  */
    ADC_ChannelConfTypeDef xConfig;
    xConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
    xConfig.Channel      = ADC_CHANNEL_12;
    xConfig.Rank         = ADC_REGULAR_RANK_1;
    HAL_ADC_ConfigChannel(&s_hAdc, &xConfig);
    xConfig.Channel      = ADC_CHANNEL_13;
    xConfig.Rank         = ADC_REGULAR_RANK_2;
    HAL_ADC_ConfigChannel(&s_hAdc, &xConfig);
    xConfig.Channel      = ADC_CHANNEL_3;
    xConfig.Rank         = ADC_REGULAR_RANK_3;
    HAL_ADC_ConfigChannel(&s_hAdc, &xConfig);
    xConfig.Channel      = ADC_CHANNEL_6;
    xConfig.Rank         = ADC_REGULAR_RANK_4;
    HAL_ADC_ConfigChannel(&s_hAdc, &xConfig);
    xConfig.Channel      = ADC_CHANNEL_15;
    xConfig.Rank         = ADC_REGULAR_RANK_5;
    HAL_ADC_ConfigChannel(&s_hAdc, &xConfig);
    xConfig.Channel      = ADC_CHANNEL_8;
    xConfig.Rank         = ADC_REGULAR_RANK_6;
    HAL_ADC_ConfigChannel(&s_hAdc, &xConfig);
    xConfig.Channel      = ADC_CHANNEL_9;
    xConfig.Rank         = ADC_REGULAR_RANK_7;
    HAL_ADC_ConfigChannel(&s_hAdc, &xConfig);
    xConfig.Channel      = ADC_CHANNEL_10;
    xConfig.Rank         = ADC_REGULAR_RANK_8;
    HAL_ADC_ConfigChannel(&s_hAdc, &xConfig);
    
    /* Config timer */
    s_hTim.Instance               = TIM3;
    s_hTim.Init.Prescaler         = 7200 - 1;  /* 72MHz -> 10KHz */
    s_hTim.Init.CounterMode       = TIM_COUNTERMODE_UP;
    s_hTim.Init.Period            = (10 * ADC_SMP_PRD) - 1;
    s_hTim.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    s_hTim.Init.RepetitionCounter = 0;          /* only for TIM1 and TIM8 */
    HAL_TIM_Base_Init(&s_hTim);
    
    TIM_ClockConfigTypeDef xClkSrcConfig;
    xClkSrcConfig.ClockSource     = TIM_CLOCKSOURCE_INTERNAL;
    HAL_TIM_ConfigClockSource(&s_hTim, &xClkSrcConfig);
    
    TIM_MasterConfigTypeDef xMasterConfig;
    xMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    xMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&s_hTim, &xMasterConfig);
    
    /* ADC + TIMER + DMA start */
    HAL_ADCEx_Calibration_Start(&s_hAdc);
    HAL_TIM_Base_Start_IT(&s_hTim);
    HAL_ADC_Start_DMA(&s_hAdc, (uint32_t*)s_usData, (sizeof(s_usData) / sizeof(s_usData[0])));
    
    return STATUS_OK;
}

Status_t DrvAdcTerm(void)
{
    /* Do nothing */
    return STATUS_OK;
}

uint16_t AdcGet(AdcChan_t xChan)
{
    switch (xChan) {
    case ADC_CHAN_1:
        return s_usData[0];
    case ADC_CHAN_2:
        return s_usData[1];
    case ADC_CHAN_3:
        return s_usData[2];
    case ADC_CHAN_4:
        return s_usData[3];
    case ADC_CHAN_5:
        return s_usData[4];
    case ADC_CHAN_6:
        return s_usData[5];
    case ADC_CHAN_7:
        return s_usData[6];
    case ADC_CHAN_8:
        return s_usData[7];
    default:
        return 0xFFF;
    }
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* pxAdc)
{
    GPIO_InitTypeDef xConfig;
    if (pxAdc->Instance == ADC1) {
        /* Peripheral clock enable */
        __HAL_RCC_ADC1_CLK_ENABLE();
        /* Gpio configuration */
        xConfig.Pin  = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_5;
        xConfig.Mode = GPIO_MODE_ANALOG;
        HAL_GPIO_Init(GPIOC, &xConfig);
        xConfig.Pin  = GPIO_PIN_3|GPIO_PIN_6;
        xConfig.Mode = GPIO_MODE_ANALOG;
        HAL_GPIO_Init(GPIOA, &xConfig);
        xConfig.Pin  = GPIO_PIN_0|GPIO_PIN_1;
        xConfig.Mode = GPIO_MODE_ANALOG;
        HAL_GPIO_Init(GPIOB, &xConfig);
        /* Dma init */
        s_hDma.Instance                 = DMA1_Channel1;
        s_hDma.Init.Direction           = DMA_PERIPH_TO_MEMORY;
        s_hDma.Init.PeriphInc           = DMA_PINC_DISABLE;
        s_hDma.Init.MemInc              = DMA_MINC_ENABLE;
        s_hDma.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
        s_hDma.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
        s_hDma.Init.Mode                = DMA_NORMAL;
        s_hDma.Init.Priority            = DMA_PRIORITY_LOW;
        HAL_DMA_Init(&s_hDma);
        __HAL_LINKDMA(pxAdc, DMA_Handle, s_hDma);
    }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* pxAdc)
{
    if (pxAdc->Instance==ADC1) {
        /* Peripheral clock disable */
        __HAL_RCC_ADC1_CLK_DISABLE();
        /* Gpio configuration */
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_5);
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_3|GPIO_PIN_6);
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0|GPIO_PIN_1);
        /* Dma deinit */
        HAL_DMA_DeInit(pxAdc->DMA_Handle);
    }
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* pxTim)
{
    if(pxTim->Instance==TIM3) {
        /* Peripheral clock enable */
        __HAL_RCC_TIM3_CLK_ENABLE();
        /* Interrupt init */
        HAL_NVIC_SetPriority(TIM3_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(TIM3_IRQn);
    }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* pxTim)
{
    if (pxTim->Instance==TIM3) {
        /* Peripheral clock disable */
        __HAL_RCC_TIM3_CLK_DISABLE();
        /* Interrupt deinit */
        HAL_NVIC_DisableIRQ(TIM3_IRQn);
    }
}

void DMA1_Channel1_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&s_hDma);
}

void TIM3_IRQHandler(void)
{
#if 1
    HAL_ADC_Stop_DMA(&s_hAdc);
#else
    /* Enalbe DMA interrupt */
#endif
    HAL_TIM_IRQHandler(&s_hTim);
    HAL_ADC_Start_DMA(&s_hAdc, (uint32_t*)s_usData, (sizeof(s_usData) / sizeof(s_usData[0])));
    HAL_ADC_Start(&s_hAdc);
}

static void prvCliCmdAdcStatus(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    cliprintf("ADC status:\n");
    cliprintf("    ADC1: %4d, %4d mV [AUX1_CS]\n", s_usData[0], ADC_TO_MVOL(s_usData[0]));
    cliprintf("    ADC2: %4d, %4d mV [AUX1_VS]\n", s_usData[1], ADC_TO_MVOL(s_usData[1]));
    cliprintf("    ADC3: %4d, %4d mV [AUX2_CS]\n", s_usData[2], ADC_TO_MVOL(s_usData[2]));
    cliprintf("    ADC4: %4d, %4d mV [AUX2_VS]\n", s_usData[3], ADC_TO_MVOL(s_usData[3]));
    cliprintf("    ADC5: %4d, %4d mV [AUX3_CS]\n", s_usData[4], ADC_TO_MVOL(s_usData[4]));
    cliprintf("    ADC6: %4d, %4d mV [AUX3_VS]\n", s_usData[5], ADC_TO_MVOL(s_usData[5]));
    //cliprintf("    ADC7: %4d, %4d mV [PD_VS  ]\n", s_usData[6], ADC_TO_MVOL(s_usData[6]));
    cliprintf("    ADC7: RESERVED\n");
    cliprintf("    ADC8: %4d, %4d mV [LED_CUR]\n", s_usData[7], ADC_TO_MVOL(s_usData[7]));
}
CLI_CMD_EXPORT(adc_status, show adc status, prvCliCmdAdcStatus)
