/***************************************************************************
* Copyright (c) 2016-2018 Qualcomm Technologies, Inc.                      *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
***************************************************************************/
#ifndef __MODULE_QDC_COMMON_H__
#define __MODULE_QDC_COMMON_H__

#include "module_imgbase.h"
#include "module_qdc_sat.h"
#include "module_qdc_rtb.h"
#include "module_sac.h"
#include "module_rtbdm.h"

/** MODULE_MASK:
 *
 * Mask to enable dynamic logging
 **/
#undef MODULE_MASK
#define MODULE_MASK IMGLIB_QDC_COMMON_SHIFT

/**img_qdc_flavor_t
*
* @IMG_QDC_QC_SOL: QC flavor mask
* @IMG_QDC_OEM_SOL: OEM flavor mask
*
* Mask indicating QC or OEM flavor of dual cam solution
*/
typedef enum {
  IMG_QDC_QTI_SOL,
  IMG_QDC_OEM_SOL,
} img_qdc_flavor_t;

/**img_qdc_common_client_cmd_t
*
* @IMG_QDC_COMMON_BYPASS: Set the client in bypass mode
* @IMG_QDC_COMMON_ROLE_SWITCH: Current sesion role switch
* @IMG_QDC_COMMON_LPM: Is Low Power mode enabled
*
* Data to be propagated to multiple clients in a given session
*/
typedef enum {
  IMG_QDC_COMMON_BYPASS,
  IMG_QDC_COMMON_ROLE_SWITCH,
  IMG_QDC_COMMON_LPM,
} img_qdc_common_client_cmd_t;

/**img_qdc_lib_info_t
*
* @p_lib_name: library name
* @lib_flavor: library flavor
*
* Struct capturing library name and flavor
*/
typedef struct {
  char *p_lib_name;
  img_qdc_flavor_t lib_flavor;
} img_qdc_lib_info_t;

/**img_qdc_feature_info_t
*
* @feature_mask: feature mask
* @p_libs_arr: pointer to array of libs
* @arr_size: array size
*
* Struct capturing library name and flavor
*/
typedef struct {
  cam_feature_mask_t feature_mask;
  img_qdc_lib_info_t *p_libs_arr;
  uint32_t arr_size;
} img_qdc_feature_info_t;

/**img_qdc_common_multi_client_data_t
*
* @session_id: Current session id
* @qdc_cmd: Commands to the client
* @data: Data associated with the cmd
*
* Data to be propagated to multiple clients in a given session
*/
typedef struct {
  uint32_t session_id;
  img_qdc_common_client_cmd_t qdc_cmd;
  void *data;
} img_qdc_common_multi_client_data_t;

/** img_qdc_common_cfg_t:
 *   @sat_cfg: SAT config
 *   @sac_cfg: SAC config
 *   @rtb_cfg: RTB config
 *   @rtbdm_cfg: RTBDM config
 *
 *   QDC_Common client private structure
 */
typedef union {
  img_qdc_sat_cfg_t sat_cfg;
  img_sac_cfg_t sac_cfg;
  img_rtb_cfg_t rtb_cfg;
  img_rtbdm_cfg_t rtbdm_cfg;
} img_qdc_common_cfg_t;

/** img_qdc_common_session_data_t:
 *
 *   @sensor_max_dim: sensor max dim
 *   @sensor_crop: sensor crop request
 *   @af_tuning_cfg: AF tuning data
 *   @calib_data: sensor calibration data
 *   @input_size: input sizes
 *   @output_size: output sizes
 *   @output_size_changed: flag for output size changed
 *   @orignal_output_size: orignal output sizes
 *   @qdc_cfg: dual call config
 *   @session_mutex: Mutex to syncronize session data
 *   @process_stream_type: Stream to process
 *   @zoom_level: session zoom level
 *   @p_sensor_cap: pointer to sensor capabilities
 *   @pix_pitch: pixel pitch
 *   @focal_length: focal length
 *   @user_zoom_level: User zoom level
 *   @blur_level_info: Blur intensity level data
 *
 *   Session based parameters for DC modules
 */
typedef struct {
  img_dim_t sensor_max_dim;
  sensor_request_crop_t sensor_crop;
  mct_imglib_af_focus_config_t af_tuning_cfg;
  sensor_isp_stream_sizes_t input_size;
  sensor_isp_stream_sizes_t output_size;
  bool output_size_changed[MAX_NUM_STREAMS];
  sensor_isp_stream_sizes_t orignal_output_size;
  img_qdc_common_cfg_t qdc_cfg;
  pthread_mutex_t session_mutex;
  cam_stream_type_t process_stream_type;
  int32_t zoom_level;
  mct_pipeline_sensor_cap_t *p_sensor_cap;
  float pix_pitch;
  float focal_length;
  int32_t user_zoom_level;
  cam_rtb_blur_info_t blur_level_info;
} img_qdc_common_session_data_t;

/** img_qdc_common_result_t:
 *
 *   @sac_result: SAC config
 *   @rtbdm_result: RTBDM config
 *
 *   QDC_Common client private structure
 */
typedef union {
  img_qdc_sat_result_t sat_result;
  img_sac_result_t sac_result;
  img_rtb_result_t rtb_result;
  img_rtbdm_result_t rtbdm_result;
} img_qdc_common_result_t;

/** img_qdc_common_client_t:
 *
 *   @p_session_data: pointer to the session based data
 *   @p_client: pointer to IMGLIB base client
 *   @qdc_cfg: dual call config
 *   @qdc_result: dual call result
 *
 *   client private structure
 */
typedef struct {
  img_qdc_common_session_data_t *p_session_data;
  imgbase_client_t *p_client;
  img_qdc_common_cfg_t qdc_cfg;
  img_qdc_common_result_t qdc_result;
} img_qdc_common_client_t;


/** img_qdc_common_module_t:
 *
 *   @session_data: Session data
 *   @qdc_algo_flavor: algo flavor
 *   @system_calib_data: sensor calibration data.
 *
 *   QDC_Common module private structure
 */
typedef struct {
  img_qdc_common_session_data_t session_data[MAX_IMGLIB_SESSIONS];
  img_qdc_flavor_t qdc_algo_flavor;
  cam_related_system_calibration_data_t system_calib_data;
} img_qdc_common_module_t;

#endif

