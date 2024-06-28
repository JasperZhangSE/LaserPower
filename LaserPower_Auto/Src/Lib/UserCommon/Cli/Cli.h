/*
    Cli.h

    Head File for Cli Module
    
    Reference: FreeRTOS Sample-CLI-commands.c
*/

/* Copyright 2019 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 11Dec18, Karl Created
    01b, 01Aug19, Karl Reconstructured Cli lib
    01c, 27Aug19, Karl Modified include files
*/

#ifndef __CLI_H__
#define __CLI_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

/* Includes */
#include <stdint.h>
#include "Include/Include.h"
#include "Cli/CliConfig.h"
#include "Cli/CliCustom.h"

/* Defines */
#define CLI_CMD_EXPORT(name, help, func)                              \
        const char __cli_##name##_name[] = #name;                     \
        const char __cli_##name##_help[] = #help;                     \
        USED const cli_command_t __cli_##name SECTION("CliCmdTab") =  \
        {                                                             \
            __cli_##name##_name,                                      \
            __cli_##name##_help,                                      \
            func                                                      \
        };

/* Functions */
Status_t CliInit(void);
Status_t CliTerm(void);

Status_t CliLoadCmd(void* pvCmd);

#if CLI_TEST
Status_t CliTestFreeRtos(uint8_t ucSel);
Status_t CliTestCustom(uint8_t ucSel);
#endif /* CLI_TEST */
    
#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif /* __CLI_H__ */
