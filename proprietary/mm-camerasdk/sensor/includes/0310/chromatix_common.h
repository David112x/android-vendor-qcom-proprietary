//=============================================================================
// Copyright (c) 2016 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//=============================================================================

#ifndef CHROMATIX_VFE_COMMON_H
#define CHROMATIX_VFE_COMMON_H

#include "chromatix.h"
#include "chromatix_3a.h"

//=============================================================================
//                      CONSTANTS
//=============================================================================

#define CHROMATIX_VFE_COMMON_VERSION    0x310

#define CHROMATIX_REVISION_COMMON       0   // Must match chromatix_version_info.revision_number in header data file

#define NUM_BLK_REGIONS 6
//=============================================================================
// VFE basic struct
//=============================================================================

//=============================================================================
// Pedestal correction type
// 2-D black correction to To replace the single black point
//=============================================================================

#define MESH_PedestalTable_SIZE    (13 * 10)

typedef struct
{
    unsigned short  mesh_pedestal_table_size;                           // TableSize

    unsigned short  channel_black_level_r[MESH_PedestalTable_SIZE];     //12u
    unsigned short  channel_black_level_gr[MESH_PedestalTable_SIZE];    //12u
    unsigned short  channel_black_level_gb[MESH_PedestalTable_SIZE];    //12u
    unsigned short  channel_black_level_b[MESH_PedestalTable_SIZE];     //12u
} pedestalcorrection_table;

//=============================================================================
// Feature name: Pedestal correction
// Applicable chipset(s): MSM8994
// Applicable operation mode:
//     Viewfinder, snapshot processing, and video modes.
//
// Note:
// 2-D black correction to To replace the single black level;
// Separate mesh tables for interlaced HDR T1/T2 line pairs;
// Black level in chromatix_linearization_type data should be set to zero;
// Use parameters from chromatix_black_level_type to adjust black for low light
// if needed: Use trigger and max_blk_increase to interpolate blk_increase, and add
// blk_increase to each node in LUTs.
//
//
// variable name: mesh_pedestal_table_size
// Should be (13*10).
//
// Variable name: channel_black_level_r[MESH_Pedestal_SIZE]; //14u
// R channel black level.
// Default value: 0.
// Range: 0 to 16384.
// Effect: Increasing the value increases the black subtraction and increase the contrast.
//
// Variable name: channel_black_level_gr[MESH_Pedestal_SIZE]; //14u
// Gr channel black level.
// Default value: 0.
// Range: 0 to 16384.
// Effect: Increasing the value increases the black subtraction and increase the contrast.
//
// Variable name: channel_black_level_gb[MESH_Pedestal_SIZE]; //14u
// Gb channel black level.
// Default value: 0.
// Range: 0 to 16384.
// Effect: Increasing the value increases the black subtraction and increase the contrast.
//
// Variable name: channel_black_level_b[MESH_Pedestal_SIZE]; //14u
// B channel black level.
// Default value: 0.
// Range: 0 to 16384.
// Effect: Increasing the value increases the black subtraction and increase the contrast.
//
// Variable name: pctable[2],pctable_lowlight[2]
// Two pedestalcorrection_table sets for HDR (T1 and T2). For normal range, using first set.
// pctable_lowlight set is for low light, selected with lowlight trigger.
// Constraints : T1 & T2 tables are active for 8996
//             : Only T1 table is active for 8998
// Default value:
// Range:
// Effect:
//=============================================================================
typedef struct
{
    int                         pedestalcorrection_enable;
    int                         pedestalcorrection_control_enable;

    tuning_control_type         control_pedestal;
    trigger_point_type          pedestal_lowlight_trigger;

    pedestalcorrection_table    pctable[2];          //need 2 for HDR (T1 and T2)
    pedestalcorrection_table    pctable_lowlight[2]; //need 2 for HDR (T1 and T2)
} chromatix_pedestalcorrection_type;

//=============================================================================
// Linearization data types
//=============================================================================

//=============================================================================
// Feature name: Linearization.
// Applicable chipset(s): MSM8960 (Waverider), MSM8660A (Aurora), (Badger).
// Applicable operation mode:
//     Viewfinder, snapshot processing, and video modes.
//
// Note: For MSM8960, it needs only one set of the following variables;
//       For MSM8660A and Badger with native 3D support, it needs two sets of
//       the following variables -- one set for the left image (default set when
//       3D support is off) and another set for the right image.
//
// Constraints: header version 310 and later, this feature is enabled
//              when 6 region Linearization(v2) is disabled
//
// variable name: r_lut_p[8];  // HW: 12uQ0
//  Input knee points of the 9 segments (0, 1, ..., 8) of the linearization
//  curve of R channel, with segment 0 knee point defaulted to 0 hence excluded
//  from the list.
// Default value: {(black_level), (4095 - black_level) / 8 + black_level, ...,
//                 7 * (4095 - black_level) / 8} + black_level}.
// Data range: 0 to 4095.
// Effect: Changing knee points changes the shape of the linearization curve.
//
// Variable name: r_lut_base[9];  // HW: 12uQ0
//  Output base values at the starting point of the 9 segments of the
//  linearization curve of R channel.
// Default value: {0, 0, (4095 - black_level) / 8, ...,
//                 7 * (4095 - black_level) / 8}.
// Data range: 0 to 4095.
// Effect: Changing base points changes the shape of the linearization curve.
//
// Variable name: gr_lut_p[8];  // HW: 12uQ0
//  Input knee points of the 9 segments (0, 1, ..., 8) of the linearization
//  curve of Gr channel, with segment 0 knee point defaulted to 0 hence excluded
//  from the list.
// default value: {(black_level), (4095 - black_level) / 8 + black_level, ...,
//                 7 * (4095 - black_level) / 8} + black_level}.
// Data range: 0 to 4095.
// Effect: Changing knee points changes the shape of the linearization curve.
//
// Variable name: gr_lut_base[9];  // HW: 12uQ0
//  Output base values at the starting point of the 9 segments of the
//  linearization curve of Gr channel.
// Default value: {0, 0, (4095 - black_level) / 8, ...,
//                 7 * (4095 - black_level) / 8}.
// Data range: 0 to 4095.
// Effect: Changing base points changes the shape of the linearization curve.
//
// Variable name: gb_lut_p[8];  // HW: 12uQ0
//  Input knee points of the 9 segments (0, 1, ..., 8) of the linearization
//  curve of Gb channel, with segment 0 knee point defaulted to 0 hence excluded
//  from the list.
// Default value: {(black_level), (4095 - black_level) / 8 + black_level, ...,
//                 7 * (4095 - black_level) / 8} + black_level}.
// Data range: 0 to 4095.
// Effect: Changing knee points changes the shape of the linearization curve.
//
// Variable name: gb_lut_base[9];  // HW: 12uQ0
//  Output base values at the starting point of the 9 segments of the
//  linearization curve of Gb channel.
// Default value: {0, 0, (4095 - black_level) / 8, ...,
//                 7 * (4095 - black_level) / 8}.
// Data range: 0 to 4095.
// Effect: Changing base points changes the shape of the linearization curve.
//
// Variable name: b_lut_p[8];  // HW: 12uQ0
//  Input knee points of the 9 segments (0, 1, ..., 8) of the linearization
//  curve of B channel, with segment 0 knee point defaulted to 0 hence excluded
//  from the list.
// Default value: {(black_level), (4095 - black_level) / 8 + black_level, ...,
//                 7 * (4095 - black_level) / 8} + black_level}.
// Data range: 0 to 4095.
// Effect: Changing knee points changes the shape of the linearization curve.
//
// Variable name: b_lut_base[9];  // HW: 12uQ0
//  Output base values at the starting point of the 9 segments of the
//  linearization curve of B channel.
// Default value: {0, 0, (4095 - black_level) / 8, ...,
//                 7 * (4095 - black_level) / 8}.
// Data range: 0 to 4095.
// Effect: Changing base points changes the shape of the linearization curve.
//
// Slopes are compulted by SW from the lut_base and lut_p
//=============================================================================
typedef struct
{
    // R channel knee points & LUT (2 banks)
    unsigned short r_lut_p[8];      // 12uQ0
    unsigned short r_lut_base[9];   // 12uQ0
    // GR channel knee points & LUT (2 banks)
    unsigned short gr_lut_p[8];     // 12uQ0
    unsigned short gr_lut_base[9];  // 12uQ0
    // GB channel knee points & LUT (2 banks)
    unsigned short gb_lut_p[8];     // 12uQ0
    unsigned short gb_lut_base[9];  // 12uQ0
    // B channel knee points & LUT (2 banks)
    unsigned short b_lut_p[8];      // 12uQ0
    unsigned short b_lut_base[9];   // 12uQ0
} chromatix_linearization_type;

//=============================================================================
// Feature name : Black level subtraction and Scaling (BLSS)
// Supported Chipsets : 8996 and later
// BLS removes the residual black level from linearization (and
// pedestal correction) that is purposed left there to avoid low light
// color tint due to non-zero noise mean after zero-clamping in the
// pipeline if black level is totally removed.  After linearization,
// black levels of the channels are supposed to be equal.  Same BLS is
// used on the main pipe and on the statistics track.
//
// Constraints: header version 310 and later, this feature is enabled
//              when 6 region BLSS is disabled
//
// Bit Width : 14 Bit
// Stats Path will also have BLSS module, Software will configure always in
// Non HDR mode
//
// Offset :
// Residual black level to be removed from pixels.
// Range: 0 to 1023 (10u) for 8996
//      : 0 to 4095 (12u) for 8998
// Effect: Increasing this value alone will make image dimmer.
// For Non-HDR : Software will multiply Chromatix value by 4
//=============================================================================
typedef struct
{
    unsigned short black_level_offset;
} Chromatix_BLSS_type;

//=============================================================================
// roll-off data types
//=============================================================================
typedef enum
{
    ROLLOFF_TL84_LIGHT, /* Flourescent */
    ROLLOFF_A_LIGHT,    /* Incandescent */
    ROLLOFF_D65_LIGHT,  /* Day Light */
    ROLLOFF_H_LIGHT,    /* Horizon */ // 306
    ROLLOFF_MAX_LIGHT,
    ROLLOFF_INVALID_LIGHT = ROLLOFF_MAX_LIGHT
} chromatix_rolloff_light_type;

#define MESH_ROLLOFF_SIZE    (17 * 13) //0x302

typedef struct
{
    unsigned short         mesh_rolloff_table_size;     // TableSize

    float                  r_gain[MESH_ROLLOFF_SIZE];   // RGain
    float                  gr_gain[MESH_ROLLOFF_SIZE];  // GRGain
    float                  gb_gain[MESH_ROLLOFF_SIZE];  // GBGain
    float                  b_gain[MESH_ROLLOFF_SIZE];   // BGain
} mesh_rolloff_array_type;

//=============================================================================
// Variable name: last_region_unity_slope_enable
// Linearization Last region Slope will be unity when this flag is set to 1,
// Software will compute the stretch gain ( max_offset/(max_offset-blk_offset)) and apply to channel gain.
// Default value is 0, means software will compute the last region slope from base and knee points.
// Default value: 0
//=============================================================================
typedef struct
{
    int                             linearization_enable;
    int                             linearization_control_enable;
    int                             last_region_unity_slope_enable;

    tuning_control_type             control_linearization;
    trigger_point_type              linearization_lowlight_trigger;

    chromatix_linearization_type    linear_table_lowlight;
    chromatix_linearization_type    linear_table_normal;
} chromatix_L_type;

typedef struct
{
    trigger_point_type     linearization_v2_trigger;
    // R channel knee points & LUT (2 banks)
    unsigned short         r_lut_p[8];      // 12uQ0
    unsigned short         r_lut_base[9];   // 12uQ0
    // GR channel knee points & LUT (2 banks)
    unsigned short         gr_lut_p[8];     // 12uQ0
    unsigned short         gr_lut_base[9];  // 12uQ0
    // GB channel knee points & LUT (2 banks)
    unsigned short         gb_lut_p[8];     // 12uQ0
    unsigned short         gb_lut_base[9];  // 12uQ0
    // B channel knee points & LUT (2 banks)
    unsigned short         b_lut_p[8];      // 12uQ0
    unsigned short         b_lut_base[9];   // 12uQ0
} chromatix_linearization_v2_type;

//=============================================================================
// Feature name: Linearization with 6 region data.
// Applicable chipset(s): MSM8998 and later chipsets with 6 region data
// Applicable operation mode:
//     Viewfinder, snapshot processing, and video modes.
// Constraints: When linearization_v2_enable flag = 1 , software will use 6 region L data
//              else, software will use 2 region data from original structure
typedef struct
{
    int                             linearization_v2_enable;
    int                             linearization_v2_control_enable;
    int                             last_region_unity_slope_enable;

    tuning_control_type             control_linearization;

    chromatix_linearization_v2_type linear_table_data[NUM_BLK_REGIONS];
} chromatix_L_v2_type;

typedef struct
{
    int                  BLSS_enable;
    int                  BLSS_control_enable;

    tuning_control_type  control_BLSS;
    trigger_point_type   BLSS_low_light_trigger;

    Chromatix_BLSS_type  black_level_lowlight;
    Chromatix_BLSS_type  black_level_normal;
} Chromatix_blk_subtract_scale_type;

typedef struct
{
    trigger_point_type   blss_v2_trigger;
    unsigned short       black_level_offset;
} Chromatix_BLSS_v2_type;
//=============================================================================
//=============================================================================
// Feature name : Black level subtraction and Scaling (BLSS) 6 region
// Supported Chipsets : 8998 and later chipsets ( using 310 header )
// Applicable operation mode:
//     Viewfinder, snapshot processing, and video modes.
// Constraints: When BLSS_v2_enable flag = 1 , software will use 6 region BLSS data
//              else, software will use 2 region data from original structure

typedef struct
{
    int                     BLSS_v2_enable;
    int                     BLSS_v2_control_enable;

    tuning_control_type     control_BLSS;

    Chromatix_BLSS_v2_type  black_level_data[NUM_BLK_REGIONS];
} Chromatix_blk_subtract_scale_v2_type;

typedef struct
{
    chromatix_CCT_trigger_type  rolloff_H_trigger;
    chromatix_CCT_trigger_type  rolloff_A_trigger;
    chromatix_CCT_trigger_type  rolloff_D65_trigger;

    float                       rolloff_LED_start;
    float                       rolloff_LED_end;

    int                         scale_cubic;        // bicubic interpolation scale from grid to subgrid
    int                         subgridh_offset;    // horizontal size of subgrid
    int                         subgridv_offset;    // vertical size of subgrid

    mesh_rolloff_array_type     chromatix_mesh_rolloff_table[ROLLOFF_MAX_LIGHT];
    mesh_rolloff_array_type     chromatix_mesh_rolloff_table_lowlight[ROLLOFF_MAX_LIGHT];
    mesh_rolloff_array_type     chromatix_mesh_rolloff_table_golden_module[ROLLOFF_MAX_LIGHT];
    mesh_rolloff_array_type     chromatix_mesh_rolloff_table_LED;   // 1st LED table
    mesh_rolloff_array_type     chromatix_mesh_rolloff_table_LED2;  // 308: 2nd LED table
} chromatix_rolloff_type;

typedef struct
{
    float LA_LUT_backlit[64];
    float LA_LUT_solarize[64];
    float LA_LUT_posterize[64];
    float LA_LUT_blackboard[64];
    float LA_LUT_whiteboard[64];
} chromatix_LA_special_effects_type;

//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//  CHROMATIX COMMON VFE HEADER definition
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================

typedef struct
{
    //=========================================================================
    // Chromatix header version info (MUST BE THE FIRST PARAMETER)
    //=========================================================================
    ChromatixVersionInfoType                chromatix_version_info;

    //=========================================================================
    // Pedestal correction type:
    // 2-D black correction to To replace the single black point in 0x304
    //=========================================================================
    chromatix_pedestalcorrection_type       chromatix_pedestal_correction;

    //=========================================================================
    // Linearization
    //=========================================================================
    chromatix_L_type                        chromatix_L;
    //=========================================================================
    // Linearization 6 region spatial data
    //=========================================================================
    chromatix_L_v2_type                     chromatix_L_v2_data;

    //=========================================================================
    // Black Level Subtract and Scaling
    //=========================================================================
    Chromatix_blk_subtract_scale_type       Chromatix_BLSS_data;

    //=========================================================================
    // Black Level Subtract and Scaling 6 region spatial data
    //=========================================================================
    Chromatix_blk_subtract_scale_v2_type    Chromatix_BLSS_v2_data;
    //=========================================================================
    // Lens Rolloff   (Rolloff)
    //=========================================================================
    chromatix_rolloff_type                  chromatix_rolloff;

    //=========================================================================
    // Luma adaptation
    // These are parameters for special effects, manual BSM
    //=========================================================================
    chromatix_LA_special_effects_type       chromatix_LA_special_effects;

} chromatix_VFE_common_type;

#endif
