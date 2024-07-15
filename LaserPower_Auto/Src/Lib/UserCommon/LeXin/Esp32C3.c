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
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "stdlib.h"
#include "LeXin/Esp32C3.h"
#include "Uart/Uart.h"
#include "Cli/Cli.h"
#include "User/Cli.h"

/* Forward functions */
static Status_t prvUartRecvWbc(uint8_t *pucBuf, uint16_t usLength, void *pvIsrPara);

UartHandle_t g_Esp32C3Uart = NULL;

Status_t Esp32C3Init(void) {
    
    g_Esp32C3Uart = UartCreate();
    UartConfigCb(g_Esp32C3Uart, prvUartRecvWbc, UartIsrCb, NULL, NULL, NULL);
    UartConfigCom(g_Esp32C3Uart, UART5, 115200, UART5_IRQn);
    
    BtInit(g_Esp32C3Uart);
    return STATUS_OK;
}

Status_t BtInit(UartHandle_t UartHandle)
{
    SendAtCmd(UartHandle, "AT+BLEINIT=2\r");
    SendAtCmd(UartHandle, "AT+BLEGATTSSRVCRE\r");
    SendAtCmd(UartHandle, "AT+BLEGATTSSRVSTART\r");
    SendAtCmd(UartHandle, "AT+BLEADVPARAM=50,50,0,0,7,0,,\r");
    SendAtCmd(UartHandle, "AT+BLEADVDATAEX=\"LaserPower\",\"A002\",\"0102030405\",1\r");
    
//    SendAtCmd(UartHandle, "AT+BLESPPCFG=1,1,6,1,3\r");
//    SendAtCmd(UartHandle, "AT+BLESPP\r");
    return STATUS_OK;
}

Status_t SendAtCmd(UartHandle_t UartHandle, const char * CmdStr)
{
    UartBlkSend(UartHandle, (uint8_t *)CmdStr, strlen(CmdStr), 10);
    osDelay(1000);
    return STATUS_OK;
}

static Status_t prvUartRecvWbc(uint8_t *pucBuf, uint16_t usLength, void *pvIsrPara) {
    UartBlkSend(g_Esp32C3Uart, pucBuf, usLength, 10);
    return STATUS_OK;
}

static void prvCliCmdWbcSend(cli_printf cliprintf, int argc, char **argv) {
    CHECK_CLI();

    if (argc < 2) {
        cliprintf("wbc_send CDM\n");
        return;
    }
    
    uint8_t *pucBuffer = (uint8_t *)argv[1];

    cliprintf("cmd : %s\n", pucBuffer);
    
    UartBlkSend(g_Esp32C3Uart, pucBuffer, sizeof(pucBuffer) / sizeof(pucBuffer[0]), 10);

    return;
}
CLI_CMD_EXPORT(wbc_send, wifi bluetooth command send, prvCliCmdWbcSend)

static void prvCliCmdBtInit(cli_printf cliprintf, int argc, char **argv) {
    CHECK_CLI();

    if (argc != 1) {
        cliprintf("bt_init\n");
        return;
    }
    
    BtInit(g_Esp32C3Uart);
    
    return;
}
CLI_CMD_EXPORT(bt_init, Bluetooth init, prvCliCmdBtInit)

static void prvCliCmdBtTrs(cli_printf cliprintf, int argc, char **argv) {
    CHECK_CLI();

    if (argc != 2) {
        cliprintf("bt_trs MODE\n");
        return;
    }
    
    int mode = atoi(argv[1]);
    
    if (mode == 1){
        SendAtCmd(g_Esp32C3Uart, "AT+BLESPPCFG=1,1,6,1,3\r");
        osDelay(1000);
        SendAtCmd(g_Esp32C3Uart, "AT+BLESPP\r");
        
        cliprintf("Enter transprent mode\n");
    }
    else{
        SendAtCmd(g_Esp32C3Uart, "+++");
        osDelay(1000);
        SendAtCmd(g_Esp32C3Uart, "AT\r");
        
        cliprintf("Exit transprent mode\n");
    }
    
    return;
}
CLI_CMD_EXPORT(bt_trs, Bluetooth transparent mode, prvCliCmdBtTrs)

void UART5_IRQHandler(void) {
    if (g_Esp32C3Uart) {
        UartIsr(g_Esp32C3Uart);
    }
}
