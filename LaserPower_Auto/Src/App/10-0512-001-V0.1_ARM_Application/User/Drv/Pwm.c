
//#include "User/Drv/Pwm.h"
#include "include.h"

#undef TRACE
#define TRACE(...) DebugPrintf(__VA_ARGS__)

//static bool s_bSysLed = true;
//uint8_t  breathing_up = 1;
//uint16_t light = 0;

static TIM_HandleTypeDef s_hTim1;
static TIM_HandleTypeDef s_hTim2;

/* Functions */
Status_t DrvPwmInit(void)
{
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};
    
#if TIM2_CH1_PWM
    /* TIM2 CH1 */
    s_hTim1.Instance = TIM2;
    s_hTim1.Init.Prescaler = 18-1;
    s_hTim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    s_hTim1.Init.Period = 100-1;
    s_hTim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_PWM_Init(&s_hTim1);
    /* Disable auto-reload preload */
    //TIM2->CR1 &= ~TIM_CR1_ARPE; // Clear the ARPE bit
      
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&s_hTim1, &sMasterConfig);
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&s_hTim1, &sConfigOC, TIM_CHANNEL_1);
    HAL_TIM_MspPostInit(&s_hTim1);
    HAL_TIM_PWM_Start(&s_hTim1,TIM_CHANNEL_1);
#endif
    
#if TIM4_CH3_PWM
    /* TIM4 CH3 */
    s_hTim2.Instance = TIM4;
    s_hTim2.Init.Prescaler = 720-1;
    s_hTim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    s_hTim2.Init.Period = 10000-1;
    s_hTim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    
    HAL_TIM_PWM_Init(&s_hTim2);

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&s_hTim2, &sMasterConfig);
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&s_hTim2, &sConfigOC, TIM_CHANNEL_3);
    HAL_TIM_MspPostInit(&s_hTim2);
    
    /* ���ʹ������ж� ����Ҫʹ�� IT */
    HAL_TIM_PWM_Start(&s_hTim2,TIM_CHANNEL_3);

#endif
    
    return STATUS_OK;
}

Status_t ToggleAimLight(uint16_t OnOff)
{
    if (OnOff == 1){
        HAL_TIM_PWM_Start(&s_hTim1,TIM_CHANNEL_1);
    }
    else
    {
        HAL_TIM_PWM_Stop(&s_hTim1,TIM_CHANNEL_1);
    }
    return STATUS_OK;
}

Status_t Set_AimLight_Cur(uint16_t light)
{
    __HAL_TIM_SET_COMPARE(&s_hTim1, TIM_CHANNEL_1, light);

    return STATUS_OK;
}

Status_t ToggleAStatus(uint16_t OnOff)
{
    if (OnOff == 1){
        HAL_TIM_PWM_Start(&s_hTim2,TIM_CHANNEL_3);
    }
    else
    {
        HAL_TIM_PWM_Stop(&s_hTim2,TIM_CHANNEL_3);
    }
    return STATUS_OK;
}

Status_t SetADuty(uint16_t Duty)
{
    __HAL_TIM_SET_COMPARE(&s_hTim2, TIM_CHANNEL_3, Duty);
    return STATUS_OK;
}

Status_t SetAFreq(uint16_t Freq)
{
    if (Freq < 10 || Freq > 5000) {
        // ���Ƶ�ʳ�����Χ�����ش���
        return STATUS_ERR;
    }

     uint32_t ulPsc = 72000000 / (Freq * ((TIM4->CCR3) + 1)) - 1;
    __HAL_TIM_SET_PRESCALER(&s_hTim2, ulPsc);
    
    TRACE("%d\n",TIM4->PSC);
    return STATUS_OK;
}

static void prvCliCmdSetAFreq(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    if (argc != 2) {
        cliprintf("pulse_set_freq ON_OFF\n");
        return;
    }
    
    uint32_t usFreq = atoi(argv[1]);
    
    // 10 - 5000 HZ
    
    SetAFreq(usFreq);

}
CLI_CMD_EXPORT(set_a_freq, set Tim4 ch3 pwm freq, prvCliCmdSetAFreq)

#if 0
// ��ʱ�������жϷ������
void TIM4_IRQHandler() {
    // ���ƺ����Ƶ�����
    if (s_bSysLed) {
        if (breathing_up) {
            light++;
            if (light >= 100) {
                breathing_up = 0; // ��ʼ����
            }
        } else {
            light--;
            if (light <= 0) {
                breathing_up = 1; // ��ʼ����
            }
        }
        Set_SysLed_Light(light);
    }
    else
    {
        Set_SysLed_Light(0);
    }
    // ��������жϱ�־
    HAL_TIM_IRQHandler(&s_hTim2);
}

static void prvCliCmdSysLed(cli_printf cliprintf, int argc, char** argv)
{
    CHECK_CLI();
    
    if (argc != 2) {
        cliprintf("sys_led ON_OFF\n");
        return;
    }
    
    uint16_t usSw = atoi(argv[1]);
    
    usSw ? (s_bSysLed = true) : (s_bSysLed = false);
    
}
CLI_CMD_EXPORT(sys_led, enable manual ctrl, prvCliCmdSysLed)
#endif
