/* q3a_stats_hw.h
 *
 * Copyright (c) 2013-2015, 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __Q3A_STATS_HW_H__
#define __Q3A_STATS_HW_H__
#include "mtype.h"
#include "mct_event_stats.h"
#include "mct_module.h"
#include "include/q3a_3hdr_stats.h"

#define MAX_BE_STATS_NUM     3888 /*32x24, usually we only use 32x24*/
#define MAX_BG_STATS_NUM     3888  /* 72x54, usually we only use 64x48 */
#define MAX_BF_STATS_NUM      252  /* 18x14 */
#define MAX_YUV_STATS_NUM     256  /* 16x16, aec & awb */
#define MAX_YUV_AF_STATS_NUM   81  /* 9x9, AF */
#define MAX_HIST_STATS_NUM    256
#define MAX_BHIST_STATS_NUM  4096

#define MAX_CS_STATS_NUM     1560
#define MAX_RS_STATS_NUM     4096
#define MAX_RS_H_REGIONS       16

#define MAX_BE_H_NUM     64
#define MAX_BE_V_NUM     48

#define MAX_BF_H_NUM           18
#define MAX_BF_V_NUM           14
#define MAX_BF_BLOCK_WIDTH    336
#define MAX_BF_BLOCK_HEIGHT   252

/* by HW the min block sixe is 6x2
 * but noise is too bad for stats */
#define MIN_BF_BLOCK_WIDTH     64
#define MIN_BF_BLOCK_HEIGHT    48

#define MAX_BF_KERNEL_NUM      2

/** camera_bestshot_mode_type
 *
 *  This enumeration represents different bestshot (or scene) modes.
 **/
typedef enum {
  CAMERA_BESTSHOT_OFF = 0,
  CAMERA_BESTSHOT_AUTO = 1,
  CAMERA_BESTSHOT_LANDSCAPE = 2,
  CAMERA_BESTSHOT_SNOW,
  CAMERA_BESTSHOT_BEACH,
  CAMERA_BESTSHOT_SUNSET,
  CAMERA_BESTSHOT_NIGHT,
  CAMERA_BESTSHOT_PORTRAIT,
  CAMERA_BESTSHOT_BACKLIGHT,
  CAMERA_BESTSHOT_SPORTS,
  CAMERA_BESTSHOT_ANTISHAKE,
  CAMERA_BESTSHOT_FLOWERS,
  CAMERA_BESTSHOT_CANDLELIGHT,
  CAMERA_BESTSHOT_FIREWORKS,
  CAMERA_BESTSHOT_PARTY,
  CAMERA_BESTSHOT_NIGHT_PORTRAIT,
  CAMERA_BESTSHOT_THEATRE,
  CAMERA_BESTSHOT_ACTION,
  CAMERA_BESTSHOT_AR,
  CAMERA_BESTSHOT_FACE_PRIORITY,
  CAMERA_BESTSHOT_BARCODE,
  CAMERA_BESTSHOT_HDR,
  CAMERA_BESTSHOT_MAX
} camera_bestshot_mode_type;

/** time_stamp_t
 *    @time_stamp_sec: The seconds component of the timestamp
 *    @time_stamp_us:  The microseconds component of the timestamp
 *
 *  This structure represents a standard OS timestamp, including two
 *  components - time in seconds and time in microseconds.
 **/
typedef struct {
  uint32_t time_stamp_sec; /*time stamp second*/
  uint32_t time_stamp_us;  /*time stamp microsecond*/
} time_stamp_t;

/** q3a_roi_type_t
 *
 **/
typedef enum {
  ROI_TYPE_GENERAL,
  ROI_TYPE_FACE,
} q3a_roi_type_t;

/** stats_roi_t
 *    @x:  The x coordinate of the ROI
 *    @y:  The y coordinate of the ROI
 *    @dx: The width of the ROI
 *    @dy: The height of the ROI
 *
 *  This structure is used to pass the ROI position and size to the ISP.
 *  ISP will use this parameters to calculate the stats for the specified
 *  region of interest.
 **/
typedef struct {
  int32_t x;
  int32_t y;
  int32_t dx;
  int32_t dy;
} stats_roi_t;

/** stats_hdr_mode_t
 *
 *  This enumeration represents what HDR mode stats are collected for
 */
typedef enum {
  EXPOSURE_ALL = 0,
  EXPOSURE_LONG,
  EXPOSURE_SHORT,
} stats_hdr_mode_t;

/** stats_extra_awb_stats_t
 *    @GLB_Y:     Y sum of pixels with Ymin < Y <Ymax
 *    @GLB_Cb:    Cb sum of pixels with Ymin < Y < Ymax
 *    @GLB_Cr:    Cr sum of pixels with Ymin < Y < Ymax
 *    @GLB_N:     Num of pixels in the global AWB statistics
 *    @Green_r:   R sum of green zone pixels with Ymin < Y < Ymax
 *    @Green_g:   G sum of green zone pixels with Ymin < Y < Ymax
 *    @Green_b:   B sum of green zone pixels with Ymin < Y < Ymax
 *    @Green_N:   Number of green zone pixels
 *    @ExtBlue_r: R sum of blue zone pixels with Ymin < Y < Ymax
 *    @ExtBlue_g: G sum of blue zone pixels with Ymin < Y < Ymax
 *    @ExtBlue_b: B sum of blue zone pixels with Ymin < Y < Ymax
 *    @ExtBlue_N: Number of blue zone pixels
 *    @ExtRed_r:  R sum of red zone pixels with Ymin < Y < Ymax
 *    @ExtRed_g:  G sum of red zone pixels with Ymin < Y < Ymax
 *    @ExtRed_b:  B sum of red zone pixels with Ymin < Y < Ymax
 *    @ExtRed_N:  Number of red zone pixels
 *
 *  This structure is used for extra AWB statistics
 **/
typedef struct {
  uint32_t GLB_Y;     /* Y sum of pixels with Ymin < Y < Ymax */
  uint32_t GLB_Cb;    /* Cb sum of pixels with Ymin < Y < Ymax */
  uint32_t GLB_Cr;    /* Cr sum of pixels with Ymin < Y < Ymax */
  uint32_t GLB_N;     /* Number of pixels in the global AWB statistics */

  uint32_t Green_r;   /* R sum of green zone pixels with Ymin < Y < Ymax */
  uint32_t Green_g;   /* G sum of green zone pixels with Ymin < Y<  Ymax */
  uint32_t Green_b;   /* B sum of green zone pixels with Ymin < Y < Ymax */
  uint32_t Green_N;   /* Number of green zone pixels */

  uint32_t ExtBlue_r; /* R sum of blue zone pixels with Ymin < Y < Ymax */
  uint32_t ExtBlue_g; /* G sum of blue zone pixels with Ymin < Y < Ymax */
  uint32_t ExtBlue_b; /* B sum of blue zone pixels with Ymin < Y < Ymax */
  uint32_t ExtBlue_N; /* Number of blue zone pixels */

  uint32_t ExtRed_r;  /* R sum of red zone pixels with Ymin < Y < Ymax */
  uint32_t ExtRed_g;  /* G sum of red zone pixels with Ymin < Y < Ymax */
  uint32_t ExtRed_b;  /* B sum of red zone pixels with Ymin < Y < Ymax */
  uint32_t ExtRed_N;  /* Number of red zone pixels*/
} stats_extra_awb_stats_t;

/** q3a_aec_stats_t
 *    @ae_region_h_num: Horizontal number of regions
 *    @ae_region_v_num: Vertical number of regions
 *    @SY:              Array of calculated luma sums for each region
 *
 *  This structure is used to pass the ROI position and size to the ISP.
 *  ISP will use this parameters to calculate the stats for the specified
 *  region of interest. The coordinates and dimension are in terms of
 *  Sensor resolution.
 **/
typedef struct {
  uint32_t ae_region_h_num;        /* max 16 */
  uint32_t ae_region_v_num;        /* max 16 */
  uint32_t SY[MAX_YUV_STATS_NUM];  /* 24 bits, sum of luminance in each
                                         * of the X regions */
} q3a_aec_stats_t;

/** q3a_af_stats_t
 *    @af_region_h_num: Horizontal number of regions
 *    @af_region_v_num: Vertical number of regions
 *    @focus_val:       Array of calculated focus values for each region
 *    @Focus:           Focus vale - only 27 bits used
 *    @NFocus:          Count of number of rows having - 9 bits used
 *
 *  This structure is used to pass the calculated AF stats from the ISP.
 **/
typedef struct {
  uint32_t af_region_h_num; /* max 9 */
  uint32_t af_region_v_num; /* max 9 */
  uint32_t focus_val[MAX_YUV_AF_STATS_NUM];
  uint32_t Focus;      /* 27 bits, focus value */
  uint32_t NFocus;     /* 9 bits, count of number of rows having */
}  q3a_af_stats_t;

/** q3a_awb_stats_t
 *    @wb_region_h_num: Horizontal number of regions
 *    @wb_region_v_num: Vertical number of regions
 *    @SCb:             24 bits, sum of Cb in each of the X regions for the
 *                      pixels that meet the inequalities
 *    @SCr:             24 bits, sum of Cr in each of the X regions for the
 *                      pixels that meet the inequalities
 *    @SY1:             24 bits, sum of Y in each of the X regions for the
 *                      pixels that meet the inequalities
 *    @NSCb:            16 bits, number of pixels included in each of the
 *                      X regions for the pixels that meet the inequalities
 *    @awb_extra_stats: The extra AWB stats for the frame
 *
 *  This structure is used to pass the calculated AWB stats from the ISP.
 **/
typedef struct {
  uint32_t wb_region_h_num;             /* max 16 */
  uint32_t wb_region_v_num;             /* max 16 */
  uint32_t SCb[MAX_YUV_STATS_NUM]; /* 24 bits, sum of Cb in each of the
                                         * X regions for the pixels that meet
                                         * the inequalities */
  uint32_t SCr[MAX_YUV_STATS_NUM]; /* 24 bits, sum of Cr in each of the
                                         * X regions for the pixels that meet
                                         * the inequalities */
  uint32_t SY1[MAX_YUV_STATS_NUM]; /* 24 bits, sum of Y in each of the
                                         * X regions for the pixels that meet
                                         * the inequalities */
  uint32_t NSCb[MAX_YUV_STATS_NUM]; /* 16 bits, number of pixels included
                                          * inSCb in each of the X regions for
                                          * the pixels that meet the
                                          * inequalities */
  stats_extra_awb_stats_t awb_extra_stats;
} q3a_awb_stats_t;

/** q3a_cs_stats_t
 *    @num_col_sum: The number of columns with sums
 *    @col_sum:     Array containing the column sums
 *
 *  This structure is used to pass the column stats from the ISP.
 **/
typedef struct {
  uint32_t num_col_sum;
  uint32_t col_sum[MAX_CS_STATS_NUM * 4];
} q3a_cs_stats_t;

/** q3a_rs_stats_t
 *    @num_row_sum: The number of rows with sums
 *    @row_sum:     Array containing the row sums
 *
 *  This structure is used to pass the row stats from the ISP.
 **/
typedef struct {
  uint32_t num_row_sum;
  uint32_t row_sum[MAX_RS_H_REGIONS][MAX_RS_STATS_NUM];
  uint32_t num_h_regions;
  uint32_t num_v_regions;
} q3a_rs_stats_t;

typedef enum {
  IHIST_VALID_HISTOGRAM = (1 << 0),
  IHIST_VALID_R         = (1 << 1),
  IHIST_VALID_B         = (1 << 2),
  IHIST_VALID_G         = (1 << 3),
  IHIST_VALID_YCC       = (1 << 4),
} q3a_ihist_valid_fields_t;

/** q3a_ihist_stats_t
 *    @histogram: Array containing legacy histogram values.
 *    @r:   Array containing the R histogram values.
 *    @g:   Array containing the G histogram values.
 *    @b:   Array containing the B histogram values.
 *    @ycc: Array containing the YCbCr histogram values.
 *    @num_bins: number of bins (might be smaller than allocated storage)
 *    @valid_fields: bit mask indicating which fields are valid
 *
 *  This structure is used to pass the histogram stats from the ISP.
 **/
typedef struct {
  union {
    uint32_t histogram[MAX_HIST_STATS_NUM];
    uint32_t r[MAX_HIST_STATS_NUM];
  };
  uint32_t g[MAX_HIST_STATS_NUM];
  uint32_t b[MAX_HIST_STATS_NUM];
  uint32_t ycc[MAX_HIST_STATS_NUM];
  uint32_t num_bins;
  uint32_t valid_fields;
} q3a_ihist_stats_t;

/** q3a_be_stats_t
 *    @be_region_h_num: Horizontal number of regions for the BE
 *    @be_region_v_num: Vertical number of regions for the BE
 *    @be_r_sum:        Array with the sums for the red pixels
 *    @be_b_sum:        Array with the sums for the blue pixels
 *    @be_gr_sum:       Array with the sums for the green-red pixels
 *    @be_gb_sum:       Array with the sums for the green-blue pixels
 *    @be_r_num:
 *    @be_b_num:
 *    @be_gr_num:
 *    @be_gb_num:
 *
 *  This structure is used to pass the calculated BAYER exposure statistics
 *  from the ISP.
 **/
typedef struct {
  int nx;  // horizontal number of BE subwindows, hard coded as BE_H
  int ny;  // vertical number of BE subwindows, hard coded as BE_V

  int dx; // horizontal offset (per-channel) of BE top-left point
  int dy; // vertical offset (per-channel) of BE top-left point

  int sx; // horizontal size (per-channel) of each BE subwindow
  int sy; // vertical size (per-channel) of each BE subwindow

  int rMax;
  int grMax;
  int gbMax;
  int bMax;

  // BE stats
  int r_sum[MAX_BE_STATS_NUM]; // BE sum table for R channel
  int gr_sum[MAX_BE_STATS_NUM]; // BE sum table for GR channel
  int gb_sum[MAX_BE_STATS_NUM]; // BE sum table for GB channel
  int b_sum[MAX_BE_STATS_NUM]; // BE sum table for B channel

  int r_count[MAX_BE_STATS_NUM]; // BE count table for R channel
  int gr_count[MAX_BE_STATS_NUM]; // BE count table for GR channel
  int gb_count[MAX_BE_STATS_NUM]; // BE count table for GB channel
  int b_count[MAX_BE_STATS_NUM]; // BE count table for B channel
} q3a_be_stats_t;

typedef enum {
  BG_EX_FIELDS_NONE      = 0,
  BG_EX_FIELDS_SATURATED = (1 << 0),
} q3a_bg_ex_fields_t;

/** q3a_hdr_be_stats_t
 *    @be_region_h_num: Horizontal number of regions for the BE
 *    @be_region_v_num: Vertical number of regions for the BE
 *    @be_r_sum:        Array with the sums for the red pixels
 *    @be_b_sum:        Array with the sums for the blue pixels
 *    @be_gr_sum:       Array with the sums for the green-red pixels
 *    @be_gb_sum:       Array with the sums for the green-blue pixels
 *    @be_r_num:
 *    @be_b_num:
 *    @be_gr_num:
 *    @be_gb_num:
 *    @be_r_sat_sum:  Array with the saturated sums for the red pixels
 *    @be_b_sat_sum:  Array with the saturated sums for the blue pixels
 *    @be_gr_sat_sum: Array with the saturated sums for the green-red pixels
 *    @be_gb_sat_sum: Array with the saturated sums for the green-blue pixels
 *    @be_r_sat_num:
 *    @be_b_sat_num:
 *    @be_gr_sat_num:
 *    @be_gb_sat_num:
 *    @ex_fields: flag to indicate whether saturation info is
 *              present
 *
 *  This structure is used to pass the calculated BAYER exposure statistics
 *  from the ISP.
 **/
typedef struct {
  uint32_t be_region_h_num;
  uint32_t be_region_v_num;

  /* sx sy */
  uint32_t rgnWidth;
  uint32_t rgnHeight;

  /* dx dy */
  uint32_t rgnHOffset;
  uint32_t rgnVOffset;

  int saturation_thresh; // hard coded as SATURATION_THRES
  uint32_t region_pixel_cnt;
  uint16_t rMax, bMax, grMax, gbMax;

  uint32_t be_r_sum[MAX_BE_STATS_NUM];
  uint32_t be_b_sum[MAX_BE_STATS_NUM];
  uint32_t be_gr_sum[MAX_BE_STATS_NUM];
  uint32_t be_gb_sum[MAX_BE_STATS_NUM];
  uint32_t be_r_num[MAX_BE_STATS_NUM];
  uint32_t be_b_num[MAX_BE_STATS_NUM];
  uint32_t be_gr_num[MAX_BE_STATS_NUM];
  uint32_t be_gb_num[MAX_BE_STATS_NUM];
  uint32_t be_r_sat_sum[MAX_BE_STATS_NUM];
  uint32_t be_b_sat_sum[MAX_BE_STATS_NUM];
  uint32_t be_gr_sat_sum[MAX_BE_STATS_NUM];
  uint32_t be_gb_sat_sum[MAX_BE_STATS_NUM];
  uint32_t be_r_sat_num[MAX_BE_STATS_NUM];
  uint32_t be_b_sat_num[MAX_BE_STATS_NUM];
  uint32_t be_gr_sat_num[MAX_BE_STATS_NUM];
  uint32_t be_gb_sat_num[MAX_BE_STATS_NUM];

  stats_hdr_mode_t hdr_mode;
  q3a_bg_ex_fields_t ex_fields;
} q3a_hdr_be_stats_t;

/** q3a_bg_stats_t
 *    @bg_region_h_num:  Horizontal number of regions for the BG
 *    @bg_region_v_num:  Vertical number of regions for the BG
 *    @region_pixel_cnt: The count of the region pixels
 *    @bg_r_sum:         Array with the sums for the red pixels
 *    @bg_b_sum:         Array with the sums for the blue pixels
 *    @bg_gr_sum:        Array with the sums for the green-red pixels
 *    @bg_gb_sum:        Array with the sums for the green-blue pixels
 *    @bg_r_num:
 *    @bg_b_num:
 *    @bg_gr_num:
 *    @bg_gb_num:
 *    @bg_r_sat_sum:  Array with the saturated sums for the red pixels
 *    @bg_b_sat_sum:  Array with the saturated sums for the blue pixels
 *    @bg_gr_sat_sum: Array with the saturated sums for the green-red pixels
 *    @bg_gb_sat_sum: Array with the saturated sums for the green-blue pixels
 *    @bg_r_sat_num:
 *    @bg_b_sat_num:
 *    @bg_gr_sat_num:
 *    @bg_gb_sat_num:
 *
 *    @ex_fields  Bit mask representing whether certain
 *                extended fields are present
 *                q3a_bg_ex_fields_t lists the possible supported fields
 *
 *
 *  This structure is used to pass the calculated BAYER grid statistics
 *  from the ISP, used by the AWB library.
 **/
typedef struct {
  uint32_t bg_region_h_num; /* 64, max 72 */
  uint32_t bg_region_v_num; /* 48, max 54 */
  uint32_t region_pixel_cnt;
  uint32_t bg_region_height;
  uint32_t bg_region_width;
  uint16_t rMax, bMax, grMax, gbMax;
  uint32_t bg_r_sum[MAX_BG_STATS_NUM];
  uint32_t bg_b_sum[MAX_BG_STATS_NUM];
  uint32_t bg_gr_sum[MAX_BG_STATS_NUM];
  uint32_t bg_gb_sum[MAX_BG_STATS_NUM];
  uint32_t bg_r_num[MAX_BG_STATS_NUM];
  uint32_t bg_b_num[MAX_BG_STATS_NUM];
  uint32_t bg_gr_num[MAX_BG_STATS_NUM];
  uint32_t bg_gb_num[MAX_BG_STATS_NUM];

  /* this section is valid only when BG_EXTENDED_SATURATED is set */
  uint32_t bg_r_sat_sum[MAX_BG_STATS_NUM];
  uint32_t bg_b_sat_sum[MAX_BG_STATS_NUM];
  uint32_t bg_gr_sat_sum[MAX_BG_STATS_NUM];
  uint32_t bg_gb_sat_sum[MAX_BG_STATS_NUM];
  uint32_t bg_r_sat_num[MAX_BG_STATS_NUM];
  uint32_t bg_b_sat_num[MAX_BG_STATS_NUM];
  uint32_t bg_gr_sat_num[MAX_BG_STATS_NUM];
  uint32_t bg_gb_sat_num[MAX_BG_STATS_NUM];

  uint32_t ex_fields;
} q3a_bg_stats_t;

/** bf_grid_stats_t:
 *
 *    @bf_region_h_num: Horizontal number of regions for the BF
 *    @bf_region_v_num: Vertical number of regions for the BF
 *    @use_max_fv:
 *    @bf_r_sum:        Array with the sums for the red pixels
 *    @bf_b_sum:        Array with the sums for the blue pixels
 *    @bf_gr_sum:       Array with the sums for the green-red pixels
 *    @bf_gb_sum:       Array with the sums for the green-blue pixels
 *    @bf_r_sharp:      Array with the sums for the sharp red pixels
 *    @bf_b_sharp:      Array with the sums for the sharp blue pixels
 *    @bf_gr_sharp:     Array with the sums for the sharp green-red pixels
 *    @bf_gb_sharp:     Array with the sums for the sharp green-blue pixels
 *    @bf_r_num:        number of pixels per grid in R channel
 *    @bf_b_num:        number of pixels per grid in B channel
 *    @bf_gr_num:       number of pixels per grid in Gr channel
 *    @bf_gb_num:       number of pixels per grid in Gb channel
 *    @bf_r_max_fv:     Array with the max focus value for the red pixels
 *    @bf_b_max_fv:     Array with the max focus value for the blue pixels
 *    @bf_gr_max_fv:    Array with the max focus value for the green-red pixels
 *    @bf_gb_max_fv:    Array with the max focus value for the green-blue pixels
 *
 **/
typedef struct _bf_grid_stats_t {
  uint32_t bf_region_h_num;
  uint32_t bf_region_v_num;
  uint8_t  use_max_fv;
  uint32_t bf_r_sum[MAX_BF_STATS_NUM];
  uint32_t bf_b_sum[MAX_BF_STATS_NUM];
  uint32_t bf_gr_sum[MAX_BF_STATS_NUM];
  uint32_t bf_gb_sum[MAX_BF_STATS_NUM];
  uint64_t bf_r_sharp[MAX_BF_STATS_NUM];
  uint64_t bf_b_sharp[MAX_BF_STATS_NUM];
  uint64_t bf_gr_sharp[MAX_BF_STATS_NUM];
  uint64_t bf_gb_sharp[MAX_BF_STATS_NUM];
  uint32_t bf_r_num[MAX_BF_STATS_NUM];
  uint32_t bf_b_num[MAX_BF_STATS_NUM];
  uint32_t bf_gr_num[MAX_BF_STATS_NUM];
  uint32_t bf_gb_num[MAX_BF_STATS_NUM];
  uint64_t bf_r_max_fv[MAX_BF_STATS_NUM];
  uint64_t bf_b_max_fv[MAX_BF_STATS_NUM];
  uint64_t bf_gr_max_fv[MAX_BF_STATS_NUM];
  uint64_t bf_gb_max_fv[MAX_BF_STATS_NUM];
} bf_grid_stats_t;

/** bf_fw_stats_t:
 *
 *  @bf_h1_sum: sum of pixels in H1 filter
 *  @bf_h2_sum: sum of pixels in H2 filter
 *  @bf_v_sum: sum of pixels in V filter
 *  @bf_h1_sharp: sharpness of pixels in H1 filter
 *  @bf_h2_sharp: sharpness of pixels in H2 filter
 *  @bf_v_sharp: sharpness of pixels in v filter
 *  @bf_h1_num: number of pixels in H1 filter
 *  @bf_h2_num: number of pixels in H1 filter
 *  @bf_v_num: number of pixels in V filter
 *  @bf_fw_roi_cfg: roi cfg
 **/
typedef struct _bf_fw_stats_t {
  uint64_t                bf_h1_sum[BF_FW_MAX_ROI_REGIONS];
  uint64_t                bf_h2_sum[BF_FW_MAX_ROI_REGIONS];
  uint64_t                bf_v_sum[BF_FW_MAX_ROI_REGIONS];
  uint64_t                bf_h1_sharp[BF_FW_MAX_ROI_REGIONS];
  uint64_t                bf_h2_sharp[BF_FW_MAX_ROI_REGIONS];
  uint64_t                bf_v_sharp[BF_FW_MAX_ROI_REGIONS];
  uint32_t                bf_h1_num[BF_FW_MAX_ROI_REGIONS];
  uint32_t                bf_h2_num[BF_FW_MAX_ROI_REGIONS];
  uint32_t                bf_v_num[BF_FW_MAX_ROI_REGIONS];
  bf_fw_roi_cfg_t         bf_fw_roi_cfg;
  mct_bf_pd_stats_data_t  pd_data;
} bf_fw_stats_t;

/** q3a_bf_stats_t
 *    @bf_region_h_num: Horizontal number of regions for the BF
 *    @bf_region_v_num: Vertical number of regions for the BF
 *    @use_max_fv:
 *    @bf_r_sum:        Array with the sums for the red pixels
 *    @bf_b_sum:        Array with the sums for the blue pixels
 *    @bf_gr_sum:       Array with the sums for the green-red pixels
 *    @bf_gb_sum:       Array with the sums for the green-blue pixels
 *    @bf_r_sharp:      Array with the sums for the sharp red pixels
 *    @bf_b_sharp:      Array with the sums for the sharp blue pixels
 *    @bf_gr_sharp:     Array with the sums for the sharp green-red pixels
 *    @bf_gb_sharp:     Array with the sums for the sharp green-blue pixels
 *    @bf_r_num:
 *    @bf_b_num:
 *    @bf_gr_num:
 *    @bf_gb_num:
 *    @bf_r_max_fv:     Array with the max focus value for the red pixels
 *    @bf_b_max_fv:     Array with the max focus value for the blue pixels
 *    @bf_gr_max_fv:    Array with the max focus value for the green-red pixels
 *    @bf_gb_max_fv:    Array with the max focus value for the green-blue pixels
 *
 *  This structure is used to pass the calculated BAYER focus statistics
 *  from the ISP.
 **/
typedef struct {
  uint32_t            config_id;
  af_config_mask_type mask;
  union {
    bf_grid_stats_t grid_stats;
    bf_fw_stats_t   fw_stats;
  } u;
} q3a_bf_stats_t;

/** q3a_bhist_stats_t
 *    @bayer_r_hist:  Array containing the red histogram values
 *    @bayer_b_hist:  Array containing the blue histogram values
 *    @bayer_gr_hist: Array containing the green-red histogram values
 *    @bayer_gb_hist: Array containing the green-blue histogram values
 *    @num_bins:      Num of bins containing valid data; might be smaller
 *                    than the max storage allocated
 *                    When num_bins is 0, it indicates no valid is present
 *    @hdr_mode:      HDR mode
 *    @bayer_r_hist_valid: true if bayer_r_hist contains valid data
 *    @bayer_gr_hist_valid: true if bayer_gr_hist contains valid data
 *    @bayer_gb_hist_valid: true if bayer_gb_hist contains valid data
 *    @bayer_b_hist_valid: true if bayer_b_hist contains valid data
 *
 *  This structure is used to pass the BAYER histogram statistics from the ISP.
 **/
typedef struct {
  uint32_t bayer_r_hist[MAX_BHIST_STATS_NUM];
  uint32_t bayer_b_hist[MAX_BHIST_STATS_NUM];
  uint32_t bayer_gr_hist[MAX_BHIST_STATS_NUM];
  uint32_t bayer_gb_hist[MAX_BHIST_STATS_NUM];
  uint32_t num_bins;
  stats_hdr_mode_t hdr_mode;
  boolean bayer_r_hist_valid;
  boolean bayer_gr_hist_valid;
  boolean bayer_gb_hist_valid;
  boolean bayer_b_hist_valid;
} q3a_bhist_stats_t;

typedef struct {
  stats_hdr3_hist_data_type hdr3_hist_data;
  uint32_t hdr3_sensor_type;
}q3a_3hdr_stats_t;

typedef struct {
  uint32_t  frame_id;
  uint32_t  stats_size;
  void      *custom_stats;
} custom_stats_af;

#endif /* __Q3A_STATS_HW_H__ */
