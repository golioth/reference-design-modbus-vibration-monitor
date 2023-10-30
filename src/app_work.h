/*
 * Copyright (c) 2022-2023 Golioth, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __APP_WORK_H__
#define __APP_WORK_H__

/** The `app_work.c` file performs the important work of this application which
 * is to read sensor values and report them to the Golioth LightDB Stream as
 * time-series data.
 *
 * https://docs.golioth.io/firmware/zephyr-device-sdk/light-db-stream/
 */

#include <net/golioth/system_client.h>

void app_work_init(struct golioth_client *work_client);
void app_work_sensor_read(void);

#define LABEL_CO2	  "CO2"
#define LABEL_TEMPERATURE "Temperature"
#define LABEL_HUMIDITY	  "Humidity"
#define LABEL_BATTERY	  "Battery"
#define LABEL_FIRMWARE	  "Firmware"
#define SUMMARY_TITLE	  "CO2 Monitor"

/**
 * Each Ostentus slide needs a unique key. You may add additional slides by
 * inserting elements with the name of your choice to this enum.
 */
typedef enum {
	CO2,
	TEMPERATURE,
	HUMIDITY,
#ifdef CONFIG_ALUDEL_BATTERY_MONITOR
	BATTERY_V,
	BATTERY_LVL,
#endif
	FIRMWARE
} slide_key;

#endif /* __APP_WORK_H__ */
