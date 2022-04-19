/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "vfe_test_vector.h"
#include "chroma_enhan/chroma_enhan40/chroma_enhan40.h"
#include "chroma_enhan_reg.h"
#include "camera_dbg.h"
#include <unistd.h>

/*===========================================================================
 * FUNCTION    - color_conversion_populate_data -
 *
 * DESCRIPTION:
 *==========================================================================*/
void color_conversion_populate_data(uint32_t *reg,
  ISP_Chroma_Enhance_CfgCmdType *pcmd)
{
  reg += (ISP_CC40_OFF/4);
  memcpy((void *)pcmd, (void *)reg, sizeof(ISP_Chroma_Enhance_CfgCmdType));
}/*color_conversion_populate_data*/

/*===========================================================================
 * FUNCTION    - vfe_color_conversion_tv_validate -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_chroma_enhance_tv_validate(void *in, void *op)
{
  ISP_Chroma_Enhance_CfgCmdType in_cv, out_cv;
  vfe_test_module_input_t* input = (vfe_test_module_input_t *)in;
  vfe_test_module_output_t* output = (vfe_test_module_output_t *)op;
  color_conversion_populate_data(input->reg_dump, &in_cv);
  color_conversion_populate_data(output->reg_dump, &out_cv);

  VALIDATE_TST_VEC(in_cv.RGBtoYConversionV0, out_cv.RGBtoYConversionV0, 0, "RGBtoYConversionV0");
  VALIDATE_TST_VEC(in_cv.RGBtoYConversionV1, out_cv.RGBtoYConversionV1, 0, "RGBtoYConversionV1");
  VALIDATE_TST_VEC(in_cv.RGBtoYConversionV2, out_cv.RGBtoYConversionV2, 0, "RGBtoYConversionV2");
  VALIDATE_TST_VEC(in_cv.RGBtoYConversionOffset, out_cv.RGBtoYConversionOffset, 0, "RGBtoYConversionOffset");
  VALIDATE_TST_VEC(in_cv.ap, out_cv.ap, 0, "ap");
  VALIDATE_TST_VEC(in_cv.am, out_cv.am, 0, "am");
  VALIDATE_TST_VEC(in_cv.bp, out_cv.bp, 0, "bp");
  VALIDATE_TST_VEC(in_cv.bm, out_cv.bm, 0, "bm");
  VALIDATE_TST_VEC(in_cv.cp, out_cv.cp, 0, "cp");
  VALIDATE_TST_VEC(in_cv.cm, out_cv.cm, 0, "cm");
  VALIDATE_TST_VEC(in_cv.dp, out_cv.dp, 0, "bp");
  VALIDATE_TST_VEC(in_cv.dm, out_cv.dm, 0, "bm");
  VALIDATE_TST_VEC(in_cv.kcb, out_cv.kcb, 0, "kcb");
  VALIDATE_TST_VEC(in_cv.kcr, out_cv.kcr, 0, "kcr");
  return 0;
}/*vfe_color_conversion_tv_validate*/
