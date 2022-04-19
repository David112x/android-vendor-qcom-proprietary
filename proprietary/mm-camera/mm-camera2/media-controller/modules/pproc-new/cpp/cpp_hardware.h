/*============================================================================

  Copyright (c) 2013-2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#ifndef CPP_HARDWARE_H
#define CPP_HARDWARE_H

#include <pthread.h>
#include <media/msmb_camera.h>
#include <media/msmb_pproc.h>
#include "cam_types.h"
#include "camera_dbg.h"
#include "mtype.h"
#include "modules.h"
#include "chromatix.h"
#include "chromatix_common.h"
#include "cpp_hw_params.h"
#include "chromatix_metadata.h"

#define MAX_CPP_DEVICES       2
#define SUBDEV_NAME_SIZE_MAX  32

#define CPP_MAX_FW_VERSIONS   8
#define CPP_MAX_FW_NAME_LEN   32

#define CPP_CAPS_DENOISE      (1 << 0)
#define CPP_CAPS_SCALE        (1 << 1)
#define CPP_CAPS_SHARPENING   (1 << 2)
#define CPP_CAPS_CROP         (1 << 3)
#define CPP_CAPS_ROTATION     (1 << 4)
#define CPP_CAPS_FLIP         (1 << 5)
#define CPP_CAPS_COLOR_CONV   (1 << 6)

#define ROTATION_90       (1 << 0)
#define ROTATION_180      (2 << 1)
#define ROTATION_270      (3 << 2)

#define CPP_TOTAL_SHARPNESS_LEVELS  6
#define CPP_MAX_SHARPNESS           36
#define CPP_MIN_SHARPNESS           0
#define CPP_DEFAULT_SHARPNESS       12

/* maximum no. of unique streams supported */
#define CPP_HARDWARE_MAX_STREAMS      16
/* how many buffers hardware can hold at a time */
#define CPP_HARDWARE_MAX_PENDING_BUF  2
#define CPP_HARDWARE_DEFAULT_PENDING_BUF  1

#define CPP_HARDWARE_NO_PENDING_BUF  0

#define CPP_FW_FILENAME_MAX           64

#define CPP_ERR_PROCESS_FRAME_ERROR  1000

/* List of supported hardware versions:
  31:28  Major version
  27:16  Minor version
  15:0   Revision  */
typedef enum _cpp_hardware_version_t {
  CPP_HW_VERSION_1_0_0 = 0x10000000,
  CPP_HW_VERSION_1_1_0 = 0x10010000, /* MSM8974 Aragorn V2 */
  CPP_HW_VERSION_1_1_1 = 0x10010001, /* MSM8974 Aragorn Pro */
  CPP_HW_VERSION_2_0_0 = 0x20000000, /* MSM8x26 Frodo */
  CPP_HW_VERSION_4_0_0 = 0x40000000, /* MSM8916 Bagheera */
  CPP_HW_VERSION_4_1_0 = 0x40010000, /* APQ8084 Gandalf */
  CPP_HW_VERSION_4_2_0 = 0x40020000, /* MSM8939 Shere */
  CPP_HW_VERSION_4_3_0 = 0x40030000, /* MSM8952 Sahi */
  CPP_HW_VERSION_4_3_2 = 0x40030002, /* MSM8937 Feero */
  CPP_HW_VERSION_4_3_3 = 0x40030003, /* MSM8953 Jacala */
  CPP_HW_VERSION_4_3_4 = 0x40030004, /* MSM8917 FeeroLite */
  CPP_HW_VERSION_5_0_0 = 0x50000000, /* MSM8994 Elessar */
  CPP_HW_VERSION_5_1_0 = 0x50010000,
  CPP_HW_VERSION_6_0_0 = 0x60000000, /* MSM8996 Istari */
  CPP_HW_VERSION_6_1_0 = 0x60010000, /* MSM8998 Nazgul */
  CPP_HW_VERSION_6_1_2 = 0x60010002, /* SDM660/SDM630 SL/SH */
  CPP_HW_VERSION_6_1_4 = 0x60010004, /* TRINKET */
} cpp_hardware_version_t;

#define CPP_HARDWARE_GET_MAJOR_VERSION(ver) ((ver >> 28) & 0xf)
#define CPP_HARDWARE_GET_MINOR_VERSION(ver) ((ver >> 16) & 0xfff)
#define CPP_HARDWARE_GET_REVISION(ver)      (ver & 0xffff)

#define IS_HW_VERSION_SUPPORTS_ROTATION(version) \
  (version != CPP_HW_VERSION_6_1_0 && \
  version != CPP_HW_VERSION_6_1_2 && \
  version != CPP_HW_VERSION_6_1_4) ? 1 : 0 \

#define IS_HW_VERSION_SUPPORTS_DSDN(version) \
  (version == CPP_HW_VERSION_6_0_0 || \
  version == CPP_HW_VERSION_6_1_0 || \
  version == CPP_HW_VERSION_6_1_2 || \
  version == CPP_HW_VERSION_6_1_4) ? 1 : 0 \

#define IS_HW_VERSION_SUPPORTS_DYN_FEAT(version) \
  (version == CPP_HW_VERSION_6_0_0 || \
  version == CPP_HW_VERSION_6_1_0 || \
  version == CPP_HW_VERSION_6_1_2 || \
  version == CPP_HW_VERSION_6_1_4) ? 1 : 0 \

typedef uint32_t cpp_rotation_caps_t;

typedef struct _cpp_scaling_caps_t{
  float min_scale_factor;
  float max_scale_factor;
} cpp_scaling_caps_t;

typedef struct _cpp_color_conv_caps_t{
  cam_format_t src_fmt;
  cam_format_t dest_fmt;
} cpp_color_conv_caps_t;

typedef struct _cpp_flip_caps_t{
  uint8_t h_flip;
  uint8_t v_flip;
} cpp_flip_caps_t;

typedef struct _cpp_sharpness_caps_t{
  int32_t min_value;
  int32_t max_value;
  int32_t def_value;
  int32_t step;
} cpp_sharpness_caps_t;

typedef enum {
  CPP_HW_STATUS_INVALID,
  CPP_HW_STATUS_FW_LOADED,
  CPP_HW_STATUS_READY,
  CPP_HW_STATUS_BUSY,
  CPP_HW_STATUS_IDLE,
} cpp_hardware_status_t;

typedef struct {
  uint32_t caps_mask;
  cpp_scaling_caps_t    scaling_caps;
  cpp_rotation_caps_t   rotation_caps;
  cpp_color_conv_caps_t color_conv_caps;
  cpp_flip_caps_t       filp_caps;
  cpp_sharpness_caps_t  sharpness_caps;
} cpp_hardware_caps_t;

/* HW can send only one type of v4l2 event and actually type
   of event can be abstracted in event data */
typedef struct _cpp_hardware_event_subscribe_info {
  boolean valid;
  uint32_t id;
  uint32_t type;
} cpp_hardware_event_subscribe_info_t;

typedef struct _cpp_hardware_info_t {
  uint32_t version;
  uint32_t caps;
  unsigned long freq_tbl[MAX_FREQ_TBL];
  uint32_t freq_tbl_count;
} cpp_hardware_info_t;

typedef struct _cpp_hardware_stream_status_t {
  boolean   valid;
  int32_t   pending_buf;
  int32_t   pending_buf_divert;
  boolean   stream_off_pending;
  boolean   hold_stream_off_op;
  int32_t   pending_divert;
  int32_t   pending_partial_frame;
  uint32_t  identity;
} cpp_hardware_stream_status_t;

struct _cpp_hardware_t {
  uint32_t                            subdev_ids[MAX_CPP_DEVICES];
  int                                 num_subdev;
  int                                 subdev_fd;
  boolean                             subdev_opened;
  uint32_t                            inst_id;
  cpp_hardware_caps_t                 caps;
  cpp_hardware_info_t                 hwinfo;
  cpp_hardware_status_t               status;
  cpp_firmware_version_t              fw_version;
  cpp_hardware_event_subscribe_info_t event_subs_info;
  cpp_hardware_stream_status_t        stream_status[CPP_HARDWARE_MAX_STREAMS];
  pthread_cond_t                      subdev_cond;
  pthread_mutex_t                     mutex;
  int                                 num_iommu_cnt;
  int                                 max_pending_buffer;
  uint32_t                            dump_preview_cnt;
  uint32_t                            dump_video_cnt;
  uint32_t                            dump_snapshot_cnt;
  int32_t                             max_supported_padding;
  void                                *private_data;
  uint32_t                            preview_frame_counter;
  uint32_t                            video_frame_counter;
  uint32_t                            offline_frame_counter;
  uint32_t                            snapshot_frame_counter;
  uint32_t                            postview_frame_counter;
  uint32_t                            analysis_frame_counter;
} ;


typedef enum {
  CPP_HW_CMD_GET_CAPABILITIES,
  CPP_HW_CMD_SUBSCRIBE_EVENT,
  CPP_HW_CMD_UNSUBSCRIBE_EVENT,
  CPP_HW_CMD_NOTIFY_EVENT,
  CPP_HW_CMD_STREAMON,
  CPP_HW_CMD_STREAMOFF,
  CPP_HW_CMD_LOAD_FIRMWARE,
  CPP_HW_CMD_PROCESS_FRAME,
  CPP_HW_CMD_QUEUE_BUF,
  CPP_HW_CMD_GET_CUR_DIAG,
  CPP_HW_CMD_SET_CLK,
  CPP_HW_CMD_POP_STREAM_BUFFER,
  CPP_HW_CMD_BUF_UPDATE,
  CPP_HW_CMD_PROCESS_PARTIAL_FRAME,
  CPP_HW_CMD_NOTIFY_BUF_DONE,
  CPP_HW_CMD_UPDATE_PENDING_BUF,
} cpp_hardware_cmd_type_t;

typedef struct _cpp_hardware_event_data_t {
  uint8_t        is_buf_dirty;
  uint8_t        do_ack;
  uint32_t       frame_id;
  uint32_t       identity;
  uint32_t       buf_idx;
  uint32_t       out_buf_idx;
  unsigned long  out_fd;
  uint8_t        out_native_buff;
  uint32_t       dup_out_buf_idx;
  unsigned long  dup_out_fd;
  uint32_t       dup_identity;
  uint8_t        dup_out_native_buff;
  struct timeval timestamp;
  void          *cookie;
  uint32_t      reserved;
  uint32_t       batch_size;
  enum msm_cpp_batch_mode_t batch_mode;
  uint32_t       input_buffer_access;
  uint32_t       output_buffer_access;
  /* Store cds information (color format NV 12/21/14/41) */
} cpp_hardware_event_data_t;

typedef struct _cpp_hardware_streamoff_event {
  cam_stream_secure_t              is_secure;
  bool                             is_hw_err;
  uint32_t                         streamoff_identity;
  uint32_t                         linked_identity[CPP_HARDWARE_MAX_STREAMS];
  uint32_t                         num_linked_streams;
} cpp_hardware_streamoff_event_t;

typedef struct _cpp_hardware_buff_update_t {
  boolean                          delete_buf;
  cam_stream_secure_t              is_secure;
  cam_stream_secure_mode_t         secure_mode;
  cpp_hardware_stream_buff_info_t *stream_buff_list;
} cpp_hardware_buff_update_t;

typedef struct _cpp_hardware_clock_settings_t {
  long                             clock_rate;
  uint64_t                         avg;
  uint64_t                         inst;
} cpp_hardware_clock_settings_t;

typedef struct _cpp_hardware_partial_stripe_info_t {
  uint8_t  last_payload;
  uint8_t  first_payload;
  uint32_t first_stripe_index;
  uint32_t last_stripe_index;
  uint32_t stripe_info;
} cpp_hardware_partial_stripe_info_t;

typedef struct _cpp_hardware_cmd_t {
  cpp_hardware_cmd_type_t type;
  void                   *ctrl;
  void                   *return_payload;
  /* CPP_HW_CMD_PROCESS_FRAME - Partial frame, after the first payload is sent,
     the remainder is saved here and then split into more partial_frames
     in the thread */
  union {
    cpp_hardware_streamoff_event_t streamoff_data;
    cpp_hardware_event_data_t       *event_data;
    cpp_hardware_params_t           *hw_params;
    cpp_hardware_buff_update_t      *buff_update;
    struct msm_cpp_frame_info_t     *partial_frame;
    cpp_hardware_clock_settings_t   clock_settings;
    uint32_t                        buf_done_identity;
    uint32_t                        status;
  } u;
} cpp_hardware_cmd_t;

/* interface functions */
cpp_hardware_t* cpp_hardware_create();
int32_t cpp_hardware_open_subdev(cpp_hardware_t *cpphw);
int32_t cpp_hardware_close_subdev(cpp_hardware_t *cpphw);
int32_t cpp_hardware_destroy(cpp_hardware_t *cpphw);
int32_t cpp_hardware_process_command(cpp_hardware_t *cpphw,
  cpp_hardware_cmd_t cmd);
cpp_hardware_status_t cpp_hardware_get_status(cpp_hardware_t *cpphw);

/* static functions */
static int32_t cpp_hardware_find_subdev(cpp_hardware_t *cpphw);
static int32_t cpp_hardware_subcribe_v4l2_event(cpp_hardware_t *cpphw);
static int32_t cpp_hardware_notify_event_get_data(cpp_hardware_t *cpphw,
  cpp_hardware_event_data_t *event_data);
static int32_t cpp_hardware_get_capabilities(cpp_hardware_t *cpphw);
static int32_t cpp_hardware_load_firmware(cpp_hardware_t *cpphw);
static int32_t cpp_hardware_process_streamon(cpp_hardware_t *cpphw,
  cpp_hardware_buff_update_t *buff_update);
static int32_t cpp_hardware_process_streamoff(cpp_hardware_t *cpphw,
  cpp_hardware_streamoff_event_t streamoff_data);
static int32_t cpp_hardware_process_frame(cpp_hardware_t *cpphw,
  cpp_hardware_cmd_t *cmd);
cpp_hardware_stream_status_t*
  cpp_hardware_get_stream_status(cpp_hardware_t* cpphw, uint32_t identity);
static int32_t cpp_hardware_subcribe_v4l2_event(cpp_hardware_t *cpphw);
static int32_t cpp_hardware_unsubcribe_v4l2_event(cpp_hardware_t *cpphw);
double cpp_get_sharpness_ratio(int32_t sharpness);
static int32_t cpp_hardware_pop_stream_buffer(cpp_hardware_t *cpphw,
  cpp_hardware_event_data_t *event_data);
cpp_firmware_version_t cpp_hardware_get_fw_version(cpp_hardware_t *cpphw);
int32_t cpp_hardware_update_buffer_list(cpp_hardware_t *cpphw,
  cpp_hardware_buff_update_t *buff_update);
uint32_t cpp_hardware_calculate_stripe_info(uint32_t stripe_count,
  int32_t plane_index);
void cpp_hardware_update_partial_stripe_info(uint32_t *stripe_count,
  cpp_hardware_partial_stripe_info_t *partial_frame, uint32_t *total_stripes);
int32_t cpp_hardware_notify_buf_done(cpp_hardware_t *cpphw, uint32_t identity);
int32_t cpp_hardware_flush_frame(cpp_hardware_t *cpphw,
  struct msm_cpp_frame_info_t *frame);
void cpp_hardware_set_private_data(cpp_hardware_t *cpphw, void *data);
int32_t cpp_hardware_update_pending_buffer(cpp_hardware_t *cpphw, int32_t value);
#endif
