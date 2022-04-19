/*============================================================================
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================*/
#include "vfe_test_vector.h"
#include "gic/gic48/gic48.h"
#include "gic/gic48/include48/gic_reg.h"
#include "camera_dbg.h"
#include <unistd.h>

/*===========================================================================
 * FUNCTION    - vfe_gic_test_vector_validation -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_gic_tv_validate(void *in, void *op)
{
  uint32_t i;
  vfe_test_module_input_t *mod_in = (vfe_test_module_input_t *)in;
  vfe_test_module_output_t *mod_op = (vfe_test_module_output_t *)op;

  ISP_GIC_CfgCmdType *InCmd = NULL;
  ISP_GIC_CfgCmdType *OutCmd = NULL;

  InCmd = (ISP_GIC_CfgCmdType *)(mod_in->reg_dump +
    (ISP_GIC_OFF/4));

  OutCmd = (ISP_GIC_CfgCmdType *)(mod_op->reg_dump +
    (ISP_GIC_OFF/4));

  VALIDATE_TST_VEC(InCmd->filterStrength, OutCmd->filterStrength, 0,
    "filterStrength");
  VALIDATE_TST_VEC(InCmd->noiseOffset, OutCmd->noiseOffset, 0,
    "noiseOffset");

  for (i = 0; i < mod_in->gic_std2_l0.size; i++)
    VALIDATE_TST_LUT(mod_in->gic_std2_l0.table[i],
                     mod_op->gic_std2_l0.table[i], 0, "gic_std2_l0", i);

  return 0;
} /*vfe_gic_test_vector_validation*/
