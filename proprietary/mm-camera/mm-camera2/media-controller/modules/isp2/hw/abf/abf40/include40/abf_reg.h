/*============================================================================

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __ABF40_REG_H__
#define __ABF40_REG_H__

#define ISP_ABF_DEMOSAIC_MIX_CFG_OFF 0x00000440

#define ISP_ABF2_OFF 0x00000464
#define ISP_ABF2_LEN 45
#define ABF_CGC_OVERRIDE FALSE
#define ABF_CGC_OVERRIDE_REGISTER 0x0
#define ABF_CGC_OVERRIDE_BIT 0

typedef struct ISP_DemosaicABF2_GCfg {
  /* Demosaic ABF Green Config 0 */
  uint32_t       Cutoff1                : 12;
  uint32_t     /* reserved */           :  4;
  uint32_t       Cutoff2                : 12;
  uint32_t     /* reserved */           :  4;
  /* Demosaic ABF Green Config 1 */
  uint32_t       Cutoff3                : 12;
  uint32_t     /* reserved */           :  4;
  uint32_t       SpatialKernelA0        :  7;
  uint32_t     /* reserved */           :  1;
  uint32_t       SpatialKernelA1        :  7;
  uint32_t     /* reserved */           :  1;
  /* Demosaic ABF Green Config 2 */
  uint32_t       MultNegative           : 12;
  uint32_t     /* reserved */           :  4;
  uint32_t       MultPositive           : 12;
  uint32_t     /* reserved */           :  4;
}__attribute__((packed, aligned(4))) ISP_DemosaicABF2_GCfg;

typedef struct ISP_DemosaicABF2_Lut {
  /* Demosaic ABF LUT */
  int32_t       LUT0                    : 12;
  int32_t     /* reserved */            :  4;
  int32_t       LUT1                    : 12;
  int32_t     /* reserved */            :  4;
}__attribute__((packed, aligned(4)))  ISP_DemosaicABF2_Lut;

typedef struct ISP_DemosaicABF2_RBCfg {
  /* Demosaic ABF Blue/Red Config 0 */
  uint32_t       Cutoff1                 : 12;
  uint32_t     /* reserved */            :  4;
  uint32_t       Cutoff2                 : 12;
  uint32_t     /* reserved */            :  4;
  /* Demosaic ABF Blue/Red Config 1 */
  uint32_t       Cutoff3                 : 12;
  uint32_t     /* reserved */            :  6;
  uint32_t       gain_offset             : 13;
  uint32_t     /* reserved */            :  1;

  /* Demosaic ABF Blue/Red Config 2 */
  uint32_t       MultNegative            : 12;
  uint32_t     /* reserved */            :  4;
  uint32_t       MultPositive            : 12;
  uint32_t     /* reserved */            :  4;
}__attribute__((packed, aligned(4)))  ISP_DemosaicABF2_RBCfg;

/* Demosaic ABF Update Command  */
typedef struct ISP_DemosaicABF2_CmdType {
  /* Green config */
  ISP_DemosaicABF2_GCfg    gCfg;
  ISP_DemosaicABF2_Lut     gPosLut[8] ;
  ISP_DemosaicABF2_Lut     gNegLut[4] ;
  /* Blue config */
  ISP_DemosaicABF2_RBCfg   bCfg;
  ISP_DemosaicABF2_Lut     bPosLut[8] ;
  ISP_DemosaicABF2_Lut     bNegLut[4] ;
  /* Red config */
  ISP_DemosaicABF2_RBCfg   rCfg;
  ISP_DemosaicABF2_Lut     rPosLut[8] ;
  ISP_DemosaicABF2_Lut     rNegLut[4] ;
} __attribute__((packed, aligned(4))) ISP_DemosaicABF2_CmdType;

#endif /* __ABF40_REG_H__ */
