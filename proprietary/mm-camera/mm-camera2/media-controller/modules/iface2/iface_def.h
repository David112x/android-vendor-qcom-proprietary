/*============================================================================
Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __ISP_DEF_H__
#define __ISP_DEF_H__

#include <media/msmb_isp.h>
#include <media/msmb_ispif.h>
#include "modules.h"
#if TARGET_ION_ABI_VERSION >= 2
#include <ion/ion.h>
#endif
#define CDBG_DUMP(fmt, args...) do{}while(0)

#define SEM_WAIT_TIME(sem_ptr, timeout_ptr, ns, rc) \
  { \
    clock_gettime(CLOCK_REALTIME, (timeout_ptr)); \
    (timeout_ptr)->tv_nsec += ns;\
    if ((timeout_ptr)->tv_nsec > 999999999) { \
      (timeout_ptr)->tv_nsec -= 999999999; \
      (timeout_ptr)->tv_sec += 1;  \
    } \
    rc = sem_timedwait((sem_ptr), (timeout_ptr));\
  }

#define Q14   0x00004000
#define Q13   0x00002000
#define Q12   0x00001000
#define Q12_2 0x00002000
#define Q12_4 0x00004000
#define Q11   0x00000800
#define Q10   0x00000400
#define Q8    0x00000100
#define Q7    0x00000080
#define Q6    0x00000040
#define Q4    0x00000010

#define VFE_MAX 2
#define IFACE_MAX_NUM                 2
#define IFACE_MAX_SESSIONS            4
#define IFACE_MAX_STREAMS             ISP_NUM_PIX_STREAM_MAX
#define IFACE_SINK_PORT_CFG_ENTRY_MAX 2
#define IFACE_SUBDEV_NAME_LEN         32
#define IFACE_ISP_SD_NODE_ID_MAX_NUM      VFE_MAX
#define IFACE_METADATA_GAMMATBL_SIZE  64
#define IFACE_MAX_NUM_SCALER          2
#define IFACE_LOW_LIGHT_LUX_IDX       300

#define IFACE_MAX_IMG_BUF        CAM_MAX_NUM_BUFS_PER_STREAM

#define IFACE_PIX_MAX_MASK  ((1 << IFACE_INTF_PIX) | (1 << (16 + IFACE_INTF_PIX)))
#define IFACE_RDI0_MAX_MASK ((1 << IFACE_INTF_RDI0) | (1 << (16 + IFACE_INTF_RDI0)))
#define IFACE_RDI1_MAX_MASK ((1 << IFACE_INTF_RDI1) | (1 << (16 + IFACE_INTF_RDI1)))
#define IFACE_RDI2_MAX_MASK ((1 << IFACE_INTF_RDI2) | (1 << (16 + IFACE_INTF_RDI2)))

#define MIN(x,y) (((x)<(y)) ? (x) : (y))
#define MAX(x,y) (((x)>(y)) ? (x) : (y))
#define OFFSET_TO_ADDRESS(x, y, width) ((x) + ((y) * (width)))

#define AXI_ADDR_ALIGHN_BYTE CAM_PAD_TO_32
#define AXI_OUT_ALIGHN_BYTE CAM_PAD_TO_32
#define AXI_BUF_ALIGHN_BYTE CAM_PAD_TO_32
#define IFACE_MAX_NUM_PLANE 3

#define ISP_MSM8960V1    0x3030b
#define ISP_MSM8960V2    0x30408
#define ISP_MSM8930      0x3040f
#define ISP_MSM8610      0x3050A
#define ISP_MSM8974_V1   0x10000018
#define ISP_MSM8974_V2   0x1001001A
#define ISP_MSM8974_V3   0x1001001B
#define ISP_MSM8916_V1   0x10030000
#define ISP_MSM8939      0x10040000
#define ISP_MSM8956      0x10050000
#define ISP_MSM8952      0x10060000
#define ISP_MSM8937      0x10080000
#define ISP_MSM8917      0x10080001
#define ISP_MSM8953      0x10090000
#define ISP_MSM8226_V1   0x20000013
#define ISP_MSM8226_V2   0x20010014
#define ISP_MSM8084_V1   0x4000000A
#define ISP_MSM8992_V1   0x4002000A
#define ISP_MSM8994_V1   0x60000000
#define ISP_MSM8996_V1   0x70000000
#define ISP_MSM8998      0x80000000
#define ISP_SDM660       0x80000003


typedef struct {
  uint32_t num_isps;
  uint32_t isp_id_mask;
  uint32_t offline_isp_id_mask;
  uint32_t used_resource_mask;
  uint32_t used_wm[VFE_MAX];
  uint8_t camif_cnt;
  uint8_t rdi_cnt;
  ispif_out_info_t ispif_split_info;
  ispif_out_info_t offline_split_info;
  enum msm_vfe_input_src main_frame_intf; //intf that the sensor frame is mapped on
  enum msm_ispif_vfe_intf main_frame_vfe;
} iface_resource_t;

typedef struct {
  uint32_t isp_id;
  int num_pix; /* num pix interfaces supported */
  int num_rdi; /* num rdi interfaces supported */
  int num_wms; /* num  AXI write masters */
  int num_register;
  uint32_t stats_mask;
  uint32_t max_resolution;
  uint32_t max_pix_clk;
} iface_isp_hw_cap_t;

typedef struct {
  struct msm_isp_event_data *isp_event_data;
  boolean ack_flag;
  boolean is_buf_dirty;
  boolean use_native_buf;
} iface_frame_divert_notify_t;

typedef struct {
  uint32_t session_id;
  uint32_t frame_id;
} iface_halt_recovery_info_t;

typedef struct {
  uint32_t session_id;
  uint32_t stats_stream_id;
  isp_stripe_id_t isp_stripe_id;
  enum msm_isp_stats_type stats_type;
  enum msm_isp_buf_type buf_type;
  uint32_t comp_flag;
  uint32_t buf_len;
  uint32_t num_bufs;
  uint32_t use_native_buf;
  enum msm_vfe_frame_skip_pattern pattern;
  uint32_t init_frame_drop;
} iface_stats_stream_t;

typedef struct {
  uint8_t num_planes;
  uint32_t strides[MAX_STREAM_PLANES];
  uint32_t scanline[MAX_STREAM_PLANES];
  uint32_t addr_offset[MAX_STREAM_PLANES];
  uint32_t addr_offset_h[MAX_STREAM_PLANES];
  uint32_t addr_offset_v[MAX_STREAM_PLANES];
} iface_plane_info_t;

typedef enum {
  IFACE_HW_STREAM_STATE_INITIAL,  /* initial */
  IFACE_HW_STREAM_STATE_RESERVED,
  IFACE_HW_STREAM_STATE_HW_CFG_DONE, /* hw configured, such as pix/rdi are decided */
  IFACE_HW_STREAM_STATE_ACTIVE,   /* got start ack already */
  IFACE_HW_STREAM_STATE_STOPPING, /* stopping */
  IFACE_HW_STREAM_STATE_MAX,
} iface_hw_stream_state_t;

typedef enum {
  IFACE_STREAM_TYPE_IMAGE,
  IFACE_STREAM_TYPE_META_DATA,
  IFACE_STREAM_TYPE_STATS,
  IFACE_STREAM_TYPE_MAX
} iface_stream_type_t;

typedef enum {
  IFACE_ENABLE_ERR_LOGS,
  IFACE_ENABLE_HIGH_LOGS,
  IFACE_ENABLE_DEBUG_LOGS,
  IFACE_LOGS_MAX
}iface_loglevel_t;

volatile iface_loglevel_t g_ifaceloglevel;

typedef struct {
  cam_format_t fmt;   /* image format */
  cam_dimension_t dim;   /* hw stream width and height*/
  cam_stream_buf_plane_info_t buf_planes; /* buffer plane information */
  cam_streaming_mode_t streaming_mode;   /* streaming type: continuous/burst */
  cam_stream_type_t    cam_stream_type;
  int num_burst;   /* burst number of snapshot */
  boolean           batch_mode;
  int               batch_size;
} hw_stream_info_t;

typedef struct {
  mct_list_t       *img_buffer_list;  /* DS mapped buffer information */
  int               num_bufs;
  uint32_t          user_stream_id;
  uint32_t          bufq_handle;
  uint8_t           total_num_bufs;
  uint8_t           current_num_bufs;
  boolean           is_mapped;
} hw_stream_buffer_info_t;
/**
 @num_mct_stream_mapped: how many mct stream mapped to this stream
 @mapped_mct_stream_id: mct stream id mapped to this hw stream
 @associtate_hw_stream_ids: preview/video associate to which streams
 @native_buf_use_adspheap: Flag to indicate whether adsp heap
                         needs to be used for native buffers
*/

typedef struct {
  uint32_t                        session_id;
  uint32_t                        hw_stream_id;
  uint32_t                        mct_streamon_id;
  hw_stream_info_t                stream_info;
  uint32_t                        num_mct_stream_mapped;
  uint32_t                        mapped_mct_stream_id[IFACE_MAX_STREAMS];
  uint8_t                         use_pix;
  uint8_t                         use_native_buf;
  enum msm_vfe_axi_stream_src     axi_path;
  uint32_t                        request_op_pix_clk;
  iface_plane_info_t              plane_info;
  uint8_t                         sensor_skip_cnt;
  uint32_t                        frame_base;
  uint32_t                        last_frame_id;
  struct timeval                  last_timestamp;
  uint32_t                        need_divert;
  uint32_t                        axi_addr_offset_h;
  uint32_t                        axi_addr_offset_v;
  uint32_t                        native_buf_alignment;
  uint32_t                        native_buf_min_stride;
  uint32_t                        native_buf_min_scanline;
  enum msm_vfe_frame_skip_pattern frame_skip_pattern;
  uint32_t                        interface_mask;
  uint32_t                        num_cids;
  enum msm_ispif_cid              cids[SENSOR_CID_CH_MAX];
  enum msm_ispif_csid             csid;
  enum msm_ispif_cid              right_cids[SENSOR_CID_CH_MAX];
  enum msm_ispif_csid             right_csid;
  iface_output_pack_mode          pack_mode[CID_MAX];
  isp_out_info_t                  isp_split_output_info;
  uint8_t                         need_uv_subsample;
  iface_stream_type_t             stream_type;
  uint32_t                        associtate_hw_stream_ids[IFACE_MAX_STREAMS];
  uint32_t                        streamon_count;
  iface_hw_stream_state_t         state;
  uint32_t                        divert_to_3a;
  uint32_t                        ext_stats_type;
  uint32_t                        controllable_output;
  cam_streaming_mode_t            streaming_mode;
  int32_t                         num_burst;
  uint32_t                        vt_enable;
  uint32_t                        shared_hw_stream;
  hw_stream_buffer_info_t         buffers_info[VFE_BUF_QUEUE_MAX];
  uint32_t                        num_additional_native_buffers;
  uint16_t                        left_pixel_hor_skip;
  uint16_t                        left_pixel_ver_skip;
  uint16_t                        right_pixel_hor_skip;
  uint16_t                        right_pixel_ver_skip;
  uint16_t                        native_buf_use_adspheap;
  enum msm_vfe_camif_output_format camif_output_format;
  int8_t                          remaining_pass;
  boolean                         is_stereo;
} iface_hw_stream_t;


typedef struct {
  iface_hw_stream_t  hw_streams[IFACE_MAX_STREAMS];
  uint32_t  num_hw_streams;
} iface_hw_stream_set_params_t;

typedef struct {
  iface_stats_stream_t stats_stream[MSM_ISP_STATS_MAX];
  uint32_t num_stats_streams;
} iface_stats_stream_set_params_t;


typedef struct {
  uint32_t session_id;
  uint32_t hw_stream_id;
  enum msm_vfe_frame_skip_pattern pattern;
  boolean use_sw_skip;
  iface_sw_frameskip sw_frame_skip_info;
  mct_bracketing_state_t bracketing_state;
} iface_param_frame_skip_pattern_t;

typedef struct {
  uint32_t session_id;
  uint32_t hw_stream_id;
  uint32_t user_stream_id;
  uint32_t frame_id;
  uint32_t buf_index;
} iface_param_frame_request_t;

typedef struct {
  uint32_t session_id;
  uint32_t hw_stream_id;
  uint32_t user_stream_id;
} iface_param_add_queue_t;



typedef struct {
  void *vaddr;
  int fd;
  struct v4l2_buffer buffer;
  struct v4l2_plane planes[VIDEO_MAX_PLANES];
  unsigned long addr[VIDEO_MAX_PLANES];
  struct ion_allocation_data ion_alloc[VIDEO_MAX_PLANES];
  struct ion_fd_data fd_data[VIDEO_MAX_PLANES];
  int cached;
  boolean is_reg; /*flag to indicate buffer is already registered with kernel*/
} iface_frame_buffer_t;

#ifdef _ANDROID_
  #undef LOG_NIDEBUG
  #undef LOG_TAG
  #define LOG_NIDEBUG 0
  #define LOG_TAG "mm-camera-iface2"
  #include <utils/Log.h>
  #define IFACE_ERR(fmt, args...) \
    CLOGE(CAM_IFACE_MODULE, fmt, ##args)
  #define IFACE_HIGH(fmt, args...) \
    CLOGI(CAM_IFACE_MODULE, fmt, ##args)
  #define IFACE_DBG(fmt, args...) \
    CLOGD(CAM_IFACE_MODULE, fmt, ##args)
#else
  #include <stdio.h>
  #define IFACE_ERR(fmt, args...) fprintf(stderr, fmt, ##args)
  #define IFACE_HIGH(fmt, args...) fprintf(stderr, fmt, ##args)
  #define IFACE_DBG(fmt, args...) fprintf(stderr, fmt, ##args)
#endif

#endif /* __IFACE_DEF_H__ */
