/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef __CHROMATIX_MANAGER_H__
#define __CHROMATIX_MANAGER_H__

#include "chromatix_hash.h"
#include "chromatix_lru.h"
#include "sensor_lib.h"
#include "eeprom_lib.h"
#include "camera_config.h"

#define MAX_CHROMATIX_COUNT 40

typedef struct {
  hash_type             hash;
  chromatix_lru_type    lru;

  sensor_eeprom_data_t *eeprom_ctrl;
  void                 *eeprom_func;     /* sensor_func_tbl_t */

  void                 *eebin_hdl;

  pthread_mutex_t       mutex;
  boolean               mutex_created;
} chromatix_manager_type;


/** cm_create
 *    @cm: chromatix manager type
 *    @sensor_name: probed sensor name
 *    @chromatix_array: chromatix name info in sensor driver
 *    @eeprom_ctrl: sensor_eeprom_data_t for calibration
 *    @eebin_hdl: multimodule handle
 *
 *  Create hash, parse sensor driver's chromatix name info,
 *  open chromatix libraries and keep the loaded lib in hash.
 *
 * Return: TRUE/FALSE
 **/
boolean cm_create(chromatix_manager_type* cm,
  const char *sensor_name,
  module_chromatix_info_t *chromatix_array,
  sensor_eeprom_data_t *eeprom_ctrl, void *eebin_hdl);

/** cm_destroy
 *    @cm: chromatix manager type
 *
 *  Delete hash, unload all chromatix libraries in hash.
 */
void cm_destroy(chromatix_manager_type *cm);

/** cm_getChromatix
 *    @cm: chromatix manager type
 *    @name: chromatix file name
 *    @calibration_type: EEPROM_CALIBRATE_LSC/WB_GREEN/WB
 *
 *  Get a chromatix file symbol.
 *
 *  Return: the pointer of loaded chromatix file symbol
 **/
void *cm_getChromatix(chromatix_manager_type* cm, const char *name,
  uint32_t calibration_type);

#endif
