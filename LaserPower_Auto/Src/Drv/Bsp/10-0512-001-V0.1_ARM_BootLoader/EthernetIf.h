/*
    EthernetIf.h

    Head File for STM32 Ethernet If Module
*/

/* Copyright 2020 Shanghai Master Inc. */

/*
    Modification History
    --------------------
    01a, 20Aug19, Ziv Created
    01b, 20Aug19, Karl Modified
    01c, 18Oct19, Karl Added GetMacAddr function
    01d, 17Mar20, Karl Disabled the EthernetIf module in STM32F103
*/

#ifndef __ETHERNET_IF_H__
#define __ETHERNET_IF_H__

/* Includes */
#include <stm32f1xx_hal.h>
#include "lwip/err.h"
#include "lwip/netif.h"
#include "cmsis_os.h"

#ifdef STM32F107xC

/* Defines */
#define ETH_RMII_ENABLE (1)

/* Functions */
void HAL_ETH_MspInit(ETH_HandleTypeDef* pxEth);
void HAL_ETH_MspDeInit(ETH_HandleTypeDef* pxEth);

err_t ethernetif_init(struct netif *netif);
void ethernetif_input( void const *argument );
void ethernetif_set_link(void const *argument);
void ethernetif_update_config(struct netif *netif);
void ethernetif_notify_conn_changed(struct netif *netif);
uint8_t* GetMacAddr(void);

#endif /* STM32F107xC */

#endif  /* __ETHERNET_IF_H__ */
