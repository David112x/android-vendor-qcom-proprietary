/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "vfe_test_vector.h"
#include "chroma_suppress/chroma_suppress40/chroma_suppress40.h"
#include "chroma_suppress_reg.h"
#include "camera_dbg.h"
#include <unistd.h>

/*===========================================================================
 * FUNCTION    - vfe_chroma_suppression_test_vector_validate -
 *
 * DESCRIPTION: this function compares the test vector output with hw output
 *==========================================================================*/
int vfe_chroma_suppression_tv_validate(
  void *test_input, void *test_output)
{
  ISP_ChromaSuppress_ConfigCmdType *in, *out;
  ISP_ChromaSuppress_Mix1_ConfigCmdType *in2, *out2;
  ISP_ChromaSuppress_Mix2_ConfigCmdType *in3, *out3;
  vfe_test_module_input_t *mod_in = (vfe_test_module_input_t *)test_input;
  vfe_test_module_output_t *mod_op = (vfe_test_module_output_t *)test_output;

  CDBG("%s:\n", __func__);
  in = (ISP_ChromaSuppress_ConfigCmdType *)(mod_in->reg_dump + (ISP_CHROMA40_SUP_OFF/4));
  in2 = (ISP_ChromaSuppress_Mix1_ConfigCmdType *)(mod_in->reg_dump + (ISP_CHROMA40_SUP_MIX_OFF_1/4));
  in3 = (ISP_ChromaSuppress_Mix2_ConfigCmdType *)(mod_in->reg_dump + (ISP_CHROMA40_SUP_MIX_OFF_2/4));

  out = (ISP_ChromaSuppress_ConfigCmdType *)(mod_op->reg_dump + (ISP_CHROMA40_SUP_OFF/4));
  out2 = (ISP_ChromaSuppress_Mix1_ConfigCmdType *)(mod_op->reg_dump + (ISP_CHROMA40_SUP_MIX_OFF_1/4));
  out3 = (ISP_ChromaSuppress_Mix2_ConfigCmdType *)(mod_op->reg_dump + (ISP_CHROMA40_SUP_MIX_OFF_2/4));

  VALIDATE_TST_VEC(in->ySup1, out->ySup1, 0, "ySup1");
  VALIDATE_TST_VEC(in->ySup2, out->ySup2, 0, "ySup2");
  VALIDATE_TST_VEC(in->ySup3, out->ySup3, 0, "ySup3");
  VALIDATE_TST_VEC(in->ySup4, out->ySup4, 0, "ySup4");
  VALIDATE_TST_VEC(in2->ySupM1, out2->ySupM1, 0, "ySupM1");
  VALIDATE_TST_VEC(in2->ySupM3, out2->ySupM3, 0, "ySupM3");
  VALIDATE_TST_VEC(in2->ySupS1, out2->ySupS1, 0, "ySupS1");
  VALIDATE_TST_VEC(in2->ySupS3, out2->ySupS3, 0, "ySupS3");
  VALIDATE_TST_VEC(in3->cSup1, out3->cSup1, 0, "cSup1");
  VALIDATE_TST_VEC(in3->cSup2, out3->cSup2, 0, "cSup2");
  VALIDATE_TST_VEC(in3->cSupM1, out3->cSupM1, 0, "cSupM1");
  VALIDATE_TST_VEC(in3->cSupS1, out3->cSupS1, 0, "cSupS1");

  return 0;
}
