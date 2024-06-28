/*
    Pwr1Prot.h

    Head File for Pwr1 Prot Module
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

#ifndef __PWR1_PROT_H__
#define __PWR1_PROT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Functions */
Status_t Pwr1ProtInit(void);
Status_t Pwr1ProtTerm(void);

Status_t Pwr1Update(void);
void     Pwr1CanRxNotify(CanMsgRx_t *pxMsg);

Status_t Pwr1Output(uint8_t ucOnOff);

int32_t  Pwr1DataGet(PwrDataType_t xType);

Status_t Pwr1SetVolDef(float fV);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __PWR1_PROT_H__ */
