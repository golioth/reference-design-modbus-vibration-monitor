/*
 * Copyright (c) 2022-2024 Golioth, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_sensors, LOG_LEVEL_DBG);

#include <golioth/client.h>
#include <golioth/stream.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/modbus/modbus.h>
#include <zephyr/drivers/sensor.h>

#include "app_sensors.h"
#include "qm30vt2.h"

#ifdef CONFIG_LIB_OSTENTUS
#include <libostentus.h>
#endif

#ifdef CONFIG_ALUDEL_BATTERY_MONITOR
#include "battery_monitor/battery.h"
#endif

#define ZEPHYR_USER_NODE DT_PATH(zephyr_user)
#define MODBUS_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(zephyr_modbus_serial)

/* Formatting string for sending sensor JSON to Golioth */
/* clang-format off */
#define JSON_FMT \
"{" \
	"\"temperature\": {" \
		"\"celcius\":%f," \
		"\"farenheight\":%f" \
	"}," \
	"\"x_axis\": {" \
		"\"acceleration\": {" \
			"\"crest_factor\":%f," \
			"\"high_frequency_rms\":%f," \
			"\"kurtosis\":%f," \
			"\"peak\":%f," \
			"\"rms\":%f" \
		"}," \
		"\"velocity\": {" \
			"\"peak\": {" \
				"\"frequency\":%f," \
				"\"in_per_sec\":%f," \
				"\"mm_per_sec\":%f" \
			"}," \
			"\"rms\": {" \
				"\"in_per_sec\":%f," \
				"\"mm_per_sec\":%f" \
			"}" \
		"}" \
	"}," \
	"\"z_axis\": {" \
		"\"acceleration\": {" \
			"\"crest_factor\":%f," \
			"\"high_frequency_rms\":%f," \
			"\"kurtosis\":%f," \
			"\"peak\":%f," \
			"\"rms\":%f" \
		"}," \
		"\"velocity\": {" \
			"\"peak\": {" \
				"\"frequency\":%f," \
				"\"in_per_sec\":%f," \
				"\"mm_per_sec\":%f" \
			"}," \
			"\"rms\": {" \
				"\"in_per_sec\":%f," \
				"\"mm_per_sec\":%f" \
			"}" \
		"}" \
	"}" \
"}"
/* clang-format on */

static struct golioth_client *client;

#if DT_NODE_HAS_PROP(ZEPHYR_USER_NODE, rs485_8_click_en_gpios)
const struct gpio_dt_spec rs485_en = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, rs485_8_click_en_gpios);
#endif

static int client_iface;

const static struct modbus_iface_param client_param = {
	.mode = MODBUS_MODE_RTU,
	.rx_timeout = 500000,
	/* clang-format off */
	.serial = {
		.baud = 19200,
		.parity = UART_CFG_PARITY_NONE,
		.stop_bits_client = UART_CFG_STOP_BITS_1,
	},
	/* clang-format on */
};

int enable_rs485_transceiver(void)
{
	/* Set the RS-485 transceiver EN signal */
	return gpio_pin_configure_dt(&rs485_en, GPIO_OUTPUT_ACTIVE);
}

int init_modbus_client(void)
{
	const char iface_name[] = {DEVICE_DT_NAME(MODBUS_NODE)};

	client_iface = modbus_iface_get_by_name(iface_name);

	return modbus_init_client(client_iface, client_param);
}

/* Callback for LightDB Stream */

static void async_error_handler(struct golioth_client *client,
				const struct golioth_response *response,
				const char *path,
				void *arg)
{
	if (response->status != GOLIOTH_OK) {
		LOG_ERR("Async task failed: %d", response->status);
		return;
	}
}

/* This will be called by the main() loop */
/* Do all of your work here! */
void app_sensors_read_and_stream(void)
{
	int err;
	char json_buf[1024];
	static uint8_t qm30vt2_unit_id = 1;
	struct qm30vt2_measurement meas = {0};

	IF_ENABLED(CONFIG_ALUDEL_BATTERY_MONITOR, (
		read_and_report_battery(client);
		IF_ENABLED(CONFIG_LIB_OSTENTUS, (
			slide_set(BATTERY_V, get_batt_v_str(), strlen(get_batt_v_str()));
			slide_set(BATTERY_LVL, get_batt_lvl_str(),
					     strlen(get_batt_lvl_str()));
		));
	));

	LOG_INF("Reading temperature & vibration data from QM30VT2 sensor");

	err = qm30vt2_read_data(client_iface, qm30vt2_unit_id, &meas);
	if (err) {
		LOG_ERR("Failed to read QM30VT2 sensor values: %d", err);
		return;
	}

	/* Log Z-Axis measurements */
	LOG_DBG("QM30VT2: Temperature=%.2f °F", sensor_value_to_double(&meas.temp_f));
	LOG_DBG("QM30VT2: Temperature=%.2f °C", sensor_value_to_double(&meas.temp_c));
	LOG_DBG("QM30VT2: Z-Axis RMS Velocity=%.4f in/sec",
		sensor_value_to_double(&meas.z_vel_rms_in));
	LOG_DBG("QM30VT2: Z-Axis Peak Velocity=%.4f in/sec",
		sensor_value_to_double(&meas.z_vel_peak_in));
	LOG_DBG("QM30VT2: Z-Axis RMS Velocity=%.3f mm/sec",
		sensor_value_to_double(&meas.z_vel_rms_mm));
	LOG_DBG("QM30VT2: Z-Axis Peak Velocity=%.3f mm/sec",
		sensor_value_to_double(&meas.z_vel_peak_mm));
	LOG_DBG("QM30VT2: Z-Axis Peak Velocity Component Frequency=%.1f Hz",
		sensor_value_to_double(&meas.z_vel_peak_freq));
	LOG_DBG("QM30VT2: Z-Axis RMS Acceleration=%.3f G", sensor_value_to_double(&meas.z_acc_rms));
	LOG_DBG("QM30VT2: Z-Axis Peak Acceleration=%.3f G",
		sensor_value_to_double(&meas.z_acc_peak));
	LOG_DBG("QM30VT2: Z-Axis Crest Factor=%.3f", sensor_value_to_double(&meas.z_acc_cf));
	LOG_DBG("QM30VT2: Z-Axis Kurtosis=%.3f", sensor_value_to_double(&meas.z_acc_kurt));
	LOG_DBG("QM30VT2: Z-Axis High-Frequency RMS Acceleration=%.3f G",
		sensor_value_to_double(&meas.z_acc_rms_hf));

	/* Log X-Axis measurements */
	LOG_DBG("QM30VT2: X-Axis RMS Velocity=%.4f in/sec",
		sensor_value_to_double(&meas.x_vel_rms_in));
	LOG_DBG("QM30VT2: X-Axis Peak Velocity=%.4f in/sec",
		sensor_value_to_double(&meas.x_vel_peak_in));
	LOG_DBG("QM30VT2: X-Axis RMS Velocity=%.3f mm/sec",
		sensor_value_to_double(&meas.x_vel_rms_mm));
	LOG_DBG("QM30VT2: X-Axis Peak Velocity=%.3f mm/sec",
		sensor_value_to_double(&meas.x_vel_peak_mm));
	LOG_DBG("QM30VT2: X-Axis Peak Velocity Component Frequency=%.1f Hz",
		sensor_value_to_double(&meas.x_vel_peak_freq));
	LOG_DBG("QM30VT2: X-Axis RMS Acceleration=%.3f G", sensor_value_to_double(&meas.x_acc_rms));
	LOG_DBG("QM30VT2: X-Axis Peak Acceleration=%.3f G",
		sensor_value_to_double(&meas.x_acc_peak));
	LOG_DBG("QM30VT2: X-Axis Crest Factor=%.3f", sensor_value_to_double(&meas.x_acc_cf));
	LOG_DBG("QM30VT2: X-Axis Kurtosis=%.3f", sensor_value_to_double(&meas.x_acc_kurt));
	LOG_DBG("QM30VT2: X-Axis High-Frequency RMS Acceleration=%.3f G",
		sensor_value_to_double(&meas.x_acc_rms_hf));

	/* Send sensor data to Golioth */
	/* clang-format off */
	snprintk(json_buf, sizeof(json_buf), JSON_FMT,
		/* Temperature */
		sensor_value_to_double(&meas.temp_c),
		sensor_value_to_double(&meas.temp_f),

		/* X-Axis Vibration */
		sensor_value_to_double(&meas.x_acc_cf),
		sensor_value_to_double(&meas.x_acc_rms_hf),
		sensor_value_to_double(&meas.x_acc_kurt),
		sensor_value_to_double(&meas.x_acc_peak),
		sensor_value_to_double(&meas.x_acc_rms),
		sensor_value_to_double(&meas.x_vel_peak_freq),
		sensor_value_to_double(&meas.x_vel_peak_in),
		sensor_value_to_double(&meas.x_vel_peak_mm),
		sensor_value_to_double(&meas.x_vel_rms_in),
		sensor_value_to_double(&meas.x_vel_rms_mm),

		/* Z-Axis Vibration */
		sensor_value_to_double(&meas.z_acc_cf),
		sensor_value_to_double(&meas.z_acc_rms_hf),
		sensor_value_to_double(&meas.z_acc_kurt),
		sensor_value_to_double(&meas.z_acc_peak),
		sensor_value_to_double(&meas.z_acc_rms),
		sensor_value_to_double(&meas.z_vel_peak_freq),
		sensor_value_to_double(&meas.z_vel_peak_in),
		sensor_value_to_double(&meas.z_vel_peak_mm),
		sensor_value_to_double(&meas.z_vel_rms_in),
		sensor_value_to_double(&meas.z_vel_rms_mm)
	);
	/* clang-format on */

	LOG_DBG("%s", json_buf);

	err = golioth_stream_set_async(client,
				       "sensor",
				       GOLIOTH_CONTENT_TYPE_JSON,
				       json_buf,
				       strlen(json_buf),
				       async_error_handler,
				       NULL);
	if (err) {
		LOG_ERR("Failed to send sensor data to Golioth: %d", err);
	}

	IF_ENABLED(CONFIG_LIB_OSTENTUS, (
		/* Update slide values on Ostentus
		 *  -values should be sent as strings
		 *  -use the enum from app_sensors.h for slide key values
		 */
		snprintk(json_buf, sizeof(json_buf), "%.2f F",
			 sensor_value_to_double(&meas.temp_f));
		slide_set(TEMP_F, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.2f C",
			 sensor_value_to_double(&meas.temp_c));
		slide_set(TEMP_C, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.4f in/sec",
			 sensor_value_to_double(&meas.z_vel_rms_in));
		slide_set(Z_VEL_RMS_IN, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.3f mm/sec",
			 sensor_value_to_double(&meas.z_vel_rms_mm));
		slide_set(Z_VEL_RMS_MM, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.4f in/sec",
			 sensor_value_to_double(&meas.x_vel_rms_in));
		slide_set(X_VEL_RMS_IN, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.3f mm/sec",
			 sensor_value_to_double(&meas.x_vel_rms_mm));
		slide_set(X_VEL_RMS_MM, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.3f G",
			 sensor_value_to_double(&meas.z_acc_peak));
		slide_set(Z_ACC_PEAK, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.3f G",
			 sensor_value_to_double(&meas.x_acc_peak));
		slide_set(X_ACC_PEAK, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.1f Hz",
			 sensor_value_to_double(&meas.z_vel_peak_freq));
		slide_set(Z_VEL_FREQ, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.1f Hz",
			 sensor_value_to_double(&meas.x_vel_peak_freq));
		slide_set(X_VEL_FREQ, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.3f G",
			 sensor_value_to_double(&meas.z_acc_rms));
		slide_set(Z_ACC_RMS, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.3f G",
			 sensor_value_to_double(&meas.x_acc_rms));
		slide_set(X_ACC_RMS, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.3f",
			 sensor_value_to_double(&meas.z_acc_kurt));
		slide_set(Z_ACC_KURT, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.3f",
			 sensor_value_to_double(&meas.x_acc_kurt));
		slide_set(X_ACC_KURT, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.3f",
			 sensor_value_to_double(&meas.z_acc_cf));
		slide_set(Z_ACC_CF, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.3f",
			 sensor_value_to_double(&meas.x_acc_cf));
		slide_set(X_ACC_CF, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.4f in/sec",
			 sensor_value_to_double(&meas.z_vel_peak_in));
		slide_set(Z_VEL_PEAK_IN, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.3f mm/sec",
			 sensor_value_to_double(&meas.z_vel_peak_mm));
		slide_set(Z_VEL_PEAK_MM, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.4f in/sec",
			 sensor_value_to_double(&meas.x_vel_peak_in));
		slide_set(X_VEL_PEAK_IN, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.3f mm/sec",
			 sensor_value_to_double(&meas.x_vel_peak_mm));
		slide_set(X_VEL_PEAK_MM, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.3f G",
			 sensor_value_to_double(&meas.z_acc_rms_hf));
		slide_set(Z_ACC_RMS_HF, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.3f G",
			 sensor_value_to_double(&meas.x_acc_rms_hf));
		slide_set(X_ACC_RMS_HF, json_buf, strlen(json_buf));
	));
}

void app_sensors_init(struct golioth_client *sensors_client)
{
	client = sensors_client;
}
