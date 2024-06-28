/*
    Config.h

    Configuration File for Master Application
*/

/* Copyright 2020 Shanghai BiNY Inc. */

/*
    modification history
    --------------------
    01a, 28May20, David Created
    01a, 28May20, Karl Modified
    01c, 17May21, Karl Tailed for Chengdu tunnel project
*/

#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

/* Defines */
#if (BOARD_CONFIG == BOARD_BY_SC2_106)
#define USER_FRAM_CS_PIN                GPIO_PIN_7
#define USER_FRAM_CS_PORT               GPIOD
#define USER_FRAM_SPI_SEL               (SPI3)
#define SPI3_PB3_PB4_PB5_ENABLE         (1)     /* XXX: Include "User/_Config.h" in Src/Drv/Bsp/BootLoader BspSpi.h */
#define USER_FRAM_CAPACITY              (256*1024)
#define USER_FRAM_ADDR_MQTT_PARA        (16*92)
#define USER_FRAM_ADDR_UPGRADE_CONFIG   (16*132)
#define USER_FRAM_ADDR_UPGRADE_ENABLE   (16*142)
#endif /* (BOARD_CONFIG == BOARD_BY_SC2_106) */

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif /* __USER_CONFIG_H__ */
