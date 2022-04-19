/* Copyright (c) 2015-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc. */

/*
 * eeprom_dump.h
 * This is the utility file to collect the eeprom data dump in text file
 * in /data/vendor/camera/ folder of the device.
 */

#include "sensor_common.h"

#define DUMP_NAME_SIZE 64
#define MESH_ROLLOFF_ROW_SIZE 13
#define MESH_ROLLOFF_COL_SIZE 17
#define RELATIVE_ROTATION_MATRIX_MAX 9
#define RELATIVE_GEOMETRIC_SURFACE_PARAMS_MAX 32

typedef enum {
  EEPROM_DUMP_WB,
  EEPROM_DUMP_LSC,
  EEPROM_DUMP_AF,
  EEPROM_DUMP_PDAF,
  EEPROM_DUMP_DPC,
  EEPROM_DUMP_DUALC,
  EEPROM_DUMP_KBUF,
  EEPROM_DUMP_OTP,
  EEPROM_DUMP_CALIB,
  EEPROM_DUMP_MAX
}eeprom_dump_type;

void eeprom_dbg_data_dump(void *dump, void* data, eeprom_dump_type dump_type);

void dump_otp_data(sensor_eeprom_data_t* e_ctrl);
void dump_wbc_data(sensor_eeprom_data_t* e_ctrl, eeprom_dump_type dump_type);
void dump_lsc_data(sensor_eeprom_data_t* e_ctrl, eeprom_dump_type dump_type);
void dump_af_data(sensor_eeprom_data_t* e_ctrl, eeprom_dump_type dump_type);
void dump_pdaf_data(sensor_eeprom_data_t* e_ctrl, eeprom_dump_type dump_type);
void dump_dpc_data(sensor_eeprom_data_t* e_ctrl, eeprom_dump_type dump_type);
void dump_dualc_data(sensor_eeprom_data_t* e_ctrl, eeprom_dump_type dump_type);
void dump_kbuffer_data(eeprom_params_t* e_params, eeprom_dump_type dump_type);
void dump_wbc_calib_data(sensor_eeprom_data_t* e_ctrl, void* data,
  eeprom_dump_type dump_type);
void dump_lsc_calib_data(sensor_eeprom_data_t* e_ctrl, void* data,
  eeprom_dump_type dump_type);
