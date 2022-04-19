/*============================================================================

  Copyright (c) 2015 - 2016 Qualcomm Technologies, Inc.
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


#define CPP_FW_CMD_HEADER           0x3E646D63
#define CPP_FW_CMD_TRAILER          0xabcdefaa
#define CPP_FW_CMD_ID_PROCESS_FRAME 0x6
/* cpp_pack_asf_kernel_1_5:
*
* @frame_msg: pointer to CPP frame payload
* @filter: pointer to ASF H and V filter
* Description:
*     Packs ASF filter values into frame payload as per firmware
*     register.
* Return: void
**/
void cpp_pack_asf_kernel_1_5(uint32_t *frame_msg, int16_t *filter)
{
  int i;

  for (i = 0; i < 25; i++) {
    if ( i == 24 ) {
      // Center coeff
      filter[i] = clamp(filter[i], -2048, 2047);
    } else if (((i+1) % 5 == 0) || ((i >= 20) && (i <= 23))) {
      // Axes coeff
      filter[i] = clamp(filter[i], -1024, 1023);
    } else {
      // Quandrant coeff
      filter[i] = clamp(filter[i], -512, 511);
    }
  }
  frame_msg[0] = ((filter[1]) << 16) | ((filter[0]) & 0x3FF);
  frame_msg[1] = ((filter[3]) << 16) | ((filter[2]) & 0x3FF);
  frame_msg[2] = ((filter[5]) << 16) | ((filter[4]) & 0x7FF);
  frame_msg[3] = ((filter[7]) << 16) | ((filter[6]) & 0x3FF);
  frame_msg[4] = ((filter[9]) << 16) | ((filter[8]) & 0x3FF);
  frame_msg[5] = ((filter[11]) << 16) | ((filter[10]) & 0x3FF);
  frame_msg[6] = ((filter[13]) << 16) | ((filter[12]) & 0x3FF);
  frame_msg[7] = ((filter[15]) << 16) | ((filter[14]) & 0x7FF);
  frame_msg[8] = ((filter[17]) << 16) | ((filter[16]) & 0x3FF);
  frame_msg[9] = ((filter[19]) << 16) | ((filter[18]) & 0x3FF);
  frame_msg[10] = ((filter[21]) << 16) | ((filter[20]) & 0x7FF);
  frame_msg[11] = ((filter[23]) << 16) | ((filter[22]) & 0x7FF);
  frame_msg[12] = (filter[24] & 0xFFF);
}

/* cpp_create_frame_message_1_5:
*
* @cpp_frame_info: pointer to CPP frame info
* @len: pointer to CPP frame payload length
* Description:
*     Creates/packs CPP frame payload for firmware version
*     1.5 before sending to CPP firmware.
*
* Return: int*
*     Pointer to CPP frame payload
**/
uint32_t* cpp_create_frame_message_1_5(
  struct cpp_frame_info_t *cpp_frame_info, uint32_t* len,
  cpp_firmware_version_t fw_version)
{
  struct cpp_stripe_info *stripe_info = NULL;
  uint32_t i = 0, j = 0, k = 0, num_stripes = 0, msg_len = 0;
  uint32_t *frame_msg = NULL;
  uint16_t LUT1[24] = {0};
  uint16_t LUT2[24] = {0};
  uint16_t LUT3[12] = {0};
  int16_t  F1[25] = {0};
  int16_t  F2[25] = {0};
  int16_t  F3[25] = {0};
  int16_t  F4[25] = {0};
  int16_t  F5[25] = {0};
  int32_t LUT1_Value[24] = {0};
  int32_t LUT1_Delta[24] = {0};
  int32_t LUT2_Value[24] = {0};
  int32_t LUT2_Delta[24] = {0};
  int32_t LUT3_Value[12] = {0};
  int32_t LUT3_Delta[12] = {0};
  int checksum = 0;
  uint8_t checksum_enable = 0;
  uint8_t checksum_lpf_enable = 0;
  struct cpp_asf_info *asf_info = &(cpp_frame_info->asf_info);
  uint32_t checksum_mask = asf_info->checksum_en;

  if (asf_info->sp_eff_en == 1) {
    checksum_mask = 0;
  }

  for (i =0; i < cpp_frame_info->num_planes; i++) {
    num_stripes += cpp_frame_info->plane_info[i].num_stripes;
  }
  /*frame info size*/
  msg_len = 156 + 27 * num_stripes + 1;
  /*Total message size = frame info + header + length + trailer*/
  frame_msg = CAM_MALLOC(sizeof(uint32_t) * msg_len);
  if (!frame_msg){
      CPP_ERR("CAM_MALLOC() failed");
      return NULL;
  }
  memset(frame_msg, 0, sizeof(uint32_t) * msg_len);

  *len = msg_len;

  /*Top Level*/
  frame_msg[0] = CPP_FW_CMD_HEADER;
  frame_msg[1] = msg_len - 3;
  frame_msg[2] = CPP_FW_CMD_ID_PROCESS_FRAME;
  frame_msg[3] = 0;
  frame_msg[4] = 0;
  /*Plane info*/
  if (fw_version == CPP_FW_VERSION_1_5_2) {
    frame_msg[5] = (cpp_frame_info->in_plane_fmt << 24) |
      (cpp_frame_info->out_plane_fmt << 16) |
      (cpp_frame_info->plane_info[0].stripe_info1[0].rot_info.rot_cfg << 13) |
      (cpp_frame_info->power_collapse << 12) |
      (cpp_frame_info->batch_info.batch_size << 1) | 0;
  } else {
    frame_msg[5] = (cpp_frame_info->in_plane_fmt << 24) |
      (cpp_frame_info->out_plane_fmt << 16) |
      (cpp_frame_info->plane_info[0].stripe_info1[0].rot_info.rot_cfg << 13) |
      (cpp_frame_info->batch_info.batch_size << 1) | 0;
  }

  /*Input Plane address HFR*/
  frame_msg[6] = cpp_frame_info->batch_info.intra_plane_offset[0];
  frame_msg[7] = cpp_frame_info->batch_info.intra_plane_offset[1];
  frame_msg[8] = cpp_frame_info->batch_info.intra_plane_offset[2];
  /*Output Plane HFR*/
  frame_msg[9] = 0x0;
  frame_msg[10] = 0x0;
  frame_msg[11] = 0x0;
  for ( i = 0; i < 25; i++) {
    F1[i] = (int16_t)(Round(asf_info->sobel_h_coeff[i]*(1<<10)));
    F2[i] = (int16_t)(Round(asf_info->sobel_v_coeff[i]*(1<<10)));
    F3[i] = (int16_t)(Round(asf_info->hpf_h_coeff[i]*(1<<10)));
    F4[i] = (int16_t)(Round(asf_info->hpf_v_coeff[i]*(1<<10)));
    F5[i] = (int16_t)(Round(asf_info->lpf_coeff[i]*(1<<10)));
  }

  checksum_enable = checksum_mask & 1;
  checksum_lpf_enable = (checksum_mask >> 1) & 1;
  if (checksum_enable) {
    checksum = 4*(F1[0] + F1[1] + F1[2] + F1[3] \
      + F1[5] + F1[6] + F1[7] + F1[8] \
      + F1[10] + F1[11] + F1[12] + F1[13] \
      + F1[15] + F1[16] + F1[17] + F1[18] )\
      + 2*(F1[4] + F1[9] + F1[14] + F1[19] \
      + F1[20] + F1[21] + F1[22] + F1[23] )
      + F1[24];
    if (checksum != (checksum_lpf_enable << 10)) {
      if ( checksum_lpf_enable )
        F1[24] = F1[24]+ ((1<<10) - checksum);
      else
        F1[24] = F1[24]- checksum;
    }
  }

  checksum_enable = (checksum_mask >> 2) & 1;
  checksum_lpf_enable = (checksum_mask >> 3) & 1;
  if (checksum_enable) {
    checksum = 4*(F2[0] + F2[1] + F2[2] + F2[3] \
      + F2[5] + F2[6] + F2[7] + F2[8] \
      + F2[10] + F2[11] + F2[12] + F2[13] \
      + F2[15] + F2[16] + F2[17] + F2[18] )\
      + 2*(F2[4] + F2[9] + F2[14] + F2[19] \
      + F2[20] + F2[21] + F2[22] + F2[23] )
      + F2[24];
    if (checksum != (checksum_lpf_enable << 10)) {
      if ( checksum_lpf_enable )
        F2[24] = F2[24]+ ((1<<10) - checksum);
      else
        F2[24] = F2[24]- checksum;
    }
  }
  checksum_enable = (checksum_mask >> 4) & 1;
  checksum_lpf_enable = (checksum_mask >> 5) & 1;
  if (checksum_enable) {
    checksum = 4*(F3[0] + F3[1] + F3[2] + F3[3] \
      + F3[5] + F3[6] + F3[7] + F3[8] \
      + F3[10] + F3[11] + F3[12] + F3[13] \
      + F3[15] + F3[16] + F3[17] + F3[18] )\
      + 2*(F3[4] + F3[9] + F3[14] + F3[19] \
      + F3[20] + F3[21] + F3[22] + F3[23] )
      + F3[24];
    if (checksum != (checksum_lpf_enable << 10)) {
      if ( checksum_lpf_enable )
        F3[24] = F3[24]+ ((1<<10) - checksum);
      else
        F3[24] = F3[24]- checksum;
    }
  }

  checksum_enable = (checksum_mask >> 6) & 1;
  checksum_lpf_enable = (checksum_mask >> 7) & 1;
  if (checksum_enable) {
    checksum = 4*(F4[0] + F4[1] + F4[2] + F4[3] \
      + F4[5] + F4[6] + F4[7] + F4[8] \
      + F4[10] + F4[11] + F4[12] + F4[13] \
      + F4[15] + F4[16] + F4[17] + F4[18] )\
      + 2*(F4[4] + F4[9] + F4[14] + F4[19] \
      + F4[20] + F4[21] + F4[22] + F4[23] )
      + F4[24];
    if (checksum != (checksum_lpf_enable << 10)) {
      if ( checksum_lpf_enable )
        F4[24] = F4[24]+ ((1<<10) - checksum);
      else
        F4[24] = F4[24]- checksum;
    }
  }

  checksum_enable = (checksum_mask >> 8) & 1;
  checksum_lpf_enable = (checksum_mask >> 9) & 1;

  if (checksum_enable) {
    checksum = 4*(F5[0] + F5[1] + F5[2] + F5[3] \
      + F5[5] + F5[6] + F5[7] + F5[8] \
      + F5[10] + F5[11] + F5[12] + F5[13] \
      + F5[15] + F5[16] + F5[17] + F5[18] )\
      + 2*(F5[4] + F5[9] + F5[14] + F5[19] \
      + F5[20] + F5[21] + F5[22] + F5[23] )
      + F5[24];
    if (checksum != (checksum_lpf_enable << 10)) {
      if ( checksum_lpf_enable )
        F5[24] = F5[24]+ ((1<<10) - checksum);
      else
        F5[24] = F5[24]- checksum;
    }
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
        ((uint32_t)(clamp(Round(bf_info->noise_threshold[j] *
        (1 << 4)), 0, 0xfff))) << 8 |
         ((uint32_t)(clamp(Round(bf_info->weight[j] *
         (1 << 4)), 0, 0x1f)));
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

  //ASF_CLAMP_TL_LIMIT
  frame_msg[70] = (asf_info->clamp_tl_ul & 0x1FF) << 9 | (asf_info->clamp_tl_ll & 0x1FF);
  frame_msg[71] = ((uint32_t)(Round(asf_info->clamp_scale_max * (1 << 4)))
                    & 0x1FF) << 16 |
                  ((uint32_t)(Round(asf_info->clamp_scale_min * (1 << 4)))
                    & 0x1FF);

  frame_msg[72] = (asf_info->clamp_offset_max & 0x7F) << 16 |
                  (asf_info->clamp_offset_min & 0x7F);

  frame_msg[73] = asf_info->nz_flag;

  cpp_pack_asf_kernel_1_5(&frame_msg[74], &F1[0]);
  cpp_pack_asf_kernel_1_5(&frame_msg[87], &F2[0]);
  cpp_pack_asf_kernel_1_5(&frame_msg[100], &F3[0]);
  cpp_pack_asf_kernel_1_5(&frame_msg[113], &F4[0]);
  cpp_pack_asf_kernel_1_5(&frame_msg[126], &F5[0]);

  frame_msg[139] = asf_info->nz_flag_f2;
  frame_msg[140] = asf_info->nz_flag_f3_f5;

  for (i = 0; i < cpp_frame_info->num_planes; i++) {
    j = 141 + i * 5;
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

  i = 156;
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
  frame_msg[msg_len-1] = CPP_FW_CMD_TRAILER;
  return frame_msg;
}

