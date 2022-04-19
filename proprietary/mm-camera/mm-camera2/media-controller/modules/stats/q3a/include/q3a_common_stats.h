/* q3a_common_stats.h
 *
 * Copyright (c) 2014-2016, 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __Q3A_COMMON_STATS_H__
#define __Q3A_COMMON_STATS_H__


/* ==========================================================================
                     INCLUDE FILES FOR MODULE
========================================================================== */
#include "q3a_platform.h"
#include "q3a_3hdr_stats.h"
/* ==========================================================================
                       Preprocessor Definitions and Constants
========================================================================== */


#ifdef __cplusplus
extern "C"
{
#endif

/* ==========================================================================
                       Static Declarations
========================================================================== */

#define MAX_EXP_ENTRIES                 5    /**< Defines the maximun exposure bracketing entries. */
#define MAX_EXP_CHAR                    50   /**< Defines the maximun size of the exposure bracketing characters. */
#define MAX_YUV_STATS_NUM               256  /**< Defines the maximun size of the Legacy stats */
#define MAX_BG_STATS_NUM                3888 /**< 72x54, usually we only use 64x48 */
#define MAX_HIST_STATS_NUM              256  /**< Defines the maximun size of hist stats */
#define MAX_BHIST_STATS_NUM             4096 /**< Defines the maximun size of hist stats */
#define AEC_BG_STATS_CONSUMP_BIT_WIDTH  8    /**< Defines the BG stats bit needed for AEC algo */
#define AEC_BE_STATS_CONSUMP_BIT_WIDTH  8    /**< Defines the BE stats bit needed for AEC algo */
#define AWB_BG_STATS_CONSUMP_BIT_WIDTH  8    /**< Defines the BG stats bit needed for AWB algo */
#define MAX_NUM_OF_BHIST_CHANNELS       4    /**< Defines the maximum number of BHist channels present */
#define MAX_ROW_SUM_STATS_NUM           4096 /**< Defines the maximum number of row sum stats */
#define MAX_ROW_SUM_H_REGIONS           16   /**< Defines the maximum number of row sum horizontal regions */
#define MAX_BE_H_NUM                    64   /**< Defines the maximum number of BE horizontal stats */
#define MAX_BE_V_NUM                    48   /**< Defines the maximum number of BE vertical stats */
#define MAX_3A_MULTI_CAMERA_ID          2    /**< Defines the MAX number of simultaneous cameras supported by multi 3A algo thread*/

/* ==========================================================================
                       Type Declarations
========================================================================== */
/**
 * Defines the format of the Bayer Grid stats for each channel.
 *
 * @channel_sums: Pointer to the list of channel sums.
 * @channel_counts: Pointer to the list of channel counts.
 * @channel_sat_sums: Pointer to the list of channel sums for
 *                  regions that have saturated pixels.
 * @channel_sat_counts: Pointer to the list of channel counts
 *                    for regions that have saturated pixels.
 */
typedef struct
{
  const uint32*     channel_sums;
  const uint32*     channel_counts;
  const uint32*     channel_sat_sums;
  const uint32*     channel_sat_counts;
} q3a_core_bg_stats_channel_info_type;

/**
 * Defines the format of the Bayer Exposure stats for each channel.
 *
 * @channel_sums: Pointer to the list of channel sums.
 * @channel_counts: Pointer to the list of channel counts.
 */
typedef struct
{
  const int*     channel_sums;
  const int*     channel_counts;
} q3a_core_be_stats_channel_info_type;

/**
 * Defines the format of the Bayer Grid stats.
 *
 * @frame_id: BG stats frame id.
 * @num_horizontal_regions: Number of horiontal regions.
 * @num_vertical_regions: Number of vertical regions.
 * @region_width: Width of each region.
 * @region_height: Height of each region.
 * @bit_depth: Bit depth of the stats from the hardware.
 * @r_max: Max R value that was used.
 * @gr_max: Max Gr value that was used.
 * @b_max: Max B value that was used.
 * @gb_max: Max Gb value that was used.
 * @r_info: R channel stats.
 * @gr_info: Gr channel stats.
 * @b_info: B channel stats.
 * @gb_info: Gb channel stats.
 * @array_length: Length of each stats array. All arrays are the
 *              same size.
 * @region_pixel_cnt: Number of pixels per region.
 */
typedef struct
{
  uint32                                  frame_id;
  uint32                                  num_horizontal_regions;
  uint32                                  num_vertical_regions;
  uint32                                  region_width;
  uint32                                  region_height;
  uint8                                   bit_depth;
  uint32                                  r_max;
  uint32                                  gr_max;
  uint32                                  b_max;
  uint32                                  gb_max;
  q3a_core_bg_stats_channel_info_type     r_info;
  q3a_core_bg_stats_channel_info_type     gr_info;
  q3a_core_bg_stats_channel_info_type     b_info;
  q3a_core_bg_stats_channel_info_type     gb_info;
  uint32                                  array_length;
  uint32                                  region_pixel_cnt;
} q3a_core_bg_stats_type;

/** aec_hdr_mode_t
 *
 *  This enumeration represents what HDR mode stats are collected for
 */
typedef enum
{
  Q3A_CORE_EXPOSURE_ALL = 0,
  Q3A_CORE_EXPOSURE_LONG,
  Q3A_CORE_EXPOSURE_SHORT
} q3a_core_stats_hdr_type;

/** This enumeration defines the stats type that are valid.
*
* @STATS_YUV:   Mask to ensure YUV stats are present
* @STATS_BAYER: Mask to ensure Bayer stats are present
* @STATS_BHIST: Mask to ensure Bhist stats are present
* @STATS_IHIST: Mask to ensure IHIST stats are present
* @STATS_BE:    Mask to ensure BE stats are present
*/
typedef enum {
  Q3A_CORE_STATS_YUV,
  Q3A_CORE_STATS_BAYER,
  Q3A_CORE_STATS_BHIST,
  Q3A_CORE_STATS_IHIST,
  Q3A_CORE_STATS_HDR_VID,
  Q3A_CORE_STATS_BE,
  Q3A_CORE_STATS_3HDR,
} q3a_core_stats_type;

/** This enumeration defines the various channels for the Bayer histogram (BHist) stats
*
* @HIST_INVALID: Mask to ensure none of the BHist stats are present
* @HIST_R:       Mask to ensure R channel BHist stats are present
* @HIST_GR:      Mask to ensure Gr channel BHist stats are present
* @HIST_GB:      Mask to ensure Gb channel BHist stats are present
* @HIST_B:       Mask to ensure B channel BHist stats are present
* @HIST_G:       Mask to ensure G channel BHist stats are present
* @HIST_Y:       Mask to ensure Y channel BHist stats are present
*/
typedef enum {
  Q3A_CORE_BAYER_HIST_INVALID = 0,
  Q3A_CORE_BAYER_HIST_R,
  Q3A_CORE_BAYER_HIST_GR,
  Q3A_CORE_BAYER_HIST_GB,
  Q3A_CORE_BAYER_HIST_B,
  Q3A_CORE_BAYER_HIST_G,
  Q3A_CORE_BAYER_HIST_Y
} q3a_core_bhist_stats_channel_type;

/** aec_bhist_stats_type
 *  This structure is used to pass the BAYER histogram statistics from the ISP.
 *
 * @num_of_active_channels: Number of active channels (1, 3 or 4 based on stats configuration)
 * @hist_data: Histogram data for each active channel
 * @hist_data_type: Type of data being carried
 * @num_bins: Number of bins
 * @hdr_mode: HDR mode
 * @array_size: array size of stats
 **/
typedef struct
{
  uint8                               num_of_active_channels;
  const uint32*                       hist_data[MAX_NUM_OF_BHIST_CHANNELS];
  q3a_core_bhist_stats_channel_type   hist_data_type[MAX_NUM_OF_BHIST_CHANNELS];
  uint32                              num_bins;
  q3a_core_stats_hdr_type             hdr_mode;
  uint16                              array_size;
} q3a_core_bhist_stats_type;

/** aec_ihist_stats_type
 *  This structure is used to pass the histogram stats from the ISP.
 *
 * @histogram: Array containing legacy histogram values.
 * @r: Array containing the R histogram values.
 * @g: Array containing the G histogram values.
 * @b: Array containing the B histogram values.
 * @ycc: Array containing the YCbCr histogram values.
 * @array_size: array size of the stats
 * @num_bins: number of bins (might be smaller than allocated storage
 * @valid_fileds:bit mask indicating which fields are valid
 **/
typedef struct {
  union {
    const uint32*   histogram;
    const uint32*   r;
  }u;
  const uint32*     g;
  const uint32*     b;
  const uint32*     ycc;
  uint16            array_size;
  uint32            num_bins;
  uint32            valid_fields;
} q3a_core_ihist_stats_type;

/**  This structure is used to pass the ROI position and size to the ISP.
 *  ISP will use this parameters to calculate the stats for the specified
 *  region of interest. The coordinates and dimension are in terms of
 *  Sensor resolution.
 *
 *  @ae_region_h_num: Horizontal number of regions
 *  @ae_region_v_num: Vertical number of regions
 *  @SY: Array of calculated luma sums for each region
 *  @array_size: array size of stats
 **/
typedef struct
{
  uint32         ae_region_h_num;
  uint32         ae_region_v_num;
  const uint32*  SY;
  uint16         array_size;
} q3a_core_yuv_stats_type;

/** Defines the 3HDR statistic  data structure
*
* @stats_hdr3_hist_data: 3HDR statistic sub data structures
* @hdr3_sensor_type:     Indicate 3HDR sensor type
*/
typedef struct
{
  stats_hdr3_hist_data_type stats_hdr3_hist_data;
  stats_hdr3_sensor_type    hdr3_sensor_type;
}q3a_core_3hdr_stats_type;

/**
 * Defines the format of the Bayer Exposure stats.
 *
 * @frame_id: BE stats frame id.
 * @num_horizontal_regions: Number of horiontal regions.
 * @num_vertical_regions: Number of vertical regions.
 * @region_width: Width of each region.
 * @region_height: Height of each region.
 * @bit_depth: Bit depth of the stats from the hardware.
 * @r_max: Max R value that was used.
 * @gr_max: Max Gr value that was used.
 * @b_max: Max B value that was used.
 * @gb_max: Max Gb value that was used.
 * @r_info: R channel stats.
 * @gr_info: Gr channel stats.
 * @b_info: B channel stats.
 * @gb_info: Gb channel stats.
 * @array_length: Length of each stats array. All arrays are the
 *              same size.
 */
typedef struct
{
  uint32                                  frame_id;
  uint32                                  num_horizontal_regions;
  uint32                                  num_vertical_regions;
  uint32                                  region_width;
  uint32                                  region_height;
  uint8                                   bit_depth;
  uint32                                  r_max;
  uint32                                  gr_max;
  uint32                                  b_max;
  uint32                                  gb_max;
  q3a_core_be_stats_channel_info_type     r_info;
  q3a_core_be_stats_channel_info_type     gr_info;
  q3a_core_be_stats_channel_info_type     b_info;
  q3a_core_be_stats_channel_info_type     gb_info;
  uint32                                  array_length;
} q3a_core_be_stats_type;

/**
*  Defines the format of the Row Sum (RS) stats from the ISP
*
* @num_row_sum: Number of row sums
* @row sum: Pointer to the row sum 2-D array
* @num_horizontal_regions: Number of horizontal regions
* @num_vertical_regions: Number of vertical regions
**/
typedef struct
{
    uint32                   num_row_sum;
    uint32                   (*row_sum)[MAX_ROW_SUM_H_REGIONS][MAX_ROW_SUM_STATS_NUM];
    uint32                   num_horizontal_regions;
    uint32                   num_vertical_regions;
} q3a_core_row_sum_stats_type;

/**
 *  This structure is used to pass the calculated stats from the ISP to
 *  the libraries for processing. It packs additional information such as stats id
 *
 * @stats_type_mask:This mask shows what types of stats are passed by ISP
 * @frame_id: Unique Stats Id
 * @yuv_stats: The YUV stats from the ISP.
 * @bg_stats: The BAYER stats from the ISP.
 * @bhist: The Bhist stats from the ISP.
 * @ihist: The Ihist stats from the ISP.
 */
typedef struct
{
  q3a_core_stats_type        stats_type_mask;
  uint32                     frame_id;
  q3a_core_yuv_stats_type    yuv_stats;
  q3a_core_bg_stats_type     bg_stats;
  q3a_core_bhist_stats_type  bhist;
  q3a_core_ihist_stats_type  ihist;
  q3a_core_be_stats_type     be_stats;
  q3a_core_3hdr_stats_type   hdr3_stats;
} aec_stats_input_type;

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __Q3A_STATS_H__


