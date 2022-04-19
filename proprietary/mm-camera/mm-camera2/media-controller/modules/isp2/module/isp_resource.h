/* isp_resource.h
 * Copyright (c) 2012-2014, 2017-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef __ISP_RESOURCE_H__
#define __ISP_RESOURCE_H__

/* hal headers */
#include "cam_types.h"

/* isp headers */
#include "isp_sub_module_common.h"
#include "isp_pipeline.h"
#include "isp_common.h"
#include "isp_port.h"

#define SOC_ID_TRINKET 394
typedef enum {
    NO_LIMIT,
    MAX_13MP,
    MAX_16MP,
    MAX_21MP,
} isp_hw_limit_t;

typedef enum {
    NO_LIMIT_DUAL,
    MAX_8MP_DUAL,
    MAX_13MP_DUAL,
    MAX_16MP_DUAL,
} isp_hw_limit_v2_t;


typedef enum {
  ISP_RESOURCE_FREE,
  ISP_RESOURCE_RESERVED,
  ISP_RESOURCE_CONFIGURED,
  ISP_RESOURCE_STREAMING,
} isp_resource_state_t;


/** isp_submod_hw_streams_info_t
 *
 *  @num_hw_streams: stream number
 *  @hw_stream_ids: HW streams ids
 **/
typedef struct {
  uint32_t          num_hw_streams;
  isp_hw_streamid_t hw_stream_ids[ISP_HW_STREAM_MAX];
} isp_submod_hw_streams_info_t;


/** isp_submod_hw_limit_desc_t
 *
 *  @hw_stream_id: HW streams id
 *  @max_width: max supported width
 *  @max_height: max supported height
 *  @max_scale_ratio: max supported downscale ration
 **/
typedef struct {
  isp_hw_streamid_t hw_stream_id;
  uint32_t          max_width;
  uint32_t          max_height;
  uint32_t          max_scale_ratio;
} isp_submod_hw_limit_desc_t;

/** isp_submod_hw_limit_t
 *
 *  @num_hw_streams: stream number
 *  @hw_limits: HW limitation description
 **/
typedef struct {
  uint32_t          num_hw_streams;
  isp_submod_hw_limit_desc_t hw_limits[ISP_HW_STREAM_MAX];
} isp_submod_hw_limit_t;

/** isp_submod_reserve_t
 *
 *  @stream_info: stream info
 *  @isp_port: isp port handle
 **/
typedef struct {
  mct_stream_info_t *stream_info;
  mct_port_t        *isp_port;
} isp_submod_reserve_t;

/** isp_submod_info_t:
 *
 *  @lib_handle: lib handle for submod
 *  @isp_submode_init_table: ISP submod initialization table
 *  @isp_submod: ISP sumodule handle
 **/
typedef struct {
  void                    *lib_handle;
  isp_submod_init_table_t *isp_submode_init_table;
  mct_module_t            *isp_submod;
} isp_submod_info_t;

/** isp_resource_alloc_t:
 *
 *  @state: state of resource
 *  @is_reserved: is resource reserved / available
 *  @session_id: session id that is holding the resource
 *  @num_streams: number of streams
 *  @l_identity: list of identities
 **/
typedef struct {
  isp_resource_state_t  state;
  uint32_t              session_id;
  uint32_t              num_streams;
  mct_list_t           *l_identity[ISP_HW_STREAM_MAX]; /* uint32_t */
} isp_resource_alloc_t;

/** isp_hw_data_t:
 *
 *  @subdev_name: subdev name for this resource
 *  @isp_pipeline: ISP pipeline handle
 *  @common_hw_info: common hw info handle
 *  @common_hw_size: common hw info size
 *  @stream_hw_info: stream hw info handle
 *  @stream_hw_size: stream hw info size
 *  @resource_alloc: resource allocation info
 *  @dim: dimension
 *  @out_info: output info
 *  @is_snapshot: flag that indicate whether snapshot stream is
 *              using this hw stream
 **/
typedef struct {
  char                  subdev_name[ISP_SUBDEV_NAME_LEN];
  isp_pipeline_t       *isp_pipeline;
  isp_submod_info_t    *common_hw_info;
  uint32_t              common_hw_size;
  isp_submod_info_t    *stream_hw_info[ISP_HW_STREAM_MAX];
  uint32_t              stream_hw_size[ISP_HW_STREAM_MAX];
  isp_resource_alloc_t  resource_alloc;
  isp_dim_t             dim[ISP_HW_STREAM_MAX];
  isp_out_info_t        out_info[ISP_HW_STREAM_MAX];
  boolean               is_snapshot[ISP_HW_STREAM_MAX];
} isp_resource_info_t;

typedef struct {
  char subdev_name[ISP_SUBDEV_NAME_LEN];
} isp_resource_subdev_names_t;

/** isp_session_info_t:
 *
 *  @is_valid: is valid
 *  @session_id: session id
 *  @format: bayer / yuv
 *  @max_op_pixel_clk: max op pixel clk
 *  @num_isp_preallocate: num of isp to preallocate
 **/
typedef struct {
  boolean                is_valid;
  uint32_t               session_id;
  mct_sensor_format_t    format;
  uint32_t               max_op_pixel_clk;
  uint32_t               num_isp_preallocate;
} isp_session_info_t;

/** isp_resource_hw_limits_t:
 *
 *  @max_width: Max supported width
 *  @max_height: Max supported height
 *
 **/
typedef struct {
    isp_hw_limit_t hw_limit;
    uint32_t max_resolution;
    int         vfe1_reserved_rdi;
} isp_resource_hw_limits_t;

/** isp_resource_t:
 *
 *  @isp_resource_info_t: ISP hardware info per ISP
 *  @num_isp: Total number of ISP hardwares on current
 *          platform
 *  @session_info: session info
 *  @num_session_opened: num session opened
 *  @num_isp_preallocate: num isp pre allocated
 *  @mutex: lock
 **/
typedef struct {
  isp_resource_info_t isp_resource_info[ISP_HW_MAX];
  uint32_t            sorted_hw_ids[ISP_HW_MAX];
  uint32_t            num_isp;
  uint32_t            request_op_clk;
  isp_session_info_t  session_info[ISP_MAX_SESSION];
  uint32_t            num_session_opened;
  uint32_t            num_isp_preallocate;
  pthread_mutex_t     mutex;
  isp_resource_hw_limits_t isp_hw_limt;
} isp_resource_t;

boolean isp_resource_get_hw_streams(isp_resource_t *isp_resource,
  isp_submod_hw_streams_info_t *hw_streams, uint32_t isp_id);

boolean isp_resource_get_hw_limitations(isp_resource_t *isp_resource,
  isp_submod_hw_limit_t *hw_limit, uint32_t isp_num, isp_hw_id_t *hw_ids);

boolean isp_resource_allocate(mct_module_t *module,
  isp_resource_t *isp_resource, uint32_t session_id, uint32_t *num_isp,
  isp_hw_id_t *hw_ids, uint8_t offline,  uint32_t request_dual_isp);

boolean isp_resource_deallocate(mct_module_t *module,
  isp_resource_t *isp_resource, uint32_t session_id,
  uint32_t *num_isp, isp_hw_id_t *hw_ids);

boolean isp_resource_init(isp_resource_t *isp_resource);

void isp_resource_destroy(isp_resource_t *isp_resource);

boolean isp_resource_create_link(mct_module_t *module, mct_port_t *port,
  isp_resource_t *isp_resource, mct_stream_info_t *stream_info);

boolean isp_resource_create_link_without_alloc(mct_module_t *module,
  mct_port_t *port, isp_resource_t *isp_resource,
  mct_stream_info_t *stream_info, isp_hw_id_t hw_id);

boolean isp_resource_destroy_internal_link(mct_port_t *port, uint32_t identity);

void isp_resource_destroy_internal_link_hw_id(mct_port_t *port,
  isp_port_data_t *port_data, uint32_t identity, isp_hw_id_t hw_id);

boolean isp_resource_iface_request_output_resource(mct_module_t *module,
  mct_port_t *port, mct_event_t *event);

boolean isp_resource_iface_request_offline_isp(mct_module_t *module,
  mct_port_t *port, mct_event_t *event);

boolean isp_resource_iface_request_offline_output_resource(mct_module_t *module,
  mct_port_t *port, mct_event_t *event);

boolean isp_resource_start_session(isp_resource_t *isp_resource,
  uint32_t session_id);

boolean isp_resource_stop_session(isp_resource_t *isp_resource,
  uint32_t session_id);

boolean isp_resource_update_module_cfg(isp_resource_t *isp_resource,
  isp_hw_id_t hw_id, int32_t fd, isp_module_enable_info_t *enable_bit_info,
  cam_format_t sensor_fmt);

boolean isp_resource_pipeline_parse(isp_resource_t *isp_resource,
  isp_hw_id_t hw_id, enum msm_isp_stats_type stats_type,
  uint32_t raw_buf_len, void *input_buf,
  mct_event_stats_isp_t *output,
  isp_saved_stats_params_t *stats_params_isp0,
  isp_saved_stats_params_t *stats_params_isp1,
  isp_parser_session_params_t *parser_session_params);

boolean isp_resource_query_mod(isp_resource_t *isp_resource,
  void *query_buf, unsigned int session_id);

boolean isp_resource_calculate_roi_map(isp_resource_t *isp_resource,
  isp_hw_id_t hw_id, isp_zoom_params_t *zoom_params_arr,
  mct_bus_msg_stream_crop_t *stream_crop, uint32_t identity);

boolean isp_resource_get_vfe_version(isp_resource_t *isp_resource,
  uint32_t vfe_id, uint32_t * version);

boolean isp_resource_request_offline_stripe_info(isp_resource_t *isp_resource,
  ispif_out_info_t *ispif_split_output_info,
  void  *param, uint32_t hw_id, uint32_t identity);
#endif
