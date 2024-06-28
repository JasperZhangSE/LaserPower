/*
    Stc.h

    Head File for Stc Module
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 10Nov23, Karl Created
    01b, 15Nov23, Karl Added StcGetTemp
    01c, 15Nov23, Karl Optimized prvStcTask
    01d, 23Nov23, Karl Added prvGetTemp
    01e, 30Nov23, Karl Added AdcToTemp
    01f, 04Dec23, Karl Added StcGetTempH and StcGetTempL
    01g, 17Jan24, Karl Added StcGetTempHFrom
*/

#ifndef __STC_H__
#define __STC_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Defines */
#define STC_EN_DEV1             1
#define STC_EN_DEV2             1   /* XXX: STC_EN_DEV2 */
#define STC_EN_DEV3             1   /* XXX: STC_EN_DEV3 */
#define STC_QUERY_TASK_DELAY    20
#define STC_QUERY_TEMP_PRD      20
#define STC_QUERY_DIAG_PRD      1000

/* Types */
typedef enum {
    STC_DEV_1,
    STC_DEV_2,
    STC_DEV_3
}StcDev_t;

typedef enum {
    STC_TEMP_NODE_1,
    STC_TEMP_NODE_2,
    STC_TEMP_NODE_3,
    STC_TEMP_NODE_4,
    STC_TEMP_NODE_5,
    STC_TEMP_NODE_6,
    STC_TEMP_NODE_7,
    STC_TEMP_NODE_8,
    STC_TEMP_NODE_9,
    STC_TEMP_NODE_10
}StcTempNode_t;

/* Functions */
Status_t DrvStcInit(void);
Status_t DrvStcTerm(void);

int16_t  StcGetTemp(StcDev_t xDev, StcTempNode_t xTempNode);
/* Get highest temperature */
int16_t  StcGetTempH(void);
/* Get lowest temperature */
int16_t  StcGetTempL(void);
/* Get highest temperature */
int16_t  StcGetTempHFrom(uint8_t st, uint8_t ed);
int16_t  StcGetPdHFrom(void);

int16_t  AdcToTemp(uint16_t usAdc);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __STC_H__ */
