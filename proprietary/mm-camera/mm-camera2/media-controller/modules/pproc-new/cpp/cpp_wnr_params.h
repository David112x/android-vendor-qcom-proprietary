/**********************************************************************
*  Copyright (c) 2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __CPP_WNR_PARAMS_H__
#define __CPP_WNR_PARAMS_H__

#ifndef CHROMATIX_EXT
#include "cpp_hw_macro.h"
#else
#include "cpp_hw_macro_ext.h"
#endif

#include <math.h>
#include "mtype.h"
#include "chromatix_cpp.h"

/**
 *  Number of WNR planes
 */
#define CPP_DENOISE_NUM_PLANES 3

/**
 * Maximum mumber of RNR segments
 */
#define MAX_BF_RNR_SEGMENT 3

/**
 * Maximum mumber of RNR points
 */
#define MAX_BF_RADIAL_POINTS (MAX_BF_RNR_SEGMENT + 1)

/**
 * Maximum mumber of RNR segments
 */
#define MAX_BF_LNR_SEGMENT 32

/**
 * Maximum mumber of LNR points
 */
#define MAX_BF_LNR_POINTS (MAX_BF_LNR_SEGMENT + 1)

/**
 * Maximum mumber of unique CPP planes
 */
#define CPP_WNR_UNIQUE_PLANE_TYPE_CNT 2

/** pp_point_t:
 *
 *   @x: column number
 *   @y: row number
 *
 *   Represents a point/pixel
 **/
typedef struct {
  uint32_t x;
  uint32_t y;
} pp_point_t;

/** cpp_bf_rnr_bilat_adj_t:
 *
 *   @cf: correction factor
 *   @slope: slope of adjustment curve
 *   @cf_shift: cf shift factor
 *
 *   RNR curve adjustment
 **/
typedef struct {
  uint32_t cf;
  int32_t slope;
  uint32_t cf_shift;
} cpp_bf_rnr_bilat_adj_t;

/** cpp_bf_rnr_bilat_adj_t:
 *
 *   @r_sq_lut: LUT of radius square
 *   @bilat_scale_adj: bilateral scale adjustment curve
 *   @weight1_adj: thresholding weight adjustment curve
 *
 *   RNR segments information
 **/
typedef struct {
  uint32_t r_sq_lut;
  cpp_bf_rnr_bilat_adj_t bilat_scale_adj;
  cpp_bf_rnr_bilat_adj_t weight1_adj;
} cpp_bf_rnr_segment_t;

/** cpp_bf_rnr_bilat_t:
 *
 *   @rnr_wt_enable: flag to enable/disable rnr wt
 *   @rnr_enable: flag to enable/disable rnr
 *   @padding_mode: padding mode
 *   @trilat_filt_enable: flag to enable trilateral filter
 *
 *   RNR bilateral information
 **/
typedef struct {
  uint32_t rnr_wt_enable;
  uint32_t rnr_enable;
  uint32_t padding_mode;
  uint32_t trilat_filt_enable;
} cpp_bf_rnr_bilat_t;

/** cpp_wnr_rnr_plane_t:
 *
 *   @r_sq_shift: shift factor for r square
 *   @seg_info: segment information
 *   @bilat_info: bilateral block info for RNR
 *
 *   RNR plane information
 **/
typedef struct  {
  uint32_t r_sq_shift;
  cpp_bf_rnr_segment_t seg_info[MAX_BF_RNR_SEGMENT];
  cpp_bf_rnr_bilat_t bilat_info[BILITERAL_LAYERS];
} cpp_wnr_rnr_plane_t;

/** cpp_bf_lnr_curve_t:
 *
 *   @cf: correction factor
 *   @slope: slope of adjustment curve
 *
 *   LNR curve information
 **/
typedef struct {
  uint16_t cf;
  int16_t slope;
} cpp_bf_lnr_curve_t;

/** cpp_bf_lnr_region_t:
 *
 *   @bilat_scale: bilateral scale  curve
 *   @weight1: thresholding weight
 *
 *   LNR region information
 **/
typedef struct {
  cpp_bf_lnr_curve_t bilat_scale;
  cpp_bf_lnr_curve_t weight1;
} cpp_bf_lnr_region_t;

/** cpp_wnr_lnr_plane_t:
 *
 *   @lregion_info: region information
 *
 *   LNR plane information
 **/
typedef struct  {
  cpp_bf_lnr_region_t lregion_info[MAX_BF_LNR_SEGMENT];
} cpp_wnr_lnr_plane_t;

/** cpp_wnr_rnr_internal_seg_t:
 *
 *   @edgesoftness_adj_factor: adj factor for edge softness
 *   @weight_adj_factor: adj factor for weight thresholding
 *   @r_sq_tbl: square table per segment
 *
 *   Intermediate segment info
 **/
typedef struct {
  double edgesoftness_adj_factor;
  double weight_adj_factor;
  uint32_t r_sq_tbl;
} cpp_wnr_rnr_internal_seg_t;

/** cpp_wnr_rnr_internal_pln_t:
 *
 *   @scale_factor: scale factor for each plane w.r.t dimensions
 *   @center: center of the frame
 *   @r_sq_shift: radial square shift
 *   @r_sq: radial square
 *   @seg_info: segment information
 *
 *   Intermediate plane info
 **/
typedef struct {
  float scale_factor;
  pp_point_t center;
  int32_t r_sq_shift;
  double r_sq;
  cpp_wnr_rnr_internal_seg_t seg_info[MAX_BF_RNR_SEGMENT + 1];
} cpp_wnr_rnr_internal_pln_t;

/** cpp_wnr_internal_t:
 *
 *   @rnr_pln_int: RNR plane info
 *
 *   WNR internal structure
 **/
typedef struct {
  cpp_wnr_rnr_internal_pln_t rnr_pln_int[CPP_WNR_UNIQUE_PLANE_TYPE_CNT];
} cpp_wnr_internal_t;

/** cpp_wnr_plane_t:
 *
 *   @rnr: RNR plane information
 *   @lnr: LNR plane information
 *
 *   WNR plane information
 **/
typedef struct  {
  cpp_wnr_rnr_plane_t rnr;
  cpp_wnr_lnr_plane_t lnr;
} cpp_wnr_plane_t;

/** cpp_wnr_frame_info_t:
 *
 *   @wnr_pl_info: plane information
 *
 *   WNR frame information
 **/
typedef struct  {
  cpp_wnr_plane_t wnr_pl_info[CPP_DENOISE_NUM_PLANES];
} cpp_wnr_frame_info_t;

/** cpp_2d_wnr_adj_param:
 *
 *   @weight: computed weight interpolation output
 *   @edge_soft: edgesoftness interpolation output
 *
 *   WNR interpolated output information
 **/
typedef struct _cpp_2d_wnr_adj_param {
  float weight[WNR_LNR_POINTS];
  float edge_soft[WNR_LNR_POINTS];
} cpp_2d_wnr_adj_param;

/**
 * Function: cpp_wnr_params_rnr_update
 *
 * Description: This external function is used to
 *          calculate BF RNR params.
 *
 * Arguments:
 *   @p_params: CPP hardware params
 *   @p_noise_prof_start: Start pointer for HW noise profile
 *   @p_noise_prof_end: End pointer for HW wavelet profile
 *   @p_wnr_reserved: Pointer of WNR reserved field
 *   @int_ratio: Interpolation ratio
 *
 * Return values:
 *   CPP error values
 *
 * Notes: none
 **/
int32_t cpp_wnr_params_rnr_update(void *p_params,
  void *p_noise_prof_start,
  void *p_noise_prof_end,
  void *p_wnr_reserved,
  float int_ratio,
  void *pmodule_chromatix);

/**
 * Function: cpp_wnr_params_calc_lnr_params
 *
 * Description: This external function is used to
 *          calculate BF LNR params.
 *
 * Arguments:
 *   @p_hw_params: CPP hardware params
 *   @p_noise_prof_start: Start pointer for HW noise profile
 *   @p_noise_prof_end: End pointer for HW wavelet profile
 *   @int_ratio: interpolation ratio
 *
 * Return values:
 *   CPP error values
 *
 * Notes: none
 **/
int32_t cpp_wnr_params_lnr_update(void *p_params,
  void *p_noise_prof_start,
  void *p_noise_prof_end,
  float int_ratio,
  void *pmodule_chromatix);

/**
 * Function: cpp_wnr_params_dbg_chromatix_np
 *
 * Description: This function is used to dump chromatix params
 *
 * Arguments:
 *   @p_np: pointer to noise profile
 *   @deb_str: debug string
 *   @p_params: pointer to CPP hw params
 *
 * Return values:
 *   none
 *
 * Notes: updates only the luma
 **/
void cpp_wnr_params_dbg_chromatix_np(void *p_np __unused,
  const char *deb_str,
  void *p_params);

/**
 * Function: cpp_wnr_params_dbg_output
 *
 * Description: This function is used to dump chromatix params
 *
 * Arguments:
 *   @p_np: pointer to noise profile
 *   @deb_str: debug string
 *
 * Return values:
 *   none
 *
 * Notes: updates only the luma
 **/
void cpp_wnr_params_dbg_output(void *p_params);

#endif //__CPP_WNR_PARAMS_H__

