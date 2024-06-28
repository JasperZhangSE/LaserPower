/*
    Cli.h

    Head File for App Cli Module
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 17Nov23, Karl Created
    01b, 24Nov23, Karl Added reset and upgrade
*/

#ifndef ___CLI_H__
#define ___CLI_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

/* Defines */
#if 0
#ifndef CLI_ENABLE_SECURITY
#define CLI_ENABLE_SECURITY             1
#endif /* CLI_ENABLE_SECURITY */
#ifndef CLI_UART_HANDLE
#define CLI_UART_HANDLE                 USART1
#endif /* CLI_UART_HANDLE */
#ifndef CLI_UART_BAUDRATE
#define CLI_UART_BAUDRATE               115200
#endif /* CLI_UART_BAUDRATE */
#ifndef CLI_UART_ISR
#define CLI_UART_ISR                    USART1_IRQn
#endif /* CLI_UART_ISR */
#ifndef CLI_UART_ISR_HANDLER
#define CLI_UART_ISR_HANDLER            USART1_IRQHandler
#endif /* CLI_UART_ISR_HANDLER */
#ifndef CLI_ENABLE_UART_DMA
#define CLI_ENABLE_UART_DMA             1
#endif /* CLI_ENABLE_UART_DMA */
#ifndef CLI_UART_DMA_RX_CHAN
#define CLI_UART_DMA_RX_CHAN            DMA1_Channel5
#endif /* CLI_UART_DMA_RX_CHAN */
#ifndef CLI_UART_DMA_RX_ISR
#define CLI_UART_DMA_RX_ISR             DMA1_Channel5_IRQn
#endif /* CLI_UART_DMA_RX_ISR */
#ifndef CLI_UART_DMA_RX_ISR_HANDLER
#define CLI_UART_DMA_RX_ISR_HANDLER     DMA1_Channel5_IRQHandler
#endif /* CLI_UART_DMA_RX_ISR_HANDLER */
#ifndef CLI_UART_DMA_TX_CHAN
#define CLI_UART_DMA_TX_CHAN            DMA1_Channel4
#endif /* CLI_UART_DMA_TX_CHAN */
#ifndef CLI_UART_DMA_TX_ISR
#define CLI_UART_DMA_TX_ISR             DMA1_Channel4_IRQn
#endif /* CLI_UART_DMA_TX_ISR */
#ifndef CLI_UART_DMA_TX_ISR_HANDLER
#define CLI_UART_DMA_TX_ISR_HANDLER     DMA1_Channel4_IRQHandler
#endif /* CLI_UART_DMA_TX_ISR_HANDLER */
#endif

#if 1
#ifndef CLI_ENABLE_SECURITY
#define CLI_ENABLE_SECURITY             1
#endif /* CLI_ENABLE_SECURITY */
#ifndef CLI_UART_HANDLE
#define CLI_UART_HANDLE                 UART4
#endif /* CLI_UART_HANDLE */
#ifndef CLI_UART_BAUDRATE
#define CLI_UART_BAUDRATE               115200
#endif /* CLI_UART_BAUDRATE */
#ifndef CLI_UART_ISR
#define CLI_UART_ISR                    UART4_IRQn
#endif /* CLI_UART_ISR */
#ifndef CLI_UART_ISR_HANDLER
#define CLI_UART_ISR_HANDLER            UART4_IRQHandler
#endif /* CLI_UART_ISR_HANDLER */
#ifndef CLI_ENABLE_UART_DMA
#define CLI_ENABLE_UART_DMA             1
#endif /* CLI_ENABLE_UART_DMA */
#ifndef CLI_UART_DMA_RX_CHAN
#define CLI_UART_DMA_RX_CHAN            DMA2_Channel3
#endif /* CLI_UART_DMA_RX_CHAN */
#ifndef CLI_UART_DMA_RX_ISR
#define CLI_UART_DMA_RX_ISR             DMA2_Channel3_IRQn
#endif /* CLI_UART_DMA_RX_ISR */
#ifndef CLI_UART_DMA_RX_ISR_HANDLER
#define CLI_UART_DMA_RX_ISR_HANDLER     DMA2_Channel3_IRQHandler
#endif /* CLI_UART_DMA_RX_ISR_HANDLER */
#ifndef CLI_UART_DMA_TX_CHAN
#define CLI_UART_DMA_TX_CHAN            DMA2_Channel5
#endif /* CLI_UART_DMA_TX_CHAN */
#ifndef CLI_UART_DMA_TX_ISR
#define CLI_UART_DMA_TX_ISR             DMA2_Channel5_IRQn
#endif /* CLI_UART_DMA_TX_ISR */
#ifndef CLI_UART_DMA_TX_ISR_HANDLER
#define CLI_UART_DMA_TX_ISR_HANDLER     DMA2_Channel5_IRQHandler
#endif /* CLI_UART_DMA_TX_ISR_HANDLER */
#endif

#if 0
#ifndef CLI_ENABLE_SECURITY
#define CLI_ENABLE_SECURITY             1
#endif /* CLI_ENABLE_SECURITY */
#ifndef CLI_UART_HANDLE
#define CLI_UART_HANDLE                 UART5
#endif /* CLI_UART_HANDLE */
#ifndef CLI_UART_BAUDRATE
#define CLI_UART_BAUDRATE               115200
#endif /* CLI_UART_BAUDRATE */
#ifndef CLI_UART_ISR
#define CLI_UART_ISR                    UART5_IRQn
#endif /* CLI_UART_ISR */
#ifndef CLI_UART_ISR_HANDLER
#define CLI_UART_ISR_HANDLER            UART5_IRQHandler
#endif /* CLI_UART_ISR_HANDLER */
#ifndef CLI_ENABLE_UART_DMA
#define CLI_ENABLE_UART_DMA             0
#endif /* CLI_ENABLE_UART_DMA */
#ifndef CLI_UART_DMA_RX_CHAN
#define CLI_UART_DMA_RX_CHAN            NULL
#endif /* CLI_UART_DMA_RX_CHAN */
#ifndef CLI_UART_DMA_RX_ISR
#define CLI_UART_DMA_RX_ISR             NULL
#endif /* CLI_UART_DMA_RX_ISR */
#ifndef CLI_UART_DMA_RX_ISR_HANDLER
#define CLI_UART_DMA_RX_ISR_HANDLER     NULL
#endif /* CLI_UART_DMA_RX_ISR_HANDLER */
#ifndef CLI_UART_DMA_TX_CHAN
#define CLI_UART_DMA_TX_CHAN            NULL
#endif /* CLI_UART_DMA_TX_CHAN */
#ifndef CLI_UART_DMA_TX_ISR
#define CLI_UART_DMA_TX_ISR             NULL
#endif /* CLI_UART_DMA_TX_ISR */
#ifndef CLI_UART_DMA_TX_ISR_HANDLER
#define CLI_UART_DMA_TX_ISR_HANDLER     NULL
#endif /* CLI_UART_DMA_TX_ISR_HANDLER */
#endif

#define CHECK_CLI()                     if (!CliIsEnabled()) {cliprintf("Please enable cli first\n"); return;}

/* Functions */
Status_t AppCliInit(void);
Status_t AppCliTerm(void);

Bool_t   CliIsEnabled(void);

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif /* ___CLI_H__ */
