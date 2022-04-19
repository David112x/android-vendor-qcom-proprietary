/* sce40.h
 *
 * Copyright (c) 2014, 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __SCE40_H__
#define __SCE40_H__

/* mctl headers */
#include "chromatix.h"
#include "chromatix_common.h"

/* isp headers */
#include "sce_reg.h"
#include "isp_sub_module_common.h"

typedef struct {
  int x;
  int y;
} sce_coor_t;

typedef struct {
  double a,b,c,d,e,f;
} sce_affine_transform_2d_double;

typedef struct {
  float cr;
  float cb;
} cr_cb_point_float;

typedef struct {
  cr_cb_point_float point1;
  cr_cb_point_float point2;
  cr_cb_point_float point3;
} cr_cb_triangle_float;

typedef struct {
  cr_cb_triangle_float triangle1;
  cr_cb_triangle_float triangle2;
  cr_cb_triangle_float triangle3;
  cr_cb_triangle_float triangle4;
  cr_cb_triangle_float triangle5;
} sce_cr_cb_triangle_set_float;

/** ISP_Skin_enhan_line
 *    @point0: origin point
 *    @shift_cr: cb axis coefficient
 *    @shift_cb: cr axis coefficient
 *
 * parametric and equation of the SCE triangels common point
 * interpolation line:
 * cr = cr0 + shift_cr * t
 * cb = cb0 + shift_cb * t
 **/
typedef struct ISP_Skin_enhan_line {
  cr_cb_point_float point0;
  double shift_cr;
  double shift_cb;
} ISP_Skin_enhan_line;

/** ISP_Skin_enhan_line
 *    @interpolation_line: line along which the common vertex is shifted
 *    @shift_cr: cb axis boundary
 *    @shift_cb: cr axis boundary
 *
 * Range in which the common triangles vertex can be shifted defined by a line
 * and boundaries
 **/
typedef struct ISP_Skin_enhan_range {
  ISP_Skin_enhan_line interpolation_line;
  double pos_step;
  double neg_step;
}ISP_Skin_enhan_range;

typedef struct {
  ISP_Skin_enhan_ConfigCmdType sce_cmd;
  sce_cr_cb_triangle_set_float origin_triangles_A;
  sce_cr_cb_triangle_set_float origin_triangles_D65;
  sce_cr_cb_triangle_set_float origin_triangles_TL84;
  sce_cr_cb_triangle_set_float origin_triangles_H;
  sce_cr_cb_triangle_set_float origin_triangles_outdoor;
  sce_cr_cb_triangle_set_float destination_triangles_H;
  sce_cr_cb_triangle_set_float destination_triangles_A;
  sce_cr_cb_triangle_set_float destination_triangles_D65;
  sce_cr_cb_triangle_set_float destination_triangles_TL84;
  sce_cr_cb_triangle_set_float destination_triangles_outdoor;
  sce_cr_cb_triangle_set_float orig;
  sce_cr_cb_triangle_set_float dest;

  sce_shift_vector interp_vector;
  ISP_Skin_enhan_range interp_range;
  double sce_adjust_factor;
  double prev_sce_adj;
  cct_trigger_info trigger_info;
  aec_update_t aec_update;
  awb_cct_type prev_cct_type;
  awb_cct_type cur_cct_type;
  float prev_aec_ratio;
  trigger_lighting_t prev_lighting;
  float prev_color_drc_gain;
  float portrait_severity;
  float skin_color_boost_factor;
  boolean is_dest_match_orig;
  boolean enable_adrc;
} sce40_t;

boolean sce40_init(isp_sub_module_t *isp_sub_module);
void sce40_destroy(isp_sub_module_t *isp_sub_module);
boolean sce40_set_chromatix_ptr(isp_sub_module_t *isp_sub_module, void *data);
boolean sce40_set_sce_factor(isp_sub_module_t *isp_sub_module, void *data);
boolean sce40_streamoff(isp_sub_module_t *isp_sub_module, void *data);
boolean sce40_stats_aec_update(isp_sub_module_t *isp_sub_module, void *data);
boolean sce40_stats_awb_update(isp_sub_module_t *isp_sub_module, void *data);
boolean sce40_stats_asd_update(isp_sub_module_t *isp_sub_module, void *data);
boolean sce40_trigger_update(isp_sub_module_t *isp_sub_module, void *data);
void trigger_interpolate_sce_triangles(sce_cr_cb_triangle_set_float* ip1,
  sce_cr_cb_triangle_set_float* ip2, sce_cr_cb_triangle_set_float* op, float ratio);
void trigger_interpolate_sce_triangles_int(sce_cr_cb_triangle_set* ip1,
  sce_cr_cb_triangle_set* ip2, sce_cr_cb_triangle_set_float* op, float ratio);
void trigger_interpolate_sce_vectors(sce_shift_vector* ip1,
  sce_shift_vector* ip2, sce_shift_vector* op, float ratio);
void sce_copy_triangles_from_chromatix(sce40_t *sce,
  chromatix_SCE_type *p_sce);
boolean trigger_sce_get_triangles(sce40_t *sce,
  chromatix_SCE_type *p_sce, awb_cct_type cct_type);
boolean sce40_adjust_dest_triangle_by_aec(sce40_t *sce, float aec_ratio);
void sce_copy_triangle_float(sce_cr_cb_triangle_set *int_triangle,
  sce_cr_cb_triangle_set_float *float_triangle);
boolean sce40_select_table(isp_sub_module_t *isp_sub_module,
  void *data);
void sce40_copy_triangles_from_chromatix(void *data,
  chromatix_SCE_type *p_sce);

#endif /* __SCE40_H__ */
