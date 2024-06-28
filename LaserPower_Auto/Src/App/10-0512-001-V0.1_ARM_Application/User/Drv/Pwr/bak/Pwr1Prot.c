/*
    Pwr1Prot.c

    Implementation File for Pwr1 Prot Module
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 11Nov23, Karl Created
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

/* Local defines */
#define PWR_ADDR                1
#define SEND_DELAY              5
#define BYTE_ENDIAN_UINT16(a)   (((a) && 0xFF) << 8) | (((a) && 0xFF00) >> 8)
#define BYTE_ENDIAN_UINT32(a)   (((a) & 0xFF) << 24) | (((a) & 0xFF00) << 8) | (((a) & 0xFF0000) >> 8) | (((a) & 0xFF000000) >> 24)

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
}Data_t;
#pragma pack(pop)

/* Forward declarations */
static Status_t prvProcRequestByteDataResp(Data_t xData);
static Status_t prvProcConfigByteDataResp(Data_t xData);
static Status_t prvSetVol(float fV);
static Status_t prvSetVolDef(float fV);
static Status_t prvSetOnOff(uint8_t ucOnOff);
static Status_t prvGetData(uint16_t usValueType);

/* Local variables */
static float         s_fData[65];
static uint32_t      s_ulData[65];
static QueueHandle_t s_xQueue = NULL;

/* Functions */
Status_t Pwr1ProtInit(void)
{
    memset(s_fData, 0, sizeof(s_fData));
    memset(s_ulData, 0, sizeof(s_ulData));
    s_xQueue = xQueueCreate(1/*Queue length*/, 1/*Queue item size*/);
    return STATUS_OK;
}

Status_t Pwr1ProtTerm(void)
{
    /* Do nothing */
    return STATUS_OK;
}

void Pwr2CanRxNotify(CanMsgRx_t *pxMsg)
{
    if (pxMsg->IDE != CAN_ID_EXT) {
        return;
    }
    
    if (pxMsg->DLC != 8) {
        return;
    }
    
    Data_t xData;
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

static Status_t prvProcRequestByteDataResp(Data_t xData)
{
    /* TODO: Check me carefully! */
    uint32_t ulValue = BYTE_ENDIAN_UINT32(xData.Value);
    switch (BYTE_ENDIAN_UINT16(xData.ValueType)) {
    case 0x0001:  /* 取模块电压              (4B float) */
    case 0x0002:  /* 取模块电流真实值        (4B float) */
    case 0x0003:  /* 取模块限流点            (4B float) */
    case 0x0004:  /* 取模块DCDC温度          (4B float) */
    case 0x0005:  /* 取模块AC综合电压        (4B float) */
    case 0x0006:  /* 取模块电压上限          (4B float) */
    case 0x0007:  /* 取模块电流显示值        (4B float) */
    case 0x0008:  /* 取模块PFC正母线电压     (4B float) */
    case 0x0009:  /* 取模块过温点            (4B float) */
    case 0x000A:  /* 取模块PFC负母线电压     (4B float) */
    case 0x000C:  /* 取模块AC A相电压        (4B float) */
    case 0x000D:  /* 取模块AC B相电压        (4B float) */
    case 0x000E:  /* 取模块AC C相电压        (4B float) */
    case 0x000F:  /* 取模块额定/缺省输出电压 (4B float) */
    case 0x0010:  /* 取模块PFC温度           (4B float) */
    case 0x0011:  /* 取模块额定输出功率      (4B float) */
    case 0x0012:  /* 取模块额定输出电流      (4B float) */
    case 0x0013:  /* 取模块额定输入电压      (4B float) */
    case 0x0014:  /* 取模块限功率点          (1B integer) */
    case 0x0040:  /* 读取当前告警/状态       (4B integer) */
        memcpy(&s_fData[xData.Value], &ulValue, sizeof(ulValue));
        memcpy(&s_ulData[xData.Value], &ulValue, sizeof(ulValue));
        break;
    default:
        /* Do nothing */
        break;
    }
    
    /* TODO: Check me carefully! */
    return STATUS_OK;
}

static Status_t prvProcConfigByteDataResp(Data_t xData)
{
    /* TODO: Check me carefully! */
    return STATUS_OK;
}

static Status_t prvSetVol(float fV)
{
    Id_t xId;
    xId.Bit.Fix1    = 3;
    xId.Bit.Cnt     = 0;
    xId.Bit.SrcAddr = 0;
    xId.Bit.DstAddr = PWR_ADDR;
    xId.Bit.Ptp     = 1;
    xId.Bit.Fix2    = 0x60;
    xId.Bit.Fix3    = 0;
    
    uint32_t ulTmp;
    memcpy(&ulTmp, &fV, sizeof(ulTmp));
    ulTmp = BYTE_ENDIAN_UINT32(ulTmp);
    
    Data_t xData;
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
    
    uint8_t r;
    if(xQueueReceive(s_xQueue, &r, pdMS_TO_TICKS(10)) == pdPASS) {
        if (0x43 == r) {
            return STATUS_OK;
        }
    }
        
    return STATUS_ERR;
}

static Status_t prvSetVolDef(float fV)
{
    Id_t xId;
    xId.Bit.Fix1    = 3;
    xId.Bit.Cnt     = 0;
    xId.Bit.SrcAddr = 0;
    xId.Bit.DstAddr = PWR_ADDR;
    xId.Bit.Ptp     = 1;
    xId.Bit.Fix2    = 0x60;
    xId.Bit.Fix3    = 0;
    
    uint32_t ulTmp;
    memcpy(&ulTmp, &fV, sizeof(ulTmp));
    ulTmp = BYTE_ENDIAN_UINT32(ulTmp);
    
    Data_t xData;
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
    
    uint8_t r;
    if(xQueueReceive(s_xQueue, &r, pdMS_TO_TICKS(10)) == pdPASS) {
        if (0x43 == r) {
            return STATUS_OK;
        }
    }
        
    return STATUS_ERR;
}

static Status_t prvSetOnOff(uint8_t ucOnOff)
{
    Id_t xId;
    xId.Bit.Fix1    = 3;
    xId.Bit.Cnt     = 0;
    xId.Bit.SrcAddr = 0;
    xId.Bit.DstAddr = PWR_ADDR;
    xId.Bit.Ptp     = 1;
    xId.Bit.Fix2    = 0x60;
    xId.Bit.Fix3    = 0;
    
    uint32_t ulTmp;
    ulTmp = ucOnOff;    /* TODO: Check me carefully! */
    
    Data_t xData;
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
    
    uint8_t r;
    if(xQueueReceive(s_xQueue, &r, pdMS_TO_TICKS(10)) == pdPASS) {
        if (0x43 == r) {
            return STATUS_OK;
        }
    }
        
    return STATUS_ERR;
}

static Status_t prvGetData(uint16_t usValueType)
{
    Id_t xId;
    xId.Bit.Fix1    = 3;
    xId.Bit.Cnt     = 0;
    xId.Bit.SrcAddr = 0;
    xId.Bit.DstAddr = PWR_ADDR;
    xId.Bit.Ptp     = 1;
    xId.Bit.Fix2    = 0x60;
    xId.Bit.Fix3    = 0;
    
    Data_t xData;
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
    
    uint8_t r;
    if(xQueueReceive(s_xQueue, &r, pdMS_TO_TICKS(10)) == pdPASS) {
        if (0x41 == r) {
            return STATUS_OK;
        }
    }
        
    return STATUS_ERR;
}

static void prvCliCmdPwr1SetVol(cli_printf cliprintf, int argc, char** argv)
{
    if (argc != 2) {
        cliprintf("pwr1_set_vol VOL\n");
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
    if (argc != 2) {
        cliprintf("pwr1_set_vol_def VOL\n");
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
    cliprintf("电压          : %.4f\n", s_fData[0x0001]);
    cliprintf("电流真实值    : %.4f\n", s_fData[0x0002]);
    cliprintf("限流点        : %.4f\n", s_fData[0x0003]);
    cliprintf("DCDC温度      : %.4f\n", s_fData[0x0004]);
    cliprintf("AC 综合电压   : %.4f\n", s_fData[0x0005]);
    cliprintf("电压上限      : %.4f\n", s_fData[0x0006]);
    cliprintf("电流显示值    : %.4f\n", s_fData[0x0007]);
    cliprintf("PFC正母线电压 : %.4f\n", s_fData[0x0008]);
    cliprintf("过温点        : %.4f\n", s_fData[0x0009]);
    cliprintf("PFC负母线电压 : %.4f\n", s_fData[0x000A]);
    cliprintf("AC-A相电压    : %.4f\n", s_fData[0x000C]);
    cliprintf("AC-B相电压    : %.4f\n", s_fData[0x000D]);
    cliprintf("AC-C相电压    : %.4f\n", s_fData[0x000E]);
    cliprintf("额定输出电压  : %.4f\n", s_fData[0x000F]);
    cliprintf("PFC温度       : %.4f\n", s_fData[0x0010]);
    cliprintf("额定输出功率  : %.4f\n", s_fData[0x0011]);
    cliprintf("额定输出电流  : %.4f\n", s_fData[0x0012]);
    cliprintf("额定输入电压  : %.4f\n", s_fData[0x0013]);
    cliprintf("限功率点      : %.4f\n", s_fData[0x0014]);
    cliprintf("前告警/状态   : %4d \n", s_ulData[0x0040]);
}
CLI_CMD_EXPORT(pwr1_status, show power(9.5kw) status, prvCliCmdPwr1Status)
