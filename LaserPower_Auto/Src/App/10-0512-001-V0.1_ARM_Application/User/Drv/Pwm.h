
#ifndef __PWM_H__
#define __PWM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "Include/Type.h"
#include "stm32f1xx_hal.h"

#ifndef TIM2_CH1_PWM
#define TIM2_CH1_PWM    1
#endif  /* TIM2_CH1_PWM */
#ifndef TIM4_CH3_PWM
#define TIM4_CH3_PWM    1
#endif  /* TIM4_CH3_PWM */


extern TIM_HandleTypeDef s_hTim;

Status_t DrvPwmInit(void);
Status_t Set_AimLight_Cur(uint16_t light);
Status_t SetADuty(uint16_t Duty);
Status_t ToggleAimLight(uint16_t OnOff);
Status_t ToggleAStatus(uint16_t OnOff);


#ifdef __cplusplus 
}
#endif

#endif /* __TIM_H__ */
