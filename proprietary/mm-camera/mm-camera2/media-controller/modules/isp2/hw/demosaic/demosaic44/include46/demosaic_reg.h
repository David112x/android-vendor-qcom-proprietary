/* demosaic_reg.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __DEMOSAIC_REG_H__
#define __DEMOSAIC_REG_H__

#define ISP_DEMOSAIC_CLASSIFIER_CNT 18

#define ISP_DEMOSAIC_MIX_CFG_OFF 0x000005DC
#define ISP_DEMOSAIC_ENABLE_LEN 1

#define ISP_DEMOSAIC_WB_GAIN_OFF 0x000005E0
#define ISP_DEMOSAIC_WB_GAIN_LEN 2

#define ISP_DEMOSAIC_CLASSIFIER_OFF 0x000005E8
#define ISP_DEMOSAIC_CLASSIFIER_LEN 18

#define ISP_DEMOSAIC_INTERP_GAIN_OFF 0x00000630
#define ISP_DEMOSAIC_INTERP_GAIN_LEN 2

#define CFG_VALID_BITS 0x00000F13

typedef struct ISP_DemosaicInterpClassifierType {
  uint32_t w_n                             : 10;
  uint32_t   /* reserved */                : 2;
  uint32_t t_n                             : 12;
  uint32_t l_n                             : 5;
  uint32_t   /* reserved */                : 2;
  uint32_t b_n                             : 1;
}__attribute__((packed, aligned(4))) ISP_DemosaicInterpClassifierType;

typedef struct ISP_DemosaicMixConfigCmdType{

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

  /* center clamp exclusion bit */
  uint32_t dynGClampCDisable               : 1;
  uint32_t dynRBClampCDisable              : 1;

  /* reserved 12-31 */ /* Dummy Variables in 8994*/
  uint32_t   /* reserved */                : 3;
  uint32_t demoV4Enable /* reserved*/      : 1;

  /* pipe flush count */
  uint32_t pipeFlushCount                  : 13;
  uint32_t   /* reserved */                : 1;
  uint32_t pipeFlushOvd                    : 1;
  uint32_t flushHaltOvd                    : 1;

}__attribute__((packed, aligned(4))) ISP_DemosaicMixConfigCmdType;

typedef struct ISP_DemosaicConfigCmdType {
  /* Interp WB Gain 0 */
  uint32_t rgWbGain                        : 9;
  uint32_t wgr1                            : 3;
  uint32_t wgr2                            : 3;
  uint32_t bgWbGain                        : 9;
  uint32_t lambdaG                         : 8;

  /* Interp WB Gain 1 */
  uint32_t grWbGain                        : 9;
  uint32_t   /* reserved */                : 6;
  uint32_t gbWbGain                        : 9;
  uint32_t lambdaRB                        : 8;

  /* Interp Classifier */
  ISP_DemosaicInterpClassifierType
    interpClassifier[ISP_DEMOSAIC_CLASSIFIER_CNT];

  /* Interp G 0 */
  uint32_t bl                              : 8;
  uint32_t bu                              : 8;
  uint32_t   /* reserved */                : 2;
  uint32_t noiseLevelG                     : 14;

  /* Interp G 1 */
  uint32_t dblu                            : 9;
  uint32_t   /* reserved */                : 9;
  uint32_t noiseLevelRB                    : 14;

  /*Dummy Variable for 8994 */
  uint32_t a                                :32;
}__attribute__((packed, aligned(4))) ISP_DemosaicConfigCmdType;

#endif /* __DEMOSAIC46_REG_H__ */
