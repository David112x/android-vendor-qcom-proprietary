/*
* Copyright (c) 2012-2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef __MESH_ROLLOFF44_H__
#define __MESH_ROLLOFF44_H__

/* mctl headers */
#include "chromatix.h"
#include "chromatix_common.h"

/* isp headers */
#include "mesh_rolloff_reg.h"
#include "isp_sub_module_common.h"

#ifndef ROLLOFF_CHROMATIX_TABLE_LED2
#define ROLLOFF_CHROMATIX_TABLE_LED2(pchromatix) (pchromatix->chromatix_mesh_rolloff_table_LED2)
#endif
/* Macros */
/* init 0 or 64 compute by system team code*/
#define NUM_OF_SUB_GRID 4
/* 2 ^ INTERP_FACTOR = NUM_OF_SUB_GRID */
#define INTERP_FACTOR 3
#define DBG_BUF_LEN \
  (CHROMATIX_MESH_ROLL_NUM_COL * CHROMATIX_MESH_ROLL_NUM_ROW * 4)

#define TINTLESS_TEMPORAL_RATIO 0.7
#define CLAMP_DATA(data) \
  data= (data < 1024)? 1024:(data > 8191)? 8191:data; \

#ifndef sign
#define sign(x) (((x) < 0) ? (-1) : (1))
#endif
#ifndef Round
#define Round(x) (int)((x) + sign(x)*0.5)
#endif
#define GET_INTERPOLATION_RATIO_ROLLOFF(ct, s, e) \
  (float)(1.0 - ((ct) - (s))/((e) - (s)))
#define TBL_INTERPOLATE_ROLL_OFF(in1, in2, out, ratio, size, i) ({\
    for (i=0; i<size; i++) \
        out[i] = LINEAR_INTERPOLATION((double)in1[i], (double)in2[i], (double)ratio); })

/* chromatix data */
#define CHROMATIX_MESH_ROLL_NUM_COL 13
#define CHROMATIX_MESH_ROLL_NUM_ROW 17
#define CHROMATIX_MESH_TABLE_SIZE  (17 * 13)

#define HW_MESH_ROLL_NUM_ROW 13
#define HW_MESH_ROLL_NUM_COL 17

#define MIN_BICUBIC_H_GRID_NUM 8
#define MIN_BICUBIC_V_GRID_NUM 6

#define MESH_ROLLOFF_HORIZONTAL_GRIDS_MAX  16
#define MESH_ROLLOFF_VERTICAL_GRIDS_MAX    12
#define MESH_ROLL_OFF_V4_TABLE_SIZE  MESH_ROLLOFF_SIZE

#define MESH_ROLL_OFF_V4_EXTEND_MESH_SIZE ((17+4) * (13+4))

typedef enum {
  ISP_ROLLOFF_TL84_LIGHT,   /* Flourescent  */
  ISP_ROLLOFF_A_LIGHT,      /* Incandescent */
  ISP_ROLLOFF_D65_LIGHT,    /* Day Light    */
  ISP_ROLLOFF_H_LIGHT,    /* Horizon Light    */
  ISP_ROLLOFF_TL84_LOW_LIGHT,    /* Flourescent - Low Light */
  ISP_ROLLOFF_A_LOW_LIGHT,    /* Incandescent - Low Light */
  ISP_ROLLOFF_D65_LOW_LIGHT,    /* Day Light - Low Light */
  ISP_ROLLOFF_H_LOW_LIGHT,    /* Horizon Light - Low Light */
  ISP_ROLLOFF_LED_FLASH,    /* Led Flash    */
  ISP_ROLLOFF_STROBE_FLASH, /* Stribe Flash */
#if OVERRIDE_FUNC
  ISP_ROLLOFF_TL84_OUTDOOR_LIGHT,    /* Flourescent - Outdoor Light */
#endif
  ISP_ROLLOFF_MAX_LIGHT,
  ISP_ROLLOFF_INVALID_LIGHT = ISP_ROLLOFF_MAX_LIGHT
} rolloff_light_type;

typedef enum {
  ISP_ROLLOFF_LENS_POSITION_INF = 0,
  ISP_ROLLOFF_LENS_POSITION_MACRO,
  ISP_ROLLOFF_LENS_POSITION_MAX
} rolloff_lens_position;

/** MESH_RollOffTable_V4:
 *
 *  @TableR: Table for R
 *  @TableGr: Table for Gr
 *  @TableB: Table for B
 *  @TableGb: Table for Gb
 **/
typedef struct {
  float TableR[MESH_ROLL_OFF_V4_TABLE_SIZE];
  float TableGr[MESH_ROLL_OFF_V4_TABLE_SIZE];
  float TableB[MESH_ROLL_OFF_V4_TABLE_SIZE];
  float TableGb[MESH_ROLL_OFF_V4_TABLE_SIZE];
} MESH_RollOffTable_V4;

/** isp_rolloff_info_t:
 *
 *  @left: left region
 *  @right: right region
 **/
typedef struct {
  MESH_RollOffTable_V4 left[ISP_ROLLOFF_MAX_LIGHT];
  MESH_RollOffTable_V4 right[ISP_ROLLOFF_MAX_LIGHT];
} isp_rolloff_info_t;

/** isp_rolloff_tableset_t:
 *
 *  @rolloff_tableset: rolloff table set
 **/
typedef struct {
  isp_rolloff_info_t *rolloff_tableset[ISP_ROLLOFF_LENS_POSITION_MAX];
} isp_rolloff_tableset_t;

/** MESH_RollOff_V4_ConfigTable:
 *
 *  @Table: mesh rolloff config table
 **/
typedef struct MESH_RollOff_V4_ConfigTable {
  uint32_t Table[MESH_ROLL_OFF_V4_TABLE_SIZE];
} MESH_RollOff_V4_ConfigTable;

typedef struct {
  int scale_cubic; // bicubic interpolation scale (8, 4, 2, 1) from grid to subgrid
  int deltah;      // horizontal offset (per-channel) of rolloff top-left point
  int deltav;      // vertical offset (per-channel) of rolloff top-left point
  int subgridh;    // horizontal size (per-channel) of rolloff subgrid
  int subgridv;    // vertical size (per-channel) of rolloff subgrid
} BE_Rolloff_Correction_CfgParams; /* cfg params for BE Rolloff correction algo */


/** MESH_RollOff_V4_ConfigCmdType:
 *
 *  @dmi_set_gr_r: dmi set Gr
 *  @Table_GR_R: Gr table
 *  @dmi_reset_gr_r: dmi rset Gr
 *  @dmi_set_gb_b: dmi set Gb
 *  @Table_GB_B: Gb able
 *  @dmi_reset_gb_b: dmi reset Gb
 *  @CfgParams: cfg params
 **/
typedef struct MESH_RollOff_V4_ConfigCmdType {
  uint32_t                     dmi_set_gr_r[2];
  MESH_RollOff_V4_ConfigTable  Table_GR_R;
  uint32_t                     dmi_reset_gr_r[2];
  uint32_t                     dmi_set_gb_b[2];
  MESH_RollOff_V4_ConfigTable  Table_GB_B;
  uint32_t                     dmi_reset_gb_b[2];
  MESH_RollOff_v4_ConfigParams CfgParams;
  BE_Rolloff_Correction_CfgParams be_cfg_params;
} MESH_RollOff_V4_ConfigCmdType;

/** mesh_rolloff_V4_params_t:
 *
 *  @input_table: input table
 **/
typedef struct {
  MESH_RollOffTable_V4 input_table;
} mesh_rolloff_V4_params_t;

/** mesh_rolloff44_tintless_params_t:
 *
 *  @tintless_array_valid: flag to indicate whether tintless
 *                       array is valid
 *  @tintless_array: tintless array
 *  @last_non_flash_tbl: last non flash table
 *  @lowlight_adjust: low light adjust table
 *  @current_adjust: current adjust table
 **/
typedef struct {
  boolean                 tintless_array_valid;
  mesh_rolloff_array_type tintless_array;
  mesh_rolloff_array_type adjust_tintless_array;
  mesh_rolloff_array_type pre_tintless_array;
  MESH_RollOffTable_V4    last_non_flash_tbl;
  float                   lowlight_adjust[MESH_ROLLOFF_SIZE];
  float                   current_adjust[MESH_ROLLOFF_SIZE];
  boolean                 update_input_table;
} mesh_rolloff44_tintless_params_t;

typedef void (*calc_interpolation)(isp_sub_module_t *isp_sub_module,
  void *mesh_rolloff44, isp_rolloff_info_t *mesh_tbls,
  MESH_RollOffTable_V4 *tblOut);
typedef void (*normalize)(isp_sub_module_t *isp_sub_module,
  void *mesh_mod_in);
typedef void (*calc_tintless_src_table)(isp_sub_module_t *isp_sub_module,
  MESH_RollOffTable_V4 *meshRolloffTableTintless);

typedef struct {
  normalize          normalize;
  calc_interpolation calc_interpolation;
  calc_tintless_src_table calc_tintless_src_table;
} ext_override_func;

/** mesh_rolloff44_t:
 *
 *  @cur_real_gain: cur real gain
 *  @cur_mired_color_temp: cur mired color temp
 *  @cur_flash_mode: cur flash mode
 *  @mesh_rolloff_cmd: mesh rolloff cmd config
 *  @mesh_rolloff_param: mesh rolloff param config
 *  @rolloff_calibration_table: rolloff calibration table
 *  @rolloff_tbls: rolloff table
 *  @applied_table: applied table
 *  @applied_hw_table_gr_r: applied hw table for Gr
 *  @applied_hw_table_gb_r: applied hw table for Gb
 *  @af_macro: af macro
 *  @af_infinity: af infinity
 *  @sensor_out_info: sensor out info
 *  @ispif_out_info: ISPIF out info
 *  @isp_out_info: ISP out info
 *  @aec_update: aec update info
 *  @flash type: flash type
 *  @lens_current_step: current lens step
 *  @tintless_params: tintless params
 *  @dump_enabled: flag to store dump enable / disable
 *  @dump_frames: number of frames to dump
 **/
typedef struct {
  float                            cur_mired_color_temp;
  cam_flash_mode_t                 cur_flash_mode;
  MESH_RollOff_V4_ConfigCmdType    mesh_rolloff_cmd;
  mesh_rolloff_V4_params_t         mesh_rolloff_param;
  isp_rolloff_tableset_t           rolloff_calibration_table;
  isp_rolloff_tableset_t           rolloff_tbls;
  MESH_RollOffTable_V4             applied_table;
  MESH_RollOff_V4_ConfigTable      applied_hw_table_gr_r;
  MESH_RollOff_V4_ConfigTable      applied_hw_table_gb_b;
  uint16_t                         af_macro;
  uint16_t                         af_infinity;
  sensor_out_info_t                sensor_out_info;
  af_rolloff_info_t                af_rolloff_info;
  AF_ROLLOFF_CHROMATIX_TYPE        af_rolloff_chromatix;
  boolean                          af_rolloff_info_valid;
  ispif_out_info_t                 ispif_out_info;
  isp_out_info_t                   isp_out_info;
  aec_update_t                     aec_update;
  af_update_t                      af_update;
  camera_flash_type                flash_type;
  stats_dual_led_settings_t        dual_led_setting;
  int                              lens_current_step;
  mesh_rolloff44_tintless_params_t tintless_params;
  isp_temporal_luxfilter_params_t  lux_filter;
  boolean                          dump_enabled;
  uint32_t                         dump_frames;
  uint32_t                         g_even_is_Gr;
  boolean                          module_streamon;
  ext_override_func               *ext_func_table;
  uint32_t                         metadump_enable;
  boolean                          prev_low_light_mode;
} mesh_rolloff44_t;

#if OVERRIDE_FUNC
boolean mesh_rolloff44_fill_func_table_ext(mesh_rolloff44_t *);
#define FILL_FUNC_TABLE(field) mesh_rolloff44_fill_func_table_ext(field)
#else
boolean mesh_rolloff44_fill_func_table(mesh_rolloff44_t *);
#define FILL_FUNC_TABLE(field) mesh_rolloff44_fill_func_table(field)
#endif

boolean mesh_rolloff44_init(isp_sub_module_t *isp_sub_module);

void mesh_rolloff44_destroy(isp_sub_module_t *isp_sub_module);

boolean mesh_rolloff44_query_cap(mct_module_t *module,
  void *query_buf);

boolean mesh_rolloff44_streamon(isp_sub_module_t *isp_sub_module, void *data);

boolean mesh_rolloff44_streamoff(isp_sub_module_t *isp_sub_module, void *data);

boolean mesh_rolloff44_set_chromatix_ptr(isp_sub_module_t *isp_sub_module,
  void *data);

boolean mesh_rolloff44_stats_aec_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean mesh_rolloff44_stats_awb_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean mesh_rolloff44_stats_af_update(isp_sub_module_t *isp_sub_module,
  void *data);
boolean mesh_rolloff44_get_abf_table_from_cct(float *mesh_table,
  isp_sub_module_t *isp_sub_module, mesh_rolloff44_t *mesh_rolloff);

boolean mesh_rolloff44_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean mesh_rolloff44_set_flash_mode(isp_sub_module_t *isp_sub_module,
  void *data);

boolean mesh_rolloff44_set_af_rolloff_params(isp_sub_module_t *isp_sub_module,
  void *data);

boolean mesh_rolloff44_set_stream_config(isp_sub_module_t *isp_sub_module,
  void *data);

boolean mesh_rolloff44_sensor_lens_position_update(
  isp_sub_module_t *isp_sub_module, void *data);

boolean mesh_rolloff44_update_min_stripe_overlap(
  isp_sub_module_t *isp_sub_module, void *data);

boolean mesh_rolloff44_set_stripe_info(
  isp_sub_module_t *isp_sub_module, void *data);

boolean mesh_rolloff44_set_split_info(isp_sub_module_t *isp_sub_module,
  void *data);

boolean mesh_rolloff44_set_tintless_table(isp_sub_module_t *isp_sub_module,
  void *data);

boolean mesh_rolloff44_set_parm_tintless(isp_sub_module_t *isp_sub_module,
  void *data);

boolean mesh_rolloff44_set_parm_hdr(isp_sub_module_t *isp_sub_module,
  mct_event_t *event);

boolean mesh_rolloff44_request_stats_type(isp_sub_module_t *isp_sub_module,
  void *data);

void mesh_rolloff44_table_interpolate(MESH_RollOffTable_V4 *in1,
  MESH_RollOffTable_V4 *in2, MESH_RollOffTable_V4 *out, float ratio);

void mesh_rolloff44_sensor_calc_tbl(mesh_rolloff_array_type *inTbl,
  mesh_rolloff_array_type *outTbl, sensor_rolloff_config_t *sensor_info);

void mesh_rolloff44_calc_flash_trigger(isp_sub_module_t *isp_sub_module,
  mesh_rolloff44_t *mod, MESH_RollOffTable_V4 *tblNormalLight,
  MESH_RollOffTable_V4 *tblOut, isp_rolloff_info_t *mesh_tbls);

void mesh_rolloff44_normalize_table(isp_sub_module_t *isp_sub_module,
  void *mesh_mod_in);

void mesh_rolloff44_calc_awb_trigger(isp_sub_module_t *isp_sub_module,
  void *data1, void *data2,
  isp_rolloff_info_t *mesh_tbls);

void mesh_rolloff44_calc_awb_trigger_lowLight(
  isp_sub_module_t *isp_sub_module, void *data1,
  void *data2, isp_rolloff_info_t *mesh_tbls);

boolean mesh_rolloff44_set_stream_config_overwrite(isp_sub_module_t *isp_sub_module);
#endif /* __MESH_ROLLOFF44_H__ */
