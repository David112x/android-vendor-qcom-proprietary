/* mesh_rolloff_reg.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MESH_ROLLOFF_REG_H__
#define __MESH_ROLLOFF_REG_H__

#define Y_DELTA_Q_LEN 13
#define X_DELTA_Q_LEN 20

#define ISP_MESH_ROLLOFF_CFG_OFF             0x00000400
#define ISP_MESH_ROLLOFF_CFG_LEN             9

#define ISP_MESH_ROLLOFF40_TABLE_SIZE          13*10
#define ROLLOFF_CGC_OVERRIDE FALSE
#define ROLLOFF_CGC_OVERRIDE_REGISTER 0x2C
#define ROLLOFF_CGC_OVERRIDE_BIT 8
#define ISP_MESH_ROLLOFF_MIN_STRIPE_OVERLAP    32
#define ISP_MESH_ROLLOFF_MIN_DIST_TO_SUBGRID 16

/** MESH_RollOff_v4_ConfigParams:
 *
 *  @pixelOffset: pixel offset to add higher dynamic range to
 *              the output pixels
 *  @pcaLutBankSel: PCA based LUT bank select
 *  @num_meshgain_h: number of horizontal grids for a line
 *  @num_meshgain_v: number of vertical grids for a line
 *  @blockWidth: horizontal separation between adjacents
 *             points in the block in terms of pairs of
 *             pixels. N is programmed as N/2 -1
 *  @blockHeight: vertical separation between adjacents
 *             points in the block in terms of pairs of
 *             pixels. N is programmed as N/2 -1
 *  @subGridXDelta: sub grid X delta
 *  @subGridYDelta: sub grid Y delta
 *  @interpFactor: bicubic interpolation factor
 *  @subGridWidth: sub grid width
 *  @subGridHeight: sub grid height
 *  @blockWidthRight: blockWidthRight
 *  @blockHeightRight: blockHeightRight
 *  @subGridXDeltaRight: subGridXDeltaRight
 *  @subGridYDeltaRight: subGridYDeltaRight
 *  @interpFactorRight: interpFactorRight
 *  @subGridWidthRight: subGridWidthRight
 *  @subGridHeightRight: subGridHeightRight
 *  @blockXIndex: blockXIndex
 *  @blockYIndex: blockYIndex
 *  @PixelXIndex: PixelXIndex
 *  @PixelYIndex: PixelYIndex
 *  @yDeltaAccum: yDeltaAccum
 *  @subGridXIndex: subGridXIndex
 *  @subGridYIndex: subGridYIndex
 **/
typedef struct MESH_RollOff_v4_ConfigParams {
  /* ISP_ROLLOFF_CONFIG */
  uint32_t                     pixelOffset             : 9;
  uint32_t                     /* reserved */          : 7;
  uint32_t                     pcaLutBankSel           : 1;
  uint32_t                     /* reserved */          : 15;
   /* ISP_ROLLOFF_GRID_CFG_0 */
  uint32_t                     blockWidth              : 9;
  uint32_t                     blockHeight             : 9;
  uint32_t                     /* reserved */          : 14;
  /* ISP_ROLLOFF_GRID_CFG_1 */
  uint32_t                     subGridXDelta           : 17;
  uint32_t                     /* reserved */          : 3;
  uint32_t                     subGridYDelta           : 10;
  uint32_t                     interpFactor            : 2;
    /* ISP_ROLLOFF_GRID_CFG_2 */
  uint32_t                     subGridWidth            : 9;
  uint32_t                     subGridHeight           : 9;
  uint32_t                     /* reserved */          : 14;
  /* ISP_ROLLOFF_RIGHT_GRID_CFG_0 */
  uint32_t                     blockWidthRight         : 9;
  uint32_t                     blockHeightRight        : 9;
  uint32_t                     /* reserved */          : 14;
  /* ISP_ROLLOFF_RIGHT_GRID_CFG_1 */
  uint32_t                     subGridXDeltaRight      : 17;
  uint32_t                     /* reserved */          : 3;
  uint32_t                     subGridYDeltaRight      : 10;
  uint32_t                     interpFactorRight       : 2;
  /* ISP_ROLLOFF_RIGHT_GRID_CFG_2 */
  uint32_t                      subGridWidthRight      : 9;
  uint32_t                      subGridHeightRight     : 9;
  uint32_t                      /* reserved */         : 14;
  /* ISP_ROLLOFF_STRIPE_CFG_0 */
  uint32_t                     blockXIndex             : 4;
  uint32_t                     blockYIndex             : 4;
  uint32_t                     PixelXIndex             : 9;
   uint32_t                     /* reserved */          : 3;
  uint32_t                     PixelYIndex             : 9;
  uint32_t                     /* reserved */          : 3;
  /* ISP_ROLLOFF_STRIPE_CFG_1 */
  uint32_t                     yDeltaAccum             : 13;
  uint32_t                     /* reserved */          : 3;
  uint32_t                     subGridXIndex           : 3;
  uint32_t                     /* reserved */          : 5;
  uint32_t                     subGridYIndex           : 3;
  uint32_t                     /* reserved */          : 5;
}__attribute__((packed, aligned(4))) MESH_RollOff_v4_ConfigParams;

#endif /* __MESH_ROLLOFF_REG_H__ */
