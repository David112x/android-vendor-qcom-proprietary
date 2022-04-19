/*============================================================================

   Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#ifndef DMLRO_CORRECTION_H
#define DMLRO_CORRECTION_H

/*========================================================================

*//** @file dmlrocorrection.h

This file contains DLMRO correction functions

@par EXTERNALIZED FUNCTIONS
(none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
(none)

*/ /*====================================================================== */
/*============================================================================
INCLUDE FILES
============================================================================*/
#include <stdarg.h>

/*============================================================================
DEFINITIONS and CONSTANTS
============================================================================*/

#define DMLROC_API_VERSION 4
#define DMLROC_FUNC_VERSION 63

/// Largest rolloff table sizes supported by the lib
#define DMLROC_MAX_MESH_ROLLOFF_WIDTH  17
#define DMLROC_MAX_MESH_ROLLOFF_HEIGHT 13
#define DMLROC_MAX_MESH_ROLLOFF_SIZE  (DMLROC_MAX_MESH_ROLLOFF_WIDTH * DMLROC_MAX_MESH_ROLLOFF_HEIGHT)

/*============================================================================
Type DECLARATIONS
============================================================================*/

typedef enum dmlroc_return_t {
    DMLROC_SUCCESS           =  0,
    DMLROC_NO_MEMORY         = -1,
    DMLROC_ERROR             = -2,
    DMLROC_INVALID_STATS     = -3,
    DMLROC_BAD_OUTPUT_TABLE  = -4,
    DMLROC_LIB_NOT_LOADED    = -5,
    DMLROC_NULL_PTR          = -6,
    DMLROC_WRONG_STATS_CFG   = -7,
    DMLROC_WRONG_ROLLOFF_CFG = -8
} dmlroc_return_t;

// Defines mesh based roll off correction tables
typedef struct
{
    uint16_t table_size;   /**< Size of Mesh Tables */
    float  *const r_gain;        /**< Red Channel Mesh Table */
    float  *const gr_gain;       /**< GreenRed Channel Mesh Table */
    float  *const gb_gain;       /**< GreenBlue Channel Mesh Table */
    float  *const b_gain;        /**< Blue Channel Mesh Table */
} dmlroc_mesh_rolloff_array_t;

/*
* bayer_stats_info_t defines the Bayer stats information.
*/
typedef struct
{
    uint32_t const * const channel_counts;
    uint32_t const * const channel_sums;
    // Size of each array. All the array sizes are same.
    uint32_t const array_length;
} dmlroc_bayer_stats_info_t;

typedef struct {
    uint32_t  camif_win_w;           // width of the camif window
    uint32_t  camif_win_h;           // heigth of the camif window
    uint32_t  stat_elem_w;           // width of one stat element
    uint32_t  stat_elem_h;           // heigth of one stat element
    uint32_t  num_stat_elem_rows;    // number of stat element rows
    uint32_t  num_stat_elem_cols;    // number of stat element columns
    uint32_t  b_post_stats;          // 1 = post; 0= pre stats
    uint32_t  stats_bit_depth;
    uint32_t  stats_saturation_limit;
    // from chromatix tintless_threshold
    unsigned char tint_correction_strength;
    // from chromatix tintless_high_accuracy_mode
    unsigned char tint_accuracy;     // q4.4 format
    uint32_t  num_mesh_elem_rows;    // number of mesh rows
    uint32_t  num_mesh_elem_cols;    // number of mesh columns
    uint32_t  mesh_offset_horizontal;
    uint32_t  mesh_offset_vertical;
    uint32_t  mesh_subgrid_height;
    uint32_t  mesh_subgrid_width;
    uint32_t  num_mesh_subgrids;    // any interpolation inside the mesh
    uint32_t  tintless_update_delay;
    uint32_t  tintless_threshold;
} dmlroc_config_t;

typedef struct {
    uint16_t  api_version;
    uint16_t  func_version;  // represents the Matlab version of the algo
} dmlroc_version_t;

/*============================================================================
Function DECLARATIONS
============================================================================*/

/*===========================================================================

  Function           : dmlroc_entry

  Description        : API function that computes correction rolloff table
  with Post Bayer stats

  Input parameter(s) : red bayer stats
  green-red bayer stats
  green-blue bayer stats
  blue bayer stats
  current rolloff table
  3A rolloff table

  Output parameter(s): correction rolloff table. only R and B chans are valid

  Return Value       : dmlroc_return_t

  Side Effects       : None

=========================================================================== */
dmlroc_return_t dmlroc_entry(void * const res,
      const dmlroc_bayer_stats_info_t * const pbayer_r,
      const dmlroc_bayer_stats_info_t * const pbayer_gr,
      const dmlroc_bayer_stats_info_t * const pbayer_gb,
      const dmlroc_bayer_stats_info_t * const pbayer_b,
      const dmlroc_mesh_rolloff_array_t * const ptable_current,
      const dmlroc_mesh_rolloff_array_t * const ptable_3a,
      dmlroc_mesh_rolloff_array_t * const ptable_correction);


/*===========================================================================

Function           : dmlroc_init

Description        : Initialize and configure dmlroc.
                     Call this function when there's a change to the stats
                     or if the fixed table has changed.

Input parameter(s) : cfg: bg stats & mesh config + tuning param

Return Value       : dmlroc_return_t

Side Effects       : None

=========================================================================== */
dmlroc_return_t dmlroc_init(void ** const res, const dmlroc_config_t * const cfg);


/*===========================================================================

Function           : dmlroc_deinit

Description        : De-allocated resources used by the dmlroc library.

Input parameter(s) : none

Return Value       : void

Side Effects       : camif_win_w=0 represents an uninitiated library.
                     Init must be called before running the main algo.

=========================================================================== */
void dmlroc_deinit(void ** const res);


/*===========================================================================

Function           : dmlroc_get_version

Description        : Returns the libaray version of dmlroc.

Input parameter(s) : Pointer to a dmlroc_version_t structure

Return Value       : void

Side Effects       : Input struct is updated with the version info.

=========================================================================== */
void dmlroc_get_version(dmlroc_version_t * const pVersion);

#endif /* DMLRO_CORRECTION_H */
