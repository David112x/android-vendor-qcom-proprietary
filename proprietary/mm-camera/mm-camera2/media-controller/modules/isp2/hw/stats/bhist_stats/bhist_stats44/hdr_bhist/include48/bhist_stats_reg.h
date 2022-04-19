/* bhist_stats_reg.h
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __BHIST_STATS_REG_H__
#define __BHIST_STATS_REG_H__

#define ISP_MODULE_NAME ISP_MOD_HDR_BHIST_STATS
#define ISP_MODULE_STRING "hdr_bhist_stats"
#define ISP_LOG_NAME ISP_LOG_HDR_BHIST_STATS

#define NUM_DMI 4
#define DMI_RAM_START STATS_HDR_BHIST_RAM0

#define BHIST_STATS_OFF 0x00000AD4
#define BHIST_STATS_LEN 2

#define BHIST_STATS_CFG_OFF 0x00000AB0

#define BHIST_CGC_OVERRIDE TRUE
#define BHIST_CGC_OVERRIDE_REGISTER 0x30
#define BHIST_CGC_OVERRIDE_BIT 1

/** ISP_StatsBhist_CfgCmdType:
 *
 *  @rgnHOffset: H offset
 *  @rgnVOffset: V offset
 *  @rgnHNum: H num
 *  @rgnVNum: V num
 **/
typedef struct ISP_StatsBhist_CfgCmdType {
  /*  VFE_STATS_BHIST_RGN_OFFSET_CFG   */
  uint32_t        rgnHOffset            :   13;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnVOffset            :   14;
  uint32_t       /*reserved */          :    2;
  /*  VFE_STATS_BHIST_RGN_SIZE_CFG */
  uint32_t        rgnHNum               :    12;
  uint32_t      /* reserved 23:31 */    :     4;
  uint32_t        rgnVNum               :    13;
  uint32_t      /* reserved 23:31 */    :     3;
} __attribute__((packed, aligned(4))) ISP_StatsBhist_CfgCmdType;

typedef union ISP_Stats_CfgCmdType {
  /* DO NOT configure STATS SITE SEL from HDR BHIST stats
   * It is selected from HDR BE stats driver
   */
  struct
  {
    uint32_t     /* reserved*/                :   18;
    uint32_t     hdr_bhist_field_sel          :   2;
    uint32_t     hdr_bhist_channel_sel        :   1;
    uint32_t     /* reserved*/                :   11;
  }__attribute__((packed, aligned(4)));

  uint32_t mask;
}__attribute__((packed, aligned(4))) ISP_Stats_CfgType;

#endif /* __BHIST_STATS_REG_H__ */
