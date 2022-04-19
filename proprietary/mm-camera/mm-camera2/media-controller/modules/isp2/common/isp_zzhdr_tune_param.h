/* isp_zzhdr_tune_params.h
 *
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

(isp_zzhdr_2d_lut_type){
  .zzhdr_v2_enable = 0,
  .abf_2d_data = (abf_gic_demosaic_2d_adj_type){
    .abf_adj_enable = 0,
    .gic_adj_enable = 0,
    .demosaic_adj_enable = 0,
    .exp_control = 0,
    .hdr_exp_control = 0,
    .abf_gic_demosaic_rgn_data = {
       //SET0
       (abf_gic_demosaic_2d_rgn_type){
         .trigger = (trigger_point_type){
           .gain_start = 1.0f,
           .gain_end = 2.5f,
           .lux_index_start = 200,
           .lux_index_end = 220,
         },
         .abf_gic_demosaic_exp_data = {
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 1.0f,
                .end = 2.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 1.0f,
                .end = 2.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 2.0f,
                .end = 3.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 2.0f,
                .end = 3.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 3.0f,
                .end = 4.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 3.0f,
                .end = 4.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 4.0f,
                .end = 5.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 4.0f,
                .end = 5.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 5.0f,
                .end = 6.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 5.0f,
                .end = 6.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 6.0f,
                .end = 7.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 6.0f,
                .end = 7.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
         },
       },
       //SET1
       (abf_gic_demosaic_2d_rgn_type){
         .trigger = (trigger_point_type){
           .gain_start = 5.0f,
           .gain_end = 6.5f,
           .lux_index_start = 280,
           .lux_index_end = 300,
         },
         .abf_gic_demosaic_exp_data = {
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 1.0f,
                .end = 2.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 1.0f,
                .end = 2.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 2.0f,
                .end = 3.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 2.0f,
                .end = 3.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 3.0f,
                .end = 4.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 3.0f,
                .end = 4.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 4.0f,
                .end = 5.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 4.0f,
                .end = 5.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 5.0f,
                .end = 6.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 5.0f,
                .end = 6.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 6.0f,
                .end = 7.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 6.0f,
                .end = 7.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
         },
       },
       //SET2
       (abf_gic_demosaic_2d_rgn_type){
         .trigger = (trigger_point_type){
           .gain_start = 10.0f,
           .gain_end = 11.5f,
           .lux_index_start = 360,
           .lux_index_end = 380,
         },
         .abf_gic_demosaic_exp_data = {
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 1.0f,
                .end = 2.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 1.0f,
                .end = 2.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 2.0f,
                .end = 3.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 2.0f,
                .end = 3.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 3.0f,
                .end = 4.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 3.0f,
                .end = 4.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 4.0f,
                .end = 5.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 4.0f,
                .end = 5.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 5.0f,
                .end = 6.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 5.0f,
                .end = 6.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 6.0f,
                .end = 7.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 6.0f,
                .end = 7.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
         },
       },
       //SET3
       (abf_gic_demosaic_2d_rgn_type){
         .trigger = (trigger_point_type){
           .gain_start = 12.0f,
           .gain_end = 13.0f,
           .lux_index_start = 390,
           .lux_index_end = 400,
         },
         .abf_gic_demosaic_exp_data = {
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 1.0f,
                .end = 2.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 1.0f,
                .end = 2.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 2.0f,
                .end = 3.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 2.0f,
                .end = 3.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 3.0f,
                .end = 4.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 3.0f,
                .end = 4.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 4.0f,
                .end = 5.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 4.0f,
                .end = 5.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 5.0f,
                .end = 6.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 5.0f,
                .end = 6.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 6.0f,
                .end = 7.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 6.0f,
                .end = 7.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
         },
       },
       //SET4
       (abf_gic_demosaic_2d_rgn_type){
         .trigger = (trigger_point_type){
           .gain_start = 14.0f,
           .gain_end = 15.0f,
           .lux_index_start = 410,
           .lux_index_end = 420,
         },
         .abf_gic_demosaic_exp_data = {
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 1.0f,
                .end = 2.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 1.0f,
                .end = 2.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 2.0f,
                .end = 3.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 2.0f,
                .end = 3.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 3.0f,
                .end = 4.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 3.0f,
                .end = 4.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 4.0f,
                .end = 5.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 4.0f,
                .end = 5.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 5.0f,
                .end = 6.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 5.0f,
                .end = 6.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 6.0f,
                .end = 7.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 6.0f,
                .end = 7.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
         },
       },
       //SET5
       (abf_gic_demosaic_2d_rgn_type){
         .trigger = (trigger_point_type){
           .gain_start = 16.0f,
           .gain_end = 17.0f,
           .lux_index_start = 430,
           .lux_index_end = 440,
         },
         .abf_gic_demosaic_exp_data = {
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 1.0f,
                .end = 2.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 1.0f,
                .end = 2.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 2.0f,
                .end = 3.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 2.0f,
                .end = 3.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 3.0f,
                .end = 4.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 3.0f,
                .end = 4.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 4.0f,
                .end = 5.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 4.0f,
                .end = 5.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 5.0f,
                .end = 6.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 5.0f,
                .end = 6.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
             },
           },
            (abf_gic_demosaic_2d_exp_type){
              .aec_sensitivity_ratio = (trigger_point2_type){
                .start = 6.0f,
                .end = 7.0f,
              },
              .exp_time_trigger = (trigger_point2_type){
                .start = 6.0f,
                .end = 7.0f,
              },
              .abf_gic_demosaic_2d_reserve_data = (abf_gic_demosaic_2d_reserve){
                 .abf_noise_profile_adjust_ratio = 16.0f,
                 .abf_HDR_thr_1 = 60,
                 .abf_HDR_thr_2 = 64,
                 .abf_noise_offset = 1024.0f,
                 .abf_bpc_offset_adj = 0.0f,
                 .abf_Fmax_adj = 1.0f,
                 .abf_Fmin_adj = 1.0f,
                 .bpc_maxshft = 0,
                 .bpc_minshft = 0,
                 .gic_ds_adj = 0.0f,
                 .gic_fs_adj = 0.0f,
                 .gic_thin_line_noise_offset = 0,
                 .demosaic_ak =0,
            },
          },
        },
      },
    },
  },
  .bpc_2d_data = (bpc_pdpc_2d_adj_type) {
    .bpc_adj_enable = 0,
    .pdpc_adj_enable = 0,
    .exp_control = 0,
    .hdr_exp_control = 0,
    .bpc_pdpc_hdr_rgn_data = {
      //SET0
      (bpc_pdpc_hdr_2d_rgn_type){
        .trigger = (trigger_point_type) {
          .gain_start = 1.0f,
          .gain_end = 1.1f,
          .lux_index_start = 200,
          .lux_index_end = 220,
        },
        .bpc_pdpc_hdr_exp_data = {
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 1.0f,
              .end = 2.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 1.0f,
              .end = 2.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 2.0f,
              .end = 3.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 2.0f,
              .end = 3.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 3.0f,
              .end = 4.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 3.0f,
              .end = 4.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 4.0f,
              .end = 5.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 4.0f,
              .end = 5.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 5.0f,
              .end = 6.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 5.0f,
              .end = 6.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 6.0f,
              .end = 7.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 6.0f,
              .end = 7.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
        },
      },
      //SET1
      (bpc_pdpc_hdr_2d_rgn_type){
        .trigger = (trigger_point_type) {
          .gain_start = 2.5f,
          .gain_end = 3.0f,
          .lux_index_start = 280,
          .lux_index_end = 300,
        },
        .bpc_pdpc_hdr_exp_data = {
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 1.0f,
              .end = 2.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 1.0f,
              .end = 2.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 2.0f,
              .end = 3.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 2.0f,
              .end = 3.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 3.0f,
              .end = 4.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 3.0f,
              .end = 4.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 4.0f,
              .end = 5.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 4.0f,
              .end = 5.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 5.0f,
              .end = 6.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 5.0f,
              .end = 6.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 6.0f,
              .end = 7.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 6.0f,
              .end = 7.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
        },
      },
      //SET2
      (bpc_pdpc_hdr_2d_rgn_type){
        .trigger = (trigger_point_type) {
          .gain_start = 4.2f,
          .gain_end = 4.3f,
          .lux_index_start = 360,
          .lux_index_end = 380,
        },
        .bpc_pdpc_hdr_exp_data = {
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 1.0f,
              .end = 2.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 1.0f,
              .end = 2.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 2.0f,
              .end = 3.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 2.0f,
              .end = 3.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 3.0f,
              .end = 4.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 3.0f,
              .end = 4.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 4.0f,
              .end = 5.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 4.0f,
              .end = 5.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 5.0f,
              .end = 6.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 5.0f,
              .end = 6.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 6.0f,
              .end = 7.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 6.0f,
              .end = 7.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
        },
      },
      //SET3
      (bpc_pdpc_hdr_2d_rgn_type){
        .trigger = (trigger_point_type) {
          .gain_start = 8.9f,
          .gain_end = 9.1f,
          .lux_index_start = 390,
          .lux_index_end = 400,
        },
        .bpc_pdpc_hdr_exp_data = {
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 1.0f,
              .end = 2.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 1.0f,
              .end = 2.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 2.0f,
              .end = 3.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 2.0f,
              .end = 3.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 3.0f,
              .end = 4.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 3.0f,
              .end = 4.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 4.0f,
              .end = 5.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 4.0f,
              .end = 5.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 5.0f,
              .end = 6.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 5.0f,
              .end = 6.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 6.0f,
              .end = 7.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 6.0f,
              .end = 7.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
        },
      },
      //SET4
      (bpc_pdpc_hdr_2d_rgn_type){
        .trigger = (trigger_point_type) {
          .gain_start = 11.0f,
          .gain_end = 11.5f,
          .lux_index_start = 410,
          .lux_index_end = 420,
        },
        .bpc_pdpc_hdr_exp_data = {
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 1.0f,
              .end = 2.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 1.0f,
              .end = 2.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 2.0f,
              .end = 3.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 2.0f,
              .end = 3.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 3.0f,
              .end = 4.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 3.0f,
              .end = 4.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 4.0f,
              .end = 5.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 4.0f,
              .end = 5.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 5.0f,
              .end = 6.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 5.0f,
              .end = 6.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 6.0f,
              .end = 7.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 6.0f,
              .end = 7.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
        },
      },
      //SET5
      (bpc_pdpc_hdr_2d_rgn_type){
        .trigger = (trigger_point_type) {
          .gain_start = 12.1f,
          .gain_end = 12.7f,
          .lux_index_start = 430,
          .lux_index_end = 440,
        },
        .bpc_pdpc_hdr_exp_data = {
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 1.0f,
              .end = 2.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 1.0f,
              .end = 2.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 2.0f,
              .end = 3.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 2.0f,
              .end = 3.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 3.0f,
              .end = 4.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 3.0f,
              .end = 4.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 4.0f,
              .end = 5.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 4.0f,
              .end = 5.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 5.0f,
              .end = 6.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 5.0f,
              .end = 6.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 6.0f,
              .end = 7.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 6.0f,
              .end = 7.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 1.0f,
               .bpc_Fmax_adj = 1.0f,
               .pdpc_bp_offset_G_adj = 0.0f,
               .pdpc_bp_offset_rb_adj = 0.0f,
               .pdpc_Fmax_adj = 0.0f,
            },
          },
        },
      },
    },
  },
  .pdpc_2d_data = (bpc_pdpc_2d_adj_type) {
    .bpc_adj_enable = 0,
    .pdpc_adj_enable = 0,
    .exp_control = 0,
    .hdr_exp_control = 0,
    .bpc_pdpc_hdr_rgn_data = {
      //SET0
      (bpc_pdpc_hdr_2d_rgn_type){
        .trigger = (trigger_point_type) {
          .gain_start = 1.0f,
          .gain_end = 1.1f,
          .lux_index_start = 200,
          .lux_index_end = 220,
        },
        .bpc_pdpc_hdr_exp_data = {
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 1.0f,
              .end = 2.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 1.0f,
              .end = 2.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 1.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 2.0f,
              .end = 3.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 2.0f,
              .end = 3.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 2.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 3.0f,
              .end = 4.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 3.0f,
              .end = 4.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 3.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 4.0f,
              .end = 5.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 4.0f,
              .end = 5.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 4.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 5.0f,
              .end = 6.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 5.0f,
              .end = 6.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 5.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 6.0f,
              .end = 7.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 6.0f,
              .end = 7.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 6.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
        },
      },
      //SET1
      (bpc_pdpc_hdr_2d_rgn_type){
        .trigger = (trigger_point_type) {
          .gain_start = 2.5f,
          .gain_end = 3.0f,
          .lux_index_start = 280,
          .lux_index_end = 300,
        },
        .bpc_pdpc_hdr_exp_data = {
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 1.0f,
              .end = 2.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 1.0f,
              .end = 2.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 1.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 2.0f,
              .end = 3.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 2.0f,
              .end = 3.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 2.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 3.0f,
              .end = 4.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 3.0f,
              .end = 4.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 3.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 4.0f,
              .end = 5.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 4.0f,
              .end = 5.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 4.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 5.0f,
              .end = 6.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 5.0f,
              .end = 6.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 5.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 6.0f,
              .end = 7.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 6.0f,
              .end = 7.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 6.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
        },
      },
      //SET2
      (bpc_pdpc_hdr_2d_rgn_type){
        .trigger = (trigger_point_type) {
          .gain_start = 4.2f,
          .gain_end = 4.3f,
          .lux_index_start = 360,
          .lux_index_end = 380,
        },
        .bpc_pdpc_hdr_exp_data = {
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 1.0f,
              .end = 2.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 1.0f,
              .end = 2.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 1.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 2.0f,
              .end = 3.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 2.0f,
              .end = 3.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 2.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 3.0f,
              .end = 4.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 3.0f,
              .end = 4.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 3.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 4.0f,
              .end = 5.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 4.0f,
              .end = 5.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 4.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 5.0f,
              .end = 6.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 5.0f,
              .end = 6.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 5.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 6.0f,
              .end = 7.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 6.0f,
              .end = 7.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 6.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
        },
      },
      //SET3
      (bpc_pdpc_hdr_2d_rgn_type){
        .trigger = (trigger_point_type) {
          .gain_start = 8.9f,
          .gain_end = 9.1f,
          .lux_index_start = 390,
          .lux_index_end = 400,
        },
        .bpc_pdpc_hdr_exp_data = {
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 1.0f,
              .end = 2.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 1.0f,
              .end = 2.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 1.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 2.0f,
              .end = 3.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 2.0f,
              .end = 3.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 2.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 3.0f,
              .end = 4.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 3.0f,
              .end = 4.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 3.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 4.0f,
              .end = 5.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 4.0f,
              .end = 5.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 4.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 5.0f,
              .end = 6.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 5.0f,
              .end = 6.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 5.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 6.0f,
              .end = 7.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 6.0f,
              .end = 7.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 6.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
        },
      },
      //SET4
      (bpc_pdpc_hdr_2d_rgn_type){
        .trigger = (trigger_point_type) {
          .gain_start = 11.0f,
          .gain_end = 11.5f,
          .lux_index_start = 410,
          .lux_index_end = 420,
        },
        .bpc_pdpc_hdr_exp_data = {
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 1.0f,
              .end = 2.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 1.0f,
              .end = 2.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 1.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 2.0f,
              .end = 3.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 2.0f,
              .end = 3.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 2.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 3.0f,
              .end = 4.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 3.0f,
              .end = 4.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 3.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 4.0f,
              .end = 5.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 4.0f,
              .end = 5.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 4.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 5.0f,
              .end = 6.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 5.0f,
              .end = 6.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 5.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 6.0f,
              .end = 7.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 6.0f,
              .end = 7.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 6.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
        },
      },
       //SET5
      (bpc_pdpc_hdr_2d_rgn_type){
        .trigger = (trigger_point_type) {
          .gain_start = 12.1f,
          .gain_end = 12.7f,
          .lux_index_start = 430,
          .lux_index_end = 440,
        },
        .bpc_pdpc_hdr_exp_data = {
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 1.0f,
              .end = 2.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 1.0f,
              .end = 2.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 1.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 2.0f,
              .end = 3.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 2.0f,
              .end = 3.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 2.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 3.0f,
              .end = 4.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 3.0f,
              .end = 4.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 3.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 4.0f,
              .end = 5.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 4.0f,
              .end = 5.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 4.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 5.0f,
              .end = 6.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 5.0f,
              .end = 6.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 5.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
          (bpc_pdpc_hdr_2d_exp_type){
            .aec_sensitivity_ratio = (trigger_point2_type){
              .start = 6.0f,
              .end = 7.0f,
            },
            .exp_time_trigger = (trigger_point2_type){
              .start = 6.0f,
              .end = 7.0f,
            },
            .bpc_pdpc_hdr_2d_reserve_data = (bpc_pdpc_hdr_2d_reserve){
               .bpc_offset_adj = 0.0f,
               .bpc_Fmax_adj = 0.0f,
               .pdpc_bp_offset_G_adj = 6.0f,
               .pdpc_bp_offset_rb_adj = 1.0f,
               .pdpc_Fmax_adj = 1.0f,
            },
          },
        },
      },
    },
  },
}
