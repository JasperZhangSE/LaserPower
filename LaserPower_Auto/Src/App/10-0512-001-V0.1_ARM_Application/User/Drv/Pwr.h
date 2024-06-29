/*
    Pwr.h

    Head File for Pwr Module
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 10Nov23, Karl Created
    01b, 24Nov23, Karl Added pwr1_enable and pwr2_enable
    01c, 24Nov23, Karl Added pwr_a_set_cur
    01d, 30Nov23, Karl Added PwrIsOk function
    01e, 07Dec23, Karl Added PwrOutput function
    01f, 22Dec23, Karl Fixed pwr_a_set_cur bug
    01g, 27Dec23, Karl Added PwrDataGet
    01h, 03Jan24, Karl Modified ADC_TO_VOL definition
    01i, 08Jan24, Karl Added th_AdVolPara in ADC_TO_VOL definition
    01j, 17Jan24, Karl Added PwrSetVolDef
    01k, 20Jan24, Karl Added PWR_STATUS
*/

#ifndef __POWER1_H__
#define __POWER1_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Defines */
#ifndef PWR1_ENABLE
#define PWR1_ENABLE     1
#endif /* PWR1_ENABLE */
#ifndef PWR2_ENABLE
#define PWR2_ENABLE     1
#endif /* PWR2_ENABLE */

#define APWR1_CTRL      DAC_CHAN_1
#define APWR2_CTRL      DAC_CHAN_2
#define APWR3_CTRL      DAC_CHAN_3

#define AUX1_CS         ADC_CHAN_1
#define AUX1_VS         ADC_CHAN_2
#define AUX2_CS         ADC_CHAN_3
#define AUX2_VS         ADC_CHAN_4
#define AUX3_CS         ADC_CHAN_5
#define AUX3_VS         ADC_CHAN_6
#define PD_VS           ADC_CHAN_7

#define APWR1_CUR       ADC_CHAN_1
#define APWR1_VOL       ADC_CHAN_2
#define APWR2_CUR       ADC_CHAN_3
#define APWR2_VOL       ADC_CHAN_4
#define APWR3_CUR       ADC_CHAN_5
#define APWR3_VOL       ADC_CHAN_6

#define DAC_TO_CUR(d)   ((d) * DAC_VREF * 50./*A*/ / 4096 / 2500/*mV*/)
#define ADC_TO_CUR(d)   ((d) * ADC_VREF * 50./*A*/ / 4096 / 2500/*mV*/)

#define ADC_TO_VOL(d,a)   (PwrDataGet(a, PWR_OUTPUT_VOL) * 0.1/*V*/ - ((d) * ADC_VREF / 4096 * (th_AdVolPara * 0.001) * 0.001/*V*/))

#define CUR_TO_DAC(d)   ((d) * 4096 * 2500/*mV*/ / DAC_VREF / 50/*A*/)
#define CUR_TO_ADC(d)   ((d) * 4096 * 2500/*mV*/ / ADC_VREF / 50/*A*/)

/* Types */
typedef enum {
    PWR_OUTPUT_VOL,
    PWR_STATUS,
}PwrDataType_t;

/* Functions */
Status_t DrvPwr1Enable(void);
Status_t DrvPwr2Enable(void);

Status_t DrvPwrInit(void);
Status_t DrvPwrTerm(void);

bool     PwrIsOk(void);
Status_t PwrOutput(uint8_t ucOnOff);

int32_t  PwrDataGet(uint32_t ulPwr2Addr, PwrDataType_t xType);

Status_t PwrSetVolDef(float fV);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __POWER1_H__ */
