/* q3a_3hdr_stats.h
 *
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __Q3A_3HDR_STATS_H__
#define __Q3A_3HDR_STATS_H__


/* ==========================================================================
                     INCLUDE FILES FOR MODULE
========================================================================== */


/* ==========================================================================
                       Preprocessor Definitions and Constants
========================================================================== */


#ifdef __cplusplus
extern "C"
{
#endif

/* ==========================================================================
                       Preprocessor Definitions and Constants
========================================================================== */
#if 0
#if !defined(uint8) && !defined(_UINT8_DEFINED)
typedef uint8_t     uint8;
#define _UINT8_DEFINED
#endif

#if !defined(int8) && !defined(_INT8_DEFINED)
typedef int8_t      int8;
#define _INT8_DEFINED
#endif

#if !defined(uint16) && !defined(_UINT16_DEFINED)
typedef uint16_t    uint16;
#define _UINT16_DEFINED
#endif

#if !defined(int16) && !defined(_INT16_DEFINED)
typedef int16_t     int16;
#define _INT16_DEFINED
#endif

#if !defined(uint32) && !defined(_UINT32_DEFINED)
typedef uint32_t            uint32;
#define _UINT32_DEFINED
#endif

#if !defined(int32) && !defined(_INT32_DEFINED)
typedef int32_t              int32;
#define _INT32_DEFINED
#endif

#if !defined(uint64) && !defined(_UINT64_DEFINED)
typedef uint64_t    uint64;
#define _UINT64_DEFINED
#endif

#if !defined(int64) && !defined(_INT64_DEFINED)
typedef int64_t     int64;
#define _INT64_DEFINED
#endif

#ifndef        TRUE
#define        TRUE         1
#endif
#ifndef        FALSE
#define        FALSE        0
#endif
#endif

#define MAX_NUM_OF_3HDR_BHIST_CHANNELS        4       /**< Defines the maximum number of 3HDR BHist channels present */
#define MAX_NUM_OF_3HDR_BHIST_BINS            1024    /**< Defines the maximum number of 3HDR BHist bin counts present */
#define MAX_NUM_OF_3HDR_GRID_REGIONS          3072    /**< Defines the maximum number of 3HDR bayer grid regions count present */

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
  const uint32_t*     channel_sums;
  const uint32_t*     channel_counts;
  const uint32_t*     channel_sat_sums;
  const uint32_t*     channel_sat_counts;
} bg_stats_channel_info_type;

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
  Q3A_CORE_3HDR_BAYER_HIST_INVALID = 0,
  Q3A_CORE_3HDR_BAYER_HIST_R,
  Q3A_CORE_3HDR_BAYER_HIST_GR,
  Q3A_CORE_3HDR_BAYER_HIST_GB,
  Q3A_CORE_3HDR_BAYER_HIST_B,
  Q3A_CORE_3HDR_BAYER_HIST_G,
  Q3A_CORE_3HDR_BAYER_HIST_Y
}stats_3hdr_channel_type;

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
  uint32_t*                               grid_data[MAX_NUM_OF_3HDR_BHIST_CHANNELS];
  stats_3hdr_channel_type                 hist_data_type[MAX_NUM_OF_3HDR_BHIST_CHANNELS];
  uint32_t                                frame_id;
  uint32_t                                num_horizontal_regions;
  uint32_t                                num_vertical_regions;
  uint32_t                                region_width;
  uint32_t                                region_height;
  uint32_t                                bit_depth;
  uint32_t                                r_max;
  uint32_t                                gr_max;
  uint32_t                                b_max;
  uint32_t                                gb_max;
  bg_stats_channel_info_type              r_info;
  bg_stats_channel_info_type              gr_info;
  bg_stats_channel_info_type              b_info;
  bg_stats_channel_info_type              gb_info;
  uint32_t                                array_length;
  uint32_t                                region_pixel_cnt;
} bg_stats_type;

/** Defines the measurement region
*
* @global_region:           stats is measured by full frame region
* @ROI_window_region:       statistics data is measured by specific ROI window
*/
typedef enum
{
    global_region,
    ROI_window_region
}measurement_region_type;

/** Defines the 3HDR statistic sub histogram data structure
*
* @hdr3_exp_split_long:       statitics data is measured by long exposure pixel
* @hdr3_exp_split_middle:     statitics data is measured by middle exposure pixel
* @hdr3_exp_split_short:      statitics data is measured by short exposure pixel
* @hdr3_exp_merged:           statitics data is measured by long/middle/short exposure pixel pixel after merged
*/
typedef enum
{
    hdr3_exp_split_long,
    hdr3_exp_split_middle,
    hdr3_exp_split_short,
    hdr3_exp_merged
}hdr3_merged_type;

/** Defines the 3HDR statistic sub histogram data structure
*
* @hist_data:            The actual histogram data in each channel
* @hist_data_type:       Indicate each channel's color type
* @bin_count:            The bin count for the histogram
* @total_hist_count:     total count for all histogram bin
* @hist_weight_unit:     Histogram weight
* @measurement_region:   Meaturement region
* @merged_type:          merged or slipt long/middle/short expsoure
*/
typedef struct
{
    uint32_t*               hist_data[MAX_NUM_OF_3HDR_BHIST_CHANNELS];
    stats_3hdr_channel_type hist_data_type[MAX_NUM_OF_3HDR_BHIST_CHANNELS];
    uint32_t                bin_count;
    uint32_t                total_hist_count;
    uint32_t                hist_weight_unit;
    measurement_region_type measurement_region;
    hdr3_merged_type        merged_type;
}stats_3hdr_hist_stats;

/** Defines the 3HDR statistic sub data structure
*
* @split_bayer_hist:            RDI split histogram stats
* @split_hist_channel_count:    Indicate the number of channels in the split histogram data
* @merged_bayer_hist:           RDI merged histogram stats
* @merged_hist_channel_count:   Indicate the number of channels in the merged histogram data
* @grid_bayer_stats:            RDI grid bayer stats
* @ROI_bayer_hist:              RDI ROI bayer stats
*/
typedef struct
{
    stats_3hdr_hist_stats   split_bayer_hist;
    uint32_t                split_hist_channel_count;
    stats_3hdr_hist_stats   merged_bayer_hist;
    uint32_t                merged_hist_channel_count;
    bg_stats_type           grid_bayer_stats;
    stats_3hdr_hist_stats   ROI_bayer_hist;
} stats_hdr3_hist_data_type;

/** Defines the 3HDR statistic  data structure
*
* @HDR3ExposureTypeUnknown: Invalid 3ExposureHDR type
* @HDR3ExposureType1: 3ExposureHDR type1, with split histogram data and merged histogram data
* @HDR3ExposureType2: 3ExposureHDR type2, with split histogram data, merged histogram data and thumbnail(grid) statistics data
* @HDR3ExposureType3: 3ExposureHDR type3, with split histogram data and merged histogram data and ROI related statistics data
*/
typedef enum {
  HDR3ExposureTypeUnknown = 0,
  HDR3ExposureType1,
  HDR3ExposureType2,
  HDR3ExposureType3,
} stats_hdr3_sensor_type;

/** Defines the 3HDR statistic  data structure
*
* @hdr3_hist_data:      3HDR statistic sub data structures
* @hdr3_sensor_type:    Indicate 3HDR sensor type
*/
typedef struct {
  stats_hdr3_hist_data_type hdr3_hist_data;
  stats_hdr3_sensor_type hdr3_sensor_type;
}hdr3_stats_t;

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __Q3A_3HDR_H__


