/**********************************************************************
*  Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __MODULE_EDGE_SMOOTH_H__
#define __MODULE_EDGE_SMOOTH_H__

#include "module_imgbase.h"
#include "edge_smooth_params.h"
#include "chromatix_swpostproc.h"
#include "mgr/scene_detect_mgr.h"

/** img_edge_session_data_t:
 *
 *   @es_config: edge smooth configuration
 *   @p_chromatix: SW postproc chromatix pointer
 *   @ea_state: state of edge alignment feature
 *   @sd_client_id: scene detect client id
 *
 *   Session based parameters for edge alignment module
 */
typedef struct {
  img_edge_smooth_meta_t es_config;
  chromatix_sw_postproc_type *p_chromatix;
  int32_t ea_state;
  uint32_t sd_client_id;
} img_edge_session_data_t;

/** img_edge_client_t:
 *
 *   @p_session_data: pointer to the session based data
 *   @cur_gain: current real gain
 *   @cur_lux_idx: current lux idx
 *   @nr_flag: Noise reduction flag from OIS capture mode
 *
 *   edge client private structure
 */
typedef struct {
  img_edge_session_data_t *p_session_data;
  imgbase_client_t *p_client;
} img_edge_client_t;

/** img_edge_module_t:
 *
 *   @max_dim: maximum dimension of the
 *   @es_config: edge smooth configuration
 *   @client_id: alloc client ID
 *   @prealloc_bufs: flag to indicate whether the preallocation
 *                 is done
 *   @last_error: holds the last error
 *   @force_enable: Flag to indicate whether edge needs to be
 *                enabled by default
 *   @bypass: Flag to indicate if Edge needs to be bypassed
 *
 *   edge module private structure
 */
typedef struct {
  img_dim_t max_dim;
  img_edge_session_data_t session_data[MAX_IMGLIB_SESSIONS];
  uint32_t client_id;
  bool prealloc_bufs;
  int32_t last_error;
  bool force_enable;
  bool bypass;
} img_edge_module_t;

/**
 * Function: module_edge_config_set_default_params
 *
 * Description: function called to set the default parameters
 *
 * Arguments:
 *   @p_config: image edge smooth meta
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
void module_edge_config_set_default_params(img_edge_smooth_meta_t *p_config);

/**
 * Function: module_edge_config_update_from_chromatix
 *
 * Description: function called to interpolate the parameters
 *
 * Arguments:
 *   @p_client: pointer to the edge client
 *   @p_config: image edge smooth meta
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
void module_edge_config_update_from_chromatix(img_edge_client_t *p_client,
  img_edge_smooth_meta_t *p_config);

/**
 * Function: module_edge_config_handle_hysteresis
 *
 * Description: function called by scene detect manager to
 *       handle hysteresis
 *
 * Arguments:
 *  @p_userdata: pointer to the userdata
 *  @session_id: session id
 *  @p_scenelist: scene data list
 *  @p_dyn_data: output dynamic data set by the client
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t module_edge_config_handle_hysteresis(void *p_userdata,
  uint32_t session_id __unused,
  img_scene_detect_list_t *p_scenelist,
  cam_dyn_img_data_t *p_dyn_data);
#endif //__MODULE_EDGE_SMOOTH_H__
