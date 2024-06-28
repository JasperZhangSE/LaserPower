/*
    Pwr2Prot.c

    Implementation File for Pwr2 Prot Module
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 20Nov23, Karl Created
*/

/* Includes */
#include "Include.h"

/* Debug config */
#if PWR_DEBUG
    #undef TRACE
    #define TRACE(...)  DebugPrintf(__VA_ARGS__)
#else
    #undef TRACE
    #define TRACE(...)
#endif /* PWR_DEBUG */
#if PWR_DEBUG
    #undef ASSERT
    #define ASSERT(a)   while(!(a)){DebugPrintf("ASSERT failed: %s %d\n", __FILE__, __LINE__);}
#else
    #undef ASSERT
    #define ASSERT(...)
#endif /* PWR_DEBUG */

/* Local defines */
#define PWR_ADDR                1
#define SEND_DELAY              5
#define ADDR_MASK               0x7F
#define ID_MASK                 0xFFFFFFF8
#define BYTE_ENDIAN_UINT16(a)   (((a) && 0xFF) << 8) | (((a) && 0xFF00) >> 8)
#define BYTE_ENDIAN_UINT32(a)   (((a) & 0xFF) << 24) | (((a) & 0xFF00) << 8) | (((a) & 0xFF0000) >> 8) | (((a) & 0xFF000000) >> 24)

/* Local types */
#pragma pack(push)
#pragma pack(1)
typedef struct {
    /* �������, �����ѹ, ״̬0, ״̬1 */
    uint16_t usOutCur;  /* 0.1A */
    uint16_t usOutVol;  /* 0.1V */
    uint8_t  ucStatus0;
    uint8_t  ucStatus1;
    
    /* �趨����, �趨��ѹ */
    uint16_t usCfgCur;  /* 0.1A */
    uint16_t usCfgVol;  /* 0.1V */
    
    /* �����ѹ(3��) */
    uint16_t usInVab;   /* 0.1V */
    uint16_t usInVbc;   /* 0.1V */
    uint16_t usInVca;   /* 0.1V */
    
    /* �����¶� */
    int16_t  sTemp;     /* 0.1��*/
}Data_t;

typedef struct {
    uint32_t ulId;
    uint8_t  ucData[8];
}Msg_t;
#pragma pack(pop)

/* Forward declarations */
/* GROUP - 1 */
/* ����趨��� */
static Status_t prvSetOutput(uint32_t ulCur, uint32_t ulVol);
/* ��ȡģ����Ϣ */
static Status_t prvGetStatus(void);
/* ����ģ�鿪�ػ� */
static Status_t prvSetOnOff(uint8_t ucOnOff);
/* GROUP - 2 */
/* ��ȡ����趨��ѹ */
static Status_t prvGetCfgVol(void);
/* ��ȡ����趨���� */
static Status_t prvGetCfgCur(void);
/* ��ȡ�����ѹ(3��) */
static Status_t prvGetInputVol(void);
/* ��ȡ�����¶� */
static Status_t prvGetEnvTemp(void);
/* GROUP - 3 */
/* ģ��������� */
static Status_t prvSetCurShare(uint8_t ucOnOff);
/* ģ���ַѰ�� */
static Status_t prvSearchModule(uint8_t ucAddr);

/* Local variables */
static Data_t        s_xData;
static QueueHandle_t s_xQueue;

/* Functions */
Status_t Pwr2ProtInit(void)
{
    memset(&s_xData, 0, sizeof(s_xData));
    s_xQueue = xQueueCreate(1/*Queue length*/, sizeof(Msg_t)/*Queue item size*/);
    return STATUS_OK;
}

Status_t Pwr2ProtTerm(void)
{
    /* Do nothing */
    return STATUS_OK;
}

void Pwr1CanRxNotify(CanMsgRx_t *pxMsg)
{
    if (pxMsg->IDE != CAN_ID_EXT) {
        return;
    }
    
    if ((pxMsg->ExtId & ADDR_MASK) != PWR_ADDR) {
        return;
    }
    
    if (pxMsg->DLC != 8) {
        return;
    }
    
    Bool_t bNotify = FALSE;
    switch(pxMsg->ExtId & ID_MASK) {
    case 0x1807C080:
        switch (pxMsg->Data[0]) {
        case 0:     /* ģ�鷵����� */
        case 1:     /* ģ�鷵��״̬ */
        case 2:     /* ģ��ظ����ػ� */
            bNotify = TRUE;
            break;
        default:
            ASSERT(0);
            break;
        }
        break;
    
    
    case 0x18010080:    /* ģ�鷵�ص�ѹֵ */
    case 0x18010880:    /* ģ�鷵������ֵ */
    case 0x1807A080:    /* ģ�鷵�������ѹ */
    case 0x18008080:    /* ģ�鷵�ػ����¶� */
        bNotify = TRUE;
        break;
    
    default:
        break;
    }
    
    if (bNotify) {
        Msg_t m;
        m.ulId = pxMsg->ExtId;
        memcpy(m.ucData, pxMsg->Data, 8);
        
        BaseType_t xHigherPriorityTaskWoken;
        xQueueSendToBackFromISR(s_xQueue, &m, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

/* ����趨��� */
static Status_t prvSetOutput(uint32_t ulCur, uint32_t ulVol)
{
    CanMsgTx_t xMsg;
    xMsg.StdId   = 0;
    xMsg.ExtId   = 0x1907C080 | PWR_ADDR;
    xMsg.IDE     = CAN_ID_EXT;
    xMsg.RTR     = CAN_RTR_DATA;
    xMsg.DLC     = 8;
    /* CMD */
    xMsg.Data[0] = 0;
    /* ����(mA) */
    xMsg.Data[1] = (ulCur & 0xFF0000) >> 16;
    xMsg.Data[2] = (ulCur & 0xFF00) >> 8;
    xMsg.Data[3] = (ulCur & 0xFF) >> 0;
    /* ��ѹ(mV) */
    xMsg.Data[4] = (ulVol & 0xFF000000) >> 24;
    xMsg.Data[5] = (ulVol & 0xFF0000) >> 16;
    xMsg.Data[6] = (ulVol & 0xFF00) >> 8;
    xMsg.Data[7] = (ulVol & 0xFF) >> 0;
    CanSend(&xMsg, SEND_DELAY);
    
    Msg_t m;
    if(xQueueReceive(s_xQueue, &m, pdMS_TO_TICKS(10)) == pdPASS) {
        if (((m.ulId & ADDR_MASK) == 0x1807C080) && (m.ucData[0] == 0) && (m.ucData[1] == 0xFF)) {
            return STATUS_OK;
        }
    }
    
    return STATUS_ERR;
}

/* ��ȡģ����Ϣ */
static Status_t prvGetStatus(void)
{
    CanMsgTx_t xMsg;
    xMsg.StdId   = 0;
    xMsg.ExtId   = 0x1907C080 | PWR_ADDR;
    xMsg.IDE     = CAN_ID_EXT;
    xMsg.RTR     = CAN_RTR_DATA;
    xMsg.DLC     = 8;
    /* CMD */
    xMsg.Data[0] = 1;
    /* NOUSE */
    xMsg.Data[1] = 0;
    xMsg.Data[2] = 0;
    xMsg.Data[3] = 0;
    xMsg.Data[4] = 0;
    xMsg.Data[5] = 0;
    xMsg.Data[6] = 0;
    xMsg.Data[7] = 0;
    CanSend(&xMsg, SEND_DELAY);
    
    Msg_t m;
    if(xQueueReceive(s_xQueue, &m, pdMS_TO_TICKS(10)) == pdPASS) {
        if (((m.ulId & ADDR_MASK) == 0x1807C080) && (m.ucData[0] == 1)) {
            s_xData.usOutCur  = ((uint16_t)m.ucData[2] << 8) | (uint16_t)m.ucData[3];
            s_xData.usOutVol  = ((uint16_t)m.ucData[4] << 8) | (uint16_t)m.ucData[5];
            s_xData.ucStatus1 = m.ucData[6];
            s_xData.ucStatus0 = m.ucData[7];
            return STATUS_OK;
        }
    }
    
    return STATUS_ERR;
}

/* ����ģ�鿪�ػ� */
static Status_t prvSetOnOff(uint8_t ucOnOff)
{
    CanMsgTx_t xMsg;
    xMsg.StdId   = 0;
    xMsg.ExtId   = 0x1907C080 | PWR_ADDR;
    xMsg.IDE     = CAN_ID_EXT;
    xMsg.RTR     = CAN_RTR_DATA;
    xMsg.DLC     = 8;
    /* CMD */
    xMsg.Data[0] = 2;
    /* NOUSE */
    xMsg.Data[1] = 0;
    xMsg.Data[2] = 0;
    xMsg.Data[3] = 0;
    xMsg.Data[4] = 0;
    xMsg.Data[5] = 0;
    xMsg.Data[6] = 0;
    xMsg.Data[7] = ucOnOff ? 0x55 : 0xAA;
    CanSend(&xMsg, SEND_DELAY);
    
    Msg_t m;
    if(xQueueReceive(s_xQueue, &m, pdMS_TO_TICKS(10)) == pdPASS) {
        if (((m.ulId & ADDR_MASK) == 0x1807C080) && (m.ucData[0] == 2) && (m.ucData[1] == 0xFF)) {
            return STATUS_OK;
        }
    }
    
    return STATUS_ERR;
}

/* ��ȡ����趨��ѹ */
static Status_t prvGetCfgVol(void)
{
    CanMsgTx_t xMsg;
    xMsg.StdId   = 0;
    xMsg.ExtId   = 0x19010080 | PWR_ADDR;
    xMsg.IDE     = CAN_ID_EXT;
    xMsg.RTR     = CAN_RTR_DATA;
    xMsg.DLC     = 0;
    CanSend(&xMsg, SEND_DELAY);
    
    Msg_t m;
    if(xQueueReceive(s_xQueue, &m, pdMS_TO_TICKS(10)) == pdPASS) {
        if ((m.ulId & ADDR_MASK) == 0x18010080) {
            s_xData.usCfgVol = ((uint16_t)m.ucData[6] << 8) | (uint16_t)m.ucData[7];
            return STATUS_OK;
        }
    }
    
    return STATUS_ERR;
}

/* ��ȡ����趨���� */
static Status_t prvGetCfgCur(void)
{
    CanMsgTx_t xMsg;
    xMsg.StdId   = 0;
    xMsg.ExtId   = 0x19010880 | PWR_ADDR;
    xMsg.IDE     = CAN_ID_EXT;
    xMsg.RTR     = CAN_RTR_DATA;
    xMsg.DLC     = 0;
    CanSend(&xMsg, SEND_DELAY);
    
    Msg_t m;
    if(xQueueReceive(s_xQueue, &m, pdMS_TO_TICKS(10)) == pdPASS) {
        if ((m.ulId & ADDR_MASK) == 0x18010880) {
            s_xData.usCfgCur = ((uint16_t)m.ucData[0] << 8) | (uint16_t)m.ucData[1];
            return STATUS_OK;
        }
    }
    
    return STATUS_ERR;
}

/* ��ȡ�����ѹֵ */
static Status_t prvGetInputVol(void)
{
    CanMsgTx_t xMsg;
    xMsg.StdId   = 0;
    xMsg.ExtId   = 0x1907A080 | PWR_ADDR;
    xMsg.IDE     = CAN_ID_EXT;
    xMsg.RTR     = CAN_RTR_DATA;
    xMsg.DLC     = 8;
    /* CMD */
    xMsg.Data[0] = 0x31;
    /* NOUSE */
    xMsg.Data[1] = 0;
    xMsg.Data[2] = 0;
    xMsg.Data[3] = 0;
    xMsg.Data[4] = 0;
    xMsg.Data[5] = 0;
    xMsg.Data[6] = 0;
    xMsg.Data[7] = 0;
    CanSend(&xMsg, SEND_DELAY);
    
    Msg_t m;
    if(xQueueReceive(s_xQueue, &m, pdMS_TO_TICKS(10)) == pdPASS) {
        if (((m.ulId & ADDR_MASK) == 0x1807A080) && (m.ucData[0] == 0x31)) {
            s_xData.usInVab = ((uint16_t)m.ucData[2] << 8) | (uint16_t)m.ucData[3];
            s_xData.usInVbc = ((uint16_t)m.ucData[4] << 8) | (uint16_t)m.ucData[5];
            s_xData.usInVca = ((uint16_t)m.ucData[6] << 8) | (uint16_t)m.ucData[7];
            return STATUS_OK;
        }
    }
    
    return STATUS_ERR;
}

/* ��ȡ�����¶� */
static Status_t prvGetEnvTemp(void)
{
    CanMsgTx_t xMsg;
    xMsg.StdId   = 0;
    xMsg.ExtId   = 0x19008080 | PWR_ADDR;
    xMsg.IDE     = CAN_ID_EXT;
    xMsg.RTR     = CAN_RTR_DATA;
    xMsg.DLC     = 0;
    CanSend(&xMsg, SEND_DELAY);
    
    Msg_t m;
    if(xQueueReceive(s_xQueue, &m, pdMS_TO_TICKS(10)) == pdPASS) {
        if ((m.ulId & ADDR_MASK) == 0x18008080) {
            s_xData.sTemp = ((uint16_t)m.ucData[4] << 8) | (uint16_t)m.ucData[5];
            return STATUS_OK;
        }
    }
    
    return STATUS_ERR;
}

/* ģ��������� */
static Status_t prvSetCurShare(uint8_t ucOnOff)
{
    CanMsgTx_t xMsg;
    xMsg.StdId   = 0;
    xMsg.ExtId   = 0x19C21880 | 0;
    xMsg.IDE     = CAN_ID_EXT;
    xMsg.RTR     = CAN_RTR_DATA;
    xMsg.DLC     = 6;
    xMsg.Data[0] = 0;
    xMsg.Data[1] = 0;
    xMsg.Data[2] = 0;
    xMsg.Data[3] = ucOnOff ? 0xAA : 0x55;
    xMsg.Data[4] = 0;
    xMsg.Data[5] = 0;
    CanSend(&xMsg, SEND_DELAY);
    
    return STATUS_OK;
}

/* ģ���ַѰ�� */
static Status_t prvSearchModule(uint8_t ucAddr)
{
    CanMsgTx_t xMsg;
    xMsg.StdId   = 0;
    xMsg.ExtId   = 0x19C22880 | ucAddr;
    xMsg.IDE     = CAN_ID_EXT;
    xMsg.RTR     = CAN_RTR_DATA;
    xMsg.DLC     = 8;
    xMsg.Data[0] = 0;
    xMsg.Data[1] = 0;
    xMsg.Data[2] = 0;
    xMsg.Data[3] = 0x55;
    xMsg.Data[4] = 0;
    xMsg.Data[5] = 0;
    xMsg.Data[6] = 0;
    xMsg.Data[7] = 0;
    CanSend(&xMsg, SEND_DELAY);
    
    return STATUS_OK;
}

static void prvCliCmdPwr2SetCurVol(cli_printf cliprintf, int argc, char** argv)
{
    if (argc != 3) {
        cliprintf("pwr2_set_cur_vol CUR VOL\n");
    }
    
    float fC = atof(argv[1]);
    float fV = atof(argv[2]);
    if (STATUS_OK == prvSetOutput(fC * 1000, fV * 1000)) {
        cliprintf("    ok\n");
    }
    else {
        cliprintf("    err\n");
    }
}
CLI_CMD_EXPORT(pwr2_set_cur_vol, set power(9.5kw) voltage, prvCliCmdPwr2SetCurVol)

static void prvCliCmdPwr2CtrlOn(cli_printf cliprintf, int argc, char** argv)
{
    if (STATUS_OK == prvSetOnOff(1)) {
        cliprintf("    ok\n");
    }
    else {
        cliprintf("    err\n");
    }
}
CLI_CMD_EXPORT(pwr2_ctrl_on, set power(18kw) output on, prvCliCmdPwr2CtrlOn)

static void prvCliCmdPwr2CtrlOff(cli_printf cliprintf, int argc, char** argv)
{
    if (STATUS_OK == prvSetOnOff(0)) {
        cliprintf("    ok\n");
    }
    else {
        cliprintf("    err\n");
    }
}
CLI_CMD_EXPORT(pwr2_ctrl_off, set power(18kw) output off, prvCliCmdPwr2CtrlOff)

static void prvCliCmdPwr2Status(cli_printf cliprintf, int argc, char** argv)
{
    cliprintf("�������     : %.1f A\n", s_xData.usOutCur * 0.1);
    cliprintf("�����ѹ     : %.1f V\n", s_xData.usOutVol * 0.1);
    cliprintf("״̬1        : 0x%02X\n", s_xData.ucStatus1);
    cliprintf("״̬0        : 0x%02X\n", s_xData.ucStatus0);
    
    cliprintf("�趨����     : %.1f A\n", s_xData.usCfgCur * 0.1);
    cliprintf("�趨��ѹ     : %.1f V\n", s_xData.usCfgVol * 0.1);
    
    cliprintf("�����ѹ(Vab): %.1f V\n", s_xData.usInVab * 0.1);
    cliprintf("�����ѹ(Vbc): %.1f V\n", s_xData.usInVbc * 0.1);
    cliprintf("�����ѹ(Vca): %.1f V\n", s_xData.usInVca * 0.1);

    cliprintf("�����¶�     : %.1f ��\n", s_xData.sTemp * 0.1);
}
CLI_CMD_EXPORT(pwr2_status, show power(18kw) status, prvCliCmdPwr2Status)
