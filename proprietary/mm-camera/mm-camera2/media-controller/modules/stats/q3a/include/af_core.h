 /*af_core.h
 *                                                                   .
 * Copyright (c) 2014-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __AF_CORE_H__
#define __AF_CORE_H__

/*===========================================================================

    AF Core D a t a  S t r u c t u r e  D e c l a r a t i o n

*//** @file af_core.h
    @brief Function prototypes that are part of the interface to the core
    AF algorithm.

    This contains the prototypes that are part of the interface to the
    core AF algorithm and any macros, constants, and global variables
    that you will need. The methods exposed by the interface are NOT
    thread safe. Thread safety is the responsiblity of the user of the
    interface.

===========================================================================*/

/* ==========================================================================
                        INCLUDE FILES FOR MODULE
========================================================================== */
// 3A common include files.
#include "q3a_common_types.h"

// Tuning data includes.
#include "stats_chromatix_wrapper.h"
#include "chromatix_common.h"
#include "pdaf_lib.h"

/* ==========================================================================
                    Preprocessor Definitions and Constants
========================================================================== */
/* TBD: Probably need to define these somewhere else - probably in tuning header?*/
#define AF_MAX_NUM_OF_GRIDS       (18 * 14)           /**< Maximum number of grids supported in each ROI */
#define AF_MAX_NUM_OF_KERNELS     (3)                 /**< Maximum number of AF kernels supported by HW */
#define AF_MAX_NUM_OF_ZONES       (2)                 /**< Maximum number of zones that can be configured. Each
                                                                                                       zone can have multiple focus windows */
#define AF_MAX_NUM_OF_WINDOWS     (255)               /**< Maximum number of indepndently configurable windows */
#define AF_MAX_ROI_COUNT          (Q3A_CORE_MAX_ROI_COUNT) /**< Maximum Region of Interests(RoI) that can be passed
                                                                                                       to core algorithm */
#define AF_MAX_Y_CHANNEL          (3)                 /**< Maximum number of Y channels */
#define MAX_YUV_STATS_NUM         256                 /**< Defines the maximun size of the Legacy stats move to commmon */
#define AF_COLLECTION_POINTS      (50)                /**< Define the maximum af history size */
#define MAX_AF_STATS_DATA_SIZE    1000                /**< Define Debug data array size */
#define MAX_SWAF_COEFFA_NUM       (6)                 /**< Define IIR CoeffcientA max size */
#define MAX_SWAF_COEFFB_NUM       (6)                 /**< Define IIR CoeffcientB max size */
#define MAX_SWAF_COEFFFIR_NUM     (11)                /**< Define FIR Coeffcient max size */
#define MAX_DEPTH_WINDOW_SIZE     (200)               /**< Define Maximun PDAF size */
#define MAX_PERIPHERAL_WINDOWS    (8)               /** < Maximum number of peripheral windows */
#define AF_CORE_BF_FW_MAX_ROI_REGIONS  (281)          /**< Maximum number of supported focus regions. */
#define AF_CORE_MAX_BF_GAMMA_ENTRIES (32)             /**< Maximum number of Bayer Focus Gamma entries. */
#define AF_CORE_MAX_BF_FILTER_CORING_SIZE (17)        /**< Mximum size of the Bayer Focus Coring filter. */
#define AF_CORE_MAX_HPF_BUFF_SIZE (MAX_HPF_2x11_BUFF_SIZE) /**< Maximum size of HPF buffer. */
#define AF_GYRO_MAX_SAMPLE_SIZE   (32)



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
 * Typedef af core handle.
 */
typedef void* af_core_handle_type;

/**********************************************************
  Enum definitions
 **********************************************************/

/**
* List of AF stats kernel Config.
*/
typedef enum
{
    AF_CORE_STATS_GRID_CFG = 1,              /**< Grid type enum */
    AF_CORE_STATS_FW_CFG                     /**< FW type enum. */
} af_core_bayer_stats_cfg_type;

/**
* List of AF stats kernel type.
*/
typedef enum {
  AF_CORE_BF_KERNEL_TYPE_H_1,           /**< H1 type enum */
  AF_CORE_BF_KERNEL_TYPE_H_2,           /**< H2 type enum */
  AF_CORE_BF_KERNEL_TYPE_V,             /**< V type enum */
  AF_CORE_BF_KERNEL_TYPE_MAX,
} af_core_bf_kernel_type_t;

/**
* List of AF stats source mask currently updated.
*/
typedef enum
{
    AF_CORE_STATS_MASK_BAYER = 1,                   /**< Mask to indicate bayer hw stats are updated. */
    AF_CORE_STATS_MASK_PREVIEW_ASSISTED = (1 << 1), /**< Mask to indicate preview stats are updated. */
    AF_CORE_STATS_MASK_DEPTH_ASSISTED = (1 << 2),   /**< Mask to indicate depth information is updated. */
} af_core_stats_mask_type;

/**
 * List of AF modes supported by core algorithm.
 */
typedef enum
{
    AF_CORE_TRIGGER_SEARCH_OFF = 0,                    /**< Auto focus trigger off; current search needs to be cancelled. */
    AF_CORE_TRIGGER_SEARCH_CONTINUOUS_SCAN_CAMERA,     /**< Trigger/Start continuous auto focus during camera preview.*/
    AF_CORE_TRIGGER_SEARCH_CONTINUOUS_SCAN_CAMCORDER,  /**< Trigger/Start continuous auto-focus during camcorder preview and recording. */
    AF_CORE_TRIGGER_SEARCH_SINGLE_SCAN_CAMERA,         /**< Trigger/Start single search based on user input during camera mode. */
    AF_CORE_TRIGGER_SEARCH_SINGLE_SCAN_CAMCORDER,      /**< Trigger/Start single search based on user input during camcorder mode. */
} af_core_search_trigger_type;

/**
 * List of AF status reported by core algorithm.
 */
typedef enum
{
    AF_CORE_STATUS_INACTIVE = 0,                        /**< Default AF status. */
    AF_CORE_STATUS_SCENE_CHANGED,                       /**< In Continuous autofocus mode, this status means scene change
                                                           has been detected but search has not been started yet. */
    AF_CORE_STATUS_SCANNING,                            /**< Search is initiated */
    AF_CORE_STATUS_FOCUSED,                             /**< Search is complete successfully; object is expected to be in focus */
    AF_CORE_STATUS_NOT_FOCUSED,                         /**< Search complete with failure; object may or may not be in focus. */
} af_core_status_type;

/**
 * Type of AF filter supported by current HW stats engine.
 */
typedef enum
{
    AF_CORE_FILTER_FIR = 1,                                 /**< Mask to indicate AF filter is of type FIR */
    AF_CORE_FILTER_IIR = (1 << 1),                          /**< Mask to indicate AF filter is of type IIR */
} af_core_filter_type;

/**
* Enum to differentiate different type of roi system that can
* be configured - grid and floating windows. Grid system has
* single roi which can be divided into multiple grids. Floating
* window has multiple independently placed windows that form
* one focus zone.
*/
typedef enum
{
    AF_CORE_ROI_SYSTEM_GRID = 0,                        /**< Roi configuration supported is of grid type. */
    AF_CORE_ROI_SYSTEM_FLOATING,                        /**< Roi configuration supported is of floating windows type. */
} af_core_roi_system_type;

/**
 * List of AF Region of Interest (RoI) currently supported by
 * core algorithm.
 */
typedef enum
{
    AF_CORE_ROI_TYPE_FIXED = 0,                         /**< Default roi type - fixed central roi of tunable size. */
    AF_CORE_ROI_TYPE_TOUCH,                             /**< User specified roi from upper layer. */
    AF_CORE_ROI_TYPE_FACE,                              /**< Roi from face detection module. */
} af_core_roi_type;

/**
 * Enum for bayer channels.
 */
typedef enum
{
    AF_CORE_BAYER_CHANNEL_G = 0,                        /**< Bayer channel Y*/
    AF_CORE_BAYER_CHANNEL_Y,                            /**< Bayer channel green */
    AF_CORE_BAYER_CHANNEL_MAX,
} af_core_bayer_channel_type;

/**
 * Enum for bayer G channels.
 */
typedef enum
{
    AF_CORE_BAYER_G_CHANNEL_GR = 0,                       /**< Bayer channel green-red */
    AF_CORE_BAYER_G_CHANNEL_GB,                           /**< Bayer channel green-blue */
    AF_CORE_BAYER_G_CHANNEL_MAX,
} af_core_bayer_g_channel_type;


/**
 * Bit masks to indicate updated parameters from core algorithm.
 */
typedef enum
{
    AF_CORE_OUTPUT_MOVE_LENS      = 1,                  /**< Mask for lens move request */
    AF_CORE_OUTPUT_CONFIG_STATS   = (1 << 1),           /**< Mask for stats engine configuration request */
    AF_CORE_OUTPUT_OPAQUE_DATA = (1 << 2),              /**< Mask to indicate opaque data update */
    AF_CORE_OUTPUT_FOCUS_STATUS = (1 << 3),             /**< Mask to indicate focus status update */
    AF_CORE_OUTPUT_EZ_METADATA = (1 << 4),              /**< Mask to indicate eztune metadata update */
    AF_CORE_OUTPUT_MOBICAT_METADATA = (1 << 5),         /**< Mask to indicate mobicat metadata update */
    AF_CORE_OUTPUT_RESET_AEC       = (1 << 6),          /**< Mask to indicate Reset AEC update */
    AF_CORE_OUTPUT_DEBUG_DATA   = (1 << 7),             /**< Mask to indicate debug data update */
    AF_CORE_OUTPUT_SPOT_LIGHT_DETECTION = (1 << 8),     /**< Mask to indicate spot light detect update */
    AF_CORE_OUTPUT_FOCUS_VALUE = (1 << 9),              /**< Mask to indicate focus value for every frame */
    AF_CORE_OUTPUT_EXP_COMPENSATE  = (1 << 10),         /**< Mask to indicate exposure compensate request */

} af_core_output_update_mask_type;

/**
* Enum for lens movement direction.
*/
typedef enum
{
    AF_CORE_MOVE_LENS_NEAR = 0,         /**< Move the lens towards near object. */
    AF_CORE_MOVE_LENS_FAR,              /**< Move the lens towards far object */
} af_core_lens_move_direction_type;

typedef enum af_core_fw_region_type_t {
   AF_CORE_BF_FW_PRIMARY_REGION,
   AF_CORE_BF_FW_SECONDARY_REGION,
   AF_CORE_BF_FW_MAX_REGION,
} af_core_fw_region_type_t;

typedef struct _af_core_bf_fw_roi_dim_t {
   af_core_fw_region_type_t region_type;
   uint32_t                x;
   uint32_t                y;
   uint32_t                w;
   uint32_t                h;
   boolean                 is_valid;
} af_core_bf_fw_roi_dim_t;

/**
 * List of focus modes supported.
 *
 **/
typedef enum
{
    AF_MODE_NORMAL,        /**< Normal AF mode - Infy to boundary */
    AF_MODE_MACRO,         /**< Macro mode - search range Macro to Infy */
    AF_MODE_AUTO,          /**< Currently same as Macro mode */
    AF_MODE_CAF,           /**< Continuous AF mode - default */
    AF_MODE_CAF_NORMAL,    /**< CAF with Normal mode - Infy to boundary */
    AF_MODE_CAF_MACRO,     /**< CAF with Macro - Macro to Infy search range */
    AF_MODE_INFINITY,      /**< Lens is fixed at Infinity */
    AF_MODE_MANUAL,        /**< Lens is controled by HAL */
    AF_MODE_NOT_SUPPORTED, /**< The mode is not supported */
    AF_MODE_MANUAL_HAL1,        /* Lens is controled by HAL */
    AF_MODE_MAX            /**< The size of the enum */
} af_mode_type;

/**
 *
 * Enum to distinguish if it's camcorder or camera mode.
 *
 **/
typedef enum
{
    AF_RUN_MODE_INIT,        /**< Stream Type Init */
    AF_RUN_MODE_CAMERA,      /**< Stream Type Preview */
    AF_RUN_MODE_VIDEO,       /**< Stream Type Camcorder */
    AF_RUN_MODE_SNAPSHOT     /**< Stream Type Snapshot */
} af_run_mode_type;

/**
 * Different fullsweep modes.
 **/
typedef enum
{
    AF_FULLSWEEP_SRCH_MODE_OFF,              /**< FullSweep mode is off */
    AF_FULLSWEEP_SRCH_MODE_INFY_TO_NEAR,     /**< FullSweep mode is far to near end. */
    AF_FULLSWEEP_SRCH_MODE_NEAR_TO_INFY,     /**< FullSweep mode is near to far end */
    AF_FULLSWEEP_SRCH_MODE_BOTH,             /**< Fullsweep mode is dual direction */
} af_fullsweep_srch_mode_type;
/**
 * Enum definition for Depth Assisted AF (DAAF).
 */
typedef enum
{
    AF_DAAF_NONE = 0,
    AF_DAAF_PDAF = (1<<0),           /**< DAAF technology is PDAF */
    AF_DAAF_TOF  = (1<<1),           /**< DAAF technology is TOF */
    AF_DAAF_DCIAF = (1<<2),          /**< DAAF technology is DCIAF */
    AF_DAAF_MAX
} af_core_daaf_type;


typedef enum
{
   AF_CORE_BF_SINGLE, /* Single BF grid stats */
   AF_CORE_BF_SCALE,  /* Single BF grid stats + BF scale grid stats */
   AF_CORE_BF_FW, /* Floating window BF stats */
} af_core_bayer_stats_type;

typedef enum
{
   AF_CORE_FINE_SEARCH_INACTIVE,
   AF_CORE_FINE_SEARCH_INPROGRESS,
   AF_CORE_FINE_SEARCH_ABORT,
} af_core_fine_search_cmd_type;

/**********************************************************
  Structure definitions
 **********************************************************/
/* TBD - can be moved to common parameter header? */
/**
* This structure defines the width and height of an object in two-dimensional
* space.
*/
typedef struct
{
    uint32         width;                             /**< width of an object. */
    uint32         height;                            /**< height of an object. */
} af_core_size_type;

typedef struct _af_lens_actuator_info_t {
  float     focal_length;                           /**< Focal Length */
  float     actuator_sensitivity;                   /**< Actuator Senistivity*/
  float     af_f_num;
  float     af_f_pix;
} af_lens_actuator_info_t;

typedef struct _af_lens_params{
  boolean                                   isSlopeComputationRequired;          /**< Slope computation already done or not */
  float                                     sensitivity_um_per_step_index;       /**< Sensitivity which has been precomputed*/
  float                                     hypf_dist_mm;                        /**< Hyperfocal distance*/
  float                                     lens_pos_20cm;                       /**< Lens Position  for 20 cm*/
  float                                     lens_pos_hypf;                       /**< Hyperfocal Len s position*/
  af_lens_actuator_info_t                   lens_info;                           /**< Lens Specific Information*/
}af_lens_params;

typedef struct _af_lens_anchor_info{
  af_single_index_adapter_t                       anchor_pos1;
  af_single_index_adapter_t                       anchor_pos2;
  af_single_index_adapter_t                       minimum_focus_index;
  unsigned short                                  lens_mapping[SINGLE_MAX_IDX];
}af_lens_anchor_info;


typedef struct {
  boolean is_pdaf_supported;
  float af_cal_inf_ff;
  float af_cal_inf_fd;
  float af_cal_inf_fu;
  float eff_focal_length;
  float f_num;
  float pixel_size;
  float actuator_sensitivity;
} af_core_cal_data_t;

/**
 * Data structure definition for Depth Assisted AF (DAAF).
 */
typedef struct
{
    uint64        frame_id;                         /**< current frame-id */
    uint32_t      h_num;                            /**< Current horizontal num */
    uint32_t      v_num;                            /**< Current vertical num */
    int32_t       af_focus_pos_dac;                   /**< Lens position in DAC*/
    boolean       status;                            /**< Valid - True or False */
    af_core_cal_data_t cal_data;
} af_core_stats_daaf_info;

typedef struct
{
     int32_t defocus;                                /**< Defocus value */
     uint32_t df_conf_level;                         /**< Confidence level */
     float phase_diff;                               /**< Raw phase difference */
} af_core_pdaf_info_t;

typedef struct
{
  af_core_stats_daaf_info   daaf_info;
  af_core_pdaf_info_t       pdaf_update[MAX_DEPTH_WINDOW_SIZE]; /**< Pdaf data for center windows */
  boolean                   is_peripheral_valid;                /**< Indicate if peripheral pdaf data are valid */
  af_core_pdaf_info_t       peripheral[MAX_PERIPHERAL_WINDOWS];  /**< PDAF data for peripheral windows */
  boolean                   is_single_valid;                /**< Indicate if single pdaf data are valid */
  af_core_pdaf_info_t       single[MAX_SINGLE_WINDOWS];;  /**< PDAF data for single window */
} af_core_pdaf_update_t;

/**
 * Details of the region of interest to focus on.
 */
typedef struct {
    float          x;                          /**< X-coordinate of the top,left of the roi */
    float          y;                          /**< Y-coordinate of the top,left of the roi */
    float          width;                      /**< Width of the roi */
    float          height;                     /**< Height of the roi */
} af_core_roi_info_type;

/**
 * Data structure definition for Dual Camera Assisted AF
 * (DCIAF).
 */
typedef struct {
  af_core_stats_daaf_info      daaf_info;
  uint32_t id;                   /**< frame id */
  long long timestamp;           /**< Time stamp from DCRF library*/
  int32_t  distance_in_mm;       /**< distance from DCRF library*/
  int32_t  confidence;           /**< Confidence level from library*/
  uint32_t status;               /**< Success or failure*/
  af_core_roi_info_type focused_roi;        /**< Roi  used by DCRF library*/
  uint32_t focused_x;            /**< x coordinate of roi that is focused.*/
  uint32_t focused_y;            /**< Y coordinate of roi that is focused.*/
} af_core_dciaf_update_t;

/** af_core_tof_update_t:
*
*  @frame_id : current frame id, can be optional since it relies on timestamp
*  @timestamp : timestamp of arrival of the laser data
*  @valid_data: flag to indicate if tof data is valid or not
*  @range_mm: flag to indicate laser data is valid or not
*  @signalRate_mcps: signal rate (MCPS)\n these is a 9.7 fix point value,
*   which is effectively a measure of target reflectance.
*  @errorStatus: Error status of the current measurement. \n
*  @DMaxSq: DMax²  when applicable
*  @rtnRate: Return signal rate  related to RESULT_RANGE_RETURN_SIGNAL_COUNT
*  @refRate: Reference signal signal rate related to
*   RESULT_RANGE_REFERENCE_SIGNAL_COUNT
*  @rtnAmbRate: Return Ambient rate related to RESULT_RANGE_RETURN_AMB_COUNT
*  @refAmbRate: Reference Ambient rate related to
*   RESULT_RANGE_REFERENCE_SIGNAL_COUNT
*  @rtnConvTime: Return Convergence time  RESULT_RANGE_RETURN_CONV_TIME
*  @refConvTime: Reference convergence time RESULT_RANGE_REFERENCE_CONV_TIME
*
**/
typedef struct {
  af_core_stats_daaf_info   daaf_info;
  uint32                    frame_id;
  int64                     timestamp;
  int32                     distance;
  int32                     confidence;
  int32                     near_limit;
  int32                     far_limit;
  int32                     max_distance;
} af_core_tof_update_t;

/**
 * Data used for enabling/disabling HAF
 **/
typedef struct{
  boolean enable;                          /**< Top level switch to disable/enable HAF*/
  boolean algo_enable[AF_HAF_ALGO_MAX];    /**< Individual level switch to enable/disable */
} af_haf_enable_type;


/**
 * Data structure definition for Preview Assisted AF (PAAF).
 * This is software stats generated afte processing preview
 * frame.
 */
typedef struct
{
    uint64         frame_id;                          /**< frame-id of the preview frame from which preview stats
                                                                                                       are calculated. */
    uint32         fv;                                /**< list of focus values */
    uint8          grid_count;                        /**< number of grids in roi */
} af_core_stats_paaf_type;

typedef struct {
  uint8       horizontal_regions;                     /**< Number of horizontal regions  */
  uint8       vertical_regions;                       /**< Number of vertical regions  */
  uint8       kernel_count;                           /**< Number of AF kernel current HW supports */
  uint8       use_max_fv[AF_MAX_NUM_OF_KERNELS];      /**< Max FV value or sum of FV value of each grid/window */
  uint32*     red_sum[AF_MAX_NUM_OF_KERNELS];         /**< Sum of valid red pixel values for each grid/window for each kernel. */
  uint32*     blue_sum[AF_MAX_NUM_OF_KERNELS];        /**< Sum of valid blue pixel values for each grid/window for each kernel. */
  uint32*     gr_sum[AF_MAX_NUM_OF_KERNELS];          /**< Sum of valid green red pixel values for each grid/window for each kernel. */
  uint32*     gb_sum[AF_MAX_NUM_OF_KERNELS];          /**< Sum of valid green blue pixel values for each grid/window for each kernel. */
  uint64*     red_sharpness[AF_MAX_NUM_OF_KERNELS];   /**< List of red channel focus values for each AF kernel. */
  uint64*     blue_sharpness[AF_MAX_NUM_OF_KERNELS];  /**< List of blue channel focus values for each AF kernel. */
  uint64*     gr_sharpness[AF_MAX_NUM_OF_KERNELS];    /**< List of green red channel focus values for each AF kernel. */
  uint64*     gb_sharpness[AF_MAX_NUM_OF_KERNELS];    /**< List of green blue channel focus values for each AF kernel. */
  uint32*     red_count[AF_MAX_NUM_OF_KERNELS];       /**< Number of red channel valid pixels for each grid/window for each kernel. */
  uint32*     blue_count[AF_MAX_NUM_OF_KERNELS];      /**< Number of blue channel valid pixels for each grid/window for each kernel. */
  uint32*     gr_count[AF_MAX_NUM_OF_KERNELS];        /**< Number of green red channel valid pixels for each grid/window for each kernel. */
  uint32*     gb_count[AF_MAX_NUM_OF_KERNELS];        /**< Number of green blue channel valid pixels for each grid/window for each kernel. */
  uint64*     red_max_fv[AF_MAX_NUM_OF_KERNELS];      /**< List of red channel max focus values for each AF kernel. */
  uint64*     blue_max_fv[AF_MAX_NUM_OF_KERNELS];     /**< List of blue channel max focus values for each AF kernel. */
  uint64*     gr_max_fv[AF_MAX_NUM_OF_KERNELS];       /**< List of green red channel max focus values for each AF kernel. */
  uint64*     gb_max_fv[AF_MAX_NUM_OF_KERNELS];       /**< List of green blue channel max focus values for each AF kernel. */
  uint32      array_size;
} af_core_stats_bf_grid_t;

typedef struct {
  uint64*            bf_h1_sum;
  uint64*            bf_h2_sum;
  uint64*            bf_v_sum;
  uint64*            bf_h1_sharp;
  uint64*            bf_h2_sharp;
  uint64*            bf_v_sharp;
  uint32*            bf_h1_num;
  uint32*            bf_h2_num;
  uint32*            bf_v_num;
  af_core_bf_fw_roi_dim_t bf_fw_roi_dim[AF_CORE_BF_FW_MAX_ROI_REGIONS];
  uint32_t            num_bf_fw_roi_dim;
} af_core_stats_bf_fw_t;

/**
 * Bayer stats data received from HW stats engine. Depedning
 * upon current HW capability, there will be multiple AF
 * kernels, multiple zones with different interrupts and
 * number of focus windows/grids.
 */
typedef struct
{
  af_core_bayer_stats_type bf_type;
  boolean     isFake;                                 /**< Real or Fake stats*/
  uint64      frame_id;                               /**< frame-id associated with current stats */
  uint8       irq_id;                                 /**< interrup id of current stats.Useful when multiple
                                                          AF interrupts are supported. */
  union {
    af_core_stats_bf_fw_t fw_stats;
    af_core_stats_bf_grid_t grid_stats;
  } u;
  /* TBD: Now not sure if we really need this here. I think we should separate the ROI configured as another
     parameter in af_input_params. */
} af_core_stats_bayer_type;

typedef struct
{
  uint32 /*af_core_daaf_type*/               mask;           /**< Bit mask to indicate which AF stats are updated. */
  af_core_pdaf_update_t                    pdaf_info;/**< Pdaf info */
  af_core_dciaf_update_t                   dciaf_info; /**< Dual camera AF info */
  af_core_tof_update_t                     tof_info;
}af_core_stats_daaf_type;

/**
 * Data structures definition for AF stats. There can be
 * different sources of AF stats - HW stats, SW stats,
 * depth estimation.
 */
typedef struct
{
    uint32 /*af_core_stats_mask_type*/    mask;           /**< Bit mask to indicate which AF stats are updated. */
    af_core_stats_bayer_type            bayer_stats;    /**< Bayer AF stats from HW stats engine */
    af_core_stats_paaf_type             preview_stats;  /**< SW stats from preview frame for Preview-Assisted AF (paaf) */
    af_core_stats_daaf_type             depth_stats;    /**< Depth estimation inforamtion for Depth Assisted AF(DAAF)*/
} af_core_stats_type;

/**
* Structures defines the parameters for AF search range.
*/
typedef struct
{
    uint16  default_position;                         /**< Default position */
    uint16  near_end_position;                        /**< Near end position */
    uint16  far_end_position;                         /**< Far end position */
    uint16  hyperfocal_position;                      /**< Hyperfocal position */
} af_core_search_range_type;

/**
 * Definition for APEX values. Apex values allows easy exposure
 * value calculation: Ev = Av + Tv = Bv + Sv.
 */
typedef struct
{
    float          av;                                /**< Aperture value calculated from aperture or f-number. */
    float          bv;                                /**< Luminance or brightness value. */
    float          sv;                                /**< Speed or sensitivity value. */
    float          tv;                                /**< Time value calculated from exposure time. */
    float          ev;                                /**< Exposure value. */
} af_core_apex_type;

/**
 * AEC parameters required for AF opeartion.
 */
typedef struct
{
    uint8          frame_period;                      /**< frame period in ms. Inverse will give fps. */
    uint16         lux_index;                         /**< current lux index. */
    uint16         exp_index;                         /**< current exposure index. */
    uint16         max_exp_table_entry;               /**< maximum entry allowed in exposure table. If exposure
                                                                                                       index is equal or more than this, it means luma
                                                                                                       target cannot be reached. */
    float          cur_luma;                          /**< current luma value */
    float          comp_luma;                         /**< compensated luma value */
    float          target_luma;                       /**< target luma to reach */
    float          cur_gain;                          /**< current AEC gain. */
    float          cur_exp_time;                      /**< current exposure time. */
    int            preview_fps;                       /**< Current fps */
    float          converge_percent;                  /**< current percentage of AEC convergance. 100%
                                                                                                       convergance means AEC is settled. */
    uint8          luma_values[MAX_YUV_STATS_NUM];    /**< bayer grid luma values. */
    af_core_apex_type   apex_values;                  /**< apex values - Av, Bv, Sv, Tv, Ev. */
    uint32         pixels_per_region;                 /**< Pixel per region */
    float          total_drc_gain;
} af_core_input_aec_type;


/**
 * Updated parameters from AWB module.
 */
typedef struct
{
    uint16         decision;                          /**< awb decision */
    float          r_gain;                            /**< r channel awb gain. */
    float          b_gain;                            /**< r channel awb gain. */
    float          g_gain;                            /**< g channel awb gain. */
} af_core_input_awb_type;


/**
 * Updated parameters from ASD module.
 */
typedef struct
{
    uint8          dummy;                             /**< dummy placeholder. */
} af_core_input_asd_type;

/**
 * Gyro data from gyro sensor. It is used to detect motion
 * across all the three axes.
 */
typedef struct
{
    boolean        enabled;                           /**< flag to indicate if gyro data is enabled or not. */
    boolean        float_ready;
    uint16         sample_index;
    float          angular_velocity_x[AF_GYRO_MAX_SAMPLE_SIZE];              /**< angular velocity along x-axis */
    float          angular_velocity_y[AF_GYRO_MAX_SAMPLE_SIZE];              /**< angular velocity along y-axis */
    float          angular_velocity_z[AF_GYRO_MAX_SAMPLE_SIZE];              /**< angular velocity along z-axis */
} af_core_input_gyro_type;

/**
 * Gravity vector data from accelerometer. It is used to
 * detect camera tilt along all three axes.
 */
typedef struct
{
    boolean        enabled;                           /**< flag to indicate if gravity data is enabled or not. */
    float          gravity_vector_x;                  /**< gravity vector along x-axis*/
    float          gravity_vector_y;                  /**< gravity vector along y-axis*/
    float          gravity_vector_z;                  /**< gravity vector along z-axis*/
} af_core_input_gravity_type;

/**
 * Linear acceleration data along three axes from
 * accelerometer.
 */
typedef struct
{
    boolean        enabled;                           /**< flag to indicate if accelerometer is enabled or not. */
    float          accel_x;                           /**< linear accelerometer along x-axis*/
    float          accel_y;                           /**< linear accelerometer along y-axis*/
    float          accel_z;                           /**< linear accelerometer along z-axis*/
} af_core_input_accel_type;

/**
 * Parameter updates from motion sensors like gyro
 * and accelrometer.
 */
typedef struct
{
    af_core_input_gyro_type       gyro_info;               /**< data from gyro sensor. */
    af_core_input_gravity_type    gravity_info;            /**< gravity vector from accelrometer */
    af_core_input_accel_type      accel_info;              /**< linear acceleration from accelrometer */
} af_core_input_motion_sensor_type;

/* TBD: need to talk with system team to complete this */
/**
 * Actuator specific parameters updated by sensor module.
 */
typedef struct
{
    uint32         start_code;                        /**< starting dac value */
    uint32         calibrated_macro;                  /**< calibrated dac value for lens at macro end,
                                                           usually 10cm*/
    uint32         calibrated_infy;                   /**< calibrated dac value for lens at infinity. */
    uint32         end_limit_macro;                   /**< mechanical stop at macro end (in dac value) */
    uint32         end_limit_infy;                    /**< mechanical stop at infinity (in dac value) */
    uint32         hyperfocal_ff;                     /**< face forward hyperfocal lens position in dac */
    uint32         hyperfocal_fu;                     /**< face up hyperfocal lens position in dac */
    uint32         hyperfocal_fd;                     /**< face down hyperfocal lens position in dac */
} af_core_input_actuator_type;

/**
 * Sensor specific parameters required by core algorithm.
 */
typedef struct
{
    float          focal_length;                        /**< focal length of the sensor. */
    float          f_number;                            /**< f-number or aperture of the lens. */
    float          pixel_size;                          /**< pixel size of the sensor. */
    float          focus_distance;                      /**< max focus distance corresponds to infinity in cm. */
    uint16         max_preview_fps;                     /**< max preview FPS. */
    af_core_size_type   camif;                          /**< current camif size. */
    float          actuator_sensitivity;                 /**< sensitivity of the current actuator */
    uint32_t       frame_time;
} af_core_input_img_sensor_type;

/**
 * Details of type and coefficient size for each kernel.
 */
typedef struct
{
    uint8                       mask;                /**< mask for filter used - FIR/IIR  af_core_filter_type*/
    uint8                       num_fir_a_kernel;    /**< fir a size of kernel coefficients */
    uint8                       num_iir_a_kernel;    /**< iir a size of kernel coefficients */
    uint8                       num_iir_b_kernel;    /** < iir b size of kernel coefficients */
    boolean                     downscale_supported; /** <  downscale supported */
    boolean                     is_coring_variable;  /** <  Is coring fixed / variable */

} af_core_hw_cap_filter_kernel_type;

/**
 * This data structure details the capability of hardware to
 * support multiple kernels and type of filters.
 */
typedef struct
{
    uint8                           num_of_kernels;                    /**< number of AF kernels supported. */
    af_core_hw_cap_filter_kernel_type  kernel_info[AF_MAX_NUM_OF_KERNELS];/**< information about each kernel. */

} af_core_hw_cap_filter_type;


/**
 * Details about the configurable parameters of roi in grid
 * system.
 */
typedef struct
{
    uint8          max_h_num;                         /**< maximum number of horizontal blocks in a grid. */
    uint8          max_v_num;                         /**< maximum number of vertical blocks in a grid.*/
    uint32         max_block_width;                   /**< maximum width of each block. */
    uint32         max_block_height;                  /**< maximum height of each block. */
    uint32         min_block_width;                   /**< minimum width of each block. */
    uint32         min_block_height;                  /**< minimum height of each block. */
} af_core_hw_capability_roi_grid_type;

/**
 * This structure gives details about focus zone currently
 * supported by the hardware. Each zone has multiple
 * independently confgurable windows.
 */
typedef struct
{
    uint16         max_windows;                       /**< maximum number of windows supported in the zone. */
    uint16         max_win_width;                     /**< maximum width of each window. */
    uint16         min_win_width;                     /**< minimum width of each window. */
    uint16         max_win_height;                    /**< maximum height of each window. */
    uint16         min_win_height;                    /**< minimum height of each window. */
    uint16         min_hor_offset;                    /**< minimum horizontal offset */
    uint16         max_hor_offset;                    /**< minimum horizontal offset */
    uint16         min_ver_offset;                    /**< minimum vertical offset */
    uint16         max_ver_offset;                    /**< minimum vertical offset */
} af_core_hw_cap_roi_fw_zone_type;

/**
 * Details of the floating window roi configuration. In this
 * system there are independent floating windows that can be
 * grouped into multiple focus zones.
 */
typedef struct
{
    uint8                         num_of_zones;                         /**< number of zones supported. */
    af_core_hw_cap_roi_fw_zone_type    zone_info[AF_MAX_NUM_OF_ZONES];  /**< information about each zone. */
} af_core_hw_cap_roi_fw_type;

/**
 * This structure details the type of roi system (grid vs
 * floating windows) currently supported by HW.
 */
typedef struct
{
    af_core_roi_system_type            roi_type;            /**< roi system - grid or floating windows. */
    union
    {
        af_core_hw_capability_roi_grid_type  grid_info;     /**< details about grid system currently supported. */
        af_core_hw_cap_roi_fw_type           fw_info;       /**< details about floating window system supported. */
    } u;                                                    /**< based on roi type one data parameter will be used. */
} af_core_hw_cap_roi_type;

/**
 * This structure provides an information about current
 * capability of HW stats engine.
 */
typedef struct
{
    af_core_hw_cap_filter_type         filter_info;        /**< type and numbers of af filters currently supported. */
    af_core_hw_cap_roi_type            roi_info;           /**< type of roi system (grid vs floating windows) and
                                                                                                                                                                other roi details */
    uint32_t                           capability_mask;    /**< mask to determine grid,scale vs fw */
} af_core_hw_cap_type;

/**
 * AF RoI from external module. Only AF_ROI_TYPE_FACE will have
 * multiple entries (sorted bigger to smaller faces).
 */
typedef struct
{
    af_core_roi_info_type        roi[AF_MAX_ROI_COUNT];   /**< List of RoIs passed to core algorithm. */
    uint8                        num_of_roi;              /**< Number of RoIs currently updated */
} af_core_config_roi_cordinates_type;


/**
 * Opaque data passed by core algorithm - to be used based on
 * the specification and need.
 */
typedef struct
{
    void*                    data;                   /**< Pointer to opaque data */
    uint32                   size;                   /**< Size of data in bytes */
} af_core_opaque_data_type;

// TBD - need to confirm with system team
/**
 * Bayer noise coring information.
 */
typedef struct
{
    uint8            r_min;                             /**< noise coring threshold for r channel */
    uint8            b_min;                             /**< noise coring threshold for b channel */
    uint8            gr_min;                            /**< noise coring threshold for gr channel */
    uint8            gb_min;                            /**< noise coring threshold for gb channel */
} af_core_config_noise_coring_bayer_grid_type;

typedef struct
{
    int32           threshold;                                  /**< Coring threshold. */
    uint32          gain;                                       /**< Gain. */
    uint32          index[AF_CORE_MAX_BF_FILTER_CORING_SIZE];   /**< Coring table. */
} af_core_config_noise_coring_bayer_fw_type;


typedef struct
{
  union {
   af_core_config_noise_coring_bayer_fw_type    fw_coring_cfg;    /**< Bayer FW noiuse coring information */
   af_core_config_noise_coring_bayer_grid_type  grid_coring_cfg;  /**< Bayer Grid noiuse coring information */
  } u;
} af_core_config_noise_coring_bayer_type;


/**
 * Bayer noise coring information.
 */
typedef struct
{
    af_core_config_noise_coring_bayer_type  bayer_coring_info;  /**< Bayer noiuse coring information */
} af_core_config_noise_coring_type;

/**
 * Configuration details of FIR filter.
 */
typedef struct
{
    boolean          enabled;                  /**< Only checked for fw stats cfg */
    int8             a[AF_CORE_MAX_HPF_BUFF_SIZE];     /**< FIR filter coefficients */
    uint8            coeff_size;               /**< Filter coefficient size */
} af_core_filter_fir_type;

/**
 * Configuration details of IIR filter.
 */
typedef struct
{
    boolean          enabled;                    /**<  Only checked for fw stats cfg */
    float           a[AF_CORE_MAX_HPF_BUFF_SIZE];       /**< feedback filter coefficients */
    float           b[AF_CORE_MAX_HPF_BUFF_SIZE];       /**< feedforward filter coefficients */
    uint8            coeffa_size;                /**< coefficient size a */
    uint8            coeffb_size;                /**< coefficient size b */
} af_core_filter_iir_type;

/**
 * Grid type roi configuration details.
 */
typedef struct
{
    af_core_roi_info_type roi;                          /**< Coordinates of requested roi */
    uint8            h_num;                             /**< Number of horizontal blocks */
    uint8            v_num;                             /**< Number of vertical blocks */
} af_core_config_roi_grid_type;

/**
 * Floating Window type roi configuration request. In this
 * system  there are multiple zones that can have multiple
 * indepndent floating windows.
 */
typedef struct
{
    af_core_roi_info_type    roi;              /**< Coordinates of requested roi */
    baf_roi_pattern_enum_t   pattern;          /**< Pattern of requested roi */
    float                    grid_size_h;
    float                    grid_size_v;
    float                    grid_gap_h;
    float                    grid_gap_v;
} af_core_config_roi_fw_type;


/**
 * This structure defines type of roi system being used and
 * details of roi configuration request.
 */
typedef struct
{
    af_core_roi_system_type            type;                   /**< Type of ROI system currently supported - Grid or Floating Window */
    union {
        af_core_config_roi_grid_type   grid_info;              /**< Grid type ROI system configuration details */
        af_core_config_roi_fw_type     fw_info;                /**< Floating windows configuration details */
    } u;                                                       /**< Depending upon type only one of the follow definition is used */
} af_core_config_system_roi_type;

/**
 * Information about each AF kernel.
 */
typedef struct
{
    boolean                             is_valid;         /**< validity flag */
    boolean                             scalar_enabled;   /**< scalar enable flag */
    int32_t                             shift_bits;       /**< shift bits only for fw */
    af_core_filter_iir_type             iir;              /**< Details of IIR filter */
    af_core_filter_fir_type             fir;              /**< Details of FIR filter*/
    af_core_config_noise_coring_type    coring_info;      /**< Noise coring info */
    float                               scale_factor;     /**< Scale factor only for grid. */
} af_core_config_filter_kernel_type;


/**
 * Details Bayer AF filter configuration request.
 */
typedef struct
{
    uint8                            num_of_kernel;                        /**< Number of AF kernels to be configured */
    af_core_config_filter_kernel_type  kernel_info[AF_MAX_NUM_OF_KERNELS]; /**< Details of each AF kernel */
} af_core_config_filter_type;

/**
* This structure defines type of system ROI and user ROI configuration.
*/
typedef struct
{

    af_core_roi_type                    roi_type;                  /**< Type of user ROI */
    af_core_config_roi_cordinates_type  roi_cordinates;            /**< ROI coordinates for each ROI  */;
    boolean                             roi_updated;               /**< ROI update flag */
    uint32_t                            weight[AF_MAX_ROI_COUNT];  /**< ROI weight */
} af_core_config_roi_type;

/**
* This structure defines type of peer focus info passed between main/aux camera.
*/
typedef struct
{
    af_core_status_type                  focusStatus;                /**< Focus Status */
    af_core_fine_search_cmd_type         fineSearch_cmd;             /**< Fine Search Command */
    int                                  distanceOfObjectOfInterest; /**< Distance for Object of Interest Estimated by AF in mm */
    int                                  lensPosOfObjectOfInterest;  /**< Lens Position for Object of Interest Estimated by AF in mm */
    int                                  currentLensPos;             /**< Current Scanning Lens Position*/
    int                                  currentdistanceOfObject;    /**< Current Scanning Distance  of Object */
    float                                focal_length_ratio;         /**< Focal Length Ratio */
    float                                lens_shift_um;              /**<Lens position - actual displacement [um]. 0 when focusing at infinity, maximal when focusing on closest macro distance */
    float                                zoom_factor;                /**< Zoom factor for main camera */
    af_core_config_roi_type              af_roi;                     /**< main camera ROI */
    uint32_t                             sensor_res_width;           /**< main camera resolution width */
    uint32_t                             sensor_res_height;          /**< main camera resolution height */
    int                                  current_dist_near;
    int                                  current_dist_far;

} af_core_focus_info;

/**
* This structure defines bayer channel weights.
*/
typedef struct
{
    float       rChannel;                                   /**< Bayer red channel weight */
    float       bChannel;                                   /**< Bayer blue channel weight */
    float       grChannel;                                  /**< Bayer green red channel weight */
    float       gbChannel;                                  /**< Bayer green blue channel weight */
} af_core_bayer_channel_weight_type;

/**
* This structure defines bayer input config.
*/
typedef struct _af_core_bf_input_cfg_t {
  boolean                             is_valid;                             /**< Flag to incate input config */
  af_core_bayer_channel_type          bf_input_sel;                         /**< Bayer channel input selection */
  af_core_bayer_g_channel_type        bf_g_sel;                             /**< Bayer channel g selection */
  af_core_bayer_channel_weight_type   bayer_channel_weight;                 /**< Bayer channel weights to calculate Focus Values */
  float                               y_channel_weight[AF_MAX_Y_CHANNEL];   /**< Y channel weights to calculate Focus Values */
} af_core_bf_input_cfg_t;

/**
* This structure defines bayer gama config.
*/
typedef struct {
  boolean  is_valid;                                             /**< Flag to incate gama config */
  uint32_t gamma_lut[AF_CORE_MAX_BF_GAMMA_ENTRIES];              /**< Gama Lut value*/
  uint32_t num_gamm_lut;                                         /**< Number of Gama table value.*/
} af_core_bf_gamma_lut_cfg_t;

/**
* This structure defines bayer Scale config.
*/
typedef struct {
  boolean    is_valid;                /**< Flag to incate scale config */
  boolean    bf_scale_en;             /**< Flag to incate scale enable */
  int32_t    scale_m;                 /**< Size of Scale. */
  int32_t    scale_n;                 /**< Size of Scale. */
} af_core_bf_scale_cfg_t;

/**
 * Hardware bayer stats configuration details.
 */
typedef struct
{
  af_core_bayer_stats_cfg_type        bf_cfg_type;      /**< Stats COnfig type grid/fw.*/
  af_core_config_filter_type          filter_info;      /**< Details about the AF filter configuration request */
  af_core_config_system_roi_type      roi_info;         /**< ROI windows/grids configuration request */
  af_core_bf_input_cfg_t              bf_input_cfg;     /**< Bayer input config. */
  af_core_bf_gamma_lut_cfg_t          bf_gamma_lut_cfg; /**< Bayer gama config. */
  af_core_bf_scale_cfg_t              bf_scale_cfg;     /**< Bayer scale config. */
  af_scene_type_enum_t                scene_type;       /**< Scene type default/low/face */
} af_core_config_stats_engine_bayer_type;

/**
 * Stats configuration details for preview assisted af.
 */
typedef struct
{
    af_core_roi_info_type         roi;                     /**< Coordinates of requested roi */
    uint8                         h_num;                   /**< Number of horizontal blocks */
    uint8                         v_num;                   /**< Number of vertical blocks */
    af_core_config_filter_type    filter_info;             /**< AF filter configuration details */
} af_core_config_stats_engine_paaf;

/** pdaf_config_data_t
 *
 *   @roi: Output ROI update configuration information
 *
 *   Imglib Dual camera instant AF
 **/
typedef struct {
  af_core_roi_info_type   roi;
  int      rgn_h_num;
  int      rgn_v_num;
  boolean  reset_to_default_config;
  pdaf_roi_type           roi_type;
} af_core_pdaf_config_data_t;

/** dciaf_config_data_t
 *
 *   @roi:       ROI of the stats collection
 *   @is_af_search_active: Inform DCRF library if search is active
 *   @requireDistanceInfo: Inform DCRF library to start providing Distance
 *   @focal_length_ratio: from the DCIAF algorithm
 *
 *   Imglib Dual camera instant AF
 **/
typedef struct {
  af_core_roi_info_type   roi;
  boolean is_af_search_active;
  boolean requireDistanceInfo;
  float focal_length_ratio;
} af_core_dciaf_config_data_t;

/**
 * Stats configuration details for depth assisted af.
 */
typedef struct
{
  af_core_daaf_type daaf_type;                           /**< HAF technology type*/
  union {
    af_core_pdaf_config_data_t  pdaf_config;
    af_core_dciaf_config_data_t dciaf_config;
  } u;

} af_core_config_stats_engine_daaf;

/**
 * This structure defines AF stats engine configuration details
 * requested by core algorithm.
 */
typedef struct
{
    uint32 /*af_core_stats_mask_type*/        mask;          /**< Bit mask to indicate which stats config has been updated. */
    af_core_config_stats_engine_bayer_type  bayer_config;  /**< Configuration details of bayer hw stats engine */
    af_core_config_stats_engine_paaf        paaf_config;   /**< Configuration details for preview assisted af */
    af_core_config_stats_engine_daaf        daaf_config;   /**< Configuration details of depth assisted af */
} af_core_config_stats_engine_type;

/**
* Details of AF move lens using direction and steps.
*/
typedef struct
{
    af_core_lens_move_direction_type    direction;      /**< AF move lens direction */
    uint32                              steps_to_move;  /**< Number of steps to move */
} af_core_lens_move_steps_type;

/**
 * Details of AF move lens using DAC value.
 */
typedef struct
{
    uint16         dac;                                 /**< Dac position to move the lens to */
} af_core_lens_move_dac_type;

/**
* Details of AF move lens.
*/
typedef struct
{
    boolean     use_dac_value;                          /**< Flag, indicates lens movement using DAC or steps. */
    union
    {
        af_core_lens_move_steps_type    steps;          /**< Move lens using steps. */
        af_core_lens_move_dac_type      dac;            /**< Move lens using dac. */
    } u;
    boolean fovc_enabled;                               /**< If true FOVC is enabled */
    float mag_factor;                                   /**< FOV compensation factor */
} af_core_lens_move_type;

/**
 * Tuning parameters for core algorithm.
 */
typedef struct
{
    af_algo_tune_parms_adapter_t* ptr;                         /**< Pointer to AF core algorithm tuning header */
} af_core_tuning_header_type;

/**
 *    Extended tuning param for extensibility.
**/
typedef struct
{
    uint32                    af_caf_trigger_after_taf;    /**< Tuning param to control CAF triffer after TAF */
    float                     af_sensitiviy;
    int                       scene_change_distance_thres[AF_DISTANCE_ENTRY_MAX];
    int                       scene_change_distance_std_dev_thres;
    int                       num_near_steps_near_direction;
    int                       num_far_steps_near_direction;
    int                       num_near_steps_far_direction;
    int                       num_far_steps_far_direction;
    int                       start_pos_for_tof;
    af_tuning_sad_adapter_t   af_sad;
    int                       distance_region[AF_DISTANCE_ENTRY_MAX];
    int                       jump_to_start_limit_low_light;
    int                       tof_step_size_low_light;
    int                       far_distance_fine_step_size;
    int                       far_distance_unstable_cnt_factor;
    int                       far_converge_point;

    /* Note: Parameters that need to be moved to newer chromatix
     * version in next revision. Currently some are defined
     * in reserved section of TOF header as related reserved
     * area is already used. */
    int num_near_steps_far_converge_point;
    int num_far_steps_far_converge_point;
    int panning_stable_cnt;
    int panning_stable_thres;
    int enable_sad_when_tof_is_conf;
    unsigned short frames_to_wait_stable;
    int num_of_samples;
    /* These are all for TOF - to be updated in next revision. */
    int frame_delay_focus_converge_bright_light;
    int frame_delay_focus_converge_low_light;
    int frame_delay_fine_search_bright_light;
    int frame_delay_fine_search_low_light;
    int frame_delay_outdoor_light;
    int aec_wait_check_flag_enable;
    int use_custom_sensitivity;
    int median_filter_enable;
    int median_filter_num_of_samples;
    int average_filter_enable;
    int average_filter_num_of_samples;
    int run_tof_in_outdoor_light_flag;
     int confi_level_change_th;
     int tof_min_distance_range;
     /* adding parameters for pdaf+tof */
    int pdaf_tof_stable_cnt_th;
    int pdaf_tof_stable_cnt_th_low_light;
} af_core_tuning_params_t;

/**
 * AF parameters that control search range for a particular AF mode.
 **/
typedef struct
{
    af_mode_type mode;           /**< current focus mode. */
    af_mode_type prev_mode;      /**< nearest end of the AF search range */
    int          near_end;       /**< farthest end of the AF search range */
    int          far_end;        /**< pan position */
    int          default_pos;    /**< default lens reset position */
    int          hyp_pos;        /**< the hyperfocal position of the lens */
} af_mode_info_t;

/**
* EZ Tune parameters that are output by the core AF algorithm.
*/
typedef struct
{
    int          peak_location_index;
    boolean      ez_running;
    boolean      enable;
    uint16     roi_left;
    uint16     roi_top;
    uint16     roi_width;
    uint16     roi_height;
    int          grid_info_h_num;
    int          grid_info_v_num;
    uint32     r_fv_min;
    uint32     gr_fv_min;
    uint32     gb_fv_min;
    uint32     b_fv_min;
    int          hpf[10];
    int          mode;
    int          status;
    int          far_end;
    int          near_end;
    int          hyp_pos;
    int          state;
    int          stats_index;
    int          stats_pos;
    int          stats_fv[50];
    int          stats_max_fv;
    int          stats_min_fv;
    int          frame_delay;
    int          enable_multiwindow;
    int          Mwin[14];
    int          num_downhill;
    int          caf_state;
    uint32     cur_luma;
    int          exp_index;
    unsigned int luma_settled_cnt;
    int          ave_fv;
    int          caf_panning_unstable_cnt;
    int          caf_panning_stable_cnt;
    int          caf_panning_stable;
    int          caf_sad_change;
    int          caf_exposure_change;
    int          caf_luma_chg_during_srch;
    int          caf_trig_refocus;
    int          caf_gyro_assisted_panning;
} af_output_eztune_data_t;

/**
 * AF EZ-Tune parameters.
**/
typedef struct
{
    boolean                 enable;                              /**< Eztune is enabled/disabled. */
    boolean                 running;                             /**< index to max FV position */
    int                     peakpos_index;                       /**< current index in the array */
    int                     tracing_index;                       /**< array of subsequent focus values */
    int                     tracing_stats[AF_COLLECTION_POINTS]; /**< corresponding lens positions */
    int                     tracing_pos[AF_COLLECTION_POINTS];   /**< eztune variables */
    af_output_eztune_data_t eztune_data;
} af_eztune_t;

/**
 * AF Mobicat parameters.
**/
typedef struct
{
    boolean                 enable;                              /**< Mobicat is enabled/disabled. */
    boolean                 running;                             /**< index to max FV position */
    int                     peakpos_index;                       /**< current index in the array */
    int                     tracing_index;                       /**< array of subsequent focus values */
    int                     tracing_stats[AF_COLLECTION_POINTS]; /**< corresponding lens positions */
    int                     tracing_pos[AF_COLLECTION_POINTS];   /**< eztune variables */
    uint8                   mobicat_data[MAX_AF_STATS_DATA_SIZE];/**< mobicat variables */
    uint32                  mobicat_data_size;                   /**< Mobicat araay size */
} af_mobicat_t;

/**
 * AF Debug parameters.
 */
typedef struct {
    char *debug_data;                       /**< Debug data array pointer */
    uint32 af_debug_size;                   /**< AF debug data size */
} af_debug_t;

/**
 *  Top level input parameters to AF core algorithm.
 */
typedef struct
{
    float                              zoom_ratio;         /**< user selected magnification factor */
    uint16                             cur_lens_pos;       /**< current lens position (usually in dac) */
    af_core_stats_type                 stats_info;         /**< AF stats data - can be HW stats, SW stats,
                                                                                                               depth information. */
    af_core_search_trigger_type        search_mode;        /**< Trigger to control AF search behaviour. not used */
    af_core_search_range_type          search_range;       /**< search range. not used */
    af_core_input_aec_type             aec_info;           /**< parameters update from AEC module. */
    af_core_input_awb_type             awb_info;           /**< parameters update from AWB module. */
    af_core_input_asd_type             asd_info;           /**< parameters from asd module. */
    af_core_input_motion_sensor_type   motion_sensor_info; /**< input from motion sensors like gyro and accelerometer. */
    af_core_input_actuator_type        actuator_info;      /**< actuator specific information */
    af_core_input_img_sensor_type      img_sensor_info;    /**< parameters update from image sensor module. */
    af_core_hw_cap_type                hw_cap_info;        /**< information pertaining to AF stats capability. */
    af_core_config_roi_type            roi_info;           /**< region of interest (roi) to focus on */
    af_algo_tune_parms_adapter_t       tuning_info;        /**< AF algorithm tuning parameters */
    af_core_tuning_params_t            extended_tuning_data;/**< Advance Tuning data */
    boolean                            enable_caf_lock;     /**< True if caf lock is enabled.*/
    boolean                            force_paaf_enable;   /**< use to control force paaf enable */
    boolean                            force_roi_reconfig;  /**< set to true if force roi config needed */
    void                               *debug_data;         /**< debug data output pointer */
    uint32                             debug_data_max_size; /**< max debug array size. */
    void                               *debug_stats;        /**< debug data of stats output pointer */
    uint32                             debug_stats_max_size;/**< max debug stats array size */
    af_run_mode_type                   stream_type;         /**< Stream Type can be preview/snapshot/video. */
    boolean                            enable_exif;         /**< True if debug data is enabled */
    uint8                              exif_dbg_level;      /**< Verbose, Concise or None */
    boolean                            enable_eztune;       /**< True if eztune is enabled means tool runs manual AF */
    boolean                            eztune_running;      /**< True if eztune tool is running*/
    boolean                            enable_mobicat;      /**< True if mobicat data is enabled */
    af_fullsweep_srch_mode_type        full_sweep_mode;     /**< full sweep mode, default is 0 */
    boolean                            roi_changed;          /**< ROI changed flag */
    int                                ae_scan_test_config;       /**< AE test config */
    boolean                            face_detected;       /**< Face detected */
} af_core_input_params_type;


/**
 * Output data from AF core algorithm.
 */
typedef struct
{
    uint32                             mask;               /**< Bit masks of updated core algorithm output. */
    af_core_lens_move_type             lens_move_info;     /**< Information required to move the lens. */
    af_core_config_stats_engine_type   config_info;        /**< Data to configure AF stats configuration. */
    af_core_status_type                status;             /**< Core algorithm status to be reported. */
    af_core_opaque_data_type           data;               /**< Opaque data output specific to core algorithm. */
    float                              gravity_compensation;/**< Grav offset required for manual mode */
    af_eztune_t                        eztune_output_data;  /**< Eztune Output data */
    af_mobicat_t                       mobicat_output_data; /**< Mobicat Output data */
    af_debug_t                         debug_output_data;   /**< Debug Output data */
    boolean                            af_depth_based_focus;/**< Depth based focus in progress */
    af_core_focus_info                 af_peer_focus_info;  /**< Focus Info for peer algorithm*/
    boolean                            af_spot_light_detected;/**< Spot light detection result */
    float                              focus_value;         /**< fv for every frame */
    boolean                            af_exp_compensate;   /**< exposure compensate request */
} af_core_output_params_type;

/**
 * Define Stereo Calibration Data for AF
 */
typedef struct
{
  /*Stereo Calibration info*/
  cam_related_system_calibration_data_t af_syscalib_data;
  boolean                               isValid;
}af_core_calibration_data;
/**
 * Define Set param for focus mode event.
 */
typedef struct
{
    boolean af_mode_changed;         /**< flag for focus mode change status */
    af_mode_info_t focus_mode;       /**< payload data for focus mode set cmd */
} af_core_set_focus_mode_params_type;

/**
 * Define Set param for tuning info.
 */
typedef struct
{
  af_algo_tune_parms_adapter_t*   algo_tuning_info;         /**< Algo tuning info pointer */
    af_core_tuning_params_t       extended_tuning_info;     /**< Extended tuning info */
} af_core_set_tuning_info_param_type;

/**
 * Defines the parameters that can be set on the core AF algorithm.
 */
typedef enum
{
    AF_CORE_SET_PARAM_DEBUG_DATA_ENABLE = 0,                     /**< Enable the debug data. */
    AF_CORE_SET_PARAM_START,                                     /**< AF trigger */
    AF_CORE_SET_PARAM_CANCEL,                                    /**< Cancel Af trigger */
    AF_CORE_SET_PARAM_RESET_CAF,                                 /**< Trigger in CAF mode */
    AF_CORE_SET_PARAM_PREVIEW_STATS,                             /**< PAAF stats */
    AF_CORE_SET_PARAM_FOCUS_MODE,                                /**< Focus mode change event */
    AF_CORE_SET_PARAM_TUNING_INFO,                               /**< Set Tuning Info after Init */
    AF_CORE_SET_PARAM_HAF_ENABLE,                                /**< Set HAF Enable flag to haf init modules */
    AF_CORE_SET_PEER_FOCUS_INFO,                                 /**< Set Peer Af Focus Info */
    AF_CORE_SET_ROLE_SWITCH,                                     /**< Set AF Algo Role */
    AF_CORE_SET_STEREO_CAL_DATA,                                 /**< Set AF Stereo Calibration Data*/
    AF_CORE_SET_PARAM_CONFIG_AE_SCAN_TEST,                       /**< AE Scan test config*/
    AF_CORE_SET_PARAM_WAIT_FOR_AEC_EST,
    AF_CORE_SET_PARAM_FD_ENABLE,                                 /**< Set face detection enable flag*/
    AF_CORE_SET_PARAM_HAF_CUSTOM_FACTORY_PTR,                    /**< Set HAF custom library factory func ptr*/
    AF_CORE_SET_PARAM_HAF_CUSTOM_PARAM,                          /**< Set HAF custom param*/
    AF_CORE_SET_PARAM_FOV_COMP_ENABLE,                           /**< Set FOV compensation param*/
    AF_CORE_SET_PARAM_MW_ENABLE,                                 /**< Enable/disable MW feature*/
    AF_CORE_SET_PARAM_AF_HJ_ENABLE,                              /**< Set HJ Enable */
    AF_CORE_SET_PARAM_SKIP_FINE_SCAN,                            /**< Param to skip fine scan at runtime */
    AF_CORE_SET_PARAM_BG_STATS,                                  /**< Set BG stats */
    AF_CORE_SET_PARAM_LAST
} af_core_set_param_enum_type;

/**
 * Defines the debug data levels supported by the AF algorithm.
 */
typedef enum
{
  AF_DEBUG_DATA_LEVEL_NONE = 0,              /**< Disables debug data. */
  AF_DEBUG_DATA_LEVEL_CONCISE,               /**< Enables concise level of debug data. */
  AF_DEBUG_DATA_LEVEL_VERBOSE,               /**< Enables verbose level of debug data. */
  AF_DEBUG_DATA_LEVEL_INVALID = 0xFF         /**< Invalid/Max level. */
} af_debug_data_level_type;


/** af_role_switch_params_t:
 *    @master: If AF is master/slave
 **/
typedef struct _af_role_switch_params_t {
  boolean             master;
  af_core_focus_info  af_peer_focus_info;     /**< Peer AF Focus info */
}af_role_switch_params_t;



typedef void * (* haf_custom_factory_create_type)();

/**
 * Defines the format of the set param input structure that is passed to the core AF algorithm.
 */
typedef struct
{
    af_core_set_param_enum_type  param_type; /**< Type of parameter being set. */
    union
    {
        boolean                 debug_data_enable;                 /**< Flag for debug data enable */
        af_core_stats_paaf_type preview_stats;                     /**< PAAF stats payload */
        af_core_set_focus_mode_params_type set_focus;              /**< focus mode set payload */
        af_core_set_tuning_info_param_type tuning_load;            /**< Payload for tuning info */
        af_haf_enable_type                 haf;                    /**< enable/disable HAF technologies */
        af_core_focus_info                 af_peer_focus_info;     /**< Peer AF Focus info */
        af_role_switch_params_t            role_switch;            /**< AF Algorithm in  Follow Mode or not*/
        haf_custom_factory_create_type  haf_custom_factory_create; /**< Function pointer to HAF custom factory creation */
        cam_related_system_calibration_data_t *p_af_syscalib_data;
        int       ae_scan_test_config;
        boolean                 af_led_assist;                     /**< LED assist is on or off */
        boolean                 fd_enabled;                        /**< Face detection enable flag */
        void                            *customData;               /**< Custom Data for CustomAlgo */
        boolean fov_comp_enable;                                   /**< Is focuscompensation enabled*/
        boolean mw_enable;                                         /** < Enable disable multi-window feature */
        boolean af_hj_enable;                                      /**< Is af HJ enabled*/
        boolean skip_fine_scan;                                    /**< Skip fine scan at runtime*/
        q3a_core_bg_stats_type bg_stats;                           /** < BG stats */
    } u;

} af_core_set_param_info_type;

/**
 * Defines the parameters that can be obtained from the core AF algorithm.
 */
typedef enum
{
    AF_GET_METADATA_INFO = 0,   /**< Metadata infomation. */
    AF_GET_PARAM_LAST

} af_core_get_param_enum_type;
/**
 * Defines the format of the get param output structures that is output from the core AF algorithm.
 */
typedef struct
{
    af_core_get_param_enum_type     param_type; /**< Type of parameter being obtained. */
    union
    {
        uint32     metadata_size;               /**< Metadata size. */
    } u;

} af_core_get_param_info_type;

/**
 * Defines the format of the structure output by the core AF algorithm
 */
typedef struct
{
    uint32                          param_length;       /**< Param length in bytes. */
    union
    {
        const void*                 metadata;           /**< AF metadata buffer pointer. */
    } u;

} af_core_get_param_output_type;


/* ==========================================================================
                       Function Declarations
========================================================================== */

/** @brief AF core initialization routine.
 *
 *  Function to initialize AF core algorithm. It includes
 *  such activities as allocating memory for internal data
 *  structures, initializing core parameters and initialize
 *  AF state machines.
 *
 *  @param handle pointer to internal af handle
 *  @return standard common error code.
 */
q3a_core_result_type af_core_init(af_core_handle_type* handle);

/**
 * Gets parameters pertaining to the core AF algorithm.
 *
 * @param[in] handle                Handle to the AWB core module.
 * @param[in] get_param             Parameters for which info is requested from the AWB core module.
 * @param[out] get_param_output     Parameters output by the AWB core module.
 *
 * @return Returns Q3A_CORE_RESULT_SUCCESS upon success, any other error upon failure.
 */
q3a_core_result_type af_core_get_param
(
    af_core_handle_type                 handle,
    const af_core_get_param_info_type*  get_param_info,
    af_core_get_param_output_type*      get_param_output
);

/**
 * Sets parameters pertaining to the core AF algorithm.
 *
 * @param[in] handle                Handle to the AF core module.
 * @param[in] set_param          Parameters set on the AF core module.
 * @param[out] output             output of AF after processing stats
 *
 * @return Returns Q3A_CORE_RESULT_SUCCESS upon success, any other error upon failure.
 */
q3a_core_result_type af_core_set_param
(
    af_core_handle_type                 handle,
    const af_core_set_param_info_type*  set_param_info,
    af_core_output_params_type*         output
);

/** @brief Routine to process AF stats.
 *
 *  Main entry point to AF algorithm. It processes
 *  AF stats along with other input parameters to
 *  determine the optimal focal position of the lens.
 *
 *  @param handle to core algorithm
 *  @param in input parameters to AF core
 *  @param out output of AF after processing stats
 *  @return standard common error code.
 */
q3a_core_result_type af_core_process
(
    af_core_handle_type              handle,
    const af_core_input_params_type *in,
    af_core_output_params_type       *out
);


/** @brief AF core de-initialization routine.
 *
 *  Routine to clean up AF core. It resets internal state
 *  and AF_FREE allocated memories.
 *
 *  @param handle to core algorithm
 *  @return standard common error code.
 */

void af_core_deinit
(
    af_core_handle_type handle
);




#ifdef __cplusplus
} // extern "C"
#endif

#endif // __AFCORE_H__
