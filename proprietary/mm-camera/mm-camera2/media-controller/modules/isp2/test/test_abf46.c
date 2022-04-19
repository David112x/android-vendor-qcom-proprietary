/*============================================================================
  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include "vfe_test_vector.h"
#include "abf/abf44/abf44.h"
#include "abf_reg.h"
#include "camera_dbg.h"
#include <unistd.h>

/*===========================================================================
 * FUNCTION    - abf_populate_data -
 *
 * DESCRIPTION:
 *==========================================================================*/
void abf_populate_data(uint32_t *reg, ISP_ABF3_CmdType *pcmd)
{
  int size = sizeof(ISP_ABF3_CmdType);
  uint8_t *ptr = (uint8_t *)pcmd;
  reg += (ISP_ABF3_OFF/4);
  memcpy((void *)ptr, (void *)reg, size);
} /* abf_populate_data*/

/*===========================================================================
 * FUNCTION    - abf_validate_master_cfg -
 *
 * DESCRIPTION:
 *==========================================================================*/
void abf_validate_master_cfg(ISP_ABF3_Cfg *in, ISP_ABF3_Cfg *out)
{
#define MASTER_TEST(field) VALIDATE_TST_VEC(in->field, out->field, 0, #field)

  MASTER_TEST(iirLevel0En);
  MASTER_TEST(iirLevel1En);
  MASTER_TEST(lutBankSel);
  MASTER_TEST(crossPlLevel0En);
  MASTER_TEST(crossPlLevel1En);
  MASTER_TEST(minMaxEn);

#if 0
  MASTER_TEST(abf3En);
  MASTER_TEST(pipeFlushOvd);
  MASTER_TEST(flushHaltOvd);
  MASTER_TEST(pipeFlushCnt);
#endif
} /* abf_validate_master_cfg*/

/*===========================================================================
 * FUNCTION    - abf_validate_lvl_cfg -
 *
 * DESCRIPTION:
 *==========================================================================*/
void abf_validate_lvl_cfg(ISP_ABF3_Lv1Cfg *in, ISP_ABF3_Lv1Cfg *out)
{
#define LVL_TEST(field) VALIDATE_TST_VEC(in->field, out->field, 0, #field)

  LVL_TEST(distGrGb_0);
  LVL_TEST(distGrGb_1);
  LVL_TEST(distGrGb_2);
  LVL_TEST(distGrGb_3);
  LVL_TEST(distGrGb_4);
  LVL_TEST(minMaxSelGrGb0);
  LVL_TEST(minMaxSelGrGb1);
  LVL_TEST(distRb0);
  LVL_TEST(distRb1);
  LVL_TEST(distRb2);
  LVL_TEST(distRb3);
  LVL_TEST(distRb4);
  LVL_TEST(minMaxSelRb0);
  LVL_TEST(minMaxSelRb1);
} /* abf_validate_lvl_cfg*/

/*===========================================================================
 * FUNCTION    - abf_validate_ch_cfg -
 *
 * DESCRIPTION:
 *==========================================================================*/
void abf_validate_ch_cfg(ISP_ABF3_ChannelCfg *in, ISP_ABF3_ChannelCfg *out, char* channel)
{
#define CH_TEST(f)VALIDATE_TST_VEC2(in->f, out->f, 0, channel, #f)

  CH_TEST(gr_noiseScale0Lv0);
  CH_TEST(gr_wTableAdjLv0);
  CH_TEST(gr_noiseScale0Lv1);
  CH_TEST(gr_wTableAdjLv1);
  CH_TEST(gr_noiseScale1Lv0);
  CH_TEST(gr_noiseScale1Lv1);
  CH_TEST(gr_noiseScale2Lv0);
  CH_TEST(gr_noiseScale2Lv1);
  CH_TEST(gr_noiseOffLv0);
  CH_TEST(gr_noiseOffLv1);
  CH_TEST(gr_filterStrLv0);
  CH_TEST(gr_filterStrLv1);
  CH_TEST(gr_curvOffLv0);
  CH_TEST(gr_curvOffLv1);
  CH_TEST(gr_sftThrdNoiseScaleLv0);
  CH_TEST(gr_sftThrdNoiseShiftLv0);
  CH_TEST(gr_sftThrdNoiseScaleLv1);
  CH_TEST(gr_sftThrdNoiseShiftLv1);

  CH_TEST(gb_noiseScale0Lv0);
  CH_TEST(gb_wTableAdjLv0);
  CH_TEST(gb_noiseScale0Lv1);
  CH_TEST(gb_wTableAdjLv1);
  CH_TEST(gb_noiseScale1Lv0);
  CH_TEST(gb_noiseScale1Lv1);
  CH_TEST(gb_noiseScale2Lv0);
  CH_TEST(gb_noiseScale2Lv1);
  CH_TEST(gb_noiseOffLv0);
  CH_TEST(gb_noiseOffLv1);
  CH_TEST(gb_filterStrLv0);
  CH_TEST(gb_filterStrLv1);
  CH_TEST(gb_curvOffLv0);
  CH_TEST(gb_curvOffLv1);
  CH_TEST(gb_sftThrdNoiseScaleLv0);
  CH_TEST(gb_sftThrdNoiseShiftLv0);
  CH_TEST(gb_sftThrdNoiseScaleLv1);
  CH_TEST(gb_sftThrdNoiseShiftLv1);

  CH_TEST(r_noiseScale0Lv0);
  CH_TEST(r_wTableAdjLv0);
  CH_TEST(r_noiseScale0Lv1);
  CH_TEST(r_wTableAdjLv1);
  CH_TEST(r_noiseScale1Lv0);
  CH_TEST(r_noiseScale1Lv1);
  CH_TEST(r_noiseScale2Lv0);
  CH_TEST(r_noiseScale2Lv1);
  CH_TEST(r_noiseOffLv0);
  CH_TEST(r_noiseOffLv1);
  CH_TEST(r_filterStrLv0);
  CH_TEST(r_filterStrLv1);
  CH_TEST(r_curvOffLv0);
  CH_TEST(r_curvOffLv1);
  CH_TEST(r_sftThrdNoiseScaleLv0);
  CH_TEST(r_sftThrdNoiseShiftLv0);
  CH_TEST(r_sftThrdNoiseScaleLv1);
  CH_TEST(r_sftThrdNoiseShiftLv1);

  CH_TEST(b_noiseScale0Lv0);
  CH_TEST(b_wTableAdjLv0);
  CH_TEST(b_noiseScale0Lv1);
  CH_TEST(b_wTableAdjLv1);
  CH_TEST(b_noiseScale1Lv0);
  CH_TEST(b_noiseScale1Lv1);
  CH_TEST(b_noiseScale2Lv0);
  CH_TEST(b_noiseScale2Lv1);
  CH_TEST(b_noiseOffLv0);
  CH_TEST(b_noiseOffLv1);
  CH_TEST(b_filterStrLv0);
  CH_TEST(b_filterStrLv1);
  CH_TEST(b_curvOffLv0);
  CH_TEST(b_curvOffLv1);
  CH_TEST(b_sftThrdNoiseScaleLv0);
  CH_TEST(b_sftThrdNoiseShiftLv0);
  CH_TEST(b_sftThrdNoiseScaleLv1);
  CH_TEST(b_sftThrdNoiseShiftLv1);
}/* abf_validate_ch_cfg*/

/*===========================================================================
 * FUNCTION    - abf_validate_tables -
 *
 * DESCRIPTION:
 *==========================================================================*/
void abf_validate_tables(vfe_test_table_t *in, vfe_test_table_t *out, char* tbl_name)
{
  uint32_t i;
  for (i = 0; i < in->size; i++)
    VALIDATE_TST_LUT(in->table[i], out->table[i], 0, tbl_name, i);
} /* abf_validate_tables*/

/*===========================================================================
 * FUNCTION    - vfe_abf_tv_validate -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_abf_tv_validate(void *test_input,
  void *test_output)
{
  ISP_ABF3_CmdType in, out;
  vfe_test_module_input_t* input = (vfe_test_module_input_t *) test_input;
  vfe_test_module_output_t* output = (vfe_test_module_output_t *)test_output;
  abf_populate_data(input->reg_dump, &in);
  abf_populate_data(output->reg_dump, &out);

  abf_validate_master_cfg(&in.cfg, &out.cfg);
  abf_validate_lvl_cfg(&in.level1, &out.level1);
  abf_validate_ch_cfg(&in.ch, &out.ch, "gr");
  /*
  abf_validate_ch_cfg(&in.gb, &out.gb, "gb");
  abf_validate_ch_cfg(&in.r, &out.r, "r");
  abf_validate_ch_cfg(&in.b, &out.b, "b");
  */
  abf_validate_tables(&input->abf_sig2_l0, &output->abf_sig2_l0, "abf_sig2_l0");
  abf_validate_tables(&input->abf_sig2_l1, &output->abf_sig2_l1, "abf_sig2_l1");
  abf_validate_tables(&input->abf_std2_l0, &output->abf_std2_l0, "abf_std2_l0");
  abf_validate_tables(&input->abf_std2_l1, &output->abf_std2_l1, "abf_std2_l1");

  return 0;
}/*vfe_abf_tv_validate*/
