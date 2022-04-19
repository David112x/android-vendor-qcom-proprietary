/* clf44_reg.h
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __CLF44_REG_H__
#define __CLF44_REG_H__

#define ISP_CLF44_CFG_OFF_1 0x00000588
#define ISP_CLF44_CFG_LEN_1 1
#define ISP_CLF44_CFG_OFF_2 0x000005AC
#define ISP_CLF44_CFG_LEN_2 9
#define ISP_CLF44_CFG_OFF_3 0x00000A18
#define ISP_CLF44_CFG_LEN_3 7
#define ISP_CLF44_CFG_CFG_MASK 0x31FFF0

typedef struct ISP_CLF_Cfg {
  uint32_t       cac_filter_off         :   1;
  uint32_t    /* reserved */            :   3;
  uint32_t       pipe_flush_cnt         :  13;
  uint32_t    /* reserved */            :   3;
  uint32_t       pipe_flush_ovd         :   1;
  uint32_t       flush_halt_ovd         :   1;
  uint32_t    /* reserved */            :  10;

}__attribute__((packed, aligned(4))) ISP_CLF_Cfg;

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
  uint32_t          edge                : 17;
  uint32_t         /* reserved */       : 15;
  /* CFG_1 */
  uint32_t          sat                 : 12;
  uint32_t         /* reserved */       : 20;
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
  /* CLF_CFG */
  ISP_CLF_Cfg               cfg;
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

#endif //__CLF44_REG_H__
