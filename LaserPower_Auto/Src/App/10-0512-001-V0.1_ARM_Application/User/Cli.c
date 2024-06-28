/*
    Cli.c

    Implementation File for App Cli Module
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 17Nov23, Karl Created
    01b, 24Nov23, Karl Added reset and upgrade
*/

/* Includes */
#include "Include.h"

/* Debug config */
#if CLI_DEBUG
    #undef TRACE
    #define TRACE(...)  DebugPrintf(__VA_ARGS__)
#else
    #undef TRACE
    #define TRACE(...)
#endif /* CLI_DEBUG */
#if CLI_ASSERT
    #undef ASSERT
    #define ASSERT(a)   while(!(a)){DebugPrintf("ASSERT failed: %s %d\n", __FILE__, __LINE__);}
#else
    #undef ASSERT
    #define ASSERT(...)
#endif /* CLI_ASSERT */

/* Local types */
typedef struct {
    Bool_t       bInit;
    UartHandle_t xUart;
    xQueueHandle xRxQueue; 
    char         cBuffer[64];
    char         cFmtBuffer[128];
}CliUartCtrl_t;

/* Forward declaration */
static void     prvCliUartTask  (void* pvPara);
static void     prvCliUartPrintf(const char* cFormat, ...);
static Status_t prvCliUartRecvCb(uint8_t *pucBuf, uint16_t usLength, void* pvIsrPara);

/* Local variables */
#if CLI_ENABLE_SECURITY
static Bool_t s_bEnCli = FALSE;
#else
static Bool_t s_bEnCli = TRUE;
#endif /* CLI_ENABLE_SECURITY */
static const char* s_cConfigMark = "Master!23";
static CliUartCtrl_t s_xCliUartCtrl;

/* Functions */
Status_t AppCliInit(void)
{
    extern const int CliCmdTab$$Base;
    extern const int CliCmdTab$$Limit;
    
    CliInit();
    CliCustomInit();
    CliCustomLoadCmd((const cli_command_t*)(&CliCmdTab$$Base), ((uint32_t)&CliCmdTab$$Limit - (uint32_t)&CliCmdTab$$Base) / sizeof(cli_command_t));
    
    xTaskCreate(prvCliUartTask, "tCli", 256, (void*)&s_xCliUartCtrl, tskIDLE_PRIORITY, NULL);

    return STATUS_OK;
}

Status_t AppCliTerm(void)
{
    /* Do nothing */
    return STATUS_OK;
}

Bool_t CliIsEnabled(void)
{
    return s_bEnCli;
}

static void prvCliUartTask(void* pvPara)
{
    CliUartCtrl_t *pxCtrl = (CliUartCtrl_t*)pvPara;
    
    TRACE("Enter prvCliUartTask\n");
    
    /* Create the queues used to hold Rx characters. */
    pxCtrl->xRxQueue = xQueueCreate(256, (portBASE_TYPE)sizeof(char));
    
    /* Create UART com port */
    pxCtrl->xUart = UartCreate();
    UartConfigCb(pxCtrl->xUart, prvCliUartRecvCb, UartIsrCb, UartDmaRxIsrCb, UartDmaTxIsrCb, pxCtrl);
#if CLI_ENABLE_UART_DMA
    UartConfigRxDma(pxCtrl->xUart, CLI_UART_DMA_RX_CHAN, CLI_UART_DMA_RX_ISR);
    UartConfigTxDma(pxCtrl->xUart, CLI_UART_DMA_TX_CHAN, CLI_UART_DMA_TX_ISR);
#endif /* CLI_ENABLE_UART_DMA */
    UartConfigCom(pxCtrl->xUart, CLI_UART_HANDLE, CLI_UART_BAUDRATE, CLI_UART_ISR);
    pxCtrl->bInit = TRUE;
    
    /* Read and process data */
    while (1) {
        char cChr;
        xQueueReceive(pxCtrl->xRxQueue, &cChr, portMAX_DELAY);
        CliCustomInput(prvCliUartPrintf, cChr);
    }
}

static void prvCliUartPrintf(const char* cFormat, ...)
{
    CliUartCtrl_t *pxCtrl = &s_xCliUartCtrl;
    if (pxCtrl && pxCtrl->xUart) {
        va_list va;
        va_start(va, cFormat);
        uint16_t usLength = vsprintf(pxCtrl->cFmtBuffer, cFormat, va);
        if (usLength) {
            UartBlkSend(pxCtrl->xUart, (uint8_t*)pxCtrl->cFmtBuffer, usLength, 1000);
        }
    }
    return;
}

static Status_t prvCliUartRecvCb(uint8_t *pucBuf, uint16_t usLength, void* pvIsrPara)
{
    CliUartCtrl_t *pxCtrl = (CliUartCtrl_t*)pvIsrPara;
    
    ASSERT(NULL != pxCtrl);
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    for (uint16_t n = 0; n < usLength; n++) {
        xQueueSendFromISR(pxCtrl->xRxQueue, pucBuf + n, &xHigherPriorityTaskWoken); 
    }
    
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    
    return STATUS_OK;
}

void CLI_UART_ISR_HANDLER(void)
{
    UartIsr(s_xCliUartCtrl.xUart);
}

#if CLI_ENABLE_UART_DMA
#ifdef CLI_UART_DMA_RX_ISR_HANDLER
void CLI_UART_DMA_RX_ISR_HANDLER(void)
{
    UartDmaRxIsr(s_xCliUartCtrl.xUart);
}
#endif /* CLI_UART_DMA_RX_ISR_HANDLER */

#ifdef CLI_UART_DMA_TX_ISR_HANDLER
void CLI_UART_DMA_TX_ISR_HANDLER(void)
{
    UartDmaTxIsr(s_xCliUartCtrl.xUart);
}
#endif /* CLI_UART_DMA_TX_ISR_HANDLER */
#endif /* CLI_ENABLE_UART_DMA */

static void prvCliCmdEncli(cli_printf cliprintf, int argc, char** argv)
{
    if (3 == argc) {
        int lEnCli;
    #if CLI_ENABLE_SECURITY
        if (0 != strcmp(argv[1], s_cConfigMark)) {
            cliprintf("CLI error: wrong config mark\n");
            return;
        }
    #endif /* CLI_ENABLE_SECURITY */
        lEnCli = atoi(argv[2]);
        if (lEnCli > 0) {
            s_bEnCli = TRUE;
            cliprintf("Enable cli ok\n");
        }
        else {
            s_bEnCli = FALSE;
            cliprintf("Disable cli ok\n");
        }
    }
    else {
        cliprintf("CLI error: wrong parameters\n");
    }
    return;
}    
CLI_CMD_EXPORT(encli, enable cli, prvCliCmdEncli)

static void prvCliCmdReset(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    NVIC_SystemReset();
}
CLI_CMD_EXPORT(reset, system reset, prvCliCmdReset)

static void prvCliCmdUpgrade(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    uint32_t ulUpdateMark = 0x1234ABCD;
    MemFlashWrite(0, sizeof(ulUpdateMark), (uint8_t*)&ulUpdateMark);
    
    cliprintf("enter upgrade mode after system reset\n");
}
CLI_CMD_EXPORT(upgrade, enter upgrade mode after system reset, prvCliCmdUpgrade)
