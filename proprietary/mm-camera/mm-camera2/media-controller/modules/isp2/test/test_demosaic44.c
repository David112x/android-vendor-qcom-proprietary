/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "vfe_test_vector.h"
#include "demosaic/demosaic44/demosaic44.h"
#include "demosaic_reg.h"
#include "camera_dbg.h"
#include <unistd.h>

/*===========================================================================
 * FUNCTION    - demosaic_populate_data -
 *
 * DESCRIPTION:
 *==========================================================================*/
void demosaic_populate_data(uint32_t *reg,
  ISP_DemosaicConfigCmdType *pcfg,
  ISP_DemosaicMixConfigCmdType *pMixCfg)
{
  memcpy((void*)pMixCfg, (void*)(reg+(ISP_DEMOSAIC_MIX_CFG_OFF/4)), sizeof(ISP_DemosaicMixConfigCmdType));
  memcpy((void*)pcfg, (void*)(reg+(ISP_DEMOSAIC_WB_GAIN_OFF/4)), sizeof(ISP_DemosaicConfigCmdType));
} /* demosaic_populate_data*/

/*===========================================================================
 * FUNCTION    - vfe_demosaic_tv_validate -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_demosaic_tv_validate(void *test_input,
  void *test_output)
{
  uint32_t i;
  ISP_DemosaicConfigCmdType in, out;
  ISP_DemosaicMixConfigCmdType inMixCfg, outMixCfg;
  vfe_test_module_input_t *input = (vfe_test_module_input_t *)test_input;
  vfe_test_module_output_t *output = (vfe_test_module_output_t *)test_output;

  demosaic_populate_data(input->reg_dump, &in, &inMixCfg);
  demosaic_populate_data(output->reg_dump, &out, &outMixCfg);

#define VALIDATE(i, o, field) VALIDATE_TST_VEC(i.field, o.field, 0, #field)

  VALIDATE(inMixCfg, outMixCfg, demoV4Enable);
  VALIDATE(inMixCfg, outMixCfg, dirGInterpDisable);
  VALIDATE(inMixCfg, outMixCfg, dirRBInterpDisable);
  VALIDATE(inMixCfg, outMixCfg, cositedRgbEnable);
  VALIDATE(inMixCfg, outMixCfg, dynGClampEnable);
  VALIDATE(inMixCfg, outMixCfg, dynRBClampEnable);
  VALIDATE(inMixCfg, outMixCfg, dynGClampCDisable);
  VALIDATE(inMixCfg, outMixCfg, dynRBClampCDisable);
  VALIDATE(inMixCfg, outMixCfg, pipeFlushCount);
  VALIDATE(inMixCfg, outMixCfg, pipeFlushOvd);
  VALIDATE(inMixCfg, outMixCfg, flushHaltOvd);

  VALIDATE(in, out, rgWbGain);
  VALIDATE(in, out, wgr1);
  VALIDATE(in, out, wgr2);
  VALIDATE(in, out, bgWbGain);
  VALIDATE(in, out, lambdaG);
  VALIDATE(in, out, grWbGain);
  VALIDATE(in, out, gbWbGain);
  VALIDATE(in, out, lambdaRB);
  VALIDATE(in, out, bl);
  VALIDATE(in, out, bu);
  VALIDATE(in, out, noiseLevelG);
  VALIDATE(in, out, dblu);
  VALIDATE(in, out, a);
  VALIDATE(in, out, noiseLevelRB);

  for (i = 0; i < ISP_DEMOSAIC_CLASSIFIER_CNT; i++) {
    VALIDATE_TST_LUT(in.interpClassifier[i].w_n, out.interpClassifier[i].w_n, 0, "classifier.w_n", i);
    VALIDATE_TST_LUT(in.interpClassifier[i].t_n, out.interpClassifier[i].t_n, 0, "classifier.t_n", i);
    VALIDATE_TST_LUT(in.interpClassifier[i].l_n, out.interpClassifier[i].l_n, 0, "classifier.l_n", i);
    VALIDATE_TST_LUT(in.interpClassifier[i].b_n, out.interpClassifier[i].b_n, 0, "classifier.b_n", i);
  }

  return 0;
} /*vfe_demosaic_tv_validate*/
