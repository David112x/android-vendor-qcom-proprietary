/*==========================================================

  Copyright (c) 2014-2015, 2017 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================*/
#ifndef __ACTUATOR_DRIVER_H__
#define __ACTUATOR_DRIVER_H__

#include "sensor_sdk_common.h"

/*
 * Actuator driver version is given by:
 * <Major version>.<Minor version>.<Patch version>
 */
#define ACTUATOR_DRIVER_VERSION "1.0.0"
#define ACTUATOR_SDK_CAPABILITIES "VCM, PIEZO, HVCM, BIVCM"

#define MAX_ACTUATOR_REG_TBL_SIZE 8
#define MAX_ACTUATOR_REGION       5
#define NUM_ACTUATOR_DIR          2
#define MAX_ACTUATOR_SCENARIO     8
#define MAX_ACTUATOR_INIT_SET     120

enum actuator_module_type {
  ACTUATOR_TYPE_VCM,
  ACTUATOR_TYPE_PIEZO,
  ACTUATOR_TYPE_HVCM,
  ACTUATOR_TYPE_BIVCM,
};

enum actuator_i2c_operation {
  ACTUATOR_I2C_OP_WRITE = 0,
  ACTUATOR_I2C_OP_POLL,
};

enum actuator_write_type {
  ACTUATOR_WRITE_HW_DAMP,
  ACTUATOR_WRITE_DAC,
  ACTUATOR_WRITE,
  ACTUATOR_WRITE_DIR_REG,
  ACTUATOR_POLL,
  ACTUATOR_READ_WRITE,
};

struct actuator_reg_settings_t {
  unsigned short reg_addr;
  enum camera_i2c_reg_addr_type addr_type;
  unsigned short reg_data;
  enum camera_i2c_data_type data_type;
  enum actuator_i2c_operation i2c_operation;
  unsigned int delay;
};

struct actuator_reg_params_t {
  enum actuator_write_type reg_write_type;
  unsigned int hw_mask;
  unsigned short reg_addr;
  unsigned short hw_shift;
  unsigned short data_shift;
  unsigned short data_type;
  unsigned short addr_type;
  unsigned short reg_data;
  unsigned short delay;
};

struct actuator_region_params_t {
  /* [0] = ForwardDirection Macro boundary
     [1] = ReverseDirection Inf boundary
  */
  unsigned short step_bound[2];
  unsigned short code_per_step;
  /* qvalue for converting float type numbers to integer format */
  unsigned int qvalue;
};

/**
 * actuator_damping_params_t:
 * @damping_step: Damping step
 * @damping_delay: Damping delay
 * @hw_params: hw params for hw damping
 *
 **/
struct actuator_damping_params_t {
  unsigned int damping_step;
  unsigned int damping_delay;
  unsigned int hw_params;
};

/**
 * actuator_reg_tbl_t:
 * @reg_tbl_size: Table size
 * @reg_params: Register params
 *
 **/
struct actuator_reg_tbl_t {
  unsigned char reg_tbl_size;
  struct actuator_reg_params_t reg_params[MAX_ACTUATOR_REG_TBL_SIZE];
};

/**
 * damping_t:
 * @ringing_params: ringing params in all regions
 **/
struct damping_t {
  struct actuator_damping_params_t ringing_params[MAX_ACTUATOR_REGION];
};

/** _actuator_tuned_params:
 * @scenario_size: Scenario size in both directios.
 * @ringing_scenario: ringing parameters.
 * @initial_code: initial code.
 * @region_size: region size.
 * @region_params: region_params
 * @damping: damping params
 **/
typedef struct _actuator_tuned_params {
  unsigned short scenario_size[NUM_ACTUATOR_DIR];
  unsigned short ringing_scenario[NUM_ACTUATOR_DIR][MAX_ACTUATOR_SCENARIO];
  short initial_code;
  unsigned short region_size;
  struct actuator_region_params_t region_params[MAX_ACTUATOR_REGION];
  struct damping_t damping[NUM_ACTUATOR_DIR][MAX_ACTUATOR_SCENARIO];
} actuator_tuned_params_t;

/**
 * _actuator_params:
 * @module_name: Module name
 * @actuator_name: actuator name
 * @i2c_addr: I2C address of slave
 * @i2c_data_type: data width
 * @i2c_addr_type: address width
 * @data_size: data size
 * @reg_tbl: actuator table info
 * @init_settings: Initial register settings
 * @init_setting_size: initial tabel size
 * @power_on_delay: usec delay after power on
 **/
typedef struct _actuator_params {
  char module_name[NAME_SIZE_MAX];
  char actuator_name[NAME_SIZE_MAX];
  unsigned int i2c_addr;
  enum camera_i2c_freq_mode i2c_freq_mode;
  enum camera_i2c_data_type i2c_data_type;
  enum camera_i2c_reg_addr_type i2c_addr_type;
  enum actuator_module_type act_type;
  unsigned short data_size;
  struct actuator_reg_tbl_t reg_tbl;
  unsigned short init_setting_size;
  struct actuator_reg_settings_t init_settings[MAX_ACTUATOR_INIT_SET];
  unsigned short deinit_setting_size;
  struct actuator_reg_settings_t deinit_settings[MAX_ACTUATOR_INIT_SET];
  unsigned int power_on_delay;
} actuator_params_t;

/**
 * _actuator_driver_params:
 * @actuator_params: parameters specific to actuator
 * @actuator_tuned_params:
**/
typedef struct _actuator_driver_params {
  actuator_params_t actuator_params;
  actuator_tuned_params_t actuator_tuned_params;
} actuator_driver_params_t;

/**
 * _actuator_driver_ctrl:
 * @actuator_driver_params: actuator_driver_params
 **/
typedef struct _actuator_driver_ctrl {
  actuator_driver_params_t actuator_driver_params;
} actuator_driver_ctrl_t;

#endif /* __ACTUATOR_DRIVER_H__ */
