/* q3a_common_types.h
 *
 * Copyright (c) 2014-2016, 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __Q3A_COMMON_TYPES_H__
#define __Q3A_COMMON_TYPES_H__


/* ==========================================================================
                     INCLUDE FILES FOR MODULE
========================================================================== */
#include "q3a_platform.h"
#include "q3a_common_stats.h"

/* ==========================================================================
                       Preprocessor Definitions and Constants
========================================================================== */
/**
 * Defines the max number of ROIs supported.
 */
#define Q3A_CORE_MAX_ROI_COUNT (MAX_STATS_ROI_NUM)

#define GET_SESSION_ID(reserved_id) ((reserved_id >> 16) & 0xff)
#define STATS_PROC_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define STATS_PROC_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define FLOAT_TO_Q(exp, f)   ((int32_t)((f*(1<<(exp))) + ((f<0) ? -0.5 : 0.5)))

#define POINTER_OF(PARAM_ID,TABLE_PTR)    \
        (&(TABLE_PTR->entry[PARAM_ID].data))

#define GET_FIRST_PARAM_ID(TABLE_PTR)     \
        (TABLE_PTR->first_flagged_entry)

#define SET_FIRST_PARAM_ID(TABLE_PTR,PARAM_ID)     \
        TABLE_PTR->first_flagged_entry=PARAM_ID

#define GET_NEXT_PARAM_ID(CURRENT_PARAM_ID,TABLE_PTR)    \
        (TABLE_PTR->entry[CURRENT_PARAM_ID].next_flagged_entry)

#define SET_NEXT_PARAM_ID(CURRENT_PARAM_ID,TABLE_PTR,NEXT_PARAM_ID)    \
        TABLE_PTR->entry[CURRENT_PARAM_ID].next_flagged_entry=NEXT_PARAM_ID;

#define PARAM_POSITION_LINK_RESET(TABLE_PTR, CURRENT_PARAM_ID) \
do{ \
  int position = CURRENT_PARAM_ID; \
  int current, next; \
  current = GET_FIRST_PARAM_ID(TABLE_PTR); \
  if (position == current) { \
  } else if (position < current) { \
    SET_NEXT_PARAM_ID(position, TABLE_PTR, current); \
    SET_FIRST_PARAM_ID(TABLE_PTR, position); \
  } else { \
    while (position > GET_NEXT_PARAM_ID(current, TABLE_PTR)) \
      current = GET_NEXT_PARAM_ID(current, TABLE_PTR); \
    if (position != GET_NEXT_PARAM_ID(current, TABLE_PTR)) { \
      next = GET_NEXT_PARAM_ID(current, TABLE_PTR); \
      SET_NEXT_PARAM_ID(current, TABLE_PTR, position); \
      SET_NEXT_PARAM_ID(position, TABLE_PTR, next); \
    } \
  } \
}while (0);

/* ==========================================================================
                       Static Declarations
========================================================================== */


/* ==========================================================================
                       Type Declarations
========================================================================== */
#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Defines the 3A return types.
 *
 * @Q3A_CORE_RESULT_SUCCESS: Success.
 * @Q3A_CORE_RESULT_FAILED: Generic failure.
 * @Q3A_CORE_RESULT_ENOMEMORY: Memory allocation failure.
 * @Q3A_CORE_RESULT_BAD_PARAM: Bad parameter.
 */
typedef enum
{
  Q3A_CORE_RESULT_SUCCESS = 0,
  Q3A_CORE_RESULT_FAILED,
  Q3A_CORE_RESULT_ENOMEMORY,
  Q3A_CORE_RESULT_BAD_PARAM,
  Q3A_CORE_RESULT_LAST
} q3a_core_result_type;

/**
* Defines the camera operation mode types.
*
* @Q3A_CORE_OPERATIONAL_MODE_PREVIEW: Preview mode.
* @Q3A_CORE_OPERATIONAL_MODE_CAMCORDER: Camcorder mode.
* @Q3A_CORE_OPERATIONAL_MODE_SNAPSHOT: snapshot mode.
*/
typedef enum {
  Q3A_CORE_OPERATIONAL_MODE_PREVIEW,
  Q3A_CORE_OPERATIONAL_MODE_CAMCORDER,
  Q3A_CORE_OPERATIONAL_MODE_SNAPSHOT,
  Q3A_CORE_OPERATIONAL_MODE_LAST
} q3a_core_operational_mode_type;

/**
 * Defines the types of ROIs supported by the core algorithm.
 *
 * @Q3A_CORE_ROI_TYPE_FACE: Face ROI.
 * @Q3A_CORE_ROI_TYPE_TOUCH: Touch ROI.
 * @Q3A_CORE_ROI_TYPE_FIXED: Fixed ROI.
 */
typedef enum
{
  Q3A_CORE_ROI_TYPE_FACE = 0,
  Q3A_CORE_ROI_TYPE_TOUCH,
  Q3A_CORE_ROI_TYPE_FIXED,
  Q3A_CORE_ROI_TYPE_LAST

} q3a_core_roi_type;

/**
 * Defines the dimensions of an ROI in terms of rectangle
 * coordinate
 *
 * @x: X co-ordinate of left top of ROI.
 * @y: Y co-ordicate of left top of ROI.
 * @dx: width.
 * @dy: height.
 * @weight: weight factor for each roi.
 */
typedef struct
{
  float x;
  float y;
  float dx;
  float dy;
  float weight;

} q3a_core_roi_dimension_type;

/**
 * Defines the format of the ROI configuration.
 *
 * @roi: List of ROIs and their dimensions.
 * @roi_count: Number of elements in the list.
 */
typedef struct
{
  q3a_core_roi_dimension_type     roi[Q3A_CORE_MAX_ROI_COUNT];
  uint8                           roi_count;

} q3a_core_roi_configuration_type;

/** q3a_led_flash_state_t
 *
 *  This enumeration represents the state of the flash.
 **/
typedef enum {
  Q3A_LED_OFF,
  Q3A_LED_LOW,
  Q3A_LED_HIGH,
  Q3A_LED_INIT,
  Q3A_LED_RELEASE
} q3a_led_flash_state_t;

/** q3a_led_flash_mode_t
 *
 *  This enumeration represents different modes of the flash.
 **/
typedef enum {
  LED_MODE_OFF,
  LED_MODE_AUTO,
  LED_MODE_ON,
  LED_MODE_TORCH,
  LED_MODE_SINGLE,
  /* New modes should be added above this line */
  LED_MODE_MAX
} q3a_led_flash_mode_t;


typedef enum {
  Q3A_FAST_AEC_STATE_IDLE = 0,
  Q3A_FAST_AEC_STATE_AEC_RUNNING,
  Q3A_FAST_AEC_STATE_AWB_RUNNING,
  Q3A_FAST_AEC_STATE_DONE,
  Q3A_FAST_AEC_STATE_MAX
} q3a_fast_aec_state_t;

typedef enum {
  Q3A_CAPTURE_NORMAL,
  Q3A_CAPTURE_FLASH,
  Q3A_CAPTURE_BRACKETING,
  Q3A_CAPTURE_LOW_LIGHT,
  Q3A_CAPTURE_RESET,
  Q3A_CAPTURE_MANUAL_3A,
} q3a_capture_type_t;

/**
 * This structure defines the sensitivity values related to flash
 *
 **/
 typedef struct
{
  float off;  /**< Sensitivity value when the flash is off */
  float low;  /**< Sensitivity value when the flash is low */
  float high; /**< Sensitivity value when the flash is high */
} q3a_flash_sensitivity_type;

/** q3a_stream_crop_t This structure is used to store the scale
 *  and crop information that is given by the ISP and CPP. The
 *  info is then used by the library to calculate the real ROI
 *  in terms of sensor resolution dimensions.
 **/
 typedef struct
{
  uint32   vfe_map_x; /**< Left coordinate of the map */
  uint32   vfe_map_y; /**< Top coordinate of the map */
  uint32   vfe_map_width; /**< The width of the map */
  uint32   vfe_map_height; /**< The height of the map */
  uint32   pp_x; /**< Left coordinate of the crop */
  uint32   pp_y; /**< Top coordinate of the crop */
  uint32   pp_crop_out_x; /**< Width of the crop window */
  uint32   pp_crop_out_y; /**< Height of the crop window */
  uint32   vfe_out_width; /**< The width on which the remapping is calculated */
  uint32   vfe_out_height; /**< The height on which the remapping is calculated */
} q3a_stream_crop_t;

/**
 * Struct containing SY data
 */
typedef struct {
  boolean   is_valid;
  uint32    SY[MAX_YUV_STATS_NUM];
} q3a_SY_data_t;

/**
 * For algorithms handling multiple sensor types simultaneously.
 */
typedef enum {
  Q3A_SENSOR_BAYER,       /**< Usually default Bayer sensor, with wide angle lens*/
  Q3A_SENSOR_MONO,        /**< Monochromatic sensor */
  Q3A_SENSOR_YUV,         /**< YUV sensor */
  Q3A_SENSOR_MAX          /**< Max enum value */
} q3a_sensor_type_t;

/**
 * For algorithms handling multiple lens types simultaneously.
 */
typedef enum {
  Q3A_LENS_NORMAL,        /**< Usually default Bayer sensor, with wide angle lens*/
  Q3A_LENS_WIDE,          /**< Bayer sensor, with tele lens */
  Q3A_LENS_TELE,          /**< Monochromatic sensor */
  Q3A_LENS_MAX          /**< Max enum value */
} q3a_lens_type_t;

/**
 * For algorithms handling the syncronization of multi camera use case
 */
typedef enum {
  Q3A_STAND_ALONE_CAM,  /**< Algo is not handling any syncronization */
  Q3A_MAIN_CAM,         /**< This camera is usually the one seen on the preview */
  Q3A_SECONDARY_CAM,    /**< This camera should sync to main camera */
  Q3A_SYNC_TYPE_MAX     /**< Max enum value */
} q3a_sync_mode_t;

/* Enum to define different low performance modes in dual camera*/
typedef enum {
  Q3A_PERF_NONE,
  Q3A_PERF_SENSOR_SUSPEND,
  Q3A_PERF_ISPIF_FRAME_DROP,
  Q3A_PERF_ISPIF_FRAME_SKIP,
  Q3A_PERF_STATS_FPS_CONTROL,
} q3a_dual_camera_perf_mode_t;

/**
 * For algorithms handling power savings control.
 */
typedef struct {
  uint8_t enable;
  q3a_dual_camera_perf_mode_t perf_mode;
  uint8_t priority;
} q3a_low_power_mode_t;

/**
 * Common structure used to propagate opaque data from OEM
 *
 */
typedef struct
{
  uint32  size; /**< Size allocated at @data */
  void    *data; /**< Opaque data allocated */
} q3a_custom_data_t;

typedef struct {
  boolean enable;
  uint16_t num_frames;
  q3a_fast_aec_state_t state;
} q3a_fast_aec_data_t;

/**
 * This structure defines the parameters for AE scan test config
 */
typedef struct {
  int test_config;
  int test_config_freq;
} q3a_ae_scan_test_config_t;

/**
 * Enumerates the EXIF debug masks for the various modules.
 *
 * @EXIF_DEBUG_MASK_AEC: exif mask for AEC.
 * @EXIF_DEBUG_MASK_AWB: exif mask for AWB.
 * @EXIF_DEBUG_MASK_AF: exif mask for AF.
 * @EXIF_DEBUG_MASK_ASD: exif mask for ASD.
 * @EXIF_DEBUG_MASK_AFD: exif mask for AFD.
 * @EXIF_DEBUG_MASK_STATS: exif mask for STATS.
 * @EXIF_DEBUG_MASK_BHIST: exif mask for BHIST stats.
 * @EXIF_DEBUG_MASK_BE: exif mask for Bayer Exposure Stats.
 */
typedef enum
{
  EXIF_DEBUG_MASK_AEC   = (0x10000 << 0),
  EXIF_DEBUG_MASK_AWB   = (0x10000 << 1),
  EXIF_DEBUG_MASK_AF    = (0x10000 << 2),
  EXIF_DEBUG_MASK_ASD   = (0x10000 << 3),
  EXIF_DEBUG_MASK_AFD   = (0x10000 << 4),
  EXIF_DEBUG_MASK_STATS = (0x10000 << 5),
  EXIF_DEBUG_MASK_BHIST = (0x10000 << 6),
  EXIF_DEBUG_MASK_BE    = (0x10000 << 7)
} exif_debug_mask_type;

/**
 * Enumerates the Debug Data log levels.
 *
 */
typedef enum
{
  Q3A_DEBUG_DATA_LEVEL_CONCISE = (0x00001 << 0),
  Q3A_DEBUG_DATA_LEVEL_VERBOSE = (0x00001 << 1),
} q3a_debug_data_level_type;


/** stats_type_t
 *
 *  This enumeration defines the types of the stats coming from the ISP
 *  Entries beginning with STATS_B are for the bayer stats, the rest are for
 *  the legacy YUV stats.
 */
typedef enum {
  /* Bayer stats */
  STATS_BE     = (1<<0),
  STATS_BG     = (1<<1),
  STATS_BF     = (1<<2),
  STATS_BHISTO = (1<<3),
  STATS_BF_SCL = (1<<4),
  STATS_HDR_BE = (1<<5),
  STATS_BG_AEC = (1<<6),
  STATS_HBHISTO = (1<<7),

  /* YUV stats */
  STATS_AEC    = (1<<8),
  STATS_AWB    = (1<<9),
  STATS_AF     = (1<<10),
  STATS_CS     = (1<<11),
  STATS_RS     = (1<<12),
  STATS_IHISTO = (1<<13),
  STATS_HDR_VID = (1<<14),

  /* 3EXP STATS*/
  STATS_3HDR   = (1<<15),
} stats_type_t;

/**
 * Stats streaming mode from ISP - online or offline.
 */
typedef enum
{
  Q3A_STATS_STREAM_ONLINE,
  Q3A_STATS_STREAM_OFFLINE,
  Q3A_STATS_STREAM_MAX
} q3a_stats_stream_type;

/**
 * Stats region skip pattern index.
 *
 * A skip pattern is applied to 16 (4x4) regions
 * organized as a block, and the pattern is repeated
 * throughout the entire ROI. Each block's skip
 * pattern is represented with a 16bits mask.
 *
 * For example, SKIP_PATTERN_1 corresponds to a pattern
 * with bit mask 0xA5A5:
 *
 * 1 0 1 0 (A)
 * 0 1 0 1 (5)
 * 1 0 1 0 (A)
 * 0 1 0 1 (5)
 *
 * So for this 4x4 regions block, a region position
 * with bit 1 meaning this region is sampled, while
 * a region position with bit 0 meaning this region
 * stats is skipped. A pattern of 0xFFFF meaning all
 * 16 regions are sampled, ie, no region is skipped.
 */
typedef enum
{
  SKIP_PATTERN_0,  /* 0xFFFF */
  SKIP_PATTERN_1,  /* 0xA5A5 */
  SKIP_PATTERN_2,  /* 0xAAAA */
  SKIP_PATTERN_3,  /* 0xA5A1 */
  SKIP_PATTERN_4,  /* 0xA149 */
  SKIP_PATTERN_5,  /* 0x41A1 */
  SKIP_PATTERN_6,  /* 0x1A0A */
  SKIP_PATTERN_7,  /* 0x8421 */
  SKIP_PATTERN_8,  /* 0xA0A0 */
  SKIP_PATTERN_9,  /* 0x8204 */
  SKIP_PATTERN_10, /* 0x1820 */
  SKIP_PATTERN_11, /* 0x8020 */
  SKIP_PATTERN_12, /* 0x4020 */
  SKIP_PATTERN_13, /* 0x8000 */
  SKIP_PATTERN_MAX /* invalid pattern num */
} q3a_rgn_skip_pattern;

/* enum to select AEC convergence type */
typedef enum
{
  /* Normal AEC connvergence */
  AEC_CONVERGENCE_NORMAL = 0,

  /* Aggressive AEC connvergence */
  AEC_CONVERGENCE_AGGRESSIVE,

  /* Fast AEC convergence */
  AEC_CONVERGENCE_FAST,

  AEC_CONVERGENCE_MAX,
} aec_convergence_type;


#ifdef __cplusplus
} // extern "C"
#endif

#endif // __Q3A_COMMON_TYPES_H__

