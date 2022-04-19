/* isp_pipeline.h
 *
 * Copyright (c) 2012-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __ISP_PIPELINE_H__
#define __ISP_PIPELINE_H__

/* mctl headers */
#include "q3a_stats_hw.h"

/* isp headers */
#include "isp_common.h"
#include "isp_defs.h"

/** isp_hw_table_t:
 *
 *  hw_name: sub module name
 **/
typedef struct {
  char hw_name[ISP_HW_NAME_LEN];
  isp_hw_module_id_t hw_module_id;
  enum msm_isp_stats_type stats_type;
} isp_hw_info_t;

/** isp_hw_params_t:
 *
 *  @hw_table_name: array of hw names
 *  @num_hw: number of hw's
 **/
typedef struct {
  isp_hw_info_t *hw_info_table;
  uint32_t           num_hw;
} isp_hw_params_t;

/** isp_resource_hw_stream_info_t:
 *
 *  @max_width: max width supported by hw stream
 *  @max_height: max height supported by hw stream
 **/
typedef struct {
  int32_t max_width;
  int32_t max_height;
} isp_resource_hw_stream_info_t;

typedef enum {
  ISP_REGISTER_MODULE_CFG,
  ISP_REGISER_MAX
} isp_register_name_t;

typedef struct {
  uint32_t       stats_mask;
  uint32_t       parse_mask;
  uint32_t       num_stats_comp_grp;
  uint32_t       stats_comp_grp_mask[2];
} isp_stats_mask_ctrl_t;

/** isp_parser_session_params_t:
 *
 *  @saturated_ext: bg saturated ext
 **/
typedef struct {
  boolean saturated_ext;
} isp_parser_session_params_t;

typedef enum {
  ISP_CX_LEVEL_SVS,
  ISP_CX_LEVEL_NOMINAL,
  ISP_CX_LEVEL_TURBO,
  ISP_CX_LEVEL_MAX
} isp_cx_levels_t;

/** isp_pipeline_func_table_t:
 *
 *  @pipeline_parse: pipeline parse function
 *  @pipeline_fill_stats_info: pipeline fill stats info
 *  @pipeline_calculate_roi_map: pipeline calculate roi map
 *  @pipeline_update_module_cfg: update module cfg register
 **/
typedef struct {
  boolean (*(pipeline_stats_parse[MSM_ISP_STATS_MAX]))(void *,
    mct_event_stats_isp_t *,
    isp_saved_stats_params_t *, isp_parser_session_params_t *);
  boolean (*pipeline_fill_stats_info)(uint32_t, iface_resource_request_t *,
    isp_stats_mask_ctrl_t *, boolean , uint32_t, isp_parser_session_params_t *);
  boolean (*pipeline_calculate_roi_map)(isp_zoom_params_t *,
    mct_bus_msg_stream_crop_t *, uint32_t);
  uint32_t (*adrc_hw_module_id_mask)(void);
  boolean (*pipeline_update_module_cfg)(int32_t, uint8_t *,
    isp_module_enable_info_t *, cam_format_t);
  boolean (*pipeline_update_stats_mask_for_mono_sensor)(uint32_t *stats_mask);
  boolean (*pipeline_map_stats_mask_to_hw)(uint32_t *stats_mask);
} isp_pipeline_stats_func_table_t;

/** isp_pipeline_t:
 *
 *  @isp_id: ISP id
 *  @isp_version: ISP version
 *  @num_pix: number of pixel interfaces
 *  @num_rdi: number of RDI intefaces
 *  @num_wms: number of write masters
 *  @num_register: number registers
 *  @max_width: max supported resolution
 *  @max_height: max supported resolution
 *  @max_pix_clk: max pixel clock supported
 *  @num_hw_streams: number of hw streams
 *  @max_scale_ratio: ISP max downscale ratio
 *  @register_addr_table: register addr table
 *  @hw_stream_ids: container of all HW stream IDs
 *  @hw_stream_info: hw stream info
 *  @common_hw_params: handle to store common hw params
 *  @stream_hw_params: handle to store stream hw params
 *  @max_supported_stats: handle to store max supported stats
 *  @func_table: function table for parser
 **/
typedef struct {
  uint32_t                        isp_id;
  uint32_t                        isp_version;
  uint32_t                        hw_version;
  uint32_t                        num_pix;
  uint32_t                        num_rdi;
  uint32_t                        num_wms;
  uint32_t                        num_register;
  uint32_t                        max_width;
  uint32_t                        max_height;
  uint32_t                        max_pix_clk_rate[ISP_CX_LEVEL_MAX];
  uint32_t                        num_hw_streams;
  boolean                         is_camif_crop_supported;
  boolean                         is_camif_raw_op_fmt_supported;
  uint8_t                         is_mono_stats_supported;
  uint8_t                         submod_enable[ISP_MOD_MAX_NUM];
  uint8_t                         pipeline_applying_delay;
  uint8_t                         max_scale_ratio;
  uint32_t                        is_hvx_present;
  isp_hw_streamid_t               hw_stream_ids[ISP_HW_STREAM_MAX];
  isp_resource_hw_stream_info_t   hw_stream_info[ISP_HW_STREAM_MAX];
  isp_hw_params_t                *common_hw_params;
  isp_hw_params_t                *stream_hw_params[ISP_HW_STREAM_MAX];
  isp_pipeline_stats_func_table_t *func_table;
  uint32_t                        *supported_stats_tap_location;
  boolean                         is_ahb_clk_cfg_supported;
  uint32_t                        num_stats_comp_grp;
  uint32_t                        single_vfe_cx_level;
} isp_pipeline_t;

isp_pipeline_t *isp_pipeline48_open(uint32_t isp_version, uint32_t isp_id);

isp_pipeline_t *isp_pipeline47_open(uint32_t isp_version, uint32_t isp_id);

isp_pipeline_t *isp_pipeline46_open(uint32_t isp_version, uint32_t isp_id);

isp_pipeline_t *isp_pipeline44_open(uint32_t isp_version, uint32_t isp_id);

isp_pipeline_t *isp_pipeline41_open(uint32_t isp_version, uint32_t isp_id);

isp_pipeline_t *isp_pipeline40_open(uint32_t isp_version, uint32_t isp_id);

isp_pipeline_t *isp_pipeline32_open(uint32_t isp_version, uint32_t isp_id);

isp_pipeline_t *isp_pipeline42_open(uint32_t isp_version, uint32_t isp_id);

#endif
