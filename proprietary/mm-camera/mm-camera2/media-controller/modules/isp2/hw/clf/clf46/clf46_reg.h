/* clf46_reg.h
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __CLF46_REG_H__
#define __CLF46_REG_H__

#define ISP_CLF46_WB_CFG_OFF 0x0000063C
#define ISP_CLF46_HV_NZ_FLAG_OFF 0x0000065C
#define ISP_CLF46_CFG_LEN 24
#define ISP_CLF46_LEN 16


typedef struct ISP_CLF_NZ_FLAG{
  uint32_t          nz_flag_0           :  2;
  uint32_t          nz_flag_1           :  2;
  uint32_t          nz_flag_2           :  2;
  uint32_t          nz_flag_3           :  2;
  uint32_t          nz_flag_4           :  2;
  uint32_t          nz_flag_5           :  2;
  uint32_t          nz_flag_6           :  2;
  uint32_t          nz_flag_7           :  2;
  uint32_t         /* reserved */       : 16;
} __attribute__((packed, aligned(4))) ISP_CLF_NZ_FLAG;

typedef struct ISP_CLF_COEFF_CFG {
  /* CFG_0 */
  uint32_t          coeff0              :  5;
  uint32_t         /* reserved */       :  3;
  uint32_t          coeff1              :  5;
  uint32_t         /* reserved */       :  3;
  uint32_t          coeff3              :  5;
  uint32_t         /* reserved */       :  3;
  uint32_t          coeff4              :  5;
  uint32_t         /* reserved */       :  3;
  /* CFG_1 */
  uint32_t          coeff2              :  6;
  uint32_t         /* reserved */       :  2;
  uint32_t          coeff5              :  6;
  uint32_t         /* reserved */       :  2;
  uint32_t          coeff6              :  6;
  uint32_t         /* reserved */       :  2;
  uint32_t          coeff7              :  6;
  uint32_t         /* reserved */       :  2;
  /* CFG_2 */
  uint32_t          coeff8              :  7;
  uint32_t         /* reserved */       : 25;
} __attribute__((packed, aligned(4))) ISP_CLF_COEFF_CFG;

typedef struct ISP_CLF_THRESHOLD_CFG {
  /* CFG_0 */
  uint32_t          edge                : 19;
  uint32_t         /* reserved */       : 13;
  /* CFG_1 */
  uint32_t          sat                 : 14;
  uint32_t         /* reserved */       : 18;
  /* CFG_2 */
  uint32_t          rg_hi               : 12;
  uint32_t         /* reserved */       :  4;
  uint32_t          rg_low              : 12;
  uint32_t         /* reserved */       :  4;
  /* CFG_3 */
  uint32_t          bg_hi               : 12;
  uint32_t         /* reserved */       :  4;
  uint32_t          bg_low              : 12;
  uint32_t         /* reserved */       :  4;
} __attribute__((packed, aligned(4))) ISP_CLF_THRESHOLD_CFG;

typedef struct ISP_CLF_INTERP_SLOPE_CFG {
  /* CFG_0 */
  uint32_t          rg_inv              : 10;
  uint32_t         /* reserved */       :  6;
  uint32_t          bg_inv              : 10;
  uint32_t         /* reserved */       :  6;
  /* CFG_1 */
  uint32_t          rg_inv_qfactor      :  5;
  uint32_t         /* reserved */       :  3;
  uint32_t          bg_inv_qfactor      :  5;
  uint32_t         /* reserved */       : 19;
} __attribute__((packed, aligned(4))) ISP_CLF_INTERP_SLOPE_CFG;

typedef struct ISP_CLF_INTERP_THRESH_CFG {
  /* CFG_0 */
  uint32_t          rg_hi               : 10;
  uint32_t         /* reserved */       :  6;
  uint32_t          rg_low              : 10;
  uint32_t         /* reserved */       :  6;
  /* CFG_1 */
  uint32_t          bg_hi               : 10;
  uint32_t         /* reserved */       :  6;
  uint32_t          bg_low              : 10;
  uint32_t         /* reserved */       :  6;
} __attribute__((packed, aligned(4))) ISP_CLF_INTERP_THRESH_CFG;

typedef struct ISP_CLF_CmdType {
  /* CLF_EDGE_HV_NZ_FLAG */
  ISP_CLF_NZ_FLAG           edge_hv_nz_flag;
  /* CLF_EDGE_DIAG_NZ_FLAG */
  ISP_CLF_NZ_FLAG           edge_diag_nz_flag;
  /* CLF_EDGE_HV_COEFF_CFG */
  ISP_CLF_COEFF_CFG         edge_hv_coeff;
  /* CLF_EDGE_DIAG_COEFF_CFG */
  ISP_CLF_COEFF_CFG         edge_diag_coeff;
  /* CLF_THRESHOLD_CFG */
  ISP_CLF_THRESHOLD_CFG     threshold;
  /* CLF_INTERP_SLOPE_CFG */
  ISP_CLF_INTERP_SLOPE_CFG  interp_slope;
  /* CLF_INTERP_THRESH_CFG */
  ISP_CLF_INTERP_THRESH_CFG interp_thresh;
}__attribute__((packed, aligned(4))) ISP_CLF_CmdType;

#endif //__CLF46_REG_H__
