/*============================================================================
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================*/
#include "vfe_test_vector.h"
#include "demux/demux48/demux48.h"
#include "demux_reg.h"
#include "camera_dbg.h"
#include <unistd.h>

#define ISP_DEMUX48_EVEN_CFG_OFF 0x00000574
#define ISP_DEMUX48_ODD_CFG_OFF 0x00000578
/*===========================================================================
 * FUNCTION    - vfe_demux_tv_validate -
 *
 * DESCRIPTION: this function compares the test vector output with hw output
 *==========================================================================*/
int vfe_demux_tv_validate(void *input, void *output)
{
  vfe_test_module_input_t *mod_in = (vfe_test_module_input_t *)input;
  vfe_test_module_output_t *mod_op = (vfe_test_module_output_t *)output;
  ISP_DemuxConfigCmdType *in, *out;
  uint32_t * cfg;

  in = (ISP_DemuxConfigCmdType *)(mod_in->reg_dump + (ISP_DEMUX48_OFF/4));
  out = (ISP_DemuxConfigCmdType *)(mod_op->reg_dump + (ISP_DEMUX48_OFF/4));

  VALIDATE_TST_VEC(in->period, out->period, 0, "period");
  VALIDATE_TST_VEC(in->blk_out, out->blk_out, 0, "blk_out");
  VALIDATE_TST_VEC(in->blk_in, out->blk_in, 0, "blk_in");
  VALIDATE_TST_VEC(in->ch0EvenGain, out->ch0EvenGain, 0, "ch0EvenGain");
  VALIDATE_TST_VEC(in->ch0OddGain, out->ch0OddGain, 0, "ch0OddGain");
  VALIDATE_TST_VEC(in->ch1Gain, out->ch1Gain, 0, "ch1Gain");
  VALIDATE_TST_VEC(in->ch2Gain, out->ch2Gain, 0, "ch2Gain");

  /* Test f/w for some reason is not reading the cfg reference vectors
     properly from the file */
  cfg = (uint32_t *)(mod_in->reg_dump + (ISP_DEMUX48_EVEN_CFG_OFF/4));
  VALIDATE_TST_VEC(*cfg, out->evenCfg, 0, "evenCfg");
  cfg = (uint32_t *)(mod_in->reg_dump + (ISP_DEMUX48_ODD_CFG_OFF/4));
  VALIDATE_TST_VEC(*cfg, out->oddCfg, 0, "oddCfg");

  /* For now NOT testing the right channel since 3D is NOT supported.
     Also in ISP h/w update we don't program these values to h/w */
  //VALIDATE_TST_VEC(in->R_ch0EvenGain, out->R_ch0EvenGain, 0, "R_ch0EvenGain");
  //VALIDATE_TST_VEC(in->R_ch0OddGain, out->R_ch0OddGain, 0, "R_ch0OddGain");
  //VALIDATE_TST_VEC(in->R_ch1Gain, out->R_ch1Gain, 0, "R_ch1Gain");
  //VALIDATE_TST_VEC(in->R_ch2Gain, out->R_ch2Gain, 0, "R_ch2Gain");

  return 0;
}
