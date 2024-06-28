/*
    Main.c

    Implementation File for Main Module
*/

/* Copyright 2019 Shanghai Master Inc. */

/*
    modification history
    --------------------
    01a, 20Aug19, Karl Created
*/

/* Includes */
#include "Include.h"

/* Forward declaration */
void SystemClock_Config(void);

int main(void)
{
    /* MCU Configuration */
    /* Reset of all peripherals, Initializes the Flash interface and the Systick */
    HAL_Init();
    /* Configure the system clock */
    SystemClock_Config();
    
    /* Start boot process */
    BootRun();
    
    /* We should never get here! */
    return 0;
}
