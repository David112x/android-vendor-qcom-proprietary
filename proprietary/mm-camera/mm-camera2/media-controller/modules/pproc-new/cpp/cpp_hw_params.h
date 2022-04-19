
/*============================================================================

  Copyright (c) 2013-2017, 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#ifndef CPP_HW_PARAMS_H
#define CPP_HW_PARAMS_H

#include <media/msmb_camera.h>
#include <media/msmb_pproc.h>
#include "cam_types.h"
#include "mtype.h"
#include "modules.h"
#include "chromatix.h"
#include "chromatix_common.h"
#include "chromatix_cpp_stripped.h"
#include "eztune_diagnostics.h"
#include "cpp_wnr_params.h"

#if defined (CHROMATIX_304) || defined (CHROMATIX_306) || \
  defined (CHROMATIX_307) || defined (CHROMATIX_308) || defined (CHROMATIX_308E) || \
  defined (CHROMATIX_309)|| defined (CHROMATIX_310) || defined (CHROMATIX_310E)

#include "chromatix_cpp.h"
#include "cpp_chromatix_int.h"
#endif

#ifndef CHROMATIX_EXT
#include "cpp_hw_macro.h"
#else
#include "cpp_hw_macro_ext.h"
#endif

#define PAD_TO_32(a)               (((a)+31)&~31)
#define PAD_TO_2(a)                (((a)+1)&~1)

#define F_EQUAL(a, b) \
  ( fabs(a-b) < 1e-4 )

#define F_EQUAL_1(a, b) \
  ( fabs(a-b) < 1e-2 )

/** CPP error values
*    CPP_SUCCESS - success
*    CPP_ERR_GENERAL - any generic errors which cannot be defined
*    CPP_ERR_NO_MEMORY - memory failure ION or heap
*    CPP_ERR_NOT_SUPPORTED -  mode or operation not supported
*    CPP_ERR_INVALID_INPUT - input passed by the user is invalid
*    CPP_ERR_INVALID_OPERATION - operation sequence is invalid
*    CPP_ERR_TIMEOUT - operation timed out
*    CPP_ERR_NOT_FOUND - object is not found
*    CPP_GET_FRAME_FAILED - get frame failed
*    CPP_ERR_OUT_OF_BOUNDS - input to function is out of bounds
*    CPP_ERR_BUSY - HW is busy
*    CPP_ERR_CONNECTION_FAILED - connection failure
*
**/
#define CPP_SUCCESS                   0
#define CPP_ERR_GENERAL              -1
#define CPP_ERR_NO_MEMORY            -2
#define CPP_ERR_NOT_SUPPORTED        -3
#define CPP_ERR_INVALID_INPUT        -4
#define CPP_ERR_INVALID_OPERATION    -5
#define CPP_ERR_TIMEOUT              -6
#define CPP_ERR_NOT_FOUND            -7
#define CPP_GET_FRAME_FAILED         -8
#define CPP_ERR_OUT_OF_BOUNDS        -9
#define CPP_ERR_BUSY                 -10
#define CPP_ERR_CONNECTION_FAILED    -11

/** IS_CPP_ERROR:
 *
 * Macro to abstract if the function returns success/failure
 */
#define IS_CPP_ERROR(x) ((x) != CPP_SUCCESS)

/** CPP_SQUARE:
 *
 * Returns square of the variable
 */
#define CPP_SQUARE(x) ((x) * (x))

/** CPP_SQUARE_SUM:
 *
 * Returns sum of square of the point provided by c
 */
#define CPP_SQUARE_SUM(c) (CPP_SQUARE(c.x) + CPP_SQUARE(c.y))

/** CPP_F_TO_Q:
 *
 * Float to q conversion
 */
#define CPP_F_TO_Q(v, q) round((v) * (1 << (q)))

/** CPP_F_TO_Q_CEIL:
 *
 * Float to q conversion. ceil the output
 */
#define CPP_F_TO_Q_CEIL(v, q) ceil((v) * (1 << (q)))

/** CPP_TO_Q:
 *
 * q value of the variable
 */
#define CPP_TO_Q(q) (1 << (q))

/** CPP_IS_POW_2:
 *
 * check if the variable is multiple of 2
 */
#define CPP_IS_POW_2(x) ((x) && !((x) & ((x) - 1)))

/** CPP_ARRAY_SIZE:
 *
 * number of elements in array x
 */
#define CPP_ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

/** CPP_MIN:
 *
 * minimum of a, b
 */
#define CPP_MIN(a,b)  (((a) < (b)) ? (a) : (b))

/** CPP_MAX:
 *
 * maxiumum of a, b
 */
#define CPP_MAX(a,b)  (((a) > (b)) ? (a) : (b))

/** CPP_CLAMP:
 *
 * clamp 'x' with minimum val 'min_x' and maximum 'max_x'
 */
#define CPP_CLAMP(x, min_x, max_x) MIN(MAX((x),(min_x)),(max_x))

#if defined (CHROMATIX_304) || defined (CHROMATIX_306) || \
  defined (CHROMATIX_307) || defined (CHROMATIX_308) || defined (CHROMATIX_308E) || \
  defined (CHROMATIX_309) || defined (CHROMATIX_310) || defined (CHROMATIX_310E)

#define CPP_ASF_F_KERNEL_CHROMATIX_ENTRIES \
  (sizeof(((chromatix_asf_7_7_core_type *)0)->f1)/sizeof(float))
#define CPP_ASF_LUT12_CHROMATIX_ENTRIES \
   (sizeof(((chromatix_asf_7_7_core_type *)0)->lut1)/sizeof(float))
#define CPP_ASF_LUT3_CHROMATIX_ENTRIES \
   (sizeof(((chromatix_asf_7_7_core_type *)0)->lut3)/sizeof(float))
#else
#define CPP_ASF_F_KERNEL_CHROMATIX_ENTRIES 16
#define CPP_ASF_LUT12_CHROMATIX_ENTRIES 24
#define CPP_ASF_LUT3_CHROMATIX_ENTRIES 12
#endif

#define CPP_ASF_F_KERNEL_ENTRIES_1_5_x    25 // For 1.5 FW kernel coeffs is 25
#define CPP_ASF_F_KERNEL_ENTRIES_1_2_x    16 // For 1.2 and 1.4 FW, kernel coeffs is 16
#define CPP_ASF_F_KERNEL_ENTRIES_1_4_x    16
#define CPP_ASF_LUT12_ENTRIES_1_2_x 24
#define CPP_ASF_LUT3_ENTRIES_1_2_x 12

#define CPP_ASF_LUT_ENTRIES_1_6_x         256
#define CPP_ASF_F_KERNEL_ENTRIES_1_6_x    25

#define CPP_ASF_LUT_ENTRIES_1_12_x         256
#define CPP_ASF_RNR_LUT_ENTRIES_1_12_x    3

#define ISP_SCALOR                        0
#define CPP_SCALOR                        1

//TODO : do the CRCB version
#define UBWC_ENABLE(fmt) (((fmt) == (cpp_params_plane_fmt_t)CPP_PARAM_PLANE_CBCR_UBWC) ? 1 : 0)

#ifndef MAX_LDS_REGIONS
#ifdef LDS_ENABLE
#define MAX_LDS_REGIONS 2
#else
#define MAX_LDS_REGIONS 0
#endif
#endif

#if defined CHROMATIX_304 || defined (CHROMATIX_305)
#define NUM_GRAY_PATCHES (MAX_LIGHT_TYPES_FOR_SPATIAL)
#endif

#define ROTATION_BUFFER_SIZE 16384
#define LINE_BUFFER_SIZE 512
#define MAL_SIZE 32
#define UBWC_BANK_SPREAD_ENABLE 1
#define UBWC_HIGESHT_BANK_ENABLE 1
#define UBWC_HIGHEST_BANK_BIT 15
#define UBWC_HIGHEST_BANK_BIT_14 14
#define UBWC_COMPRESSED_OUTPUT 1
#define UBWC_UNCOMPRESSED_OUTPUT 0
#define CHROMA_TILE_FORMAT 1
#define LUMA_TILE_FORMAT 0
#define LUMA_TILE_WIDTH 32
#define CHROMA_TILE_WIDTH 16
#define TILE_HEIGHT 8
#define CHROMA_TILE_SIZE 128
#define LUMA_TILE_SIZE 256
#define MACRO_TILE 4
#define DATA_ALIGN 4096
#define META_WIDTH_ALIGN 64
#define META_HEIGHT_ALIGN 16
#define META_STRIDE_BYTE_ALIGN 256
#define DISABLE_POWER_COLLAPSE 0x1

#define CPP_TNR_SCRATCH_BUFF_COUNT         2

#define NUM_GRAY_PATCHES (MAX_LIGHT_TYPES_FOR_SPATIAL)

#ifndef MAX_LIGHT_TYPES_FOR_SPATIAL_EXT_POINTS
#define MAX_LIGHT_TYPES_FOR_SPATIAL_EXT_POINTS (MAX_LIGHT_TYPES_FOR_SPATIAL)
#endif

#define ASF_7x7_NUM_GRAY_PATCHES (MAX_LIGHT_TYPES_FOR_SPATIAL)
#define WNR_SW_NUM_GRAY_PATCHES (MAX_LIGHT_TYPES_FOR_SPATIAL)

#if defined (CHROMATIX_310E) || defined (CHROMATIX_308E)
#define ASF_9x9_NUM_GRAY_PATCHES (MAX_LIGHT_TYPES_FOR_SPATIAL_EXT_POINTS)
#define WNR_HW_NUM_GRAY_PATCHES (MAX_LIGHT_TYPES_FOR_SPATIAL_EXT_POINTS)
#else
#define ASF_9x9_NUM_GRAY_PATCHES (MAX_LIGHT_TYPES_FOR_SPATIAL)
#define WNR_HW_NUM_GRAY_PATCHES (MAX_LIGHT_TYPES_FOR_SPATIAL)
#endif

#define CPP_PARAM_ASF_7x7_TRIGGER_MAX (ASF_7x7_NUM_GRAY_PATCHES + MAX_LDS_REGIONS)
#define CPP_PARAM_ASF_9x9_TRIGGER_MAX (ASF_9x9_NUM_GRAY_PATCHES + MAX_LDS_REGIONS)

#define CPP_INTERPOLATE_ALL       0x0F
#define CPP_INTERPOLATE_ASF       0x01
#define CPP_INTERPOLATE_WNR       0x02
#define CPP_INTERPOLATE_TNR       0x04
#define CPP_INTERPOLATE_PBF       0x08

/* Hardware capability bit fields */
#define WNR_CAPS 0x1
#define WNR_CROP_CAPS 0x2
#define SCALE_CAPS 0x4
#define ASF_CAPS 0x8
#define ASF_CROP_CAPS 0x10
#define ROTATOR_CAPS 0x20
#define TNR_CAPS 0x40
#define TNR_CROP_CAPS 0x80
#define PIPELINE_CAPS 0x300
#define DYN_IMG_CDS_HYS 0x20
#define DYN_IMG_TNR_HYS 0x40
#define DYN_IMG_PBF_HYS 0x80
#define DYN_IMG_ASF_RNR_HYS 0x100
#define DYN_IMG_CDS_HYS_BIT 5
#define DYN_IMG_TNR_HYS_BIT 6
#define DYN_IMG_PBF_HYS_BIT 7
#define DYN_IMG_ASF_RNR_HYS_BIT 8

/* CPP UP-SCALE LIMIT */
/* Use (1 /CPP_UPSCALE_LIMIT) */
#define CPP_UPSCALE_LIMIT 32.0f
/*
 * 16.0f is CPP Downscale limit in normal pipeline usecases.
 * This drops to 6.0f in case of TNR.
 */
#define CPP_DOWNSCALE_LIMIT 16.0f
#define CPP_DOWNSCALE_LIMIT_UBWC 6.0f
#define CPP_DOWNSCALE_LIMIT_TNR 6.0f
#define CPP_DOWNSCALE_LIMIT_TNR_UBWC 3.4f
#define CPP_TNR_DS_RES_LIMIT (320*240)

typedef enum {
  CPP_PARAM_PLANE_CBCR,
  CPP_PARAM_PLANE_CRCB,
  CPP_PARAM_PLANE_CBCR_UBWC,
  CPP_PARAM_PLANE_CRCB_UBWC,
  CPP_PARAM_PLANE_Y,
  CPP_PARAM_PLANE_CB,
  CPP_PARAM_PLANE_CR,
  CPP_PARAM_PLANE_CBCR422,
  CPP_PARAM_PLANE_CRCB422,
  CPP_PARAM_PLANE_CRCB420,
  CPP_PARAM_PLANE_CBCR444,
  CPP_PARAM_PLANE_CRCB444
} cpp_params_plane_fmt_t;

typedef struct _cpp_params_plane_info_t {
  int32_t plane_offsets;
  int32_t plane_offset_x;
  int32_t plane_offset_y;
  int32_t plane_len;
  /* UBWC parameters for planes */
  uint32_t                meta_stride;
  uint32_t                meta_scanline;
  uint32_t                meta_len;

  /* RNR related */
  uint32_t                horizontal_center;
  uint32_t                vertical_center;
} cpp_params_plane_info_t;

typedef struct _cpp_params_dim_info_t {
  uint32_t                width;
  uint32_t                height;
  uint32_t                stride;
  uint32_t                scanline;
  uint32_t                frame_len;
  cpp_params_plane_fmt_t  plane_fmt;
  cpp_params_plane_info_t plane_info[3];
} cpp_params_dim_info_t;

/* List of supported firmware versions:
  31:28  Major version
  27:16  Minor version
  15:0   Step version */
typedef enum _cpp_firmware_version_t {
  CPP_FW_VERSION_1_1_6 = 0x10010006,
  CPP_FW_VERSION_1_2_0 = 0x10020000,
  CPP_FW_VERSION_1_4_0 = 0x10040000,
  CPP_FW_VERSION_1_5_0 = 0x10050000,
  CPP_FW_VERSION_1_5_1 = 0x10050001,
  CPP_FW_VERSION_1_5_2 = 0x10050002,
  CPP_FW_VERSION_1_6_0 = 0x10060000,
  CPP_FW_VERSION_1_8_0 = 0x10080000,
  CPP_FW_VERSION_1_10_0 = 0x100a0000,
  CPP_FW_VERSION_1_12_0 = 0x100C0000,
  CPP_FW_VERSION_1_12_1 = 0x100C0001,
  CPP_FW_VERSION_MAX = 0xffffffff,
} cpp_firmware_version_t;

/* forward declaration of cpp_hardware_t structures */
typedef struct _cpp_hardware_t cpp_hardware_t;

#define LTM_SIZE 129
#define GTM_SIZE 66
#define GAMMA_SIZE 512
/* Input  curve params */
typedef struct _isp_curves {
  /* check size */
  uint8_t ltm_enable;
  uint32_t ltm_size;
  float ltm_master_curve[MAX_ENTRIES_LTM];
  float ltm_master_scale[MAX_ENTRIES_LTM];
  float ltm_orig_master_scale[MAX_ENTRIES_LTM];
  /* check size */
  uint8_t gamma_enable;
  uint32_t gamma_size;
  float gamma_G[MAX_ENTRIES_GAMMA];
  /* check size */
  uint8_t gtm_enable;
  uint32_t gtm_size;
  int32_t gtm_yratio_base[MAX_ENTRIES_GTM];
  int32_t gtm_yratio_slope[MAX_ENTRIES_GTM];
  uint32_t hdr_msb_mode;
} isp_curves;

typedef enum {
  ROT_0,
  ROT_0_H_FLIP,
  ROT_0_V_FLIP,
  ROT_0_HV_FLIP,
  ROT_90,
  ROT_90_H_FLIP,
  ROT_90_V_FLIP,
  ROT_90_HV_FLIP,
} cpp_rot_cfg;

typedef enum {
  PLANE_CBCR,
  PLANE_CRCB,
  PLANE_Y,
  PLANE_CB,
  PLANE_CR,
} cpp_plane_fmt;

typedef enum {
  CPP_PARAM_ASF_OFF,
  CPP_PARAM_ASF_DUAL_FILTER,
  CPP_PARAM_ASF_EMBOSS,
  CPP_PARAM_ASF_SKETCH,
  CPP_PARAM_ASF_NEON,
  CPP_PARAM_ASF_BEAUTYSHOT,
} cpp_params_asf_mode_t;

/*Assumption: Assume that the left top point of source image matches
            the left top point of destination image before rotation

dst_with: destination image width before rotation
dst_height: destination image height before rotation
fetch_address: address of top left point of source image
destination_address: address of top left point of destination "after" rotation

phase_x_cur:  phase of the topleft corner of current fetch block
phase_x_next: phase of the topleft corner of fetch block right
              next to the current fetch block
phase_y_cur:  phase of the topleft corner of current fetch block
phase_y_next: phase of the topleft corner of fetch block right
              below the current fetch block
INIT_PHASE_X: in_ary->horizontal_scale_initial_phase for leftmost stripe,
              in_ary->horizontal_scale_block_initial_phase for others
INIT_PHASE_Y: in_ary->vertical_scale_initial_phase for topmost stripe,
              in_ary->vertical_scale_block_initial_phase for others
PHASE_X_STEP: in_ary->horizontal_scale_ratio
PHASE_Y_STEP: in_ary->vertical_scale_ratio
DST_TILE_SIZE_X: destination stripe width
DST_TILE_SIZE_Y: destination stripe height
X_tile_index: horizontal index starting from 0
Y_tile_index: vertical index starting from 0
*/

struct cpp_accumulated_phase_t {
long long phase_x_cur;
long long phase_x_next;
long long phase_y_cur;
long long phase_y_next;
long long INIT_PHASE_X;
long long INIT_PHASE_Y;
long long PHASE_X_STEP;
long long PHASE_Y_STEP;
uint32_t DST_TILE_SIZE_X;
uint32_t DST_TILE_SIZE_Y;
uint32_t X_tile_index;
uint32_t Y_tile_index;
};

typedef struct {
  int fd;
  uint32_t index;
  uint32_t offset;
  uint8_t native_buff;
  uint8_t processed_divert;
  uint32_t  identity;
  void *vaddr;
  size_t alloc_len;
  uint32_t buffer_access;
} cpp_hardware_buffer_info_t;

typedef struct _cpp_tnr_params_scale_info_t {
  float denoise_profile_Y_adj[TNR_SCALE_LAYERS];
  float denoise_profile_chroma_adj[TNR_SCALE_LAYERS];
  float denoise_weight_y_VFE_adj[TNR_SCALE_LAYERS];
  float denoise_weight_chroma_VFE_adj[TNR_SCALE_LAYERS];
} cpp_tnr_params_scale_info_t;

typedef struct asf_1_12_rnr_output_t {
  /* radial sqruare LUT*/
  uint32_t rnr_r_sqr_lut[CPP_ASF_RNR_LUT_ENTRIES_1_12_x];
  uint32_t rnr_r_sqr_shift;
  /*radial activity level*/
  uint32_t rnr_activity_cf_lut[CPP_ASF_RNR_LUT_ENTRIES_1_12_x];
  uint32_t rnr_activity_slope_lut[CPP_ASF_RNR_LUT_ENTRIES_1_12_x];
  uint32_t rnr_activity_shift_lut[CPP_ASF_RNR_LUT_ENTRIES_1_12_x];
  /*radial gain level*/
  uint32_t rnr_gain_cf_lut[CPP_ASF_RNR_LUT_ENTRIES_1_12_x];
  uint32_t rnr_gain_slope_lut[CPP_ASF_RNR_LUT_ENTRIES_1_12_x];
  uint32_t rnr_gain_shift_lut[CPP_ASF_RNR_LUT_ENTRIES_1_12_x];
} asf_1_12_rnr_output_t;

struct asf_1_12_rnr_input_t {
  float rnr_gain_tbl_in[ASF_RNR_IN_ENTRIES];
  float rnr_activity_tbl_in[ASF_RNR_IN_ENTRIES];
  float radial_point[ASF_RNR_IN_ENTRIES];
  uint32_t h_center;
  uint32_t v_center;
  float isp_scale_ratio;
  float cpp_scale_ratio;
};

struct cpp_1_12_asf_ext_t {
  bool is_valid;
  uint8_t gain_neg_lut[CPP_ASF_LUT_ENTRIES_1_12_x]; /* LUT 5*/
  struct asf_1_12_rnr_output_t asf_1_12_rnr_output;
  struct asf_1_12_rnr_input_t asf_1_12_rnr_input;
};

typedef struct _cpp_asf_info_1_6_x {
  /* ASF_CFG_0 */
  uint8_t    sp;
  uint8_t    neg_abs_y1;
  uint8_t    dyna_clamp_en;
  uint8_t    sp_eff_en;
  uint8_t    sp_eff_abs_en;
  uint8_t    nz_flag_sel;
  uint8_t    L2_norm_en;
  uint8_t    symm_filter_only;
  uint8_t    radial_enable;
  /* ASF_CFG_1 */
  uint8_t    activity_clamp_threshold;
  uint8_t    perpen_scale_factor;
  /* ASF_CFG_2 */
  uint8_t    gamma_cor_luma_target;
  uint16_t   max_val_threshold;
  /* ASF_CFG_3 */
  uint8_t    gain_cap;
  uint8_t    median_blend_offset;
  uint8_t    median_blend_lower_offset;
  /* ASF_CFG_4 */
  uint8_t   norm_scale;
  /* Clamp Limits */
  int16_t   clamp_h_ul;
  int16_t   clamp_h_ll;
  int16_t   clamp_v_ul;
  int16_t   clamp_v_ll;
  /* Clamp Scale */
  uint16_t   clamp_scale_max;
  uint16_t   clamp_scale_min;
  /* Clamp Offset */
  uint8_t    clamp_offset_max;
  uint8_t    clamp_offset_min;
  /* Clamp TL Limit */
  int16_t   clamp_tl_ll;
  int16_t   clamp_tl_ul;
  /* NZ Flags */
  uint32_t   nz_flag;
  uint32_t   nz_flag_2;
  uint32_t   nz_flag_3_5;
  /* Sobel Coeffs */
  int16_t   sobel_h_coeff[26]; /* F1 */
  int16_t   sobel_v_coeff[26]; /* F2 */
  int16_t   sobel_se_coeff[26]; /* F3 */
  /* HPF Coeffs */
  int16_t   hpf_h_coeff[26]; /* F5 */
  int16_t   hpf_v_coeff[26]; /* F6 */
  int16_t   hpf_se_coeff[26]; /* F7 */
  /* LPF Coeffs */
  int16_t   lpf_coeff[26]; /* F9 */
  /*
   * Symmetric HPF Coeffs. 0-14 is used.
   * One more index added to keep it inline with
   * frame message code used across targets.
   */
  int16_t   hpf_sym_coeff[16]; /* F9 */
  /* Activity LPF Coeffs */
  int8_t    activity_lpf_coeff[6];
  /* Activity BPF Coeffs */
  int16_t   activity_bpf_coeff[6];
  /* 256 8-bit entries for 5 LUTs
      LUT0 : activity norm LUT
      LUT1 : gain weight LUT
      LUT2 : gain LUT
      LUT3 : soft th LUT
      LUT4 : soft th weight LUT
  */
  uint8_t   lut[5][256];
  struct cpp_1_12_asf_ext_t cpp_1_12_ext;
} cpp_1_6_x_asf_info_t;

union cpp_asf_info_u {
    cpp_1_6_x_asf_info_t  info_1_6_x;
};

/* TODO: To be removed */
struct cpp_asf_info {
  float sp;
  uint8_t neg_abs_y1;
  uint8_t dyna_clamp_en;
  uint8_t sp_eff_en;
  int16_t clamp_h_ul;
  int16_t clamp_h_ll;
  int16_t clamp_v_ul;
  int16_t clamp_v_ll;
  float clamp_scale_max;
  float clamp_scale_min;
  uint16_t clamp_offset_max;
  uint16_t clamp_offset_min;
  uint32_t nz_flag;
  uint32_t nz_flag_f2;
  uint32_t nz_flag_f3_f5;
  uint32_t checksum_en;
  float sobel_h_coeff[25];
  float sobel_v_coeff[25];
  float hpf_h_coeff[25];
  float hpf_v_coeff[25];
  float lpf_coeff[25];
  float lut1[CPP_ASF_LUT12_ENTRIES_1_2_x];
  float lut2[CPP_ASF_LUT12_ENTRIES_1_2_x];
  float lut3[CPP_ASF_LUT3_ENTRIES_1_2_x];
  /* Clamp TL Limit */
  int16_t clamp_tl_ll;
  int16_t clamp_tl_ul;
};

struct cpp_bf_info {
  double bilateral_scale[BILITERAL_LAYERS];
  double noise_threshold[CPP_DENOISE_NUM_PROFILES];
  double weight[CPP_DENOISE_NUM_PROFILES];
};

typedef struct _cpp_tnr_info_t {
  double bilateral_scale[4];
  double noise_threshold[5];
  double weight[5];
} cpp_tnr_info_t;

typedef struct _cpp_prescaler_bf_info_t {
  double bilateral_scale;
  double noise_threshold;
  double weight;
} cpp_prescaler_bf_info_t;

struct cpp_plane_scale_info {
  uint8_t v_scale_fir_algo;
  uint8_t h_scale_fir_algo;
  uint8_t v_scale_algo;
  uint8_t h_scale_algo;
  uint8_t subsample_en;
  uint8_t upsample_en;
  uint8_t vscale_en;
  uint8_t hscale_en;

  uint32_t phase_h_step;
  uint32_t phase_v_init;
  uint32_t phase_v_step;
};

struct cpp_stripe_scale_info {
  uint16_t block_width;
  uint16_t block_height;
  uint32_t phase_h_init;
};

struct cpp_rotator_info {
  cpp_rot_cfg rot_cfg;
  uint16_t block_width;
  uint16_t block_height;
  uint32_t block_size;
  uint16_t rowIndex0;
  uint16_t rowIndex1;
  uint16_t colIndex0;
  uint16_t colIndex1;
  uint16_t initIndex;
  uint16_t modValue;
  uint16_t tile_width;
  uint16_t tile_height;
  uint16_t tile_size;
  uint16_t tile_voffset;
  uint16_t blocks_per_stripe;
};

struct cpp_crop_info {
  uint32_t enable;
  uint16_t first_pixel;
  uint16_t last_pixel;
  uint16_t first_line;
  uint16_t last_line;
};

struct cpp_fe_info {
  uint32_t buffer_ptr;
  uint32_t buffer_width;
  uint32_t buffer_height;
  uint32_t buffer_stride;
  uint16_t block_width;
  uint16_t block_height;
  uint8_t left_pad;
  uint8_t right_pad;
  uint8_t top_pad;
  uint8_t bottom_pad;
};

struct cpp_we_info {
  uint32_t buffer_ptr[4];
  uint32_t buffer_width;
  uint32_t buffer_height;
  uint32_t buffer_stride;
  uint16_t blocks_per_col;
  uint16_t blocks_per_row;
  int32_t h_step;
  int32_t v_step;
  int32_t h_init;
  int32_t v_init;
  //ubwc parameters
  uint16_t ubwc_tile_format;
  uint16_t ubwc_out_type;
  uint16_t ubwc_en;
  uint16_t tile_height;
  uint16_t tile_width;
  uint16_t ubwc_bank_spread_en;
  uint16_t highest_bank_en;
  uint16_t tiles_per_block_row;
  uint16_t tiles_per_block_col;
  uint16_t highest_bank_bit;
  uint16_t rot_mode;
};

/* CPP Cds data structure for fetch and write */
typedef struct _cpp_dsdn_info_t {
  int32_t vertical_cnt32;
  int32_t vertical_cnt64;
  int32_t horizontal_cnt32;
  int32_t horizontal_cnt64;
  int32_t pre_upscale_top_crop;
  int32_t pre_upscale_left_crop;
  int32_t upscale_out_width;
  int32_t upscale_out_height;
}cpp_dsdn_info;

/* mmu prefetch data structure for fetch and write */
typedef struct _cpp_plane_mmu_prefetch_info_t {
  int32_t l1_mmu_pf_en;
  int32_t l1_period;
  int32_t l1_direction;
  int32_t l2_mmu_pf_en;
  int32_t l2_range;
  int32_t l2_distance;
  uint32_t addr_min;
  uint32_t addr_max;
}cpp_plane_mmu_prefetch_info;

struct cpp_stripe_info {
  uint32_t dst_addr;
  uint32_t image_stride;
  uint32_t stripe_index;
  uint32_t stripe_width;
  uint32_t stripe_height;
  uint8_t bytes_per_pixel;
  uint8_t rotation;
  uint8_t mirror;
  struct cpp_fe_info fe_info;
  struct cpp_fe_info fe_r_info; /* TNR scratch buffer */
  struct cpp_crop_info bf_crop_info;
  struct cpp_crop_info prescaler_bf_crop_info;
  struct cpp_crop_info temporal_bf_crop_info;
  struct cpp_stripe_scale_info scale_info;
  struct cpp_crop_info asf_crop_info;
  struct cpp_crop_info next_state_bf_crop_info;
  struct cpp_rotator_info rot_info;
  struct cpp_we_info we_info;
  struct cpp_we_info we_r_info; /* TNR scratch buffer */
  cpp_dsdn_info dsdn_info;
};

struct cpp_plane_info_t {
  /*input*/
  int src_fd;
  int dst_fd;
  uint32_t src_width;
  uint32_t src_height;
  uint32_t src_stride;
  uint32_t dst_width;
  uint32_t dst_height;
  uint32_t dst_stride;
  uint32_t dst_x_offset;
  uint32_t dst_y_offset;
  uint32_t temporal_stride;
  uint32_t rotate;
  uint32_t mirror;
  int32_t prescale_padding;
  int32_t postscale_padding;
  uint32_t state_padding;
  uint32_t spatial_denoise_padding;
  uint32_t prescaler_spatial_denoise_padding;
  uint32_t temporal_denoise_padding;
  uint32_t sharpen_padding;
  double h_scale_ratio;
  double v_scale_ratio;
  int64_t horizontal_scale_ratio;
  int64_t vertical_scale_ratio;
  double h_scale_initial_phase;
  double v_scale_initial_phase;
  long long horizontal_scale_initial_phase;
  long long vertical_scale_initial_phase;
  uint32_t maximum_dst_stripe_height;
  uint32_t maximum_src_stripe_height;
  cpp_plane_fmt input_plane_fmt;
  cpp_plane_fmt output_plane_fmt;
  /*  the following two are needed to support color conversion
   *  from CbCr to Cb, Cr or vise versa.
   *  they have the same value as in cpp_striping_algorithm_info
   */
  uint32_t input_bytes_per_pixel;
  uint32_t output_bytes_per_pixel;
  uint32_t temporal_bytes_per_pixel;
  /*  address[2] only needed for color conversion
   * from CbCr to Cb, Cr or vise versa
   */
  uint32_t source_address[2];
  uint32_t destination_address[2];
  uint32_t temporal_source_address[2];
  uint32_t temporal_destination_address[2];

  /*derived*/
  uint32_t is_not_y_plane;
  uint32_t tnr_enable;  /* 1 -> TNR is on, 0 -> TNR is off */
  uint32_t pbf_enable;  /* 1 -> PBF is on, 0 -> PBF is off */
  uint32_t denoise_after_scale_en;
  uint32_t num_stripes;
  uint32_t frame_width_mcus; /*The number of stripes*/
  uint32_t frame_height_mcus; /*The number of blocks in the vertical direction*/
  /*dst_height rounded up to the rotation_block_height*/
  uint32_t dst_height_block_aligned;
  /*common stripe width
    a stripe that is on the left or right boundary can have a smaller width*/
  uint32_t dst_block_width;
  /*common stripe height,
    a block that is on the top or bottom can have a smaller height*/
  uint32_t dst_block_height;
  uint32_t *stripe_block_width;/*actual block width in the horizontal direction*/
  uint32_t *stripe_block_height;/*actual block height in the vertical direction*/
  /*The initial phase of the topleft corner fetch block
    assuming each dst block has the same width dst_block_width*/
  long long horizontal_scale_block_initial_phase;
  /*The initial phase of the topleft corner fetch block
    assuming each dst block has the same height dst_block_height*/
  long long vertical_scale_block_initial_phase;

  uint32_t prescaler_crop_enable;
  uint32_t tnr_crop_enable;
  uint32_t bf_crop_enable;
  uint32_t asf_crop_enable;
  uint32_t next_state_crop_enable;
  uint32_t bf_enable;
  uint32_t dsdn_enable;
  struct cpp_plane_scale_info scale_info;
  struct cpp_stripe_info *stripe_info1;
  struct cpp_striping_algorithm_info *stripe_info;

  /* UBWC parameters */
  uint32_t tile_output_enable;
  uint32_t tile_byte_width;
  uint32_t tile_byte_height;
  uint32_t dst_width_block_aligned;
  uint32_t rotation_buffer_size;
  uint32_t mal_byte_size;
  uint32_t line_buffer_size;
  uint32_t prescaler_padding;
  uint32_t ubwc_hbb;


  uint32_t metadata_stride; //stride for UBWC metadata
  uint32_t metadata_offset; // offset for metadata
  uint32_t framedata_offset; //offset for framedata

  uint32_t pad_boundary; // Overfetch or pad using the fetch engine instead of internal HW padding
  // 1: more bandwidth, recommended for chroma
  // 0: less bandwidth, recommended for luma

  uint32_t horizontal_center;
  uint32_t vertical_center;
  uint32_t sharpen_before_scale;

  cpp_params_plane_info_t input_plane_config;
  cpp_params_plane_info_t output_plane_config;
  cpp_plane_mmu_prefetch_info mmu_prefetch_fe_info;
  cpp_plane_mmu_prefetch_info mmu_prefetch_ref_fe_info;
  cpp_plane_mmu_prefetch_info mmu_prefetch_we_info;
  cpp_plane_mmu_prefetch_info mmu_prefetch_dup_we_info;
  cpp_plane_mmu_prefetch_info mmu_prefetch_ref_we_info;
};

typedef struct {
    boolean batch_mode;
    uint32_t batch_size;
    uint32_t intra_plane_offset[MAX_PLANES];
    uint32_t pick_preview_idx;
} cpp_hardware_batch_info_t;

// Fwd declaration
struct _cpp_hardware_params_t;

struct cpp_frame_info_t {
  int32_t frame_id;
  struct timeval timestamp;
  int32_t processed_divert;
  uint32_t identity;
  boolean dup_output;
  uint32_t buff_index;
  uint32_t native_buff;
  uint32_t in_buff_identity;
  uint32_t in_buff_buffer_access;
  void    *cookie;
  cpp_hardware_buffer_info_t out_buff_info;
  cpp_hardware_buffer_info_t dup_buff_info;
  cpp_hardware_buffer_info_t tnr_input_scratch_buff_info;
  cpp_hardware_buffer_info_t tnr_output_scratch_buff_info;
  enum msm_cpp_frame_type frame_type;
  cpp_params_asf_mode_t asf_mode;
  struct cpp_asf_info asf_info; /* TODO: remove */
  union cpp_asf_info_u u_asf;
  struct cpp_bf_info bf_info[CPP_DENOISE_NUM_PLANES];
  cpp_tnr_info_t     tnr_info[CPP_DENOISE_NUM_PLANES];
  cpp_prescaler_bf_info_t prescaler_info[CPP_DENOISE_NUM_PLANES];
  /*
   * Value does not go beyond 255 currently
   * Revisit if value goes beyond 32 bit integer
   */
  int32_t clamp_limit_UL[CPP_DENOISE_NUM_PLANES];
  int32_t clamp_limit_LL[CPP_DENOISE_NUM_PLANES];
  struct cpp_plane_info_t plane_info[MAX_PLANES];
  uint32_t num_planes;
  cpp_plane_fmt out_plane_fmt;
  cpp_plane_fmt in_plane_fmt;
  double noise_profile[CPP_DENOISE_NUM_PLANES][CPP_DENOISE_NUM_PROFILES];
  double weight[CPP_DENOISE_NUM_PLANES][CPP_DENOISE_NUM_PROFILES];
  double denoise_ratio[CPP_DENOISE_NUM_PLANES][CPP_DENOISE_NUM_PROFILES];
  double edge_softness[CPP_DENOISE_NUM_PLANES][BILITERAL_LAYERS];
  float profile_adj[CPP_DENOISE_NUM_PLANES][CPP_DENOISE_NUM_PROFILES];
  float weight_VFE_adj[CPP_DENOISE_NUM_PLANES][CPP_DENOISE_NUM_PROFILES];
  double pbf_noise_profile[CPP_DENOISE_NUM_PLANES];
  double pbf_weight[CPP_DENOISE_NUM_PLANES];
  double pbf_denoise_ratio[CPP_DENOISE_NUM_PLANES];
  double pbf_edge_softness[CPP_DENOISE_NUM_PLANES];
  float pbf_profile_adj[CPP_DENOISE_NUM_PLANES];
  float pbf_weight_VFE_adj[CPP_DENOISE_NUM_PLANES];

  double sharpness_ratio;
  boolean we_disable;
  uint8_t power_collapse;
  cpp_hardware_batch_info_t  batch_info;
  struct _cpp_hardware_params_t *p_hw_params;
};

/* ---------------------------- new params --------------------------------- */

typedef struct _cpp_params_scale_info_t {
  double h_scale_ratio;
  double v_scale_ratio;
} cpp_params_scale_info_t;

typedef struct _cpp_params_denoise_info_t {
  double noise_profile;
  double weight;
  double denoise_ratio;
  double edge_softness;
  float profile_adj[2][CPP_DENOISE_NUM_PROFILES];
  float weight_VFE_adj[2][CPP_DENOISE_NUM_PROFILES];

} cpp_params_denoise_info_t;

typedef struct _cpp_params_pbf_denoise_info_t {
  double noise_profile;
  double weight;
  double denoise_ratio;
  double edge_softness;
  float profile_adj[2];
  float weight_VFE_adj[2];

} cpp_params_pbf_denoise_info_t;

typedef struct _cpp_params_crop_window_t {
  uint32_t x, y, dx, dy;
} cpp_params_crop_window_t;

typedef struct _cpp_params_crop_info_t {
  cpp_params_crop_window_t stream_crop;
  cpp_params_crop_window_t is_crop;
  float fovc_crop_factor;
  cam_frame_margins_t sac_crop_margins;
  cam_crop_data_t     dualcam_crop_data;
  uint32_t process_window_first_pixel;
  uint32_t process_window_first_line;
  uint32_t process_window_width;
  uint32_t process_window_height;
} cpp_params_crop_info_t;

typedef struct {
  uint32_t identity;
  uint32_t num_buffs;
  cpp_hardware_buffer_info_t *buffer_info;
} cpp_hardware_stream_buff_info_t;

#define CPP_ASF_CONTROL_LUX_BASED   0
#define CPP_ASF_CONTROL_GAIN_BASED  1

#if defined (CHROMATIX_304) || defined (CHROMATIX_306) || \
  defined (CHROMATIX_307) || defined (CHROMATIX_308) || defined (CHROMATIX_308E) || \
  defined (CHROMATIX_309) || defined (CHROMATIX_310) || defined (CHROMATIX_310E)

typedef enum _cpp_params_asf_region1 {
  CPP_PARAM_ASF_REGION_1,
  CPP_PARAM_ASF_REGION_12_INTERPOLATE,
  CPP_PARAM_ASF_REGION_2,
  CPP_PARAM_ASF_REGION_23_INTERPOLATE,
  CPP_PARAM_ASF_REGION_3,
  CPP_PARAM_ASF_REGION_34_INTERPOLATE,
  CPP_PARAM_ASF_REGION_4,
  CPP_PARAM_ASF_REGION_45_INTERPOLATE,
  CPP_PARAM_ASF_REGION_5,
  CPP_PARAM_ASF_REGION_56_INTERPOLATE,
  CPP_PARAM_ASF_REGION_6,
#ifdef LDS_ENABLE
  CPP_PARAM_ASF_REGION_7,
  CPP_PARAM_ASF_REGION_78_INTERPOLATE,
  CPP_PARAM_ASF_REGION_8,
#endif
  CPP_PARAM_ASF_REGION_MAX
} cpp_params_asf_region_t;
#else
typedef enum _cpp_params_asf_region {
  CPP_PARAM_ASF_LOW_LIGHT,
  CPP_PARAM_ASF_LOW_LIGHT_INTERPOLATE,
  CPP_PARAM_ASF_NORMAL_LIGHT,
  CPP_PARAM_ASF_BRIGHT_LIGHT_INTERPOLATE,
  CPP_PARAM_ASF_BRIGHT_LIGHT,
  CPP_PARAM_ASF_MAX_LIGHT
} cpp_params_asf_region_t;
#endif

typedef struct _cpp_params_asf_info_t {

  /* TODO: param for fw 1_2_x */
  float sp;
  uint8_t neg_abs_y1;
  uint8_t dyna_clamp_en;
  uint8_t sp_eff_en;
  int16_t clamp_h_ul;
  int16_t clamp_h_ll;
  int16_t clamp_v_ul;
  int16_t clamp_v_ll;
  float clamp_scale_max;
  float clamp_scale_min;
  uint16_t clamp_offset_max;
  uint16_t clamp_offset_min;
  uint32_t nz_flag;
  float sobel_h_coeff[25];
  float sobel_v_coeff[25];
  float hpf_h_coeff[25];
  float hpf_v_coeff[25];
  float lpf_coeff[25];
  float lut1[CPP_ASF_LUT12_ENTRIES_1_2_x];
  float lut2[CPP_ASF_LUT12_ENTRIES_1_2_x];
  float lut3[CPP_ASF_LUT3_ENTRIES_1_2_x];

  /* asf params for fw 1_6_x */
  cpp_1_6_x_asf_info_t info_1_6_x;
  float downscale_ratio;
  uint32_t nz_flag_f2;
  uint32_t nz_flag_f3_f5;
  float sharp_min_ds_factor;
  float sharp_max_ds_factor;
  float sharp_max_factor;
  cpp_params_asf_region_t region;
  uint32_t asf_reg1_idx;
  uint32_t asf_reg2_idx;
  uint32_t checksum_enable;
  float sharpness_level;
  /* Clamp TL Limit */
  int16_t clamp_tl_ll;
  int16_t clamp_tl_ul;
  cpp_firmware_version_t cpp_fw_version;

} cpp_params_asf_info_t;

#if defined (CHROMATIX_304) || defined (CHROMATIX_306) || \
  defined (CHROMATIX_307) || defined (CHROMATIX_308) || defined (CHROMATIX_308E) || \
  defined (CHROMATIX_309) || defined (CHROMATIX_310) || defined (CHROMATIX_310E)

typedef struct _cpp_params_aec_trigger_params1 {
  float trigger_start[CPP_PARAM_ASF_7x7_TRIGGER_MAX];
  float trigger_end[CPP_PARAM_ASF_7x7_TRIGGER_MAX];
  float aec_trigger_input;
} cpp_params_aec_trigger_t;
#else
typedef struct _cpp_params_aec_trigger_params {
  float lowlight_trigger_start;
  float lowlight_trigger_end;
  float brightlight_trigger_start;
  float brightlight_trigger_end;
  float aec_trigger_input;
} cpp_params_aec_trigger_t;
#endif

typedef struct _cpp_params_aec_trigger_info_t {
  float lux_idx;
  float gain;
  float aec_sensitivity_ratio;
  float exp_time_ratio;
  float total_adrc_gain;
  float drc_color_gain;
  boolean low_light_capture_update_flag;
} cpp_params_aec_trigger_info_t;

typedef struct _cpp_module_native_buff_array {
  cpp_hardware_buffer_info_t buff_array[CPP_TNR_SCRATCH_BUFF_COUNT];
} cpp_hardware_native_buff_array;

typedef struct _cpp_hardware_params_t {
  /* TODO: Remove frameid, identity and timestamp */
  uint32_t                  frame_id;
  uint32_t                  identity;
  struct timeval            timestamp;
  int32_t                   retry_disable;
  boolean                   duplicate_output;
  uint32_t                  duplicate_identity;
  boolean                   processed_divert;
  double                    sharpness_level;
  uint8_t                   edge_mode;
  cpp_params_asf_mode_t     asf_mode;
  cpp_params_asf_info_t     asf_info; // to be removed
  union cpp_asf_info_u      u_asf;
  cpp_params_scale_info_t   scale_info;
  uint32_t                  rotation;
  uint32_t                  mirror;
  boolean                   scale_enable;
  boolean                   crop_enable;
  boolean                   fovc_enable;
  boolean                   sac_enable;
  cpp_params_crop_info_t    crop_info; /* for Dual Cam: SAT/SAC margins & crop */
  cpp_params_dim_info_t     input_info;
  cpp_params_dim_info_t     output_info;
  cam_sac_output_info_t     dualcam_shift_offsets;
  float                     zoom_factor;
  boolean                   denoise_enable;
  boolean                   dsdn_enable;
  boolean                   tnr_enable;
  boolean                   pbf_enable;
  boolean                   expect_divert;
  boolean                   uv_upsample_enable;
  boolean                   diagnostic_enable;
  boolean                   denoise_lock;
  boolean                   tnr_denoise_lock;
  boolean                   asf_lock;
  boolean                   ez_tune_wnr_enable;
  boolean                   ez_tune_asf_enable;
  boolean                   scene_mode_on;
  boolean                   color_xform_is_on;
  cpp_tnr_info_t            tnr_info_Y;
  cpp_tnr_info_t            tnr_info_Cb;
  cpp_tnr_info_t            tnr_info_Cr;
  cpp_params_pbf_denoise_info_t  pbf_denoise_info[CPP_DENOISE_NUM_PLANES];
  void                     *cookie;
  cpp_hardware_buffer_info_t buffer_info;
  sensor_dim_output_t       sensor_dim_info;
  cpp_params_aec_trigger_info_t aec_trigger;
  /* Current diagnostics */
  asfsharpness7x7_t          asf_diag;
  asfsharpness9x9_t          asf9x9_diag;
  wavelet_t                  wnr_diag;
  cam_stream_type_t          stream_type;
  uint32_t                  isp_width_map;
  uint32_t                  isp_height_map;
  boolean                      grey_ref_enable;
  boolean                      we_disable;
  cpp_hardware_buffer_info_t output_buffer_info;
  cpp_hardware_buffer_info_t dup_buffer_info;
  cpp_hardware_native_buff_array tnr_scratch_bfr_array;
  cam_effect_mode_type       effect_mode;
  chromatix_cpp_stripped_type *def_chromatix;
  float                      asf_scalor_adj;
  float                      asf_hdr_adj;
  uint32_t                   face_count;
  float                      isp_scale_ratio;
  float                      cpp_scale_ratio;
  uint32_t                   lower_clamp_limit [3];
  uint32_t                   upper_clamp_limit [3];
  boolean                    downsample_mask;
  uint32_t                   interpolate_mask;
  boolean                    asf_mask;
  boolean                    denoise_mask;
  uint32_t                   tnr_mask;
  boolean                    dsdn_mask;
  bool                       hyst_dsdn_status;
  bool                       hyst_tnr_status;
  bool                       hyst_pbf_status;
  bool                       hyst_wnr_rnr_status;
  bool                       hyst_asf_rnr_status;
  /* lds enable on frame basis */
  boolean                    lds_enable;
  int32_t                    luma_dsdn;
  cpp_hardware_batch_info_t  batch_info;
  boolean                    lpm_enabled;
  void                       *vaddr;
  boolean                    low_light_capture_enable;
  uint32_t                   drop_count;
  cam_sensor_hdr_type_t      hdr_mode;

  /* WNR */
  cpp_params_denoise_info_t
    denoise_info[CPP_DENOISE_NUM_PLANES][CPP_DENOISE_NUM_PROFILES];
  cpp_wnr_frame_info_t wnr_frame_info;

  /* session params */
  cam_dimension_t            camif_dim;
  isp_curves                 curves;
  modulesChromatix_t         module_chromatix;
  float                      isp_scaler_ratio;
} cpp_hardware_params_t;

/* data structure with striping algorithm parameters */
struct cpp_striping_algorithm_info {
  uint32_t scale_v_en;
  uint32_t scale_h_en;

  uint32_t upscale_v_en;
  uint32_t upscale_h_en;

  int32_t src_start_x;
  uint32_t src_end_x;
  int32_t src_start_y;
  uint32_t src_end_y;

  /* extra 5th and 6th layer parameters */
  int32_t extra_src_start_x;
  uint32_t extra_src_end_x;
  int32_t extra_src_start_y;
  uint32_t extra_src_end_y;

  int32_t extra_initial_vertical_count[2];
  int32_t extra_initial_horizontal_count[2];

  /* crop downscale 32x pixels */
  int32_t extra_left_crop;
  int32_t extra_top_crop;

  int32_t extra_pad_bottom;
  int32_t extra_pad_top;
  int32_t extra_pad_right;
  int32_t extra_pad_left;

  int32_t extra_upscale_width;
  int32_t extra_upscale_height;

  uint32_t temporal_pad_bottom;
  uint32_t temporal_pad_top;
  uint32_t temporal_pad_right;
  uint32_t temporal_pad_left;

  int32_t temporal_src_start_x;
  uint32_t temporal_src_end_x;
  int32_t temporal_src_start_y;
  uint32_t temporal_src_end_y;

  /*
   * Padding is required for upscaler because it does not
   * pad internally like other blocks, also needed for rotation
   * rotation expects all the blocks in the stripe to be the same size
   * Padding is done such that all the extra padded pixels
   * are on the right and bottom
   */
  uint32_t pad_bottom;
  uint32_t pad_top;
  uint32_t pad_right;
  uint32_t pad_left;

  uint32_t v_init_phase;
  uint32_t h_init_phase;
  uint32_t h_phase_step;
  uint32_t v_phase_step;

  uint32_t spatial_denoise_crop_width_first_pixel;
  uint32_t spatial_denoise_crop_width_last_pixel;
  uint32_t spatial_denoise_crop_height_first_line;
  uint32_t spatial_denoise_crop_height_last_line;

  uint32_t sharpen_crop_height_first_line;
  uint32_t sharpen_crop_height_last_line;
  uint32_t sharpen_crop_width_first_pixel;
  uint32_t sharpen_crop_width_last_pixel;

  uint32_t temporal_denoise_crop_width_first_pixel;
  uint32_t temporal_denoise_crop_width_last_pixel;
  uint32_t temporal_denoise_crop_height_first_line;
  uint32_t temporal_denoise_crop_height_last_line;

  uint32_t prescaler_spatial_denoise_crop_width_first_pixel;
  uint32_t prescaler_spatial_denoise_crop_width_last_pixel;
  uint32_t prescaler_spatial_denoise_crop_height_first_line;
  uint32_t prescaler_spatial_denoise_crop_height_last_line;

  uint32_t state_crop_width_first_pixel;
  uint32_t state_crop_width_last_pixel;
  uint32_t state_crop_height_first_line;
  uint32_t state_crop_height_last_line;

  int32_t dst_start_x;
  uint32_t dst_end_x;
  int32_t dst_start_y;
  uint32_t dst_end_y;

  int32_t temporal_dst_start_x;
  uint32_t temporal_dst_end_x;
  int32_t temporal_dst_start_y;
  uint32_t temporal_dst_end_y;

  uint32_t input_bytes_per_pixel;
  uint32_t output_bytes_per_pixel;
  uint32_t temporal_bytes_per_pixel;

  uint32_t source_address[2];
  uint32_t extra_source_address[2];
  uint32_t destination_address[2];
  /*
   * source_address[1] is used for CbCR planar
   * to CbCr interleaved conversion
   */
  uint32_t temporal_source_address[2];
  /*
   * destination_address[1] is used for CbCr interleved
   * to CbCr planar conversion
   */

  uint32_t temporal_destination_address[2];
  uint32_t src_stride;
  uint32_t dst_stride;
  uint32_t temporal_stride;
  uint32_t rotate_270;
  uint32_t horizontal_flip;
  uint32_t vertical_flip;
  uint32_t scale_output_width;
  uint32_t scale_output_height;
  uint32_t spatial_denoise_crop_en;
  uint32_t sharpen_crop_en;
  uint32_t temporal_denoise_crop_en;
  uint32_t prescaler_spatial_denoise_crop_en;
  uint32_t state_crop_en;

  int32_t we_h_init;
  int32_t we_v_init;
  int32_t we_h_step;
  int32_t we_v_step;

  int32_t temporal_we_h_init;
  int32_t temporal_we_v_init;
  int32_t temporal_we_h_step;
  int32_t temporal_we_v_step;

  int32_t horizontal_center;
  int32_t vertical_center;
  int32_t asf_start_horizontal_offset;
  int32_t asf_start_vertical_offset;
  int32_t wnr_start_horizontal_offset;
  int32_t wnr_start_vertical_offset;
};

#define CPP_GET_FW_MAJOR_VERSION(ver) ((ver >> 28) & 0xf)
#define CPP_GET_FW_MINOR_VERSION(ver) ((ver >> 16) & 0xfff)
#define CPP_GET_FW_STEP_VERSION(ver)  (ver & 0xffff)

void increment_phase (struct cpp_accumulated_phase_t *in_phase,
  uint32_t current_block_width, uint32_t current_block_height);
void cpp_debug_fetch_engine_info(struct cpp_fe_info *fe_info);
void cpp_debug_write_engine_info(struct cpp_we_info *we_info);
void cpp_debug_crop_info(struct cpp_crop_info *crop_info);

void cpp_params_prepare_frame_info(cpp_hardware_t *cpphw,
  struct cpp_frame_info_t *frame_info, struct msm_cpp_frame_info_t *out_info);
int32_t cpp_params_create_frame_info(cpp_hardware_t *cpphw,
  cpp_hardware_params_t *hw_params, struct cpp_frame_info_t *frame_info);
boolean cpp_hardware_validate_params(cpp_hardware_params_t *hw_params);
boolean cpp_hardware_rotation_swap(cpp_hardware_params_t *hw_params,
  uint32_t video_type);
int32_t cpp_hw_params_update_wnr_params(modulesChromatix_t *module_chromatix,
  cpp_hardware_params_t *hw_params,
  cpp_params_aec_trigger_info_t *aec_trigger);
int32_t cpp_hw_params_noninterpolate_wnr_params(
  cpp_hardware_params_t *cpp_hw_params, float effects_factor,
  ReferenceNoiseProfile_type *ref_noise_profile);
int32_t cpp_hw_params_interpolate_wnr_params(float interpolation_factor,
  cpp_hardware_params_t *cpp_hw_params, float effects_factor,
  void *ref_noise_profile_i,
  void *ref_noise_profile_iplus1,
  void *p_reseved_WNR_data,
  void *module_chromatix);
int32_t cpp_hw_params_asf_interpolate(cpp_hardware_t *cpphw,
  cpp_hardware_params_t *hw_params, modulesChromatix_t *module_chromatix,
  cpp_params_aec_trigger_info_t *trigger_input);
void cpp_hw_params_copy_asf_diag_params(struct cpp_frame_info_t *frame_info,
  asfsharpness7x7_t *diag_cpyto_params);
void cpp_hw_params_copy_asf9x9_diag_params(struct cpp_frame_info_t *frame_info,
  asfsharpness9x9_t *diag_cpyto_params);
void cpp_hw_params_copy_wnr_diag_params(struct cpp_frame_info_t *frame_info,
  wavelet_t *diag_cpyto_params);
cpp_firmware_version_t cpp_hardware_get_fw_version(cpp_hardware_t *cpphw);
int32_t cpp_hw_params_update_tnr_params(modulesChromatix_t *module_chromatix,
  cpp_hardware_params_t *hw_params, cpp_params_aec_trigger_info_t *aec_trigger);
void cpp_hw_params_interpolate_zoom_scale_ext(cpp_hardware_params_t *hw_params,
  chromatix_wavelet_type *wavelet_chromatix_data, float scale_ratio,
  uint32_t reg_strt_idx_start, uint32_t reg_strt_idx_end,
  float reg_strt_interp_ratio, uint32_t scalor);
uint32_t cpp_hw_params_update_out_clamp(modulesChromatix_t *module_chromatix,
  cpp_hardware_params_t *hw_params);

int32_t cpp_params_calculate_crop(cpp_hardware_params_t *hw_params);

void cpp_hardware_prepare_frame(cpp_hardware_t *cpphw,
  cpp_hardware_params_t *hw_params,
  struct cpp_frame_info_t *frame_info, uint32_t plane);
/*--------------------------- firmware 1.8 functions -------------------------*/

void run_TW_logic_1_8(struct cpp_plane_info_t *in_ary,
  uint32_t src_tile_index_x_counter, uint32_t src_tile_index_y_counter,
  struct cpp_striping_algorithm_info *out_ary);

void run_TF_logic_1_8(struct cpp_plane_info_t *in_ary,
  uint32_t src_tile_index_x_counter, uint32_t src_tile_index_y_counter,
  struct cpp_striping_algorithm_info *out_ary);

void set_start_of_frame_parameters_1_8(struct cpp_plane_info_t *in_ary);

void cpp_prepare_tnr_fetch_engine_info(struct cpp_plane_info_t *plane_info,
  uint32_t stripe_num);

void cpp_prepare_tnr_write_engine_info(struct cpp_plane_info_t *plane_info,
  uint32_t stripe_num);

uint32_t* cpp_create_frame_message_1_8(
   struct cpp_frame_info_t *cpp_frame_info, uint32_t* len);

void set_default_crop_padding_1_8(struct cpp_plane_info_t *plane_info);

void cpp_prepare_crop_info_1_8(struct cpp_plane_info_t *plane_info,
  uint32_t stripe_num);

void enable_crop_engines_1_8(struct cpp_plane_info_t *in_info,
  struct cpp_striping_algorithm_info *cur_strip_info);

int32_t cpp_prepare_prescaler_bf_info(struct cpp_frame_info_t *frame_info);

int32_t cpp_prepare_tnr_bf_info(struct cpp_frame_info_t *frame_info,
  cpp_hardware_params_t *hw_params);

void cpp_prepare_pbf_info(struct cpp_frame_info_t *frame_info);

int32_t cpp_hw_params_update_wnr_params_1_10(modulesChromatix_t
  *module_chromatix, cpp_hardware_params_t *hw_params,
  cpp_params_aec_trigger_info_t *aec_trigger);

void run_TW_logic_1_10(struct cpp_plane_info_t* in_ary,
  uint32_t src_tile_index_x_counter,
  uint32_t src_tile_index_y_counter,
  struct cpp_striping_algorithm_info* out_ary);

void run_TF_logic_1_10(struct cpp_plane_info_t* in_ary,
  uint32_t src_tile_index_x_counter,
  uint32_t src_tile_index_y_counter,
  struct cpp_striping_algorithm_info* out_ary);

void cpp_hw_params_radial_1_12(struct cpp_plane_info_t *in_ary,
  uint32_t src_tile_index_x_counter, uint32_t src_tile_index_y_counter,
  int64_t *pad_left, int64_t *pad_top,
  struct cpp_striping_algorithm_info* out_ary);

void set_start_of_frame_parameters_1_10(struct cpp_plane_info_t * in_ary);

uint32_t* cpp_create_frame_message_1_10(
  struct cpp_frame_info_t* cpp_frame_info, uint32_t* len);

uint32_t* cpp_create_frame_message_1_12(
  struct cpp_frame_info_t* cpp_frame_info, uint32_t* len);

uint32_t* cpp_create_frame_message_1_5(
   struct cpp_frame_info_t *cpp_frame_info, uint32_t* len,
   cpp_firmware_version_t fw_version);

void cpp_pack_asf_kernel_1_5(uint32_t *frame_msg, int16_t *filter);

uint32_t cpp_hw_params_get_num_k_entries
  (cpp_firmware_version_t fw_version);

void cpp_prepare_dsdn_stripe_info(struct cpp_plane_info_t *plane_info,
  int stripe_num);
void cpp_prepare_dsdn_fetch_engine_info(struct cpp_plane_info_t *plane_info,
  uint32_t stripe_num);
void cpp_prepare_mmu_prefetch_fetch_engine_config(
  struct cpp_plane_info_t *plane_info, int32_t plane);
void cpp_prepare_mmu_prefetch_write_engine_config(
  struct cpp_plane_info_t *plane_info, int32_t plane);

#ifndef CHROMATIX_EXT
#define cpp_hw_update_hdr_2d_asf_radial_ext( \
  chromatix_cpp, hw_params, \
  aec_trigger, \
  input_rnr_params) 0 \

#define cpp_hw_update_hdr_2d_wnr_radial_ext( \
  module_chromatix, hw_params, \
  aec_trigger, wnr_info) 0 \

#define cpp_hw_update_hdr_2d_adj_params(module_chromatix, \
  hw_params, \
  aec_trigger, \
  wnr_adj_param) 0
#endif

#endif
