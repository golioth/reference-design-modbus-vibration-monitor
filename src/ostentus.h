/*
 * Copyright (c) 2024 Golioth, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __OSTENTUS_H__
#define __OSTENTUS_H__

#define OSTENTUS_READY_POLL_INTERVAL K_MSEC(50)
#define OSTENTUS_READY_TIMEOUT	     K_MSEC(1000)

int ostentus_i2c_readbyte(uint8_t reg, uint8_t *value);
int ostentus_i2c_readarray(uint8_t reg, uint8_t *read_reg, uint8_t read_len);
bool ostentus_ready(k_timeout_t poll_interval, k_timeout_t timeout);
int ostentus_clear_memory(void);
int ostentus_show_splash(void);
int ostentus_update_display(void);
int ostentus_update_thickness(uint8_t thickness);
int ostentus_update_font(uint8_t font);
int ostentus_clear_text_buffer(void);
int ostentus_clear_rectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
int ostentus_slide_add(uint8_t id, char *str, uint8_t len);
int ostentus_slide_set(uint8_t id, char *str, uint8_t len);
int ostentus_summary_title(char *str, uint8_t len);
int ostentus_slideshow(uint32_t setting);
int ostentus_store_text(char *str, uint8_t len);
int ostentus_write_text(uint8_t x, uint8_t y, uint8_t thickness);
int ostentus_version(char *buf, uint8_t buf_len);
int ostentus_fifo_ready(uint8_t *slots_remaining);
int ostentus_reset(void);
int ostentus_led_bitmask(uint8_t bitmask);
int ostentus_led_power_set(uint8_t state);
int ostentus_led_battery_set(uint8_t state);
int ostentus_led_internet_set(uint8_t state);
int ostentus_led_golioth_set(uint8_t state);
int ostentus_led_user_set(uint8_t state);

int ostentus_init(void);

#endif /* __OSTENTUS_H__ */
