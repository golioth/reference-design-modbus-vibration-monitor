/*
 * Copyright (c) 2024 Golioth, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
#include <libostentus.h>

#include "ostentus.h"

LOG_MODULE_REGISTER(ostentus, LOG_LEVEL_DBG);

uint8_t slots_remaining = 0;

bool ostentus_ready(k_timeout_t poll_interval, k_timeout_t timeout)
{
	int err;

	/* Compute the end time from the timeout */
	k_timepoint_t end = sys_timepoint_calc(timeout);

	if (slots_remaining > 0) {
		return true;
	}

	do {
		err = ostentus_fifo_ready(&slots_remaining);
		if (err) {
			LOG_ERR("Error reading Ostentus I2C FIFO remaining slots: %d", err);
		}

		if (slots_remaining > 0) {
			return true;
		}

		k_sleep(poll_interval);
	} while (!sys_timepoint_expired(end));

	return false;
}

int ostentus_clear_memory(void)
{
	if (!ostentus_ready(OSTENTUS_READY_POLL_INTERVAL, OSTENTUS_READY_TIMEOUT)) {
		return -EBUSY;
	}

	int err = clear_memory();
	if (err == 0) {
		slots_remaining--;
	}

	return err;
}

int ostentus_show_splash(void)
{
	if (!ostentus_ready(OSTENTUS_READY_POLL_INTERVAL, OSTENTUS_READY_TIMEOUT)) {
		return -EBUSY;
	}

	int err = show_splash();
	if (err == 0) {
		slots_remaining--;
	}

	return err;
}

int ostentus_update_display(void)
{
	if (!ostentus_ready(OSTENTUS_READY_POLL_INTERVAL, OSTENTUS_READY_TIMEOUT)) {
		return -EBUSY;
	}

	int err = update_display();
	if (err == 0) {
		slots_remaining--;
	}

	return err;
}

int ostentus_update_thickness(uint8_t thickness)
{
	if (!ostentus_ready(OSTENTUS_READY_POLL_INTERVAL, OSTENTUS_READY_TIMEOUT)) {
		return -EBUSY;
	}

	int err = update_thickness(thickness);
	if (err == 0) {
		slots_remaining--;
	}

	return err;
}

int ostentus_update_font(uint8_t font)
{
	if (!ostentus_ready(OSTENTUS_READY_POLL_INTERVAL, OSTENTUS_READY_TIMEOUT)) {
		return -EBUSY;
	}

	int err = update_font(font);
	if (err == 0) {
		slots_remaining--;
	}

	return err;
}

int ostentus_clear_text_buffer(void)
{
	if (!ostentus_ready(OSTENTUS_READY_POLL_INTERVAL, OSTENTUS_READY_TIMEOUT)) {
		return -EBUSY;
	}

	int err = clear_text_buffer();
	if (err == 0) {
		slots_remaining--;
	}

	return err;
}

int ostentus_clear_rectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	if (!ostentus_ready(OSTENTUS_READY_POLL_INTERVAL, OSTENTUS_READY_TIMEOUT)) {
		return -EBUSY;
	}

	int err = clear_rectangle(x, y, w, h);
	if (err == 0) {
		slots_remaining--;
	}

	return err;
}

int ostentus_slide_add(uint8_t id, char *str, uint8_t len)
{
	if (!ostentus_ready(OSTENTUS_READY_POLL_INTERVAL, OSTENTUS_READY_TIMEOUT)) {
		return -EBUSY;
	}

	int err = slide_add(id, str, len);
	if (err == 0) {
		slots_remaining--;
	}

	return err;
}

int ostentus_slide_set(uint8_t id, char *str, uint8_t len)
{
	if (!ostentus_ready(OSTENTUS_READY_POLL_INTERVAL, OSTENTUS_READY_TIMEOUT)) {
		return -EBUSY;
	}

	int err = slide_set(id, str, len);
	if (err == 0) {
		slots_remaining--;
	}

	return err;
}

int ostentus_summary_title(char *str, uint8_t len)
{
	if (!ostentus_ready(OSTENTUS_READY_POLL_INTERVAL, OSTENTUS_READY_TIMEOUT)) {
		return -EBUSY;
	}

	int err = summary_title(str, len);
	if (err == 0) {
		slots_remaining--;
	}

	return err;
}

int ostentus_slideshow(uint32_t setting)
{
	if (!ostentus_ready(OSTENTUS_READY_POLL_INTERVAL, OSTENTUS_READY_TIMEOUT)) {
		return -EBUSY;
	}

	int err = slideshow(setting);
	if (err == 0) {
		slots_remaining--;
	}

	return err;
}

int ostentus_store_text(char *str, uint8_t len)
{
	if (!ostentus_ready(OSTENTUS_READY_POLL_INTERVAL, OSTENTUS_READY_TIMEOUT)) {
		return -EBUSY;
	}

	int err = store_text(str, len);
	if (err == 0) {
		slots_remaining--;
	}

	return err;
}

int ostentus_write_text(uint8_t x, uint8_t y, uint8_t thickness)
{
	if (!ostentus_ready(OSTENTUS_READY_POLL_INTERVAL, OSTENTUS_READY_TIMEOUT)) {
		return -EBUSY;
	}

	int err = write_text(x, y, thickness);
	if (err == 0) {
		slots_remaining--;
	}

	return err;
}

int ostentus_version(char *buf, uint8_t buf_len)
{
	return ostentus_version_get(buf, buf_len);
}

int ostentus_led_bitmask(uint8_t bitmask)
{
	return led_bitmask(bitmask);
}

int ostentus_led_power_set(uint8_t state)
{
	return led_power_set(state);
}

int ostentus_led_battery_set(uint8_t state)
{
	return led_battery_set(state);
}

int ostentus_led_internet_set(uint8_t state)
{
	return led_internet_set(state);
}

int ostentus_led_golioth_set(uint8_t state)
{
	return led_golioth_set(state);
}

int ostentus_led_user_set(uint8_t state)
{
	return led_user_set(state);
}

int ostentus_init(void)
{
	int err;
	char ostentus_version_buf[32];

	/* Clear Ostentus memory */
	err = ostentus_clear_memory();
	if (err) {
		LOG_ERR("Error clearing Ostentus memory: %d", err);
		return err;
	}

	/* Log Ostentus version */
	err = ostentus_version(ostentus_version_buf, sizeof(ostentus_version_buf));
	if (err) {
		LOG_ERR("Error getting Ostentus version: %d", err);
		return err;
	}
	LOG_INF("Ostentus firmware version: %s", ostentus_version_buf);

	/* Update Ostentus LEDS using bitmask (Power On and Battery) */
	err = ostentus_led_bitmask(LED_POW | LED_BAT);
	if (err) {
		LOG_ERR("Error setting Ostentus LEDs: %d", err);
		return err;
	}

	/* Show Golioth Logo on Ostentus ePaper screen */
	err = ostentus_show_splash();
	if (err) {
		LOG_ERR("Error showing Ostentus splash screen: %d", err);
		return err;
	}

	return 0;
}
