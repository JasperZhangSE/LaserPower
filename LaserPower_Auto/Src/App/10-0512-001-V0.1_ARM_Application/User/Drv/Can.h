/*
    Can.h

    Head File for Can Module
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 15Nov23, Karl Created
*/

#ifndef __CAN_H__
#define __CAN_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Types */
typedef CanTxMsgTypeDef CanMsgTx_t;
typedef CanRxMsgTypeDef CanMsgRx_t;

/* Functions */
Status_t DrvCanInit(void);
Status_t DrvCanTerm(void);

Status_t CanSend(IN CanMsgTx_t *pxMsg, uint16_t usWaitMs);
Status_t CanRead(OUT CanMsgRx_t *pxMsg, uint16_t usWaitMs);

void     CanRxNotify(uint32_t ulPwr2Addr, CanMsgRx_t *pxMsg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __CAN_H__ */
