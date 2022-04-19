/* isp_zzhdr_tune_def.h
 *
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
#ifndef __ISP_ZZHDR_TUNE_DEFS_H__
#define __ISP_ZZHDR_TUNE_DEFS_H__

#ifndef MAX_SETS_FOR_2D_EXP_ADJ
#define MAX_SETS_FOR_2D_EXP_ADJ                6
#endif

typedef struct
{
    float                                   abf_noise_profile_adjust_ratio;
    unsigned int                            abf_HDR_thr_1;
    unsigned int                            abf_HDR_thr_2;
    float                                   abf_noise_offset;
    float                                   abf_bpc_offset_adj;
    float                                   abf_Fmax_adj;
    float                                   abf_Fmin_adj;
    unsigned short                          bpc_maxshft;
    unsigned short                          bpc_minshft;
    float                                   gic_ds_adj;
    float                                   gic_fs_adj;
    unsigned int                            gic_thin_line_noise_offset;
    unsigned int                            demosaic_ak;
} abf_gic_demosaic_2d_reserve;

typedef struct
{
    trigger_point2_type                     aec_sensitivity_ratio;
    trigger_point2_type                     exp_time_trigger;
    abf_gic_demosaic_2d_reserve             abf_gic_demosaic_2d_reserve_data;
} abf_gic_demosaic_2d_exp_type;

typedef struct
{
    trigger_point_type                      trigger;
    abf_gic_demosaic_2d_exp_type            abf_gic_demosaic_exp_data[MAX_SETS_FOR_TONE_NOISE_ADJ];
} abf_gic_demosaic_2d_rgn_type;

typedef struct
{
    int                                     abf_adj_enable;
    int                                     gic_adj_enable;
    int                                     demosaic_adj_enable;
    tuning_control_type                     exp_control;
    tuning_control_type                     hdr_exp_control;
    abf_gic_demosaic_2d_rgn_type            abf_gic_demosaic_rgn_data[MAX_SETS_FOR_2D_EXP_ADJ];
} abf_gic_demosaic_2d_adj_type;


typedef struct
{
    float                                   bpc_offset_adj;
    float                                   bpc_Fmax_adj;
    float                                   pdpc_bp_offset_G_adj;
    float                                   pdpc_bp_offset_rb_adj;
    float                                   pdpc_Fmax_adj;
} bpc_pdpc_hdr_2d_reserve;

typedef struct
{
    trigger_point2_type                     aec_sensitivity_ratio;
    trigger_point2_type                     exp_time_trigger;
    bpc_pdpc_hdr_2d_reserve                 bpc_pdpc_hdr_2d_reserve_data;
} bpc_pdpc_hdr_2d_exp_type;

typedef struct
{
    trigger_point_type                      trigger;
    bpc_pdpc_hdr_2d_exp_type                bpc_pdpc_hdr_exp_data[MAX_SETS_FOR_TONE_NOISE_ADJ];
} bpc_pdpc_hdr_2d_rgn_type;

typedef struct
{
    int                                     bpc_adj_enable;
    int                                     pdpc_adj_enable;
    tuning_control_type                     exp_control;
    tuning_control_type                     hdr_exp_control;
    bpc_pdpc_hdr_2d_rgn_type                bpc_pdpc_hdr_rgn_data[MAX_SETS_FOR_2D_EXP_ADJ];
} bpc_pdpc_2d_adj_type;

typedef struct
{
  int zzhdr_v2_enable;
  abf_gic_demosaic_2d_adj_type abf_2d_data;
  bpc_pdpc_2d_adj_type         bpc_2d_data;
  bpc_pdpc_2d_adj_type         pdpc_2d_data;
} isp_zzhdr_2d_lut_type;

#endif // __ISP_ZZHDR_TUNE_DEFS_H__
