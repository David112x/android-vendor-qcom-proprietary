/* autoltm.h
*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef __AUTOLTM_H__
#define __AUTOLTM_H__

typedef struct autoltm_tuning_s
{
  int halo_score_target;
  int brightness_target;
  float contrast_target;
  float ltm_gain_limit;
  int low_code_tone_end;
  int mid_tone_start;
  int mid_tone_end;
  float smear_prev_low_limit;
  float smear_prev_high_limit;
} autoltm_tuning_t;

typedef struct autoltm_input_s
{
  unsigned int* statsR;
  unsigned int* statsG;
  unsigned int* statsB;
  unsigned int* statsR_cnt;
  unsigned int* statsG_cnt;
  unsigned int* statsB_cnt;
  unsigned int* bhist;
} autoltm_input_t;

typedef struct ltm_curves_s
{
  int *mask_rect_curve;
  double *master_curve;
  double *master_scale;
  double *shift_curve;
  double *shift_scale;
} ltm_curves_t;

void* autoltm_init(void);
int autoltm_deinit(void* self);
int autoltm_main(void* context,
  autoltm_input_t* input, autoltm_tuning_t* tuning, ltm_curves_t* ltm_curves);

#endif
