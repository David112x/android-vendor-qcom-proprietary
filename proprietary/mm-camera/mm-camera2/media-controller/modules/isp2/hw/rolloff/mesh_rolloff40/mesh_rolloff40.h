/*
* Copyright (c) 2012-2015 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef __MESH_ROLLOFF40_H__
#define __MESH_ROLLOFF40_H__

/* mctl headers */
#include "chromatix.h"
#include "chromatix_common.h"

/* isp headers */
#include "mesh_rolloff_reg.h"
#include "isp_sub_module_common.h"


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
        out[i] = LINEAR_INTERPOLATION(in1[i], in2[i], ratio); })

#define CHROMATIX_MESH_ROLL_NUM_COL 13
#define CHROMATIX_MESH_ROLL_NUM_ROW 17
#define CHROMATIX_MESH_TABLE_SIZE  (17 * 13)

#define HW_MESH_ROLL_NUM_COL 13
#define HW_MESH_ROLL_NUM_ROW 10

#define HW_MESH_SCALE_ROLL_NUM_COL 17
#define HW_MESH_SCALE_ROLL_NUM_ROW 13


#define MIN_BICUBIC_H_GRID_NUM 8
#define MIN_BICUBIC_V_GRID_NUM 6

#define MESH_ROLLOFF_HORIZONTAL_GRIDS_MAX  12
#define MESH_ROLLOFF_VERTICAL_GRIDS_MAX    9
#define MESH_ROLL_OFF_V4_TABLE_SIZE  (13 * 10)
#define MESH_ROLLOFF_SIZE (17 * 13)

#define MESH_ROLL_OFF_V4_EXTEND_MESH_SIZE ((12+3) * (9+3))
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

/** MESH_RollOff_V4_ConfigCmdType:
 *
 *  @dmi_set_gr_r: dmi set Gr
 *  @Table_GR_R: Gr table
 *  @Table_GB_B: Gb able
 *  @dmi_reset_gr_r: dmi rset Gr
 *  @CfgParams: cfg params
 **/
typedef struct MESH_RollOff_V4_ConfigCmdType {
  uint32_t                     dmi_set_gr_r[2];
  MESH_RollOff_V4_ConfigTable  Table_GR_R;
  MESH_RollOff_V4_ConfigTable  Table_GB_B;
  uint32_t                     dmi_reset_gr_r[2];
  MESH_RollOff_v4_ConfigParams CfgParams;
} MESH_RollOff_V4_ConfigCmdType;

/** mesh_rolloff_V4_params_t:
 *
 *  @input_table: input table
 **/
typedef struct {
  MESH_RollOffTable_V4 input_table;
} mesh_rolloff_V4_params_t;

/** mesh_rolloff40_tintless_params_t:
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
  MESH_RollOffTable_V4    last_non_flash_tbl;
  MESH_RollOffTable_V4    tableout;
  float                   lowlight_adjust[MESH_ROLLOFF_SIZE];
  float                   current_adjust[MESH_ROLLOFF_SIZE];
  boolean                 update_input_table;
} mesh_rolloff40_tintless_params_t;

typedef void (*calc_interpolation)(isp_sub_module_t *isp_sub_module,
  void *mesh_rolloff44, isp_rolloff_info_t *mesh_tbls,
  MESH_RollOffTable_V4 *tblOut);
typedef void (*normalize)(isp_sub_module_t *isp_sub_module,
  void *mesh_mod_in);

typedef struct {
  normalize          normalize;
  calc_interpolation calc_interpolation;
} ext_override_func;

/** mesh_rolloff40_t:
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
 **/
typedef struct {
  float                         cur_mired_color_temp;
  cam_flash_mode_t              cur_flash_mode;
  MESH_RollOff_V4_ConfigCmdType mesh_rolloff_cmd;
  mesh_rolloff_V4_params_t      mesh_rolloff_param;
  isp_rolloff_tableset_t        rolloff_calibration_table;
  isp_rolloff_tableset_t        rolloff_tbls;
  MESH_RollOffTable_V4          applied_table;
  MESH_RollOff_V4_ConfigTable   applied_hw_table_gr_r;
  MESH_RollOff_V4_ConfigTable   applied_hw_table_gb_b;
  uint16_t                      af_macro;
  uint16_t                      af_infinity;
  sensor_out_info_t             sensor_out_info;
  af_rolloff_info_t             af_rolloff_info;
  chromatix_VFE_common_type     af_rolloff_chromatix;
  boolean                       af_rolloff_info_valid;
  ispif_out_info_t              ispif_out_info;
  isp_out_info_t                isp_out_info;
  aec_update_t                  aec_update;
  camera_flash_type             flash_type;
  stats_dual_led_settings_t     dual_led_setting;
  int                           lens_current_step;
  mesh_rolloff40_tintless_params_t tintless_params;
  boolean                          dump_enabled;
  uint32_t                         dump_frames;
  uint32_t                         g_even_is_Gr;
  boolean                          module_streamon;
  uint32_t                         metadump_enable;
  boolean                          prev_low_light_mode;
  MESH_RollOffTable_V4             led_rolloff_stored;
  ext_override_func               *ext_func_table;
} mesh_rolloff40_t;

#if OVERRIDE_FUNC
boolean mesh_rolloff40_fill_func_table_ext(mesh_rolloff40_t *);
#define FILL_FUNC_TABLE(field) mesh_rolloff40_fill_func_table_ext(field)
#else
boolean mesh_rolloff40_fill_func_table(mesh_rolloff40_t *);
#define FILL_FUNC_TABLE(field) mesh_rolloff40_fill_func_table(field)
#endif

boolean mesh_rolloff40_init(isp_sub_module_t *isp_sub_module);

void mesh_rolloff40_destroy(isp_sub_module_t *isp_sub_module);

boolean mesh_rolloff40_query_cap(mct_module_t *module,
  void *query_buf);

boolean mesh_rolloff40_streamon(isp_sub_module_t *isp_sub_module, void *data);

boolean mesh_rolloff40_streamoff(isp_sub_module_t *isp_sub_module, void *data);

boolean mesh_rolloff40_set_chromatix_ptr(isp_sub_module_t *isp_sub_module,
  void *data);

boolean mesh_rolloff40_stats_aec_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean mesh_rolloff40_stats_awb_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean mesh_rolloff40_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean mesh_rolloff40_set_flash_mode(isp_sub_module_t *isp_sub_module,
  void *data);

boolean mesh_rolloff40_set_af_rolloff_params(isp_sub_module_t *isp_sub_module,
  void *data);

boolean mesh_rolloff40_set_stream_config(isp_sub_module_t *isp_sub_module,
  void *data);

boolean mesh_rolloff40_set_flash_mode(isp_sub_module_t *isp_sub_module,
  void *data);

boolean mesh_rolloff40_sensor_lens_position_update(
  isp_sub_module_t *isp_sub_module, void *data);

boolean mesh_rolloff40_update_min_stripe_overlap(
  isp_sub_module_t *isp_sub_module, void *data);

boolean mesh_rolloff40_set_stripe_info(
  isp_sub_module_t *isp_sub_module, void *data);

boolean mesh_rolloff40_set_split_info(isp_sub_module_t *isp_sub_module,
  void *data);

boolean mesh_rolloff40_set_tintless_table(isp_sub_module_t *isp_sub_module,
  void *data);

boolean mesh_rolloff40_set_parm_tintless(isp_sub_module_t *isp_sub_module,
  void *data);

boolean mesh_rolloff40_set_parm_hdr(isp_sub_module_t *isp_sub_module,
  mct_event_t *event);

boolean mesh_rolloff40_request_stats_type(isp_sub_module_t *isp_sub_module,
  void *data);

void mesh_rolloff40_table_interpolate(MESH_RollOffTable_V4 *in1,
  MESH_RollOffTable_V4 *in2, MESH_RollOffTable_V4 *out, float ratio);

void mesh_rolloff40_sensor_calc_tbl(mesh_rolloff_array_type *inTbl,
  mesh_rolloff_array_type *outTbl, sensor_rolloff_config_t *sensor_info);

void mesh_rolloff40_calc_awb_trigger_lowLight(
  isp_sub_module_t *isp_sub_module, mesh_rolloff40_t *mod,
  MESH_RollOffTable_V4 *tblOut, isp_rolloff_info_t *mesh_tbls);

void mesh_rolloff40_calc_awb_trigger(isp_sub_module_t *isp_sub_module,
  mesh_rolloff40_t *mod, MESH_RollOffTable_V4 *tblOut,
  isp_rolloff_info_t *mesh_tbls);

void mesh_rolloff40_downscale_rolloff_table (mesh_rolloff_array_type *tableIn,
  MESH_RollOffTable_V4 *tableOut);

boolean mesh_rolloff40_set_stream_config_overwrite(isp_sub_module_t *isp_sub_module);

void mesh_rolloff40_calc_flash_trigger(isp_sub_module_t *isp_sub_module,
  mesh_rolloff40_t *mod, MESH_RollOffTable_V4 *tblNormalLight,
  MESH_RollOffTable_V4 *tblOut, isp_rolloff_info_t *mesh_tbls);

void mesh_rolloff40_normalize_table(isp_sub_module_t *isp_sub_module,
  void *mesh_in_mod);
#endif /* __MESH_ROLLOFF40_H__ */
