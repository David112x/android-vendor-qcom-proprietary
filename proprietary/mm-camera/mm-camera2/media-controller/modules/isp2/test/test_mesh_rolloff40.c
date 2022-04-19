/*============================================================================

 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include "vfe_test_vector.h"
#include "rolloff/mesh_rolloff40/mesh_rolloff40.h"
#include "mesh_rolloff_reg.h"
#include "camera_dbg.h"
#include <unistd.h>

/*===========================================================================
 * FUNCTION    - mesh_rolloff_populate_data -
 *
 * DESCRIPTION:
 *==========================================================================*/
void mesh_rolloff_populate_data(uint32_t *reg, MESH_RollOff_v4_ConfigParams *pcmd)
{
  int size = sizeof(MESH_RollOff_v4_ConfigParams);
  uint8_t *ptr = (uint8_t *)pcmd;
  reg += (ISP_MESH_ROLLOFF_CFG_OFF/4);
  memcpy((void *)ptr, (void *)reg, size);
}/*mesh_rolloff_populate_data*/

/*===========================================================================
 * FUNCTION    - mesh_rolloff_validate_g_cfg -
 *
 * DESCRIPTION:
 *==========================================================================*/
void mesh_rolloff_validate_cfg(
  MESH_RollOff_v4_ConfigParams *in, MESH_RollOff_v4_ConfigParams *out)
{
/* Bank Select bit need not be validated*/
  VALIDATE_TST_VEC(in->pixelOffset, out->pixelOffset, 0, "pixelOffset");
  VALIDATE_TST_VEC(in->blockWidth, out->blockWidth, 0, "blockWidth");
  VALIDATE_TST_VEC(in->blockHeight, out->blockHeight, 0, "blockHeight");
  VALIDATE_TST_VEC(in->subGridXDelta, out->subGridXDelta, 0, "subGridXDelta");
  VALIDATE_TST_VEC(in->subGridYDelta, out->subGridYDelta, 0, "subGridYDelta");
  VALIDATE_TST_VEC(in->interpFactor, out->interpFactor, 0, "interpFactor");
  VALIDATE_TST_VEC(in->subGridWidth, out->subGridWidth, 0, "subGridWidth");
  VALIDATE_TST_VEC(in->subGridHeight, out->subGridHeight, 0, "subGridHeight");
  VALIDATE_TST_VEC(in->blockXIndex, out->blockXIndex, 0, "blockXIndex");
  VALIDATE_TST_VEC(in->blockYIndex, out->blockYIndex, 0, "blockYIndex");
  VALIDATE_TST_VEC(in->PixelXIndex, out->PixelXIndex, 0, "PixelXIndex");
  VALIDATE_TST_VEC(in->PixelYIndex, out->PixelYIndex, 0, "PixelYIndex");
  VALIDATE_TST_VEC(in->yDeltaAccum, out->yDeltaAccum, 0, "yDeltaAccum");
  VALIDATE_TST_VEC(in->subGridXIndex, out->subGridXIndex, 0, "subGridXIndex");
  VALIDATE_TST_VEC(in->subGridYIndex, out->subGridYIndex, 0, "subGridYIndex");


}/*mesh_rolloff_validate_g_cfg*/

/*===========================================================================
 * FUNCTION    - mesh_rolloff_validate_table -
 *
 * DESCRIPTION:
 *==========================================================================*/
void mesh_rolloff_validate_table(
  vfe_test_table_t *in, vfe_test_table_t *out, uint32_t size, char* channel)
{
  uint32_t i;
  for (i = 0; i < size; i++) {
    VALIDATE_TST_LUT(in->table[i], out->table[i], 0, channel, i);
  }
}/*mesh_rolloff_validate_rb_cfg*/

/*===========================================================================
 * FUNCTION    - vfe_mesh_rolloff_tv_validate -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_mesh_rolloff_tv_validate(void *test_input,
  void *test_output)
{
  MESH_RollOff_v4_ConfigParams in, out;
  uint32_t size;

  vfe_test_module_input_t* input = (vfe_test_module_input_t *) test_input;
  vfe_test_module_output_t* output = (vfe_test_module_output_t *)test_output;
  mesh_rolloff_populate_data(input->reg_dump, &in);
  mesh_rolloff_populate_data(output->reg_dump, &out);

  mesh_rolloff_validate_cfg(&in, &out);

  size = 130;
  mesh_rolloff_validate_table(&input->mesh_rolloff.gr_r, &output->mesh_rolloff.gr_r, size,  "GR_R");
  mesh_rolloff_validate_table(&input->mesh_rolloff.gb_b, &output->mesh_rolloff.gb_b, size, "GB_B");
  return 0;
}/*vfe_mesh_rolloff_tv_validate*/
