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
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "LeXin/Esp32C3.h"
#include "Uart/Uart.h"

Status_t Esp32C3Init(void) {
    return STATUS_OK;
}

Status_t SendAtCmd(UartHandle_t UartHandle, uint16_t type_id, uint16_t cmd_id)
{
    const char * cmd_str = NULL;
    switch (type_id)
    {
        /* Basic at cmd */
        case basic:
            cmd_str = basic_cmd[cmd_id];
        break;
        
        /* Ble at cmd */
        case ble:
            cmd_str = ble_cmd[cmd_id];
        break;

        /* No search */
        default:
            /* Do nothing */
        break;
    }
    
    UartBlkSend(UartHandle, (uint8_t *)cmd_str, strlen(cmd_str), 10);
    
    return STATUS_OK;
}
