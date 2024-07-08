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
 * Author:          Jasper <jasperzhangse@gmail.com>
 * Version:         v1.0.0
 */

#ifndef __ESP32C3_H__
#define __ESP32C3_H__

#include "Include/Include.h"
#include "Uart/Uart.h"

const char *basic_cmd[] = {
    /* Basic command */
    "AT",                 /* 测试 AT 启动 */
    "AT+RST",             /* 重启模块 */
    "AT+GMR",             /* 查看版本信息 */
    "AT+CMD",             /* 查询当前固件支持的所有命令及命令类型 */
    "AT+GSLP",            /* 进入 Deep-sleep 模式 */
    "ATE",                /* 开启或关闭 AT 回显功能 */
    "AT+RESTORE",         /* 恢复出厂设置 */
    "AT+SAVETRANSLINK",   /* 设置开机透传模式 信息 */
    "AT+TRANSINTVL",      /* 设置透传模式 模式下的数据发送间隔 */
    "AT+UART_CUR",        /* 设置 UART 当前临时配置，不保存到 flash */
    "AT+UART_DEF",        /* 设置UART 默认配置, 保存到 flash */
    "AT+SLEEP",           /* 设置 sleep 模式 */
    "AT+SYSRAM",          /* 查询当前剩余堆空间和最小堆空间 */
    "AT+SYSMSG",          /* 查询/设置系统提示信息 */
    "AT+SYSMSGFILTER",    /* 启用或禁用系统消息 过滤 */
    "AT+SYSMSGFILTERCFG", /* 查询/配置系统消息 的过滤器 */
    "AT+SYSFLASH",        /* 查询或读写 flash 用户分区 */
    "AT+SYSMFG",          /* 查询或读写manufacturing nvs 用户分区 */
    "AT+RFPOWER",         /* 查询/设置 RF TX Power */
    "AT+SYSROLLBACK",     /* 回滚到以前的固件 */
    "AT+SYSTIMESTAMP",    /* 查询/设置本地时间戳 */
    "AT+SYSLOG",          /* 启用或禁用AT 错误代码提示 */
    "AT+SLEEPWKCFG",      /* 设置 Light-sleep 唤醒源和唤醒GPIO */
    "AT+SYSSTORE",        /* 设置参数存储模式 */
    "AT+SYSREG",          /* 读写寄存器 */
    "AT+SYSTEMP"          /* 读取芯片内部摄氏温度值 */
};

const char *ble_cmd[] = {
    "AT+BLEINIT",             /* Bluetooth LE 初始化 */
    "AT+BLEADDR",             /* 设置Bluetooth LE 设备地址 */
    "AT+BLENAME",             /* 查询/设置 Bluetooth LE 设备名称 */
    "AT+BLESCANPARAM",        /* 查询/设置 Bluetooth LE 扫描参数 */
    "AT+BLESCAN",             /* 使能Bluetooth LE 扫描 */
    "AT+BLESCANRSPDATA",      /* 设置Bluetooth LE 扫描响应 */
    "AT+BLEADVPARAM",         /* 查询/设置 Bluetooth LE 广播参数 */
    "AT+BLEADVDATA",          /* 设置Bluetooth LE 广播数据 */
    "AT+BLEADVDATAEX",        /* 自动设置 Bluetooth LE 广播数据 */
    "AT+BLEADVSTART",         /* 开始 Bluetooth LE 广播 */
    "AT+BLEADVSTOP",          /* 停止Bluetooth LE 广播 */
    "AT+BLECONN",             /* 建立Bluetooth LE 连接 */
    "AT+BLECONNPARAM",        /* 查询/更新 Bluetooth LE 连接参数 */
    "AT+BLEDISCONN",          /* 断开 Bluetooth LE 连接 */
    "AT+BLEDATALEN",          /* 设置Bluetooth LE 数据包长度 */
    "AT+BLECFGMTU",           /* 设置 Bluetooth LE MTU 长度 */
    "AT+BLEGATTSSRVCRE",      /* GATTS 创建服务 */
    "AT+BLEGATTSSRVSTART",    /* GATTS 开启服务 */
    "AT+BLEGATTSSRVSTOP",     /* GATTS 停止服务 */
    "AT+BLEGATTSSRV",         /* GATTS 发现服务 */
    "AT+BLEGATTSCHAR",        /* GATTS 发现服务特征 */
    "AT+BLEGATTSNTFY",        /* 服务器 notify 服务特征值给客户端 */
    "AT+BLEGATTSIND",         /* 服务器indicate 服务特征值给客户端 */
    "AT+BLEGATTSSETATTR",     /* GATTS 设置服务特征值 */
    "AT+BLEGATTCPRIMSRV",     /* GATTC 发现基本服务 */
    "AT+BLEGATTCINCLSRV",     /* GATTC 发现包含的服务 */
    "AT+BLEGATTCCHAR",        /* GATTC 发现服务特征 */
    "AT+BLEGATTCRD",          /* GATTC 读取服务特征值 */
    "AT+BLEGATTCWR",          /* GATTC 写服务特征值 */
    "AT+BLESPPCFG",           /* 查询/设置 Bluetooth LE SPP 参数 */
    "AT+BLESPP",              /* 进入 Bluetooth LE SPP 模式 */
    "AT+SAVETRANSLINK",       /* 设置 Bluetooth LE 开机透传模式 信息 */
    "AT+BLESECPARAM",         /* 查询/设置 Bluetooth LE 加密参数 */
    "AT+BLEENC",              /* 发起 Bluetooth LE 加密请求 */
    "AT+BLEENCRSP",           /* 回复对端设备发起的配对请求 */
    "AT+BLEKEYREPLY",         /* 给对方设备回复密钥 */
    "AT+BLECONFREPLY",        /* 给对方设备回复确认结果（传统连接阶段） */
    "AT+BLEENCDEV",           /* 查询绑定的 Bluetooth LE 加密设备列表 */
    "AT+BLEENCCLEAR",         /* 清除 Bluetooth LE 加密设备列表 */
    "AT+BLESETKEY",           /* 设置Bluetooth LE 静态配对密钥 */
    "AT+BLEHIDINIT",          /* Bluetooth LE HID 协议初始化 */
    "AT+BLEHIDKB",            /* 发送Bluetooth LE HID 键盘信息 */
    "AT+BLEHIDMUS",           /* 发送 Bluetooth LE HID 鼠标信息 */
    "AT+BLEHIDCONSUMER",      /* 发送Bluetooth LE HID consumer 信息 */
    "AT+BLUFI",               /* 开启或关闭 BluFi */
    "AT+BLUFINAME",           /* 查询/设置 BluFi 设备名称 */
    "AT+BLUFISEND",           /* 发送 BluFi 用户自定义数据 */
    "AT+BLEPERIODICDATA",     /* 设置 Bluetooth LE 周期性广播数据 */
    "AT+BLEPERIODICSTART",    /* 开启 Bluetooth LE 周期性广播 */
    "AT+BLEPERIODICSTOP",     /* 停止 Bluetooth LE 周期性广播 */
    "AT+BLESYNCSTART",        /* 开启周期性广播同步 */
    "AT+BLESYNCSTOP",         /* 停止周期性广播同步 */
    "AT+BLEREADPHY",          /* 查询当前连接使用的 PHY */
    "AT+BLESETPHY"            /* 设置当前连接使用的 PHY */
};

/* Cmd type id */
typedef enum {
    basic = 0,
    ble,
    wifi
}at_cmd_type_t;

/* Basic command */
typedef enum {
    CMD_AT,                 /* AT */
    CMD_AT_RST,             /* AT+RST */
    CMD_AT_GMR,             /* AT+GMR */
    CMD_AT_CMD,             /* AT+CMD */
    CMD_AT_GSLP,            /* AT+GSLP */
    CMD_ATE,                /* ATE */
    CMD_AT_RESTORE,         /* AT+RESTORE */
    CMD_AT_SAVETRANSLINK,   /* AT+SAVETRANSLINK */
    CMD_AT_TRANSINTVL,      /* AT+TRANSINTVL */
    CMD_AT_UART_CUR,        /* AT+UART_CUR */
    CMD_AT_UART_DEF,        /* AT+UART_DEF */
    CMD_AT_SLEEP,           /* AT+SLEEP */
    CMD_AT_SYSRAM,          /* AT+SYSRAM */
    CMD_AT_SYSMSG,          /* AT+SYSMSG */
    CMD_AT_SYSMSGFILTER,    /* AT+SYSMSGFILTER */
    CMD_AT_SYSMSGFILTERCFG, /* AT+SYSMSGFILTERCFG */
    CMD_AT_SYSFLASH,        /* AT+SYSFLASH */
    CMD_AT_SYSMFG,          /* AT+SYSMFG */
    CMD_AT_RFPOWER,         /* AT+RFPOWER */
    CMD_AT_SYSROLLBACK,     /* AT+SYSROLLBACK */
    CMD_AT_SYSTIMESTAMP,    /* AT+SYSTIMESTAMP */
    CMD_AT_SYSLOG,          /* AT+SYSLOG */
    CMD_AT_SLEEPWKCFG,      /* AT+SLEEPWKCFG */
    CMD_AT_SYSSTORE,        /* AT+SYSSTORE */
    CMD_AT_SYSREG,          /* AT+SYSREG */
    CMD_AT_SYSTEMP,         /* AT+SYSTEMP */
    NUM_BASIC_CMDS,         /* Number of basic commands */
}basic_cmd_t;

/* Bluetooth command */
typedef enum {
    CMD_BLEINIT,             /* AT+BLEINIT */
    CMD_BLEADDR,             /* AT+BLEADDR */
    CMD_BLENAME,             /* AT+BLENAME */
    CMD_BLESCANPARAM,        /* AT+BLESCANPARAM */
    CMD_BLESCAN,             /* AT+BLESCAN */
    CMD_BLESCANRSPDATA,      /* AT+BLESCANRSPDATA */
    CMD_BLEADVPARAM,         /* AT+BLEADVPARAM */
    CMD_BLEADVDATA,          /* AT+BLEADVDATA */
    CMD_BLEADVDATAEX,        /* AT+BLEADVDATAEX */
    CMD_BLEADVSTART,         /* AT+BLEADVSTART */
    CMD_BLEADVSTOP,          /* AT+BLEADVSTOP */
    CMD_BLECONN,             /* AT+BLECONN */
    CMD_BLECONNPARAM,        /* AT+BLECONNPARAM */
    CMD_BLEDISCONN,          /* AT+BLEDISCONN */
    CMD_BLEDATALEN,          /* AT+BLEDATALEN */
    CMD_BLECFGMTU,           /* AT+BLECFGMTU */
    CMD_BLEGATTSSRVCRE,      /* AT+BLEGATTSSRVCRE */
    CMD_BLEGATTSSRVSTART,    /* AT+BLEGATTSSRVSTART */
    CMD_BLEGATTSSRVSTOP,     /* AT+BLEGATTSSRVSTOP */
    CMD_BLEGATTSSRV,         /* AT+BLEGATTSSRV */
    CMD_BLEGATTSCHAR,        /* AT+BLEGATTSCHAR */
    CMD_BLEGATTSNTFY,        /* AT+BLEGATTSNTFY */
    CMD_BLEGATTSIND,         /* AT+BLEGATTSIND */
    CMD_BLEGATTSSETATTR,     /* AT+BLEGATTSSETATTR */
    CMD_BLEGATTCPRIMSRV,     /* AT+BLEGATTCPRIMSRV */
    CMD_BLEGATTCINCLSRV,     /* AT+BLEGATTCINCLSRV */
    CMD_BLEGATTCCHAR,        /* AT+BLEGATTCCHAR */
    CMD_BLEGATTCRD,          /* AT+BLEGATTCRD */
    CMD_BLEGATTCWR,          /* AT+BLEGATTCWR */
    CMD_BLESPPCFG,           /* AT+BLESPPCFG */
    CMD_BLESPP,              /* AT+BLESPP */
    CMD_SAVETRANSLINK,       /* AT+SAVETRANSLINK */
    CMD_BLESECPARAM,         /* AT+BLESECPARAM */
    CMD_BLEENC,              /* AT+BLEENC */
    CMD_BLEENCRSP,           /* AT+BLEENCRSP */
    CMD_BLEKEYREPLY,         /* AT+BLEKEYREPLY */
    CMD_BLECONFREPLY,        /* AT+BLECONFREPLY */
    CMD_BLEENCDEV,           /* AT+BLEENCDEV */
    CMD_BLEENCCLEAR,         /* AT+BLEENCCLEAR */
    CMD_BLESETKEY,           /* AT+BLESETKEY */
    CMD_BLEHIDINIT,          /* AT+BLEHIDINIT */
    CMD_BLEHIDKB,            /* AT+BLEHIDKB */
    CMD_BLEHIDMUS,           /* AT+BLEHIDMUS */
    CMD_BLEHIDCONSUMER,      /* AT+BLEHIDCONSUMER */
    CMD_BLUFI,               /* AT+BLUFI */
    CMD_BLUFINAME,           /* AT+BLUFINAME */
    CMD_BLUFISEND,           /* AT+BLUFISEND */
    CMD_BLEPERIODICDATA,     /* AT+BLEPERIODICDATA */
    CMD_BLEPERIODICSTART,    /* AT+BLEPERIODICSTART */
    CMD_BLEPERIODICSTOP,     /* AT+BLEPERIODICSTOP */
    CMD_BLESYNCSTART,        /* AT+BLESYNCSTART */
    CMD_BLESYNCSTOP,         /* AT+BLESYNCSTOP */
    CMD_BLEREADPHY,          /* AT+BLEREADPHY */
    CMD_BLESETPHY,           /* AT+BLESETPHY */
    NUM_BLE_CMDS             /* Number of Bluetooth commands */
}ble_cmd_t;

Status_t    Esp32C3Init(void);

#endif /* __ESP32C3_H__ */
