/* demux_reg.h
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __DEMUX_REG_H__
#define __DEMUX_REG_H__

#define ISP_DEMUX48_OFF    0x00000560
#define ISP_DEMUX48_LEN    7


typedef struct ISP_DemuxConfigCmdType {
  /*  period and blk level */
  uint32_t  period         : 3;
  uint32_t  /* reserved */ : 1;
  uint32_t  blk_out        :12;
  uint32_t  /* reserved */ : 4;
  uint32_t  blk_in         :12;
  /* Demux Gain 0 Config */
  uint32_t  ch0EvenGain    :15;
  uint32_t  /* reserved */ : 1;
  uint32_t  ch0OddGain     :15;
  uint32_t  /* reserved */ : 1;
  /* Demux Gain 1 Config */
  uint32_t  ch1Gain        :15;
  uint32_t  /* reserved */ : 1;
  uint32_t  ch2Gain        :15;
  uint32_t  /* reserved */ : 1;
  /* Demux Gain 0 Config */
  uint32_t  R_ch0EvenGain  :15;
  uint32_t  /* reserved */ : 1;
  uint32_t  R_ch0OddGain   :15;
  uint32_t  /* reserved */ : 1;
  /* Demux Gain 1 Config */
  uint32_t  R_ch1Gain      :15;
  uint32_t  /* reserved */ : 1;
  uint32_t  R_ch2Gain      :15;
  uint32_t  /* reserved */ : 1;
  /* Demux Gain 1 Config */
  uint32_t  evenCfg        :32;
  /* Demux Gain 1 Config */
  uint32_t  oddCfg         :32;
} __attribute__((packed, aligned(4))) ISP_DemuxConfigCmdType;

typedef struct ISP_DemuxGainCfgCmdType {
  uint32_t  ch0EvenGain    :10;
  uint32_t  /* reserved */ : 6;
  uint32_t  ch0OddGain     :10;
  uint32_t  /* reserved */ : 6;
  uint32_t  ch1Gain        :10;
  uint32_t  /* reserved */ : 6;
  uint32_t  ch2Gain        :10;
  uint32_t  /* reserved */ : 6;
} __attribute__((packed, aligned(4))) ISP_DemuxGainCfgCmdType;

#endif //__DEMUX_REG_H__
