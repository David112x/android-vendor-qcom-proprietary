/* awb_ext_test_wrapper.c
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "awb_ext_test_wrapper.h"
#include "awb_biz.h"

/* awb_ext_test_wrapper_private_t
 *
 * Place holder for data neede for ext_test_wrapper
 *
 * @q3a_handler: AWB library handler
 * @awb_ops: Place holder for awb_biz ops
 * @awb: Place holder for awb_biz instance, saved at init()
 * @is_awb_output_share: Save setprop value that decides if sharing or not the AWB algo.
 */
typedef struct {
  void            *q3a_handler;
  awb_ops_t       awb_ops;
  void            *awb;
  int             is_awb_output_share;
} awb_ext_test_wrapper_private_t;


/* These functions will allow testing framework features not supported by the
 *  current algorithm
 **/

/** awb_set_parameters:
*
**/
static boolean awb_ext_test_wrapper_set_params(awb_set_parameter_t *param,
  awb_output_data_t *output_arr, uint8_t num_of_outs, void *awb_obj)
{
  boolean rc            = TRUE;
  boolean need_callback = FALSE;
  awb_ext_test_wrapper_private_t *awb_ext_test_wrapper = awb_obj;
  uint8 cam_idx = 0;
  uint8 camera_main = 0;
  uint8 camera_secondary = 1;
  uint8 camera_id = 0;

  if (!param || !awb_ext_test_wrapper || !awb_ext_test_wrapper->awb) {
    return FALSE;
  }

  AWB_HIGH("Biz: %p, param type =%d, camera_id: %d",
    awb_ext_test_wrapper->awb, param->type, param->camera_id);

  if (camera_secondary == param->camera_id &&
      TRUE == awb_ext_test_wrapper->is_awb_output_share) {
    AWB_HIGH("Skip processing param for cam: %d", param->camera_id);
    return TRUE;
  }

  camera_id = param->camera_id;
  /* For most of the cases, set parameter does not requires output */
  if (output_arr == NULL) {
    rc = awb_ext_test_wrapper->awb_ops.set_parameters(param, NULL, 0,
      awb_ext_test_wrapper->awb);
    return rc;
  }


  /* Clear older output values */
  STATS_MEMSET(output_arr, 0, num_of_outs*sizeof(awb_output_data_t));

  for (cam_idx = 0; cam_idx < num_of_outs; cam_idx++) {
    output_arr[cam_idx].type = AWB_OUTPUT_UPDATE_PENDING;
  }



  rc = awb_ext_test_wrapper->awb_ops.set_parameters(param,
    &output_arr[camera_id], 1, awb_ext_test_wrapper->awb);

  if (param->type != AWB_SET_PARAM_PACK_OUTPUT) {
    return rc;
  }

  /* AWB_SET_PARAM_PACK_OUTPUT */
  if (awb_ext_test_wrapper->is_awb_output_share && num_of_outs == 2 &&
      camera_id == camera_main) {
    AWB_HIGH("Copy output from %u to %u", camera_main, camera_secondary);
    STATS_MEMCPY(&output_arr[camera_secondary], sizeof(awb_output_data_t),
      &output_arr[camera_main], sizeof(awb_output_data_t));
  }

  return rc;
}

static boolean awb_ext_test_wrapper_get_params(
  awb_get_parameter_t *param, void *awb_obj)
{
  awb_ext_test_wrapper_private_t *awb_ext_test_wrapper = awb_obj;
  uint8 camera_secondary = 1;
  boolean rc = FALSE;

  if (!param || !awb_ext_test_wrapper || !awb_ext_test_wrapper->awb) {
   AWB_ERR("Invalid input: %p,%p",param, awb_ext_test_wrapper);
    return FALSE;
  }

  AWB_HIGH("Biz: %p, param type =%d, camera_id: %d",
      awb_ext_test_wrapper->awb, param->type, param->camera_id);

  if (camera_secondary == param->camera_id &&
      TRUE == awb_ext_test_wrapper->is_awb_output_share) {
    AWB_HIGH("Skip get param for cam: %d", param->camera_id);
    return TRUE;
  }

  rc = awb_ext_test_wrapper->awb_ops.get_parameters(param,
    awb_ext_test_wrapper->awb);
  return rc;
}

/** awb_ext_test_wrapper_process:
*
**/
static void awb_ext_test_wrapper_process(stats_t *stats, void *obj,
  awb_output_data_t *output_arr, uint8_t num_of_outs)
{
  boolean rc = FALSE;
  awb_ext_test_wrapper_private_t *awb_ext_test_wrapper = obj;
  uint8 cam_idx = 0;
  uint8 camera_main = 0;
  uint8 camera_secondary = 1;

  AWB_HIGH("Biz: %p, process for camera_id: %u",
    awb_ext_test_wrapper->awb, stats->camera_id);

  if (camera_secondary == stats->camera_id &&
      TRUE == awb_ext_test_wrapper->is_awb_output_share) {
    AWB_HIGH("Skip processing param for cam: %d", stats->camera_id);
    return;
  }

  /* Clear older output values */
  STATS_MEMSET(output_arr, 0, num_of_outs*sizeof(awb_output_data_t));

  /* Verify that output array has a valid pointer and assign camera_id to output */
  for (cam_idx = 0; cam_idx < num_of_outs; cam_idx++) {
    /* Assign output type */
    output_arr[cam_idx].type = AWB_OUTPUT_UPDATE_PENDING;
  }

  awb_ext_test_wrapper->awb_ops.process(stats, awb_ext_test_wrapper->awb,
    &output_arr[stats->camera_id], 1);

  if (awb_ext_test_wrapper->is_awb_output_share) {
    STATS_MEMCPY(&output_arr[camera_secondary], sizeof(awb_output_data_t),
      &output_arr[camera_main], sizeof(awb_output_data_t));
  }

  return;
}

static boolean awb_ext_test_wrapper_estimate_cct_by_gains (void *obj,
  float r_gain, float g_gain, float b_gain, float *cct, uint8_t camera_id)
{
  awb_ext_test_wrapper_private_t *awb_ext_test_wrapper = obj;

  AWB_HIGH("Biz: %p, gains: %f:%f:%f camera_id: %u",
    awb_ext_test_wrapper->awb, r_gain, g_gain, b_gain, camera_id);
  return awb_ext_test_wrapper->awb_ops.estimate_cct(awb_ext_test_wrapper->awb,
    r_gain, g_gain, b_gain, cct, camera_id);
}

static boolean awb_ext_test_wrapper_estimate_gains_by_cct(void *obj,
  float *r_gain, float* g_gain, float* b_gain, float cct, uint8_t camera_id)
{
  awb_ext_test_wrapper_private_t *awb_ext_test_wrapper = obj;

  AWB_HIGH("Biz: %p, cct: %f camera_id: %u",
    awb_ext_test_wrapper->awb, cct, camera_id);
  return awb_ext_test_wrapper->awb_ops.estimate_gains(awb_ext_test_wrapper->awb,
    r_gain, g_gain, b_gain, cct, camera_id);
}

static boolean awb_ext_test_wrapper_estimate_ccm(void *obj, float cct,
  awb_ccm_type *ccm, uint8_t camera_id)
{
  awb_ext_test_wrapper_private_t *awb_ext_test_wrapper = obj;

  AWB_HIGH("Biz: %p, cct: %f camera_id: %u",
    awb_ext_test_wrapper->awb, cct, camera_id);
  return awb_ext_test_wrapper->awb_ops.estimate_ccm(awb_ext_test_wrapper->awb,
    cct, ccm, camera_id);
}

void *awb_ext_test_wrapper_init(void *libptr)
{
  awb_ext_test_wrapper_private_t *awb_ext_test_wrapper = libptr;

  awb_ext_test_wrapper->awb =
    awb_ext_test_wrapper->awb_ops.init(awb_ext_test_wrapper->q3a_handler);

  AWB_HIGH("Biz: %p, awb_test_wrapper: %p",
    awb_ext_test_wrapper->awb, awb_ext_test_wrapper);

  return awb_ext_test_wrapper;
}

void awb_ext_test_wrapper_deinit(void *awb_obj)
{
  awb_ext_test_wrapper_private_t *awb_ext_test_wrapper = awb_obj;

  AWB_HIGH("Biz: %p, awb_test_wrapper: %p",
    awb_ext_test_wrapper->awb, awb_ext_test_wrapper)

  awb_ext_test_wrapper->awb_ops.deinit(awb_ext_test_wrapper->awb);
  return;
}

/**
 * awb_ext_test_wrapper_load_function
 *
 * @awb_ops: structure with function pointers to be assign
 *
 * Return: Handler to AWB interface library
 **/
void * awb_ext_test_wrapper_load_function(awb_ops_t *awb_ops)
{
  awb_ext_test_wrapper_private_t *awb_ext_test_wrapper = NULL;

  awb_ext_test_wrapper = calloc(1, sizeof(awb_ext_test_wrapper_private_t));
  if (!awb_ext_test_wrapper) {
    return NULL;
  }

  memset(awb_ext_test_wrapper, 0,sizeof(awb_ext_test_wrapper_private_t));
  AWB_HIGH("Biz: %p, awb_test_wrapper:%p",
    awb_ext_test_wrapper->awb, awb_ext_test_wrapper);
  awb_ext_test_wrapper->q3a_handler = awb_biz_load_function(awb_ops);
  if (!awb_ext_test_wrapper->q3a_handler) {
    free(awb_ext_test_wrapper);
    return NULL;
  }

  /* Save awb_biz functions to interface */
  awb_ext_test_wrapper->awb_ops = *awb_ops;

  /* Overwrite ops */
  AWB_HIGH("Overwrite default AWB biz functions, use now test wrapper! wrapper: %p, awb_lib_handle: %p",
    awb_ext_test_wrapper, awb_ext_test_wrapper->q3a_handler);
  awb_ops->set_parameters = awb_ext_test_wrapper_set_params;
  awb_ops->get_parameters = awb_ext_test_wrapper_get_params;
  awb_ops->process = awb_ext_test_wrapper_process;
  awb_ops->estimate_cct = awb_ext_test_wrapper_estimate_cct_by_gains;
  awb_ops->estimate_gains = awb_ext_test_wrapper_estimate_gains_by_cct;
  awb_ops->estimate_ccm = awb_ext_test_wrapper_estimate_ccm;
  awb_ops->init = awb_ext_test_wrapper_init;
  awb_ops->deinit = awb_ext_test_wrapper_deinit;

  STATS_TEST_SHARE_AECAWB_ALGO(awb_ext_test_wrapper->is_awb_output_share);

  return awb_ext_test_wrapper;
}

/**
 * awb_ext_test_wrapper_unload_function
 *
 * @awb_object: structure with function pointers to be assign
 * @lib_handler: Handler to the algo library
 *
 * Return: void
 **/
void awb_ext_test_wrapper_unload_function(awb_ops_t *awb_ops,
  void *lib_handler)
{
  awb_ext_test_wrapper_private_t *awb_ext_test_wrapper = lib_handler;

  AWB_HIGH("Free: awb_test_wrapper: %p, awb_lib_handle: %p, ",
    awb_ext_test_wrapper, awb_ext_test_wrapper->q3a_handler);

  awb_biz_unload_function(awb_ops, awb_ext_test_wrapper->q3a_handler);
  free(awb_ext_test_wrapper);

  return;
}
