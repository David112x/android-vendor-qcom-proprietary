/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "vfe_test_vector.h"
#include "pedestal_correct/pedestal_correct46/pedestal_correct46.h"
#include "pedestal_correct/pedestal_correct46/include46/pedestal_correct_reg.h"
#include "camera_dbg.h"
#include "isp_pipeline_reg.h"
#include <unistd.h>

/*===========================================================================
 * FUNCTION    - vfe_pedestal_test_vector_validation -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_pedestal_tv_validate(void *in, void *op)
{
  uint32_t i;
  vfe_test_module_input_t *mod_in = (vfe_test_module_input_t *)in;
  vfe_test_module_output_t *mod_op = (vfe_test_module_output_t *)op;

  ISP_Pedestal_CfgCmdType *InCmd = NULL;
  ISP_Pedestal_CfgCmdType *OutCmd = NULL;
  ISP_ModuleEn            *InMod_En = NULL;
  ISP_ModuleEn            *OutMod_En = NULL;

  InCmd = (ISP_Pedestal_CfgCmdType *)(mod_in->reg_dump +
    (ISP_PEDESTAL_OFF/4));

  OutCmd = (ISP_Pedestal_CfgCmdType *)(mod_op->reg_dump +
    (ISP_PEDESTAL_OFF/4));

  InMod_En = (ISP_ModuleEn *)(mod_in->reg_dump +
    (ISP_MODULE_LENS_EN_OFFSET/4));

  OutMod_En = (ISP_ModuleEn *)(mod_op->reg_dump +
    (ISP_MODULE_LENS_EN_OFFSET/4));

  if (!InMod_En->fields.pedestal && !OutMod_En->fields.pedestal) {
    CDBG_TV("Pedestal module disabled. Skip BET for pedestal\n");
    return 0;
  }

#define TEST(field) \
  VALIDATE_TST_VEC(InCmd->field, OutCmd->field, 0, #field);

  TEST(scaleBypass);
  TEST(hdrEnable);
  TEST(blockWidth);
  TEST(blockHeight);
  TEST(subGridHeight);
  TEST(subGridYDelta);
  TEST(interpFactor);
  TEST(subGridWidth);
  TEST(subGridXDelta);
  TEST(blockXIndex);
  TEST(blockYIndex);
  TEST(yDeltaAccum);
  TEST(pixelXIndex);
  TEST(pixelYIndex);
  TEST(subGridXIndex);
  TEST(subGridYIndex);

#define TEST_TBL(tbl) { \
    for (i = 0; i < mod_in->tbl.size; i++) \
      VALIDATE_TST_LUT(mod_in->tbl.table[i], mod_op->tbl.table[i], 0, #tbl, i); \
  }

  if (PEDESTAL_T1_T2 == 1) {
    TEST_TBL(pedestal_t1_gr_r);
    TEST_TBL(pedestal_t1_gb_b);
  } else {
    TEST_TBL(pedestal_t1_gr_r);
    TEST_TBL(pedestal_t1_gb_b);
    TEST_TBL(pedestal_t2_gr_r);
    TEST_TBL(pedestal_t2_gb_b);
  }

  return 0;
} /*vfe_pedestal_test_vector_validation*/
