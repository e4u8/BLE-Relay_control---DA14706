/*
 * platform_devices.h
 *
 *  Created on: Mar 12, 2026
 *      Author: User
 */

#ifndef PLATFORM_DEVICES_H_
#define PLATFORM_DEVICES_H_

#include "ad.h"

/* Relay — MikroBUS 1, PWM pin = P100 */
#define RELAY_PORT    (HW_GPIO_PORT_1)
#define RELAY_PIN     (HW_GPIO_PIN_0)

static const ad_io_conf_t relay_gpio_cfg[] = {
    {
        .port = RELAY_PORT,
        .pin  = RELAY_PIN,
        .on = {
            .mode     = HW_GPIO_MODE_OUTPUT,
            .function = HW_GPIO_FUNC_GPIO,
            .high     = true    /* relay ON  = pin HIGH */
        },
        .off = {
            .mode     = HW_GPIO_MODE_OUTPUT,
            .function = HW_GPIO_FUNC_GPIO,
            .high     = false   /* relay OFF = pin LOW  */
        },
    }
};

#endif /* PLATFORM_DEVICES_H_ */
