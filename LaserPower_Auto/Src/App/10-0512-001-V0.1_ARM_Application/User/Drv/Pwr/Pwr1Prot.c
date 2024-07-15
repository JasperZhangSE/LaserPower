/*
    Pwr1Prot.c

    Implementation File for Pwr1 Prot Module
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 11Nov23, Karl Created
    01b, 07Dec23, Karl Added Pwr1Output function
    01c, 13Dec23, Karl Fixed prvProcRequestByteDataResp bug
    01d, 15Dec23, Karl Optimized prvCliCmdPwr1Status
    01e, 27Dec23, Karl Added Pwr1DataGet
    01f, 17Jan24, Karl Added Pwr1SetVolDef
    01g, 20Jan24, Karl Added PWR_STATUS
*/

/* Includes */
#include "Include.h"

/* Pragmas */
#pragma anon_unions

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

#if PWR1_ENABLE

/* Local defines */
#define PWR_ADDR                1
#define MONITOR_ADDR            0xF0
#define SEND_DELAY              5
#define BYTE_ENDIAN_UINT16(a)   (((a) & 0xFF) << 8) | (((a) & 0xFF00) >> 8)
#define BYTE_ENDIAN_UINT32(a)   (((a) & 0xFF) << 24) | (((a) & 0xFF00) << 8) | (((a) & 0xFF0000) >> 8) | (((a) & 0xFF000000) >> 24)
#define WAIT_MS                 50

/* Local types */
#pragma pack(push)
#pragma pack(1)
typedef union {
    struct {
        uint32_t Fix1:2;    /* Fixed 0x03 */
        uint32_t Cnt:1;     /* Subsequent data identification */
        uint32_t SrcAddr:8; /* Source address */
        uint32_t DstAddr:8; /* Destination address */
        uint32_t Ptp:1;     /* 1:Peer to peer, 0:Broadcast */
        uint32_t Fix2:9;    /* Fixed 0x60 */
        uint32_t Fix3:3;    /* Fixed 0x00 */
    }Bit;
    uint32_t All;
}Id_t;

typedef struct {
    struct {
        uint8_t MsgType:7;
        uint8_t Err:1;
    };
    uint8_t  ErrType;
    uint16_t ValueType;
    uint32_t Value;
}_Data_t;
#pragma pack(pop)

/* Forward declarations */
static Status_t prvProcRequestByteDataResp(_Data_t xData);
static Status_t prvProcConfigByteDataResp(_Data_t xData);
static Status_t prvSetVol(float fV);
static Status_t prvSetVolDef(float fV);
static Status_t prvSetOnOff(uint8_t ucOnOff);
static Status_t prvGetData(uint16_t usValueType);

/* Local variables */
static bool          s_bInit = false;
static float         s_fData[65];
static uint32_t      s_ulData[65];
static QueueHandle_t s_xQueue = NULL;
static uint32_t      s_ulTxCnt = 0;
static uint32_t      s_ulRxCnt = 0;
static uint32_t      s_ulErrCnt = 0;

/* Functions */
Status_t Pwr1ProtInit(void)
{
    if (!s_bInit) {
        memset(s_fData, 0, sizeof(s_fData));
        memset(s_ulData, 0, sizeof(s_ulData));
        s_xQueue = xQueueCreate(1/*Queue length*/, 1/*Queue item size*/);
        s_bInit = true;
    }
    return STATUS_OK;
}

Status_t Pwr1ProtTerm(void)
{
    /* Do nothing */
    return STATUS_OK;
}

Status_t Pwr1Update(void)
{
    prvGetData(0x0001);
    prvGetData(0x0002);
    prvGetData(0x0003);
    prvGetData(0x0004);
    prvGetData(0x0005);
    prvGetData(0x0006);
    prvGetData(0x0007);
    prvGetData(0x0008);
    prvGetData(0x0009);
    prvGetData(0x000A);
    prvGetData(0x000C);
    prvGetData(0x000D);
    prvGetData(0x000E);
    prvGetData(0x000F);
    prvGetData(0x0010);
    prvGetData(0x0011);
    prvGetData(0x0012);
    prvGetData(0x0013);
    prvGetData(0x0014);
    prvGetData(0x0040);
    return STATUS_OK;
}

void Pwr1CanRxNotify(CanMsgRx_t *pxMsg)
{
#if 0
    TRACE("RX: 0x%08X - %02X %02X %02X %02X %02X %02X %02X %02X\n", pxMsg->ExtId, \
                                                                    pxMsg->Data[0], pxMsg->Data[1], pxMsg->Data[2], pxMsg->Data[3], \
                                                                    pxMsg->Data[4], pxMsg->Data[5], pxMsg->Data[6], pxMsg->Data[7]);
#endif
    
    s_ulRxCnt++;
    
    if (pxMsg->IDE != CAN_ID_EXT) {
        return;
    }
    
    if (pxMsg->DLC != 8) {
        return;
    }
    
    _Data_t xData;
    memcpy(&xData, pxMsg->Data, 8);
    
    if (xData.Err) {
        /* Wrong frame format */
        return;
    }
    
    if (xData.ErrType != 0xF0) {
        /* 0xF0: Ok; 0xF1: Wrong address; 0xF2: Invalid command; */
        return;
    }
    
    uint8_t m = xData.MsgType;
    BaseType_t xHigherPriorityTaskWoken;
    switch (xData.MsgType) {
    case 0x41:  /* Request byte data response */
        prvProcRequestByteDataResp(xData);
        xQueueSendToBackFromISR(s_xQueue, &m, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        break;
    case 0x42:  /* Request bit data response */
        /* Do nothing */
        break;
    case 0x43:  /* Config byte data response */
        prvProcConfigByteDataResp(xData);
        xQueueSendToBackFromISR(s_xQueue, &m, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        break;
    default:
        /* Do nothing */
        break;
    }
}

Status_t Pwr1Output(uint8_t ucOnOff)
{
    return prvSetOnOff(ucOnOff);
}

int32_t Pwr1DataGet(PwrDataType_t xType)
{
    int32_t r = 0;
    
    switch (xType) {
    case PWR_OUTPUT_VOL:
        r = (int32_t)(s_fData[1] * 10); /* 0.1V */
        break;
    case PWR_STATUS:
        r = s_ulData[0x0040];
        break;
    default:
        r = 0;
        break;
    }
    
    return r;
}

Status_t Pwr1SetVolDef(float fV)
{
    return prvSetVolDef(fV);
}

static Status_t prvProcRequestByteDataResp(_Data_t xData)
{
    uint32_t ulValue = BYTE_ENDIAN_UINT32(xData.Value);
    uint16_t usValueType = BYTE_ENDIAN_UINT16(xData.ValueType);
    switch (usValueType) {
    case 0x0001:  /* ȡģ���ѹ              (4B float) */
    case 0x0002:  /* ȡģ�������ʵֵ        (4B float) */
    case 0x0003:  /* ȡģ��������            (4B float) */
    case 0x0004:  /* ȡģ��DCDC�¶�          (4B float) */
    case 0x0005:  /* ȡģ��AC�ۺϵ�ѹ        (4B float) */
    case 0x0006:  /* ȡģ���ѹ����          (4B float) */
    case 0x0007:  /* ȡģ�������ʾֵ        (4B float) */
    case 0x0008:  /* ȡģ��PFC��ĸ�ߵ�ѹ     (4B float) */
    case 0x0009:  /* ȡģ����µ�            (4B float) */
    case 0x000A:  /* ȡģ��PFC��ĸ�ߵ�ѹ     (4B float) */
    case 0x000C:  /* ȡģ��AC A���ѹ        (4B float) */
    case 0x000D:  /* ȡģ��AC B���ѹ        (4B float) */
    case 0x000E:  /* ȡģ��AC C���ѹ        (4B float) */
    case 0x000F:  /* ȡģ��/ȱʡ�����ѹ (4B float) */
    case 0x0010:  /* ȡģ��PFC�¶�           (4B float) */
    case 0x0011:  /* ȡģ���������      (4B float) */
    case 0x0012:  /* ȡģ���������      (4B float) */
    case 0x0013:  /* ȡģ�������ѹ      (4B float) */
    case 0x0014:  /* ȡģ���޹��ʵ�          (1B integer) */
    case 0x0040:  /* ��ȡ��ǰ�澯/״̬       (4B integer) */
        memcpy(&s_fData[usValueType], &ulValue, sizeof(ulValue));
        memcpy(&s_ulData[usValueType], &ulValue, sizeof(ulValue));
        break;
    default:
        /* Do nothing */
        break;
    }
    
    return STATUS_OK;
}

static Status_t prvProcConfigByteDataResp(_Data_t xData)
{
    /* XXX: Do nothing */
    return STATUS_OK;
}

static Status_t prvSetVol(float fV)
{
    Id_t xId;
    xId.Bit.Fix1    = 3;
    xId.Bit.Cnt     = 0;
    xId.Bit.SrcAddr = MONITOR_ADDR;
    xId.Bit.DstAddr = PWR_ADDR;
    xId.Bit.Ptp     = 1;
    xId.Bit.Fix2    = 0x60;
    xId.Bit.Fix3    = 0;
    
    uint32_t ulTmp;
    memcpy(&ulTmp, &fV, sizeof(ulTmp));
    ulTmp = BYTE_ENDIAN_UINT32(ulTmp);
    
    _Data_t xData;
    xData.Err       = 0;
    xData.MsgType   = 0x03;
    xData.ErrType   = 0;
    xData.ValueType = BYTE_ENDIAN_UINT16(0x0021);
    xData.Value     = ulTmp;
    
    CanMsgTx_t xMsg;
    xMsg.StdId = xId.All;
    xMsg.ExtId = xId.All;
    xMsg.IDE   = CAN_ID_EXT;
    xMsg.RTR   = CAN_RTR_DATA;
    xMsg.DLC   = 8;
    memcpy(xMsg.Data, &xData, sizeof(xData));
    CanSend(&xMsg, SEND_DELAY);
    s_ulTxCnt++;
    
    uint8_t r;
    if(xQueueReceive(s_xQueue, &r, pdMS_TO_TICKS(WAIT_MS)) == pdPASS) {
        if (0x43 == r) {
            return STATUS_OK;
        }
    }
    s_ulErrCnt++;
    return STATUS_ERR;
}

static Status_t prvSetVolDef(float fV)
{
    Id_t xId;
    xId.Bit.Fix1    = 3;
    xId.Bit.Cnt     = 0;
    xId.Bit.SrcAddr = MONITOR_ADDR;
    xId.Bit.DstAddr = PWR_ADDR;
    xId.Bit.Ptp     = 1;
    xId.Bit.Fix2    = 0x60;
    xId.Bit.Fix3    = 0;
    
    uint32_t ulTmp;
    memcpy(&ulTmp, &fV, sizeof(ulTmp));
    ulTmp = BYTE_ENDIAN_UINT32(ulTmp);
    
    _Data_t xData;
    xData.Err       = 0;
    xData.MsgType   = 0x03;
    xData.ErrType   = 0;
    xData.ValueType = BYTE_ENDIAN_UINT16(0x0024);
    xData.Value     = ulTmp;
    
    CanMsgTx_t xMsg;
    xMsg.StdId = xId.All;
    xMsg.ExtId = xId.All;
    xMsg.IDE   = CAN_ID_EXT;
    xMsg.RTR   = CAN_RTR_DATA;
    xMsg.DLC   = 8;
    memcpy(xMsg.Data, &xData, sizeof(xData));
    CanSend(&xMsg, SEND_DELAY);
    s_ulTxCnt++;
    
    uint8_t r;
    if(xQueueReceive(s_xQueue, &r, pdMS_TO_TICKS(WAIT_MS)) == pdPASS) {
        if (0x43 == r) {
            return STATUS_OK;
        }
    }
    
    s_ulErrCnt++;
    return STATUS_ERR;
}

static Status_t prvSetOnOff(uint8_t ucOnOff)
{
    Id_t xId;
    xId.Bit.Fix1    = 3;
    xId.Bit.Cnt     = 0;
    xId.Bit.SrcAddr = MONITOR_ADDR;
    xId.Bit.DstAddr = PWR_ADDR;
    xId.Bit.Ptp     = 1;
    xId.Bit.Fix2    = 0x60;
    xId.Bit.Fix3    = 0;
    
    uint32_t ulTmp;
    ulTmp = ucOnOff;    /* TODO: Check me carefully! */
    
    _Data_t xData;
    xData.Err       = 0;
    xData.MsgType   = 0x03;
    xData.ErrType   = 0;
    xData.ValueType = BYTE_ENDIAN_UINT16(0x0030);
    xData.Value     = ulTmp;
    
    CanMsgTx_t xMsg;
    xMsg.StdId = xId.All;
    xMsg.ExtId = xId.All;
    xMsg.IDE   = CAN_ID_EXT;
    xMsg.RTR   = CAN_RTR_DATA;
    xMsg.DLC   = 8;
    memcpy(xMsg.Data, &xData, sizeof(xData));
    CanSend(&xMsg, SEND_DELAY);
    s_ulTxCnt++;
    
    uint8_t r;
    if(xQueueReceive(s_xQueue, &r, pdMS_TO_TICKS(WAIT_MS)) == pdPASS) {
        if (0x43 == r) {
            return STATUS_OK;
        }
    }
    
    s_ulErrCnt++;
    return STATUS_ERR;
}

static Status_t prvGetData(uint16_t usValueType)
{
    Id_t xId;
    xId.Bit.Fix1    = 3;
    xId.Bit.Cnt     = 0;
    xId.Bit.SrcAddr = MONITOR_ADDR;
    xId.Bit.DstAddr = PWR_ADDR;
    xId.Bit.Ptp     = 1;
    xId.Bit.Fix2    = 0x60;
    xId.Bit.Fix3    = 0;
    
    _Data_t xData;
    xData.Err       = 0;
    xData.MsgType   = 0x01;
    xData.ErrType   = 0;
    xData.ValueType = BYTE_ENDIAN_UINT16(usValueType);
    xData.Value     = 0;
    
    CanMsgTx_t xMsg;
    xMsg.StdId = xId.All;
    xMsg.ExtId = xId.All;
    xMsg.IDE   = CAN_ID_EXT;
    xMsg.RTR   = CAN_RTR_DATA;
    xMsg.DLC   = 8;
    memcpy(xMsg.Data, &xData, sizeof(xData));
    CanSend(&xMsg, SEND_DELAY);
    s_ulTxCnt++;

#if 0
    TRACE("TX: 0x%08X - %02X %02X %02X %02X %02X %02X %02X %02X\n", xMsg.ExtId, \
                                                                    xMsg.Data[0], xMsg.Data[1], xMsg.Data[2], xMsg.Data[3], \
                                                                    xMsg.Data[4], xMsg.Data[5], xMsg.Data[6], xMsg.Data[7]);
#endif

    uint8_t r;
    if(xQueueReceive(s_xQueue, &r, pdMS_TO_TICKS(WAIT_MS)) == pdPASS) {
        if (0x41 == r) {
            return STATUS_OK;
        }
    }
    
    s_ulErrCnt++;
    return STATUS_ERR;
}

static void prvCliCmdPwr1SetVol(cli_printf cliprintf, int argc, char** argv)
{
    if (!s_bInit) {
        cliprintf("enable the device first\n");
        return;
    }
    
    if (argc != 2) {
        cliprintf("pwr1_set_vol VOL\n");
        return;
    }
    
    float fV = atof(argv[1]);
    if (STATUS_OK == prvSetVol(fV)) {
        cliprintf("    ok\n");
    }
    else {
        cliprintf("    err\n");
    }
}
CLI_CMD_EXPORT(pwr1_set_vol, set power(9.5kw) voltage, prvCliCmdPwr1SetVol)

static void prvCliCmdPwr1SetVolDef(cli_printf cliprintf, int argc, char** argv)
{
    if (!s_bInit) {
        cliprintf("enable the device first\n");
        return;
    }
    
    if (argc != 2) {
        cliprintf("pwr1_set_vol_def VOL\n");
        return;
    }
    
    float fV = atof(argv[1]);
    if (STATUS_OK == prvSetVolDef(fV)) {
        cliprintf("    ok\n");
    }
    else {
        cliprintf("    err\n");
    }
}
CLI_CMD_EXPORT(pwr1_set_vol_def, set power(9.5kw) default voltage, prvCliCmdPwr1SetVolDef)

static void prvCliCmdPwr1CtrlOn(cli_printf cliprintf, int argc, char** argv)
{
    if (!s_bInit) {
        cliprintf("enable the device first\n");
        return;
    }
    
    if (STATUS_OK == prvSetOnOff(1)) {
        cliprintf("    ok\n");
    }
    else {
        cliprintf("    err\n");
    }
}
CLI_CMD_EXPORT(pwr1_ctrl_on, set power(9.5kw) output on, prvCliCmdPwr1CtrlOn)

static void prvCliCmdPwr1CtrlOff(cli_printf cliprintf, int argc, char** argv)
{
    if (!s_bInit) {
        cliprintf("enable the device first\n");
        return;
    }
    
    if (STATUS_OK == prvSetOnOff(0)) {
        cliprintf("    ok\n");
    }
    else {
        cliprintf("    err\n");
    }
}
CLI_CMD_EXPORT(pwr1_ctrl_off, set power(9.5kw) output off, prvCliCmdPwr1CtrlOff)

static void prvCliCmdPwr1Status(cli_printf cliprintf, int argc, char** argv)
{
    if (!s_bInit) {
        cliprintf("enable the device first\n");
        return;
    }
    
    cliprintf("��ѹ          : %.4f\n", s_fData[0x0001]);
    cliprintf("������ʵֵ    : %.4f\n", s_fData[0x0002]);
    cliprintf("������        : %.4f, %.4f\n", s_fData[0x0003], s_fData[0x0003] * 120/*A*/ * 83/*V*/);
    cliprintf("DCDC�¶�      : %.4f\n", s_fData[0x0004]);
    cliprintf("AC �ۺϵ�ѹ   : %.4f\n", s_fData[0x0005]);
    cliprintf("��ѹ����      : %.4f\n", s_fData[0x0006]);
    cliprintf("������ʾֵ    : %.4f\n", s_fData[0x0007]);
    cliprintf("PFC��ĸ�ߵ�ѹ : %.4f\n", s_fData[0x0008]);
    cliprintf("���µ�        : %.4f\n", s_fData[0x0009]);
    cliprintf("PFC��ĸ�ߵ�ѹ : %.4f\n", s_fData[0x000A]);
    cliprintf("AC-A���ѹ    : %.4f\n", s_fData[0x000C]);
    cliprintf("AC-B���ѹ    : %.4f\n", s_fData[0x000D]);
    cliprintf("AC-C���ѹ    : %.4f\n", s_fData[0x000E]);
    cliprintf("������ѹ  : %.4f\n", s_fData[0x000F]);
    cliprintf("PFC�¶�       : %.4f, %.4f\n", s_fData[0x0010], s_fData[0x0010] + 50);
    cliprintf("��������  : %.4f\n", s_fData[0x0011]); /* NO USE */
    cliprintf("��������  : %.4f\n", s_fData[0x0012]); /* NO USE */
    cliprintf("������ѹ  : %.4f\n", s_fData[0x0013]);
    cliprintf("�޹��ʵ�      : %.4f\n", s_fData[0x0014], s_fData[0x0014] * 120/*A*/ * 83/*V*/);
    cliprintf("��ǰ�澯/״̬ : 0x%08X\n", s_ulData[0x0040]);
    cliprintf("TX  CNT       : %d\n", s_ulTxCnt);
    cliprintf("RX  CNT       : %d\n", s_ulRxCnt);
    cliprintf("ERR CNT       : %d\n", s_ulErrCnt);
}
CLI_CMD_EXPORT(pwr1_status, show power(9.5kw) status, prvCliCmdPwr1Status)

#endif /* PWR1_ENALBE */
