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
    "AT",                 /* ���� AT ���� */
    "AT+RST",             /* ����ģ�� */
    "AT+GMR",             /* �鿴�汾��Ϣ */
    "AT+CMD",             /* ��ѯ��ǰ�̼�֧�ֵ���������������� */
    "AT+GSLP",            /* ���� Deep-sleep ģʽ */
    "ATE",                /* ������ر� AT ���Թ��� */
    "AT+RESTORE",         /* �ָ��������� */
    "AT+SAVETRANSLINK",   /* ���ÿ���͸��ģʽ ��Ϣ */
    "AT+TRANSINTVL",      /* ����͸��ģʽ ģʽ�µ����ݷ��ͼ�� */
    "AT+UART_CUR",        /* ���� UART ��ǰ��ʱ���ã������浽 flash */
    "AT+UART_DEF",        /* ����UART Ĭ������, ���浽 flash */
    "AT+SLEEP",           /* ���� sleep ģʽ */
    "AT+SYSRAM",          /* ��ѯ��ǰʣ��ѿռ����С�ѿռ� */
    "AT+SYSMSG",          /* ��ѯ/����ϵͳ��ʾ��Ϣ */
    "AT+SYSMSGFILTER",    /* ���û����ϵͳ��Ϣ ���� */
    "AT+SYSMSGFILTERCFG", /* ��ѯ/����ϵͳ��Ϣ �Ĺ����� */
    "AT+SYSFLASH",        /* ��ѯ���д flash �û����� */
    "AT+SYSMFG",          /* ��ѯ���дmanufacturing nvs �û����� */
    "AT+RFPOWER",         /* ��ѯ/���� RF TX Power */
    "AT+SYSROLLBACK",     /* �ع�����ǰ�Ĺ̼� */
    "AT+SYSTIMESTAMP",    /* ��ѯ/���ñ���ʱ��� */
    "AT+SYSLOG",          /* ���û����AT ���������ʾ */
    "AT+SLEEPWKCFG",      /* ���� Light-sleep ����Դ�ͻ���GPIO */
    "AT+SYSSTORE",        /* ���ò����洢ģʽ */
    "AT+SYSREG",          /* ��д�Ĵ��� */
    "AT+SYSTEMP"          /* ��ȡоƬ�ڲ������¶�ֵ */
};

const char *ble_cmd[] = {
    "AT+BLEINIT",             /* Bluetooth LE ��ʼ�� */
    "AT+BLEADDR",             /* ����Bluetooth LE �豸��ַ */
    "AT+BLENAME",             /* ��ѯ/���� Bluetooth LE �豸���� */
    "AT+BLESCANPARAM",        /* ��ѯ/���� Bluetooth LE ɨ����� */
    "AT+BLESCAN",             /* ʹ��Bluetooth LE ɨ�� */
    "AT+BLESCANRSPDATA",      /* ����Bluetooth LE ɨ����Ӧ */
    "AT+BLEADVPARAM",         /* ��ѯ/���� Bluetooth LE �㲥���� */
    "AT+BLEADVDATA",          /* ����Bluetooth LE �㲥���� */
    "AT+BLEADVDATAEX",        /* �Զ����� Bluetooth LE �㲥���� */
    "AT+BLEADVSTART",         /* ��ʼ Bluetooth LE �㲥 */
    "AT+BLEADVSTOP",          /* ֹͣBluetooth LE �㲥 */
    "AT+BLECONN",             /* ����Bluetooth LE ���� */
    "AT+BLECONNPARAM",        /* ��ѯ/���� Bluetooth LE ���Ӳ��� */
    "AT+BLEDISCONN",          /* �Ͽ� Bluetooth LE ���� */
    "AT+BLEDATALEN",          /* ����Bluetooth LE ���ݰ����� */
    "AT+BLECFGMTU",           /* ���� Bluetooth LE MTU ���� */
    "AT+BLEGATTSSRVCRE",      /* GATTS �������� */
    "AT+BLEGATTSSRVSTART",    /* GATTS �������� */
    "AT+BLEGATTSSRVSTOP",     /* GATTS ֹͣ���� */
    "AT+BLEGATTSSRV",         /* GATTS ���ַ��� */
    "AT+BLEGATTSCHAR",        /* GATTS ���ַ������� */
    "AT+BLEGATTSNTFY",        /* ������ notify ��������ֵ���ͻ��� */
    "AT+BLEGATTSIND",         /* ������indicate ��������ֵ���ͻ��� */
    "AT+BLEGATTSSETATTR",     /* GATTS ���÷�������ֵ */
    "AT+BLEGATTCPRIMSRV",     /* GATTC ���ֻ������� */
    "AT+BLEGATTCINCLSRV",     /* GATTC ���ְ����ķ��� */
    "AT+BLEGATTCCHAR",        /* GATTC ���ַ������� */
    "AT+BLEGATTCRD",          /* GATTC ��ȡ��������ֵ */
    "AT+BLEGATTCWR",          /* GATTC д��������ֵ */
    "AT+BLESPPCFG",           /* ��ѯ/���� Bluetooth LE SPP ���� */
    "AT+BLESPP",              /* ���� Bluetooth LE SPP ģʽ */
    "AT+SAVETRANSLINK",       /* ���� Bluetooth LE ����͸��ģʽ ��Ϣ */
    "AT+BLESECPARAM",         /* ��ѯ/���� Bluetooth LE ���ܲ��� */
    "AT+BLEENC",              /* ���� Bluetooth LE �������� */
    "AT+BLEENCRSP",           /* �ظ��Զ��豸������������ */
    "AT+BLEKEYREPLY",         /* ���Է��豸�ظ���Կ */
    "AT+BLECONFREPLY",        /* ���Է��豸�ظ�ȷ�Ͻ������ͳ���ӽ׶Σ� */
    "AT+BLEENCDEV",           /* ��ѯ�󶨵� Bluetooth LE �����豸�б� */
    "AT+BLEENCCLEAR",         /* ��� Bluetooth LE �����豸�б� */
    "AT+BLESETKEY",           /* ����Bluetooth LE ��̬�����Կ */
    "AT+BLEHIDINIT",          /* Bluetooth LE HID Э���ʼ�� */
    "AT+BLEHIDKB",            /* ����Bluetooth LE HID ������Ϣ */
    "AT+BLEHIDMUS",           /* ���� Bluetooth LE HID �����Ϣ */
    "AT+BLEHIDCONSUMER",      /* ����Bluetooth LE HID consumer ��Ϣ */
    "AT+BLUFI",               /* ������ر� BluFi */
    "AT+BLUFINAME",           /* ��ѯ/���� BluFi �豸���� */
    "AT+BLUFISEND",           /* ���� BluFi �û��Զ������� */
    "AT+BLEPERIODICDATA",     /* ���� Bluetooth LE �����Թ㲥���� */
    "AT+BLEPERIODICSTART",    /* ���� Bluetooth LE �����Թ㲥 */
    "AT+BLEPERIODICSTOP",     /* ֹͣ Bluetooth LE �����Թ㲥 */
    "AT+BLESYNCSTART",        /* ���������Թ㲥ͬ�� */
    "AT+BLESYNCSTOP",         /* ֹͣ�����Թ㲥ͬ�� */
    "AT+BLEREADPHY",          /* ��ѯ��ǰ����ʹ�õ� PHY */
    "AT+BLESETPHY"            /* ���õ�ǰ����ʹ�õ� PHY */
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
