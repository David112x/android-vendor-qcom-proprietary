/* linearization_reg.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __LINEARIZATION_REG_H__
#define __LINEARIZATION_REG_H__

#define ISP_LINEARIZATION_OFF 0x00000408
#define ISP_LINEARIZATION_LEN 17

#define LINEAR_MAX_VAL 16383
#define DELATQ_FACTOR 11

#define DMI_WRITE_MODE VFE_WRITE_DMI_64BIT
#define DMI_WRITE_TYPE uint64_t
#define LINEAR_TABLE_WIDTH 64
#define ISP32_LINEARIZATON_TABLE_LENGTH 36
#define LINEAR_TABLE_FORMAT "%llx"
#define LINEAR_CGC_OVERRIDE TRUE
#define LINEAR_CGC_OVERRIDE_REGISTER 0x2C
#define LINEAR_CGC_OVERRIDE_BIT 1


/** ISP_PointSlopeData:
 *
 *  @kneePoint_P1: knee point P1
 *  @kneePoint_P0: knee point P0
 *  @kneePoint_P3: knee point P3
 *  @kneePoint_P2: knee point P2
 *  @kneePoint_P5: knee point P5
 *  @kneePoint_P4: knee point P4
 *  @kneePoint_P7: knee point P7
 *  @kneePoint_P6: knee point P6
 **/
typedef struct ISP_PointSlopeData {
  /* INTERP_0 */
  uint32_t kneePoint_P1         :14;
  uint32_t /* reserved */       : 2;
  uint32_t kneePoint_P0         :14;
  uint32_t /* reserved */       : 2;
  /* INTERP_1 */
  uint32_t kneePoint_P3         :14;
  uint32_t /* reserved */       : 2;
  uint32_t kneePoint_P2         :14;
  uint32_t /* reserved */       : 2;
  /* INTERP_2 */
  uint32_t kneePoint_P5         :14;
  uint32_t /* reserved */       : 2;
  uint32_t kneePoint_P4         :14;
  uint32_t /* reserved */       : 2;
  /* INTERP_3 */
  uint32_t kneePoint_P7         :14;
  uint32_t /* reserved */       : 2;
  uint32_t kneePoint_P6         :14;
  uint32_t /* reserved */       : 2;
}__attribute__((packed, aligned(4))) ISP_PointSlopeData;

/** ISP_LinearizationCfgParams:
 *
 *  @lutBankSel: lut bank sel
 *  @pointSlopeR: R slope
 *  @pointSlopeGb: gb slope
 *  @pointSlopeB: B slope
 *  @pointSlopeGr: Gr slope
 **/
typedef struct ISP_LinearizationCfgParams {
  /* BLACK_CONFIG */
  uint32_t           lutBankSel           : 1;
  uint32_t /* reserved */       :31;

  /* Knee points for R channel */
  ISP_PointSlopeData pointSlopeR;

  /* Knee points for Gr channel */
  ISP_PointSlopeData pointSlopeGb;

  /* Knee points for B channel */
  ISP_PointSlopeData pointSlopeB;

  /* Knee points for Gb channel */
  ISP_PointSlopeData pointSlopeGr;

}__attribute__((packed, aligned(4))) ISP_LinearizationCfgParams;



/** ISP_LinearizationCfgTable:
 *
 *  @Lut: LUT table
 **/
typedef struct VFE_LinearizationCfgTable {
  uint64_t Lut[ISP32_LINEARIZATON_TABLE_LENGTH];
}ISP_LinearizationCfgTable;


#endif /* __LINEARIZATION_REG_H__ */
