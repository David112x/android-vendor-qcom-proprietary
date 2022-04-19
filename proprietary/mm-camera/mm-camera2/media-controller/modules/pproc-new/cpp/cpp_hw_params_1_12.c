/*============================================================================

  Copyright (c) 2016 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/

#include "cam_types.h"
#include "mtype.h"
#include "modules.h"
#include <media/msmb_camera.h>
#include <media/msmb_pproc.h>
#include <math.h>
#include "cpp_log.h"
#include "eztune_diagnostics.h"
#include "cpp_hw_params.h"
#include "chromatix.h"


#define CPP_FW_CMD_HEADER           0x3E646D63
#define CPP_FW_CMD_TRAILER          0xabcdefaa
#define CPP_FW_CMD_ID_PROCESS_FRAME 0x6
#define MMU_PREFETCH_ENABLE 1

/*q factor used in CPP*/
static const int q_factor = 21;

/*mask to get the fractional bits*/
static const int q_mask = (1 << 21) - 1;

/*Number of extra pixels needed on the left or stop for upscaler*/
static const int upscale_left_top_padding = 1;

/*Number of extra pixels needed on the right or bottom for upscaler*/
static const int upscale_right_bottom_padding = 2;

/*The number of fractional bits used by downscaler*/
static const int downscale_interpolation_resolution = 3;

/** cpp_create_frame_message_1_12:
 *
 *  @cpp_frame_info - This parameter holds all data needed to setup CPP
 *          hardware for the current frame.
 *  @len - This parameter holds the length of the current frame message.
 *
 *  This function pack's all data into a firmware frame message according to
 *  the micro-controller ISOD document.
 *
 *  Return: Returns pointer to the packed message on success.
 **/
uint32_t* cpp_create_frame_message_1_12(
    struct cpp_frame_info_t* cpp_frame_info, uint32_t* len)
{
  int32_t idx, total_stripes;
  uint32_t msg_len, i, j, k, base;
  uint32_t* frame_msg;
  cpp_wnr_rnr_plane_t *p_rnr;
  cpp_wnr_lnr_plane_t *p_lnr;
  cpp_wnr_frame_info_t *p_wnr = &cpp_frame_info->p_hw_params->wnr_frame_info;

  total_stripes = 0;
  for (i = 0; i < cpp_frame_info->num_planes; i++) {
    total_stripes += cpp_frame_info->plane_info[i].num_stripes;
  }

  uint32_t stripe_base = 790;
  cpp_hardware_params_t *p_hw_params = cpp_frame_info->p_hw_params;

  /* top level + planes + stripes + 1 (trailer) */
  msg_len = stripe_base + 63 * total_stripes + 1;
  *len = msg_len;

  frame_msg = (uint32_t*)CAM_MALLOC(sizeof(uint32_t) * msg_len);
  if (!frame_msg) {
    CPP_ERR("CAM_MALLOC() failed");
    return NULL;
  }
  memset(frame_msg, 0, sizeof(uint32_t) * msg_len);
  base = 0;

  /* Header and Length */
  frame_msg[0] = CPP_FW_CMD_HEADER;
  frame_msg[1] = msg_len - 3;
  /*Top Level*/
  frame_msg[2] = CPP_FW_CMD_ID_PROCESS_FRAME;
  frame_msg[3] = 0x0;
  frame_msg[4] = 0x0;

  /*Input/Output Image info*/
  frame_msg[5] = (cpp_frame_info->in_plane_fmt << 24) |
          (cpp_frame_info->out_plane_fmt << 16) |
          cpp_frame_info->plane_info[0].stripe_info1[0].rot_info.rot_cfg << 13 |
          cpp_frame_info->we_disable << 12 |
          cpp_frame_info->power_collapse << 11 |
          cpp_frame_info->batch_info.batch_size << 1;
  /*Input Plane address HFR*/
  frame_msg[6] = cpp_frame_info->batch_info.intra_plane_offset[0];
  frame_msg[7] = cpp_frame_info->batch_info.intra_plane_offset[1];
  frame_msg[8] = cpp_frame_info->batch_info.intra_plane_offset[2];
  /* Number of stripes */
  frame_msg[9] = (cpp_frame_info->plane_info[0].num_stripes) |
          (cpp_frame_info->plane_info[1].num_stripes << 10) |
          (cpp_frame_info->plane_info[2].num_stripes << 20);

  /*nextStateBufferDistance */
  frame_msg[10] = 0x0;
  /* ASF LUT config */
  base = 11;
  cpp_1_6_x_asf_info_t* asf_info = &(cpp_frame_info->u_asf.info_1_6_x);
  for (i = 0; i < 5; i++) {
    for (j = 0; j < 64; j++) {
      frame_msg[base + i * 64 + j] =
        asf_info->lut[i][j * 4] |
            asf_info->lut[i][j * 4 + 1] << 8 |
            asf_info->lut[i][j * 4 + 2] << 16 |
            asf_info->lut[i][j * 4 + 3] << 24;
    }
  }
  /*pack gain neg LUT table*/
  base = 331;
  for (j = 0; j < 64; j++) {
    frame_msg[base + j] =
      asf_info->cpp_1_12_ext.gain_neg_lut[j * 4] |
          asf_info->cpp_1_12_ext.gain_neg_lut[j * 4 + 1] << 8 |
          asf_info->cpp_1_12_ext.gain_neg_lut[j * 4 + 2] << 16 |
          asf_info->cpp_1_12_ext.gain_neg_lut[j * 4 + 3] << 24;
  }

  /* WNR Config */
  base = 395;
  struct cpp_bf_info* bf_info;
  cpp_prescaler_bf_info_t* prescaler_bf_info;
  cpp_tnr_info_t* tnr_info;

  /* for Y, CB and CR components */
  for (i = 0; i < 3; i++) {
    /* Fill in prescaler_bf */
    prescaler_bf_info = &cpp_frame_info->prescaler_info[i];
    k = base + i * 80;
    frame_msg[k] =
        (uint32_t)(clamp(Round(prescaler_bf_info->bilateral_scale *
        (1 << 10)), 0, 0x3ffff));
    frame_msg[k + 1] =
        ((uint32_t)(clamp(Round(prescaler_bf_info->noise_threshold *
                           (1 << 4)), 0, 0xfff))) << 8 |
            ((uint32_t)(clamp(Round(prescaler_bf_info->weight *
              (1 << 4)), 0, 0x10)));
    /* Fill in temporal_bf */
    tnr_info = &cpp_frame_info->tnr_info[i];
    for (j = 0; j < 4; j++) {
      k = base + 2 + i * 80 + j;
      frame_msg[k] =
          ((uint32_t)(clamp(Round(tnr_info->bilateral_scale[j] *
          (1 << 8)), 0, 0x3ffff)));
      frame_msg[k + 4] =
          ((uint32_t)(clamp(Round(tnr_info->noise_threshold[j] *
                                  (1 << 4)), 0, 0xfff))) << 8 |
              ((uint32_t)(clamp(Round(tnr_info->weight[j] *
                                       (1 << 4)), 0, 0x10)));
    }
    /* Fill in temporal_bf CFG1[4] */
    frame_msg[k + 5] =
        ((uint32_t)(clamp(Round(tnr_info->noise_threshold[j] *
                                (1 << 4)), 0, 0xfff))) << 8 |
            ((uint32_t)(clamp(Round(tnr_info->weight[j] * (1 << 4)), 0, 0x10)));

    /* Fill in spatial bf */
    bf_info = &cpp_frame_info->bf_info[i];
    p_rnr = &p_wnr->wnr_pl_info[i].rnr; //populate RNR pointer

    for (j = 0; j < 4; j++) {
      k = base + 11 + i * 80 + j;
      if (j != 3) {
        frame_msg[k] =
            (uint32_t)(clamp(Round(bf_info->bilateral_scale[j] *
            (1 << 10)), 0, 0x1ffff));
      } else {
        frame_msg[k] =
            (uint32_t)(clamp(Round(bf_info->bilateral_scale[j] *
            (1 << 8)), 0, 0x3ffff));
      }
      frame_msg[k] |= (((p_rnr->bilat_info[j].padding_mode & 0x1) << 24) |
        ((p_rnr->bilat_info[j].rnr_enable & 0x1) << 28) |
        ((p_rnr->bilat_info[j].trilat_filt_enable & 0x1) << 29));
    }

    for (j = 0; j < CPP_DENOISE_NUM_PROFILES; j++) {
      k = base + 11 + i * 80 + j + 4;
      /* Fill in spatial_bf CFG1[4] */
      frame_msg[k] =
        ((uint32_t)(clamp(Round(bf_info->noise_threshold[j] *
          (1 << 4)), 0, 0xfff))) << 8 |
        ((uint32_t)(clamp(Round(bf_info->weight[j] * (1 << 4)), 0, 0x10)));
    }

    /* Fill in clamp_limit */
    frame_msg[k + 1] =
        ((uint32_t)(Round(cpp_frame_info->clamp_limit_UL[i] * (1 << 16))) |
             (uint32_t)(Round(cpp_frame_info->clamp_limit_LL[i])));

    /* Fill BF RNR tables */

    for (j = 0; j < 3; j++) {
      k = base + 22 + i * 80 + j;
      frame_msg[k]      = p_rnr->seg_info[j].r_sq_lut & 0xfff;
      frame_msg[k + 4]  = p_rnr->seg_info[j].bilat_scale_adj.cf & 0xff;
      frame_msg[k + 7]  = p_rnr->seg_info[j].bilat_scale_adj.slope & 0xff;
      frame_msg[k + 10] = p_rnr->seg_info[j].bilat_scale_adj.cf_shift & 0x1f;
      frame_msg[k + 13] = p_rnr->seg_info[j].weight1_adj.cf & 0xff;
      frame_msg[k + 16] = p_rnr->seg_info[j].weight1_adj.slope & 0xff;
      frame_msg[k + 19] = p_rnr->seg_info[j].weight1_adj.cf_shift & 0x1f;
    }

    k = base + 25 + i * 80;
    frame_msg[k] = p_rnr->r_sq_shift & 0x1f;

    for (j = 0; j < 4; j++) {
      k = base + 44 + i * 80 + j;
      frame_msg[k] = (!((p_rnr->bilat_info[j].rnr_wt_enable) & 0x1));
    }

    /* Fill BF LNR tables */
    p_lnr = &p_wnr->wnr_pl_info[i].lnr;

    for (j = 0; j < 8; j++) {
      k = base + 48 + i * 80 + j;
      frame_msg[k] =
        (p_lnr->lregion_info[j * 4].bilat_scale.slope & 0xff) |
        ((p_lnr->lregion_info[j * 4 + 1].bilat_scale.slope & 0xff) << 8) |
        ((p_lnr->lregion_info[j * 4 + 2].bilat_scale.slope & 0xff) << 16) |
        ((p_lnr->lregion_info[j * 4 + 3].bilat_scale.slope & 0xff) << 24);
      frame_msg[k + 8] =
        (p_lnr->lregion_info[j * 4].bilat_scale.cf & 0xff) |
        ((p_lnr->lregion_info[j * 4 + 1].bilat_scale.cf & 0xff) << 8) |
        ((p_lnr->lregion_info[j * 4 + 2].bilat_scale.cf & 0xff) << 16) |
        ((p_lnr->lregion_info[j * 4 + 3].bilat_scale.cf & 0xff) << 24);
      frame_msg[k + 16] =
        (p_lnr->lregion_info[j * 4].weight1.slope & 0xff) |
        ((p_lnr->lregion_info[j * 4 + 1].weight1.slope & 0xff) << 8) |
        ((p_lnr->lregion_info[j * 4 + 2].weight1.slope & 0xff) << 16) |
        ((p_lnr->lregion_info[j * 4 + 3].weight1.slope & 0xff) << 24);
      frame_msg[k + 24] =
        (p_lnr->lregion_info[j * 4].weight1.cf & 0xff) |
        ((p_lnr->lregion_info[j * 4 + 1].weight1.cf & 0xff) << 8) |
        ((p_lnr->lregion_info[j * 4 + 2].weight1.cf & 0xff) << 16) |
        ((p_lnr->lregion_info[j * 4 + 3].weight1.cf & 0xff) << 24);
    }

  }

  /* ASF Settings */
  /* pp_m_asf_cfg_0 */
  frame_msg[635] =
      (1 << 31) |
          /* set asf version to 1, for fw 1.6.0 */
          ((asf_info->sp_eff_en & 0x1) << 0) |
          ((asf_info->dyna_clamp_en & 0x1) << 1) |
          ((asf_info->neg_abs_y1 & 0x1) << 2) |
          ((asf_info->sp & 0x1f) << 4) |
          ((asf_info->sp_eff_abs_en & 0x1) << 16) |
          ((asf_info->L2_norm_en & 0x1) << 17) |
          ((asf_info->symm_filter_only & 0x1) << 18) |
          ((asf_info->radial_enable & 0x1) << 24);
  /* pp_m_asf_cfg_1 */
  frame_msg[636] =
      (asf_info->perpen_scale_factor & 0x7f) |
          ((asf_info->activity_clamp_threshold & 0xff) << 16);
  /* pp_m_asf_cfg_2 */
  frame_msg[637] =
      (asf_info->max_val_threshold & 0x3fff) |
          ((asf_info->gamma_cor_luma_target & 0xff) << 16);
  /* pp_m_asf_cfg_3 */
  frame_msg[638] =
      (asf_info->gain_cap & 0xff) |
          ((asf_info->median_blend_offset & 0xf) << 16) |
          ((asf_info->median_blend_lower_offset & 0xf) << 20);
  /* pp_m_asf_cfg_4 */
  frame_msg[639] = (asf_info->norm_scale & 0xff);
  /* pp_m_asf_clamp_tl_limit */
  frame_msg[640] =
      (asf_info->clamp_tl_ll & 0x1ff) |
          ((asf_info->clamp_tl_ul & 0x1ff) << 9);
  /* pp_m_asf_clamp_scale */
  frame_msg[641] =
      (asf_info->clamp_scale_min & 0x1ff) |
          ((asf_info->clamp_scale_max & 0x1ff) << 16);
  /* pp_m_asf_clamp_offset */
  frame_msg[642] =
      (asf_info->clamp_offset_min & 0x7f) |
          ((asf_info->clamp_offset_max & 0x7f) << 16);
  /* pp_m_asf_nz_flag and _nz_flag_2 */
  frame_msg[643] = asf_info->nz_flag;
  frame_msg[644] = asf_info->nz_flag_2;

  /* pp_m_asf_sobel_h_coeff */
  base = 645;
  for (i = 0; i < 13; i++) {
    frame_msg[base + i] =
        (asf_info->sobel_h_coeff[i * 2] & 0xfff) |
            ((asf_info->sobel_h_coeff[i * 2 + 1] & 0xfff) << 16);
  }
  /* pp_m_asf_sobel_v_coeff */
  base = 658;
  for (i = 0; i < 13; i++) {
    frame_msg[base + i] =
        (asf_info->sobel_v_coeff[i * 2] & 0xfff) |
            ((asf_info->sobel_v_coeff[i * 2 + 1] & 0xfff) << 16);
  }

  /* pp_m_asf_lpf_coeff */
  base = 671;
  for (i = 0; i < 8; i++) {
    frame_msg[base + i] =
        (asf_info->lpf_coeff[i * 2] & 0xfff) |
            ((asf_info->lpf_coeff[i * 2 + 1] & 0xfff) << 16);
  }
  /* pp_m_asf_hpf_symmetric_coeff */
  base = 679;
  for (i = 0; i < 8; i++) {
    frame_msg[base + i] =
        (asf_info->hpf_sym_coeff[i * 2] & 0xfff) |
            ((asf_info->hpf_sym_coeff[i * 2 + 1] & 0xfff) << 16);
  }
  /* pp_m_asf_activity_bpf_coeff */
  base = 687;
  for (i = 0; i < 3; i++) {
    frame_msg[base + i] =
        (asf_info->activity_bpf_coeff[i * 2] & 0x3ff) |
            ((asf_info->activity_bpf_coeff[i * 2 + 1] & 0x3ff) << 16);
  }
  /* pp_m_asf_activity_lpf_coeff */
  base = 690;
  for (i = 0; i < 3; i++) {
    frame_msg[base + i] =
        (asf_info->activity_lpf_coeff[i * 2] & 0xff) |
            ((asf_info->activity_lpf_coeff[i * 2 + 1] & 0xff) << 16);
  }

  /* ASF RNR parameters */
  base = 693;
  for (i = 0; i < 3; i++) {
    frame_msg[base + i] =
      asf_info->cpp_1_12_ext.asf_1_12_rnr_output.rnr_r_sqr_lut[i] & 0xfff;
  }
  frame_msg[696] = asf_info->cpp_1_12_ext.
    asf_1_12_rnr_output.rnr_r_sqr_shift & 0xf;

  base = 697;
  for (i = 0; i < 3; i++) {
    frame_msg[base + i] =
      asf_info->cpp_1_12_ext.asf_1_12_rnr_output.rnr_activity_cf_lut[i] & 0x7ff;
  }
  base = 700;
  for (i = 0; i < 3; i++) {
    frame_msg[base + i] =
      asf_info->cpp_1_12_ext.asf_1_12_rnr_output.rnr_activity_slope_lut[i] & 0xfff;
  }
  base = 703;
  for (i = 0; i < 3; i++) {
    frame_msg[base + i] =
      asf_info->cpp_1_12_ext.asf_1_12_rnr_output.rnr_activity_shift_lut[i] & 0xfff;
  }

  base = 706;
  for (i = 0; i < 3; i++) {
    frame_msg[base + i] =
      asf_info->cpp_1_12_ext.asf_1_12_rnr_output.rnr_gain_cf_lut[i] & 0x7ff;
  }
  base = 709;
  for (i = 0; i < 3; i++) {
    frame_msg[base + i] =
      asf_info->cpp_1_12_ext.asf_1_12_rnr_output.rnr_gain_slope_lut[i] & 0xfff;
  }
  base = 712;
  for (i = 0; i < 3; i++) {
    frame_msg[base + i] =
      asf_info->cpp_1_12_ext.asf_1_12_rnr_output.rnr_gain_shift_lut[i] & 0xfff;
  }

  /* Plane Specific Configuration */
  base = 715;
  for (i = 0; i < cpp_frame_info->num_planes; i++) {
    j = base + i * 25;

    frame_msg[j] = 1 << 5 |
            cpp_frame_info->plane_info[i].next_state_crop_enable << 10 |
            cpp_frame_info->plane_info[i].prescaler_crop_enable << 9 |
            cpp_frame_info->plane_info[i].tnr_crop_enable << 7 |
            cpp_frame_info->plane_info[i].tnr_enable << 6 |
            cpp_frame_info->plane_info[i].asf_crop_enable << 4 |
            cpp_frame_info->plane_info[i].bf_crop_enable << 1 |
            cpp_frame_info->plane_info[i].bf_enable |
            cpp_frame_info->plane_info[i].dsdn_enable << 25;
    if (cpp_frame_info->plane_info[i].scale_info.hscale_en ||
        cpp_frame_info->plane_info[i].scale_info.vscale_en) {
      frame_msg[j] |= 0x4;
    }
    if (cpp_frame_info->plane_info[i].input_plane_fmt == PLANE_Y &&
        cpp_frame_info->asf_mode != 0) {
      frame_msg[j] |= (0x1 << 3);
    }
    /* enable video mode */
    if (cpp_frame_info->plane_info[i].denoise_after_scale_en) {
      frame_msg[j] |= (0x1 << 24);
    }
    CPP_FRAME_MSG(" FRAME_MSG ### i %d, j %d, frame msg %x", i, j, frame_msg[j]);
    frame_msg[j + 1] =
        cpp_frame_info->plane_info[i].scale_info.v_scale_fir_algo << 12 |
            cpp_frame_info->plane_info[i].scale_info.h_scale_fir_algo << 8 |
            cpp_frame_info->plane_info[i].scale_info.v_scale_algo << 5 |
            cpp_frame_info->plane_info[i].scale_info.h_scale_algo << 4 |
            cpp_frame_info->plane_info[i].scale_info.subsample_en << 3 |
            cpp_frame_info->plane_info[i].scale_info.upsample_en << 2 |
            cpp_frame_info->plane_info[i].scale_info.vscale_en << 1 |
            cpp_frame_info->plane_info[i].scale_info.hscale_en;
    frame_msg[j + 2] = cpp_frame_info->plane_info[i].scale_info.phase_h_step;
    frame_msg[j + 3] = cpp_frame_info->plane_info[i].scale_info.phase_v_init;
    frame_msg[j + 4] = cpp_frame_info->plane_info[i].scale_info.phase_v_step;

    /* PP_m_CENTER_PIXEL_COORDINATES */
    frame_msg[j + 5] =
      (cpp_frame_info->plane_info[i].stripe_info->vertical_center << 16) |
      cpp_frame_info->plane_info[i].stripe_info->horizontal_center;

    /* mmu prefetch fetch engine params. This does not have l2 parameters */
    frame_msg[j+6] =
      cpp_frame_info->plane_info[i].mmu_prefetch_fe_info.l1_mmu_pf_en |
      cpp_frame_info->plane_info[i].mmu_prefetch_fe_info.l1_period  << 1 |
      cpp_frame_info->plane_info[i].mmu_prefetch_fe_info.l1_direction << 2 |
      cpp_frame_info->plane_info[i].mmu_prefetch_fe_info.l2_mmu_pf_en << 3;
    frame_msg[j+7] =
      cpp_frame_info->plane_info[i].mmu_prefetch_fe_info.addr_min;
    frame_msg[j+8] =
      cpp_frame_info->plane_info[i].mmu_prefetch_fe_info.addr_max;

    /* mmu prefetch ref fetch engine params. This does not have l2 parameters */
    frame_msg[j+9] =
      cpp_frame_info->plane_info[i].mmu_prefetch_ref_fe_info.l1_mmu_pf_en |
      cpp_frame_info->plane_info[i].mmu_prefetch_ref_fe_info.l1_period  << 1 |
      cpp_frame_info->plane_info[i].mmu_prefetch_ref_fe_info.l1_direction << 2 |
      cpp_frame_info->plane_info[i].mmu_prefetch_ref_fe_info.l2_mmu_pf_en << 3;
    frame_msg[j+10] =
      cpp_frame_info->plane_info[i].mmu_prefetch_ref_fe_info.addr_min;
    frame_msg[j+11]=
      cpp_frame_info->plane_info[i].mmu_prefetch_ref_fe_info.addr_max;

    /* mmu prefetch write engine params */
    frame_msg[j+12] =
      cpp_frame_info->plane_info[i].mmu_prefetch_we_info.l1_mmu_pf_en |
      cpp_frame_info->plane_info[i].mmu_prefetch_we_info.l1_period  << 1 |
      cpp_frame_info->plane_info[i].mmu_prefetch_we_info.l1_direction << 2 |
      cpp_frame_info->plane_info[i].mmu_prefetch_we_info.l2_mmu_pf_en << 3;
    frame_msg[j+13] =
      cpp_frame_info->plane_info[i].mmu_prefetch_we_info.addr_min;
    frame_msg[j+14] =
      cpp_frame_info->plane_info[i].mmu_prefetch_we_info.addr_max;
    frame_msg[j+15] =
      cpp_frame_info->plane_info[i].mmu_prefetch_we_info.l2_range;
    frame_msg[j+16] =
      cpp_frame_info->plane_info[i].mmu_prefetch_we_info.l2_distance;

    /* mmu prefetch duplicate write engine params */
    frame_msg[j+17] =
      cpp_frame_info->plane_info[i].mmu_prefetch_dup_we_info.l1_mmu_pf_en |
      cpp_frame_info->plane_info[i].mmu_prefetch_dup_we_info.l1_period  << 1 |
      cpp_frame_info->plane_info[i].mmu_prefetch_dup_we_info.l1_direction << 2 |
      cpp_frame_info->plane_info[i].mmu_prefetch_dup_we_info.l2_mmu_pf_en << 3;
    frame_msg[j+18] =
      cpp_frame_info->plane_info[i].mmu_prefetch_dup_we_info.addr_min;
    frame_msg[j+19] =
      cpp_frame_info->plane_info[i].mmu_prefetch_dup_we_info.addr_max;
    frame_msg[j+20] =
      cpp_frame_info->plane_info[i].mmu_prefetch_dup_we_info.l2_range;
    frame_msg[j+21] =
      cpp_frame_info->plane_info[i].mmu_prefetch_dup_we_info.l2_distance;

    /* mmu prefetch ref write engine params. This does not have l2 parameters */
    frame_msg[j+22] =
      cpp_frame_info->plane_info[i].mmu_prefetch_ref_we_info.l1_mmu_pf_en |
      cpp_frame_info->plane_info[i].mmu_prefetch_ref_we_info.l1_period  << 1 |
      cpp_frame_info->plane_info[i].mmu_prefetch_ref_we_info.l1_direction << 2 |
      cpp_frame_info->plane_info[i].mmu_prefetch_ref_we_info.l2_mmu_pf_en << 3;
    frame_msg[j+23] =
      cpp_frame_info->plane_info[i].mmu_prefetch_ref_we_info.addr_min;
    frame_msg[j+24] =
      cpp_frame_info->plane_info[i].mmu_prefetch_ref_we_info.addr_max;
  }

  /* Stripe Specific Configuration */
  base = 790;
  idx = base;
  struct cpp_stripe_info *stripe_info;
  struct cpp_plane_info_t *plane_info;
  for (j = 0; j < cpp_frame_info->num_planes; j++) {
    plane_info = &(cpp_frame_info->plane_info[j]);
    for (k = 0; k < cpp_frame_info->plane_info[j].num_stripes; k++, idx += 63) {
      stripe_info = &(cpp_frame_info->plane_info[j].stripe_info1[k]);
      frame_msg[idx + 0] = /* STRIPE[0]_PP_m_ROT_CFG_0 */
          (plane_info->tile_output_enable & 0x1) << 28 | /* UBWC enable */
          (stripe_info->rot_info.rot_cfg & 0x7) << 24 |
              ((stripe_info->rot_info.block_size - 1) & 0xFFFF);
      frame_msg[idx + 1] = /* STRIPE[0]_PP_m_ROT_CFG_1 */
          (stripe_info->rot_info.block_height - 1) << 16 |
              (stripe_info->rot_info.block_width - 1);
      frame_msg[idx + 2] = /* STRIPE[0]_PP_m_ROT_CFG_2 */
          stripe_info->rot_info.rowIndex0;
      frame_msg[idx + 3] = /* STRIPE[0]_PP_m_ROT_CFG_3 */
          stripe_info->rot_info.rowIndex1;
      frame_msg[idx + 4] = /* STRIPE[0]_PP_m_ROT_CFG_4 */
          stripe_info->rot_info.colIndex0;
      frame_msg[idx + 5] = /* STRIPE[0]_PP_m_ROT_CFG_5 */
          stripe_info->rot_info.colIndex1;
      frame_msg[idx + 6] = /* STRIPE[0]_PP_m_ROT_CFG_6 */
          stripe_info->rot_info.initIndex;
      frame_msg[idx + 7] = /* STRIPE[0]_PP_m_ROT_CFG_7 */
          stripe_info->rot_info.modValue;

      /* tile width and tile height */
      frame_msg[idx + 8] = /* STRIPE[0]_PP_m_ROT_CFG_8 */
        (((stripe_info->rot_info.tile_height - 1) & 0x7) << 16) |
        ((stripe_info->rot_info.tile_width -1) & 0x1f);
      /* tile offset and tile size */
      frame_msg[idx + 9] = /* STRIPE[0_PP_m_ROT_CFG_9 */
         (((stripe_info->rot_info.tile_voffset) & 0x1fff) << 16) |
         ((stripe_info->rot_info.tile_size -1) & 0xff);
      /* blocks per stripe */
      frame_msg[idx + 10] = /* STRIPE[0]_PP_m_ROT_CFG_10 */
          ((stripe_info->rot_info.blocks_per_stripe - 1) & 0x1FFF);


      frame_msg[idx + 11] = stripe_info->fe_info.buffer_ptr;
      frame_msg[idx + 12] = /* STRIPE[0]_FE_n_RD_BUFFER_SIZE */
          (stripe_info->fe_info.buffer_height - 1) << 16 |
              (stripe_info->fe_info.buffer_width - 1);
      frame_msg[idx + 13] = /* STRIPE[0]_FE_n_RD_STRIDE */
          stripe_info->fe_info.buffer_stride;
      frame_msg[idx + 14] = /* STRIPE[0]_FE_n_SWC_RD_BLOCK_DIMENSION */
          (stripe_info->fe_info.block_height - 1) << 16 |
              (stripe_info->fe_info.block_width - 1);
      frame_msg[idx + 15] = /* STRIPE[0]_FE_n_SWC_RD_BLOCK_HPAD */
          stripe_info->fe_info.right_pad << 16 |
              stripe_info->fe_info.left_pad;
      if (p_hw_params->mirror == 3) {
        frame_msg[idx + 16] = /* STRIPE[0]_FE_n_SWC_RD_BLOCK_VPAD */
            stripe_info->fe_info.bottom_pad |
                stripe_info->fe_info.top_pad;
      } else {
        frame_msg[idx + 16] = /* STRIPE[0]_FE_n_SWC_RD_BLOCK_VPAD */
          stripe_info->fe_info.bottom_pad << 16 |
              stripe_info->fe_info.top_pad;
      }
      frame_msg[idx + 17] = stripe_info->fe_r_info.buffer_ptr;
      frame_msg[idx + 18] = /* STRIPE[0]_FE_n_RD_BUFFER_SIZE */
          (stripe_info->fe_r_info.buffer_height - 1) << 16 |
              (stripe_info->fe_r_info.buffer_width - 1);
      frame_msg[idx + 19] = /* STRIPE[0]_FE_n_RD_STRIDE */
          stripe_info->fe_r_info.buffer_stride;
      frame_msg[idx + 20] = /* STRIPE[0]_FE_n_SWC_RD_BLOCK_DIMENSION */
          (stripe_info->fe_r_info.block_height - 1) << 16 |
              (stripe_info->fe_r_info.block_width - 1);
      frame_msg[idx + 21] = /* STRIPE[0]_FE_n_SWC_RD_BLOCK_HPAD */
          stripe_info->fe_r_info.right_pad << 16 |
              stripe_info->fe_r_info.left_pad;
      frame_msg[idx + 22] = /* STRIPE[0]_FE_n_SWC_RD_BLOCK_VPAD */
          stripe_info->fe_r_info.bottom_pad << 16 |
              stripe_info->fe_r_info.top_pad;
      frame_msg[idx + 23] = stripe_info->we_info.buffer_ptr[0];
      frame_msg[idx + 24] = stripe_info->we_info.buffer_ptr[1];
      frame_msg[idx + 25] = stripe_info->we_info.buffer_ptr[2];
      frame_msg[idx + 26] = stripe_info->we_info.buffer_ptr[3];
      frame_msg[idx + 27] = /*STRIPE[0]_WE_PLN_n_WR_BUFFER_SIZE */
          stripe_info->we_info.buffer_height << 16 |
              stripe_info->we_info.buffer_width;
      frame_msg[idx + 28] = stripe_info->we_info.buffer_stride;
      frame_msg[idx + 29] = /* STRIPE[0]_WE_PLN_n_WR_CFG_0 */
          (stripe_info->we_info.blocks_per_row - 1) << 16 |
              (stripe_info->we_info.blocks_per_col - 1);
      frame_msg[idx + 30] = stripe_info->we_info.h_step;
      frame_msg[idx + 31] = stripe_info->we_info.v_step;
      frame_msg[idx + 32] = /* STRIPE[0]_WE_PLN_n_WR_CFG_3 */
          (stripe_info->we_info.h_init) << 16 |
              (stripe_info->we_info.v_init);

      /* tile height, tile width */
      frame_msg[idx + 33] = //STRIPE[0]_WE_PLN_n_WR_CFG_4
        ((((stripe_info->we_info.tile_height - 1) & 0x7) << 16) |
        ((stripe_info->we_info.tile_width - 1) & 0x1f));
      /* tiles per block col, tiles per block row */
      frame_msg[idx + 34] = /* STRIPE[0]_WE_PLN_n_WR_CFG_5 */
        ((stripe_info->we_info.tiles_per_block_col - 1) & 0xFF) << 16 |
        ((stripe_info->we_info.tiles_per_block_row - 1) & 0xFF);
      /* UBWC control bits */
      frame_msg[idx + 35] = /*STRIPE[0]_WE_PLN_n_WR_CFG_6 */
        (((stripe_info->we_info.ubwc_bank_spread_en & 0x1) << 22) | /* bank spread enable */
        ((stripe_info->we_info.highest_bank_en & 0x1) << 21) | /* highest bank bit enable */
        ((stripe_info->we_info.highest_bank_bit & 0xF) << 16) | /* highest bank bit */
        ((stripe_info->we_info.rot_mode & 0x7) << 8) | /* rotation mode */
        ((stripe_info->we_info.ubwc_tile_format & 0x3) << 2) | /* tile format */
        ((stripe_info->we_info.ubwc_out_type & 0x1) << 1) | // compressed output
        (stripe_info->we_info.ubwc_en & 0x1)); /* UBWC enable */

      frame_msg[idx + 36] = stripe_info->we_r_info.buffer_ptr[0];
      frame_msg[idx + 37] = /* STRIPE[0]_WE_PLN_n_WR_BUFFER_SIZE */
          stripe_info->we_r_info.buffer_height << 16 |
              stripe_info->we_r_info.buffer_width;
      frame_msg[idx + 38] = /* STRIPE[0]_WE_PLN_n_WR_CFG_0 */
          (stripe_info->we_r_info.blocks_per_row - 1) << 16 |
              (stripe_info->we_r_info.blocks_per_col - 1);
      frame_msg[idx + 39] = stripe_info->we_r_info.h_step;
      frame_msg[idx + 40] = stripe_info->we_r_info.v_step;
      frame_msg[idx + 41] = /* STRIPE[0]_WE_PLN_n_WR_CFG_3 */
          (stripe_info->we_r_info.h_init) << 16 |
              (stripe_info->we_r_info.v_init);

      frame_msg[idx + 42] = /* WE_PLN_0_ META_DATA_WR_PNTR */
                            plane_info->metadata_offset;
      frame_msg[idx + 43] = /* WE_PLN_1_ META_DATA_WR_PNTR */
                            plane_info->metadata_offset;
      frame_msg[idx + 44] = /* WE_PLN_2_ META_DATA_WR_PNTR */
                            plane_info->metadata_offset;
      frame_msg[idx + 45] = /* WE_PLN_3_ META_DATA_WR_PNTR */
                            plane_info->metadata_offset;
      frame_msg[idx + 46] = /* WE_PLN_n_META_DATA_STRIDE */
                            plane_info->metadata_stride;

      frame_msg[idx + 47] = /* STRIPE[0]_PP_m_PRESCALER_BF_CROP_CFG_0 */
          stripe_info->prescaler_bf_crop_info.last_pixel << 16 |
              stripe_info->prescaler_bf_crop_info.first_pixel;
      frame_msg[idx + 48] = /* STRIPE[0]_PP_m_PRESCALER_BF_CROP_CFG_1 */
          stripe_info->prescaler_bf_crop_info.last_line << 16 |
              stripe_info->prescaler_bf_crop_info.first_line;
      frame_msg[idx + 49] = /* STRIPE[0]_PP_m_TEMPORAL_BF_CROP_CFG_0 */
          stripe_info->temporal_bf_crop_info.last_pixel << 16 |
              stripe_info->temporal_bf_crop_info.first_pixel;
      frame_msg[idx + 50] = /* STRIPE[0]_PP_m_TEMPORAL_BF_CROP_CFG_1 */
          stripe_info->temporal_bf_crop_info.last_line << 16 |
              stripe_info->temporal_bf_crop_info.first_line;
      frame_msg[idx + 51] = /* STRIPE[0]_PP_m_BF_CROP_CFG_0 */
          stripe_info->bf_crop_info.last_pixel << 16 |
              stripe_info->bf_crop_info.first_pixel;
      frame_msg[idx + 52] = /* STRIPE[0]_PP_m_BF_CROP_CFG_1 */
          stripe_info->bf_crop_info.last_line << 16 |
              stripe_info->bf_crop_info.first_line;

      /*
       * horizontal and vertical count
       * PP_m_BF_DEC_CFG_1
       * CPP CDS (DSDN) config
       */
      frame_msg[idx + 53] = stripe_info->dsdn_info.vertical_cnt32 << 24 |
                            stripe_info->dsdn_info.horizontal_cnt32 << 16 |
                            stripe_info->dsdn_info.vertical_cnt64 << 8  |
                            stripe_info->dsdn_info.horizontal_cnt64;
      /*
       * upscale crop and dimension
       * PP_m_BF_DEC_CFG_2
       * CPP CDS (DSDN) config
       */
      frame_msg[idx + 54] = stripe_info->dsdn_info.pre_upscale_top_crop  << 30 |
                            stripe_info->dsdn_info.pre_upscale_left_crop << 28 |
                            (stripe_info->dsdn_info.upscale_out_width - 1) << 16 |
                            (stripe_info->dsdn_info.upscale_out_height - 1);

      frame_msg[idx + 55] = /* STRIPE[0]_PP_m_SCALE_OUTPUT_CFG */
          (stripe_info->scale_info.block_height - 1) << 16 |
              (stripe_info->scale_info.block_width - 1);
      frame_msg[idx + 56] = /* STRIPE[0]_PP_m_SCALE_PHASEH_INIT */
          stripe_info->scale_info.phase_h_init;
      frame_msg[idx + 57] = /* STRIPE[0]_PP_m_NEXT_STATE_BF_CROP_CFG_0 */
          stripe_info->next_state_bf_crop_info.last_pixel << 16 |
              stripe_info->next_state_bf_crop_info.first_pixel;
      frame_msg[idx + 58] = /* STRIPE[0]_PP_m_NEXT_STATE_BF_CROP_CFG_1 */
          stripe_info->next_state_bf_crop_info.last_line << 16 |
              stripe_info->next_state_bf_crop_info.first_line;

      /* ASF & BF RNR offsets */
      frame_msg[idx + 59] =
        (cpp_frame_info->plane_info[j].stripe_info[k].\
        wnr_start_vertical_offset << 16) |
        cpp_frame_info->plane_info[j].stripe_info[k].\
        wnr_start_horizontal_offset;

      frame_msg[idx + 60] =
        (cpp_frame_info->plane_info[j].stripe_info[k].\
        asf_start_vertical_offset << 16) |
        cpp_frame_info->plane_info[j].stripe_info[k].\
        asf_start_horizontal_offset;

      frame_msg[idx + 61] = /* STRIPE[0]_PP_m_ASF_CROP_CFG_0 */
          stripe_info->asf_crop_info.last_pixel << 16 |
              stripe_info->asf_crop_info.first_pixel;
      frame_msg[idx + 62] = /* STRIPE[0]_PP_m_ASF_CROP_CFG_1 */
          stripe_info->asf_crop_info.last_line << 16 |
              stripe_info->asf_crop_info.first_line;
    }
  }
  frame_msg[msg_len - 1] = CPP_FW_CMD_TRAILER;

  return frame_msg;
}

void cpp_hw_params_radial_1_12(struct cpp_plane_info_t *in_ary,
  uint32_t src_tile_index_x_counter, uint32_t src_tile_index_y_counter,
  int64_t *pad_left, int64_t *pad_top,
  struct cpp_striping_algorithm_info* out_ary)
{
  int64_t dst_horizontal_center;
  int64_t dst_vertical_center;

  if (in_ary->denoise_after_scale_en) {
    if ((src_tile_index_y_counter == 0) && (src_tile_index_x_counter == 0)) {
      dst_horizontal_center = (int64_t)(in_ary->horizontal_center << q_factor);
      dst_horizontal_center -= in_ary->horizontal_scale_initial_phase;
      dst_horizontal_center /= in_ary->horizontal_scale_ratio;
      dst_horizontal_center += out_ary->sharpen_crop_width_first_pixel +
        out_ary->spatial_denoise_crop_width_first_pixel;
      *pad_left = out_ary->sharpen_crop_width_first_pixel +
        out_ary->spatial_denoise_crop_width_first_pixel;
      out_ary->horizontal_center = dst_horizontal_center + *pad_left;

      dst_vertical_center = (int64_t)(in_ary->vertical_center << q_factor);
      dst_vertical_center -= in_ary->vertical_scale_initial_phase;
      dst_vertical_center /= in_ary->vertical_scale_ratio;
      *pad_top = out_ary->sharpen_crop_height_first_line +
        out_ary->spatial_denoise_crop_height_first_line;
      out_ary->vertical_center = dst_vertical_center + *pad_top;
    }
    out_ary->asf_start_horizontal_offset = out_ary->temporal_dst_start_x -
      out_ary->sharpen_crop_width_first_pixel + *pad_left;
    out_ary->asf_start_vertical_offset = out_ary->temporal_dst_start_y -
      out_ary->sharpen_crop_height_first_line + *pad_top;
    out_ary->wnr_start_horizontal_offset =
      out_ary->asf_start_horizontal_offset -
      out_ary->spatial_denoise_crop_width_first_pixel;
    out_ary->wnr_start_vertical_offset = out_ary->asf_start_vertical_offset -
      out_ary->spatial_denoise_crop_height_first_line;
  } else {
    if ((src_tile_index_y_counter == 0) && (src_tile_index_x_counter == 0)) {
      *pad_left = out_ary->pad_left;
      *pad_top = out_ary->pad_top;
      out_ary->horizontal_center = in_ary->horizontal_center + *pad_left;
      out_ary->vertical_center = in_ary->vertical_center + *pad_top;
    }
    out_ary->wnr_start_horizontal_offset = out_ary->src_start_x + *pad_left -
      out_ary->pad_left +
      out_ary->prescaler_spatial_denoise_crop_width_first_pixel +
      out_ary->temporal_denoise_crop_width_first_pixel;
    out_ary->wnr_start_vertical_offset = out_ary->src_start_y + *pad_top -
      out_ary->pad_top +
      out_ary->prescaler_spatial_denoise_crop_height_first_line +
      out_ary->temporal_denoise_crop_height_first_line;
    out_ary->asf_start_horizontal_offset =
      out_ary->wnr_start_horizontal_offset +
      out_ary->sharpen_crop_width_first_pixel;
    out_ary->asf_start_vertical_offset = out_ary->wnr_start_vertical_offset +
      out_ary->sharpen_crop_height_first_line;
  }
}
