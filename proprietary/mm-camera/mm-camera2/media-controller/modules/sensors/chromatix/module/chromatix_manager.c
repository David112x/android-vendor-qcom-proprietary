/*
 * Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include "chromatix_manager.h"
#include "sensor_common.h"
#include "eebin_interface.h"

/* load chromatix file and add it into the hash */
static boolean addLib(chromatix_manager_type* cm, const char *key,
  int32_t cal_type);
/* load chromatix file, add it into the hash
   and return chromatix symbol pointer */
static void* addLib_getSymbol(chromatix_manager_type* cm, const char *key,
  int32_t cal_type);

static eebin_ctl_t bin_ctl;

#define CHECK_RETURN(ret) \
  if (ret == FALSE) { SERR("failed"); goto ERROR; }

boolean cm_create(chromatix_manager_type* cm,
  const char *sensor_name,
  module_chromatix_info_t *chromatix_array,
  sensor_eeprom_data_t *eeprom_ctrl, void *eebin_hdl)
{
  module_chromatix_name_t *chromatix_name;
  uint32_t                 i;
  boolean                  rc;

  RETURN_ON_NULL(cm);
  RETURN_ON_NULL(sensor_name);
  /* chromatix_array can be NULL */
  /* eeprom_ctrl can be NULL */
  /* eebin_hdl can be NULL */

  SLOW("Enter for %s", sensor_name);

  if (chromatix_array->size == 0) {
    SERR("YUV sensor : no chromatix loading");
    return TRUE;
  }

  if (hash_create(&cm->hash, 0) == FALSE) {
    SERR("failed to create hash");
    return FALSE;
  }

  if (lru_create(&cm->lru, MAX_CHROMATIX_COUNT) == FALSE) {
    SERR("failed to create LRU");
    return FALSE;
  }

  pthread_mutex_init(&cm->mutex, NULL);
  cm->mutex_created = TRUE;

  cm->eeprom_ctrl = eeprom_ctrl;
  if (eeprom_ctrl) {
    cm->eeprom_func = (sensor_func_tbl_t *)malloc(sizeof(sensor_func_tbl_t));
    RETURN_ON_NULL(cm->eeprom_func);
    eeprom_sub_module_init(cm->eeprom_func);
  } else
    cm->eeprom_func = NULL;

  cm->eebin_hdl = eebin_hdl;
  bin_ctl.cmd = EEPROM_BIN_GET_LIB_NAME_DATA;
  bin_ctl.ctl.q_num.type = EEPROM_BIN_LIB_CHROMATIX;

  /* add all chromatix libraries */
  for (i = 0; i < chromatix_array->size; i++) {
    chromatix_name = &chromatix_array->chromatix_name[i];

    rc = addLib(cm, chromatix_name->isp_common, EEPROM_CALIBRATE_LSC);
    CHECK_RETURN(rc);
    rc = addLib(cm, chromatix_name->isp_preview, EEPROM_CALIBRATE_WB_GREEN);
    CHECK_RETURN(rc);
    rc = addLib(cm, chromatix_name->isp_snapshot, EEPROM_CALIBRATE_WB_GREEN);
    CHECK_RETURN(rc);
    rc = addLib(cm, chromatix_name->isp_video, EEPROM_CALIBRATE_WB_GREEN);
    CHECK_RETURN(rc);
    rc = addLib(cm, chromatix_name->cpp_preview, 0);
    CHECK_RETURN(rc);
    rc = addLib(cm, chromatix_name->cpp_snapshot, 0);
    CHECK_RETURN(rc);
    rc = addLib(cm, chromatix_name->cpp_video, 0);
    CHECK_RETURN(rc);
    rc = addLib(cm, chromatix_name->cpp_liveshot, 0);
    CHECK_RETURN(rc);
    rc = addLib(cm, chromatix_name->postproc, 0);
    CHECK_RETURN(rc);
    rc = addLib(cm, chromatix_name->a3_video, EEPROM_CALIBRATE_WB);
    CHECK_RETURN(rc);
    rc = addLib(cm, chromatix_name->a3_preview, EEPROM_CALIBRATE_WB);
    CHECK_RETURN(rc);
    rc = addLib(cm, chromatix_name->iot, 0);
    CHECK_RETURN(rc);

    if (lru_count(&cm->lru) == MAX_CHROMATIX_COUNT) {
      SERR("HASH cache is full");
      break;
    }
  }

  lru_traverse(&cm->lru);

  SLOW("Exit");

  return TRUE;

ERROR:
  if (cm->mutex_created) {
    pthread_mutex_destroy(&cm->mutex);
    cm->mutex_created = FALSE;
  }
  SERR("failed");
  return FALSE;
}

void cm_destroy(chromatix_manager_type *cm)
{
  if (!cm) {
    SERR("failed NULL pointer detected");
    return;
  }

  if (cm->mutex_created)
    pthread_mutex_destroy(&cm->mutex);

  if (cm->eeprom_func)
    free(cm->eeprom_func);

  // close all open library
  hash_traverseFuction(&cm->hash, unload_chromatix);

  lru_destroy(&cm->lru);
  hash_destroy(&cm->hash);
}

void *cm_getChromatix(chromatix_manager_type* cm, const char *name,
  uint32_t calibration_type)
{
  void    *data_sym = NULL;
  void    *data_hdl = NULL;
  char    *key      = NULL;
  uint32_t cIndex;

  if (!cm || !name) {
    SERR("NULL pointer detected");
    return NULL;
  }

  SLOW("%s", name);

  PTHREAD_MUTEX_LOCK(&cm->mutex);
  if (hash_find(&cm->hash, name, &data_sym, &cIndex)) {
    /* Chromatix file is in the hash : update LRU */
    lru_update(&cm->lru, cIndex);
  } else {
    SLOW("Can't find the data of key[%s]", name);

    /* Chromatix file is NOT in the hash :
       (1) cache memory is available, add
       (2) delete least used file from cache, and add */
    if (lru_count(&cm->lru) < MAX_CHROMATIX_COUNT) {
      data_sym = addLib_getSymbol(cm, name, calibration_type);
    } else {
      /* get the least used chromatix file info */
      lru_getLeastRecent(&cm->lru, &key, &cIndex);
      /* delete has node and unload the library */
      hash_delete(&cm->hash, key, &data_hdl, &data_sym);
      unload_chromatix(data_hdl, data_sym);

      data_sym = addLib_getSymbol(cm, name, calibration_type);
      if (data_sym == NULL)
        SERR("failed : addLib2");
    }
  }

  lru_traverse(&cm->lru);
  PTHREAD_MUTEX_UNLOCK(&cm->mutex);

  return data_sym;
}

static boolean addLib(chromatix_manager_type* cm, const char *key,
  int32_t cal_type)
{
  void    *data_hdl = NULL;
  void    *data_sym = NULL;
  uint32_t cIndex;
  int32_t  rc;

  if (strlen(key) == 0) return TRUE;

  SLOW("chromatix: %s", key);

  if (lru_count(&cm->lru) == MAX_CHROMATIX_COUNT) {
    SERR("HASH cache is full");
    return TRUE;
  }

  /* check if chromatix library is already in hash */
  if (hash_find(&cm->hash, key, &data_sym, &cIndex) == TRUE)
    return TRUE;

  if (addLib_getSymbol(cm, key, cal_type) == NULL) {
    SERR("failed to add chromatix to hash");
    return FALSE;
  }

  return TRUE;
}

static void* addLib_getSymbol(chromatix_manager_type* cm, const char *key,
  int32_t cal_type)
{
  void    *data_hdl = NULL;
  void    *data_sym = NULL;
  uint32_t cIndex;
  int32_t  rc;

  if (key == NULL) return NULL;

  SLOW("chromatix: %s", key);

  /* open library */
  bin_ctl.ctl.name_data.lib_name = (char *)key;
  bin_ctl.ctl.name_data.path = NULL;
  if (cm->eebin_hdl)
    if (eebin_interface_control(cm->eebin_hdl, &bin_ctl) < 0)
      SERR("No Camera Multimodule data.");

  rc = load_chromatix(key, bin_ctl.ctl.name_data.path,
    &data_hdl, &data_sym);
  if (rc != SENSOR_SUCCESS) {
    SERR("fail : load_chromatix %s", key);
    return NULL;
  }

  /* calibration */
  if (cm->eeprom_ctrl && cm->eeprom_func && cal_type > 0)
    ((sensor_func_tbl_t*)(cm->eeprom_func))->process(
      cm->eeprom_ctrl, cal_type, data_sym);

  if (lru_add(&cm->lru, key, &cIndex) == FALSE)
    goto ERROR;
  if (hash_add(&cm->hash, key, data_hdl, data_sym, cIndex)
    == FALSE)
    goto ERROR;

  return data_sym;

ERROR:
  unload_chromatix(data_hdl, data_sym);
  return NULL;
}
