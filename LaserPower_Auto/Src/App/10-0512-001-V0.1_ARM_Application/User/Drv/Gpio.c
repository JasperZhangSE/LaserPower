/*
    Gpio.c

    Implementation File for Gpio Module
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 17Nov23, Karl Created
    01b, 22Nov23, Karl Implemented di and do command
*/

/* Includes */
#include "Include.h"

/* Debug config */
#if GPIO_DEBUG
    #undef TRACE
    #define TRACE(...)  DebugPrintf(__VA_ARGS__)
#else
    #undef TRACE
    #define TRACE(...)
#endif /* GPIO_DEBUG */
#if GPIO_ASSERT
    #undef ASSERT
    #define ASSERT(a)   while(!(a)){DebugPrintf("ASSERT failed: %s %d\n", __FILE__, __LINE__);}
#else
    #undef ASSERT
    #define ASSERT(...)
#endif /* GPIO_ASSERT */

/* Functions */
Status_t DrvGpioInit(void)
{
    GpioInit();
    return STATUS_OK;
}

Status_t DrvGpioTerm(void)
{
    /* Do nothing */
    return STATUS_OK;
}

static void prvCliCmdDi(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    if (argc != 2) {
        cliprintf("di CHAN\n");
        return;
    }
    
    int lChan = atoi(argv[1]);
    int lData = GpioGetInput(lChan);
    
    if (lData != 0xFFFF) {
        cliprintf("DI-%d, %d\n", lChan, lData);
    }
    else {
        cliprintf("    wrong channel\n");
    }
}
CLI_CMD_EXPORT(di, show input gpio status, prvCliCmdDi)

static void prvCliCmdDo(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    if (argc != 3) {
        cliprintf("do CHAN ONOFF\n");
        return;
    }
    
    int lChan = atoi(argv[1]);
    int lCtrl = atoi(argv[2]);
    
    if (GpioSetOutput(lChan, lCtrl) == STATUS_OK) {
        cliprintf("DO-%d, %d\n", lChan, lCtrl);
    }
    else {
        cliprintf("    wrong channel\n");
    }
}
CLI_CMD_EXPORT(do, ctrl output gpio, prvCliCmdDo)
