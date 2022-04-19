/* mct_stream.h
 *
 * Copyright (c) 2012-2016, 2018 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MCT_STREAM_H__
#define __MCT_STREAM_H__

#include "mct_object.h"
#include "mct_module.h"
#include "cam_intf.h"
#include "mct_event.h"


# if 0
#define MCT_ST_STATE(stream)         (MCT_STREAM_CAST(stream)->current_state)
#define MCT_ST_STATE_NEXT(stream)    (MCT_STREAM_CAST(stream)->next_state)
#define MCT_ST_STATE_PENDING(stream) (MCT_STREAM_CAST(stream)->pending_state)
#define MCT_ST_STATE_TARGET(stream)  (MCT_STREAM_CAST(stream)->target_state)

/**
 * MctSTStateChange:
 *
 * */
typedef enum {
  MCT_ST_STATE_CHANGE_STOP_TO_RESET    = (MCT_ST_STATE_STOP<<3)  |
    MCT_ST_STATE_RESET,

  MCT_ST_STATE_CHANGE_RESET_TO_RUNNING = (MCT_ST_STATE_RESET<<3) |
    MCT_ST_STATE_RUNNING,

  MCT_ST_STATE_CHANGE_RUNNING_TO_STOP = (MCT_ST_STATE_RUNNING<<3)|
    MCT_ST_STATE_STOP
} MctSTStateChange;

#define MCT_ST_STATE_RETURN(stream)  (MCT_STREAM_CAST(mod)->last_return)

#define MCT_STREAM(val)                 ((val) < 0 ? -1 : ((val) > 0 ? 1 : 0))
#define MCT_ST_STATE_GET_NEXT(cur,pending) \
  ((MctSTState)((cur) + MCT_STREAM ((int)(pending) - (int)(cur))))
#define MCT_ST_STATE_TRANSITION(cur,next)  \
  ((MctSTStateChange)(((cur)<<3)|(next)))

#define MCT_ST_STATE_TRANSITION_CURRENT(trans) ((MctSTState)((trans)>>3))
#define MCT_ST_STATE_TRANSITION_NEXT(trans)    ((MctSTState)((trans)&0x7))

typedef boolean (* MctSTSetStateFunction)    (MctStream *stream,
                                              MctSTState state);

typedef boolean (* MctSTChangeStateFunction) (MctStream *stream,
                                              MctSTState state);
#endif /* if 0 */

#define MCT_SESSION_STREAM_ID 15
#define MAX_STREAM_MODULES  6

typedef enum {
  MCT_STREAM_STATUS_OK,
  MCT_STRAEM_STATUS_NO_ISP_RESOURCE,
} mct_stream_status_t;

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
typedef struct _mct_stream_info_t {
  unsigned int identity;

  /* stream type*/
  cam_stream_type_t stream_type;

  /* image format */ /* for sensor, */
  cam_format_t fmt;

  /* image dimension */ /* for sensor, */
  cam_dimension_t dim;

  /* buffer plane information, will be calc based on stream_type, fmt,
     dim, and padding_info(from stream config). Info including:
     offset_x, offset_y, stride, scanline, plane offset */
  cam_stream_buf_plane_info_t buf_planes;


  /* streaming type */
  cam_streaming_mode_t streaming_mode;

  /* burst number of snapshot */
  int num_burst;

  /*num of frames generated
   * only valid when streaming mode = CAM_STREAMING_MODE_BATCH*/
  cam_stream_user_buf_info_t user_buf_info;

  /*DS mapped buffer information*/
  mct_list_t *img_buffer_list;

  /* Stream buffer parameters */
  cam_stream_parm_buffer_t parm_buf;

  cam_pp_feature_config_t pp_config;
  cam_stream_reproc_config_t reprocess_config;
  int num_bufs;

  mct_stream_status_t status;

  mct_stream_t *stream;

  /* Buffer params during aspect ratio mismatch */
  uint32_t resize;
  cam_dimension_t original_dim;
  boolean expect_divert;
  /*TODO: Add more fileds based on requirements*/

  /* Image Stabilization type */
  cam_is_type_t is_type;
  uint8_t dis_enable;
  cam_stream_secure_t is_secure;
  cam_stream_secure_mode_t secure_mode;
  cam_perf_mode_t perf_mode;
  boolean bayerdata;

  /* DT for this stream */
  uint32_t dt;

  /* VC for this stream */
  uint32_t vc;
  cam_sub_format_type_t sub_format;

} mct_stream_info_t;

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
  MCT_ST_STATE_IDLE             = 0,
  MCT_ST_STATE_STARTING         = 1,
  MCT_ST_STATE_LINKED           = 2,
  MCT_ST_STATE_STOP             = 3,
  MCT_ST_STATE_RUNNING          = 4,
  MCT_ST_STATE_PENDING_RESTART  = 5,
  MCT_ST_STATE_BAD              = 6,
  MCT_ST_STATE_MAX              = 7
} mct_stream_state_t;

/** mct_stream_buf_plane_t
 *    @buf:   vitual address from mmap
 *    @size:  buffer length
 *    @fd:    file descriptor from domain socket
 *
 *  Buffer plane Mapped from HAL via domain socket
 **/
typedef struct {
  void *buf;
  size_t size;
  int  fd;
  uint32_t offset;
  int32_t stride;
  int32_t scanline;
} mct_stream_buf_plane_t;

/** MctStreamMapBuf_t
 *    @buf:   vitual address from mmap
 *    @size:  buffer length
 *    @fd:    file descriptor from domain socket
 *    @index: buffer index
 *
 *  Buffers Mapped from HAL via domain socket
 **/
typedef struct {
  uint32_t buf_index;
  uint32_t num_planes;
  size_t buf_size;
  cam_mapping_buf_type buf_type;
  boolean common_fd;
  int container_fd;
  mct_stream_buf_plane_t buf_planes[VIDEO_MAX_PLANES];
  void *buf_container_info;
} mct_stream_map_buf_t;

/** MctStreamBufs_t:
 *    @config:
 *    @imgBuf:
 *    @aecBuf:
 *    @awbBuf:
 *    @afBuf:
 *    @histBuf:
 *
 **/
typedef struct {
  void                 *stream_info;
  size_t               stream_size;
  int                   stream_fd;
  mct_list_t           *img_buf;
  mct_list_t           *container_buf;
  pthread_mutex_t       lock_img_buf;
} mct_stream_bufs_t;

/** mct_bus_msg_iface_metadata_t:
 *
 *  @is_uv_subsampled: uv subsample flag
 **/
typedef struct {
  uint32_t                is_uv_subsampled;
  cam_stream_ID_t         frame_drop;
  mct_bus_msg_type_t      type;
} mct_bus_msg_iface_metadata_t;

typedef struct {
  boolean is_lpm_enabled;
}mct_bus_msg_isp_low_power_mode_t;

/** mct_stream_session_metadata_info:
 *
 *  @sensor_data: sensor metadata
 *  @stats_aec_data: aec metadata
 *  @isp_stats_awb_data: awb metadata
 *  @isp_gamma_data: gamma metadata
 *  @iface_metadata: isp metadata
 *  @sac_output_info: sac result
 **/
typedef struct {
  mct_bus_msg_sensor_metadata_t sensor_data;
  mct_bus_msg_stats_aec_metadata_t stats_aec_data;
  mct_bus_msg_isp_stats_awb_metadata_t isp_stats_awb_data;
  mct_bus_msg_isp_gamma_t isp_gamma_data;
  mct_bus_msg_iface_metadata_t iface_metadata;
  mct_bus_msg_isp_low_power_mode_t isp_lpm_data;
  mct_bus_msg_hdr_data hdr_data;
  cam_sac_output_info_t sac_output_info;
  mct_bus_msg_stats_awb_metadata_t stats_awb_data;
} mct_stream_session_metadata_info;

typedef struct {
  int32_t                               current_buf_idx;
  int                                   buf_mgr_dev_fd;
  uint32_t                              get_buf_err;
  uint32_t                              current_frame_idx;
  mct_list_t                            *frame_num_id_map;
  mct_stream_session_metadata_info      session_meta;
  metadata_buffer_t                     *metadata_buffer;
  pthread_mutex_t                       metadata_buf_lock;
} mct_stream_metadata_t;

/* add modules to and remove modules from the stream */
typedef boolean (* mct_stream_add_module_func)
  (mct_stream_t *stream, mct_module_t *module);

typedef boolean (* mct_stream_remove_module_func)
  (mct_stream_t *stream, mct_module_t *module);

typedef boolean (* mct_stream_insert_module_func)
  (/* FIXME */);

typedef boolean (* mct_stream_configure_func)
  (/* FIXME */);

typedef boolean (* mct_stream_send_event_func)
  (mct_stream_t *stream, mct_event_t *event);

typedef boolean (* mct_stream_link_modules_func) (mct_stream_t *stream);

typedef void    (* mct_stream_unlink_modules_func)
  (mct_stream_t *stream, mct_module_t *mod1, mct_module_t *mod2);

typedef boolean (* mct_stream_map_buf_func)
  (void *msg, mct_stream_t *stream);

typedef boolean (* mct_stream_unmap_buf_func)
  (void *msg, mct_stream_t *stream);

typedef boolean (* mct_stream_map_parm_func)
  (mct_stream_t *stream);

typedef boolean (* mct_stream_unmap_parm_func)
  (mct_stream_t *stream);

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
struct _mct_stream {
  /* Stream's parent should be pipeline,
   * Stream's children should be modules which is arranged from
   * source module to sink module -
   * Note: Only Stream's children list can grow into a TREE form */
  mct_object_t     object;

  uint32_t streamid;
  mct_stream_info_t  streaminfo;

  /*
   * Everything is in one thread context, there is no need
   * to stuck in State. We shall difinitely NOT consider
   * asynchronous State transition, otherwise things could
   * end up with much complexity.
   *
   * At stream layer, 3 States: LINKED STOP and RUNNING should be
   * sufficient.
   */
  mct_stream_state_t     state;
  uint32_t               current_frame_id;
  struct isp_kstate      *isp_kstate_ptr;
  uint32_t               hw_sleep_frame_id;
  mct_stream_bufs_t      buffers;
  void                   *stream_private;
  int                    buf_mgr_fd;
  mct_stream_metadata_t  metadata_stream; /*Could be added to stream private?*/
  mct_list_t             *frame_num_idx_list;
  boolean                valid_buffer;

  /* virtual functions */
  mct_stream_add_module_func     add_module;
  mct_stream_remove_module_func  remove_module;
  mct_stream_insert_module_func  insert_module;
  mct_stream_configure_func      configure;
  mct_stream_send_event_func     send_event;
  mct_stream_link_modules_func   link;
  mct_stream_unlink_modules_func unlink;
  mct_stream_map_buf_func        map_buf;
  mct_stream_unmap_buf_func      unmap_buf;
  mct_stream_map_parm_func       map_parm;
  mct_stream_unmap_parm_func     unmap_parm;
};

#define MCT_STREAM_CAST(stream)        ((mct_stream_t *)(stream))
#define MCT_STREAM_PARENT(stream)      (MCT_OBJECT_PARENT(stream))
#define MCT_STREAM_NUM_CHILDREN(stream)(MCT_OBJECT_NUM_CHILDREN(stream))
#define MCT_STREAM_CHILDREN(stream)    (MCT_OBJECT_CHILDREN(stream))
#define MCT_STREAM_LOCK(stream)        (MCT_OBJECT_LOCK(stream))
#define MCT_STREAM_UNLOCK(stream)      (MCT_OBJECT_UNLOCK(stream))
#define MCT_STREAM_LINK(stream)        (MCT_STREAM_CAST(stream)->link)
#define MCT_STREAM_STREAMINFO(stream)  \
  (MCT_STREAM_CAST(stream)->buffers.stream_info)

#define mct_stream_set_add_module_func(stream,f)  \
  MCT_STREAM_CAST(stream)->add_module = f

#define mct_stream_set_remove_module_func(stream,f)  \
  MCT_STREAM_CAST(stream)->remove_module = f

#define mct_stream_set_insert_module_func(stream,f)  \
  MCT_STREAM_CAST(stream)->insert_module = f

#define mct_stream_set_configure_func(stream,f)  \
  MCT_STREAM_CAST(stream)->configure = f

#define mct_stream_set_send_event_func(stream,f)  \
  MCT_STREAM_CAST(stream)->send_event = f

#define mct_stream_set_link_modules_func(stream,f)  \
  MCT_STREAM_CAST(stream)->link_modules = f

#define mct_stream_set_unlink_modules_func(stream,f)  \
  MCT_STREAM_CAST(stream)->unlink_modules = f

#define mct_stream_set_map_buf_func(stream,f)  \
  MCT_STREAM_CAST(stream)->map_buf = f

#define mct_stream_set_unmap_buf_func(stream,f)  \
  MCT_STREAM_CAST(stream)->unmap_buf = f

#define mct_stream_set_map_parm_func(stream,f)  \
  MCT_STREAM_CAST(stream)->map_parm = f

#define mct_stream_set_unmap_parm_func(stream,f)  \
  MCT_STREAM_CAST(stream)->unmap_parm = f

mct_stream_t *mct_stream_new(uint32_t stream_id);

boolean mct_stream_destroy(mct_stream_t *stream);

boolean mct_stream_process_bus_msg(mct_stream_t *stream,
  mct_bus_msg_t *bus_msg);

boolean mct_stream_link_modules(mct_stream_t *stream,
  mct_module_t *mod1, mct_module_t *mod2, ...);

boolean mct_stream_link_branch_modules(mct_stream_t *stream,
  mct_module_t *mod1, mct_module_t *mod2, ...);
boolean mct_stream_remove_stream_from_module(void *data,
  void *user_data);
void mct_stream_operate_unlink(void *d1, void *d2,
  const void *user_data);
boolean mct_stream_map_frame_number(mct_pipeline_t *pipeline,
  uint32_t frame_num, uint32_t frame_id);
boolean mct_stream_free_frame_num_list(void *data, void *user_data);

boolean mct_stream_send_super_event(mct_stream_t *parm_stream,
  mct_event_super_control_parm_t *super_event);

boolean mct_stream_send_ctrl_sof(mct_stream_t *parm_stream,
  mct_bus_msg_isp_sof_t *isp_sof_bus_msg);

boolean mct_stream_send_hw_error(mct_stream_t *stream);

void add_metadata_entry(int meta_type, uint32_t meta_length,
  void *meta_value, metadata_buffer_t *m_table);

void *get_pointer_of(cam_intf_parm_type_t meta_id,
  metadata_buffer_t* metadata);
uint32_t get_size_of(cam_intf_parm_type_t param_id);

boolean mct_stream_get_metadata_buffer(mct_stream_t *stream,
  metadata_buffer_t **hal_buffer);

boolean mct_stream_put_metadata_buffer(mct_stream_t *stream);

boolean mct_stream_fill_metadata_buffer(metadata_buffer_t *pdst,
  void *psrc, mct_bus_msg_type_t type, unsigned int size,
  mct_stream_session_metadata_info *local);

boolean mct_stream_update_metadata_buffer(mct_stream_t *stream,
  metadata_buffer_t *metadata_buffer);

boolean mct_stream_send_offline_meta(mct_stream_t *stream,
  mct_bus_msg_isp_rd_done_t *isp_rd_done_msg);

boolean mct_stream_link_module_array(mct_stream_t *stream,
  char *(*stream_type_mod)[MAX_STREAM_MODULES]);

boolean mct_stream_update_all_frame_id_by(mct_pipeline_t *pipeline,
  unsigned int delay, unsigned int cur_sof_id);

boolean mct_stream_send_super_event_slave(mct_stream_t *parm_stream,
  mct_event_super_control_parm_t *super_event);


#endif /* __MCT_STREAM_H__ */
