/* modules.h
 *
 * Copyright (c) 2012-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __MODULES_H__
#define __MODULES_H__

#include <media/msmb_camera.h>
#include "mct_stream.h"

mct_module_t *module_iface_init(const char *name);
void  module_iface_deinit(mct_module_t *mod);
mct_module_t *module_isp_init(const char *name);
void  module_isp_deinit(mct_module_t *mod);

#define SENSOR_CID_CH_MAX 8
#define MAX_META          8
#define ISP_NUM_PIX_STREAM_MAX 8
#define PPROC_DIVERT_PROCESSED    (1 << 1)
#define PPROC_DIVERT_UNPROCESSED  (1 << 2)
#define PPROC_PROCESS             (1 << 3)
#define PPROC_INVALID_IDENTITY    0x00000000
#define MAX_DEFECTIVE_PIXELS      512
#define MAX_STREAM_PLANES         8
#define MAX_VIDEO_HDR_FRAMES         3
#define MAX_PACK_MODE_OVERRIDES 4

#if 0
typedef enum _modulesEventID {
  GET_CHROMATIX_PTR = 1,
  /* MCT events start */
  MCT_ADD_STREAM,
  MCT_REMOVE_STREAM,
  MCT_STREAM_ON,
  MCT_STREAM_OFF,
  /* MCT events end */
  /* sensor events start */
  MODULE_SENSOR_STREAM_CONFIG,
  MODULE_SENSOR_SET_CHROMATIX_PTR,
  /* sensor events end */
  /* IFACE events start */
  MODULE_IFACE_STREAM_CONFIG,
  /* IFACE event end */
  /* VFE events start */
  MODULE_VFE_SOF,
  /* VFE events end */
  /* 3A events start */
  MODULE_3A_AEC_UPDATE,
  MODULE_3A_AF_UPDATE,
  MODULE_3A_GET_CUR_FPS,
  /* 3A events end */
} modulesEventID_t;
#endif

/*
 * @ PACK_MODE_BYTE = output data as is from input
 * @ PACK_MODE_PLAIN_PACK = Used for RDI path to unpack MIPI data
 * @ PACK_MODE_NV_P8 = Used for RDI pat to pack into Plain 8
 * @ PACK_MODE_NV_P16 =Used for RDI pat to pack into Plain 16
*/
typedef enum {
  PACK_MODE_BYTE,
  PACK_MODE_PLAIN_PACK,
  PACK_MODE_NV_P8,
  PACK_MODE_NV_P16
} iface_output_pack_mode;

/** af_sw_filter_type: AF SW stats filter type
 **/
typedef enum {
  AFS_OFF = 0,
  AFS_ON_IIR,
  AFS_ON_FIR,
} af_sw_filter_type;

typedef enum {
  CAMERA_CHROMATIX_MODULE_ISP = 1,
  CAMERA_CHROMATIX_MODULE_CPP = 2,
  CAMERA_CHROMATIX_MODULE_SW_PPROC = 4,
  CAMERA_CHROMATIX_MODULE_3A = 8,
  CAMERA_CHROMATIX_MODULE_IOT = 16,
  CAMERA_CHROMATIX_MODULE_ALL = 0xFFFF,
} camera_chromatix_module;

typedef struct _modulesChromatix {
  void *chromatixPtr;
  void *chromatixComPtr;
  void *chromatixCppPtr;
  void *chromatixOisSnapCppPtr;
  void *chromatixOisUsCppPtr;
  void *chromatixOisDsCppPtr;
  void *chromatixSnapCppPtr;
  void *chromatixFlashSnapCppPtr;
  void *chromatixDsCppPtr;
  void *chromatixUsCppPtr;
  void *chromatixVideoCppPtr;
  void *chromatixPostProcPtr;
  void *chromatixCppStrippedPtr;
  void *chromatix3APtr;
  void *chromatixIotPtr;
  char *external;
  int32_t use_stripped;
  uint16_t modules_reloaded;
} modulesChromatix_t;

typedef struct _modules_liveshot_Chromatix {
  void *liveshot_chromatix_ptr;
} modules_liveshot_Chromatix_t;

typedef struct _af_rolloff_info_t {
  void *rolloff_tables_macro;
  uint16_t af_macro;
  uint16_t af_infinity;
} af_rolloff_info_t;

typedef struct _sensor_dim_output {
  uint32_t width;
  uint32_t height;
} sensor_dim_output_t;

typedef struct {
  uint32_t need_cds_subsample;
  uint32_t w_scale;
  uint32_t v_scale;
} iface_cds_info_t;

typedef struct isp_cds_request {
  boolean cds_request;
  boolean cds_request_done;
  iface_cds_info_t cds_request_info;
} isp_cds_request_t;

typedef struct isp_hdr_mode {
  boolean use_gtm_fix_curve;
  cam_sensor_hdr_type_t  hdr_mode;
} isp_hdr_mode_t;

/*
 * @ IFACE_SKIP_NONE = no buffer skip,
 * @ IFACE_SKIP_ALL = skip all buffers,
 * @ IFACE_SKIP_RANGE = skip buffer between some frame id min/max,
 * @ IFACE_SKIP_MAX = invalid value,
*/
typedef enum {
  IFACE_SKIP_NONE = 0,
  IFACE_SKIP_ALL,
  IFACE_SKIP_RANGE,
  IFACE_SKIP_MAX,
} iface_skip_mode;

/*
 * @ IFACE_SKIP_STATUS_OK     = Requested frames will be skipped
 * @ IFACE_SKIP_STATUS_LATE   = Too late to skip requested frame range
 * @ IFACE_SKIP_STATUS_RETRY  = IFACE thread is busy, retry next frame
 * @ IFACE_SKIP_STATUS_FAIL   = Error during sw frame skip execution or
 *                              propagation
 * @ IFACE_SKIP_STATUS_MAX,
*/
typedef enum {
  IFACE_SKIP_STATUS_OK    = 0,
  IFACE_SKIP_STATUS_LATE,
  IFACE_SKIP_STATUS_RETRY,
  IFACE_SKIP_STATUS_FAIL,
  IFACE_SKIP_STATUS_MAX,
} iface_skip_status;

/*
 * @session_idi: Session id
 * @skip_stats_frames: Skip all stats type including bf/bf scale etc.
 * @skip_image_frames: Skip all image streams
 * @ack_msg: return msg, if success/retry to sensor
 * @skip_mode: skip pattern, if skip mode is range only then min/max is used
 * @min_frame_id: minimum frame id (valid only if skip_mode = RANGE)
 * @max_frame_id:maximum frame id (valid only if skip_mode = RANGE)
 * min and max frame id will also be skipped.
*/
typedef struct iface_sw_framskip {
  uint32_t session_id;
  boolean skip_stats_frames;
  boolean skip_image_frames;
  iface_skip_status ack_msg;
  iface_skip_mode skip_mode;
  uint32_t min_frame_id;
  uint32_t max_frame_id;
} iface_sw_frameskip;

typedef struct {
  boolean dual_vfe;
  uint32_t image_width;
  uint32_t image_height;
  uint32_t H_global_offset;
  uint32_t V_global_offset;
  uint32_t left_buffer_width;
  uint32_t left_buffer_stride;
  uint32_t right_buffer_width;
  uint32_t right_buffer_stride;
  uint32_t buffer_height;
  uint32_t overlap;
  uint32_t right_image_offset;
} isp_pdaf_camif_info;

/* custom_format_t: Describe sensor custom output
 *
 * enable: set if sensor output is custom format
 * subframes_cnt: number of interleaved frames
 * start_x: start x of active area
 * start_y: start y of active area
 * width: sensor output width (extra data included)
 * height: sensor output height (extra data included)
 */
typedef struct {
  unsigned short enable;
  unsigned int subframes_cnt;
  unsigned int start_x;
  unsigned int start_y;
  unsigned int width;
  unsigned int height;
} custom_format_t;

typedef struct {
  sensor_dim_output_t dim_output;
  sensor_dim_output_t full_res;
  uint32_t            op_pixel_clk;
  uint32_t            output_format;
  custom_format_t     custom_format;
  char *              sensor_name;
  int32_t             res_idx;
  boolean             need_camif_raw;
} sensor_set_dim_t;

typedef struct _sensor_request_crop {
  uint32_t first_pixel;
  uint32_t last_pixel;
  uint32_t first_line;
  uint32_t last_line;
} sensor_request_crop_t;

/**
 *  @need_divert:
 *  @buf_alignment:
 *  @min_stride:
 *  @min_scanline:
 *  @force_streaming_mode:
 *  @num_additional_buffers:
 *  @need_adsp_heap: Flag to indicate whether the consumer needs
 *                 ADSP heap for native buffers. Note that this
 *                 flag is invalid if "need_divert" is FALSE
 */
typedef struct {
  boolean  need_divert;
  uint32_t buf_alignment;
  uint32_t min_stride;
  uint32_t min_scanline;
  uint32_t min_addr_offset_h;
  uint32_t min_addr_offset_v;
  cam_streaming_mode_t force_streaming_mode;
  uint32_t num_additional_buffers;
  boolean  need_adsp_heap;
} pp_buf_divert_request_t;

/** pp_native_buf_info_t:
 *
 *  Description: Per stream buffer information
 *
 *  stram info from downstream module
 *  @stream_type: type of the stream
 *  @need_adsp_heap: Flag to indicate whether the ADSP heap is
 *                 needed for the client
 *  @num_additional_buffers: count of extra buffers needed for
 *                         the client
 */
typedef struct {
  cam_stream_type_t stream_type;
  boolean  need_adsp_heap;
  uint32_t num_additional_buffers;
} pp_native_buf_info_t;

/** pp_meta_stream_info_request_t:
 *
 *  Description: This struct used by modules which allocate
 *    native buffers to query the requirements
 *
 *  @num_streams: Number of streams to be used for the usecase
 *  @buf_info: Buffer details for each stream
 *
 */
typedef struct {
  uint32_t num_streams;
  pp_native_buf_info_t buf_info[MAX_NUM_STREAMS];
} pp_meta_stream_info_request_t;


typedef struct {
  int32_t af_supported;
  float   focal_length;
  float   pix_size;
  float   f_number;
  float   total_f_dist;
  float   hor_view_angle;
  float   ver_view_angle;
  float   min_focus_distance;
} af_lens_info_t;

/** tof_update_t:
 * This is the structure that would be passed from laser sensor to Android
 * sensor Framework.
 *
 *  @frame_id : current frame id, can be optional since it relies on timestamp
 *  @timestamp : timestamp of arrival of the laser data
 *  @distance: calculated object distance in mm
 *  @confidence: confidence of distance measured
 *  @near_limit: near end of distance measured in mm (when
 *             laser is semi confident)
 *  @far_limit: far end of distance measured in mm
 *  @max_distance: maximum distance that can be measured by this
 *               laser
 *
 **/
typedef struct {
  uint32_t frame_id;
  int64_t timestamp;
  int32_t distance;
  int32_t confidence;
  int32_t near_limit;
  int32_t far_limit;
  int32_t max_distance;
} tof_update_t;

typedef struct {
  boolean                is_valid;
  uint32_t               stats_type;
  uint8_t                dt;
  sensor_dim_output_t    dim;
  cam_format_t           fmt;
} sensor_meta_t;

typedef struct {
  uint32_t                  num_meta;
  sensor_meta_t             sensor_meta_info[MAX_META];
} sensor_meta_data_t;

typedef struct {
  boolean enable;         /* if false feature disabled */
  uint32_t full_width;    /* full-resolution width */
  uint32_t full_height;   /* full-resolution height */
  uint32_t output_width;  /* output width */
  uint32_t output_height; /* output height */
  uint32_t offset_x;      /* x-index of the top-left corner of output
                           * image on the full-resolution sensor */
  uint32_t offset_y;      /* y-index of the top-left corner of output image
                           * on the full-resolution sensor */
  uint32_t scale_factor;  /* the sensor scaling factor
                           * (=binning_factor * digal_sampling_factor) */
} sensor_rolloff_config_t;


/** sensor_RDI_parser_func_t
 *
 *   Callback function for parse statistics
 *
 **/
typedef struct {
  int32_t (*parse_VHDR_stats_callback)(uint32_t *, void *);
  int32_t (*parse_PDAF_stats_callback)(uint32_t *, void *);
  int32_t (*parse_flicker_stats_callback)(uint32_t *, void *);
} sensor_RDI_parser_func_t;

/** isp_hw_id_t
 *
 *  ISP hardware id 0/1
 *
 **/
typedef enum {
  ISP_HW_0,
  ISP_HW_1,
  ISP_HW_MAX,
} isp_hw_id_t;

typedef struct {
  uint8_t                cid;
  iface_output_pack_mode pack_mode;
} sensor_pack_mode_entry_t;

typedef struct {
  uint32_t                 num_entries;
  sensor_pack_mode_entry_t pack_mode_info[MAX_PACK_MODE_OVERRIDES];
} sensor_pack_mode_info_t;

typedef struct {
  enum                      camerab_mode_t mode;
  sensor_dim_output_t       dim_output;
  sensor_request_crop_t     request_crop;
  cam_format_t              fmt;
  uint32_t                  offset_x;
  uint32_t                  offset_y;
  uint32_t                  op_pixel_clk;
  uint16_t                  num_frames_skip;
  float                     max_gain;
  uint32_t                  max_linecount;
  uint32_t                  min_linecount;
  uint32_t                  vt_pixel_clk;
  uint32_t                  ll_pck;
  uint32_t                  fl_lines;
  uint16_t                  binning_factor;
  uint16_t                  pixel_sum_factor;
  float                     max_fps;
  float                     orig_max_fps;
  af_lens_info_t            af_lens_info;
  sensor_meta_data_t        meta_cfg;
  boolean                   is_retry;
  uint32_t                  sensor_mount_angle;
  enum                      camb_position_t position;
  sensor_rolloff_config_t   sensor_rolloff_config;
  boolean                   prep_flash_on;
  boolean                   is_dummy;
  unsigned int              sensor_immediate_pipeline_delay;
  unsigned int              sensor_additive_pipeline_delay;
  /* Sensor frame delay to be exposed to other modules in pipeline*/
  uint32_t                  sensor_max_pipeline_frame_delay;
  /* Pointer to parse RDI stats callback function */
  sensor_RDI_parser_func_t  parse_RDI_statistics;
  cam_stream_secure_t       is_secure;
  uint32_t                  is_pdaf_supported;
  uint32_t                  sensor_hdr_enable;
  void                      *pdaf_data;
  float                     actuator_sensitivity;
  uint32_t                  min_vbi;
  uint32_t                  min_hbi;
  int                       full_width;
  int                       full_height;
  int                       filter_arrangement;
  unsigned int              pedestal;
  void*                     custom_cal_data;
  custom_format_t           custom_format;
  boolean                   svhdr_use_separate_gain;
  boolean                   svhdr_use_separate_limits;
  uint32_t                  min_line_cnt[MAX_VIDEO_HDR_FRAMES];
  uint32_t                  max_line_cnt[MAX_VIDEO_HDR_FRAMES];
  boolean                   is_stereo_config;
  sensor_pack_mode_info_t   pack_mode_override;
  uint32_t                  is_depth_sensor;
} sensor_out_info_t;

typedef enum {
  ISP_STRIPE_LEFT,
  ISP_STRIPE_RIGHT,
  ISP_STRIPE_MAX,
} isp_stripe_id_t;

typedef enum {
  IFACE_INTF_PIX,
  IFACE_INTF_RDI0,
  IFACE_INTF_RDI1,
  IFACE_INTF_RDI2,
  IFACE_INTF_MAX
} iface_intf_type_t;

typedef struct {
  boolean             is_split;
  isp_stripe_id_t     stripe_id;
  uint32_t            left_output_width;
  uint32_t            right_output_width;
  uint32_t            right_stripe_offset;
  uint32_t            split_point;
} isp_out_info_t;

typedef struct {
  boolean                   is_split;
  uint32_t                  overlap;
  uint32_t                  right_stripe_offset;
  uint32_t                  split_point;
} ispif_out_info_t;

/* This is the description of what the ISP outputs when
2 ISPs are working co-operatively on a single frame. */
typedef struct {
  uint32_t num_mct_stream_mapped; /* how many mct stream mapped to this stream*/
  uint32_t mapped_mct_stream_id[ISP_NUM_PIX_STREAM_MAX]; /* mct stream id mapped to this hw stream*/
  uint32_t need_divert;
  isp_out_info_t  isp_split_output_info;
  enum msm_vfe_axi_stream_src axi_path;
  cam_format_t fmt;   /* image format */
  cam_dimension_t dim;   /* hw stream width and height*/
  cam_streaming_mode_t streaming_mode;   /* streaming type: continuous/burst */
  boolean use_native_buffer; /* use_native_buffer */
  /* many total frames isp wants AXI to skip */
  uint8_t isp_frame_skip;
  boolean controlable_output; /* use controlable_output */
  boolean shared_output; /* use controlable_output */
} isp_pix_out_info_t;

typedef struct {
  uint32_t session_id;
  enum msm_isp_stats_type stats_type; /*BG/BE/BF...ETC*/
  uint32_t parse_flag;
  uint32_t comp_flag;
  enum msm_isp_buf_type buf_type; /*shared or private*/
  uint32_t buf_len;
  uint32_t num_bufs;
} isp_stats_info_t;

typedef struct {
  uint32_t is_camif_raw_crop_supported;
  uint32_t is_camif_raw_op_fmt_supported;
} isp_camif_caps_info;

typedef struct {
  void *raw_stats_buffer[MSM_ISP_STATS_MAX];
  uint32_t raw_stats_buf_len[MSM_ISP_STATS_MAX];
  uint32_t session_id;
  uint32_t stats_mask;                        /* 4 bytes */
  uint8_t stats_buf_idxs[MSM_ISP_STATS_MAX];  /* 11 bytes */
  uint32_t frame_id;
  isp_hw_id_t  hw_id;
  struct timeval timestamp;
  boolean ack_flag;
  mct_bf_pd_stats_data_t          pd_data;
  struct timeval sof_timestamp;
} iface_raw_stats_buf_info_t;

typedef struct {
  uint32_t                        num_isps;
  /* TODO remove mask and put array */
  uint32_t                        isp_id_mask;
  uint32_t                        num_pix_stream;
  uint32_t                        request_pix_op_clk;
  uint32_t                        max_nominal_clk;
  ispif_out_info_t                ispif_split_output_info;
  /* use another macro : MAX_PIX_INTF = 4 (2 x 2) */
  isp_pix_out_info_t              isp_pix_output[ISP_NUM_PIX_STREAM_MAX];
  uint32_t                        num_stats_stream;
  isp_stats_info_t                isp_stats_info[MSM_ISP_STATS_MAX];
  uint32_t                        cds_capable[ISP_NUM_PIX_STREAM_MAX];
  uint32_t                        hw_version;
  isp_camif_caps_info             camif_cap;
  uint32_t                        multi_pass;
} iface_resource_request_t;

typedef enum {
  ISP_FETCH_PIPELINE_START,
  ISP_FETCH_PIPELINE_REPROCESS,
  ISP_FETCH_PIPELINE_STOP,
  ISP_FETCH_PIPELINE_MAX,
} iface_fetch_engine_cmd_t;

typedef struct {
  uint32_t                 isp_id_mask;
  iface_fetch_engine_cmd_t fetch_cfg_cmd;
  cam_stream_parm_buffer_t *stream_param_buf;
  mct_stream_info_t        input_stream_info;
  cam_dimension_t          input_dim;
  cam_format_t             input_fmt;
  ispif_out_info_t         ispif_split_output_info;
  isp_out_info_t           isp_split_output_info;
} iface_fetch_engine_cfg_t;

typedef struct {
  int luma_target;
  int current_luma;
  float gain;
  float digital_gain;
  uint32_t linecount;
  uint32_t fps;
} sensor_set_aec_data_t;

typedef struct {
  uint8_t       cid;
  uint32_t      csid;
  uint8_t       stereo_right_cid;
  uint32_t      stereo_right_csid;
  uint8_t       is_bayer_sensor;
  uint32_t      csid_version;
  cam_format_t  fmt;
  uint8_t       dt;
  iface_output_pack_mode pack_mode;
} sensor_src_port_cap_entry_t;

typedef struct {
  uint32_t session_id;
  uint16_t num_cid_ch;     /* num CIDs from sensor */
  sensor_src_port_cap_entry_t sensor_cid_ch[SENSOR_CID_CH_MAX];
  uint8_t num_meta_ch;    /* num of meta channels */
  sensor_src_port_cap_entry_t meta_ch[MAX_META];
  uint8_t is_stereo_config;
} sensor_src_port_cap_t;

typedef enum {
  ISP_INPUT_ISPIF,
  ISP_INPUT_FETCH_ENG,
  ISP_INPUT_TEST_GEN
} isp_input_type_t;

typedef struct {
  isp_input_type_t input_type;
  uint8_t use_pix;
  sensor_src_port_cap_t sensor_cap;
} ispif_src_port_caps_t;

typedef struct {
  sensor_out_info_t sensor_cfg;
  uint32_t meta_use_output_mask;
  uint32_t vfe_output_mask; /* hi 16 bits - VFE1, lo 16 bits VFE0 */
  uint32_t vfe_mask;        /* which vfe associated */
  uint32_t session_id;      /* session id */
  uint32_t stream_id;       /* stream id */
} ispif_src_port_stream_cfg_t;

typedef struct {
  int x;
  int y;
}pix_t;

typedef struct {
  pix_t pix[MAX_DEFECTIVE_PIXELS];
  int count;
} defective_pix_array_t;

/* isp_batch_info_t:
  * @batch_mode : Buffer is part of batch mode.
  * @batch_size : how many frames are packed in this single buffer.
  * @intra_plane_offset: tells distance between 2 Y planes or 2 Cb/Cr planes.
  * @pick_preview_idx: To notify CPP, which buffer num out of batch to pick for
  * generating preview frame.
*/
typedef struct {
  boolean batch_mode;
  uint32_t batch_size;
  uint32_t intra_plane_offset[MAX_STREAM_PLANES];
  uint32_t pick_preview_idx;
} isp_batch_info_t;

typedef struct {
  boolean native_buf;            /* TRUE VFE allocated buf */
  void *vaddr;                   /* NULL if not native buf */
  uint32_t plane_vaddr[VIDEO_MAX_PLANES];  /* vaddr per plane */
  int fd;                        /* not used if not native buf */
  struct v4l2_buffer buffer;     /* v4l2 buffer */
  boolean is_locked;             /* this flag is only used for
                                  * downstream module for sharing the
                                  * same struct. ISP does not read it. */
  boolean ack_flag;              /* as discussed with MCT and IMAGE_LIB team,
                                  * ack_flag is added for downstream module
                                  * to piggy back the ack. */
  boolean is_buf_dirty;          /* when ack_flag is TRUE is_buf_dirty
                                  * is checked by ISP to determine
                                  * if buf_done is needed */
  boolean is_uv_subsampled;      /* indicates that the buffer is additionally
                                  * chroma subsampled on top of the subsampling format */
  unsigned int identity;         /* identity 0x0000 0000
                                  * (session/stream index) */
  int channel_id;                /* channel id */
  boolean pass_through;          /* When pass_through is set to 1 some modules
                                    can simply by pass this event */
  void *meta_data;               /* This holds the pointer for meta data
                                    associated with this buffer */
  boolean stats_valid;
  uint32_t stats_type;
  isp_batch_info_t batch_info; /* For batch, num of bufs, plane offsets etc */
  uint8_t lds_enabled;         /*TRUE if LDS was applied, FALSE otherwise */
  uint8_t bayerdata;           /*TRUE if ISP output is bayer_raw for SVHDR*/
  uint32_t buffer_access;      /* Flag to indicate whether CPU
                                * has read the buffer (requires
                                * cache invalidate) or CPU has updated
                                * the contents of buffer (requires
                                * WRITE BACK / flush on cache)
                                */
  struct timeval sof_timestamp;
  uint32_t frame_id;
  uint32_t max_buffers;
} isp_buf_divert_t;

typedef struct {
  uint32_t buf_idx;
  boolean is_buf_dirty;
  uint32_t identity;
  int32_t channel_id;
  uint32_t frame_id;
  struct timeval timestamp;
  void *meta_data;               /* This holds the pointer for meta data
                                    associated with this buffer */
  /*
   * Store cds information if down stream module
   * handles buffer asynchronously
   */
  uint32_t reserved;
  boolean bayerdata;           /*TRUE if ack is for bayer buf from SVHDR*/
  uint32_t buffer_access;      /* Flag to indicate whether CPU
                                * has read the buffer (requires
                                * cache invalidate) or CPU has updated
                                * the contents of buffer (requires
                                * WRITE BACK / flush on cache)
                                */
  uint32_t buf_identity;
} isp_buf_divert_ack_t;

typedef struct {
  uint32_t session_id;
  uint32_t stream_id;
  uint32_t width;
  uint32_t height;
  cam_streaming_mode_t streaming_mode;
  uint32_t col_num;
  uint32_t row_num;
  uint32_t fps;
} isp_dis_config_info_t;

typedef struct {
  enum msm_vfe_frame_skip_pattern skip_pattern;
} isp_stream_skip_pattern_t;

/** mct_face_data_t
 *   @roi: face boundary
 *   @score: score for determining the prominence
 *   @face_id: Detected face unique id
 *
 *   Face data
 **/
typedef struct {
  cam_rect_t roi;
  int8_t score;
  int face_id;
} mct_face_data_t;

/** mct_face_hist_t
 *   @bin: histogram bin
 *   @num_samples: number of samples
 *
 *   Face histogram
 **/
typedef struct {
  uint32_t bin[256];
  uint32_t num_samples;
} mct_face_hist_t;

/** mct_face_info_t
 *   @face_count: number of faces detected
 *   @faces: data for each face detected
 *   @orig_faces: origin face postion before translate
 *   @p_histogram: Pointer to the histogram of detected faces
 *   @frame_id: sequence number of the frame
 *   @region_priority: indicate if region priority is enabled for 3A
 *   Face event information
 *
 *   Notes: If the consumer of the event uses the histogram
 *   asynchronously, deep copy of the data must be taken.
 **/
typedef struct {
  uint8_t face_count;
  mct_face_data_t faces[MAX_ROI];
  mct_face_data_t orig_faces[MAX_ROI];
  mct_face_hist_t *p_histogram[MAX_ROI];
  uint32_t frame_id;
  boolean region_priority;
} mct_face_info_t;

/** pproc_divert_info_t
 *   @num_pass: number of pass
 *   @divert_flags: divert flag bitfield (0:processed,
 *                  1:unprocessed)
 *   @unprocess_identity: identity for unprocess divert
 *   @processed_identity[2]: identity for processed divert
 *   @proc_identity[2]: identities that are processed(# of pass)
 *
 *   pproc divert information
 **/
typedef struct {
  uint32_t      num_passes;
  uint32_t     divert_flags;
  uint32_t     div_unproc_identity;
  uint32_t     div_proc_identity[2];
  uint32_t     proc_identity[2];
} pproc_divert_info_t;

typedef enum {
  PPROC_CFG_UPDATE_SINGLE,
  PPROC_CFG_UPDATE_DUAL,
} pproc_cfg_update_t;

/** pproc_divert_config_t
 *   @name: submodule name
 *   @update_mode: update mode (single/dual)
 *   @divert_info: divert info like #pass, relevant identities
 *
 *   divert config event information
 **/
typedef struct {
  char               *name;
  pproc_cfg_update_t  update_mode;
  pproc_divert_info_t divert_info;
} pproc_divert_config_t;

typedef struct {
  void *meta_data;
  uint32_t frame_id;
} pproc_meta_data_dump_t;

/** mct_isp_table_t
 *   @gamma_table - pointer to gamma table
 *   @gamma_num_entries: number of gamma table entries
 *   @la_enable - field to check if LA is enabled
 *   @luma_table - luma adaptation table
 *   @luma_num_entries - number of entries in LA table
 *
 *   ISP tables
 **/
typedef struct {
  uint16_t *gamma_table;
  uint32_t gamma_num_entries;
  int8_t la_enable;
  uint8_t *luma_table;
  uint32_t luma_num_entries;
} mct_isp_table_t;

/** meta_data_buf_divert_request_t
 *   @mata_idx_mask - bit mask of meta channel index mask
 *
 **/
typedef struct {
  uint32_t meta_idx_mask;
} meta_channel_buf_divert_request_t;

typedef struct {
  uint8_t enable;
  uint16_t num_frames_in_fast_aec;
} mct_fast_aec_mode_t;

typedef enum {
  AF_CFG_SWAF,
  AF_CFG_DCIAF,
  AF_CFG_FOCUS,
} af_cfg_type_t;

/** mct_imglib_af_config_t
 *   @enable: Flag to enable/disable stats collection
 *   @frame_id: current frame id
 *   @roi: ROI of the stats collection
 *   @coeffa: filterA coefficients
 *   @coeffb: filterB coefficients
 *   @coeff_len: length of coefficient table
 *   @sw_filter_type: SW filter type
 *
 *   Imglib preview assisted AF coefficients
 **/
typedef struct {
  int8_t enable;
  int frame_id;
  cam_rect_t roi;
  double coeffa[6];
  double coeffb[6];
  int coeff_fir[11];
  uint32_t coeff_len;
  af_sw_filter_type filter_type;
  double FV_min;
} mct_imglib_swaf_config_t;

/** mct_imglib_dciaf_config_t
 *
 *   @roi:       ROI of the stats collection
 *   @is_af_search_active: Inform DCRF library if search is active
 *   @requireDistanceInfo: Inform DCRF library to start providing Distance
 *   @focal_length_ratio: Zoom ratio from DCAF algorithm
 *   Imglib Dual cam AF coefficients
 **/
typedef struct {
  cam_rect_t roi;
  boolean is_af_search_active;
  boolean requireDistanceInfo;
  float focal_length_ratio;
} mct_imglib_dciaf_config_t;

/** mct_imglib_af_focus_config_t
 *
 *   @lens_shift_um:          Lens position - actual displacement [um]. 0 when focusing at infinity, maximal when focusing on closest macro distance.
 *   @object_distance_cm: The estimated distance to the in-focus object [cm].
 *   @focal_length_ratio:    Zoom ratio from Algorithm
 *   @near_field_cm:         The nearest distance in the scene that appears acceptably sharp [cm].
 *   @far_field_cm:            The farthest distance in the scene that appears acceptably sharp [cm].
 *   @roi:                          ROI of the stats collection
 *   AF Focus Configuration paramaters required for Spatial Transform
 **/
typedef struct {
  float lens_shift_um;
  int object_distance_cm;
  float focal_length_ratio;
  int near_field_cm;
  int far_field_cm;
  boolean af_supported;
  cam_rect_t roi;
} mct_imglib_af_focus_config_t;

/** mct_imglib_swaf_config_t
 *   @type: Type: DCIAF or SWAF or AF focus config
 *
 **/
typedef struct {
  af_cfg_type_t type;
  union {
    mct_imglib_swaf_config_t swaf;
    mct_imglib_dciaf_config_t dciaf;
    mct_imglib_af_focus_config_t affocus;
  } u;
} mct_imglib_af_config_t;

/** mct_imglib_dcrf_result_t
 *   @id: Frame ID
 *   @timestamp: Time stamp
 *   @distance_in_mm: Distance of object in ROI's in mili meters
 *   @confidence: Confidence on distance from 0(No confidence)
 *              to 1024(Max confidence)
 *   @status: Status of DCRF library execution call
 *   @focused_roi: ROI's for which distance is estimated
 *   @focused_x: Focus location X inside ROI with distance
 *             estimation
 *   @focused_y: Focus location Y inside ROI with distance
 *             estimation
 **/
typedef struct
{
  uint32_t id;
  uint64_t timestamp;
  uint32_t distance_in_mm;
  uint32_t confidence;
  uint32_t status;
  cam_rect_t focused_roi;
  uint32_t focused_x;
  uint32_t focused_y;
} mct_imglib_dcrf_result_t;

/** mct_imglib_af_output_t
 *   @frame_id: Frame ID out the AF output
 *   @fV: Focus value
 *
 *   Imglib preview assisted AF output
 **/
typedef struct {
  int frame_id;
  double fV;
} mct_imglib_af_output_t;

/** mct_bracketing_state_t
 *   @MCT_BRACKETING_STATE_OFF: bracketing is Off
 *   @MCT_BRACKETING_STATE_ON : bracketing is On
 *
 **/
typedef enum {
  MCT_BRACKETING_STATE_OFF,
  MCT_BRACKETING_STATE_ON,
  MCT_BRACKETING_STATE_INIT,
  MCT_BRACKETING_STATE_DEINIT,
} mct_bracketing_state_t;

/** mct_bracketing_update_t
 *   @state: bracketing state
 *   @index: current bracketing index
 *
 *   Status update information during bracketing
 *
 **/
typedef struct {
  mct_bracketing_state_t state;
  uint32_t index;
} mct_bracketing_update_t;

/** isp_preferred_streams_desc
 *   @stream_mask: mandatory streams for this output
 *   @max_streams_num: Maximum number of streams for this output. It might be
 *   more that actual number of streams in stream_mask. This can be used to
 *   map two particular streams on separate ports without any preference for
 *   others.
 *
 * Describe one preferred output.
 *
 **/
/* TODO: max_streams_num is still not implemented in ISP. Because of that
 * max_streams_num should be number of streams in stream_mask so far. */
typedef struct {
  uint32_t stream_mask;
  uint32_t max_streams_num;
} isp_preferred_streams_desc;

/** isp_preferred_streams
 *   @stream_num: number of preferred outputs
 *   @streams: description of one output
 *
 * Describe all preferred output.
 *
 **/
typedef struct {
  uint32_t stream_num;
  isp_preferred_streams_desc streams[ISP_NUM_PIX_STREAM_MAX];
} isp_preferred_streams;

typedef struct {
  cam_stream_type_t stream_type;
  cam_dimension_t dim;   /* hw stream width and height*/
  boolean use_native_buffer; /* use_native_buffer */
  cam_stream_buf_plane_info_t buf_planes;
  enum msm_vfe_axi_stream_src axi_stream_src;
  cam_format_t fmt;
  boolean  need_adsp_heap;
  uint32_t num_additional_buffers;
} iface_stream_info_t;

typedef struct {
  uint32_t              num_pix_stream;
  iface_stream_info_t   stream_info[ISP_NUM_PIX_STREAM_MAX];
  cam_dimension_t       hw_stream_super_dim[VFE_AXI_SRC_MAX];
} iface_buf_alloc_t;

/** sensor_isp_stream_sizes_t
 *   @stream_sizes: Stream sizes for all configured streams
 *   @num_streams: Num of streams configured
 *   @type: stream type
 *   @postprocess_mask: pp mask per stream
 *   @margins: margin req per stream in %
 *   stream_sz_plus_margin: stream_sizes + margin
 *   @streams: description of one output
 *   @width_alignment: Buffer alignment along with
 *   @height_alignment: Buffer alignment along height
 *
 *  Structre that holds the ISP output size with margins
 *
 **/
typedef struct {
  cam_dimension_t stream_sizes[MAX_NUM_STREAMS];
  uint32_t num_streams;
  cam_stream_type_t type[MAX_NUM_STREAMS];
  cam_is_type_t is_type[MAX_NUM_STREAMS];
  cam_format_t stream_format[MAX_NUM_STREAMS];
  cam_feature_mask_t postprocess_mask[MAX_NUM_STREAMS];
  cam_frame_margins_t margins[MAX_NUM_STREAMS];
  cam_dimension_t stream_sz_plus_margin[MAX_NUM_STREAMS]; /* stream sizes + margin */
  cam_pad_format_t width_alignment;
  cam_pad_format_t height_alignment;
  uint32_t num_additional_buffers[MAX_NUM_STREAMS];
} sensor_isp_stream_sizes_t;

#endif /* __MODULES_H__ */
