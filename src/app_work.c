/*
 * Copyright (c) 2022-2023 Golioth, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_work, LOG_LEVEL_DBG);

#include <net/golioth/system_client.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/modbus/modbus.h>

#include "app_work.h"

#ifdef CONFIG_LIB_OSTENTUS
#include <libostentus.h>
#endif

#ifdef CONFIG_ALUDEL_BATTERY_MONITOR
#include "battery_monitor/battery.h"
#endif

#define ZEPHYR_USER_NODE DT_PATH(zephyr_user)
#define MODBUS_NODE	 DT_COMPAT_GET_ANY_STATUS_OKAY(zephyr_modbus_serial)

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

/* Formatting string for sending sensor JSON to Golioth */
#define JSON_FMT "{\"co2\":%f,\"tem\":%f,\"hum\":%f}"

static int init_modbus_client(void)
{
	const char iface_name[] = {DEVICE_DT_NAME(MODBUS_NODE)};

	client_iface = modbus_iface_get_by_name(iface_name);

	return modbus_init_client(client_iface, client_param);
}

/* Callback for LightDB Stream */
static int async_error_handler(struct golioth_req_rsp *rsp)
{
	if (rsp->err) {
		LOG_ERR("Async task failed: %d", rsp->err);
		return rsp->err;
	}
	return 0;
}

static uint32_t words_to_uint32_t(const uint16_t *words)
{
	return (uint32_t)words[0] << 16 | (uint32_t)words[1];
}

static float words_to_float(const uint16_t *words)
{
	uint32_t tmp;

	tmp = words_to_uint32_t(words);
	return *(float *)&tmp;
}

/* This will be called by the main() loop */
/* Do all of your work here! */
void app_work_sensor_read(void)
{
	int err;
	char json_buf[256];
	static uint8_t node = 0x61;
	uint16_t holding_reg[6] = {0};

	IF_ENABLED(CONFIG_ALUDEL_BATTERY_MONITOR, (
		read_and_report_battery();
		IF_ENABLED(CONFIG_LIB_OSTENTUS, (
			slide_set(BATTERY_V, get_batt_v_str(), strlen(get_batt_v_str()));
			slide_set(BATTERY_LVL, get_batt_lvl_str(), strlen(get_batt_lvl_str()));
		));
	));

	err = modbus_read_holding_regs(client_iface, node, 0x0028, holding_reg,
				       ARRAY_SIZE(holding_reg));
	if (err != 0) {
		LOG_ERR("FC03 failed with %d", err);
	}
	/* LOG_HEXDUMP_INF(holding_reg, sizeof(holding_reg), "WR|RD holding register:"); */

	float co2 = words_to_float(&holding_reg[0]);
	float tem = words_to_float(&holding_reg[2]);
	float hum = words_to_float(&holding_reg[4]);

	LOG_DBG("SCD30: CO₂=%.2f ppm, Temperature=%.2f °C, Humidity=%.2f%% RH", co2, tem, hum);

	/* Send sensor data to Golioth */
	snprintk(json_buf, sizeof(json_buf), JSON_FMT, co2, tem, hum);
	/* LOG_DBG("%s", json_buf); */

	err = golioth_stream_push_cb(client, "sensor", GOLIOTH_CONTENT_FORMAT_APP_JSON, json_buf,
				     strlen(json_buf), async_error_handler, NULL);
	if (err) {
		LOG_ERR("Failed to send sensor data to Golioth: %d", err);
	}

	IF_ENABLED(CONFIG_LIB_OSTENTUS, (
		/* Update slide values on Ostentus
		 *  - values should be sent as strings
		 *  - use the enum from app_work.h for slide key values
		 */
		snprintk(json_buf, sizeof(json_buf), "%.2f ppm", co2);
		slide_set(CO2, json_buf, strlen(json_buf));
		snprintk(json_buf, sizeof(json_buf), "%.2f °C", tem);
		slide_set(TEMPERATURE, json_buf, strlen(json_buf));
		snprintk(json_buf, sizeof(json_buf), "%.2f%% RH", tem);
		slide_set(HUMIDITY, json_buf, strlen(json_buf));
	));
}

void app_work_init(struct golioth_client *work_client)
{
	client = work_client;

#if DT_NODE_HAS_PROP(ZEPHYR_USER_NODE, rs485_8_click_en_gpios)
	/* Set the RS485 8 Click board EN signal */
	gpio_pin_configure_dt(&rs485_en, GPIO_OUTPUT_ACTIVE);
#endif

	if (init_modbus_client()) {
		LOG_ERR("Modbus RTU client initialization failed");
	}
}
