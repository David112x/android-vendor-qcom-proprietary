/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "vfe_test_vector.h"
#include "../modules/colorxform/colorxform44/colorxform44.h"
#include "../modules/colorxform/colorxform44/colorxform44_reg.h"
#include "camera_dbg.h"
#include <unistd.h>


/*===========================================================================
 * FUNCTION    - colorxform_validate_xformcfg -
 *
 * DESCRIPTION:
 *==========================================================================*/
void colorxform_validate_xformcfg(ISP_colorXformCfgCmdType *in, ISP_colorXformCfgCmdType
  *out)
{
    VALIDATE_TST_VEC(in->m00, out->m00, 0, "m00");
    VALIDATE_TST_VEC(in->m01, out->m01, 0, "m01");
    VALIDATE_TST_VEC(in->m02, out->m02, 0, "m02");
    VALIDATE_TST_VEC(in->o0, out->o0, 0, "o0");
    VALIDATE_TST_VEC(in->S0, out->S0, 0, "S0");
    VALIDATE_TST_VEC(in->m10, out->m10, 0, "m10");
    VALIDATE_TST_VEC(in->m11, out->m11, 0, "m11");
    VALIDATE_TST_VEC(in->m12, out->m12, 0, "m12");
    VALIDATE_TST_VEC(in->o1, out->o1, 0, "o1");
    VALIDATE_TST_VEC(in->s1, out->s1, 0, "s1");
    VALIDATE_TST_VEC(in->m20, out->m20, 0, "m20");
    VALIDATE_TST_VEC(in->m21, out->m21, 0, "m21");
    VALIDATE_TST_VEC(in->m22, out->m22, 0, "m22");
    VALIDATE_TST_VEC(in->o2, out->o2, 0, "o2");
    VALIDATE_TST_VEC(in->s2, out->s2, 0, "s2");
    VALIDATE_TST_VEC(in->c01, out->c01, 0, "c01");
    VALIDATE_TST_VEC(in->c11, out->c11, 0, "c11");
    VALIDATE_TST_VEC(in->c21, out->c21, 0, "c21");
    VALIDATE_TST_VEC(in->c00, out->c00, 0, "c00");
    VALIDATE_TST_VEC(in->c10, out->c10, 0, "c10");
    VALIDATE_TST_VEC(in->c20, out->c20, 0, "c20");
}/*colorxform_validate_xformcfg*/

/*===========================================================================
 * FUNCTION    - colorxform_populate_data -
 *
 * DESCRIPTION:
 *==========================================================================*/
void colorxform_populate_data(uint32_t *reg, ISP_colorXformCfgCmdType *pcmd)
{
  int size = sizeof(ISP_colorXformCfgCmdType);
  uint8_t *ptr = (uint8_t *)pcmd;
  reg += (ISP_COLOR_XFORM40_OFF/4);
  memcpy((void *)ptr, (void *)reg, size);
}

/*===========================================================================
 * FUNCTION    - vfe_c_tv_validate -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_colorxform_tv_validate(void* test_input,
  void* test_output)
{
  vfe_test_module_input_t *input = (vfe_test_module_input_t *)test_input;
  vfe_test_module_output_t *output = (vfe_test_module_output_t *)test_output;
  ISP_colorXformCfgCmdType in, out;
  colorxform_populate_data(input->reg_dump, &in);
  colorxform_populate_data(output->reg_dump, &out);

  colorxform_validate_xformcfg(&in, &out);
  return 0;
}/*vfe_colorxform_tv_validate*/
