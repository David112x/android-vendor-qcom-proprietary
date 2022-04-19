/* bpc46_reg.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __BPC_REG_H__
#define __BPC_REG_H__

#define ISP_DBPC_CFG_OFF  0x000004F0
#define ISP_DBPC_LEN 5

#define ISP_BPC_CFG_OFF  0x000004EC
#define ISP_BPC_CFG_LEN  1
#define ABCC_LUT_COUNT 512

/** ISP_BPCCfg_CmdType:
 *
 *  @enable: enable bit
 *  @cfg_reg: cfg reg
 **/
typedef union  ISP_BPCCfg_CmdType {
  struct  {
    /* Demosaic Config */
    /* Reserved for dbpc enable  */
    uint32_t     enable                           : 1;
    /* Reserved for dbcc enable  */
    uint32_t     /* reserved */                   : 1;
    /* Reserved for abcc enable */
    uint32_t     /* reserved */                   : 1;
    /* Reserved */
    uint32_t     /* reserved */                   : 5;
    /* reserved for abcc lut bank selection   */
    uint32_t      /* reserved */                  : 1;
    /* reserved bits */
    uint32_t     /* reserved */                   :23;
  }__attribute__((packed, aligned(4)));
  uint32_t cfg_reg;
}__attribute__((packed, aligned(4))) ISP_BPCCfg_CmdType;

/** ISP_DemosaicDBPC_CmdType:
 *
 *  @fminThreshold: fmin threshold
 *  @fmaxThreshold: fmax threshold
 *  @rOffsetLo: r offset LO
 *  @rOffsetHi: r offset Hi
 *  @grOffsetLo: gr offset Lo
 *  @gbOffsetLo: gb offset Lo
 *  @gbOffsetHi: gc offset Hi
 *  @grOffsetHi: gr offset Hi
 *  @bOffsetLo: b offset Lo
 *  @bOffsetHi: b offset Hi
 **/
typedef struct ISP_DemosaicDBPC_CmdType {
  /* DBPC_CFG */
  uint32_t     fminThreshold                    : 8;
  uint32_t     fmaxThreshold                    : 8;
  uint32_t     /* reserved */                   :16;

  /* DBPC_OFFSET_CFG_0 */
  uint32_t     rOffsetLo                        :12;
  uint32_t     /* reserved */                   : 4;
  uint32_t     rOffsetHi                        :12;
  uint32_t     /* reserved */                   : 4;

  /* DBPC_OFFSET_CFG_1 */
  uint32_t     grOffsetLo                       :12;
  uint32_t     /* reserved */                   : 4;
  uint32_t     grOffsetHi                       :12;
  uint32_t     /* reserved */                   : 4;

  /* DBPC_OFFSET_CFG_2 */
  uint32_t     bOffsetLo                        :12;
  uint32_t     /* reserved */                   : 4;
  uint32_t     bOffsetHi                        :12;
  uint32_t     /* reserved */                   : 4;
/* DBPC_OFFSET_CFG_3*/
  uint32_t     gbOffsetLo                       :12;
  uint32_t     /* reserved */                   : 4;
  uint32_t     gbOffsetHi                       :12;
  uint32_t     /* reserved */                   : 4;
}__attribute__((packed, aligned(4))) ISP_DemosaicDBPC_CmdType;

#endif /* __BPC_REG_H__ */
