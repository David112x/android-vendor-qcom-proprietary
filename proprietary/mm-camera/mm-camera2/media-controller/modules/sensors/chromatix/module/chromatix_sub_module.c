/* chromatix_sub_module.c
 *
 *Copyright (c) 2012-2016, 2018 Qualcomm Technologies, Inc.
 *All Rights Reserved.
 *Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <dlfcn.h>
#include "sensor_common.h"
#include "mct_profiler.h"
#include "chromatix_sub_module.h"

#define PATH_SIZE_255 255

const uint32_t pick_calibration_type[SENSOR_CHROMATIX_MAX] = {
  /* SENSOR_CHROMATIX_ISP */                 EEPROM_CALIBRATE_WB_GREEN,
  /* SENSOR_CHROMATIX_ISP_SNAPSHOT */        EEPROM_CALIBRATE_WB_GREEN,
  /* SENSOR_CHROMATIX_ISP_COMMON */          EEPROM_CALIBRATE_LSC,
  /* SENSOR_CHROMATIX_CPP_PREVIEW */         0,
  /* SENSOR_CHROMATIX_CPP_SNAPSHOT */        0,
  /* SENSOR_CHROMATIX_CPP_FLASH_SNAPSHOT */  0,
  /* SENSOR_CHROMATIX_CPP_DS */              0,
  /* SENSOR_CHROMATIX_CPP_US */              0,
  /* SENSOR_CHROMATIX_CPP_VIDEO */           0,
  /* SENSOR_CHROMATIX_SW_PPROC */            0,
  /* SENSOR_CHROMATIX_3A */                  EEPROM_CALIBRATE_WB
};

int32_t load_chromatix(const char *name, const char *path,
  void **handle, void **symbol)
{
  FILE    *fp = NULL;
  char     lib_name[PATH_SIZE_255] = {0};
  char     bin_name[PATH_SIZE_255] = {0};
  uint32_t file_size = 0;
  char     retry = FALSE;
  char    *abs_path = MOD_SENSOR_LIB_PATH;
  uint32_t bytes_read = 0;
  void *(*open_lib)(void);
  void * pBuffer = NULL;

  RETURN_ERROR_ON_NULL(name);
  RETURN_ERROR_ON_NULL(handle);
  RETURN_ERROR_ON_NULL(symbol);

  *handle = NULL;
  *symbol = NULL;

  if (path){
    snprintf(lib_name, PATH_SIZE_255, "%slibchromatix_%s.so", path, name);
    snprintf(bin_name, PATH_SIZE_255, "%schromatix_%s.bin", path, name);
  } else if (abs_path) {
    retry = TRUE;
    snprintf(lib_name, PATH_SIZE_255, "%s/libchromatix_%s.so", abs_path, name);
    snprintf(bin_name, PATH_SIZE_255, "%schromatix_%s.bin",
      CONFIG_XML_PATH, name);
  }

  SLOW ("lib_name %s", (char *)lib_name);

  *handle = dlopen((const char *)lib_name, RTLD_NOW);
  if (!(*handle) && retry)
    *handle = dlopen((const char *)lib_name, RTLD_NOW);

  JUMP_ON_NULL(*handle, LIB_LOAD_ERROR);

  *(void **)&open_lib = dlsym(*handle, "load_chromatix");
  JUMP_ON_NULL(open_lib, LIB_LOAD_ERROR);

  *symbol = open_lib();
  JUMP_ON_NULL(*symbol, LIB_LOAD_ERROR);

  SLOW("handle = 0x%p symbol = 0x%p", *handle, *symbol);
  return SENSOR_SUCCESS;

LIB_LOAD_ERROR:
  if (*handle)
    dlclose(*handle);

  SLOW ("bin_name %s", (char *)bin_name);

  fp = fopen(bin_name, "rb");
  if (!fp) {
    SERR("%s fopen failed (non-fatal)", bin_name);
    goto BIN_READ_ERROR;
  }

  /* Seek to the end of the stream */
  if (fseek(fp, 0L, SEEK_END)) {
    SERR("fseek to SEEK_END failed (non-fatal) ");
    goto BIN_READ_ERROR;
  }

  /* Get the number of bytes in file */
  file_size = ftell(fp);

  /* Seek to the begin of the stream */
  if (fseek(fp, 0L, SEEK_SET)) {
    SERR("fseek to SEEK_SET failed (non-fatal) ");
    goto BIN_READ_ERROR;
  }

  pBuffer = malloc(file_size);
  JUMP_ON_NULL(pBuffer, BIN_READ_ERROR);

  bytes_read = fread(pBuffer, 1, file_size, fp);
  if (bytes_read != file_size) {
    SERR("Invalid bytes read bytes_read = %d file_size = %d (non-fatal)",
      bytes_read, file_size);
    goto BIN_READ_ERROR;
  }

  *symbol = pBuffer;

  fclose(fp);
  return SENSOR_SUCCESS;

BIN_READ_ERROR:
  free(pBuffer);
  if (fp) fclose(fp);

  return SENSOR_FAILURE;
}

int32_t unload_chromatix(void *handle, void* symbol)
{
  SLOW("handle = 0x%p symbol = 0x%p", handle, symbol);
  /* Close the handle for the library opened */
  if (handle)
    dlclose(handle);
  /* Free the memory which was allocated for the binary */
  else if (symbol)
    free(symbol);
  else {
    SERR("unload_chromatix failed: both handle and symbol are NULL");
    return SENSOR_FAILURE;
  }

  return SENSOR_SUCCESS;
}

/*
 * CHROMATIX_GET_PTR
 *   : Return chromatix library pointer
 *     based on input chromatix name */
static int32_t chromatix_get_ptr(chromatix_data_t *ctrl, void *data)
{
  int32_t i;
  sensor_chromatix_params_t *params;

  SLOW("Enter");

  RETURN_ERROR_ON_NULL(ctrl);
  RETURN_ERROR_ON_NULL(data);

  params = (sensor_chromatix_params_t *)data;

  for(i = 0; i < SENSOR_CHROMATIX_MAX; i++) {
    SLOW("type[%d]:[%s]", i, params->chromatix_lib_name[i]);

    if (params->chromatix_lib_name[i] == ctrl->chromatix_name[i])
      params->chromatix_reloaded[i] = FALSE;
    else
      params->chromatix_reloaded[i] = TRUE;

    if (i == SENSOR_CHROMATIX_EXTERNAL) continue;

    if (!params->chromatix_lib_name[i]) {
      params->chromatix_ptr[i] = NULL;
      continue;
    }

    ctrl->chromatix_ptr[i] =
      cm_getChromatix(ctrl->cm, params->chromatix_lib_name[i],
      pick_calibration_type[i]);
    if (!ctrl->chromatix_ptr[i]) {
      SERR("Can't get chromatix pointer : %s", params->chromatix_lib_name[i]);
      return SENSOR_FAILURE;
    }

    params->chromatix_ptr[i] = ctrl->chromatix_ptr[i];

    SLOW("chromatix version: 0x%x",
      ((chromatix_parms_type*)ctrl->chromatix_ptr[i])
      ->chromatix_version_info.chromatix_version);
    ctrl->chromatix_name[i] = params->chromatix_lib_name[i];
  }

  SLOW("Exit");
  return SENSOR_SUCCESS;
}

/*
 * CHROMATIX_GET_CACHED_PTR
 *   : Return chromatix library pointer alreay in chromatix_data_t
 *     (no input of chromatix name) */
static int32_t chromatix_get_cached_ptr(chromatix_data_t *ctrl, void *data)
{
  int32_t i;
  sensor_chromatix_params_t *params;

  SLOW("Enter");

  RETURN_ERROR_ON_NULL(ctrl);
  RETURN_ERROR_ON_NULL(data);

  params = (sensor_chromatix_params_t *)data;

  for(i = 0; i < SENSOR_CHROMATIX_MAX; i++) {
    params->chromatix_ptr[i] = ctrl->chromatix_ptr[i];
    params->chromatix_lib_name[i] = ctrl->chromatix_name[i];
  }

  SLOW("Exit");
  return SENSOR_SUCCESS;
}

/*==========================================================
 * FUNCTION    - chromatix_open -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t chromatix_open(void **chromatix_ctrl,
  void *data  __attribute__((unused)) )
{
  chromatix_data_t *ctrl;

  SLOW("Enter");

  RETURN_ERROR_ON_NULL(chromatix_ctrl);

  ctrl = malloc(sizeof(chromatix_data_t));
  if (!ctrl) {
    SERR("failed");
    return SENSOR_ERROR_NOMEM;
  }
  memset(ctrl, 0, sizeof(chromatix_data_t));

  *chromatix_ctrl = (void *)ctrl;

  SLOW("Exit");

  return SENSOR_SUCCESS;
}

/*
 * Reset chromatix names
 */
static void chromatix_reset_name(chromatix_data_t *ctrl)
{
  memset(ctrl->chromatix_name, 0, sizeof(ctrl->chromatix_name));
}

/*==========================================================
 * FUNCTION    - chromatix_process -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t chromatix_process(void *chromatix_ctrl,
  sensor_submodule_event_type_t event, void *data)
{
  chromatix_data_t *ctrl;
  int32_t           rc = SENSOR_SUCCESS;

  RETURN_ERROR_ON_NULL(chromatix_ctrl);

  MCT_PROF_LOG_BEG(PROF_SENSOR_CHROMATIX, "event: ", event);

  ctrl = (chromatix_data_t *)chromatix_ctrl;

  switch (event) {
  case CHROMATIX_SET_CM:
    ctrl->cm = (chromatix_manager_type *)data;
    break;
  case CHROMATIX_GET_PTR:
    chromatix_get_ptr(ctrl, data);
    break;
  case CHROMATIX_GET_CACHED_PTR:
    chromatix_get_cached_ptr(ctrl, data);
    break;
  case CHROMATIX_RESET_NAME:
    chromatix_reset_name(ctrl);
    break;
  default:
    SERR("invalid event %d", event);
    rc = SENSOR_FAILURE;
   break;
  }
  MCT_PROF_LOG_END();

  return rc;
}

/*==========================================================
 * FUNCTION    - chromatix_close -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t chromatix_close(void *chromatix_ctrl)
{
  RETURN_ERROR_ON_NULL(chromatix_ctrl);

  free(chromatix_ctrl);
  return SENSOR_SUCCESS;
}

/*==========================================================
 * FUNCTION    - chromatix_sub_module_init -
 *
 * DESCRIPTION:
 *==========================================================*/
int32_t chromatix_sub_module_init(sensor_func_tbl_t *func_tbl)
{
  if (!func_tbl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  func_tbl->open = chromatix_open;
  func_tbl->process = chromatix_process;
  func_tbl->close = chromatix_close;
  return SENSOR_SUCCESS;
}
