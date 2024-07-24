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

#define PWR2_RTOS 1
/* Pragmas */
#pragma diag_suppress 177   /* warning: #177-D: function "FUNC" was set but never used */

/* Debug config */
#if PWR_DEBUG || 0
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

#if PWR2_RTOS
static osMutexId s_xPwr2Mutex;
#define PWR2_MUTEX_INIT()                                                                                            \
    do {                                                                                                               \
        osMutexDef(Pwr2Mutex);                                                                                          \
        s_xPwr2Mutex = osMutexCreate(osMutex(Pwr2Mutex));                                                                \
    } while (0)
#define PWR2_LOCK()   osMutexWait(s_xPwr2Mutex, osWaitForever)
#define PWR2_UNLOCK() osMutexRelease(s_xPwr2Mutex)
#else
#define PWR2_MUTEX_INIT()
#define PWR2_LOCK()
#define PWR2_UNLOCK()
#endif /* SW_I2C_RTOS */

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
static _Data_t       s_M2_xData;
static _Data_t       s_M1_xData;
static _Data_t       s_M3_xData;

static QueueHandle_t s_xQueue;

/* Functions */
Status_t Pwr2ProtInit(void)
{
    if (!s_bInit) {
        PWR2_MUTEX_INIT();
        memset(&s_M2_xData, 0, sizeof(s_M2_xData));
        memset(&s_M1_xData, 0, sizeof(s_M1_xData));
        memset(&s_M3_xData, 0, sizeof(s_M3_xData));
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
    
    PWR2_LOCK();
    TRACE("PWR_MUTEX\n");
#if 1
    prvGetStatus(PWR2_M2_ADDR);
    prvGetCfgVol(PWR2_M2_ADDR);
    prvGetCfgCur(PWR2_M2_ADDR);
    prvGetInputVol(PWR2_M2_ADDR);
    prvGetEnvTemp(PWR2_M2_ADDR);
#endif

#if 1
    memset(&s_M1_xData, 0, sizeof(s_M1_xData));
    prvGetStatus(PWR2_M1_ADDR);
    prvGetCfgVol(PWR2_M1_ADDR);
    prvGetCfgCur(PWR2_M1_ADDR);
    prvGetInputVol(PWR2_M1_ADDR);
    prvGetEnvTemp(PWR2_M1_ADDR);
#endif

    
#if 1
    prvGetStatus(PWR2_M3_ADDR);
    prvGetCfgVol(PWR2_M3_ADDR);
    prvGetCfgCur(PWR2_M3_ADDR);
    prvGetInputVol(PWR2_M3_ADDR);
    prvGetEnvTemp(PWR2_M3_ADDR);
#endif
    
    PWR2_UNLOCK();
    TRACE("PWR_UNLOCK\n");
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
    PWR2_LOCK();
    int32_t r = 0;
    switch (ulAddr)
    {
    case PWR2_M1_ADDR: 
    switch (xType) {
        case PWR_OUTPUT_VOL:
            r = s_M1_xData.usOutVol; /* 0.1V */
            break;
        case PWR_STATUS:
            r = (s_M1_xData.ucStatus1 << 8) | s_M1_xData.ucStatus0;
            break;
        case PWR_INPUT_VOL:
            r = s_M1_xData.usInVca; /* 0.1V */
            break;
        case PWR_OUTPUT_CUR:
            r = s_M1_xData.usOutCur * 0.1;
            break;
        default:
            r = 0;
            break;
    }break;
    case PWR2_M2_ADDR: 
        switch (xType) {
            case PWR_OUTPUT_VOL:
                r = s_M2_xData.usOutVol; /* 0.1V */
                break;
            case PWR_STATUS:
                r = (s_M2_xData.ucStatus1 << 8) | s_M2_xData.ucStatus0;
                break;
            default:
                r = 0;
                break;
        }break;
    
    case PWR2_M3_ADDR: 
        switch (xType) {
            case PWR_OUTPUT_VOL:
                r = s_M3_xData.usOutVol; /* 0.1V */
                break;
            case PWR_STATUS:
                r = (s_M3_xData.ucStatus1 << 8) | s_M3_xData.ucStatus0;
                break;
            default:
                r = 0;
                break;
        }break;        
    }
    PWR2_UNLOCK();
    
//    memset(&s_M1_xData, 0, sizeof(s_M1_xData));
    
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
                case PWR2_M2_ADDR: 
                    s_M2_xData.usOutCur  = ((uint16_t)m.ucData[2] << 8) | (uint16_t)m.ucData[3];
                    s_M2_xData.usOutVol  = ((uint16_t)m.ucData[4] << 8) | (uint16_t)m.ucData[5];
                    s_M2_xData.ucStatus1 = m.ucData[6];
                    s_M2_xData.ucStatus0 = m.ucData[7];
                    break;
                case PWR2_M1_ADDR: 
                    s_M1_xData.usOutCur  = ((uint16_t)m.ucData[2] << 8) | (uint16_t)m.ucData[3];
                    s_M1_xData.usOutVol  = ((uint16_t)m.ucData[4] << 8) | (uint16_t)m.ucData[5];
                    s_M1_xData.ucStatus1 = m.ucData[6];
                    s_M1_xData.ucStatus0 = m.ucData[7];
                    break;
                case PWR2_M3_ADDR: 
                    s_M3_xData.usOutCur  = ((uint16_t)m.ucData[2] << 8) | (uint16_t)m.ucData[3];
                    s_M3_xData.usOutVol  = ((uint16_t)m.ucData[4] << 8) | (uint16_t)m.ucData[5];
                    s_M3_xData.ucStatus1 = m.ucData[6];
                    s_M3_xData.ucStatus0 = m.ucData[7];
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
                case PWR2_M2_ADDR: s_M2_xData.usCfgVol = ((uint16_t)m.ucData[6] << 8) | (uint16_t)m.ucData[7];break;
                case PWR2_M1_ADDR: s_M1_xData.usCfgVol = ((uint16_t)m.ucData[6] << 8) | (uint16_t)m.ucData[7];break;
                case PWR2_M3_ADDR: s_M3_xData.usCfgVol = ((uint16_t)m.ucData[6] << 8) | (uint16_t)m.ucData[7];break;
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
                case PWR2_M2_ADDR: s_M2_xData.usCfgCur = ((uint16_t)m.ucData[0] << 8) | (uint16_t)m.ucData[1];break;
                case PWR2_M1_ADDR: s_M1_xData.usCfgCur = ((uint16_t)m.ucData[0] << 8) | (uint16_t)m.ucData[1];break;
                case PWR2_M3_ADDR: s_M3_xData.usCfgCur = ((uint16_t)m.ucData[0] << 8) | (uint16_t)m.ucData[1];break;
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
                case PWR2_M2_ADDR: 
                    s_M2_xData.usInVab = ((uint16_t)m.ucData[2] << 8) | (uint16_t)m.ucData[3];
                    s_M2_xData.usInVbc = ((uint16_t)m.ucData[4] << 8) | (uint16_t)m.ucData[5];
                    s_M2_xData.usInVca = ((uint16_t)m.ucData[6] << 8) | (uint16_t)m.ucData[7];
                    break;
                case PWR2_M1_ADDR: 
                    s_M1_xData.usInVab = ((uint16_t)m.ucData[2] << 8) | (uint16_t)m.ucData[3];
                    s_M1_xData.usInVbc = ((uint16_t)m.ucData[4] << 8) | (uint16_t)m.ucData[5];
                    s_M1_xData.usInVca = ((uint16_t)m.ucData[6] << 8) | (uint16_t)m.ucData[7];
                    break;
                case PWR2_M3_ADDR: 
                    s_M3_xData.usInVab = ((uint16_t)m.ucData[2] << 8) | (uint16_t)m.ucData[3];
                    s_M3_xData.usInVbc = ((uint16_t)m.ucData[4] << 8) | (uint16_t)m.ucData[5];
                    s_M3_xData.usInVca = ((uint16_t)m.ucData[6] << 8) | (uint16_t)m.ucData[7];
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
                case PWR2_M2_ADDR: s_M2_xData.sTemp = ((uint16_t)m.ucData[4] << 8) | (uint16_t)m.ucData[5];break;
                case PWR2_M1_ADDR: s_M1_xData.sTemp = ((uint16_t)m.ucData[4] << 8) | (uint16_t)m.ucData[5];break;
                case PWR2_M3_ADDR: s_M3_xData.sTemp = ((uint16_t)m.ucData[4] << 8) | (uint16_t)m.ucData[5];break;
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
    
    cliprintf("PWR_M1_STATUS :\n");
    cliprintf("    �������     : %.1f A\n", s_M1_xData.usOutCur * 0.1);
    cliprintf("    �����ѹ     : %.1f V\n", s_M1_xData.usOutVol * 0.1);
    cliprintf("    ״̬1        : 0x%02X\n", s_M1_xData.ucStatus1);
    cliprintf("    ״̬0        : 0x%02X\n", s_M1_xData.ucStatus0);
    cliprintf("    �趨����     : %.1f A\n", s_M1_xData.usCfgCur * 0.1);
    cliprintf("    �趨��ѹ     : %.1f V\n", s_M1_xData.usCfgVol * 0.1);
    cliprintf("    �����ѹ(Vab): %.1f V\n", s_M1_xData.usInVab / 32.);
    cliprintf("    �����ѹ(Vbc): %.1f V\n", s_M1_xData.usInVbc / 32.);
    cliprintf("    �����ѹ(Vca): %.1f V\n", s_M1_xData.usInVca / 32.);
    cliprintf("    �����¶�     : %.1f ��\n",s_M1_xData.sTemp * 0.1);
    cliprintf("\n");
    
    cliprintf("PWR_M2_STATUS :\n");
    cliprintf("    �������     : %.1f A\n", s_M2_xData.usOutCur * 0.1);
    cliprintf("    �����ѹ     : %.1f V\n", s_M2_xData.usOutVol * 0.1);
    cliprintf("    ״̬1        : 0x%02X\n", s_M2_xData.ucStatus1);
    cliprintf("    ״̬0        : 0x%02X\n", s_M2_xData.ucStatus0);
    cliprintf("    �趨����     : %.1f A\n", s_M2_xData.usCfgCur * 0.1);
    cliprintf("    �趨��ѹ     : %.1f V\n", s_M2_xData.usCfgVol * 0.1);
    cliprintf("    �����ѹ(Vab): %.1f V\n", s_M2_xData.usInVab / 32.);
    cliprintf("    �����ѹ(Vbc): %.1f V\n", s_M2_xData.usInVbc / 32.);
    cliprintf("    �����ѹ(Vca): %.1f V\n", s_M2_xData.usInVca / 32.);
    cliprintf("    �����¶�     : %.1f ��\n",s_M2_xData.sTemp * 0.1);
    cliprintf("\n");
    
    cliprintf("PWR_M3_STATUS :\n");
    cliprintf("    �������     : %.1f A\n", s_M3_xData.usOutCur * 0.1);
    cliprintf("    �����ѹ     : %.1f V\n", s_M3_xData.usOutVol * 0.1);
    cliprintf("    ״̬1        : 0x%02X\n", s_M3_xData.ucStatus1);
    cliprintf("    ״̬0        : 0x%02X\n", s_M3_xData.ucStatus0);
    cliprintf("    �趨����     : %.1f A\n", s_M3_xData.usCfgCur * 0.1);
    cliprintf("    �趨��ѹ     : %.1f V\n", s_M3_xData.usCfgVol * 0.1);
    cliprintf("    �����ѹ(Vab): %.1f V\n", s_M3_xData.usInVab / 32.);
    cliprintf("    �����ѹ(Vbc): %.1f V\n", s_M3_xData.usInVbc / 32.);
    cliprintf("    �����ѹ(Vca): %.1f V\n", s_M3_xData.usInVca / 32.);
    cliprintf("    �����¶�     : %.1f ��\n",s_M3_xData.sTemp * 0.1);
    cliprintf("\n");

}
CLI_CMD_EXPORT(pwr2_status, show power(18kw) status, prvCliCmdPwr2Status)
