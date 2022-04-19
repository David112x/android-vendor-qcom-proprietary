/* mesh_rolloff_reg.h
 *
 * Copyright (c)2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MESH_ROLLOFF_REG_H__
#define __MESH_ROLLOFF_REG_H__

#define Y_DELTA_Q_LEN 20
#define X_DELTA_Q_LEN 20

#define ISP_MESH_ROLLOFF_CFG_OFF             0x000006BC
#define ISP_MESH_ROLLOFF_CFG_LEN             11

#define ISP_MESH_ROLLOFF_TABLE_SIZE          13 * 17
#define ISP_MESH_ROLLOFF_MIN_STRIPE_OVERLAP    36
#define ISP_MESH_ROLLOFF_MIN_DIST_TO_SUBGRID 18

#define ROLLOFF_CGC_OVERRIDE TRUE
#define ROLLOFF_CGC_OVERRIDE_REGISTER 0x2C
#define ROLLOFF_CGC_OVERRIDE_BIT 8

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
  uint32_t                     pixelOffset             : 12;
  uint32_t                     /* reserved */          : 4;
  uint32_t                     pcaLutBankSel           : 1;
  uint32_t                     /* reserved */          : 1;
  uint32_t                     num_meshgain_h          : 6;
  uint32_t                     num_meshgain_v          : 6;
  uint32_t                     /* reserved */          : 2;
   /* ISP_ROLLOFF_GRID_CFG_0 */
  uint32_t                     blockWidth              : 11;
  uint32_t                     /* reserved */          : 5;
  uint32_t                     blockHeight             : 10;
  uint32_t                     /* reserved */          : 6;
  /* ISP_ROLLOFF_GRID_CFG_1 */
  uint32_t                     subGridHeight           : 10;
  uint32_t                     /* reserved */          : 2;
  uint32_t                     subGridYDelta           : 17;
  uint32_t                     /* reserved */          : 1;
  uint32_t                     interpFactor            : 2;
    /* ISP_ROLLOFF_GRID_CFG_2 */
  uint32_t                     subGridWidth            : 11;
  uint32_t                     /* reserved */          : 1;
  uint32_t                     subGridXDelta           : 17;
  uint32_t                     /* reserved */          : 3;
  /* ISP_ROLLOFF_RIGHT_GRID_CFG_0 */
  uint32_t                     blockWidthRight         : 11;
  uint32_t                     /* reserved */          : 5;
  uint32_t                     blockHeightRight        : 10;
  uint32_t                     /* reserved */          : 6;
  /* ISP_ROLLOFF_RIGHT_GRID_CFG_1 */
  uint32_t                      subGridHeightRight     : 10;
  uint32_t                     /* reserved */          : 2;
  uint32_t                     subGridYDeltaRight      : 17;
  uint32_t                     /* reserved */          : 1;
  uint32_t                     interpFactorRight       : 2;
  /* ISP_ROLLOFF_RIGHT_GRID_CFG_2 */
  uint32_t                      subGridWidthRight      : 11;
  uint32_t                      /* reserved */         : 1;
  uint32_t                     subGridXDeltaRight      : 17;
  uint32_t                      /* reserved */         : 3;
  /* ISP_ROLLOFF_STRIPE_CFG_0 */
  uint32_t                     blockXIndex             : 6;
  uint32_t                     blockYIndex             : 6;
  uint32_t                     yDeltaAccum             : 20;
  /* ISP_ROLLOFF_STRIPE_CFG_1 */
  uint32_t                     PixelXIndex             : 11;
  uint32_t                     /* reserved */          : 1;
  uint32_t                     PixelYIndex             : 10;
  uint32_t                     /* reserved */          : 2;
  uint32_t                     subGridXIndex           : 3;
  uint32_t                     /* reserved */          : 1;
  uint32_t                     subGridYIndex           : 3;
  uint32_t                     /* reserved */          : 1;
  /* ISP_ROLLOFF_STRIPE_RIGHT_CFG_0 */
  uint32_t                     blockXIndexRight        : 6;
  uint32_t                     blockYIndexRight        : 6;
  uint32_t                     yDeltaAccumRight        : 20;
  /* ISP_ROLLOFF_STRIPE_RIGHT_CFG_1 */
  uint32_t                     PixelXIndexRight        : 11;
  uint32_t                     /* reserved */          : 1;
  uint32_t                     PixelYIndexRight        : 10;
  uint32_t                     /* reserved */          : 2;
  uint32_t                     subGridXIndexRight      : 3;
  uint32_t                     /* reserved */          : 1;
  uint32_t                     subGridYIndexRight      : 3;
  uint32_t                     /* reserved */          : 1;
}__attribute__((packed, aligned(4))) MESH_RollOff_v4_ConfigParams;

#endif /* __MESH_ROLLOFF_REG_H__ */
