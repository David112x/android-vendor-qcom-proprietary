/**********************************************************************
*  Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __IMG_SCENE_DETECT_MGR_H__
#define __IMG_SCENE_DETECT_MGR_H__

#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include "mct_event_stats.h"
#include "modules.h"
#include "cam_types.h"

/**
 * size of scene data ring buffer
 */
#define MAX_IMG_SCENE_DATA 4
/**
 * max number of clients
 */
#define MAX_IMG_SCENE_CLIENT 10
/**
 * max number of sessions
 */
#define MAX_IMG_SCENE_SESSIONS 8
/**
 * maximum pipeline delay
 */
#define MAX_IMG_PIPELINE_DELAY 4

/** img_scene_detect_data_t:
 *
 * @lux_idx: lux index
 * @gain: real gain
 * @iso_sensitivity: ISO sensitivity
 * @color_temp: color temparature
 * @frame_id: frame id corresponding to the data
 *
 * Structure to hold the scene data
 */
typedef struct {
  float lux_idx;
  float gain;
  float iso_sensitivity;
  uint32_t color_temp;
  int32_t frame_id;
} img_scene_detect_data_t;

/** img_scene_detect_list_t:
 *
 * @sd_data: list of scene data
 * @cur_idx: current index
 * @count: count of the data
 * @frame_id: frame index
 * @chromatix_param: list of chromatix header pointers
 *
 * Structure to hold the scene data
 */
typedef struct {
  img_scene_detect_data_t sd_data[MAX_IMG_SCENE_DATA];
  uint32_t cur_idx;
  uint32_t count;
  uint32_t frame_id;
  modulesChromatix_t chromatix_param;
} img_scene_detect_list_t;

/** scene_detect_func_t:
 *
 *  @p_userdata: pointer to the userdata
 *  @session_id: session id
 *  @p_scenelist: scene data list
 *  @p_dyn_data: output dynamic data set by the client
 *
 *  Scene detect callback function
 */
typedef int32_t (*scene_detect_func_t) (void *p_userdata,
  uint32_t session_id,
  img_scene_detect_list_t *p_scenelist,
  cam_dyn_img_data_t *p_dyn_data);

/** img_sd_client_data_t:
 *
 * @p_detect: callback function registered by the client
 * @p_appdata: pointer to the appdata
 *
 * Structure passed by the client for registration
 */
typedef struct {
  scene_detect_func_t p_detect;
  void *p_appdata;
} img_sd_client_data_t;

/** img_sd_client_internal_t:
 *
 * @data: scene client data
 * @is_used: flag to indicate whether the client is used
 * @client_id: client identity
 *
 * Structure to client internal data
 */
typedef struct {
  img_sd_client_data_t data;
  bool is_used;
  uint32_t client_id;
} img_sd_client_internal_t;

/** img_dyn_feat_metadata_t:
 *
 * @dyn_data: dynamic feature data
 * @dyn_data_ready: flag to indicate whether dynamic data is
 *                ready to be posted
 *
 * Structure to hold the dynaic feature metadata
 */
typedef struct {
  cam_dyn_img_data_t dyn_data;
  bool dyn_data_ready;
} img_dyn_feat_metadata_t;

/** img_scene_detect_mgr_t:
 *
 * @lock: mutex lock
 * @is_init: flasg to indicate whether the scene detec manager
 *         is initialized
 * @client_info: list of client information
 * @num_clients: number of active clients
 * @scene_list: list of data used for scene detect
 * @max_apply_delay: maximum apply delay
 * @max_report_delay: maximum report delay
 *
 * Structure to client internal data
 */
typedef struct {
  pthread_mutex_t lock;
  bool is_init;
  img_sd_client_internal_t client_info[MAX_IMG_SCENE_CLIENT];
  uint32_t num_clients;
  img_scene_detect_list_t scene_list[MAX_IMG_SCENE_SESSIONS];
  img_dyn_feat_metadata_t md_dynfeat[MAX_IMG_PIPELINE_DELAY];
  uint32_t max_apply_delay;
  uint32_t max_report_delay;
} img_scene_detect_mgr_t;

/** get_scene_mgr:
 *
 *  Arguments:
 *  none
 *
 * Description: This function gets the scene manager
 *
 * Return values:
 *     scene detect manager instance, NULL if scene manager is
 *     not available
 *
 * Notes: can be called by the clients
 **/
img_scene_detect_mgr_t *get_scene_mgr();

/** img_scene_mgr_init:
 *
 *  Arguments:
 *  @p_scene_mgr: pointer to scene manager
 *
 * Description: This function initializes scene detect manager
 *
 * Return values:
 *     imaging errors
 *
 * Notes: none
 **/
int32_t img_scene_mgr_init(img_scene_detect_mgr_t *p_scene_mgr);

/** img_scene_mgr_deinit:
 *
 *  Arguments:
 *  @p_scene_mgr: pointer to scene manager
 *
 * Description: This function de-initializes scene detect
 * manager
 *
 * Return values:
 *     imaging errors
 *
 * Notes: none
 **/
int32_t img_scene_mgr_deinit(img_scene_detect_mgr_t *p_scene_mgr);

/** img_scene_mgr_register:
 *
 *  Arguments:
 *  @p_scene_mgr: pointer to scene manager
 *  @p_client_data: pointer to the client data
 *
 * Description: This function registers scene detect data to the
 *      manager
 *
 * Return values:
 *     client id, else return 0
 *
 * Notes: none
 **/
int32_t img_scene_mgr_register(img_scene_detect_mgr_t *p_scene_mgr,
  img_sd_client_data_t *p_client_data);

/** img_scene_mgr_unregister:
 *
 *  Arguments:
 *  @p_scene_mgr: pointer to scene manager
 *  @p_client_data: pointer to the client data
 *
 * Description: This function unregisters scene detect data to
 *      the manager
 *
 * Return values:
 *     imaging error
 *
 * Notes: none
 **/
int32_t img_scene_mgr_unregister(img_scene_detect_mgr_t *p_scene_mgr,
  uint32_t client_idx);

/** img_scene_mgr_handle_sof_update:
 *
 *  Arguments:
 *  @p_scene_mgr: pointer to scene manager
 *  @session_id: session identity
 *  @frame_id: frame idx
 *
 * Description: This function is called during SOF update
 *
 * Return values:
 *     imaging error
 *
 * Notes: none
 **/
int32_t img_scene_mgr_handle_sof_update(img_scene_detect_mgr_t *p_scene_mgr,
  uint32_t session_id,
  int32_t frame_id);

/** img_scene_mgr_handle_aec_update:
 *
 *  Arguments:
 *  @p_scene_mgr: pointer to scene manager
 *  @session_id: session identity
 *  @p_aec_update: pointer to AEC update
 *
 * Description: This function is called during AEC update
 *
 * Return values:
 *     imaging error
 *
 * Notes: none
 **/
int32_t img_scene_mgr_handle_aec_update(img_scene_detect_mgr_t *p_scene_mgr,
  uint32_t session_id,
  aec_update_t *p_aec_update);

/** img_scene_mgr_handle_awb_update:
 *
 *  Arguments:
 *  @p_scene_mgr: pointer to scene manager
 *  @session_id: session identity
 *  @p_aec_update: pointer to AWB update
 *
 * Description: This function is called during AWB update
 *
 * Return values:
 *     imaging error
 *
 * Notes: none
 **/
int32_t img_scene_mgr_handle_awb_update(img_scene_detect_mgr_t *p_scene_mgr,
  uint32_t session_id,
  awb_update_t *p_awb_update);

/** img_scene_mgr_handle_chromatix_update:
 *
 *  Arguments:
 *  @p_scene_mgr: pointer to scene manager
 *  @session_id: session identity
 *  @p_chromatix: chromatix pointer
 *
 * Description: This function is called during chromatix update
 *
 * Return values:
 *     imaging error
 *
 * Notes: none
 **/
int32_t img_scene_mgr_handle_chromatix_update(img_scene_detect_mgr_t *p_scene_mgr,
  uint32_t session_id,
  void *p_chromatix);

/** img_scene_mgr_reset:
 *
 *  Arguments:
 *  @p_scene_mgr: pointer to scene manager
 *  @session_id: session identity
 *  @max_apply_delay: maximum applying delay for the session
 *  @max_report_delay: maximum report delay for the session
 *
 * Description: This function resets the scene manager
 *
 * Return values:
 *     None
 *
 * Notes: none
 **/
void img_scene_mgr_reset(img_scene_detect_mgr_t *p_scene_mgr,
  uint32_t session_id,
  uint32_t max_apply_delay,
  uint32_t max_report_delay);

/** img_scene_mgr_get_featmeta:
 *
 *  Arguments:
 *  @frame_id: frame_id
 *
 * Description: This function gets the feature metadata
 *
 * Return values:
 *     feat metadata pointer
 *
 * Notes: none
 **/
img_dyn_feat_metadata_t *img_scene_mgr_get_featmeta(
  img_scene_detect_mgr_t *p_scene_mgr,
  uint32_t frame_id);

#endif //__IMG_SCENE_DETECT_MGR_H__
