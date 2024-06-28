/* Includes */
#include "config.h"
#include "STC8G_H_ADC.h"
#include "STC8G_H_GPIO.h"
#include "STC8G_H_Delay.h"
#include "STC8G_H_UART.h"
#include "STC8G_H_NVIC.h"
#include "STC8G_H_Switch.h"
#include "STC8G_H_Timer.h"
#include "STC8G_H_WDT.h"
#include "TempTbl.h"

/* Debug configuration */
/* XXX: Please redefine UART1 in STC8G_H_UART.h */
#define DEBUG 0

#if DEBUG
#define TRACE printf
#else
#define TRACE
#endif /* DEBUG */

/* Local defines */
/* Work mode */
#define WORK_MODE       1       /* 0: Standby; 1: Normal; */
/* Device ID */
#define DEV_ID          1       /* 1/2/3 */
/* Temp info type */
#define TEMP_INFO_TYPE  0       /* 0: Raw adc data; 1: Temperature; */
/* Software version  */
#define SW_VER1         1
#define SW_VER2         0
#define SW_VER3         0
/* Timer frequency */
#define TIMER_FREQ      1000    /* XXX: Check it if accurate <MAIN_Fosc> */
/* Com buffer size */
#define COM_RX_SIZE     64
/* Adc channel number */
#define CHAN_NUM        10
/* RS485 rw config */
#define RS485_RD()      P36 = 1
#define RS485_WT()      P36 = 0

/* Local types */
typedef signed char     int8_t;
typedef signed int      int16_t;
typedef signed long     int32_t;
typedef unsigned char   uint8_t;
typedef unsigned int    uint16_t;
typedef unsigned long   uint32_t;

enum {
    TS_1  = 2,
    TS_2  = 3,
    TS_3  = 4,
    TS_4  = 5,
    TS_5  = 6,
    TS_6  = 7,
    TS_7  = 13,
    TS_8  = 12,
    TS_9  = 11,
    TS_10 = 10,
};

enum {
    iCmdQueryInfo = 0x01,
    iCmdSysReset  = 0x02,
    rCmdTempInfo  = 0x81,
    rCmdDiagInfo  = 0x82,
};

typedef struct {
    uint16_t adc;
    int16_t  temp;
}TempTblItem_t;

/* Forward declarations */
/* Config */
static void WdgConfig(void);
static void GpioConfig(void);
static void AdcConfig(void);
static void UartConfig(void);
static void TimerConfig(void);
/* Proc */
static void AdcProc(void);
static void UartProc(void);
static void ProcCmdQueryInfo(uint8_t cmd);
static void ProcCmdSysReset(uint8_t mark[4]);
/* Help */
static int16_t GetTemp(uint16_t adc);
static void SendTempInfo(void);
static void SendDiagInfo(void);
/* Timer */
void TimerProc(void);

/* Global variables */
uint8_t g_bAdSmp = 1;
uint8_t g_bComRx = 0;
uint32_t g_ulRunTime = 0;

/* Local variables */
static int16_t s_sTemp[CHAN_NUM];
static uint32_t s_ulAdcErrCnt = 0;

/* Functions */
void main(void)
{
    EAXSFR();
    WdgConfig();
    GpioConfig();
#if (WORK_MODE == 1)
    AdcConfig();
    UartConfig();
    TimerConfig();
    EA = 1;
#endif /* (WORK_MODE == 1) */
    
    TRACE("STC starts\r\n");
    while (1) {
    #if (WORK_MODE == 1)
        AdcProc();
        UartProc();
    #endif /* (WORK_MODE == 1) */
        WDT_Clear();
    }
}

static void WdgConfig(void)
{
    WDT_InitTypeDef init;
    init.WDT_Enable    = ENABLE;
    init.WDT_IDLE_Mode = WDT_IDLE_STOP;
    init.WDT_PS        = WDT_SCALE_16;  /* about 0.4s */
    WDT_Inilize(&init);
}

static void GpioConfig(void)
{
    /* GPIO defines:
     * TS_1    P1.2    ADC2
     * TS_2    P1.3    ADC3
     * TS_3    P1.4    ADC4
     * TS_4    P1.5    ADC5
     * TS_5    P1.6    ADC6
     * TS_6    P1.7    ADC7
     * TS_7    P3.5    ADC13
     * TS_8    P3.4    ADC12
     * TS_9    P3.3    ADC11
     * TS_10   P3.2    ADC10
     * ADC_RX  P1.0    RXD2
     * ADC_TX  P1.1    TXD2
     * ADC_EN  P3.6    GPIO  */
    GPIO_InitTypeDef init;
    /* ADC */
    init.Pin  = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
    init.Mode = GPIO_HighZ;
    GPIO_Inilize(GPIO_P1, &init);
    init.Pin  = GPIO_Pin_5|GPIO_Pin_4|GPIO_Pin_3|GPIO_Pin_2;
    init.Mode = GPIO_HighZ;
    GPIO_Inilize(GPIO_P3, &init);
    /* RS485 */
    init.Pin  = GPIO_Pin_0|GPIO_Pin_1;
    init.Mode = GPIO_PullUp;
    GPIO_Inilize(GPIO_P1, &init);
    /* RS485 RW enable */
    init.Pin  = GPIO_Pin_6;
    init.Mode = GPIO_OUT_PP;
    GPIO_Inilize(GPIO_P3, &init);
#if DEBUG
    init.Pin  = GPIO_Pin_0 | GPIO_Pin_1;
    init.Mode = GPIO_PullUp;
    GPIO_Inilize(GPIO_P3,&init);
#endif /* DEBUG */
    
    /* Enable RS485 read */
    RS485_RD();
}

static void AdcConfig(void)
{
    ADC_InitTypeDef init;
    init.ADC_SMPduty   = 31;
    init.ADC_CsSetup   = 0;
    init.ADC_CsHold    = 1;
    init.ADC_Speed     = ADC_SPEED_2X16T;
    init.ADC_AdjResult = ADC_RIGHT_JUSTIFIED;
    ADC_Inilize(&init);
    ADC_PowerControl(ENABLE);
    NVIC_ADC_Init(DISABLE, Priority_0);
}

static void UartConfig(void)
{
    /* UART2: 115200, N, 8, 1 */
    COMx_InitDefine init;
    init.UART_Mode     = UART_8bit_BRTx;
    init.UART_BRT_Use  = BRT_Timer2;
    init.UART_BaudRate = 115200ul;
    init.UART_RxEnable = ENABLE;
    UART_Configuration(UART2, &init);
    NVIC_UART2_Init(ENABLE, Priority_1);
    UART2_SW(UART2_SW_P10_P11);
    
#if DEBUG
    init.UART_Mode      = UART_8bit_BRTx;
    init.UART_BRT_Use   = BRT_Timer1;
    init.UART_BaudRate  = 115200ul;
    init.UART_RxEnable  = ENABLE;
    init.BaudRateDouble = DISABLE;
    UART_Configuration(UART1, &init);
    NVIC_UART1_Init(ENABLE,Priority_1);
    UART1_SW(UART1_SW_P30_P31);
#endif /* DEBUG */
}

static void TimerConfig(void)
{
    TIM_InitTypeDef init;

    init.TIM_Mode      = TIM_16BitAutoReload;
    init.TIM_ClkSource = TIM_CLOCK_1T;
    init.TIM_ClkOut    = DISABLE;
    init.TIM_Value     = 65536UL - (MAIN_Fosc / TIMER_FREQ);
    init.TIM_Run       = ENABLE;
    Timer_Inilize(Timer0, &init);
    NVIC_Timer0_Init(ENABLE, Priority_0);
}

static void AdcProc(void)
{
    if (g_bAdSmp) {
        uint8_t n;
        static uint8_t c[CHAN_NUM] = {TS_1, TS_2, TS_3, TS_4, TS_5, TS_6, TS_7, TS_8, TS_9, TS_10};
        for (n = 0; n < CHAN_NUM; n++) {
            uint16_t adc = Get_ADCResult(c[n]);
            if (4096 == adc) {
                s_ulAdcErrCnt++;
            }
        #if (TEMP_INFO_TYPE == 0)
            s_sTemp[n] = adc;
        #endif /* (TEMP_INFO_TYPE == 0) */
        #if (TEMP_INFO_TYPE == 1)
            s_sTemp[n] = GetTemp(adc);
        #endif /* (TEMP_INFO_TYPE == 1) */
        }
        g_bAdSmp = 0;
    }
}

static void UartProc(void)
{
    static uint8_t b[COM_RX_SIZE];
    static uint8_t i = 0;
    
    if(g_bComRx && (COM2.RX_Cnt > 0)) {
        uint8_t n;
        for (n = 0; n < COM2.RX_Cnt; n++) {
            uint8_t d = RX2_Buffer[n];
            if (i > COM_RX_SIZE) {
                i = 0;
            }
            b[i++] = d;
            switch (i) {
            case 1: /* Head1 */
                if (d != 0x7E) {
                    TRACE("Com err: head 1\n");
                    i = 0;
                }
                break;
            case 2: /* Head2 */
                if (d != 0x7E) {
                    TRACE("Com err: head 2\n");
                    i = 0;
                }
                break;
            case 3: /* Length */
                if (d > COM_RX_SIZE) {
                    TRACE("Com err: len\n");
                    i = 0;
                }
                break;
            case 4: /* Command */
                /* Do nothing */
                break;
            case 5: /* Source address */
                if (d != 0) {
                    TRACE("Com err: src addr\n");
                    i = 0;
                }
                break;
            case 6: /* Destination address */
                if (d != DEV_ID) {
                    TRACE("Com err: dst addr\n");
                    i = 0;
                }
                break;
            default:
                /* Get whole packet */
                if (i == b[2]) {
                    uint8_t m = 0, sum = 0;
                    /* Check tail */
                    if (((b[i-2] != 0x0A) && (b[i-2] != 0x0D)) || ((b[i-1] != 0x0A) && (b[i-1] != 0x0D))) {
                        TRACE("Com err: tail\n");
                        i = 0;
                        break;
                    }
                    /* Check sum */
                    for (m = 0; m < i; m++) {
                        sum += b[m];
                    }
                    if (sum != 0xFF) {
                        TRACE("Com err: chk sum\n");
                        i = 0;
                        break;
                    }
                    /* Process data */
                    switch (b[3]) {
                    case iCmdQueryInfo:
                        TRACE("Com ok: query info\n");
                        if ((9 + 1) == b[2]) {
                            ProcCmdQueryInfo(b[6]);
                        }
                        else {
                            TRACE("    wrong length\n");
                        }
                        break;
                    case iCmdSysReset:
                        TRACE("Com ok: sys reset\n");
                        if ((9 + 4) == b[2]) {
                            ProcCmdSysReset(b + 6);
                        }
                        else {
                            TRACE("    wrong length\n");
                        }
                        break;
                    default:
                        TRACE("Com ok: unknown cmd\n");
                        break;
                    }
                    i = 0;
                }
                break;
            }
        }
        g_bComRx = 0;
        COM2.RX_Cnt = 0;
    }
}

static void ProcCmdQueryInfo(uint8_t cmd)
{
    switch (cmd) {
    case rCmdTempInfo:
        TRACE("    Temp info\n");
        SendTempInfo();
        break;
    case rCmdDiagInfo:
        TRACE("    Diag info\n");
        SendDiagInfo();
        break;
    default:
        TRACE("    Unknown info\n");
        break;
    }
}

static void ProcCmdSysReset(uint8_t mark[4])
{
    if ((0xCD == mark[0]) && ((0xAB == mark[1])) && (0x34 == mark[2]) && ((0x12 == mark[3]))) {
        /* Wait wdog reset */
        TRACE("    Reset\n");
        while(1);
    }
    else {
        TRACE("    Wrong mark\n");
    }
}

static int16_t GetTemp(uint16_t adc)
{
    static TempTblItem_t T[TEMP_TBL_SIZE] = TEMP_TBL_CONT;

    uint8_t  ok = 0;
    uint16_t st = 0;
    uint16_t ed = 0;
    uint16_t n = 0;
    int16_t  temp;
    
    if ((T[0].adc > adc) && (adc > T[TEMP_TBL_SIZE - 1].adc)) {
        for (n = 0; n < (TEMP_TBL_SIZE - 1); n++) {
            if (adc >= T[n + 1].adc) {
                ok = 1;
                st = n;
                ed = n + 1;
                break;
            }
        }
    }

    if (ok) {
        /* Linear interpolation */
        /* y = (x - x1) * (y2 - y1) / (x2 - x1) + y1 */
        temp = (adc - T[st].adc) * (T[ed].temp - T[st].temp) / (T[ed].adc - T[st].adc) + T[st].temp;
    }
    else {
        if (adc > T[0].adc) {
            TRACE("Temp: under range\n");
            temp = T[0].temp;
        }
        else if (adc < T[TEMP_TBL_SIZE - 1].adc) {
            TRACE("Temp: over range\n");
            temp = T[TEMP_TBL_SIZE - 1].temp;
        }
    }

    return temp;
}

static void SendTempInfo(void)
{
    #define LEN1 29
    static uint8_t c[LEN1];
    uint8_t i = 0, n = 0, sum = 0;
    
    /* Enable RS485 write */
    RS485_WT();
    
    /* Head */
    c[i++] = 0x7E;
    c[i++] = 0x7E;
    c[i++] = LEN1;
    c[i++] = rCmdTempInfo;
    c[i++] = DEV_ID;
    c[i++] = 0;
    /* Content */
    c[i++] = s_sTemp[0] & 0xFF;
    c[i++] = (s_sTemp[0] >> 8) & 0xFF;
    c[i++] = s_sTemp[1] & 0xFF;
    c[i++] = (s_sTemp[1] >> 8) & 0xFF;
    c[i++] = s_sTemp[2] & 0xFF;
    c[i++] = (s_sTemp[2] >> 8) & 0xFF;
    c[i++] = s_sTemp[3] & 0xFF;
    c[i++] = (s_sTemp[3] >> 8) & 0xFF;
    c[i++] = s_sTemp[4] & 0xFF;
    c[i++] = (s_sTemp[4] >> 8) & 0xFF;
    c[i++] = s_sTemp[5] & 0xFF;
    c[i++] = (s_sTemp[5] >> 8) & 0xFF;
    c[i++] = s_sTemp[6] & 0xFF;
    c[i++] = (s_sTemp[6] >> 8) & 0xFF;
    c[i++] = s_sTemp[7] & 0xFF;
    c[i++] = (s_sTemp[7] >> 8) & 0xFF;
    c[i++] = s_sTemp[8] & 0xFF;
    c[i++] = (s_sTemp[8] >> 8) & 0xFF;
    c[i++] = s_sTemp[9] & 0xFF;
    c[i++] = (s_sTemp[9] >> 8) & 0xFF;
    /* Tail */
    c[i++] = 0;
    c[i++] = 0x0A;
    c[i++] = 0x0D;
    for (n = 0; n < LEN1; n++) {
        sum += c[n];
    }
    sum = 0xFF - sum;
    c[LEN1-3] = sum;
    
    /* Send */
    for (n = 0; n < LEN1; n++) {
        TX2_write2buff(c[n]);
    }
    
    /* Enable RS485 read */
    RS485_RD();
}

static void SendDiagInfo(void)
{
    #define LEN2 21
    static uint8_t c[LEN2];
    uint8_t i = 0, n = 0, sum = 0;
    
    /* Enable RS485 write */
    RS485_WT();
    
    /* Head */
    c[i++] = 0x7E;
    c[i++] = 0x7E;
    c[i++] = LEN2;
    c[i++] = rCmdDiagInfo;
    c[i++] = DEV_ID;
    c[i++] = 0;
    /* Content */
    c[i++] = SW_VER3;
    c[i++] = SW_VER2;
    c[i++] = SW_VER1;
    c[i++] = 0xFF;
    c[i++] = g_ulRunTime & 0xFF;
    c[i++] = (g_ulRunTime >> 8) & 0xFF;
    c[i++] = (g_ulRunTime >> 16) & 0xFF;
    c[i++] = (g_ulRunTime >> 24) & 0xFF;
    c[i++] = s_ulAdcErrCnt & 0xFF;
    c[i++] = (s_ulAdcErrCnt >> 8) & 0xFF;
    c[i++] = (s_ulAdcErrCnt >> 16) & 0xFF;
    c[i++] = (s_ulAdcErrCnt >> 24) & 0xFF;
    /* Tail */
    c[i++] = 0;
    c[i++] = 0x0A;
    c[i++] = 0x0D;
    for (n = 0; n < LEN2; n++) {
        sum += c[n];
    }
    sum = 0xFF - sum;
    c[LEN2-3] = sum;
    
    /* Send */
    for (n = 0; n < LEN2; n++) {
        TX2_write2buff(c[n]);
    }
    
    /* Enable RS485 read */
    RS485_RD();
}

void TimerProc(void)
{
    static uint16_t n = 0;
    
    n++;
    
    /* 1ms */
    if(COM2.RX_TimeOut > 0) {
        if(--COM2.RX_TimeOut == 0) {
            g_bComRx = 1;
        }
    }
    
    /* 10ms */
    if (0 == (n % 10)) {
        g_bAdSmp = 1;
    }
    
    /* 1s */
    if (0 == (n % 1000)) {
        g_ulRunTime++;
    }
}
