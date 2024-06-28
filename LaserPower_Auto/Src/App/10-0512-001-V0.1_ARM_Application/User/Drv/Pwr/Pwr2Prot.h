/*
    Pwr2Prot.h

    Head File for Pwr2 Prot Module
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
*/

#ifndef __PWR2_PROT_H__
#define __PWR2_PROT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define PWR_T_ADDR          1
#define PWR_M_ADDR          2
#define PWR_B_ADDR          3

/* Functions */
Status_t Pwr2ProtInit(void);
Status_t Pwr2ProtTerm(void);

Status_t Pwr2Update(void);
void     Pwr2CanRxNotify(uint32_t ulPwr2Addr, CanMsgRx_t *pxMsg);

Status_t Pwr2Output(uint32_t ulPwr2Addr, uint8_t ucOnOff);

int32_t  Pwr2DataGet(uint32_t ulAddr, PwrDataType_t xType);

Status_t Pwr2SetVolDef(float fV);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __PWR2_PROT_H__ */
