/*
    Pwr2Prot.h

    Head File for Pwr2 Prot Module
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 20Nov23, Karl Created
*/

#ifndef __PWR2_PROT_H__
#define __PWR2_PROT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Functions */
Status_t Pwr2ProtInit(void);
Status_t Pwr2ProtTerm(void);

void     Pwr2CanRxNotify(CanMsgRx_t *pxMsg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __PWR2_PROT_H__ */
