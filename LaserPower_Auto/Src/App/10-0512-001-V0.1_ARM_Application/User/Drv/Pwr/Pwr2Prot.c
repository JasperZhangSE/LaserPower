/*
    Pwr2Prot.c

    Implementation File for Pwr2 Prot Module
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 20Nov23, Karl Created
    01b, 07Dec23, Karl Added Pwr2Output function
    01c, 27Dec23, Karl Added Pwr2DataGet
    01d, 08Jan24, Karl Added pwr2_set_vol_def
    01e, 17Jan24, Karl Added Pwr2SetVolDef
    01f, 20Jan24, Karl Added PWR_STATUS
    01g, 27Jun24, Jasper Added Three-machine parallel operation.
*/

/* Includes */
#include "Include.h"

/* Pragmas */
#pragma diag_suppress 177   /* warning: #177-D: function "FUNC" was set but never used */

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
//#define PWR_ADDR                1
#define SEND_DELAY              5
#define ADDR_MASK               0x7F
#define ID_MASK                 0xFFFFFFF8
#define BYTE_ENDIAN_UINT16(a)   (((a) && 0xFF) << 8) | (((a) && 0xFF00) >> 8)
#define BYTE_ENDIAN_UINT32(a)   (((a) & 0xFF) << 24) | (((a) & 0xFF00) << 8) | (((a) & 0xFF0000) >> 8) | (((a) & 0xFF000000) >> 24)
#define WAIT_MS                 50

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
    uint16_t usInVab;   /* 1/32V */
    uint16_t usInVbc;   /* 1/32V */
    uint16_t usInVca;   /* 1/32V */
    
    /* �����¶� */
    int16_t  sTemp;     /* 0.1��*/
}_Data_t;

typedef struct {
    uint32_t ulId;
    uint8_t  ucData[8];
}Msg_t;
#pragma pack(pop)

/* Forward declarations */
/* GROUP - 1 */
/* ����趨��� */
static Status_t prvSetOutput(uint32_t ulPwr2Addr, uint32_t ulCur, uint32_t ulVol);
/* ��ȡģ����Ϣ */
static Status_t prvGetStatus(uint32_t ulPwr2Addr);
/* ����ģ�鿪�ػ� */
static Status_t prvSetOnOff(uint32_t ulPwr2Addr, uint8_t ucOnOff);
/* GROUP - 2 */
/* ��ȡ����趨��ѹ */
static Status_t prvGetCfgVol(uint32_t ulPwr2Addr);
/* ��ȡ����趨���� */
static Status_t prvGetCfgCur(uint32_t ulPwr2Addr);
/* ��ȡ�����ѹ(3��) */
static Status_t prvGetInputVol(uint32_t ulPwr2Addr);
/* ��ȡ�����¶� */
static Status_t prvGetEnvTemp(uint32_t ulPwr2Addr);
/* GROUP - 3 */
/* ģ��������� */
static Status_t prvSetCurShare(uint8_t ucOnOff);
/* ģ���ַѰ�� */
static Status_t prvSearchModule(uint8_t ucAddr);

/* Local variables */
static bool          s_bInit = false;

//static _Data_t       s_xData;
static _Data_t       s_T_xData;
static _Data_t       s_M_xData;
static _Data_t       s_B_xData;

static QueueHandle_t s_xQueue;

/* Functions */
Status_t Pwr2ProtInit(void)
{
    if (!s_bInit) {
        memset(&s_T_xData, 0, sizeof(s_T_xData));
        memset(&s_M_xData, 0, sizeof(s_M_xData));
        memset(&s_B_xData, 0, sizeof(s_B_xData));
        s_xQueue = xQueueCreate(1/*Queue length*/, sizeof(Msg_t)/*Queue item size*/);
        s_bInit = true;
    }
    return STATUS_OK;
}

Status_t Pwr2ProtTerm(void)
{
    /* Do nothing */
    return STATUS_OK;
}

Status_t Pwr2Update(void)
{

#if 1
    prvGetStatus(PWR_T_ADDR);
    prvGetCfgVol(PWR_T_ADDR);
    prvGetCfgCur(PWR_T_ADDR);
    prvGetInputVol(PWR_T_ADDR);
    prvGetEnvTemp(PWR_T_ADDR);
#endif

#if 1
    prvGetStatus(PWR_M_ADDR);
    prvGetCfgVol(PWR_M_ADDR);
    prvGetCfgCur(PWR_M_ADDR);
    prvGetInputVol(PWR_M_ADDR);
    prvGetEnvTemp(PWR_M_ADDR);
#endif

    
#if 1
    prvGetStatus(PWR_B_ADDR);
    prvGetCfgVol(PWR_B_ADDR);
    prvGetCfgCur(PWR_B_ADDR);
    prvGetInputVol(PWR_B_ADDR);
    prvGetEnvTemp(PWR_B_ADDR);
#endif
    
    return STATUS_OK;
}

void Pwr2CanRxNotify(uint32_t ulPwr2Addr, CanMsgRx_t *pxMsg)
{
    if (pxMsg->IDE != CAN_ID_EXT) {
        return;
    }
    
    if ((pxMsg->ExtId & ADDR_MASK) != ulPwr2Addr) {
        return;
    }
    
    /* "��ȡ����趨����"���صĳ���Ϊ6, �����Ϊ8 */
    if ((pxMsg->DLC != 8) && (pxMsg->DLC != 6)) {
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

Status_t Pwr2Output(uint32_t ulPwr2Addr, uint8_t ucOnOff)
{
    return prvSetOnOff(ulPwr2Addr, ucOnOff);
}

int32_t Pwr2DataGet(uint32_t ulAddr, PwrDataType_t xType)
{
    int32_t r = 0;
    switch (ulAddr)
    {
    case PWR_T_ADDR: 
        switch (xType) {
            case PWR_OUTPUT_VOL:
                r = s_T_xData.usOutVol; /* 0.1V */
                break;
            case PWR_STATUS:
                r = (s_T_xData.ucStatus1 << 8) | s_T_xData.ucStatus0;
                break;
            default:
                r = 0;
                break;
        }break;
    case PWR_M_ADDR: 
        switch (xType) {
            case PWR_OUTPUT_VOL:
                r = s_M_xData.usOutVol; /* 0.1V */
                break;
            case PWR_STATUS:
                r = (s_M_xData.ucStatus1 << 8) | s_M_xData.ucStatus0;
                break;
            default:
                r = 0;
                break;
        }break;
    case PWR_B_ADDR: 
        switch (xType) {
            case PWR_OUTPUT_VOL:
                r = s_B_xData.usOutVol; /* 0.1V */
                break;
            case PWR_STATUS:
                r = (s_B_xData.ucStatus1 << 8) | s_B_xData.ucStatus0;
                break;
            default:
                r = 0;
                break;
        }break;            
    }
    
    return r;
}

Status_t Pwr2SetVolDef(float fV)
{
    uint32_t ulV = (uint32_t)(fV * 10);
    
    CanMsgTx_t xMsg;
    xMsg.StdId   = 0;
    xMsg.ExtId   = 0x19420080;
    xMsg.IDE     = CAN_ID_EXT;
    xMsg.RTR     = CAN_RTR_DATA;
    xMsg.DLC     = 8;
    /* ����(0.1A) */
    xMsg.Data[0] = 0;
    xMsg.Data[1] = 0;
    xMsg.Data[2] = 0;
    xMsg.Data[3] = 0;
    /* ��ѹ(0.1V) */
    xMsg.Data[4] = (ulV >> 24) & 0xFF;
    xMsg.Data[5] = (ulV >> 16) & 0xFF;
    xMsg.Data[6] = (ulV >>  8) & 0xFF;
    xMsg.Data[7] = (ulV >>  0) & 0xFF;
    CanSend(&xMsg, SEND_DELAY);
    
    return STATUS_OK;
}

/* ����趨��� */
static Status_t prvSetOutput(uint32_t ulPwr2Addr, uint32_t ulCur, uint32_t ulVol)
{
    CanMsgTx_t xMsg;
    xMsg.StdId   = 0;
    xMsg.ExtId   = 0x1907C080 | ulPwr2Addr;
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
    if(xQueueReceive(s_xQueue, &m, pdMS_TO_TICKS(WAIT_MS)) == pdPASS) {
        if (((m.ulId & ID_MASK) == 0x1807C080) && (m.ucData[0] == 0) && (m.ucData[1] == 0xFF)) {
            return STATUS_OK;
        }
    }
    
    return STATUS_ERR;
}

/* ��ȡģ����Ϣ */
static Status_t prvGetStatus(uint32_t ulPwr2Addr)
{
    CanMsgTx_t xMsg;
    xMsg.StdId   = 0;
    xMsg.ExtId   = 0x1907C080 | ulPwr2Addr;
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
    if(xQueueReceive(s_xQueue, &m, pdMS_TO_TICKS(WAIT_MS)) == pdPASS) {
        if (((m.ulId & ID_MASK) == 0x1807C080) && (m.ucData[0] == 1)) {
            switch (ulPwr2Addr)
            {
                case PWR_T_ADDR: 
                    s_T_xData.usOutCur  = ((uint16_t)m.ucData[2] << 8) | (uint16_t)m.ucData[3];
                    s_T_xData.usOutVol  = ((uint16_t)m.ucData[4] << 8) | (uint16_t)m.ucData[5];
                    s_T_xData.ucStatus1 = m.ucData[6];
                    s_T_xData.ucStatus0 = m.ucData[7];
                    break;
                case PWR_M_ADDR: 
                    s_M_xData.usOutCur  = ((uint16_t)m.ucData[2] << 8) | (uint16_t)m.ucData[3];
                    s_M_xData.usOutVol  = ((uint16_t)m.ucData[4] << 8) | (uint16_t)m.ucData[5];
                    s_M_xData.ucStatus1 = m.ucData[6];
                    s_M_xData.ucStatus0 = m.ucData[7];
                    break;
                case PWR_B_ADDR: 
                    s_B_xData.usOutCur  = ((uint16_t)m.ucData[2] << 8) | (uint16_t)m.ucData[3];
                    s_B_xData.usOutVol  = ((uint16_t)m.ucData[4] << 8) | (uint16_t)m.ucData[5];
                    s_B_xData.ucStatus1 = m.ucData[6];
                    s_B_xData.ucStatus0 = m.ucData[7];
                    break;
            }
            return STATUS_OK;
        }
    }
    return STATUS_ERR;
}

/* ����ģ�鿪�ػ� */
static Status_t prvSetOnOff(uint32_t ulPwr2Addr, uint8_t ucOnOff)
{
    CanMsgTx_t xMsg;
    xMsg.StdId   = 0;
    xMsg.ExtId   = 0x1907C080 | ulPwr2Addr;
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
    if(xQueueReceive(s_xQueue, &m, pdMS_TO_TICKS(WAIT_MS)) == pdPASS) {
        if (((m.ulId & ID_MASK) == 0x1807C080) && (m.ucData[0] == 2) && (m.ucData[1] == 0xFF)) {
            return STATUS_OK;
        }
    }
    
    return STATUS_ERR;
}

/* ��ȡ����趨��ѹ */
static Status_t prvGetCfgVol(uint32_t ulPwr2Addr)
{
    CanMsgTx_t xMsg;
    xMsg.StdId   = 0;
    xMsg.ExtId   = 0x19010080 | ulPwr2Addr;
    xMsg.IDE     = CAN_ID_EXT;
    xMsg.RTR     = CAN_RTR_DATA;
    xMsg.DLC     = 0;
    CanSend(&xMsg, SEND_DELAY);
    
    Msg_t m;
    if(xQueueReceive(s_xQueue, &m, pdMS_TO_TICKS(WAIT_MS)) == pdPASS) {
        if ((m.ulId & ID_MASK) == 0x18010080) {
            switch (ulPwr2Addr)
            {
                case PWR_T_ADDR: s_T_xData.usCfgVol = ((uint16_t)m.ucData[6] << 8) | (uint16_t)m.ucData[7];break;
                case PWR_M_ADDR: s_M_xData.usCfgVol = ((uint16_t)m.ucData[6] << 8) | (uint16_t)m.ucData[7];break;
                case PWR_B_ADDR: s_B_xData.usCfgVol = ((uint16_t)m.ucData[6] << 8) | (uint16_t)m.ucData[7];break;
            }
            
            return STATUS_OK;
        }
    }
    
    return STATUS_ERR;
}

/* ��ȡ����趨���� */
static Status_t prvGetCfgCur(uint32_t ulPwr2Addr)
{
    CanMsgTx_t xMsg;
    xMsg.StdId   = 0;
    xMsg.ExtId   = 0x19010880 | ulPwr2Addr;
    xMsg.IDE     = CAN_ID_EXT;
    xMsg.RTR     = CAN_RTR_DATA;
    xMsg.DLC     = 0;
    CanSend(&xMsg, SEND_DELAY);
    
    Msg_t m;
    if(xQueueReceive(s_xQueue, &m, pdMS_TO_TICKS(WAIT_MS)) == pdPASS) {
        if ((m.ulId & ID_MASK) == 0x18010880) {
            switch (ulPwr2Addr)
            {
                case PWR_T_ADDR: s_T_xData.usCfgCur = ((uint16_t)m.ucData[0] << 8) | (uint16_t)m.ucData[1];break;
                case PWR_M_ADDR: s_M_xData.usCfgCur = ((uint16_t)m.ucData[0] << 8) | (uint16_t)m.ucData[1];break;
                case PWR_B_ADDR: s_B_xData.usCfgCur = ((uint16_t)m.ucData[0] << 8) | (uint16_t)m.ucData[1];break;
            }
            
            return STATUS_OK;
        }
    }
    
    return STATUS_ERR;
}

/* ��ȡ�����ѹֵ */
static Status_t prvGetInputVol(uint32_t ulPwr2Addr)
{
    CanMsgTx_t xMsg;
    xMsg.StdId   = 0;
    xMsg.ExtId   = 0x1907A080 | ulPwr2Addr;
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
    if(xQueueReceive(s_xQueue, &m, pdMS_TO_TICKS(WAIT_MS)) == pdPASS) {
        if (((m.ulId & ID_MASK) == 0x1807A080) && (m.ucData[0] == 0x31)) {
            switch (ulPwr2Addr)
            {
                case PWR_T_ADDR: 
                    s_T_xData.usInVab = ((uint16_t)m.ucData[2] << 8) | (uint16_t)m.ucData[3];
                    s_T_xData.usInVbc = ((uint16_t)m.ucData[4] << 8) | (uint16_t)m.ucData[5];
                    s_T_xData.usInVca = ((uint16_t)m.ucData[6] << 8) | (uint16_t)m.ucData[7];
                    break;
                case PWR_M_ADDR: 
                    s_M_xData.usInVab = ((uint16_t)m.ucData[2] << 8) | (uint16_t)m.ucData[3];
                    s_M_xData.usInVbc = ((uint16_t)m.ucData[4] << 8) | (uint16_t)m.ucData[5];
                    s_M_xData.usInVca = ((uint16_t)m.ucData[6] << 8) | (uint16_t)m.ucData[7];
                    break;
                case PWR_B_ADDR: 
                    s_B_xData.usInVab = ((uint16_t)m.ucData[2] << 8) | (uint16_t)m.ucData[3];
                    s_B_xData.usInVbc = ((uint16_t)m.ucData[4] << 8) | (uint16_t)m.ucData[5];
                    s_B_xData.usInVca = ((uint16_t)m.ucData[6] << 8) | (uint16_t)m.ucData[7];
                    break;
            }
            return STATUS_OK;
        }
    }
    
    return STATUS_ERR;
}

/* ��ȡ�����¶� */
static Status_t prvGetEnvTemp(uint32_t ulPwr2Addr)
{
    CanMsgTx_t xMsg;
    xMsg.StdId   = 0;
    xMsg.ExtId   = 0x19008080 | ulPwr2Addr;
    xMsg.IDE     = CAN_ID_EXT;
    xMsg.RTR     = CAN_RTR_DATA;
    xMsg.DLC     = 0;
    CanSend(&xMsg, SEND_DELAY);
    
    Msg_t m;
    if(xQueueReceive(s_xQueue, &m, pdMS_TO_TICKS(WAIT_MS)) == pdPASS) {
        if ((m.ulId & ID_MASK) == 0x18008080) {
            switch (ulPwr2Addr)
            {
                case PWR_T_ADDR: s_T_xData.sTemp = ((uint16_t)m.ucData[4] << 8) | (uint16_t)m.ucData[5];break;
                case PWR_M_ADDR: s_M_xData.sTemp = ((uint16_t)m.ucData[4] << 8) | (uint16_t)m.ucData[5];break;
                case PWR_B_ADDR: s_B_xData.sTemp = ((uint16_t)m.ucData[4] << 8) | (uint16_t)m.ucData[5];break;
            }
            
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
    if (!s_bInit) {
        cliprintf("enable the device first\n");
        return;
    }
    
    if (argc != 4) {
        cliprintf("pwr2_set_cur_vol DEV CUR VOL\n");
        return;
    }
    
    int   lD = atoi(argv[1]);
    float fC = atof(argv[2]);
    float fV = atof(argv[3]);
    if (STATUS_OK == prvSetOutput(lD, fC * 1000, fV * 1000)) {
        cliprintf("    ok\n");
    }
    else {
        cliprintf("    err\n");
    }
}
CLI_CMD_EXPORT(pwr2_set_cur_vol, set power(18kw) voltage, prvCliCmdPwr2SetCurVol)

static void prvCliCmdPwr2CtrlOn(cli_printf cliprintf, int argc, char** argv)
{
    if (!s_bInit) {
        cliprintf("enable the device first\n");
        return;
    }
    
    int   lD = atoi(argv[1]);
    
    if (STATUS_OK == prvSetOnOff(lD, 1)) {
        cliprintf("    ok\n");
    }
    else {
        cliprintf("    err\n");
    }
}
CLI_CMD_EXPORT(pwr2_ctrl_on, set power(18kw) output on, prvCliCmdPwr2CtrlOn)

static void prvCliCmdPwr2CtrlOff(cli_printf cliprintf, int argc, char** argv)
{
    if (!s_bInit) {
        cliprintf("enable the device first\n");
        return;
    }
    
    
    int   lD = atoi(argv[1]);
    if (STATUS_OK == prvSetOnOff(lD, 0)) {
        cliprintf("    ok\n");
    }
    else {
        cliprintf("    err\n");
    }
}
CLI_CMD_EXPORT(pwr2_ctrl_off, set power(18kw) output off, prvCliCmdPwr2CtrlOff)

static void prvCliCmdPwr2SetVolDef(cli_printf cliprintf, int argc, char** argv)
{
    if (!s_bInit) {
        cliprintf("enable the device first\n");
        return;
    }
    
    if (argc != 2) {
        cliprintf("pwr2_set_vol_def VOL\n");
        return;
    }
    
    float fV = atof(argv[1]);
    uint32_t ulV = (uint32_t)(fV * 10);
    
    CanMsgTx_t xMsg;
    xMsg.StdId   = 0;
    xMsg.ExtId   = 0x19420080;
    xMsg.IDE     = CAN_ID_EXT;
    xMsg.RTR     = CAN_RTR_DATA;
    xMsg.DLC     = 8;
    /* ����(0.1A) */
    xMsg.Data[0] = 0;
    xMsg.Data[1] = 0;
    xMsg.Data[2] = 0;
    xMsg.Data[3] = 0;
    /* ��ѹ(0.1V) */
    xMsg.Data[4] = (ulV >> 24) & 0xFF;
    xMsg.Data[5] = (ulV >> 16) & 0xFF;
    xMsg.Data[6] = (ulV >>  8) & 0xFF;
    xMsg.Data[7] = (ulV >>  0) & 0xFF;
    CanSend(&xMsg, SEND_DELAY);
    
}
CLI_CMD_EXPORT(pwr2_set_vol_def, set power(18kw) default voltage, prvCliCmdPwr2SetVolDef)

static void prvCliCmdPwr2Status(cli_printf cliprintf, int argc, char** argv)
{
    if (!s_bInit) {
        cliprintf("enable the device first\n");
        return;
    }
    
    cliprintf("PWR_T_STATUS :\n");
    cliprintf("    �������     : %.1f A\n", s_T_xData.usOutCur * 0.1);
    cliprintf("    �����ѹ     : %.1f V\n", s_T_xData.usOutVol * 0.1);
    cliprintf("    ״̬1        : 0x%02X\n", s_T_xData.ucStatus1);
    cliprintf("    ״̬0        : 0x%02X\n", s_T_xData.ucStatus0);
    cliprintf("    �趨����     : %.1f A\n", s_T_xData.usCfgCur * 0.1);
    cliprintf("    �趨��ѹ     : %.1f V\n", s_T_xData.usCfgVol * 0.1);
    cliprintf("    �����ѹ(Vab): %.1f V\n", s_T_xData.usInVab / 32.);
    cliprintf("    �����ѹ(Vbc): %.1f V\n", s_T_xData.usInVbc / 32.);
    cliprintf("    �����ѹ(Vca): %.1f V\n", s_T_xData.usInVca / 32.);
    cliprintf("    �����¶�     : %.1f ��\n", s_T_xData.sTemp * 0.1);
    cliprintf("\n");
    
    cliprintf("PWR_M_STATUS :\n");
    cliprintf("    �������     : %.1f A\n", s_M_xData.usOutCur * 0.1);
    cliprintf("    �����ѹ     : %.1f V\n", s_M_xData.usOutVol * 0.1);
    cliprintf("    ״̬1        : 0x%02X\n", s_M_xData.ucStatus1);
    cliprintf("    ״̬0        : 0x%02X\n", s_M_xData.ucStatus0);
    cliprintf("    �趨����     : %.1f A\n", s_M_xData.usCfgCur * 0.1);
    cliprintf("    �趨��ѹ     : %.1f V\n", s_M_xData.usCfgVol * 0.1);
    cliprintf("    �����ѹ(Vab): %.1f V\n", s_M_xData.usInVab / 32.);
    cliprintf("    �����ѹ(Vbc): %.1f V\n", s_M_xData.usInVbc / 32.);
    cliprintf("    �����ѹ(Vca): %.1f V\n", s_M_xData.usInVca / 32.);
    cliprintf("    �����¶�     : %.1f ��\n", s_M_xData.sTemp * 0.1);
    cliprintf("\n");
    
    cliprintf("PWR_B_STATUS :\n");
    cliprintf("    �������     : %.1f A\n", s_B_xData.usOutCur * 0.1);
    cliprintf("    �����ѹ     : %.1f V\n", s_B_xData.usOutVol * 0.1);
    cliprintf("    ״̬1        : 0x%02X\n", s_B_xData.ucStatus1);
    cliprintf("    ״̬0        : 0x%02X\n", s_B_xData.ucStatus0);
    cliprintf("    �趨����     : %.1f A\n", s_B_xData.usCfgCur * 0.1);
    cliprintf("    �趨��ѹ     : %.1f V\n", s_B_xData.usCfgVol * 0.1);
    cliprintf("    �����ѹ(Vab): %.1f V\n", s_B_xData.usInVab / 32.);
    cliprintf("    �����ѹ(Vbc): %.1f V\n", s_B_xData.usInVbc / 32.);
    cliprintf("    �����ѹ(Vca): %.1f V\n", s_B_xData.usInVca / 32.);
    cliprintf("    �����¶�     : %.1f ��\n", s_B_xData.sTemp * 0.1);
    cliprintf("\n");
}
CLI_CMD_EXPORT(pwr2_status, show power(18kw) status, prvCliCmdPwr2Status)
