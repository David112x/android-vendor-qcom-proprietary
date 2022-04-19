/* af_ext_test_wrapper.c
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "af_ext_test_wrapper.h"
#include "af_biz.h"

typedef struct {
  void            *q3a_handler;
  af_ops_t        af_ops;
  void            *af;
  int             is_af_output_share;
} af_ext_test_wrapper_private_t;

/* These functions will allow testing framework features not supported by the
 *  current algorithm
 **/

/** af_ext_test_wrapper_set_parameters: Set AF parameters based on parameter
 *  type.
 *
 *  @param: information about parameter to be set
 *
 *  @af: internal AF data structure
 *
 *
 * Return: Success- TRUE  Failure- FALSE
 **/
boolean af_ext_test_wrapper_set_parameters(af_set_parameter_t *param,
  af_output_data_t *output_arr, uint8_t num_of_outputs, void *af_internal)
{
  af_ext_test_wrapper_private_t *af_ext_test_wrapper = af_internal;
  boolean rc = TRUE;
  af_output_data_t *output = NULL;
  uint8 cam_idx = 0;
  uint8 camera_main = 0;
  uint8 camera_secondary = 1;

  if (!param || !af_ext_test_wrapper || !af_ext_test_wrapper->af) {
    AF_ERR(" Invalid parameters! num_of_outputs: %u", num_of_outputs);
    return FALSE;
  }

  AF_HIGH("Biz:%p, param type =%d, camera_id:%u,",
    af_ext_test_wrapper->af, param->type, param->camera_id);

  if (af_ext_test_wrapper->is_af_output_share &&
      param->camera_id == camera_secondary) {
    AF_HIGH("Do not set_params for cam %d", camera_secondary);
    return FALSE;
  }

  /* Clear older output values */
  STATS_MEMSET(output_arr, 0, num_of_outputs*sizeof(af_output_data_t));
  /* Verify that output array has a valid pointer and assign camera_id to output */
  for (cam_idx = 0; cam_idx < num_of_outputs; cam_idx++) {
    /* Set default output type */
    output_arr[cam_idx].type = AF_OUTPUT_UPDATE_PENDING;
  }

  rc = af_ext_test_wrapper->af_ops.set_parameters(param,
    &output_arr[param->camera_id], 1, af_ext_test_wrapper->af);
  return rc;
}

/** af_ext_test_wrapper_process: Main entry point to process the AF stats.
 *
 *  @stats: AF stats
 *  @output: Data exposed externally and updated once AF state
 *         is processed.
 *  @af_obj: pointer to internal AF data.
 *
 *  Demo dual camera - Single algo configuration
 *
 **/
void af_ext_test_wrapper_process(stats_af_t *stats,
 af_output_data_t *output_arr, uint8_t num_of_outputs, void *af_obj)
{
  af_debug_data_level_type debug_level = AF_DEBUG_DATA_LEVEL_VERBOSE;
  af_ext_test_wrapper_private_t *af_ext_test_wrapper = af_obj;
  q3a_core_result_type result;
  uint8 cam_idx = 0;
  uint8 camera_main = 0;
  uint8 camera_secondary = 1;

  AF_HIGH("Biz:%p, camera_id:%u,",
    af_ext_test_wrapper->af, stats->camera_id);


  if (af_ext_test_wrapper->is_af_output_share
      && stats->camera_id == camera_secondary) {
    AF_HIGH("Do not process stats for cam %u", camera_secondary);
    return;
  }

  /* Verify output availability
   * check if available output for all registered cameras */
  STATS_MEMSET(output_arr, 0, num_of_outputs*sizeof(af_output_data_t));
  for (cam_idx = 0; cam_idx < num_of_outputs; cam_idx++) {
    output_arr[cam_idx].type = AF_OUTPUT_UPDATE_PENDING;
  }

  af_ext_test_wrapper->af_ops.process(stats, &output_arr[stats->camera_id], 1,
   af_ext_test_wrapper->af);

  if (af_ext_test_wrapper->is_af_output_share) {
    memcpy(&output_arr[camera_secondary], &output_arr[camera_main],
      sizeof(af_output_data_t));
    AF_HIGH("Copy output type: 0x%x, to camera: 1, output: %p",
      output_arr[camera_main].type,
      &output_arr[camera_secondary]);
    if (output_arr[camera_secondary].type & AF_OUTPUT_MOVE_LENS) {
      AF_HIGH("cam %u req: type:AF_OUTPUT_MOVE_LENS, direction: %d, steps: %d, cur_pos: %d",
        camera_secondary,
        output_arr[camera_secondary].move_lens.direction,
        output_arr[camera_secondary].move_lens.num_of_steps,
        output_arr[camera_secondary].move_lens.cur_pos);
    }
  }
}

/** af_ext_test_wrapper_get_parameters:
 *  Interface for outside components to request access to AF
 *  parameters.
 *
 *  @param: information of speicific parameter to access
 *
 *  @af_internal: internal af data structure
 *
 *  Return: TRUE - success  FALSE - failure
 **/
boolean af_ext_test_wrapper_get_parameters(af_get_parameter_t *param,
  void *af_internal)
{
  af_ext_test_wrapper_private_t *af_ext_test_wrapper = af_internal;

  AF_HIGH("Biz:%p, param type =%d, camera_id:%u,",
    af_ext_test_wrapper->af, param->type, param->camera_id);

  return af_ext_test_wrapper->af_ops.get_parameters(param, af_ext_test_wrapper->af);
}

/** af_ext_test_wrapper_init
 *
 **/
 void *af_ext_test_wrapper_init(void *libptr)
{
  af_ext_test_wrapper_private_t *af_ext_test_wrapper = libptr;

  af_ext_test_wrapper->af =
    af_ext_test_wrapper->af_ops.init(af_ext_test_wrapper->q3a_handler);

  AF_HIGH("Biz:%p, af_test_wrapper: %p",
    af_ext_test_wrapper->af, af_ext_test_wrapper);

  return af_ext_test_wrapper;
}

/** af_ext_test_wrapper_destroy
 *
 **/
void af_ext_test_wrapper_destroy(void *af_obj)
{
  af_ext_test_wrapper_private_t *af_ext_test_wrapper = af_obj;

  AF_HIGH("Biz:%p, af_test_wrapper: %p",
    af_ext_test_wrapper->af, af_ext_test_wrapper);

  af_ext_test_wrapper->af_ops.deinit(af_ext_test_wrapper->af);
  return;
}

/**
 * af_ext_test_wrapper_load_function
 *
 * @af_ops: structure with function pointers to be assign
 *
 * Return: Handler to AF interface library
 **/
void * af_ext_test_wrapper_load_function(af_ops_t *af_ops)
{
  af_ext_test_wrapper_private_t *af_ext_test_wrapper = NULL;

  af_ext_test_wrapper = malloc(sizeof(af_ext_test_wrapper_private_t));
  if (!af_ext_test_wrapper) {
   return NULL;
  }

  af_ext_test_wrapper->q3a_handler = af_biz_load_function(af_ops);
  if (!af_ext_test_wrapper->q3a_handler) {
    free(af_ext_test_wrapper);
    return NULL;
  }

  /* Save af_biz functions to interface */
  af_ext_test_wrapper->af_ops = *af_ops;

  /* Overwrite ops */
  AF_HIGH("Overwrite default AF biz functions, use now test wrapper! wrapper: %p, af_lib_handle: %p",
    af_ext_test_wrapper, af_ext_test_wrapper->q3a_handler);

  af_ops->init = af_ext_test_wrapper_init;
  af_ops->deinit = af_ext_test_wrapper_destroy;
  af_ops->process = af_ext_test_wrapper_process;
  af_ops->set_parameters = af_ext_test_wrapper_set_parameters;
  af_ops->get_parameters = af_ext_test_wrapper_get_parameters;

  STATS_TEST_SHARE_AF_ALGO(af_ext_test_wrapper->is_af_output_share);

  return af_ext_test_wrapper;
}

/**
 * af_biz_unload_function
 *
 * @af_ops: structure with function pointers to be assign
 * @lib_handler: Handler to the algo library
 *
 * Return: void
 **/
void af_ext_test_wrapper_unload_function(af_ops_t *af_ops,
  void *lib_handler)
{
  af_ext_test_wrapper_private_t *af_ext_test_wrapper = lib_handler;

  AF_HIGH("Free: af_test_wrapper: %p, af_lib_handle: %p",
    af_ext_test_wrapper, af_ext_test_wrapper->q3a_handler);
  af_biz_unload_function(af_ops, af_ext_test_wrapper->q3a_handler);
  free(af_ext_test_wrapper);

  return;
}
