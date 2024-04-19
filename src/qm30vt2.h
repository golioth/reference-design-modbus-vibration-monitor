/*
 * Copyright (c) 2024 Golioth, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __QM30VT2_H__
#define __QM30VT2_H__

#include <zephyr/drivers/sensor.h>

/* QM30VT2 Modbus Register Alias Indexes */
#define QM30VT2_Z_VEL_RMS_IN  0U
#define QM30VT2_Z_VEL_RMS_MM  1U
#define QM30VT2_TEMP_F	      2U
#define QM30VT2_TEMP_C	      3U
#define QM30VT2_X_VEL_RMS_IN  4U
#define QM30VT2_X_VEL_RMS_MM  5U
#define QM30VT2_Z_ACC_PEAK    6U
#define QM30VT2_X_ACC_PEAK    7U
#define QM30VT2_Z_VEL_FREQ    8U
#define QM30VT2_X_VEL_FREQ    9U
#define QM30VT2_Z_ACC_RMS     10U
#define QM30VT2_X_ACC_RMS     11U
#define QM30VT2_Z_ACC_KURT    12U
#define QM30VT2_X_ACC_KURT    13U
#define QM30VT2_Z_ACC_CF      14U
#define QM30VT2_X_ACC_CF      15U
#define QM30VT2_Z_VEL_PEAK_IN 16U
#define QM30VT2_Z_VEL_PEAK_MM 17U
#define QM30VT2_X_VEL_PEAK_IN 18U
#define QM30VT2_X_VEL_PEAK_MM 19U
#define QM30VT2_Z_ACC_RMS_HF  20U
#define QM30VT2_X_ACC_RMS_HF  21U

/* QM30VT2 Modbus Register Alias Addresses */
#define QM30VT2_ALIAS_BASE_ADDR	    5200U /* 45201 */
#define QM30VT2_ALIAS_SIZE	    22U
#define QM30VT2_ALIAS_Z_VEL_RMS_IN  QM30VT2_ALIAS_BASE_ADDR + QM30VT2_Z_VEL_RMS_IN  /* 45201 */
#define QM30VT2_ALIAS_Z_VEL_RMS_MM  QM30VT2_ALIAS_BASE_ADDR + QM30VT2_Z_VEL_RMS_IN  /* 45202 */
#define QM30VT2_ALIAS_TEMP_F	    QM30VT2_ALIAS_BASE_ADDR + QM30VT2_TEMP_F	    /* 45203 */
#define QM30VT2_ALIAS_TEMP_C	    QM30VT2_ALIAS_BASE_ADDR + QM30VT2_TEMP_C	    /* 45204 */
#define QM30VT2_ALIAS_X_VEL_RMS_IN  QM30VT2_ALIAS_BASE_ADDR + QM30VT2_X_VEL_RMS_IN  /* 45205 */
#define QM30VT2_ALIAS_X_VEL_RMS_MM  QM30VT2_ALIAS_BASE_ADDR + QM30VT2_X_VEL_RMS_MM  /* 45206 */
#define QM30VT2_ALIAS_Z_ACC_PEAK    QM30VT2_ALIAS_BASE_ADDR + QM30VT2_Z_ACC_PEAK    /* 45207 */
#define QM30VT2_ALIAS_X_ACC_PEAK    QM30VT2_ALIAS_BASE_ADDR + QM30VT2_X_ACC_PEAK    /* 45208 */
#define QM30VT2_ALIAS_Z_VEL_FREQ    QM30VT2_ALIAS_BASE_ADDR + QM30VT2_Z_VEL_FREQ    /* 45209 */
#define QM30VT2_ALIAS_X_VEL_FREQ    QM30VT2_ALIAS_BASE_ADDR + QM30VT2_X_VEL_FREQ    /* 45210 */
#define QM30VT2_ALIAS_Z_ACC_RMS	    QM30VT2_ALIAS_BASE_ADDR + QM30VT2_Z_ACC_RMS	    /* 45211 */
#define QM30VT2_ALIAS_X_ACC_RMS	    QM30VT2_ALIAS_BASE_ADDR + QM30VT2_X_ACC_RMS	    /* 45212 */
#define QM30VT2_ALIAS_Z_ACC_KURT    QM30VT2_ALIAS_BASE_ADDR + QM30VT2_Z_ACC_KURT    /* 45213 */
#define QM30VT2_ALIAS_X_ACC_KURT    QM30VT2_ALIAS_BASE_ADDR + QM30VT2_X_ACC_KURT    /* 45214 */
#define QM30VT2_ALIAS_Z_ACC_CF	    QM30VT2_ALIAS_BASE_ADDR + QM30VT2_Z_ACC_CF	    /* 45215 */
#define QM30VT2_ALIAS_X_ACC_CF	    QM30VT2_ALIAS_BASE_ADDR + QM30VT2_X_ACC_CF	    /* 45216 */
#define QM30VT2_ALIAS_Z_VEL_PEAK_IN QM30VT2_ALIAS_BASE_ADDR + QM30VT2_Z_VEL_PEAK_IN /* 45217 */
#define QM30VT2_ALIAS_Z_VEL_PEAK_MM QM30VT2_ALIAS_BASE_ADDR + QM30VT2_Z_VEL_PEAK_MM /* 45218 */
#define QM30VT2_ALIAS_X_VEL_PEAK_IN QM30VT2_ALIAS_BASE_ADDR + QM30VT2_X_VEL_PEAK_IN /* 45219 */
#define QM30VT2_ALIAS_X_VEL_PEAK_MM QM30VT2_ALIAS_BASE_ADDR + QM30VT2_X_VEL_PEAK_MM /* 45220 */
#define QM30VT2_ALIAS_Z_ACC_RMS_HF  QM30VT2_ALIAS_BASE_ADDR + QM30VT2_Z_ACC_RMS_HF  /* 45221 */
#define QM30VT2_ALIAS_X_ACC_RMS_HF  QM30VT2_ALIAS_BASE_ADDR + QM30VT2_X_ACC_RMS_HF  /* 45222 */

struct qm30vt2_measurement {
	struct sensor_value temp_f;
	struct sensor_value temp_c;
	struct sensor_value z_vel_rms_in;
	struct sensor_value z_vel_rms_mm;
	struct sensor_value x_vel_rms_in;
	struct sensor_value x_vel_rms_mm;
	struct sensor_value z_acc_peak;
	struct sensor_value x_acc_peak;
	struct sensor_value z_vel_peak_freq;
	struct sensor_value x_vel_peak_freq;
	struct sensor_value z_acc_rms;
	struct sensor_value x_acc_rms;
	struct sensor_value z_acc_kurt;
	struct sensor_value x_acc_kurt;
	struct sensor_value z_acc_cf;
	struct sensor_value x_acc_cf;
	struct sensor_value z_vel_peak_in;
	struct sensor_value z_vel_peak_mm;
	struct sensor_value x_vel_peak_in;
	struct sensor_value x_vel_peak_mm;
	struct sensor_value z_acc_rms_hf;
	struct sensor_value x_acc_rms_hf;
};

int qm30vt2_read_data(const int iface, uint8_t unit_id, struct qm30vt2_measurement *meas);
void qm30vt2_log_measurements(struct qm30vt2_measurement *meas);

#endif /* __QM30VT2_H__ */
