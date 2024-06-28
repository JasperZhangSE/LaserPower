/*
    Pwr1Prot.h

    Head File for Pwr1 Prot Module
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 11Nov23, Karl Created
*/

#ifndef __PWR1_PROT_H__
#define __PWR1_PROT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Functions */
Status_t Pwr1ProtInit(void);
Status_t Pwr1ProtTerm(void);

void     Pwr1CanRxNotify(CanMsgRx_t *pxMsg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __PWR1_PROT_H__ */
