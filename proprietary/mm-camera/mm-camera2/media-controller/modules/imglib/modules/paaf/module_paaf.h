/**********************************************************************
*  Copyright (c) 2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/
#ifndef __MODULE_PAAF_H__
#define __MODULE_PAAF_H__

#include "module_imgbase.h"
#include "chromatix_3a.h"

#define PAAF_ROI_MIN_SIZE 180
#define PAAF_ROI_BOUNDARY_PIXELS 12
#define PAAF_RECOMMENDED_ANALYSIS_WIDTH 1920
#define PAAF_RECOMMENDED_ANALYSIS_HEIGHT 1080

/**img_paaf_multi_client_data_t
*
* @session_id: Current session id
* @processing_disable: PAAF disabled/Enabled
* @processing_stream: Current processing stream
*
* Data to be propagated to multiple clients in a given session
*/
 typedef struct {
   uint32_t session_id;
   boolean processing_disable;
   cam_stream_type_t processing_stream;
 } img_paaf_multi_client_data_t;

/** img_paaf_defs_t
 *   @streams: Array containing matching streams.
 *   @processing_str_id: Normal light stream selection.
 *   @streams_to_process: stream selection.
 *
 *   Structure used to describe faceproc client
 *   definitions.
 **/
typedef struct {
  cam_stream_type_t streams[CAM_STREAM_TYPE_MAX];
  cam_stream_type_t processing_stream;
  uint32_t streams_to_process;
} img_paaf_defs_t;

/** img_paaf_session_data_t:
 *
 *   @max_dim: max dim for the session (CAMIF dim)
 *   @paaf_config: PAAF configuration recieved from 3A
 *   @paaf_cfg_updated: Flag indicating if the PAAF cfg was
 *                    updated
 *   @session_mutex: Mutex to syncronize session data
 *   @camif_info: CAMIF info needed for translation
 *   @paaf_state: state of PAAF feature
 *   @processing_stream: processing stream
 *                       for the current session
 *   @streams_to_process: stream selection.
 *   @af_tuning_trans_info: AF Tuning translation params
 *
 *   Session based parameters for PAAF module
 */
typedef struct {
  img_dim_t max_dim;
  mct_imglib_swaf_config_t paaf_cfg;
  bool paaf_cfg_updated;
  pthread_mutex_t session_mutex;
  int32_t paaf_state;
  cam_stream_type_t processing_stream;
  uint32_t streams_to_process;
  img_trans_info_t af_tuning_trans_info;
} img_paaf_session_data_t;

/** img_paaf_client_t:
 *
 *   @p_session_data: pointer to the session based data
 *   @p_client: pointer to IMGLIB base client
 *   @use_af_tuning_trans: If true, Calculate AFS client ROI
 *                         position based on vfe map scale
 *   @crop_info: Crop Info updated from stream crop event
 *   @camif_trans_info: Translation info to translate ROI from
 *                    CAMIF
 *   @paaf_config: Updated PAAF config with translated ROI
 *
 *   paaf client private structure
 */
typedef struct {
  img_paaf_session_data_t *p_session_data;
  imgbase_client_t *p_client;
  boolean use_af_tuning_trans;
  img_rect_t crop_info;
  img_trans_info_t camif_trans_info;
  img_paaf_cfg_t paaf_config;
} img_paaf_client_t;

/** img_paaf_module_t:
 *
 *   @session_data: Session data
 *   @prealloc_bufs: flag to indicate whether the preallocation
 *                 is done
 *
 *   Paaf module private structure
 */
typedef struct {
  img_paaf_session_data_t session_data[MAX_IMGLIB_SESSIONS];
  uint32_t client_id;
  bool prealloc_bufs;
} img_paaf_module_t;


#endif //__MODULE_PAAF_H__

