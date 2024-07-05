/*
    Boot.c
    
    Implementation File for Application Boot Module
*/

/* Copyright 2022 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 08Apr22, Karl Created
*/

/* Includes */
#include "Boot.h"

/* Debug config */
#if APP_BOOT_DEBUG
#undef TRACE
#define TRACE(...) DebugPrintf(__VA_ARGS__)
#else
#undef TRACE
#define TRACE(...)
#endif /* APP_BOOT_DEBUG */
#if APP_BOOT_ASSERT
#undef ASSERT
#define ASSERT(a)                                                                                                      \
    while (!(a)) {                                                                                                     \
        DebugPrintf("ASSERT failed: %s %d\n", __FILE__, __LINE__);                                                     \
    }
#else
#undef ASSERT
#define ASSERT(...)
#endif /* APP_BOOT_ASSERT */

/* Defines */
/*            Uart bootloader
 * Please define these macros in Config.h
 * #define BOOT_UART_FLASH_PAGE_SIZE   (0x800)
 * #define BOOT_UART_APP_ADDR          (0x08010000)
 * #define BOOT_UART_APP_SIZE          (0x30000)
*/

#define NVIC_VECTTAB_RAM         ((uint32_t)0x20000000) /* RAM base address */
#define NVIC_VECTTAB_FLASH       ((uint32_t)0x08000000) /* Flash base address */
#define NVIC_VECTTAB_OFFSET_MASK ((uint32_t)0x1FFFFF80) /* Set the NVIC vector table offset mask */

/* Forward declarations */
#if APP_BOOT_ENABLE_WDOG
void prvStartTimer(void);
#endif /* APP_BOOT_ENABLE_WDOG */
#if BOOT_UART_ENABLE
static void prvBootUartDone(void);
#endif /* BOOT_UART_ENABLE */
static void       prvLoadApplication(uint32_t ulAddr);
static void       prvDisableInterrupt(void);
static void       prvClockDeInit(void);
static void       prvNvicVectorTableSet(uint32_t ulNvicVectorTab, uint32_t ulOffset);
static __asm void prvJumpToAddress(uint32_t ulAddr);

/* Global variables */
extern TIM_HandleTypeDef htim7;

/* Local variables */
#if APP_BOOT_ENABLE_WDOG
static TIM_HandleTypeDef s_hTimer;
#endif /* APP_BOOT_ENABLE_WDOG */
static MemHandle_t s_xMem;

/* Functions */
Status_t BootInit(void) {
    /* Do nothing */
    return STATUS_OK;
}

Status_t BootTerm(void) {
    /* Do nothing */
    return STATUS_OK;
}

Status_t BootRun(void) {
#if APP_BOOT_ENABLE_WDOG
    /* Start watch dog */
    WdogStart(2500 /*ms*/);
    /* Start 500ms timer */
    prvStartTimer();
#endif /* APP_BOOT_ENABLE_WDOG */

    /* Fram init */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    MemInit(MEM_DEVICE_SPIFLASH);
    s_xMem = MemCreate(MEM_DEVICE_SPIFLASH);
    MemConfig_t xMemCfg;
    MemConfigInit(&xMemCfg);
    MemConfigSpi(&xMemCfg, SPI3);
    MemConfigCsPin(&xMemCfg, GPIOD, GPIO_PIN_7);
    MemConfigCapacity(&xMemCfg, 0, 8 * 1024 * 1024, 8 * 1024 * 1024);
    MemConfig(s_xMem, &xMemCfg);

    /* Check bootmark */
    uint32_t ulBootMark = 0;
    MemRead(s_xMem, 0, sizeof(ulBootMark), (uint8_t *)&ulBootMark);
    if (0x1234ABCD != ulBootMark) {
#if APP_BOOT_ENABLE_WDOG
        WdogFeed();
        WdogStart(10000 /*ms*/);
#endif /* APP_BOOT_ENABLE_WDOG */
        /* XXX: Disable SPI to save power */
        prvLoadApplication(BOOT_UART_APP_ADDR);
    }

    /* Config bootloader */
#if BOOT_UART_ENABLE
    BootUartConfig_t xConfig;
    xConfig.pxRxPinPort  = GPIOA;
    xConfig.ulRxPin      = GPIO_PIN_10;
    xConfig.pxTxPinPort  = GPIOA;
    xConfig.ulTxPin      = GPIO_PIN_9;
    xConfig.pxInstance   = USART1;
    xConfig.ulWordLength = UART_WORDLENGTH_8B;
    xConfig.ulStopBits   = UART_STOPBITS_1;
    xConfig.ulParity     = UART_PARITY_NONE;
    xConfig.ulBaudRate   = 115200;
    xConfig.pxDoneFunc   = prvBootUartDone;
    BootUartConfig(&xConfig);
    TickInit();
#endif /* BOOT_UART_ENABLE */

#if APP_BOOT_ENABLE_WDOG
    WdogFeed();
    WdogStart(2500 /*ms*/);
#endif /* APP_BOOT_ENABLE_WDOG */

    /* Run bootloader */
#if BOOT_UART_ENABLE
    return BootUartRun();
#endif /* BOOT_UART_ENABLE */
}

#if APP_BOOT_ENABLE_WDOG
void prvStartTimer(void) {
    /*Configure the TIM5 IRQ priority */
    HAL_NVIC_SetPriority(TIM5_IRQn, (1UL << __NVIC_PRIO_BITS), 0);

    /* Enable the TIM5 global Interrupt */
    HAL_NVIC_EnableIRQ(TIM5_IRQn);

    /* Enable TIM5 clock */
    __HAL_RCC_TIM5_CLK_ENABLE();

    /* Initialize TIM5 */
    s_hTimer.Instance = TIM5;

    /* Initialize TIMx peripheral */
    s_hTimer.Init.Period        = 625 - 1;   /* 0.5s */
    s_hTimer.Init.Prescaler     = 57600 - 1; /* 1250Hz */
    s_hTimer.Init.ClockDivision = 0;
    s_hTimer.Init.CounterMode   = TIM_COUNTERMODE_UP;

    if (HAL_TIM_Base_Init(&s_hTimer) == HAL_OK) {
        /* Start the TIM time Base generation in interrupt mode */
        HAL_TIM_Base_Start_IT(&s_hTimer);
    }

    /* We should never get here! */
    ASSERT(0);
}

void TIM5_IRQHandler(void) {
    WdogFeed();
    HAL_TIM_IRQHandler(&s_hTimer);
}
#endif /* APP_BOOT_ENABLE_WDOG */

#if BOOT_UART_ENABLE
/* Boot uart done callback function */
static void prvBootUartDone(void) {
    /* Clear boot mark */
    uint32_t ulBootMark = 0;
    MemEraseWrite(s_xMem, 0, sizeof(ulBootMark), (uint8_t *)&ulBootMark);
    /* Reset system */
    NVIC_SystemReset();
}
#endif /* BOOT_UART_ENABLE */

static void prvLoadApplication(uint32_t ulAddr) {
    if (((*(__IO uint32_t *)ulAddr) & 0x2FFE0000) == 0x20000000) {
        prvDisableInterrupt();
        prvClockDeInit();
        prvNvicVectorTableSet(NVIC_VECTTAB_FLASH, 0x10000);
        prvJumpToAddress(ulAddr);
    }
}

static void prvDisableInterrupt(void) {
    __HAL_TIM_DISABLE(&htim7);
    __disable_irq();
    for (IRQn_Type irq = (IRQn_Type)0; irq <= (IRQn_Type)67; irq++) {
        HAL_NVIC_DisableIRQ(irq);
        NVIC_ClearPendingIRQ(irq);
    }
    return;
}

static void prvClockDeInit(void) {
    HAL_RCC_DeInit();
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;
    HAL_DeInit();
    return;
}

static void prvNvicVectorTableSet(uint32_t ulNvicVectorTab, uint32_t ulOffset) {
    SCB->VTOR = ulNvicVectorTab | (ulOffset & NVIC_VECTTAB_OFFSET_MASK);
    return;
}

static __asm void prvJumpToAddress(uint32_t ulAddr) {
    LDR SP, [R0] ADD R0, #0x4 LDR R1, [R0] BX R1
}
