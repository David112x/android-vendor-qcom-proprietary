/* mct_pipeline.h
 *
 * Copyright (c) 2012-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __MCT_PIPELINE_H__
#define __MCT_PIPELINE_H__

#include "mct_object.h"
#include "mct_stream.h"
#include "mct_util.h"

/* Maximum number of resolution for non HFR and HFR mode */
#define SENSOR_MAX_RESOLUTION 13
#define TOTAL_RAM_SIZE_512MB 536870912
#define PICTURE_SIZE_5MP_WIDTH 2592
#define PICTURE_SIZE_5MP_HEIGHT 1944
#define DEFAULT_FORMAT_NV12_420 1.5
#define FPS_VARIANCE_LIMIT 5.0  /* floating point */
#define MAX_FPS_RANGE_ENTRIES 10
#define MAX_META_CHANNELS 15

/*Mask required for session data */
#define MCT_PIPELINE_SESSION_MASK 1
#define FRAME_APPLYING_DELAY_MASK          (MCT_PIPELINE_SESSION_MASK << 1)
#define META_REPORTING_DELAY_MASK          (MCT_PIPELINE_SESSION_MASK << 2)
#define MAX_SENSOR_DELAY_MASK              (MCT_PIPELINE_SESSION_MASK << 3)
#define Q3A_LIB_UPDATE_MASK                (MCT_PIPELINE_SESSION_MASK << 4)
#define ISP_FRAME_RETRY_MASK               (MCT_PIPELINE_SESSION_MASK << 5)
#define SESSION_DATA_SUPERSET              (FRAME_APPLYING_DELAY_MASK|\
                                            META_REPORTING_DELAY_MASK|\
                                            MAX_SENSOR_DELAY_MASK|\
                                            Q3A_LIB_UPDATE_MASK|\
                                            ISP_FRAME_RETRY_MASK)

/* Fixed FOVC factor */
#define MCT_PIPELINE_FIXED_FOVC_FACTOR (0.06f)

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
typedef struct {
  cam_dimension_t  dim;
  cam_fps_range_t fps;
  int32_t mode;
} mct_pipeline_sensor_res_table_t;

typedef enum {
  CAMIF_QCOM_FORMAT = 0,
  CAMIF_MIPI_FORMAT,
  CAMIF_PLAIN8_FORMAT,
  CAMIF_PLAIN16_FORMAT,
  CAMIF_MAX_RAW_FORMAT,
} camif_fmt_t;

typedef struct {
  uint32_t format_count;
  camif_fmt_t format[CAMIF_MAX_RAW_FORMAT];
} camif_cap_t;

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/

typedef enum {
 FORMAT_BAYER,
 FORMAT_YCBCR,
 FORMAT_GREY
} mct_sensor_format_t;

typedef enum {
  FPS_SEL_MCT,
  FPS_SEL_SENSOR,
  FPS_SEL_CPP,
  FPS_SEL_MAX
} mct_fps_contributor_t;

typedef struct meta_chnl_info {
  uint8_t               vc;
  uint8_t               dt;
  uint32_t              width;
  uint32_t              height;
  cam_format_t          fmt;
  cam_sub_format_type_t sub_fmt;
} mct_meta_chnl_info;

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
typedef struct {
  int32_t                         modes_supported;
  mct_sensor_format_t             sensor_format;
  uint32_t                        op_pixel_clk;
  cam_position_t                  position;
  uint32_t                        sensor_mount_angle;
  float                           focal_length;
  float                           hor_view_angle;
  float                           ver_view_angle;
  cam_lens_type_t                 lens_type;
  int32_t                         af_supported;
  int32_t                         ae_lock_supported;
  int32_t                         wb_lock_supported;
  int32_t                         scene_mode_supported;
  uint8_t                         dim_fps_table_count;
  mct_pipeline_sensor_res_table_t dim_fps_table[SENSOR_MAX_RESOLUTION];
  uint8_t                         supported_effects_cnt;
  cam_effect_mode_type            supported_effects[CAM_EFFECT_MODE_MAX];
  uint32_t                        is_flash_supported;
  uint8_t                         supported_flash_modes_cnt;
  cam_flash_mode_t                supported_flash_modes[CAM_FLASH_MODE_MAX];
  uint8_t                         supported_focus_modes_cnt;
  cam_focus_mode_type             supported_focus_modes[CAM_FOCUS_MODE_MAX];
  uint32_t                        supported_raw_fmts_cnt;
  cam_format_t                    supported_raw_fmts[CAM_FORMAT_MAX];
  uint32_t                        supported_preview_fmt_cnt;
  cam_format_t                    supported_preview_fmts[CAM_FORMAT_MAX];
  cam_feature_mask_t              feature_mask;
  uint32_t                        max_frame_applying_delay;
  uint32_t                        max_meta_reporting_delay;
  int64_t                         raw_min_duration;
  uint32_t                        max_pipeline_frame_delay;
  uint32_t                        max_frame_delay;
  float                           min_focus_distance;
  float                           hyper_focal_distance;
  float                           focal_lengths[CAM_FOCAL_LENGTHS_MAX];
  uint8_t                         focal_lengths_count;
  float                           apertures[CAM_APERTURES_MAX];
  uint8_t                         apertures_count;
  int64_t                         max_frame_duration;
  float                           sensor_physical_size[2];
  cam_dimension_t                 pixel_array_size;
  cam_dimension_t                 scale_picture_sizes[MAX_SCALE_SIZES_CNT];
  uint8_t                         scale_picture_sizes_cnt;
  uint32_t                        sensor_supported_scene_modes;
  uint32_t                        sensor_supported_effect_modes;
  cam_rect_t                      active_array_size;
  int64_t                         flash_charge_duration;
  cam_color_filter_arrangement_t  color_arrangement;
  uint8_t                         num_color_channels;
  int32_t                         white_level;
  int32_t                         black_level_pattern[BLACK_LEVEL_PATTERN_CNT];
  int32_t                         max_tone_map_curve_points;
  float                           filter_densities[CAM_FILTER_DENSITIES_MAX];
  uint8_t                         filter_densities_count;
  cam_optical_stab_modes_t        optical_stab_modes[CAM_OPT_STAB_MAX];
  uint8_t                         optical_stab_modes_count;
  uint64_t                        min_exp_time;
  uint64_t                        max_exp_time;
  int32_t                         max_analog_gain; /* In ISO Unit */
  int32_t                         max_gain; /* In ISO Unit */
  int32_t                         min_gain; /* In ISO Unit */
  uint8_t                         focus_dist_calibrated;
  uint8_t                         reference_illuminant1;
  uint8_t                         reference_illuminant2;

  cam_rational_type_t             forward_transform_D65[3][3];
  cam_rational_type_t             forward_transform_A[3][3];

  cam_rational_type_t             color_transform_D65[3][3];
  cam_rational_type_t             color_transform_A[3][3];

  cam_rational_type_t             calibration_transform_D65[3][3];
  cam_rational_type_t             calibration_transform_A[3][3];
  cam_rational_type_t             base_gain_factor;
  uint8_t                         supported_test_pattern_modes_cnt;
  cam_test_pattern_mode_t         supported_test_pattern_modes[
                                    MAX_TEST_PATTERN_CNT];

  double                          gradient_S;
  double                          offset_S;
  double                          gradient_O;
  double                          offset_O;

  char                            flash_dev_name[32];

  /* QCOM specific control */
  cam_control_range_t brightness_ctrl;
  cam_control_range_t sharpness_ctrl;
  cam_control_range_t contrast_ctrl;
  cam_control_range_t saturation_ctrl;
  cam_control_range_t sce_ctrl;
  cam_rect_t                      sensor_optical_black_region[
                                    MAX_OPTICAL_BLACK_REGIONS];
  uint8_t                         sensor_optical_black_region_count;
  uint32_t                       is_valid_dualcalib;
  cam_related_system_calibration_data_t dualcam_tune_data;
  cam_ir_mode_type_t              supported_ir_modes[CAM_IR_MODE_MAX];
  int8_t                          num_meta_channels;
  float                           pix_size; /*in micrometer*/
  mct_meta_chnl_info              meta_chnl_info[MAX_META_CHANNELS];
  float                           max_roll_degree;
  float                           max_pitch_degree;
  float                           max_yaw_degree;

  int32_t                         sensor_rotation;
  uint32_t                        is_depth_sensor;
} mct_pipeline_sensor_cap_t;

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
typedef struct {
  uint8_t                supported_effects_cnt;
  cam_effect_mode_type   supported_effects[CAM_EFFECT_MODE_MAX];
  uint8_t                supported_scene_modes_cnt;
  cam_scene_mode_type    supported_scene_modes[CAM_SCENE_MODE_MAX];
  uint8_t                zoom_ratio_tbl_cnt;
  int                    zoom_ratio_tbl[MAX_ZOOMS_CNT];
  cam_feature_mask_t     feature_mask;
  uint32_t               max_frame_delay;
  int32_t                max_sharpness_map_value;
  uint32_t               max_frame_applying_delay;
  uint32_t               max_meta_reporting_delay;
  int32_t                histogram_size;
  int32_t                max_histogram_count;
  cam_dimension_t        lens_shading_map_size;
  int32_t                max_gamma_curve_points;
  uint8_t                aberration_modes_count;
  cam_aberration_mode_t aberration_modes[CAM_COLOR_CORRECTION_ABERRATION_MAX];
  uint8_t                max_downscale_factor;
  cam_dimension_t        max_viewfinder_size;
  uint32_t               min_isp_sw_frame_skip_retry;
  uint8_t                dual_vfe_enabled;
  uint32_t               is_hvx_present;
  camif_cap_t            camif_supported_formats;
  cam_sensitivity_range_t isp_sensitivity_range;
  uint8_t                is_mono_stats_support;
  cam_dimension_t        max_encoder_size;
} mct_pipeline_isp_cap_t;

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
typedef struct {
  cam_q3a_version_t           q3a_version;
  uint8_t                     supported_white_balances_cnt;
  cam_wb_mode_type            supported_white_balances[CAM_WB_MODE_MAX];
  uint8_t                     supported_antibandings_cnt;
  cam_antibanding_mode_type   supported_antibandings[CAM_ANTIBANDING_MODE_MAX];
  uint8_t                     supported_scene_modes_cnt;
  cam_scene_mode_type         supported_scene_modes[CAM_SCENE_MODE_MAX];
  uint8_t                     supported_flash_modes_cnt;
  cam_flash_mode_t            supported_flash_modes[CAM_FLASH_MODE_MAX];
  uint8_t                     supported_focus_modes_cnt;
  cam_focus_mode_type         supported_focus_modes[CAM_FOCUS_MODE_MAX];
  uint8_t supported_focus_algos_cnt;
  cam_focus_algorithm_type supported_focus_algos[CAM_FOCUS_ALGO_MAX];
  uint8_t                     max_num_roi;
  uint8_t                     supported_iso_modes_cnt;
  cam_iso_mode_type           supported_iso_modes[CAM_ISO_MODE_MAX];
  uint8_t                     supported_aec_modes_cnt;
  cam_auto_exposure_mode_type supported_aec_modes[CAM_AEC_MODE_MAX];
  int                         exposure_compensation_min;
  int                         exposure_compensation_max;
  int                         exposure_compensation_default;
  float                       exposure_compensation_step;
  cam_rational_type_t         exp_compensation_step;
  uint8_t                     auto_wb_lock_supported;
  uint8_t                     auto_exposure_lock_supported;
  uint8_t                     video_stablization_supported;
  uint8_t                     supported_is_types_cnt;
  uint32_t                    supported_is_types[IS_TYPE_MAX];
  float                       supported_is_type_margins[IS_TYPE_MAX];
  uint8_t                     max_num_focus_areas;
  uint8_t                     max_num_metering_areas;
  cam_feature_mask_t          feature_mask;
  uint32_t                    max_frame_delay;
  uint32_t                    max_frame_applying_delay;
  uint32_t                    max_meta_reporting_delay;
  uint32_t                    max_additive_frame_delay; /*TO BE REMOVED*/
  int64_t                     exposure_time_range[2];
  uint8_t                     supported_ae_modes_cnt;
  cam_ae_mode_type            supported_ae_modes[CAM_AE_MODE_MAX];
  cam_scene_mode_overrides_t  scene_mode_overrides[CAM_SCENE_MODE_MAX];
  uint8_t                     supported_instant_aec_modes_cnt;
  cam_aec_convergence_type    supported_instant_aec_modes[CAM_AEC_CONVERGENCE_MAX];
} mct_pipeline_stats_cap_t;

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
typedef struct {
  uint64_t               max_pixel_bandwidth;
  uint8_t                supported_effects_cnt;
  cam_effect_mode_type   supported_effects[CAM_EFFECT_MODE_MAX];
  cam_pad_format_t       width_padding;
  cam_pad_format_t       height_padding;
  cam_pad_format_t       plane_padding;
  int8_t                 min_num_pp_bufs;
  uint32_t               max_supported_pp_batch_size;
  cam_feature_mask_t     feature_mask;
  uint32_t               max_frame_applying_delay;
  uint32_t               max_meta_reporting_delay;
  uint32_t               max_frame_delay;
  cam_dimension_t        sharpness_map_size;
  int32_t                max_sharpness_map_value;
  uint8_t                aberration_modes_count;
  cam_aberration_mode_t  aberration_modes[CAM_COLOR_CORRECTION_ABERRATION_MAX];
  uint8_t                supported_binning_correction_mode_cnt;
  cam_binning_correction_mode_t bincorr_modes[CAM_BINNING_CORRECTION_MODE_MAX];
  uint8_t                bincorr_mode_count;
  uint32_t               is_remosaic_lib_present;
  cam_format_t           quadra_cfa_format;
} mct_pipeline_pp_cap_t;

typedef struct {
  uint32_t        cond_posted;
  pthread_cond_t  cond;
  pthread_mutex_t mutex;
} mct_sync_data_t;

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
typedef struct{
  pthread_t pid;
  mct_module_t *module;
  unsigned int session_id;
  pthread_condattr_t condattr;
  pthread_cond_t cond_v;
  mct_sync_data_t sync;
  pthread_mutex_t mutex;
  unsigned int started_num;
  unsigned int started_num_success;
  unsigned int modules_num;
}mct_pipeline_thread_data_t;

/** mct_pipeline_imaging_cap_t:
 *
 *  Arguments/Fields:
 *    @max_num_roi: Face detection number of ROI
 *    @feature_mask: imglib featuremask
 *    @max_frame_delay: maximum frame delay
 *    @max_face_detection_count: face detection count
 *    @hdr_bracketing_setting: AE bracketting config
 *    @ubifocus_af_bracketing_need: AF bracketting config
 *    @refocus_af_bracketing_need: AF bracketting config
 *    @stillmore_settings: StillMore settings
 *    @opti_zoom_settings: optizoom configuration
 *    @analysis_info: Analysis stream information
 *    @true_portrait_settings: trueportrait configuration
 *    @chroma_flash_settings_need: chroma flash configuration
 *    @supported_feature_mask: Set of features supported by
 *                           imglib module
 *    @bayer_procecessing_delay: Bayerprocessing delay
 *
 *  Structure to define the imaging capabilities
 *
 **/
typedef struct {
  cam_hdr_bracketing_info_t hdr_bracketing_setting;
  uint8_t  max_num_roi;
  cam_feature_mask_t feature_mask;
  uint32_t max_face_detection_count;
  uint8_t face_bsgc;
  uint32_t max_frame_applying_delay;
  uint32_t max_meta_reporting_delay;
  cam_af_bracketing_t ubifocus_af_bracketing_need;
  cam_af_bracketing_t refocus_af_bracketing_need;
  cam_still_more_t stillmore_settings;
  cam_opti_zoom_t opti_zoom_settings;
  uint8_t supported_scene_modes_cnt;
  cam_scene_mode_type supported_scene_modes[CAM_SCENE_MODE_MAX];
  cam_scene_mode_overrides_t scene_mode_overrides[CAM_SCENE_MODE_MAX];
  cam_analysis_info_t analysis_info[CAM_ANALYSIS_INFO_MAX];
  cam_true_portrait_t true_portrait_settings;
  cam_chroma_flash_t chroma_flash_settings_need;
  cam_feature_mask_t supported_feature_mask;
  uint32_t bayer_procecessing_delay;
  uint32_t avail_spatial_align_solns;
} mct_pipeline_imaging_cap_t;

typedef struct {
  uint32_t buf_alignment;
  uint32_t min_stride;
  uint32_t min_scanline;
  cam_pad_format_t plane_padding;
  cam_offset_info_t offset_info;
} mct_pipeline_common_cap_t;

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
typedef struct {
  mct_pipeline_sensor_cap_t  sensor_cap;
  mct_pipeline_isp_cap_t     isp_cap;
  mct_pipeline_stats_cap_t   stats_cap;
  mct_pipeline_pp_cap_t      pp_cap;
  mct_pipeline_imaging_cap_t imaging_cap;
  mct_pipeline_common_cap_t  common_cap;
} mct_pipeline_cap_t;

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
typedef struct {
  void* session_stream_info;
  mct_stream_state_t state;
} mct_pipeline_session_stream_info_t;

typedef struct {
  void *aec_libptr;
  void *awb_libptr;
  void *af_libptr;
} mct_pipeline_session_data_q3a_t;

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
typedef struct {
  cam_stream_size_info_t streamConfigInfo;
  unsigned int max_pipeline_frame_applying_delay;
  unsigned int max_pipeline_meta_reporting_delay;
  unsigned int max_sensor_delay;
  mct_pipeline_session_data_q3a_t q3a_lib_info;
  uint32_t min_isp_sw_frame_skip_retry;
  uint32_t sensor_pix_clk;
  boolean dual_vfe_enabled;
  mct_sensor_format_t sensor_format;
  cam_position_t position;
  uint8_t zoom_ratio_tbl_cnt;
  int zoom_ratio_tbl[MAX_ZOOMS_CNT];
  uint32_t set_session_mask;
  boolean zzhdr_hw_available;
  cam_related_system_calibration_data_t dual_data;
  uint32_t is_valid_dualcalib;
  cam_lens_type_t lens_type;
  boolean is_aux_camera;
} mct_pipeline_session_data_t;


/** Name:mct_offline_metadata_t
 *
 *  Description:
 *    This structure holds information about offline metadata stream.
 **/
typedef struct {
  uint32_t output_buf_idx;
  uint32_t stream_id;
  int buf_mgr_fd;
  pthread_mutex_t offline_meta_queue_lock;
  mct_queue_t     *offline_meta_queue;

  /* Bus signalling constructs */
  pthread_mutex_t offline_meta_mutex;
  pthread_cond_t  offline_meta_cond;

  /* Input and output offline reprocess metadata buffers */
  metadata_buffer_t *input_metadata_buffer;
  metadata_buffer_t *output_metadata_buffer;
  uint8_t stream_on_cnt;
}mct_offline_metadata_t;

/** Name:mct_offline_meta_type_t
 *
 *  Description:
 *    This enum holds type of offline metadata collection stream.
 **/
typedef enum {
  MCT_OFFLINE_META_INPUT_BUF,
  MCT_OFFLINE_META_ISP_RD_DONE,
  MCT_OFFLINE_META_THREAD_EXIT,
  MCT_OFFLINE_META_MAX
} mct_offline_meta_type_t;

/** Name:mct_offline_meta_input_buf_index
 *
 *  Description:
 *    This structure holds information for
 *    input buffer of offline metadata collection stream.
 **/
typedef struct {
  uint32_t buf_index;
  uint32_t streamid;
  uint8_t is_offline_meta_bypass;
}mct_offline_meta_input_buf_index;

/** Name:mct_offline_meta_payload
 *
 *  Description:
 *    This structure holds information for
 *    offline metadata payload that post to queue.
 **/
typedef struct {
  mct_offline_meta_type_t type;
  void *data;
  size_t size;
}mct_offline_meta_payload;

typedef struct {
  mct_stream_t *stream;
  mct_event_t event_data;
} mct_pipeline_defer_cmd_t;

typedef boolean (* mct_pipeline_add_stream_func)
  (mct_pipeline_t *pipeline, uint32_t stream_id);

typedef boolean (* mct_pipeline_remove_stream_func)
  (mct_pipeline_t *pipeline, mct_stream_t *stream);

typedef boolean (* mct_pipeline_event_func)
  (mct_pipeline_t *pipeline, uint32_t stream_id, mct_event_t *event);

typedef boolean (* mct_pipeline_set_bus_func)
  (mct_pipeline_t *pipeline, mct_bus_t *bus);

typedef mct_bus_t* (* mct_pipeline_get_bus_func)
  (mct_pipeline_t *pipeline);

typedef boolean (* mct_pipeline_map_buf_func)
  (void *msg, mct_pipeline_t *pipeline);

typedef boolean (* mct_pipeline_unmap_buf_func)
  (void *msg, mct_pipeline_t *pipeline);

typedef boolean (* mct_pipeline_map_parm_func)
  (mct_pipeline_t *pipeline);

typedef boolean (* mct_pipeline_unmap_parm_func)
  (mct_pipeline_t *pipeline);

typedef boolean (* mct_pipeline_proc_msg_func)
  (void *msg, mct_pipeline_t *pipeline);

/** _mct_pipeline
 *
 *    @ module           : Pipeline itself can be a Module, s;
 *    @ bus              : The mct_bus associated with this session. This bus
 *                         is used for sending events up to the mctl.
 *    @ controller       : Context of the controller object which is the parent
 *                         of this pipeline.
 *    @ config_parm      : HAL's configuration buffer
 *    @ query_buf        : Query caps buffer used during boot up time.
 *    @ related_sensors_sync_buf      : Sync buffers for dual cameras
 *    @ modules          : global list of modules to choose from
 *    @mct_module_hw_state_t: HW state of downstream modules
 *    @wake_mode: Decide if wake_mode should be set per-frame or forever
 *    @ session          : session id assigned to this pipeline
 *    @ session_data     : frame and metadata delays on this session
 *    @ session_stream   : MCT internal stream to send
 *                         session-events to all modules
 *    @ pipeline_private : private data @ parm_buf_count :
 *    Number of valid entries in config_parm buffer @
 *                                              valid_parm_index_tbl:
 *    Table for storing indices of valid entries in config_parm
 *    buffer @ frame_num_idx_list: Per-frame control Map for
 *    (frame_num, frame_id) @ super_param_queue: Queue of
 *    super-param buffers sent from HAL @ frame_num_idx_list:
 *    list tracking frame nums and corresponding SOF IDs.

 *    @ add_stream       :
 *    @ remove_stream    :
 *    @ send_event       :
 *    @ set_bus          :
 *    @ get_bus          :
 *    @ map_buf          :
 *    @ unmap_buf        :
 *    @ process_serv_msg :
 *    @ process_bus_msg  :
 *
 *  Pipeline object is unique to a session and is contained in mct_controller.
 *  Pipeline's children shall be streams.
 *
 **/
struct _mct_pipeline {
  uint32_t                session;
  mct_module_t            module;
  /* pipeline's parent is MCT */
  /* pipeline's child is stream */
  mct_object_t            object;
  mct_bus_t               *bus;
  mct_controller_t        *controller;
  mct_pipeline_cap_t       query_data;
  mct_pipeline_session_data_t session_data;
  mct_pipeline_session_stream_info_t session_stream;

  void                    *config_parm;
  size_t                   config_parm_size;
  int                      config_parm_fd;

  void                    *query_buf;
  size_t                   query_buf_size;
  int                      query_buf_fd;

  void                    *related_sensors_sync_buf;
  size_t                   related_sensors_sync_buf_size;
  int                      related_sensors_sync_buf_fd;

  mct_list_t              *modules;
  mct_module_hw_state_t    module_hw_state;
  mct_module_hw_state_t    updated_hw_state;
  uint32_t                 state_update_count;
  mct_module_hw_state_t    current_hw_state_requested;
  uint32_t                 wakeup_bit_mask;
  mct_frame_request_mode_t wake_mode;
  cam_dual_camera_perf_control_t lpm_mode;

  uint16_t                 parm_buf_count;
  uint16_t                 valid_parm_index_tbl[CAM_INTF_PARM_MAX];
  int8_t                   stream_on_cnt;
  mct_list_t              *frame_num_idx_list;
  mct_queue_t             *super_param_queue;
  uint32_t                 delay_dequeue_count;
  void                    *pipeline_private;
  int32_t                  is_zsl_mode;
  pthread_mutex_t          super_param_q_lock;

  mct_queue_t             *param_queue;
  pthread_mutex_t          param_q_lock;

  cam_hal_version_t        hal_version;

  mct_offline_metadata_t   offline_meta;
  uint32_t                 set_param_cnt;
  uint32_t                 sleep_duration;
  boolean                  state_change;
  boolean aux_cam;

  uint32_t                 int_streamoff;
  uint32_t                 max_ack_id;
  uint32_t                 start_int_streamoff_on;
  mct_queue_t              *defer_queue;
  pthread_mutex_t          defer_q_lock;
  pthread_mutex_t          stream_ops_lock;
  cam_dual_camera_defer_cmd_t deferred_state;
  uint32_t                 sof_delay;
  //True if MCT can trigger lpm
  uint32_t                 lpm_owner;

  mct_pipeline_add_stream_func    add_stream;
  mct_pipeline_remove_stream_func remove_stream;
  mct_pipeline_event_func         send_event;
  mct_pipeline_set_bus_func       set_bus;
  mct_pipeline_get_bus_func       get_bus;
  mct_pipeline_map_buf_func       map_buf;
  mct_pipeline_unmap_buf_func     unmap_buf;
  mct_pipeline_map_parm_func      map_parm;
  mct_pipeline_unmap_parm_func     unmap_parm;
  mct_pipeline_proc_msg_func      process_serv_msg;
  mct_pipeline_proc_msg_func      process_bus_msg;
  mct_pipeline_thread_data_t      thread_data;

};

#define MCT_PIPELINE_CAST(mod)         ((mct_pipeline_t *)(mod))
#define MCT_PIPELINE_NUM_CHILDREN(mod) (MCT_OBJECT_NUM_CHILDREN(mod))
#define MCT_PIPELINE_CHILDREN(mod)     (MCT_OBJECT_CHILDREN(mod))
#define MCT_PIPELINE_BUS(mod)          (MCT_PIPELINE_CAST(mod)->bus)
#define MCT_PIPELINE_SESSION(mod)      (MCT_PIPELINE_CAST(mod)->session)
#define MCT_PIPELINE_MODULES(mod)      (MCT_PIPELINE_CAST(mod)->modules)

#define MCT_PIPELINE_PARENT(pipeline_object)  (pipeline_object.parent)
#define MCT_PIPELINE_OBJ_CHILDREN(pipeline_object) (pipeline_object.child)
#define MCT_PIPELINE_LOCK(pipeline_object) pthread_mutex_lock \
                                             (&pipeline_object.lock)
#define MCT_PIPELINE_UNLOCK(pipeline_object) pthread_mutex_unlock \
                                             (&pipeline_object.lock)

/** mct_pipeline_check_stream_t
 *    @
 *    @
 *
 **/
typedef enum _mct_pipeline_check_stream {
  CHECK_INDEX,
  CHECK_TYPE,
  CHECK_SESSION,
  CHECK_FEATURE_MASK
} mct_pipeline_check_stream_t;

/** mct_pipeline_get_stream_info_t
 *    @
 *    @
 **/
typedef struct _mct_pipeline_get_stream_info {
  mct_pipeline_check_stream_t check_type;
  uint32_t                    stream_index;
  cam_stream_type_t           stream_type;
  uint32_t                    session_index;
  cam_feature_mask_t          feature_mask;
} mct_pipeline_get_stream_info_t;

#if defined(__cplusplus)
extern "C" {
#endif

mct_pipeline_t* mct_pipeline_new (unsigned int session_idx,
  mct_controller_t *pController);
void mct_pipeline_destroy(mct_pipeline_t *pipeline);

cam_status_t mct_pipeline_start_session(mct_pipeline_t *pipeline);
void mct_pipeline_stop_session(mct_pipeline_t *pipeline);

boolean mct_pipeline_send_ctrl_events(mct_pipeline_t *pipeline,
  mct_event_control_type_t event_type, boolean send_immediately);

boolean mct_pipeline_control_hw_state(mct_pipeline_t *pipeline,
  mct_bus_msg_ctrl_request_frame_t *bus_msg);

boolean mct_pipeline_send_module_wakeup(mct_pipeline_t *pipeline);

boolean mct_pipeline_send_module_sleep(mct_pipeline_t *pipeline);

boolean mct_pipeline_flush_super_param_queue(mct_pipeline_t *pipeline);

mct_stream_t* mct_pipeline_find_stream (mct_module_t *module,
  unsigned int session_id);

mct_stream_t* mct_pipeline_get_stream(mct_pipeline_t *pipeline,
  mct_pipeline_get_stream_info_t *get_info);

void *mct_pipeline_get_buffer_ptr(mct_pipeline_t *pipeline, uint32_t buf_idx,
  uint32_t stream_id);

mct_stream_map_buf_t *mct_pipeline_get_buffer(mct_pipeline_t *pipeline,
  uint32_t buf_idx, uint32_t stream_id);

mct_stream_t* mct_pipeline_find_stream_from_stream_id
  (mct_pipeline_t *pipeline, uint32_t stream_id);

boolean mct_pipeline_pack_event(mct_event_type type, unsigned int identity,
  mct_event_direction direction, void *payload, mct_event_t* event);

boolean mct_pipeline_send_special_event(mct_pipeline_t *pipeline,
  mct_stream_t *parm_stream);

boolean mct_pipeline_free_param(void *data, void *user_data);

boolean mct_pipeline_unmap_buf(void *message, mct_pipeline_t *pipeline);

mct_pipeline_session_data_t const* mct_pipeline_lookup_session_data (
  mct_module_t *module, uint32_t sessionid);

boolean mct_pipeline_flush_defer_queue(mct_pipeline_t *pipeline);

boolean mct_pipeline_process_defer_queue(mct_pipeline_t *pipeline);

boolean mct_pipeline_push_defer_cmd(mct_pipeline_t * pipeline,
  mct_event_t *event, mct_stream_t *stream);


#if defined(__cplusplus)
}
#endif

#endif /* __MCT_PIPELINE_H__ */
