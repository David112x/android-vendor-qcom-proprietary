/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "vfe_test_vector.h"
#include "bcc/bcc44/bcc44.h"
#include "bcc_reg.h"
#include "camera_dbg.h"
#include <unistd.h>

/*===========================================================================
 * FUNCTION    - vfe_bcc_test_vector_validation -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_bcc_tv_validate(void *in, void *op)
{
  vfe_test_module_input_t *mod_in = (vfe_test_module_input_t *)in;
  vfe_test_module_output_t *mod_op = (vfe_test_module_output_t *)op;

  ISP_DemosaicDBCC_CmdType *InCmd = NULL;
  ISP_DemosaicDBCC_CmdType *OutCmd = NULL;

  //4 bytes back to set the correct location
  InCmd = (ISP_DemosaicDBCC_CmdType *)(mod_in->reg_dump +
    (ISP_DBCC_OFF/4));

  //4 bytes back to set the correct location
  OutCmd = (ISP_DemosaicDBCC_CmdType *)(mod_op->reg_dump +
    (ISP_DBCC_OFF/4));

  VALIDATE_TST_VEC(InCmd->fminThreshold, OutCmd->fminThreshold, 0,
    "fminThreshold");
  VALIDATE_TST_VEC(InCmd->fmaxThreshold, OutCmd->fmaxThreshold, 0,
    "fmaxThreshold");

  VALIDATE_TST_VEC(InCmd->rOffsetLo, OutCmd->rOffsetLo, 0,
    "rOffsetLo");
  VALIDATE_TST_VEC(InCmd->rOffsetHi, OutCmd->rOffsetHi, 0,
    "rOffsetHi");
  VALIDATE_TST_VEC(InCmd->grOffsetLo, OutCmd->grOffsetLo, 0,
    "grOffsetLo");

  VALIDATE_TST_VEC(InCmd->gbOffsetLo, OutCmd->gbOffsetLo, 0,
    "gbOffsetLo");
  VALIDATE_TST_VEC(InCmd->gbOffsetHi, OutCmd->gbOffsetHi, 0,
    "gbOffsetHi");
  VALIDATE_TST_VEC(InCmd->grOffsetHi, OutCmd->grOffsetHi, 0,
    "grOffsetHi");

  VALIDATE_TST_VEC(InCmd->bOffsetLo, OutCmd->bOffsetLo, 0,
    "bOffsetLo");
  VALIDATE_TST_VEC(InCmd->bOffsetHi, OutCmd->bOffsetHi, 0,
    "bOffsetHi");

  return 0;
} /*vfe_bcc_test_vector_validation*/
