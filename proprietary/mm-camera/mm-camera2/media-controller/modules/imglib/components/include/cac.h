/***************************************************************************
* Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
****************************************************************************/

#ifndef __CAC_H__
#define __CAC_H__

#include "img_common.h"

/** cac_3a_info_t
 *   @awb_gr_gain: Whitebalance gr gain
 *   @awb_gb_gain: Whitebalance gb gain
 *   @lux_idx:  Lux Idx
 *   @gain : AEC gain
 *
 *   3a info
 **/
typedef struct {
  float awb_gr_gain;
  float awb_gb_gain;
  float lux_idx;
  float gain;
} cac_3a_info_t;

/** cac_v1_chromatix_info_t
 *   @edgeTH: edge detection threshold
 *   @saturatedTH: Y component saturation threshold
 *   @chrom0LowTH: R/G hue low threshold
 *   @chrom0HighTH: R/G hue high threshold
 *   @chrom1LowTH: B/G hue low threshold
 *   @chrom1HighTH: B/G hue low threshold
 *   @chrom0LowDiffTH: R/G hue difference low threshold
 *   @chorm0HighDiffTH: R/G hue difference high threshold
 *   @chrom1LowDiffTH: B/G hue difference low threshold
 *   @chorm1HighDiffTH: B/G hue difference high threshold
 *
 *  CAC Chromatix info
 **/
typedef struct {
  int16_t edgeTH;
  uint8_t saturatedTH;
  int32_t chrom0LowTH;
  int32_t chrom0HighTH;
  int32_t chrom1LowTH;
  int32_t chrom1HighTH;
  int32_t chrom0LowDiffTH;
  int32_t chorm0HighDiffTH;
  int32_t chrom1LowDiffTH;
  int32_t chorm1HighDiffTH;
} cac_v1_chromatix_info_t;

/** cac_v2_chromatix_info_t
 *   @bright_spot_highTH: high threshold
 *   @bright_spot_lowTH: low threshold
 *   @saturation_TH: saturation threshold
 *   @color_Cb_TH: color threshold for cb
 *   @color_Cr_TH: color threshold for cr
 *   @corrRatio_TH: ration threshold
 *
 *  CAC V2 Chromatix info
 **/
typedef struct {
  int32_t detection_th1;
  int32_t detection_th2;
  int32_t detection_th3;
  int32_t verification_th1;
  int32_t correction_strength;
} cac_v2_chromatix_info_t;

/** cac_v3_chromatix_info_t
 *   @detection_th1: Bright spot High
 *   @detection_th2: saturation threshold
 *   @verification_th1: Verification threshold1
 *   @verification_th2: Verification threshold2
 *
 *  CAC V3 Chromatix info
 **/
typedef struct {
  int32_t detection_th1;
  int32_t detection_th2;
  int32_t verification_th1;
  int32_t verification_th2;
} cac_v3_chromatix_info_t;

/** rnr_chromatix_info_t
 *   @sampling_factor: hRNR downsample/upsample factor
 *   @sigma_lut: Pointer to RNR sigma (threshold) lookup table,
 *               162 length
 *   @lut_size: Size of the sigma_lut
 *   @scale_factor: Size of the sigma_lut
 *   @center_noise_sigma: center ratio
 *   @center_noise_weight: default 1.0
 *   @weight_order: 2.0f if sampling factor=2, 1.5f if sampling
 *                factor=4, 1.0f if sampling factor=8
 *
 *  CAC V2 Chromatix info
 **/
typedef struct {
  uint8_t sampling_factor;
  float sigma_lut[RNR_LUT_SIZE];
  int lut_size;
  float scale_factor;
  float center_noise_sigma;
  float center_noise_weight;
  float weight_order;
} rnr_chromatix_info_t;

/** skin_rnr_info_t
 *   @skin_sigma_lut: Skin RNR sigma Lookup table
 *   @skin_sigma_lut_size: Size of the skin_sigma_lut
 *   @skin_strength: Skin strength
 *   @skin_cr: Skin Cr
 *   @skin_cb: Skin CB
 *   @skin_ymin: Skin Y minimum
 *   @skin_ymax: Skin Y maximim
 *   @skin_scaler: Skin scaler
 *   @skin_threshold: Skin Threshold
 *   @skin_sigma_scale: Skin Sigma Scale
 *   @skin_chroma_ds_factor: Chroma Down Scale Factor
 *   @ds_us_skin_detection: Skin detection downscale factor
 *
 *  Skin RNR Configuration
 **/
typedef struct {
  uint8_t  s_rnr_enabled;
  float    skin_sigma_lut[SKINR_LUT_SIZE];
  int      skin_sigma_lut_size;
  float    skin_strength; // skin_slope = 1/skin_strength
  uint8_t  skin_cr;
  uint8_t  skin_cb;
  uint8_t  skin_ymin;
  uint8_t  skin_ymax;
  uint16_t skin_scaler;
  float skin_threshold;
  float skin_sigma_scale;
  int   skin_chroma_ds_factor;
  uint8_t  ds_us_skin_detection;
} skin_rnr_info_t;

/** cac_chroma_order
 *   @CAC_CHROMA_ORDER_CBCR: Order is CBCR
 *   @CAC_CHROMA_ORDER_CRCB: order is CrCb
 *
 * CAC Chroma order
 **/
typedef enum {
  CAC_CHROMA_ORDER_CBCR = 0,
  CAC_CHROMA_ORDER_CRCB
} cac_chroma_order;

/** face_dt_rect_t
 *   @x: left cordinate
 *   @y: top cordinate
 *   @dx: width of window
 *   @dy: height of the window
 *
 *   faceproc rect information
 **/
typedef struct {
  uint32_t x;
  uint32_t y;
  uint32_t dx;
  uint32_t dy;
} face_dt_rect_t;

/** face_detection_data_t
 *   @num_faces_detected: Number of detected faces
 *   @faces: Detailed information of faces detected
 *
 *   faceproc detection information
 **/
typedef struct {
  uint8_t num_faces_detected;
  face_dt_rect_t faces[MAX_FD_ROI];
} face_detection_data_t;

/** hysterisis_trend
 * @HYSTERISIS_TREND_UPWARD : values increased
 * @HYSTERISIS_TREND_DOWNWARD: value decresed
 * @HYSTERISIS_TREND_NONE: Trend not recorded yet
**/
typedef enum {
  HYSTERISIS_TREND_NONE,
  HYSTERISIS_TREND_UPWARD,
  HYSTERISIS_TREND_DOWNWARD,
} hysterisis_trend;

/** hysterisis_info_t
 * @prev_lux_value: Previous lux value
 * @lux_trend: Lux bases hysterisis trend
 * @prev_gain_value: Previous gain value
 * @gain_trend: Gain Based hysterisis trend
 * @norml_hyst_enabled: Flag indicating normal light hysterisis
 *                    is enabled
 * @lowl_hyst_enabled: Flag indicating low light hysterisis
 *                    is enabled
 * @prev_sampling_factor: Previous sampling factor
 * @lux_idx: LUX index value
 * @gain: Gain value
 * @s_rnr_enabled: if skin rnr is enabled
**/
typedef struct {
  float prev_lux_value;
  hysterisis_trend lux_trend;
  float prev_gain_value;
  hysterisis_trend gain_trend;
  uint8_t norml_hyst_enabled;
  uint8_t lowl_hyst_enabled;
  uint8_t prev_sampling_factor;
  float lux_idx;
  float gain;
  uint32_t s_rnr_enabled;
} hysterisis_info_t;

/* Setprop CAC3 flag shift values*/
#define CAC3_CAC_FLAG   0
#define CAC3_RNR_FLAG   1
#define CAC3_SRNR_FLAG  2

/** CAC parameters
 *
 **/
#define QCAC_RGAMMA_TABLE     (QIMG_CAC_PARAM_OFF +  1)
#define QCAC_GGAMMA_TABLE     (QIMG_CAC_PARAM_OFF +  2)
#define QCAC_BGAMMA_TABLE     (QIMG_CAC_PARAM_OFF +  3)
#define QCAC_CHROMATIX_INFO   (QIMG_CAC_PARAM_OFF +  4)
#define QCAC_3A_INFO          (QIMG_CAC_PARAM_OFF +  5)
#define QCAC_CHROMA_ORDER     (QIMG_CAC_PARAM_OFF +  6)
#define QCDS_ENABLED          (QIMG_CAC_PARAM_OFF +  7)
#define QLDS_ENABLED          (QIMG_CAC_PARAM_OFF +  8)
#define QFD_RESULT            (QIMG_CAC_PARAM_OFF +  9)
#define QYUYV422_TO_420_ENABLED (QIMG_CAC_PARAM_OFF +  10)
#define QCDS_PROCESS          (QIMG_CAC_PARAM_OFF +  11)

#endif

