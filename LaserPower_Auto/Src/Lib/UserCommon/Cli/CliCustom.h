/*
    CliCustom.h

    Head File for Cli Custom Module
    
    Reference: https://github.com/MaJerle/ESP_AT_Lib
*/

/* Copyright 2020 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 01Aug19, Karl Created
    01b, 27Mar20, Karl Added cmd_cur_pos to improve function key
    01c, 01Jun20, Karl Fixed CliCustomLoadCmd bug - call printf before UART has been configured
*/

#ifndef __CLI_CUSTOM_H__
#define __CLI_CUSTOM_H__

/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include "Include/Include.h"
#include "Cli/CliConfig.h"


/**
 * \ingroup         CLI
 * \defgroup        CLI_CONFIG Configuration
 * \brief           Default CLI configuration
 * \{
 *
 * Configuration for command line interface (CLI).
 */

/**
 * \brief           CLI promet, printed on every NL
 */
#ifndef CLI_PROMPT
#define CLI_PROMPT                  "> "
#endif

/**
 * \brief           CLI NL, default is NL and CR
 */
#ifndef CLI_NL
#define CLI_NL                      "\r\n"
#endif

/**
 * \brief           Max CLI command length
 */
#ifndef CLI_MAX_CMD_LENGTH
#define CLI_MAX_CMD_LENGTH          256
#endif

/**
 * \brief           Max sotred CLI commands to history
 */
#ifndef CLI_CMD_HISTORY
#define CLI_CMD_HISTORY             10
#endif


/**
 * \brief           Max CLI arguments in a single command
 */
#ifndef CLI_MAX_NUM_OF_ARGS
#define CLI_MAX_NUM_OF_ARGS         16
#endif

/**
 * \brief           Max modules for CLI
 */
#ifndef CLI_MAX_MODULES
#define CLI_MAX_MODULES             16
#endif

/**
 * \}
 */

/**
 * \defgroup        CLI Command line interface
 * \brief           Command line interface
 * \{
 *
 * Functions to initialize everything needed for command line interface (CLI).
 */

typedef void cli_printf(const char* format, ...);
typedef void cli_function(cli_printf cliprintf, int argc, char** argv);

/**
 * \brief           CLI command structure
 */
typedef struct {                                
    const char* name;                           /*!< Command name */
    const char* help;                           /*!< Command help */
    cli_function* func;                         /*!< Command function */
} cli_command_t;

/**
 * \brief           List of commands
 */
typedef struct {
    const cli_command_t* commands;              /*!< Pointer to commands */
    size_t num_of_commands;                     /*!< Total number of commands */
} cli_commands_t;

/* Functions */
Status_t    CliCustomInit(void);
Status_t    CliCustomTerm(void);

Status_t    CliCustomLoadCmd(const cli_command_t *commands, size_t num_of_commands);
void        CliCustomInput(cli_printf cliprintf, char ch);

/**
 * \}
 */

/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif /* __CLI_CUSTOM_H__ */

