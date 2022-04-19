/*============================================================================

  Copyright (c) 2014-2016 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __PEDESTAL46_H__
#define __PEDESTAL46_H__

#include "chromatix.h"
#include "chromatix_common.h"

#include "isp_sub_module_common.h"
#include "isp_defs.h"
#include "pedestal_correct_reg.h"

#define PEDESTAL_H_GRIDS        12
#define PEDESTAL_V_GRIDS         9
#define PEDESTAL_INTERP_FACTOR   3
#define PEDESTAL_LUT_SIZE   ((PEDESTAL_H_GRIDS+1)*(PEDESTAL_V_GRIDS+1))

#define PEDESTAL_X_DELTA_Q_LEN 20
#define PEDESTAL_Y_DELTA_Q_LEN 20

#define PED_MIN_BICUBIC_H_GRID_NUM 12
#define PED_MIN_BICUBIC_V_GRID_NUM 9

#define HW_PED_NUM_ROW 12
#define HW_PED_NUM_COL 9

#define PEDESTAL_MAX_VALUE 4095

#define INSENSOR_HDR_BIT_WIDTH 10 /* todo: should send from sensor */

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_PEDESTAL, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_PEDESTAL, fmt, ##args)
typedef struct {
  uint32_t gr_r[PEDESTAL_LUT_SIZE];
  uint32_t gb_b[PEDESTAL_LUT_SIZE];
} pedestal46_hw_table_t;

typedef struct {
  ISP_Pedestal_CfgCmdType     pedestal_cfg;
  chromatix_pedestalcorrection_type    *chromatix_tbls;
  chromatix_pedestalcorrection_type    chromatix_tbls_output;
  chromatix_black_level_type *chromatix_black_lvl;
  pedestal46_hw_table_t       hw_tables[2];
  sensor_out_info_t           sensor_out_info;
  float                       aec_ratio;
  boolean                     is_hdr_mode;
  unsigned short              max_blk_increase;
  isp_out_info_t              isp_out_info;
  ispif_out_info_t            ispif_out_info;
} pedestal46_mod_t;

/** pedestal46_set_chromatix_ptr
 *
 *  @module: mct module handle
 *  @isp_sub_mdoule: isp sub module handle
 *  @event: event handle
 *
 *  Update chromatix ptr
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean pedestal46_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

/** pedestal46_trigger_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: ISP sub module handle
 *  @event: mct event handle
 *
 *  Perform trigger update using aec_update
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean pedestal46_trigger_update(
    mct_module_t     *module,
    isp_sub_module_t *isp_sub_module,
    mct_event_t      *event);

/** pedestal46_set_stripe_info:
 *
 *  @module: mct module handle
 *  @isp_sub_module: ISP sub module handle
 *  @event: mct event handle
 *
 *  Perform set stripe info
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean pedestal46_set_stripe_info(
  mct_module_t     *module,
  isp_sub_module_t *isp_sub_module,
  mct_event_t      *event);

/** pedestal46_set_split_info:
 *
 *  @module: mct module handle
 *  @isp_sub_module: ISP sub module handle
 *  @event: mct event handle
 *
 *  Perform set split info
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean pedestal46_set_split_info(
    mct_module_t     *module,
    isp_sub_module_t *isp_sub_module,
    mct_event_t      *event);

/** pedestal46_stats_aec_update:
 *
 *  @module: pedestal module
 *  @isp_sub_module: ISP sub module handle
 *  @event: mct event handle
 *
 *  Handle AEC update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean pedestal46_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

/** pedestal46_streamon:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function makes initial configuration during first
 *  stream ON
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean pedestal46_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

/** pedestal46_streamoff:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean pedestal46_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

/** pedestal46_init:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the pedestal46 module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean pedestal46_init(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);

/** pedestal46_set_stream_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  Store stream configuration
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean pedestal46_set_stream_config(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean pedestal46_set_hdr_mode(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

/** pedestal46_destroy:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void pedestal46_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);

void pedestal46_bicubic (float fs, float* fc0, float* fc1, float* fc2,
  float* fc3);

/** pedestal46_scale_table:
 *    @meshOut: output new meshtbl according to camif info
 *
 *  @MeshIn: the input Rolloff table (10x13) at the full sensor
 *  @MeshOut: the output rolloff table (10x13) at the current
 *           output resolution
 *  @full_width: the full-resolution width
 *  @full_height: the full-resolution height
 *  @output_width: the output width
 *  @output_height: the output height
 *  @offset_x: x-index of the top-left corner of output image on
 *            the full-resolution sensor
 *  @offset_y: y-index of the top-left corner of output image on
 *            the full-resolution sensor
 *  @scale_factor: the sensor scaling factor
 *  (= binning_factor digal_sampling_factor)
 *  @hdr_factor: multiplication factor for in-sensor hdr mode
 *
 *  This function is used to resample the Rolloff table from
 *  the full-resolution sensor to the (CAMIF) output size, which
 *  is decided by the sensor mode.
 *
 *  Return void
 **/
void pedestal46_scale_table(
    unsigned short *MeshIn,       // Input Rolloff table (10x13) at the full sensor
    unsigned short *MeshOut,      // Output rolloff table (10x13) at current o/p resolution
    int            full_width,    // Full-resolution width
    int            full_height,   // Full-resolution height
    int            output_width,  // Output width
    int            output_height, // Output height
    int            offset_x,      // x-index of top-left corner of o/p image
    int            offset_y,      // y-index of top-left corner of o/p image
    int            scale_factor,  // Sensor scaling factor(=binning_factor * digal_sampling_factor)
    float          hdr_factor);

void pedestal46_scale_all_tables(chromatix_pedestalcorrection_type *inTbl,
  chromatix_pedestalcorrection_type *outTbl, sensor_rolloff_config_t* sensor_info,
  float hdr_factor);

float pedestal_correct46_get_aec_ratio(pedestal46_mod_t *mod, stats_update_t *stats_update);

void pedestal_correct46_get_table (pedestal46_mod_t *mod, pedestalcorrection_table *tblOut, int idx);

void pedestal46_table_interpolate(pedestalcorrection_table *in1,
  pedestalcorrection_table *in2, pedestalcorrection_table *out, float ratio);
#endif //__PEDESTAL46_H__
