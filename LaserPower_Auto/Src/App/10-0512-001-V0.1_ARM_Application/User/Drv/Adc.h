/*
    Adc.h

    Head File for Adc Module
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 15Nov23, Karl Created
    01b, 23Nov23, Karl Added ADC_CHAN_8
*/

#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Defines */
#define ADC_VREF            3300
#define MVOL_TO_ADC(d)      ((d) * 4096 / ADC_VREF)
#define ADC_TO_MVOL(d)      ((d) * ADC_VREF / 4096)
#define ADC_SMP_PRD         10  /* ms */

/* Types */
typedef enum {
    ADC_CHAN_1 = ADC_CHANNEL_12,
    ADC_CHAN_2 = ADC_CHANNEL_13,
    ADC_CHAN_3 = ADC_CHANNEL_3,
    ADC_CHAN_4 = ADC_CHANNEL_6,
    ADC_CHAN_5 = ADC_CHANNEL_15,
    ADC_CHAN_6 = ADC_CHANNEL_8,
    ADC_CHAN_7 = ADC_CHANNEL_9,
    ADC_CHAN_8 = ADC_CHANNEL_10,
}AdcChan_t;

/* Functions */
Status_t DrvAdcInit(void);
Status_t DrvAdcTerm(void);

uint16_t AdcGet(AdcChan_t xChan);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ADC_H__ */
