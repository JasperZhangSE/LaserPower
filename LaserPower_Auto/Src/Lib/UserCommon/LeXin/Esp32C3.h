/**
 * \file            Esp32C3.c
 * \brief           Esp32C3 library
 */

/*
 * Copyright (c) 2024 Jasper
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file contrains all the Esp32C3 driver functions.
 *
 * Author:          Jasper <JasperZhangSE@gmail.com>
 * Version:         v1.0.0
 */
#ifndef __ESP32C3_H__
#define __ESP32C3_H__

#include "Include/Include.h"
#include "Uart/Uart.h"

typedef enum {
    AT_CMD_TEST,
    AT_CMD_RST,
    AT_CMD_GMR,
    AT_CMD_CMD,
    AT_CMD_GSLP,
    AT_CMD_ATE,
    AT_CMD_RESTORE,
    AT_CMD_SAVETRANSLINK,
    AT_CMD_TRANSINTVL,
    AT_CMD_UART_CUR,
    AT_CMD_UART_DEF,
    AT_CMD_SLEEP,
    AT_CMD_SYSRAM,
    AT_CMD_SYSMSG,
    AT_CMD_SYSMSGFILTER,
    AT_CMD_SYSMSGFILTERCFG,
    AT_CMD_SYSFLASH,
    AT_CMD_SYSMFG,
    AT_CMD_RFPOWER,
    AT_CMD_SYSROLLBACK,
    AT_CMD_SYSTIMESTAMP,
    AT_CMD_SYSLOG,
    AT_CMD_SLEEPWKCFG,
    AT_CMD_SYSSTORE,
    AT_CMD_SYSREG,
    AT_CMD_SYSTEMP,
    AT_CMD_MAX  // Keep this as the last item to indicate the number of commands
} AT_Command;

const char *AT_COMMAND_STRINGS[AT_CMD_MAX] = {"AT\r\n",
                                              "AT+RST\r\n",
                                              "AT+GMR\r\n",
                                              "AT+CMD\r\n",
                                              "AT+GSLP\r\n",
                                              "ATE\r\n",
                                              "AT+RESTORE\r\n",
                                              "AT+SAVETRANSLINK\r\n",
                                              "AT+TRANSINTVL\r\n",
                                              "AT+UART_CUR\r\n",
                                              "AT+UART_DEF\r\n",
                                              "AT+SLEEP\r\n",
                                              "AT+SYSRAM\r\n",
                                              "AT+SYSMSG\r\n",
                                              "AT+SYSMSGFILTER\r\n",
                                              "AT+SYSMSGFILTERCFG\r\n",
                                              "AT+SYSFLASH\r\n",
                                              "AT+SYSMFG\r\n",
                                              "AT+RFPOWER\r\n",
                                              "AT+SYSROLLBACK\r\n",
                                              "AT+SYSTIMESTAMP\r\n",
                                              "AT+SYSLOG\r\n",
                                              "AT+SLEEPWKCFG\r\n",
                                              "AT+SYSSTORE\r\n",
                                              "AT+SYSREG\r\n",
                                              "AT+SYSTEMP\r\n"};

Status_t    Esp32C3Init(void);
Status_t    SendAtCmd(UartHandle_t UartHandle, AT_Command command);

#endif /* __ESP32C3_H__ */
