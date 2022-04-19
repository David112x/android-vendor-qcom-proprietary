/*
  Copyright (c) 2015-2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef __DISP_COLOR_APIS_PRIVATE_H__
#define __DISP_COLOR_APIS_PRIVATE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <sys/types.h>
#include <log/log.h>

#define DISP_APIS_PRIV_OFFSET 100
enum DISP_APIS_PRIV {
  DISP_APIS_PRIV_GET_SOURCE_PCC_CONFIG = 0x0,
  DISP_APIS_PRIV_SET_SOURCE_PCC_CONFIG,
  DISP_APIS_PRIV_GET_GLOBAL_PCC_CONFIG,
  DISP_APIS_PRIV_SET_GLOBAL_PCC_CONFIG,
  DISP_APIS_PRIV_GET_GLOBAL_3D_LUT_CONFIG,
  DISP_APIS_PRIV_SET_GLOBAL_3D_LUT_CONFIG,
  DISP_APIS_PRIV_GET_SOURCE_IGC_LUT_CONFIG,
  DISP_APIS_PRIV_SET_SOURCE_IGC_LUT_CONFIG,
  DISP_APIS_PRIV_GET_MIXER_GC_LUT_CONFIG,
  DISP_APIS_PRIV_SET_MIXER_GC_LUT_CONFIG,
  DISP_APIS_PRIV_GET_GLOBAL_IGC_LUT_CONFIG,
  DISP_APIS_PRIV_SET_GLOBAL_IGC_LUT_CONFIG,
  DISP_APIS_PRIV_GET_GLOBAL_GC_LUT_CONFIG,
  DISP_APIS_PRIV_SET_GLOBAL_GC_LUT_CONFIG,
  DISP_APIS_PRIV_SET_GLOBAL_SIXZONE_THRESHOLDS,
  DISP_APIS_PRIV_SET_GLOBAL_SIXZONE_CONFIG,
  DISP_APIS_PRIV_SET_GLOBAL_SIXZONE_PROTECTION,
  DISP_APIS_PRIV_GET_SOURCE_PA_RANGE,
  DISP_APIS_PRIV_GET_SOURCE_PA_CONFIG,
  DISP_APIS_PRIV_SET_SOURCE_PA_CONFIG,
  DISP_APIS_PRIV_SET_SOURCE_DITHER_STRENGTH_CONFIG,
  DISP_APIS_PRIV_GET_SOURCE_DITHER_STRENGTH_CONFIG,
  DISP_APIS_PRIV_SET_GLOBAL_DITHER_STRENGTH_CONFIG,
  DISP_APIS_PRIV_GET_GLOBAL_DITHER_STRENGTH_CONFIG,
  DISP_APIS_PRIV_GET_SOURCE_MEM_COLOR_RANGE,
  DISP_APIS_PRIV_GET_SOURCE_MEM_COLOR_CONFIG,
  DISP_APIS_PRIV_SET_SOURCE_MEM_COLOR_CONFIG,
  DISP_APIS_PRIV_GET_SOURCE_MEM_COLOR_CONFIG_OFFSET,
  DISP_APIS_PRIV_SET_SOURCE_MEM_COLOR_CONFIG_OFFSET,
  DISP_APIS_PRIV_SET_SOURCE_MEM_COLOR_PROTECTION,
  DISP_APIS_PRIV_GET_SOURCE_MEM_COLOR_PROTECTION,
  DISP_APIS_PRIV_SET_GLOBAL_MEM_COLOR_PROTECTION,
  DISP_APIS_PRIV_GET_GLOBAL_MEM_COLOR_PROTECTION,
  DISP_APIS_PRIV_GET_GLOBAL_GAMMA,
  DISP_APIS_PRIV_SET_GLOBAL_GAMMA,
  DISP_APIS_PRIV_GET_GLOBAL_MEM_COLOR_RANGE,
  DISP_APIS_PRIV_GET_GLOBAL_MEM_COLOR_CONFIG,
  DISP_APIS_PRIV_SET_GLOBAL_MEM_COLOR_CONFIG,
  DISP_APIS_PRIV_GET_GLOBAL_MEM_COLOR_CONFIG_OFFSET,
  DISP_APIS_PRIV_SET_GLOBAL_MEM_COLOR_CONFIG_OFFSET,
  DISP_APIS_PRIV_GET_SOURCE_COLOR_INFO,
  DISP_APIS_PRIV_SET_SOURCE_COLOR_INFO,
  DISP_APIS_PRIV_SET_SOURCE_MEM_COLOR_RANGE,
  DISP_APIS_PRIV_SET_GLOBAL_MEM_COLOR_RANGE,
  DISP_APIS_PRIV_SET_GLOBAL_COLOR_BALANCE_CONFIG,
  DISP_APIS_PRIV_SET_GLOBAL_PA_DITHER_CONFIG,
  DISP_APIS_PRIV_GET_GLOBAL_PA_DITHER_CONFIG,
  DISP_APIS_PRIV_SET_GLOBAL_DE_CONFIG,
  DISP_APIS_PRIV_GET_GLOBAL_DE_CONFIG,
  DISP_APIS_PRIV_GET_GLOBAL_VIRTUAL_PCC_CONFIG,
  DISP_APIS_PRIV_SET_GLOBAL_VIRTUAL_PCC_CONFIG,
  DISP_APIS_PRIV_GET_PANEL_BRIGHTNESS_INFO,
  DISP_APIS_PRIV_SET_PANEL_BRIGHTNESS_INFO,
  DISP_APIS_PRIV_NUM_APIS
};

#define LUT3D_ENTRIES_SIZE (17 * 17 * 17)
#define LUT3D_ENTRIES_COARSE_SIZE (5 * 5 * 5)
#define DITHER_ENTRIES_SIZE (4*4)
#define COLOR_BALANCE_ENTRIES_SIZE 100
#define SIX_ZONE_ENTRIES_SIZE 384

struct dither_coeff_data {
  uint32_t flags;
  uint32_t g_y_depth;
  uint32_t r_cr_depth;
  uint32_t b_cb_depth;
  uint32_t temporal_en;
  uint32_t num_entries;
  uint32_t entries[DITHER_ENTRIES_SIZE];
};

struct pa_dither_cfg_data {
  uint32_t flags;
  uint32_t strength;
  uint32_t offset_en;
  uint32_t num_entries;
  uint32_t entries[DITHER_ENTRIES_SIZE];
};

struct rgb {
  uint32_t r;
  uint32_t g;
  uint32_t b;
};

struct rgb_entry {
  struct rgb in;
  struct rgb out;
};

/*
struct lut3d_info
  flags     -- Reserved
  lut3d_id  -- Unique ID used to distiguish individual 3D Lut tables
  uniform   -- Identifies if coefficients are uniform (1) or non-uniform (0)
  num_entries-- Identifies the number of lut values in the table
  entries   -- Table buffer holding the lut values.
               The user of this structure needs to ensure that all the entries are filled
               per the requirements of the core being accessed.

               All coeffiencts are tightly packed in the structure - any unused buffer space
               will exist at the end of the buffer.

               Examples:
                - For cores that support 17x17x17:
                    If the caller decides to implement some functionality that only requires 9x9x9
                    the expectation is that the remaining entries will still be filled with padded
                    (or whatever the appropriate values are) entries.
                - Future core that supports less than 17x17x17 (e.g.13x13x13)
                    The caller will only need to fill 13x13x13 entries since this is all
                    it requires.  The remaining entries can be ignored since they will not be used.
*/
struct lut3d_info {
  uint32_t flags;
  uint32_t lut3d_id;
  uint32_t uniform;
  uint32_t num_entries;
  struct rgb_entry entries[LUT3D_ENTRIES_SIZE];
};

/*
struct lut3d_info_coarse
  flags     -- Reserved
  lut3d_id  -- Unique ID used to distiguish individual 3D Lut coarse tables
  uniform   -- Identifies if coefficients are uniform (1) or non-uniform (0)
  num_entries -- Identifies the number of lut values in the table
  entries   -- Table buffer holding the lut values.
               The user of this structure needs to ensure that all the entries are filled
               per the requirements of the core being accessed.
               Examples:
                - For cores that support 5x5x5:
                    If the caller decides to implement some functionality that only requires 3x3x3
                    the expectation is that the remaining entries will still be filled with padded
                    (or whatever the appropriate values are) entries.
                - Future core that supports less than 5x5x5 (e.g.3x3x3)
                    The caller will only need to fill 3x3x3 entries since this is all it requires.
                    The remaining entries can be ignored since they will not be used.
*/
struct lut3d_info_coarse {
  uint32_t flags;
  uint32_t lut3d_id;
  uint32_t uniform;
  uint32_t num_entries;
  struct rgb_entry entries[LUT3D_ENTRIES_COARSE_SIZE];
};

/* Definitions for Six Zone Protection controls from PA processing
  DISP_MEM_PROT_SIX_ZONE_EN           -- Protect SixZone Color Adjust From PA Hue processing
*/
#define DISP_MEM_PROT_SIX_ZONE_EN 0x00000001

/*
struct disp_six_zone_config_data -- structure for storing six zone data
  hue        -- Hue setting in units degrees
  saturation -- Saturation setting in units of percentage
  brightness -- Brightness setting in units of percentage
*/
struct disp_six_zone_config_data {
  double hue;
  double saturation;
  double value;
};

struct disp_six_zone_config {
  uint32_t flags;
  uint32_t num_entries;
  struct disp_six_zone_config_data entries[SIX_ZONE_ENTRIES_SIZE];
};

/*
This enum contains fields that allow the trapezoid sides to be defined as a sharp cut off.
This takes care of cases where we only want to roll off at the low or high end,
while the other side is defined to be at the maximum (or minimum) value.
*/
typedef enum {
  LEFT_EDGE_SHARP_RIGHT_EDGE_SHARP,
  LEFT_EDGE_SLOPE_RIGHT_EDGE_SHARP,
  LEFT_EDGE_SHARP_RIGHT_EDGE_SLOPE,
  LEFT_EDGE_SLOPE_RIGHT_EDGE_SLOPE,
} trapezoid_edge_type;

/*
Note: step parameter in disp_range will be ignored.
*/
struct disp_six_zone_threshold {
  uint32_t flags;
  struct disp_range sat;
  struct disp_range value;
  uint32_t sat_offset;
  uint32_t value_offset;
  trapezoid_edge_type val_edge_type;
  trapezoid_edge_type sat_edge_type;
};

/*
The following macros are to be used as define feature Identifers to be used
as part of the versioning queries.
  FEATURE_VER_HW_CHIP           - For describing the version of the Chip HW
  FEATURE_VER_HW_SDE            - For describing the version of the SDE Core HW
  FEATURE_VER_HW_PA_GLOBAL      - For describing the version of the Global PA HW
  FEATURE_VER_HW_PA_SOURCE      - For describing the version of the Source PA HW
  FEATURE_VER_HW_MEMCOL_GLOBAL  - For describing the version of the Global Memory Color HW
  FEATURE_VER_HW_MEMCOL_SOURCE  - For describing the version of the Source Memory Color HW
  FEATURE_VER_HW_SIXZONE_GLOBAL - For describing the version of the Global Six Zone HW
  FEATURE_VER_HW_SIXZONE_SOURCE - For describing the version of the Source Six Zone HW
  FEATURE_VER_HW_PCC_SOURCE     - For describing the version of the Source PCC HW
  FEATURE_VER_HW_IGC_SOURCE     - For describing the version of the Source IGC HW
  FEATURE_VER_HW_GC_MIXER       - For describing the version of the Mixer GC HW

    For the above items, refer to Design Documentation for Version information

  FEATURE_VER_SW_QDCM           - For describing the verison of the QDCM support
                                        - x.x.x : Dynamically queried from QDCM Tool
  FEATURE_VER_SW_SIX_ZONE_API   - For describing the version of the Six Zone Support
                                        - 1.0.0 : Initial Release
  FEATURE_VER_SW_QDCM_SAVE_MODE_API - For describing the version of the QDCM Save Mode Support
                                        - 1.0.0 : Initial Release
  FEATURE_VER_SW_COLOR_SOURCE_INFO_API - For describing the version of the Color Source Info Support
                                        - 1.0.0 : Initial Release
  FEATURE_VER_SW_3DLUT_API      - For describing the version of the 3DLUT Support
                                        - 1.0.0 : Initial Release
  FEATURE_VER_SW_COMP_INFO_API  - For describing the version of the Composition Info Support
                                        - 1.0.0 : Initial Release


 Note: The range 0x00000000->0xEFFFFFFF is reserved for publicly exposed
 functionality.  Refer to the public header for details.
*/
#define FEATURE_VER_HW_GLOBAL_COUNT 25
#define FEATURE_VER_HW_CHIP 0xF0000000
#define FEATURE_VER_HW_SDE 0xF0000001
#define FEATURE_VER_HW_PA_GLOBAL 0xF0000002
#define FEATURE_VER_HW_PA_SOURCE 0xF0000003
#define FEATURE_VER_HW_MEMCOL_GLOBAL 0xF0000004
#define FEATURE_VER_HW_MEMCOL_SOURCE 0xF0000005
#define FEATURE_VER_HW_SIXZONE_GLOBAL 0xF0000006
#define FEATURE_VER_HW_SIXZONE_SOURCE 0xF0000007
#define FEATURE_VER_HW_PCC_SOURCE 0xF0000009
#define FEATURE_VER_HW_3DLUT 0xF000000A
#define FEATURE_VER_HW_IGC_SOURCE 0xF000000B
#define FEATURE_VER_HW_GC_MIXER 0xF000000C
#define FEATURE_VER_HW_DITHER_GLOBAL 0xF000000F
#define FEATURE_VER_HW_DITHER_SOURCE 0xF0000010
#define FEATURE_VER_HW_PA_DITHER_GLOBAL 0xF0000011
#define FEATURE_VER_HW_PA_DITHER_SOURCE 0xF0000012
#define FEATURE_VER_HW_FRAME_CAPTURE 0xF0000013
#define FEATURE_VER_HW_DETAILED_ENHANCER_GLOBAL 0xF0000014
#define FEATURE_VER_HW_VIRTUAL_PCC_GLOBAL 0xF0000015
#define FEATURE_VER_HW_PANEL_BRIGHTNESS_INFO 0xF0000016
#define FEATURE_VER_SW_QDCM 0xF1000000
#define FEATURE_VER_SW_SIX_ZONE_API 0xF1000002
#define FEATURE_VER_SW_QDCM_SAVE_MODE_API 0xF1000003
#define FEATURE_VER_SW_COLOR_SOURCE_INFO_API 0xF1000004
#define FEATURE_VER_SW_3DLUT_API 0xF1000006
#define FEATURE_VER_SW_COMP_INFO_API 0xF1000008
#define FEATURE_VER_SW_DITHER_API 0xF1000009
#define FEATURE_VER_SW_PA_DITHER_API 0xF100000A
#define FEATURE_VER_SW_FRAME_CAPTURE_API 0xF100000B
#define FEATURE_VER_SW_DETAILED_ENHANCER_API 0xF100000C
#define FEATURE_VER_SW_VIRTUAL_PCC_GLOBAL_API 0xF100000D
#define FEATURE_VER_SW_PANEL_BRIGHTNESS_INFO_API 0xF100000E
#define FEATURE_VER_SW_ATTRIBUTE_INFO_API 0xF100000F

/*
This enum includes the value gamma values that can be set for global gamma
  GLOBAL_GAMMA_1P0        -- Gamma value of 1.0
  GLOBAL_GAMMA_1P15       -- Gamma value of 1.15
  GLOBAL_GAMMA_1P25       -- Gamma value of 1.25
  GLOBAL_GAMMA_2P2        -- Gamma value of 2.2
  GLOBAL_GAMMA_2P6        -- Gamma value of 2.6
  GLOBAL_GAMMA_1OVER_1P15 -- Gamma value of 1/1.15
  GLOBAL_GAMMA_1OVER_1P3  -- Gamma value of 1/1.3
  GLOBAL_GAMMA_1OVER_1P8  -- Gamma value of 1/1.8
  GLOBAL_GAMMA_1OVER_1P9  -- Gamma value of 1/1.9
  GLOBAL_GAMMA_1OVER_2P0  -- Gamma value of 1/2.0
  GLOBAL_GAMMA_REC601     -- Gamma value associate with REC601
  GLOBAL_GAMMA_REC709     -- Gamma value associated with REC709
  GLOBAL_GAMMA_SRGB       -- Gamma value associated with sRGB
  GLOBAL_CUSTOM_0         -- Custom gamma value specified by an OEM
  GLOBAL_CUSTOM_1         -- Custom gamma value specified by an OEM
  GLOBAL_CUSTOM_2         -- Custom gamma value specified by an OEM
*/
typedef enum {
  GLOBAL_GAMMA_1P0 = 0,
  GLOBAL_GAMMA_1P15,
  GLOBAL_GAMMA_1P25,
  GLOBAL_GAMMA_2P2,
  GLOBAL_GAMMA_2P6,
  GLOBAL_GAMMA_1OVER_1P15,
  GLOBAL_GAMMA_1OVER_1P3,
  GLOBAL_GAMMA_1OVER_1P8,
  GLOBAL_GAMMA_1OVER_1P9,
  GLOBAL_GAMMA_1OVER_2P0,
  GLOBAL_GAMMA_REC601,
  GLOBAL_GAMMA_REC709,
  GLOBAL_GAMMA_SRGB,
  GLOBAL_GAMMA_CUSTOM_0 = 0xF0000000,
  GLOBAL_GAMMA_CUSTOM_1,
  GLOBAL_GAMMA_CUSTOM_2,
  GLOBAL_GAMMA_MAX,
  GLOBAL_GAMMA_FORCE32BIT = 0x7FFFFFFF
} global_gamma_type;

struct color_balance_pcc_entry {
  int32_t index;
  float r;
  float g;
  float b;
};

struct color_balance_pcc_table {
  uint32_t flags;
  uint32_t num_entries;
  struct color_balance_pcc_entry entries[COLOR_BALANCE_ENTRIES_SIZE];
};

/*
This enum includes all various memory colors as part ot the memory color support.
  MEM_COL_SKIN     -- Skin Tone Color
  MEM_COL_SKY,     -- Sky Color
  MEM_COL_FOLIAGE  -- Foliage Color
*/
typedef enum {
  MEM_COL_SKIN = 0,
  MEM_COL_SKY,
  MEM_COL_FOLIAGE,
  MEM_COL_MAX,
  MEM_COL_FORCE32BIT = 0x7FFFFFFF
} disp_mem_col_type;

/* Definitions for Memory Color Protection controls from PA processing
  DISP_MEM_COL_PROT_HUE           -- Protect Memory Color Adjust From PA Hue processing
  DISP_MEM_COL_PROT_SATURATION    -- Protect Memory Color Adjust From PA Saturation processing
  DISP_MEM_COL_PROT_VALUE         -- Protect Memory Color Adjust From PA Value processing
  DISP_MEM_COL_PROT_CONTRAST      -- Protect Memory Color Adjust From PA Contrast processing
*/
#define DISP_MEM_COL_PROT_HUE 0x00000001
#define DISP_MEM_COL_PROT_SATURATION 0x00000002
#define DISP_MEM_COL_PROT_VALUE 0x00000004
#define DISP_MEM_COL_PROT_CONTRAST 0x00000008
#define DISP_MEM_COL_PROT_BLEND_EN 0x00000010

/*
struct disp_mem_color_config_data -- structure for storing memory color data
  hue           -- Hue setting, units of degrees
  saturation    -- Saturation setting, units of percentage
  value         -- Value setting, units of percentage
*/
struct disp_mem_color_config_data {
  int32_t hue;
  float saturation;
  float value;
};

/*
struct disp_mem_color_config -- structure for storing memory color config
  flags         -- Reserved
  col           -- Indicates memory color region
  data          -- Memory color region configuration data
*/
struct disp_mem_color_config {
  uint32_t flags;
  struct disp_mem_color_config_data data;
};

/*
struct disp_mem_color_range -- structure for getting the ranges of global PA data
  flags         -- Reserved
  hue           -- Supported range and step sizes for hue
  saturation    -- Supported range and step sizes for saturation
  value         -- Supported range and step sizes for value
*/
struct disp_mem_color_range {
  uint32_t flags;
  struct disp_range hue;
  struct disp_range_float saturation;
  struct disp_range_float value;
};

/*
struct disp_mem_color_config_offset -- structure for setting the offsets of global PA data
  flags         -- Reserved
  hue_offset    -- Offset of the Hue Trapezoid
  saturation_offset -- Offset of the Saturation Trapezoid
  value_offset  -- Offset of the Value Trapezoid
*/
struct disp_mem_color_config_offset {
  uint32_t flags;
  uint32_t hue_offset;
  uint32_t saturation_offset;
  uint32_t value_offset;
  trapezoid_edge_type val_edge_type;
  trapezoid_edge_type sat_edge_type;
};

/*
This enum includes all the various pixel color ranges supported.
  COLOR_RANGE_FULL    -- Full range : 0-255 for Luma and Chroma Planes
  COLOR_RANGE_LIMITED -- Limited or reduced range:
                         Luma : 16-235
                         Chroma : 16-240

  Note: Above ranges are defined for 8 bit depth color depths.
  For bit depths greater than 8 bit (e.g 10), the above ranges need
  to be adjusted by the corresponding bit depth differences from 8 bit.

  Range value = Orig << (Bit Depth-8)
*/
typedef enum {
  COLOR_RANGE_FULL = 0,
  COLOR_RANGE_LIMITED,
  COLOR_RANGE_MAX,
  COLOR_RANGE_FORCE32 = 0x7FFFFFFF
} color_range_type;

/*
This enum includes all the various pixel color primaries supported.
  COLOR_PRIM_601      -- Color primaries as defined by REC601
  COLOR_PRIM_709      -- Color primaries as defined by REC709
  COLOR_PRIM_2020     -- Color primaries as defined by REC2020
  COLOR_PRIM_SRGB     -- Color primaries as defined by SRGB
  COLOR_PRIM_ADOBERGB -- Color primaries as defined by Adobe RGB
*/
typedef enum {
  COLOR_PRIM_601 = 0,
  COLOR_PRIM_709,
  COLOR_PRIM_2020,
  COLOR_PRIM_SRGB,
  COLOR_PRIM_ADOBERGB,
  COLOR_PRIM_MAX,
  COLOR_PRIM_FORCE32 = 0x7FFFFFFF
} color_primaries;

/*
This enum includes all the various cositing settings supported.
  COSITING_NONE    -- Not Applicable\Center Cositing
  COSITING_IMAGE   -- Cosite : Typically associated with JPEG\MPEG1
  COSITING_VIDEO   -- Offsited : Typically associated with MPEG2\H.264
*/
typedef enum {
  COSITING_NONE = 0,
  COSITING_IMAGE,
  COSITING_VIDEO,
  COSITING_MAX,
  COSITING_FORCE32 = 0x7FFFFFFF
} cositing_type;

/*
struct color_info -- structure for color information data
  flags     -- Reserved
  range     -- Pixel color range setting
  gamma     -- Gamma value setting
  cosite    -- Cositing setting
  primaries -- Color primary setting
*/
struct color_info {
  uint32_t flags;
  color_range_type range;
  disp_gamma_type gamma;
  cositing_type cosite;
  color_primaries primaries;
};

/*
 * Detailed Enhancer (DE) configuration selection flags
 */
#define QSEED_TUNING_FLAGS_SHARP_FACTOR         0x00000001
#define QSEED_TUNING_FLAGS_CLIP                 0x00000002
#define QSEED_TUNING_FLAGS_THR_QUIET            0x00000004
#define QSEED_TUNING_FLAGS_THR_DIEOUT           0x00000008
#define QSEED_TUNING_FLAGS_THR_LOW              0x00000010
#define QSEED_TUNING_FLAGS_THR_HIGH             0x00000020
#define QSEED_TUNING_FLAGS_CONTENT_QUAL_LEVEL   0x00000040

/*
 * This enum describes the content quality level
 */
typedef enum {
  DE_CONTENT_QUALITY_UNKNOWN = 0,
  DE_CONTENT_QUALITY_LOW,
  DE_CONTENT_QUALITY_MEDIUM,
  DE_CONTENT_QUALITY_HIGH,
  DE_CONTENT_QUALITY_MAX,
  DE_CONTENT_QUALITY_FORCE32BIT = 0x7FFFFFFF,
} disp_de_content_qual_level;

/*
 * This enum describes the content type
 */
typedef enum {
  DE_CONTENT_TYPE_UNKNOWN = 0,
  DE_CONTENT_TYPE_VIDEO,
  DE_CONTENT_TYPE_GRAPHICS,
  DE_CONTENT_TYPE_MAX,
  DE_CONTENT_TYPE_FORCE32BIT = 0x7FFFFFFF,
} disp_de_content_type;

/*
struct disp_de_tuning_cfg -- structure to configure or retrieve detailed enhancer.
  flags - QSEED_TUNING_FLAGS_X
  sharp_factor -- sharpness factor.
  thr_quiet -- quiet threshold
  thr_dieout -- dieout threshold
  thr_low -- low threshold
  thr_high -- high threshold
  clip -- clip
  quality -- content quality level
  content_type -- content type
*/
struct disp_de_tuning_cfg {
  uint32_t flags;
  int32_t sharp_factor;
  uint16_t thr_quiet;
  uint16_t thr_dieout;
  uint16_t thr_low;
  uint16_t thr_high;
  uint16_t clip;
  disp_de_content_qual_level quality;
  disp_de_content_type content_type;
};

/*
struct dip_api_internal_global_virtual_pcc_config -- structure to get virtual pcc values.
  enable -- denotes status enable (1)/disable(0)
  pcc_coeff_data -- pcc data
*/
struct dip_api_internal_global_virtual_pcc_config {
  uint32_t enable;
  pcc_coeff_data cfg;
};
/*
struct disp_api_internal_global_3dlut_config -- structure to set 3d lut config setting.
  enable -- denotes status enable (1)/disable(0)
  cfg -- lut3d fine settings
  cfg_coarse -- lut3d coarse settings
*/
struct disp_api_internal_global_3dlut_config {
  uint32_t enable;
  lut3d_info *cfg;
  lut3d_info_coarse *cfg_coarse;
};


/* struct disp_panel_brightness_info -- structure to represent backlight level info
  level -- denotes panel brightness level at the creation of color mode
  black_luminance_in_nits -- denotes black luminance level
  white_luminance_in_nits -- denotes white luminance level
  flags -- denote validity of the other memebers 0 : invalid , 1 - valid
    bit 0 - level
    bit 1 - black_luminance_in_nits
    bit 2 - white_luminance_in_nits
*/
struct disp_panel_brightness_info {
  float level;
  float black_luminance_in_nits;
  float white_luminance_in_nits;
  uint32_t flags;
};

/*
struct disp_api_internal_panel_brightness_info -- structure to get mode brightness level info
  enable -- denotes status enable (1) / disable (0)
  disp_panel_brightness_info - mode's panel brightness level information
*/
struct disp_api_internal_panel_brightness_info {
   uint32_t enable;
   disp_panel_brightness_info cfg;
};

/*=========================================================================
FUNCTION
  disp_api_get_source_pcc_config()

DESCRIPTION
  This API retrieves the per source layer PCC info.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  layer_id  -- Identifies the associated source layer
  enable    -- State of mode : 0 = disable, 1 = enable
  cfg       -- PCC data

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_get_source_pcc_config(DISPAPI_HANDLE hctx, uint32_t disp_id, uint64_t layer_id,
                                       uint32_t *enable, struct pcc_coeff_data *cfg);

/*=========================================================================
FUNCTION
  disp_api_set_source_pcc_config()

DESCRIPTION
  This API sets the per source layer PCC Info.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  layer_id  -- identifies the associated source layer
  enable    -- State of mode : 0 = disable, 1 = enable
  cfg       -- PCC configuration data

  Parameter Usage:
  1: enable = 1 & cfg = non-NULL => enable functionality and apply configuration specified via cfg
  2: enable = 1 & cfg = NULL => enable functionality and apply previous (cached) configuration
  3: enable = 0 & cfg = non-NULL => disable functionality and cache configuration specified via cfg
  4: enable = 0 & cfg = NULL => disable functionality

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_set_source_pcc_config(DISPAPI_HANDLE hctx, uint32_t disp_id, uint64_t layer_id,
                                       uint32_t enable, struct pcc_coeff_data *cfg);

/*=========================================================================
FUNCTION
  disp_api_set_global_color_balance_config()

DESCRIPTION
  This API saves the color temperature LUT on the target, and is expected to call after
  disp_api_start_save_qdcm_modes_procedure.

  hctx      -- Context handle.
  disp_id   -- display ID type
  cfg       -- color temperature LUT config

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_set_global_color_balance_config(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                                 struct color_balance_pcc_table *cfg);

/*=========================================================================
FUNCTION
  disp_api_get_global_3dlut_config()

DESCRIPTION
  This API retrieves the 3dlut info.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  enable    -- State of mode : 0 = disable, 1 = enable
  cfg       -- 3d lut coefficients
  cfg_coarse-- 3d lut coarse coefficients.

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_get_global_3dlut_config(DISPAPI_HANDLE hctx, uint32_t disp_id, uint32_t *enable,
                                         struct lut3d_info *cfg,
                                         struct lut3d_info_coarse *cfg_coarse);

/*=========================================================================
FUNCTION
  disp_api_set_global_3dlut_config()

DESCRIPTION
  This API sets the 3DLUT Info.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  enable    -- State of mode : 0 = disable, 1 = enable
  cfg       -- 3d lut coefficients
  cfg_coarse-- 3d lut coarse coefficients.

  Parameter Usage:
  1: enable = 1 & cfg (or cfg_coarse) = non-NULL => enable functionality and apply configuration
                             specified via data (cfg or cfg_coarse)
  2: enable = 1 & cfg (or cfg_coarse)= NULL => enable functionality and apply previous (cached)
      configuration
  3: enable = 0 & cfg (or cfg_coarse)= non-NULL => disable functionality and cache configuration
                      specified via data (cfg or cfg_coarse)
  4: enable = 0 & cfg (or cfg_coarse)= NULL => disable functionality

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_set_global_3dlut_config(DISPAPI_HANDLE hctx, uint32_t disp_id, uint32_t enable,
                                         struct lut3d_info *cfg,
                                         struct lut3d_info_coarse *cfg_coarse);

/*=========================================================================
FUNCTION
  disp_api_get_source_igclut_config()

DESCRIPTION
  This API retrieves the per source layer igc lut info.

  hctx      -- Context handle.
  disp_id   -- display ID type
  layer_id  -- Identifies the associated source layer
  enable    -- State of mode : 0 = disable, 1 = enable
  cfg       -- Igc lut Info

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_get_source_igclut_config(DISPAPI_HANDLE hctx, uint32_t disp_id, uint64_t layer_id,
                                          uint32_t *enable, struct lut_info *cfg);

/*=========================================================================
FUNCTION
  disp_api_set_source_igclut_config()

DESCRIPTION
  This API sets the per source layer igc lut Info.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  layer_id  -- Identifies the associated source layer
  enable    -- State of mode : 0 = disable, 1 = enable
  cfg       -- Igc lut Info

  Parameter Usage:
  1: enable = 1 & cfg = non-NULL => enable functionality and apply configuration specified via cfg
  2: enable = 1 & cfg = NULL => enable functionality and apply previous (cached) configuration
  3: enable = 0 & cfg = non-NULL => disable functionality and cache configuration specified via cfg
  4: enable = 0 & cfg = NULL => disable functionality

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_set_source_igclut_config(DISPAPI_HANDLE hctx, uint32_t disp_id, uint64_t layer_id,
                                          uint32_t enable, struct lut_info *cfg);

/*=========================================================================
FUNCTION
  disp_api_get_mixer_gclut_config()

DESCRIPTION
  This API retrieves the mixer gc lut info.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  enable    -- State of mode : 0 = disable, 1 = enable
  cfg       -- gc lut Info

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_get_mixer_gclut_config(DISPAPI_HANDLE hctx, uint32_t disp_id, uint32_t *enable,
                                        struct lut_info *cfg);

/*=========================================================================
FUNCTION
  disp_api_set_mixer_gclut_config()

DESCRIPTION
  This API sets the mixer gc lut Info.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  enable    -- State of mode : 0 = disable, 1 = enable
  cfg       -- gc lut Info

  Parameter Usage:
  1: enable = 1 & cfg = non-NULL => enable functionality and apply configuration specified via cfg
  2: enable = 1 & cfg = NULL => enable functionality and apply previous (cached) configuration
  3: enable = 0 & cfg = non-NULL => disable functionality and cache configuration specified via cfg
  4: enable = 0 & cfg = NULL => disable functionality

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_set_mixer_gclut_config(DISPAPI_HANDLE hctx, uint32_t disp_id, uint32_t enable,
                                        struct lut_info *cfg);

/*=========================================================================
FUNCTION
  disp_api_set_global_six_zone_thresholds()

DESCRIPTION
  This API sets the global six zone thresholds.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  thresholds-- Threshold values

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_set_global_six_zone_thresholds(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                                struct disp_six_zone_threshold *thresholds);

/*=========================================================================
FUNCTION
  disp_api_set_global_six_zone_config()

DESCRIPTION
  This API sets the values for global_six zone color adjustment.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  enable    -- Indicates enablement mode of six zone specifed via cfg:
               0 = disable, 1 = enable
  cfg       -- Six zone adjustment configuration

  Parameter Usage:
  1: enable = enable & cfg = non-NULL =>
                                   enable functionality and apply configuration specified via cfg
  2: enable = enable & cfg = NULL => enable functionality and apply previous (cached) configuration
  3: enable = disable & cfg = non-NULL =>
                                   disable functionality and cache configuration specified via cfg
  4: enable = disable & cfg = NULL => disable functionality

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_set_global_six_zone_config(DISPAPI_HANDLE hctx, uint32_t disp_id, uint32_t enable,
                                            struct disp_six_zone_config *cfg);

/*=========================================================================
FUNCTION
  disp_api_set_global_six_zone_protection()

DESCRIPTION
  This API sets the global protection of six zone adjustments from the global PA processing.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  enable    -- State of mode : 0 = disable, 1 = enable
  level     -- Procesing to Protect.
  flags     -- Reserved

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_set_global_six_zone_protection(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                                    uint32_t enable, uint32_t level,
                                                    uint32_t flags);

/*=========================================================================
FUNCTION
  disp_api_get_source_pa_range()

DESCRIPTION
  This API retrieves the per source picture adjustment ranges for hue, saturation, value
  and contrast.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  layer_id  -- identifies the associated source layer
  range     -- PA Range Information

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_get_source_pa_range(DISPAPI_HANDLE hctx, uint32_t disp_id, uint64_t layer_id,
                                     struct disp_pa_range *range);

/*=========================================================================
FUNCTION
  disp_api_get_source_pa_config()

DESCRIPTION
  This API retrieves the current per source picture adjustment configuration.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  layer_id  -- Identifies the associated source layer
  enable    -- State of mode : 0 = disable, 1 = enable
  cfg       -- PA configuration information

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_get_source_pa_config(DISPAPI_HANDLE hctx, uint32_t disp_id, uint64_t layer_id,
                                      uint32_t *enable, struct disp_pa_config *cfg);

/*=========================================================================
FUNCTION
  disp_api_set_source_pa_config()

DESCRIPTION
  This API sets the per source picture adjustment configuration.

  hctx      -- Context handle.
  disp_id   -- display ID type
  layer_id  -- Identifies the associated source layer
  enable    -- State of mode : 0 = disable, 1 = enable
  cfg       -- PA configuration information

  Parameter Usage:
  1: enable = 1 & cfg = non-NULL => enable functionality and apply configuration specified via cfg
  2: enable = 1 & cfg = NULL => enable functionality and apply previous (cached) configuration
  3: enable = 0 & cfg = non-NULL => disable functionality and cache configuration specified via cfg
  4: enable = 0 & cfg = NULL => disable functionality

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_set_source_pa_config(DISPAPI_HANDLE hctx, uint32_t disp_id, uint64_t layer_id,
                                      uint32_t enable, struct disp_pa_config *cfg);

/*=========================================================================
FUNCTION
  disp_api_get_source_memory_color_range()

DESCRIPTION
  This API retrieves the supported source memory color adjustment range information.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  layer_id  -- Identifies the associated source layer
  range     -- Memory Color Range information

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_get_source_memory_color_range(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                               uint64_t layer_id, disp_mem_col_type col,
                                               struct disp_mem_color_range *range);

/*=========================================================================
FUNCTION
  disp_api_set_source_memory_color_range()

DESCRIPTION
  This API sets the source memory color adjustment range information.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  layer_id  -- Identifies the associated source layer
  col       -- identify which mc_zone to configure.
  range     -- Memory Color Range information

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_set_source_memory_color_range(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                               uint64_t layer_id, disp_mem_col_type col,
                                               struct disp_mem_color_range *range);

/*=========================================================================
FUNCTION
  disp_api_get_source_memory_color_config()

DESCRIPTION
  This API retrieves the current per source memory color adjustment configuration.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  layer_id  -- Identifies the associated source layer
  enable    -- State of memory color region mode : 0 = disable, 1 = enable
  col       -- identify which mc_zone to configure.
  cfg       -- Memory Color configuration

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_get_source_memory_color_config(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                                uint64_t layer_id, uint32_t *enable,
                                                disp_mem_col_type col,
                                                struct disp_mem_color_config *cfg);

/*=========================================================================
FUNCTION
  disp_api_set_source_memory_color_config()

DESCRIPTION
  This API sets the per source memory color adjustment configuration.

  hctx      -- Context handle.
  disp_id   -- display ID type
  layer_id  -- identifies the associated source layer
  enable    -- State of memory color region mode : 0 = disable, 1 = enable
  col       -- identify which mc_zone to configure.
  cfg       -- Memory Color configuration information.

  Parameter Usage:
  1: enable = enable & cfg = non-NULL =>
                                  enable functionality and apply configuration specified via cfg
  2: enable = enable & cfg = NULL => enable functionality and apply previous (cached) configuration
  3: enable = disable & cfg = non-NULL =>
                                  disable functionality and cache configuration specified via cfg
  4: enable = disable & cfg = NULL => disable functionality

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_set_source_memory_color_config(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                                uint64_t layer_id, uint32_t enable,
                                                disp_mem_col_type col,
                                                struct disp_mem_color_config *cfg);

/*=========================================================================
FUNCTION
  disp_api_set_source_memory_color_protection()

DESCRIPTION
  This API sets the per source protection of memory color adjustments from the
  per source PA processing.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  layer_id  -- Identifies the associated source layer
  enable    -- State of mode : 0 = disable, 1 = enable
  prot      -- Procesing to Protect.
  flags     -- Reserved

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_set_source_memory_color_protection(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                                    uint32_t layer_id, uint32_t enable,
                                                    uint32_t level, uint32_t flags);

/*=========================================================================
FUNCTION
  disp_api_get_source_memory_color_protection()

DESCRIPTION
  This API retrieves the per source protection settings of memory color adjustments from
  the per source PA processing.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  layer_id  -- Identifies the associated source layer
  enable    -- State of mode : 0 = disable, 1 = enable
  prot      -- Procesing to Protect.
  flags     -- Reserved

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_get_source_memory_color_protection(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                                    uint64_t layer_id, uint32_t *enable,
                                                    uint32_t *level, uint32_t *flags);

/*=========================================================================
FUNCTION
  disp_api_set_source_memory_color_config_offset()

DESCRIPTION
  This API sets the supported source memory color adjustment offset information.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  layer_id  -- Identifies the associated source layer
  col       -- identify which mc_zone to configure.
  cfg       -- Memory Color offset information

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_set_source_memory_color_config_offset(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                                       uint64_t layer_id, disp_mem_col_type col,
                                                       struct disp_mem_color_config_offset *cfg);

/*=========================================================================
FUNCTION
  disp_api_get_source_memory_color_config_offset()

DESCRIPTION
  This API retrieves the supported source memory color adjustment offset information.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  layer_id  -- Identifies the associated source layer
  col       -- identify which mc_zone to configure.
  cfg       -- Memory Color offset information

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_get_source_memory_color_config_offset(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                                       uint64_t layer_id, disp_mem_col_type col,
                                                       struct disp_mem_color_config_offset *cfg);

/*=========================================================================
FUNCTION
  disp_api_set_global_memory_color_protection()

DESCRIPTION
  This API sets the global protection of memory color adjustments from the global PA processing.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  enable    -- State of mode : 0 = disable, 1 = enable
  prot      -- Procesing to Protect.
  flags     -- Reserved

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_set_global_memory_color_protection(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                                    uint32_t enable, uint32_t level,
                                                    uint32_t flags);

/*=========================================================================
FUNCTION
  disp_api_get_global_memory_color_protection()

DESCRIPTION
  This API retrieves the global protection settings of memory color adjustments
                                                                  fromm the global PA processing.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  enable    -- State of mode : 0 = disable, 1 = enable
  prot      -- Procesing to Protect.
  flags     -- Reserved

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_get_global_memory_color_protection(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                                    uint32_t *enable, uint32_t *level,
                                                    uint32_t *flags);

/*=========================================================================
FUNCTION
  disp_api_get_global_gamma()

DESCRIPTION
  This API retrieves the global gamma setting.
  hctx      -- Context handle.
  disp_id   -- Display ID type
  gamma     -- Global gamma value
  flags     -- Reserved

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_get_global_gamma(DISPAPI_HANDLE hctx, uint32_t disp_id, global_gamma_type *gamma,
                                  uint32_t *flags);

/*=========================================================================
FUNCTION
  disp_api_set_global_gamma()

DESCRIPTION
  This API sets the global gamma setting.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  gamma     -- Global_gamma value
  flags     -- Reserved

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_set_global_gamma(DISPAPI_HANDLE hctx, uint32_t disp_id, global_gamma_type gamma,
                                  uint32_t flags);

/*=========================================================================
FUNCTION
  disp_api_get_source_dither_strength_config()

DESCRIPTION
  This API retrieves the per source layer dither strength info.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  layer_id  -- Identifies the associated source layer
  enable    -- State of mode : 0 = disable, 1 = enable
  cfg       -- Dither data

  Parameter Usage:
  1: enable = 1 & cfg = non-NULL => enable functionality and apply configuration specified via cfg
  2: enable = 1 & cfg = NULL => enable functionality and apply previous (cached) configuration
  3: enable = 0 & cfg = non-NULL => disable functionality and cache configuration specified via cfg
  4: enable = 0 & cfg = NULL => disable functionality

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_get_source_dither_strength_config(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                                   uint64_t layer_id, uint32_t *enable,
                                                   struct dither_coeff_data *cfg);

/*=========================================================================
FUNCTION
  disp_api_set_source_dither_strength_config()

DESCRIPTION
  This API sets the per source layer dither strength info.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  layer_id  -- Identifies the associated source layer
  enable    -- State of mode : 0 = disable, 1 = enable
  cfg       -- Dither balance table data

  Parameter Usage:
  1: enable = 1 & cfg = non-NULL => enable functionality and apply configuration specified via cfg
  2: enable = 1 & cfg = NULL => enable functionality and apply previous (cached) configuration
  3: enable = 0 & cfg = non-NULL => disable functionality and cache configuration specified via cfg
  4: enable = 0 & cfg = NULL => disable functionality

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_set_source_dither_strength_config(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                                   uint64_t layer_id, uint32_t enable,
                                                   struct dither_coeff_data *cfg);

/*=========================================================================
FUNCTION
  disp_api_get_global_dither_strength_config()

DESCRIPTION
  This API retrieves the per global layer dither strength info.

  hctx      -- Context handle.
  disp_id   -- display ID type
  enable    -- Dtate of mode : 0 = disable, 1 = enable
  cfg       -- Dither data

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_get_global_dither_strength_config(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                                   uint32_t *enable, struct dither_coeff_data *cfg);

/*=========================================================================
FUNCTION
  disp_api_set_global_dither_strength_config()

DESCRIPTION
  This API sets the per global layer dither strength info.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  enable    -- Dtate of mode : 0 = disable, 1 = enable
  cfg       -- Dither data

  Parameter Usage:
  1: enable = 1 & cfg = non-NULL => enable functionality and apply configuration specified via cfg
  2: enable = 1 & cfg = NULL => enable functionality and apply previous (cached) configuration
  3: enable = 0 & cfg = non-NULL => disable functionality and cache configuration specified via cfg
  4: enable = 0 & cfg = NULL => disable functionality

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_set_global_dither_strength_config(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                                   uint32_t enable, struct dither_coeff_data *cfg);

/*=========================================================================
FUNCTION
  disp_api_get_global_pa_dither_config()

DESCRIPTION
  This API retrieves the per global layer dither strength info.

  hctx      -- Context handle.
  disp_id   -- display ID type
  enable    -- Dtate of mode : 0 = disable, 1 = enable
  cfg       -- Dither data

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_get_global_pa_dither_config(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                             uint32_t *enable, struct pa_dither_cfg_data *cfg);

/*=========================================================================
FUNCTION
  disp_api_set_global_pa_dither_config()

DESCRIPTION
  This API sets the per global layer dither strength info.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  enable    -- Dtate of mode : 0 = disable, 1 = enable
  cfg       -- Dither data

  Parameter Usage:
  1: enable = 1 & cfg = non-NULL => enable functionality and apply configuration specified via cfg
  2: enable = 1 & cfg = NULL => enable functionality and apply previous (cached) configuration
  3: enable = 0 & cfg = non-NULL => disable functionality and cache configuration specified via cfg
  4: enable = 0 & cfg = NULL => disable functionality

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_set_global_pa_dither_config(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                             uint32_t enable, struct pa_dither_cfg_data *cfg);

/*=========================================================================
FUNCTION
  disp_api_get_global_memory_color_range()

DESCRIPTION
  This API retrieves the supported global memory color adjustment range information.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  col       -- identify which mc_zone to configure.
  range     -- Memory Color Range information

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_get_global_memory_color_range(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                               disp_mem_col_type col,
                                               struct disp_mem_color_range *range);

/*=========================================================================
FUNCTION
  disp_spi_set_global_memory_color_range()

DESCRIPTION
  This API sets the global memory color adjustment range information.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  col       -- identify which mc_zone to configure.
  range     -- Memory Color Range information

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_set_global_memory_color_range(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                               disp_mem_col_type col,
                                               struct disp_mem_color_range *range);

/*=========================================================================
FUNCTION
  disp_api_get_global_memory_color_config()

DESCRIPTION
  This API retrieves the current global memory color adjustment configuration.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  enable    -- State of memory color region mode : 0 = disable, 1 = enable
  col       -- identify which mc_zone to configure.
  cfg       -- Memory Color configuration information.

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_get_global_memory_color_config(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                                uint32_t *enable, disp_mem_col_type col,
                                                struct disp_mem_color_config *cfg);

/*=========================================================================
FUNCTION
  disp_api_set_global_memory_color_config()

DESCRIPTION
  This API sets the global memory color adjustment configuration.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  enable    -- State of memory color region mode : 0 = disable, 1 = enable
  col       -- identify which mc_zone to configure.
  cfg       -- Memory Color configuration information.

  Parameter Usage:
  1: enable = 1 & cfg = non-NULL => enable functionality and apply configuration specified via cfg
  2: enable = 1 & cfg = NULL => enable functionality and apply previous (cached) configuration
  3: enable = 0 & cfg = non-NULL => disable functionality and cache configuration specified via cfg
  4: enable = 0 & cfg = NULL => disable functionality

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_set_global_memory_color_config(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                                uint32_t enable, disp_mem_col_type col,
                                                struct disp_mem_color_config *cfg);

/*=========================================================================
FUNCTION
  disp_api_set_global_memory_color_config_offset()

DESCRIPTION
  This API sets the current global memory color adjustment configuration.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  col       -- identify which mc_zone to configure.
  cfg       -- Memory Color configuration information.

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_set_global_memory_color_config_offset(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                                       disp_mem_col_type col,
                                                       struct disp_mem_color_config_offset *cfg);

/*=========================================================================
FUNCTION
  disp_api_get_global_memory_color_config_offset()

DESCRIPTION
  This API retrieves the current global memory color adjustment configuration.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  col       -- identify which mc_zone to configure.
  cfg       -- Memory Color configuration information.

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_get_global_memory_color_config_offset(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                                       disp_mem_col_type col,
                                                       struct disp_mem_color_config_offset *cfg);

/*=========================================================================
FUNCTION
  disp_api_get_source_color_info()

DESCRIPTION
  This API retrieves the color properties for the source content.

  This information is not known for all cases, for those cases where the source
  content does not provide the information, the implementation defaults to an
  internal configuration and it is this information that will be returned.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  layer_id  -- Identifies the associated source layer
  info      -- Color Properties Information

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_get_source_color_info(DISPAPI_HANDLE hctx, uint32_t disp_id, uint64_t layer_id,
                                       struct color_info *info);

/*=========================================================================
FUNCTION
  disp_api_set_source_color_info()

DESCRIPTION
  This API sets the color properties for the source content.

  For cases where the source content does not provide property information, the API client
  may wish to override the default settings that the implementation will apply to the content.

  hctx     -- Context handle.
  disp_id  -- Display ID type
  layer_id -- Identifies the associated source layer
  info     -- Color Properties Information

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_set_source_color_info(DISPAPI_HANDLE hctx, uint32_t disp_id, uint64_t layer_id,
                                       struct color_info *info);

/*=========================================================================
FUNCTION
  disp_api_get_global_de_config()

DESCRIPTION
  This API retrieves the per global detailed enhancer programming info.

  hctx      -- Context handle.
  disp_id   -- display ID type
  enable    -- State of mode : 0 = disable, 1 = enable
  cfg       -- Detailed Enhancer data

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_get_global_de_config(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                      uint32_t *enable, struct disp_de_tuning_cfg *cfg);

/*=========================================================================
FUNCTION
  disp_api_set_global_de_config()

DESCRIPTION
  This API sets the per global detailed enhancer.

  hctx      -- Context handle.
  disp_id   -- Display ID type
  enable    -- State of mode : 0 = disable, 1 = enable
  cfg       -- Detailed Enhancer data

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_set_global_de_config(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                      uint32_t enable, struct disp_de_tuning_cfg *cfg);


/*=========================================================================
FUNCTION
  disp_api_get_global_virtual_pcc_config()

DESCRIPTION
  This API retrieves the virtual PCC info

  hctx      -- Context handle.
  disp_id   -- display ID type
  enable    -- State of mode : 0 = disable, 1 = enable
  cfg       -- PCC configuration data

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_get_global_virtual_pcc_config(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                       uint32_t *enable,
                                       struct pcc_coeff_data *cfg);

/*=========================================================================
FUNCTION
  disp_api_set_global_virtual_pcc_config()

DESCRIPTION
  This API sets the virtual PCC Info

  hctx      -- Context handle.
  disp_id   -- display ID type
  enable    -- State of mode : 0 = disable, 1 = enable
  cfg       -- PCC data

  Parameter Usage:
  1: enable = 1 & cfg = non-NULL => enable functionality and apply configuration specified via cfg
  2: enable = 1 & cfg = NULL => enable functionality and apply previous (cached) configuration
  3: enable = 0 & cfg = non-NULL => disable functionality and cache configuration specified via cfg
  4: enable = 0 & cfg = NULL => disable functionality

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_set_global_virtual_pcc_config(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                       uint32_t enable,
                                       struct pcc_coeff_data *cfg);

/*=========================================================================
FUNCTION
  disp_api_get_panel_brightness_info()

DESCRIPTION
  This API retrieves the backlight info from the mode

  hctx      -- Context handle.
  disp_id   -- display ID type
  enable    -- State of mode : 0 = disable, 1 = enable
  cfg       -- mode backlight level configuration data to be filled

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_get_panel_brightness_info(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                       uint32_t *enable,
                                       struct disp_panel_brightness_info *cfg);

/*=========================================================================
FUNCTION
  disp_api_set_panel_brightness_info()

DESCRIPTION
  This API sets the backlight level info to the mode

  hctx      -- Context handle.
  disp_id   -- display ID type
  cfg       -- backlight level info to be set

  Parameter Usage:
  1: enable = 1 & cfg = non-NULL => enable functionality and apply configuration specified via cfg
  2: enable = 1 & cfg = NULL => enable functionality and apply previous (cached) configuration
  3: enable = 0 & cfg = non-NULL => disable functionality and cache configuration specified via cfg
  4: enable = 0 & cfg = NULL => disable functionality

RETURN VALUE
  ZERO (SUCCESS)- success.
  Negative Value - error\failure
=========================================================================*/
int32_t disp_api_set_panel_brightness_info(DISPAPI_HANDLE hctx, uint32_t disp_id,
                                       uint32_t enable,
                                       struct disp_panel_brightness_info *cfg);

#ifdef __cplusplus
}
#endif
#endif  // __DISP_COLOR_APIS_PRIVATE_H__
