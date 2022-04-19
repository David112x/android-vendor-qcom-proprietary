/* hvx_add_constant.c
*
* Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*/

/* std headers */
#include <stdio.h>
#include <stdlib.h>
#include <utils/Log.h>
#include <string.h>

/* hvx headers */
#include "hvx_lib.h"

#define ILOG(fmt, args...) \
  ALOGE("%s:%d hvx_debug "fmt"\n", __func__, __LINE__, ##args)

#define INLOG(fmt, args...) \
  ALOGE("hvx_debug "fmt"\n", ##args)

struct hvx_update_t {
  uint32_t constant;
};

static hvx_ret_type_t hvx_lib_open(void **oem_data)
{
  uint32_t *data = (uint32_t *)calloc(1, sizeof(uint32_t));
  if (!data) {
    /* log error */
    ALOGE("%s:%d failed: data %p\n", __func__, __LINE__, data);
    return HVX_RET_FAILURE;
  }
  *oem_data = (void *)data;
  return HVX_RET_SUCCESS;
}

static hvx_ret_type_t hvx_lib_get_hvx_info(void *oem_data,
    hvx_lib_get_hvx_info_t *hvx_info)
{
  if (!oem_data || !hvx_info) {
   ALOGE("%s:%d failed: oem_data %p hvx_info %p\n", __func__, __LINE__,
     oem_data, hvx_info);
   return HVX_RET_FAILURE;
  }

  hvx_info->hvx_enable = HVX_TRUE;

  strlcpy(hvx_info->algo_name, "hvx_add_pixel_test",
    sizeof(hvx_info->algo_name));
  hvx_info->hvx_out_width = hvx_info->sensor_width;
  hvx_info->hvx_out_height = hvx_info->sensor_height;
  if (hvx_info->available_hvx_units >= 1) {
    hvx_info->request_hvx_units = 1;
  }
  if (hvx_info->available_hvx_vector_mode >= HVX_LIB_VECTOR_64) {
    hvx_info->request_hvx_vector_mode = HVX_LIB_VECTOR_64;
  }
  hvx_info->is_pix_intf_needed = HVX_TRUE;
  hvx_info->is_stats_needed = HVX_FALSE;

  return HVX_RET_SUCCESS;
}

static hvx_ret_type_t hvx_lib_set_config(void *oem_data,
  const hvx_lib_config_t *lib_config,
  const hvx_lib_adsp_config_t *adsp_config,
  void *caller_data)
{
  hvx_ret_type_t               ret = HVX_RET_SUCCESS;
  uint32_t                    *data = NULL;
  struct hvx_update_t          dynamic_config;

  if (!oem_data || !lib_config || !adsp_config ||
    !adsp_config->adsp_config_call) {
    ALOGE("%s:%d failed: %p %p %p\n", __func__, __LINE__, oem_data,
      lib_config, adsp_config);
    return HVX_RET_FAILURE;
  }

  data = (uint32_t *)oem_data;

  dynamic_config.constant = *data = 5;

  /* Call adsp_config to pass call to ADSP */
  ret = adsp_config->adsp_config_call(&dynamic_config, sizeof(dynamic_config),
    caller_data);
  if (ret != HVX_RET_SUCCESS) {
    /* log error */
    ALOGE("%s:%d failed: adsp_config_call ret %d\n", __func__, __LINE__, ret);
  }

  return ret;
}

static hvx_ret_type_t hvx_lib_sof(void *oem_data,
  const hvx_lib_sof_params_t *sof_params,
  const hvx_lib_adsp_config_t *adsp_config,
  void *caller_data)
{
  hvx_ret_type_t       ret = HVX_RET_SUCCESS;
  uint32_t            *data = NULL;
  struct hvx_update_t  hvx_update;

  if (!oem_data || !adsp_config || !adsp_config->adsp_config_call ||
    !sof_params) {
    /* log error */
    ALOGE("%s:%d failed: %p %p %p\n", __func__, __LINE__, oem_data,
      adsp_config, sof_params);
    return HVX_RET_FAILURE;
  }

  data = (uint32_t *)oem_data;

  hvx_update.constant = *data;
  *data = *data + 5;
  if (*data >= 100) {
    *data = 5;
  }

  /* Call adsp_config to pass call to ADSP */
  ret = adsp_config->adsp_config_call(&hvx_update, sizeof(hvx_update),
    caller_data);
  if (ret != HVX_RET_SUCCESS) {
    /* log error */
    ALOGE("%s:%d failed: adsp_config_call ret %d\n", __func__, __LINE__, ret);
  }

  return HVX_RET_SUCCESS;
}

static hvx_ret_type_t hvx_lib_close(void *oem_data)
{
  if (!oem_data) {
     /* log error */
     ALOGE("%s:%d failed: hvx_lib_close oem_data %p\n", __func__, __LINE__,
       oem_data);
     return HVX_RET_FAILURE;
  }

  free(oem_data);

  return HVX_RET_SUCCESS;
}

hvx_ret_type_t hvx_lib_fill_function_table(
  hvx_lib_function_table_t *func_table)
{
  ILOG("");
  if (!func_table) {
    ALOGE("%s:%d failed func_table %p\n", __func__, __LINE__, func_table);
    return HVX_RET_FAILURE;
  }

  func_table->hvx_lib_open = hvx_lib_open;
  func_table->hvx_lib_get_hvx_info = hvx_lib_get_hvx_info;
  func_table->hvx_lib_set_config = hvx_lib_set_config;
  func_table->hvx_lib_sof = hvx_lib_sof;
  func_table->hvx_lib_close = hvx_lib_close;

  return HVX_RET_SUCCESS;
}
