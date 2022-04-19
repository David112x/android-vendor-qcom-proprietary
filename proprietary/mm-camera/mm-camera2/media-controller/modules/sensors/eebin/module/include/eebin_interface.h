/* eebin_interface.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __EEPROM_BIN_INTERFACE_H__
#define __EEPROM_BIN_INTERFACE_H__
#include <linux/types.h>
#include "eebin_lib.h"

typedef enum {
  EEPROM_BIN_GET_BIN_DATA,
  EEPROM_BIN_GET_NUM_DEV,
  EEPROM_BIN_GET_DEV_DATA,
  EEPROM_BIN_GET_NAME_DATA,
  EEPROM_BIN_GET_LIB_NAME_DATA,
}eebin_ctlcmd_t;

typedef enum {
 EEPROM_BIN_LIB_INVALID = -1,
 EEPROM_BIN_LIB_SENSOR,
 EEPROM_BIN_LIB_EEPROM,
 EEPROM_BIN_LIB_ACTUATOR,
 EEPROM_BIN_LIB_CHROMATIX,
 EEPROM_BIN_LIB_MAX,
}eebin_lib_t;

typedef struct {
  eebin_lib_t type;
  uint32_t num_devs;
}eebin_query_num_t;

typedef struct {
  eebin_lib_t type;
  uint32_t num;
  char *name;
  char *lib_name;
  char *path;
}eebin_query_dev_t;

typedef struct {
  eebin_lib_t type;
  char *name;
  char *lib_name;
  char *path;
}eebin_query_name_t;

typedef struct {
  eebin_ctlcmd_t cmd;
  union {
    eebin_query_num_t q_num;
    eebin_query_dev_t dev_data;
    eebin_query_name_t name_data;
  } ctl;
}eebin_ctl_t;

int32_t eebin_interface_init(void **eebin_hdl);
int32_t eebin_interface_deinit(void *eebin_hdl);
int32_t eebin_interface_control(void *eebin_hdl,
  eebin_ctl_t *bin_ctl);

#endif
