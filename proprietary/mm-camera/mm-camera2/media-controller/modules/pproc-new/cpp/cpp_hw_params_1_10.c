/*============================================================================

  Copyright (c) 2014-2017 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

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


/** run_TW_logic_1_10:
 *
 *  @in_ary - This parameter holds the input data for the current stripe.
 *  @src_tile_index_x_counter - This counter shows the number of the width mcu
 *  @src_tile_index_y_counter - This counter shows the number of the height mcu
 *  @out_ary - This parameter holds the output data for the current stripe.
 *
 *  This function performs all calculations needed to setup the write engine
 *  for the current stripe.
 *
 *  Return: It is void function.
 **/
void run_TW_logic_1_10(struct cpp_plane_info_t* in_ary,
                              uint32_t src_tile_index_x_counter,
                              uint32_t src_tile_index_y_counter,
                              struct cpp_striping_algorithm_info* out_ary)
{
  uint32_t i;
  out_ary->dst_start_x = in_ary->dst_x_offset;
  out_ary->dst_start_y = in_ary->dst_y_offset;
  switch (in_ary->rotate) {
    /* 270 degree */
    case 3:
      if (in_ary->mirror & 0x01) {
        for (i = in_ary->frame_height_mcus - 1; i > src_tile_index_y_counter;
               i--) {
          out_ary->dst_start_x += in_ary->stripe_block_height[i];
        }
        out_ary->dst_end_x = out_ary->dst_start_x +
           in_ary->stripe_block_height[src_tile_index_y_counter] - 1;
        out_ary->we_h_step =  0 - in_ary->tile_byte_width;
        if (in_ary->tile_output_enable) {
          out_ary->we_h_init =
              (out_ary->dst_end_x + 1) * in_ary->output_bytes_per_pixel -
            in_ary->tile_byte_width;
        } else {
          out_ary->we_h_init =
              in_ary->stripe_block_height[src_tile_index_y_counter] -
             in_ary->tile_byte_width;
        }
      } else {
        for (i = 0; i < src_tile_index_y_counter; i++) {
          out_ary->dst_start_x += in_ary->stripe_block_height[i];
        }
        out_ary->dst_end_x = out_ary->dst_start_x +
           in_ary->stripe_block_height[src_tile_index_y_counter] - 1;
        out_ary->we_h_step =  in_ary->tile_byte_width;
        if (in_ary->tile_output_enable) {
          out_ary->we_h_init =
             out_ary->dst_start_x * in_ary->output_bytes_per_pixel;
        } else {
          out_ary->we_h_init =  0;
        }
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
      out_ary->we_v_step =
         in_ary->stripe_block_width[src_tile_index_x_counter];
      if (in_ary->tile_output_enable) {
        out_ary->we_v_init =  out_ary->dst_start_y;
      } else {
        out_ary->we_v_init = 0;
      }
      break;
    case 2:
      if (in_ary->mirror & 0x01) {
        for (i = 0; i < src_tile_index_x_counter; i++) {
          out_ary->dst_start_x += in_ary->stripe_block_width[i];
        }
        out_ary->dst_end_x = out_ary->dst_start_x +
           in_ary->stripe_block_width[src_tile_index_x_counter] - 1;
      } else {
        for (i = in_ary->frame_width_mcus - 1;
               i > src_tile_index_x_counter; i--) {
          out_ary->dst_start_x += in_ary->stripe_block_width[i];
        }
        out_ary->dst_end_x = out_ary->dst_start_x +
          in_ary->stripe_block_width[src_tile_index_x_counter] - 1;
      }
      for (i = in_ary->frame_height_mcus - 1;
             i > src_tile_index_y_counter; i--) {
        out_ary->dst_start_y += in_ary->stripe_block_height[i];
      }
      out_ary->dst_end_y = out_ary->dst_start_y +
         in_ary->stripe_block_height[src_tile_index_y_counter] - 1;
      out_ary->we_v_step =
         0 - in_ary->tile_byte_height;
      out_ary->we_h_step =
         in_ary->stripe_block_width[src_tile_index_x_counter] *
         in_ary->output_bytes_per_pixel;
      if (in_ary->tile_output_enable) {
        out_ary->we_h_init =  out_ary->dst_start_x *
           in_ary->output_bytes_per_pixel;
        out_ary->we_v_init =  out_ary->dst_end_y + 1 - in_ary->tile_byte_height;
      } else {
        out_ary->we_h_init =  0;
           in_ary->output_bytes_per_pixel;
        out_ary->we_v_init =
            in_ary->stripe_block_height[src_tile_index_y_counter] -
           in_ary->tile_byte_height;
      }
      break;
    /* 90 degree */
    case 1:
      if (in_ary->mirror & 0x01) {
        for (i = 0; i < src_tile_index_y_counter; i++) {
          out_ary->dst_start_x += in_ary->stripe_block_height[i];
        }
        out_ary->dst_end_x = out_ary->dst_start_x +
           in_ary->stripe_block_height[src_tile_index_y_counter] - 1;
        out_ary->we_h_step =  in_ary->tile_byte_width;
        if (in_ary->tile_output_enable) {
          out_ary->we_h_init =  out_ary->dst_start_x *
             in_ary->output_bytes_per_pixel;
        } else {
          out_ary->we_h_init =  0;
        }
      } else {
        for (i = in_ary->frame_height_mcus - 1;
               i > src_tile_index_y_counter; i--) {
          out_ary->dst_start_x += in_ary->stripe_block_height[i];
        }
        out_ary->dst_end_x = out_ary->dst_start_x +
           in_ary->stripe_block_height[src_tile_index_y_counter] - 1;
        out_ary->we_h_step =  0 - in_ary->tile_byte_width;
        if (in_ary->tile_output_enable) {
          out_ary->we_h_init =  (out_ary->dst_end_x + 1) *
             in_ary->output_bytes_per_pixel - in_ary->tile_byte_width;
        } else {
          out_ary->we_h_init =
              in_ary->stripe_block_height[src_tile_index_y_counter] *
             in_ary->output_bytes_per_pixel - in_ary->tile_byte_width;
        }
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
      out_ary->we_v_step =
            in_ary->stripe_block_width[src_tile_index_x_counter];
      if (in_ary->tile_output_enable) {
        out_ary->we_v_init =  out_ary->dst_start_y;
      } else {
        out_ary->we_v_init = 0;
      }
      break;
    case 0:
    default:
      if (in_ary->mirror & 0x01) {
        for (i = in_ary->frame_width_mcus - 1;
             i > src_tile_index_x_counter; i--) {
          out_ary->dst_start_x += in_ary->stripe_block_width[i];
        }
        out_ary->dst_end_x = out_ary->dst_start_x +
            in_ary->stripe_block_width[src_tile_index_x_counter] - 1;
      } else {
        for (i = 0; i < src_tile_index_x_counter; i++) {
          out_ary->dst_start_x += in_ary->stripe_block_width[i];
        }
        out_ary->dst_end_x = out_ary->dst_start_x +
            in_ary->stripe_block_width[src_tile_index_x_counter] - 1;
      }
      for (i = 0; i < src_tile_index_y_counter; i++) {
        out_ary->dst_start_y += in_ary->stripe_block_height[i];
      }
      out_ary->dst_end_y = out_ary->dst_start_y +
          in_ary->stripe_block_height[src_tile_index_y_counter] - 1;
      out_ary->we_v_step =  in_ary->tile_byte_height;
      out_ary->we_h_step =
          in_ary->stripe_block_width[src_tile_index_x_counter] *
          in_ary->output_bytes_per_pixel;
      if (in_ary->tile_output_enable) {
        out_ary->we_h_init =  out_ary->dst_start_x *
            in_ary->output_bytes_per_pixel;
        out_ary->we_v_init =  out_ary->dst_start_y;
      } else {
        out_ary->we_h_init =  0;
        out_ary->we_v_init =  0;
      }
      break;
  }
  if (in_ary->tile_output_enable) {
    out_ary->destination_address[0] = in_ary->destination_address[0];
    out_ary->destination_address[1] = in_ary->destination_address[1];
  } else {
    out_ary->destination_address[0] =
      in_ary->destination_address[0] + out_ary->dst_start_x *
      in_ary->output_bytes_per_pixel +
      out_ary->dst_start_y * in_ary->dst_stride;
    out_ary->destination_address[1] =
      in_ary->destination_address[1] + out_ary->dst_start_x *
      in_ary->output_bytes_per_pixel +
      out_ary->dst_start_y * in_ary->dst_stride;
  }

  /* temporal denoise state buffer*/
  if(in_ary->denoise_after_scale_en) {
    out_ary->temporal_dst_start_x = 0;
    out_ary->temporal_dst_start_y = 0;

    for (i = 0; i < src_tile_index_x_counter; i++) {
      out_ary->temporal_dst_start_x += in_ary->stripe_block_width[i];
    }
    out_ary->temporal_dst_end_x = out_ary->temporal_dst_start_x +
      in_ary->stripe_block_width[src_tile_index_x_counter] - 1;
    for (i = 0; i < src_tile_index_y_counter; i++) {
      out_ary->temporal_dst_start_y += in_ary->stripe_block_height[i];
    }
    out_ary->temporal_dst_end_y = out_ary->temporal_dst_start_y +
      in_ary->stripe_block_height[src_tile_index_y_counter] - 1;

    if((src_tile_index_x_counter != 0) || in_ary->pad_boundary) {
      out_ary->temporal_src_start_x = out_ary->temporal_dst_start_x -
        in_ary->postscale_padding;
    } else {
      out_ary->temporal_src_start_x = out_ary->temporal_dst_start_x;
    }
    if((src_tile_index_x_counter != in_ary->frame_width_mcus - 1) ||
      in_ary->pad_boundary) {
      out_ary->temporal_src_end_x = out_ary->temporal_dst_end_x +
        in_ary->postscale_padding;
    } else {
      out_ary->temporal_src_end_x = out_ary->temporal_dst_end_x;
    }
    if ((src_tile_index_y_counter != 0) || in_ary->pad_boundary) {
      out_ary->temporal_src_start_y = out_ary->temporal_dst_start_y -
        in_ary->postscale_padding;
    } else {
      out_ary->temporal_src_start_y = out_ary->temporal_dst_start_y;
    }
    if ((src_tile_index_y_counter != in_ary->frame_height_mcus - 1) ||
      in_ary->pad_boundary) {
      out_ary->temporal_src_end_y = out_ary->temporal_dst_end_y +
        in_ary->postscale_padding;
    } else {
      out_ary->temporal_src_end_y = out_ary->temporal_dst_end_y;
    }

    /* left boundary */
    if(out_ary->temporal_src_start_x < 0) {
      out_ary->temporal_pad_left = 0 - out_ary->temporal_src_start_x;
      out_ary->temporal_src_start_x = 0;
    } else {
      out_ary->temporal_pad_left = 0;
    }
    /* top boundary */
    if (out_ary->temporal_src_start_y < 0) {
      out_ary->temporal_pad_top = 0 - out_ary->temporal_src_start_y;
      out_ary->temporal_src_start_y = 0;
    } else {
      out_ary->temporal_pad_top = 0;
    }
    /* right boundary */
    if(out_ary->temporal_src_end_x >= in_ary->dst_width_block_aligned) {
      out_ary->temporal_pad_right = out_ary->temporal_src_end_x -
        (in_ary->dst_width_block_aligned - 1);
      out_ary->temporal_src_end_x = in_ary->dst_width_block_aligned-1;
    } else {
      out_ary->temporal_pad_right = 0;
    }
    /* bottom boundary */
    if (out_ary->temporal_src_end_y >= in_ary->dst_height_block_aligned) {
      out_ary->temporal_pad_bottom = out_ary->temporal_src_end_y -
        (in_ary->dst_height_block_aligned - 1);
      out_ary->temporal_src_end_y = in_ary->dst_height_block_aligned - 1;
    } else {
      out_ary->temporal_pad_bottom = 0;
    }
    /* The leftmost point of the fetch block in byte addresses */
  }
  out_ary->temporal_we_h_init =  0;
  out_ary->temporal_we_h_step =
    (out_ary->temporal_dst_end_x - out_ary->temporal_dst_start_x + 1) *
    in_ary->temporal_bytes_per_pixel;
  out_ary->temporal_we_v_init =  0;
  out_ary->temporal_we_v_step =  2;
  out_ary->temporal_destination_address[0] =
    in_ary->temporal_destination_address[0] +
    out_ary->temporal_dst_start_x * in_ary->temporal_bytes_per_pixel +
    out_ary->temporal_dst_start_y * in_ary->temporal_stride;
  out_ary->temporal_destination_address[1] =
    in_ary->temporal_destination_address[1] +
    out_ary->temporal_dst_start_x * in_ary->temporal_bytes_per_pixel +
    out_ary->temporal_dst_start_y * in_ary->temporal_stride;

  /* The leftmost point of the fetch block in byte addresses */
  out_ary->temporal_source_address[0] = in_ary->temporal_source_address[0] +
      out_ary->temporal_src_start_x * in_ary->temporal_bytes_per_pixel +
      out_ary->temporal_src_start_y * in_ary->temporal_stride;
  out_ary->temporal_source_address[1] = in_ary->temporal_source_address[1] +
      out_ary->temporal_src_start_x * in_ary->temporal_bytes_per_pixel +
      out_ary->temporal_src_start_y * in_ary->temporal_stride;
}

/** run_TF_logic_1_10:
 *
 *  @in_ary - This parameter holds the input data for the current stripe.
 *  @src_tile_index_x_counter - This counter shows the number of the width mcu
 *  @src_tile_index_y_counter - This counter shows the number of the height mcu
 *  @out_ary - This parameter holds the output data for the current stripe.
 *
 *  This function performs all calculations needed to setup the fetch engine
 *  for the current stripe.
 *
 *  Return: It is void function.
 **/
void run_TF_logic_1_10(struct cpp_plane_info_t* in_ary,
                              uint32_t src_tile_index_x_counter,
                              uint32_t src_tile_index_y_counter,
                              struct cpp_striping_algorithm_info* out_ary)
{
  /* set up information needed by phase accumulator */
  struct cpp_accumulated_phase_t phase_fg;
  if (src_tile_index_x_counter) {
    /* not the leftmost stripe */
    phase_fg.INIT_PHASE_X = in_ary->horizontal_scale_block_initial_phase;
  } else {
    /* the leftmost stripe */
    phase_fg.INIT_PHASE_X = in_ary->horizontal_scale_initial_phase;
  }
  if (src_tile_index_y_counter) {
    /* not the topmost block */
    phase_fg.INIT_PHASE_Y = in_ary->vertical_scale_block_initial_phase;
  } else {
    /* the topmost block */
    phase_fg.INIT_PHASE_Y = in_ary->vertical_scale_initial_phase;
  }

  phase_fg.phase_x_cur = in_ary->horizontal_scale_ratio *
    in_ary->dst_block_width * src_tile_index_x_counter + phase_fg.INIT_PHASE_X;
  phase_fg.phase_x_next = phase_fg.phase_x_cur +
     in_ary->horizontal_scale_ratio *
     in_ary->stripe_block_width[src_tile_index_x_counter];
  phase_fg.phase_y_cur = in_ary->vertical_scale_ratio *
     in_ary->dst_block_height * src_tile_index_y_counter + phase_fg.INIT_PHASE_Y;
  phase_fg.phase_y_next = phase_fg.phase_y_cur +
     in_ary->vertical_scale_ratio *
     in_ary->stripe_block_height[src_tile_index_y_counter];

  /* only the fractional bits are needed */
  out_ary->h_init_phase = (int32_t)phase_fg.phase_x_cur & q_mask;
  /* only the fractional bits are needed */
  out_ary->v_init_phase = (int32_t)phase_fg.phase_y_cur & q_mask;

  out_ary->h_phase_step = in_ary->horizontal_scale_ratio;
  out_ary->v_phase_step = in_ary->vertical_scale_ratio;

  if (in_ary->horizontal_scale_ratio < (1 << q_factor) ||
    (in_ary->horizontal_scale_ratio == (1 << q_factor) &&
    out_ary->h_init_phase != 0)) {
    /* that is, we are using upscaler for scaling in the x direction */
    out_ary->scale_h_en = 1;
    out_ary->upscale_h_en = 1;
    if (src_tile_index_x_counter || in_ary->pad_boundary) {
      /* not the leftmost stripe, need to fetch more on the left */
      int64_t temp = phase_fg.phase_x_cur -
        in_ary->postscale_padding * in_ary->horizontal_scale_ratio;
      /* position of the first pixel of the upscaler output */
      out_ary->h_init_phase = (int32_t)(temp & q_mask);
      /* only the fractional bits are needed */
      out_ary->src_start_x = (int32_t)(temp >> q_factor) -
        upscale_left_top_padding - in_ary->prescaler_padding;
    } else {
      /*
       * the leftmost stripe. left padding is
       * provided by denoise and sharpening but not upscaling
       */
      out_ary->h_init_phase = (int32_t)(phase_fg.phase_x_cur & q_mask);
      /* only the fractional bits are needed */
      out_ary->src_start_x = (int32_t)(phase_fg.phase_x_cur >> q_factor) -
         upscale_left_top_padding;
    }
    if ((src_tile_index_x_counter == in_ary->frame_width_mcus - 1) &&
      (!in_ary->pad_boundary)) {
      /*
       * the rightmost stripe, right padding is provided by
       * denoise and sharpening but not upscaling
       */
      out_ary->src_end_x =
        (int32_t) ((phase_fg.phase_x_next -
        in_ary->horizontal_scale_ratio) >> q_factor) +
        upscale_right_bottom_padding;
    } else {
      /* not the rightmost stripe. need to fetch more on the right */
      out_ary->src_end_x =
          (int32_t)((phase_fg.phase_x_next - in_ary->horizontal_scale_ratio +
          (in_ary->postscale_padding *  in_ary->horizontal_scale_ratio)) >>
          q_factor) + upscale_right_bottom_padding + in_ary->prescaler_padding;
    }
  } else if (in_ary->horizontal_scale_ratio > (1 << q_factor)) {
    /* horizontal downscaler */
    out_ary->scale_h_en = 1;
    out_ary->upscale_h_en = 0;
    if (src_tile_index_x_counter || in_ary->pad_boundary) {
      /*
       * not the leftmost stripe, need to fetch more on the left
       * position of the first pixel of the upscaler output
       */
      int64_t temp = phase_fg.phase_x_cur - in_ary->postscale_padding *
         in_ary->horizontal_scale_ratio;
      /* only the fractional bits are needed */
      out_ary->h_init_phase = (int32_t)(temp & q_mask);
      out_ary->src_start_x = (int32_t)(temp >> q_factor) - in_ary->prescaler_padding;
    } else {
      /* the leftmost stripe, left padding is done by denoise and sharpening */
      out_ary->h_init_phase = (int32_t)(phase_fg.phase_x_cur & q_mask);
      /* only the fractional bits are needed */
      out_ary->src_start_x = (int32_t)(phase_fg.phase_x_cur >> q_factor);
    }
    if ((src_tile_index_x_counter == in_ary->frame_width_mcus - 1) &&
      (!in_ary->pad_boundary)) {
      /*
       * the rightmost stripe, right padding is provided by denoise
       * and sharpening
       */
      if ((phase_fg.phase_x_next & q_mask) >>
        (q_factor - downscale_interpolation_resolution)) {
        out_ary->src_end_x = (int32_t)(phase_fg.phase_x_next >> q_factor);
      } else {
        /* need to fetch one pixel less */
        out_ary->src_end_x = (int32_t)(phase_fg.phase_x_next >> q_factor) - 1;
      }
    } else {
      /* not the rightmost stripe, need to fetch more on the right */
      int64_t temp = phase_fg.phase_x_next + in_ary->postscale_padding *
        in_ary->horizontal_scale_ratio;
      if ((temp & q_mask) >> (q_factor - downscale_interpolation_resolution)) {
        out_ary->src_end_x = (int32_t)(temp >> q_factor) +
          in_ary->prescaler_padding;
      } else {
        /* need to fetch one pixel less */
        out_ary->src_end_x = (int32_t)(temp >> q_factor) - 1 +
          in_ary->prescaler_padding;
      }
    }
  } else {
    /* that is, scaling is disabled in the x direction */
    out_ary->scale_h_en = 0;
    out_ary->upscale_h_en = 0;
    out_ary->h_init_phase = 0;
    /* only the fractional bits are needed */
    if (src_tile_index_x_counter || in_ary->pad_boundary) {
      /* not the leftmost stripe, need to fetch more on the left */
      out_ary->src_start_x = (int32_t)(phase_fg.phase_x_cur >> q_factor) -
         in_ary->postscale_padding - in_ary->prescaler_padding;
    } else {
      /*
       * the leftmost stripe, padding is done internally in denoise
       * and sharpening block
       */
      out_ary->src_start_x = (int32_t)(phase_fg.phase_x_cur >> q_factor);
    }
    if ((src_tile_index_x_counter == in_ary->frame_width_mcus - 1) &&
      (!in_ary->pad_boundary)) {
      /* the rightmost stripe, right padding is done internally in the HW */
      out_ary->src_end_x = (int32_t)(phase_fg.phase_x_next >> q_factor) - 1;
    } else {
      /* not the rightmost stripe, need to fetch more on the right */
      out_ary->src_end_x =
        (int32_t) (phase_fg.phase_x_next >> q_factor) - 1 +
        in_ary->postscale_padding + in_ary->prescaler_padding;
    }
  }

  if (src_tile_index_x_counter || in_ary->pad_boundary) {
    /* not the leftmost stripe, need to fetch more on the left */
    out_ary->prescaler_spatial_denoise_crop_width_first_pixel =
       in_ary->prescaler_spatial_denoise_padding;
    out_ary->temporal_denoise_crop_width_first_pixel =
       in_ary->temporal_denoise_padding;
    out_ary->spatial_denoise_crop_width_first_pixel =
       in_ary->spatial_denoise_padding;
    out_ary->sharpen_crop_width_first_pixel = in_ary->sharpen_padding;
    out_ary->state_crop_width_first_pixel = in_ary->state_padding;
  } else {
    /*
     * the leftmost stripe. left padding is
     * provided by denoise and sharpening but not upscaling
     */
    out_ary->spatial_denoise_crop_width_first_pixel = 0;
    out_ary->prescaler_spatial_denoise_crop_width_first_pixel = 0;
    out_ary->temporal_denoise_crop_width_first_pixel = 0;
    out_ary->sharpen_crop_width_first_pixel = 0;
    out_ary->state_crop_width_first_pixel = 0;
  }

  if (!in_ary->sharpen_before_scale) {
    out_ary->sharpen_crop_width_last_pixel =
        out_ary->sharpen_crop_width_first_pixel +
       in_ary->stripe_block_width[src_tile_index_x_counter] - 1;
  }
  /*
   * output width of the scaler should be the write
   * stripe width plus some padding required by sharpening
   */
  out_ary->scale_output_width =
     in_ary->stripe_block_width[src_tile_index_x_counter];
  if ((src_tile_index_x_counter != 0) || in_ary->pad_boundary) {
    out_ary->scale_output_width += in_ary->postscale_padding;
  }
  if ((src_tile_index_x_counter != in_ary->frame_width_mcus - 1) ||
    in_ary->pad_boundary) {
    out_ary->scale_output_width += in_ary->postscale_padding;
  }

  if ((src_tile_index_x_counter == in_ary->frame_width_mcus - 1) &&
    (!in_ary->pad_boundary)) {
      /*
       * the rightmost stripe, right padding is provided by denoise
       * and sharpening but not upscaling
       */
    out_ary->prescaler_spatial_denoise_crop_width_last_pixel =
       out_ary->src_end_x - out_ary->src_start_x;
    if (in_ary->denoise_after_scale_en) {
      if (in_ary->sharpen_before_scale) {
        out_ary->sharpen_crop_width_last_pixel =
          out_ary->sharpen_crop_width_first_pixel +
          in_ary->stripe_block_width[src_tile_index_x_counter] - 1;
      }
      out_ary->spatial_denoise_crop_width_last_pixel =
         out_ary->spatial_denoise_crop_width_first_pixel +
         out_ary->sharpen_crop_width_last_pixel;
      out_ary->temporal_denoise_crop_width_last_pixel =
         out_ary->temporal_denoise_crop_width_first_pixel +
         out_ary->spatial_denoise_crop_width_last_pixel;
      out_ary->state_crop_width_last_pixel =
        out_ary->state_crop_width_first_pixel +
         in_ary->stripe_block_width[src_tile_index_x_counter] - 1;
    } else {
      out_ary->temporal_denoise_crop_width_last_pixel =
        out_ary->prescaler_spatial_denoise_crop_width_last_pixel -
        out_ary->prescaler_spatial_denoise_crop_width_first_pixel;
      out_ary->spatial_denoise_crop_width_last_pixel =
        out_ary->temporal_denoise_crop_width_last_pixel -
        out_ary->temporal_denoise_crop_width_first_pixel;
      if (in_ary->sharpen_before_scale) {
        out_ary->sharpen_crop_width_last_pixel =
          out_ary->spatial_denoise_crop_width_last_pixel -
          out_ary->spatial_denoise_crop_width_first_pixel;
      }
      out_ary->state_crop_width_last_pixel =
        out_ary->spatial_denoise_crop_width_last_pixel;
    }
  } else {
     /* not the rightmost stripe. need to fetch more on the right */
    out_ary->prescaler_spatial_denoise_crop_width_last_pixel =
      out_ary->src_end_x - out_ary->src_start_x -
      in_ary->prescaler_spatial_denoise_padding;
    if (in_ary->denoise_after_scale_en) {
      if (in_ary->sharpen_before_scale) {
        out_ary->sharpen_crop_width_last_pixel =
          out_ary->sharpen_crop_width_first_pixel +
          in_ary->stripe_block_width[src_tile_index_x_counter] - 1;
      }
      out_ary->spatial_denoise_crop_width_last_pixel =
         out_ary->spatial_denoise_crop_width_first_pixel +
        out_ary->sharpen_crop_width_last_pixel + in_ary->sharpen_padding;
      out_ary->temporal_denoise_crop_width_last_pixel =
        out_ary->temporal_denoise_crop_width_first_pixel +
         out_ary->spatial_denoise_crop_width_last_pixel +
        in_ary->spatial_denoise_padding;
      out_ary->state_crop_width_last_pixel =
        out_ary->state_crop_width_first_pixel +
         in_ary->stripe_block_width[src_tile_index_x_counter] - 1;
    } else {
      out_ary->temporal_denoise_crop_width_last_pixel =
        out_ary->prescaler_spatial_denoise_crop_width_last_pixel -
        out_ary->prescaler_spatial_denoise_crop_width_first_pixel -
        in_ary->temporal_denoise_padding;
      out_ary->spatial_denoise_crop_width_last_pixel =
        out_ary->temporal_denoise_crop_width_last_pixel -
        out_ary->temporal_denoise_crop_width_first_pixel -
        in_ary->spatial_denoise_padding;
      if (in_ary->sharpen_before_scale) {
        out_ary->sharpen_crop_width_last_pixel =
          out_ary->spatial_denoise_crop_width_last_pixel -
          out_ary->spatial_denoise_crop_width_first_pixel -
          in_ary->sharpen_padding;
        out_ary->state_crop_width_last_pixel =
          out_ary->spatial_denoise_crop_width_last_pixel -
          in_ary->sharpen_padding;
      } else {
        out_ary->state_crop_width_last_pixel =
          out_ary->spatial_denoise_crop_width_last_pixel;
      }
    }
  }

  /* that is, we are using FIR for scaling in the y direction */
  if (in_ary->vertical_scale_ratio < (1 << q_factor)
       || (in_ary->vertical_scale_ratio == (1 << q_factor) &&
           out_ary->v_init_phase != 0)) {
    out_ary->scale_v_en = 1;
    out_ary->upscale_v_en = 1;
    if (src_tile_index_y_counter || in_ary->pad_boundary) {
      /* not the topmost block, need to fetch more on top */
      int64_t temp = phase_fg.phase_y_cur - in_ary->postscale_padding *
         in_ary->vertical_scale_ratio;
      /* only the fractional bits are needed */
      out_ary->v_init_phase =(int32_t)(temp & q_mask);
      out_ary->src_start_y = (int32_t)(temp >> q_factor) -
         upscale_left_top_padding -
         in_ary->prescaler_padding;
    } else {
      /*
       * the topmost block, top padding is done internally by denoise
       * and sharpening but not upscaling
       */
      out_ary->v_init_phase = (int32_t)(phase_fg.phase_y_cur & q_mask);
      /* only the fractional bits are needed */
      out_ary->src_start_y = (int32_t)((phase_fg.phase_y_cur) >> q_factor) -
         upscale_left_top_padding;
    }
    if ((src_tile_index_y_counter == in_ary->frame_height_mcus - 1) &&
      (!in_ary->pad_boundary)) {
       /*
        * the bottommost stripe, bottom padding is done internally
        * by denoise and sharpening but not upscaler
        */
      out_ary->src_end_y =
        (int32_t)((phase_fg.phase_y_next - in_ary->vertical_scale_ratio) >>
        q_factor) + upscale_right_bottom_padding;
    } else {
      /* not the bottommost stripe, need to fetch more on the bottom */
      out_ary->src_end_y =
          (int32_t) ((phase_fg.phase_y_next -
          in_ary->vertical_scale_ratio + (in_ary->postscale_padding *
          in_ary->vertical_scale_ratio)) >> q_factor) +
          upscale_right_bottom_padding + in_ary->prescaler_padding;
    }
  } else if (in_ary->vertical_scale_ratio > (1 << q_factor)) {
    /* vertical downscaler */
    out_ary->scale_v_en = 1;
    out_ary->upscale_v_en = 0;
    if (src_tile_index_y_counter || (in_ary->pad_boundary)) {
      /* not the topmost block, need to fetch more on the top */
      int64_t temp = phase_fg.phase_y_cur - in_ary->postscale_padding *
         in_ary->vertical_scale_ratio;
      out_ary->v_init_phase = (int32_t)(temp & q_mask);
      /* only the fractional bits are needed */
      out_ary->src_start_y = (int32_t)(temp >> q_factor) -
      in_ary->prescaler_padding;
    } else {
      /*
       * the topmost block, top padding internally done by denoise
       * and sharpening
       */
      out_ary->v_init_phase = (int32_t)(phase_fg.phase_y_cur & q_mask);
      /* only the fractional bits are needed */
      out_ary->src_start_y = (int32_t)((phase_fg.phase_y_cur) >> q_factor);
    }

    if ((src_tile_index_y_counter == in_ary->frame_height_mcus - 1) &&
      (!in_ary->pad_boundary)) {
      if ((phase_fg.phase_y_next & q_mask) >>
           (q_factor - downscale_interpolation_resolution)) {
        out_ary->src_end_y = (int32_t)(phase_fg.phase_y_next >> q_factor);
      } else {
        /* need to fetch one pixel less */
        out_ary->src_end_y = (int32_t)(phase_fg.phase_y_next >> q_factor) - 1;
      }
    } else {
      /*
       * not the bottommost block,
       * need to fetch more on the bottom
       */
      int64_t temp = phase_fg.phase_y_next + in_ary->postscale_padding *
        in_ary->vertical_scale_ratio;
      if ((temp & q_mask) >> (q_factor - downscale_interpolation_resolution)) {
        out_ary->src_end_y = (int32_t)(temp >> q_factor) +
        in_ary->prescaler_padding;
      } else {
        /* need to fetch one less */
        out_ary->src_end_y = (int32_t)(temp >> q_factor) - 1 +
        in_ary->prescaler_padding;
      }
    }
  } else {
    /* that is, scaling is disabled in the y direction */
    out_ary->scale_v_en = 0;
    out_ary->upscale_v_en = 0;
    /* only the fractional bits are needed */
    out_ary->v_init_phase = 0;
    if (src_tile_index_y_counter || in_ary->pad_boundary) {
      /* not the topmost block, need to fetch more on top */
      out_ary->src_start_y = (int32_t)(phase_fg.phase_y_cur >> q_factor) -
         in_ary->postscale_padding - in_ary->prescaler_padding;
    } else {
      /*
       * the topmost block,
       * top padding is done in the denoise and sharpening block
       */
      out_ary->src_start_y = (int32_t)(phase_fg.phase_y_cur >> q_factor);
    }
    if ((src_tile_index_y_counter == in_ary->frame_height_mcus - 1) &&
      (!in_ary->pad_boundary)) {
      /*
       * the bottommost block,
       * bottom padding by the denoise and sharpening block
       */
      out_ary->src_end_y = (int32_t)(phase_fg.phase_y_next >> q_factor) - 1;
    } else {
      /*
       * not the bottommost block,
       * need to fetch more on the bottom
       */
      out_ary->src_end_y =
          (int32_t)((phase_fg.phase_y_next) >> q_factor) - 1 +
         in_ary->postscale_padding + in_ary->prescaler_padding;
    }
  }
  if (src_tile_index_y_counter || in_ary->pad_boundary) {
    /* not the topmost block, need to fetch more on top */
    out_ary->prescaler_spatial_denoise_crop_height_first_line =
       in_ary->prescaler_spatial_denoise_padding;
    out_ary->temporal_denoise_crop_height_first_line =
       in_ary->temporal_denoise_padding;
    out_ary->spatial_denoise_crop_height_first_line =
       in_ary->spatial_denoise_padding;
    out_ary->sharpen_crop_height_first_line = in_ary->sharpen_padding;
    out_ary->state_crop_height_first_line = in_ary->state_padding;
  } else {
    /*
     * the topmost block, top padding is done internally
     * by denoise and sharpening but not upscaling
     */
    out_ary->spatial_denoise_crop_height_first_line = 0;
    out_ary->prescaler_spatial_denoise_crop_height_first_line = 0;
    out_ary->temporal_denoise_crop_height_first_line = 0;
    out_ary->sharpen_crop_height_first_line = 0;
    out_ary->state_crop_height_first_line = 0;
  }
  if (!in_ary->sharpen_before_scale) {
    out_ary->sharpen_crop_height_last_line =
        out_ary->sharpen_crop_height_first_line +
       in_ary->stripe_block_height[src_tile_index_y_counter] - 1;
  }
  /*
   * output height of the scaler should be the write
   * block height plus some padding required by sharpening
   */
  out_ary->scale_output_height =
     in_ary->stripe_block_height[src_tile_index_y_counter];
  if ((src_tile_index_y_counter != 0) || in_ary->pad_boundary) {
    out_ary->scale_output_height += in_ary->postscale_padding;
  }
  if ((src_tile_index_y_counter != in_ary->frame_height_mcus - 1) ||
    in_ary->pad_boundary) {
    out_ary->scale_output_height += in_ary->postscale_padding;
  }

  if ((src_tile_index_y_counter == in_ary->frame_height_mcus - 1) &&
    (!in_ary->pad_boundary)) {
    /*
     * the rightmost stripe, right padding is provided by
     * denoise and sharpening but not upscaling
     */
    out_ary->prescaler_spatial_denoise_crop_height_last_line =
       out_ary->src_end_y - out_ary->src_start_y;
    if (in_ary->denoise_after_scale_en) {
      if (in_ary->sharpen_before_scale) {
        out_ary->sharpen_crop_height_last_line =
          out_ary->sharpen_crop_height_first_line +
          in_ary->stripe_block_height[src_tile_index_y_counter] - 1;
      }
      out_ary->spatial_denoise_crop_height_last_line =
         out_ary->spatial_denoise_crop_height_first_line +
         out_ary->sharpen_crop_height_last_line;
      out_ary->temporal_denoise_crop_height_last_line =
         out_ary->temporal_denoise_crop_height_first_line +
         out_ary->spatial_denoise_crop_height_last_line;
      out_ary->state_crop_height_last_line =
         out_ary->state_crop_height_first_line +
         in_ary->stripe_block_height[src_tile_index_y_counter] - 1;
    } else {
      out_ary->temporal_denoise_crop_height_last_line =
        out_ary->prescaler_spatial_denoise_crop_height_last_line -
        out_ary->prescaler_spatial_denoise_crop_height_first_line;
      out_ary->spatial_denoise_crop_height_last_line =
        out_ary->temporal_denoise_crop_height_last_line -
        out_ary->temporal_denoise_crop_height_first_line;
      if (in_ary->sharpen_before_scale) {
        out_ary->sharpen_crop_height_last_line =
          out_ary->spatial_denoise_crop_height_last_line -
          out_ary->spatial_denoise_crop_height_first_line;
      }
      out_ary->state_crop_height_last_line =
        out_ary->spatial_denoise_crop_height_last_line;
    }
  } else {
    /* not the rightmost stripe. need to fetch more on the right */
    out_ary->prescaler_spatial_denoise_crop_height_last_line =
       out_ary->src_end_y - out_ary->src_start_y -
       in_ary->prescaler_spatial_denoise_padding;
    if (in_ary->denoise_after_scale_en) {
      if (in_ary->sharpen_before_scale) {
        out_ary->sharpen_crop_height_last_line =
          out_ary->sharpen_crop_height_first_line +
          in_ary->stripe_block_height[src_tile_index_y_counter] - 1;
      }
      out_ary->spatial_denoise_crop_height_last_line =
         out_ary->spatial_denoise_crop_height_first_line +
         out_ary->sharpen_crop_height_last_line + in_ary->sharpen_padding;
      out_ary->temporal_denoise_crop_height_last_line =
        out_ary->temporal_denoise_crop_height_first_line +
         out_ary->spatial_denoise_crop_height_last_line +
         in_ary->spatial_denoise_padding;
      out_ary->state_crop_height_last_line =
         out_ary->state_crop_height_first_line +
         in_ary->stripe_block_height[src_tile_index_y_counter] - 1;
    } else {
      out_ary->temporal_denoise_crop_height_last_line =
        out_ary->prescaler_spatial_denoise_crop_height_last_line -
        out_ary->prescaler_spatial_denoise_crop_height_first_line -
        in_ary->temporal_denoise_padding;
      out_ary->spatial_denoise_crop_height_last_line =
        out_ary->temporal_denoise_crop_height_last_line -
        out_ary->temporal_denoise_crop_height_first_line -
        in_ary->spatial_denoise_padding;
      if (in_ary->sharpen_before_scale) {
        out_ary->sharpen_crop_height_last_line =
          out_ary->spatial_denoise_crop_height_last_line -
          out_ary->spatial_denoise_crop_height_first_line -
          in_ary->sharpen_padding;
        out_ary->state_crop_height_last_line =
          out_ary->spatial_denoise_crop_height_last_line -
          in_ary->sharpen_padding;
      } else {
        out_ary->state_crop_height_last_line =
           out_ary->spatial_denoise_crop_height_last_line;
      }
    }
  }

  if(!in_ary->denoise_after_scale_en) {
    if(out_ary->upscale_h_en) {
      out_ary->state_crop_width_first_pixel +=  upscale_left_top_padding;
      out_ary->state_crop_width_last_pixel -=  upscale_right_bottom_padding;
    }
    if(out_ary->upscale_v_en) {
      out_ary->state_crop_height_first_line +=  upscale_left_top_padding;
      out_ary->state_crop_height_last_line -=  upscale_right_bottom_padding;
    }
  }

  if(!in_ary->denoise_after_scale_en) {
    out_ary->temporal_dst_start_x = out_ary->src_start_x +
      out_ary->temporal_denoise_crop_width_first_pixel +
      out_ary->state_crop_width_first_pixel;
    out_ary->temporal_dst_end_x = out_ary->src_start_x +
      out_ary->temporal_denoise_crop_width_first_pixel +
      out_ary->state_crop_width_last_pixel;
    out_ary->temporal_dst_start_y = out_ary->src_start_y +
      out_ary->temporal_denoise_crop_height_first_line +
      out_ary->state_crop_height_first_line;
    out_ary->temporal_dst_end_y = out_ary->src_start_y +
      out_ary->temporal_denoise_crop_height_first_line +
      out_ary->state_crop_height_last_line;
    /* left boundary */
    if (out_ary->temporal_dst_start_x < 0) {
      out_ary->state_crop_width_first_pixel -= out_ary->temporal_dst_start_x;
      out_ary->temporal_dst_start_x = 0;
    }
    /* right boundary */
    if (out_ary->temporal_dst_end_x >= in_ary->src_width) {
      out_ary->state_crop_width_last_pixel -=
        (out_ary->temporal_dst_end_x-(in_ary->src_width-1));
      out_ary->temporal_dst_end_x = (in_ary->src_width - 1);
    }
    /* top boundary */
    if (out_ary->temporal_dst_start_y < 0) {
      out_ary->state_crop_height_first_line -=
        out_ary->temporal_dst_start_y;
       out_ary->temporal_dst_start_y = 0;
    }
    /* bottom boundary */
    if (out_ary->temporal_dst_end_y >= in_ary->src_height) {
      out_ary->state_crop_height_last_line -=
        (out_ary->temporal_dst_end_y-(in_ary->src_height-1));
      out_ary->temporal_dst_end_y = (in_ary->src_height - 1);
    }
  }

  /* extra 5th and 6th layer spatial denoise */
  out_ary->extra_upscale_width = out_ary->src_end_x - out_ary->src_start_x + 1;
  out_ary->extra_upscale_height = out_ary->src_end_y - out_ary->src_start_y + 1;
  out_ary->extra_src_start_x = (floor((out_ary->src_start_x+0.5)/64)) * 64 - 32;
  out_ary->extra_src_end_x = (ceil((out_ary->src_end_x+0.5)/64)) * 64 + 31;
  out_ary->extra_src_start_y = (floor((out_ary->src_start_y+0.5)/64)) * 64 - 32;
  out_ary->extra_src_end_y = (ceil((out_ary->src_end_y+0.5)/64)) * 64 + 31;

  /* upscale 64x */
  out_ary->extra_initial_horizontal_count[1] =
     (2 * out_ary->src_start_x + 1) & 0x7f;
  out_ary->extra_initial_vertical_count[1] =
     (2 * out_ary->src_start_y + 1) & 0x7f;

  /* upscale 32x */
  out_ary->extra_initial_horizontal_count[0] =
     (2 * out_ary->src_start_x + 33) & 0x3f;
  out_ary->extra_initial_vertical_count[0] =
     (2 * out_ary->src_start_y + 33) & 0x3f;

  /* crop pixel */
  out_ary->extra_left_crop =
     (out_ary->extra_initial_horizontal_count[1] + 32) / 64;
  out_ary->extra_top_crop =
     (out_ary->extra_initial_vertical_count[1] + 32) / 64;

  /* left boundary */
  if (out_ary->extra_src_start_x < 0) {
    out_ary->extra_pad_left = 0 - out_ary->extra_src_start_x;
    out_ary->extra_src_start_x = 0;
  } else {
        out_ary->extra_pad_left = 0;
  }
  /* top boundary */
  if (out_ary->extra_src_start_y < 0) {
    out_ary->extra_pad_top = 0 - out_ary->extra_src_start_y;
    out_ary->extra_src_start_y = 0;
  } else {
    out_ary->extra_pad_top = 0;
  }

  /* right boundary */
  if (out_ary->extra_src_end_x >= in_ary->src_width) {
    out_ary->extra_pad_right = out_ary->extra_src_end_x -
       (in_ary->src_width - 1);
    out_ary->extra_src_end_x = (in_ary->src_width - 1);
  } else {
    out_ary->extra_pad_right = 0;
  }

  /* bottom boundary */
  if (out_ary->extra_src_end_y >= in_ary->src_height) {
    out_ary->extra_pad_bottom = out_ary->extra_src_end_y -
       (in_ary->src_height - 1);
    out_ary->extra_src_end_y = (in_ary->src_height - 1);
  } else {
    out_ary->extra_pad_bottom = 0;
  }

  /* left boundary */
  if (out_ary->src_start_x < 0) {
    out_ary->pad_left = 0 - out_ary->src_start_x;
    out_ary->src_start_x = 0;
  } else {
    out_ary->pad_left = 0;
  }

  /* top boundary */
  if (out_ary->src_start_y < 0) {
    out_ary->pad_top = 0 - out_ary->src_start_y;
    out_ary->src_start_y = 0;
  } else {
    out_ary->pad_top = 0;
  }

  /* right boundary */
  if (out_ary->src_end_x >= in_ary->src_width) {
    out_ary->pad_right = out_ary->src_end_x - (in_ary->src_width - 1);
    out_ary->src_end_x = (in_ary->src_width - 1);
  } else {
    out_ary->pad_right = 0;
  }

  /* bottom boundary */
  if (out_ary->src_end_y >= in_ary->src_height) {
    out_ary->pad_bottom = out_ary->src_end_y - (in_ary->src_height - 1);
    out_ary->src_end_y = (in_ary->src_height - 1);
  } else {
    out_ary->pad_bottom = 0;
  }
  if(!in_ary->denoise_after_scale_en) {
    out_ary->temporal_src_start_x = out_ary->src_start_x;
    out_ary->temporal_src_end_x = out_ary->src_end_x;
    out_ary->temporal_src_start_y = out_ary->src_start_y;
    out_ary->temporal_src_end_y = out_ary->src_end_y;
    out_ary->temporal_pad_left = out_ary->pad_left;
    out_ary->temporal_pad_right = out_ary->pad_right;
    out_ary->temporal_pad_top = out_ary->pad_top;
    out_ary->temporal_pad_bottom = out_ary->pad_bottom;
  }

  /* The leftmost point of the fetch block in byte addresses */
  out_ary->source_address[0] =
      in_ary->source_address[0] + out_ary->src_start_x *
     in_ary->input_bytes_per_pixel +
      out_ary->src_start_y * in_ary->src_stride;
  out_ary->source_address[1] =
      in_ary->source_address[1] + out_ary->src_start_x *
    in_ary->input_bytes_per_pixel +
      out_ary->src_start_y * in_ary->src_stride;
  out_ary->extra_source_address[0] =
      in_ary->source_address[0] + out_ary->extra_src_start_x *
     in_ary->input_bytes_per_pixel +
      out_ary->extra_src_start_y * in_ary->src_stride;
  out_ary->extra_source_address[1] =
      in_ary->source_address[1] + out_ary->extra_src_start_x *
    in_ary->input_bytes_per_pixel +
      out_ary->extra_src_start_y * in_ary->src_stride;
}

/** set_start_of_frame_parameters_1_10:
 *
 *  @in_ary - This parameter holds the input data for the current stripe.
 *
 *  This function calculates the number of height and width mcu, the size of
 *  block width and neight according to the scale ratio and rotation.
 *
 *  Return: It is void function.
 **/
void set_start_of_frame_parameters_1_10(struct cpp_plane_info_t * in_ary)
{
  int block_width;
  uint32_t i;
  int rotation_block_height;
  int rotation_block_width;
  if (in_ary->is_not_y_plane) {
    in_ary->sharpen_padding = 0;
  }

  if (in_ary->denoise_after_scale_en) {
    if (in_ary->horizontal_scale_ratio == (1 << q_factor) &&
         ((in_ary->horizontal_scale_initial_phase & q_mask) == 0)
        && in_ary->vertical_scale_ratio == (1 << q_factor) &&
         ((in_ary->vertical_scale_initial_phase & q_mask) == 0)) {
      // scaler is off
      in_ary->prescaler_spatial_denoise_padding = 0;
    } else {
      in_ary->prescaler_spatial_denoise_padding = 0;
    }
    in_ary->postscale_padding = in_ary->temporal_denoise_padding +
       in_ary->spatial_denoise_padding + in_ary->sharpen_padding;
    in_ary->prescaler_padding = in_ary->prescaler_spatial_denoise_padding;
    /* temporal state buffer crop is after temporal denoise crop */
    in_ary->state_padding = in_ary->spatial_denoise_padding +
       in_ary->sharpen_padding;
  } else {
    if (in_ary->sharpen_before_scale) {
      in_ary->prescaler_spatial_denoise_padding = 0;
      in_ary->postscale_padding = 0;
      in_ary->prescaler_padding = in_ary->sharpen_padding +
        in_ary->spatial_denoise_padding +
        in_ary->temporal_denoise_padding;
      in_ary->state_padding = in_ary->sharpen_padding +
        in_ary->spatial_denoise_padding;
    } else {
      in_ary->prescaler_spatial_denoise_padding = 0;
      in_ary->postscale_padding = in_ary->sharpen_padding;
      in_ary->prescaler_padding = in_ary->spatial_denoise_padding +
        in_ary->temporal_denoise_padding;
      in_ary->state_padding = in_ary->spatial_denoise_padding;
    }
  }
  /* crop out asf pixels */
  block_width = in_ary->line_buffer_size - 2 * in_ary->postscale_padding;
   /* determine destination stripe width */
  if (in_ary->horizontal_scale_ratio < (1 << q_factor)
      || (in_ary->horizontal_scale_ratio == (1 << q_factor) &&
           (in_ary->horizontal_scale_initial_phase & q_mask))) {
    /*
     * upscaler is used
     * crop out asf pixels, upscale pixels, and denoise pixels
     */
    int64_t temp =
        in_ary->line_buffer_size - 2 * in_ary->prescaler_padding -
       upscale_left_top_padding - upscale_right_bottom_padding;
    temp <<= q_factor;
    /* safety margin */
    temp -= q_mask + 1;
    temp /= in_ary->horizontal_scale_ratio;
    /* number of pixels that can be produced by upscaler */
    /* safety margin */
    temp += 1;
    temp -= 2 * in_ary->postscale_padding;
    if (temp < block_width) {
      block_width = (int32_t)(temp);
    }
  } else if (in_ary->horizontal_scale_ratio > (1 << q_factor)) {
    /*
     * downscaler
     * crop out asf pixels, and denoise pixels
     */
    int64_t temp = in_ary->line_buffer_size - 2 * in_ary->prescaler_padding;
    temp <<= q_factor;
    /* safety margin */
    temp -= q_mask;
    temp /= in_ary->horizontal_scale_ratio;
    /* number of pixels that can be produced by downscaler */
    temp -= 2 * in_ary->postscale_padding;
    if (temp < block_width) {
      block_width = (int32_t)(temp);
    }
  } else {
    /*
     * no scaler
     * crop out asf pixels, and denoise pixels
     */
    int64_t temp = in_ary->line_buffer_size - 2 * in_ary->prescaler_padding;
    temp -= 2 * in_ary->postscale_padding;
    if (temp < block_width) {
      block_width = (int32_t)(temp);
    }
  }

  if (in_ary->rotate == 0 || in_ary->rotate == 2) {
    /* 0 or 180 degree rotation */
    if (in_ary->tile_output_enable) {
      rotation_block_height = in_ary->tile_byte_height;
    } else {
      /*
       * rotation block height is 2,
       * the destination image height
       * will be a multiple of 2.
       */
      rotation_block_height = 2;
      in_ary->tile_byte_height = 2;
    }
  } else {
    /*
     * 90 or 270 degree rotation
     * rotation block height is MAL length
     * (32?). The destination image height
     *  will be a multiple of MAL.
     */
    if (in_ary->tile_output_enable) {
      rotation_block_height = in_ary->tile_byte_width /
        in_ary->output_bytes_per_pixel;
    } else {
      /*
       * rotation block height
       * is MAL length (32?).
       * The destination image
       * height will be a
       * multiple of MAL.
       */
      rotation_block_height = in_ary->mal_byte_size;
      in_ary->tile_byte_width = in_ary->mal_byte_size *
         in_ary->output_bytes_per_pixel;
    }
  }

  if (in_ary->tile_output_enable) {
    /* rotation buffer is split into two */
    rotation_block_width = in_ary->rotation_buffer_size /
       (2 * rotation_block_height);
  } else {
    rotation_block_width = in_ary->rotation_buffer_size / rotation_block_height;
  }

  if (block_width > rotation_block_width) {
    block_width = rotation_block_width;
  }

  if (in_ary->tile_output_enable) {
    int tile_width_before_rotation;
    if (in_ary->rotate == 0 || in_ary->rotate == 2) {
      tile_width_before_rotation = in_ary->tile_byte_width /
        in_ary->output_bytes_per_pixel;
    } else {
      tile_width_before_rotation = in_ary->tile_byte_height;
    }
    int normalized_dst_width = (in_ary->dst_width +
                                 tile_width_before_rotation - 1) /
       tile_width_before_rotation;
    block_width /= tile_width_before_rotation;
    in_ary->frame_width_mcus =
       (normalized_dst_width + block_width - 1) / block_width;
    if ((!in_ary->is_not_y_plane) && (in_ary->frame_width_mcus % 2)) {
      /*
       * try to make number of stripes even
       * for Y plane to run faster, there are
       * two cores
       */
      in_ary->frame_width_mcus++;
    }
    in_ary->dst_block_width =
       (normalized_dst_width + in_ary->frame_width_mcus - 1) /
       in_ary->frame_width_mcus;
    in_ary->frame_width_mcus =
       (normalized_dst_width + in_ary->dst_block_width - 1) /
      in_ary->dst_block_width;
    in_ary->dst_block_width *= tile_width_before_rotation;
    /*
     * The actual destination image width,
     * multiple of tile width before rotation
     */
    in_ary->dst_width_block_aligned = normalized_dst_width *
       tile_width_before_rotation;
  } else {
    /* minimum number of stripes */
    in_ary->frame_width_mcus = (in_ary->dst_width + block_width - 1) /
       block_width;
    if ((!in_ary->is_not_y_plane) && (in_ary->frame_width_mcus % 2)) {
      /*
       * try to make number of stripes even for
       * Y plane to run faster, there are two cores
       */
      in_ary->frame_width_mcus++;
    }

    /* evenly distribute the stripe width */
    in_ary->dst_block_width =
       (in_ary->dst_width + in_ary->frame_width_mcus - 1) /
      in_ary->frame_width_mcus;
    /* number of stripes */
    in_ary->frame_width_mcus =
       (in_ary->dst_width + in_ary->dst_block_width - 1) /
       in_ary->dst_block_width;
    /*
     * The actual destination image width,
     * multiple of tile width before rotation
     */
    in_ary->dst_width_block_aligned = in_ary->dst_width;
  }

  /* destination stripe width */
  in_ary->stripe_block_width =
    (uint32_t *)CAM_MALLOC(sizeof(uint32_t) *in_ary->frame_width_mcus);
  if (!in_ary->stripe_block_width) {
      CPP_ERR("CAM_MALLOC() failed");
      return;
  }
  for (i = 0; i < in_ary->frame_width_mcus; i++) {
     /*
      * First assume all the stripe widths are the same,
      * one entry will be changed later
      */
    in_ary->stripe_block_width[i] = in_ary->dst_block_width;
  }

  in_ary->dst_height_block_aligned =
     ((in_ary->dst_height + rotation_block_height - 1) /
       rotation_block_height) * rotation_block_height;
  /* The actual destination image height, multiple of rotation block height */
  if (in_ary->dst_height_block_aligned > in_ary->maximum_dst_stripe_height) {
    /*
     * Maximum allowed block height is smaller than destination image height
     * do block processing
     */
    in_ary->dst_block_height = (in_ary->maximum_dst_stripe_height /
                                 rotation_block_height) * rotation_block_height;
  } else {
    /* do stripe processing */
    in_ary->dst_block_height = in_ary->dst_height_block_aligned;
  }
  in_ary->frame_height_mcus =
     (in_ary->dst_height_block_aligned + in_ary->dst_block_height - 1) /
     in_ary->dst_block_height;
  /* Number of blocks in the vertical direction */
  in_ary->stripe_block_height =
     (uint32_t *)CAM_MALLOC(sizeof(uint32_t) * in_ary->frame_height_mcus);
  if (!in_ary->stripe_block_height) {
      CPP_ERR("Cannot assign memory to in_ary->stripe_block_height");
      free(in_ary->stripe_block_width);
      return;
  }
  /* destination block height */
  for (i = 0; i < in_ary->frame_height_mcus; i++){
    /*
     * First assume all the block heights are the same,
     * one entry will be changed later
     */
    in_ary->stripe_block_height[i] = in_ary->dst_block_height;
  }

  switch (in_ary->rotate) {
    case 3:
      /* 270 degree */
      if (in_ary->mirror & 0x01) {
        /* the rightmost destination stripe width is smaller than others */
        in_ary->vertical_scale_initial_phase -=
           (in_ary->dst_height_block_aligned - in_ary->dst_height) *
          in_ary->vertical_scale_ratio;
        in_ary->vertical_scale_block_initial_phase =
           in_ary->vertical_scale_initial_phase -
           (in_ary->frame_height_mcus * in_ary->dst_block_height -
           in_ary->dst_height_block_aligned) * in_ary->vertical_scale_ratio;
        in_ary->stripe_block_height[0] = in_ary->dst_height_block_aligned -
          in_ary->dst_block_height * (in_ary->frame_height_mcus - 1);
      } else {
        /*
         * the bottommost block heights are smaller than others.
         * fetch a little more on the bottom so padding will be on the right side
         */
        in_ary->vertical_scale_block_initial_phase =
           in_ary->vertical_scale_initial_phase;
        in_ary->stripe_block_height[in_ary->frame_height_mcus - 1] =
           in_ary->dst_height_block_aligned - in_ary->dst_block_height *
           (in_ary->frame_height_mcus - 1);
      }

      /* the leftmost destination stripe width is smaller than others */
      in_ary->horizontal_scale_initial_phase -=
         (in_ary->dst_width_block_aligned - in_ary->dst_width) *
         in_ary->horizontal_scale_ratio;
      in_ary->horizontal_scale_block_initial_phase =
         in_ary->horizontal_scale_initial_phase -
         (in_ary->frame_width_mcus * in_ary->dst_block_width -
           in_ary->dst_width_block_aligned) * in_ary->horizontal_scale_ratio;
      in_ary->stripe_block_width[0] = in_ary->dst_width_block_aligned -
         in_ary->dst_block_width * (in_ary->frame_width_mcus - 1);
      break;
    case 2:
      if (in_ary->mirror & 0x02) {
        /* the rightmost destination stripe width is smaller than others */
        in_ary->horizontal_scale_block_initial_phase =
          in_ary->horizontal_scale_initial_phase;
        in_ary->stripe_block_width[in_ary->frame_width_mcus - 1] =
           in_ary->dst_width_block_aligned - in_ary->dst_block_width *
           (in_ary->frame_width_mcus - 1);
      } else {
        /* the leftmost destination stripe width is smaller than others */
        in_ary->horizontal_scale_initial_phase -=
           (in_ary->dst_width_block_aligned - in_ary->dst_width) *
           in_ary->horizontal_scale_ratio;
        in_ary->horizontal_scale_block_initial_phase =
           in_ary->horizontal_scale_initial_phase - (in_ary->frame_width_mcus *
                in_ary->dst_block_width - in_ary->dst_width_block_aligned) *
           in_ary->horizontal_scale_ratio;
        in_ary->stripe_block_width[0] = in_ary->dst_width_block_aligned -
           in_ary->dst_block_width * (in_ary->frame_width_mcus - 1);
      }
      /*
       * the topmost block heights are smaller than others
       * fetch a little more on the top so padding will be on the bottom
       */
      in_ary->vertical_scale_initial_phase -=
         (in_ary->dst_height_block_aligned - in_ary->dst_height) *
         in_ary->vertical_scale_ratio;
      in_ary->vertical_scale_block_initial_phase =
        in_ary->vertical_scale_initial_phase - (in_ary->frame_height_mcus *
           in_ary->dst_block_height - in_ary->dst_height_block_aligned) *
         in_ary->vertical_scale_ratio;
      in_ary->stripe_block_height[0] = in_ary->dst_height_block_aligned -
         in_ary->dst_block_height * (in_ary->frame_height_mcus - 1);
      break;
    case 1:
      /* 90 degree */
      if (in_ary->mirror & 0x01) {
        /*
         * the bottommost block heights are smaller than others.
         * fetch a little more on the bottom so padding will be on the bottom
         */
        in_ary->vertical_scale_block_initial_phase =
           in_ary->vertical_scale_initial_phase;
        in_ary->stripe_block_height[in_ary->frame_height_mcus - 1] =
           in_ary->dst_height_block_aligned - in_ary->dst_block_height *
           (in_ary->frame_height_mcus - 1);
      } else {
        /*
         * the topmost block heights are smaller than others
         * fetch a little more on the top so padding will be on the right
         */
        in_ary->vertical_scale_initial_phase -=
          (in_ary->dst_height_block_aligned - in_ary->dst_height) *
           in_ary->vertical_scale_ratio;
        in_ary->vertical_scale_block_initial_phase =
           in_ary->vertical_scale_initial_phase - (in_ary->frame_height_mcus *
              in_ary->dst_block_height - in_ary->dst_height_block_aligned) *
           in_ary->vertical_scale_ratio;
        in_ary->stripe_block_height[0] = in_ary->dst_height_block_aligned -
           in_ary->dst_block_height * (in_ary->frame_height_mcus - 1);
      }
      /* the rightmost destination stripe width is smaller than others */
      in_ary->horizontal_scale_block_initial_phase =
         in_ary->horizontal_scale_initial_phase;
      in_ary->stripe_block_width[in_ary->frame_width_mcus - 1] =
         in_ary->dst_width_block_aligned - in_ary->dst_block_width *
        (in_ary->frame_width_mcus - 1);
      break;
    case 0:
    default:
      if (in_ary->mirror & 0x02) {
        /* the leftmost destination stripe width is smaller than others */
        in_ary->horizontal_scale_initial_phase -=
           (in_ary->dst_width_block_aligned - in_ary->dst_width) *
           in_ary->horizontal_scale_ratio;
        in_ary->horizontal_scale_block_initial_phase =
           in_ary->horizontal_scale_initial_phase -
          (in_ary->frame_width_mcus * in_ary->dst_block_width -
            in_ary->dst_width_block_aligned) * in_ary->horizontal_scale_ratio;
        in_ary->stripe_block_width[0] = in_ary->dst_width_block_aligned -
           in_ary->dst_block_width * (in_ary->frame_width_mcus - 1);
      } else {
        /* the rightmost destination stripe width is smaller than others */
        in_ary->horizontal_scale_block_initial_phase =
           in_ary->horizontal_scale_initial_phase;
        in_ary->stripe_block_width[in_ary->frame_width_mcus - 1] =
           in_ary->dst_width_block_aligned - in_ary->dst_block_width *
           (in_ary->frame_width_mcus - 1);
      }
      /*
       * the bottommost block heights are smaller than others.
       * fetch a little more on the bottom so padding will be on the bottom
       */
      in_ary->vertical_scale_block_initial_phase =
        in_ary->vertical_scale_initial_phase;
      in_ary->stripe_block_height[in_ary->frame_height_mcus - 1] =
         in_ary->dst_height_block_aligned - in_ary->dst_block_height *
         (in_ary->frame_height_mcus - 1);
      break;
  }
  in_ary->num_stripes = (uint32_t)
    in_ary->frame_width_mcus * in_ary->frame_height_mcus;
  CPP_DBG("num stripe %d, width mcus %d, height mcus %d", in_ary->num_stripes,
    in_ary->frame_width_mcus, in_ary->frame_height_mcus);
}

/** cpp_create_frame_message_1_10:
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
uint32_t* cpp_create_frame_message_1_10(
    struct cpp_frame_info_t* cpp_frame_info, uint32_t* len)
{
  int32_t idx, total_stripes;
  uint32_t msg_len, i, j, k, base;
  uint32_t* frame_msg;

  total_stripes = 0;
  for (i = 0; i < cpp_frame_info->num_planes; i++) {
    total_stripes += cpp_frame_info->plane_info[i].num_stripes;
  }

  uint32_t stripe_base = 553;

  /* top level + planes + stripes + 1 (trailer) */
  msg_len = stripe_base + 61 * total_stripes + 1;
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
  /* TODO: need to adjust base due to HFR */
  base = 11;
  /* TODO: ASF same as 1.6, do we use 1.8? */
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

  /* WNR Config */
  base = 331;
  struct cpp_bf_info* bf_info;
  cpp_prescaler_bf_info_t* prescaler_bf_info;
  cpp_tnr_info_t* tnr_info;

  /* for Y, CB and CR components */
  for (i = 0; i < 3; i++) {
    /* Fill in prescaler_bf */
    prescaler_bf_info = &cpp_frame_info->prescaler_info[i];
    k = base + i * 22;
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
      k = base + 2 + i * 22 + j;
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
    for (j = 0; j < 4; j++) {
      k = base + 11 + i * 22 + j;
      if (j != 3) {
        frame_msg[k] =
            (uint32_t)(clamp(Round(bf_info->bilateral_scale[j] *
            (1 << 10)), 0, 0x1ffff));
      } else {
        frame_msg[k] =
            (uint32_t)(clamp(Round(bf_info->bilateral_scale[j] *
            (1 << 8)), 0, 0x3ffff));
      }

    }
    for (j = 0; j < CPP_DENOISE_NUM_PROFILES; j++) {
      k = base + 11 + i * 22 + j + 4;
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
  }

  /* ASF Settings */
  /* pp_m_asf_cfg_0 */
  frame_msg[397] =
      (1 << 31) |
          /* set asf version to 1, for fw 1.6.0 */
          ((asf_info->sp_eff_en & 0x1) << 0) |
          ((asf_info->dyna_clamp_en & 0x1) << 1) |
          ((asf_info->neg_abs_y1 & 0x1) << 2) |
          ((asf_info->sp & 0x1f) << 4) |
          ((asf_info->sp_eff_abs_en & 0x1) << 16) |
          ((asf_info->L2_norm_en & 0x1) << 17);
  /* pp_m_asf_cfg_1 */
  frame_msg[398] =
      (asf_info->perpen_scale_factor & 0x7f) |
          ((asf_info->activity_clamp_threshold & 0xff) << 16);
  /* pp_m_asf_cfg_2 */
  frame_msg[399] =
      (asf_info->max_val_threshold & 0x3fff) |
          ((asf_info->gamma_cor_luma_target & 0xff) << 16);
  /* pp_m_asf_cfg_3 */
  frame_msg[400] =
      (asf_info->gain_cap & 0xff) |
          ((asf_info->median_blend_offset & 0xf) << 16) |
          ((asf_info->median_blend_lower_offset & 0xf) << 20);
  /* pp_m_asf_cfg_4 */
  frame_msg[401] = (asf_info->norm_scale & 0xff);
  /* pp_m_asf_clamp_tl_limit */
  frame_msg[402] =
      (asf_info->clamp_tl_ll & 0x1ff) |
          ((asf_info->clamp_tl_ul & 0x1ff) << 9);
  /* pp_m_asf_clamp_scale */
  frame_msg[403] =
      (asf_info->clamp_scale_min & 0x1ff) |
          ((asf_info->clamp_scale_max & 0x1ff) << 16);
  /* pp_m_asf_clamp_offset */
  frame_msg[404] =
      (asf_info->clamp_offset_min & 0x7f) |
          ((asf_info->clamp_offset_max & 0x7f) << 16);
  /* pp_m_asf_nz_flag and _nz_flag_2 */
  frame_msg[405] = asf_info->nz_flag;
  frame_msg[406] = asf_info->nz_flag_2;

  /* pp_m_asf_sobel_h_coeff */
  base = 407;
  for (i = 0; i < 13; i++) {
    frame_msg[base + i] =
        (asf_info->sobel_h_coeff[i * 2] & 0xfff) |
            ((asf_info->sobel_h_coeff[i * 2 + 1] & 0xfff) << 16);
  }
  /* pp_m_asf_sobel_v_coeff */
  base = 420;
  for (i = 0; i < 13; i++) {
    frame_msg[base + i] =
        (asf_info->sobel_v_coeff[i * 2] & 0xfff) |
            ((asf_info->sobel_v_coeff[i * 2 + 1] & 0xfff) << 16);
  }
  /* pp_m_asf_hpf_h_coeff */
  base = 433;
  for (i = 0; i < 13; i++) {
    frame_msg[base + i] =
        (asf_info->hpf_h_coeff[i * 2] & 0xfff) |
            ((asf_info->hpf_h_coeff[i * 2 + 1] & 0xfff) << 16);
  }
  /* pp_m_asf_hpf_v_coeff */
  base = 446;
  for (i = 0; i < 13; i++) {
    frame_msg[base + i] =
        (asf_info->hpf_v_coeff[i * 2] & 0xfff) |
            ((asf_info->hpf_v_coeff[i * 2 + 1] & 0xfff) << 16);
  }
  /* pp_m_asf_lpf_coeff */
  base = 459;
  for (i = 0; i < 8; i++) {
    frame_msg[base + i] =
        (asf_info->lpf_coeff[i * 2] & 0xfff) |
            ((asf_info->lpf_coeff[i * 2 + 1] & 0xfff) << 16);
  }
  /* pp_m_asf_hpf_symmetric_coeff */
  base = 467;
  for (i = 0; i < 8; i++) {
    frame_msg[base + i] =
        (asf_info->hpf_sym_coeff[i * 2] & 0xfff) |
            ((asf_info->hpf_sym_coeff[i * 2 + 1] & 0xfff) << 16);
  }
  /* pp_m_asf_activity_bpf_coeff */
  base = 475;
  for (i = 0; i < 3; i++) {
    frame_msg[base + i] =
        (asf_info->activity_bpf_coeff[i * 2] & 0x3ff) |
            ((asf_info->activity_bpf_coeff[i * 2 + 1] & 0x3ff) << 16);
  }
  /* pp_m_asf_activity_lpf_coeff */
  base = 478;
  for (i = 0; i < 3; i++) {
    frame_msg[base + i] =
        (asf_info->activity_lpf_coeff[i * 2] & 0xff) |
            ((asf_info->activity_lpf_coeff[i * 2 + 1] & 0xff) << 16);
  }

  /* Plane Specific Configuration */
  base = 481;
  for (i = 0; i < cpp_frame_info->num_planes; i++) {
    j = base + i * 24;

    frame_msg[j] = 1 << 5 |
            cpp_frame_info->plane_info[i].next_state_crop_enable << 10 |
            cpp_frame_info->plane_info[i].prescaler_crop_enable << 9 |
            cpp_frame_info->plane_info[i].pbf_enable << 8 |
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
    CPP_FRAME_MSG("FRAME_MSG ### i %d, j %d, frame msg %x", i, j, frame_msg[j]);
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

    /* mmu prefetch fetch engine params. This does not have l2 parameters */
    frame_msg[j+5] =
      cpp_frame_info->plane_info[i].mmu_prefetch_fe_info.l1_mmu_pf_en |
      cpp_frame_info->plane_info[i].mmu_prefetch_fe_info.l1_period  << 1 |
      cpp_frame_info->plane_info[i].mmu_prefetch_fe_info.l1_direction << 2 |
      cpp_frame_info->plane_info[i].mmu_prefetch_fe_info.l2_mmu_pf_en << 3;
    frame_msg[j+6] =
      cpp_frame_info->plane_info[i].mmu_prefetch_fe_info.addr_min;
    frame_msg[j+7] =
      cpp_frame_info->plane_info[i].mmu_prefetch_fe_info.addr_max;

    /* mmu prefetch ref fetch engine params. This does not have l2 parameters */
    frame_msg[j+8] =
      cpp_frame_info->plane_info[i].mmu_prefetch_ref_fe_info.l1_mmu_pf_en |
      cpp_frame_info->plane_info[i].mmu_prefetch_ref_fe_info.l1_period  << 1 |
      cpp_frame_info->plane_info[i].mmu_prefetch_ref_fe_info.l1_direction << 2 |
      cpp_frame_info->plane_info[i].mmu_prefetch_ref_fe_info.l2_mmu_pf_en << 3;
    frame_msg[j+9] =
      cpp_frame_info->plane_info[i].mmu_prefetch_ref_fe_info.addr_min;
    frame_msg[j+10]=
      cpp_frame_info->plane_info[i].mmu_prefetch_ref_fe_info.addr_max;

    /* mmu prefetch write engine params */
    frame_msg[j+11] =
      cpp_frame_info->plane_info[i].mmu_prefetch_we_info.l1_mmu_pf_en |
      cpp_frame_info->plane_info[i].mmu_prefetch_we_info.l1_period  << 1 |
      cpp_frame_info->plane_info[i].mmu_prefetch_we_info.l1_direction << 2 |
      cpp_frame_info->plane_info[i].mmu_prefetch_we_info.l2_mmu_pf_en << 3;
    frame_msg[j+12] =
      cpp_frame_info->plane_info[i].mmu_prefetch_we_info.addr_min;
    frame_msg[j+13] =
      cpp_frame_info->plane_info[i].mmu_prefetch_we_info.addr_max;
    frame_msg[j+14] =
      cpp_frame_info->plane_info[i].mmu_prefetch_we_info.l2_range;
    frame_msg[j+15] =
      cpp_frame_info->plane_info[i].mmu_prefetch_we_info.l2_distance;

    /* mmu prefetch duplicate write engine params */
    frame_msg[j+16] =
      cpp_frame_info->plane_info[i].mmu_prefetch_dup_we_info.l1_mmu_pf_en |
      cpp_frame_info->plane_info[i].mmu_prefetch_dup_we_info.l1_period  << 1 |
      cpp_frame_info->plane_info[i].mmu_prefetch_dup_we_info.l1_direction << 2 |
      cpp_frame_info->plane_info[i].mmu_prefetch_dup_we_info.l2_mmu_pf_en << 3;
    frame_msg[j+17] =
      cpp_frame_info->plane_info[i].mmu_prefetch_dup_we_info.addr_min;
    frame_msg[j+18] =
      cpp_frame_info->plane_info[i].mmu_prefetch_dup_we_info.addr_max;
    frame_msg[j+19] =
      cpp_frame_info->plane_info[i].mmu_prefetch_dup_we_info.l2_range;
    frame_msg[j+20] =
      cpp_frame_info->plane_info[i].mmu_prefetch_dup_we_info.l2_distance;

    /* mmu prefetch ref write engine params. This does not have l2 parameters */
    frame_msg[j+21] =
      cpp_frame_info->plane_info[i].mmu_prefetch_ref_we_info.l1_mmu_pf_en |
      cpp_frame_info->plane_info[i].mmu_prefetch_ref_we_info.l1_period  << 1 |
      cpp_frame_info->plane_info[i].mmu_prefetch_ref_we_info.l1_direction << 2 |
      cpp_frame_info->plane_info[i].mmu_prefetch_ref_we_info.l2_mmu_pf_en << 3;
    frame_msg[j+22] =
      cpp_frame_info->plane_info[i].mmu_prefetch_ref_we_info.addr_min;
    frame_msg[j+23] =
      cpp_frame_info->plane_info[i].mmu_prefetch_ref_we_info.addr_max;
  }

  /* Stripe Specific Configuration */
  base = 553;
  idx = base;
  struct cpp_stripe_info *stripe_info;
  struct cpp_plane_info_t *plane_info;
  for (j = 0; j < cpp_frame_info->num_planes; j++) {
    plane_info = &(cpp_frame_info->plane_info[j]);
    for (k = 0; k < cpp_frame_info->plane_info[j].num_stripes; k++, idx += 61) {
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
      frame_msg[idx + 16] = /* STRIPE[0]_FE_n_SWC_RD_BLOCK_VPAD */
          stripe_info->fe_info.bottom_pad << 16 |
              stripe_info->fe_info.top_pad;
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
      frame_msg[idx + 59] = /* STRIPE[0]_PP_m_ASF_CROP_CFG_0 */
          stripe_info->asf_crop_info.last_pixel << 16 |
              stripe_info->asf_crop_info.first_pixel;
      frame_msg[idx + 60] = /* STRIPE[0]_PP_m_ASF_CROP_CFG_1 */
          stripe_info->asf_crop_info.last_line << 16 |
              stripe_info->asf_crop_info.first_line;
    }
  }
  frame_msg[msg_len - 1] = CPP_FW_CMD_TRAILER;

  return frame_msg;
}

void cpp_prepare_dsdn_fetch_engine_info(struct cpp_plane_info_t *plane_info,
  uint32_t stripe_num)
{
  struct cpp_stripe_info *stripe_info1 =
    &plane_info->stripe_info1[stripe_num];
  struct cpp_striping_algorithm_info *stripe_info =
    &plane_info->stripe_info[stripe_num];
  struct cpp_fe_info *fe_info = &stripe_info1->fe_info;
  struct cpp_fe_info *fe_r_info = &stripe_info1->fe_r_info;

  /* Prepare CPP CDS (DSDN) fetch engine parameters */
  fe_r_info->buffer_ptr = stripe_info->extra_source_address[0];
  fe_r_info->buffer_width =
    stripe_info->extra_src_end_x - stripe_info->extra_src_start_x + 1;
  fe_r_info->buffer_height =
  stripe_info->extra_src_end_y - stripe_info->extra_src_start_y + 1;
  fe_r_info->buffer_stride = stripe_info->src_stride;
  fe_r_info->left_pad = stripe_info->extra_pad_left;
  fe_r_info->right_pad = stripe_info->extra_pad_right;
  fe_r_info->top_pad = stripe_info->extra_pad_top;
  fe_r_info->bottom_pad = stripe_info->extra_pad_bottom;

  fe_r_info->block_width =
    fe_r_info->buffer_width + fe_r_info->left_pad + fe_r_info->right_pad;
  fe_r_info->block_height =
    fe_r_info->buffer_height + fe_r_info->top_pad + fe_r_info->bottom_pad;
  CPP_STRIPE_DSDN("Debug CPP CDS (DSDN) fetch engine info\n");
  cpp_debug_fetch_engine_info(fe_r_info);
}

static void cpp_debug_dsdn_info(cpp_dsdn_info *dsdn_info)
{
  if (!(IS_VALID_MASK(g_cpp_log_featureMask, CPP_STRIPE_DSDN_BIT)))
    return;

  CPP_STRIPE_DSDN("horizontal_cnt32: %d\n", dsdn_info->horizontal_cnt32);
  CPP_STRIPE_DSDN("vertical_cnt32: %d\n", dsdn_info->vertical_cnt32);
  CPP_STRIPE_DSDN("horizontal_cnt64: %d\n", dsdn_info->horizontal_cnt64);
  CPP_STRIPE_DSDN("vertical_cnt64: %d\n", dsdn_info->vertical_cnt64);
  CPP_STRIPE_DSDN("pre_upscale_top_crop: %d\n", dsdn_info->pre_upscale_top_crop);
  CPP_STRIPE_DSDN("pre_upscale_left_crop: %d\n", dsdn_info->pre_upscale_left_crop);
  CPP_STRIPE_DSDN("horizontal_cnt64: %d\n", dsdn_info->horizontal_cnt64);
  CPP_STRIPE_DSDN("horizontal_cnt64: %d\n", dsdn_info->horizontal_cnt64);
  CPP_STRIPE_DSDN("upscale_out_width: %d\n", dsdn_info->upscale_out_width);
  CPP_STRIPE_DSDN("upscale_out_height: %d\n", dsdn_info->upscale_out_height);
}

void cpp_prepare_dsdn_stripe_info(struct cpp_plane_info_t *plane_info,
  int stripe_num)
{
  struct cpp_stripe_info *stripe_info1 =
    &plane_info->stripe_info1[stripe_num];
  struct cpp_striping_algorithm_info *stripe_info =
    &plane_info->stripe_info[stripe_num];
  cpp_dsdn_info *dsdn_info = &stripe_info1->dsdn_info;

  dsdn_info->vertical_cnt32 = stripe_info->extra_initial_vertical_count[0];
  dsdn_info->horizontal_cnt32 = stripe_info->extra_initial_horizontal_count[0];
  dsdn_info->vertical_cnt64 = stripe_info->extra_initial_vertical_count[1];
  dsdn_info->horizontal_cnt64 = stripe_info->extra_initial_horizontal_count[1];
  dsdn_info->pre_upscale_top_crop = stripe_info->extra_top_crop ;
  dsdn_info->pre_upscale_left_crop = stripe_info->extra_left_crop;
  dsdn_info->upscale_out_width = stripe_info->extra_upscale_width;
  dsdn_info->upscale_out_height = stripe_info->extra_upscale_height;
  cpp_debug_dsdn_info(dsdn_info);
}

static void cpp_debug_mmu_prefetch_info(cpp_plane_mmu_prefetch_info *mmu_prefetch_info)
{

  if (!(IS_VALID_MASK(g_cpp_log_featureMask, CPP_STRIPE_MMU_BIT)))
    return;

  CPP_STRIPE_MMU("l1_mmu_pf_en: %d", mmu_prefetch_info->l1_mmu_pf_en);
  CPP_STRIPE_MMU("l1_period: %d", mmu_prefetch_info->l1_period);
  CPP_STRIPE_MMU("l1_direction: %d", mmu_prefetch_info->l1_direction);
  CPP_STRIPE_MMU("addr_min: %d", mmu_prefetch_info->addr_min);
  CPP_STRIPE_MMU("addr_max: %d", mmu_prefetch_info->addr_max);
  CPP_STRIPE_MMU("l2_mmu_pf_en: %d", mmu_prefetch_info->l2_mmu_pf_en);
  CPP_STRIPE_MMU("l2_range: %d", mmu_prefetch_info->l2_range);
  CPP_STRIPE_MMU("l2_distance: %d", mmu_prefetch_info->l2_distance);
}

void cpp_prepare_mmu_prefetch_fetch_engine_config(
  struct cpp_plane_info_t *plane_info, int32_t plane __unused)
{
  cpp_plane_mmu_prefetch_info *mmu_prefetch_fe_info =
    &plane_info->mmu_prefetch_fe_info;
  cpp_plane_mmu_prefetch_info *mmu_prefetch_ref_fe_info =
    &plane_info->mmu_prefetch_ref_fe_info;

  uint32_t plane_len = ((plane_info->src_stride * plane_info->src_height) - 1);

  /*
   * l1 period and direction is zero for both primary & reference fetch engine.
   * This is initialized to zero by memset hw params structure.
   * l2 params not present for fetch engine.
   * TODO: discuss do we use a seperate data structure or use the same one
   */

  mmu_prefetch_fe_info->l1_mmu_pf_en = MMU_PREFETCH_ENABLE;
  mmu_prefetch_fe_info->addr_min     = plane_info->source_address[0];
  mmu_prefetch_fe_info->addr_max     = plane_info->source_address[0] +
    plane_len;

  CPP_STRIPE_MMU("MMU PREFETCH FE INFO");
  cpp_debug_mmu_prefetch_info(mmu_prefetch_fe_info);

  if (!plane_info->tnr_enable)
    return;
  mmu_prefetch_ref_fe_info->l1_mmu_pf_en = MMU_PREFETCH_ENABLE;
  mmu_prefetch_ref_fe_info->addr_min = plane_info->temporal_source_address[0];

  /* update the plane length for reference buffer based on scalar position */
  plane_len = plane_info->denoise_after_scale_en ?
    ((plane_info->temporal_stride * plane_info->dst_height) - 1) :
    plane_len;

  /* No update for cpp cds (dsdn) required as same input buffer is used by ref engine */
  mmu_prefetch_ref_fe_info->addr_max =
    plane_info->temporal_source_address[0] + plane_len;

  CPP_STRIPE("MMU PREFETCH REFERENCE FE INFO");
  cpp_debug_mmu_prefetch_info(mmu_prefetch_ref_fe_info);
}

void cpp_prepare_mmu_prefetch_write_engine_config(
  struct cpp_plane_info_t *plane_info, int32_t plane)
{
  cpp_plane_mmu_prefetch_info *mmu_prefetch_we_info =
    &plane_info->mmu_prefetch_we_info;
  cpp_plane_mmu_prefetch_info *mmu_prefetch_dup_we_info =
    &plane_info->mmu_prefetch_dup_we_info;
  cpp_plane_mmu_prefetch_info *mmu_prefetch_ref_we_info =
    &plane_info->mmu_prefetch_ref_we_info;

  uint32_t plane_len = (plane_info->output_plane_config.plane_len - 1);
  int32_t l2_range = 0, l2_distance = 0, l1_dir = 0, l2_mmu_pf_en = 0;
  struct cpp_stripe_info *stripe_info1 = &plane_info->stripe_info1[0];
  cpp_rot_cfg rot_cfg =  stripe_info1->rot_info.rot_cfg;

  if (!plane_info->tile_output_enable) {

    /* l1 mmu prefetch same for duplicate and regular  write engines */
    mmu_prefetch_we_info->l1_mmu_pf_en =
    mmu_prefetch_dup_we_info->l1_mmu_pf_en = MMU_PREFETCH_ENABLE;

    /* addr min parameter for reference, duplicate and regular write engine */
    mmu_prefetch_we_info->addr_min = plane_info->destination_address[0];
    mmu_prefetch_dup_we_info->addr_min = plane_info->destination_address[1];

    /* addr max parameter for reference, duplicate and regular write engine */
    mmu_prefetch_we_info->addr_max = plane_info->destination_address[0] +
      plane_len;
    mmu_prefetch_dup_we_info->addr_max = plane_info->destination_address[1] +
      plane_len;



    /*
     * l1 period is zero for all write engines, memset part of hw params
     * l1 direction is 1 for 180 degree rotation with no flip or HFlip
     * OR Rot 0 with V or HV flip
     * l2 MMU prfetch enable and l2 range is configured for 90 degree rotation
     * combinations.
     * l2 distance is plane specific and is configured for 90 degree rotation
     * combinations
     *
     */
    switch (rot_cfg) {
      case ROT_0_V_FLIP:
      case ROT_0_HV_FLIP:
        l1_dir = 1;
        break;
      case ROT_90:
      case ROT_90_V_FLIP:
        l2_mmu_pf_en = MMU_PREFETCH_ENABLE;
        l2_distance = plane ? 0x3f00 : 0x3f80;
        l2_range = plane_len;
        break;
      case ROT_90_H_FLIP:
      case ROT_90_HV_FLIP:
        l2_mmu_pf_en = MMU_PREFETCH_ENABLE;
        l2_distance = plane ? 0x0100 : 0x0080;
        l2_range = plane_len;
        break;
      case ROT_0:
      case ROT_0_H_FLIP:
      default:
        break;
    }

    mmu_prefetch_we_info->l1_direction =
      mmu_prefetch_dup_we_info->l1_direction =
      l1_dir;

    /* l2 mmu prefetch param is same for regular and duplicate write engines */
    mmu_prefetch_we_info->l2_mmu_pf_en =
      mmu_prefetch_dup_we_info->l2_mmu_pf_en =
      l2_mmu_pf_en;
    mmu_prefetch_we_info->l2_range =
      mmu_prefetch_dup_we_info->l2_range =
      l2_range;
    mmu_prefetch_we_info->l2_distance =
      mmu_prefetch_dup_we_info->l2_distance =
      l2_distance;

    CPP_STRIPE_MMU("MMU PREFETCH WE INFO");
    cpp_debug_mmu_prefetch_info(mmu_prefetch_we_info);
    CPP_STRIPE_MMU("MMU PREFETCH DUPLICATE WE INFO");
    cpp_debug_mmu_prefetch_info(mmu_prefetch_dup_we_info);
  }

  /* Reference write engine parameters for mmu prefetch */

  if (!plane_info->tnr_enable)
    return;
  /*
   * All l2 parameters are not applicable.
   * l1 period and l1 direction is not applicable
   * l1 enable based on prefetch enable from hw params
   * min address is temporal  destination address
   */
  mmu_prefetch_ref_we_info->l1_mmu_pf_en = MMU_PREFETCH_ENABLE;
  mmu_prefetch_ref_we_info->addr_min = plane_info->temporal_destination_address[0];

  /* update the plane length for reference buffer based on scale position */
  plane_len = plane_info->denoise_after_scale_en ?
    (plane_info->output_plane_config.plane_len - 1) :
    (plane_info->input_plane_config.plane_len - 1);

  mmu_prefetch_ref_we_info->addr_max =
    plane_info->temporal_destination_address[0] +
    plane_len;

  CPP_STRIPE_MMU("MMU PREFETCH REFERENCE WE INFO");
  cpp_debug_mmu_prefetch_info(mmu_prefetch_ref_we_info);
}
