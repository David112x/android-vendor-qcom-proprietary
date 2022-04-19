/**********************************************************************
*  Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __EDGE_SMOOTH_PARAMS_H__
#define __EDGE_SMOOTH_PARAMS_H__

#include "img_common.h"

/** edge_smooth_cfg_t:
 *
 *   @flat_thr: Threshold Value For Flat Area Decision
 *   @texture_thr: Threshold Value for texture Detection
 *   @similarity_thr: Threshold Value For corner Detection
 *   @vStrength: Control Factor For vertical smoothening
 *   @zStrength: Control Factor For z diagonal smoothening
 *   @nStrength: Control Factor For n diagonal smoothening
 *   @hStrength: Control Factor For horizontal smoothening
 *   @layer_2_flatThr: Layer 2 Threshold Value For Flat Area
 *                   Decision
 *   @layer_2_textureThr: Layer 2 Threshold Value for texture
 *                      Detection
 *
 *   Structure to hold edge smooth configuration
 */
typedef struct __attribute__((__packed__))
{
    // 0 ~ 255 (default 8)
    uint32_t             flat_thr;
    // 0 ~ 255 (default 8)
    uint32_t             texture_thr;
    // 1024 ~ 4086 (default 1382)
    uint16_t               similarity_thr;

    /* Control Factor For Edge Smoothing Strength:: 0 to 1024, default 1024 */
    uint16_t               vStrength;      // vertical
    uint16_t               zStrength;      // z diagonal
    uint16_t               nStrength;      // n diagonal
    uint16_t               hStrength;      // horizontal

    /* Range : 0-255, default 4  */
    uint32_t             layer_2_flatThr;
    /* Range : 0-255, default 4 */
    uint32_t             layer_2_textureThr;

} edge_smooth_cfg_t;

/** img_edge_smooth_meta_t:
 * @config: edge smooth configuration
 *
 *   Structure to hold edge smooth specific meta
 */
typedef struct __attribute__((__packed__))  {
  edge_smooth_cfg_t config;
} img_edge_smooth_meta_t;

#endif //__EDGE_SMOOTH_PARAMS_H__
