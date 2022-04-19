/* hvx_grid_sum.c
*
* Copyright (c) 2015-2018 Qualcomm Technologies, Inc. All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*/

/* std headers */
#include <stdio.h>
#include <stdlib.h>
#include <utils/Log.h>
#include <string.h>
#include "cam_types.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

/* hvx headers */
#include "hvx_lib.h"

struct hvx_lib_private_data_t {
  uint32_t dump_enabled;
  uint32_t dump_stats;
};

static hvx_ret_type_t hvx_lib_open(void **oem_data)
{
  struct hvx_lib_private_data_t *data =
   (struct hvx_lib_private_data_t *)calloc(1,
   sizeof(struct hvx_lib_private_data_t));
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

  strlcpy(hvx_info->algo_name, "hvx_grid_stats_test",
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
  hvx_info->is_stats_needed = HVX_TRUE;
  /* 64 pixels per line, 2 bytes per pixel*/
  hvx_info->stats_data_size = hvx_info->hvx_out_height * 128;

  return HVX_RET_SUCCESS;
}

static hvx_ret_type_t hvx_lib_consume_stats(void *oem_data,
  unsigned int frame_id, hvx_lib_stats_t *stats_data,
  const hvx_lib_adsp_config_t *adsp_config, void *caller_data)
{
  struct hvx_lib_private_data_t *private_data = NULL;
  hvx_ret_type_t                 ret = HVX_RET_SUCCESS;
  uint32_t                      *data = NULL;
  char                           value[PROPERTY_VALUE_MAX];
  char                           buf[100];
  FILE                          *fptr = NULL;
  uint32_t                       i = 0;

  if (!oem_data || !adsp_config || !adsp_config->adsp_config_call ||
    !stats_data || !caller_data) {
    /* log error */
    ALOGE("%s:%d failed: %p %p %p %p\n", __func__, __LINE__, oem_data,
      adsp_config, stats_data, caller_data);
    return HVX_RET_FAILURE;
  }

  private_data = (struct hvx_lib_private_data_t *)oem_data;
  /* Check whether setprop is enabled to dump configuration */
  if (private_data->dump_enabled == HVX_FALSE) {
     property_get("persist.vendor.camera.hvx.dumpstats", value, "0");
     private_data->dump_stats = atoi(value);
     private_data->dump_enabled = HVX_TRUE;
  }

  if (private_data->dump_stats) {
    /* Dump stats */
    if (stats_data->stats_left && stats_data->stats_left_size) {
      data = (uint32_t *)stats_data->stats_left;
      snprintf(buf, sizeof(buf), QCAMERA_DUMP_FRM_LOCATION"hvx_left_stats%d.txt", frame_id);
      fptr = fopen(buf, "w+");
      if (!fptr) {
        ALOGE("%s:%d failed fptr %p buf %s\n", __func__, __LINE__, fptr, buf);
      } else {
        for (i = 0; i < stats_data->stats_left_size/4; i++) {
          fprintf(fptr, "%d\n", data[i]);
        }
        fclose(fptr);
      }
    }
    if (stats_data->stats_right && stats_data->stats_right_size) {
      data = (uint32_t *)stats_data->stats_right;
      snprintf(buf, sizeof(buf), QCAMERA_DUMP_FRM_LOCATION"hvx_right_stats%d.txt", frame_id);
      fptr = fopen(buf, "w+");
      if (!fptr) {
        ALOGE("%s:%d failed fptr %p buf %s\n", __func__, __LINE__, fptr, buf);
      } else {
        for (i = 0; i < stats_data->stats_right_size/4; i++) {
          fprintf(fptr, "%d\n", data[i]);
        }
        fclose(fptr);
      }
    }
    private_data->dump_stats--;
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
  if (!func_table) {
    ALOGE("%s:%d failed func_table %p\n", __func__, __LINE__, func_table);
    return HVX_RET_FAILURE;
  }

  func_table->hvx_lib_open = hvx_lib_open;
  func_table->hvx_lib_get_hvx_info = hvx_lib_get_hvx_info;
  func_table->hvx_lib_consume_stats = hvx_lib_consume_stats;
  func_table->hvx_lib_close = hvx_lib_close;

  return HVX_RET_SUCCESS;
}
