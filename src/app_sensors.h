/*
 * Copyright (c) 2022-2024 Golioth, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __APP_SENSORS_H__
#define __APP_SENSORS_H__

/** The `app_sensors.c` file performs the important work of this application
 * which is to read sensor values and report them to the Golioth LightDB Stream
 * as time-series data.
 *
 * https://docs.golioth.io/firmware/zephyr-device-sdk/light-db-stream/
 */

#include <golioth/client.h>

void app_sensors_init(void);
void app_sensors_set_client(struct golioth_client *sensors_client);
void app_sensors_read_and_stream(void);

#define LABEL_TEMP	   "Temperature"
#define LABEL_Z_VEL_RMS	   "Z RMS V"
#define LABEL_X_VEL_RMS	   "X RMS V"
#define LABEL_Z_ACC_PEAK   "Z Peak A"
#define LABEL_X_ACC_PEAK   "X Peak A"
#define LABEL_Z_VEL_FREQ   "Z Peak F"
#define LABEL_X_VEL_FREQ   "X Peak F"
#define LABEL_Z_ACC_RMS	   "Z RMS A"
#define LABEL_X_ACC_RMS	   "X RMS A"
#define LABEL_Z_ACC_KURT   "Z Kurt"
#define LABEL_X_ACC_KURT   "X Kurt"
#define LABEL_Z_ACC_CF	   "Z CF"
#define LABEL_X_ACC_CF	   "X CF"
#define LABEL_Z_VEL_PEAK   "Z Peak Vel"
#define LABEL_X_VEL_PEAK   "X Peak Vel"
#define LABEL_Z_ACC_RMS_HF "Z RMS HF A"
#define LABEL_X_ACC_RMS_HF "X RMS HF A"
#define LABEL_BATTERY	   "Battery"
#define LABEL_FIRMWARE	   "Firmware"
#define SUMMARY_TITLE	   "Modbus RD"

/**
 * Each Ostentus slide needs a unique key. You may add additional slides by
 * inserting elements with the name of your choice to this enum.
 */
typedef enum {
	TEMP_F,
	TEMP_C,
	Z_VEL_RMS_IN,
	Z_VEL_RMS_MM,
	X_VEL_RMS_IN,
	X_VEL_RMS_MM,
	Z_ACC_PEAK,
	X_ACC_PEAK,
	Z_VEL_FREQ,
	X_VEL_FREQ,
	Z_ACC_RMS,
	X_ACC_RMS,
	Z_ACC_KURT,
	X_ACC_KURT,
	Z_ACC_CF,
	X_ACC_CF,
	Z_VEL_PEAK_IN,
	Z_VEL_PEAK_MM,
	X_VEL_PEAK_IN,
	X_VEL_PEAK_MM,
	Z_ACC_RMS_HF,
	X_ACC_RMS_HF,
#ifdef CONFIG_ALUDEL_BATTERY_MONITOR
	BATTERY_V,
	BATTERY_LVL,
#endif
	FIRMWARE
} slide_key;

#endif /* __APP_SENSORS_H__ */
