/*
    Dac.h

    Head File for Dac Module
*/

/* Copyright 2023 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 15Nov23, Karl Created
    01b, 22Nov23, Karl Added DacGet function
    01c, 22Nov23, Karl Added MVOL_TO_DAC and DAC_TO_MVOL
*/

#ifndef __DAC_H__
#define __DAC_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Defines */
#define DAC_VREF            3300
#define MVOL_TO_DAC(d)      ((d) * 4096 / DAC_VREF)
#define DAC_TO_MVOL(d)      ((d) * DAC_VREF / 4096)

#define DAC2_ENABLE         0

/* Types */
typedef enum {
    DAC_CHAN_1 = 1, /* Onchip DAC channel 1 */
    DAC_CHAN_2,     /* Onchip DAC channel 1 */
    DAC_CHAN_3      /* Onchip DAC channel 2 */
}DacChan_t;

/* Functions */
Status_t DrvDacInit(void);
Status_t DrvDacTerm(void);

Status_t DacSet(DacChan_t xChan, uint16_t usData);
uint16_t DacGet(DacChan_t xChan);
uint32_t DacGetValue(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DAC_H__ */
