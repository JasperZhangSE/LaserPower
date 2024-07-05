/*
    Main.c

    Implementation File for Main Module
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 13Nov23, Karl Created
*/

/* PID : PD24D06-B */

/* Includes */
#include "Include.h"

/* Forward declaration */
void SystemClock_Config(void);

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    /* TODO: Delete me later! */
    return;
}

int main(void) {
#if RUN_WITH_BOOTLOADER
#warning "Please config IROM1: Start 0x8010000, Size 0x30000 in Options->Target"
    SCB->VTOR = 0x8000000 | ((0x10000) & (uint32_t)0x1FFFFF80);
    __enable_irq();
#else
#warning "Please config IROM1: Start 0x8000000, Size 0x40000 in Options->Target"
#endif /* RUN_WITH_BOOTLOADER */

    /* MCU Configuration */
    /* Reset of all peripherals, Initializes the Flash interface and the Systick
     */
    HAL_Init();
    /* Configure the system clock */
    SystemClock_Config();

    /* User application init */
    DebugInit();
    DebugUartInit();
    DebugUartConfig(UART4, 115200);
    DebugChanSet(DEBUG_CHAN_UART);
    DrvGpioInit();
    DrvAdcInit();
    DrvCanInit();
    DrvDacInit();
    DrvMemInit();
    // DrvNetInit();
    DrvStcInit();
    DrvTimeInit();
    DrvPwmInit();
    I2cInit();
    Aht30Init();
    AppCliInit();
    AppDataInit();

    AppComInit();
    AppSysInit();

    /* Start scheduler */
    osKernelStart();

    /* We should never get here as control is now taken by the scheduler */
    /* Infinite loop */
    while (1) {
    }
}
