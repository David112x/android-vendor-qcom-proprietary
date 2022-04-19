/* linearization_reg.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __LINEARIZATION_REG_H__
#define __LINEARIZATION_REG_H__

#define ISP_LINEARIZATION_OFF 0x0000037C
#define ISP_LINEARIZATION_LEN 17

#define LINEAR_MAX_VAL 4095
#define DELATQ_FACTOR 9
#define DMI_WRITE_MODE VFE_WRITE_DMI_32BIT
#define DMI_WRITE_TYPE uint32_t

#define LINEAR_TABLE_WIDTH 32
#define ISP32_LINEARIZATON_TABLE_LENGTH 36
#define LINEAR_TABLE_FORMAT "%x"

#define LINEAR_CGC_OVERRIDE FALSE
#define LINEAR_CGC_OVERRIDE_REGISTER 0
#define LINEAR_CGC_OVERRIDE_BIT 0

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
  uint32_t kneePoint_P1         :12;
  uint32_t /* reserved */       : 4;
  uint32_t kneePoint_P0         :12;
  uint32_t /* reserved */       : 4;
  /* INTERP_1 */
  uint32_t kneePoint_P3         :12;
  uint32_t /* reserved */       : 4;
  uint32_t kneePoint_P2         :12;
  uint32_t /* reserved */       : 4;
  /* INTERP_2 */
  uint32_t kneePoint_P5         :12;
  uint32_t /* reserved */       : 4;
  uint32_t kneePoint_P4         :12;
  uint32_t /* reserved */       : 4;
  /* INTERP_3 */
  uint32_t kneePoint_P7         :12;
  uint32_t /* reserved */       : 4;
  uint32_t kneePoint_P6         :12;
  uint32_t /* reserved */       : 4;
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
  uint32_t Lut[ISP32_LINEARIZATON_TABLE_LENGTH];
}ISP_LinearizationCfgTable;


#endif /* __LINEARIZATION_REG_H__ */
