/* autoltm.c
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "isp_log.h"
#include "autoltm.h"

#define CONST_LUT_SIZE (128)
#define MAX_LTM_GAIN (7.9f)

#ifdef DEBUG_AUTOLTM
  #define AUTOLTM_LOG(fmt, args...) \
    {ALOGE("AUTOLTM %s:%d  "fmt"\n", __func__ , __LINE__ , ##args); }
#else
  #define AUTOLTM_LOG(fmt, args...) do{}while(0)
#endif

#define AUTOLTM_VERSION  "1.0.0"

static int init_flag = 0;

typedef struct autoltm_obj_s
{
  float* gain_lut;
  float* gaussian_hist;
  float* gaussian_cdf;
  float* bhist;
} autoltm_obj_t;

static int  __mask_rect[CONST_LUT_SIZE] = {
  -8192, -8192, -8192, -8192, -7982, -7772, -7578, -7384, -7210,
  -7036, -6878, -6720, -6574, -6428, -6290, -6152, -6020, -5888, -5760,
  -5632, -5504, -5376, -5248, -5120, -4992, -4864, -4736, -4608, -4480,
  -4352, -4224, -4096, -3968, -3840, -3712, -3584, -3456, -3328, -3200,
  -3072, -2944, -2816, -2688, -2560, -2432, -2304, -2176, -2048, -1920,
  -1792, -1664, -1536, -1408, -1280, -1152, -1024, -896, -768, -640,
  -512, -384, -256, -128, 0, 128, 256, 384, 512, 640,
  768, 896, 1024, 1152, 1280, 1408, 1536, 1664, 1792, 1920,
  2048, 2176, 2304, 2432, 2560, 2688, 2816, 2944, 3072, 3200,
  3328, 3456, 3584, 3712, 3840, 3968, 4096, 4224, 4352, 4480,
  4608, 4736, 4864, 4992, 5120, 5248, 5376, 5504, 5632, 5760,
  5888, 6016, 6144, 6278, 6412, 6554, 6696, 6854, 7012, 7192,
  7372, 7572, 7772, 7980, 8191, 8191, 8191, 8191, 8191};

static int  __master_curve[CONST_LUT_SIZE] = {
  64, 128, 192, 256, 316, 376, 436, 496, 550,
  604, 656, 708, 752, 796, 838, 880, 916, 952, 980,
  1008, 1032, 1056, 1076, 1096, 1112, 1128, 1136, 1144, 1150,
  1156, 1160, 1164, 1164, 1164, 1160, 1156, 1150, 1144, 1138,
  1132, 1124, 1116, 1106, 1096, 1086, 1076, 1064, 1052, 1040,
  1028, 1016, 1004, 992, 980, 966, 952, 940, 928, 914,
  900, 888, 876, 862, 848, 836, 824, 810, 796, 784,
  772, 758, 744, 732, 720, 706, 692, 680, 668, 654,
  640, 628, 616, 602, 588, 576, 564, 550, 536, 522,
  508, 496, 484, 470, 456, 444, 432, 418, 404, 392,
  380, 366, 352, 340, 328, 314, 300, 288, 276, 262,
  248, 234, 220, 208, 196, 184, 172, 158, 144, 132,
  120, 106, 92, 78, 64, 52, 40, 30, 20};
/*
static int  __master_scale[CONST_LUT_SIZE] = {
  8000, 7553, 7124, 6714, 6322, 5948, 5590, 5249, 4923, 4613,
  4318, 4037, 3771, 3517, 3277, 3049, 2833, 2629, 2436, 2254,
  2082, 1921, 1769, 1626, 1492, 1366, 1249, 1139, 1037, 942,
  853, 771, 695, 624, 560, 500, 445, 395, 349, 308,
  270, 236, 205, 177, 152, 130, 111, 93, 78, 65,
  53, 43, 35, 28, 22, 17, 13, 10, 7, 5,
  3, 2, 1, 1, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0};
*/
static int  __shift_scale[CONST_LUT_SIZE] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0};

static int  __shift_curve[CONST_LUT_SIZE] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0};

void* autoltm_init(void)
{
  AUTOLTM_LOG("init entering\n");
  float* gaussian_cdf = (float*)CAM_CALLOC(CONST_LUT_SIZE, sizeof(float));
  if (gaussian_cdf == NULL) {
    return NULL;
  }

  float* gain_lut = (float*)CAM_CALLOC(CONST_LUT_SIZE, sizeof(float));
  if (gaussian_cdf == NULL) {
    free(gaussian_cdf);
    return NULL;
  }

  float* bhist = (float*)CAM_CALLOC(CONST_LUT_SIZE, sizeof(float));
  if (bhist == NULL) {
    free(gaussian_cdf);free(gain_lut);
    return NULL;
  }

  float* gaussian_hist = (float*)CAM_CALLOC(CONST_LUT_SIZE, sizeof(float));
  if (gaussian_hist == NULL) {
    free(gaussian_cdf);free(gain_lut); free(bhist);
    return NULL;
  }

  autoltm_obj_t* self = (autoltm_obj_t*)CAM_CALLOC(1, sizeof(autoltm_obj_t));
  if (self == NULL) {
    free(gaussian_cdf);free(gain_lut);free(bhist);free(gaussian_hist);
    return NULL;
  }

  self->gain_lut = gain_lut;
  self->gaussian_cdf = gaussian_cdf;
  self->gaussian_hist = gaussian_hist;
  self->bhist = bhist;
  init_flag = 1;

  AUTOLTM_LOG("init sucess, version is %s\n",AUTOLTM_VERSION);
  return ((void*)self);
}

int autoltm_deinit(void* ptr)
{
  AUTOLTM_LOG("deinit entering\n");
  autoltm_obj_t* handler = (autoltm_obj_t*)ptr;

  if (!handler) {
    return -1;
  }

  if (handler->gain_lut) {
    free(handler->gain_lut);
  }

  if (handler->gaussian_cdf) {
    free(handler->gaussian_cdf);
  }

  if (handler->bhist) {
    free(handler->bhist);
  }

  if (handler->gaussian_hist) {
    free(handler->gaussian_hist);
  }

  free(handler);
  init_flag = 0;

  AUTOLTM_LOG("deinit sucess\n");
  return 0;
}

static void autoltm_histgen(unsigned int* bhist, float* bhist_ds)
{
  int x;
  if ((NULL == bhist_ds) || (NULL == bhist)) {
    return;
  }

  memset(bhist_ds, 0, CONST_LUT_SIZE*sizeof(unsigned int));
  int pixel_cnt = 0;
  for (x = 0; x < 256; x+=2) {
    bhist_ds[x/2]  = bhist[x] + bhist[x+1];
    pixel_cnt += bhist_ds[x/2];
  }

  for (x = 0; x < CONST_LUT_SIZE; x++) {
    bhist_ds[x] = bhist_ds[x] / (float)pixel_cnt;
  }
}

static void autoltm_histogram_matching
  (float* cdf2, float* cdf1, float* gain_lut, autoltm_tuning_t* tuning)
{
  int x = 0, y = 0;
  float delta = 0;
  int low_code_tone_end = tuning->low_code_tone_end;
  float max_gain_limit = tuning->ltm_gain_limit;
  int mid_tone_start = tuning->mid_tone_start;
  int mid_tone_end = tuning->mid_tone_end;
  float smear_prev_low_limit = tuning->smear_prev_low_limit;
  float smear_prev_high_limit = tuning->smear_prev_high_limit;

  for (x = 1; x < CONST_LUT_SIZE; x++) {
    cdf1[x] = cdf1[x] + cdf1[x-1];
  }

  for (x = 0; x < CONST_LUT_SIZE; x++) {
    float temp_y = cdf1[x];
    for (y = 1; y < CONST_LUT_SIZE; y++) {
      if (cdf2[y] > temp_y) {
        goto BREAK;
      }
    }

    if (y == CONST_LUT_SIZE) {
      y = CONST_LUT_SIZE-1;
    }

BREAK:
    delta = cdf2[y] - cdf2[y-1];
    if (delta == 0) {
      temp_y = y;
    } else {
      temp_y = y - (cdf2[y] - temp_y) / delta;
    }

    temp_y = temp_y / (x+1);

    gain_lut[x] = (temp_y) - 1.0f;

    float _max_gain = max_gain_limit;
    if (_max_gain > MAX_LTM_GAIN) {
      _max_gain = MAX_LTM_GAIN;
    }

    if (gain_lut[x] < 0.01f) {
      gain_lut[x] = 0.01f;
    }

    if (gain_lut[x] > _max_gain) {
      gain_lut[x] = _max_gain;
    }
  }

  float max_gain = 0.0f;

  for (x = 0; x < low_code_tone_end; x++) {
    if (gain_lut[x] > max_gain) {
      max_gain = gain_lut[x];
    }
  }

  for (x = 0; x < CONST_LUT_SIZE; x++) {
    gain_lut[x] = gain_lut[x]/max_gain;
  }

  if (max_gain < 1.0f) {
    max_gain = 1.0f;
  }

  float log_0p5 = (float)log(0.5f);
  float offset = log(max_gain) / log_0p5;

  float b = 0.0f;
  int sample_cnt = 0;
  for (x = mid_tone_start; x < mid_tone_end; x++) {
    float tmp = gain_lut[x];
    tmp = log(tmp) / log_0p5;
    tmp = tmp - offset;
    tmp = tmp / (float)x;
    b = b + tmp;
  }

  b = b / ((float)(mid_tone_end - mid_tone_start));

  if (b < smear_prev_low_limit) {
    b = smear_prev_low_limit;
  }
  if (b > smear_prev_high_limit) {
    b = smear_prev_high_limit;
  }

  AUTOLTM_LOG("a = %f, b = %f\n", max_gain, b);

  for (x=0; x < CONST_LUT_SIZE; x++) {
    gain_lut[x] = max_gain * (float)pow(0.5, (b * (float)x));
  }
}

static void autoltm_gaussian_cdf_gen
  (float* gaussian_hist, float* cdf, int target_luma, float sigma)
{
  int x = 0;

  for (x = 0; x < CONST_LUT_SIZE; x++) {
    float tmp = (float)x - (float)target_luma / 2.0f;
    tmp = tmp / sigma;
    tmp = -1.0f * tmp * tmp;
    gaussian_hist[x] = (float)exp(tmp);
  }
  cdf[0] = gaussian_hist[0];

  for (x = 1; x < CONST_LUT_SIZE; x++) {
    cdf[x] = cdf[x-1] + gaussian_hist[x];
  }

  float scale = 1.0f / cdf[CONST_LUT_SIZE-1];

  for (x = 0; x < CONST_LUT_SIZE; x++) {
    gaussian_hist[x] = gaussian_hist[x] * scale;
  }

  cdf[0] = gaussian_hist[0];

  for (x = 1; x < CONST_LUT_SIZE; x++) {
    cdf[x] = cdf[x-1] + gaussian_hist[x];
  }
}

static void autoltm_normalize_gain(float* gain_lut, ltm_curves_t* ltm_curves)
{
  int x;
  if (init_flag) {
    for (x = 0; x < CONST_LUT_SIZE; x++) {
      ltm_curves->mask_rect_curve[x] = __mask_rect[x];
    }

    for (x = 0; x < CONST_LUT_SIZE; x++) {
      ltm_curves->master_curve[x] = (double)__master_curve[x];
    }

    for (x = 0; x < CONST_LUT_SIZE; x++) {
      ltm_curves->shift_curve[x] = (double)__shift_curve[x];
    }

    for (x = 0; x < CONST_LUT_SIZE; x++) {
      ltm_curves->shift_scale[x] = (double)__shift_scale[x];
    }

    for (x = 0; x < CONST_LUT_SIZE; x++) {
      ltm_curves->master_scale[x] = (double)0.0;
    }

    init_flag = 0;
  }

  for (x = 0; x < CONST_LUT_SIZE; x++) {
    ltm_curves->master_scale[x] = (double)(gain_lut[x] * 1024.0f);
  }
}

int autoltm_main(void* context, autoltm_input_t* input, autoltm_tuning_t* tuning,
  ltm_curves_t* ltm_curves)
{
  AUTOLTM_LOG("main entering\n");

  if (!(context && input && tuning && ltm_curves)) {
    return -1;
  }

  autoltm_obj_t* self = (autoltm_obj_t*)context;
  unsigned int* bhist = input->bhist;
  float* gaussian_cdf = self->gaussian_cdf;
  float* gaussian_hist = self->gaussian_hist;
  float* gain_lut = self->gain_lut;

  if (!(gaussian_cdf && gain_lut && self->bhist)) {
    AUTOLTM_LOG("%s%d null pointer\n", __func__, __LINE__);
    return -1;
  }

  autoltm_gaussian_cdf_gen(
    gaussian_hist, gaussian_cdf, tuning->brightness_target, tuning->contrast_target);
  autoltm_histgen(bhist, self->bhist);
  autoltm_histogram_matching(gaussian_cdf, self->bhist, gain_lut, tuning);
  autoltm_normalize_gain(gain_lut, ltm_curves);

  AUTOLTM_LOG("main sucess, exit\n");
  return 1;
}
