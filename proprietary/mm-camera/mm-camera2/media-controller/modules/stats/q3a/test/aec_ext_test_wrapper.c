/* aec_ext_test_wrapper.c
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "aec_ext_test_wrapper.h"
#include "aec_biz.h"

/* These functions will allow testing framework features not supported by the
*  current algorithm
**/

typedef struct {
  void            *q3a_handler;
  aec_object_t    aec_ops;
  void            *aec;
  int             is_aec_output_share;
} aec_ext_test_wrapper_private_t;

/** aec_ext_test_wrapper_get_param:
*
**/
static boolean aec_ext_test_wrapper_get_param(aec_get_parameter_t *param,
  void *aec_obj)
{
  aec_ext_test_wrapper_private_t *aec_ext_test_wrapper = aec_obj;
  boolean rc = FALSE;
  uint8 camera_main = 0;
  uint8 camera_secondary = 1;

  if (!param || !aec_ext_test_wrapper || !aec_ext_test_wrapper->aec) {
    AEC_ERR("Invalid input: %p,%p",param, aec_ext_test_wrapper);
    return FALSE;
  }

  AEC_HIGH("Biz: %p, param type =%d, camera_id: %d",
    aec_ext_test_wrapper->aec, param->type, param->camera_id);

  if (camera_secondary == param->camera_id &&
      TRUE == aec_ext_test_wrapper->is_aec_output_share) {
    AEC_HIGH("Skip get param for cam: %d", param->camera_id);;
    return TRUE;
  }

  rc = aec_ext_test_wrapper->aec_ops.get_parameters(param,
    aec_ext_test_wrapper->aec);
  return rc;
}

/** aec_ext_test_wrapper_set_param:
*
**/
boolean aec_ext_test_wrapper_set_param(aec_set_parameter_t *param,
 aec_output_data_t *output_arr, uint8 num_of_outs, void *aec_obj)
{
  boolean rc            = FALSE;
  aec_ext_test_wrapper_private_t *aec_ext_test_wrapper = aec_obj;
  uint32 linecount_secondary = 0;
  uint8 camera_main = 0;
  uint8 camera_secondary = 1;
  uint8 cam_idx = 0;
  uint8 camera_id = 0;

  STATS_TEST_AEC_SECONDARY_LC(linecount_secondary);

  if (!param || !aec_ext_test_wrapper || !aec_ext_test_wrapper->aec) {
    return FALSE;
  }

  AEC_HIGH("Biz: %p, param type =%d, camera_id: %d",
    aec_ext_test_wrapper->aec, param->type, param->camera_id);

  camera_id = param->camera_id;
  if (camera_secondary == param->camera_id &&
      TRUE == aec_ext_test_wrapper->is_aec_output_share) {
    AEC_HIGH("Skip processing param for cam: %d", param->camera_id);
    return TRUE;
  }

  /* For most of the cases, set parameter does not requires output */
  if (output_arr == NULL) {
    rc = aec_ext_test_wrapper->aec_ops.set_parameters(param, NULL, 0,
      aec_ext_test_wrapper->aec);
    return rc;
  }

  /* Clear older output values */
  STATS_MEMSET(output_arr, 0, num_of_outs*sizeof(aec_output_data_t));
  /* Verify that output array has a valid pointer and assign camera_id to output */
  for (cam_idx = 0; cam_idx < num_of_outs; cam_idx++) {
    output_arr[cam_idx].type = AEC_OUTPUT_UPDATE_PENDING;
  }

  rc = aec_ext_test_wrapper->aec_ops.set_parameters(param,
    &output_arr[param->camera_id], 1, aec_ext_test_wrapper->aec);

  if (param->type != AEC_SET_PARAM_PACK_OUTPUT) {
    return rc;
  }

  /* AEC_SET_PARAM_PACK_OUTPUT */
  if (aec_ext_test_wrapper->is_aec_output_share) {
    STATS_MEMCPY(&output_arr[camera_secondary], sizeof(aec_output_data_t),
      &output_arr[camera_main], sizeof(aec_output_data_t));

    if (linecount_secondary) {
      output_arr[camera_secondary].stats_update.aec_update.linecount =
        linecount_secondary;
      AEC_HIGH("Setting cam: %u, lc = %u", camera_secondary, linecount_secondary);
    }
  }

  return rc;
}

boolean aec_ext_test_wrapper_process(stats_t *stats, void *aec_obj,
 aec_output_data_t *output_arr, uint8 num_of_outs)
{
  boolean rc = FALSE;
  aec_ext_test_wrapper_private_t *aec_ext_test_wrapper = aec_obj;
  uint8 cam_idx = 0;
  int linecount_secondary = 0;
  uint8 camera_main = 0;
  uint8 camera_secondary = 1;
  uint32 camera_id = 0;

  STATS_TEST_AEC_SECONDARY_LC(linecount_secondary);

  if (!aec_ext_test_wrapper || !aec_ext_test_wrapper->aec || !stats) {
    AEC_ERR("Invalid input: %p, %p", aec_ext_test_wrapper, stats);
    return FALSE;
  }

  AEC_HIGH("Biz: %p, camera_id: %d",
    aec_ext_test_wrapper->aec, stats->camera_id);

  if (camera_secondary == stats->camera_id &&
      TRUE == aec_ext_test_wrapper->is_aec_output_share) {
    AEC_HIGH("Skip processing param for cam: %d", stats->camera_id);
    return TRUE;
  }

  /* Verify that output array has a valid pointer and assign camera_id to output */
  for (cam_idx = 0; cam_idx < num_of_outs; cam_idx++) {
    output_arr[cam_idx].type = AEC_OUTPUT_UPDATE_PENDING;
  }

  rc = aec_ext_test_wrapper->aec_ops.process(stats, aec_ext_test_wrapper->aec,
    &output_arr[stats->camera_id], 1);

  if (aec_ext_test_wrapper->is_aec_output_share) {
    STATS_MEMCPY(&output_arr[camera_secondary], sizeof(aec_output_data_t),
      &output_arr[camera_main], sizeof(aec_output_data_t));

    if (linecount_secondary) {
      output_arr[camera_secondary].stats_update.aec_update.linecount =
        linecount_secondary;
      AEC_HIGH("Setting cam: %d, lc = %u", camera_secondary, linecount_secondary);
    }
  }

  return rc;
}

float aec_ext_test_wrapper_map_iso_to_real_gain(void *aec_obj,
  uint32_t iso, uint8_t camera_id)
{
  aec_ext_test_wrapper_private_t *aec_ext_test_wrapper = aec_obj;

  AEC_HIGH("Biz: %p, ISO =%d, camera_id: %d",
    aec_ext_test_wrapper->aec, iso, camera_id);

  return aec_ext_test_wrapper->aec_ops.iso_to_real_gain(
    aec_ext_test_wrapper->aec, iso, camera_id);
}

boolean aec_ext_test_wrapper_get_version(void *aec_obj,
  Q3a_version_t *version, uint8_t camera_id)
{
  aec_ext_test_wrapper_private_t *aec_ext_test_wrapper = aec_obj;

  AEC_HIGH("Biz: %p, camera_id: %d",
      aec_ext_test_wrapper->aec, camera_id);

  return aec_ext_test_wrapper->aec_ops.get_version(aec_ext_test_wrapper->aec,
    version, camera_id);
}

void *aec_ext_test_wrapper_init(void *libptr)
{
  aec_ext_test_wrapper_private_t *aec_ext_test_wrapper = libptr;

  aec_ext_test_wrapper->aec =
    aec_ext_test_wrapper->aec_ops.init(aec_ext_test_wrapper->q3a_handler);

  AEC_HIGH("Biz: %p, aec_test_wrapper:%p",
    aec_ext_test_wrapper->aec, aec_ext_test_wrapper);

  return aec_ext_test_wrapper;
}

void aec_ext_test_wrapper_destroy(void *aec_obj)
{
  aec_ext_test_wrapper_private_t *aec_ext_test_wrapper = aec_obj;

  AEC_HIGH("Biz: %p, aec_test_wrapper:%p",
    aec_ext_test_wrapper->aec, aec_ext_test_wrapper);

  aec_ext_test_wrapper->aec_ops.deinit(aec_ext_test_wrapper->aec);
  return;
}

/**
 * aec_ext_test_wrapper_load_function
 *
 * @aec_object: structure with function pointers to be assign
 *
 * Return: Handler to AEC interface library
 **/
void * aec_ext_test_wrapper_load_function(aec_object_t *aec_object)
{
  aec_ext_test_wrapper_private_t *aec_ext_test_wrapper = NULL;

  aec_ext_test_wrapper = malloc(sizeof(aec_ext_test_wrapper_private_t));
  if (!aec_ext_test_wrapper) {
    return NULL;
  }

  aec_ext_test_wrapper->q3a_handler = aec_biz_load_function(aec_object);
  if (!aec_ext_test_wrapper->q3a_handler) {
    free(aec_ext_test_wrapper);
    return NULL;
  }

  /* Save aec_biz functions to interface */
  aec_ext_test_wrapper->aec_ops = *aec_object;

  /* Overwrite ops */
  AEC_HIGH("Overwrite default AEC biz functions, use now test wrapper! wrapper: %p, aec_lib_handle: %p",
    aec_ext_test_wrapper, aec_ext_test_wrapper->q3a_handler);
  aec_object->set_parameters = aec_ext_test_wrapper_set_param;
  aec_object->get_parameters = aec_ext_test_wrapper_get_param;
  aec_object->process = aec_ext_test_wrapper_process;
  aec_object->init = aec_ext_test_wrapper_init;
  aec_object->deinit = aec_ext_test_wrapper_destroy;
  aec_object->iso_to_real_gain = aec_ext_test_wrapper_map_iso_to_real_gain;
  aec_object->get_version = aec_ext_test_wrapper_get_version;

  STATS_TEST_SHARE_AECAWB_ALGO(aec_ext_test_wrapper->is_aec_output_share);

  return aec_ext_test_wrapper;
}

/**
 * aec_ext_test_wrapper_unload_function
 *
 * @aec_object: structure with function pointers to be assign
 * @lib_handler: Handler to the algo library
 *
 * Return: void
 **/
void aec_ext_test_wrapper_unload_function(aec_object_t *aec_object,
  void *lib_handler)
{
  aec_ext_test_wrapper_private_t *aec_ext_test_wrapper = lib_handler;

  AEC_HIGH("Free: aec_test_wrapper:%p, aec_lib_handle: %p",
    aec_ext_test_wrapper, aec_ext_test_wrapper->q3a_handler);

  aec_biz_unload_function(aec_object, aec_ext_test_wrapper->q3a_handler);
  free(aec_ext_test_wrapper);

  return;
}

