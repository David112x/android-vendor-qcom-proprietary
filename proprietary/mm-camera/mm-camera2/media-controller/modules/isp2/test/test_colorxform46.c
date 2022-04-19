/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "vfe_test_vector.h"
//#include "../hw/color_xform/color_xform46/color_xform46.h"
#include "color_xform_reg.h"
#include "camera_dbg.h"
#include <unistd.h>


/*===========================================================================
 * FUNCTION    - colorxform_validate_xformcfg -
 *
 * DESCRIPTION:
 *==========================================================================*/
void colorxform_validate_xformcfg(ISP_ColorXformCfgCmdType *in,
  ISP_ColorXformCfgCmdType *out, char *xform_path, boolean *has_match)
{
  int temp_has_mismatch = has_mismatch;
  has_mismatch = 0;
  VALIDATE_TST_VEC2(in->m00, out->m00, 0, "m00", xform_path);
  VALIDATE_TST_VEC2(in->m01, out->m01, 0, "m01", xform_path);
  VALIDATE_TST_VEC2(in->m02, out->m02, 0, "m02", xform_path);
  VALIDATE_TST_VEC2(in->o0, out->o0, 0, "o0", xform_path);
  VALIDATE_TST_VEC2(in->s0, out->s0, 0, "s0", xform_path);
  VALIDATE_TST_VEC2(in->m10, out->m10, 0, "m10", xform_path);
  VALIDATE_TST_VEC2(in->m11, out->m11, 0, "m11", xform_path);
  VALIDATE_TST_VEC2(in->m12, out->m12, 0, "m12", xform_path);
  VALIDATE_TST_VEC2(in->o1, out->o1, 0, "o1", xform_path);
  VALIDATE_TST_VEC2(in->s1, out->s1, 0, "s1", xform_path);
  VALIDATE_TST_VEC2(in->m20, out->m20, 0, "m20", xform_path);
  VALIDATE_TST_VEC2(in->m21, out->m21, 0, "m21", xform_path);
  VALIDATE_TST_VEC2(in->m22, out->m22, 0, "m22", xform_path);
  VALIDATE_TST_VEC2(in->o2, out->o2, 0, "o2", xform_path);
  VALIDATE_TST_VEC2(in->s2, out->s2, 0, "s2", xform_path);
  VALIDATE_TST_VEC2(in->c01, out->c01, 0, "c01", xform_path);
  VALIDATE_TST_VEC2(in->c11, out->c11, 0, "c11", xform_path);
  VALIDATE_TST_VEC2(in->c21, out->c21, 0, "c21", xform_path);
  VALIDATE_TST_VEC2(in->c00, out->c00, 0, "c00", xform_path);
  VALIDATE_TST_VEC2(in->c10, out->c10, 0, "c10", xform_path);
  VALIDATE_TST_VEC2(in->c20, out->c20, 0, "c20", xform_path);

  if (has_mismatch == 0) {
    *has_match = TRUE;
  }

}/*colorxform_validate_xformcfg*/

/*===========================================================================
 * FUNCTION    - colorxform_populate_data -
 *
 * DESCRIPTION:
 *==========================================================================*/
void colorxform_populate_data(uint32_t *reg, ISP_ColorXformCfgCmdType *pcmd, uint32_t reg_offset)
{
  int size = sizeof(ISP_ColorXformCfgCmdType);
  uint8_t *ptr = (uint8_t *)pcmd;
  reg += (reg_offset/4);
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
  ISP_ColorXformCfgCmdType in, out;
  int show_mismatch_temp = show_mismatch;
  boolean has_match  = FALSE;

  show_mismatch = 0;
  colorxform_populate_data(input->reg_dump, &in, ISP_COLOR_XFORM46_VIEW_OFF);
  colorxform_populate_data(output->reg_dump, &out, ISP_COLOR_XFORM46_VIEW_OFF);
  colorxform_validate_xformcfg(&in, &out, "VIEWFINDER", &has_match);
  if (has_match == TRUE) {
    printf("\tBET: Match color_xform VIEWFINDER path\n");
    has_mismatch = FALSE;
    return 0;
  }

  colorxform_populate_data(input->reg_dump, &in, ISP_COLOR_XFORM46_ENC_OFF);
  colorxform_populate_data(output->reg_dump, &out, ISP_COLOR_XFORM46_ENC_OFF);
  colorxform_validate_xformcfg(&in, &out, "ENCODER", &has_match);

  if (has_match == TRUE) {
    printf("\tBET: Match color_xform ENCODER path\n");
    has_mismatch = FALSE;
    return 0;
  }
  colorxform_populate_data(input->reg_dump, &in, ISP_COLOR_XFORM46_VIDEO_OFF);
  colorxform_populate_data(output->reg_dump, &out, ISP_COLOR_XFORM46_VIDEO_OFF);
  colorxform_validate_xformcfg(&in, &out, "VIDEO", &has_match);
  if (has_match == TRUE) {
    printf("\tBET: Match color_xform VIDEO path\n");
    has_mismatch = FALSE;
    return 0;
  }

  if (has_match == FALSE) {
    show_mismatch = show_mismatch_temp;

    printf("\tBET: Mismatch: color_xform ALL path Mismatch\n");
    colorxform_populate_data(input->reg_dump, &in, ISP_COLOR_XFORM46_VIEW_OFF);
    colorxform_populate_data(output->reg_dump, &out, ISP_COLOR_XFORM46_VIEW_OFF);
    colorxform_validate_xformcfg(&in, &out, "VIEWFINDER", &has_match);

    colorxform_populate_data(input->reg_dump, &in, ISP_COLOR_XFORM46_ENC_OFF);
    colorxform_populate_data(output->reg_dump, &out, ISP_COLOR_XFORM46_ENC_OFF);
    colorxform_validate_xformcfg(&in, &out, "ENCODER", &has_match);

    colorxform_populate_data(input->reg_dump, &in, ISP_COLOR_XFORM46_VIDEO_OFF);
    colorxform_populate_data(output->reg_dump, &out, ISP_COLOR_XFORM46_VIDEO_OFF);
    colorxform_validate_xformcfg(&in, &out, "VIDEO", &has_match);
  }
  return 0;
}/*vfe_colorxform_tv_validate*/
