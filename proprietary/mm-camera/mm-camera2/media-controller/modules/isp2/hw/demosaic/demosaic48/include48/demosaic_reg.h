/* demosaic_reg.h
 *
 * Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __DEMOSAIC_REG_H__
#define __DEMOSAIC_REG_H__

#define ISP_DEMOSAIC_CFG_OFF 0x000006F8
#define ISP_DEMOSAIC_CFG_LEN 1

#define ISP_DEMOSAIC_WB_OFF 0x000006FC
#define ISP_DEMOSAIC_WB_LEN 8

#define ISP_DEMOSAIC_INTERP_COEFF_OFF 0x0000071C
#define ISP_DEMOSAIC_INTERP_COEFF_LEN 1

#define ISP_DEMOSAIC_CLASSIFIER_OFF 0x00000720
#define ISP_DEMOSAIC_CLASSIFIER_LEN 1

#define CFG_VALID_BITS 0x00000F13


/* Important note:
   These structures represent demosaic HW module registers.
   Please do not arbitrary add/remove fields or modify
   their types or their order */

typedef struct ISP_DemosaicWBCfg0Type {
  uint32_t g_gain                          : 12;
  uint32_t   /* reserved */                : 4;
  uint32_t b_gain                          : 12;
  uint32_t   /* reserved */                : 4;
}__attribute__((packed, aligned(4))) ISP_DemosaicWBCfg0Type;

typedef struct ISP_DemosaicWBCfg1Type {
  uint32_t r_gain                          : 12;
  uint32_t   /* reserved */                : 20;
}__attribute__((packed, aligned(4))) ISP_DemosaicWBCfg1Type;

typedef struct ISP_DemosaicWBOffsetCfg0Type {
  uint32_t g_offset                        : 15;
  uint32_t   /* reserved */                : 1;
  uint32_t b_offset                        : 15;
  uint32_t   /* reserved */                : 1;
}__attribute__((packed, aligned(4))) ISP_DemosaicWBOffsetCfg0Type;

typedef struct ISP_DemosaicWBOffsetCfg1 {
  uint32_t r_offset                        : 15;
  uint32_t   /* reserved */                : 17;
}__attribute__((packed, aligned(4))) ISP_DemosaicWBOffsetCfg1Type;

typedef struct ISP_DemosaicInterpCoeffCfgType {
  uint32_t   /* reserved */                : 8;
  uint32_t lambda_g                        : 8;
  uint32_t   /* reserved */                : 8;
  uint32_t lambda_rb                       : 8;
}__attribute__((packed, aligned(4))) ISP_DemosaicInterpCoeffCfgType;

typedef struct ISP_DemosaicInterpClassifierType {
  uint32_t w_n                             : 10;
  uint32_t   /* reserved */                : 10;
  uint32_t a_n                             : 12;
}__attribute__((packed, aligned(4))) ISP_DemosaicInterpClassifierType;

typedef struct ISP_DemosaicConfigType{

  /* directional interpolation disable */
  uint32_t dirGInterpDisable               : 1;
  uint32_t dirRBInterpDisable              : 1;
  uint32_t  /*reserve*/                    : 2;

   /* cosited rgb enable */
  uint32_t cositedRgbEnable                : 1;
  uint32_t   /* reserved */                : 3;

  /* dynamic clamp enable */
  uint32_t dynGClampEnable                 : 1;
  uint32_t dynRBClampEnable                : 1;
  uint32_t   /* reserved */                : 1;

  /* line buffer enable bit */
  uint32_t lbOnlyEnable                      : 1;
  uint32_t   /* reserved */                : 20;

}__attribute__((packed, aligned(4))) ISP_DemosaicConfigType;

typedef struct ISP_DemosaicConfigCmdType {
  /* General configuration */
  ISP_DemosaicConfigType          demoCfg;

  /* Left WB CFG */
  ISP_DemosaicWBCfg0Type          leftWBCfg0;
  ISP_DemosaicWBCfg1Type          leftWBCfg1;
  ISP_DemosaicWBOffsetCfg0Type    leftWBOffsetCfg0;
  ISP_DemosaicWBOffsetCfg1Type    leftWBOffsetCfg1;

  /* Right WB CFG */
  ISP_DemosaicWBCfg0Type          rightWBCfg0;
  ISP_DemosaicWBCfg1Type          rightWBCfg1;
  ISP_DemosaicWBOffsetCfg0Type    rightWBOffsetCfg0;
  ISP_DemosaicWBOffsetCfg1Type    rightWBOffsetCfg1;


  /* Interp Coeff */
  ISP_DemosaicInterpCoeffCfgType  interpCoeffCfg;

  /* Interp Classifier */
  ISP_DemosaicInterpClassifierType interpClassifier;

}__attribute__((packed, aligned(4))) ISP_DemosaicConfigCmdType;

#endif /* __DEMOSAIC46_REG_H__ */
