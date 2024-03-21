/*
 * Copyright (c) 2024 Golioth, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
#include <zephyr/modbus/modbus.h>
#include <zephyr/drivers/sensor.h>

#include "qm30vt2.h"

LOG_MODULE_REGISTER(qm30vt2, LOG_LEVEL_DBG);

int qm30vt2_reg_to_value(uint8_t reg_type, uint16_t reg_val, struct sensor_value *val)
{
	uint16_t scaling_factor;
	bool reg_val_is_signed;

	switch (reg_type) {
	/* value = register value ÷ 10000 */
	case QM30VT2_Z_VEL_RMS_IN:
	case QM30VT2_X_VEL_RMS_IN:
	case QM30VT2_Z_VEL_PEAK_IN:
	case QM30VT2_X_VEL_PEAK_IN:
		scaling_factor = 10000;
		reg_val_is_signed = false;
		break;
	/* value = register value ÷ 1000 */
	case QM30VT2_Z_VEL_RMS_MM:
	case QM30VT2_X_VEL_RMS_MM:
	case QM30VT2_Z_ACC_PEAK:
	case QM30VT2_X_ACC_PEAK:
	case QM30VT2_Z_ACC_RMS:
	case QM30VT2_X_ACC_RMS:
	case QM30VT2_Z_ACC_KURT:
	case QM30VT2_X_ACC_KURT:
	case QM30VT2_Z_ACC_CF:
	case QM30VT2_X_ACC_CF:
	case QM30VT2_Z_VEL_PEAK_MM:
	case QM30VT2_X_VEL_PEAK_MM:
	case QM30VT2_Z_ACC_RMS_HF:
	case QM30VT2_X_ACC_RMS_HF:
		scaling_factor = 1000;
		reg_val_is_signed = false;
		break;
	/* value = register value ÷ 100 */
	case QM30VT2_TEMP_F:
	case QM30VT2_TEMP_C:
		scaling_factor = 100;
		reg_val_is_signed = true; /* ONLY temp values are signed */
		break;
	/* value = register value ÷ 10 */
	case QM30VT2_Z_VEL_FREQ:
	case QM30VT2_X_VEL_FREQ:
		scaling_factor = 10;
		reg_val_is_signed = false;
		break;
	default:
		LOG_ERR("Invalid Modbus holding register alias index");
		return -EINVAL;
	}

	if (reg_val_is_signed) {
		val->val1 = ((int16_t)reg_val / scaling_factor);
		val->val2 = ((int16_t)reg_val % scaling_factor) * (1000000 / scaling_factor);
	} else {
		val->val1 = (reg_val / scaling_factor);
		val->val2 = (reg_val % scaling_factor) * (1000000 / scaling_factor);
	}

	return 0;
}

int qm30vt2_read_data(const int iface, uint8_t unit_id, struct qm30vt2_measurement *meas)
{
	int err;
	uint16_t holding_reg[QM30VT2_ALIAS_SIZE] = {0};

	err = modbus_read_holding_regs(iface, unit_id, QM30VT2_ALIAS_BASE_ADDR, holding_reg,
				       ARRAY_SIZE(holding_reg));
	if (err != 0) {
		LOG_ERR("Modbus FC03 failed with %d", err);
		return err;
	}

	/* LOG_HEXDUMP_INF(holding_reg, sizeof(holding_reg), "WR|RD holding register:"); */

	err |= qm30vt2_reg_to_value(QM30VT2_Z_VEL_RMS_IN, holding_reg[QM30VT2_Z_VEL_RMS_IN],
				    &meas->z_vel_rms_in);
	err |= qm30vt2_reg_to_value(QM30VT2_Z_VEL_RMS_MM, holding_reg[QM30VT2_Z_VEL_RMS_MM],
				    &meas->z_vel_rms_mm);
	err |= qm30vt2_reg_to_value(QM30VT2_TEMP_F, holding_reg[QM30VT2_TEMP_F], &meas->temp_f);
	err |= qm30vt2_reg_to_value(QM30VT2_TEMP_C, holding_reg[QM30VT2_TEMP_C], &meas->temp_c);
	err |= qm30vt2_reg_to_value(QM30VT2_X_VEL_RMS_IN, holding_reg[QM30VT2_X_VEL_RMS_IN],
				    &meas->x_vel_rms_in);
	err |= qm30vt2_reg_to_value(QM30VT2_X_VEL_RMS_MM, holding_reg[QM30VT2_X_VEL_RMS_MM],
				    &meas->x_vel_rms_mm);
	err |= qm30vt2_reg_to_value(QM30VT2_Z_ACC_PEAK, holding_reg[QM30VT2_Z_ACC_PEAK],
				    &meas->z_acc_peak);
	err |= qm30vt2_reg_to_value(QM30VT2_X_ACC_PEAK, holding_reg[QM30VT2_X_ACC_PEAK],
				    &meas->x_acc_peak);
	err |= qm30vt2_reg_to_value(QM30VT2_Z_VEL_FREQ, holding_reg[QM30VT2_Z_VEL_FREQ],
				    &meas->z_vel_peak_freq);
	err |= qm30vt2_reg_to_value(QM30VT2_X_VEL_FREQ, holding_reg[QM30VT2_X_VEL_FREQ],
				    &meas->x_vel_peak_freq);
	err |= qm30vt2_reg_to_value(QM30VT2_Z_ACC_RMS, holding_reg[QM30VT2_Z_ACC_RMS],
				    &meas->z_acc_rms);
	err |= qm30vt2_reg_to_value(QM30VT2_X_ACC_RMS, holding_reg[QM30VT2_X_ACC_RMS],
				    &meas->x_acc_rms);
	err |= qm30vt2_reg_to_value(QM30VT2_Z_ACC_KURT, holding_reg[QM30VT2_Z_ACC_KURT],
				    &meas->z_acc_kurt);
	err |= qm30vt2_reg_to_value(QM30VT2_X_ACC_KURT, holding_reg[QM30VT2_X_ACC_KURT],
				    &meas->x_acc_kurt);
	err |= qm30vt2_reg_to_value(QM30VT2_Z_ACC_CF, holding_reg[QM30VT2_Z_ACC_CF],
				    &meas->z_acc_cf);
	err |= qm30vt2_reg_to_value(QM30VT2_X_ACC_CF, holding_reg[QM30VT2_X_ACC_CF],
				    &meas->x_acc_cf);
	err |= qm30vt2_reg_to_value(QM30VT2_Z_VEL_PEAK_IN, holding_reg[QM30VT2_Z_VEL_PEAK_IN],
				    &meas->z_vel_peak_in);
	err |= qm30vt2_reg_to_value(QM30VT2_Z_VEL_PEAK_MM, holding_reg[QM30VT2_Z_VEL_PEAK_MM],
				    &meas->z_vel_peak_mm);
	err |= qm30vt2_reg_to_value(QM30VT2_X_VEL_PEAK_IN, holding_reg[QM30VT2_X_VEL_PEAK_IN],
				    &meas->x_vel_peak_in);
	err |= qm30vt2_reg_to_value(QM30VT2_X_VEL_PEAK_MM, holding_reg[QM30VT2_X_VEL_PEAK_MM],
				    &meas->x_vel_peak_mm);
	err |= qm30vt2_reg_to_value(QM30VT2_Z_ACC_RMS_HF, holding_reg[QM30VT2_Z_ACC_RMS_HF],
				    &meas->z_acc_rms_hf);
	err |= qm30vt2_reg_to_value(QM30VT2_X_ACC_RMS_HF, holding_reg[QM30VT2_X_ACC_RMS_HF],
				    &meas->x_acc_rms_hf);

	return err;
}

void qm30vt2_log_measurements(struct qm30vt2_measurement *meas)
{
	/* Log Temperature measurements */
	LOG_DBG("QM30VT2: Temperature=%.2f °C", sensor_value_to_double(&meas->temp_c));

	/* Log Z-Axis measurements */
	LOG_DBG("QM30VT2: Z-Axis RMS Velocity=%.3f mm/sec",
		sensor_value_to_double(&meas->z_vel_rms_mm));
	LOG_DBG("QM30VT2: Z-Axis Peak Velocity=%.3f mm/sec",
		sensor_value_to_double(&meas->z_vel_peak_mm));
	LOG_DBG("QM30VT2: Z-Axis Peak Velocity Component Frequency=%.1f Hz",
		sensor_value_to_double(&meas->z_vel_peak_freq));
	LOG_DBG("QM30VT2: Z-Axis RMS Acceleration=%.3f G",
		sensor_value_to_double(&meas->z_acc_rms));
	LOG_DBG("QM30VT2: Z-Axis Peak Acceleration=%.3f G",
		sensor_value_to_double(&meas->z_acc_peak));
	LOG_DBG("QM30VT2: Z-Axis Crest Factor=%.3f", sensor_value_to_double(&meas->z_acc_cf));
	LOG_DBG("QM30VT2: Z-Axis Kurtosis=%.3f", sensor_value_to_double(&meas->z_acc_kurt));
	LOG_DBG("QM30VT2: Z-Axis High-Frequency RMS Acceleration=%.3f G",
		sensor_value_to_double(&meas->z_acc_rms_hf));

	/* Log X-Axis measurements */
	LOG_DBG("QM30VT2: X-Axis RMS Velocity=%.3f mm/sec",
		sensor_value_to_double(&meas->x_vel_rms_mm));
	LOG_DBG("QM30VT2: X-Axis Peak Velocity=%.3f mm/sec",
		sensor_value_to_double(&meas->x_vel_peak_mm));
	LOG_DBG("QM30VT2: X-Axis Peak Velocity Component Frequency=%.1f Hz",
		sensor_value_to_double(&meas->x_vel_peak_freq));
	LOG_DBG("QM30VT2: X-Axis RMS Acceleration=%.3f G",
		sensor_value_to_double(&meas->x_acc_rms));
	LOG_DBG("QM30VT2: X-Axis Peak Acceleration=%.3f G",
		sensor_value_to_double(&meas->x_acc_peak));
	LOG_DBG("QM30VT2: X-Axis Crest Factor=%.3f", sensor_value_to_double(&meas->x_acc_cf));
	LOG_DBG("QM30VT2: X-Axis Kurtosis=%.3f", sensor_value_to_double(&meas->x_acc_kurt));
	LOG_DBG("QM30VT2: X-Axis High-Frequency RMS Acceleration=%.3f G",
		sensor_value_to_double(&meas->x_acc_rms_hf));
}
