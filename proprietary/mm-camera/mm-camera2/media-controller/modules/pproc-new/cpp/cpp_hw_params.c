/*============================================================================

  Copyright (c) 2013-2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include "eztune_diagnostics.h"
#include "cpp_hw_params.h"
#include "cpp_hardware.h"
#include "cpp_log.h"
#include "cpp_default_asf9.h"
#include "chromatix_cpp_stripped.h"
#include <math.h>
#include "pp_buf_mgr.h"

#ifdef CHROMATIX_EXT
#include "cpp_hw_params_ext.h"
#endif


#define CPP_FW_CMD_HEADER           0x3E646D63
#define CPP_FW_CMD_TRAILER          0xabcdefaa
#define CPP_FW_CMD_ID_PROCESS_FRAME 0x6

#define CPP_PARAM_BEAUTY_EFFECT_ASF_FACTOR 0.0f  // 1.0f
#define CPP_PARAM_BEAUTY_EFFECT_WNR_FACTOR 0.0f  // 1.0f

//#define __CPP_PARAM_DEBUG__

/* Fixed point conversions */
#define to_8uQ8(val) ((int16_t)(Round((val) * 256.0)))
#define to_8uQ5(val) ((int16_t)((Round((val) * 32.0f))))
#define to_7uQ4(val) ((int16_t)((Round((val) * 16.0f))))
#define to_5uQ4(val) ((int16_t)((Round((val) * 16.0f))))
#define to_4uQ4(val) ((int16_t)((Round((val) * 16.0f)) & 0x0f))
#define to_8uQ4(val) ((int16_t)((Round((val) * 16.0f))))


/* generic fixed point conversion to MuQn format, M = 32 max */
#define to_MuQn(M, n, val) \
 (((uint32_t) round((val) * (1 << (n))) & ((1 << (M)) - 1)))

/* convert a number to 9-bit 2's complement */
#define int_to_9bit_2sComplement(x) \
  ((uint16_t)(((x) < 0) ? ((((-(x)) & 0xff) + 1) & 0x100) : x))

/* 4x4 indicies within a 5x5 kernel */
uint32_t const asf_4_4_Idx[16] = {6,7,8,9,11,12,13,14,16,17,18,19,21,22,23,24};

inline uint16_t to_9bit_2sComplement(int x)
{
  if(x >= 0) return x;
  uint16_t y = -x;
  return ((~y & 0xff) + 1) | (1<<8);
}

inline uint8_t conv_nz_flag_int_to_uint(int x)
{
  if(x == 0) return 1;
  if(x == 1) return 0;
  if(x == -1) return 2;
  CPP_ERR("invalid nz_flag %d", x);
  return 3;
}

#define TOSTR(__val) #__val

static char *__cam_type_str[] = {
  TOSTR(CAM_TYPE_STANDALONE),
  TOSTR(CAM_TYPE_MAIN),
  TOSTR(CAM_TYPE_AUX),
  TOSTR(CAM_TYPE_SECURE)
};
#define CAM_SYNC_TYPE(__enum_val) ({ \
  (((__enum_val) >= CAM_TYPE_STANDALONE) && \
   ((__enum_val) <= CAM_TYPE_SECURE)) ? \
    __cam_type_str[(__enum_val)] : "InValid"; \
  })

static char *__cam_role_str[] = {
  TOSTR(CAM_ROLE_MASTER),
  TOSTR(CAM_ROLE_SLAVE)
};
#define CAM_ROLE(__enum_val) ({ \
  (((__enum_val) >= 0) && ((__enum_val) <= 1)) ? \
    __cam_role_str[(__enum_val)] : "InValid"; \
  })

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

static bool cpp_hw_params_is_lln_enabled(cpp_hardware_t *cpphw,
  cpp_hardware_params_t *hw_params, uint32_t plane) {

  if(hw_params->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC)
    return false;

  if (!cpphw->max_supported_padding)
    return false;

  if (!hw_params->denoise_enable)
    return false;

  if (plane == 0)
    return false;

  if (!hw_params->input_info.plane_info[plane].plane_offset_x ||
    !hw_params->input_info.plane_info[plane].plane_offset_y)
    return false;

  return true;
}

static bool cpp_hw_params_is_tnr_enabled(cpp_hardware_t *cpphw,
  cpp_hardware_params_t *hw_params, struct cpp_plane_info_t *plane_info)
{
  double h_scale_ratio = plane_info->h_scale_ratio;
  double v_scale_ratio = plane_info->v_scale_ratio;
  uint32_t dst_res = hw_params->output_info.width *
    hw_params->output_info.height;

  if(!(cpphw->hwinfo.caps & TNR_CAPS))
    return false;

  if (!hw_params->tnr_enable)
    return false;

  /* Do not enable tnr if cpp cpds (dsdn) is enabled */
  if (hw_params->dsdn_enable)
    return false;

  if(!hw_params->hyst_tnr_status)
    return false;

  /*
   * TNR cannot be supported if scale ratio is greater than or equal to 6.0f
   * for NVformats and 4.4f for UBWC format.
   * This is checked based on scale ratio computed with process window
   * dimension and output dimension. chroma plane / CDS / LDS dimension
   * change is not accounted to avoid tnr being enabled on plane basis
   */
  double tnr_downscale_limit = UBWC_ENABLE(hw_params->output_info.plane_fmt) ?
    CPP_DOWNSCALE_LIMIT_TNR_UBWC : CPP_DOWNSCALE_LIMIT_TNR;
  if ((F_EQUAL(h_scale_ratio, tnr_downscale_limit)) ||
    (F_EQUAL(v_scale_ratio, tnr_downscale_limit)) ||
    (h_scale_ratio > tnr_downscale_limit) ||
    (v_scale_ratio > tnr_downscale_limit))
    return false;

  if (((h_scale_ratio > 1.0f) || (v_scale_ratio > 1.0f)) &&
    (dst_res <= CPP_TNR_DS_RES_LIMIT)) {
    return false;
  }

  CPP_TNR_LOW("TNR enabled for stream %d", hw_params->stream_type);
  return true;
}

static bool cpp_hw_params_is_pbf_enabled(struct cpp_plane_info_t *plane_info,
  cpp_hardware_params_t *hw_params) {

  if (!plane_info->tnr_enable)
    return false;

  if (!hw_params->pbf_enable)
    return false;

  if ((F_EQUAL(plane_info->h_scale_ratio, 1.0f)) &&
    (F_EQUAL(plane_info->v_scale_ratio, 1.0f))) {
    return false;
  }

  if(!hw_params->hyst_pbf_status)
    return false;

  CPP_TNR_LOW("PBF enabled for stream %d", hw_params->stream_type);
  return true;
}

static bool cpp_hw_params_is_dsdn_enabled(cpp_hardware_t *cpphw,
  cpp_hardware_params_t *hw_params, int32_t plane) {

  if (!hw_params->dsdn_mask)
    return false;

  if ((plane == 0) && (!hw_params->luma_dsdn))
    return false;

  /* Do not enable cpp cds if lds is enabled is enabled */
  if (hw_params->lds_enable)
    return false;

  /* Do not enable cpp cds if vfe cds is enabled */
  if (hw_params->uv_upsample_enable)
    return false;

  /* Enable cpp cds for hw version 6_0_0 - 8996 and cds flag is enabled */
  if ( (IS_HW_VERSION_SUPPORTS_DSDN(cpphw->hwinfo.version)) &&
    (!hw_params->dsdn_enable))
    return false;

  /* Do not enable cpp cds if tnr is enabled */
  if (hw_params->tnr_enable)
    return false;

  /* Do not enable cpp cds if wnr is not enabled */
  if (!hw_params->denoise_enable)
    return false;

  /* Check hysteresis info to turn on /off */
  if (!hw_params->hyst_dsdn_status)
    return false;

  CPP_DENOISE_LOW("DSDN enabled for stream %d", hw_params->stream_type);
  return true;
}

void increment_phase (struct cpp_accumulated_phase_t *in_phase,
  uint32_t current_block_width, uint32_t current_block_height)
{
  in_phase->phase_x_cur =
  in_phase->PHASE_X_STEP * in_phase->DST_TILE_SIZE_X *
  in_phase->X_tile_index + in_phase->INIT_PHASE_X;
  in_phase->phase_x_next =
  in_phase->phase_x_cur + in_phase->PHASE_X_STEP * current_block_width;
  in_phase->phase_y_cur =
  in_phase->PHASE_Y_STEP * in_phase->DST_TILE_SIZE_Y *
  in_phase->Y_tile_index + in_phase->INIT_PHASE_Y;
  in_phase->phase_y_next =
  in_phase->phase_y_cur + in_phase->PHASE_Y_STEP * current_block_height;
}

void run_TW_logic (struct cpp_plane_info_t *in_ary,
  uint32_t src_tile_index_x_counter, uint32_t src_tile_index_y_counter,
  struct cpp_striping_algorithm_info *out_ary)
{
  uint32_t i;
  out_ary->dst_start_x = in_ary->dst_x_offset;
  out_ary->dst_start_y = in_ary->dst_y_offset;
  switch (in_ary->rotate) {
  case 1: /*90 degree*/
    if (in_ary->mirror & 0x1) {
      for (i = 0; i < src_tile_index_y_counter; i++) {
        out_ary->dst_start_x += in_ary->stripe_block_height[i];
      }
      out_ary->dst_end_x = out_ary->dst_start_x +
        in_ary->stripe_block_height[src_tile_index_y_counter] - 1;
    } else {
      for (i = in_ary->frame_height_mcus - 1;
        i > src_tile_index_y_counter; i--) {
        out_ary->dst_start_x += in_ary->stripe_block_height[i];
      }
      out_ary->dst_end_x = out_ary->dst_start_x +
        in_ary->stripe_block_height[src_tile_index_y_counter] - 1;
    }
    if (in_ary->mirror & 0x2) {
      for (i = in_ary->frame_width_mcus - 1;
           i > src_tile_index_x_counter; i--) {
        out_ary->dst_start_y += in_ary->stripe_block_width[i];
      }
    } else {
      for (i = 0; i < src_tile_index_x_counter; i++) {
        out_ary->dst_start_y += in_ary->stripe_block_width[i];
    }
    }
    out_ary->dst_end_y = out_ary->dst_start_y +
      in_ary->stripe_block_width[src_tile_index_x_counter] - 1;
    break;
  case 2:
    if (in_ary->mirror & 0x1) {
      for (i = 0; i < src_tile_index_x_counter; i++) {
        out_ary->dst_start_x += in_ary->stripe_block_width[i];
      }
      out_ary->dst_end_x =
      out_ary->dst_start_x +
      in_ary->stripe_block_width[src_tile_index_x_counter] - 1;
    } else {
      for (i = in_ary->frame_width_mcus - 1;
          i > src_tile_index_x_counter; i--) {
        out_ary->dst_start_x += in_ary->stripe_block_width[i];
      }
      out_ary->dst_end_x =
      out_ary->dst_start_x +
      in_ary->stripe_block_width[src_tile_index_x_counter] - 1;
    }
    for (i = in_ary->frame_height_mcus - 1;
        i > src_tile_index_y_counter; i--) {
      out_ary->dst_start_y += in_ary->stripe_block_height[i];
    }
    out_ary->dst_end_y =
    out_ary->dst_start_y +
    in_ary->stripe_block_height[src_tile_index_y_counter] - 1;
    break;
  case 3:
    if (in_ary->mirror & 0x1) {
      for (i = in_ary->frame_height_mcus - 1;
        i > src_tile_index_y_counter; i--) {
        out_ary->dst_start_x += in_ary->stripe_block_height[i];
      }
      out_ary->dst_end_x = out_ary->dst_start_x +
        in_ary->stripe_block_height[src_tile_index_y_counter] - 1;
    } else {
      for (i = 0; i < src_tile_index_y_counter; i++) {
        out_ary->dst_start_x += in_ary->stripe_block_height[i];
      }
      out_ary->dst_end_x = out_ary->dst_start_x +
        in_ary->stripe_block_height[src_tile_index_y_counter] - 1;
    }
    if (in_ary->mirror & 0x2) {
      for (i = 0; i < src_tile_index_x_counter; i++) {
        out_ary->dst_start_y += in_ary->stripe_block_width[i];
      }
    } else {
      for (i = in_ary->frame_width_mcus - 1;
        i > src_tile_index_x_counter; i--) {
        out_ary->dst_start_y += in_ary->stripe_block_width[i];
  }
    }
    out_ary->dst_end_y = out_ary->dst_start_y +
      in_ary->stripe_block_width[src_tile_index_x_counter] - 1;
    break;
  case 0:
  default:
    if (in_ary->mirror & 0x1) {
      for (i = in_ary->frame_width_mcus - 1;
          i > src_tile_index_x_counter; i--) {
        out_ary->dst_start_x += in_ary->stripe_block_width[i];
      }
      out_ary->dst_end_x =
      out_ary->dst_start_x +
      in_ary->stripe_block_width[src_tile_index_x_counter] - 1;
    } else {
      for (i = 0; i < src_tile_index_x_counter; i++) {
        out_ary->dst_start_x += in_ary->stripe_block_width[i];
      }
      out_ary->dst_end_x =
      out_ary->dst_start_x +
      in_ary->stripe_block_width[src_tile_index_x_counter] - 1;
    }
    for (i = 0; i < src_tile_index_y_counter; i++) {
      out_ary->dst_start_y += in_ary->stripe_block_height[i];
    }
    out_ary->dst_end_y =
    out_ary->dst_start_y +
    in_ary->stripe_block_height[src_tile_index_y_counter] - 1;
    break;
  }
  out_ary->destination_address[0] =
    in_ary->destination_address[0] +
    out_ary->dst_start_x * out_ary->output_bytes_per_pixel +
    out_ary->dst_start_y * in_ary->dst_stride;
  out_ary->destination_address[1] =
    in_ary->destination_address[1] +
    out_ary->dst_start_x * out_ary->output_bytes_per_pixel +
    out_ary->dst_start_y * in_ary->dst_stride;
}

/*run tile fetch logic*/
void run_TF_logic (struct cpp_plane_info_t *in_ary,
  uint32_t src_tile_index_x_counter, uint32_t src_tile_index_y_counter,
  struct cpp_striping_algorithm_info *out_ary)
{
  /*et up information needed by phase accumulator*/

  struct cpp_accumulated_phase_t phase_fg;
  if (src_tile_index_x_counter) { /*not the leftmost stripe*/
    phase_fg.INIT_PHASE_X = in_ary->horizontal_scale_block_initial_phase;
  } else {              /*the leftmost stripe*/
    phase_fg.INIT_PHASE_X = in_ary->horizontal_scale_initial_phase;
  }
  if (src_tile_index_y_counter) { /*not the topmost block*/
    phase_fg.INIT_PHASE_Y = in_ary->vertical_scale_block_initial_phase;
  } else {              /*the topmost block*/
    phase_fg.INIT_PHASE_Y = in_ary->vertical_scale_initial_phase;
  }

  phase_fg.PHASE_X_STEP = in_ary->horizontal_scale_ratio;
  phase_fg.PHASE_Y_STEP = in_ary->vertical_scale_ratio;
  phase_fg.X_tile_index = src_tile_index_x_counter;
  phase_fg.Y_tile_index = src_tile_index_y_counter;
  phase_fg.DST_TILE_SIZE_X = in_ary->dst_block_width;
  phase_fg.DST_TILE_SIZE_Y = in_ary->dst_block_height;

  /*invoke phase accumlator*/
  increment_phase (&phase_fg,
                   in_ary->stripe_block_width[src_tile_index_x_counter],
                   in_ary->stripe_block_height[src_tile_index_y_counter]);

  /*only the fractional bits are needed*/
  out_ary->h_init_phase = (int) (phase_fg.phase_x_cur & q_mask);
  out_ary->v_init_phase = (int) (phase_fg.phase_y_cur & q_mask);

  out_ary->h_phase_step = in_ary->horizontal_scale_ratio;
  out_ary->v_phase_step = in_ary->vertical_scale_ratio;

  /*that is, we are using upscaler for scaling in the x direction*/
  if (in_ary->horizontal_scale_ratio < (1 << q_factor) ||
      (in_ary->horizontal_scale_ratio == (1 << q_factor) &&
       out_ary->h_init_phase != 0)) {
    out_ary->scale_h_en = 1;
    out_ary->upscale_h_en = 1;
    if (src_tile_index_x_counter) {
      /*not the leftmost stripe, need to fetch more on the left*/
        unsigned long long temp = // position of the first pixel of the upscaler output
        phase_fg.phase_x_cur -
        in_ary->postscale_padding * in_ary->horizontal_scale_ratio;

      /*only the fractional bits are needed*/
      out_ary->h_init_phase = (int) (temp & q_mask);
      out_ary->src_start_x =
      (int) (temp >> q_factor) -
        upscale_left_top_padding - in_ary->prescale_padding;

      out_ary->spatial_denoise_crop_width_first_pixel = in_ary->prescale_padding;
      out_ary->sharpen_crop_width_first_pixel =
      in_ary->postscale_padding;
    } else {
      /*the leftmost stripe.
        left padding is provided by denoise and sharpening but not upscaling*/

      /*only the fractional bits are needed*/
      out_ary->h_init_phase = (int) (phase_fg.phase_x_cur & q_mask);
      out_ary->src_start_x =
      (int) ((phase_fg.phase_x_cur) >> q_factor) -
      upscale_left_top_padding;
      out_ary->spatial_denoise_crop_width_first_pixel = 0;
      out_ary->sharpen_crop_width_first_pixel = 0;
    }
    if (src_tile_index_x_counter == in_ary->frame_width_mcus - 1) {
      /*the rightmost stripe,
        right padding is provided by denoise and sharpening but not upscaling*/
      out_ary->src_end_x =
      (int) ((phase_fg.phase_x_next -
              in_ary->horizontal_scale_ratio) >> q_factor) +
      upscale_right_bottom_padding;
      out_ary->spatial_denoise_crop_width_last_pixel =
      out_ary->src_end_x - out_ary->src_start_x;
    } else { /*not the rightmost stripe. need to fetch more on the right*/
      out_ary->src_end_x =
      (int) ((phase_fg.phase_x_next +
              (in_ary->postscale_padding -
               1) * in_ary->horizontal_scale_ratio) >> q_factor) +
      upscale_right_bottom_padding + in_ary->prescale_padding;
      out_ary->spatial_denoise_crop_width_last_pixel =
      out_ary->src_end_x - out_ary->src_start_x -
      in_ary->prescale_padding;
    }
  } else if (in_ary->horizontal_scale_ratio > (1 << q_factor)) {
    /*horizontal downscaler*/
    out_ary->scale_h_en = 1;
    out_ary->upscale_h_en = 0;
    if (src_tile_index_x_counter) {
      /*not the leftmost stripe, need to fetch more on the left*/

      unsigned long long temp = // position of the first pixel of the upscaler output
        phase_fg.phase_x_cur -
        in_ary->postscale_padding * in_ary->horizontal_scale_ratio;
      /*only the fractional bits are needed*/
      out_ary->h_init_phase = (int) (temp & q_mask);
      out_ary->src_start_x =
        (int) (temp >> q_factor) - in_ary->prescale_padding;

      out_ary->spatial_denoise_crop_width_first_pixel = in_ary->prescale_padding;
      out_ary->sharpen_crop_width_first_pixel =
      in_ary->postscale_padding;
    } else {
      /*the leftmost stripe, left padding is done by denoise and sharpening*/

      /*only the fractional bits are needed*/
      out_ary->h_init_phase = (int) (phase_fg.phase_x_cur & q_mask);
      out_ary->src_start_x = (int) (phase_fg.phase_x_cur >> q_factor);
      out_ary->spatial_denoise_crop_width_first_pixel = 0;
      out_ary->sharpen_crop_width_first_pixel = 0;
    }
    if (src_tile_index_x_counter == in_ary->frame_width_mcus - 1) {
      /*the rightmost stripe,
        right padding is provided by denoise and sharpening*/
      if ((phase_fg.phase_x_next & q_mask) >> (q_factor -
                                               downscale_interpolation_resolution)) {
        out_ary->src_end_x = (int) (phase_fg.phase_x_next >> q_factor);
      } else {
        /*need to fetch one pixel less*/
        out_ary->src_end_x = (int) (phase_fg.phase_x_next >> q_factor) - 1;
      }
      out_ary->spatial_denoise_crop_width_last_pixel =
      out_ary->src_end_x - out_ary->src_start_x;
    } else {
      /*not the rightmost stripe, need to fetch more on the right*/
      long long temp =
      phase_fg.phase_x_next +
      in_ary->postscale_padding * in_ary->horizontal_scale_ratio;
      if ((temp & q_mask) >>
          (q_factor - downscale_interpolation_resolution)) {
        out_ary->src_end_x =
        (int) (temp >> q_factor) + in_ary->prescale_padding;
      } else {
        /*need to fetch one pixel less*/
        out_ary->src_end_x = (int) (temp >> q_factor) - 1 + in_ary->prescale_padding;
      }
      out_ary->spatial_denoise_crop_width_last_pixel =
      out_ary->src_end_x - out_ary->src_start_x -
      in_ary->prescale_padding;
    }
  } else {
    /*that is, scaling is disabled in the x direction*/
    out_ary->scale_h_en = 0;
    out_ary->upscale_h_en = 0;
    out_ary->h_init_phase = 0;
    if (src_tile_index_x_counter) {
      /*not the leftmost stripe, need to fetch more on the left*/
      out_ary->src_start_x =
      (int) (phase_fg.phase_x_cur >> q_factor) -
      in_ary->postscale_padding - in_ary->prescale_padding;
      out_ary->spatial_denoise_crop_width_first_pixel = in_ary->prescale_padding;
      out_ary->sharpen_crop_width_first_pixel =
      in_ary->postscale_padding;
    } else {
      CPP_STRIPE("Left most stripe\n");
      /*the leftmost stripe,
        padding is done internally in denoise and sharpening block*/
      out_ary->src_start_x = (int) (phase_fg.phase_x_cur >> q_factor);
      out_ary->spatial_denoise_crop_width_first_pixel = 0;
      out_ary->sharpen_crop_width_first_pixel = 0;
    }
    if (src_tile_index_x_counter == in_ary->frame_width_mcus - 1) {
      /*the rightmost stripe, right padding is done internally in the HW*/
      out_ary->src_end_x = (int) (phase_fg.phase_x_next >> q_factor) - 1;
      out_ary->spatial_denoise_crop_width_last_pixel =
      out_ary->src_end_x - out_ary->src_start_x;
    } else {
      /*not the rightmost stripe, need to fetch more on the right*/
      out_ary->src_end_x =
      (int) (phase_fg.phase_x_next >> q_factor) - 1 +
      in_ary->postscale_padding + in_ary->prescale_padding;
      out_ary->spatial_denoise_crop_width_last_pixel =
      out_ary->src_end_x - out_ary->src_start_x -
      in_ary->prescale_padding;
    }
  }
  out_ary->sharpen_crop_width_last_pixel =
  out_ary->sharpen_crop_width_first_pixel +
  in_ary->stripe_block_width[src_tile_index_x_counter] - 1;
  /*output width of the scaler should be
    the write stripe width plus some padding required by sharpening*/
  out_ary->scale_output_width =
  in_ary->stripe_block_width[src_tile_index_x_counter];
  if (src_tile_index_x_counter != 0) {
    out_ary->scale_output_width += in_ary->postscale_padding;
  }
  if (src_tile_index_x_counter != in_ary->frame_width_mcus - 1) {
    out_ary->scale_output_width += in_ary->postscale_padding;
  }

  if (in_ary->vertical_scale_ratio < (1 << q_factor) ||
      (in_ary->vertical_scale_ratio == (1 << q_factor) &&
       out_ary->v_init_phase != 0)) {
    /*that is, we are using FIR for scaling in the y direction*/
    out_ary->scale_v_en = 1;
    out_ary->upscale_v_en = 1;
    if (src_tile_index_y_counter) {
      /*not the topmost block, need to fetch more on top*/
      unsigned long long temp = phase_fg.phase_y_cur -
        in_ary->postscale_padding * in_ary->vertical_scale_ratio;
      out_ary->v_init_phase = (int) (temp & q_mask);
      out_ary->src_start_y = (int) (temp >> q_factor) -
        upscale_left_top_padding - in_ary->prescale_padding;

      out_ary->spatial_denoise_crop_height_first_line = in_ary->prescale_padding;
      out_ary->sharpen_crop_height_first_line =
      in_ary->postscale_padding;
    } else {
      /*the topmost block, top padding is done
        internally by denoise and sharpening but not upscaling*/
      out_ary->v_init_phase = (int) (phase_fg.phase_y_cur & q_mask);
      out_ary->src_start_y =
      (int) ((phase_fg.phase_y_cur) >> q_factor) -
      upscale_left_top_padding;
      out_ary->spatial_denoise_crop_height_first_line = 0;
      out_ary->sharpen_crop_height_first_line = 0;
    }
    if (src_tile_index_y_counter == in_ary->frame_height_mcus - 1) {
      /*the bottommost stripe, bottom padding is done
        internally by denoise and sharpening but not upscaler*/
      // src_end_x = floor( phase_x_next - PHASE_X_STEP ) + 2
      out_ary->src_end_y =
      (int) ((phase_fg.phase_y_next -
              in_ary->vertical_scale_ratio) >> q_factor) +
      upscale_right_bottom_padding;
      out_ary->spatial_denoise_crop_height_last_line =
      out_ary->src_end_y - out_ary->src_start_y;
    } else {
      /*not the bottommost stripe, need to fetch more on the bottom*/
      out_ary->src_end_y =
      (int) ((phase_fg.phase_y_next +
              (in_ary->postscale_padding -
               1) * in_ary->vertical_scale_ratio) >> q_factor) +
      upscale_right_bottom_padding + in_ary->prescale_padding;
      out_ary->spatial_denoise_crop_height_last_line =
      out_ary->src_end_y - out_ary->src_start_y -
      in_ary->prescale_padding;
    }
  } else if (in_ary->vertical_scale_ratio > (1 << q_factor)) {
    /*vertical downscaler*/
    out_ary->scale_v_en = 1;
    out_ary->upscale_v_en = 0;
    if (src_tile_index_y_counter) {
      /*not the topmost block, need to fetch more on the top*/
      unsigned long long temp = phase_fg.phase_y_cur -
        in_ary->postscale_padding * in_ary->vertical_scale_ratio;
        out_ary->v_init_phase = (int) (temp & q_mask);
        out_ary->src_start_y = (int) (temp >> q_factor) - in_ary->prescale_padding;

      out_ary->spatial_denoise_crop_height_first_line = in_ary->prescale_padding;
      out_ary->sharpen_crop_height_first_line =
      in_ary->postscale_padding;
    } else {
      /*the topmost block, top padding internally
        done by denoise and sharpening*/
      out_ary->v_init_phase = (int) (phase_fg.phase_y_cur & q_mask);
      out_ary->src_start_y = (int) ((phase_fg.phase_y_cur) >> q_factor);
      out_ary->spatial_denoise_crop_height_first_line = 0;
      out_ary->sharpen_crop_height_first_line = 0;
    }

    if (src_tile_index_y_counter == in_ary->frame_height_mcus - 1) {
      if ((phase_fg.phase_y_next & q_mask) >> (q_factor -
                                               downscale_interpolation_resolution)) {
        out_ary->src_end_y = (int) (phase_fg.phase_y_next >> q_factor);
      } else {
        /*need to fetch one pixel less*/
        out_ary->src_end_y = (int) (phase_fg.phase_y_next >> q_factor) - 1;
      }
      out_ary->spatial_denoise_crop_height_last_line =
      out_ary->src_end_y - out_ary->src_start_y;
    } else {
      /*not the bottommost block, need to fetch more on the bottom*/
      long long temp =
      phase_fg.phase_y_next +
      in_ary->postscale_padding * in_ary->vertical_scale_ratio;
      if ((temp & q_mask) >>
          (q_factor - downscale_interpolation_resolution)) {
        out_ary->src_end_y =
        (int) (temp >> q_factor) + in_ary->prescale_padding;
      } else {
        /*need to fetch one less*/
        out_ary->src_end_y = (int) (temp >> q_factor) - 1 + in_ary->prescale_padding;
      }
      out_ary->spatial_denoise_crop_height_last_line =
      out_ary->src_end_y - out_ary->src_start_y -
      in_ary->prescale_padding;
    }
  } else {
    /*that is, scaling is disabled in the y direction*/
    out_ary->scale_v_en = 0;
    out_ary->upscale_v_en = 0;
    out_ary->v_init_phase = 0;
    if (src_tile_index_y_counter) {
      /*not the topmost block, need to fetch more on top*/
      out_ary->src_start_y =
      (int) (phase_fg.phase_y_cur >> q_factor) -
      in_ary->postscale_padding - in_ary->prescale_padding;
      out_ary->spatial_denoise_crop_height_first_line = in_ary->prescale_padding;
      out_ary->sharpen_crop_height_first_line =
      in_ary->postscale_padding;
    } else {
      /*the topmost block, top padding is done
        in the denoise and sharpening block*/
      out_ary->src_start_y = (int) (phase_fg.phase_y_cur >> q_factor);
      out_ary->spatial_denoise_crop_height_first_line = 0;
      out_ary->sharpen_crop_height_first_line = 0;
    }
    if (src_tile_index_y_counter == in_ary->frame_height_mcus - 1) {
      /*the bottommost block, bottom padding
        by the denoise and sharpening block*/
      out_ary->src_end_y = (int) (phase_fg.phase_y_next >> q_factor) - 1;
      out_ary->spatial_denoise_crop_height_last_line =
      out_ary->src_end_y - out_ary->src_start_y;
    } else {
      /*not the bottommost block, need to fetch more on the bottom*/
      out_ary->src_end_y =
      (int) ((phase_fg.phase_y_next) >> q_factor) - 1 +
      in_ary->postscale_padding + in_ary->prescale_padding;
      out_ary->spatial_denoise_crop_height_last_line =
      out_ary->src_end_y - out_ary->src_start_y -
      in_ary->prescale_padding;
    }
  }
  out_ary->sharpen_crop_height_last_line =
  out_ary->sharpen_crop_height_first_line +
  in_ary->stripe_block_height[src_tile_index_y_counter] - 1;
  /*output height of the scaler should be the write block
    height plus some padding required by sharpening*/
  out_ary->scale_output_height =
  in_ary->stripe_block_height[src_tile_index_y_counter];
  if (src_tile_index_y_counter != 0) {
    out_ary->scale_output_height += in_ary->postscale_padding;
  }
  if (src_tile_index_y_counter != in_ary->frame_height_mcus - 1) {
    out_ary->scale_output_height += in_ary->postscale_padding;
  }

  /*left boundary*/
  if (out_ary->src_start_x < 0) {
    out_ary->pad_left = 0 - out_ary->src_start_x;
    out_ary->src_start_x = 0;
  } else {
    out_ary->pad_left = 0;
  }

  /*top boundary*/
  if (out_ary->src_start_y < 0) {
    out_ary->pad_top = 0 - out_ary->src_start_y;
    out_ary->src_start_y = 0;
  } else {
    out_ary->pad_top = 0;
  }

  /*right boundary*/
  if (out_ary->src_end_x >= in_ary->src_width) {
    out_ary->pad_right = out_ary->src_end_x - (in_ary->src_width - 1);
    out_ary->src_end_x = (in_ary->src_width - 1);
  } else {
    out_ary->pad_right = 0;
  }

  /*bottom boundary*/
  if (out_ary->src_end_y >= in_ary->src_height) {
    out_ary->pad_bottom = out_ary->src_end_y - (in_ary->src_height - 1);
    out_ary->src_end_y = (in_ary->src_height - 1);
  } else {
    out_ary->pad_bottom = 0;
  }
  /*The leftmost point of the fetch block in byte addresses*/
  out_ary->source_address[0] =
  in_ary->source_address[0] + out_ary->src_start_x *
  in_ary->input_bytes_per_pixel + out_ary->src_start_y * in_ary->src_stride;
}

void set_start_of_frame_parameters (struct cpp_plane_info_t *in_ary)
{
  uint32_t i;
  /*determine destination stripe width*/
  uint32_t block_width = LINE_BUFFER_SIZE - 2 * in_ary->postscale_padding;
  uint32_t rotation_block_height;
  uint32_t rotation_block_width;
  if (in_ary->horizontal_scale_ratio < (1 << q_factor)
      || (in_ary->horizontal_scale_ratio == (1 << q_factor)
          && (in_ary->horizontal_scale_initial_phase & q_mask))) {
    /* upscaler is used
       crop out asf pixels, upscale pixels, and denoise pixels */
    long long temp =
    LINE_BUFFER_SIZE - 2 * in_ary->prescale_padding -
    upscale_left_top_padding - upscale_right_bottom_padding;
    temp <<= q_factor;
    temp -= q_mask + 1; /* safety margin */
    /*number of pixels that can be produced by upscaler*/
    temp /= in_ary->horizontal_scale_ratio;
    temp += 1; //safety margin
    temp -= 2 * in_ary->postscale_padding;
    if (temp < block_width) {
      block_width = (int) temp;
    }
  } else { /*downscaler or no scaler*/
    /*crop out asf pixels, and denoise pixels*/
    long long temp = LINE_BUFFER_SIZE - 2 * in_ary->prescale_padding;
    temp <<= q_factor;
    temp -= q_mask; /* safety margin */
    /*number of pixels that can be produced by downscaler*/
    temp /= in_ary->horizontal_scale_ratio;
    temp -= 2 * in_ary->postscale_padding;
    if (temp < block_width) {
      block_width = (int) temp;
    }
  }

  if (in_ary->rotate == 0 || in_ary->rotate == 2)// 0 or 180 degree rotation
  {
    /*rotation block height is 2,
      the destination image height will be a multiple of 2.*/
    rotation_block_height = 2;
  } else { /*90 or 270 degree rotation*/
    /*rotation block height is MAL length (32?).
      The destination image height will be a multiple of MAL.*/
    rotation_block_height = MAL_SIZE;
  }
  rotation_block_width = ROTATION_BUFFER_SIZE / rotation_block_height;

  if (block_width > rotation_block_width) {
    block_width = rotation_block_width;
  }

  in_ary->frame_width_mcus =
    (in_ary->dst_width + block_width - 1) / block_width;
  CPP_STRIPE("MCUS: %d\n", in_ary->frame_width_mcus);

  if ((in_ary->input_plane_fmt == PLANE_Y) && (in_ary->frame_width_mcus % 2)) {
    // try to make number of stripes even for Y plane to run faster
    in_ary->frame_width_mcus++;
  }

  /*evenly distribute the stripe width*/
  in_ary->dst_block_width =
    (in_ary->dst_width + in_ary->frame_width_mcus - 1) /
    in_ary->frame_width_mcus;

  /* number of stripes */
  in_ary->frame_width_mcus =
    (in_ary->dst_width + in_ary->dst_block_width - 1)
    / in_ary->dst_block_width;

  /*destination stripe width*/
  in_ary->stripe_block_width = CAM_MALLOC(sizeof(uint32_t) *
                               in_ary->frame_width_mcus);
  if (!in_ary->stripe_block_width){
      CPP_ERR("CAM_MALLOC() failed");
      return;
  }
  for (i = 0; i < in_ary->frame_width_mcus; i++) {
    /*First assume all the stripe widths are the same,
      one entry will be changed later*/
    in_ary->stripe_block_width[i] = in_ary->dst_block_width;
  }

  /*The actual destination image height, multiple of rotation block height*/
  in_ary->dst_height_block_aligned =
    ((in_ary->dst_height + rotation_block_height - 1) /
     rotation_block_height) * rotation_block_height;

  CPP_STRIPE("Aligned block height: %d", in_ary->dst_height_block_aligned);

  if (in_ary->dst_height_block_aligned > in_ary->maximum_dst_stripe_height) {
    /*Maximum allowed block height is smaller than destination image height*/
    /*do block processing*/
    CPP_STRIPE("Block processing?\n");
    in_ary->dst_block_height =
    (in_ary->maximum_dst_stripe_height / rotation_block_height) *
    rotation_block_height;
  } else {
    /*do stripe processing*/
    in_ary->dst_block_height = in_ary->dst_height_block_aligned;
  }

  /*Number of blocks in the vertical direction*/
  in_ary->frame_height_mcus =
    (in_ary->dst_height_block_aligned + in_ary->dst_block_height - 1) /
    in_ary->dst_block_height;
  CPP_STRIPE("HMCUS: %d\n", in_ary->frame_height_mcus);

  /*destination block height*/
  in_ary->stripe_block_height = CAM_MALLOC(sizeof(uint32_t) *
                                in_ary->frame_height_mcus);
  if (!in_ary->stripe_block_height){
      CPP_ERR("Cannot assign memory to in_ary->stripe_block_height");
      free(in_ary->stripe_block_width);
      return;
  }
  for (i = 0; i < in_ary->frame_height_mcus; i++) {
    /*First assume all the block heights are the same,
      one entry will be changed later*/
    in_ary->stripe_block_height[i] = in_ary->dst_block_height;
  }

  switch (in_ary->rotate) {
  case 1: /* 90 degree */
    if (in_ary->mirror & 0x1) {
      // the bottommost block heights are smaller than others.
      // fetch a little more on the bottom so padding will be on the bottom
      in_ary->stripe_block_height[in_ary->frame_height_mcus - 1] =
        in_ary->dst_height_block_aligned - in_ary->dst_block_height *
        (in_ary->frame_height_mcus - 1);
    } else {
      // the topmost block heights are smaller than others
      // fetch a little more on the top so padding will be on the right
      in_ary->vertical_scale_initial_phase -=
        (in_ary->dst_height_block_aligned - in_ary->dst_height) *
        in_ary->vertical_scale_ratio;
      in_ary->vertical_scale_block_initial_phase =
        in_ary->vertical_scale_initial_phase - (in_ary->frame_height_mcus *
        in_ary->dst_block_height - in_ary->dst_height_block_aligned) *
        in_ary->vertical_scale_ratio;
      in_ary->stripe_block_height[0] =
        in_ary->dst_height_block_aligned - in_ary->dst_block_height *
        (in_ary->frame_height_mcus - 1);
    }
    // the rightmost destination stripe width is smaller than others
    in_ary->stripe_block_width[in_ary->frame_width_mcus - 1] =
      in_ary->dst_width - in_ary->dst_block_width *
      (in_ary->frame_width_mcus - 1);
    break;
  case 2:
    if (in_ary->mirror & 0x1) {
      /*the rightmost destination stripe width is smaller than others*/
      in_ary->stripe_block_width[in_ary->frame_width_mcus - 1] =
      in_ary->dst_width -
      in_ary->dst_block_width * (in_ary->frame_width_mcus - 1);
    } else {
      /*the leftmost destination stripe width is smaller than others*/
      in_ary->horizontal_scale_block_initial_phase -=
      (in_ary->frame_width_mcus * in_ary->dst_block_width -
       in_ary->dst_width) * in_ary->horizontal_scale_ratio;
      in_ary->stripe_block_width[0] =
      in_ary->dst_width -
      in_ary->dst_block_width * (in_ary->frame_width_mcus - 1);
    }
    /*the topmost block heights are smaller than others
      fetch a little more on the top so padding will be on the bottom*/
    in_ary->vertical_scale_initial_phase -=
    (in_ary->dst_height_block_aligned -
     in_ary->dst_height) * in_ary->vertical_scale_ratio;
    in_ary->vertical_scale_block_initial_phase =
    in_ary->vertical_scale_initial_phase -
    (in_ary->frame_height_mcus * in_ary->dst_block_height -
     in_ary->dst_height_block_aligned) * in_ary->vertical_scale_ratio;
    in_ary->stripe_block_height[0] =
    in_ary->dst_height_block_aligned -
    in_ary->dst_block_height * (in_ary->frame_height_mcus - 1);
    break;
  case 3:
    if (in_ary->mirror & 0x1) {
      // the rightmost destination stripe width is smaller than others
      in_ary->vertical_scale_initial_phase -=
        (in_ary->dst_height_block_aligned - in_ary->dst_height) *
        in_ary->vertical_scale_ratio;
      in_ary->vertical_scale_block_initial_phase =
        in_ary->vertical_scale_initial_phase -
        (in_ary->frame_height_mcus * in_ary->dst_block_height -
        in_ary->dst_height_block_aligned) * in_ary->vertical_scale_ratio;
      in_ary->stripe_block_height[0] =
        in_ary->dst_height_block_aligned - in_ary->dst_block_height *
        (in_ary->frame_height_mcus - 1);
    } else {
      //the bottommost block heights are smaller than others.
      //fetch a little more on the bottom so padding will be on the right side
      in_ary->stripe_block_height[in_ary->frame_height_mcus - 1] =
        in_ary->dst_height_block_aligned - in_ary->dst_block_height *
        (in_ary->frame_height_mcus - 1);
    }

    // the leftmost destination stripe width is smaller than others
    in_ary->horizontal_scale_block_initial_phase -=
      (in_ary->frame_width_mcus * in_ary->dst_block_width -
      in_ary->dst_width) * in_ary->horizontal_scale_ratio;
    in_ary->stripe_block_width[0] = in_ary->dst_width -
        in_ary->dst_block_width * (in_ary->frame_width_mcus - 1);
    break;
  case 0:
  default:
    if(in_ary->mirror & 0x1) {
      /*the leftmost destination stripe width is smaller than others*/
      in_ary->horizontal_scale_block_initial_phase -=
      (in_ary->frame_width_mcus * in_ary->dst_block_width -
       in_ary->dst_width) * in_ary->horizontal_scale_ratio;
      in_ary->stripe_block_width[0] =
      in_ary->dst_width -
      in_ary->dst_block_width * (in_ary->frame_width_mcus - 1);
    } else {
      /*the rightmost destination stripe width is smaller than others*/
      in_ary->stripe_block_width[in_ary->frame_width_mcus - 1] =
      in_ary->dst_width -
      in_ary->dst_block_width * (in_ary->frame_width_mcus - 1);
    }
    /*the bottommost block heights are smaller than others.
      fetch a little more on the bottom so padding will be on the bottom*/
    in_ary->stripe_block_height[in_ary->frame_height_mcus - 1] =
    in_ary->dst_height_block_aligned -
    in_ary->dst_block_height * (in_ary->frame_height_mcus - 1);
    in_ary->vertical_scale_block_initial_phase =
    in_ary->vertical_scale_initial_phase;
    break;
  }
  in_ary->num_stripes = (uint32_t)
    in_ary->frame_width_mcus * in_ary->frame_height_mcus;
}

void cpp_init_frame_params (struct cpp_plane_info_t *frame)
{
  frame->horizontal_scale_ratio =
  (uint32_t)(frame->h_scale_ratio * (1 << q_factor));
  if (frame->horizontal_scale_ratio <= 0) /*Assume scaler is off*/
    frame->horizontal_scale_ratio = 1 << q_factor;

  frame->vertical_scale_ratio =
  (uint32_t)(frame->v_scale_ratio * (1 << q_factor));
  if (frame->vertical_scale_ratio <= 0) /*Assume scaler is off*/
    frame->vertical_scale_ratio = 1 << q_factor;

  frame->horizontal_scale_initial_phase =
  (long long) (floor(frame->h_scale_initial_phase * (1 << q_factor)));
  frame->vertical_scale_initial_phase =
  (long long)  (floor(frame->v_scale_initial_phase * (1 << q_factor)));

  frame->horizontal_scale_block_initial_phase =
  frame->horizontal_scale_initial_phase;
  frame->vertical_scale_block_initial_phase =
  frame->vertical_scale_initial_phase;
}

void cpp_debug_input_info(struct cpp_plane_info_t *frame __unused)
{
  if (!(IS_VALID_MASK(g_cpp_log_featureMask, CPP_STRIPE_IP_BIT)))
    return;

  CPP_STRIPE_IP("CPP: src_width %d\n", frame->src_width);
  CPP_STRIPE_IP("CPP: src_height %d\n", frame->src_height);
  CPP_STRIPE_IP("CPP: src_stride %d\n", frame->src_stride);
  CPP_STRIPE_IP("CPP: dst_width %d\n", frame->dst_width);
  CPP_STRIPE_IP("CPP: dst_height %d\n", frame->dst_height);
  CPP_STRIPE_IP("CPP: dst_stride %d\n", frame->dst_stride);
  CPP_STRIPE_IP("CPP: temporal_stride %d\n", frame->temporal_stride);
  CPP_STRIPE_IP("CPP: rotate %d\n", frame->rotate);
  CPP_STRIPE_IP("CPP: mirror %d\n", frame->mirror);
  CPP_STRIPE_IP("CPP: spatial_denoise_padding %d\n", frame->spatial_denoise_padding);

  CPP_STRIPE_IP("CPP: prescaler_spatial_denoise_padding %d\n",
    frame->prescaler_spatial_denoise_padding);
  CPP_STRIPE_IP("CPP: temporal_denoise_padding %d\n", frame->temporal_denoise_padding);
  CPP_STRIPE_IP("CPP: sharpen_padding %d\n", frame->sharpen_padding);
  CPP_STRIPE_IP("CPP: horizontal_scale_ratio %d\n", frame->horizontal_scale_ratio);
  CPP_STRIPE_IP("CPP: vertical_scale_ratio %d\n", frame->vertical_scale_ratio);
  CPP_STRIPE_IP("CPP: horizontal_scale_initial_phase %lld\n",
       frame->horizontal_scale_initial_phase);
  CPP_STRIPE_IP("CPP: vertical_scale_initial_phase %lld\n",
       frame->vertical_scale_initial_phase);
  CPP_STRIPE_IP("CPP: maximum_dst_stripe_height %d\n",
    frame->maximum_dst_stripe_height);
  CPP_STRIPE_IP("CPP: input_plane_fmt %d\n", frame->input_plane_fmt);
  CPP_STRIPE_IP("CPP: source_address 0x%x\n", frame->source_address[0]);

  CPP_STRIPE_IP("CPP: destination_address[0] 0x%x\n", frame->destination_address[0]);
  CPP_STRIPE_IP("CPP: destination_address[1] 0x%x\n", frame->destination_address[1]);
  CPP_STRIPE_IP("CPP: temporal_source_address 0x%x\n",
    frame->temporal_source_address[0]);
  CPP_STRIPE_IP("CPP: temporal_destination_address 0x%x\n",
    frame->temporal_destination_address[0]);
  CPP_STRIPE_IP("CPP: is_not_y_plane 0x%x\n", frame->is_not_y_plane);
  CPP_STRIPE_IP("CPP: denoise_after_scale_en 0x%x\n", frame->denoise_after_scale_en);
  CPP_STRIPE_IP("CPP: frame_width_mcus %d\n", frame->frame_width_mcus);
  CPP_STRIPE_IP("CPP: frame_height_mcus %d\n", frame->frame_height_mcus);
  CPP_STRIPE_IP("CPP: dst_height_block_aligned %d\n", frame->dst_height_block_aligned);
  CPP_STRIPE_IP("CPP: dst_block_width %d\n", frame->dst_block_width);
  CPP_STRIPE_IP("CPP: dst_block_height %d\n", frame->dst_block_height);
  CPP_STRIPE_IP("CPP: stripe_block_width %d\n", frame->stripe_block_width[0]);
  CPP_STRIPE_IP("CPP: stripe_block_height %d\n", frame->stripe_block_height[0]);

  CPP_STRIPE_IP("CPP: horizontal_scale_block_initial_phase %lld\n",
       frame->horizontal_scale_block_initial_phase);
  CPP_STRIPE_IP("CPP: vertical_scale_block_initial_phase %lld\n",
       frame->vertical_scale_block_initial_phase);
  CPP_STRIPE_IP("CPP: postscale_padding %d\n", frame->postscale_padding);
  CPP_STRIPE_IP("CPP: prescale_padding %d\n", frame->prescale_padding);
  CPP_STRIPE_IP("CPP: state_padding %d\n", frame->state_padding);
  CPP_STRIPE_IP("CPP: tile_output_enable %d\n", frame->tile_output_enable);
  CPP_STRIPE_IP("CPP: tile_byte_width %d\n", frame->tile_byte_width);
  CPP_STRIPE_IP("CPP: tile_byte_height %d\n", frame->tile_byte_height);
  CPP_STRIPE_IP("CPP: dst_width_block_aligned %d\n", frame->dst_width_block_aligned);
  CPP_STRIPE_IP("CPP: rotation_buffer_size %d\n", frame->rotation_buffer_size);
  CPP_STRIPE_IP("CPP: mal_byte_size %d\n", frame->mal_byte_size);
  CPP_STRIPE_IP("CPP: line_buffer_size %d\n", frame->line_buffer_size);
  CPP_STRIPE_IP("CPP: prescaler_padding %d\n", frame->prescaler_padding);
  CPP_STRIPE_IP("CPP: metadata_stride %d\n", frame->metadata_stride);
  CPP_STRIPE_IP("CPP: metadata_offset %d\n", frame->metadata_offset);
  CPP_STRIPE_IP("CPP: framedata_offset %d\n", frame->framedata_offset);
}

static void enable_crop_engines(struct cpp_plane_info_t *in_info,
  struct cpp_striping_algorithm_info *cur_strip_info)
{
  if(in_info->prescale_padding == 0 ||
      (in_info->frame_width_mcus == 1 && in_info->frame_height_mcus == 1)) {
     cur_strip_info->spatial_denoise_crop_en = 0;
     in_info->bf_crop_enable = 0;
   } else {
     cur_strip_info->spatial_denoise_crop_en = 1;
     in_info->bf_crop_enable = 1;
   }

   if(in_info->postscale_padding == 0 ||
      (in_info->frame_width_mcus == 1 && in_info->frame_height_mcus == 1)) {
     cur_strip_info->sharpen_crop_en = 0;
     in_info->asf_crop_enable = 0;
   } else {
     cur_strip_info->sharpen_crop_en = 1;
     in_info->asf_crop_enable = 1;
   }
}

void cpp_init_strip_info(cpp_hardware_t *cpphw,
  struct cpp_plane_info_t *in_info,
  struct cpp_striping_algorithm_info *stripe_info, uint32_t num_stripes)
{
  struct cpp_striping_algorithm_info *cur_strip_info;
  uint32_t i;

  CPP_FRAME_DBG("num_stripes %d", num_stripes);
  for (i = 0; i < num_stripes; i++) {
    cur_strip_info = &stripe_info[i];
    if(cpphw->hwinfo.version == CPP_HW_VERSION_5_0_0 ||
       cpphw->hwinfo.version == CPP_HW_VERSION_5_1_0 ||
       cpphw->hwinfo.version == CPP_HW_VERSION_6_0_0 ||
       cpphw->hwinfo.version == CPP_HW_VERSION_6_1_0 ||
       cpphw->hwinfo.version == CPP_HW_VERSION_6_1_2 ||
       cpphw->hwinfo.version == CPP_HW_VERSION_6_1_4) {
      if (in_info->frame_width_mcus == 1 && in_info->frame_height_mcus == 1 &&
        !in_info->pad_boundary) {
        in_info->prescaler_spatial_denoise_padding = 0;
        in_info->temporal_denoise_padding = 0;
        in_info->spatial_denoise_padding = 0;
        in_info->sharpen_padding = 0;
        in_info->state_padding = 0;
      }
      enable_crop_engines_1_8(in_info,cur_strip_info);
    } else {
      enable_crop_engines(in_info,cur_strip_info);
    }
    cur_strip_info->output_bytes_per_pixel =
      in_info->output_bytes_per_pixel;
    cur_strip_info->input_bytes_per_pixel =
      in_info->input_bytes_per_pixel;
    cur_strip_info->src_stride = in_info->src_stride;
    cur_strip_info->dst_stride = in_info->dst_stride;
    cur_strip_info->temporal_stride = in_info->temporal_stride;

    switch (in_info->rotate) {
    case 1:
      if (in_info->mirror & 0x1) {
        cur_strip_info->horizontal_flip = 0;
      } else {
        cur_strip_info->horizontal_flip = 1;
      }
      cur_strip_info->rotate_270 = 1;
      cur_strip_info->vertical_flip = 1;
      break;
    case 2:
      if (in_info->mirror & 0x1) {
        cur_strip_info->horizontal_flip = 0;
      } else {
        cur_strip_info->horizontal_flip = 1;
      }
      cur_strip_info->rotate_270 = 0;
      cur_strip_info->vertical_flip = 1;
      break;
    case 3:
      if (in_info->mirror & 0x1) {
        cur_strip_info->horizontal_flip = 1;
      } else {
        cur_strip_info->horizontal_flip = 0;
      }
      cur_strip_info->rotate_270 = 1;
      cur_strip_info->vertical_flip = 0;
      break;
    case 0:
    default:
      cur_strip_info->horizontal_flip = in_info->mirror & 0x1;
      cur_strip_info->vertical_flip = in_info->mirror >> 1;
      cur_strip_info->rotate_270 = 0;
      cur_strip_info->vertical_flip = 0;
      break;
    }
  }
}

void cpp_debug_strip_info(struct cpp_striping_algorithm_info *strip_info __unused,
  uint32_t n __unused)
{
  if (!(IS_VALID_MASK(g_cpp_log_featureMask, CPP_STRIPE_BIT)))
    return;
  CPP_STRIPE("CPP FRAME STRIP %d", n);

  /*Is vertical scale enabled?*/
  CPP_STRIPE("CPP scale_v_en %d\n", strip_info->scale_v_en);
  /*Is horizontal scale enabled?*/
  CPP_STRIPE("CPP scale_h_en %d\n", strip_info->scale_h_en);
  /*Leftmost position of the fetch block in terms of pixels*/
  CPP_STRIPE("CPP src_start_x %d\n", strip_info->src_start_x);
  /*Rightmost position of the fetch block in terms of pixels*/
  CPP_STRIPE("CPP src_end_x %d\n", strip_info->src_end_x);
  /*Topmost position of the fetch block in terms of pixels*/
  CPP_STRIPE("CPP src_start_y %d\n", strip_info->src_start_y);
  /*Bottommost positin of the fetch block in terms of pixels*/
  CPP_STRIPE("CPP src_end_y %d\n", strip_info->src_end_y);
  /*The amount padded at the fetch block left boundary in terms of pixels*/
  CPP_STRIPE("CPP pad_left %d\n", strip_info->pad_left);
  /*The amount padded at the fetch block right boundary in terms of pixels*/
  CPP_STRIPE("CPP pad_right %d\n", strip_info->pad_right);
  /*The amount padded at the fetch block top boundary in terms of pixels*/
  CPP_STRIPE("CPP pad_top %d\n", strip_info->pad_top);
  /*The amount padded at the fetch block bottom boundary in terms of pixels*/
  CPP_STRIPE("CPP pad_bottom %d\n", strip_info->pad_bottom);
  /*Leftmost sampling position, range (0, 1)*/
  CPP_STRIPE("CPP h_init_phase %d\n", strip_info->h_init_phase);
  /*Topmost sampling position, range (0, 1)*/
  CPP_STRIPE("CPP v_init_phase %d\n", strip_info->v_init_phase);
  /*Horizontal sampling distance, Q21 number*/
  CPP_STRIPE("CPP h_phase_step %d\n", strip_info->h_phase_step);
  /*Vertical sampling distance, Q21 number*/
  CPP_STRIPE("CPP v_phase_step %d\n", strip_info->v_phase_step);
  /*Leftmost position of the cropping window right after denoise block*/
  CPP_STRIPE("CPP spatial_denoise_crop_width_first_pixel 0x%x\n",
    strip_info->spatial_denoise_crop_width_first_pixel);
  /*Rightmost position of the cropping window right after denoise block*/
  CPP_STRIPE("CPP spatial_denoise_crop_width_last_pixel 0x%x\n",
    strip_info->spatial_denoise_crop_width_last_pixel);
  /*Topmost position of the cropping window right after denoise block*/
  CPP_STRIPE("CPP spatial_denoise_crop_height_first_line 0x%x\n",
    strip_info->spatial_denoise_crop_height_first_line);
  /*Bottommost position of the cropping window right after denoise block*/
  CPP_STRIPE("CPP spatial_denoise_crop_height_last_line 0x%x\n",
    strip_info->spatial_denoise_crop_height_last_line);
  /*Leftmostposition of the cropping window right after sharpening block*/
  CPP_STRIPE("CPP sharpen_crop_width_first_pixel 0x%x\n",
    strip_info->sharpen_crop_width_first_pixel);
  /*Rightmost position of the cropping window right after sharpening block*/
  CPP_STRIPE("CPP sharpen_crop_width_last_pixel 0x%x\n",
    strip_info->sharpen_crop_width_last_pixel);
  /*Topmost position of the cropping window right after denoise block*/
  CPP_STRIPE("CPP sharpen_crop_height_first_line 0x%x\n",
    strip_info->sharpen_crop_height_first_line);
  /*Bottommost position of the cropping window right after denoise block*/
  CPP_STRIPE("CPP sharpen_crop_height_last_line 0x%x\n",
    strip_info->sharpen_crop_height_last_line);
  /*Leftmost position of the write block in terms of pixels*/
  CPP_STRIPE("CPP dst_start_x %d\n", strip_info->dst_start_x);
  /*Rightmost position of the write block in terms of pixels*/
  CPP_STRIPE("CPP dst_end_x %d\n", strip_info->dst_end_x);
  /*Topmost position of the write block in terms of pixels*/
  CPP_STRIPE("CPP dst_start_y %d\n", strip_info->dst_start_y);
  /*Bottommost position of the write block in terms of pixels*/
  CPP_STRIPE("CPP dst_end_y %d\n", strip_info->dst_end_y);
  /*1-> Planar, 2->CbCr*/
  CPP_STRIPE("CPP input bytes_per_pixel %d\n", strip_info->input_bytes_per_pixel);
  CPP_STRIPE("CPP output bytes_per_pixel %d\n", strip_info->output_bytes_per_pixel);
  /*Topleft corner of the fetch block in terms of memory address*/
  CPP_STRIPE("CPP source_address 0x%x\n", strip_info->source_address[0]);
  /*Topleft corner of the write block in terms of memory address*/
  CPP_STRIPE("CPP destination_address[0] 0x%x\n",
    strip_info->destination_address[0]);
  /*Topleft corner of the write block in terms of memory address in planar frames*/
  CPP_STRIPE("CPP destination_address[1] 0x%x\n",
    strip_info->destination_address[1]);
  /*Source image stride in terms of bytes*/
  CPP_STRIPE("CPP src_stride %d\n", strip_info->src_stride);
  /*Destination image stride in terms of bytes*/
  CPP_STRIPE("CPP dst_stride %d\n", strip_info->dst_stride);
  /*Do we rotate 270 degree or not?*/
  CPP_STRIPE("CPP rotate_270 %d\n", strip_info->rotate_270);
  /*Do we follow rotation with horizontal flip?*/
  CPP_STRIPE("CPP horizontal_flip %d\n", strip_info->horizontal_flip);
  /*Do we follow rotation with vertical flip?*/
  CPP_STRIPE("CPP vertical_flip %d\n", strip_info->vertical_flip);
  /*Scaler output width*/
  CPP_STRIPE("CPP scale_output_width %d\n", strip_info->scale_output_width);
  /*Scaler output height*/
  CPP_STRIPE("CPP scale_output_height %d\n", strip_info->scale_output_height);
  /*Is vertical upscale enabled?*/
  CPP_STRIPE("CPP upscale_v_en %d\n", strip_info->upscale_v_en);
  /*Is horizontal upscale enabled?*/
  CPP_STRIPE("CPP upscale_h_en %d\n", strip_info->upscale_h_en);
}

void cpp_debug_fetch_engine_info(struct cpp_fe_info *fe_info __unused)
{
  if (!(IS_VALID_MASK(g_cpp_log_featureMask, CPP_STRIPE_FE_BIT)))
    return;

  CPP_STRIPE_FE("buffer_ptr   : %d\n", fe_info->buffer_ptr);
  CPP_STRIPE_FE("buffer_width : %d\n", fe_info->buffer_width);
  CPP_STRIPE_FE("buffer_height: %d\n", fe_info->buffer_height);
  CPP_STRIPE_FE("buffer_stride: %d\n", fe_info->buffer_stride);
  CPP_STRIPE_FE("block_width  : %d\n", fe_info->block_width);
  CPP_STRIPE_FE("block_height : %d\n", fe_info->block_height);
  CPP_STRIPE_FE("left_pad     : %d\n", fe_info->left_pad);
  CPP_STRIPE_FE("right_pad    : %d\n", fe_info->right_pad);
  CPP_STRIPE_FE("top_pad      : %d\n", fe_info->top_pad);
  CPP_STRIPE_FE("bottom_pad   : %d\n", fe_info->bottom_pad);
}

void cpp_prepare_fetch_engine_info(struct cpp_plane_info_t *plane_info,
  uint32_t stripe_num)
{
  struct cpp_stripe_info *stripe_info1 =
    &plane_info->stripe_info1[stripe_num];
  struct cpp_striping_algorithm_info *stripe_info =
    &plane_info->stripe_info[stripe_num];
  struct cpp_fe_info *fe_info = &stripe_info1->fe_info;

  fe_info->buffer_ptr = stripe_info->source_address[0];
  fe_info->buffer_width =
    stripe_info->src_end_x - stripe_info->src_start_x + 1;
  fe_info->buffer_height =
    stripe_info->src_end_y - stripe_info->src_start_y + 1;
  fe_info->buffer_stride = stripe_info->src_stride;
  fe_info->left_pad = stripe_info->pad_left;
  fe_info->right_pad = stripe_info->pad_right;
  fe_info->top_pad = stripe_info->pad_top;
  fe_info->bottom_pad = stripe_info->pad_bottom;
  /*
  if (plane_info->rotate == 1 || plane_info->rotate == 3) {
    if (plane_info->mirror & 0x1) {
      fe_info->top_pad = stripe_info->pad_bottom;
      fe_info->bottom_pad = stripe_info->pad_top;
    } else {
      fe_info->top_pad = stripe_info->pad_top;
      fe_info->bottom_pad = stripe_info->pad_bottom;
    }
  }*/
/*
  if (plane_info->input_plane_fmt == PLANE_Y) {
    fe_info->top_pad = 16;
  } else {
    fe_info->top_pad = 8;
  }
  fe_info->bottom_pad = 0;
*/
  fe_info->block_width =
    fe_info->buffer_width + fe_info->left_pad + fe_info->right_pad;
  fe_info->block_height =
    fe_info->buffer_height + fe_info->top_pad + fe_info->bottom_pad;
  cpp_debug_fetch_engine_info(fe_info);
}

void cpp_debug_bf_info(struct cpp_bf_info *bf_info __unused)
{
  uint32_t i;

  for (i = 0; i < 4; i++) {
    //Q10 and Q8 format for bilateral_scale is calculated in cpp_prepare_bf_info()
    CPP_STRIPE_WNR("bilateral_scale(bf cfg0)[%d]: %f\n",
      i, bf_info->bilateral_scale[i]);
    CPP_STRIPE_WNR("noise_threshold[%d]: %f\n", i, bf_info->noise_threshold[i]);
    CPP_STRIPE_WNR("weight[%d]: %f\n", i, bf_info->weight[i]);
    CPP_STRIPE_WNR("bf cfg1: 0x%x\n", (uint32_t)
         ((uint32_t)(bf_info->noise_threshold[i] * (1 << 4))) << 8 |
         ((uint32_t)(bf_info->weight[i] * (1 << 4))));
  }
}

void cpp_prepare_bf_info(struct cpp_frame_info_t *frame_info)
{
  uint32_t i, j;
  struct cpp_bf_info *bf_info;

  for (j = 0; j < CPP_DENOISE_NUM_PLANES; j++) {
    bf_info = &frame_info->bf_info[j];
    for (i = 0; i < CPP_DENOISE_NUM_PROFILES; i++) {
      frame_info->noise_profile[j][i] *= frame_info->profile_adj[j][i];
      frame_info->weight[j][i] *= frame_info->weight_VFE_adj[j][i];
    }
  }
  for (j = 0; j < CPP_DENOISE_NUM_PLANES; j++) {
    bf_info = &frame_info->bf_info[j];
    for (i = 0; i < BILITERAL_LAYERS; i++) {
      if (i != 3) {
        bf_info->bilateral_scale[i] = ((double) 64.0/3.0) /
          (sqrt((double) frame_info->edge_softness[j][i] / 1.31) *
          frame_info->noise_profile[j][i]) / 9;
      } else {
        bf_info->bilateral_scale[i] = ((double) 64.0/3.0) /
          (sqrt((double) frame_info->edge_softness[j][i] / 1.31) *
          frame_info->noise_profile[j][i]);
      }
    }
    for (i = 0; i < CPP_DENOISE_NUM_PROFILES; i++) {
      bf_info->weight[i] = 1.0 - frame_info->weight[j][i];
      bf_info->noise_threshold[i] = frame_info->denoise_ratio[j][i] *
        frame_info->noise_profile[j][i];
    }
    cpp_debug_bf_info(bf_info);
  }
}

void cpp_debug_asf_info(struct cpp_asf_info *asf_info __unused)
{
  int i;

  if (!(IS_VALID_MASK(g_cpp_log_featureMask, CPP_STRIPE_ASF_BIT)))
    return;

  CPP_STRIPE_ASF("sp: %f\n", asf_info->sp);
  CPP_STRIPE_ASF("neg_abs_y1: %d\n", asf_info->neg_abs_y1);
  CPP_STRIPE_ASF("dyna_clamp_en: %d\n", asf_info->dyna_clamp_en);
  CPP_STRIPE_ASF("sp_eff_en: %d\n", asf_info->sp_eff_en);
  CPP_STRIPE_ASF("clamp_h_ul: %d\n", asf_info->clamp_h_ul);
  CPP_STRIPE_ASF("clamp_h_ll: %d\n", asf_info->clamp_h_ll);
  CPP_STRIPE_ASF("clamp_v_ul: %d\n", asf_info->clamp_v_ul);
  CPP_STRIPE_ASF("clamp_v_ll: %d\n", asf_info->clamp_v_ll);
  CPP_STRIPE_ASF("clamp_offset_max: %d\n", asf_info->clamp_offset_max);
  CPP_STRIPE_ASF("clamp_offset_min: %d\n", asf_info->clamp_offset_min);
  CPP_STRIPE_ASF("clamp_scale_max: %f\n", asf_info->clamp_scale_max);
  CPP_STRIPE_ASF("clamp_scale_min: %f\n", asf_info->clamp_scale_min);
  CPP_STRIPE_ASF("nz_flag: %d\n", asf_info->nz_flag);
  CPP_STRIPE_ASF("nz_flag_f2: %d\n", asf_info->nz_flag_f2);
  CPP_STRIPE_ASF("nz_flag_f3_f4: %d\n", asf_info->nz_flag_f3_f5);
  CPP_STRIPE_ASF("sobel_h_coeff\n");
  for (i = 0; i < 4; i++) {
    CPP_STRIPE_ASF("%f %f %f %f\n",
         asf_info->sobel_h_coeff[i*4], asf_info->sobel_h_coeff[i*4+1],
         asf_info->sobel_h_coeff[i*4+2], asf_info->sobel_h_coeff[i*4+3]);
  }
  CPP_STRIPE_ASF("sobel_v_coeff\n");
  for (i = 0; i < 4; i++) {
    CPP_STRIPE_ASF("%f %f %f %f\n",
         asf_info->sobel_v_coeff[i*4], asf_info->sobel_v_coeff[i*4+1],
         asf_info->sobel_v_coeff[i*4+2], asf_info->sobel_v_coeff[i*4+3]);
  }
  CPP_STRIPE_ASF("hpf_h_coeff\n");
  for (i = 0; i < 4; i++) {
    CPP_STRIPE_ASF("%f %f %f %f\n",
         asf_info->hpf_h_coeff[i*4], asf_info->hpf_h_coeff[i*4+1],
         asf_info->hpf_h_coeff[i*4+2], asf_info->hpf_h_coeff[i*4+3]);
  }
  CPP_STRIPE_ASF("hpf_v_coeff\n");
  for (i = 0; i < 4; i++) {
    CPP_STRIPE_ASF("%f %f %f %f\n",
         asf_info->hpf_v_coeff[i*4], asf_info->hpf_v_coeff[i*4+1],
         asf_info->hpf_v_coeff[i*4+2], asf_info->hpf_v_coeff[i*4+3]);
  }
  CPP_STRIPE_ASF("lpf_coeff\n");
  for (i = 0; i < 4; i++) {
    CPP_STRIPE_ASF("%f %f %f %f\n",
         asf_info->lpf_coeff[i*4], asf_info->lpf_coeff[i*4+1],
         asf_info->lpf_coeff[i*4+2], asf_info->lpf_coeff[i*4+3]);
  }
  CPP_STRIPE_ASF("lut1\n");
  for (i = 0; i < 6; i++) {
    CPP_STRIPE_ASF("%f %f %f %f\n",
         asf_info->lut1[i*4], asf_info->lut1[i*4+1],
         asf_info->lut1[i*4+2], asf_info->lut1[i*4+3]);
  }
  CPP_STRIPE_ASF("lut2\n");
    for (i = 0; i < 6; i++) {
      CPP_STRIPE_ASF("%f %f %f %f\n",
         asf_info->lut2[i*4], asf_info->lut2[i*4+1],
         asf_info->lut2[i*4+2], asf_info->lut2[i*4+3]);
  }
  CPP_STRIPE_ASF("lut3\n");
  for (i = 0; i < 3; i++) {
    CPP_STRIPE_ASF("%f %f %f %f\n",
         asf_info->lut3[i*4], asf_info->lut3[i*4+1],
         asf_info->lut3[i*4+2], asf_info->lut3[i*4+3]);
  }
}

void cpp_debug_stripe_scale_info(struct cpp_stripe_scale_info *scale_info __unused)
{

  if (!(IS_VALID_MASK(g_cpp_log_featureMask, CPP_STRIPE_SCALE_BIT)))
    return;

  CPP_STRIPE_SCALE("block_width: %d\n", scale_info->block_width);
  CPP_STRIPE_SCALE("block_height: %d\n", scale_info->block_height);
  CPP_STRIPE_SCALE("phase_h_init: %d\n", scale_info->phase_h_init);
}

void cpp_prepare_stripe_scale_info(struct cpp_plane_info_t *plane_info,
  uint32_t stripe_num)
{
  struct cpp_stripe_info *stripe_info1 =
    &plane_info->stripe_info1[stripe_num];
  struct cpp_striping_algorithm_info *stripe_info =
    &plane_info->stripe_info[stripe_num];
  struct cpp_stripe_scale_info *stripe_scale_info = &stripe_info1->scale_info;

/*
  if (stripe_num == 0) {
    stripe_scale_info->block_width =
      stripe_info->sharpen_crop_width_last_pixel + 1;
    stripe_scale_info->block_width = 480;
  } else if (stripe_num == (plane_info->num_stripes - 1)) {
    stripe_scale_info->block_width = stripe_info->scale_output_width;
  } else {
    stripe_scale_info->block_width =
    ((stripe_info->sharpen_crop_width_last_pixel -
     stripe_info->sharpen_crop_width_first_pixel + 1) *
    ((double) (1 << q_factor) / stripe_info->h_phase_step));
    if (stripe_scale_info->block_width > 512) {
      stripe_scale_info->block_width = 512;
    }
    stripe_scale_info->block_width = 489;
  }

  stripe_scale_info->block_width = stripe_info->scale_output_width;

  ((double)(((stripe_info->spatial_denoise_crop_width_last_pixel -
   stripe_info->spatial_denoise_crop_width_first_pixel + 1 - 4) << 21)
   - stripe_info->h_init_phase) / stripe_info->h_phase_step) + 1);

  if (stripe_num != 0 && stripe_num != (plane_info->num_stripes - 1)) {
    stripe_scale_info->block_width = (int)
      ((double)(((stripe_info->spatial_denoise_crop_width_last_pixel -
     stripe_info->spatial_denoise_crop_width_first_pixel + 1 - 4) << 21)
     - stripe_info->h_init_phase) / stripe_info->h_phase_step) + 1;
    stripe_scale_info->block_width = 208;
  }
*/

/*
  stripe_scale_info->block_width = (int)
    ((double)(((stripe_info->spatial_denoise_crop_width_last_pixel -
   stripe_info->spatial_denoise_crop_width_first_pixel + 1 - 4) << 21)
   - stripe_info->h_init_phase) / stripe_info->h_phase_step) + 1;
*/

/*
  stripe_scale_info->block_width = 480;
  if (stripe_num > 0) {
    stripe_scale_info->block_width = 512;
  }
  if (stripe_num == 3) {
    stripe_scale_info->block_width = 484;
  }
*/
//    stripe_info->sharpen_crop_width_last_pixel + 1;
//    (stripe_info->sharpen_crop_width_last_pixel -
//     stripe_info->sharpen_crop_width_first_pixel + 1);// *
//    ((double) (1 << q_factor) / stripe_info->h_phase_step)) - 1;
/*
  stripe_scale_info->block_height =
    ((double)(stripe_info->spatial_denoise_crop_height_last_line -
     stripe_info->spatial_denoise_crop_height_first_line + 1) *
    ((double) (1 << q_factor) / stripe_info->v_phase_step));
*/
  stripe_scale_info->block_width = stripe_info->scale_output_width;
  stripe_scale_info->block_height = stripe_info->scale_output_height;
  stripe_scale_info->phase_h_init = stripe_info->h_init_phase;
  cpp_debug_stripe_scale_info(stripe_scale_info);
}

void cpp_debug_plane_scale_info(struct cpp_plane_scale_info *scale_info __unused)
{

  if (!(IS_VALID_MASK(g_cpp_log_featureMask, CPP_STRIPE_SCALE_BIT)))
    return;

  CPP_STRIPE_SCALE("v_scale_fir_algo: %d\n", scale_info->v_scale_fir_algo);
  CPP_STRIPE_SCALE("h_scale_fir_algo: %d\n", scale_info->h_scale_fir_algo);
  CPP_STRIPE_SCALE("v_scale_algo: %d\n", scale_info->v_scale_algo);
  CPP_STRIPE_SCALE("h_scale_algo: %d\n", scale_info->h_scale_algo);
  CPP_STRIPE_SCALE("subsample_en: %d\n", scale_info->subsample_en);
  CPP_STRIPE_SCALE("upsample_en: %d\n", scale_info->upsample_en);
  CPP_STRIPE_SCALE("vscale_en: %d\n", scale_info->vscale_en);
  CPP_STRIPE_SCALE("hscale_en: %d\n", scale_info->hscale_en);
  CPP_STRIPE_SCALE("phase_h_step: %d\n", scale_info->phase_h_step);
  CPP_STRIPE_SCALE("phase_v_init: %d\n", scale_info->phase_v_init);
  CPP_STRIPE_SCALE("phase_v_step: %d\n", scale_info->phase_v_step);
}

void cpp_prepare_plane_scale_info(struct cpp_plane_info_t *plane_info)
{
  struct cpp_stripe_info *stripe_info1 =
    &plane_info->stripe_info1[0];
  struct cpp_striping_algorithm_info *stripe_info =
    &plane_info->stripe_info[0];
  struct cpp_plane_scale_info *scale_info = &plane_info->scale_info;

  scale_info->vscale_en = stripe_info->scale_v_en;
  scale_info->hscale_en = stripe_info->scale_h_en;

  scale_info->v_scale_algo = stripe_info->upscale_v_en;
  scale_info->h_scale_algo = stripe_info->upscale_h_en;

  scale_info->v_scale_fir_algo = 1;
  scale_info->h_scale_fir_algo = 1;

  scale_info->subsample_en = 1;
  scale_info->upsample_en = 1;

  scale_info->phase_h_step = stripe_info->h_phase_step;
  scale_info->phase_v_init = stripe_info->v_init_phase;
  scale_info->phase_v_step = stripe_info->v_phase_step;
  cpp_debug_plane_scale_info(scale_info);
}

void cpp_debug_crop_info(struct cpp_crop_info *crop_info __unused)
{
  if (!(IS_VALID_MASK(g_cpp_log_featureMask, CPP_STRIPE_SCALE_BIT)))
    return;

  CPP_STRIPE_SCALE("enable: %d\n", crop_info->enable);
  CPP_STRIPE_SCALE("first_pixel: %d\n", crop_info->first_pixel);
  CPP_STRIPE_SCALE("last_pixel: %d\n", crop_info->last_pixel);
  CPP_STRIPE_SCALE("first_line: %d\n", crop_info->first_line);
  CPP_STRIPE_SCALE("last_line: %d\n", crop_info->last_line);
}

void cpp_prepare_crop_info(struct cpp_plane_info_t *plane_info,
  uint32_t stripe_num)
{
  struct cpp_stripe_info *stripe_info1 =
    &plane_info->stripe_info1[stripe_num];
  struct cpp_striping_algorithm_info *stripe_info =
    &plane_info->stripe_info[stripe_num];
  struct cpp_crop_info *bf_crop_info = &stripe_info1->bf_crop_info;
  struct cpp_crop_info *asf_crop_info = &stripe_info1->asf_crop_info;

  bf_crop_info->enable = stripe_info->spatial_denoise_crop_en;
  bf_crop_info->first_pixel = stripe_info->spatial_denoise_crop_width_first_pixel;
  bf_crop_info->last_pixel = stripe_info->spatial_denoise_crop_width_last_pixel;
  bf_crop_info->first_line = stripe_info->spatial_denoise_crop_height_first_line;
  bf_crop_info->last_line = stripe_info->spatial_denoise_crop_height_last_line;

  asf_crop_info->enable = stripe_info->sharpen_crop_en;
  asf_crop_info->first_pixel = stripe_info->sharpen_crop_width_first_pixel;
  asf_crop_info->last_pixel = stripe_info->sharpen_crop_width_last_pixel;
  asf_crop_info->first_line = stripe_info->sharpen_crop_height_first_line;
  asf_crop_info->last_line = stripe_info->sharpen_crop_height_last_line;

  CPP_STRIPE_SCALE("BF Crop info\n");
  cpp_debug_crop_info(bf_crop_info);
  CPP_STRIPE_SCALE("ASF Crop info\n");
  cpp_debug_crop_info(asf_crop_info);
}

void cpp_debug_rotation_info(struct cpp_rotator_info *rot_info __unused)
{
  if (!(IS_VALID_MASK(g_cpp_log_featureMask, CPP_STRIPE_ROT_BIT)))
    return;

  CPP_STRIPE_ROT("rot_cfg: %d\n", rot_info->rot_cfg);
  CPP_STRIPE_ROT("block_width: %d\n", rot_info->block_width);
  CPP_STRIPE_ROT("block_height: %d\n", rot_info->block_height);
  CPP_STRIPE_ROT("block_size: %d\n", rot_info->block_size);
  CPP_STRIPE_ROT("rowIndex0: %d\n", rot_info->rowIndex0);
  CPP_STRIPE_ROT("rowIndex1: %d\n", rot_info->rowIndex1);
  CPP_STRIPE_ROT("colIndex0: %d\n", rot_info->colIndex0);
  CPP_STRIPE_ROT("colIndex1: %d\n", rot_info->colIndex1);
  CPP_STRIPE_ROT("initIndex: %d\n", rot_info->initIndex);
  CPP_STRIPE_ROT("modValue: %d\n", rot_info->modValue);
  CPP_STRIPE_ROT("tile_width: %d\n", rot_info->tile_width);
  CPP_STRIPE_ROT("tile_height: %d\n", rot_info->tile_height);
  CPP_STRIPE_ROT("tile_size: %d\n", rot_info->tile_size);
  CPP_STRIPE_ROT("tile_voffset: %d\n", rot_info->tile_voffset);
  CPP_STRIPE_ROT("blocks_per_stripe: %d\n", rot_info->blocks_per_stripe);
}

static void set_default_crop_padding(struct cpp_plane_info_t *plane_info)
{
  plane_info->prescale_padding = 22;//org
  if (plane_info->input_plane_fmt == PLANE_Y)
    plane_info->postscale_padding = 4;//org
  else
    plane_info->postscale_padding = 0;
}

void cpp_prepare_rotation_info(struct cpp_plane_info_t *plane_info,
  uint32_t stripe_num)
{
  struct cpp_stripe_info *stripe_info =
    &plane_info->stripe_info1[stripe_num];

  struct cpp_striping_algorithm_info *frame_strip_info =
    &plane_info->stripe_info[stripe_num];

  uint16_t block_width, block_height;
  uint32_t block_size, output_block_width = 0;
  struct cpp_rotator_info *rot_info = &stripe_info->rot_info;
  struct cpp_stripe_scale_info *stripe_scale_info = &stripe_info->scale_info;

  /* Rotation config depends on scaler position */
  if (plane_info->sharpen_before_scale) {
    rot_info->block_width = stripe_scale_info->block_width;
  } else {
    rot_info->block_width = stripe_info->asf_crop_info.last_pixel -
      stripe_info->asf_crop_info.first_pixel + 1;
  }
  rot_info->block_height = 2;

 if ((plane_info->input_plane_fmt == PLANE_CBCR) ||
    (plane_info->input_plane_fmt == PLANE_CRCB))  {
    rot_info->tile_width  = CHROMA_TILE_WIDTH;
    rot_info->tile_size = CHROMA_TILE_SIZE;
  } else {
    rot_info->tile_width  = LUMA_TILE_WIDTH;
    rot_info->tile_size = LUMA_TILE_SIZE;
  }
  rot_info->tile_height = TILE_HEIGHT;

  if(stripe_info->rotation == 0) {
    rot_info->rot_cfg = ROT_0 + stripe_info->mirror;
    rot_info->block_height = plane_info->tile_output_enable ?
      rot_info->tile_height : rot_info->block_height;
  } else if (stripe_info->rotation == 1) {
    rot_info->rot_cfg = ROT_90 + stripe_info->mirror;
    //rot_info->rot_cfg = ROT_90_HV_FLIP - stripe_info->mirror;
    rot_info->block_height = plane_info->tile_output_enable ?
      rot_info->tile_width : plane_info->mal_byte_size;
  } else if (stripe_info->rotation == 2) {
    rot_info->rot_cfg = ROT_0_HV_FLIP - stripe_info->mirror;
    rot_info->block_height = plane_info->tile_output_enable ?
      rot_info->tile_height : rot_info->block_height;
  } else if (stripe_info->rotation == 3) {
   rot_info->rot_cfg = ROT_90_HV_FLIP - stripe_info->mirror;
   //rot_info->rot_cfg = ROT_90 + stripe_info->mirror;
    rot_info->block_height = plane_info->tile_output_enable ?
      rot_info->tile_width : plane_info->mal_byte_size;
  }

/*
  rot_info->rot_cfg =
    plane_info->stripe_info[stripe_num].rotate_270 << 2 |
    plane_info->stripe_info[stripe_num].vertical_flip << 1|
    plane_info->stripe_info[stripe_num].horizontal_flip;

  if (plane_info->stripe_info[stripe_num].rotate_270) {
    rot_info->block_height = 32;
  }
*/
  rot_info->block_size = rot_info->block_width * rot_info->block_height;
  block_width = rot_info->block_width;
  block_height = rot_info->block_height;
  block_size = rot_info->block_size;

  if (plane_info->sharpen_before_scale) {
    rot_info->block_width = stripe_scale_info->block_width;
    rot_info->blocks_per_stripe = ceil((float)
      (stripe_scale_info->block_height)/ rot_info->block_height);
  } else {
    rot_info->blocks_per_stripe = ceil((float)
      (stripe_info->asf_crop_info.last_line -
      stripe_info->asf_crop_info.first_line + 1)/ rot_info->block_height);
  }

  switch (rot_info->rot_cfg) {
  case ROT_0:
    rot_info->rowIndex0 = 1;
    rot_info->rowIndex1 = 0;
    rot_info->colIndex0 = 1;
    rot_info->colIndex1 = 0;
    rot_info->modValue = block_size + 1;
    rot_info->initIndex = 0;
    output_block_width = block_width * frame_strip_info->output_bytes_per_pixel;
    break;
  case ROT_0_H_FLIP:
    rot_info->rowIndex0 = 0;
    rot_info->rowIndex1 = 1;
    rot_info->colIndex0 = (block_width * 2)-1;
    rot_info->colIndex1 = 0;
    rot_info->modValue = block_size + 1;
    rot_info->initIndex = block_width - 1;
    output_block_width = block_width * frame_strip_info->output_bytes_per_pixel;
    break;
  case ROT_0_V_FLIP:
    rot_info->rowIndex0 = 1;
    rot_info->rowIndex1 = 0;
    rot_info->colIndex0 = (block_height - 2) * block_width;
    rot_info->colIndex1 = block_size - 1;
    rot_info->modValue = block_size + 1;
    rot_info->initIndex = (block_height - 1) * block_width;
    output_block_width = block_width * frame_strip_info->output_bytes_per_pixel;
    break;
  case ROT_0_HV_FLIP:
    rot_info->rowIndex0 = 0;
    rot_info->rowIndex1 = 1;
    rot_info->colIndex0 = 0;
    rot_info->colIndex1 = 1;
    rot_info->modValue = block_size + 1;
    rot_info->initIndex = block_size - 1;
    output_block_width = block_width * frame_strip_info->output_bytes_per_pixel;
    break;
  case ROT_90:
    rot_info->rowIndex0 = 0;
    rot_info->rowIndex1 = block_width;
    rot_info->colIndex0 = ((block_height - 1) * block_width) + 1;
    rot_info->colIndex1 = 0;
    rot_info->modValue = block_size + 1;
    rot_info->initIndex = (block_height - 1) * block_width;
    output_block_width = rot_info->block_height;
    break;
  case ROT_90_H_FLIP:
    rot_info->rowIndex0 = block_width;
    rot_info->rowIndex1 = 0;
    rot_info->colIndex0 = 1;
    rot_info->colIndex1 = (block_height - 1) * block_width;
    rot_info->modValue = block_size - 1;
    rot_info->initIndex = 0;
    output_block_width = rot_info->block_height;
    break;
  case ROT_90_V_FLIP:
    rot_info->rowIndex0 = ((block_height - 1) * block_width) - 1;
    rot_info->rowIndex1 = block_size - 1;
    rot_info->colIndex0 = block_size - 2;
    rot_info->colIndex1 = block_width - 1;
    rot_info->modValue = block_size - 1;
    rot_info->initIndex = block_size - 1;
    output_block_width = rot_info->block_height;
    break;
  case ROT_90_HV_FLIP:
    rot_info->rowIndex0 = (block_width * 2) - 1;
    rot_info->rowIndex1 = block_width - 1;
    rot_info->colIndex0 = block_width - 2;
    rot_info->colIndex1 = block_size - 1;
    rot_info->modValue = block_size + 1;
    rot_info->initIndex = block_width - 1;
    output_block_width = rot_info->block_height;
    break;
  }

  int tiles_per_block_row = (output_block_width / (rot_info->tile_width));
  rot_info->tile_voffset = tiles_per_block_row * rot_info->tile_size;

  cpp_debug_rotation_info(rot_info);
}

void cpp_debug_write_engine_info(struct cpp_we_info *we_info __unused)
{
  if (!(IS_VALID_MASK(g_cpp_log_featureMask, CPP_STRIPE_WE_BIT)))
    return;

  CPP_STRIPE_WE("buffer_ptr[0]: %d\n", we_info->buffer_ptr[0]);
  CPP_STRIPE_WE("buffer_ptr[1]: %d\n", we_info->buffer_ptr[1]);
  CPP_STRIPE_WE("buffer_ptr[2]: %d\n", we_info->buffer_ptr[2]);
  CPP_STRIPE_WE("buffer_ptr[3]: %d\n", we_info->buffer_ptr[3]);
  CPP_STRIPE_WE("buffer_width: %d\n", we_info->buffer_width);
  CPP_STRIPE_WE("buffer_height: %d\n", we_info->buffer_height);
  CPP_STRIPE_WE("buffer_stride: %d\n", we_info->buffer_stride);
  CPP_STRIPE_WE("blocks_per_col: %d\n", we_info->blocks_per_col);
  CPP_STRIPE_WE("blocks_per_row: %d\n", we_info->blocks_per_row);
  CPP_STRIPE_WE("h_step: %d\n", we_info->h_step);
  CPP_STRIPE_WE("v_step: %d\n", we_info->v_step);
  CPP_STRIPE_WE("h_init: %d\n", we_info->h_init);
  CPP_STRIPE_WE("v_init: %d\n", we_info->v_init);
  CPP_STRIPE_WE("ubwc_tile_format: %d\n", we_info->ubwc_tile_format);
  CPP_STRIPE_WE("ubwc_out_type: %d\n", we_info->ubwc_out_type);
  CPP_STRIPE_WE("ubwc_en: %d\n", we_info->ubwc_en);
  CPP_STRIPE_WE("tile_height: %d\n", we_info->tile_height);
  CPP_STRIPE_WE("tile_width: %d\n", we_info->tile_width);
  CPP_STRIPE_WE("ubwc_bank_spread_en: %d\n", we_info->ubwc_bank_spread_en);
  CPP_STRIPE_WE("highest_bank_en: %d\n", we_info->highest_bank_en);
  CPP_STRIPE_WE("tiles_per_block_row: %d\n", we_info->tiles_per_block_row);
  CPP_STRIPE_WE("tiles_per_block_col: %d\n", we_info->tiles_per_block_col);
  CPP_STRIPE_WE("highest_bank_bit: %d\n", we_info->highest_bank_bit);
  CPP_STRIPE_WE("rot_mode: %d\n", we_info->rot_mode);
}

void cpp_prepare_write_engine_info(struct cpp_plane_info_t *plane_info,
  uint32_t stripe_num)
{
  struct cpp_stripe_info *stripe_info1 =
    &plane_info->stripe_info1[stripe_num];
  struct cpp_striping_algorithm_info *stripe_info =
    &plane_info->stripe_info[stripe_num];
  struct cpp_rotator_info *rot_info = &stripe_info1->rot_info;
  struct cpp_we_info *we_info = &stripe_info1->we_info;
  uint32_t blocks_per_stripe;
  int32_t output_block_height, output_block_width;
  int32_t h_pix_offset, v_pix_offset;
  char value[PROPERTY_VALUE_MAX];

  /* Write engine config depends on scaler position */
  if (plane_info->sharpen_before_scale) {
    blocks_per_stripe = ceil((float)
      (stripe_info1->scale_info.block_height)/
      rot_info->block_height);
  } else {
    blocks_per_stripe = ceil((float)
      (stripe_info1->asf_crop_info.last_line -
       stripe_info1->asf_crop_info.first_line + 1)/ rot_info->block_height);
  }

  if (rot_info->rot_cfg < ROT_90) {
    output_block_height = rot_info->block_height;
    output_block_width = rot_info->block_width * stripe_info->output_bytes_per_pixel;
    we_info->buffer_width = output_block_width;
    //we_info->buffer_height = stripe_info->dst_end_y - stripe_info->dst_start_y + 1;
    we_info->buffer_height = output_block_height * blocks_per_stripe;
    we_info->buffer_stride = stripe_info->dst_stride;
    we_info->buffer_ptr[0] = stripe_info->destination_address[0];
    we_info->buffer_ptr[1] = we_info->buffer_ptr[0];
    we_info->buffer_ptr[2] = stripe_info->destination_address[1];
    we_info->buffer_ptr[3] = we_info->buffer_ptr[2];
    we_info->blocks_per_col = blocks_per_stripe;
    we_info->blocks_per_row = 1;
  } else {
    output_block_height = rot_info->block_width;
    output_block_width = rot_info->block_height * stripe_info->output_bytes_per_pixel;
    we_info->buffer_width = (output_block_width * blocks_per_stripe);
    we_info->buffer_height = output_block_height;
    we_info->buffer_stride = stripe_info->dst_stride;
    we_info->buffer_ptr[0] = stripe_info->destination_address[0];
    we_info->buffer_ptr[1] = we_info->buffer_ptr[0];
/*
    if (rot_info->rot_cfg == ROT_90_H_FLIP ||
         rot_info->rot_cfg == ROT_90_HV_FLIP) {
      we_info->buffer_ptr[0] -= stripe_info1->fe_info.top_pad *
        stripe_info->bytes_per_pixel;
    }
*/
    we_info->buffer_ptr[2] = stripe_info->destination_address[1];
    we_info->buffer_ptr[3] = we_info->buffer_ptr[2];
    we_info->blocks_per_col = 1;
    we_info->blocks_per_row = blocks_per_stripe;
  }


  h_pix_offset = (blocks_per_stripe - 1) * output_block_width;
  v_pix_offset = (blocks_per_stripe - 1) * output_block_height;

 //UBWC write engine parameters
 we_info->ubwc_en = plane_info->tile_output_enable;
 if (we_info->ubwc_en) {
   if ((plane_info->input_plane_fmt == PLANE_CBCR) ||
      (plane_info->input_plane_fmt == PLANE_CRCB))  {
      we_info->ubwc_tile_format = CHROMA_TILE_FORMAT;
   } else {
      we_info->ubwc_tile_format = LUMA_TILE_FORMAT;
   }
   we_info->tile_width  = rot_info->tile_width;
   we_info->tile_height  = rot_info->tile_height;
   we_info->tiles_per_block_col = (output_block_height / we_info->tile_height);
    we_info->tiles_per_block_row =
      ((output_block_width / stripe_info->output_bytes_per_pixel) /
      (we_info->tile_width));
    we_info->ubwc_bank_spread_en = UBWC_BANK_SPREAD_ENABLE;
    we_info->highest_bank_en =  UBWC_HIGESHT_BANK_ENABLE;
    we_info->highest_bank_bit = plane_info->ubwc_hbb;
    we_info->rot_mode = rot_info->rot_cfg;
    property_get("vendor.camera.ubwc.no.compression", value, "0");
    if (atoi(value))
      we_info->ubwc_out_type = UBWC_UNCOMPRESSED_OUTPUT;
    else
      we_info->ubwc_out_type = UBWC_COMPRESSED_OUTPUT;
  }

  switch (rot_info->rot_cfg) {
  case ROT_0:
  case ROT_0_H_FLIP:
  case ROT_90_H_FLIP:
  case ROT_90_HV_FLIP:
    we_info->h_init = we_info->ubwc_en ?
      (stripe_info->dst_start_x * (stripe_info->output_bytes_per_pixel)): 0;
    we_info->v_init = we_info->ubwc_en ? stripe_info->dst_start_y : 0;
    we_info->h_step = output_block_width;
    we_info->v_step = output_block_height;
    break;
  case ROT_0_V_FLIP:
  case ROT_0_HV_FLIP:
    we_info->h_init = we_info->ubwc_en ?
      stripe_info->dst_start_x * (stripe_info->output_bytes_per_pixel) : 0;
    we_info->v_init = we_info->ubwc_en ?
      (int32_t)(stripe_info->dst_end_y + 1 - 8) :
      v_pix_offset;
    we_info->h_step = output_block_width;
    we_info->v_step = output_block_height * -1;
    break;
  case ROT_90:
  case ROT_90_V_FLIP:
    we_info->h_init = we_info->ubwc_en ?
      (int32_t)(((stripe_info->dst_end_x + 1) *
      stripe_info->output_bytes_per_pixel) - 32) : h_pix_offset;
    we_info->v_init = we_info->ubwc_en ? stripe_info->dst_start_y : 0;
    we_info->h_step = output_block_width * -1;
    we_info->v_step = output_block_height;
    break;
  }
  cpp_debug_write_engine_info(we_info);
}

/* cpp_pack_asf_kernel_1_4_x:
*
* @frame_msg: pointer to CPP frame payload
* @filter: pointer to ASF H and V filter
* Description:
*     Packs ASF filter values into frame payload as per firmware
*     register.
* Return: void
**/
void cpp_pack_asf_kernel_1_4_x(uint32_t *frame_msg, int16_t *filter)
{
  frame_msg[0] = ((filter[1]) << 16) | ((filter[0]) & 0xFFF);
  frame_msg[1] = ((filter[3]) << 16) | ((filter[2]) & 0xFFF);
  frame_msg[2] = ((filter[4]) << 16);
  frame_msg[3] = ((filter[6]) << 16) | ((filter[5]) & 0xFFF);
  frame_msg[4] = ((filter[7]) & 0xFFF);
  frame_msg[5] = ((filter[9]) << 16) | ((filter[8]) & 0xFFF);
  frame_msg[6] = ((filter[11]) << 16) | ((filter[10]) & 0xFFF);
  frame_msg[7] = ((filter[12]) << 16);
  frame_msg[8] = ((filter[14]) << 16) | ((filter[13]) & 0xFFF);
  frame_msg[9] = ((filter[15]) & 0xFFF);
}

uint32_t* cpp_create_frame_message_1_6_x(
   struct cpp_frame_info_t *cpp_frame_info, uint32_t* len)
{
  int32_t idx, total_stripes;
  uint32_t msg_len, i, j, k, base;
  uint32_t *frame_msg;

  total_stripes = 0;
  for (i=0; i < cpp_frame_info->num_planes; i++) {
    total_stripes += cpp_frame_info->plane_info[i].num_stripes;
  }

  msg_len = 465 + 27 * total_stripes;
  *len = msg_len;

  frame_msg = (uint32_t *) CAM_MALLOC(sizeof(uint32_t) * msg_len);
  if (!frame_msg){
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
    cpp_frame_info->batch_info.batch_size << 1;
  /*Input Plane address HFR*/
  frame_msg[6] = cpp_frame_info->batch_info.intra_plane_offset[0];
  frame_msg[7] = cpp_frame_info->batch_info.intra_plane_offset[1];
  frame_msg[8] = cpp_frame_info->batch_info.intra_plane_offset[2];
  /*Output Plane address HFR*/
  frame_msg[9] = 0x0;
  frame_msg[10] = 0x0;
  frame_msg[11] = 0x0;
  /* Number of stripes */
  frame_msg[12] = (cpp_frame_info->plane_info[0].num_stripes) |
    (cpp_frame_info->plane_info[1].num_stripes << 10) |
    (cpp_frame_info->plane_info[2].num_stripes << 20);

  /* ASF LUT config */
  base = 13;
  cpp_1_6_x_asf_info_t *asf_info = &(cpp_frame_info->u_asf.info_1_6_x);
  for (i=0; i<5; i++) {
    for (j=0; j<64; j++) {
      frame_msg[base + i*64 + j] =
        asf_info->lut[i][j*4] |
        asf_info->lut[i][j*4+1] << 8 |
        asf_info->lut[i][j*4+2] << 16 |
        asf_info->lut[i][j*4+3] << 24;
    }
  }

  /* WNR Config */
  base = 333;
  struct cpp_bf_info *bf_info;
  for (i=0; i < 3; i++) {
    bf_info = &cpp_frame_info->bf_info[i];
    for (j = 0; j < 4; j++) {
      k = base + i*8 + j;
      if (j != 3) {
        frame_msg[k] =
          (uint32_t)(Round(bf_info->bilateral_scale[j] * (1 << 10)));
      } else {
        frame_msg[k] =
          (uint32_t)(Round(bf_info->bilateral_scale[j] * (1 << 8)));
      }
      frame_msg[k+4] =
        ((uint32_t)(Round(bf_info->noise_threshold[j] * (1 << 4)))) << 8 |
         ((uint32_t)(Round(bf_info->weight[j] * (1 << 4))));
    }
  }

  /* ASF Settings */
  frame_msg[357] =
    (1 << 31) | /* set asf version to 1, for fw 1.6.0 */
    ((asf_info->sp_eff_en & 0x1) << 0) |
    ((asf_info->dyna_clamp_en & 0x1) << 1) |
    ((asf_info->neg_abs_y1 & 0x1) << 2) |
    ((asf_info->sp & 0x1f) << 4) |
    ((asf_info->sp_eff_abs_en) << 16) |
    ((asf_info->L2_norm_en & 0x1) << 17);
  frame_msg[358] =
    (asf_info->clamp_h_ll & 0x1ff) |
    ((asf_info->clamp_h_ul & 0x1ff) << 8);
  frame_msg[359] =
    (asf_info->clamp_v_ll & 0x1ff) |
    ((asf_info->clamp_v_ul & 0x1ff) << 8);
  frame_msg[360] =
    (asf_info->clamp_scale_min & 0x1ff) |
    ((asf_info->clamp_scale_max & 0x1ff) << 16);
  frame_msg[361] =
    (asf_info->clamp_offset_min & 0x7f) |
    ((asf_info->clamp_offset_max & 0x7f) << 16);
  frame_msg[362] = asf_info->nz_flag;

  base = 363;
  for (i=0; i<8; i++) {
    frame_msg[base + i] =
      (asf_info->sobel_h_coeff[i*2] & 0xfff)|
      ((asf_info->sobel_h_coeff[i*2 + 1] & 0xfff) << 16);
  }
  base = 371;
  for (i=0; i<8; i++) {
    frame_msg[base + i] =
      (asf_info->sobel_v_coeff[i*2] & 0xfff)|
      ((asf_info->sobel_v_coeff[i*2 + 1] & 0xfff) << 16);
  }
  base = 379;
  for (i=0; i<8; i++) {
    frame_msg[base + i] =
      (asf_info->hpf_h_coeff[i*2] & 0xfff)|
      ((asf_info->hpf_h_coeff[i*2 + 1] & 0xfff) << 16);
  }
  base = 387;
  for (i=0; i<8; i++) {
    frame_msg[base + i] =
      (asf_info->hpf_v_coeff[i*2] & 0xfff)|
      ((asf_info->hpf_v_coeff[i*2 + 1] & 0xfff) << 16);
  }
  base = 395;
  for (i=0; i<8; i++) {
    frame_msg[base + i] =
      (asf_info->lpf_coeff[i*2] & 0xfff)|
      ((asf_info->lpf_coeff[i*2 + 1] & 0xfff) << 16);
  }
  frame_msg[403] = asf_info->nz_flag_2;
  frame_msg[404] = asf_info->nz_flag_3_5;
  frame_msg[405] =
    (asf_info->clamp_tl_ll & 0x1ff) |
    ((asf_info->clamp_tl_ul & 0x1ff) << 9);

  base = 406;
  for (i=8; i<13; i++) {
    frame_msg[base + i] =
      (asf_info->sobel_h_coeff[i*2] & 0xfff)|
      ((asf_info->sobel_h_coeff[i*2 + 1] & 0xfff) << 16);
  }
  base = 411;
  for (i=8; i<13; i++) {
    frame_msg[base + i] =
      (asf_info->sobel_v_coeff[i*2] & 0xfff)|
      ((asf_info->sobel_v_coeff[i*2 + 1] & 0xfff) << 16);
  }
  base = 416;
  for (i=8; i<13; i++) {
    frame_msg[base + i] =
      (asf_info->hpf_h_coeff[i*2] & 0xfff)|
      ((asf_info->hpf_h_coeff[i*2 + 1] & 0xfff) << 16);
  }
  base = 421;
  for (i=8; i<13; i++) {
    frame_msg[base + i] =
      (asf_info->hpf_v_coeff[i*2] & 0xfff)|
      ((asf_info->hpf_v_coeff[i*2 + 1] & 0xfff) << 16);
  }
  base = 426;
  for (i=8; i<13; i++) {
    frame_msg[base + i] =
      (asf_info->lpf_coeff[i*2] & 0xfff)|
      ((asf_info->lpf_coeff[i*2 + 1] & 0xfff) << 16);
  }
  base = 431;
  for (i=0; i<8; i++) {
    frame_msg[base + i] =
      (asf_info->hpf_sym_coeff[i*2] & 0xfff)|
      ((asf_info->hpf_sym_coeff[i*2 + 1] & 0xfff) << 16);
  }
  base = 439;
  for (i=0; i<3; i++) {
    frame_msg[base + i] =
      (asf_info->activity_bpf_coeff[i*2] & 0x3ff)|
      ((asf_info->activity_bpf_coeff[i*2 + 1] & 0x3ff) << 16);
  }
  base = 442;
  for (i=0; i<3; i++) {
    frame_msg[base + i] =
      (asf_info->activity_lpf_coeff[i*2] & 0xff)|
      ((asf_info->activity_lpf_coeff[i*2 + 1] & 0xff) << 16);
  }
  frame_msg[445] =
    (asf_info->perpen_scale_factor & 0x7f) |
    ((asf_info->activity_clamp_threshold & 0xff) << 16);
  frame_msg[446] =
    (asf_info->max_val_threshold & 0x3fff) |
    ((asf_info->gamma_cor_luma_target & 0xff) << 16);
  frame_msg[447] =
    (asf_info->gain_cap & 0xff) |
    ((asf_info->median_blend_offset & 0xf) << 16) |
    ((asf_info->median_blend_lower_offset & 0xf) << 20);
  frame_msg[448] = (asf_info->norm_scale & 0xff);

  /* Plane Specific Configuration */
  base = 449;
  for (i = 0; i < cpp_frame_info->num_planes; i++) {
    j = base + i*5;

    frame_msg[j] = 1 << 5 | /* todo: why rotation is enabled */
      cpp_frame_info->plane_info[i].asf_crop_enable << 4 |
      cpp_frame_info->plane_info[i].bf_crop_enable << 1 |
      cpp_frame_info->plane_info[i].bf_enable;
    if (cpp_frame_info->plane_info[i].scale_info.hscale_en ||
        cpp_frame_info->plane_info[i].scale_info.vscale_en) {
      frame_msg[j] |= 0x4;
    }
    if (cpp_frame_info->plane_info[i].input_plane_fmt == PLANE_Y &&
        cpp_frame_info->asf_mode != 0) {
      frame_msg[j] |= (0x1 << 3);
    }
    frame_msg[j+1] =
      cpp_frame_info->plane_info[i].scale_info.v_scale_fir_algo << 12 |
      cpp_frame_info->plane_info[i].scale_info.h_scale_fir_algo << 8 |
      cpp_frame_info->plane_info[i].scale_info.v_scale_algo << 5 |
      cpp_frame_info->plane_info[i].scale_info.h_scale_algo << 4 |
      cpp_frame_info->plane_info[i].scale_info.subsample_en << 3 |
      cpp_frame_info->plane_info[i].scale_info.upsample_en << 2 |
      cpp_frame_info->plane_info[i].scale_info.vscale_en << 1 |
      cpp_frame_info->plane_info[i].scale_info.hscale_en;
    frame_msg[j+2] = cpp_frame_info->plane_info[i].scale_info.phase_h_step;
    frame_msg[j+3] = cpp_frame_info->plane_info[i].scale_info.phase_v_init;
    frame_msg[j+4] = cpp_frame_info->plane_info[i].scale_info.phase_v_step;
  }

  /* Stripe Specific Configuration */
  base = base + 15;
  idx = base;
  struct cpp_stripe_info *stripe_info;
  for (j = 0; j < cpp_frame_info->num_planes; j++) {
    for (k = 0; k < cpp_frame_info->plane_info[j].num_stripes; k++, idx+=27) {
      stripe_info = &(cpp_frame_info->plane_info[j].stripe_info1[k]);
      frame_msg[idx + 0] = //STRIPE[0]_PP_m_ROT_CFG_0
        stripe_info->rot_info.rot_cfg << 24 |
        (stripe_info->rot_info.block_size - 1);
      frame_msg[idx + 1] = //STRIPE[0]_PP_m_ROT_CFG_1
        (stripe_info->rot_info.block_height - 1) << 16 |
        (stripe_info->rot_info.block_width - 1);
      frame_msg[idx + 2] = //STRIPE[0]_PP_m_ROT_CFG_2
        stripe_info->rot_info.rowIndex1 << 16 |
        stripe_info->rot_info.rowIndex0;
      frame_msg[idx + 3] = //STRIPE[0]_PP_m_ROT_CFG_3
        stripe_info->rot_info.colIndex1 << 16 |
        stripe_info->rot_info.colIndex0;
      frame_msg[idx + 4] = //STRIPE[0]_PP_m_ROT_CFG_4
        stripe_info->rot_info.modValue << 16 |
        stripe_info->rot_info.initIndex;
      frame_msg[idx + 5] = stripe_info->fe_info.buffer_ptr;
      frame_msg[idx + 6] = //STRIPE[0]_FE_n_RD_BUFFER_SIZE
        (stripe_info->fe_info.buffer_height - 1) << 16 |
        (stripe_info->fe_info.buffer_width - 1);
      frame_msg[idx + 7] = //STRIPE[0]_FE_n_RD_STRIDE
        stripe_info->fe_info.buffer_stride;
      frame_msg[idx + 8] = //STRIPE[0]_FE_n_SWC_RD_BLOCK_DIMENSION
        (stripe_info->fe_info.block_height - 1) << 16 |
        (stripe_info->fe_info.block_width- 1);
      frame_msg[idx + 9] = //STRIPE[0]_FE_n_SWC_RD_BLOCK_HPAD
      stripe_info->fe_info.right_pad << 16 |
      stripe_info->fe_info.left_pad;
      frame_msg[idx + 10] = //STRIPE[0]_FE_n_SWC_RD_BLOCK_VPAD
      stripe_info->fe_info.bottom_pad << 16 |
      stripe_info->fe_info.top_pad;
      frame_msg[idx + 11] = stripe_info->we_info.buffer_ptr[0];
      frame_msg[idx + 12] = stripe_info->we_info.buffer_ptr[1];
      frame_msg[idx + 13] = stripe_info->we_info.buffer_ptr[2];
      frame_msg[idx + 14] = stripe_info->we_info.buffer_ptr[3];
      frame_msg[idx + 15] = //STRIPE[0]_WE_PLN_n_WR_BUFFER_SIZE
        stripe_info->we_info.buffer_height << 16 |
        stripe_info->we_info.buffer_width;
      frame_msg[idx + 16] = stripe_info->we_info.buffer_stride;
      frame_msg[idx + 17] = //STRIPE[0]_WE_PLN_n_WR_CFG_0
        (stripe_info->we_info.blocks_per_row - 1) << 16 |
        (stripe_info->we_info.blocks_per_col - 1);
      frame_msg[idx + 18] = stripe_info->we_info.h_step;
      frame_msg[idx + 19] = stripe_info->we_info.v_step;
      frame_msg[idx + 20] = //STRIPE[0]_WE_PLN_n_WR_CFG_3
      (stripe_info->we_info.h_init) << 16 |
      (stripe_info->we_info.v_init);
      frame_msg[idx + 21] = //STRIPE[0]_PP_m_BF_CROP_CFG_0
        stripe_info->bf_crop_info.last_pixel << 16 |
        stripe_info->bf_crop_info.first_pixel;
      frame_msg[idx + 22] = //STRIPE[0]_PP_m_BF_CROP_CFG_1
        stripe_info->bf_crop_info.last_line << 16 |
        stripe_info->bf_crop_info.first_line;
      frame_msg[idx + 23] = //STRIPE[0]_PP_m_SCALE_OUTPUT_CFG
        (stripe_info->scale_info.block_height-1) << 16 |
        (stripe_info->scale_info.block_width-1);
      frame_msg[idx + 24] = //STRIPE[0]_PP_m_SCALE_PHASEH_INIT
        stripe_info->scale_info.phase_h_init;
      frame_msg[idx + 25] = //STRIPE[0]_PP_m_ASF_CROP_CFG_0
        stripe_info->asf_crop_info.last_pixel << 16 |
        stripe_info->asf_crop_info.first_pixel;
      frame_msg[idx + 26] = //STRIPE[0]_PP_m_ASF_CROP_CFG_1
        stripe_info->asf_crop_info.last_line << 16 |
        stripe_info->asf_crop_info.first_line;
    }
  }
  frame_msg[msg_len - 1] = CPP_FW_CMD_TRAILER;
  return frame_msg;
}

/* cpp_create_frame_message_1_4_x:
*
* @cpp_frame_info: pointer to CPP frame info
* @len: pointer to CPP frame payload length
* Description:
*     Creates/packs CPP frame payload for firmware version
*     1.4 before sending to CPP firmware.
*
* Return: int*
*     Pointer to CPP frame payload
**/
uint32_t* cpp_create_frame_message_1_4_x(
  struct cpp_frame_info_t *cpp_frame_info, uint32_t* len)
{
  struct cpp_stripe_info *stripe_info = NULL;
  uint32_t i = 0, j = 0, k = 0, num_stripes = 0, msg_len = 0;
  uint32_t *frame_msg = NULL;
  uint16_t LUT1[24] = {0};
  uint16_t LUT2[24] = {0};
  uint16_t LUT3[12] = {0};
  int16_t  F1[16] = {0};
  int16_t  F2[16] = {0};
  int16_t  F3[16] = {0};
  int16_t  F4[16] = {0};
  int16_t  F5[16] = {0};
  int32_t LUT1_Value[24] = {0};
  int32_t LUT1_Delta[24] = {0};
  int32_t LUT2_Value[24] = {0};
  int32_t LUT2_Delta[24] = {0};
  int32_t LUT3_Value[12] = {0};
  int32_t LUT3_Delta[12] = {0};
  int checksum = 0;
  uint8_t checksum_enable = 0;
  struct cpp_asf_info *asf_info = &(cpp_frame_info->asf_info);
  uint32_t checksum_mask = asf_info->checksum_en;

  if (asf_info->sp_eff_en == 1) {
    checksum_mask = 0;
  }

  for (i =0; i < cpp_frame_info->num_planes; i++) {
    num_stripes += cpp_frame_info->plane_info[i].num_stripes;
  }
  /*frame info size*/
  msg_len = 136 + 2 + 27 * num_stripes + 3;
  /*Total message size = frame info + header + length + trailer*/
  frame_msg = CAM_MALLOC(sizeof(uint32_t) * msg_len);
  if (!frame_msg){
      CPP_ERR("CAM_MALLOC() failed");
      return NULL;
  }
  memset(frame_msg, 0, sizeof(uint32_t) * msg_len);

  *len = msg_len;

  /*Top Level*/
  frame_msg[0] = 0x3E646D63;
  frame_msg[1] = msg_len - 3;
  frame_msg[2] = 0x6;
  frame_msg[3] = 0;
  frame_msg[4] = 0;
  /*Plane info*/
  frame_msg[5] = (cpp_frame_info->in_plane_fmt << 24) |
    (cpp_frame_info->out_plane_fmt << 16) |
    (cpp_frame_info->plane_info[0].stripe_info1[0].rot_info.rot_cfg << 13) |
    cpp_frame_info->batch_info.batch_size << 1;
  /*Plane address HFR*/
  frame_msg[6] = cpp_frame_info->batch_info.intra_plane_offset[0];
  frame_msg[7] = cpp_frame_info->batch_info.intra_plane_offset[1];
  frame_msg[8] = cpp_frame_info->batch_info.intra_plane_offset[2];
  /*Output Plane HFR*/
  frame_msg[9] = 0x0;
  frame_msg[10] = 0x0;
  frame_msg[11] = 0x0;

  for ( i = 0; i < 16; i++) {
    F1[i] = (int16_t)(Round(asf_info->sobel_h_coeff[i]*(1<<10)));
    F2[i] = (int16_t)(Round(asf_info->sobel_v_coeff[i]*(1<<10)));
    F3[i] = (int16_t)(Round(asf_info->hpf_h_coeff[i]*(1<<10)));
    F4[i] = (int16_t)(Round(asf_info->hpf_v_coeff[i]*(1<<10)));
    F5[i] = (int16_t)(Round(asf_info->lpf_coeff[i]*(1<<10)));
  }
  checksum_enable = checksum_mask & 1;

  if (checksum_enable) {
    checksum = 4*(F1[0] + F1[1] + F1[2]\
      + F1[4] + F1[5] + F1[6]\
      + F1[8] + F1[9] + F1[10])\
      + 2*(F1[3] + F1[7] + F1[11]\
      + F1[12] + F1[13] + F1[14])
      + F1[15];
    if (checksum != 0)
      F1[15] = F1[15]- checksum;
  }

  checksum_enable = (checksum_mask >> 1) & 1;
  if (checksum_enable) {
    checksum = 4*(F2[0] + F2[1] + F2[2]\
      + F2[4] + F2[5] + F2[6]\
      + F2[8] + F2[9] + F2[10])\
      + 2*(F2[3] + F2[7] + F2[11]\
      + F2[12] + F2[13] + F2[14])
      + F2[15];
    if (checksum != 0)
      F2[15] = F2[15]- checksum;
  }
  checksum_enable = (checksum_mask >> 2) & 1;
  if (checksum_enable) {
    checksum = 4*(F3[0] + F3[1] + F3[2]\
      + F3[4] + F3[5] + F3[6]\
      + F3[8] + F3[9] + F3[10])\
      + 2*(F3[3] + F3[7] + F3[11]\
      + F3[12] + F3[13] + F3[14])
      + F3[15];
    if (checksum != 0)
      F3[15] = F3[15]- checksum;
  }

  checksum_enable = (checksum_mask >> 3) & 1;
  if (checksum_enable) {
    checksum = 4*(F4[0] + F4[1] + F4[2]\
      + F4[4] + F4[5] + F4[6]\
      + F4[8] + F4[9] + F4[10])\
      + 2*(F4[3] + F4[7] + F4[11]\
      + F4[12] + F4[13] + F4[14])
      + F4[15];
    if (checksum != 0)
      F4[15] = F4[15]- checksum;
  }

  checksum_enable = (checksum_mask >> 4) & 1 ;
  if (checksum_enable) {
    checksum = 4*(F5[0] + F5[1] + F5[2]\
      + F5[4] + F5[5] + F5[6]\
      + F5[8] + F5[9] + F5[10])\
      + 2*(F5[3] + F5[7] + F5[11]\
      + F5[12] + F5[13] + F5[14])
      + F5[15];
    if (checksum != (1<<10))
      F5[15] = F5[15] + ((1<<10)-checksum);
  }

  for (i = 0; i < 23; i++) {
    LUT1_Delta[i] =
      (int32_t)(Round((asf_info->lut1[i+1]-asf_info->lut1[i])*(1<<4)));
    LUT1_Value[i] =
      (int32_t)(Round((asf_info->lut1[i])*(1<<5)));
    LUT2_Delta[i] =
      (int32_t)(Round((asf_info->lut2[i+1]-asf_info->lut2[i])*(1<<4)));
    LUT2_Value[i] = (int32_t)(Round((asf_info->lut2[i])*(1<<5)));
  }
  LUT1_Delta[23] = 0;
  LUT1_Value[23] = (int32_t)(Round(asf_info->lut1[23]*(1<<5)));
  LUT2_Delta[23] = 0;
  LUT2_Value[23] = (int32_t)(Round(asf_info->lut2[23]*(1<<5)));

  for (i = 0; i < 11; i++) {
    LUT3_Delta[i] =
      (int32_t)(Round((asf_info->lut3[i+1]-asf_info->lut3[i])*(1<<6)));
    LUT3_Value[i] = (int32_t)(Round(asf_info->lut3[i]*(1<<6)));
  }
  LUT3_Delta[11] = 0;
  LUT3_Value[11] = (int32_t)(Round(asf_info->lut3[11]*(1<<6)));

  for (i =0; i < cpp_frame_info->num_planes; i++) {
    frame_msg[12] |= cpp_frame_info->plane_info[i].num_stripes << (i * 10);
  }

  for (i = 0; i < 23; i++) {
    LUT1[i] = ((LUT1_Delta[i]) << 8) | (LUT1_Value[i]);
    LUT2[i] = (( LUT2_Delta[i]) << 8) | (LUT2_Value[i]);
  }
  LUT1[23] = LUT1_Value[23] ;
  LUT2[23] = LUT2_Value[23] ;

  for (i = 0; i < 11; i++) {
    LUT3[i] = ((LUT3_Delta[i]) << 7) | (LUT3_Value[i]);
  }
  LUT3[11] = LUT3_Value[11] ;

  for (i = 0; i < 12; i++) {
    frame_msg[13 + i] = (((LUT1[(i * 2) + 1]) << 16) | (LUT1[i * 2]));
    frame_msg[25 + i] = (((LUT2[(i * 2) + 1]) << 16) | (LUT2[i * 2]));
  }
  for (i = 0; i < 6; i++) {
    frame_msg[37 + i] = (((LUT3[(i * 2) + 1]) << 16) | (LUT3[i * 2]));
  }

  for (i=0; i < 3; i++) {
    struct cpp_bf_info *bf_info = &cpp_frame_info->bf_info[i];
    j= 43 + i * 8;
    for (j = 0; j < 4; j++) {
      k = 43 + i * 8 + j;
      if (j != 3) {
        frame_msg[k] =
          (uint32_t)(Round(bf_info->bilateral_scale[j] * (1 << 10)));
      } else {
        frame_msg[k] =
          (uint32_t)(Round(bf_info->bilateral_scale[j] * (1 << 8)));
      }
      frame_msg[k+4] =
        ((uint32_t)(Round(bf_info->noise_threshold[j] * (1 << 4)))) << 8 |
         ((uint32_t)(Round(bf_info->weight[j] * (1 << 4))));
    }
  }

  frame_msg[67] = (((uint32_t)Round(asf_info->sp * (1 << 4))) & 0x1F) << 4 |
                   (asf_info->neg_abs_y1 & 0x1) << 2 |
                   (asf_info->dyna_clamp_en & 0x1) << 1 |
                   (asf_info->sp_eff_en & 0x1);

  frame_msg[68] = (asf_info->clamp_h_ul & 0x1FF) << 9 |
                  (asf_info->clamp_h_ll & 0x1FF);

  frame_msg[69] = (asf_info->clamp_v_ul & 0x1FF) << 9 |
                  (asf_info->clamp_v_ll & 0x1FF);

  frame_msg[70] = ((uint32_t)(Round(asf_info->clamp_scale_max * (1 << 4)))
                    & 0x1FF) << 16 |
                  ((uint32_t)(Round(asf_info->clamp_scale_min * (1 << 4)))
                    & 0x1FF);

  frame_msg[71] = (asf_info->clamp_offset_max & 0x7F) << 16 |
                  (asf_info->clamp_offset_min & 0x7F);

  frame_msg[72] = asf_info->nz_flag;

  cpp_pack_asf_kernel_1_4_x(&frame_msg[73], &F1[0]);
  cpp_pack_asf_kernel_1_4_x(&frame_msg[83], &F2[0]);
  cpp_pack_asf_kernel_1_4_x(&frame_msg[93], &F3[0]);
  cpp_pack_asf_kernel_1_4_x(&frame_msg[103], &F4[0]);
  cpp_pack_asf_kernel_1_4_x(&frame_msg[113], &F5[0]);

  frame_msg[123] = asf_info->nz_flag_f2;
  frame_msg[124] = asf_info->nz_flag_f3_f5;

  for (i = 0; i < cpp_frame_info->num_planes; i++) {
    j = (123 + 2) + i * 5;
    frame_msg[j] = 0x20 |
      cpp_frame_info->plane_info[i].asf_crop_enable << 4 |
      cpp_frame_info->plane_info[i].bf_crop_enable << 1 |
      cpp_frame_info->plane_info[i].bf_enable;
    if (cpp_frame_info->plane_info[i].scale_info.hscale_en ||
        cpp_frame_info->plane_info[i].scale_info.vscale_en) {
      frame_msg[j] |= 0x4;
    }
    if (cpp_frame_info->plane_info[i].input_plane_fmt == PLANE_Y &&
        cpp_frame_info->asf_mode != 0) {
      frame_msg[j] |= 0x8;
    }

    frame_msg[j+1] =
      cpp_frame_info->plane_info[i].scale_info.v_scale_fir_algo << 12 |
      cpp_frame_info->plane_info[i].scale_info.h_scale_fir_algo << 8 |
      cpp_frame_info->plane_info[i].scale_info.v_scale_algo << 5 |
      cpp_frame_info->plane_info[i].scale_info.h_scale_algo << 4 |
      cpp_frame_info->plane_info[i].scale_info.subsample_en << 3 |
      cpp_frame_info->plane_info[i].scale_info.upsample_en << 2 |
      cpp_frame_info->plane_info[i].scale_info.vscale_en << 1 |
      cpp_frame_info->plane_info[i].scale_info.hscale_en;
    frame_msg[j+2] = cpp_frame_info->plane_info[i].scale_info.phase_h_step;
    frame_msg[j+3] = cpp_frame_info->plane_info[i].scale_info.phase_v_init;
    frame_msg[j+4] = cpp_frame_info->plane_info[i].scale_info.phase_v_step;
  }

  i = 138 + 2;
  for (j = 0; j < cpp_frame_info->num_planes; j++) {
    for (k = 0; k < cpp_frame_info->plane_info[j].num_stripes; k++) {
      stripe_info = &cpp_frame_info->plane_info[j].stripe_info1[k];
      frame_msg[i + 0] = //STRIPE[0]_PP_m_ROT_CFG_0
        stripe_info->rot_info.rot_cfg << 24 |
        (stripe_info->rot_info.block_size - 1);
      frame_msg[i + 1] = //STRIPE[0]_PP_m_ROT_CFG_1
        (stripe_info->rot_info.block_height - 1) << 16 |
        (stripe_info->rot_info.block_width - 1);
      frame_msg[i + 2] = //STRIPE[0]_PP_m_ROT_CFG_2
        stripe_info->rot_info.rowIndex1 << 16 |
        stripe_info->rot_info.rowIndex0;
      frame_msg[i + 3] = //STRIPE[0]_PP_m_ROT_CFG_3
        stripe_info->rot_info.colIndex1 << 16 |
        stripe_info->rot_info.colIndex0;
      frame_msg[i + 4] = //STRIPE[0]_PP_m_ROT_CFG_4
        stripe_info->rot_info.modValue << 16 |
        stripe_info->rot_info.initIndex;
      //STRIPE[0]_FE_n_RD_PNTR
      frame_msg[i + 5] = stripe_info->fe_info.buffer_ptr;
      frame_msg[i + 6] = //STRIPE[0]_FE_n_RD_BUFFER_SIZE
        (stripe_info->fe_info.buffer_height - 1) << 16 |
        (stripe_info->fe_info.buffer_width - 1);
      frame_msg[i + 7] = //STRIPE[0]_FE_n_RD_STRIDE
        stripe_info->fe_info.buffer_stride;
      frame_msg[i + 8] = //STRIPE[0]_FE_n_SWC_RD_BLOCK_DIMENSION
        (stripe_info->fe_info.block_height - 1) << 16 |
        (stripe_info->fe_info.block_width- 1);
      frame_msg[i + 9] = //STRIPE[0]_FE_n_SWC_RD_BLOCK_HPAD
      stripe_info->fe_info.right_pad << 16 |
      stripe_info->fe_info.left_pad;
      frame_msg[i + 10] = //STRIPE[0]_FE_n_SWC_RD_BLOCK_VPAD
      stripe_info->fe_info.bottom_pad << 16 |
      stripe_info->fe_info.top_pad;
      //STRIPE[0]_WE_PLN_0_WR_PNTR
      frame_msg[i + 11] = stripe_info->we_info.buffer_ptr[0];
      //STRIPE[0]_WE_PLN_1_WR_PNTR
      frame_msg[i + 12] = stripe_info->we_info.buffer_ptr[1];
      //STRIPE[0]_WE_PLN_2_WR_PNTR
      frame_msg[i + 13] = stripe_info->we_info.buffer_ptr[2];
      //STRIPE[0]_WE_PLN_3_WR_PNTR
      frame_msg[i + 14] = stripe_info->we_info.buffer_ptr[3];
      frame_msg[i + 15] = //STRIPE[0]_WE_PLN_n_WR_BUFFER_SIZE
        stripe_info->we_info.buffer_height << 16 |
        stripe_info->we_info.buffer_width;
      //STRIPE[0]_WE_PLN_n_WR_STRIDE
      frame_msg[i + 16] = stripe_info->we_info.buffer_stride;
      frame_msg[i + 17] = //STRIPE[0]_WE_PLN_n_WR_CFG_0
        (stripe_info->we_info.blocks_per_row - 1) << 16 |
        (stripe_info->we_info.blocks_per_col - 1);
      //STRIPE[0]_WE_PLN_n_WR_CFG_1
      frame_msg[i + 18] = stripe_info->we_info.h_step;
      //STRIPE[0]_WE_PLN_n_WR_CFG_2
      frame_msg[i + 19] = stripe_info->we_info.v_step;
      frame_msg[i + 20] = //STRIPE[0]_WE_PLN_n_WR_CFG_3
      (stripe_info->we_info.h_init) << 16 |
      (stripe_info->we_info.v_init);
      frame_msg[i + 21] = //STRIPE[0]_PP_m_BF_CROP_CFG_0
        stripe_info->bf_crop_info.last_pixel << 16 |
        stripe_info->bf_crop_info.first_pixel;
      frame_msg[i + 22] = //STRIPE[0]_PP_m_BF_CROP_CFG_1
        stripe_info->bf_crop_info.last_line << 16 |
        stripe_info->bf_crop_info.first_line;
      frame_msg[i + 23] = //STRIPE[0]_PP_m_SCALE_OUTPUT_CFG
        (stripe_info->scale_info.block_height-1) << 16 |
        (stripe_info->scale_info.block_width-1);
      frame_msg[i + 24] = //STRIPE[0]_PP_m_SCALE_PHASEH_INIT
        stripe_info->scale_info.phase_h_init;
      frame_msg[i + 25] = //STRIPE[0]_PP_m_ASF_CROP_CFG_0
        stripe_info->asf_crop_info.last_pixel << 16 |
        stripe_info->asf_crop_info.first_pixel;
      frame_msg[i + 26] = //STRIPE[0]_PP_m_ASF_CROP_CFG_1
        stripe_info->asf_crop_info.last_line << 16 |
        stripe_info->asf_crop_info.first_line;
      i+=27;
    }
  }
  frame_msg[msg_len-1] = 0xABCDEFAA;
  return frame_msg;
}
uint32_t* cpp_create_frame_message_1_2_x(
  struct cpp_frame_info_t *cpp_frame_info, uint32_t* len)
{
  struct cpp_stripe_info *stripe_info;
  uint32_t i, j, k, num_stripes = 0, msg_len;
  uint32_t *frame_msg;
  uint16_t LUT1[24];
  uint16_t LUT2[24];
  uint16_t LUT3[12];
  int16_t  F1[16];
  int16_t  F2[16];
  int16_t  F3[16];
  int16_t  F4[16];
  int16_t  F5[16];
  int32_t LUT1_Value[24];
  int32_t LUT1_Delta[24];
  int32_t LUT2_Value[24];
  int32_t LUT2_Delta[24];
  int32_t LUT3_Value[12];
  int32_t LUT3_Delta[12];
  int32_t checksum;
  uint8_t checksum_enable;
  struct cpp_asf_info *asf_info = &(cpp_frame_info->asf_info);
  uint32_t checksum_mask = asf_info->checksum_en;

  if (asf_info->sp_eff_en == 1) {
    checksum_mask = 0;
  }

  for (i =0; i < cpp_frame_info->num_planes; i++) {
    num_stripes += cpp_frame_info->plane_info[i].num_stripes;
  }
  /*frame info size*/
  msg_len = 126 + 2 + 27 * num_stripes + 3;
  /*Total message size = frame info + header + length + trailer*/
  frame_msg = CAM_MALLOC(sizeof(uint32_t) * msg_len);
  if (!frame_msg){
      CPP_ERR("CAM_MALLOC() failed");
      return NULL;
  }
  memset(frame_msg, 0, sizeof(uint32_t) * msg_len);

  *len = msg_len;

  /*Top Level*/
  frame_msg[0] = 0x3E646D63;
  frame_msg[1] = msg_len - 3;
  frame_msg[2] = 0x6;
  frame_msg[3] = 0;
  frame_msg[4] = 0;
  /*Plane info*/
  frame_msg[5] = (cpp_frame_info->in_plane_fmt << 24) | (cpp_frame_info->out_plane_fmt << 16) |
    cpp_frame_info->plane_info[0].stripe_info1[0].rot_info.rot_cfg << 13 |
    cpp_frame_info->batch_info.batch_size << 1;
  /*Plane address HFR*/
  frame_msg[6] = cpp_frame_info->batch_info.intra_plane_offset[0];
  frame_msg[7] = cpp_frame_info->batch_info.intra_plane_offset[1];
  frame_msg[8] = cpp_frame_info->batch_info.intra_plane_offset[2];
  /*Output Plane HFR*/
  frame_msg[9] = 0x0;
  frame_msg[10] = 0x0;
  frame_msg[11] = 0x0;

  for ( i = 0; i < 16; i++) {
    F1[i] = (int16_t)(Round(asf_info->sobel_h_coeff[i]*(1<<10)));
    F2[i] = (int16_t)(Round(asf_info->sobel_v_coeff[i]*(1<<10)));
    F3[i] = (int16_t)(Round(asf_info->hpf_h_coeff[i]*(1<<10)));
    F4[i] = (int16_t)(Round(asf_info->hpf_v_coeff[i]*(1<<10)));
    F5[i] = (int16_t)(Round(asf_info->lpf_coeff[i]*(1<<10)));
  }
  checksum_enable = checksum_mask & 1;

  if (checksum_enable) {
    checksum = 4*(F1[0] + F1[1] + F1[2]\
      + F1[4] + F1[5] + F1[6]\
      + F1[8] + F1[9] + F1[10])\
      + 2*(F1[3] + F1[7] + F1[11]\
      + F1[12] + F1[13] + F1[14])
      + F1[15];
    if (checksum != 0)
      F1[15] = F1[15]- checksum;
  }

  checksum_enable = (checksum_mask >> 1) & 1;
  if (checksum_enable) {
    checksum = 4*(F2[0] + F2[1] + F2[2]\
      + F2[4] + F2[5] + F2[6]\
      + F2[8] + F2[9] + F2[10])\
      + 2*(F2[3] + F2[7] + F2[11]\
      + F2[12] + F2[13] + F2[14])
      + F2[15];
    if (checksum != 0)
      F2[15] = F2[15]- checksum;
  }
  checksum_enable = (checksum_mask >> 2) & 1;
  if (checksum_enable) {
    checksum = 4*(F3[0] + F3[1] + F3[2]\
      + F3[4] + F3[5] + F3[6]\
      + F3[8] + F3[9] + F3[10])\
      + 2*(F3[3] + F3[7] + F3[11]\
      + F3[12] + F3[13] + F3[14])
      + F3[15];
    if (checksum != 0)
      F3[15] = F3[15]- checksum;
  }

  checksum_enable = (checksum_mask >> 3) & 1;
  if (checksum_enable) {
    checksum = 4*(F4[0] + F4[1] + F4[2]\
      + F4[4] + F4[5] + F4[6]\
      + F4[8] + F4[9] + F4[10])\
      + 2*(F4[3] + F4[7] + F4[11]\
      + F4[12] + F4[13] + F4[14])
      + F4[15];
    if (checksum != 0)
      F4[15] = F4[15]- checksum;
  }

  checksum_enable = (checksum_mask >> 4) & 1 ;
  if (checksum_enable) {
    checksum = 4*(F5[0] + F5[1] + F5[2]\
      + F5[4] + F5[5] + F5[6]\
      + F5[8] + F5[9] + F5[10])\
      + 2*(F5[3] + F5[7] + F5[11]\
      + F5[12] + F5[13] + F5[14])
      + F5[15];
    if (checksum != (1<<10))
      F5[15] = F5[15] + ((1<<10)-checksum);
  }

  for (i = 0; i < 23; i++) {
    LUT1_Delta[i] =
      (int32_t)(Round((asf_info->lut1[i+1]-asf_info->lut1[i])*(1<<4)));
    LUT1_Value[i] =
      (int32_t)(Round((asf_info->lut1[i])*(1<<5)));
    LUT2_Delta[i] =
      (int32_t)(Round((asf_info->lut2[i+1]-asf_info->lut2[i])*(1<<4)));
    LUT2_Value[i] = (int32_t)(Round((asf_info->lut2[i])*(1<<5)));
  }
  LUT1_Delta[23] = 0;
  LUT1_Value[23] = (int32_t)(Round(asf_info->lut1[23]*(1<<5)));
  LUT2_Delta[23] = 0;
  LUT2_Value[23] = (int32_t)(Round(asf_info->lut2[23]*(1<<5)));

  for (i = 0; i < 11; i++) {
    LUT3_Delta[i] =
      (int32_t)(Round((asf_info->lut3[i+1]-asf_info->lut3[i])*(1<<6)));
    LUT3_Value[i] = (int32_t)(Round(asf_info->lut3[i]*(1<<6)));
  }
  LUT3_Delta[11] = 0;
  LUT3_Value[11] = (int32_t)(Round(asf_info->lut3[11]*(1<<6)));

  for (i =0; i < cpp_frame_info->num_planes; i++) {
    frame_msg[12] |= cpp_frame_info->plane_info[i].num_stripes << (i * 10);
  }

  for (i = 0; i < 23; i++) {
    LUT1[i] = ((LUT1_Delta[i]) << 8) | (LUT1_Value[i]);
    LUT2[i] = (( LUT2_Delta[i]) << 8) | (LUT2_Value[i]);
  }
  LUT1[23] = LUT1_Value[23] ;
  LUT2[23] = LUT2_Value[23] ;

  for (i = 0; i < 11; i++) {
    LUT3[i] = ((LUT3_Delta[i]) << 7) | (LUT3_Value[i]);
  }
  LUT3[11] = LUT3_Value[11] ;

  for (i = 0; i < 12; i++) {
    frame_msg[13 + i] = (((LUT1[(i * 2) + 1]) << 16) | (LUT1[i * 2]));
    frame_msg[25 + i] = (((LUT2[(i * 2) + 1]) << 16) | (LUT2[i * 2]));
  }
  for (i = 0; i < 6; i++) {
    frame_msg[37 + i] = (((LUT3[(i * 2) + 1]) << 16) | (LUT3[i * 2]));
  }

  for (i=0; i < 3; i++) {
    struct cpp_bf_info *bf_info = &cpp_frame_info->bf_info[i];
    j= 43 + i * 8;
    for (j = 0; j < 4; j++) {
      k = 43 + i * 8 + j;
      //Q10 and Q8 format for bilateral_scale is calculated in cpp_prepare_bf_info()
      frame_msg[k] = (uint32_t)(bf_info->bilateral_scale[j]);
      frame_msg[k+4] =
        ((uint32_t)(Round(bf_info->noise_threshold[j] * (1 << 4)))) << 8 |
         ((uint32_t)(Round(bf_info->weight[j] * (1 << 4))));
    }
  }

  frame_msg[67] = (((uint32_t)Round(asf_info->sp * (1 << 4))) & 0x1F) << 4 |
                   (asf_info->neg_abs_y1 & 0x1) << 2 |
                   (asf_info->dyna_clamp_en & 0x1) << 1 |
                   (asf_info->sp_eff_en & 0x1);

  frame_msg[68] = (asf_info->clamp_h_ul & 0x1FF) << 9 |
                  (asf_info->clamp_h_ll & 0x1FF);

  frame_msg[69] = (asf_info->clamp_v_ul & 0x1FF) << 9 |
                  (asf_info->clamp_v_ll & 0x1FF);

  frame_msg[70] = ((uint32_t)(Round(asf_info->clamp_scale_max * (1 << 4))) & 0x7F) << 16 |
                  ((uint32_t)(Round(asf_info->clamp_scale_min * (1 << 4))) & 0x7F);

  frame_msg[71] = (asf_info->clamp_offset_max & 0x7F) << 16 |
                  (asf_info->clamp_offset_min & 0x7F);

  frame_msg[72] = asf_info->nz_flag;

  for (i = 0; i < 8; i++) {
    frame_msg[73+i] =
      ((F1[i*2+1]) << 16 )|  ((F1[i*2]) & 0xFFF);
    frame_msg[81+i] =
      ((F2[i*2+1] ) << 16) | ((F2[i*2] ) & 0xFFF);
    frame_msg[89+i] =
      ((F3[i*2+1] ) << 16) | ((F3[i*2] ) & 0xFFF);
    frame_msg[97+i] =
      ((F4[i*2+1]) << 16) |  ((F4[i*2] ) & 0xFFF);
  }
  for (i = 0; i < 8; i++) {
    frame_msg[105+i] =
      ((F5[i*2+1] ) << 16 )| ((F5[i*2] ) & 0xFFF);
  }


  for (i = 0; i < cpp_frame_info->num_planes; i++) {
    j = (113 + 2) + i * 5;
    frame_msg[j] = 0x20 |
      cpp_frame_info->plane_info[i].asf_crop_enable << 4 |
      cpp_frame_info->plane_info[i].bf_crop_enable << 1 |
      cpp_frame_info->plane_info[i].bf_enable;
    if (cpp_frame_info->plane_info[i].scale_info.hscale_en ||
        cpp_frame_info->plane_info[i].scale_info.vscale_en) {
      frame_msg[j] |= 0x4;
    }
    if (cpp_frame_info->plane_info[i].input_plane_fmt == PLANE_Y &&
        cpp_frame_info->asf_mode != 0) {
      frame_msg[j] |= 0x8;
    }

    frame_msg[j+1] =
      cpp_frame_info->plane_info[i].scale_info.v_scale_fir_algo << 12 |
      cpp_frame_info->plane_info[i].scale_info.h_scale_fir_algo << 8 |
      cpp_frame_info->plane_info[i].scale_info.v_scale_algo << 5 |
      cpp_frame_info->plane_info[i].scale_info.h_scale_algo << 4 |
      cpp_frame_info->plane_info[i].scale_info.subsample_en << 3 |
      cpp_frame_info->plane_info[i].scale_info.upsample_en << 2 |
      cpp_frame_info->plane_info[i].scale_info.vscale_en << 1 |
      cpp_frame_info->plane_info[i].scale_info.hscale_en;
    frame_msg[j+2] = cpp_frame_info->plane_info[i].scale_info.phase_h_step;
    frame_msg[j+3] = cpp_frame_info->plane_info[i].scale_info.phase_v_init;
    frame_msg[j+4] = cpp_frame_info->plane_info[i].scale_info.phase_v_step;
  }

  i = 128 + 2;
  for (j = 0; j < cpp_frame_info->num_planes; j++) {
    for (k = 0; k < cpp_frame_info->plane_info[j].num_stripes; k++) {
      stripe_info = &cpp_frame_info->plane_info[j].stripe_info1[k];
      frame_msg[i + 0] = //STRIPE[0]_PP_m_ROT_CFG_0
        stripe_info->rot_info.rot_cfg << 24 | (stripe_info->rot_info.block_size - 1);
      frame_msg[i + 1] = //STRIPE[0]_PP_m_ROT_CFG_1
        (stripe_info->rot_info.block_height - 1) << 16 |
        (stripe_info->rot_info.block_width - 1);
      frame_msg[i + 2] = //STRIPE[0]_PP_m_ROT_CFG_2
        stripe_info->rot_info.rowIndex1 << 16 |
        stripe_info->rot_info.rowIndex0;
      frame_msg[i + 3] = //STRIPE[0]_PP_m_ROT_CFG_3
        stripe_info->rot_info.colIndex1 << 16 |
        stripe_info->rot_info.colIndex0;
      frame_msg[i + 4] = //STRIPE[0]_PP_m_ROT_CFG_4
        stripe_info->rot_info.modValue << 16 |
        stripe_info->rot_info.initIndex;
      frame_msg[i + 5] = stripe_info->fe_info.buffer_ptr; //STRIPE[0]_FE_n_RD_PNTR
      frame_msg[i + 6] = //STRIPE[0]_FE_n_RD_BUFFER_SIZE
        (stripe_info->fe_info.buffer_height - 1) << 16 |
        (stripe_info->fe_info.buffer_width - 1);
      frame_msg[i + 7] = //STRIPE[0]_FE_n_RD_STRIDE
        stripe_info->fe_info.buffer_stride;
      frame_msg[i + 8] = //STRIPE[0]_FE_n_SWC_RD_BLOCK_DIMENSION
        (stripe_info->fe_info.block_height - 1) << 16 |
        (stripe_info->fe_info.block_width- 1);
      frame_msg[i + 9] = //STRIPE[0]_FE_n_SWC_RD_BLOCK_HPAD
      stripe_info->fe_info.right_pad << 16 |
      stripe_info->fe_info.left_pad;
      frame_msg[i + 10] = //STRIPE[0]_FE_n_SWC_RD_BLOCK_VPAD
      stripe_info->fe_info.bottom_pad << 16 |
      stripe_info->fe_info.top_pad;
      frame_msg[i + 11] = stripe_info->we_info.buffer_ptr[0]; //STRIPE[0]_WE_PLN_0_WR_PNTR
      frame_msg[i + 12] = stripe_info->we_info.buffer_ptr[1]; //STRIPE[0]_WE_PLN_1_WR_PNTR
      frame_msg[i + 13] = stripe_info->we_info.buffer_ptr[2]; //STRIPE[0]_WE_PLN_2_WR_PNTR
      frame_msg[i + 14] = stripe_info->we_info.buffer_ptr[3]; //STRIPE[0]_WE_PLN_3_WR_PNTR
      frame_msg[i + 15] = //STRIPE[0]_WE_PLN_n_WR_BUFFER_SIZE
        stripe_info->we_info.buffer_height << 16 |
        stripe_info->we_info.buffer_width;
      frame_msg[i + 16] = stripe_info->we_info.buffer_stride; //STRIPE[0]_WE_PLN_n_WR_STRIDE
      frame_msg[i + 17] = //STRIPE[0]_WE_PLN_n_WR_CFG_0
        (stripe_info->we_info.blocks_per_row - 1) << 16 |
        (stripe_info->we_info.blocks_per_col - 1);
      frame_msg[i + 18] = stripe_info->we_info.h_step; //STRIPE[0]_WE_PLN_n_WR_CFG_1
      frame_msg[i + 19] = stripe_info->we_info.v_step; //STRIPE[0]_WE_PLN_n_WR_CFG_2
      frame_msg[i + 20] = //STRIPE[0]_WE_PLN_n_WR_CFG_3
      (stripe_info->we_info.h_init) << 16 |
      (stripe_info->we_info.v_init);
      frame_msg[i + 21] = //STRIPE[0]_PP_m_BF_CROP_CFG_0
        stripe_info->bf_crop_info.last_pixel << 16 |
        stripe_info->bf_crop_info.first_pixel;
      frame_msg[i + 22] = //STRIPE[0]_PP_m_BF_CROP_CFG_1
        stripe_info->bf_crop_info.last_line << 16 |
        stripe_info->bf_crop_info.first_line;
      frame_msg[i + 23] = //STRIPE[0]_PP_m_SCALE_OUTPUT_CFG
        (stripe_info->scale_info.block_height-1) << 16 |
        (stripe_info->scale_info.block_width-1);
      frame_msg[i + 24] = //STRIPE[0]_PP_m_SCALE_PHASEH_INIT
        stripe_info->scale_info.phase_h_init;
      frame_msg[i + 25] = //STRIPE[0]_PP_m_ASF_CROP_CFG_0
        stripe_info->asf_crop_info.last_pixel << 16 |
        stripe_info->asf_crop_info.first_pixel;
      frame_msg[i + 26] = //STRIPE[0]_PP_m_ASF_CROP_CFG_1
        stripe_info->asf_crop_info.last_line << 16 |
        stripe_info->asf_crop_info.first_line;
      i+=27;
    }
  }
  frame_msg[msg_len-1] = 0xABCDEFAA;
  return frame_msg;
}

void cpp_create_frame_message(cpp_hardware_t *cpphw,
  struct msm_cpp_frame_info_t *msm_cpp_frame_info,
  struct cpp_frame_info_t *cpp_frame_info)
{
  if (!msm_cpp_frame_info || !cpp_frame_info || !cpphw) {
    CPP_ERR("failed msm_cpp_frame_info %p, cpp_frame_info %p, cpphw %p",
      msm_cpp_frame_info, cpp_frame_info, cpphw);
    return;
  }
  cpp_firmware_version_t fw_version =
    cpp_hardware_get_fw_version(cpphw);
  switch (fw_version) {
  case CPP_FW_VERSION_1_2_0:
    msm_cpp_frame_info->cpp_cmd_msg =
      cpp_create_frame_message_1_2_x(cpp_frame_info,
        &(msm_cpp_frame_info->msg_len));
    break;
  case CPP_FW_VERSION_1_4_0:
    msm_cpp_frame_info->cpp_cmd_msg =
      cpp_create_frame_message_1_4_x(cpp_frame_info,
        &(msm_cpp_frame_info->msg_len));
    break;
  case CPP_FW_VERSION_1_5_0:
  case CPP_FW_VERSION_1_5_1:
  case CPP_FW_VERSION_1_5_2:
    msm_cpp_frame_info->cpp_cmd_msg =
      cpp_create_frame_message_1_5(cpp_frame_info,
        &(msm_cpp_frame_info->msg_len),fw_version);
    break;
  case CPP_FW_VERSION_1_6_0:
    msm_cpp_frame_info->cpp_cmd_msg =
      cpp_create_frame_message_1_6_x(cpp_frame_info,
        &(msm_cpp_frame_info->msg_len));
    break;
  case CPP_FW_VERSION_1_8_0:
    msm_cpp_frame_info->cpp_cmd_msg =
      cpp_create_frame_message_1_8(cpp_frame_info,
        &(msm_cpp_frame_info->msg_len));
    break;
  case CPP_FW_VERSION_1_10_0:
    msm_cpp_frame_info->cpp_cmd_msg =
      cpp_create_frame_message_1_10(cpp_frame_info,
        &(msm_cpp_frame_info->msg_len));
    break;
  case CPP_FW_VERSION_1_12_0:
  case CPP_FW_VERSION_1_12_1:
    msm_cpp_frame_info->cpp_cmd_msg =
      cpp_create_frame_message_1_12(cpp_frame_info,
        &(msm_cpp_frame_info->msg_len));
    break;
  default:
    CPP_ERR("failed, unsupported fw version: %d", fw_version);
    return;
  }
}

void cpp_params_prepare_frame_info(cpp_hardware_t *cpphw,
  struct cpp_frame_info_t *frame_info,
  struct msm_cpp_frame_info_t *out_info)
{
  struct cpp_striping_algorithm_info *strip_info;
  struct cpp_stripe_info *stripe_info1;
  struct cpp_plane_info_t *cur_plane_info;
  uint32_t i, j, k, strip_num, num_strips;
  int ubwc_en = 0, mmu_pf_en = 0, tnr_en = 0, dsdn_en = 0, batch_dup = 0;
  int64_t pad_left = 0, pad_top = 0;

  out_info->frame_id = frame_info->frame_id;
  out_info->frame_type = frame_info->frame_type;
  out_info->timestamp = frame_info->timestamp;
  out_info->output_buffer_info[0].processed_divert = frame_info->processed_divert;
  out_info->src_fd = frame_info->plane_info[0].src_fd;
  out_info->dst_fd = frame_info->plane_info[0].dst_fd;
  out_info->input_buffer_info.fd = frame_info->plane_info[0].src_fd;
  out_info->input_buffer_info.index = frame_info->buff_index;
  out_info->input_buffer_info.native_buff = frame_info->native_buff;
  out_info->input_buffer_info.identity = frame_info->in_buff_identity;
#ifdef CACHE_PHASE2
  out_info->input_buffer_info.buffer_access = frame_info->in_buff_buffer_access;
#else
  // By now we should have saved input buffer access flags in cookie
#endif
  out_info->identity = frame_info->identity;
  out_info->cookie = frame_info->cookie;
  out_info->we_disable = frame_info->we_disable;
  out_info->duplicate_output = (int32_t) frame_info->dup_output;
  out_info->duplicate_identity = frame_info->dup_buff_info.identity;
  /* Update duplicate buffer info parameters */
  out_info->duplicate_buffer_info.identity =
    frame_info->dup_buff_info.identity;
  out_info->duplicate_buffer_info.processed_divert =
    frame_info->dup_buff_info.processed_divert;
  out_info->duplicate_buffer_info.fd = frame_info->dup_buff_info.fd;
  out_info->duplicate_buffer_info.index = frame_info->dup_buff_info.index;
  out_info->duplicate_buffer_info.native_buff =
    frame_info->dup_buff_info.native_buff;
  out_info->output_buffer_info[0].native_buff =
    frame_info->out_buff_info.native_buff;
  out_info->output_buffer_info[0].fd = frame_info->out_buff_info.fd;
  out_info->output_buffer_info[0].index = frame_info->out_buff_info.index;
  out_info->output_buffer_info[0].offset = frame_info->out_buff_info.offset;
  out_info->output_buffer_info[0].processed_divert =
    frame_info->out_buff_info.processed_divert;
#ifdef CACHE_PHASE2
  out_info->output_buffer_info[0].buffer_access =
    frame_info->out_buff_info.buffer_access;
#endif
  out_info->tnr_scratch_buffer_info[0].fd =
    frame_info->tnr_input_scratch_buff_info.fd;
  out_info->tnr_scratch_buffer_info[0].index =
    frame_info->tnr_input_scratch_buff_info.index;
  out_info->tnr_scratch_buffer_info[0].offset =
    frame_info->tnr_input_scratch_buff_info.offset;
  out_info->tnr_scratch_buffer_info[0].processed_divert =
    frame_info->tnr_input_scratch_buff_info.processed_divert;
  out_info->tnr_scratch_buffer_info[0].native_buff =
    frame_info->tnr_input_scratch_buff_info.native_buff;
  out_info->tnr_scratch_buffer_info[1].fd =
    frame_info->tnr_output_scratch_buff_info.fd;
  out_info->tnr_scratch_buffer_info[1].index =
    frame_info->tnr_output_scratch_buff_info.index;
  out_info->tnr_scratch_buffer_info[1].offset =
    frame_info->tnr_output_scratch_buff_info.offset;
  out_info->tnr_scratch_buffer_info[1].processed_divert =
    frame_info->tnr_output_scratch_buff_info.processed_divert;
  out_info->tnr_scratch_buffer_info[1].native_buff =
    frame_info->tnr_output_scratch_buff_info.native_buff;
  out_info->batch_info.batch_mode = frame_info->batch_info.batch_mode;
  out_info->batch_info.batch_size= frame_info->batch_info.batch_size;
  for (i = 0; i < frame_info->num_planes; i++) {
    out_info->batch_info.intra_plane_offset[i] =
      frame_info->batch_info.intra_plane_offset[i];
  }
  out_info->batch_info.pick_preview_idx= frame_info->batch_info.pick_preview_idx;
  if (cpphw->fw_version >= CPP_FW_VERSION_1_8_0) {
    out_info->stripe_info_offset = 9;
  } else {
    out_info->stripe_info_offset = 12;
  }
  for (i = 0; i < frame_info->num_planes; i++) {
    cur_plane_info = &frame_info->plane_info[i];
    cur_plane_info->horizontal_center =
      frame_info->p_hw_params->input_info.plane_info[i].horizontal_center;
    cur_plane_info->vertical_center =
      frame_info->p_hw_params->input_info.plane_info[i].vertical_center;
    cpp_init_frame_params(cur_plane_info);
    if (cpphw->hwinfo.version == CPP_HW_VERSION_5_0_0 ||
        cpphw->hwinfo.version == CPP_HW_VERSION_5_1_0) {
      set_start_of_frame_parameters_1_8(cur_plane_info);
    } else if (cpphw->hwinfo.version == CPP_HW_VERSION_6_0_0 ||
               cpphw->hwinfo.version == CPP_HW_VERSION_6_1_0 ||
               cpphw->hwinfo.version == CPP_HW_VERSION_6_1_2 ||
               cpphw->hwinfo.version == CPP_HW_VERSION_6_1_4) {
      set_start_of_frame_parameters_1_10(cur_plane_info);
    } else {
      set_start_of_frame_parameters(cur_plane_info);
    }
    CPP_FRAME_DBG("PLANE %d, i %d", frame_info->num_planes, i);
    cpp_debug_input_info(cur_plane_info);

    CPP_FRAME_DBG("wmcus: %d hmcus: %d num: %d\n",
         cur_plane_info->frame_width_mcus, cur_plane_info->frame_height_mcus,
         cur_plane_info->num_stripes);
    strip_info = CAM_MALLOC(sizeof(struct cpp_striping_algorithm_info) *
                        cur_plane_info->num_stripes);
    if (!strip_info){
        CPP_ERR("CAM_MALLOC() failed");
        return;
    }
    memset(strip_info, 0, sizeof(struct cpp_striping_algorithm_info) *
           cur_plane_info->num_stripes);

    stripe_info1 = CAM_MALLOC(sizeof(struct cpp_stripe_info) *
                        cur_plane_info->num_stripes);
    if (!stripe_info1){
        CPP_ERR("CAM_MALLOC() failed");
        free(strip_info);
        return;
    }
    memset(stripe_info1, 0, sizeof(struct cpp_stripe_info) *
           cur_plane_info->num_stripes);

    cur_plane_info->stripe_info = strip_info;
    cur_plane_info->stripe_info1 = stripe_info1;

    cpp_init_strip_info(cpphw, cur_plane_info, strip_info,
      cur_plane_info->num_stripes);

    for (j = 0; j < cur_plane_info->frame_height_mcus; j++) {
      for (k = 0; k < cur_plane_info->frame_width_mcus; k++) {
        strip_num = j * cur_plane_info->frame_width_mcus + k;
        CPP_FRAME_DBG("Plane: %d, x stripe: %d, y stripe: %d\n", i, k, j);
        if (cpphw->hwinfo.version == CPP_HW_VERSION_5_0_0 ||
          cpphw->hwinfo.version == CPP_HW_VERSION_5_1_0) {
          run_TF_logic_1_8(cur_plane_info, k, j, &strip_info[strip_num]);
          run_TW_logic_1_8(cur_plane_info, k, j, &strip_info[strip_num]);
        } else if (cpphw->hwinfo.version == CPP_HW_VERSION_6_0_0 ||
          cpphw->hwinfo.version == CPP_HW_VERSION_6_1_0 ||
          cpphw->hwinfo.version == CPP_HW_VERSION_6_1_2 ||
          cpphw->hwinfo.version == CPP_HW_VERSION_6_1_4) {
          run_TF_logic_1_10(cur_plane_info, k, j, &strip_info[strip_num]);
          run_TW_logic_1_10(cur_plane_info, k, j, &strip_info[strip_num]);
        } else {
          run_TF_logic(cur_plane_info, k, j, &strip_info[strip_num]);
          run_TW_logic(cur_plane_info, k, j, &strip_info[strip_num]);
        }

        if (cpphw->hwinfo.version == CPP_HW_VERSION_6_1_0 ||
            cpphw->hwinfo.version == CPP_HW_VERSION_6_1_2 ||
            cpphw->hwinfo.version == CPP_HW_VERSION_6_1_4)
          cpp_hw_params_radial_1_12(cur_plane_info, k, j, &pad_left, &pad_top,
            &strip_info[strip_num]);

        cpp_debug_strip_info(&strip_info[strip_num], strip_num);
        stripe_info1[strip_num].rotation = cur_plane_info->rotate;
        stripe_info1[strip_num].mirror = cur_plane_info->mirror;
        CPP_FRAME_LOW("debug: mirror %d\n", stripe_info1[strip_num].mirror);

        if(cpphw->hwinfo.version == CPP_HW_VERSION_5_0_0 ||
           cpphw->hwinfo.version == CPP_HW_VERSION_5_1_0 ||
           cpphw->hwinfo.version == CPP_HW_VERSION_6_0_0 ||
           cpphw->hwinfo.version == CPP_HW_VERSION_6_1_0 ||
           cpphw->hwinfo.version == CPP_HW_VERSION_6_1_2 ||
           cpphw->hwinfo.version == CPP_HW_VERSION_6_1_4) {
          cpp_prepare_crop_info_1_8(cur_plane_info, strip_num);
        } else {
          cpp_prepare_crop_info(cur_plane_info, strip_num);
        }

        if(cpphw->hwinfo.version == CPP_HW_VERSION_6_1_2 ||
           cpphw->hwinfo.version == CPP_HW_VERSION_6_1_4) {
          cur_plane_info->ubwc_hbb = UBWC_HIGHEST_BANK_BIT_14;
        } else {
          cur_plane_info->ubwc_hbb = UBWC_HIGHEST_BANK_BIT;
        }

        cpp_prepare_stripe_scale_info(cur_plane_info, strip_num);
        cpp_prepare_rotation_info(cur_plane_info, strip_num);
        if (cur_plane_info->tnr_enable) {
          cpp_prepare_tnr_fetch_engine_info(cur_plane_info, strip_num);
          cpp_prepare_tnr_write_engine_info(cur_plane_info, strip_num);
        }
        if (cur_plane_info->dsdn_enable) {
          dsdn_en |= cur_plane_info->dsdn_enable;
          cpp_prepare_dsdn_fetch_engine_info(cur_plane_info, strip_num);
        }
        cpp_prepare_fetch_engine_info(cur_plane_info, strip_num);
        cpp_prepare_write_engine_info(cur_plane_info, strip_num);
        if (cur_plane_info->dsdn_enable) {
            cpp_prepare_dsdn_stripe_info(cur_plane_info, strip_num);
        }
      }
    }
    if (cpphw->hwinfo.version == CPP_HW_VERSION_6_0_0 ||
        cpphw->hwinfo.version == CPP_HW_VERSION_6_1_0 ||
        cpphw->hwinfo.version == CPP_HW_VERSION_6_1_2 ||
        cpphw->hwinfo.version == CPP_HW_VERSION_6_1_4) {
      cpp_prepare_mmu_prefetch_fetch_engine_config(cur_plane_info, i);
      cpp_prepare_mmu_prefetch_write_engine_config(cur_plane_info, i);
    }
    cpp_prepare_plane_scale_info(cur_plane_info);

    ubwc_en |= cur_plane_info->tile_output_enable;
    tnr_en |= cur_plane_info->tnr_enable;
    mmu_pf_en |= cur_plane_info->mmu_prefetch_fe_info.l1_mmu_pf_en;
  }
  cpp_prepare_bf_info(frame_info);
  cpp_prepare_pbf_info(frame_info);
  cpp_create_frame_message(cpphw, out_info, frame_info);
  /*
   * Update feature mask info if feature is enabled on any of the planes
   * Feature mask  bit 0 - WNR
   * Feature mask  bit 1 - ASF
   * Feature mask  bit 2 - TNR
   * Feature mask  bit 3 - ROTATION /FLIP
   * Feature mask  bit 4 - CROP
   * Feature mask  bit 5 - UBWC
   * Feature mask  bit 6   CPP CDS (DSDN)
   * Feature mask  bit 7   MMU prefetch
   * Feature mask  bit 8   BATCH_DUP
   * Feature mask  bit 9   SCALE
   */
  if(cpphw->hwinfo.version == CPP_HW_VERSION_5_0_0 ||
    cpphw->hwinfo.version == CPP_HW_VERSION_5_1_0 ||
    cpphw->hwinfo.version == CPP_HW_VERSION_6_0_0 ||
    cpphw->hwinfo.version == CPP_HW_VERSION_6_1_0 ||
    cpphw->hwinfo.version == CPP_HW_VERSION_6_1_2 ||
    cpphw->hwinfo.version == CPP_HW_VERSION_6_1_4)
    batch_dup = 1;
  out_info->feature_mask |= (tnr_en << 2 ) | (ubwc_en << 5) |
    (dsdn_en << 6) | (mmu_pf_en << 7) | (batch_dup << 8);
  CPP_FRAME_DBG("tnr %d ubwc_en %d, mmu_pf_en %d, dsnd_en %d, feature_mask %x frame_id:%d",
    tnr_en, ubwc_en, mmu_pf_en, dsdn_en, out_info->feature_mask,out_info->frame_id);

  CPP_FRAME_LOW("output duplication %d", out_info->duplicate_output);
  for (i = 0; i < frame_info->num_planes; i++) {
    cur_plane_info = &frame_info->plane_info[i];
    free(cur_plane_info->stripe_block_width);
    free(cur_plane_info->stripe_block_height);
    free(cur_plane_info->stripe_info);
    free(cur_plane_info->stripe_info1);
  }
  out_info->num_strips =
    (out_info->cpp_cmd_msg[out_info->stripe_info_offset] & 0x3FF) +
    ((out_info->cpp_cmd_msg[out_info->stripe_info_offset] >> 10) & 0x3FF) +
    ((out_info->cpp_cmd_msg[out_info->stripe_info_offset] >> 20) & 0x3FF);

  out_info->first_stripe_index = 0;
  out_info->last_stripe_index = out_info->num_strips - 1;
  out_info->first_payload = 1;
  out_info->last_payload = 1;
  out_info->partial_frame_indicator = 0;

}

/*========================= ASF 7x7 functions and APIs ======================
 * This ASF version is used for platforms with CPP firmware versions:
 *  v1.2.x
 *  v1.4.x
 *  v1.5.x
 *===========================================================================*/
static void cpp_hw_params_set_emboss_effect(cpp_hardware_params_t *hw_params)
{
  uint32_t i = 0;
  cpp_params_asf_info_t *asf_info = &hw_params->asf_info;
  uint32_t asf_info_filter_k_entries =
      cpp_hw_params_get_num_k_entries(asf_info->cpp_fw_version);

  asf_info->sp_eff_en = 1;
  asf_info->sp = 0;
  asf_info->nz_flag = asf_info->nz_flag_f2 = asf_info->nz_flag_f3_f5 = 0x1A90;
  asf_info->dyna_clamp_en = 0;
  for (i = 0; i < asf_info_filter_k_entries; i++) {
    asf_info->sobel_v_coeff[i] = 0.0;
  }
  for (i = 0; i < asf_info_filter_k_entries; i++) {
    asf_info->hpf_h_coeff[i] = 0.0;
  }
  for (i = 0; i < CPP_ASF_LUT12_ENTRIES_1_2_x; i++) {
    asf_info->lut1[i] = 0.0;
  }
  for (i = 0; i < CPP_ASF_LUT3_ENTRIES_1_2_x; i++) {
    asf_info->lut3[i] = 0.0;
  }

  for (i = 0; i < asf_info_filter_k_entries; i++) {
    asf_info->sobel_h_coeff[i] = 0.0;
  }
  asf_info->neg_abs_y1 = 0;
  for (i = 0; i < asf_info_filter_k_entries; i++) {
    asf_info->hpf_v_coeff[i] = 0.0;
  }
  for (i = 0; i < asf_info_filter_k_entries; i++) {
    asf_info->lpf_coeff[i] = 0.0;
  }

  if (asf_info_filter_k_entries == 25) { // If 5x5 kernel
   asf_info->hpf_v_coeff[6] = -0.25;
   asf_info->hpf_v_coeff[18] = 0.25;
   asf_info->lpf_coeff[12] = 0.125;
  } else if (asf_info_filter_k_entries == 16) { //If 4x4 kernel
   asf_info->hpf_v_coeff[0] = -0.25;
   asf_info->hpf_v_coeff[10] = 0.25;
   asf_info->lpf_coeff[5] = 0.125;
  }

  for (i = 0; i < CPP_ASF_LUT12_ENTRIES_1_2_x; i++) {
    asf_info->lut2[i] = 1.0;
  }
  asf_info->clamp_h_ul = 128;
  asf_info->clamp_h_ll = 128;
  asf_info->clamp_v_ul = 255;
  asf_info->clamp_v_ll = -255;
  asf_info->clamp_tl_ul = 255;
  asf_info->clamp_tl_ll = -255;
}

static void cpp_hw_params_set_sketch_effect(cpp_hardware_params_t *hw_params)
{
  uint32_t i = 0;
  cpp_params_asf_info_t *asf_info = &hw_params->asf_info;
  uint32_t asf_info_filter_k_entries =
    cpp_hw_params_get_num_k_entries(asf_info->cpp_fw_version);

  asf_info->sp_eff_en = 1;
  asf_info->sp = 0;
  asf_info->nz_flag = asf_info->nz_flag_f2 = asf_info->nz_flag_f3_f5 = 0x1A90;
  asf_info->dyna_clamp_en = 0;
  for (i = 0; i < asf_info_filter_k_entries; i++) {
    asf_info->sobel_v_coeff[i] = 0.0;
  }
  for (i = 0; i < asf_info_filter_k_entries; i++) {
    asf_info->hpf_h_coeff[i] = 0.0;
  }
  for (i = 0; i < CPP_ASF_LUT12_ENTRIES_1_2_x; i++) {
    asf_info->lut1[i] = 0.0;
  }
  for (i = 0; i < CPP_ASF_LUT3_ENTRIES_1_2_x; i++) {
    asf_info->lut3[i] = 0.0;
  }

  for (i = 0; i < asf_info_filter_k_entries; i++) {
    asf_info->sobel_h_coeff[i] = 0.0;
  }

  asf_info->neg_abs_y1 = 1;
  for (i = 0; i < asf_info_filter_k_entries; i++) {
    asf_info->hpf_v_coeff[i] = 0.0;
  }
  for (i = 0; i < asf_info_filter_k_entries; i++) {
    asf_info->lpf_coeff[i] = 0.0;
  }

  if (asf_info_filter_k_entries == 25) { // If 5x5 kernel i.e 9x9 ASF
    asf_info->sobel_h_coeff[12] = -0.0625;
    asf_info->sobel_h_coeff[13] = -0.125;
    asf_info->sobel_h_coeff[14] = -0.0625;
    asf_info->sobel_h_coeff[17] = -0.125;
    asf_info->sobel_h_coeff[18] = -0.25;
    asf_info->sobel_h_coeff[19] = -0.25;
    asf_info->sobel_h_coeff[22] = -0.0625;
    asf_info->sobel_h_coeff[23] = -0.25;
    asf_info->lpf_coeff[24] = 0.25;
  } else if (asf_info_filter_k_entries == 16) { //If 4x4 kernel i.e 7x7 ASF
    asf_info->sobel_h_coeff[5] = -0.0625;
    asf_info->sobel_h_coeff[6] = -0.125;
    asf_info->sobel_h_coeff[7] = -0.0625;
    asf_info->sobel_h_coeff[9] = -0.125;
    asf_info->sobel_h_coeff[10] = -0.25;
    asf_info->sobel_h_coeff[11] = -0.25;
    asf_info->sobel_h_coeff[13] = -0.0625;
    asf_info->sobel_h_coeff[14] = -0.25;
    asf_info->lpf_coeff[15] = 0.25;
  }

  for (i = 0; i < CPP_ASF_LUT12_ENTRIES_1_2_x; i++) {
    asf_info->lut2[i] = 0.0;
  }
  asf_info->clamp_h_ul = 255;
  asf_info->clamp_h_ll = -255;
  asf_info->clamp_v_ul = 192;
  asf_info->clamp_v_ll = 192;
  asf_info->clamp_tl_ul = 255;
  asf_info->clamp_tl_ll = -255;
}

static void cpp_hw_params_set_neon_effect(cpp_hardware_params_t *hw_params)
{
  uint32_t i = 0;
  cpp_params_asf_info_t *asf_info = &hw_params->asf_info;
  uint32_t asf_info_filter_k_entries =
    cpp_hw_params_get_num_k_entries(asf_info->cpp_fw_version);

  asf_info->sp_eff_en = 1;
  asf_info->sp = 0;
  asf_info->nz_flag = asf_info->nz_flag_f2 = asf_info->nz_flag_f3_f5 = 0x1A90;
  asf_info->dyna_clamp_en = 0;
  for (i = 0; i < asf_info_filter_k_entries; i++) {
    asf_info->sobel_v_coeff[i] = 0.0;
  }
  for (i = 0; i < asf_info_filter_k_entries; i++) {
    asf_info->hpf_h_coeff[i] = 0.0;
  }
  for (i = 0; i < CPP_ASF_LUT12_ENTRIES_1_2_x; i++) {
    asf_info->lut1[i] = 0.0;
  }
  for (i = 0; i < CPP_ASF_LUT3_ENTRIES_1_2_x; i++) {
    asf_info->lut3[i] = 0.0;
  }

  for (i = 0; i < asf_info_filter_k_entries; i++) {
    asf_info->sobel_h_coeff[i] = 0.0;
  }

  if (asf_info_filter_k_entries == 25) {
    asf_info->sobel_h_coeff[18] = 0.25;
    asf_info->sobel_h_coeff[19] = 0.25;
    asf_info->sobel_h_coeff[23] = 0.25;
  } else if (asf_info_filter_k_entries == 16) {
    asf_info->sobel_h_coeff[10] = 0.25;
    asf_info->sobel_h_coeff[11] = 0.25;
    asf_info->sobel_h_coeff[14] = 0.25;
  }
  asf_info->neg_abs_y1 = 0;
  for (i = 0; i < asf_info_filter_k_entries; i++) {
    asf_info->hpf_v_coeff[i] = 0.0;
  }
  for (i = 0; i < asf_info_filter_k_entries; i++) {
    asf_info->lpf_coeff[i] = 0.0;
  }
  for (i = 0; i < CPP_ASF_LUT12_ENTRIES_1_2_x; i++) {
    asf_info->lut2[i] = 0.0;
  }
  asf_info->clamp_h_ul = 255;
  asf_info->clamp_h_ll = -255;
  asf_info->clamp_v_ul = 0;
  asf_info->clamp_v_ll = 0;
  asf_info->clamp_tl_ul = 255;
  asf_info->clamp_tl_ll = -255;
}

//Default value for 1.5FW ?
static void cpp_hw_params_update_default_asf_params(
  cpp_hardware_params_t *hw_params)
{
  uint32_t i = 0;
  cpp_params_asf_info_t *asf_info = &hw_params->asf_info;

  CPP_ASF_LOW("fill default params");
  if (hw_params->asf_mode == CPP_PARAM_ASF_DUAL_FILTER ||
      hw_params->asf_mode == CPP_PARAM_ASF_BEAUTYSHOT ||
      hw_params->scene_mode_on) {
    asf_info->sobel_h_coeff[0] = 0;
    asf_info->sobel_h_coeff[1] = 0;
    asf_info->sobel_h_coeff[2] = -0.010;
    asf_info->sobel_h_coeff[3] = -0.010;
    asf_info->sobel_h_coeff[4] = 0;
    asf_info->sobel_h_coeff[5] = -0.010;
    asf_info->sobel_h_coeff[6] = -0.030;
    asf_info->sobel_h_coeff[7] = -0.030;
    asf_info->sobel_h_coeff[8] = -0.010;
    asf_info->sobel_h_coeff[9] = -0.030;
    asf_info->sobel_h_coeff[10] = 0;
    asf_info->sobel_h_coeff[11] = 0.070;
    asf_info->sobel_h_coeff[12] = -0.010;
    asf_info->sobel_h_coeff[13] = -0.030;
    asf_info->sobel_h_coeff[14] = 0.070;
    asf_info->sobel_h_coeff[15] = 0.240;

    asf_info->sobel_v_coeff[0] = 0;
    asf_info->sobel_v_coeff[1] = 0;
    asf_info->sobel_v_coeff[2] = -0.010;
    asf_info->sobel_v_coeff[3] = -0.010;
    asf_info->sobel_v_coeff[4] = 0;
    asf_info->sobel_v_coeff[5] = -0.010;
    asf_info->sobel_v_coeff[6] = -0.030;
    asf_info->sobel_v_coeff[7] = -0.030;
    asf_info->sobel_v_coeff[8] = -0.010;
    asf_info->sobel_v_coeff[9] = -0.030;
    asf_info->sobel_v_coeff[10] = 0;
    asf_info->sobel_v_coeff[11] = 0.070;
    asf_info->sobel_v_coeff[12] = -0.010;
    asf_info->sobel_v_coeff[13] = -0.030;
    asf_info->sobel_v_coeff[14] = 0.07;
    asf_info->sobel_v_coeff[15] = 0.24;

    asf_info->hpf_h_coeff[0] = 0;
    asf_info->hpf_h_coeff[1] = 0;
    asf_info->hpf_h_coeff[2] = -0.010;
    asf_info->hpf_h_coeff[3] = -0.010;
    asf_info->hpf_h_coeff[4] = 0;
    asf_info->hpf_h_coeff[5] = -0.010;
    asf_info->hpf_h_coeff[6] = -0.030;
    asf_info->hpf_h_coeff[7] = -0.030;
    asf_info->hpf_h_coeff[8] = -0.010;
    asf_info->hpf_h_coeff[9] = -0.030;
    asf_info->hpf_h_coeff[10] = 0;
    asf_info->hpf_h_coeff[11] = 0.070;
    asf_info->hpf_h_coeff[12] = -0.010;
    asf_info->hpf_h_coeff[13] = -0.030;
    asf_info->hpf_h_coeff[14] = 0.070;
    asf_info->hpf_h_coeff[15] = 0.240;

    asf_info->hpf_v_coeff[0] = 0;
    asf_info->hpf_v_coeff[1] = 0;
    asf_info->hpf_v_coeff[2] = -0.010;
    asf_info->hpf_v_coeff[3] = -0.010;
    asf_info->hpf_v_coeff[4] = 0;
    asf_info->hpf_v_coeff[5] = -0.010;
    asf_info->hpf_v_coeff[6] = -0.030;
    asf_info->hpf_v_coeff[7] = -0.030;
    asf_info->hpf_v_coeff[8] = -0.010;
    asf_info->hpf_v_coeff[9] = -0.030;
    asf_info->hpf_v_coeff[10] = 0;
    asf_info->hpf_v_coeff[11] = 0.070;
    asf_info->hpf_v_coeff[12] = -0.010;
    asf_info->hpf_v_coeff[13] = -0.030;
    asf_info->hpf_v_coeff[14] = 0.07;
    asf_info->hpf_v_coeff[15] = 0.24;

    asf_info->lpf_coeff[0] = 0;
    asf_info->lpf_coeff[1] = 0;
    asf_info->lpf_coeff[2] = 0;
    asf_info->lpf_coeff[3] = 0;
    asf_info->lpf_coeff[4] = 0;
    asf_info->lpf_coeff[6] = 0;
    asf_info->lpf_coeff[7] = 0;
    asf_info->lpf_coeff[8] = 0;
    asf_info->lpf_coeff[9] = 0;
    asf_info->lpf_coeff[10] = 0;
    asf_info->lpf_coeff[11] = 0;
    asf_info->lpf_coeff[12] = 0;
    asf_info->lpf_coeff[13] = 0;
    asf_info->lpf_coeff[14] = 0;
    asf_info->lpf_coeff[15] = 1.0;

    asf_info->lut1[0] =  0.3502;
    asf_info->lut1[1] =  0.6286;
    asf_info->lut1[2] =  1.0791;
    asf_info->lut1[3] =  1.2971;
    asf_info->lut1[4] =  1.3446;
    asf_info->lut1[5] =  1.3497;
    asf_info->lut1[6] =  1.3500;
    asf_info->lut1[7] =  1.3500;
    asf_info->lut1[8] =  1.3500;
    asf_info->lut1[9] =  1.3500;
    asf_info->lut1[10] = 1.3500;
    asf_info->lut1[11] = 1.3500;
    asf_info->lut1[12] = 1.3500;
    asf_info->lut1[13] = 1.3500;
    asf_info->lut1[14] = 1.3500;
    asf_info->lut1[15] = 1.3500;
    asf_info->lut1[16] = 1.3500;
    asf_info->lut1[17] = 1.3500;
    asf_info->lut1[18] = 1.3500;
    asf_info->lut1[19] = 1.3500;
    asf_info->lut1[20] = 1.3500;
    asf_info->lut1[21] = 1.3500;
    asf_info->lut1[22] = 1.3500;
    asf_info->lut1[23] = 1.3500;

    asf_info->lut2[0] =  0.3502;
    asf_info->lut2[1] =  0.6286;
    asf_info->lut2[2] =  1.0791;
    asf_info->lut2[3] =  1.2971;
    asf_info->lut2[4] =  1.3446;
    asf_info->lut2[5] =  1.3497;
    asf_info->lut2[6] =  1.3500;
    asf_info->lut2[7] =  1.3500;
    asf_info->lut2[8] =  1.3500;
    asf_info->lut2[9] =  1.3500;
    asf_info->lut2[10] = 1.3500;
    asf_info->lut2[11] = 1.3500;
    asf_info->lut2[12] = 1.3500;
    asf_info->lut2[13] = 1.3500;
    asf_info->lut2[14] = 1.3500;
    asf_info->lut2[15] = 1.3500;
    asf_info->lut2[16] = 1.3500;
    asf_info->lut2[17] = 1.3500;
    asf_info->lut2[18] = 1.3500;
    asf_info->lut2[19] = 1.3500;
    asf_info->lut2[20] = 1.3500;
    asf_info->lut2[21] = 1.3500;
    asf_info->lut2[22] = 1.3500;
    asf_info->lut2[23] = 1.3500;

    for (i = 0; i < 24; i++) {
      asf_info->lut1[i] *= hw_params->sharpness_level;
      asf_info->lut2[i] *= hw_params->sharpness_level;
    }

    asf_info->lut3[0] = 1;
    asf_info->lut3[1] = 1;
    asf_info->lut3[2] = 1;
    asf_info->lut3[3] = 1;
    asf_info->lut3[4] = 1;
    asf_info->lut3[5] = 1;
    asf_info->lut3[6] = 1;
    asf_info->lut3[7] = 1;
    asf_info->lut3[8] = 1;
    asf_info->lut3[9] = 1;
    asf_info->lut3[10] = 1;
    asf_info->lut3[11] = 1;

    asf_info->sp = 0;
    asf_info->clamp_h_ul = 14;
    asf_info->clamp_h_ll = -14;
    asf_info->clamp_v_ul = 14;
    asf_info->clamp_v_ll = -14;
    asf_info->clamp_scale_max = 1;
    asf_info->clamp_scale_min = 1;
    asf_info->clamp_offset_max = 6;
    asf_info->clamp_offset_min = 6;
  } else if (hw_params->asf_mode == CPP_PARAM_ASF_EMBOSS) {
    cpp_hw_params_set_emboss_effect(hw_params);
  } else if (hw_params->asf_mode == CPP_PARAM_ASF_SKETCH) {
    cpp_hw_params_set_sketch_effect(hw_params);
  } else if (hw_params->asf_mode == CPP_PARAM_ASF_NEON) {
    cpp_hw_params_set_neon_effect(hw_params);
  }
}
#if defined (CHROMATIX_304) || defined (CHROMATIX_306) || \
  defined (CHROMATIX_307) || defined (CHROMATIX_308) || defined (CHROMATIX_308E) || \
  defined (CHROMATIX_309) || defined (CHROMATIX_310) || defined (CHROMATIX_310E)

/**
 * Function: cpp_hw_params_asf_find_region_idxs
 *
 * Description: This function finds the asf regions in chromatix
 * based on trigger input and calculates interpolation factor.
 *
 * Input parameters:
 *   @p_chromatix_asf: pointer to chromatix asf 9x9 type.
 *   @lds_enable: flag for lds enabled.
 *   @trigger_input: aec trigger input.
 *   @p_reg1_idx: pointer to region 1 index.
 *   @p_reg2_idx: pointer to region 2 index.
 *   @p_interpolate_factor: pointer to interpolation factor
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
static void cpp_hw_params_asf_find_region_idxs(
  chromatix_ASF_9x9_type *p_chromatix_asf, boolean lds_enable __unused,
  float trigger_input, uint32_t *p_reg1_idx, uint32_t *p_reg2_idx,
  float *p_interpolate_factor)
{
  uint32_t i = 0;
  uint32_t end_idx = ASF_9x9_NUM_GRAY_PATCHES - 1;
  float trigger_start, trigger_end;
  float numerator, denominator;

#ifdef LDS_ENABLE
  if (lds_enable) {
    i = ASF_9x9_NUM_GRAY_PATCHES;
    end_idx = ASF_9x9_NUM_GRAY_PATCHES + MAX_LDS_REGIONS - 1;
    CPP_ASF_DBG("[LDS] i %d, end_idx %d", i, end_idx);
  }
#endif

  *p_reg1_idx = end_idx;
  *p_reg2_idx = end_idx;
  *p_interpolate_factor = 0.0f;
  /* Find the range in the availble grey patches */
  CPP_ASF_DBG("aec trigger_input = %f", trigger_input);
  for (; i < end_idx; i++) {
    if (p_chromatix_asf->control_asf_9x9 == CPP_ASF_CONTROL_LUX_BASED) {
      trigger_start = GET_ASF_LUXIDX_TRIGER_START(p_chromatix_asf, i);
      trigger_end = GET_ASF_LUXIDX_TRIGER_END(p_chromatix_asf, i);
    } else {
      trigger_start = GET_ASF_GAIN_TRIGER_START(p_chromatix_asf, i);
      trigger_end = GET_ASF_GAIN_TRIGER_END(p_chromatix_asf, i);
    }

    CPP_ASF_DBG("Region_idx = %d, trigger start %f, trigger end %f",
      i , trigger_start, trigger_end);

    if (trigger_input <= trigger_start) {
      *p_reg1_idx = i;
      *p_reg2_idx = i;
      CPP_ASF_DBG("non interpolate asf_region1_idx=%d,"
        "asf_region2_idx=%d", *p_reg1_idx, *p_reg2_idx);
      break;
    }
    if (trigger_input < trigger_end) {
      /* Interpolate all the values */
      numerator = (trigger_input - trigger_start);
      denominator = (trigger_end - trigger_start);
      if (denominator == 0.0f) {
        *p_reg1_idx = i;
        *p_reg2_idx = i;
        break;
      }
      *p_interpolate_factor = numerator / denominator;
      *p_reg1_idx = i;
      *p_reg2_idx = i + 1;
      CPP_ASF_DBG("interpolate, factor=%f, asf_region1_idx=%d,"
        "asf_region2_idx=%d", *p_interpolate_factor, *p_reg1_idx,
        *p_reg2_idx);
      break;
    } /* else iterate */
  }
}

static cpp_params_asf_region_t cpp_hw_params_asf_find_region(
  cpp_params_aec_trigger_t *aec_trigger_params, boolean lds_enable __unused)
{
#ifdef LDS_ENABLE
  if (lds_enable) {
    int32_t start_idx = ASF_7x7_NUM_GRAY_PATCHES;
    if (aec_trigger_params->aec_trigger_input <=
      aec_trigger_params->trigger_start[start_idx]) {
      CPP_ASF_LOW("[LDS:ASF]: aec_trigger_input = %f, trigger_start[6] = %f\n",
        aec_trigger_params->aec_trigger_input,
        aec_trigger_params->trigger_start[start_idx]);
      return CPP_PARAM_ASF_REGION_7;
    } else if (aec_trigger_params->aec_trigger_input >
      aec_trigger_params->trigger_start[start_idx]  &&
      aec_trigger_params->aec_trigger_input <=
      aec_trigger_params->trigger_end[start_idx]) {
      CPP_ASF_LOW("[LDS:ASF]: aec_trigger_input = %f, trigger_start[6] = %f, "
        "trigger_end[6] = %f\n",
        aec_trigger_params->aec_trigger_input,
        aec_trigger_params->trigger_start[start_idx],
        aec_trigger_params->trigger_end[start_idx]);
      return CPP_PARAM_ASF_REGION_78_INTERPOLATE;
    } else if (aec_trigger_params->aec_trigger_input >
      aec_trigger_params->trigger_end[start_idx]) {
      CPP_ASF_LOW("[LDS:ASF]: aec_trigger_input = %f, trigger_start[7] = %f, "
        "trigger_end[6] = %f\n",
        aec_trigger_params->aec_trigger_input,
        aec_trigger_params->trigger_start[start_idx + 1],
        aec_trigger_params->trigger_end[start_idx]);
      return CPP_PARAM_ASF_REGION_8;
    } else {
      CPP_ASF_ERR("LDS:invalid input: should not reach here.\n");
    }
    return CPP_PARAM_ASF_REGION_MAX;
  }
#endif
  if (aec_trigger_params->aec_trigger_input <=
      aec_trigger_params->trigger_start[0]) {
    return CPP_PARAM_ASF_REGION_1;
  } else if (aec_trigger_params->aec_trigger_input >
             aec_trigger_params->trigger_start[0]  &&
             aec_trigger_params->aec_trigger_input <
             aec_trigger_params->trigger_end[0]) {
    return CPP_PARAM_ASF_REGION_12_INTERPOLATE;
  } else if (aec_trigger_params->aec_trigger_input >=
             aec_trigger_params->trigger_end[0] &&
             aec_trigger_params->aec_trigger_input <=
             aec_trigger_params->trigger_start[1]) {
    return CPP_PARAM_ASF_REGION_2;
  } else if (aec_trigger_params->aec_trigger_input >
             aec_trigger_params->trigger_start[1]  &&
             aec_trigger_params->aec_trigger_input <
             aec_trigger_params->trigger_end[1]) {
    return CPP_PARAM_ASF_REGION_23_INTERPOLATE;
  } else if (aec_trigger_params->aec_trigger_input >=
             aec_trigger_params->trigger_end[1] &&
             aec_trigger_params->aec_trigger_input <=
             aec_trigger_params->trigger_start[2]) {
    return CPP_PARAM_ASF_REGION_3;
  } else if (aec_trigger_params->aec_trigger_input >
             aec_trigger_params->trigger_start[2]  &&
             aec_trigger_params->aec_trigger_input <
             aec_trigger_params->trigger_end[2]) {
    return CPP_PARAM_ASF_REGION_34_INTERPOLATE;
  } else if (aec_trigger_params->aec_trigger_input >=
             aec_trigger_params->trigger_end[2] &&
             aec_trigger_params->aec_trigger_input <=
             aec_trigger_params->trigger_start[3]) {
    return CPP_PARAM_ASF_REGION_4;
  } else if (aec_trigger_params->aec_trigger_input >
             aec_trigger_params->trigger_start[3]  &&
             aec_trigger_params->aec_trigger_input <
             aec_trigger_params->trigger_end[3]) {
    return CPP_PARAM_ASF_REGION_45_INTERPOLATE;
  } else if (aec_trigger_params->aec_trigger_input >=
             aec_trigger_params->trigger_end[3] &&
             aec_trigger_params->aec_trigger_input <=
             aec_trigger_params->trigger_start[4]) {
    return CPP_PARAM_ASF_REGION_5;
  } else if (aec_trigger_params->aec_trigger_input >
             aec_trigger_params->trigger_start[4]  &&
             aec_trigger_params->aec_trigger_input <
             aec_trigger_params->trigger_end[4]) {
    return CPP_PARAM_ASF_REGION_56_INTERPOLATE;
  } else if (aec_trigger_params->aec_trigger_input >=
             aec_trigger_params->trigger_end[4]) {
    return CPP_PARAM_ASF_REGION_6;
  } else {
    CPP_ASF_ERR("invalid trigger input");
  }
  return CPP_PARAM_ASF_REGION_MAX;
}

static int32_t cpp_hw_params_update_asf_kernel_coeff(
  chromatix_asf_7_7_core_type *asf_7_7, cpp_params_asf_info_t *asf_info,
  int32_t region_trg_num)
{
  if (!asf_7_7 || !asf_info) {
    CPP_ASF_ERR("failed: asf_7_7 %p, asf_info %p", asf_7_7, asf_info);
    return -EINVAL;
  }
  if (region_trg_num < 1 || region_trg_num > CPP_PARAM_ASF_7x7_TRIGGER_MAX) {
    CPP_ASF_ERR("failed, region_trg_num=%d", region_trg_num);
    return -EINVAL;
  }
  uint32_t idx = region_trg_num - 1;
  uint32_t i;
  uint32_t chrmtx_idx,asf_idx;
  uint32_t asf_filter_k_entries = 0;
  /* Update en_dyna_clamp */
  asf_info->dyna_clamp_en = asf_7_7[idx].en_dyna_clamp;

  asf_filter_k_entries = cpp_hw_params_get_num_k_entries(asf_info->cpp_fw_version);
  for (i=0; i < MIN(asf_filter_k_entries,CPP_ASF_F_KERNEL_CHROMATIX_ENTRIES); i++) {

    /* HW ASF kernel is 4x4, so read 4x4 within chromatix 5x5 */
    if (asf_filter_k_entries  < CPP_ASF_F_KERNEL_CHROMATIX_ENTRIES) {
      asf_idx = i;
      chrmtx_idx = asf_4_4_Idx[i];
    }
    /* HW ASF kernel is 5x5, but chromatix has 4x4 filter, so read only 4x4 */
    else if (asf_filter_k_entries  > CPP_ASF_F_KERNEL_CHROMATIX_ENTRIES) {
      asf_idx = asf_4_4_Idx[i];
      chrmtx_idx = i;
    } else {
      asf_idx = chrmtx_idx = i;
    }
    /* Update F1 kernel */
    asf_info->sobel_h_coeff[asf_idx] = asf_7_7[idx].f1[chrmtx_idx];
    /* Update F2 kernel */
    asf_info->sobel_v_coeff[asf_idx] = asf_7_7[idx].f2[chrmtx_idx];
    /* Update F3 kernel */
    asf_info->hpf_h_coeff[asf_idx] = asf_7_7[idx].f3[chrmtx_idx];
    /* Update F4 kernel */
    asf_info->hpf_v_coeff[asf_idx] = asf_7_7[idx].f4[chrmtx_idx];
    /* Update F5 kernel */
    asf_info->lpf_coeff[asf_idx] = asf_7_7[idx].f5[chrmtx_idx];
  }
  asf_info->checksum_enable = asf_7_7[idx].checksum_flag;
  return 0;
}

static int32_t cpp_hw_params_fill_asf_kernel(
  chromatix_asf_7_7_core_type *asf_7_7, cpp_params_asf_info_t *asf_info,
  cpp_params_asf_region_t asf_region)
{
  int32_t rc = 0;
  uint32_t i = 0;
  switch (asf_region) {
  case CPP_PARAM_ASF_REGION_1:
  case CPP_PARAM_ASF_REGION_12_INTERPOLATE:
    rc = cpp_hw_params_update_asf_kernel_coeff(asf_7_7, asf_info, 1);
    break;
  case CPP_PARAM_ASF_REGION_2:
  case CPP_PARAM_ASF_REGION_23_INTERPOLATE:
    rc = cpp_hw_params_update_asf_kernel_coeff(asf_7_7, asf_info, 2);
    break;
  case CPP_PARAM_ASF_REGION_3:
  case CPP_PARAM_ASF_REGION_34_INTERPOLATE:
    rc = cpp_hw_params_update_asf_kernel_coeff(asf_7_7, asf_info, 3);
    break;
  case CPP_PARAM_ASF_REGION_4:
  case CPP_PARAM_ASF_REGION_45_INTERPOLATE:
    rc = cpp_hw_params_update_asf_kernel_coeff(asf_7_7, asf_info, 4);
    break;
  case CPP_PARAM_ASF_REGION_5:
  case CPP_PARAM_ASF_REGION_56_INTERPOLATE:
    rc = cpp_hw_params_update_asf_kernel_coeff(asf_7_7, asf_info, 5);
    break;
  case CPP_PARAM_ASF_REGION_6:
    rc = cpp_hw_params_update_asf_kernel_coeff(asf_7_7, asf_info, 6);
    break;
#ifdef LDS_ENABLE
  case CPP_PARAM_ASF_REGION_7:
  case CPP_PARAM_ASF_REGION_78_INTERPOLATE:
    rc = cpp_hw_params_update_asf_kernel_coeff(asf_7_7, asf_info, 7);
    break;
  case CPP_PARAM_ASF_REGION_8:
    rc = cpp_hw_params_update_asf_kernel_coeff(asf_7_7, asf_info, 8);
    break;
#endif
  default:
    CPP_ASF_ERR("invalid asf_region=%d", asf_region);
    rc = -EINVAL;
    break;
  }
  return rc;
}
#else
static cpp_params_asf_region_t cpp_hw_params_asf_find_region(
  cpp_params_aec_trigger_t *aec_trigger_params, boolean lds_enable __unused)
{
  if (aec_trigger_params->aec_trigger_input <=
      aec_trigger_params->brightlight_trigger_end) {
    return CPP_PARAM_ASF_BRIGHT_LIGHT;
  } else if ((aec_trigger_params->aec_trigger_input >
              aec_trigger_params->brightlight_trigger_end) &&
             (aec_trigger_params->aec_trigger_input <
              aec_trigger_params->brightlight_trigger_start)) {
    return CPP_PARAM_ASF_BRIGHT_LIGHT_INTERPOLATE;
  } else if ((aec_trigger_params->aec_trigger_input >=
              aec_trigger_params->brightlight_trigger_start) &&
             (aec_trigger_params->aec_trigger_input <=
              aec_trigger_params->lowlight_trigger_start)) {
    return CPP_PARAM_ASF_NORMAL_LIGHT;
  } else if ((aec_trigger_params->aec_trigger_input >
              aec_trigger_params->lowlight_trigger_start) &&
             (aec_trigger_params->aec_trigger_input <
              aec_trigger_params->lowlight_trigger_end)) {
    return CPP_PARAM_ASF_LOW_LIGHT_INTERPOLATE;
  } else if (aec_trigger_params->aec_trigger_input >=
             aec_trigger_params->lowlight_trigger_end) {
    return CPP_PARAM_ASF_LOW_LIGHT;
  } else {
    CPP_ASF_ERR("invalid trigger input");
  }
  return CPP_PARAM_ASF_MAX_LIGHT;
}

static void cpp_hw_params_update_asf_kernel_coeff(
  chromatix_asf_7_7_type *asf_7_7, cpp_params_asf_info_t *asf_info,
  ASF_7x7_light_type type)
{
  uint32_t i = 0;
  /* Update en_dyna_clamp */
  asf_info->dyna_clamp_en = asf_7_7->en_dyna_clamp[type];

  for (i = 0; i < CPP_ASF_F_KERNEL_ENTRIES_1_2_x; i++) {
    /* Update F1 kernel */
    asf_info->sobel_h_coeff[i] = asf_7_7->f1[type][i];
    /* Update F2 kernel */
    asf_info->sobel_v_coeff[i] = asf_7_7->f2[type][i];
    /* Update F3 kernel */
    asf_info->hpf_h_coeff[i] = asf_7_7->f3[type][i];
    /* Update F4 kernel */
    asf_info->hpf_v_coeff[i] = asf_7_7->f4[type][i];
    /* Update F5 kernel */
    asf_info->lpf_coeff[i] = asf_7_7->f5[i];
  }
  //asf_info->checksum_enable = asf_7_7->checksum_flag;
  asf_info->checksum_enable = 255;
  return;
}

static int32_t cpp_hw_params_fill_asf_kernel(
  chromatix_asf_7_7_type *asf_7_7, cpp_params_asf_info_t *asf_info,
  cpp_params_asf_region_t asf_region)
{
  int32_t ret = 0;
  uint32_t i = 0;
  switch (asf_region) {
  case CPP_PARAM_ASF_LOW_LIGHT:
  case CPP_PARAM_ASF_LOW_LIGHT_INTERPOLATE:
    cpp_hw_params_update_asf_kernel_coeff(asf_7_7, asf_info,
      ASF_7x7_LOW_LIGHT);
    break;
  case CPP_PARAM_ASF_NORMAL_LIGHT:
    cpp_hw_params_update_asf_kernel_coeff(asf_7_7, asf_info,
      ASF_7x7_NORMAL_LIGHT);
    break;
  case CPP_PARAM_ASF_BRIGHT_LIGHT:
  case CPP_PARAM_ASF_BRIGHT_LIGHT_INTERPOLATE:
    cpp_hw_params_update_asf_kernel_coeff(asf_7_7, asf_info,
      ASF_7x7_BRIGHT_LIGHT);
    break;
  default:
    CPP_ASF_ERR("invalid asf_region=%d", asf_region);
    ret = -EINVAL;
    break;
  }
  return ret;
}

#endif /* #if defined (CHROMATIX_304) || defined (CHROMATIX_306) || defined (CHROMATIX_307) */

static float cpp_hw_params_calculate_interpolate_factor(
  float trigger_start, float trigger_end, float trigger_input)
{
  float factor = 0.0f;
  if (trigger_end - trigger_start) {
    factor = (trigger_input - trigger_start) / (trigger_end - trigger_start);
    CPP_LOW("factor %f = (trigger_input %f - trigger_start %f) /"
      "(trigger_end %f - trigger_start %f)",
      factor, trigger_input, trigger_start, trigger_end,
      trigger_start);
  } else {
    CPP_LOW("trigger start and end has same values %f %f",
  trigger_start, trigger_end);
    factor = (trigger_input - trigger_start);
    CPP_LOW("factor %f = (trigger_input %f - trigger_start %f)",
      factor, trigger_input, trigger_start);
  }
  return factor;
}

#if defined (CHROMATIX_304) || defined (CHROMATIX_306) || \
  defined (CHROMATIX_307) || defined (CHROMATIX_308) || defined (CHROMATIX_308E) || \
  defined (CHROMATIX_309) || defined (CHROMATIX_310) || defined (CHROMATIX_310E)

void cpp_hw_params_asf_upscalor_adj(void *chromatix_asf_upscale,
  float scalor_ratio, float *asf_scalor_adj)
{
  float numerator, denominator, interpolation_factor = 1.0f;
  int i = 0;

  if (!asf_scalor_adj || !chromatix_asf_upscale)
    return;

  for (i = 0; i < UP_SCALING_LEVELS - 1; i++) {
    if (scalor_ratio >= GET_SCALORUP(chromatix_asf_upscale, i)) {
      (*asf_scalor_adj) *= LINEAR_INTERPOLATE(interpolation_factor,
        GET_UP_FACTOR(chromatix_asf_upscale, i),
        GET_UP_FACTOR(chromatix_asf_upscale, i));
      return;
    }

    if (scalor_ratio > GET_SCALORUP(chromatix_asf_upscale, i + 1)) {
      /* Interpolate all the values */
      numerator = (GET_SCALORUP(chromatix_asf_upscale, i) -
        scalor_ratio);
      denominator = (GET_SCALORUP(chromatix_asf_upscale, i) -
        GET_SCALORUP(chromatix_asf_upscale, i + 1));
      if (denominator == 0.0f) {
        return;
      }
      interpolation_factor = numerator / denominator;

      (*asf_scalor_adj) *= LINEAR_INTERPOLATE(interpolation_factor,
        GET_UP_FACTOR(chromatix_asf_upscale, i),
        GET_UP_FACTOR(chromatix_asf_upscale, i + 1));
      return;
    } /* else iterate */
  }

  (*asf_scalor_adj) *= LINEAR_INTERPOLATE(interpolation_factor,
    GET_UP_FACTOR(chromatix_asf_upscale, i),
    GET_UP_FACTOR(chromatix_asf_upscale, i));
  return;
}

void cpp_hw_params_asf_downscalor_adj(void *chromatix_asf_downscale,
  float scalor_ratio, float *asf_scalor_adj)
{
  float numerator, denominator, interpolation_factor = 1.0f;
  int i = 0;

  if (!asf_scalor_adj || !chromatix_asf_downscale)
    return;

  for (i = 0; i < DOWN_SCALING_LEVELS - 1; i++) {
    if (scalor_ratio <= GET_SCALORDOWN(chromatix_asf_downscale, i)) {
      (*asf_scalor_adj) *= LINEAR_INTERPOLATE(interpolation_factor,
        GET_DOWN_FACTOR(chromatix_asf_downscale, i),
        GET_DOWN_FACTOR(chromatix_asf_downscale, i));
      return;
    }

    if (scalor_ratio < GET_SCALORDOWN(chromatix_asf_downscale, i+1)) {
      /* Interpolate all the values */
      numerator = (scalor_ratio - GET_SCALORDOWN(chromatix_asf_downscale, i));
      denominator = (GET_SCALORDOWN(chromatix_asf_downscale, i+1) -
        GET_SCALORDOWN(chromatix_asf_downscale, i));
      if (denominator == 0.0f) {
        return;
      }
      interpolation_factor = numerator / denominator;

      (*asf_scalor_adj) *= LINEAR_INTERPOLATE(interpolation_factor,
        GET_DOWN_FACTOR(chromatix_asf_downscale, i),
        GET_DOWN_FACTOR(chromatix_asf_downscale, i+1));
      return;
    } /* else iterate */
  }

  (*asf_scalor_adj) *= LINEAR_INTERPOLATE(interpolation_factor,
    GET_DOWN_FACTOR(chromatix_asf_downscale, i),
    GET_DOWN_FACTOR(chromatix_asf_downscale, i));
  return;
 }

int cpp_hw_params_asf_scalor_interpolate(cpp_hardware_params_t *hw_params __unused,
  chromatix_cpp_type *chromatix_cpp __unused, float scalor_ratio __unused)
{
#if defined (CHROMATIX_310E) || defined (CHROMATIX_308E)
  if (F_EQUAL(scalor_ratio, 1.0f)) {
    return 0;
  } else if (scalor_ratio < 1.0) {
    void *scale_data = NULL;
    GET_ASF_UPSCALE_DATA(chromatix_cpp, hw_params, scale_data);
    if (scale_data == NULL) {
       return -1;
    } else {
      cpp_hw_params_asf_upscalor_adj(scale_data,
        scalor_ratio, &hw_params->asf_scalor_adj);
    }
  } else if (scalor_ratio > 1.0) {
    void *scale_data = NULL;
    GET_ASF_DOWNSCALE_DATA(chromatix_cpp, hw_params, scale_data);
    if (scale_data == NULL) {
       return -1;
    } else {
      cpp_hw_params_asf_downscalor_adj(scale_data,
        scalor_ratio, &hw_params->asf_scalor_adj);
    }
  }
  return 0;
#else
  return -1;
#endif
}

static int32_t cpp_hw_params_asf_adj_gain_lut_1_2x(
  cpp_params_asf_info_t *asf_info, float *gain_lut64, float effects_factor)
{
  float x0, y0, x1, y1, x2, y2;
  float x, y;
  float scale_adj_factor, sharpness_ctrl_factor;
  uint32_t i;
  x = asf_info->downscale_ratio;
  x0 = asf_info->sharp_min_ds_factor;
  y0 = asf_info->sharp_max_factor;
  x1 = asf_info->sharp_max_ds_factor;
  y1 = 0.0f;
  if (x <= 1.0f) {
    if (x >= x0)
      y = 1 + ((1-y0)/(1-x0))*(x-1);
    else
      y = y0;
  } else {
    if (x < x1)
      y = 1 + ((1-y1)/(1-x1))*(x-1);
    else
      y = y1;
  }
  scale_adj_factor = y;
  sharpness_ctrl_factor = asf_info->sharpness_level;
  for (i = 0; i < CPP_ASF_LUT12_ENTRIES_1_2_x; i++) {
    gain_lut64[i] = gain_lut64[i] * scale_adj_factor *
      sharpness_ctrl_factor * effects_factor;
  }
  return 0;
}


static void cpp_hw_params_interpolate_lut_params(
  cpp_hardware_params_t *hw_params __unused,
  chromatix_asf_7_7_core_type *asf_7_7, cpp_params_asf_info_t *asf_info,
  int32_t region_trg_num1, int32_t region_trg_num2, float interpolate_factor,
  float scale_adj, float effect_factor)
{
  uint32_t i = 0;
  uint32_t idx1 = region_trg_num1 - 1;
  uint32_t idx2 = region_trg_num2 - 1;

  for (i = 0; i < CPP_ASF_LUT12_ENTRIES_1_2_x; i++) {
    /* Interpolate LUT1 */
    asf_info->lut1[i] = LINEAR_INTERPOLATE(interpolate_factor,
      asf_7_7[idx1].lut1[i], asf_7_7[idx2].lut1[i]);
    CPP_ASF_LOW("lut1[%d] %f = ((factor %f * reference_iplus1 %f) +"
      "((1 - factor) %f * reference_i %f))\n",
      i, asf_info->lut1[i], interpolate_factor,
      asf_7_7[idx2].lut1[i], 1 - interpolate_factor, asf_7_7[idx1].lut1[i]);
    /* Interpolate LUT2 */
    asf_info->lut2[i] = LINEAR_INTERPOLATE(interpolate_factor,
      asf_7_7[idx1].lut2[i], asf_7_7[idx2].lut2[i]);
    CPP_ASF_LOW("lut2[%d] %f = ((factor %f * reference_iplus1 %f) +"
      "((1 - factor) %f * reference_i %f))\n",
      i, asf_info->lut2[i], interpolate_factor,
      asf_7_7[idx2].lut2[i], 1 - interpolate_factor, asf_7_7[idx1].lut2[i]);
  }
  for (i = 0; i < CPP_ASF_LUT3_ENTRIES_1_2_x; i++) {
    /* Interpolate LUT3 */
    asf_info->lut3[i] = LINEAR_INTERPOLATE(interpolate_factor,
      asf_7_7[idx1].lut3[i], asf_7_7[idx2].lut3[i]);
    CPP_ASF_LOW("lut3[%d] %f = ((factor %f * reference_iplus1 %f) +"
      "((1 - factor) %f * reference_i %f))\n",
      i, asf_info->lut3[i], interpolate_factor,
      asf_7_7[idx2].lut3[i], 1 - interpolate_factor, asf_7_7[idx1].lut3[i]);
  }

  if (scale_adj) {
    for (i = 0; i < CPP_ASF_LUT12_ENTRIES_1_2_x; i++) {
      asf_info->lut1[i] = asf_info->lut1[i] * scale_adj * effect_factor *
        asf_info->sharpness_level;
      asf_info->lut2[i] = asf_info->lut2[i] * scale_adj * effect_factor *
        asf_info->sharpness_level;
    }
  } else {
    cpp_hw_params_asf_adj_gain_lut_1_2x(asf_info, asf_info->lut1, effect_factor);
    cpp_hw_params_asf_adj_gain_lut_1_2x(asf_info, asf_info->lut2, effect_factor);
  }

  /* Interpolate sp */
  asf_info->sp = LINEAR_INTERPOLATE(interpolate_factor, asf_7_7[idx1].sp,
    asf_7_7[idx2].sp);
  /* convert the scale of [0-100] to [0.0-1.0]*/
  asf_info->sp /= 100.0f;
  /* Interpolate clamp */
  asf_info->clamp_h_ul = Round(LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7[idx1].reg_hh, asf_7_7[idx2].reg_hh));
  asf_info->clamp_h_ll = Round(LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7[idx1].reg_hl, asf_7_7[idx2].reg_hl));
  asf_info->clamp_v_ul = Round(LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7[idx1].reg_vh, asf_7_7[idx2].reg_vh));
  asf_info->clamp_v_ll = Round(LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7[idx1].reg_vl, asf_7_7[idx2].reg_vl));
  asf_info->clamp_scale_max = LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7[idx1].smax, asf_7_7[idx2].smax);
  asf_info->clamp_scale_min = LINEAR_INTERPOLATE(interpolate_factor,
   asf_7_7[idx1].smin, asf_7_7[idx2].smin);
  asf_info->clamp_offset_max = Round(LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7[idx1].omax, asf_7_7[idx2].omax));
  asf_info->clamp_offset_min = Round(LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7[idx1].omin, asf_7_7[idx2].omin));
  asf_info->clamp_tl_ul = Round(LINEAR_INTERPOLATE(interpolate_factor,
    GET_ASF_CLAMP_TL_UL(asf_7_7,idx1), GET_ASF_CLAMP_TL_UL(asf_7_7,idx2)));
  asf_info->clamp_tl_ll = Round(LINEAR_INTERPOLATE(interpolate_factor,
    GET_ASF_CLAMP_TL_LL(asf_7_7,idx1), GET_ASF_CLAMP_TL_LL(asf_7_7,idx2)));

  return;
}

static void cpp_hw_params_fill_noninterpolate_params(
  cpp_hardware_params_t *hw_params __unused,
  chromatix_asf_7_7_core_type *asf_7_7,
  cpp_params_asf_info_t *asf_info, int32_t region_trg_num,
  float scale_adj, float effect_factor)
{
  uint32_t i = 0;
  uint32_t idx = region_trg_num - 1;

  for (i = 0; i < CPP_ASF_LUT12_ENTRIES_1_2_x; i++) {
    asf_info->lut1[i] = asf_7_7[idx].lut1[i];
    asf_info->lut2[i] = asf_7_7[idx].lut2[i];
  }
  for (i = 0; i < CPP_ASF_LUT3_ENTRIES_1_2_x; i++) {
    asf_info->lut3[i] = asf_7_7[idx].lut3[i];
  }
  if (scale_adj) {
    for (i = 0; i < CPP_ASF_LUT12_ENTRIES_1_2_x; i++) {
      asf_info->lut1[i] = asf_info->lut1[i] * scale_adj * effect_factor *
        asf_info->sharpness_level;
      asf_info->lut2[i] = asf_info->lut2[i] * scale_adj * effect_factor *
        asf_info->sharpness_level;
    }
  } else {
    cpp_hw_params_asf_adj_gain_lut_1_2x(asf_info, asf_info->lut1, effect_factor);
    cpp_hw_params_asf_adj_gain_lut_1_2x(asf_info, asf_info->lut2, effect_factor);
  }

  asf_info->sp = asf_7_7[idx].sp / 100.0f;
  asf_info->clamp_h_ul = asf_7_7[idx].reg_hh;
  asf_info->clamp_h_ll = asf_7_7[idx].reg_hl;
  asf_info->clamp_v_ul = asf_7_7[idx].reg_vh;
  asf_info->clamp_v_ll = asf_7_7[idx].reg_vl;
  asf_info->clamp_scale_max = asf_7_7[idx].smax;
  asf_info->clamp_scale_min = asf_7_7[idx].smin;
  asf_info->clamp_offset_max = asf_7_7[idx].omax;
  asf_info->clamp_offset_min = asf_7_7[idx].omin;
  asf_info->clamp_tl_ul = GET_ASF_CLAMP_TL_UL(asf_7_7,idx);
  asf_info->clamp_tl_ll = GET_ASF_CLAMP_TL_LL(asf_7_7,idx);
}

static int32_t cpp_hw_params_fill_lut_params(
  cpp_hardware_params_t *hw_params,
  chromatix_asf_7_7_core_type *asf_7_7, cpp_params_asf_info_t *asf_info,
  cpp_params_aec_trigger_t *aec_trigger_params,
  cpp_params_asf_region_t asf_region, float scale_adj,
  float effect_factor)
{
  int32_t ret = 0;
  uint32_t i = 0;
  float interpolate_factor = 0.0f;
  boolean interpolate=FALSE;
  int32_t reg1, reg2;
  reg1 = reg2 = CPP_PARAM_ASF_REGION_MAX;

  switch (asf_region) {
  case CPP_PARAM_ASF_REGION_1:
    reg1 = 1;
    interpolate = FALSE;
    break;
  case CPP_PARAM_ASF_REGION_12_INTERPOLATE:
    reg1 = 1; reg2 = 2;
    interpolate = TRUE;
    interpolate_factor = cpp_hw_params_calculate_interpolate_factor(
      aec_trigger_params->trigger_start[0],
      aec_trigger_params->trigger_end[0],
      aec_trigger_params->aec_trigger_input);
    break;
  case CPP_PARAM_ASF_REGION_2:
    reg1 = 2;
    interpolate = FALSE;
    break;
  case CPP_PARAM_ASF_REGION_23_INTERPOLATE:
    reg1 = 2; reg2 = 3;
    interpolate = TRUE;
    interpolate_factor = cpp_hw_params_calculate_interpolate_factor(
      aec_trigger_params->trigger_start[1],
      aec_trigger_params->trigger_end[1],
      aec_trigger_params->aec_trigger_input);
    break;
  case CPP_PARAM_ASF_REGION_3:
    reg1 = 3;
    interpolate = FALSE;
    break;
  case CPP_PARAM_ASF_REGION_34_INTERPOLATE:
    reg1 = 3; reg2 = 4;
    interpolate = TRUE;
    interpolate_factor = cpp_hw_params_calculate_interpolate_factor(
      aec_trigger_params->trigger_start[2],
      aec_trigger_params->trigger_end[2],
      aec_trigger_params->aec_trigger_input);

    break;
  case CPP_PARAM_ASF_REGION_4:
    reg1 = 4;
    interpolate = FALSE;
    break;
  case CPP_PARAM_ASF_REGION_45_INTERPOLATE:
    reg1 = 4; reg2 = 5;
    interpolate = TRUE;
    interpolate_factor = cpp_hw_params_calculate_interpolate_factor(
      aec_trigger_params->trigger_start[3],
      aec_trigger_params->trigger_end[3],
      aec_trigger_params->aec_trigger_input);
    break;
  case CPP_PARAM_ASF_REGION_5:
    reg1 = 5;
    interpolate = FALSE;
    break;
  case CPP_PARAM_ASF_REGION_56_INTERPOLATE:
    reg1 = 5; reg2 = 6;
    interpolate = TRUE;
    interpolate_factor = cpp_hw_params_calculate_interpolate_factor(
      aec_trigger_params->trigger_start[4],
      aec_trigger_params->trigger_end[4],
      aec_trigger_params->aec_trigger_input);
    break;
  case CPP_PARAM_ASF_REGION_6:
    reg1 = 6;
    interpolate = FALSE;
    break;
#ifdef LDS_ENABLE
  case CPP_PARAM_ASF_REGION_7:
    reg1 = 7;
    interpolate = FALSE;
    break;
  case CPP_PARAM_ASF_REGION_78_INTERPOLATE:
    reg1 = 7; reg2 = 8;
    interpolate = TRUE;
    interpolate_factor = cpp_hw_params_calculate_interpolate_factor(
      aec_trigger_params->trigger_start[6],
      aec_trigger_params->trigger_end[6],
      aec_trigger_params->aec_trigger_input);
    break;
  case CPP_PARAM_ASF_REGION_8:
    reg1 = 8;
    interpolate = FALSE;
    break;
#endif
  default:
    CPP_ASF_ERR("invalid asf_region %d", asf_region);
    ret = -EINVAL;
    break;
  }
  if (interpolate) {
    if (interpolate_factor == 0.0f) {
      CPP_ASF_ERR("interpolate_factor=0.0f");
      return -EINVAL;
    }
    CPP_ASF_LOW("asf interpolation=yes factor=%f, region=(%d, %d)",
       interpolate_factor, reg1, reg2);
    cpp_hw_params_interpolate_lut_params(hw_params, asf_7_7,
      asf_info, reg1, reg2, interpolate_factor, scale_adj, effect_factor);
  } else {
    CPP_ASF_LOW("asf interpolation=no region=%d", reg1);
    cpp_hw_params_fill_noninterpolate_params(hw_params, asf_7_7, asf_info,
      reg1, scale_adj, effect_factor);
  }


  return ret;
}

/**
 * Function: cpp_hw_param_convert_region_to_idx
 *
 * Description: This function converts region numbers to
 * chromatix asf indices.
 *
 * Input parameters:
 *   @asf_region: asf region number.
 *   @p_asf_info: pointer to asf info.
 *
 * Return values:
 *   0 - success, -EINVAL - failure.
 *
 * Notes: none
 **/
static int32_t cpp_hw_param_convert_asf_region_to_idx(
  cpp_params_asf_region_t asf_region,
  cpp_params_asf_info_t  *p_asf_info)
{
    if (p_asf_info == NULL) {
      CPP_ASF_ERR("failed, asf info NULL");
      return -EINVAL;
    }

    switch (asf_region) {
    case CPP_PARAM_ASF_REGION_1:
      p_asf_info->asf_reg1_idx = 0;
      p_asf_info->asf_reg2_idx = 0;
      break;
    case CPP_PARAM_ASF_REGION_12_INTERPOLATE:
      p_asf_info->asf_reg1_idx = 0;
      p_asf_info->asf_reg2_idx = 1;
      break;
    case CPP_PARAM_ASF_REGION_2:
      p_asf_info->asf_reg1_idx = 1;
      p_asf_info->asf_reg2_idx = 1;
      break;
    case CPP_PARAM_ASF_REGION_23_INTERPOLATE:
      p_asf_info->asf_reg1_idx = 1;
      p_asf_info->asf_reg2_idx = 2;
      break;
    case CPP_PARAM_ASF_REGION_3:
      p_asf_info->asf_reg1_idx = 2;
      p_asf_info->asf_reg2_idx = 2;
      break;
    case CPP_PARAM_ASF_REGION_34_INTERPOLATE:
      p_asf_info->asf_reg1_idx = 2;
      p_asf_info->asf_reg2_idx = 3;
      break;
    case CPP_PARAM_ASF_REGION_4:
      p_asf_info->asf_reg1_idx = 3;
      p_asf_info->asf_reg2_idx = 3;
      break;
    case CPP_PARAM_ASF_REGION_45_INTERPOLATE:
      p_asf_info->asf_reg1_idx = 3;
      p_asf_info->asf_reg2_idx = 4;
      break;
    case CPP_PARAM_ASF_REGION_5:
      p_asf_info->asf_reg1_idx = 4;
      p_asf_info->asf_reg2_idx = 4;
      break;
    case CPP_PARAM_ASF_REGION_56_INTERPOLATE:
      p_asf_info->asf_reg1_idx = 4;
      p_asf_info->asf_reg2_idx = 5;
      break;
    case CPP_PARAM_ASF_REGION_6:
      p_asf_info->asf_reg1_idx = 5;
      p_asf_info->asf_reg2_idx = 5;
      break;
#ifdef LDS_ENABLE
    case CPP_PARAM_ASF_REGION_7:
      p_asf_info->asf_reg1_idx = 6;
      p_asf_info->asf_reg2_idx = 6;
      break;
    case CPP_PARAM_ASF_REGION_78_INTERPOLATE:
      p_asf_info->asf_reg1_idx = 6;
      p_asf_info->asf_reg2_idx = 7;
      break;
    case CPP_PARAM_ASF_REGION_8:
      p_asf_info->asf_reg1_idx = 7;
      p_asf_info->asf_reg2_idx = 7;
      break;
#endif
    default:
      CPP_ASF_ERR("failed, invalied region %d", asf_region);
      return -EINVAL;
    }

  return 0;
}

static int32_t cpp_hw_params_update_asf_params(cpp_hardware_params_t *hw_params,
  modulesChromatix_t *module_chromatix,
  cpp_params_aec_trigger_t *aec_trigger_params)
{
  int32_t                 ret = 0;
  uint32_t                i = 0;
  chromatix_ASF_7x7_type *chromatix_ASF_7x7 = NULL;
  chromatix_asf_7_7_core_type *asf_7_7 = NULL;
  cpp_params_asf_info_t  *asf_info = NULL;
  cpp_params_asf_region_t asf_region = CPP_PARAM_ASF_REGION_MAX;
  chromatix_cpp_type *chromatix_cpp = NULL;
  float isp_scale_ratio, cpp_scale_ratio;
  float effect_factor = 1.0f;

  if (!module_chromatix || !module_chromatix->chromatixCppPtr) {
    CPP_ASF_ERR("failed, chromatix NULL, using default params");
    return -EINVAL;
  }

  chromatix_cpp = module_chromatix->chromatixCppPtr;
  GET_ASF_7x7_POINTER(chromatix_cpp, hw_params, chromatix_ASF_7x7);
  asf_7_7 = chromatix_ASF_7x7->asf_7_7;
  asf_info = &hw_params->asf_info;

  /* Fill downscale factor in asf_info which will be used later to adjust
     LUT params */
  asf_info->sharp_min_ds_factor =
    chromatix_ASF_7x7->asf_7_7_sharp_min_ds_factor;
  asf_info->sharp_max_ds_factor =
    chromatix_ASF_7x7->asf_7_7_sharp_max_ds_factor;
  asf_info->sharp_max_factor =
    chromatix_ASF_7x7->asf_7_7_sharp_max_factor;

  CPP_ASF_LOW("aec_trigger_input = %f",
            aec_trigger_params->aec_trigger_input);

  /* Fill ASF parameters in frame_params */
  if (!chromatix_ASF_7x7->asf_7x7_en ||
    aec_trigger_params->aec_trigger_input <= 0.0f) {
    /* Use default */
    CPP_ASF_LOW("using default asf params, asf_en=%d, trig_in=%f",
      chromatix_ASF_7x7->asf_7x7_en,
      aec_trigger_params->aec_trigger_input);
    cpp_hw_params_update_default_asf_params(hw_params);
    return ret;
  }
  switch (hw_params->asf_mode) {
  case CPP_PARAM_ASF_OFF:
      break;
  case CPP_PARAM_ASF_BEAUTYSHOT:
      effect_factor *= CPP_PARAM_BEAUTY_EFFECT_ASF_FACTOR;
  case CPP_PARAM_ASF_DUAL_FILTER:
      /* non-region-specific params */
      asf_info->sp_eff_en = 0;
      asf_info->neg_abs_y1 = 0;
    asf_region = cpp_hw_params_asf_find_region(aec_trigger_params,
      hw_params->lds_enable);
    if (asf_region >= CPP_PARAM_ASF_REGION_MAX) {
      CPP_ASF_ERR("failed asf_region %d > max %d\n",
        asf_region, CPP_PARAM_ASF_REGION_MAX);
      return -EINVAL;
    }
    CPP_ASF_DBG("asf region %d\n", asf_region);
    asf_info->region = asf_region;
    ret = cpp_hw_param_convert_asf_region_to_idx(asf_region, asf_info);
    if (ret < 0) {
      CPP_ASF_ERR("failed %d", ret);
      return ret;
    }

    /* Update F kernel and sp */
    CPP_ASF_LOW("updating kernel");
    ret = cpp_hw_params_fill_asf_kernel(asf_7_7, asf_info, asf_region);
    if (ret < 0) {
      CPP_ASF_ERR("failed %d", ret);
      return ret;
    }
    hw_params->asf_scalor_adj = 1.0f;
    ret = cpp_hw_params_asf_scalor_interpolate(hw_params, chromatix_cpp,
      hw_params->isp_scale_ratio);
    ret = cpp_hw_params_asf_scalor_interpolate(hw_params, chromatix_cpp,
      hw_params->cpp_scale_ratio);

    if (ret) {
       asf_info->downscale_ratio = hw_params->isp_scale_ratio *
         hw_params->cpp_scale_ratio;
       hw_params->asf_scalor_adj = 0.0f;
    }

    asf_info->sharpness_level = hw_params->sharpness_level;

    /* Interpolate LUT params */
    CPP_ASF_LOW("updating LUT");
    ret = cpp_hw_params_fill_lut_params(hw_params, asf_7_7,
      asf_info, aec_trigger_params, asf_region, hw_params->asf_scalor_adj,
      effect_factor);

    if (ret < 0) {
      CPP_ASF_ERR("failed %d", ret);
      return ret;
    }
    asf_info->nz_flag = asf_info->nz_flag_f2 = asf_info->nz_flag_f3_f5 =
      asf_7_7[0].nz[0];
    CPP_ASF_LOW("min_ds %f max_ds %f max_factor %f",
      asf_info->sharp_min_ds_factor, asf_info->sharp_max_ds_factor,
      asf_info->sharp_max_factor);
    break;
  case CPP_PARAM_ASF_EMBOSS:
    CPP_ASF_HIGH("EMBOSS");
    cpp_hw_params_set_emboss_effect(hw_params);
    break;
  case CPP_PARAM_ASF_SKETCH:
    CPP_ASF_HIGH("SKETCH");
    cpp_hw_params_set_sketch_effect(hw_params);
    break;
  case CPP_PARAM_ASF_NEON:
    CPP_ASF_HIGH("NEON");
    cpp_hw_params_set_neon_effect(hw_params);
    break;
   default:
    CPP_ASF_HIGH("asf mode %d", hw_params->asf_mode);
    return -EINVAL;
  }
  return 0;
}

static int32_t cpp_hw_params_asf_interpolate_1_2_x(
  cpp_hardware_params_t *hw_params,
  modulesChromatix_t *module_chromatix,
  cpp_params_aec_trigger_info_t *trigger_input)
{
  chromatix_ASF_7x7_type *chromatix_ASF_7x7 = NULL;
  cpp_params_aec_trigger_t aec_trigger_params;
  chromatix_cpp_type *chromatix_cpp = NULL;

  if (hw_params->asf_lock) {
    CPP_ASF_ERR("ASF is locked by Chromatix");
    return 0;
  }

  if (!module_chromatix || !module_chromatix->chromatixCppPtr) {
    CPP_ASF_ERR("failed, chromatix NULL, using default params");
    /* Use default */
    cpp_hw_params_update_default_asf_params(hw_params);
    return 0;
  }

  chromatix_cpp = (chromatix_cpp_type *)module_chromatix->chromatixCppPtr;
  chromatix_ASF_7x7 = &chromatix_cpp->chromatix_ASF_7x7;
  GET_ASF_7x7_POINTER (chromatix_cpp, hw_params, chromatix_ASF_7x7);
  uint32_t i;

  /* determine the control method */
  if (chromatix_ASF_7x7->control_asf_7x7 == 0) {
    /* Lux index based */
    CPP_ASF_DBG("lux index based trigger points");
    for (i=0; i<CPP_PARAM_ASF_7x7_TRIGGER_MAX; i++) {
      aec_trigger_params.trigger_start[i] =
        chromatix_ASF_7x7->asf_7_7[i].asf_7x7_trigger.lux_index_start;
      aec_trigger_params.trigger_end[i] =
        chromatix_ASF_7x7->asf_7_7[i].asf_7x7_trigger.lux_index_end;
    }
    aec_trigger_params.aec_trigger_input = trigger_input->lux_idx;
  } else if (chromatix_ASF_7x7->control_asf_7x7 == 1) {
    /* Gain based */
    CPP_ASF_DBG("gain based trigger points");
    for (i=0; i<CPP_PARAM_ASF_7x7_TRIGGER_MAX; i++) {
      aec_trigger_params.trigger_start[i] =
        chromatix_ASF_7x7->asf_7_7[i].asf_7x7_trigger.gain_start;
      aec_trigger_params.trigger_end[i] =
        chromatix_ASF_7x7->asf_7_7[i].asf_7x7_trigger.gain_end;
    }
    aec_trigger_params.aec_trigger_input = trigger_input->gain;
  } else {
    /* Error in chromatix */
    CPP_ASF_ERR("invalid chromatix control type");
    return -EINVAL;
  }
  for (i=0; i<CPP_PARAM_ASF_7x7_TRIGGER_MAX; i++) {
    CPP_ASF_DBG("start[%d]=%f, end[%d]=%f", i,
      aec_trigger_params.trigger_start[i], i,
      aec_trigger_params.trigger_end[i]);
  }
  return cpp_hw_params_update_asf_params(hw_params, module_chromatix,
    &aec_trigger_params);
}

#else
static void cpp_hw_params_interpolate_lut_params(
  cpp_hardware_params_t *hw_params,
  chromatix_asf_7_7_type *asf_7_7, cpp_params_asf_info_t *asf_info,
  ASF_7x7_light_type type1, ASF_7x7_light_type type2, float interpolate_factor)
{
  uint32_t i = 0;
  for (i = 0; i < CPP_ASF_LUT12_ENTRIES_1_2_x; i++) {
    /* Interpolate LUT1 */
    asf_info->lut1[i] = LINEAR_INTERPOLATE(interpolate_factor,
      asf_7_7->lut1[type1][i], asf_7_7->lut1[type2][i]);
    CPP_ASF_LOW("lut1[%d] %f = ((factor %f * reference_iplus1 %f) +"
      "((1 - factor) %f * reference_i %f))\n",
      i, asf_info->lut1[i], interpolate_factor,
      asf_7_7->lut1[type2][i], 1 - interpolate_factor, asf_7_7->lut1[type1][i]);
    /* Update sharpness ratio */
    asf_info->lut1[i] *= hw_params->sharpness_level;
    /* Interpolate LUT2 */
    asf_info->lut2[i] = LINEAR_INTERPOLATE(interpolate_factor,
      asf_7_7->lut2[type1][i], asf_7_7->lut2[type2][i]);
    CPP_ASF_LOW("lut2[%d] %f = ((factor %f * reference_iplus1 %f) +"
      "((1 - factor) %f * reference_i %f))\n",
      i, asf_info->lut2[i], interpolate_factor,
      asf_7_7->lut2[type2][i], 1 - interpolate_factor, asf_7_7->lut2[type1][i]);
    /* Update sharpness ratio */
    asf_info->lut2[i] *= hw_params->sharpness_level;
  }
  for (i = 0; i < CPP_ASF_LUT3_ENTRIES_1_2_x; i++) {
    /* Interpolate LUT3 */
    asf_info->lut3[i] = LINEAR_INTERPOLATE(interpolate_factor,
      asf_7_7->lut3[type1][i], asf_7_7->lut3[type2][i]);
    CPP_ASF_LOW("lut3[%d] %f = ((factor %f * reference_iplus1 %f) +"
      "((1 - factor) %f * reference_i %f))\n",
      i, asf_info->lut3[i], interpolate_factor,
      asf_7_7->lut3[type2][i], 1 - interpolate_factor, asf_7_7->lut3[type1][i]);
  }
  /* Interpolate sp */
  asf_info->sp = LINEAR_INTERPOLATE(interpolate_factor, asf_7_7->sp[type1],
    asf_7_7->sp[type2]);
  /* convert the scale of [0-100] to [0.0-1.0]*/
  asf_info->sp /= 100.0f;
  /* Interpolate clamp */
  asf_info->clamp_h_ul = Round(LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7->reg_hh[type1], asf_7_7->reg_hh[type2]));
  asf_info->clamp_h_ll = Round(LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7->reg_hl[type1], asf_7_7->reg_hl[type2]));
  asf_info->clamp_v_ul = Round(LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7->reg_vh[type1], asf_7_7->reg_vh[type2]));
  asf_info->clamp_v_ll = Round(LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7->reg_vl[type1], asf_7_7->reg_vl[type2]));
  asf_info->clamp_scale_max = LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7->smax[type1], asf_7_7->smax[type2]);
  asf_info->clamp_scale_min = LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7->smin[type1], asf_7_7->smin[type2]);
  asf_info->clamp_offset_max = Round(LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7->omax[type1], asf_7_7->omax[type2]));
  asf_info->clamp_offset_min = Round(LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7->omin[type1], asf_7_7->omin[type2]));

  return;
}

static void cpp_hw_params_fill_noninterpolate_params(
  cpp_hardware_params_t *hw_params, chromatix_asf_7_7_type *asf_7_7,
  cpp_params_asf_info_t *asf_info, ASF_7x7_light_type asf_region)
{
  uint32_t i = 0;
  for (i = 0; i < CPP_ASF_LUT12_ENTRIES_1_2_x; i++) {
    asf_info->lut1[i] = asf_7_7->lut1[asf_region][i] *
      hw_params->sharpness_level;
    asf_info->lut2[i] = asf_7_7->lut2[asf_region][i] *
      hw_params->sharpness_level;
  }
  for (i = 0; i < CPP_ASF_LUT3_ENTRIES_1_2_x; i++) {
    asf_info->lut3[i] = asf_7_7->lut3[asf_region][i];
  }
  asf_info->sp = asf_7_7->sp[asf_region] / 100.0f;
  asf_info->clamp_h_ul = asf_7_7->reg_hh[asf_region];
  asf_info->clamp_h_ll = asf_7_7->reg_hl[asf_region];
  asf_info->clamp_v_ul = asf_7_7->reg_vh[asf_region];
  asf_info->clamp_v_ll = asf_7_7->reg_vl[asf_region];
  asf_info->clamp_scale_max = asf_7_7->smax[asf_region];
  asf_info->clamp_scale_min = asf_7_7->smin[asf_region];
  asf_info->clamp_offset_max = asf_7_7->omax[asf_region];
  asf_info->clamp_offset_min = asf_7_7->omin[asf_region];
}

static int32_t cpp_hw_params_fill_lut_params(
  cpp_hardware_params_t *hw_params,
  chromatix_asf_7_7_type *asf_7_7, cpp_params_asf_info_t *asf_info,
  cpp_params_aec_trigger_t *aec_trigger_params,
  cpp_params_asf_region_t asf_region)
{
  int32_t ret = 0;
  uint32_t i = 0;
  float interpolate_factor = 0.0f;
  switch (asf_region) {
  case CPP_PARAM_ASF_LOW_LIGHT:
    cpp_hw_params_fill_noninterpolate_params(hw_params, asf_7_7, asf_info,
      ASF_7x7_LOW_LIGHT);
    break;
  case CPP_PARAM_ASF_LOW_LIGHT_INTERPOLATE:
    interpolate_factor = cpp_hw_params_calculate_interpolate_factor(
      aec_trigger_params->lowlight_trigger_start,
      aec_trigger_params->lowlight_trigger_end,
      aec_trigger_params->aec_trigger_input);
    if (interpolate_factor == 0.0f) {
      CPP_ASF_LOW("interpolate_factor zero");
      ret = -EINVAL;
      break;
    }
    cpp_hw_params_interpolate_lut_params(hw_params,
      asf_7_7, asf_info, ASF_7x7_NORMAL_LIGHT, ASF_7x7_LOW_LIGHT,
      interpolate_factor);
    break;
  case CPP_PARAM_ASF_NORMAL_LIGHT:
    cpp_hw_params_fill_noninterpolate_params(hw_params, asf_7_7, asf_info,
      ASF_7x7_NORMAL_LIGHT);
    break;
  case CPP_PARAM_ASF_BRIGHT_LIGHT:
    cpp_hw_params_fill_noninterpolate_params(hw_params, asf_7_7, asf_info,
      ASF_7x7_BRIGHT_LIGHT);
    break;
  case CPP_PARAM_ASF_BRIGHT_LIGHT_INTERPOLATE:
    interpolate_factor = cpp_hw_params_calculate_interpolate_factor(
      aec_trigger_params->brightlight_trigger_start,
      aec_trigger_params->brightlight_trigger_end,
      aec_trigger_params->aec_trigger_input);
    if (interpolate_factor == 0.0f) {
      CPP_ASF_LOW("interpolate_factor zero");
      ret = -EINVAL;
      break;
    }
    cpp_hw_params_interpolate_lut_params(hw_params,
      asf_7_7, asf_info, ASF_7x7_NORMAL_LIGHT, ASF_7x7_BRIGHT_LIGHT,
      interpolate_factor);
    break;
  default:
    CPP_ASF_ERR("invalid asf_region %d", asf_region);
    ret = -EINVAL;
    break;
  }
  return ret;
}

static int32_t cpp_hw_params_update_asf_params(cpp_hardware_params_t *hw_params,
  chromatix_parms_type  *chromatix_ptr,
  cpp_params_aec_trigger_t *aec_trigger_params)
{
  int32_t                 ret = 0;
  uint32_t                i = 0;
  chromatix_ASF_7x7_type *chromatix_ASF_7x7 = NULL;
  chromatix_asf_7_7_type *asf_7_7 = NULL;
  cpp_params_asf_info_t  *asf_info = NULL;
  cpp_params_asf_region_t asf_region = CPP_PARAM_ASF_MAX_LIGHT;

  chromatix_ASF_7x7 = &chromatix_ptr->chromatix_VFE.chromatix_ASF_7x7;
  asf_7_7 = &chromatix_ASF_7x7->asf_7_7;
  asf_info = &hw_params->asf_info;

  /* Fill downscale factor in asf_info which will be used later to adjust
     LUT params */
  asf_info->sharp_min_ds_factor =
    chromatix_ASF_7x7->asf_7_7_sharp_min_ds_factor;
  asf_info->sharp_max_ds_factor =
    chromatix_ASF_7x7->asf_7_7_sharp_max_ds_factor;
  asf_info->sharp_max_factor =
    chromatix_ASF_7x7->asf_7_7_sharp_max_factor;

  /* Fill ASF parameters in frame_params */
  if (!chromatix_ASF_7x7->asf_7_7.asf_en ||
    aec_trigger_params->aec_trigger_input <= 0.0f) {
    /* Use default */
    cpp_hw_params_update_default_asf_params(hw_params);
    return ret;
  }
  if (hw_params->asf_mode == CPP_PARAM_ASF_DUAL_FILTER || hw_params->scene_mode_on) {
    asf_region = cpp_hw_params_asf_find_region(aec_trigger_params, hw_params->lds_enable);
    if (asf_region >= CPP_PARAM_ASF_MAX_LIGHT) {
      CPP_ASF_ERR("failed asf region %d > max asf region %d\n", asf_region, CPP_PARAM_ASF_MAX_LIGHT);
      return -EINVAL;
    }
    CPP_ASF_HIGH("asf region %d\n", asf_region);
    /* Update F kernel and sp */
    ret = cpp_hw_params_fill_asf_kernel(asf_7_7, asf_info, asf_region);
    if (ret < 0) {
      CPP_ASF_ERR("fill_asf_kernel failed %d", ret);
      return ret;
    }
    /* Interpolate LUT params */
    ret = cpp_hw_params_fill_lut_params(hw_params, asf_7_7,
      asf_info, aec_trigger_params, asf_region);
    if (ret < 0) {
      CPP_ASF_ERR("fill_lut_params failed %d", ret);
      return ret;
    }
    asf_info->sp_eff_en = 0;
    asf_info->neg_abs_y1 = asf_7_7->neg_abs_y1;
    asf_info->nz_flag = asf_info->nz_flag_f2 = asf_info->nz_flag_f3_f5 = asf_7_7->nz[0];

    CPP_ASF_LOW("min_ds %f max_ds %f max_factor %f",
      asf_info->sharp_min_ds_factor, asf_info->sharp_max_ds_factor,
      asf_info->sharp_max_factor);

  } else if (hw_params->asf_mode == CPP_PARAM_ASF_EMBOSS) {
    cpp_hw_params_set_emboss_effect(hw_params);
  } else if (hw_params->asf_mode == CPP_PARAM_ASF_SKETCH) {
    cpp_hw_params_set_sketch_effect(hw_params);
  } else if (hw_params->asf_mode == CPP_PARAM_ASF_NEON) {
    cpp_hw_params_set_neon_effect(hw_params);
  }
  return 0;
}

static int32_t cpp_hw_params_asf_interpolate_1_2_x(
  cpp_hardware_params_t *hw_params,
  modulesChromatix_t *module_chromatix,
  cpp_params_aec_trigger_info_t *trigger_input)
{
  chromatix_ASF_7x7_type *chromatix_ASF_7x7 = NULL;
  cpp_params_aec_trigger_t aec_trigger_params;
  chromatix_parms_type    *chromatix_ptr = NULL;

  if (hw_params->asf_lock) {
    CPP_ASF_ERR("ASF is locked by Chromatix");
    return 0;
  }

  if (!module_chromatix || !module_chromatix->chromatixPtr) {
    CPP_ASF_ERR("failed, chromatix NULL, using default params");
    /* Use default */
    cpp_hw_params_update_default_asf_params(hw_params);
    return 0;
  }

  chromatix_ptr = (chromatix_parms_type *)module_chromatix->chromatixPtr;
  chromatix_ASF_7x7 = &chromatix_ptr->chromatix_VFE.chromatix_ASF_7x7;

  hw_params->aec_trigger.lux_idx = trigger_input->lux_idx;
  hw_params->aec_trigger.gain = trigger_input->gain;

  /* determine the control method */
  if (chromatix_ASF_7x7->control_asf_7x7 == 0) {
    /* Lux index based */
    aec_trigger_params.lowlight_trigger_start =
      chromatix_ASF_7x7->asf_7x7_lowlight_trigger.lux_index_start;
    aec_trigger_params.lowlight_trigger_end =
      chromatix_ASF_7x7->asf_7x7_lowlight_trigger.lux_index_end;
    aec_trigger_params.brightlight_trigger_start =
      chromatix_ASF_7x7->asf_7x7_brightlight_trigger.lux_index_start;
    aec_trigger_params.brightlight_trigger_end =
      chromatix_ASF_7x7->asf_7x7_brightlight_trigger.lux_index_end;
    aec_trigger_params.aec_trigger_input = trigger_input->lux_idx;
  } else if (chromatix_ASF_7x7->control_asf_7x7 == 1) {
    /* Gain based */
    aec_trigger_params.lowlight_trigger_start =
      chromatix_ASF_7x7->asf_7x7_lowlight_trigger.gain_start;
    aec_trigger_params.lowlight_trigger_end =
      chromatix_ASF_7x7->asf_7x7_lowlight_trigger.gain_end;
    aec_trigger_params.brightlight_trigger_start =
      chromatix_ASF_7x7->asf_7x7_brightlight_trigger.gain_start;
    aec_trigger_params.brightlight_trigger_end =
      chromatix_ASF_7x7->asf_7x7_brightlight_trigger.gain_end;
    aec_trigger_params.aec_trigger_input = trigger_input->gain;
  } else {
    /* Error in chromatix */
    CPP_ASF_ERR("invalid chromatix control type");
    return -EINVAL;
  }
  CPP_ASF_LOW("low start, end %f %f, bright start, end %f %f, trigger %f\n",
    aec_trigger_params.lowlight_trigger_start,
    aec_trigger_params.lowlight_trigger_end,
    aec_trigger_params.brightlight_trigger_start,
    aec_trigger_params.brightlight_trigger_end,
    aec_trigger_params.aec_trigger_input);
  return cpp_hw_params_update_asf_params(hw_params, chromatix_ptr,
    &aec_trigger_params);
}

#endif /* ifdef CHROMATIX_304 */

/* ASF for FW 1.6.x */

/* interpolation of 64-entry float LUT to 256 entry uint8 LUT
    Qfactor: Qfactor for fixed point entry in lut256
    flip: boolean value specifying flip of the table
*/
static void cpp_hw_params_conv_float_lut64_to_fixed_lut256(float *lut64,
  uint8_t *lut256, int Qfactor, int flip, uint32_t thres1, uint32_t thres2)
{
  int32_t i;
  uint8_t M = 16;
  uint8_t n = Qfactor;

  for (i=0; i<64; i++) {
    lut256[i*4] = clamp(to_MuQn(M, n, lut64[i]), thres1, thres2);
    if (i < 63) {
      lut256[i*4 + 1] = clamp(to_MuQn(M, n, 0.75*lut64[i] + 0.25*lut64[i+1]),
        thres1, thres2);
      lut256[i*4 + 2] = clamp(to_MuQn(M, n, 0.5*lut64[i]  + 0.5*lut64[i+1]),
        thres1, thres2);
      lut256[i*4 + 3] = clamp(to_MuQn(M, n, 0.25*lut64[i] + 0.75*lut64[i+1]),
        thres1, thres2);
    } else {

      lut256[i*4 + 1] = clamp(lut256[i*4], thres1, thres2);
      lut256[i*4 + 2] = clamp(lut256[i*4], thres1, thres2);
      lut256[i*4 + 3] = clamp(lut256[i*4], thres1, thres2);
    }
    if (flip) {
      lut256[i*4 + 0] = 255 - lut256[i*4 + 0];
      lut256[i*4 + 1] = 255 - lut256[i*4 + 1];
      lut256[i*4 + 2] = 255 - lut256[i*4 + 2];
      lut256[i*4 + 3] = 255 - lut256[i*4 + 3];
    }
  }
}

/* interpolation of 64-entry LUT to 256 entry LUT */
static void cpp_hw_params_conv_uint_lut64_to_lut256(uint32_t *lut64,
  uint8_t *lut256, uint32_t thres1, uint32_t thres2)
{
  int32_t i;
  for (i=0; i<64; i++) {
    lut64[i] = clamp(lut64[i], thres1, thres2);
    lut256[i*4] = (lut64[i] & 0xff);
    if (i < 63) {
      lut256[i*4 + 1] = clamp((uint32_t)Round(0.75f*lut64[i] + 0.25f*lut64[i+1]),
        thres1, thres2);
      lut256[i*4 + 2] = clamp((uint32_t)Round(0.5f*lut64[i]  + 0.5f*lut64[i+1]),
        thres1, thres2);
      lut256[i*4 + 3] = clamp((uint32_t)Round(0.25f*lut64[i] + 0.75f*lut64[i+1]),
        thres1, thres2);
    } else {
      lut256[i*4 + 1] = (lut64[i] & 0xff);
      lut256[i*4 + 2] = (lut64[i] & 0xff);
      lut256[i*4 + 3] = (lut64[i] & 0xff);
    }
  }
  for (i=0; i<256; i++) {
    lut256[i] = clamp(lut256[i], thres1, thres2);
  }
}

#if defined (CHROMATIX_304) || defined (CHROMATIX_306) || \
  defined (CHROMATIX_307) || defined (CHROMATIX_308) || defined (CHROMATIX_308E) || \
  defined (CHROMATIX_309) || defined (CHROMATIX_310) || defined (CHROMATIX_310E)
static void cpp_hw_params_set_emboss_effect_1_6_x(cpp_hardware_params_t *hw_params)
{
  uint32_t i = 0;
  cpp_1_6_x_asf_info_t *info_1_6_x = NULL;

  if (!hw_params) {
    CPP_ASF_ERR("failed: hw_params\n");
    return;
  }

  info_1_6_x = &hw_params->asf_info.info_1_6_x;
  memset(info_1_6_x, 0, sizeof(cpp_1_6_x_asf_info_t));
  info_1_6_x->neg_abs_y1 = 0;
  info_1_6_x->sp_eff_en = 1;
  info_1_6_x->sp = 0;
  info_1_6_x->nz_flag = 0x1A90;
  info_1_6_x->nz_flag_2 = 0x1A90;
  info_1_6_x->dyna_clamp_en = 0;
  info_1_6_x->gamma_cor_luma_target = 128;
  info_1_6_x->sp_eff_abs_en = 0;
  for (i = 0; i < 26; i++) {
    info_1_6_x->sobel_v_coeff[i] = 0.0;
  }
  for (i = 0; i < 26; i++) {
    info_1_6_x->hpf_h_coeff[i] = 0.0;
  }

  for (i = 0; i < 26; i++) {
    info_1_6_x->sobel_h_coeff[i] = 0.0;
  }
  for (i = 0; i < 26; i++) {
    info_1_6_x->hpf_v_coeff[i] = 0.0;
  }
  info_1_6_x->sobel_h_coeff[18] = -256;
  for (i = 0; i < 26; i++) {
    info_1_6_x->lpf_coeff[i] = 0.0;
  }
  info_1_6_x->lpf_coeff[14] = 128;
  info_1_6_x->clamp_h_ul = 0;
  info_1_6_x->clamp_h_ll = 0;
  info_1_6_x->clamp_v_ul = 0;
  info_1_6_x->clamp_v_ll = 0;
  info_1_6_x->clamp_tl_ll = -255;
  info_1_6_x->clamp_tl_ul = 255;
}

static void cpp_hw_params_set_sketch_effect_1_6_x(cpp_hardware_params_t *hw_params)
{
  uint32_t i = 0;
  cpp_1_6_x_asf_info_t *info_1_6_x = NULL;

  if (!hw_params) {
    CPP_ASF_ERR("failed: hw_params \n");
    return;
  }

  info_1_6_x = &hw_params->asf_info.info_1_6_x;
  memset(info_1_6_x, 0, sizeof(cpp_1_6_x_asf_info_t));
  info_1_6_x->sp_eff_en = 1;
  info_1_6_x->neg_abs_y1 = 1;
  info_1_6_x->sp = 0;
  info_1_6_x->nz_flag = 0x1A90;
  info_1_6_x->nz_flag_2 = 0x1A90;
  info_1_6_x->dyna_clamp_en = 0;
  info_1_6_x->gamma_cor_luma_target = 192;
  info_1_6_x->sp_eff_abs_en = 1;
  for (i = 0; i < 26; i++) {
    info_1_6_x->sobel_v_coeff[i] = 0.0;
  }
  for (i = 0; i < 26; i++) {
    info_1_6_x->hpf_h_coeff[i] = 0.0;
  }

  for (i = 0; i < 26; i++) {
    info_1_6_x->sobel_h_coeff[i] = 0.0;
  }
  info_1_6_x->sobel_h_coeff[18] = -256;
  for (i = 0; i < 26; i++) {
    info_1_6_x->hpf_v_coeff[i] = 0.0;
  }
  for (i = 0; i < 26; i++) {
    info_1_6_x->lpf_coeff[i] = 0.0;
  }
  info_1_6_x->sobel_h_coeff[13] = (int16_t) (Round(-0.125*(1<<10)));
  info_1_6_x->sobel_h_coeff[14] = (int16_t) (Round(-0.0625*(1<<10)));
  info_1_6_x->sobel_h_coeff[17] = (int16_t) (Round(-0.125*(1<<10)));
  info_1_6_x->sobel_h_coeff[18] = (int16_t) (Round(-0.25*(1<<10)));
  info_1_6_x->sobel_h_coeff[19] = (int16_t) (Round(-0.25*(1<<10)));
  info_1_6_x->sobel_h_coeff[22] = (int16_t) (Round(-0.0625*(1<<10)));
  info_1_6_x->sobel_h_coeff[23] = (int16_t) (Round(-0.25*(1<<10)));

  info_1_6_x->lpf_coeff[14] = 256;
  info_1_6_x->clamp_h_ul = 0;
  info_1_6_x->clamp_h_ll = 0;
  info_1_6_x->clamp_v_ul = 0;
  info_1_6_x->clamp_v_ll = 0;
  info_1_6_x->clamp_tl_ll = -255;
  info_1_6_x->clamp_tl_ul = 255;
}

static void cpp_hw_params_set_neon_effect_1_6_x(cpp_hardware_params_t *hw_params)
{
  uint32_t i = 0;
  cpp_1_6_x_asf_info_t *info_1_6_x = NULL;

  if (!hw_params) {
    CPP_ASF_ERR("failed: hw_params \n");
    return;
  }

  info_1_6_x = &hw_params->asf_info.info_1_6_x;
  memset(info_1_6_x, 0, sizeof(cpp_1_6_x_asf_info_t));
  info_1_6_x->neg_abs_y1 = 0;
  info_1_6_x->sp_eff_en = 1;
  info_1_6_x->sp = 0;
  info_1_6_x->nz_flag = 0x1A90;
  info_1_6_x->nz_flag_2 = 0x1A90;
  info_1_6_x->dyna_clamp_en = 0;
  info_1_6_x->gamma_cor_luma_target = 0;
  info_1_6_x->sp_eff_abs_en = 1;
  for (i = 0; i < 26; i++) {
    info_1_6_x->sobel_v_coeff[i] = 0.0;
  }
  for (i = 0; i < 26; i++) {
    info_1_6_x->hpf_h_coeff[i] = 0.0;
  }

  for (i = 0; i < 26; i++) {
    info_1_6_x->sobel_h_coeff[i] = 0.0;
  }
  info_1_6_x->sobel_h_coeff[18] = 256;
  info_1_6_x->sobel_h_coeff[19] = 256;
  info_1_6_x->sobel_h_coeff[23] = 256;
  for (i = 0; i < 26; i++) {
    info_1_6_x->hpf_v_coeff[i] = 0.0;
  }
  for (i = 0; i < 26; i++) {
    info_1_6_x->lpf_coeff[i] = 0.0;
  }
  info_1_6_x->clamp_h_ul = 0;
  info_1_6_x->clamp_h_ll = 0;
  info_1_6_x->clamp_v_ul = 0;
  info_1_6_x->clamp_v_ll = 0;
  info_1_6_x->clamp_tl_ll = -255;
  info_1_6_x->clamp_tl_ul = 255;
}

static int32_t cpp_hw_params_update_asf_kernel_coeff_1_6_x(
  chromatix_asf_9_9_core_type *asf_9_9, cpp_params_asf_info_t *asf_info,
  int32_t region_trg_num)
{
  if (!asf_9_9 || !asf_info) {
    CPP_ASF_ERR("failed: asf_9_9 %p, asf_info %p", asf_9_9, asf_info);
    return -EINVAL;
  }
  if (region_trg_num < 1 || region_trg_num > CPP_PARAM_ASF_9x9_TRIGGER_MAX) {
    CPP_ASF_ERR("failed, region_trg_num=%d", region_trg_num);
    return -EINVAL;
  }
  uint32_t idx = region_trg_num - 1;
  uint32_t i;

  asf_info->info_1_6_x.dyna_clamp_en = asf_9_9[idx].en_dyna_clamp;

  asf_info->info_1_6_x.nz_flag = 0x00;
  asf_info->info_1_6_x.nz_flag_2 = 0x00;
  uint8_t h_nz, v_nz;
  for (i=0; i<8; i++) {
    h_nz = conv_nz_flag_int_to_uint(asf_9_9[idx].horizontal_nz[i]);
    v_nz = conv_nz_flag_int_to_uint(asf_9_9[idx].vertical_nz[i]);
    asf_info->info_1_6_x.nz_flag |= (h_nz & 0x3) << 2*i;
    asf_info->info_1_6_x.nz_flag_2 |= (v_nz & 0x3) << 2*i;
  }
  for (i = 0; i < 25; i++) {
    asf_info->info_1_6_x.sobel_h_coeff[i] = asf_9_9[idx].sobel_H_coeff[i];
    asf_info->info_1_6_x.sobel_v_coeff[i] =
      asf_9_9[idx].sobel_se_diagonal_coeff[i];
    asf_info->info_1_6_x.hpf_h_coeff[i] = asf_9_9[idx].hpf_h_coeff[i];
    asf_info->info_1_6_x.hpf_v_coeff[i] =
      asf_9_9[idx].hpf_se_diagonal_coeff[i];
  }
  for (i = 0; i < 15; i++) {
    asf_info->info_1_6_x.lpf_coeff[i] = asf_9_9[idx].lpf_coeff[i];
    asf_info->info_1_6_x.hpf_sym_coeff[i] =
      asf_9_9[idx].hpf_symmetric_coeff[i];
  }
  for (i = 0; i < 6; i++) {
    asf_info->info_1_6_x.activity_lpf_coeff[i] =
      (asf_9_9[idx].activity_lpf_coeff[i] & 0xff);
    asf_info->info_1_6_x.activity_bpf_coeff[i] =
      asf_9_9[idx].activity_band_pass_coeff[i];
  }
  return 0;
}

static int32_t cpp_hw_params_fill_asf_kernel_1_6_x(
  chromatix_asf_9_9_core_type *asf_9_9, cpp_params_asf_info_t *asf_info,
  int32_t region_idx)
{
  int32_t region_num = region_idx + 1;

  CPP_ASF_DBG("region_num %d", region_num);
  return cpp_hw_params_update_asf_kernel_coeff_1_6_x(asf_9_9,
    asf_info, region_num);
}

static int32_t cpp_hw_params_asf_adj_gain_lut(
  cpp_params_asf_info_t *asf_info, float *gain_lut64, float effects_factor,
  float asf_hdr_adj_factor)
{
  float x0, y0, x1, y1, x2, y2;
  float x, y;
  float scale_adj_factor, sharpness_ctrl_factor;
  uint32_t i;
  x = asf_info->downscale_ratio;
  x0 = asf_info->sharp_min_ds_factor;
  y0 = asf_info->sharp_max_factor;
  x1 = asf_info->sharp_max_ds_factor;
  y1 = 0.0f;
  if (x <= 1.0f) {
    if (x >= x0)
      y = 1 + ((1-y0)/(1-x0))*(x-1);
    else
      y = y0;
  } else {
    if (x < x1)
      y = 1 + ((1-y1)/(1-x1))*(x-1);
    else
      y = y1;
  }
  scale_adj_factor = y;
  sharpness_ctrl_factor = asf_info->sharpness_level;
  for (i=0; i<64; i++) {
    gain_lut64[i] = gain_lut64[i] * scale_adj_factor *
      sharpness_ctrl_factor * effects_factor * asf_hdr_adj_factor;
  }

  return 0;
}

static void cpp_hw_params_interpolate_lut_params_1_6_x(
  chromatix_asf_9_9_core_type *asf_9_9, cpp_params_asf_info_t *asf_info,
  int32_t region_trg_num1, int32_t region_trg_num2, float interpolate_factor,
  float scale_adj, float effect_factor, float asf_hdr_adj_factor)
{
  int32_t i = 0;
  uint8_t thres1 = 0, thres2 = 255;
  uint32_t idx1 = region_trg_num1 - 1;
  uint32_t idx2 = region_trg_num2 - 1;
  float lut0[64], lut2[64], lut4[64];
  float lut3[64];
  float gain_neg_lut[64];

  /* Interpolate 4 64-Entry luts */
  for(i=0; i<64; i++) {
    lut0[i] = LINEAR_INTERPOLATE(interpolate_factor,
      asf_9_9[idx1].activity_normalization_lut[i],
      asf_9_9[idx2].activity_normalization_lut[i]);
    lut2[i] = LINEAR_INTERPOLATE(interpolate_factor,
      asf_9_9[idx1].gain_lut[i],
      asf_9_9[idx2].gain_lut[i]);
    lut3[i] = LINEAR_INTERPOLATE(interpolate_factor,
      (float)asf_9_9[idx1].soft_threshold_lut[i],
      (float)asf_9_9[idx2].soft_threshold_lut[i]);
    lut4[i] = LINEAR_INTERPOLATE(interpolate_factor,
      asf_9_9[idx1].weight_modulation_lut[i],
      asf_9_9[idx2].weight_modulation_lut[i]);

    if (asf_info->info_1_6_x.cpp_1_12_ext.is_valid == true) {
      gain_neg_lut[i] = LINEAR_INTERPOLATE(interpolate_factor,
          ASF_NEG_LUT(asf_9_9, idx1, i),
          ASF_NEG_LUT(asf_9_9, idx2, i));
    }
  }
  if (ASF_RNR && asf_info->info_1_6_x.cpp_1_12_ext.is_valid == true) {
    for (i = 0; i < ASF_RNR_IN_ENTRIES; i++) {
      asf_info->info_1_6_x.cpp_1_12_ext.asf_1_12_rnr_input.rnr_gain_tbl_in[i] =
        LINEAR_INTERPOLATE(interpolate_factor,
        ASF_RADIAL_GAIN_ADJ(asf_9_9, idx1, i),
        ASF_RADIAL_GAIN_ADJ(asf_9_9, idx2, i));

      asf_info->info_1_6_x.cpp_1_12_ext.asf_1_12_rnr_input.rnr_activity_tbl_in[i] =
        LINEAR_INTERPOLATE(interpolate_factor,
        ASF_RADIAL_ACTIVITY_ADJ(asf_9_9, idx1, i),
        ASF_RADIAL_ACTIVITY_ADJ(asf_9_9, idx2, i));
    }
  }

  if (scale_adj) {
    for (i = 0; i < 64; i++) {
      lut2[i] = lut2[i] * scale_adj * effect_factor * asf_hdr_adj_factor *
        asf_info->sharpness_level;
    }
  } else {
    cpp_hw_params_asf_adj_gain_lut(asf_info, lut2, effect_factor,
      asf_hdr_adj_factor);
  }

  /* Convert LUTs to 256 entries */
  cpp_hw_params_conv_float_lut64_to_fixed_lut256(
   lut0, asf_info->info_1_6_x.lut[0], 8, TRUE, thres1, thres2);
  cpp_hw_params_conv_float_lut64_to_fixed_lut256(
    lut2, asf_info->info_1_6_x.lut[2], 5, FALSE, thres1, thres2);
  cpp_hw_params_conv_float_lut64_to_fixed_lut256(
    lut3, asf_info->info_1_6_x.lut[3], 0, FALSE, thres1, thres2);
  cpp_hw_params_conv_float_lut64_to_fixed_lut256(
    lut4, asf_info->info_1_6_x.lut[4], 8, FALSE, thres1, thres2);

  /*for firmware 1_12, add gain_neg LUT for halo issue resolving*/
  if (asf_info->info_1_6_x.cpp_1_12_ext.is_valid == true) {
    if (scale_adj) {
      for (i = 0; i < 64; i++) {
        gain_neg_lut[i] = gain_neg_lut[i] * scale_adj * effect_factor *
          asf_hdr_adj_factor * asf_info->sharpness_level;
      }
    } else {
      cpp_hw_params_asf_adj_gain_lut(asf_info, gain_neg_lut, effect_factor,
        asf_hdr_adj_factor);
    }

    cpp_hw_params_conv_float_lut64_to_fixed_lut256(
      gain_neg_lut, asf_info->info_1_6_x.cpp_1_12_ext.gain_neg_lut, 5,
      FALSE, thres1, thres2);
  }

  /* Interpolate the 256-Entry LUT1 */
  for(i=0; i<256; i++) {
    asf_info->info_1_6_x.lut[1][i] =
      255 - clamp(to_8uQ8(LINEAR_INTERPOLATE(interpolate_factor,
      asf_9_9[idx1].gain_weight_lut[i],
      asf_9_9[idx2].gain_weight_lut[i])),0, 255);
  }

  /* Smoothing Percentage */
  asf_info->info_1_6_x.sp = clamp(to_5uQ4(LINEAR_INTERPOLATE(interpolate_factor,
    asf_9_9[idx1].sp, asf_9_9[idx2].sp)/100.0f), 0, 16);

  /* Clamping parameters */
  asf_info->info_1_6_x.clamp_tl_ul =
    to_9bit_2sComplement(Round(LINEAR_INTERPOLATE(interpolate_factor,
      asf_9_9[idx1].clamp_UL, asf_9_9[idx2].clamp_UL)));
  asf_info->info_1_6_x.clamp_tl_ll =
    to_9bit_2sComplement(Round(LINEAR_INTERPOLATE(interpolate_factor,
      asf_9_9[idx1].clamp_LL, asf_9_9[idx2].clamp_LL)));
  asf_info->info_1_6_x.clamp_v_ul = 0; /* not used */
  asf_info->info_1_6_x.clamp_v_ll = 0; /* not used */
  asf_info->info_1_6_x.clamp_h_ul = 0; /* not used */
  asf_info->info_1_6_x.clamp_h_ll = 0; /* not used */
  asf_info->info_1_6_x.clamp_scale_max =
    to_MuQn(16, 6, LINEAR_INTERPOLATE(interpolate_factor,
      asf_9_9[idx1].smax, asf_9_9[idx2].smax));
  asf_info->info_1_6_x.clamp_scale_min =
    to_MuQn(16, 6, LINEAR_INTERPOLATE(interpolate_factor,
      asf_9_9[idx1].smin, asf_9_9[idx2].smin));
  asf_info->info_1_6_x.clamp_offset_max =
    (uint8_t) Round(LINEAR_INTERPOLATE(interpolate_factor,
      asf_9_9[idx1].omax, asf_9_9[idx2].omax));
  asf_info->info_1_6_x.clamp_offset_min =
    (uint8_t) Round(LINEAR_INTERPOLATE(interpolate_factor,
      asf_9_9[idx1].omin, asf_9_9[idx2].omin));

  asf_info->info_1_6_x.median_blend_lower_offset =
        to_4uQ4(LINEAR_INTERPOLATE(interpolate_factor,
         asf_9_9[idx1].median_blend_lower_offset,
          asf_9_9[idx2].median_blend_lower_offset));
  asf_info->info_1_6_x.median_blend_offset =
        to_4uQ4(LINEAR_INTERPOLATE(interpolate_factor,
         asf_9_9[idx1].median_blend_upper_offset,
          asf_9_9[idx2].median_blend_upper_offset));
  asf_info->info_1_6_x.activity_clamp_threshold =
    clamp(Round(LINEAR_INTERPOLATE(interpolate_factor,
    asf_9_9[idx1].activity_clamp_threshold,
    asf_9_9[idx2].activity_clamp_threshold)), thres1, thres2);
  asf_info->info_1_6_x.norm_scale =
    clamp(to_8uQ4(LINEAR_INTERPOLATE(interpolate_factor,
    asf_9_9[idx1].norm_scale,
    asf_9_9[idx2].norm_scale)), thres1, thres2);
  asf_info->info_1_6_x.max_val_threshold =
    clamp(asf_9_9[idx1].max_value_threshold, thres1, 16383);
  asf_info->info_1_6_x.perpen_scale_factor =
    clamp(to_7uQ4(asf_9_9[idx1].perpendicular_scale_factor), 0, 127);
  asf_info->info_1_6_x.L2_norm_en = asf_9_9[idx1].L2_norm_en;
  asf_info->info_1_6_x.gamma_cor_luma_target =
    clamp(Round(LINEAR_INTERPOLATE(interpolate_factor,
    asf_9_9[idx1].gamma_corrected_luma_target,
    asf_9_9[idx2].gamma_corrected_luma_target)), thres1, thres2);
  asf_info->info_1_6_x.gain_cap =
    clamp(to_8uQ5(LINEAR_INTERPOLATE(interpolate_factor,
    asf_9_9[idx1].gain_cap, asf_9_9[idx2].gain_cap)), thres1, thres2);
}

static void cpp_hw_params_fill_noninterpolate_params_1_6_x(
  chromatix_asf_9_9_core_type *asf_9_9, cpp_params_asf_info_t *asf_info,
  int32_t region_trg_num, float scale_adj, float effect_factor,
  float asf_hdr_adj_factor)
{
  int32_t i;
  uint32_t idx = region_trg_num - 1;
  float gain_lut[64], gain_neg_lut[64];
  uint8_t thres1 = 0, thres2 = 255;

  /* LUT0: activity normalization lut */
  cpp_hw_params_conv_float_lut64_to_fixed_lut256(
    asf_9_9[idx].activity_normalization_lut, asf_info->info_1_6_x.lut[0], 8,
    TRUE, thres1, thres2);
  /* LUT1: gain weight lut (Already 256 entries) */
  for (i=0; i<256; i++) {
    asf_info->info_1_6_x.lut[1][i] = 255 -
        clamp(to_8uQ8(asf_9_9[idx].gain_weight_lut[i]), thres1, thres2);
  }
  /* LUT2: gain lut, adjust based on scaling and sharpness ctrl */
  memcpy(gain_lut, asf_9_9[idx].gain_lut, 64*sizeof(float));

  if (asf_info->info_1_6_x.cpp_1_12_ext.is_valid == true) {
    for (i=0; i<64; i++){
      gain_neg_lut[i] = ASF_NEG_LUT(asf_9_9, idx, i);
    }
  }

  if (ASF_RNR && asf_info->info_1_6_x.cpp_1_12_ext.is_valid == true) {
    for (i = 0; i < ASF_RNR_IN_ENTRIES; i++) {
      asf_info->info_1_6_x.cpp_1_12_ext.asf_1_12_rnr_input.rnr_gain_tbl_in[i] =
        ASF_RADIAL_GAIN_ADJ(asf_9_9, idx, i);
      asf_info->info_1_6_x.cpp_1_12_ext.asf_1_12_rnr_input.rnr_activity_tbl_in[i] =
        ASF_RADIAL_ACTIVITY_ADJ(asf_9_9, idx, i);
    }
  }

  if (scale_adj) {
    for (i = 0; i < 64; i++) {
      gain_lut[i] = gain_lut[i] * scale_adj * effect_factor *
        asf_hdr_adj_factor * asf_info->sharpness_level;
    }
  } else {
    cpp_hw_params_asf_adj_gain_lut(asf_info, gain_lut, effect_factor,
      asf_hdr_adj_factor);
  }

  cpp_hw_params_conv_float_lut64_to_fixed_lut256(
    gain_lut, asf_info->info_1_6_x.lut[2], 5, FALSE, thres1, thres2);
  /* LUT3: soft threshold lut */
   cpp_hw_params_conv_uint_lut64_to_lut256(
    asf_9_9[idx].soft_threshold_lut, asf_info->info_1_6_x.lut[3],
    thres1, thres2);
  /* LUT4: soft threshold weight lut */
  cpp_hw_params_conv_float_lut64_to_fixed_lut256(
    asf_9_9[idx].weight_modulation_lut, asf_info->info_1_6_x.lut[4], 8, FALSE,
    thres1, thres2);
  /*for firmware 1_12, add gain_neg LUT for halo issue resolving*/
  if (asf_info->info_1_6_x.cpp_1_12_ext.is_valid == true) {
    if (scale_adj) {
      for (i = 0; i < 64; i++) {
        gain_neg_lut[i] = gain_neg_lut[i] * scale_adj * effect_factor *
          asf_hdr_adj_factor * asf_info->sharpness_level;
      }
    } else {
      cpp_hw_params_asf_adj_gain_lut(asf_info, gain_neg_lut, effect_factor,
        asf_hdr_adj_factor);
    }

    cpp_hw_params_conv_float_lut64_to_fixed_lut256(
      gain_neg_lut, asf_info->info_1_6_x.cpp_1_12_ext.gain_neg_lut, 5,
      FALSE, thres1, thres2);
  }

  /* Smoothing Percentage */
  asf_info->info_1_6_x.sp = clamp(to_5uQ4(asf_9_9[idx].sp / 100.0f),
    0, 16);

  /* Clamping parameters */
  asf_info->info_1_6_x.clamp_h_ul = 0; /* not used */
  asf_info->info_1_6_x.clamp_h_ll = 0; /* not used */
  asf_info->info_1_6_x.clamp_v_ul = 0; /* not used */
  asf_info->info_1_6_x.clamp_v_ll = 0; /* not used */
  asf_info->info_1_6_x.clamp_tl_ll = to_9bit_2sComplement(asf_9_9[idx].clamp_LL);
  asf_info->info_1_6_x.clamp_tl_ul = to_9bit_2sComplement(asf_9_9[idx].clamp_UL);

  asf_info->info_1_6_x.clamp_scale_max = to_MuQn(16, 6, asf_9_9[idx].smax);
  asf_info->info_1_6_x.clamp_scale_min = to_MuQn(16, 6, asf_9_9[idx].smin);
  asf_info->info_1_6_x.clamp_offset_max = (uint8_t) asf_9_9[idx].omax;
  asf_info->info_1_6_x.clamp_offset_min = (uint8_t) asf_9_9[idx].omin;

  asf_info->info_1_6_x.median_blend_lower_offset =
        to_4uQ4(asf_9_9[idx].median_blend_lower_offset);
  asf_info->info_1_6_x.median_blend_offset =
        to_4uQ4(asf_9_9[idx].median_blend_upper_offset);
  asf_info->info_1_6_x.activity_clamp_threshold =
    (uint8_t)clamp(asf_9_9[idx].activity_clamp_threshold, thres1, thres2);
  asf_info->info_1_6_x.norm_scale =
     clamp(to_8uQ4(asf_9_9[idx].norm_scale), thres1, thres2);
  asf_info->info_1_6_x.max_val_threshold =
    clamp(asf_9_9[idx].max_value_threshold, thres1, 16383);
  asf_info->info_1_6_x.perpen_scale_factor =
    clamp(to_7uQ4(asf_9_9[idx].perpendicular_scale_factor), 0, 127);
  asf_info->info_1_6_x.L2_norm_en = asf_9_9[idx].L2_norm_en;
  asf_info->info_1_6_x.gamma_cor_luma_target =
    clamp(asf_9_9[idx].gamma_corrected_luma_target, thres1, thres2);
  asf_info->info_1_6_x.gain_cap =
    clamp(to_8uQ5(asf_9_9[idx].gain_cap), thres1, thres2);
}

static boolean cpp_hw_param_calc_rnr_param(
  struct asf_1_12_rnr_input_t input_rnr_params,
  struct asf_1_12_rnr_output_t *output_rnr_cfg)
{
  int         r_square_q_factor = 12;
  double      luma_r_square = 0;
  int         exp = 0;
  double      fractional = 0;
  int         pseudo_mantissa = 0;
  int         luma_r_square_table[ASF_RNR_IN_ENTRIES];
  int         i = 0;
  double      temp = 0;

  if (output_rnr_cfg == NULL) {
    CPP_ASF_ERR("invalid ptr output_rnr_cfg = %p", output_rnr_cfg);
    return false;
  }

  memset(luma_r_square_table, 0 , sizeof(int) * ASF_RNR_IN_ENTRIES);
  luma_r_square = input_rnr_params.h_center * input_rnr_params.h_center +
    input_rnr_params.v_center * input_rnr_params.v_center;

  luma_r_square /= ((input_rnr_params.isp_scale_ratio *
                     input_rnr_params.cpp_scale_ratio) *
                     (input_rnr_params.isp_scale_ratio *
                     input_rnr_params.cpp_scale_ratio));

  /* algorithm: try to use 12 bit
                to express the floating point for the further calculation*/
  fractional = frexp(luma_r_square, &exp);
  pseudo_mantissa = round(fractional * (1 << r_square_q_factor));
  if (pseudo_mantissa >= (1 << r_square_q_factor)) {
    exp++;
  }

  /*fill in rnr output cfg: rnr_r_sqr_shift*/
  if (exp - (r_square_q_factor + 1) >= 0) {
    output_rnr_cfg->rnr_r_sqr_shift = exp - (r_square_q_factor + 1);
  } else {
    output_rnr_cfg->rnr_r_sqr_shift = 0;
  }

  /*fill in rnr output cfg: rnr_r_sqr_lut*/
  for (i = 0; i < ASF_RNR_IN_ENTRIES; i++) {
    pseudo_mantissa = round(luma_r_square *
                            input_rnr_params.radial_point[i] /
                            (1 << (output_rnr_cfg->rnr_r_sqr_shift + 1)));

    if (i < CPP_ASF_RNR_LUT_ENTRIES_1_12_x) {
      output_rnr_cfg->rnr_r_sqr_lut[i] = pseudo_mantissa;
    }
    luma_r_square_table[i] = pseudo_mantissa;
  }

  /*normalize input rnr param*/
  for (i = 0; i < ASF_RNR_IN_ENTRIES; i++) {
    /*range need to be 0 ~ 2047/256*/
    input_rnr_params.rnr_gain_tbl_in[i] =
      MIN((2047. / 256.), MAX(input_rnr_params.rnr_gain_tbl_in[i], 0));

    input_rnr_params.rnr_activity_tbl_in[i] =
      MIN((2047. / 256.), MAX(input_rnr_params.rnr_activity_tbl_in[i], 0));
  }

  for (i = 0; i < CPP_ASF_RNR_LUT_ENTRIES_1_12_x; i++) {
    /*fill in rnr output cfg: gain CF LUT, activity LUT*/
    output_rnr_cfg->rnr_gain_cf_lut[i] =
      round(input_rnr_params.rnr_gain_tbl_in[i] * (1 << 8));
    output_rnr_cfg->rnr_activity_cf_lut[i] =
      round(input_rnr_params.rnr_activity_tbl_in[i] * (1 << 8));


    /*fill in rnr output cfg: gain CF slope, gain CF shift */
    temp = (input_rnr_params.rnr_gain_tbl_in[i + 1]
             - input_rnr_params.rnr_gain_tbl_in[i]) /
           (luma_r_square_table[i + 1] - luma_r_square_table[i]);
    fractional = frexp(temp, &exp);
    if (fractional < 0) {
      fractional *= -1;
    }
    pseudo_mantissa = ceil(fractional * (1 << 11));
    if (pseudo_mantissa >= 2048) {
      fractional = 0.5;
      exp++;
    }

    /*slope and shift need to compensate*/
    if (-exp + 2 >= 0) {
      output_rnr_cfg->rnr_gain_shift_lut[i] = -exp + 2;
      output_rnr_cfg->rnr_gain_slope_lut[i] = ceil(fractional * (1 << 11));
    } else {
      output_rnr_cfg->rnr_gain_shift_lut[i] = 0;
      output_rnr_cfg->rnr_gain_slope_lut[i] =
        ceil(fractional * (1 << (11 + exp - 2)));
    }

    if (temp < 0) {
      output_rnr_cfg->rnr_gain_slope_lut[i] *= -1;
    }

    /*fill in rnr output cfg: activity CF slope, activity CF shift */
    temp = (input_rnr_params.rnr_activity_tbl_in[i + 1]
             - input_rnr_params.rnr_activity_tbl_in[i]) /
           (luma_r_square_table[i + 1] - luma_r_square_table[i]);
    fractional = frexp(temp, &exp);
    if (fractional < 0) {
      fractional *= -1;
    }
    pseudo_mantissa = ceil(fractional * (1 << 11));
    if (pseudo_mantissa >= 2048) {
      fractional = 0.5;
      exp++;
    }

    /*slope and shift need to compensate*/
    if (-exp + 2 >= 0) {
      output_rnr_cfg->rnr_activity_shift_lut[i] = -exp + 2;
      output_rnr_cfg->rnr_activity_slope_lut[i] = ceil(fractional * (1 << 11));
    } else {
      output_rnr_cfg->rnr_activity_shift_lut[i] = 0;
      output_rnr_cfg->rnr_activity_slope_lut[i] =
        ceil(fractional * (1 << (11 + exp - 2)));
    }

    if (temp < 0) {
      output_rnr_cfg->rnr_activity_slope_lut[i] *= -1;
    }
  }

  return true;
}

static int32_t cpp_hw_params_fill_lut_params_1_6_x(
  chromatix_asf_9_9_core_type *asf_9_9, cpp_params_asf_info_t *asf_info,
  float scale_adj, float effect_factor, float asf_hdr_adj_factor,
  float interpolate_factor, uint32_t reg1_idx, uint32_t reg2_idx,
  chromatix_cpp_type *chromatix_cpp __unused,
  cpp_hardware_params_t *hw_params __unused,
  cpp_params_aec_trigger_info_t *aec_trigger __unused)
{
  uint32_t reg1,reg2;
  boolean ret = true;

  reg1 = reg1_idx + 1;
  reg2 = reg2_idx + 1;

  if (interpolate_factor != 0.0f) {
    CPP_ASF_DBG("asf interpolation=yes factor=%f, region=(%d, %d)",
      interpolate_factor, reg1, reg2);
    cpp_hw_params_interpolate_lut_params_1_6_x(asf_9_9, asf_info,
      reg1, reg2, interpolate_factor, scale_adj, effect_factor,
      asf_hdr_adj_factor);
  } else {
    CPP_ASF_DBG("asf interpolation=no region=%d", reg1);
    cpp_hw_params_fill_noninterpolate_params_1_6_x(asf_9_9, asf_info, reg1,
      scale_adj, effect_factor, asf_hdr_adj_factor);
  }

  memset(&asf_info->info_1_6_x.cpp_1_12_ext.asf_1_12_rnr_output, 0,
    sizeof(struct asf_1_12_rnr_output_t));
  if (asf_info->info_1_6_x.cpp_1_12_ext.is_valid == true) {
    ret = cpp_hw_update_hdr_2d_asf_radial_ext(chromatix_cpp, hw_params,
      aec_trigger, &asf_info->info_1_6_x.cpp_1_12_ext.asf_1_12_rnr_input);
    if (ret < 0) {
      CPP_DBG("Update hdr 2d radial ext failed %d", ret);
    }

    ret = cpp_hw_param_calc_rnr_param(
      asf_info->info_1_6_x.cpp_1_12_ext.asf_1_12_rnr_input,
      &asf_info->info_1_6_x.cpp_1_12_ext.asf_1_12_rnr_output);
    if (ret == false) {
      CPP_ASF_ERR("failed: cpp_hw_param_calc_rnr_param failed!");
      return -EINVAL;
    }
  }
  return 0;
}

static void cpp_hw_params_update_default_asf_params_1_6_x(
  cpp_hardware_params_t *hw_params)
{
  uint32_t i = 0;
  cpp_params_asf_info_t *asf_info = &hw_params->asf_info;
  /* TODO */
}

int32_t cpp_hw_params_update_asf_hdr_factor(cpp_hardware_params_t *hw_params,
  Chromatix_HDR_ASF_adj_type *hdr_asf_adj_type_ptr,
  cpp_params_aec_trigger_info_t *aec_trigger)
{
  uint32_t                    i, end_idx;
  float                       trigger_start = 0, trigger_end = 0;
  float                       interpolation_factor;
  float                       numerator, denominator;
  float                       trigger_input = 0;
  float ref_factor_i;
  float ref_factor_iplus1;

  if (!hw_params) {
    CPP_ASF_ERR("invalid hw_params, failed");
    return -EINVAL;
  }

  if (!hdr_asf_adj_type_ptr) {
    CPP_ASF_ERR("failed: No ASF HDR chromatix!!");
    return -1;
  }

  if(GET_HDR_ASF_ADJ_CONTROL_TYPE(hdr_asf_adj_type_ptr) == CONTROL_AEC_EXP_SENSITIVITY_RATIO){
    //CONTROL_AEC_EXP_SENSITIVITY_RATIO
    trigger_input = aec_trigger->aec_sensitivity_ratio;
    CPP_ASF_DBG("zzhdr exp sens ratio trigger %f ", trigger_input);
  } else if(GET_HDR_ASF_ADJ_CONTROL_TYPE(hdr_asf_adj_type_ptr) == CONTROL_EXP_TIME_RATIO) {
    //CONTROL_EXP_TIME_RATIO
    trigger_input = aec_trigger->exp_time_ratio;
    CPP_ASF_DBG("zzhdr exp time ratio trigger %f ", trigger_input);
  }

  i = 0;
  end_idx = MAX_SETS_FOR_TONE_NOISE_ADJ - 1;
  interpolation_factor = 0;
  ref_factor_i = GET_HDR_ASF_ADJ_DATA_ADJ_FACTOR(hdr_asf_adj_type_ptr, end_idx);
  ref_factor_iplus1 = GET_HDR_ASF_ADJ_DATA_ADJ_FACTOR(hdr_asf_adj_type_ptr, end_idx);

  for (i = 0; i < end_idx; i++) {
    if(GET_HDR_WNR_ADJ_CONTROL_TYPE(hdr_asf_adj_type_ptr) == CONTROL_AEC_EXP_SENSITIVITY_RATIO){
      //CONTROL_AEC_EXP_SENSITIVITY_RATIO
      CPP_ASF_DBG(" zzhdr exp sens ratio");
      trigger_start =
        GET_HDR_ASF_ADJ_DATA_AEC_SENSITIVITY_RATIO_START(hdr_asf_adj_type_ptr,i);
      trigger_end =
        GET_HDR_ASF_ADJ_DATA_AEC_SENSITIVITY_RATIO_END(hdr_asf_adj_type_ptr, i);
    } else if(GET_HDR_ASF_ADJ_CONTROL_TYPE(hdr_asf_adj_type_ptr) == CONTROL_EXP_TIME_RATIO) {
      //CONTROL_EXP_TIME_RATIO
      CPP_ASF_DBG(" zzhdr exp time ratio");
      trigger_start =
        GET_HDR_ASF_ADJ_DATA_EXP_TIME_RATIO_START(hdr_asf_adj_type_ptr,i);
      trigger_end =
        GET_HDR_ASF_ADJ_DATA_EXP_TIME_RATIO_END(hdr_asf_adj_type_ptr, i);
    }

    if (trigger_input <= trigger_start) {
      CPP_ASF_DBG("non interpolate i = %d ", i);
      ref_factor_i = GET_HDR_ASF_ADJ_DATA_ADJ_FACTOR(hdr_asf_adj_type_ptr, i);
      ref_factor_iplus1 = GET_HDR_ASF_ADJ_DATA_ADJ_FACTOR(hdr_asf_adj_type_ptr, i);
      break;
    }
    if (trigger_input < trigger_end) {
      /* Interpolate all the values */
      numerator = (trigger_input - trigger_start);
      denominator = (trigger_end - trigger_start);
      if (denominator == 0.0f) {
        CPP_ASF_DBG("non interpolate div by 0 i = %d ", i);
        ref_factor_i = GET_HDR_ASF_ADJ_DATA_ADJ_FACTOR(hdr_asf_adj_type_ptr, i);
        ref_factor_iplus1 = GET_HDR_ASF_ADJ_DATA_ADJ_FACTOR(hdr_asf_adj_type_ptr, i);
        break;
      }
      interpolation_factor = numerator / denominator;
      ref_factor_i = GET_HDR_ASF_ADJ_DATA_ADJ_FACTOR(hdr_asf_adj_type_ptr, i);
      ref_factor_iplus1 = GET_HDR_ASF_ADJ_DATA_ADJ_FACTOR(hdr_asf_adj_type_ptr, i+1);


      CPP_ASF_DBG("interpolate, factor=%f, i=%d, i+1=%d", interpolation_factor, i, i+1);
      break;
    } /* else iterate */
  }

  CPP_ASF_DBG("sending for interpolation i = %d factor %f ref 1 %f ref2 %f",
    i, interpolation_factor, ref_factor_i, ref_factor_iplus1);

  hw_params->asf_hdr_adj =
    LINEAR_INTERPOLATE(interpolation_factor, ref_factor_i, ref_factor_iplus1);

  CPP_ASF_DBG("hw_params->asf_hdr_adj = %f ", hw_params->asf_hdr_adj);
  return 0;

}


static int32_t cpp_hw_params_update_asf_params_1_6_x(
  cpp_hardware_params_t *hw_params,
  chromatix_cpp_type *chromatix_cpp,
  cpp_params_aec_trigger_info_t *trigger_input)
{
  int32_t                 ret = 0;
  uint32_t                i = 0;
  chromatix_ASF_9x9_type *chromatix_asf = NULL;
  chromatix_asf_9_9_core_type *asf_9_9 = NULL;
  cpp_params_asf_info_t  *asf_info = NULL;
  cpp_params_asf_region_t asf_region = CPP_PARAM_ASF_REGION_MAX;
  float isp_scale_ratio, cpp_scale_ratio;
  float effect_factor = 1.0f;
  Chromatix_HDR_ASF_adj_type  *hdr_asf_adj_type_ptr;
  float aec_trigger_input;
  float interpolation_factor;
  uint32_t asf_region1_idx, asf_region2_idx;

  GET_ASF_POINTER(chromatix_cpp, hw_params, chromatix_asf);
  asf_9_9 = chromatix_asf->asf_9_9;
  asf_info = &hw_params->asf_info;

  /* Fill downscale factor in asf_info which will be used later to adjust
     LUT params */
  asf_info->sharp_min_ds_factor =
    chromatix_asf->asf_9_9_sharp_min_ds_factor;
  asf_info->sharp_max_ds_factor =
    chromatix_asf->asf_9_9_sharp_max_ds_factor;
  asf_info->sharp_max_factor =
    chromatix_asf->asf_9_9_sharp_max_factor;

  CPP_ASF_LOW("min_ds=%f max_ds=%f max_factor=%f",
    asf_info->sharp_min_ds_factor, asf_info->sharp_max_ds_factor,
    asf_info->sharp_max_factor);

  if (chromatix_asf->control_asf_9x9 == CPP_ASF_CONTROL_LUX_BASED) {
    /* Lux index based */
    CPP_ASF_DBG("ASF lux triggered");
    aec_trigger_input = trigger_input->lux_idx;
  } else {
    CPP_ASF_DBG("ASF gain triggered");
    aec_trigger_input = trigger_input->gain;
  }

  /* Fill ASF parameters in frame_params */
  if (!chromatix_asf->asf_9x9_enable ||
    aec_trigger_input <= 0.0f) {
    /* Use default */
    CPP_ASF_HIGH("using default asf params, asf_en=%d, trig_in=%f",
      chromatix_asf->asf_9x9_enable, aec_trigger_input);
    cpp_hw_params_update_default_asf_params_1_6_x(hw_params);
    return ret;
  }

  CPP_ASF_DBG("asf_mode = %d", hw_params->asf_mode);
  switch (hw_params->asf_mode) {
  case CPP_PARAM_ASF_OFF:
      break;
  case CPP_PARAM_ASF_BEAUTYSHOT:
      effect_factor *= CPP_PARAM_BEAUTY_EFFECT_ASF_FACTOR;
  case CPP_PARAM_ASF_DUAL_FILTER:
      /* non-region-specific params */
      asf_info->info_1_6_x.sp_eff_en = 0;
      asf_info->info_1_6_x.sp_eff_abs_en = 0;
      asf_info->info_1_6_x.neg_abs_y1 = 0;
      asf_info->info_1_6_x.symm_filter_only = ASF_USE_SYMM_FILTER_ONLY(chromatix_asf);
      if(hw_params->hyst_asf_rnr_status)
        asf_info->info_1_6_x.radial_enable = ASF_RADIAL_ENABLE(chromatix_asf);
      else
        asf_info->info_1_6_x.radial_enable = 0;

      /* find region based on trigger input */
      cpp_hw_params_asf_find_region_idxs(chromatix_asf,
        hw_params->lds_enable, aec_trigger_input,
        &asf_region1_idx, &asf_region2_idx,
        &interpolation_factor);

      /* assign region 2 indicies in info struct*/
      asf_info->asf_reg1_idx = asf_region1_idx;
      asf_info->asf_reg2_idx = asf_region2_idx;

      /* update non-interpolated params */
      ret = cpp_hw_params_fill_asf_kernel_1_6_x(asf_9_9, asf_info,
        asf_region1_idx);
      if (ret < 0) {
        CPP_ASF_ERR(" fill_asf_kernel_1_6_x failed %d", ret);
        return ret;
      }

      hw_params->asf_scalor_adj = 1.0f;
      ret = cpp_hw_params_asf_scalor_interpolate(hw_params, chromatix_cpp,
        hw_params->isp_scale_ratio);
      ret = cpp_hw_params_asf_scalor_interpolate(hw_params, chromatix_cpp,
        hw_params->cpp_scale_ratio);

      if (ret) {
         asf_info->downscale_ratio = hw_params->isp_scale_ratio *
           hw_params->cpp_scale_ratio;
         hw_params->asf_scalor_adj = 0.0f;
      }

      hw_params->asf_hdr_adj = 1.0f;

      GET_HDR_ASF_ADJ_TYPE_PTR(chromatix_cpp, hw_params, hdr_asf_adj_type_ptr);

      if (GET_HDR_ASF_ADJ_TYPE_ENABLE(hdr_asf_adj_type_ptr) &&
        (hw_params->hdr_mode == CAM_SENSOR_HDR_ZIGZAG)) {
        CPP_ASF_DBG("ASF HDR enabled");
        cpp_hw_params_update_asf_hdr_factor(hw_params, hdr_asf_adj_type_ptr,
          trigger_input);
      } else {
        CPP_ASF_DBG("ASF HDR not enabled");
      }

      asf_info->sharpness_level = hw_params->sharpness_level;

      if (ASF_RNR) {
        memset(&asf_info->info_1_6_x.cpp_1_12_ext.asf_1_12_rnr_input, 0 ,
          sizeof(struct asf_1_12_rnr_input_t));
        asf_info->info_1_6_x.cpp_1_12_ext.asf_1_12_rnr_input.h_center =
          hw_params->camif_dim.width/2;
        asf_info->info_1_6_x.cpp_1_12_ext.asf_1_12_rnr_input.v_center =
          hw_params->camif_dim.height/2;
        asf_info->info_1_6_x.cpp_1_12_ext.asf_1_12_rnr_input.isp_scale_ratio =
          hw_params->isp_scale_ratio;
        asf_info->info_1_6_x.cpp_1_12_ext.asf_1_12_rnr_input.cpp_scale_ratio =
          hw_params->cpp_scale_ratio;
        for (i = 0; i < ASF_RNR_IN_ENTRIES; i++) {
          asf_info->info_1_6_x.cpp_1_12_ext.asf_1_12_rnr_input.radial_point[i] =
            ASF_RNR_POINT_LUT(chromatix_asf, i);
        }
      }

      /* Update interpolated LUT params */
      ret = cpp_hw_params_fill_lut_params_1_6_x(asf_9_9, asf_info,
        hw_params->asf_scalor_adj, effect_factor,
        hw_params->asf_hdr_adj, interpolation_factor,
        asf_region1_idx, asf_region2_idx,
        chromatix_cpp, hw_params, trigger_input);
      if (ret < 0) {
        CPP_ASF_ERR("fill_lut_params_1_6 failed %d", ret);
        return ret;
      }
      break;
  case CPP_PARAM_ASF_EMBOSS:
      CPP_ASF_DBG("EFFECTDEBUG CPP_PARAM_ASF_EMBOSS\n");
      cpp_hw_params_set_emboss_effect_1_6_x(hw_params);
      break;
  case CPP_PARAM_ASF_SKETCH:
      CPP_ASF_DBG("EFFECTDEBUG CPP_PARAM_ASF_SKETCH\n");
      cpp_hw_params_set_sketch_effect_1_6_x(hw_params);
      break;
  case CPP_PARAM_ASF_NEON:
      CPP_ASF_DBG("EFFECTDEBUG CPP_PARAM_ASF_NEON\n");
      cpp_hw_params_set_neon_effect_1_6_x(hw_params);
      break;
  default:
      CPP_ASF_ERR("invalid asf_mode=%d", hw_params->asf_mode);
      return -EINVAL;
  }
  return 0;
}
#endif /* #if CHROMATIX_304 */

void cpp_hw_params_fill_default_asf_1_6_x(cpp_params_asf_info_t *asf_info)
{
  uint32_t i;
  /* LUT0: activity normalization lut */
  cpp_hw_params_conv_float_lut64_to_fixed_lut256(
    d_activity_normalization_lut, asf_info->info_1_6_x.lut[0], 8, TRUE,0 ,255);
  /* LUT1: gain weight lut (Already 256 entries) */
  for (i=0; i<256; i++) {
    asf_info->info_1_6_x.lut[1][i] = 256 - clamp(to_8uQ8(d_gain_weight_lut[i]),
      0, 255);
  }
  /* LUT2: gain lut */
  cpp_hw_params_conv_float_lut64_to_fixed_lut256(
    d_gain_lut, asf_info->info_1_6_x.lut[2], 5, FALSE, 0, 255);
  /* LUT3: soft threshold lut */
  cpp_hw_params_conv_uint_lut64_to_lut256(
    d_soft_threshold_lut, asf_info->info_1_6_x.lut[3], 0, 255);
  /* LUT4: soft threshold weight lut */
  cpp_hw_params_conv_float_lut64_to_fixed_lut256(
    d_weight_modulation_lut, asf_info->info_1_6_x.lut[4], 8, FALSE,0 ,255);
  /* Smoothing Percentage */
  asf_info->info_1_6_x.sp = clamp(to_5uQ4(d_sp / 100.0f), 0, 16);
  /* Clamping parameters */
  asf_info->info_1_6_x.clamp_h_ul = (uint16_t) d_clamp_UL;
  asf_info->info_1_6_x.clamp_h_ll = (uint16_t) d_clamp_LL;
  asf_info->info_1_6_x.clamp_v_ul = (uint16_t) d_clamp_UL;
  asf_info->info_1_6_x.clamp_v_ll = (uint16_t) d_clamp_LL;
  asf_info->info_1_6_x.clamp_scale_max = (uint16_t) d_smax;
  asf_info->info_1_6_x.clamp_scale_min = (uint16_t) d_smin;
  asf_info->info_1_6_x.clamp_offset_max = (uint16_t) d_omax;
  asf_info->info_1_6_x.clamp_offset_min = (uint16_t) d_omin;

  /* kernel */
  asf_info->info_1_6_x.dyna_clamp_en = d_en_dyna_clamp;
  asf_info->info_1_6_x.gamma_cor_luma_target =
    (uint8_t) d_gamma_corrected_luma_target;
  asf_info->info_1_6_x.gain_cap = (uint8_t) d_gain_cap;

  asf_info->info_1_6_x.nz_flag = 0x00;
  asf_info->info_1_6_x.nz_flag_2 = 0x00;
  for (i=0; i<8; i++) {
    asf_info->info_1_6_x.nz_flag |=
      ((d_horizontal_nz[i] & 0x3) << 2*i);
  }
  for (i=0; i<8; i++) {
    asf_info->info_1_6_x.nz_flag_2 |=
      ((d_downward_diagonal_nz[i] & 0x3) << 2*i);
  }
  for (i = 0; i < 25; i++) {
    asf_info->info_1_6_x.sobel_h_coeff[i] = d_sobel_H_coeff[i];
    asf_info->info_1_6_x.sobel_v_coeff[i] =
      d_sobel_se_diagonal_coeff[i];
    asf_info->info_1_6_x.hpf_h_coeff[i] = d_hpf_h_coeff[i];
    asf_info->info_1_6_x.hpf_v_coeff[i] = d_hpf_se_diagonal_coeff[i];
  }
  for (i = 0; i < 15; i++) {
    asf_info->info_1_6_x.lpf_coeff[i] = d_lpf_coeff[i];
    asf_info->info_1_6_x.hpf_sym_coeff[i] = d_hpf_symmetric_coeff[i];
  }
  for (i = 0; i < 6; i++) {
    asf_info->info_1_6_x.activity_lpf_coeff[i] =
      (d_activity_lpf_coeff[i] & 0xff);
    asf_info->info_1_6_x.activity_bpf_coeff[i] = d_activity_band_pass_coeff[i];
  }

  asf_info->info_1_6_x.sp_eff_en = 0;
  asf_info->info_1_6_x.neg_abs_y1 = 0;
  asf_info->info_1_6_x.activity_clamp_threshold =
     (uint8_t)d_activity_clamp_threshold;
  asf_info->info_1_6_x.norm_scale = (uint8_t)d_norm_scale;
  asf_info->info_1_6_x.max_val_threshold =
    (uint16_t)d_max_value_threshold;
  asf_info->info_1_6_x.median_blend_lower_offset =
    (uint8_t)d_median_blend_lower_offset;
  asf_info->info_1_6_x.median_blend_offset =
    (uint8_t)d_median_blend_upper_offset;
  asf_info->info_1_6_x.perpen_scale_factor =
    (uint8_t)d_perpendicular_scale_factor;
  asf_info->info_1_6_x.L2_norm_en = (uint8_t) d_L2_norm_en;

}

void cpp_hw_params_fill_stripped_asf_1_6_x(cpp_params_asf_info_t *asf_info,
  chromatix_cpp_stripped_type *chromatix)
{
  uint32_t i;
  CPP_ASF_HIGH("using stripped asf");

  /* LUT0: activity normalization lut */
  cpp_hw_params_conv_float_lut64_to_fixed_lut256(
    chromatix->chromatix_ASF_9x9.asf_9_9.activity_normalization_lut,
     asf_info->info_1_6_x.lut[0], 8, TRUE, 0 ,255);
  /* LUT1: gain weight lut (Already 256 entries) */
  for (i=0; i<256; i++) {
    asf_info->info_1_6_x.lut[1][i] = 256 -
      clamp(to_8uQ8(chromatix->chromatix_ASF_9x9.asf_9_9.gain_weight_lut[i]),
        0, 255);
  }
  /* LUT2: gain lut */
  cpp_hw_params_conv_float_lut64_to_fixed_lut256(
    chromatix->chromatix_ASF_9x9.asf_9_9.gain_lut,
     asf_info->info_1_6_x.lut[2], 5, FALSE, 0, 255);
  /* LUT3: soft threshold lut */
  cpp_hw_params_conv_uint_lut64_to_lut256(
    chromatix->chromatix_ASF_9x9.asf_9_9.soft_threshold_lut,
     asf_info->info_1_6_x.lut[3], 0, 255);
  /* LUT4: soft threshold weight lut */
  cpp_hw_params_conv_float_lut64_to_fixed_lut256(
    chromatix->chromatix_ASF_9x9.asf_9_9.weight_modulation_lut,
     asf_info->info_1_6_x.lut[4], 8, FALSE,0 ,255);
  /* Smoothing Percentage */
  asf_info->info_1_6_x.sp =
    clamp(to_5uQ4(chromatix->chromatix_ASF_9x9.asf_9_9.sp / 100.0f), 0, 16);
  /* Clamping parameters */
  asf_info->info_1_6_x.clamp_h_ul =
    (uint16_t) chromatix->chromatix_ASF_9x9.asf_9_9.clamp_UL;
  asf_info->info_1_6_x.clamp_h_ll =
    (uint16_t) chromatix->chromatix_ASF_9x9.asf_9_9.clamp_LL;
  asf_info->info_1_6_x.clamp_v_ul =
    (uint16_t) chromatix->chromatix_ASF_9x9.asf_9_9.clamp_UL;
  asf_info->info_1_6_x.clamp_v_ll =
    (uint16_t) chromatix->chromatix_ASF_9x9.asf_9_9.clamp_LL;
  asf_info->info_1_6_x.clamp_scale_max =
    (uint16_t) chromatix->chromatix_ASF_9x9.asf_9_9.smax;
  asf_info->info_1_6_x.clamp_scale_min =
    (uint16_t) chromatix->chromatix_ASF_9x9.asf_9_9.smin;
  asf_info->info_1_6_x.clamp_offset_max =
    (uint16_t) chromatix->chromatix_ASF_9x9.asf_9_9.omax;
  asf_info->info_1_6_x.clamp_offset_min =
    (uint16_t) chromatix->chromatix_ASF_9x9.asf_9_9.omin;

  /* kernel */
  asf_info->info_1_6_x.dyna_clamp_en =
    chromatix->chromatix_ASF_9x9.asf_9_9.en_dyna_clamp;
  asf_info->info_1_6_x.gamma_cor_luma_target =
    (uint8_t) chromatix->chromatix_ASF_9x9.asf_9_9.gamma_corrected_luma_target;
  asf_info->info_1_6_x.gain_cap =
    (uint8_t) chromatix->chromatix_ASF_9x9.asf_9_9.gain_cap;

  asf_info->info_1_6_x.nz_flag = 0x00;
  asf_info->info_1_6_x.nz_flag_2 = 0x00;
  for (i=0; i<8; i++) {
    asf_info->info_1_6_x.nz_flag |=
      ((chromatix->chromatix_ASF_9x9.asf_9_9.horizontal_nz[i] & 0x3) << 2*i);
  }
  for (i=0; i<8; i++) {
    asf_info->info_1_6_x.nz_flag_2 |=
      ((chromatix->chromatix_ASF_9x9.asf_9_9.vertical_nz[i] & 0x3) << 2*i);
  }
  for (i = 0; i < 25; i++) {
    asf_info->info_1_6_x.sobel_h_coeff[i] =
      chromatix->chromatix_ASF_9x9.asf_9_9.sobel_H_coeff[i];
    asf_info->info_1_6_x.sobel_v_coeff[i] =
      chromatix->chromatix_ASF_9x9.asf_9_9.sobel_se_diagonal_coeff[i];
    asf_info->info_1_6_x.hpf_h_coeff[i] =
      chromatix->chromatix_ASF_9x9.asf_9_9.hpf_h_coeff[i];
    asf_info->info_1_6_x.hpf_v_coeff[i] =
      chromatix->chromatix_ASF_9x9.asf_9_9.hpf_se_diagonal_coeff[i];
  }
  for (i = 0; i < 15; i++) {
    asf_info->info_1_6_x.lpf_coeff[i] =
      chromatix->chromatix_ASF_9x9.asf_9_9.lpf_coeff[i];
    asf_info->info_1_6_x.hpf_sym_coeff[i] =
      chromatix->chromatix_ASF_9x9.asf_9_9.hpf_symmetric_coeff[i];
  }
  for (i = 0; i < 6; i++) {
    asf_info->info_1_6_x.activity_lpf_coeff[i] =
      (chromatix->chromatix_ASF_9x9.asf_9_9.activity_lpf_coeff[i] & 0xff);
    asf_info->info_1_6_x.activity_bpf_coeff[i] =
      chromatix->chromatix_ASF_9x9.asf_9_9.activity_band_pass_coeff[i];
  }

  asf_info->info_1_6_x.sp_eff_en = 0;
  asf_info->info_1_6_x.neg_abs_y1 = 0;
  asf_info->info_1_6_x.activity_clamp_threshold =
    (uint8_t)chromatix->chromatix_ASF_9x9.asf_9_9.activity_clamp_threshold;
  asf_info->info_1_6_x.norm_scale =
    (uint8_t)chromatix->chromatix_ASF_9x9.asf_9_9.norm_scale;
  asf_info->info_1_6_x.max_val_threshold =
    (uint16_t)chromatix->chromatix_ASF_9x9.asf_9_9.max_value_threshold;
  asf_info->info_1_6_x.median_blend_lower_offset =
    (uint8_t)chromatix->chromatix_ASF_9x9.asf_9_9.median_blend_lower_offset;
  asf_info->info_1_6_x.median_blend_offset =
    (uint8_t)chromatix->chromatix_ASF_9x9.asf_9_9.median_blend_upper_offset;
  asf_info->info_1_6_x.perpen_scale_factor =
    (uint8_t)chromatix->chromatix_ASF_9x9.asf_9_9.perpendicular_scale_factor;
  asf_info->info_1_6_x.L2_norm_en =
    (uint8_t) chromatix->chromatix_ASF_9x9.asf_9_9.L2_norm_en;
}

void cpp_hw_params_print_asf_info_1_6_x(cpp_1_6_x_asf_info_t *asf_info __unused)
{
  CPP_ASF_LOW("sp=0x%x", asf_info->sp);
  CPP_ASF_LOW("neg_abs_y1=0x%x", asf_info->neg_abs_y1);
  CPP_ASF_LOW("dyna_clamp_en=0x%x", asf_info->dyna_clamp_en);
  CPP_ASF_LOW("sp_eff_en=0x%x", asf_info->sp_eff_en);
  CPP_ASF_LOW("nz_flag_sel=0x%x", asf_info->nz_flag_sel);
  CPP_ASF_LOW("activity_clamp_threshold=0x%x", asf_info->activity_clamp_threshold);
  CPP_ASF_LOW("perpen_scale_factor=0x%x", asf_info->perpen_scale_factor);
  CPP_ASF_LOW("gamma_cor_luma_target=0x%x", asf_info->gamma_cor_luma_target);
  CPP_ASF_LOW("max_val_threshold=0x%x", asf_info->max_val_threshold);
  CPP_ASF_LOW("gain_cap=0x%x", asf_info->gain_cap);
  CPP_ASF_LOW("median_blend_offset=0x%x", asf_info->median_blend_offset);
  CPP_ASF_LOW("median_blend_lower_offset=0x%x", asf_info->median_blend_lower_offset);
  CPP_ASF_LOW("norm_scale=0x%x", asf_info->norm_scale);
  CPP_ASF_LOW("clamp_h_ul=0x%x", asf_info->clamp_h_ul);
  CPP_ASF_LOW("clamp_h_ll=0x%x", asf_info->clamp_h_ll);
  CPP_ASF_LOW("clamp_v_ul=0x%x", asf_info->clamp_v_ul);
  CPP_ASF_LOW("clamp_v_ll=0x%x", asf_info->clamp_v_ll);
  CPP_ASF_LOW("clamp_scale_max=0x%x", asf_info->clamp_scale_max);
  CPP_ASF_LOW("clamp_scale_min=0x%x", asf_info->clamp_scale_min);
  CPP_ASF_LOW("clamp_offset_max=0x%x", asf_info->clamp_offset_max);
  CPP_ASF_LOW("clamp_offset_min=0x%x", asf_info->clamp_offset_min);
  CPP_ASF_LOW("clamp_tl_ul=0x%x", asf_info->clamp_tl_ul);
  CPP_ASF_LOW("clamp_tl_ll=0x%x", asf_info->clamp_tl_ll);
  CPP_ASF_LOW("nz_flag=0x%x", asf_info->nz_flag);
  CPP_ASF_LOW("nz_flag_2=0x%x", asf_info->nz_flag_2);
  CPP_ASF_LOW("nz_flag_3_5=0x%x", asf_info->nz_flag_3_5);
}

/* 9x9 ASF parameter update */
int32_t cpp_hw_params_asf_interpolate_1_6_x(cpp_hardware_params_t *hw_params,
  modulesChromatix_t *module_chromatix __unused,
  cpp_params_aec_trigger_info_t *trigger_input )
{
  int32_t rc = 0;

#if !defined (CHROMATIX_304) && !defined (CHROMATIX_306) &&\
 !defined (CHROMATIX_307) && !defined (CHROMATIX_308) &&\
 !defined (CHROMATIX_308E) && !defined (CHROMATIX_309)&&\
 !defined (CHROMATIX_310) && !defined (CHROMATIX_310E)
  /* Using Default params for testing */
  CPP_ASF_HIGH("chromatix 303 not available, using default ASF params");
  cpp_hw_params_fill_default_asf_1_6_x(&(hw_params->asf_info));
#else

  chromatix_cpp_type *chromatix_cpp = NULL;
  chromatix_cpp_stripped_type *chromatix_cpp_stripped = NULL;

  if (!module_chromatix) {
    CPP_ASF_ERR("failed: chromatix NULL\n");
    return -EINVAL;
  }

  /* when stripped chromatix is used, no interpolation is required */
  if (!module_chromatix->chromatixCppPtr) {
    chromatix_cpp_stripped = hw_params->def_chromatix;
    if (!chromatix_cpp_stripped) {
      CPP_ASF_LOW("no default chromatix set");
      cpp_hw_params_fill_default_asf_1_6_x(&(hw_params->asf_info));
      return -1;
    }
    cpp_hw_params_fill_stripped_asf_1_6_x(&hw_params->asf_info,
      chromatix_cpp_stripped);
    cpp_hw_params_print_asf_info_1_6_x(&(hw_params->asf_info.info_1_6_x));
    return 0;
  }

  chromatix_cpp = (chromatix_cpp_type *)module_chromatix->chromatixCppPtr;

  rc = cpp_hw_params_update_asf_params_1_6_x(hw_params, chromatix_cpp,
    trigger_input);
#endif
  cpp_hw_params_print_asf_info_1_6_x(&(hw_params->asf_info.info_1_6_x));
  return rc;
}

int32_t cpp_hw_params_asf_interpolate(cpp_hardware_t *cpphw,
  cpp_hardware_params_t *hw_params,
  modulesChromatix_t *module_chromatix,
  cpp_params_aec_trigger_info_t *trigger_input)
{
  if (!cpphw || !hw_params || !module_chromatix || !trigger_input) {
    CPP_ASF_ERR("failed,cpphw=%p, hw_params=%p, chromatix_ptr=%p, trigger_input=%p",
      cpphw, hw_params, module_chromatix, trigger_input);
    return -EINVAL;
  }
  cpp_firmware_version_t fw_version =
    cpp_hardware_get_fw_version(cpphw);
  CPP_ASF_LOW("fw_version = %x", fw_version);
  switch (fw_version) {
  case CPP_FW_VERSION_1_2_0:
  case CPP_FW_VERSION_1_4_0:
    return cpp_hw_params_asf_interpolate_1_2_x(hw_params,
             module_chromatix, trigger_input);
  case CPP_FW_VERSION_1_5_0:
  case CPP_FW_VERSION_1_5_1:
  case CPP_FW_VERSION_1_5_2:
    return cpp_hw_params_asf_interpolate_1_2_x(hw_params,
             module_chromatix, trigger_input);
  case CPP_FW_VERSION_1_8_0:
  case CPP_FW_VERSION_1_6_0:
  case CPP_FW_VERSION_1_10_0:
     memset(&hw_params->asf_info, 0x00, sizeof(cpp_params_asf_info_t));
    return cpp_hw_params_asf_interpolate_1_6_x(hw_params,
             module_chromatix, trigger_input);
  case CPP_FW_VERSION_1_12_0:
  case CPP_FW_VERSION_1_12_1:
    memset(&hw_params->asf_info, 0x00, sizeof(cpp_params_asf_info_t));
    hw_params->asf_info.info_1_6_x.cpp_1_12_ext.is_valid = TRUE;
    return cpp_hw_params_asf_interpolate_1_6_x(hw_params,
             module_chromatix, trigger_input);
  default:
    CPP_ASF_ERR("failed, unsupported fw version: 0x%x", fw_version);
    return -EINVAL;
  }
  return 0;
}

int32_t cpp_hw_params_init_wnr_params(cpp_hardware_params_t *cpp_hw_params)
{
  uint32_t j, k;
  uint32_t profile = 8;

  if (!cpp_hw_params) {
    CPP_DENOISE_ERR("null hw_params, failed");
    return -EINVAL;
  }

  for (k = 0; k < CPP_DENOISE_NUM_PROFILES; k++) {
    for (j = 0; j < CPP_DENOISE_NUM_PLANES; j++) {
      cpp_hw_params->denoise_info[j][k].noise_profile = profile;
      cpp_hw_params->denoise_info[j][k].weight = 0;
      cpp_hw_params->denoise_info[j][k].edge_softness = 12.75;
      cpp_hw_params->denoise_info[j][k].denoise_ratio = 12.75;
    }
    /* Update profile value for next level */
    if (profile > 1) {
      profile /= 2;
    }
  }

  return 0;
}

int32_t cpp_hw_params_interpolate_wnr_params(float interpolation_factor,
  cpp_hardware_params_t *cpp_hw_params, float effects_factor,
  void *ref_noise_profile_i,
  void *ref_noise_profile_iplus1,
  void *p_WNR_data,
  void *module_chromatix)
{
  uint32_t k, j, offset;

  if (!cpp_hw_params || !ref_noise_profile_i || !ref_noise_profile_iplus1) {
    CPP_DENOISE_ERR("failed hw_params %p, ref_noise_profile_i %p, ref_noise_profile_iplus1 %p",
      cpp_hw_params, ref_noise_profile_i, ref_noise_profile_iplus1);
    return -EINVAL;
  }

  for (k = 0; k < WNR_LAYERS; k++) {

    /* Weight factor */
    cpp_hw_params->denoise_info[0][k].weight =
      LINEAR_INTERPOLATE(interpolation_factor,
        WNR_WEIGHT_Y_DATA(ref_noise_profile_i,k),
        WNR_WEIGHT_Y_DATA(ref_noise_profile_iplus1,k)) * effects_factor;
    cpp_hw_params->denoise_info[1][k].weight =
      LINEAR_INTERPOLATE(interpolation_factor,
        WNR_WEIGHT_CHROMA_DATA(ref_noise_profile_i,k),
        WNR_WEIGHT_CHROMA_DATA(ref_noise_profile_iplus1,k));
    cpp_hw_params->denoise_info[2][k].weight =
      LINEAR_INTERPOLATE(interpolation_factor,
        WNR_WEIGHT_CHROMA_DATA(ref_noise_profile_i,k),
        WNR_WEIGHT_CHROMA_DATA(ref_noise_profile_iplus1,k));

    /* denoise ratio */
    cpp_hw_params->denoise_info[0][k].denoise_ratio =
      LINEAR_INTERPOLATE(interpolation_factor,
        WNR_SCALE_Y_DATA(ref_noise_profile_i, k),
        WNR_SCALE_Y_DATA(ref_noise_profile_iplus1, k));
    cpp_hw_params->denoise_info[1][k].denoise_ratio =
      LINEAR_INTERPOLATE(interpolation_factor,
        WNR_SCALE_CHROMA_DATA(ref_noise_profile_i, k),
        WNR_SCALE_CHROMA_DATA(ref_noise_profile_iplus1, k));
    cpp_hw_params->denoise_info[2][k].denoise_ratio =
      LINEAR_INTERPOLATE(interpolation_factor,
        WNR_SCALE_CHROMA_DATA(ref_noise_profile_i, k),
        WNR_SCALE_CHROMA_DATA(ref_noise_profile_iplus1, k));
  }

  for (k = 0; k < BILITERAL_LAYERS; k++) {
    /* edge softness factor */
    cpp_hw_params->denoise_info[0][k].edge_softness =
      LINEAR_INTERPOLATE(interpolation_factor,
        WNR_EDGE_SOFTNESS_Y_DATA(ref_noise_profile_i, k),
        WNR_EDGE_SOFTNESS_Y_DATA(ref_noise_profile_iplus1, k));
    cpp_hw_params->denoise_info[1][k].edge_softness =
      LINEAR_INTERPOLATE(interpolation_factor,
        WNR_EDGE_SOFTNESS_CHROMA_DATA(ref_noise_profile_i, k),
        WNR_EDGE_SOFTNESS_CHROMA_DATA(ref_noise_profile_iplus1, k));
    cpp_hw_params->denoise_info[2][k].edge_softness =
      LINEAR_INTERPOLATE(interpolation_factor,
        WNR_EDGE_SOFTNESS_CHROMA_DATA(ref_noise_profile_i, k),
        WNR_EDGE_SOFTNESS_CHROMA_DATA(ref_noise_profile_iplus1, k));
  }

#if ((defined(CHROMATIX_307) || defined(CHROMATIX_306) || \
  defined (CHROMATIX_308) || defined (CHROMATIX_308E) || \
  defined (CHROMATIX_309) || defined (CHROMATIX_310) || defined (CHROMATIX_310E)) && \
  defined(CAMERA_USE_CHROMATIX_HW_WNR_TYPE))
  for (k = 0; k < WNR_LAYERS; k++) {
    cpp_hw_params->denoise_info[0][k].noise_profile =
      LINEAR_INTERPOLATE(interpolation_factor,
        WNR_NOISE_PROFILE_Y_DATA(ref_noise_profile_i, k),
        WNR_NOISE_PROFILE_Y_DATA(ref_noise_profile_iplus1, k));
    cpp_hw_params->denoise_info[1][k].noise_profile =
      LINEAR_INTERPOLATE(interpolation_factor,
        WNR_NOISE_PROFILE_CB_DATA(ref_noise_profile_i, k),
        WNR_NOISE_PROFILE_CB_DATA(ref_noise_profile_iplus1, k));
    cpp_hw_params->denoise_info[2][k].noise_profile =
      LINEAR_INTERPOLATE(interpolation_factor,
        WNR_NOISE_PROFILE_CR_DATA(ref_noise_profile_i, k),
        WNR_NOISE_PROFILE_CR_DATA(ref_noise_profile_iplus1, k));
  }
#else
  for (k = 0; k < WNR_LAYERS; k++) {
    for (j = 0; j < CPP_DENOISE_NUM_PLANES; j++) {
      offset = k + 4 + j * 8;
      cpp_hw_params->denoise_info[j][k].noise_profile =
        LINEAR_INTERPOLATE(interpolation_factor,
          WNR_NOISE_PROFILE_DATA(ref_noise_profile_i, offset),
          WNR_NOISE_PROFILE_DATA(ref_noise_profile_iplus1, offset));
    }
  }
  cpp_hw_params->denoise_info[1][4].weight =
    LINEAR_INTERPOLATE(interpolation_factor,
      WNR_WEIGHT_5TH_CHROMA_DATA(ref_noise_profile_i),
      WNR_WEIGHT_5TH_CHROMA_DATA(ref_noise_profile_iplus1));
  cpp_hw_params->denoise_info[2][4].weight =
    LINEAR_INTERPOLATE(interpolation_factor,
      WNR_WEIGHT_5TH_CHROMA_DATA(ref_noise_profile_i),
      WNR_WEIGHT_5TH_CHROMA_DATA(ref_noise_profile_iplus1));

  cpp_hw_params->denoise_info[1][4].denoise_ratio =
    LINEAR_INTERPOLATE(interpolation_factor,
      WNR_SCALE_5TH_CHROMA_DATA(ref_noise_profile_i),
      WNR_SCALE_5TH_CHROMA_DATA(ref_noise_profile_iplus1));
  cpp_hw_params->denoise_info[2][4].denoise_ratio =
    LINEAR_INTERPOLATE(interpolation_factor,
      WNR_SCALE_5TH_CHROMA_DATA(ref_noise_profile_i),
      WNR_SCALE_5TH_CHROMA_DATA(ref_noise_profile_iplus1));

  cpp_hw_params->denoise_info[1][4].noise_profile =
    cpp_hw_params->denoise_info[1][3].noise_profile / 2;
  cpp_hw_params->denoise_info[2][4].noise_profile =
    cpp_hw_params->denoise_info[2][3].noise_profile / 2;
#endif

#ifdef __CPP_PARAM_DEBUG__
  cpp_wnr_params_dbg_chromatix_np(ref_noise_profile_i, "start", cpp_hw_params);
  if (ref_noise_profile_i != ref_noise_profile_iplus1) {
    cpp_wnr_params_dbg_chromatix_np(ref_noise_profile_iplus1, "end",
      cpp_hw_params);
  }
#endif

  if (IS_CPP_ERROR(cpp_wnr_params_rnr_update(
    cpp_hw_params, ref_noise_profile_i,
    ref_noise_profile_iplus1,
    p_WNR_data,
    interpolation_factor,
    module_chromatix))) {
    CPP_ERR("Denoise RNR failed");
  }

  if (IS_CPP_ERROR(cpp_wnr_params_lnr_update(
    cpp_hw_params, ref_noise_profile_i,
    ref_noise_profile_iplus1,
    interpolation_factor,
    module_chromatix))) {
    CPP_ERR("Denoise LNR failed");
  }

#ifdef __CPP_PARAM_DEBUG__
  cpp_wnr_params_dbg_output(cpp_hw_params);
#endif

  return 0;
}

void cpp_hw_params_fill_wnr_from_chromatix_stripped(
   cpp_hardware_params_t *hw_params,
   chromatix_cpp_stripped_type *chromatix)
{
  uint32_t k, j, offset;

  CPP_DENOISE_HIGH("using stripped wnr");

  ReferenceNoiseProfile_type *ref_noise_profile =
    &(chromatix->chromatix_wavelet.wavelet_denoise_HW_420.noise_profile);

  for (k = 0; k < WAVELET_LEVEL; k++) {
    for (j = 0; j < CPP_DENOISE_NUM_PLANES; j++) {
      offset = k + 4 + j * 8;
      hw_params->denoise_info[j][k].noise_profile =
        ref_noise_profile->referenceNoiseProfileData[offset];
    }

    /* Weight factor */
    hw_params->denoise_info[0][k].weight =
      ref_noise_profile->denoise_weight_y[k];
    hw_params->denoise_info[1][k].weight =
      ref_noise_profile->denoise_weight_chroma[k];
    hw_params->denoise_info[2][k].weight =
      ref_noise_profile->denoise_weight_chroma[k];

    /* edge softness factor */
    hw_params->denoise_info[0][k].edge_softness =
      ref_noise_profile->denoise_edge_softness_y[k];
    hw_params->denoise_info[1][k].edge_softness =
      ref_noise_profile->denoise_edge_softness_chroma[k];
    hw_params->denoise_info[2][k].edge_softness =
      ref_noise_profile->denoise_edge_softness_chroma[k];

    /* denoise ratio */
    hw_params->denoise_info[0][k].denoise_ratio =
      ref_noise_profile->denoise_scale_y[k];
    hw_params->denoise_info[1][k].denoise_ratio =
      ref_noise_profile->denoise_scale_chroma[k];
    hw_params->denoise_info[2][k].denoise_ratio =
      ref_noise_profile->denoise_scale_chroma[k];
  }

  hw_params->denoise_info[1][4].weight =
      ref_noise_profile->denoise_weight_chroma_5th,
  hw_params->denoise_info[2][4].weight =
      ref_noise_profile->denoise_weight_chroma_5th;

  hw_params->denoise_info[1][4].denoise_ratio =
      ref_noise_profile->denoise_scale_chroma_5th;
  hw_params->denoise_info[2][4].denoise_ratio =
    ref_noise_profile->denoise_scale_chroma_5th;

  hw_params->denoise_info[1][4].noise_profile =
    hw_params->denoise_info[1][3].noise_profile / 2;
  hw_params->denoise_info[2][4].noise_profile =
    hw_params->denoise_info[2][3].noise_profile / 2;
}

/** cpp_hw_params_get_downscale_ratio:
 *
 *  @wavelet_chromatix_data - Pointer to the wavelet chromatix data. It holds
 *     scale adjustment's regions and the other TNR parameters.
 *  @scale_ratio - Scale ration calculated between ISP input dimension and CPP
 *     output dimension.
 *  @reg_strt_idx_start - Scale start index .
 *  @reg_strt_idx_end - Scale end index
 *  @reg_strt_interp_ratio - Scale interpolation ratio.
 *  @scalor - Represents the type of scalor (ISP / CPP).
 *
 *  This function finds a downscale region in the chromatix header according to
 *  the scale ratio and calculates the scale interpolation ratio.
 *
 **/
static void cpp_hw_params_get_downscale_ratio(void *wavelet_downscale_data,
  float scale_ratio, uint32_t *reg_strt_idx_start, uint32_t *reg_strt_idx_end,
  float *reg_strt_interp_ratio, uint32_t scalor __unused)
{
  uint32_t i ;
  uint32_t scale_idx_start ;
  uint32_t scale_idx_end ;
  float scale_strt;
  float scale_end;

  *reg_strt_interp_ratio = 0.0f ;

  for ( i = 0 ; i < DOWN_SCALING_LEVELS ; i ++ ) {
    scale_idx_start = i ;
    scale_idx_end = i+1 ;

    if ( i == ( DOWN_SCALING_LEVELS - 1 )) {
      scale_idx_start = DOWN_SCALING_LEVELS-1 ;
      scale_idx_end = DOWN_SCALING_LEVELS-1 ;
      break ;
    }
    scale_strt = GET_DOWNSCALE_LEVELS(wavelet_downscale_data, scale_idx_start, scalor);
    scale_end  = GET_DOWNSCALE_LEVELS(wavelet_downscale_data, scale_idx_end, scalor);

    if ( scale_strt >= scale_end ) {
      CPP_DENOISE_LOW("WNR Down Scaling Ratio Settings are not in correct order");
      *reg_strt_idx_start = i;
      *reg_strt_idx_end = i;
      return;
    }

    if ((scale_ratio >= scale_strt) && (scale_ratio < scale_end)) {
      *reg_strt_interp_ratio = (float)(scale_ratio - scale_strt)/
        (float)(scale_end -scale_strt);
      break ;
    } else if ((scale_ratio < scale_strt)) {
      scale_idx_end = i ;
      break ;
    }
  }
  *reg_strt_idx_start = scale_idx_start ;
  *reg_strt_idx_end = scale_idx_end ;
}

/** cpp_hw_params_get_upscale_ratio:
 *
 *  @wavelet_chromatix_data - Pointer to the wavelet chromatix data. It holds
 *     scale adjustment's regions and the other TNR parameters.
 *  @scale_ratio - Scale ration calculated between ISP input dimension and CPP
 *     output dimension.
 *  @reg_strt_idx_start - Scale start index .
 *  @reg_strt_idx_end - Scale end index
 *  @reg_strt_interp_ratio - Scale interpolation ratio.
 *  @scalor - Represents the type of scalor (ISP / CPP).
 *
 *  This function finds a upscale region in the chromatix header according to
 *  the scale ratio and calculates the scale interpolation ratio.
 *
 **/
static void cpp_hw_params_get_upscale_ratio( void *wavelet_upscale_data,
  float scale_ratio, uint32_t *reg_strt_idx_start, uint32_t *reg_strt_idx_end,
  float *reg_strt_interp_ratio, uint32_t scalor __unused)
{
  uint32_t i ;
  uint32_t scale_idx_start ;
  uint32_t scale_idx_end ;
  float scale_strt;
  float scale_end;

  *reg_strt_interp_ratio = 0.0f ;
  for ( i = 0 ; i < UP_SCALING_LEVELS ; i ++ ) {
    scale_idx_start = i ;
    scale_idx_end = i+1 ;

    if ( i == ( UP_SCALING_LEVELS - 1 )) {
      scale_idx_start = UP_SCALING_LEVELS-1 ;
      scale_idx_end = UP_SCALING_LEVELS-1 ;
      break ;
    }
    scale_strt = GET_UPSCALE_LEVELS(wavelet_upscale_data, scale_idx_start);
    scale_end  = GET_UPSCALE_LEVELS(wavelet_upscale_data, scale_idx_end);

    if ( scale_strt <= scale_end ) {
      CPP_DENOISE_LOW("WNR Up Scaling Ratio Settings are not in correct order");
      *reg_strt_idx_start = i;
      *reg_strt_idx_end = i;
      return;
    }

    if ((scale_ratio < scale_strt) && (scale_ratio >scale_end)) {
      *reg_strt_interp_ratio = (float)(scale_ratio - scale_strt)/
        (float)(scale_end -scale_strt);
      break ;
    } else if (scale_ratio >= scale_strt) {
      scale_idx_end = i ;
      break ;
    }
  }
  *reg_strt_idx_start = scale_idx_start ;
  *reg_strt_idx_end = scale_idx_end ;
}

/** cpp_hw_params_interpolate_zoom_scale:
 *
 *  @hw_params - hardware parameters of the current stream
 *  @wavelet_chromatix_data - Pointer to the wavelet chromatix data. It holds
 *     scale adjustment's regions and the other TNR parameters.
 *  @scale_ratio - Scale ration calculated between ISP input dimension and CPP
 *     output dimension.
 *  @reg_strt_idx_start - Scale start index .
 *  @reg_strt_idx_end - Scale end index
 *  @reg_strt_interp_ratio - Scale interpolation ratio.
 *
 *  This function interpolates adjustment scale parameters from selected regions.
 *  Interpolation depends on the scale ratio. Upscale and downscale regions are
 *  different. If there is no scale the scale adjustment values are 1.0f.
 *
 **/
void cpp_hw_params_interpolate_zoom_scale(cpp_hardware_params_t *hw_params,
  void *downscale_data, void *upscale_data, float scale_ratio,
  uint32_t reg_strt_idx_start, uint32_t reg_strt_idx_end,
  float reg_strt_interp_ratio, uint32_t scalor)
{

  uint32_t k, j;

  for (k = 0; k < CPP_DENOISE_NUM_PROFILES; k++) {
    for (j = 0; j < CPP_DENOISE_NUM_PROFILES; j++) {
      if (scale_ratio > 1.0f ) {
        if (j < GET_SCALE_COUNT()) {
          hw_params->denoise_info[0][k].profile_adj[scalor][j] =
            LINEAR_INTERPOLATE(reg_strt_interp_ratio,
              DOWN_PROFILE_Y_ADJ(downscale_data, reg_strt_idx_start, j, scalor),
              DOWN_PROFILE_Y_ADJ(downscale_data, reg_strt_idx_end, j, scalor));
          hw_params->denoise_info[1][k].profile_adj[scalor][j] =
            LINEAR_INTERPOLATE(reg_strt_interp_ratio,
              DOWN_PROFILE_CBCR_ADJ(downscale_data, reg_strt_idx_start, j, scalor),
              DOWN_PROFILE_CBCR_ADJ(downscale_data, reg_strt_idx_end, j, scalor));
          hw_params->denoise_info[2][k].profile_adj[scalor][j] =
            LINEAR_INTERPOLATE(reg_strt_interp_ratio,
              DOWN_PROFILE_CBCR_ADJ(downscale_data, reg_strt_idx_start, j, scalor),
              DOWN_PROFILE_CBCR_ADJ(downscale_data, reg_strt_idx_end, j, scalor));

          /* interpolate denoise_scale_y */
          hw_params->denoise_info[0][k].weight_VFE_adj[scalor][j] =
            LINEAR_INTERPOLATE(reg_strt_interp_ratio,
              DOWN_WEIGHT_Y_ADJ(downscale_data, reg_strt_idx_start, j, scalor),
              DOWN_WEIGHT_Y_ADJ(downscale_data, reg_strt_idx_end, j, scalor));
          hw_params->denoise_info[1][k].weight_VFE_adj[scalor][j] =
            LINEAR_INTERPOLATE(reg_strt_interp_ratio,
              DOWN_WEIGHT_CBCR_ADJ(downscale_data, reg_strt_idx_start, j, scalor),
              DOWN_WEIGHT_CBCR_ADJ(downscale_data, reg_strt_idx_end, j, scalor));
          hw_params->denoise_info[2][k].weight_VFE_adj[scalor][j] =
            LINEAR_INTERPOLATE(reg_strt_interp_ratio,
              DOWN_WEIGHT_CBCR_ADJ(downscale_data, reg_strt_idx_start, j, scalor),
              DOWN_WEIGHT_CBCR_ADJ(downscale_data, reg_strt_idx_end, j, scalor));
        } else {
          hw_params->denoise_info[0][k].profile_adj[scalor][j] =
            hw_params->denoise_info[0][k].profile_adj[scalor][j - 1];
          hw_params->denoise_info[1][k].profile_adj[scalor][j] =
            hw_params->denoise_info[1][k].profile_adj[scalor][j - 1];
          hw_params->denoise_info[2][k].profile_adj[scalor][j] =
            hw_params->denoise_info[2][k].profile_adj[scalor][j - 1];
          hw_params->denoise_info[0][k].weight_VFE_adj[scalor][j] =
            hw_params->denoise_info[0][k].weight_VFE_adj[scalor][j - 1];
          hw_params->denoise_info[1][k].weight_VFE_adj[scalor][j] =
            hw_params->denoise_info[1][k].weight_VFE_adj[scalor][j - 1];
          hw_params->denoise_info[2][k].weight_VFE_adj[scalor][j] =
            hw_params->denoise_info[2][k].weight_VFE_adj[scalor][j - 1];
        }

      } else if ( scale_ratio < 1.0f ) {
        if (j < GET_SCALE_COUNT()) {
          hw_params->denoise_info[0][k].profile_adj[scalor][j] =
            LINEAR_INTERPOLATE(reg_strt_interp_ratio,
              UP_PROFILE_Y_ADJ(upscale_data, reg_strt_idx_start, j),
              UP_PROFILE_Y_ADJ(upscale_data, reg_strt_idx_end, j));
          hw_params->denoise_info[1][k].profile_adj[scalor][j] =
            LINEAR_INTERPOLATE(reg_strt_interp_ratio,
              UP_PROFILE_CBCR_ADJ(upscale_data, reg_strt_idx_start, j),
              UP_PROFILE_CBCR_ADJ(upscale_data, reg_strt_idx_end, j));
          hw_params->denoise_info[2][k].profile_adj[scalor][j] =
            LINEAR_INTERPOLATE(reg_strt_interp_ratio,
              UP_PROFILE_CBCR_ADJ(upscale_data, reg_strt_idx_start, j),
              UP_PROFILE_CBCR_ADJ(upscale_data, reg_strt_idx_end, j));
          /* interpolate denoise_scale_y */
          hw_params->denoise_info[0][k].weight_VFE_adj[scalor][j] =
            LINEAR_INTERPOLATE(reg_strt_interp_ratio,
              UP_WEIGHT_Y_ADJ(upscale_data, reg_strt_idx_start, j),
              UP_WEIGHT_Y_ADJ(upscale_data, reg_strt_idx_end, j));
          hw_params->denoise_info[1][k].weight_VFE_adj[scalor][j] =
            LINEAR_INTERPOLATE(reg_strt_interp_ratio,
              UP_WEIGHT_CBCR_ADJ(upscale_data, reg_strt_idx_start, j),
              UP_WEIGHT_CBCR_ADJ(upscale_data, reg_strt_idx_end, j));
          hw_params->denoise_info[2][k].weight_VFE_adj[scalor][j] =
            LINEAR_INTERPOLATE(reg_strt_interp_ratio,
              UP_WEIGHT_CBCR_ADJ(upscale_data, reg_strt_idx_start, j),
              UP_WEIGHT_CBCR_ADJ(upscale_data, reg_strt_idx_end, j));
        } else {
          hw_params->denoise_info[0][k].profile_adj[scalor][j] =
            hw_params->denoise_info[0][k].profile_adj[scalor][j - 1];
          hw_params->denoise_info[1][k].profile_adj[scalor][j] =
            hw_params->denoise_info[1][k].profile_adj[scalor][j - 1];
          hw_params->denoise_info[2][k].profile_adj[scalor][j] =
            hw_params->denoise_info[2][k].profile_adj[scalor][j - 1];
          hw_params->denoise_info[0][k].weight_VFE_adj[scalor][j] =
            hw_params->denoise_info[0][k].weight_VFE_adj[scalor][j - 1];
          hw_params->denoise_info[1][k].weight_VFE_adj[scalor][j] =
            hw_params->denoise_info[1][k].weight_VFE_adj[scalor][j - 1];
          hw_params->denoise_info[2][k].weight_VFE_adj[scalor][j] =
            hw_params->denoise_info[2][k].weight_VFE_adj[scalor][j -1];
        }

      } else {
        /* No scale */
        hw_params->denoise_info[0][k].profile_adj[scalor][j] = 1.0f;
        hw_params->denoise_info[1][k].profile_adj[scalor][j] = 1.0f;
        hw_params->denoise_info[2][k].profile_adj[scalor][j] = 1.0f;
        hw_params->denoise_info[0][k].weight_VFE_adj[scalor][j] =1.0f;
        hw_params->denoise_info[1][k].weight_VFE_adj[scalor][j] =1.0f;
        hw_params->denoise_info[2][k].weight_VFE_adj[scalor][j] =1.0f;
      }
    }
  }
}

int32_t cpp_hw_params_update_wnr_scalor_adj_params(
  modulesChromatix_t *module_chromatix,cpp_hardware_params_t *hw_params,
  cpp_params_aec_trigger_info_t *aec_trigger __unused)
{
  uint32_t                    reg_strt_idx_start;
  uint32_t                    reg_strt_idx_end;
  float                       reg_strt_interp_ratio = 0.0f;
  void                        *upscale_data;
  void                        *downscale_data;
  chromatix_cpp_type *        chromatix_cpp = NULL;

  if (!hw_params) {
    CPP_DENOISE_ERR("null hw params, failed");
    return -EINVAL;
  }

  if (!module_chromatix) {
    CPP_DENOISE_ERR("null module_chromatix, failed");
    return -1;
  }
  chromatix_cpp = (chromatix_cpp_type *)module_chromatix->chromatixCppPtr;
  if (!chromatix_cpp) {
    CPP_DENOISE_ERR("failed: No CPP chromatix!!");
    return -1;
  }

  GET_WNR_UPSCALE_DATA(chromatix_cpp, hw_params, upscale_data);
  GET_WNR_DOWNSCALE_DATA(chromatix_cpp, hw_params, downscale_data);

  if ( hw_params->isp_scale_ratio > 1.0f ) {
    cpp_hw_params_get_downscale_ratio(downscale_data,
      hw_params->isp_scale_ratio, &reg_strt_idx_start, &reg_strt_idx_end,
      &reg_strt_interp_ratio, ISP_SCALOR);
  } else if ( hw_params->isp_scale_ratio < 1.0f ) {
    cpp_hw_params_get_upscale_ratio(upscale_data, hw_params->isp_scale_ratio,
      &reg_strt_idx_start, &reg_strt_idx_end, &reg_strt_interp_ratio, ISP_SCALOR);
  } else {
    cpp_hw_params_get_upscale_ratio(upscale_data, hw_params->isp_scale_ratio,
      &reg_strt_idx_start, &reg_strt_idx_end, &reg_strt_interp_ratio, ISP_SCALOR);
  }
  cpp_hw_params_interpolate_zoom_scale(hw_params,downscale_data,
    upscale_data, hw_params->isp_scale_ratio, reg_strt_idx_start,
    reg_strt_idx_end, reg_strt_interp_ratio,ISP_SCALOR);

  reg_strt_idx_start = 0;
  reg_strt_idx_end = 0;
  reg_strt_interp_ratio = 0;

  if ( hw_params->cpp_scale_ratio > 1.0f ) {
    cpp_hw_params_get_downscale_ratio(downscale_data,
      hw_params->cpp_scale_ratio, &reg_strt_idx_start, &reg_strt_idx_end,
      &reg_strt_interp_ratio, CPP_SCALOR);
  } else if ( hw_params->cpp_scale_ratio < 1.0f ) {
    cpp_hw_params_get_upscale_ratio(upscale_data, hw_params->cpp_scale_ratio,
      &reg_strt_idx_start, &reg_strt_idx_end, &reg_strt_interp_ratio, CPP_SCALOR);
  } else{
    cpp_hw_params_get_upscale_ratio(upscale_data, hw_params->cpp_scale_ratio,
      &reg_strt_idx_start, &reg_strt_idx_end, &reg_strt_interp_ratio, CPP_SCALOR);
  }

  cpp_hw_params_interpolate_zoom_scale(hw_params, downscale_data,
    upscale_data, hw_params->cpp_scale_ratio , reg_strt_idx_start,
    reg_strt_idx_end, reg_strt_interp_ratio,CPP_SCALOR);

  return 0;

}

void cpp_hw_params_interpolate_wnr_adj(float interpolation_factor,
  cpp_hardware_params_t *hw_params, void *ref_noise_profile_i,
  void *ref_noise_profile_iplus1, uint32_t scalor)
{

  uint32_t k, j;

  for (k = 0; k < CPP_DENOISE_NUM_PROFILES; k++) {
    for (j = 0; j < CPP_DENOISE_NUM_PROFILES; j++) {
      if (j < GET_WNR_ADJ_COUNT()) {
        hw_params->denoise_info[0][k].profile_adj[scalor][j] *=
          LINEAR_INTERPOLATE(interpolation_factor,
            GET_WNR_ADJ_DATA_PROFILE_Y_ADJ(ref_noise_profile_i, j),
            GET_WNR_ADJ_DATA_PROFILE_Y_ADJ(ref_noise_profile_iplus1, j));
        CPP_DENOISE_DBG("y_adj = %f %f value = %f",
          GET_WNR_ADJ_DATA_PROFILE_Y_ADJ(ref_noise_profile_i, j),
          GET_WNR_ADJ_DATA_PROFILE_Y_ADJ(ref_noise_profile_iplus1, j),
          hw_params->denoise_info[0][k].profile_adj[scalor][j]);
        hw_params->denoise_info[1][k].profile_adj[scalor][j] *=
          LINEAR_INTERPOLATE(interpolation_factor,
            GET_WNR_ADJ_DATA_PROFILE_CBCR_ADJ(ref_noise_profile_i, j),
            GET_WNR_ADJ_DATA_PROFILE_CBCR_ADJ(ref_noise_profile_iplus1, j));
        hw_params->denoise_info[2][k].profile_adj[scalor][j] *=
          LINEAR_INTERPOLATE(interpolation_factor,
            GET_WNR_ADJ_DATA_PROFILE_CBCR_ADJ(ref_noise_profile_i, j),
            GET_WNR_ADJ_DATA_PROFILE_CBCR_ADJ(ref_noise_profile_iplus1, j));
        /* interpolate denoise_scale_y */
        hw_params->denoise_info[0][k].weight_VFE_adj[scalor][j] *=
          LINEAR_INTERPOLATE(interpolation_factor,
            GET_WNR_ADJ_DATA_WEIGHT_Y_ADJ(ref_noise_profile_i, j),
            GET_WNR_ADJ_DATA_WEIGHT_Y_ADJ(ref_noise_profile_iplus1, j));
        hw_params->denoise_info[1][k].weight_VFE_adj[scalor][j] *=
          LINEAR_INTERPOLATE(interpolation_factor,
            GET_WNR_ADJ_DATA_WEIGHT_CBCR_ADJ(ref_noise_profile_i, j),
            GET_WNR_ADJ_DATA_WEIGHT_CBCR_ADJ(ref_noise_profile_iplus1, j));
        hw_params->denoise_info[2][k].weight_VFE_adj[scalor][j] *=
          LINEAR_INTERPOLATE(interpolation_factor,
            GET_WNR_ADJ_DATA_WEIGHT_CBCR_ADJ(ref_noise_profile_i, j),
            GET_WNR_ADJ_DATA_WEIGHT_CBCR_ADJ(ref_noise_profile_iplus1, j));
      } else {
        hw_params->denoise_info[0][k].profile_adj[scalor][j] =
          hw_params->denoise_info[0][k].profile_adj[scalor][j - 1];
        hw_params->denoise_info[1][k].profile_adj[scalor][j] =
          hw_params->denoise_info[1][k].profile_adj[scalor][j - 1];
        hw_params->denoise_info[2][k].profile_adj[scalor][j] =
          hw_params->denoise_info[2][k].profile_adj[scalor][j - 1];
        hw_params->denoise_info[0][k].weight_VFE_adj[scalor][j] =
          hw_params->denoise_info[0][k].weight_VFE_adj[scalor][j - 1];
        hw_params->denoise_info[1][k].weight_VFE_adj[scalor][j] =
          hw_params->denoise_info[1][k].weight_VFE_adj[scalor][j - 1];
        hw_params->denoise_info[2][k].weight_VFE_adj[scalor][j] =
          hw_params->denoise_info[2][k].weight_VFE_adj[scalor][j -1];
      }
    }
  }
}


int32_t cpp_hw_params_update_wnr_hdr_adj_params(
  modulesChromatix_t *module_chromatix,cpp_hardware_params_t *hw_params,
  cpp_params_aec_trigger_info_t *aec_trigger)
{
  uint32_t                    i, end_idx;
  float                       trigger_start = 0, trigger_end = 0;
  float                       interpolation_factor;
  float                       numerator = 0, denominator = 0;
  float                       trigger_input = 0;
  chromatix_cpp_type *        chromatix_cpp = NULL;
  Chromatix_HDR_WNR_adj_type  *hdr_wnr_adj_type_ptr = NULL;
  void *ref_noise_profile_i = NULL;
  void *ref_noise_profile_iplus1 = NULL;

  if (!hw_params) {
    CPP_DENOISE_ERR("null hw_params, failed");
    return -EINVAL;
  }

  if (!module_chromatix) {
    CPP_DENOISE_ERR("module_chromatix null, failed");
    return -1;
  }
  chromatix_cpp = (chromatix_cpp_type *)module_chromatix->chromatixCppPtr;
  if (!chromatix_cpp) {
    CPP_DENOISE_ERR("failed: No CPP chromatix!!");
    return -1;
  }
  GET_HDR_WNR_ADJ_TYPE_PTR(chromatix_cpp, hw_params, hdr_wnr_adj_type_ptr);

  if(GET_HDR_WNR_ADJ_CONTROL_TYPE(hdr_wnr_adj_type_ptr) == CONTROL_AEC_EXP_SENSITIVITY_RATIO){
    //CONTROL_AEC_EXP_SENSITIVITY_RATIO
    trigger_input = aec_trigger->aec_sensitivity_ratio;
    CPP_DENOISE_DBG("zzhdr exp sens ratio trigger %f ", trigger_input);
  } else if(GET_HDR_WNR_ADJ_CONTROL_TYPE(hdr_wnr_adj_type_ptr) == CONTROL_EXP_TIME_RATIO) {
    //CONTROL_EXP_TIME_RATIO
    trigger_input = aec_trigger->exp_time_ratio;
    CPP_DENOISE_DBG("zzhdr exp time ratio trigger %f ", trigger_input);
  }

  i = 0;
  end_idx = NUM_GRAY_PATCHES - 1;
  interpolation_factor = 0;
  GET_HDR_WNR_ADJ_DATA(hdr_wnr_adj_type_ptr, end_idx, ref_noise_profile_i);
  GET_HDR_WNR_ADJ_DATA(hdr_wnr_adj_type_ptr, end_idx, ref_noise_profile_iplus1);

  for (i = 0; i < end_idx; i++) {
    if(GET_HDR_WNR_ADJ_CONTROL_TYPE(hdr_wnr_adj_type_ptr) == CONTROL_AEC_EXP_SENSITIVITY_RATIO){
      //CONTROL_AEC_EXP_SENSITIVITY_RATIO
      CPP_DENOISE_DBG("zzhdr exp sens ratio!");
      trigger_start =
        GET_HDR_WNR_ADJ_DATA_AEC_SENSITIVITY_RATIO_START(hdr_wnr_adj_type_ptr,i);
      trigger_end =
        GET_HDR_WNR_ADJ_DATA_AEC_SENSITIVITY_RATIO_END(hdr_wnr_adj_type_ptr, i);

    } else if(GET_HDR_WNR_ADJ_CONTROL_TYPE(hdr_wnr_adj_type_ptr) == CONTROL_EXP_TIME_RATIO) {
      //CONTROL_EXP_TIME_RATIO
      CPP_DENOISE_DBG("zzhdr exp time ratio!");
      trigger_start =
        GET_HDR_WNR_ADJ_DATA_EXP_TIME_RATIO_START(hdr_wnr_adj_type_ptr,i);
      trigger_end =
        GET_HDR_WNR_ADJ_DATA_EXP_TIME_RATIO_END(hdr_wnr_adj_type_ptr, i);
    }

    if (trigger_input <= trigger_start) {
      CPP_DENOISE_DBG("non interpolate i = %d ", i);
      GET_HDR_WNR_ADJ_DATA(hdr_wnr_adj_type_ptr, i, ref_noise_profile_i);
      GET_HDR_WNR_ADJ_DATA(hdr_wnr_adj_type_ptr, i, ref_noise_profile_iplus1);
      break;
    }
    if (trigger_input < trigger_end) {
      /* Interpolate all the values */
      numerator = (trigger_input - trigger_start);
      denominator = (trigger_end - trigger_start);
      if (denominator == 0.0f) {
        CPP_DENOISE_DBG("non interpolate div by 0 i = %d ", i);
        GET_HDR_WNR_ADJ_DATA(hdr_wnr_adj_type_ptr, i, ref_noise_profile_i);
        GET_HDR_WNR_ADJ_DATA(hdr_wnr_adj_type_ptr, i, ref_noise_profile_iplus1);
        break;
      }
      interpolation_factor = numerator / denominator;
      GET_HDR_WNR_ADJ_DATA(hdr_wnr_adj_type_ptr, i, ref_noise_profile_i);
      GET_HDR_WNR_ADJ_DATA(hdr_wnr_adj_type_ptr, i+1, ref_noise_profile_iplus1);

      CPP_DENOISE_DBG("interpolate, factor=%f, i=%d, i+1=%d", interpolation_factor, i, i+1);
      break;
    } /* else iterate */
  }

  CPP_DENOISE_DBG("sending for interpolation i = %d factor %f ref 1 %p ref2 %p",
    i, interpolation_factor, ref_noise_profile_i, ref_noise_profile_iplus1);
  cpp_hw_params_interpolate_wnr_adj(interpolation_factor,hw_params,
    ref_noise_profile_i, ref_noise_profile_iplus1, CPP_SCALOR);

  return 0;

}

int32_t cpp_hw_params_update_wnr_adrc_adj_params(
  modulesChromatix_t *module_chromatix,cpp_hardware_params_t *hw_params,
  cpp_params_aec_trigger_info_t *aec_trigger)
{
  uint32_t                    i, end_idx;
  float                       trigger_start = 0.0f, trigger_end = 0.0f;
  float                       interpolation_factor;
  float                       numerator, denominator;
  float                       trigger_input = 0.0f;
  chromatix_cpp_type *        chromatix_cpp = NULL;
  Chromatix_ADRC_WNR_adj_type  *adrc_wnr_adj_type_ptr;
  void *ref_noise_profile_i;
  void *ref_noise_profile_iplus1;

  if (!hw_params) {
    CPP_DENOISE_ERR("null hw_params, failed");
    return -EINVAL;
  }

  if (!module_chromatix) {
    CPP_DENOISE_ERR("null module_chromatix, failed");
    return -1;
  }
  chromatix_cpp = (chromatix_cpp_type *)module_chromatix->chromatixCppPtr;
  if (!chromatix_cpp) {
    CPP_DENOISE_ERR("failed: No CPP chromatix!!");
    return -1;
  }

  GET_ADRC_WNR_ADJ_TYPE_PTR(chromatix_cpp, hw_params, adrc_wnr_adj_type_ptr);
  if(GET_ADRC_WNR_ADJ_CONTROL_TYPE(adrc_wnr_adj_type_ptr) == CONTROL_AEC_EXP_SENSITIVITY_RATIO){
    //CONTROL_AEC_EXP_SENSITIVITY_RATIO
    if (aec_trigger->aec_sensitivity_ratio <= 0.0f){
      CPP_DENOISE_DBG("aec_sensitivity_ratio less than zero!!");
      return -1;
    }
    trigger_input = aec_trigger->aec_sensitivity_ratio;
    CPP_DENOISE_DBG("adrc exp sens ratio trigger %f ", trigger_input);
    } else if(GET_ADRC_WNR_ADJ_CONTROL_TYPE(adrc_wnr_adj_type_ptr) == CONTROL_DRC_GAIN) {
      // CONTROL_DRC_GAIN
      if (aec_trigger->total_adrc_gain < 1.0f ||
        aec_trigger->drc_color_gain < 1.0f){
        CPP_DENOISE_DBG("total_adrc_gain %f /drc color gain %f  less than zero!!",
          aec_trigger->total_adrc_gain, aec_trigger->drc_color_gain);
        return -1;
      }

    trigger_input = aec_trigger->total_adrc_gain;
    CPP_DENOISE_DBG("adrc gain trigger %f ", trigger_input);
  }

  i = 0;
  end_idx = NUM_GRAY_PATCHES - 1;
  interpolation_factor = 0;
  GET_ADRC_WNR_ADJ_DATA(adrc_wnr_adj_type_ptr, end_idx, ref_noise_profile_i);
  GET_ADRC_WNR_ADJ_DATA(adrc_wnr_adj_type_ptr, end_idx, ref_noise_profile_iplus1);

  for (i = 0; i < end_idx; i++) {
    if(GET_ADRC_WNR_ADJ_CONTROL_TYPE(adrc_wnr_adj_type_ptr) == CONTROL_AEC_EXP_SENSITIVITY_RATIO){
      //CONTROL_AEC_EXP_SENSITIVITY_RATIO
      CPP_DENOISE_DBG("adrc exp sens ratio!");
      trigger_start =
        GET_ADRC_WNR_ADJ_DATA_AEC_SENSITIVITY_RATIO_START(adrc_wnr_adj_type_ptr,i);
      trigger_end =
        GET_ADRC_WNR_ADJ_DATA_AEC_SENSITIVITY_RATIO_END(adrc_wnr_adj_type_ptr, i);

    } else if(GET_ADRC_WNR_ADJ_CONTROL_TYPE(adrc_wnr_adj_type_ptr) == CONTROL_DRC_GAIN) {
      //CONTROL_DRC_GAIN
      CPP_DENOISE_DBG("adrc drc gain ratio!");
      trigger_start =
        GET_ADRC_WNR_ADJ_DATA_DRC_GAIN_TRIGGER_START(adrc_wnr_adj_type_ptr,i);
      trigger_end =
        GET_ADRC_WNR_ADJ_DATA_DRC_GAIN_TRIGGER_END(adrc_wnr_adj_type_ptr, i);
    }

    if (trigger_input <= trigger_start) {
      CPP_DENOISE_DBG("non interpolate i = %d ", i);
      GET_ADRC_WNR_ADJ_DATA(adrc_wnr_adj_type_ptr, i, ref_noise_profile_i);
      GET_ADRC_WNR_ADJ_DATA(adrc_wnr_adj_type_ptr, i, ref_noise_profile_iplus1);
      break;
    }
    if (trigger_input < trigger_end) {
      /* Interpolate all the values */
      numerator = (trigger_input - trigger_start);
      denominator = (trigger_end - trigger_start);
      if (denominator == 0.0f) {
        CPP_DENOISE_DBG("non interpolate div by 0 i = %d ", i);
        GET_ADRC_WNR_ADJ_DATA(adrc_wnr_adj_type_ptr, i, ref_noise_profile_i);
        GET_ADRC_WNR_ADJ_DATA(adrc_wnr_adj_type_ptr, i, ref_noise_profile_iplus1);
        break;
      }
      interpolation_factor = numerator / denominator;
      GET_ADRC_WNR_ADJ_DATA(adrc_wnr_adj_type_ptr, i, ref_noise_profile_i);
      GET_ADRC_WNR_ADJ_DATA(adrc_wnr_adj_type_ptr, i+1, ref_noise_profile_iplus1);

      CPP_DENOISE_DBG("interpolate, factor=%f, i=%d, i+1=%d", interpolation_factor, i, i+1);
      break;
    } /* else iterate */
  }

  CPP_DENOISE_DBG("sending for interpolation i = %d factor %f ref 1 %p ref2 %p !",
    i, interpolation_factor, ref_noise_profile_i, ref_noise_profile_iplus1);
  cpp_hw_params_interpolate_wnr_adj(interpolation_factor,hw_params,
    ref_noise_profile_i, ref_noise_profile_iplus1, CPP_SCALOR);

  return 0;

}


int32_t cpp_hw_params_update_wnr_adj_params(modulesChromatix_t *module_chromatix,
  cpp_hardware_params_t *hw_params, cpp_params_aec_trigger_info_t *aec_trigger)
{
  chromatix_cpp_type *        chromatix_cpp = NULL;
  Chromatix_HDR_WNR_adj_type  *hdr_wnr_adj_type_ptr;
  Chromatix_ADRC_WNR_adj_type  *adrc_wnr_adj_type_ptr;

  if (!hw_params) {
    CPP_DENOISE_ERR("invalid hw_params, failed");
    return -EINVAL;
  }

  if (!module_chromatix) {
    CPP_DENOISE_ERR("invalid module_chromatix, failed");
    return -1;
  }

  chromatix_cpp = (chromatix_cpp_type *)module_chromatix->chromatixCppPtr;
  if (!chromatix_cpp) {
    CPP_DENOISE_ERR("failed: No CPP chromatix!!");
    return -1;
  }

  CPP_DENOISE_LOW("cpp_hw_params_update_wnr_adj_params");

  cpp_hw_params_update_wnr_scalor_adj_params( module_chromatix, hw_params,
    aec_trigger);

  GET_HDR_WNR_ADJ_TYPE_PTR(chromatix_cpp, hw_params, hdr_wnr_adj_type_ptr);

  if (GET_HDR_WNR_ADJ_TYPE_ENABLE(hdr_wnr_adj_type_ptr) &&
    (hw_params->hdr_mode == CAM_SENSOR_HDR_ZIGZAG)) {
    CPP_DENOISE_LOW("HDR enabled");
    cpp_hw_params_update_wnr_hdr_adj_params( module_chromatix, hw_params,
      aec_trigger);
  } else {
    CPP_DENOISE_LOW("HDR not enabled");
  }

  GET_ADRC_WNR_ADJ_TYPE_PTR(chromatix_cpp, hw_params, adrc_wnr_adj_type_ptr);

  if (GET_ADRC_WNR_ADJ_TYPE_ENABLE(adrc_wnr_adj_type_ptr)) {
    CPP_DENOISE_DBG("ADRC enabled!! ");
    cpp_hw_params_update_wnr_adrc_adj_params( module_chromatix, hw_params,
      aec_trigger);
  } else {
    CPP_DENOISE_DBG("ADRC not enabled!! ");
  }

  return 0;

}

int32_t cpp_hw_params_update_wnr_params(modulesChromatix_t *module_chromatix,
  cpp_hardware_params_t *hw_params, cpp_params_aec_trigger_info_t *aec_trigger)
{
#if ((defined(CHROMATIX_307) || defined(CHROMATIX_306) || \
  defined (CHROMATIX_308) || defined (CHROMATIX_308E) || \
  defined (CHROMATIX_309) || defined (CHROMATIX_310) || defined (CHROMATIX_310E)) && \
  defined(CAMERA_USE_CHROMATIX_HW_WNR_TYPE))
  Chromatix_hardware_wavelet_type   *wavelet_denoise;
  uint32_t num_gray_patches = WNR_HW_NUM_GRAY_PATCHES;
#else
  wavelet_denoise_type       *wavelet_denoise;
  chromatix_wavelet_type      *wavelet_chromatix_data;
  uint32_t num_gray_patches = WNR_SW_NUM_GRAY_PATCHES;
#endif
  void *ref_noise_profile_i;
  void *ref_noise_profile_iplus1;
  uint32_t                    i, end_idx;
  float                       trigger_start, trigger_end;
  float                       interpolation_factor;
  float                       numerator, denominator;
  float                       trigger_input;
  chromatix_cpp_type *        chromatix_cpp = NULL;
  float                       effects_factor = 1.0f;

  if (!hw_params) {
    CPP_DENOISE_ERR("null hwparams, failed");
    return -EINVAL;
  }

  if (hw_params->denoise_lock) {
    CPP_DENOISE_ERR("Wavelet denoise is locked by Chromatix");
    return 0;
  }

  if (!module_chromatix) {
    CPP_DENOISE_ERR("module_chromatix, failed");
    return -1;
  }

  if (!module_chromatix->chromatixCppPtr) {
    if (!hw_params->def_chromatix) {
      CPP_DENOISE_LOW("no default chromatix set");
      cpp_hw_params_init_wnr_params(hw_params);
      return -1;
    }
    cpp_hw_params_fill_wnr_from_chromatix_stripped(hw_params,
      hw_params->def_chromatix);
    return 0;
  }

  chromatix_cpp = (chromatix_cpp_type *)module_chromatix->chromatixCppPtr;
  if (!chromatix_cpp) {
    cpp_hw_params_init_wnr_params(hw_params);
    return 0;
  } else {
    GET_WAVELET_DENOISE_POINTER(chromatix_cpp, hw_params, wavelet_denoise);

    if (wavelet_denoise->control_denoise == 0) {
      CPP_DENOISE_DBG("lux triggered");
      /* Lux index based */
      trigger_input = aec_trigger->lux_idx;
    } else {
      CPP_DENOISE_DBG("gain triggered");
      /* Gain based */
      trigger_input = aec_trigger->gain;
    }

    if (trigger_input <= 0.0f) {
      CPP_DENOISE_ERR("invalid trigger input %f", trigger_input);
      return 0;
    }

    if (hw_params->asf_mode == CPP_PARAM_ASF_BEAUTYSHOT)
      effects_factor *= CPP_PARAM_BEAUTY_EFFECT_WNR_FACTOR;

    i = 0;
    end_idx = num_gray_patches - 1;

#ifdef LDS_ENABLE
    if (hw_params->lds_enable) {
      i = num_gray_patches;
      end_idx = num_gray_patches + MAX_LDS_REGIONS - 1;
      CPP_DENOISE_DBG("[LDS] i %d, end_idx %d", i, end_idx);
    }
#endif

    GET_WNR_NOISE_PROFILE(wavelet_denoise, end_idx, ref_noise_profile_i);
    GET_WNR_NOISE_PROFILE(wavelet_denoise, end_idx, ref_noise_profile_iplus1);
    interpolation_factor = 0;

    /* Find the range in the availble grey patches */
    for (; i < end_idx; i++) {
      if (wavelet_denoise->control_denoise == 0) {
        trigger_start = GET_WNR_LUXIDX_TRIGER_START(wavelet_denoise, i);
        trigger_end = GET_WNR_LUXIDX_TRIGER_END(wavelet_denoise, i);
      } else {
        trigger_start = GET_WNR_GAIN_TRIGER_START(wavelet_denoise, i);
        trigger_end = GET_WNR_GAIN_TRIGER_END(wavelet_denoise, i);
      }

      if (trigger_input <= trigger_start) {
        CPP_DENOISE_LOW("non interpolate");
        GET_WNR_NOISE_PROFILE(wavelet_denoise, i, ref_noise_profile_i);
        GET_WNR_NOISE_PROFILE(wavelet_denoise, i, ref_noise_profile_iplus1);
        break;
      }
      if (trigger_input < trigger_end) {
        /* Interpolate all the values */
        numerator = (trigger_input - trigger_start);
        denominator = (trigger_end - trigger_start);
        if (denominator == 0.0f) {
          GET_WNR_NOISE_PROFILE(wavelet_denoise, i, ref_noise_profile_i);
          GET_WNR_NOISE_PROFILE(wavelet_denoise, i, ref_noise_profile_iplus1);
          break;
        }
        interpolation_factor = numerator / denominator;
        GET_WNR_NOISE_PROFILE(wavelet_denoise, i, ref_noise_profile_i);
        GET_WNR_NOISE_PROFILE(wavelet_denoise, i+1, ref_noise_profile_iplus1);
        CPP_DENOISE_DBG("interpolate, factor=%f, i=%d, i+1=%d", interpolation_factor, i, i+1);
        break;
      } /* else iterate */
    }
  }
  /*Update adj params and interpolate */
  cpp_hw_params_update_wnr_adj_params(module_chromatix, hw_params,
    aec_trigger);
  cpp_hw_params_interpolate_wnr_params(interpolation_factor, hw_params,
    effects_factor, ref_noise_profile_i, ref_noise_profile_iplus1,
    wavelet_denoise, module_chromatix);

  return 0;
}

static void
cpp_params_apply_sac_or_sat_crop(const cpp_hardware_params_t *hw_params,
  int32_t *input_crop_width,      /* [out] */
  int32_t *input_crop_height,     /* [out] */
  int32_t *pw_fp, int32_t *pw_w,  /* [io]  */
  int32_t *pw_fl, int32_t *pw_h)  /* [io]  */
{
  int32_t rc = 0;
  float pw_cw = 0.0f, pw_ch = 0.0f;

  assert(hw_params != NULL);
  assert((input_crop_width != NULL) && (input_crop_height != NULL));
  assert((pw_fp != NULL) && (pw_w != NULL));
  assert((pw_fl != NULL) && (pw_h != NULL));

  CPP_DBG("Dual camera apply SAC/SAT crop: E");

  pw_cw = (float)hw_params->crop_info.process_window_width;
  pw_ch = (float)hw_params->crop_info.process_window_height;

  /* Apply SAC crop */
  if (hw_params->sac_enable) {
    assert((0 <= hw_params->crop_info.sac_crop_margins.widthMargins) &&
      (hw_params->crop_info.sac_crop_margins.widthMargins <= 1.0f));
    assert((0 <= hw_params->crop_info.sac_crop_margins.heightMargins) &&
      (hw_params->crop_info.sac_crop_margins.heightMargins  <= 1.0f));

    /*/
     * Apply the crop to streams on Aux/Tele session only, that too when it
     * assumes the Primary camera role. This is alreadyensured by the logic
     * implemented in cpp_module_util_decide_proc_frame_per_stream() by the
     * piggy backedack for buffers from session acting set to slave role.
    /*/

    pw_cw *= hw_params->crop_info.sac_crop_margins.widthMargins;
    pw_ch *= hw_params->crop_info.sac_crop_margins.heightMargins;

    CPP_DBG("Dual camera SAC margin crop:{cw:%d,ch:%d}", pw_cw, pw_ch);
  }
  /* Apply SAT crop */
  else {
    assert((0 <= hw_params->crop_info.dualcam_crop_data.margins.widthMargins) &&
      (hw_params->crop_info.dualcam_crop_data.margins.widthMargins <= 1.0f));
    assert((0 <= hw_params->crop_info.dualcam_crop_data.margins.heightMargins) &&
      (hw_params->crop_info.dualcam_crop_data.margins.heightMargins <= 1.0f));

    pw_cw *= hw_params->crop_info.dualcam_crop_data.margins.widthMargins;
    pw_ch *= hw_params->crop_info.dualcam_crop_data.margins.heightMargins;

    CPP_DBG("Dual camera SAT margin crop:{cw:%d,ch:%d}", pw_cw, pw_ch);
  }

  (*input_crop_width)  = pw_cw;
  (*input_crop_height) = pw_ch;

  (*pw_fp) += (pw_cw / 2);
  (*pw_w)  -= pw_cw;

  (*pw_fl) += (pw_ch / 2);
  (*pw_h)  -= pw_ch;

  CPP_DBG("Dual camera apply SAC/SAT crop: X");

  return;
}

/* cpp_module_util_compute_dualcam_shift_offset
 *
 * @shift[io]    - shift to be applied;
 *                   on return carries remaining shift that couldn't be applied
 * @location[io] - starting location;
 *                   first pixel for horizontal or first line for vertical
 * @length[in]   - segment length; on return carries updated location
 *                   relative to the given location
 * @boundary[in] - absolute boundary length;
 *                   used to cap the segment shift
 *
 * This support function computes the dual cam output shift adjustment value
 * in either horizontal/vertical direction; and will be used from within the
 * cpp_hw_params.c::cpp_params_compute_dualcam_shift_offset().
 *
 * Return: SUCCESS(0), if shift applied to input resolution
 *         -EINVAL   , otherwise
 **/
static int32_t cpp_module_util_compute_dualcam_shift_offset(int32_t *shift,
  int32_t *location,
  int32_t length,
  int32_t boundary)
{
  int32_t rc = 0;
  int32_t l_shift = 0, l_rshift = 0;
  int32_t l_location = 0;

  if ((shift == NULL) || (location == NULL) ||
      (length < 0) || (boundary < 0)) {
    return -EINVAL;
  }

  l_rshift = l_shift = *shift;
  l_location = *location;

  /*
   * Adjust for Horizontal or Vertical Shift
   */

  if (l_shift < 0) {

    int32_t margin_to_boundary_begin =
        (l_location - 0);
    int32_t shift_beyond_boundary_begin =
        (margin_to_boundary_begin + l_shift);

    /* check and clamp to the border beginning */
    if (shift_beyond_boundary_begin < 0) {
      l_shift = -margin_to_boundary_begin;
    }

  }
  /* (l_shift >= 0) */
  else {

    int32_t margin_to_boundary_end =
        (boundary - (l_location + length));
    int32_t shift_beyond_boundary_end =
        (l_location + length + l_shift) - boundary;

    /* check and clamp to the border ending */
    if (shift_beyond_boundary_end > 0) {
      l_shift = +margin_to_boundary_end;
    }

  }

  l_rshift -= l_shift;
  l_location += l_shift;

  *shift = l_rshift;       /* remaining shift not applied */
  *location = l_location;  /* shifted location */

  return rc;
} /* cpp_module_util_compute_dualcam_shift_offset() */

/* cpp_params_apply_dualcam_shift_offset_and_margins:
 *
 * @hw_params[io] - parameters to be applied by CPP hardware.
 *
 * Support function to adjust process window based on the dual cam output shift,
 * (-> in place).
 *
 * Return: SUCCESS (0), if shift applied to input resolution
 *         -EINVAL,     otherwise
 **/
static int32_t cpp_params_apply_dualcam_shift_offset_and_margins(
  cpp_hardware_params_t *hw_params)
{
  int32_t rc = 0;
  int32_t input_res_width  = hw_params->input_info.width;
  int32_t input_res_height = hw_params->input_info.height;
  boolean is_valid_ref_res = false;
  int32_t pw_fp = hw_params->crop_info.process_window_first_pixel;
  int32_t pw_fl = hw_params->crop_info.process_window_first_line;
  int32_t pw_w  = hw_params->crop_info.process_window_width;
  int32_t pw_h  = hw_params->crop_info.process_window_height;

  int32_t input_shift_horz  = 0;
  int32_t input_shift_vert  = 0;
  int32_t input_crop_width  = 0;
  int32_t input_crop_height = 0;

  CPP_DBG("Dual camera shift offsets: E");

  if (!hw_params->dualcam_shift_offsets.is_output_shift_valid) {
    /* Shift not applied, caller can choose to ignore this value */
    CPP_DBG("b4 Dual camera shift offsets: X "
      "is_output_shift_valid=%u, "
      "frame id=%d, identity=0x%x",
      hw_params->dualcam_shift_offsets.is_output_shift_valid,
      hw_params->frame_id,
      hw_params->identity);
    return -EINVAL;
  }

  CPP_DBG("b4 Dual camera shift offsets: "
    "input_info:{w=%d,h=%d}, "
    "pwfp=%d, pwfl=%d, pww=%d, pwh=%d, "
    "is_output_shift_valid=%u, output_shift:{sh=%i,sv=%i}, "
    "reference_res_for_output_shift:{w=%i,h=%i}, "
    "margins:{wm=%f,hm=%f)"
    "frame id=%d, identity=0x%x",
    hw_params->input_info.width,
    hw_params->input_info.height,
    hw_params->crop_info.process_window_first_pixel,
    hw_params->crop_info.process_window_first_line,
    hw_params->crop_info.process_window_width,
    hw_params->crop_info.process_window_height,
    hw_params->dualcam_shift_offsets.is_output_shift_valid,
    hw_params->dualcam_shift_offsets.output_shift.shift_horz,
    hw_params->dualcam_shift_offsets.output_shift.shift_vert,
    hw_params->dualcam_shift_offsets.reference_res_for_output_shift.width,
    hw_params->dualcam_shift_offsets.reference_res_for_output_shift.height,
    hw_params->crop_info.dualcam_crop_data.margins.widthMargins,  /* Width margin in %*/
    hw_params->crop_info.dualcam_crop_data.margins.heightMargins, /* Height margin in %*/
    hw_params->frame_id,
    hw_params->identity);

  /* calculate input resolution shift w.r.t the output reference shift */
  is_valid_ref_res =
    ((hw_params->dualcam_shift_offsets.reference_res_for_output_shift.width  > 0) &&
     (hw_params->dualcam_shift_offsets.reference_res_for_output_shift.height > 0));
  if (!is_valid_ref_res) {
    CPP_DBG("<b4,l8r> Dual camera shift offsets: "
      "Invalid reference resolution for output shift");
    assert(false);
    return -EINVAL;
  }

  input_shift_horz = hw_params->zoom_factor * ((float)input_res_width *
    (float)(hw_params->dualcam_shift_offsets.output_shift.shift_horz /
      (float)hw_params->dualcam_shift_offsets.reference_res_for_output_shift.width));
  input_shift_vert = hw_params->zoom_factor * ((float)input_res_height *
    (float)(hw_params->dualcam_shift_offsets.output_shift.shift_vert /
      (float)hw_params->dualcam_shift_offsets.reference_res_for_output_shift.height));

  CPP_DBG("<b4,l8r> Dual camera shift offsets: "
    "zoom_factor %f, "
    "input_shift:{sh=%i,sv=%i}, "
    "pwfp=%d, pwfl=%d, pww=%d, pwh=%d "
    "frame_id=%d, identity=0x%x",
    hw_params->zoom_factor,
    input_shift_horz, input_shift_vert,
    pw_fp, pw_fl, pw_w, pw_h,
    hw_params->frame_id, hw_params->identity);

  /* Adjust top left corner of previously computed process window,
     with Zoom, IS, margin crop applied first, shift applied next.
     Horizontal shift may be needed even when no margins included */

  cpp_params_apply_sac_or_sat_crop(hw_params,
    &input_crop_width, &input_crop_height,
    &pw_fp, &pw_w,
    &pw_fl, &pw_h);

  CPP_DBG("<b4,l8r> Dual camera shift offsets: "
    "input_shift:{sh=%i,sv=%i}, "
    "input_crop:{cw=%d,ch=%d}, "
    "pwfp=%d, pwfl=%d, pww=%d, pwh=%d",
    input_shift_horz, input_shift_vert,
    input_crop_width, input_crop_height,
    pw_fp, pw_fl, pw_w, pw_h);


  /* Adjust for Horizontal Crop and Shift */
  if (cpp_module_util_compute_dualcam_shift_offset(&input_shift_horz,
        &pw_fp, pw_w, input_res_width) == -EINVAL) {
    return -EINVAL;
  }

  /* Now, reflect updated process window first pixel and width */
  hw_params->crop_info.process_window_first_pixel = pw_fp;
  hw_params->crop_info.process_window_width = pw_w;

  CPP_DBG("<b4,l8r> Dual camera shift offsets: "
    "input_shift:{sh=%i,sv=%i}, "
    "input_crop:{cw=%d,ch=%d}, "
    "pwfp=%d, pwfl=%d, pww=%d, pwh=%d",
    input_shift_horz, input_shift_vert,
    input_crop_width, input_crop_height,
    hw_params->crop_info.process_window_first_pixel, pw_fl,
    hw_params->crop_info.process_window_width, pw_h);


  /* Adjust for Vertical Crop and Shift */
  if (cpp_module_util_compute_dualcam_shift_offset(&input_shift_vert,
        &pw_fl, pw_h, input_res_height) == -EINVAL) {
    return -EINVAL;
  }

  /* Now, reflect updated process window first line and height */
  hw_params->crop_info.process_window_first_line = pw_fl;
  hw_params->crop_info.process_window_height = pw_h;

  CPP_DBG("<b4,l8r> Dual camera shift offsets: "
    "input_shift:{sh=%i,sv=%i}, "
    "input_crop:{cw=%d,ch=%d}, "
    "pwfp=%d, pwfl=%d, pww=%d, pwh=%d",
    input_shift_horz, input_shift_vert,
    input_crop_width, input_crop_height,
    pw_fp, hw_params->crop_info.process_window_first_line,
    pw_w, hw_params->crop_info.process_window_height);


  CPP_DBG("l8r Dual camera shift offsets: "
    "shift_margins:{wm=%f,hm=%f}, "
    "pwfp=%d, pwfl=%d, pww=%d, pwh=%d frame_id=%d, identity=0x%x",
    hw_params->crop_info.dualcam_crop_data.margins.widthMargins,
    hw_params->crop_info.dualcam_crop_data.margins.heightMargins,
    hw_params->crop_info.process_window_first_pixel,
    hw_params->crop_info.process_window_first_line,
    hw_params->crop_info.process_window_width,
    hw_params->crop_info.process_window_height,
    hw_params->frame_id,
    hw_params->identity);

  CPP_DBG("Dual camera shift offsets: X");

  return rc;
}  /* cpp_params_apply_dualcam_shift_offset_and_margins() */

int32_t cpp_params_calculate_crop(cpp_hardware_params_t *hw_params)
{
  uint32_t x, y;
  float cpp_scale_ratio = 1.0f;
  uint32_t process_window_first_pixel, process_window_first_line;
  uint32_t process_window_width, process_window_height;
  assert(hw_params->input_info.width != 0);
  assert(hw_params->input_info.height != 0);


  /* Due to limitation from ISP, need to assume
     (0, 0, 0, 0) as indication of no-crop setting */
  if (hw_params->crop_info.stream_crop.x  == 0 &&
      hw_params->crop_info.stream_crop.y  == 0 &&
      hw_params->crop_info.stream_crop.dx == 0 &&
      hw_params->crop_info.stream_crop.dy == 0) {
    hw_params->crop_info.stream_crop.dx = hw_params->input_info.width;
    hw_params->crop_info.stream_crop.dy = hw_params->input_info.height;
  }

  /* apply Zoom & IS cropping to the process window */
  x = (hw_params->crop_info.stream_crop.x * hw_params->crop_info.is_crop.dx) /
    hw_params->input_info.width;
  y = (hw_params->crop_info.stream_crop.y * hw_params->crop_info.is_crop.dy) /
    hw_params->input_info.height;
  /* calculate the first pixel in window */
  hw_params->crop_info.process_window_first_pixel =
    x + hw_params->crop_info.is_crop.x;
  /* calculate the first line in window */
  hw_params->crop_info.process_window_first_line =
    y + hw_params->crop_info.is_crop.y;
  /* calculate the window width */
  hw_params->crop_info.process_window_width =
    (hw_params->crop_info.stream_crop.dx * hw_params->crop_info.is_crop.dx)/
     hw_params->input_info.width;
  /* calculate the window height */
  hw_params->crop_info.process_window_height =
    (hw_params->crop_info.stream_crop.dy * hw_params->crop_info.is_crop.dy)/
     hw_params->input_info.height;

  if(hw_params->crop_info.process_window_width <= 0 ||
    hw_params->crop_info.process_window_height <= 0) {
    CPP_CROP_ERR("Crop info is 0!");
    return -EINVAL;
  }

  CPP_CROP_DBG("b4 fovc crop: fovc_cf=%f, "
    "pwfp=%d, pwfl=%d, pww=%d, pwh=%d frame_id=%d, identity=0x%x",
    hw_params->crop_info.fovc_crop_factor,
    hw_params->crop_info.process_window_first_pixel,
    hw_params->crop_info.process_window_first_line,
    hw_params->crop_info.process_window_width,
    hw_params->crop_info.process_window_height,
    hw_params->frame_id,
    hw_params->identity);

  /*
   * now adjust process window based on the FOVC factor provided (as a %) by 3A
   */
  if (hw_params->fovc_enable &&
      !F_EQUAL(hw_params->crop_info.fovc_crop_factor, 0.0f)) {
    float fovc_cf = hw_params->crop_info.fovc_crop_factor;
    uint32_t input_width  = hw_params->crop_info.process_window_width;
    uint32_t input_height = hw_params->crop_info.process_window_height;

    /* calculate adjusted width and height
       to apply margins on left & right, top & bottom */
    input_width  -= (input_width  * (1 - fovc_cf));
    input_height -= (input_height * (1 - fovc_cf));

    /* adjust top left corner of previously computed process window,
       with Zoom and IS crop applied */
    hw_params->crop_info.process_window_first_pixel += (input_width  / 2);
    hw_params->crop_info.process_window_first_line  += (input_height / 2);

    hw_params->crop_info.process_window_width  -= input_width;
    hw_params->crop_info.process_window_height -= input_height;

    CPP_CROP_DBG("fovc crop: fovc_cf=%f, "
      "pwfp=%d, pwfl=%d, pww=%d, pwh=%d frame_id=%d, identity=0x%x",
      hw_params->crop_info.fovc_crop_factor,
      hw_params->crop_info.process_window_first_pixel,
      hw_params->crop_info.process_window_first_line,
      hw_params->crop_info.process_window_width,
      hw_params->crop_info.process_window_height,
      hw_params->frame_id,
      hw_params->identity);
  }

  hw_params->zoom_factor = 1.0f;
  CPP_CROP_DBG("ISP scaler Ratio %f", hw_params->isp_scaler_ratio);
  if (hw_params->isp_scaler_ratio) {
    hw_params->zoom_factor = 1.0f / hw_params->isp_scaler_ratio;
  }

  /*
   * Adjust process window based on the dual cam output shift and margins,
   * as needed for QC-SAC/SAT feature(s).
   */
  cpp_params_apply_dualcam_shift_offset_and_margins(hw_params);

  if (!hw_params->isp_width_map || !hw_params->isp_height_map ||
    !hw_params->input_info.width || !hw_params->input_info.height) {
    hw_params->isp_scale_ratio = 1.0f;
  } else {
    float width_ratio, height_ratio;
    width_ratio = (float)hw_params->isp_width_map /
      hw_params->input_info.width;
    height_ratio = (float)hw_params->isp_height_map /
      hw_params->input_info.height;
    if (width_ratio <= height_ratio) {
      hw_params->isp_scale_ratio = width_ratio;
    } else {
      hw_params->isp_scale_ratio = height_ratio;
    }
  }

  if (!hw_params->crop_info.process_window_width ||
      !hw_params->crop_info.process_window_height ||
    !hw_params->input_info.width || !hw_params->input_info.height) {
    hw_params->cpp_scale_ratio = 1.0f;
  } else {
    float width_ratio, height_ratio;
    width_ratio = (float)hw_params->crop_info.process_window_width /
      hw_params->output_info.width;
    height_ratio = (float)hw_params->crop_info.process_window_height /
      hw_params->output_info.height;
    if (width_ratio <= height_ratio) {
      hw_params->cpp_scale_ratio = width_ratio;
    } else {
      hw_params->cpp_scale_ratio = height_ratio;
    }
  }

  /*
   * Handle scale limitations.
   * Limit downscale to 16.0f  for NV12 formats if request is beyond this.
   * Limit downscale to 10.9f  for UBWC formats if request is beyond this.
   * Limit upscale to 32.0f if request is beyond this.
   * FOV will be compromised in case of limitting scale thresholds.
   */
  double downscale_limit = UBWC_ENABLE(hw_params->output_info.plane_fmt) ?
    CPP_DOWNSCALE_LIMIT_UBWC : CPP_DOWNSCALE_LIMIT;
  if (hw_params->cpp_scale_ratio > downscale_limit) {
    cpp_scale_ratio = downscale_limit;
  } else if (hw_params->cpp_scale_ratio < (1 / CPP_UPSCALE_LIMIT)) {
    cpp_scale_ratio = (1 / CPP_UPSCALE_LIMIT);
  }

  /* Correct crop */
  if (!F_EQUAL(cpp_scale_ratio, 1.0f)) {
    hw_params->crop_info.process_window_width *=
      (float)(cpp_scale_ratio / hw_params->cpp_scale_ratio);
    hw_params->crop_info.process_window_height *=
      (float)(cpp_scale_ratio / hw_params->cpp_scale_ratio);
    hw_params->crop_info.process_window_first_pixel *=
      (float)(cpp_scale_ratio/hw_params->cpp_scale_ratio);
    hw_params->crop_info.process_window_first_line *=
      (float)(cpp_scale_ratio/hw_params->cpp_scale_ratio);
    CPP_CROP_WARN("FOV compromised, scale ratio limit %f, "
      "corrected scale %f, pwfp=%d, pwfl=%d, pww=%d, pwh=%d",
      hw_params->cpp_scale_ratio,
      cpp_scale_ratio,
      hw_params->crop_info.process_window_first_pixel,
      hw_params->crop_info.process_window_first_line,
      hw_params->crop_info.process_window_width,
      hw_params->crop_info.process_window_height);
    hw_params->cpp_scale_ratio = cpp_scale_ratio;
  }

  CPP_CROP_LOW("corrected crop: pwfp=%d, pwfl=%d, pww=%d, pwh=%d",
    hw_params->crop_info.process_window_first_pixel,
    hw_params->crop_info.process_window_first_line,
    hw_params->crop_info.process_window_width,
    hw_params->crop_info.process_window_height);

  CPP_CROP_DBG("l8r fovc crop: fovc_cf=%f, pwfp=%d, pwfl=%d, pww=%d, pwh=%d frame_id=%d, identity=0x%x",
    hw_params->crop_info.fovc_crop_factor,
    hw_params->crop_info.process_window_first_pixel,
    hw_params->crop_info.process_window_first_line,
    hw_params->crop_info.process_window_width,
    hw_params->crop_info.process_window_height,
    hw_params->frame_id,
    hw_params->identity);

#if 1
  if (hw_params->crop_info.process_window_height &&
    hw_params->output_info.height) {
    float dst_aspect_ratio = (float)hw_params->output_info.width /
      (float)hw_params->output_info.height;
    float src_aspect_ratio = (float)hw_params->crop_info.process_window_width /
      (float)hw_params->crop_info.process_window_height;
    if (F_EQUAL_1(dst_aspect_ratio, src_aspect_ratio)) {
      process_window_height = hw_params->crop_info.process_window_height;
      process_window_width  = hw_params->crop_info.process_window_width;
      process_window_first_pixel =
        hw_params->crop_info.process_window_first_pixel;
      process_window_first_line =
        hw_params->crop_info.process_window_first_line;
    } else if (dst_aspect_ratio > src_aspect_ratio) {
      process_window_height =
        (float)hw_params->crop_info.process_window_width / dst_aspect_ratio;
      process_window_width = hw_params->crop_info.process_window_width;
      process_window_first_pixel =
        hw_params->crop_info.process_window_first_pixel;
      process_window_first_line =
        hw_params->crop_info.process_window_first_line +
        ((float)(hw_params->crop_info.process_window_height -
        process_window_height)) / 2.0;
    } else {
      process_window_width =
        (float)hw_params->crop_info.process_window_height * dst_aspect_ratio;
      process_window_height = hw_params->crop_info.process_window_height;
      process_window_first_line =
        hw_params->crop_info.process_window_first_line;
      process_window_first_pixel =
        hw_params->crop_info.process_window_first_pixel +
        ((float)(hw_params->crop_info.process_window_width -
        process_window_width)) / 2.0;
    }

    hw_params->crop_info.process_window_width = process_window_width;
    hw_params->crop_info.process_window_height = process_window_height;
    hw_params->crop_info.process_window_first_pixel =
      process_window_first_pixel;
    hw_params->crop_info.process_window_first_line =
      process_window_first_line;
  }

  CPP_CROP_DBG("dbg30 Aspect ratio: pwfp=%d, pwfl=%d, pww=%d, pwh=%d frame_id= %d",
    hw_params->crop_info.process_window_first_pixel,
    hw_params->crop_info.process_window_first_line,
    hw_params->crop_info.process_window_width,
    hw_params->crop_info.process_window_height,
    hw_params->frame_id);
#endif
#if 0
  /*
   * When upscale is done CPP driver adds scale padding to the frame settings.
   * if the current frame is used for upscale in the next reprocess pass , these paddings
   * have to be taken in account. It they are not apply the resulted image will have
   *green rim.
   */
  CPP_CROP_HIGH("dbg28 width %d vs %d, height %d vs %d",
    hw_params->crop_info.process_window_width,
       hw_params->output_info.width,
    hw_params->crop_info.process_window_height,
       hw_params->output_info.height);
  if ((hw_params->crop_info.process_window_width  <
       hw_params->output_info.width) &&
    (hw_params->crop_info.process_window_height <
       hw_params->output_info.height)) {
    if (!hw_params->scale_enable) {
      hw_params->crop_info.process_window_first_pixel -= upscale_left_top_padding;
      hw_params->crop_info.process_window_first_line -= upscale_left_top_padding;
      hw_params->crop_info.process_window_width += upscale_right_bottom_padding;
      hw_params->crop_info.process_window_height += upscale_right_bottom_padding;
      hw_params->crop_info.process_window_width =
         (hw_params->crop_info.process_window_width + 0x3) & ~0x3;
      hw_params->crop_info.process_window_height =
         (hw_params->crop_info.process_window_height + 0x3) & ~0x3;

      if (hw_params->stream_type == 1) {
        CPP_CROP_HIGH("Module padding: pwfp=%d, pwfl=%d, pww=%d, pwh=%d",
          hw_params->crop_info.process_window_first_pixel,
          hw_params->crop_info.process_window_first_line,
          hw_params->crop_info.process_window_width,
          hw_params->crop_info.process_window_height);
      }

      process_window_first_pixel =
        (hw_params->crop_info.process_window_first_pixel & 0xFFFFFFFC);
      process_window_first_line =
        (hw_params->crop_info.process_window_first_line  & 0xFFFFFFFC);
      hw_params->crop_info.process_window_width +=
        (hw_params->crop_info.process_window_first_pixel - process_window_first_pixel);
      hw_params->crop_info.process_window_height +=
        (hw_params->crop_info.process_window_first_line - process_window_first_line);

      hw_params->crop_info.process_window_first_pixel = process_window_first_pixel;
      hw_params->crop_info.process_window_first_line = process_window_first_line;

      hw_params->crop_info.process_window_width =
        (hw_params->crop_info.process_window_width + 0x3) & ~0x3;
      hw_params->crop_info.process_window_height =
        (hw_params->crop_info.process_window_height + 0x3) & ~0x3;
    } else {
      process_window_first_pixel =
        ((hw_params->crop_info.process_window_first_pixel + 0x3) & ~0x3);
      process_window_first_line =
        ((hw_params->crop_info.process_window_first_line + 0x3) & ~0x3);
      hw_params->crop_info.process_window_width -=
        (process_window_first_pixel - hw_params->crop_info.process_window_first_pixel);
      hw_params->crop_info.process_window_height -=
        (process_window_first_line - hw_params->crop_info.process_window_first_line);

      hw_params->crop_info.process_window_first_pixel = process_window_first_pixel;
      hw_params->crop_info.process_window_first_line = process_window_first_line;
      hw_params->crop_info.process_window_width &= 0xFFFFFFFC;
      hw_params->crop_info.process_window_height &= 0xFFFFFFFC;
    }
  }
#endif
  CPP_CROP_DBG("pwfp=%d, pwfl=%d, pww=%d, pwh=%d",
    hw_params->crop_info.process_window_first_pixel,
    hw_params->crop_info.process_window_first_line,
    hw_params->crop_info.process_window_width,
    hw_params->crop_info.process_window_height);
  return 0;
}

/** cpp_hw_params_interpolate_zoom_scale_ext:
 *
 *  @hw_params - hardware parameters of the current stream
 *  @frame_info - The data for the particular frame
 *
 *  Calculate and load the WNR adjustment factors to frame info structure.
 *
 **/
static void cpp_params_add_wnr_adj(cpp_hardware_params_t *hw_params,
  struct cpp_frame_info_t *frame_info) {
  uint32_t i, j;

  for (i = 0; i < CPP_DENOISE_NUM_PROFILES; i++) {
    for (j = 0; j < CPP_DENOISE_NUM_PLANES; j++) {
      frame_info->profile_adj[j][i] =
        hw_params->denoise_info[j][i].profile_adj[ISP_SCALOR][i] *
        hw_params->denoise_info[j][i].profile_adj[CPP_SCALOR][i];
      frame_info->weight_VFE_adj[j][i] =
        hw_params->denoise_info[j][i].weight_VFE_adj[ISP_SCALOR][i] *
        hw_params->denoise_info[j][i].weight_VFE_adj[CPP_SCALOR][i];
    }
  }
}


/* currently hardcodes some values */
int32_t cpp_params_create_frame_info(cpp_hardware_t *cpphw,
  cpp_hardware_params_t *hw_params, struct cpp_frame_info_t *frame_info)
{
  uint32_t i = 0, j, k, chroma_1 = 1, chroma_2 = 2;
  uint32_t plane_count;
  struct cpp_plane_info_t *plane_info = frame_info->plane_info;
  struct cpp_asf_info *asf_info = &frame_info->asf_info;
  int input_buffer_offset = 0, output_buffer_offset = 0;
  uint32_t asf_info_filter_k_entries = 0;
  char value[PROPERTY_VALUE_MAX];
  float fmt_conv_vscale_ratio = 1;
  float fmt_conv_hscale_ratio = 1;
  bool cace_inval = false;
  bool cace_clean = false;
  pp_native_buf_mgr_t *pp_buf_mgr =
    (pp_native_buf_mgr_t *)cpphw->private_data;

  /* copy the hw params */
  frame_info->p_hw_params = hw_params;

  /* calculate the crop */
  assert(hw_params->output_info.width != 0);
  assert(hw_params->output_info.height != 0);

  /* Copy the asf_info for 1.6.x */
  memcpy(&(frame_info->u_asf),
    &(hw_params->asf_info.info_1_6_x), sizeof(cpp_1_6_x_asf_info_t));

 /* Hack to equalize the snpashots in ZSL and NON-ZSl mode when flip is not 0 */
  if((hw_params->mirror == 2) && (hw_params->rotation == 1)) {
     hw_params->rotation = 3;
  }
  else if((hw_params->mirror == 2) && (hw_params->rotation == 3)) {
     hw_params->rotation = 1;
  }
  else if((hw_params->mirror == 1) && (hw_params->rotation == 1)) {
     hw_params->rotation = 3;
  }
  else if((hw_params->mirror == 1) && (hw_params->rotation == 3)) {
     hw_params->rotation = 1;
  }

  if(hw_params->input_info.plane_fmt == CPP_PARAM_PLANE_CRCB420)
    plane_count = 3;
  else if(hw_params->input_info.plane_fmt == CPP_PARAM_PLANE_Y)
    plane_count = 1;
  else
    plane_count = 2;

  for(i = 0; i < plane_count; i++) {
    memset(&plane_info[i], 0, sizeof(struct cpp_plane_info_t));
  }

  plane_info[0].is_not_y_plane = 0;
  plane_info[1].is_not_y_plane = 1;

  plane_info[0].input_plane_fmt = PLANE_Y;
  plane_info[0].input_bytes_per_pixel = 1;
  plane_info[0].output_plane_fmt = PLANE_Y;
  plane_info[0].output_bytes_per_pixel = 1;
  plane_info[0].temporal_bytes_per_pixel = 1;

  if(hw_params->input_info.plane_fmt == CPP_PARAM_PLANE_CRCB420) {
     plane_info[2].is_not_y_plane = 1;
     plane_info[1].input_plane_fmt = PLANE_CB;
     plane_info[2].input_plane_fmt = PLANE_CR;
     plane_info[1].input_bytes_per_pixel = 1;
     plane_info[2].input_bytes_per_pixel = 1;
     plane_info[1].temporal_bytes_per_pixel = 1;
     plane_info[2].temporal_bytes_per_pixel = 1;
  } else if(hw_params->input_info.plane_fmt == CPP_PARAM_PLANE_Y) {
     plane_info[1].is_not_y_plane = 0;
  } else {
     plane_info[1].input_plane_fmt = PLANE_CBCR;
     plane_info[1].input_bytes_per_pixel = 2;
     plane_info[1].temporal_bytes_per_pixel = 2;
  }

  if(hw_params->output_info.plane_fmt == CPP_PARAM_PLANE_CRCB420){
     plane_info[1].output_plane_fmt = PLANE_CB;
     plane_info[2].output_plane_fmt = PLANE_CR;
     plane_info[1].output_bytes_per_pixel = 1;
     plane_info[2].output_bytes_per_pixel = 1;
  } else if(hw_params->input_info.plane_fmt != CPP_PARAM_PLANE_Y){
     plane_info[1].output_plane_fmt = PLANE_CBCR;
     plane_info[1].output_bytes_per_pixel = 2;
  }

  for(i = 0; i < plane_count; i++) {
      plane_info[i].rotate = hw_params->rotation;
      plane_info[i].mirror = hw_params->mirror;
      memcpy(&plane_info[i].input_plane_config,
        &hw_params->input_info.plane_info[i],
        sizeof(cpp_params_plane_info_t));
      memcpy(&plane_info[i].output_plane_config,
        &hw_params->output_info.plane_info[i],
        sizeof(cpp_params_plane_info_t));
      if (hw_params->scale_enable) {
          plane_info[i].h_scale_ratio =
            (double)hw_params->crop_info.process_window_width /
              hw_params->output_info.width;
          plane_info[i].v_scale_ratio =
            (double)hw_params->crop_info.process_window_height /
              hw_params->output_info.height;
      } else {
         plane_info[i].h_scale_ratio = 1.0f;
         plane_info[i].v_scale_ratio = 1.0f;
      }

      plane_info[i].h_scale_initial_phase =
        hw_params->crop_info.process_window_first_pixel;
      plane_info[i].v_scale_initial_phase =
        hw_params->crop_info.process_window_first_line;
      plane_info[i].src_width = hw_params->input_info.width;
      plane_info[i].src_height = hw_params->input_info.height;
      plane_info[i].src_stride = hw_params->input_info.stride;
      plane_info[i]. maximum_src_stripe_height =
        PAD_TO_2(hw_params->input_info.scanline);

      if (hw_params->scale_enable) {
        plane_info[i].dst_width = hw_params->output_info.width;
        plane_info[i].dst_height = hw_params->output_info.height;
      } else {
        plane_info[i].dst_width = hw_params->crop_info.process_window_width;
        plane_info[i].dst_height = hw_params->crop_info.process_window_height;
      }

      plane_info[i].tnr_enable =
        cpp_hw_params_is_tnr_enabled(cpphw, hw_params, &plane_info[i]);

      /*If write engine is disabled and TNR is also disabled, trigger ack through -EAGAIN*/
      if(hw_params->we_disable && !plane_info[i].tnr_enable) {
        return -CPP_ERR_PROCESS_FRAME_ERROR;
      }
      /*Update previous frame tnr state*/

      plane_info[i].denoise_after_scale_en = 0;
      if (plane_info[i].tnr_enable && (!hw_params->downsample_mask)) {
        plane_info[i].denoise_after_scale_en = 1;
      }

      plane_info[i].sharpen_before_scale = 0;
      if ((cpphw->hwinfo.version == CPP_HW_VERSION_6_1_0 ||
        cpphw->hwinfo.version == CPP_HW_VERSION_6_1_2 ||
        cpphw->hwinfo.version == CPP_HW_VERSION_6_1_4) &&
        (!plane_info[i].denoise_after_scale_en)) {
        plane_info[i].sharpen_before_scale = 1;
      }

      if(cpphw->hwinfo.version == CPP_HW_VERSION_5_0_0 ||
         cpphw->hwinfo.version == CPP_HW_VERSION_5_1_0 ||
         cpphw->hwinfo.version == CPP_HW_VERSION_6_0_0 ||
         cpphw->hwinfo.version == CPP_HW_VERSION_6_1_0 ||
         cpphw->hwinfo.version == CPP_HW_VERSION_6_1_2 ||
         cpphw->hwinfo.version == CPP_HW_VERSION_6_1_4) {
        /*
         * pad boundary to be set to 0 by default.
         * For rim padding enable pad boundary
         */
        plane_info[i].pad_boundary  = 0;
        set_default_crop_padding_1_8(&plane_info[i]);
      } else {
        set_default_crop_padding(&plane_info[i]);
      }

      plane_info[i].pbf_enable = cpp_hw_params_is_pbf_enabled(&plane_info[i],
        hw_params);
      CPP_FRAME_DBG("[CREATE_FRAME] plane %d, h phase %f, v phase %f",
         i, plane_info[i].h_scale_initial_phase,
         plane_info[i].v_scale_initial_phase);

      if (plane_info[i].denoise_after_scale_en) {
        if ((hw_params->rotation == 1) || (hw_params->rotation == 3)) {
          plane_info[i].temporal_stride = hw_params->output_info.scanline;
        } else {
          plane_info[i].temporal_stride = hw_params->output_info.stride;
        }
      } else {
        plane_info[i].temporal_stride = hw_params->input_info.stride;
      }

      plane_info[i].bf_enable = hw_params->denoise_enable;

      CPP_FRAME_LOW("UBWC %d, plane_count %d",
        UBWC_ENABLE(hw_params->output_info.plane_fmt), plane_count);
      plane_info[i].tile_output_enable =
        UBWC_ENABLE(hw_params->output_info.plane_fmt);
      if (UBWC_ENABLE(hw_params->output_info.plane_fmt)) {
        /* tile width conversion to pixel happens in stripe code */
        /* Use two 2 tiles in a block to avoid tile width = block width */
        plane_info[i].tile_byte_width = LUMA_TILE_WIDTH *2;
        plane_info[i].tile_byte_height  = TILE_HEIGHT;
        if ((hw_params->rotation == 0) || (hw_params->rotation == 2)) {
          plane_info[i].dst_stride =
            (int)(ceil((float)(ceil((float)(plane_info[0].dst_width) /
            LUMA_TILE_WIDTH)) / MACRO_TILE) * DATA_ALIGN);
          plane_info[i].metadata_stride = (int)ceil((float)(floor((float)
            (ceil((float)(plane_info[0].dst_width) /
            LUMA_TILE_WIDTH) + (META_WIDTH_ALIGN - 1)) / META_WIDTH_ALIGN) *
            META_WIDTH_ALIGN) / META_HEIGHT_ALIGN) *
            META_STRIDE_BYTE_ALIGN;
        } else if ((hw_params->rotation == 1) || (hw_params->rotation == 3)) {
          plane_info[i].dst_stride =
            (int)(ceil((float)(ceil((float)(plane_info[0].dst_height) /
            LUMA_TILE_WIDTH) / MACRO_TILE)) * DATA_ALIGN);
          plane_info[i].metadata_stride = (int)ceil((float)(floor((float)
            (ceil((float)(plane_info[0].dst_height) /
            LUMA_TILE_WIDTH) + (META_WIDTH_ALIGN - 1)) / META_WIDTH_ALIGN) *
            META_WIDTH_ALIGN)/ META_HEIGHT_ALIGN) *
            META_STRIDE_BYTE_ALIGN;
        }
      } else {
        plane_info[i].dst_stride = hw_params->output_info.stride;
      }
      plane_info[i].rotation_buffer_size = ROTATION_BUFFER_SIZE;
      plane_info[i].line_buffer_size = LINE_BUFFER_SIZE;
      plane_info[i].mal_byte_size = MAL_SIZE;

      plane_info[i].maximum_dst_stripe_height =
        PAD_TO_2(hw_params->output_info.scanline);
      if (hw_params->scale_enable) {
        plane_info[i].dst_x_offset = 0;
        plane_info[i].dst_y_offset = 0;
      } else {
        plane_info[i].dst_x_offset =
          hw_params->crop_info.process_window_first_pixel;
        plane_info[i].dst_y_offset =
          hw_params->crop_info.process_window_first_line;
      }

      plane_info[i].metadata_offset = output_buffer_offset;
      plane_info[i].source_address[0] =
        input_buffer_offset +
        hw_params->input_info.plane_info[i].plane_offsets;
      if (cpp_hw_params_is_lln_enabled(cpphw, hw_params, i)) {
        plane_info[i].pad_boundary = 1;
        plane_info[i].source_address[0]  -=
          (cpphw->max_supported_padding +
          (plane_info[i].src_stride * cpphw->max_supported_padding));
        CPP_FRAME_LOW("[CREATE_FRAME] stream_type %d, plane %d,"
          "pad_boundary %d, pad %d, stride %d",
          hw_params->stream_type, i, plane_info[i].pad_boundary,
          cpphw->max_supported_padding, plane_info[i].src_stride);
      }
      plane_info[i].framedata_offset =
        output_buffer_offset +
        hw_params->output_info.plane_info[i].meta_len +
        hw_params->output_info.plane_info[i].plane_offsets;
      plane_info[i].destination_address[0] =
        plane_info[i].destination_address[1] =
        plane_info[i].framedata_offset;
      if (plane_info[i].denoise_after_scale_en) {
        plane_info[i].temporal_source_address[0] =
          output_buffer_offset +
          hw_params->output_info.plane_info[i].plane_offsets;
        plane_info[i].temporal_destination_address[0] =
          output_buffer_offset +
          hw_params->output_info.plane_info[i].plane_offsets;
      } else {
        plane_info[i].temporal_source_address[0] =
          input_buffer_offset +
          hw_params->input_info.plane_info[i].plane_offsets;
        plane_info[i].temporal_destination_address[0] =
          input_buffer_offset +
          hw_params->input_info.plane_info[i].plane_offsets;
      }
      output_buffer_offset += hw_params->output_info.plane_info[i].plane_len;
      input_buffer_offset += hw_params->input_info.plane_info[i].plane_len;

#ifdef LDS_ENABLE
      CPP_FRAME_LOW("[LDS:]stream_type %d,plane %d,cds %d,ds-mask %d, lds %d",
        hw_params->stream_type, i, hw_params->uv_upsample_enable,
        hw_params->downsample_mask, hw_params->lds_enable);

      if ((hw_params->lds_enable) && (hw_params->downsample_mask)) {
        plane_info[i].src_width /= 2;
        plane_info[i].src_height /= 2;
        plane_info[i].h_scale_initial_phase /= 2;
        plane_info[i].v_scale_initial_phase /= 2;
        plane_info[i].h_scale_ratio /= 2;
        plane_info[i].v_scale_ratio /= 2;
      }
#endif
      if (cpp_hw_params_is_dsdn_enabled(cpphw, hw_params, i))
        plane_info[i].dsdn_enable = hw_params->dsdn_enable;
      if(i > 0) {
        /* Enable CPP HW CDS for Chroma plane */
        /* Update the dsdn value for previous frame for hysteresis */
        switch (hw_params->input_info.plane_fmt) {
        case CPP_PARAM_PLANE_CRCB420: {
          plane_info[i].src_stride = (plane_info[i].src_stride + 31)&~31;
          plane_info[i].src_stride /= 2;
        case CPP_PARAM_PLANE_CBCR_UBWC:
        case CPP_PARAM_PLANE_CRCB_UBWC:
        case CPP_PARAM_PLANE_CRCB:
        case CPP_PARAM_PLANE_CBCR: {
          plane_info[i].src_height /= 2;
          plane_info[i].v_scale_initial_phase /= 2;
          plane_info[i]. maximum_src_stripe_height =
            PAD_TO_2(plane_info[i]. maximum_src_stripe_height/2);
          fmt_conv_vscale_ratio /= 2;
        case CPP_PARAM_PLANE_CRCB422:
        case CPP_PARAM_PLANE_CBCR422: {
          plane_info[i].src_width /= 2;
          plane_info[i].h_scale_initial_phase /= 2;
          fmt_conv_hscale_ratio /= 2;
          break;
        }
        }
        }
        case CPP_PARAM_PLANE_CRCB444:
        case CPP_PARAM_PLANE_CBCR444:
          plane_info[i].src_stride *= 2;
          break;
        default:
          break;
        }

        switch (hw_params->output_info.plane_fmt) {
        case CPP_PARAM_PLANE_CRCB420: {
          plane_info[i].dst_stride = (plane_info[i].dst_stride + 31)&~31;
          plane_info[i].dst_stride /= 2;
        case CPP_PARAM_PLANE_CBCR_UBWC:
        case CPP_PARAM_PLANE_CRCB_UBWC:
        case CPP_PARAM_PLANE_CRCB:
        case CPP_PARAM_PLANE_CBCR: {
            plane_info[i].dst_height /= 2;
            plane_info[i].dst_width /= 2;
            plane_info[i].dst_y_offset /= 2;
            plane_info[i].dst_x_offset /= 2;
            plane_info[i].maximum_dst_stripe_height =
              PAD_TO_2(plane_info[i].maximum_dst_stripe_height / 2);
            fmt_conv_vscale_ratio *= 2;
            fmt_conv_hscale_ratio *= 2;
            break;
        }
        }
        case CPP_PARAM_PLANE_CRCB422:
        case CPP_PARAM_PLANE_CBCR422: {
          if((hw_params->rotation == 0) || (hw_params->rotation == 2)) {
            plane_info[i].dst_width /= 2;
            plane_info[i].dst_x_offset /= 2;
            fmt_conv_hscale_ratio *= 2;
          } else if ((hw_params->rotation == 1) || (hw_params->rotation == 3)) {
            plane_info[i].dst_height /= 2;
            plane_info[i].dst_y_offset /= 2;
            fmt_conv_vscale_ratio *= 2;
          }
          break;
        }
        case CPP_PARAM_PLANE_CRCB444:
        case CPP_PARAM_PLANE_CBCR444:
          plane_info[i].dst_stride *= 2;
          break;
        default:
          break;
        }

        plane_info[i].h_scale_ratio *= fmt_conv_hscale_ratio;
        plane_info[i].v_scale_ratio *= fmt_conv_vscale_ratio;

        CPP_FRAME_DBG("stream_type %d,ds mask %d,scale %d",
          hw_params->stream_type, hw_params->uv_upsample_enable,
          hw_params->downsample_mask);
        if ((hw_params->uv_upsample_enable) && (hw_params->downsample_mask)) {
          plane_info[i].src_width /= 2;
          plane_info[i].src_height /= 2;
          plane_info[i].h_scale_initial_phase /= 2;
          plane_info[i].v_scale_initial_phase /= 2;
          plane_info[i].h_scale_ratio /= 2;
          plane_info[i].v_scale_ratio /= 2;
        }

        plane_info[i].postscale_padding = 0;
        if((hw_params->output_info.plane_fmt == CPP_PARAM_PLANE_CRCB420) &&
          (hw_params->input_info.plane_fmt != CPP_PARAM_PLANE_CRCB420)) {
          plane_info[i].destination_address[1] =
            plane_info[0].maximum_dst_stripe_height *
            plane_info[i-1].dst_stride * i +
            hw_params->output_info.plane_info[i-1].plane_offsets;
          plane_info[i].destination_address[0] =
            (plane_info[i].dst_stride) *
            plane_info[i].maximum_dst_stripe_height;
          if(hw_params->output_info.plane_info[i].plane_offsets)
            plane_info[i].destination_address[0] +=
              hw_params->output_info.plane_info[i].plane_offsets;
          else
            plane_info[i].destination_address[0] +=
              plane_info[i].destination_address[1];
        }
      }

      CPP_FRAME_HIGH("type %d, ### i %d, src %d, dst %d, %d, tmpral_src %d,"
        "tmporal_dst %d", hw_params->stream_type, i,
        plane_info[i].source_address[0], plane_info[i].destination_address[0],
        plane_info[i].destination_address[1],
        plane_info[i].temporal_source_address[0],
        plane_info[i].temporal_destination_address[0]);

      if (plane_info[i].h_scale_ratio < 1)
        plane_info[i].h_scale_initial_phase +=
          (plane_info[i].h_scale_ratio - 1) / 2;
      if (plane_info[i].v_scale_ratio < 1)
        plane_info[i].v_scale_initial_phase +=
          (plane_info[i].v_scale_ratio - 1) / 2;
      if (cpp_hw_params_is_lln_enabled(cpphw, hw_params, i)) {
        cpp_hardware_prepare_frame(cpphw, hw_params, frame_info, i);
        cace_inval = true;
        cace_clean = true;
      }
  }

  CDBG_HIGH("CAMCACHE : Input : (vaddr=%p, %p, fd=%d, len=%d), "
    "ion_fd=%d, buffer_access=%s(0x%x)",
    hw_params->vaddr, hw_params->buffer_info.vaddr, hw_params->buffer_info.fd,
    hw_params->input_info.frame_len, pp_buf_mgr->ion_fd,
    (hw_params->buffer_info.buffer_access == 0) ? "NONE" :
    (hw_params->buffer_info.buffer_access == CPU_HAS_READ) ? "READ" :
    (hw_params->buffer_info.buffer_access == CPU_HAS_WRITTEN) ? "WRITTEN" :
    "READ_WRITTEN", hw_params->buffer_info.buffer_access);

  // cpp is not going to modify the input buffer, so we dont
  // really need to invalidate the cache i.e cache contents are still valid.
  // cpp is going to read the input buffer. So if a previous module wrote
  // something into the cache which has not been flushed yet,
  // we should make sure we flush it here.

  if ((hw_params->buffer_info.buffer_access & CPU_HAS_WRITTEN) ==
    CPU_HAS_WRITTEN) {
    cace_clean = true;
    hw_params->buffer_info.buffer_access &= ~CPU_HAS_WRITTEN;

    // We are not doing cache clean, so make sure to set
    // MSM_CAMERA_BUF_FLAG_CPU_HAS_READ flag to inform that the buffer
    // is still cached
    hw_params->buffer_info.buffer_access |= CPU_HAS_READ;
  }

  if (cace_clean || cace_inval) {
    pp_buf_mgr__cache_ops_type cache_ops;

    if (cace_clean && cace_inval) {
      cache_ops = PP_BUF_MGR_CACHE_CLEAN_INVALIDATE;
    } else if (cace_clean) {
      cache_ops = PP_BUF_MGR_CACHE_CLEAN;
    } else {
      cache_ops = PP_BUF_MGR_CACHE_INVALIDATE;
    }

    pp_native_buf_mgr_cacheops(pp_buf_mgr, hw_params->vaddr, hw_params->buffer_info.fd,
      hw_params->input_info.frame_len, cache_ops);
  }

  if (plane_info[0].tnr_enable && hw_params->grey_ref_enable) {
    if (hw_params->tnr_scratch_bfr_array.buff_array[0].vaddr) {
      memset(hw_params->tnr_scratch_bfr_array.buff_array[0].vaddr, 0x80,
        hw_params->tnr_scratch_bfr_array.buff_array[0].alloc_len);
      pp_native_buf_mgr_cacheops(pp_buf_mgr,
        hw_params->tnr_scratch_bfr_array.buff_array[0].vaddr,
        hw_params->tnr_scratch_bfr_array.buff_array[0].fd,
        hw_params->tnr_scratch_bfr_array.buff_array[0].alloc_len,
        PP_BUF_MGR_CACHE_CLEAN_INVALIDATE);
    }
  }

  if ((hw_params->input_info.plane_fmt == CPP_PARAM_PLANE_CRCB) ||
    (hw_params->input_info.plane_fmt == CPP_PARAM_PLANE_CRCB422) ||
    (hw_params->input_info.plane_fmt == CPP_PARAM_PLANE_CRCB420) ||
    (hw_params->input_info.plane_fmt == CPP_PARAM_PLANE_CRCB444)) {
    /* Swap the chroma planes */
    chroma_1 = 2;
    chroma_2 = 1;
  }

  for (k = 0; k < CPP_DENOISE_NUM_PROFILES; k++) {
    frame_info->noise_profile[0][k] =
      hw_params->denoise_info[0][k].noise_profile;
    frame_info->noise_profile[chroma_1][k] =
      hw_params->denoise_info[1][k].noise_profile;
    frame_info->noise_profile[chroma_2][k] =
      hw_params->denoise_info[2][k].noise_profile;
    for (j = 0; j < CPP_DENOISE_NUM_PLANES; j++) {
      frame_info->weight[j][k] =
        hw_params->denoise_info[j][k].weight;
      frame_info->denoise_ratio[j][k] =
        hw_params->denoise_info[j][k].denoise_ratio;
    }
  }
  for (j = 0; j < CPP_DENOISE_NUM_PLANES; j++) {
    for (k = 0; k < BILITERAL_LAYERS; k++) {
      frame_info->edge_softness[j][k] =
        hw_params->denoise_info[j][k].edge_softness;
    }
  }
  cpp_params_add_wnr_adj(hw_params, frame_info);

  frame_info->pbf_noise_profile[0] =
    hw_params->pbf_denoise_info[0].noise_profile;
  frame_info->pbf_noise_profile[chroma_1] =
    hw_params->pbf_denoise_info[1].noise_profile;
  frame_info->pbf_noise_profile[chroma_2] =
    hw_params->pbf_denoise_info[2].noise_profile;
  for (j = 0; j < CPP_DENOISE_NUM_PLANES; j++) {
    frame_info->pbf_weight[j] =
      hw_params->pbf_denoise_info[j].weight;
    frame_info->pbf_denoise_ratio[j] =
      hw_params->pbf_denoise_info[j].denoise_ratio;
    frame_info->pbf_edge_softness[j] =
      hw_params->pbf_denoise_info[j].edge_softness;
    frame_info->pbf_profile_adj[j] =
      hw_params->pbf_denoise_info[j].profile_adj[ISP_SCALOR] *
      hw_params->pbf_denoise_info[j].profile_adj[CPP_SCALOR];
    frame_info->pbf_weight_VFE_adj[j] =
      hw_params->pbf_denoise_info[j].weight_VFE_adj[ISP_SCALOR] *
      hw_params->pbf_denoise_info[j].weight_VFE_adj[CPP_SCALOR];
  }

  if ((cpphw->hwinfo.caps & TNR_CAPS)) {
    cpp_prepare_tnr_bf_info(frame_info,hw_params);
  }

  frame_info->asf_mode = hw_params->asf_mode;

  if (hw_params->diagnostic_enable) {
    if (hw_params->ez_tune_asf_enable) {
        if (hw_params->sharpness_level > 0.0f)
          frame_info->asf_mode = CPP_PARAM_ASF_DUAL_FILTER;
    }
    else
      frame_info->asf_mode = CPP_PARAM_ASF_OFF;
  }

  frame_info->sharpness_ratio = hw_params->sharpness_level;
   asf_info->sp = hw_params->asf_info.sp;
  asf_info->neg_abs_y1 = hw_params->asf_info.neg_abs_y1;
  asf_info->dyna_clamp_en = hw_params->asf_info.dyna_clamp_en;
  asf_info->sp_eff_en = hw_params->asf_info.sp_eff_en;
  asf_info->clamp_h_ul = hw_params->asf_info.clamp_h_ul;
  asf_info->clamp_h_ll = hw_params->asf_info.clamp_h_ll;
  asf_info->clamp_v_ul = hw_params->asf_info.clamp_v_ul;
  asf_info->clamp_v_ll = hw_params->asf_info.clamp_v_ll;
  asf_info->clamp_tl_ul = hw_params->asf_info.clamp_tl_ul;
  asf_info->clamp_tl_ll = hw_params->asf_info.clamp_tl_ll;
  asf_info->clamp_offset_max = hw_params->asf_info.clamp_offset_max;
  asf_info->clamp_offset_min = hw_params->asf_info.clamp_offset_min;
  asf_info->clamp_scale_max = hw_params->asf_info.clamp_scale_max;
  asf_info->clamp_scale_min = hw_params->asf_info.clamp_scale_min;
  asf_info->nz_flag = hw_params->asf_info.nz_flag;
  asf_info->nz_flag_f2 = hw_params->asf_info.nz_flag_f2;
  asf_info->nz_flag_f3_f5 = hw_params->asf_info.nz_flag_f3_f5;
  asf_info->checksum_en = hw_params->asf_info.checksum_enable;
  asf_info_filter_k_entries = cpp_hw_params_get_num_k_entries(cpphw->fw_version);
  for (i = 0; i < asf_info_filter_k_entries; i++) {
    /* Update F1 kernel */
    asf_info->sobel_h_coeff[i] = hw_params->asf_info.sobel_h_coeff[i];
    /* Update F2 kernel */
    asf_info->sobel_v_coeff[i] = hw_params->asf_info.sobel_v_coeff[i];
    /* Update F3 kernel */
    asf_info->hpf_h_coeff[i] = hw_params->asf_info.hpf_h_coeff[i];
    /* Update F4 kernel */
    asf_info->hpf_v_coeff[i] = hw_params->asf_info.hpf_v_coeff[i];
    /* Update F5 kernel */
    asf_info->lpf_coeff[i] = hw_params->asf_info.lpf_coeff[i];
  }

  for (i = 0; i < CPP_ASF_LUT12_ENTRIES_1_2_x; i++) {
    asf_info->lut1[i] = hw_params->asf_info.lut1[i];
    asf_info->lut2[i] = hw_params->asf_info.lut2[i];
  }
  for (i = 0; i < CPP_ASF_LUT3_ENTRIES_1_2_x; i++) {
    asf_info->lut3[i] = hw_params->asf_info.lut3[i];
  }

  for (i = 0; i < 3; i ++) {
    frame_info->clamp_limit_LL[i] = hw_params->lower_clamp_limit[i];
    frame_info->clamp_limit_UL[i] = hw_params->upper_clamp_limit[i];
  }
  frame_info->num_planes = plane_count;

  switch(hw_params->input_info.plane_fmt) {
  case CPP_PARAM_PLANE_CRCB444:
  case CPP_PARAM_PLANE_CRCB:
  case CPP_PARAM_PLANE_CRCB422:
    frame_info->in_plane_fmt = PLANE_CRCB;
    break;
  case CPP_PARAM_PLANE_CBCR444:
  case CPP_PARAM_PLANE_CBCR:
  case CPP_PARAM_PLANE_CBCR422:
    frame_info->in_plane_fmt = PLANE_CBCR;
    break;
  case CPP_PARAM_PLANE_CRCB420:
    frame_info->in_plane_fmt = PLANE_Y;
    break;
  case CPP_PARAM_PLANE_Y:
    frame_info->in_plane_fmt = PLANE_Y;
    break;
  default:
    CPP_ERR("bad input format");
    frame_info->in_plane_fmt = PLANE_CBCR;
    break;
  }

  switch(hw_params->output_info.plane_fmt) {
  case CPP_PARAM_PLANE_CRCB444:
  case CPP_PARAM_PLANE_CRCB:
  case CPP_PARAM_PLANE_CRCB422:
  case CPP_PARAM_PLANE_CRCB_UBWC:
    frame_info->out_plane_fmt = PLANE_CRCB;
    break;
  case CPP_PARAM_PLANE_CBCR444:
  case CPP_PARAM_PLANE_CBCR:
  case CPP_PARAM_PLANE_CBCR422:
  case CPP_PARAM_PLANE_CBCR_UBWC:
    frame_info->out_plane_fmt = PLANE_CBCR;
    break;
  case CPP_PARAM_PLANE_CRCB420:
    frame_info->out_plane_fmt = PLANE_Y;
    break;
  case CPP_PARAM_PLANE_Y:
    frame_info->out_plane_fmt = PLANE_Y;
    break;
  default:
    CPP_ERR("bad output format");
    frame_info->out_plane_fmt = PLANE_CBCR;
    break;
  }

  frame_info->we_disable = hw_params->we_disable;

  /* Power collapse enable */
  property_get("vendor.camera.disable.cpp.power.collapse", value, "1");
  if (atoi(value)) {
    frame_info->power_collapse = DISABLE_POWER_COLLAPSE;
  }

  CPP_DENOISE_LOW("bf_enable:%d", hw_params->denoise_enable);
  return 0;
}

boolean cpp_hardware_validate_params(cpp_hardware_params_t *hw_params)
{

  CPP_LOW("inw=%d, inh=%d, outw=%d, outh=%d",
    hw_params->input_info.width, hw_params->input_info.height,
    hw_params->output_info.width, hw_params->output_info.height);
  CPP_LOW("inst=%d, insc=%d, outst=%d, outsc=%d",
    hw_params->input_info.stride, hw_params->input_info.scanline,
    hw_params->output_info.stride, hw_params->output_info.scanline);

  if (hw_params->input_info.width <= 0 ||
    hw_params->input_info.height <= 0 ||
    hw_params->output_info.width <= 0 ||
    hw_params->output_info.height <= 0) {
    CPP_ERR("ide %x rotation %d input dim %d x %d out dim %d x %d\n",
      hw_params->identity, hw_params->rotation,
      hw_params->input_info.width, hw_params->input_info.height,
      hw_params->output_info.width, hw_params->output_info.height);
    return FALSE;
  }
  /* TODO: add mode sanity checks */
  return TRUE;
}


boolean cpp_hardware_rotation_swap(cpp_hardware_params_t *hw_params,
  uint32_t video_type)
{
  int32_t   swap_dim;

  if((hw_params->rotation == 1) || (hw_params->rotation == 3)) {
    if(!video_type) {
      swap_dim = hw_params->output_info.stride;
      hw_params->output_info.stride = hw_params->output_info.scanline;
      hw_params->output_info.scanline = swap_dim;
    } else {
      swap_dim = hw_params->output_info.width;
      hw_params->output_info.width = hw_params->output_info.height;
      hw_params->output_info.height = swap_dim;
    }
  }

  return TRUE;
}

/** cpp_hw_params_init_out_clamp:
 * @hw_params - structure holding the data for CPP hardware
 *
 * Description:
 *      The function loads default values to CPP output clamps
 **/
uint32_t cpp_hw_params_init_out_clamp(cpp_hardware_params_t *hw_params)
{
  uint32_t i;

  if (!hw_params) {
    CPP_ERR("null hw_params, failed");
    return -EINVAL;
  }

  for (i = 0; i < 3; i ++) {
    if(!hw_params->color_xform_is_on) {
      hw_params->lower_clamp_limit[i] = 0;
      hw_params->upper_clamp_limit[i] = 255;
    } else {
      hw_params->lower_clamp_limit[i] = 16;
      if(i == 0) {
        hw_params->upper_clamp_limit[i] = 235;
      } else {
        hw_params->upper_clamp_limit[i] = 240;
      }
    }
  }

  return 0;

}

/** cpp_hw_params_update_out_clamp:
 * @module_chromatix - pointer to the structure that holds all
 *                     chromatix pointers
 * @hw_params - structure holding the data for CPP hardware
 *
 * Description:
 *      The function loads chromatix values of CPP output clamps to the hardware
 *      settings.
 **/
uint32_t cpp_hw_params_update_out_clamp(modulesChromatix_t *module_chromatix,
  cpp_hardware_params_t *hw_params)
{
  chromatix_cpp_type *        chromatix_cpp = NULL;

  if (!hw_params) {
    CPP_ERR("null hw params, failed");
    return -EINVAL;
  }

  if (!module_chromatix) {
    CPP_ERR("null module chromatix, failed");
    return -1;
  }
  if (!module_chromatix->chromatixCppPtr) {
    cpp_hw_params_init_out_clamp(hw_params);
    return 0;
  }
  chromatix_cpp = (chromatix_cpp_type *)module_chromatix->chromatixCppPtr;

  hw_params->lower_clamp_limit[0] =
    chromatix_cpp->chromatix_clamp_data.lower_clamp_Y;
  hw_params->upper_clamp_limit[0] =
    chromatix_cpp->chromatix_clamp_data.upper_clamp_Y;
  hw_params->lower_clamp_limit[1] =
    chromatix_cpp->chromatix_clamp_data.lower_clamp_Cb;
  hw_params->upper_clamp_limit[1] =
    chromatix_cpp->chromatix_clamp_data.upper_clamp_Cb;
  hw_params->lower_clamp_limit[2] =
    chromatix_cpp->chromatix_clamp_data.lower_clamp_Cr;
  hw_params->upper_clamp_limit[2] =
    chromatix_cpp->chromatix_clamp_data.upper_clamp_Cr;

  return 0;
}


/* TODO: this function does not support fw 1_6_x for APQ8084 */
void cpp_hw_params_copy_asf_diag_params(struct cpp_frame_info_t *frame_info,
  asfsharpness7x7_t *diag_cpyto_params)
{
  uint32_t i;
  struct cpp_asf_info *diag_cpyfrm_params = &frame_info->asf_info;

  /* Copy the members from hw_params to diagnostics structure */
  diag_cpyto_params->smoothpercent =
    (int32_t)Round(diag_cpyfrm_params->sp * (1 << 4));
  diag_cpyto_params->neg_abs_y1 = diag_cpyfrm_params->neg_abs_y1;
  diag_cpyto_params->dyna_clamp_en = diag_cpyfrm_params->dyna_clamp_en;
  diag_cpyto_params->sp_eff_en = diag_cpyfrm_params->sp_eff_en;
  diag_cpyto_params->clamp_hh = diag_cpyfrm_params->clamp_h_ul;
  diag_cpyto_params->clamp_hl = diag_cpyfrm_params->clamp_h_ll;
  diag_cpyto_params->clamp_vh = diag_cpyfrm_params->clamp_v_ul;
  diag_cpyto_params->clamp_vl = diag_cpyfrm_params->clamp_v_ll;
  diag_cpyto_params->clamp_scale_max =
    (int32_t)Round(diag_cpyfrm_params->clamp_scale_max * (1 << 4));
  diag_cpyto_params->clamp_scale_min =
    (int32_t)Round(diag_cpyfrm_params->clamp_scale_min * (1 << 4));
  diag_cpyto_params->clamp_offset_max = diag_cpyfrm_params->clamp_offset_max;
  diag_cpyto_params->clamp_offset_min = diag_cpyfrm_params->clamp_offset_min;
  diag_cpyto_params->nz_flag = diag_cpyfrm_params->nz_flag;
  for (i = 0; i < CPP_ASF_F_KERNEL_ENTRIES_1_2_x; i++) {
    diag_cpyto_params->sobel_h_coeff[i] =
      (int32_t)(Round(diag_cpyfrm_params->sobel_h_coeff[i]*(1<<10)));
    diag_cpyto_params->sobel_v_coeff[i] =
      (int32_t)(Round(diag_cpyfrm_params->sobel_v_coeff[i]*(1<<10)));
    diag_cpyto_params->hpf_h_coeff[i] =
      (int32_t)(Round(diag_cpyfrm_params->hpf_h_coeff[i]*(1<<10)));
    diag_cpyto_params->hpf_v_coeff[i] =
      (int32_t)(Round(diag_cpyfrm_params->hpf_v_coeff[i]*(1<<10)));
    diag_cpyto_params->lpf_coeff[i] =
      (int32_t)(Round(diag_cpyfrm_params->lpf_coeff[i]*(1<<10)));
  }

  for (i = 0; i < CPP_ASF_LUT12_ENTRIES_1_2_x; i++) {
    diag_cpyto_params->lut1[i] =
      (int32_t)(Round((diag_cpyfrm_params->lut1[i])*(1<<5)));
    diag_cpyto_params->lut2[i] =
      (int32_t)(Round((diag_cpyfrm_params->lut2[i])*(1<<5)));
  }

  for (i = 0; i < CPP_ASF_LUT3_ENTRIES_1_2_x; i++) {
    diag_cpyto_params->lut3[i] =
      (int32_t)(Round(diag_cpyfrm_params->lut3[i]*(1<<6)));
  }

  return;
}

void cpp_hw_params_copy_asf9x9_diag_params(struct cpp_frame_info_t *frame_info,
  asfsharpness9x9_t *diag_cpyto_params)
{
  int i;
  cpp_1_6_x_asf_info_t *diag_cpyfrm_params =
    &frame_info->u_asf.info_1_6_x;

  diag_cpyto_params->sp = diag_cpyfrm_params->sp;
  for (i = 0; i < 25; i++) {
    diag_cpyto_params->sobel_H_coeff[i] =
      diag_cpyfrm_params->sobel_h_coeff[i];
    diag_cpyto_params->sobel_se_diagonal_coeff[i] =
      diag_cpyfrm_params->sobel_v_coeff[i] =
    diag_cpyto_params->hpf_h_coeff[i] =
      diag_cpyfrm_params->hpf_h_coeff[i];
    diag_cpyto_params->hpf_se_diagonal_coeff[i] =
      diag_cpyfrm_params->hpf_v_coeff[i];
  }
  for (i = 0; i < 15; i++) {
    diag_cpyto_params->lpf_coeff[i] =
      diag_cpyfrm_params->lpf_coeff[i];
    diag_cpyto_params->hpf_symmetric_coeff[i] =
      diag_cpyfrm_params->hpf_sym_coeff[i];
  }
  for (i = 0; i < 6; i++) {
    diag_cpyto_params->activity_lpf_coeff[i] =
      diag_cpyfrm_params->activity_lpf_coeff[i];
    diag_cpyto_params->activity_band_pass_coeff[i] =
      diag_cpyfrm_params->activity_bpf_coeff[i];
  }

  diag_cpyto_params->gain_cap = diag_cpyfrm_params->gain_cap;
  diag_cpyto_params->gamma_corrected_luma_target =
    diag_cpyfrm_params->gamma_cor_luma_target;
  diag_cpyto_params->en_dyna_clamp = diag_cpyfrm_params->dyna_clamp_en;
  diag_cpyto_params->smax = diag_cpyfrm_params->clamp_scale_max;
  diag_cpyto_params->omax = diag_cpyfrm_params->clamp_offset_max;
  diag_cpyto_params->smin = diag_cpyfrm_params->clamp_scale_min;
  diag_cpyto_params->omin = diag_cpyfrm_params->clamp_offset_min;
  diag_cpyto_params->clamp_UL = diag_cpyfrm_params->clamp_tl_ul;
  diag_cpyto_params->clamp_LL = diag_cpyfrm_params->clamp_tl_ll;
  diag_cpyto_params->perpendicular_scale_factor =
    diag_cpyfrm_params->perpen_scale_factor;
  diag_cpyto_params->max_value_threshold =
    diag_cpyfrm_params->max_val_threshold;
  diag_cpyto_params->norm_scale = diag_cpyfrm_params->norm_scale;
  diag_cpyto_params->activity_clamp_threshold =
    diag_cpyfrm_params->activity_clamp_threshold;
  diag_cpyto_params->L2_norm_en = diag_cpyfrm_params->L2_norm_en;
  diag_cpyto_params->median_blend_upper_offset =
    diag_cpyfrm_params->median_blend_offset;
  diag_cpyto_params->median_blend_lower_offset =
    diag_cpyfrm_params->median_blend_lower_offset;
  return;
}

void cpp_hw_params_copy_wnr_diag_params(struct cpp_frame_info_t *frame_info,
  wavelet_t *diag_cpyto_params)
{
  uint32_t i;
  for (i = 0; i < CPP_DENOISE_NUM_PROFILES; i++) {
    //Q10 and Q8 format for bilateral_scale is calculated in cpp_prepare_bf_info()
    if (i < BILITERAL_LAYERS) {
      /* Plane 1 */
      diag_cpyto_params->bilateral_scalecore0[i] =
        (int32_t)frame_info->bf_info[0].bilateral_scale[i];
      /* Plane 2 */
      diag_cpyto_params->bilateral_scalecore1[i] =
        (int32_t)frame_info->bf_info[1].bilateral_scale[i];
      /* Plane 3 */
      diag_cpyto_params->bilateral_scalecore2[i] =
        (int32_t)frame_info->bf_info[2].bilateral_scale[i];
    }
    /* Plane 1 */
    diag_cpyto_params->weightcore0[i] =
      (int32_t)(frame_info->bf_info[0].weight[i] * (1 << 4));
    diag_cpyto_params->noise_thresholdcore0[i] =
      (int32_t)(frame_info->bf_info[0].noise_threshold[i] * (1 << 4));

    /* Plane 2 */
    diag_cpyto_params->weightcore1[i] =
      (int32_t)(frame_info->bf_info[1].weight[i] * (1 << 4));
    diag_cpyto_params->noise_thresholdcore1[i] =
      (int32_t)(frame_info->bf_info[1].noise_threshold[i] * (1 << 4));

    /* Plane 3 */
    diag_cpyto_params->weightcore2[i] =
      (int32_t)(frame_info->bf_info[2].weight[i] * (1 << 4));
    diag_cpyto_params->noise_thresholdcore2[i] =
      (int32_t)(frame_info->bf_info[2].noise_threshold[i] * (1 << 4));
  }
  return;
}

uint32_t cpp_hw_params_get_num_k_entries
  (cpp_firmware_version_t fw_version)
{
  uint32_t k_entries = 0;
  switch(fw_version) {
    case CPP_FW_VERSION_1_2_0:
    case CPP_FW_VERSION_1_4_0:
      k_entries = CPP_ASF_F_KERNEL_ENTRIES_1_4_x;
      break;
    case CPP_FW_VERSION_1_5_0:
    case CPP_FW_VERSION_1_5_1:
    case CPP_FW_VERSION_1_5_2:
      k_entries = CPP_ASF_F_KERNEL_ENTRIES_1_5_x;
      break;
    case CPP_FW_VERSION_1_6_0:
    case CPP_FW_VERSION_1_8_0:
    case CPP_FW_VERSION_1_10_0:
    case CPP_FW_VERSION_1_12_0:
    case CPP_FW_VERSION_1_12_1:
      k_entries = CPP_ASF_F_KERNEL_ENTRIES_1_6_x;
      break;
    default:
      CPP_ERR("Invalid FW version 0x%x,set to default filter kernel 16",
        fw_version);
      k_entries = 16;
      break;
  }
  return k_entries;
}

/** cpp_hardware_prepare_frame
 *
 *  @cpphw - This parameter holds the cpp hardware data structure.
 *  @hw_params - This parameter holds the cpp hardware params data structure.
 *  @frame_info - This parameter holds the frame information data structure.
 *  @plane -  This parameter holds the plane - chroma or luma.
 *
 *  This function is used to update grey padding around the frame
 *
 *  Return: void.
 **/
void cpp_hardware_prepare_frame(cpp_hardware_t *cpphw,
  cpp_hardware_params_t *hw_params,
  struct cpp_frame_info_t *frame_info, uint32_t plane)
{
  uint32_t j;
  struct cpp_plane_info_t *plane_info = frame_info->plane_info;
  int8_t *frame_addr = (int8_t *)hw_params->vaddr ;

  CPP_FRAME_LOW("cpp_hardware_prepare_frame");

  if(frame_addr != NULL) {
    frame_addr += plane_info[plane].source_address[0] -
      (hw_params->input_info.plane_info[plane].plane_offset_x -
      cpphw->max_supported_padding);
    CPP_FRAME_DBG("[PREPARE_FRAME:] stream type %d plane  %d, width %d,"
      "height %d stride %d, offset x %d, offset y %d, supported padding %d,"
      "plane len %d, offset %d", hw_params->stream_type, plane,
      plane_info[plane].src_width, plane_info[plane].src_height,
      plane_info[plane].src_stride,
      hw_params->input_info.plane_info[plane].plane_offset_x,
      hw_params->input_info.plane_info[plane].plane_offset_y,
      cpphw->max_supported_padding,
      hw_params->input_info.plane_info[plane].plane_len,
      hw_params->input_info.plane_info[plane].plane_offsets);

    /* Set the top rim to 0x80 */
    CPP_FRAME_LOW("[PREPARE_FRAME:] start frame_addr %p", frame_addr);
    memset(frame_addr, 0x80,
      (cpphw->max_supported_padding * plane_info[plane].src_stride));

    frame_addr += ((cpphw->max_supported_padding * plane_info[plane].src_stride) +
      (hw_params->input_info.plane_info[plane].plane_offset_x -
      cpphw->max_supported_padding));

    CPP_FRAME_LOW("PREPARE_FRAME left frame_addr %p", frame_addr);
    /* start filling left and right rims with 0x80 */
    for (j = 0; j < (plane_info[plane].src_height); j++) {
      memset(frame_addr, 0x80, cpphw->max_supported_padding);
      memset((frame_addr + (plane_info[plane].src_width * (plane + 1)) +
        cpphw->max_supported_padding), 0x80,
        cpphw->max_supported_padding);

      frame_addr += plane_info[plane].src_stride;
    }
    frame_addr -= (hw_params->input_info.plane_info[plane].plane_offset_x -
      cpphw->max_supported_padding);
    CPP_FRAME_LOW("PREPARE_FRAME bottom frame_addr %p", frame_addr);
    /*set bottom rim to 0x80 */
    memset(frame_addr, 0x80, (plane_info[plane].src_stride *
      cpphw->max_supported_padding));
    plane_info[plane].src_width +=
      2 * cpphw->max_supported_padding / (plane+1);
    plane_info[plane].src_height +=
      2 * cpphw->max_supported_padding;
    plane_info[plane].h_scale_initial_phase +=
      cpphw->max_supported_padding / (plane + 1);
    plane_info[plane].v_scale_initial_phase +=
      cpphw->max_supported_padding;

    CPP_FRAME_DBG("PREPARE_FRAME: width %d, height %d, h phase %f, v phase %f",
      plane_info[plane].src_width, plane_info[plane].src_height,
      plane_info[plane].h_scale_initial_phase,
      plane_info[plane].v_scale_initial_phase);
  }
}
