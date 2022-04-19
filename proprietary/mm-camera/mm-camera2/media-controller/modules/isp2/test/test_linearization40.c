/*============================================================================

 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include "vfe_test_vector.h"
#include "linearization/linearization40/linearization40.h"
#include "linearization_reg.h"
#include "camera_dbg.h"
#include <unistd.h>


/*===========================================================================
 * FUNCTION    - linearization_validate_interp_slope_cfg -
 *
 * DESCRIPTION:
 *==========================================================================*/
void linearization_validate_point_slope_data(ISP_PointSlopeData *in, ISP_PointSlopeData
  *out)
{
    VALIDATE_TST_VEC(in->kneePoint_P1, out->kneePoint_P1, 0, "kneePoint_P1");
    VALIDATE_TST_VEC(in->kneePoint_P0, out->kneePoint_P0, 0, "kneePoint_P0");
    VALIDATE_TST_VEC(in->kneePoint_P3, out->kneePoint_P3, 0, "kneePoint_P3");
    VALIDATE_TST_VEC(in->kneePoint_P2, out->kneePoint_P2, 0, "kneePoint_P2");
    VALIDATE_TST_VEC(in->kneePoint_P5, out->kneePoint_P5, 0, "kneePoint_P5");
    VALIDATE_TST_VEC(in->kneePoint_P4, out->kneePoint_P4, 0, "kneePoint_P4");
    VALIDATE_TST_VEC(in->kneePoint_P7, out->kneePoint_P7, 0, "kneePoint_P7");
    VALIDATE_TST_VEC(in->kneePoint_P6, out->kneePoint_P6, 0, "kneePoint_P6");
}/*linearization_validate_point_slope_data*/

/*===========================================================================
 * FUNCTION    - linearization_validate_table -
 *
 * DESCRIPTION:
 *==========================================================================*/
void linearization_validate_table(vfe_test_table_t *in, vfe_test_table_t *out, char* tbl_name)
{
  if (!in || !out || !tbl_name) {
    CDBG_ERROR("failed:%s Null pointer in %p out %p tbl_name %p",__func__, in,
       out, tbl_name);
    return;
  }
  uint32_t i;
  for (i = 0; i < in->size; i++)
//#if defined(LINEAR_TABLE_WIDTH) && (LINEAR_TABLE_WIDTH == 64)
  //  VALIDATE_TST_LUT64(in->table[i], out->table[i], 0, tbl_name, i);
//#else
    VALIDATE_TST_LUT(in->table[i], out->table[i], 0, tbl_name, i);
//#endif
} /* linearization_validate_table*/

/*===========================================================================
 * FUNCTION    - linarization_populate_data -
 *
 * DESCRIPTION:
 *==========================================================================*/
void linearization_populate_data(uint32_t *reg, ISP_LinearizationCfgParams *pcmd)
{
  int size = ISP_LINEARIZATION_LEN * sizeof(uint32_t);
  uint8_t *ptr = (uint8_t *)pcmd;
  reg += (ISP_LINEARIZATION_OFF/4);
  memcpy((void *)ptr, (void *)reg, size);
}

/*===========================================================================
 * FUNCTION    - vfe_linearization_tv_validate -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_linearization_tv_validate(void* test_input,
  void* test_output)
{
  ISP_LinearizationCfgParams in, out;
  if (!test_input || !test_output) {
    CDBG_ERROR("%s: failed: Null Pointer test_input %p test_output %p",
      __func__, test_input, test_output);
    return -1;
  }
  vfe_test_module_input_t *input = (vfe_test_module_input_t *)test_input;
  vfe_test_module_output_t *output = (vfe_test_module_output_t *)test_output;
  int i;

  linearization_populate_data(input->reg_dump, &in);
  linearization_populate_data(output->reg_dump, &out);

  linearization_validate_point_slope_data(&in.pointSlopeR, &out.pointSlopeR);
  linearization_validate_point_slope_data(&in.pointSlopeGb, &out.pointSlopeGb);
  linearization_validate_point_slope_data(&in.pointSlopeB, &out.pointSlopeB);
  linearization_validate_point_slope_data(&in.pointSlopeGr, &out.pointSlopeGr);
  linearization_validate_table(&input->linearization, &output->linearization, "linearization");

  return 0;
}/*vfe_linearization_tv_validate*/
