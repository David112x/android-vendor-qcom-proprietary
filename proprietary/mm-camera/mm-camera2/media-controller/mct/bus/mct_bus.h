/* mct_bus.h
 *
 * Copyright (c) 2012-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __MCT_BUS_H__
#define __MCT_BUS_H__

#ifndef DBG_SOF_FREEZE
#define DBG_SOF_FREEZE
#endif

#include "media_controller.h"
#include "mct_queue.h"
#include "mct_event_stats.h"
#include "mct_event.h"
#include "mct_refcount.h"

#define MCT_BUS_DEFAULT_SOF_TIMEOUT 5 /* Default SOF timeout duration in sec */

typedef mct_hdr_data mct_bus_msg_hdr_data;

/** mct_bus_metadata_collection_type_t:
 *
 *  module can post bus message for online or offline metadata.
 *  To distinguish between online / offline metadata, added this enum type.
 *  By default, it is online metadata.
 **/
typedef enum {
  MCT_BUS_ONLINE_METADATA,
  MCT_BUS_OFFLINE_METADATA,
}mct_bus_metadata_collection_type_t;

/* mct_bus_msg_type_t:
 *
 */
typedef enum {
  MCT_BUS_MSG_ISPIF_SOF,
  MCT_BUS_MSG_ISP_SOF,      /* mct_bus_msg_isp_sof_t */
  MCT_BUS_MSG_ISP_STATS_AF, /* mct_bus_msg_isp_stats_af_t */
  MCT_BUS_MSG_ISP_STREAM_CROP, /* mct_bus_msg_stream_crop_t */
  MCT_BUS_MSG_ISP_SESSION_CROP, /* mct_bus_msg_session_crop_info_t */
  MCT_BUS_MSG_Q3A_AF_STATUS,/* mct_bus_msg_isp_stats_af_t */
  MCT_BUS_MSG_FACE_INFO,    /* cam_face_detection_data_t */
  MCT_BUS_MSG_PREPARE_HW_DONE, /* cam_prep_snapshot_state_t */
  MCT_BUS_MSG_ZSL_TAKE_PICT_DONE,  /* cam_frame_idx_range_t */
  MCT_BUS_MSG_HIST_STATS_INFO, /* cam_hist_stats_t */
  MCT_BUS_MSG_SET_SENSOR_INFO, /*mct_bus_msg_sensor_metadata_t*/
  MCT_BUS_MSG_SET_STATS_AEC_INFO, /*mct_bus_msg_stats_aec_metadata_t*/
  MCT_BUS_MSG_SET_ISP_STATS_AWB_INFO, /*mct_bus_msg_isp_stats_awb_metadata_t*/
  MCT_BUS_MSG_SET_ISP_GAMMA_INFO, /*mct_bus_msg_isp_gamma_t*/
  MCT_BUS_MSG_ERROR_MESSAGE, /*mct_bus_msg_error_message_t*/
  MCT_BUS_MSG_AE_INFO, /* cam_ae_params_t */
  MCT_BUS_MSG_CAC_STAGE_DONE, /*cam_cac_info_t*/
  MCT_BUS_MSG_IMG_HYSTERSIS_INFO, /*cam_img_hysterisis_info_t*/
  MCT_BUS_MSG_SENSOR_STARTING,/*NULL*/
  MCT_BUS_MSG_SENSOR_STOPPING,/*NULL*/
  MCT_BUS_MSG_NOTIFY_KERNEL,
  MCT_BUS_MSG_SEND_HW_ERROR,  /*NULL*/
  MCT_BUS_MSG_CONTROL_REQUEST_FRAME,  /*mct_bus_msg_ctrl_request_frame_t*/
  MCT_BUS_MSG_SENSOR_AF_STATUS,
  MCT_BUS_MSG_AUTO_SCENE_INFO, /*mct_bus_msg_asd_decision_t*/
  MCT_BUS_MSG_SCENE_MODE, /* int32_t*/
  MCT_BUS_MSG_AE_EZTUNING_INFO, /* ae_eztuning_params_t */
  MCT_BUS_MSG_AWB_EZTUNING_INFO,/* awb_eztuning_params_t */
  MCT_BUS_MSG_AF_EZTUNING_INFO, /* af_eztuning_params_t */
  MCT_BUS_MSG_ASD_EZTUNING_INFO,
  MCT_BUS_MSG_AE_EXIF_DEBUG_INFO,
  MCT_BUS_MSG_AWB_EXIF_DEBUG_INFO,
  MCT_BUS_MSG_AF_EXIF_DEBUG_INFO,
  MCT_BUS_MSG_ASD_EXIF_DEBUG_INFO,
  MCT_BUS_MSG_STATS_EXIF_DEBUG_INFO,
  MCT_BUS_MSG_BESTATS_EXIF_DEBUG_INFO,
  MCT_BUS_MSG_BHIST_EXIF_DEBUG_INFO,
  MCT_BUS_MSG_3A_TUNING_EXIF_DEBUG_INFO,
  MCT_BUS_MSG_ISP_CHROMATIX_LITE,
  MCT_BUS_MSG_PP_CHROMATIX_LITE,
  MCT_BUS_MSG_ISP_META,
  MCT_BUS_MSG_ISP_GAIN, /* int32_t */
  MCT_BUS_MSG_SENSOR_BET_META,
  MCT_BUS_MSG_SET_ISP_LENS_SHADING_INFO, /* cam_lens_shading_map_t */
  MCT_BUS_MSG_SET_ISP_LENS_SHADING_MODE, /* cam_lens_shading_mode_t */
  MCT_BUS_MSG_SET_ISP_LENS_SHADING_MAP_MODE, /* cam_lens_shading_map_mode_t */
  MCT_BUS_MSG_SET_ISP_TONE_MAP, /* cam_tonemap_curve_t */
  MCT_BUS_MSG_SET_ISP_TONE_MAP_MODE, /* cam_tonemap_mode_t */
  MCT_BUS_MSG_SET_ISP_CC_MODE, /* cam_color_correct_mode_t */
  MCT_BUS_MSG_SET_ISP_CC_TRANSFORM, /* cam_color_correct_matrix_t */
  MCT_BUS_MSG_SET_ISP_PRED_CC_TRANSFORM, /* cam_color_correct_matrix_t */
  MCT_BUS_MSG_SET_ISP_PRED_CC_GAIN, /* cam_color_correct_gains_t */
  MCT_BUS_MSG_SET_ISP_CC_GAIN, /* cam_color_correct_gains_t */
  MCT_BUS_MSG_SET_ISP_BLACK_LEVEL_LOCK, /* uint8_t */
  MCT_BUS_MSG_SET_ISP_CONTROL_MODE, /* uint8_t */
  MCT_BUS_MSG_SET_ISP_ABF_MODE, /* uint8_t */
  MCT_BUS_MSG_SET_ISP_HOT_PIX_MODE, /* uint8_t */
  MCT_BUS_MSG_AEC_IMMEDIATE,   /* mct_bus_msg_aec_immediate_t */
  MCT_BUS_MSG_AEC,           /* TODO */
  MCT_BUS_MSG_AF_IMMEDIATE, /* mct_bus_msg_af_immediate_t */
  MCT_BUS_MSG_AF,          /* mct_bus_msg_af_t */
  MCT_BUS_MSG_AF_UPDATE,   /* mct_bus_msg_af_update_t */
  MCT_BUS_MSG_AWB,          /* mct_bus_msg_awb_immediate_t */
  MCT_BUS_MSG_AWB_IMMEDIATE,/* mct_bus_msg_awb_t */
  MCT_BUS_MSG_SET_ISP_CAPTURE_INTENT, /* uint8_t */
  MCT_BUS_MSG_SET_ISP_ZOOM_VALUE, /* cam_zoom_info_t */
  MCT_BUS_MSG_SET_ISP_CONTRAST,  /* int32_t */
  MCT_BUS_MSG_SET_ISP_SATURATION,  /* int32_t */
  MCT_BUS_MSG_SET_AEC_STATE, /*uint8_t*/
  MCT_BUS_MSG_SET_AEC_RESET, /* NULL */
  MCT_BUS_MSG_SET_SENSOR_SENSITIVITY, /*int32_t*/
  MCT_BUS_MSG_SET_SENSOR_EXPOSURE_TIME, /* int64_t */
  MCT_BUS_MSG_SET_SENSOR_FRAME_DURATION, /* int64_t */
  MCT_BUS_MSG_ISP_CROP_REGION, /* cam_crop_region_t */
  MCT_BUS_MSG_SET_FACEDETECT_MODE, /* cam_face_detect_mode_t */
  MCT_BUS_MSG_SENSOR_INFO, /*cam_sensor_params_t*/
  MCT_BUS_MSG_SENSOR_APERTURE, /* float */
  MCT_BUS_MSG_SENSOR_FOCAL_LENGTH, /* float */
  MCT_BUS_MSG_SENSOR_FILTERDENSITY, /* float */
  MCT_BUS_MSG_SENSOR_OPT_STAB_MODE, /* uint8_t */
  MCT_BUS_MSG_AFD, /* mct_bus_msg_afd_t */
  MCT_BUS_MSG_SENSOR_FLASH_MODE, /* cam_flash_ctrl_t */
  MCT_BUS_MSG_SENSOR_FLASH_STATE, /* cam_flash_state_t */
  MCT_BUS_MSG_FRAME_DROP, /*uint32_t: stream type*/
  MCT_BUS_MSG_SET_SHARPNESS, /* int32_t */
  MCT_BUS_MSG_SET_EFFECT, /* int32_t */
  MCT_BUS_MSG_SET_EDGE_MODE, /* cam_edge_application_t */
  MCT_BUS_MSG_SET_NOISE_REDUCTION_MODE, /* int32_t */
  MCT_BUS_MSG_SET_WAVELET_DENOISE, /* cam_denoise_param_t */
  MCT_BUS_MSG_SET_TEMPORAL_DENOISE, /* cam_denoise_param_t */
  MCT_BUS_MSG_SET_FPS_RANGE, /* cam_fps_range_t */
  MCT_BUS_MSG_SET_ROTATION, /* cam_rotation_t */
  MCT_BUS_MSG_REPROCESS_STAGE_DONE, /* NULL */
  MCT_BUS_MSG_PREPARE_HDR_ZSL_DONE, /* cam_prep_snapshot_state_t */
  MCT_BUS_MSG_CLOSE_CAM,        /* Simulated message for ending process */
  MCT_BUS_MSG_META_CURRENT_SCENE, /* cam_scene_mode_type */
  MCT_BUS_MSG_DELAY_SUPER_PARAM,       /* mct_bus_msg_delay_dequeue_t */
  MCT_BUS_MSG_FRAME_SKIP,       /* mct_bus_msg_delay_dequeue_t */
  MCT_BUS_MSG_SENSOR_EXPOSURE_TIMESTAMP, /* int64_t */
  MCT_BUS_MSG_SENSOR_ROLLING_SHUTTER_SKEW, /* int64_t */
  MCT_BUS_MSG_EZTUNE_JPEG,
  MCT_BUS_MSG_EZTUNE_RAW,
  MCT_BUS_MSG_PROFILE_TONE_CURVE, /*cam_profile_tone_curve */
  MCT_BUS_MSG_NEUTRAL_COL_POINT,   /* cam_neutral_col_point_t */
  MCT_BUS_MSG_SET_CAC_MODE, /*cam_aberration_mode_t*/
  MCT_BUS_MSG_IFACE_METADATA, /* mct_bus_msg_isp_metadata_t */
  MCT_BUG_MSG_OTP_WB_GRGB, /* float */
  MCT_BUS_MSG_LED_REDEYE_REDUCTION_MODE,
  MCT_BUS_MSG_TEST_PATTERN_DATA,
  MCT_BUS_MSG_LED_MODE_OVERRIDE,
  MCT_BUS_MSG_SNAP_CROP_INFO_SENSOR, /*crop for binning & FOV adjust*/
  MCT_BUS_MSG_SNAP_CROP_INFO_CAMIF,  /*crop for trimming edge pixels*/
  MCT_BUS_MSG_SNAP_CROP_INFO_ISP,    /*crop for FOV-adjust and zoom (1 of 2)*/
  MCT_BUS_MSG_SNAP_CROP_INFO_PP,     /*crop for EIS, DIS, and zoom (2 of 2)*/
  MCT_BUS_MSG_TRIGGER_REGION,
  MCT_BUS_MSG_DCRF_RESULT, /* cam_dcrf_result_t */
  MCT_BUS_MSG_BUF_DIVERT_INFO,
  MCT_BUS_MSG_SET_CDS, /* int32_t */
  MCT_BUS_MSG_SET_ISP_LPM_INFO,
  MCT_BUS_MSG_ISP_RD_DONE, /* mct_bus_msg_isp_rd_done */
  MCT_BUS_MSG_SET_IMG_DYN_FEAT,
  MCT_BUS_MSG_FACE_INFO_FACE_LANDMARKS,
  MCT_BUS_MSG_FACE_INFO_BLINK,
  MCT_BUS_MSG_FACE_INFO_SMILE,
  MCT_BUS_MSG_FACE_INFO_GAZE,
  MCT_BUS_MSG_FACE_INFO_CONTOUR,
  MCT_BUS_MSG_FACE_INFO_RECOG,
  MCT_BUS_MSG_SET_SENSOR_HDR_MODE,
  MCT_BUS_MSG_SET_VIDEO_STAB_MODE, /* int32_t */
  MCT_BUS_MSG_SET_IR_MODE,
  MCT_BUS_MSG_DC_SAC_OUTPUT_INFO, /* cam_sac_output_info_t */
  MCT_BUS_MSG_SET_HDR_CURVES_INFO,  /* mct_bus_msg_hdr_data */
  MCT_BUS_MSG_SET_TINTLESS,
  MCT_BUS_MSG_LED_CAL_RESULT,
  MCT_BUS_INTMSG_PROC_SOF_WO_PARAM,
  MCT_BUS_INTMSG_PROC_SOF_W_PARAM,
  MCT_BUS_MSG_STATS_AEC_META,  /* tuning_mod1_data_AEC */
  MCT_BUS_MSG_STATS_AWB_META,  /* tuning_mod1_data_AWB */
  MCT_BUS_MSG_STATS_AF_META,   /* tuning_mod1_data_AWB */
  MCT_BUS_MSG_RTB_RESULT,
  MCT_BUS_MSG_OIS_READ_DATA,
  MCT_BUS_MSG_MAX
}mct_bus_msg_type_t;

/** _mct_bus_msg_isp_sof:
 *    @frame_id:    frame id
 *    @timestamp:   timestamp
 *    @num_streams: number of streams associated with sof
 *    @streamids:   stream id list.
 *
 *  This structure defines the sof bus message
 **/
#define MAX_STREAMS_NUM 8

typedef enum {
  ISP_META_REGISTER_DUMP,
  ISP_META_ROLLOFF_TBL,
  ISP_META_GAMMA_TBL,
  ISP_META_LINEARIZATION_TBL, /*not use for now*/
  ISP_META_LA_TBL, /*not use for now*/
  ISP_META_LTM_TBL,
  ISP_META_ABF_TBL,
  ISP_META_PEDESTAL_TBL,
  ISP_META_GIC_TBL,
  ISP_META_GTM_TBL,
  ISP_META_BE_STATS,
  ISP_META_BG_STATS,
  ISP_META_BF_STATS,
  ISP_META_RS_CS_STATS,
  ISP_META_HIST_STATS,
  ISP_META_BHIST_STATS,
  ISP_META_HDRBHIST_STATS,
  ISP_META_STATS_BF_RGN_IND_LUT,
  ISP_META_STATS_BF_GAMMA_LUT,
  ISP_META_PDPC_TBL,
  ISP_META_VFE_ADDL_INFO, // Like in K //COmpatability
  ISP_META_SE_SENSOR,
  ISP_META_MAX,
} isp_meta_dump_t;

typedef struct {
  uint32_t dump_type; /*reg dump, DMI 16bit, DMI 32 bit, DMI 64 bit*/
  uint32_t len;
  uint32_t start_addr;
  uint8_t  isp_meta_dump[6144]; /* 512 * 3 * 4.
                                   Gamma is largest dmi table
                                   512 entries * 3 tables * 4 byte each entry */
}__attribute__((packed, aligned(8))) isp_meta_entry_t;

/** vfe_addl_info_t:
 *    @CDS_enable: cds enabled or disabled
 *    @CCM_enabled
 *    @raw_width: concurrent raw frame width
 *    @raw_height: concurrent raw frame height
 *    @raw_stride: concurrent raw frame stride
 *    @raw_scanline: concurrent raw frame scanlines
 */
typedef struct {
  uint32_t CDS_enable;
  uint32_t CCM_enabled;
  uint32_t raw_width;
  uint32_t raw_height;
  uint32_t raw_stride;
  uint32_t raw_scanline;
  uint32_t reserved[16];
} vfe_addl_info_t;

typedef struct
{
  double ltm_ratio;
  double la_ratio;
  double gtm_ratio;
  double gamma_ratio;
  double color_drc_gain;
  double exposure_ratio; //( or aec_sensitivity_ratio)
  double exposure_time_ratio;
  double reserved_data[32]; //total_drc_gain
}adrc_adv_feature_info;

/** isp_meta_t:
 *    @isp_version: VFE44/40/32
 *    @awb_gain: r_gain, g_gain, b_gain
 *    @color_temp: color temp
 *    @lux_idx: lunx index
 *    @se_lux_idx: short exposure lux index
 *    @exp_ratio: exposure ratio
 *    @addlinfo: addl info ( cds, ccm, raw dim etc.)
 *    @num_entry: number of reg dump/ dmi tbl entries
 *    @meta_entry: reg dump/ dmi tbl dump
 */
typedef struct {
  uint32_t isp_version; /* VFE40/32... */
  awb_gain_t awb_gain; /*r_gain, g_gain, b_gain*/
  uint32_t color_temp;
  float lux_idx;
  float  se_lux_idx ;
  float exp_ratio ;
  vfe_addl_info_t addlinfo;
  adrc_adv_feature_info adrc_info;
  uint32_t num_entry;
}__attribute__((packed, aligned(8))) isp_frame_meta_t;

typedef struct {
  isp_frame_meta_t frame_meta;
  isp_meta_entry_t meta_entry[ISP_META_MAX]; /* reg dump, dmi tbl dump*/
}__attribute__((packed, aligned(8)))isp_meta_t;

typedef struct _isp_kstate_ptr_t {
  mct_refcount_t refc;
  struct isp_kstate *isp_kstate_ptr;
} isp_kstate_ptr_t;

typedef struct _mct_bus_msg_isp_sof {
  unsigned int frame_id;
  struct timeval timestamp;
  struct timeval mono_timestamp;
  int num_streams;
  uint32_t streamids[MAX_STREAMS_NUM];
  uint32_t prev_sent_streamids[MAX_STREAMS_NUM];
  int32_t skip_meta;
  uint32_t frame_src; /* enum msm_vfe_input_src */
  isp_kstate_ptr_t *kstate_ptr;
  int tardy;
  uint8_t regs_not_updated;
  uint32_t remap_start_id;
  uint64_t nano_timestamp;
} mct_bus_msg_isp_sof_t;

typedef struct __mct_bus_msg_meta_valid {
  uint32_t frame_id;
  uint32_t meta_valid;
} mct_bus_msg_meta_valid;

typedef enum
{
  FRM_REQ_MODE_NONE,
  FRM_REQ_MODE_CONTINUOUS,
  FRM_REQ_MODE_BURST,
  FRM_REQ_MODE_MAX
} mct_frame_request_mode_t;

typedef enum
{
  LPM_REQUEST_BIT_PRIORITY = 1 << 0,
  LPM_REQUEST_BIT_MCT      = 1 << 1,
  LPM_REQUEST_BIT_SENSOR   = 1 << 2,
  LPM_REQUEST_BIT_ISP      = 1 << 3,
  LPM_REQUEST_BIT_PPROC    = 1 << 4,
  LPM_REQUEST_BIT_STATS    = 1 << 5,
  LPM_REQUEST_BIT_STATS_IS = 1 << 6
  /*Each new entry in this enum should also be added in
  mct_pipeline_is_valid_lpm_req_bit function to identify it
  as a valid request bit*/
} mct_lpm_request_bit_t;

typedef struct {
  boolean request_flag;  /* TRUE - start requesting frame; FALSE - stop */
  mct_frame_request_mode_t req_mode;  /* Type of request */
  mct_lpm_request_bit_t lpm_req_bit;
} mct_bus_msg_ctrl_request_frame_t;

typedef struct {
  void *chromatix_ptr;
  void *common_chromatix_ptr;
  void *cpp_chromatix_ptr;
  void *cpp_snapchromatix_ptr;
  void *cpp_ois_ds_chromatix_ptr;
  void *cpp_ois_us_chromatix_ptr;
  void *cpp_flsh_snapchromatix_ptr;
  void *cpp_ds_chromatix_ptr;
  void *cpp_us_chromatix_ptr;
  void *cpp_ois_snapchromatix_ptr;
  void *cpp_videochromatix_ptr;
  void *postproc_chromatix_ptr;
  void *a3_chromatix_ptr;
  void *iot_chromatix_ptr;
  int32_t use_stripped_chromatix;
  uint16_t modules_reloaded;
} mct_bus_msg_sensor_metadata_t;

typedef struct {
  char private_data[350];
} mct_bus_msg_stats_aec_metadata_t;

typedef struct {
  char private_data[468];
} mct_bus_msg_isp_stats_awb_metadata_t;

typedef struct {
  char private_data[300];
} mct_bus_msg_stats_awb_metadata_t;

/* 512 - Maximum table size of gamma *
  * 3 - G/B/R *
  */
#define GAMMA_DATA_SIZE (512*3*sizeof(uint32_t))
/** mct_bus_msg_isp_gamma_t:
 *    @isp_version: VFE44/40/32
 *    @private_data: data array of size max
 *    @entry_size: size of each gamma entry. 2 byte for vfe40, 4: vfe44
 *    @num_entries num of entries in gamma table
 *    @num_tables: num of tables gamma posted (R/G/B/all)
 *    @pack_bits: num of bits to use for value (delta << pack_bits)
 */
typedef struct {
  uint32_t  isp_version;
  char      private_data[GAMMA_DATA_SIZE];
  uint32_t  entry_size;
  uint32_t  num_entries;
  uint32_t  num_tables;
  uint32_t  pack_bits;
} mct_bus_msg_isp_gamma_t;

typedef enum {
  MCT_ERROR_MSG_NOT_USED,
  MCT_ERROR_MSG_RSTART_VFE_STREAMING, /* restart VFE straeming */
  MCT_ERROR_MSG_RESUME_VFE_STREAMING, /* resume pending VFE streaming */
  MCT_ERROR_MSG_INT_RESTART_STREAMING,
}mct_bus_msg_error_message_t;

/* mct_bus_msg_isp_stats_af_t:
 *
 */
typedef struct _mct_bus_msg_isp_stats_af {
  unsigned int frame_id;
  struct timeval timestamp;
} mct_bus_msg_isp_stats_af_t;

typedef struct {
  int32_t bhist_meta_enable;
  cam_hist_stats_t bhist_info;
} mct_bus_msg_isp_bhist_stats_t;

typedef struct _mct_bus_msg_delay_dequeue {
  uint32_t curr_frame_id;
  uint32_t delay_numframes;
}mct_bus_msg_delay_dequeue_t;

typedef struct {
  uint32_t frame_id;
  struct timeval timestamp;
  uint32_t session_id;
  uint32_t stream_id;
  uint32_t x_map; /* x map on input */
  uint32_t y_map; /* y map on input */
  uint32_t width_map;
  uint32_t height_map;
  uint32_t x; /* left */
  uint32_t y; /* top */
  uint32_t crop_out_x; /* width */
  uint32_t crop_out_y; /* height */
  uint32_t user_zoom;
  uint32_t stream_zoom;
  float width_margin;
  float height_margin;
  float scale_ratio;
} mct_bus_msg_stream_crop_t;

typedef struct {
  uint32_t num_of_streams;
  mct_bus_msg_stream_crop_t crop_info[MAX_NUM_STREAMS];
} mct_bus_msg_session_crop_info_t;


/** mct_bus_msg_af_status:
  *  AF status that needs to be sent to HAL
  *
  *  @focus_state: current focus status
  *
  *  @focus_distance: focus distance
  **/
typedef struct _mct_bus_msg_af_status {
  cam_autofocus_state_t focus_state;
  cam_focus_distances_info_t f_distance;
  cam_focus_mode_type focus_mode;
} mct_bus_msg_af_status_t;

 /** ASD scene specific info
 *
 * detected: scene considered as detected by ASD algo
 * confidence: how confident is ASD about the scene 1.0 to 0.0 (%)
 * auto_compensation: Internal compensation was requested (true/false)
 */
typedef struct {
  boolean           detected;
  float             confidence;
  boolean           auto_compensation;
} mct_asd_scene_info_t;

 /** ASD decision and scene data
 *
 * detected_scene: Main scene detected
 * max_n_scenes: Max number of scenes supported
 * scene_info: Details of all the supported scenes
 */
typedef struct {
  cam_auto_scene_t      detected_scene;
  uint8_t               max_n_scenes;
  mct_asd_scene_info_t  scene_info[S_MAX];
} mct_bus_msg_asd_decision_t;

typedef struct _mct_bus_msg_asd_hdr_status {
  uint32_t is_hdr_scene;
  float    hdr_confidence;
} mct_bus_msg_asd_hdr_status_t;

typedef struct {
  uint8_t aec_state;
  cam_trigger_t aec_trigger;
  uint8_t aec_mode;
  int32_t led_mode;
  int32_t touch_ev_status;
  float lux_index;
  int32_t lcd_flash_settled;
} mct_bus_msg_aec_immediate_t;

typedef struct {
  cam_area_t aec_roi;
  boolean    aec_roi_valid;
  int32_t exp_comp;
  uint8_t ae_lock;
  cam_fps_range_t fps;
  cam_low_light_mode_t lls_flag;
  float conv_speed;
} mct_bus_msg_aec_t;

typedef struct {
  uint8_t awb_state;
  int32_t awb_mode;
  cam_awb_params_t awb_info;
  int        awb_decision;
} mct_bus_msg_awb_immediate_t;

typedef struct {
  cam_area_t awb_roi;
  uint32_t   awb_lock;
  float      conv_speed;
} mct_bus_msg_awb_t;

/* mct_bus_msg_af_update_t
 *
 **/
typedef struct _mct_bus_msg_af_update_t {
  cam_focus_distances_info_t f_distance;
} mct_bus_msg_af_update_t;

/* mct_bus_msg_af_immediate_t
 *
 **/
typedef struct _mct_bus_msg_af_immediate_t {
  cam_af_lens_state_t lens_state;
  uint8_t af_state;
  uint8_t force_update;
  uint8_t focus_mode;
  cam_trigger_t af_trigger;
  float diopter;
  uint32_t scale_ratio;
  uint32_t focus_pos;
  float focal_length_ratio;
  float lens_shift_um;
  int object_distance_cm;
  int near_field_cm;
   int far_field_cm;
  boolean is_depth_based_focus;
  float focus_value;
  boolean spot_light_detected;
} mct_bus_msg_af_immediate_t;

/* mct_bus_msg_af_t
 *
 */
typedef struct _mct_bus_msg_af_t {
  boolean    af_roi_valid;
  cam_area_t af_roi;
  cam_rect_t default_roi;
} mct_bus_msg_af_t;

/* mct_bus_msg_afd_t
 *
 **/
typedef struct {
  uint8_t scene_flicker;
  cam_antibanding_mode_type antibanding_mode;
} mct_bus_msg_afd_t;

/* mct_bus_msg_t:
 *
 */
typedef struct _mct_bus_msg {
  uint32_t sessionid;
  mct_bus_msg_type_t type;
  uint32_t size;
  void *msg;
  mct_bus_metadata_collection_type_t metadata_collection_type;
} mct_bus_msg_t;

typedef struct _mct_bus_msg_buf_divert_info {
  uint32_t frame_id;
  uint32_t num_streams;
  uint32_t stream_id[MAX_NUM_STREAMS];
} mct_bus_msg_buf_divert_info_t;

typedef struct _mct_bus_msg_isp_rd_done {
  unsigned int frame_id;
  struct timeval timestamp;
} mct_bus_msg_isp_rd_done_t;

typedef boolean (*post_msg_to_bus_func)
  (mct_bus_t *bus, mct_bus_msg_t *msg);

struct _mct_bus {
  unsigned int session_id;
  /* Queue for holding metadata module bus msgs. */
  mct_queue_t     *bus_queue;
  /* Queue for holding msgs needing immediate attention*/
  mct_queue_t  *priority_queue;
  /* Mutex for bus_queue */
  pthread_mutex_t bus_msg_q_lock;
  /* Mutex for priority_queue */
  pthread_mutex_t priority_q_lock;
  /* Bus signalling constructs */
  pthread_mutex_t *mct_mutex;
  pthread_cond_t  *mct_cond;
  post_msg_to_bus_func post_msg_to_bus;
  /* SOF-monitor thread signalling constructs */
  pthread_mutex_t bus_sof_msg_lock;
  pthread_cond_t  bus_sof_msg_cond;
  pthread_mutex_t bus_sof_init_lock;
  pthread_cond_t  bus_sof_init_cond;
  pthread_t       bus_sof_tid;
  int             thread_run;
  uint32_t        prev_sof_id;
  mct_bus_msg_type_t msg_to_send_metadata;
  uint32_t        thread_wait_time;
};

void mct_bus_queue_flush(mct_bus_t *bus);
void mct_bus_priority_queue_flush(mct_bus_t *bus);
mct_bus_t *mct_bus_create(unsigned int session);
void mct_bus_destroy(mct_bus_t *bus);
boolean mct_bus_sof_tracker(mct_bus_t *bus, uint32_t cur_sof_id);

#endif /* __MCT_BUS_H__ */
