/*
 * Copyright (c) 2022-2024 Golioth, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_sensors, LOG_LEVEL_DBG);

#include <golioth/client.h>
#include <golioth/stream.h>
#include <zcbor_encode.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/modbus/modbus.h>
#include <zephyr/drivers/sensor.h>

#include "app_sensors.h"
#include "qm30vt2.h"

#ifdef CONFIG_LIB_OSTENTUS
#include <libostentus.h>
static const struct device *o_dev = DEVICE_DT_GET_ANY(golioth_ostentus);
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

static int init_modbus_client(void)
{
	const char iface_name[] = {DEVICE_DT_NAME(MODBUS_NODE)};

	client_iface = modbus_iface_get_by_name(iface_name);

	return modbus_init_client(client_iface, client_param);
}

void app_sensors_init(void)
{
#if DT_NODE_HAS_PROP(ZEPHYR_USER_NODE, rs485_8_click_en_gpios)
	/* Set the RS-485 transceiver EN signal */
	if (gpio_pin_configure_dt(&rs485_en, GPIO_OUTPUT_ACTIVE)) {
		LOG_ERR("RS-485 transceiver enable pin configuration failed");
	}
#endif

	if (init_modbus_client()) {
		LOG_ERR("Modbus RTU client initialization failed");
	}
}

/* Callback for LightDB Stream */
static void async_error_handler(struct golioth_client *client, enum golioth_status status,
				const struct golioth_coap_rsp_code *coap_rsp_code, const char *path,
				void *arg)
{
	if (status != GOLIOTH_OK) {
		LOG_ERR("Async task failed: %d", status);
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

	/* Golioth custom hardware for demos */
	IF_ENABLED(CONFIG_ALUDEL_BATTERY_MONITOR, (
		read_and_report_battery(client);
		IF_ENABLED(CONFIG_LIB_OSTENTUS, (
			ostentus_slide_set(o_dev, BATTERY_V, get_batt_v_str(), strlen(get_batt_v_str()));
			ostentus_slide_set(o_dev, BATTERY_LVL, get_batt_lvl_str(),
					     strlen(get_batt_lvl_str()));
		));
	));

	LOG_INF("Reading temperature & vibration data from QM30VT2 sensor");

	err = qm30vt2_read_data(client_iface, qm30vt2_unit_id, &meas);
	if (err) {
		LOG_ERR("Failed to read QM30VT2 sensor values: %d", err);
		return;
	}

	qm30vt2_log_measurements(&meas);

	/* Send sensor data to Golioth */
	if (golioth_client_is_connected(client)) {
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

		/* LOG_DBG("%s", json_buf); */

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
	} else {
		LOG_WRN("Device is not connected to Golioth, unable to send sensor data");
	}

	/* Golioth custom hardware for demos */
	IF_ENABLED(CONFIG_LIB_OSTENTUS, (
		/* Update slide values on Ostentus
		 *  -values should be sent as strings
		 *  -use the enum from app_sensors.h for slide key values
		 */
		snprintk(json_buf, sizeof(json_buf), "%.2f F",
			 sensor_value_to_double(&meas.temp_f));
		ostentus_slide_set(o_dev, TEMP_F, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.2f C",
			 sensor_value_to_double(&meas.temp_c));
		ostentus_slide_set(o_dev, TEMP_C, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.4f in/sec",
			 sensor_value_to_double(&meas.z_vel_rms_in));
		ostentus_slide_set(o_dev, Z_VEL_RMS_IN, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.3f mm/sec",
			 sensor_value_to_double(&meas.z_vel_rms_mm));
		ostentus_slide_set(o_dev, Z_VEL_RMS_MM, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.4f in/sec",
			 sensor_value_to_double(&meas.x_vel_rms_in));
		ostentus_slide_set(o_dev, X_VEL_RMS_IN, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.3f mm/sec",
			 sensor_value_to_double(&meas.x_vel_rms_mm));
		ostentus_slide_set(o_dev, X_VEL_RMS_MM, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.3f G",
			 sensor_value_to_double(&meas.z_acc_peak));
		ostentus_slide_set(o_dev, Z_ACC_PEAK, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.3f G",
			 sensor_value_to_double(&meas.x_acc_peak));
		ostentus_slide_set(o_dev, X_ACC_PEAK, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.1f Hz",
			 sensor_value_to_double(&meas.z_vel_peak_freq));
		ostentus_slide_set(o_dev, Z_VEL_FREQ, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.1f Hz",
			 sensor_value_to_double(&meas.x_vel_peak_freq));
		ostentus_slide_set(o_dev, X_VEL_FREQ, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.3f G",
			 sensor_value_to_double(&meas.z_acc_rms));
		ostentus_slide_set(o_dev, Z_ACC_RMS, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.3f G",
			 sensor_value_to_double(&meas.x_acc_rms));
		ostentus_slide_set(o_dev, X_ACC_RMS, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.3f",
			 sensor_value_to_double(&meas.z_acc_kurt));
		ostentus_slide_set(o_dev, Z_ACC_KURT, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.3f",
			 sensor_value_to_double(&meas.x_acc_kurt));
		ostentus_slide_set(o_dev, X_ACC_KURT, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.3f",
			 sensor_value_to_double(&meas.z_acc_cf));
		ostentus_slide_set(o_dev, Z_ACC_CF, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.3f",
			 sensor_value_to_double(&meas.x_acc_cf));
		ostentus_slide_set(o_dev, X_ACC_CF, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.4f in/sec",
			 sensor_value_to_double(&meas.z_vel_peak_in));
		ostentus_slide_set(o_dev, Z_VEL_PEAK_IN, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.3f mm/sec",
			 sensor_value_to_double(&meas.z_vel_peak_mm));
		ostentus_slide_set(o_dev, Z_VEL_PEAK_MM, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.4f in/sec",
			 sensor_value_to_double(&meas.x_vel_peak_in));
		ostentus_slide_set(o_dev, X_VEL_PEAK_IN, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.3f mm/sec",
			 sensor_value_to_double(&meas.x_vel_peak_mm));
		ostentus_slide_set(o_dev, X_VEL_PEAK_MM, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.3f G",
			 sensor_value_to_double(&meas.z_acc_rms_hf));
		ostentus_slide_set(o_dev, Z_ACC_RMS_HF, json_buf, strlen(json_buf));

		snprintk(json_buf, sizeof(json_buf), "%.3f G",
			 sensor_value_to_double(&meas.x_acc_rms_hf));
		ostentus_slide_set(o_dev, X_ACC_RMS_HF, json_buf, strlen(json_buf));
	));
}

void app_sensors_set_client(struct golioth_client *sensors_client)
{
	client = sensors_client;
}
