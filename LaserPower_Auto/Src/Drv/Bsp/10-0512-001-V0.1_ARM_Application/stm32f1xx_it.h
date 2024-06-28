/*
    stm32f1xx_it.h
    
    Head File for STM32 Interrupt Handler Module
*/

/* Copyright 2019 Shanghai Master Inc. */

/*
    Modification History
    --------------------
    01a, 20Aug19, Karl Created
*/

#ifndef __STM32F1XX_IT_H__
#define __STM32F1XX_IT_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Functions */
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void DebugMon_Handler(void);
void SysTick_Handler(void);
void TIM7_IRQHandler(void);
void ETH_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32F1XX_IT_H__ */
