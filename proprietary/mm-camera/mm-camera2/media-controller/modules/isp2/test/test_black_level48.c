/*============================================================================
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================*/
#include "vfe_test_vector.h"
#include "black_level/black_level48/black_level48.h"
#include "black_level48_reg.h"
#include "camera_dbg.h"
#include <unistd.h>

/*===========================================================================
 * FUNCTION    - vfe_black_level_test_vector_validate -
 *
 * DESCRIPTION: this function compares the test vector output with hw output
 *==========================================================================*/
int vfe_black_level_tv_validate(void *input, void *output)
{
  vfe_test_module_input_t *mod_in = (vfe_test_module_input_t *)input;
  vfe_test_module_output_t *mod_op = (vfe_test_module_output_t *)output;
  ISP_Black_LevelCfg *in, *out;
  ISP_Black_Level_StatsCfg *in_stats, *out_stats;

  in = (ISP_Black_LevelCfg *)(mod_in->reg_dump + (ISP_BLACK_LEVEL_CFG_OFF/4));
  out = (ISP_Black_LevelCfg *)(mod_op->reg_dump + (ISP_BLACK_LEVEL_CFG_OFF/4));

  in_stats = (ISP_Black_Level_StatsCfg *)(mod_in->reg_dump + (ISP_STATS_BLACK_LEVEL_CFG_OFF/4));
  out_stats = (ISP_Black_Level_StatsCfg *)(mod_op->reg_dump + (ISP_STATS_BLACK_LEVEL_CFG_OFF/4));

  /*Black level on pix*/
  VALIDATE_TST_VEC(in->offset, out->offset, 0, "offset");
  VALIDATE_TST_VEC(in->scale, out->scale, 0, "scale");
  /*Black level on stats*/
  VALIDATE_TST_VEC(in_stats->offset, out_stats->offset, 0, "stats offset");
  VALIDATE_TST_VEC(in_stats->scale, out_stats->scale, 0, "stats scale");

  return 0;
}
