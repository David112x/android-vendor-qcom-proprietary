/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "vfe_test_vector.h"
#include "snr/snr47/snr47.h"
#include "snr47_reg.h"
#include "camera_dbg.h"
#include "isp_pipeline_reg.h"
#include <unistd.h>

/*===========================================================================
 * FUNCTION    - vfe_snr_test_vector_validate -
 *
 * DESCRIPTION: this function compares the test vector output with hw output
 *==========================================================================*/
int vfe_snr_tv_validate( void *input, void *output)
{
  vfe_test_module_input_t *mod_in = (vfe_test_module_input_t *)input;
  vfe_test_module_output_t *mod_op = (vfe_test_module_output_t *)output;
  ISP_SnrCfg_CmdType *in_cfgCmd, *out_cfgCmd;
  ISP_Snr_CmdType    *in_cmd, *out_cmd;

  ISP_CAC_CFG *module_enable_reg_in = NULL;
  ISP_CAC_CFG *module_enable_reg_out = NULL;

  CDBG("%s:\n", __func__);

  in_cfgCmd = (ISP_SnrCfg_CmdType *)(mod_in->reg_dump + (ISP_SNR47_CFG_OFF/4));
  out_cfgCmd = (ISP_SnrCfg_CmdType *)(mod_op->reg_dump + (ISP_SNR47_CFG_OFF/4));

  in_cmd = (ISP_Snr_CmdType *)(mod_in->reg_dump + (ISP_SNR47_OFF/4));
  out_cmd = (ISP_Snr_CmdType *)(mod_op->reg_dump + (ISP_SNR47_OFF/4));

  module_enable_reg_in = (ISP_CAC_CFG *)(mod_in->reg_dump + (ISP_CAC_SNR_OFFSET/4));
  module_enable_reg_out = (ISP_CAC_CFG *)(mod_op->reg_dump + (ISP_CAC_SNR_OFFSET/4));

  if (module_enable_reg_in->fields.snr_enable == 0) {
    printf("\t(SNR disabled by Simulator)\n");
    return 0;
  }

  /* ISP_Snr_Filter_Type  NR */
  VALIDATE_TST_VEC(in_cmd->nr_filter.coeff0 , out_cmd->nr_filter.coeff0 , 0, "NR coeff0");
  VALIDATE_TST_VEC(in_cmd->nr_filter.coeff1 , out_cmd->nr_filter.coeff1 , 0, "NR coeff1");
  VALIDATE_TST_VEC(in_cmd->nr_filter.coeff2 , out_cmd->nr_filter.coeff2 , 0, "NR coeff2");
  VALIDATE_TST_VEC(in_cmd->nr_filter.coeff3 , out_cmd->nr_filter.coeff3 , 0, "NR coeff3");

  /* ISP_Snr_Filter_Type  Y */
  VALIDATE_TST_VEC(in_cmd->y_filter.coeff0 , out_cmd->y_filter.coeff0 , 0, " Y coeff0");
  VALIDATE_TST_VEC(in_cmd->y_filter.coeff1 , out_cmd->y_filter.coeff1 , 0, " Y coeff1");
  VALIDATE_TST_VEC(in_cmd->y_filter.coeff2 , out_cmd->y_filter.coeff2 , 0, " Y coeff2");
  VALIDATE_TST_VEC(in_cmd->y_filter.coeff3 , out_cmd->y_filter.coeff3 , 0, " Y coeff3");

  /* ISP_Snr_Filter_Type  C */
  VALIDATE_TST_VEC(in_cmd->c_filter.coeff0 , out_cmd->c_filter.coeff0 , 0, "c coeff0");
  VALIDATE_TST_VEC(in_cmd->c_filter.coeff1 , out_cmd->c_filter.coeff1 , 0, "c coeff1");
  VALIDATE_TST_VEC(in_cmd->c_filter.coeff2 , out_cmd->c_filter.coeff2 , 0, "c coeff2");
  VALIDATE_TST_VEC(in_cmd->c_filter.coeff3 , out_cmd->c_filter.coeff3 , 0, "c coeff3");

  /* Border Cfg0 */
  VALIDATE_TST_VEC(in_cmd->border_cfg0.ymin , out_cmd->border_cfg0.ymin , 0, "ymin");
  VALIDATE_TST_VEC(in_cmd->border_cfg0.ymax , out_cmd->border_cfg0.ymax , 0, "ymax");
  VALIDATE_TST_VEC(in_cmd->border_cfg0.shymin , out_cmd->border_cfg0.shymin , 0, "shymin");
  VALIDATE_TST_VEC(in_cmd->border_cfg0.shymax , out_cmd->border_cfg0.shymax , 0, "shymax");

  /* Border Cfg1 */
  VALIDATE_TST_VEC(in_cmd->border_cfg1.smin_para , out_cmd->border_cfg1.smin_para , 0, "smin_para");
  VALIDATE_TST_VEC(in_cmd->border_cfg1.smax_para , out_cmd->border_cfg1.smax_para , 0, "smax_para");
  VALIDATE_TST_VEC(in_cmd->border_cfg1.boundary_prob , out_cmd->border_cfg1.boundary_prob , 0, "boundary_prob");
  VALIDATE_TST_VEC(in_cmd->border_cfg1.hmin , out_cmd->border_cfg1.hmin , 0, "hmin");

  /* Border Cfg2 */
  VALIDATE_TST_VEC(in_cmd->border_cfg2.hmax , out_cmd->border_cfg2.hmax , 0, "hmax");
  VALIDATE_TST_VEC(in_cmd->border_cfg2.q_skin , out_cmd->border_cfg2.q_skin , 0, "q_skin");
  VALIDATE_TST_VEC(in_cmd->border_cfg2.q_nonskin , out_cmd->border_cfg2.q_nonskin , 0, "q_nonskin");

  /*prob filer*/
  VALIDATE_TST_VEC(in_cmd->prob_filter.coeff0 , out_cmd->prob_filter.coeff0 , 0, "prob coeff0");
  VALIDATE_TST_VEC(in_cmd->prob_filter.coeff1 , out_cmd->prob_filter.coeff1 , 0, "prob coeff1");
  VALIDATE_TST_VEC(in_cmd->prob_filter.coeff2 , out_cmd->prob_filter.coeff2 , 0, "prob coeff2");
  VALIDATE_TST_VEC(in_cmd->prob_filter.coeff3 , out_cmd->prob_filter.coeff3 , 0, "prob coeff3");

  /*prob filer 1*/
  VALIDATE_TST_VEC(in_cmd->prob_filter1.coeff0 , out_cmd->prob_filter1.coeff0 , 0, "prob1 coeff0");
  VALIDATE_TST_VEC(in_cmd->prob_filter1.coeff1 , out_cmd->prob_filter1.coeff1 , 0, "prob1 coeff1");
  VALIDATE_TST_VEC(in_cmd->prob_filter1.coeff2 , out_cmd->prob_filter1.coeff2 , 0, "prob1 coeff2");
  VALIDATE_TST_VEC(in_cmd->prob_filter1.coeff3 , out_cmd->prob_filter1.coeff3 , 0, "prob1 coeff3");

  /*skin tone cfg*/
  VALIDATE_TST_VEC(in_cmd->skinTone_cfg.soft_thr, out_cmd->skinTone_cfg.soft_thr , 0, "soft_thr");
  VALIDATE_TST_VEC(in_cmd->skinTone_cfg.soft_strength , out_cmd->skinTone_cfg.soft_strength , 0, "soft_strength");

  return 0;
}
