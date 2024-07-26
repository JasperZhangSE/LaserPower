/*
    Dac.c

    Implementation File for Dac Module
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 15Nov23, Karl Created
    01b, 22Nov23, Karl Added DacGet function
    01c, 22Nov23, Karl Added MVOL_TO_DAC and DAC_TO_MVOL
*/

/* Includes */
#include "Include.h"

/* Debug config */
#if DAC_DEBUG
#undef TRACE
#define TRACE(...) DebugPrintf(__VA_ARGS__)
#else
#undef TRACE
#define TRACE(...)
#endif /* DAC_DEBUG */
#if DAC_ASSERT
#undef ASSERT
#define ASSERT(a)                                                                                                      \
    while (!(a)) {                                                                                                     \
        DebugPrintf("ASSERT failed: %s %d\n", __FILE__, __LINE__);                                                     \
    }
#else
#undef ASSERT
#define ASSERT(...)
#endif /* DAC_ASSERT */

/* Local variables */
static DAC_HandleTypeDef s_hDac;

/* Functions */
Status_t DrvDacInit(void) {
    DAC_ChannelConfTypeDef xConfig;

    /* DAC init */
    s_hDac.Instance = DAC;
    if (HAL_DAC_Init(&s_hDac) != HAL_OK) {
        ASSERT(0);
    }

    /* DAC channel 1 */
    /* Config */
    xConfig.DAC_Trigger      = DAC_TRIGGER_NONE;
    xConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
    HAL_DAC_ConfigChannel(&s_hDac, &xConfig, DAC_CHANNEL_1);
    /* Reset */
    HAL_DAC_SetValue(&s_hDac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 0);
    /* Start */
    HAL_DAC_Start(&s_hDac, DAC_CHANNEL_1);

#if DAC2_ENABLE
    /* DAC channel 2 */
    /* Config  */
    HAL_DAC_ConfigChannel(&s_hDac, &xConfig, DAC_CHANNEL_2);
    /* Reset */
    HAL_DAC_SetValue(&s_hDac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, 0);
    /* Start */
    HAL_DAC_Start(&s_hDac, DAC_CHANNEL_2);
#endif
    return STATUS_OK;
}

Status_t DrvDacTerm(void) {
    /* Do nothing */
    return STATUS_OK;
}

Status_t DacSet(DacChan_t xChan, uint16_t usData) {
#if DAC2_ENABLE
    switch (xChan) {
    case DAC_CHAN_1:
    case DAC_CHAN_2:
        HAL_DAC_SetValue(&s_hDac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, usData);
        break;
    case DAC_CHAN_3:
        HAL_DAC_SetValue(&s_hDac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, usData);
        break;
    }
#else
    HAL_DAC_SetValue(&s_hDac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, usData);
#endif
    return STATUS_OK;
}

uint16_t DacGet(DacChan_t xChan) {
#if DAC2_ENABLE
    switch (xChan) {
    case DAC_CHAN_1:
    case DAC_CHAN_2:
        return HAL_DAC_GetValue(&s_hDac, DAC_CHANNEL_1);
    case DAC_CHAN_3:
        return HAL_DAC_GetValue(&s_hDac, DAC_CHANNEL_2);
    default:
        return 0;
    }
#else
    return HAL_DAC_GetValue(&s_hDac, DAC_CHANNEL_1);
#endif
}

void HAL_DAC_MspInit(DAC_HandleTypeDef *pxDac) {
    GPIO_InitTypeDef GPIO_InitStruct;
    if (pxDac->Instance == DAC) {
        /* Peripheral clock enable */
        __HAL_RCC_DAC_CLK_ENABLE();
#if DAC2_ENABLE
        GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5;
#else
        GPIO_InitStruct.Pin = GPIO_PIN_4;
#endif
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

void HAL_DAC_MspDeInit(DAC_HandleTypeDef *pxDac) {
    if (pxDac->Instance == DAC) {
        /* Peripheral clock disable */
        __HAL_RCC_DAC_CLK_DISABLE();
#if DAC2_ENABLE
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4 | GPIO_PIN_5);
#else
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4);
#endif
    }
}

uint32_t DacGetValue(void)
{
    return HAL_DAC_GetValue(&s_hDac, DAC_CHANNEL_1);
}

static void prvCliCmdDacStatus(cli_printf cliprintf, int argc, char **argv) {
    CHECK_CLI();

    uint32_t d1 = HAL_DAC_GetValue(&s_hDac, DAC_CHANNEL_1);
#if DAC2_ENABLE
    uint32_t d2 = HAL_DAC_GetValue(&s_hDac, DAC_CHANNEL_2);
#endif
    cliprintf("DAC status\n");
    cliprintf("    DAC1: %4d, %4d mV\n", d1, DAC_TO_MVOL(d1));
    cliprintf("    DAC2: %4d, %4d mV\n", d1, DAC_TO_MVOL(d1));
#if DAC2_ENABLE
    cliprintf("    DAC3: %4d, %4d mV\n", d2, DAC_TO_MVOL(d2));
#else
    cliprintf("    DAC3: %4d, %4d mV\n", d1, DAC_TO_MVOL(d1));
#endif
}
CLI_CMD_EXPORT(dac_status, show dac output status, prvCliCmdDacStatus)

static void prvCliCmdDacCtrl(cli_printf cliprintf, int argc, char **argv) {
    CHECK_CLI();

    int c = 0, d = 0;

    if (argc != 3) {
        cliprintf("dac_ctrl CHAN MVOL\n");
        return;
    }

    c = atoi(argv[1]);
    d = atoi(argv[2]);

    if ((c < DAC_CHAN_1) || (c > DAC_CHAN_3)) {
        cliprintf("Channel range 1 ~ 3\n");
        return;
    }

    if (d > 4095) {
        d = 4095;
    }
#if DAC2_ENABLE
    cliprintf("DAC ctrl\n");
    switch (c) {
    case DAC_CHAN_1:
    case DAC_CHAN_2:
        cliprintf("    DAC1: %4d, %4d mV\n", MVOL_TO_DAC(d), d);
        cliprintf("    DAC2: %4d, %4d mV\n", MVOL_TO_DAC(d), d);
        HAL_DAC_SetValue(&s_hDac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, MVOL_TO_DAC(d));
        break;
    case DAC_CHAN_3:
        cliprintf("    DAC3: %4d, %4d mV\n", MVOL_TO_DAC(d), d);
        HAL_DAC_SetValue(&s_hDac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, MVOL_TO_DAC(d));
        break;
    default:
        break;
    }
#else
    cliprintf("DAC ctrl\n");
    cliprintf("    DAC1: %4d, %4d mV\n", MVOL_TO_DAC(d), d);
    cliprintf("    DAC2: %4d, %4d mV\n", MVOL_TO_DAC(d), d);
    cliprintf("    DAC3: %4d, %4d mV\n", MVOL_TO_DAC(d), d);
    HAL_DAC_SetValue(&s_hDac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, MVOL_TO_DAC(d));

#endif
}
CLI_CMD_EXPORT(dac_ctrl, ctrl dac output, prvCliCmdDacCtrl)
