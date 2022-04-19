#ifndef __3A_STATS_DATA_TYPES_H__
#define __3A_STATS_DATA_TYPES_H__

/*===========================================================================

         AWBDebugData D a t a  S t r u c t u r e  D e c l a r a t i o n

*//** @file 3AStatsDataTypes.h
  This header file contains the format of the 3A stats data.


* Copyright (c) 2014-2016 Qualcomm Technologies, Inc. All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

/* ==========================================================================

                             Edit History






when       who     what, where, why
--------   ---     -------------------------------------------------------
03/27/14   vb      Initial Revision

========================================================================== */

/* ==========================================================================
                     INCLUDE FILES FOR MODULE
========================================================================== */
#include "3ADebugDataTypes.h"

/* ==========================================================================
                       Preprocessor Definitions and Constants
========================================================================== */
/// Max AWB stats size
#define BAYER_GRID_NUM_REGIONS (3072)

/// Number of bayer histogram bins.
#define BAYER_HISTOGRAM_NUM_BINS (4096)
/// Max Bayer Exposure stats size
#define BAYER_EXPOSURE_NUM_REGIONS (3072)

/* ==========================================================================
                       Static Declarations
========================================================================== */


/* ==========================================================================
                       Type Declarations
========================================================================== */
#include "3ADebugDataPackStart.h"

/// This struct is used to define the Bayer Grid Stats
typedef struct PACKED
{
    /// Number of horizontal regions for BG stats.
    uint16                          bgStatsNumHorizontalRegions;

    /// Number of vertical regions for BG stats.
    uint16                          bgStatsNumVerticalRegions;

    /// Red channel sum.
    uint32                          redChannelSum[BAYER_GRID_NUM_REGIONS];

    /// Red channel counts per region.
    uint16                          redChannelCount[BAYER_GRID_NUM_REGIONS];

    /// Gr channel sum.
    uint32                          grChannelSum[BAYER_GRID_NUM_REGIONS];

    /// Gr channel counts per region.
    uint16                          grChannelCount[BAYER_GRID_NUM_REGIONS];

    /// Gb channel sum.
    uint32                          gbChannelSum[BAYER_GRID_NUM_REGIONS];

    /// Gb channel count.
    uint16                          gbChannelCount[BAYER_GRID_NUM_REGIONS];

    /// Blue channel sum.
    uint32                          blueChannelSum[BAYER_GRID_NUM_REGIONS];

    /// Blue channel sum.
    uint16                          blueChannelCount[BAYER_GRID_NUM_REGIONS];

} BayerGridStatsType;


/// This struct is used to define the Bayer Stats Configuration.
typedef struct PACKED
{
  /// Horizontal offset ratio in Q20 format.
  uint32 horizonOffsetRatio;

  /// Vertical offset ratio in Q20 format.
  uint32 verticalOffsetRatio;

  /// Horizontal window ratio in Q20 format.
  uint32 horizonWindowRatio;

  /// Vertical window ration in Q20 format.
  uint32 verticalWindowRatio;

  /// Bit depth
  uint8 bitDepth;
} BayerStatsConfigType;

/// This struct is used to define the Bayer Exposure Stats
typedef struct PACKED
{
    /// Number of horizontal regions for BE stats.
    uint16                          NumHorizontalRegions;

    /// Number of vertical regions for BE stats.
    uint16                          NumVerticalRegions;

    /// Red channel sum.
    uint32                          redChannelSum[BAYER_EXPOSURE_NUM_REGIONS];

    /// Red channel count.
    uint16                          redChannelCount[BAYER_EXPOSURE_NUM_REGIONS];

    /// Gr channel sum.
    uint32                          grChannelSum[BAYER_EXPOSURE_NUM_REGIONS];

    /// Gr channel count.
    uint16                          grChannelCount[BAYER_EXPOSURE_NUM_REGIONS];

    /// Gb channel sum.
    uint32                          gbChannelSum[BAYER_EXPOSURE_NUM_REGIONS];

    /// Gb channel count.
    uint16                          gbChannelCount[BAYER_EXPOSURE_NUM_REGIONS];

    /// Blue channel sum.
    uint32                          blueChannelSum[BAYER_EXPOSURE_NUM_REGIONS];

    /// Blue channel count.
    uint16                          blueChannelCount[BAYER_EXPOSURE_NUM_REGIONS];

    /// Saturation information present flag.
    uint8                           saturationInfoPresent;

    /// Red channel saturation sum.
    uint32                          redChannelSatSum[BAYER_EXPOSURE_NUM_REGIONS];

    /// Red channel saturation count.
    uint16                          redChannelSatCount[BAYER_EXPOSURE_NUM_REGIONS];

    /// Gr channel saturation sum.
    uint32                          grChannelSatSum[BAYER_EXPOSURE_NUM_REGIONS];

    /// Gr channel saturation count.
    uint16                          grChannelSatCount[BAYER_EXPOSURE_NUM_REGIONS];

    /// Gb channel saturation sum.
    uint32                          gbChannelSatSum[BAYER_EXPOSURE_NUM_REGIONS];

    /// Gb channel saturation count.
    uint16                          gbChannelSatCount[BAYER_EXPOSURE_NUM_REGIONS];

    /// Blue channel saturation sum.
    uint32                          blueChannelSatSum[BAYER_EXPOSURE_NUM_REGIONS];

    /// Blue channel saturation count.
    uint16                          blueChannelSatCount[BAYER_EXPOSURE_NUM_REGIONS];

}BayerExposureStatsType;

/// Enumeration that defines the types of HDR modes that are possible.
typedef enum
{
  HDR_MODE_NONE = 0,
  HDR_MODE_LONG,
  HDR_MODE_SHORT,

  HDR_MODE_MAX = 0xFF

} HDRModeEnumType;

/// This struct is used to define the Bayer Histogram stats data.
typedef struct PACKED
{
    /// Maximum number of histogram bins supported.
    uint16 maxBinCount;

    /// Actual number of histogram bins used.
    uint16 binsUsed;

    /// HDR mode that has been used.
    uint8 hdrMode;

    /// Red channel histogram stats.
    uint32 rChannel[BAYER_HISTOGRAM_NUM_BINS];

    /// Gr channel histogram stats.
    uint32 grChannel[BAYER_HISTOGRAM_NUM_BINS];

    /// Gb channel histogram stats.
    uint32 gbChannel[BAYER_HISTOGRAM_NUM_BINS];

    /// Blue channel histogram stats.
    uint32 bChannel[BAYER_HISTOGRAM_NUM_BINS];

} BayerHistogramStatsType;


#include "3ADebugDataPackEnd.h"

#endif // __3A_STATS_DATA_TYPES_H__

