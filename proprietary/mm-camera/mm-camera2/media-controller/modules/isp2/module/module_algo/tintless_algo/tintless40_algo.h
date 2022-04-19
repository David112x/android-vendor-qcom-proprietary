/* tintless40_algo.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __TINTLESS40_ALGO_H__
#define __TINTLESS40_ALGO_H__

/* isp headers */
#include "isp_algo.h"

/* tintless headers */
#include "dmlrocorrection.h"

extern isp_algo_t algo_tintless40;

/** tintless40_algo_params_t:
 *
 *  @dmlroc_res: tintless params handle
 *  @lib_handle: lib handle
 *  @update_func: update_func function pointer
 *  @get_version_func: get version func pointer
 *  @init_func: init func pointer
 *  @deinit_func: deinit func pointer
 *  @algo_cfg: algo cfg
 *  @tintless_array: tintless array
 *  @is_be_stats_config_done: flag to store whether be stats
 *                          params is updated
 *  @is_rolloff_config_done: flag to store whether rolloff
 *                         config is updated
 *  @is_tintless_strength_config_done: flag to store whether
 *                                   chroamtix params is udpated
 *  @is_tintless_array_valid: flag to store whether input
 *                          tintless array is updated
 *  @dump_enabled: flag to store dump enable / disable flag
 *  @dump_frames: number of frames to dump
 **/
typedef struct {
  void                    *dmlroc_res;
  void                    *lib_handle;
  dmlroc_return_t        (*update_func)(void * res,
    const dmlroc_bayer_stats_info_t * const pbayer_r,
    const dmlroc_bayer_stats_info_t * const pbayer_gr,
    const dmlroc_bayer_stats_info_t * const pbayer_gb,
    const dmlroc_bayer_stats_info_t * const pbayer_b,
    const dmlroc_mesh_rolloff_array_t * const ptable_current,
    const dmlroc_mesh_rolloff_array_t * const ptable_3a,
    dmlroc_mesh_rolloff_array_t * const ptable_correction);
  void                   (*get_version_func)(dmlroc_version_t * const pversion);
  dmlroc_return_t        (*init_func)(void ** res,
    const dmlroc_config_t * const cfg);
  void                   (*deinit_func)(void ** res);
  dmlroc_config_t         algo_cfg;
  mesh_rolloff_array_type tintless_array;
  boolean                 is_be_stats_config_done;
  boolean                 is_rolloff_config_done;
  boolean                 is_tintless_strength_config_done;
  boolean                 is_tintless_array_valid;
  boolean                 dump_enabled;
  uint32_t                dump_frames;
} tintless40_algo_params_t;

#endif /* __TINTLESS40_ALGO_H__ */
