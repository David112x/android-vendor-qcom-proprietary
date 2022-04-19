/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "vfe_test_vector.h"
#include "clf/clf46/clf46.h"
#include "clf/clf46/clf46_reg.h"
#include "camera_dbg.h"
#include <unistd.h>

/*===========================================================================
 * FUNCTION    - clf_validate_nz_flag -
 *
 * DESCRIPTION:
 *==========================================================================*/
void clf_validate_nz_flag(ISP_CLF_NZ_FLAG *in, ISP_CLF_NZ_FLAG
  *out)
{
  VALIDATE_TST_VEC(in->nz_flag_0, out->nz_flag_0, 0, "nz_flag_0");
  VALIDATE_TST_VEC(in->nz_flag_1, out->nz_flag_1, 0, "nz_flag_1");
  VALIDATE_TST_VEC(in->nz_flag_2, out->nz_flag_2, 0, "nz_flag_2");
  VALIDATE_TST_VEC(in->nz_flag_3, out->nz_flag_3, 0, "nz_flag_3");
  VALIDATE_TST_VEC(in->nz_flag_4, out->nz_flag_4, 0, "nz_flag_4");
  VALIDATE_TST_VEC(in->nz_flag_5, out->nz_flag_5, 0, "nz_flag_5");
  VALIDATE_TST_VEC(in->nz_flag_6, out->nz_flag_6, 0, "nz_flag_6");
  VALIDATE_TST_VEC(in->nz_flag_7, out->nz_flag_7, 0, "nz_flag_7");
}/*clf_validate_nz_flag*/

/*===========================================================================
 * FUNCTION    - clf_validate_coeff_cfg -
 *
 * DESCRIPTION:
 *==========================================================================*/
void clf_validate_coeff_cfg(ISP_CLF_COEFF_CFG *in, ISP_CLF_COEFF_CFG
  *out)
{
  VALIDATE_TST_VEC(in->coeff0, out->coeff0, 0, "coeff0");
  VALIDATE_TST_VEC(in->coeff1, out->coeff1, 0, "coeff1");
  VALIDATE_TST_VEC(in->coeff2, out->coeff2, 0, "coeff2");
  VALIDATE_TST_VEC(in->coeff3, out->coeff3, 0, "coeff3");
  VALIDATE_TST_VEC(in->coeff4, out->coeff4, 0, "coeff4");
  VALIDATE_TST_VEC(in->coeff5, out->coeff5, 0, "coeff5");
  VALIDATE_TST_VEC(in->coeff6, out->coeff6, 0, "coeff6");
  VALIDATE_TST_VEC(in->coeff7, out->coeff7, 0, "coeff7");
  VALIDATE_TST_VEC(in->coeff8, out->coeff8, 0, "coeff8");
}/*clf_validate_coeff_cfg*/
/*===========================================================================
 * FUNCTION    - clf_validate_threshold_cfg -
 *
 * DESCRIPTION:
 *==========================================================================*/
void clf_validate_threshold_cfg(ISP_CLF_THRESHOLD_CFG *in, ISP_CLF_THRESHOLD_CFG
  *out)
{
  VALIDATE_TST_VEC(in->edge,   out->edge,   0, "edge");
  VALIDATE_TST_VEC(in->sat,    out->sat,    0, "sat");
  VALIDATE_TST_VEC(in->rg_low, out->rg_low, 0, "rg_low");
  VALIDATE_TST_VEC(in->rg_hi,  out->rg_hi,  0, "rg_hi");
  VALIDATE_TST_VEC(in->bg_low, out->bg_low, 0, "bg_low");
  VALIDATE_TST_VEC(in->bg_hi,  out->bg_hi,  0, "bg_hi");
}/*clf_validate_threshold_cfg*/

/*===========================================================================
 * FUNCTION    - clf_validate_interp_slope_cfg -
 *
 * DESCRIPTION:
 *==========================================================================*/
void clf_validate_interp_slope_cfg(ISP_CLF_INTERP_SLOPE_CFG *in, ISP_CLF_INTERP_SLOPE_CFG
  *out)
{
  VALIDATE_TST_VEC(in->rg_inv, out->rg_inv, 0, "rg_inv");
  VALIDATE_TST_VEC(in->bg_inv, out->bg_inv, 0, "bg_inv");
  VALIDATE_TST_VEC(in->rg_inv_qfactor, out->rg_inv_qfactor, 0, "rg_inv_qfactor");
  VALIDATE_TST_VEC(in->bg_inv_qfactor, out->bg_inv_qfactor, 0, "bg_inv_qfactor");
}/*clf_validate_interp_slope_cfg*/

/*===========================================================================
 * FUNCTION    - clf_validate_interp_slope_cfg -
 *
 * DESCRIPTION:
 *==========================================================================*/
void clf_validate_interp_thresh_cfg(ISP_CLF_INTERP_THRESH_CFG *in, ISP_CLF_INTERP_THRESH_CFG
  *out)
{
  VALIDATE_TST_VEC(in->rg_hi,  out->rg_hi,  0, "rg_hi");
  VALIDATE_TST_VEC(in->rg_low, out->rg_low, 0, "rg_low");
  VALIDATE_TST_VEC(in->bg_hi,  out->bg_hi,  0, "bg_hi");
  VALIDATE_TST_VEC(in->bg_low, out->bg_low, 0, "bg_low");
}/*clf_validate_interp_slope_cfg*/

/*===========================================================================
 * FUNCTION    - clf_populate_data -
 *
 * DESCRIPTION:
 *==========================================================================*/
void clf_populate_data(uint32_t *reg, ISP_CLF_CmdType *pcmd)
{
  void *dst = (void *)pcmd;
  uint32_t size = ISP_CLF46_LEN * sizeof(uint32_t);
  memcpy(dst, (void *)(reg + ISP_CLF46_HV_NZ_FLAG_OFF/4), size);
}

/*===========================================================================
 * FUNCTION    - vfe_clf_tv_validate -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_clf_tv_validate(void* test_input,
  void* test_output)
{
  vfe_test_module_input_t *input = (vfe_test_module_input_t *)test_input;
  vfe_test_module_output_t *output = (vfe_test_module_output_t *)test_output;
  ISP_CLF_CmdType in, out;

  clf_populate_data(input->reg_dump, &in);
  clf_populate_data(output->reg_dump, &out);

  clf_validate_nz_flag(&in.edge_hv_nz_flag, &out.edge_hv_nz_flag);
  clf_validate_nz_flag(&in.edge_diag_nz_flag, &out.edge_diag_nz_flag);
  clf_validate_coeff_cfg(&in.edge_hv_coeff, &out.edge_hv_coeff);
  clf_validate_coeff_cfg(&in.edge_diag_coeff, &out.edge_diag_coeff);
  clf_validate_threshold_cfg(&in.threshold, &out.threshold);
  clf_validate_interp_slope_cfg(&in.interp_slope, &out.interp_slope);
  clf_validate_interp_thresh_cfg(&in.interp_thresh, &out.interp_thresh);

  return 0;
}/*vfe_clf_tv_validate*/
