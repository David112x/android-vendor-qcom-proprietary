/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================*/

#include "vfe_test_vector.h"
#include "camera_dbg.h"
#include <ctype.h>

#include "../module/isp48/isp_pipeline_reg.h"
#include "gamma_reg.h" // for ISP_GAMMA_NUM_ENTRIES
#include "mesh_rolloff_reg.h" // for ISP_MESH_ROLLOFF44_TABLE_SIZE
//#include "linearization/linearization40/linearization40.h"
//#include "abf48.h"
#include "abf48_reg.h"
#include "linearization_reg.h"
#include "ltm_reg.h"
#include "isp_module.h"
#include "isp_hw_update_util.h"
//#include "../hw/gic/gic48/gic48.h"
#include "gic_reg.h"
//#include "../hw/abf/abf48/abf48.h"
#include "../hw/pedestal_correct/pedestal_correct46/pedestal_correct46.h"
#include "../hw/gtm/gtm46/gtm46.h"
#include "gtm_reg.h"
#include "pdaf_reg.h"

extern int vfe_abf_tv_validate(void *in, void *op);
extern int vfe_bcc_tv_validate(void *in, void *op);
extern int vfe_bpc_tv_validate(void *in, void *op);
extern int vfe_chroma_enhance_tv_validate(void *in, void *op);
extern int vfe_chroma_suppression_tv_validate(void *in, void *op);
extern int vfe_clf_tv_validate(void *in, void *op);
extern int vfe_color_correct_tv_validate(void *in, void *op);
extern int vfe_demosaic_tv_validate(void *in, void *op);
extern int vfe_gamma_tv_validate(void *in, void *op);
extern int vfe_wb_tv_validate(void *in, void *op);
extern int vfe_mce_tv_validate(void *input, void *output);
extern int vfe_sce_tv_validate(void *input, void *output);
extern int vfe_gic_tv_validate(void *input, void *output);
extern int vfe_pedestal_tv_validate(void *input, void *output);
extern int vfe_gtm_tv_validate(void *input, void *output);
extern int vfe_mesh_rolloff_tv_validate(void *in, void *op);
extern int vfe_linearization_tv_validate(void *input, void *output);
extern int vfe_colorxform_tv_validate(void *input, void *output);

extern int vfe_cac_tv_validate(void *input, void *output);
extern int vfe_snr_tv_validate(void *input, void *output);
extern int vfe_black_level_tv_validate(void *input, void *output);
extern int vfe_demux_tv_validate(void *input, void *output);
extern int vfe_ltm_tv_validate(void *input, void *output);
extern int vfe_hdr_tv_validate(void *input, void *output);
extern int vfe_pdaf_tv_validate(void *in, void *op);

extern int bet_hw_params_fd[ISP_HW_MAX];
extern int show_mismatch;
extern int show_match;
extern int has_mismatch;

static isp_bet_test_list_t bet_test_list =  {
  /*moduile name*/
  .vfe_module_name = {
     [ISP_MOD_GAMMA]                = "Gamma",
     [ISP_MOD_COLOR_CORRECT]        = "Color Correct",
     [ISP_MOD_CHROMA_ENHANCE]       = "Chroma Enhance",
     [ISP_MOD_GIC]                  = "GIC",
     [ISP_MOD_GTM]                  = "GTM",
     [ISP_MOD_CHROMA_SUPPRESS]      = "Chroma Supress",
     [ISP_MOD_ROLLOFF]              = "Mesh Rolloff",
     [ISP_MOD_MCE]                  = "MCE",
     [ISP_MOD_SCE]                  = "SCE",
     [ISP_MOD_LINEARIZATION]        = "LINEARIZATION",
     [ISP_MOD_BPC]                  = "BPC",
     [ISP_MOD_ABF]                  = "ABF",
     [ISP_MOD_DEMOSAIC]             = "Demosaic",
     [ISP_MOD_PEDESTAL]             = "Pedestal",
     [ISP_MOD_CAC]                  = "CAC2",
     [ISP_MOD_SNR]                  = "SNR",
     [ISP_MOD_BLS]                  = "BLS",
     [ISP_MOD_DEMUX]                = "Demux",
     [ISP_MOD_COLOR_XFORM_VIEWFINDER]  = "Color Xform",
     [ISP_MOD_LTM]                  = "LTM",
     [ISP_MOD_HDR]                  = "Video HDR",
     [ISP_MOD_PDAF]                 = "PDAF",
  },
  /*vfe tv validate func table*/
  .vfe_tv_validate_func = {
     [ISP_MOD_GAMMA]               = vfe_gamma_tv_validate,
     [ISP_MOD_COLOR_CORRECT]       = vfe_color_correct_tv_validate,
     [ISP_MOD_CHROMA_ENHANCE]      = vfe_chroma_enhance_tv_validate,
     [ISP_MOD_GIC]                 = vfe_gic_tv_validate,
     [ISP_MOD_GTM]                 = vfe_gtm_tv_validate,
     [ISP_MOD_CHROMA_SUPPRESS]     = vfe_chroma_suppression_tv_validate,
     [ISP_MOD_ROLLOFF]             = vfe_mesh_rolloff_tv_validate,
     [ISP_MOD_MCE]                 = vfe_mce_tv_validate,
     [ISP_MOD_SCE]                 = vfe_sce_tv_validate,
     [ISP_MOD_LINEARIZATION]       = vfe_linearization_tv_validate,
     [ISP_MOD_BPC]                 = vfe_bpc_tv_validate,
     [ISP_MOD_ABF]                 = vfe_abf_tv_validate,
     [ISP_MOD_DEMOSAIC]            = vfe_demosaic_tv_validate,
     [ISP_MOD_BLS]                 = vfe_black_level_tv_validate,
     [ISP_MOD_DEMUX]               = vfe_demux_tv_validate,
     [ISP_MOD_COLOR_XFORM_VIEWFINDER] = vfe_colorxform_tv_validate,
     [ISP_MOD_LTM]                 = vfe_ltm_tv_validate,
     [ISP_MOD_HDR]                 = vfe_hdr_tv_validate,
     [ISP_MOD_PDAF]                = vfe_pdaf_tv_validate,
#ifdef MSMCOBALT_MSM8998
     [ISP_MOD_PEDESTAL]            = vfe_pedestal_tv_validate,
     [ISP_MOD_CAC]                 = vfe_cac_tv_validate,
     [ISP_MOD_SNR]                 = vfe_snr_tv_validate,
#endif
  },
    /*legacy module*/
   .show_mismatch = {
     [ISP_MOD_GAMMA]               = 0,
     [ISP_MOD_COLOR_CORRECT]       = 0,
     [ISP_MOD_CHROMA_ENHANCE]      = 0 ,
     [ISP_MOD_GIC]                 = 0,
     [ISP_MOD_GTM]                 = 0,
     [ISP_MOD_CHROMA_SUPPRESS]     = 0,
     [ISP_MOD_ROLLOFF]             = 0,
     [ISP_MOD_MCE]                 = 0,
     [ISP_MOD_SCE]                 = 0,
     [ISP_MOD_LINEARIZATION]       = 0,
     [ISP_MOD_BPC]                 = 0,
     [ISP_MOD_ABF]                 = 0,
     [ISP_MOD_DEMOSAIC]            = 0,
     [ISP_MOD_BLS]                 = 0,
     [ISP_MOD_DEMUX]               = 0,
     [ISP_MOD_COLOR_XFORM_VIEWFINDER] = 0,
     [ISP_MOD_LTM]                 = 0,
     [ISP_MOD_HDR]                 = 0,
     [ISP_MOD_PDAF]                = 0,
#ifdef MSMCOBALT_MSM8998
     [ISP_MOD_PEDESTAL]            = 0,
     [ISP_MOD_CAC]                 = 0,
     [ISP_MOD_SNR]                 = 0,
#endif
   },
  .show_match = {
     [ISP_MOD_GAMMA]               = 0,
     [ISP_MOD_COLOR_CORRECT]       = 0,
     [ISP_MOD_CHROMA_ENHANCE]      = 0,
     [ISP_MOD_GIC]                 = 0,
     [ISP_MOD_GTM]                 = 0,
     [ISP_MOD_CHROMA_SUPPRESS]     = 0,
     [ISP_MOD_ROLLOFF]             = 0,
     [ISP_MOD_MCE]                 = 0,
     [ISP_MOD_SCE]                 = 0,
     [ISP_MOD_LINEARIZATION]       = 0,
     [ISP_MOD_BPC]                 = 0,
     [ISP_MOD_ABF]                 = 0,
     [ISP_MOD_DEMOSAIC]            = 0,
     [ISP_MOD_BLS]                 = 0,
     [ISP_MOD_DEMUX]               = 0,
     [ISP_MOD_COLOR_XFORM_VIEWFINDER] = 0,
     [ISP_MOD_LTM]                 = 0,
     [ISP_MOD_HDR]                 = 0,
     [ISP_MOD_PDAF]                = 0,
#ifdef MSMCOBALT_MSM8998
     [ISP_MOD_PEDESTAL]            = 0,
     [ISP_MOD_CAC]                 = 0,
     [ISP_MOD_SNR]                 = 0,
#endif
  },
};

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#define VFE_TEST_VEC_PARSE_INPUT 0
#define VFE_TEST_VEC_PARSE_OUTPUT 1

#define VFE_TEST_VEC_FALSE "FALSE"
#define VFE_TEST_VEC_TRUE "TRUE"

#define CASE_NUMBER              "case number"
#define CASE_NAME                "case name"
#define AWB_R_GAIN               "AWB_r_gain"
#define AWB_G_GAIN               "AWB_g_gain"
#define AWB_B_GAIN               "AWB_b_gain"
#define AWB_DECISION             "AWB_decision"
#define AWB_CCT                  "AWB_CCT"
#define LED_ENABLE               "LED_enable"
#define STROBE_ENABLE            "Strobe_enable"
#define LED_SENSITIVITY_OFF      "LED_sensitivity_off"
#define LED_SENSITIVITY_HIGH     "LED_sensitivity_high"
#define STROBE_SENSITIVITY_OFF   "strobe_sensitivity_off"
#define STROBE_SENSITIVITY_HIGH  "strobe_sensitivity_high"
#define LUX_INDEX                "Lux_index"
#define TOTAL_GAIN               "total_gain"
#define DIGITAL_GAIN             "digital_gain"
#define CAMIF_WIDTH              "CAMIF_width"
#define CAMIF_HEIGHT             "CAMIF_height"
#define OUTPUT_WIDTH             "output_width"
#define OUTPUT_HEIGHT            "output_height"
#define X_OFFSET                 "subgridh_offset"
#define Y_OFFSET                 "subgridv_offset"
#define SENSOR_SCALING           "sensor_scaling"
#define FULL_WIDTH               "Full_width"
#define FULL_HEIGHT              "Full_height"
#define AEC_SETTLE               "AEC_settle"
#define AEC_CONVERGENCE          "AEC_convergence"
#define AEC_MAX                  "AEC_max"
#define IHIST_STATS              "iHist"
#define BG_RGN_H_NUM             "bgrid_rgn_h_num"
#define BG_RGN_V_NUM             "bgrid_rgn_v_num"
#define BG_R_SUM                 "bgrid_r_sum"
#define BG_R_CNT                 "bgrid_r_cnt"
#define BG_GR_SUM                "bgrid_gr_sum"
#define BG_GR_CNT                "bgrid_gr_cnt"
#define BG_GB_SUM                "bgrid_gb_sum"
#define BG_GB_CNT                "bgrid_gb_cnt"
#define BG_B_SUM                 "bgrid_b_sum"
#define BG_B_CNT                 "bgrid_b_cnt"
#define BHIST_GR                 "bihist_gr_bin"
#define MODE                     "Mode"
#define HDR_MODE                 "hdrMode"
#define PREVIEW                  "Preview"
#define NON_HDR_MODE             "Non-HDR"
#define LED1_CURRENT             "led1_current"
#define LED2_CURRENT             "led2_current"
#define BAYER_PATTERN            "bayer_pattern"
#define SENSOR_HDR               "MODE_SENSOR_HDR"
#define HDR                      "HDR"
#define EXP_RATIO                "exp_ratio"
#define EXP_TIME_RATIO           "exp_time_ratio"
#define ADRC_ENABLE              "adrc_enable"
#define ADRC_COLOR_GAIN          "DRC_color_gain"
#define ADRC_TOTAL_GAIN          "DRC_total_gain"
#define ADRC_GAMMA_RATIO         "DRC_Gamma_ratio"
#define ADRC_LTM_RATIO           "DRC_LTM_ratio"
#define ADRC_LA_RATIO            "DRC_LA_ratio"
#define ADRC_GTM_RATIO           "DRC_GTM_ratio"
#define BSD                      "backlit_scene_enable"
#define PDAF_OFFSET_X            "pdaf_global_offset_x"
#define PDAF_OFFSET_Y            "pdaf_global_offset_y"
#define PDAF_PIXEL_COUNT         "pdaf_pixel_count"
#define PDAF_PIXEL_COORDS        "pdaf_pixel_coords"

#define VFE_TEST_VEC_COLOR_CORRECTION      "Color Correction"
#define VFE_TEST_VEC_DEMOSAIC              "Demosaic"
#define VFE_TEST_VEC_BPC                   "BPC"
#define VFE_TEST_VEC_BCC                   "BCC"
#define VFE_TEST_VEC_LINEARIZATION         "Linearization"
#define VFE_TEST_VEC_COLOR_CONVERSION      "Color_Conversion"
#define VFE_TEST_VEC_SCE                   "Skin_Color_Enhancement"
#define VFE_TEST_VEC_MESH_ROLLOFF          "Mesh_Lens_Rolloff"
#define VFE_TEST_VEC_GAMMA                 "Gamma"
#define VFE_TEST_VEC_ASF7                  "ASF_7x7"
#define VFE_TEST_VEC_ASF9                  "ASF9x9"
#define VFE_TEST_VEC_ABF                   "ABF3"
#define VFE_TEST_VEC_AWB                   "AWB"
#define VFE_TEST_VEC_MCE                   "MCE"
#define VFE_TEST_VEC_CS                    "CS"
#define VFE_TEST_VEC_CST                   "Color Space Transform"
#define VFE_TEST_VEC_GIC                   "Green Imbalance Correction"
#define VFE_TEST_VEC_PEDESTAL              "Pedestal Black Level Correction"
#define VFE_TEST_VEC_GTM                   "Global Tone Mapping"
#define VFE_TEST_VEC_LTM                   "LTM"
#define VFE_TEST_VEC_CAC                   "Chromatic_Aberration_Correction"
#define VFE_TEST_VEC_SNR                   "Skintone NR"
#define VFE_TEST_VEC_BLS                   "Black Level Subtraction"
#define VFE_TEST_VEC_DEMUX                 "Demux"
#define VFE_TEST_VEC_HDR                   "Video HDR"
#define VFE_TEST_VEC_PDAF                  "PDAF Pixel Correction"

#define VFE_TEST_VEC_ADDRESS "address"

#define VFE_TEST_VEC_IS_TRUE(bool_str) \
  (0 == strcmp(bool_str, VFE_TEST_VEC_TRUE))

#define _FREE(ptr) \
  if (ptr) { \
    free(ptr); \
    ptr = NULL; \
  }

static void vfe_test_vector_output_deinit(vfe_test_vector_t *mod);
static vfe_status_t vfe_test_vector_output_init(vfe_test_vector_t *mod);

typedef struct {
  uint32_t read_type;
  uint32_t read_length;
  uint32_t read_bank;
  uint32_t bank_idx;
} isp2_hw_read_info;

typedef struct {
  uint32_t set_channel;
  uint32_t set_start_addr;
  uint32_t dmi_tbl[MAX_DMI_TBL_SIZE];
  uint32_t reset_channel;
  uint32_t reset_start_addr;
} isp2_hw_dmi_dump_t;

/** vfe_test_vector_cgc_on
 *    @hw_params: isp hw update list params
 *
 *    read register dump when receive SOF evt will be the most
 *    acurate timing to dump the register
 *
 *   Return none
 **/
int vfe_test_vector_cgc_on()
{
  int rc =  0;
  struct msm_vfe_cfg_cmd2       cfg_cmd;
  struct msm_vfe_reg_cfg_cmd    reg_cfg_cmd[4];
  struct msm_vfe_cfg_cmd_list   cur_hw_update_list;
  isp_hw_id_t                   hw_id = 0;
  uint32_t cmd_offset, cmd_len;
  uint32_t                      cgc_setting[4];
  uint32_t                      cgc_offset[4];

  memset(cgc_setting, 0 , 4 * sizeof(uint32_t));
  memset(cgc_offset, 0 , 4 * sizeof(uint32_t));

  cgc_setting[0] = 0xFFFFFFFF;
  cgc_setting[1] = 0xFFFFFFFF;
  cgc_setting[2] = 0xFFFFFFFF;
  cgc_setting[3] = 0xFFFFFFFF;

  cgc_offset[0] = MODULE_LENS_CGC_OVERRIDE;
  cgc_offset[1] = MODULE_STATS_CGC_OVERRIDE;
  cgc_offset[2] = MODULE_COLOR_CGC_OVERRIDE;
  cgc_offset[3] = MODULE_ZOOM_CGC_OVERRIDE;

  memset(&cfg_cmd, 0, sizeof(cfg_cmd));
  memset(&reg_cfg_cmd, 0, sizeof(reg_cfg_cmd));

  /* 1. program DMI default value, write auto increment bit
     2. write DMI table
     3. reset DMI cfg */
  cfg_cmd.cfg_data = (void *)&cgc_setting[0];
  cfg_cmd.cmd_len = 4 * sizeof(uint32_t);
  cfg_cmd.cfg_cmd = (void *) &reg_cfg_cmd;
  cfg_cmd.num_cfg = 4;

  cmd_offset = 0;
  cmd_len = 1 * sizeof(uint32_t);
  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[0].cmd_type = VFE_WRITE_MB;
  reg_cfg_cmd[0].u.rw_info.reg_offset = cgc_offset[0];
  reg_cfg_cmd[0].u.rw_info.len = cmd_len;

  cmd_offset += cmd_len;
  cmd_len = 1 * sizeof(uint32_t);
  reg_cfg_cmd[1].u.rw_info.cmd_data_offset = cmd_offset;
  reg_cfg_cmd[1].cmd_type = VFE_WRITE_MB;
  reg_cfg_cmd[1].u.rw_info.reg_offset =  cgc_offset[1];
  reg_cfg_cmd[1].u.rw_info.len = cmd_len;

  cmd_offset += cmd_len;
  cmd_len = 1 * sizeof(uint32_t);
  reg_cfg_cmd[2].u.rw_info.cmd_data_offset = cmd_offset;
  reg_cfg_cmd[2].cmd_type = VFE_WRITE_MB;
  reg_cfg_cmd[2].u.rw_info.reg_offset =  cgc_offset[2];
  reg_cfg_cmd[2].u.rw_info.len = cmd_len;

  cmd_offset += cmd_len;
  cmd_len = 1 * sizeof(uint32_t);
  reg_cfg_cmd[3].u.rw_info.cmd_data_offset = cmd_offset;
  reg_cfg_cmd[3].cmd_type = VFE_WRITE_MB;
  reg_cfg_cmd[3].u.rw_info.reg_offset =  cgc_offset[3];
  reg_cfg_cmd[3].u.rw_info.len = cmd_len;

  cur_hw_update_list.cfg_cmd = cfg_cmd;
  cur_hw_update_list.next = NULL;
  cur_hw_update_list.next_size = 0;

  for (hw_id = 0; hw_id < ISP_HW_MAX; hw_id++) {
    if (bet_hw_params_fd[hw_id] > 0) {
      rc = ioctl(bet_hw_params_fd[hw_id], VIDIOC_MSM_VFE_REG_LIST_CFG,
                 &cur_hw_update_list);
      if (rc < 0) {
        printf("%s: isp read register error = %d\n", __func__, rc);
        return rc;
      }
      break;
    }
  }

  return rc;
}
/** isp2_hw_read_reg_dump
 *    @hw_params: isp hw update list params
 *
 *    read register dump when receive SOF evt will be the most
 *    acurate timing to dump the register
 *
 *   Return none
 **/
int isp2_hw_read_reg_dump(void *dump_entry, uint32_t read_type, uint32_t read_len)
{
  int rc = 0;
  uint32_t i;
  struct msm_vfe_cfg_cmd2       cfg_cmd;
  struct msm_vfe_reg_cfg_cmd    reg_cfg_cmd;
  struct msm_vfe_cfg_cmd_list   cur_hw_update_list;
  isp_hw_id_t                   hw_id = 0;

  memset(&cfg_cmd, 0, sizeof(cfg_cmd));
  memset(&reg_cfg_cmd, 0, sizeof(reg_cfg_cmd));
  memset(dump_entry, 0, read_len);

  cfg_cmd.cfg_data = dump_entry;
  cfg_cmd.cmd_len = read_len;
  cfg_cmd.cfg_cmd = (void *)&reg_cfg_cmd;
  cfg_cmd.num_cfg = 1;
  reg_cfg_cmd.u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd.cmd_type = read_type;
  reg_cfg_cmd.u.rw_info.len = read_len;
  reg_cfg_cmd.u.rw_info.reg_offset = 0;

  cur_hw_update_list.cfg_cmd = cfg_cmd;
  cur_hw_update_list.next = NULL;
  cur_hw_update_list.next_size = 0;

  for (hw_id = 0; hw_id < ISP_HW_MAX; hw_id++) {
    if (bet_hw_params_fd[hw_id] > 0) {
      rc = ioctl(bet_hw_params_fd[hw_id], VIDIOC_MSM_VFE_REG_LIST_CFG,
                 &cur_hw_update_list);
      if (rc < 0) {
        printf("%s: isp read register error = %d\n", __func__, rc);
        return rc;
      }
      break;
    }
  }

  return rc;
}
/** isp2_hw_read_dmi_dump
 *    @isp_hw: isp hardware object
 *
 *    read register dump when receive SOF evt will be the most
 *    acurate timing to dump the register
 *
 *   Return none
 **/

int isp2_hw_read_dmi_dump(void *dump_entry, isp2_hw_read_info *dmi_read_info,
                          uint32_t isp_dmi_cfg_offset, uint32_t isp_dmi_addr)
{
  int rc = 0;
  uint32_t i;

  struct msm_vfe_cfg_cmd2       cfg_cmd;
  struct msm_vfe_reg_cfg_cmd    reg_cfg_cmd[5];
  struct msm_vfe_cfg_cmd_list   cur_hw_update_list;
  isp_hw_id_t                   hw_id = 0;

  isp2_hw_dmi_dump_t dmi_cfg;
  uint32_t cmd_offset, cmd_len, dmi_channel;
  dmi_channel = dmi_read_info->read_bank + dmi_read_info->bank_idx;

  memset(&cfg_cmd, 0, sizeof(cfg_cmd));
  memset(&reg_cfg_cmd, 0, sizeof(reg_cfg_cmd));
  memset(dump_entry, 0, dmi_read_info->read_length);

  /* 1. program DMI default value, write auto increment bit
     2. write DMI table
     3. reset DMI cfg */
  cfg_cmd.cfg_data = (void *)&dmi_cfg;
  cfg_cmd.cmd_len = sizeof(dmi_cfg);
  cfg_cmd.cfg_cmd = (void *) &reg_cfg_cmd;
  cfg_cmd.num_cfg = 5;

  CDBG_ERROR("%s: bet_dbg read dmi channel %x, table start address %p\n",
    __func__, dmi_channel, &dmi_cfg.dmi_tbl[0]);
  /* set dmi to proper hist stats bank */
  dmi_cfg.set_channel = 0x100 + dmi_channel;
  cmd_offset = 0;
  cmd_len = 1 * sizeof(uint32_t);
  /* set dmi to proper linearization bank */
  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[0].cmd_type = VFE_WRITE_MB;
  reg_cfg_cmd[0].u.rw_info.reg_offset = isp_dmi_cfg_offset;
  reg_cfg_cmd[0].u.rw_info.len = 1 * sizeof(uint32_t);

  /* set start addr = 0*/
  dmi_cfg.set_start_addr = 0;
  cmd_offset += cmd_len;
  cmd_len = 1 * sizeof(uint32_t);
  reg_cfg_cmd[1].u.rw_info.cmd_data_offset = cmd_offset;
  reg_cfg_cmd[1].cmd_type = VFE_WRITE_MB;
  reg_cfg_cmd[1].u.rw_info.reg_offset = isp_dmi_addr;
  reg_cfg_cmd[1].u.rw_info.len = 1 * sizeof(uint32_t);

  /* memset dmi tbl = all 0,
     DMI read: according to read_type
     cmd_len= size of the void pointer
     table len : size of the dmi table size from read_len
     hi_tbl_offset = 0
     lo_tbl_offset = dmi_tbl offset_offset */

  memset(dmi_cfg.dmi_tbl, 0, dmi_read_info->read_length);
  cmd_offset += cmd_len;
  cmd_len = MAX_DMI_TBL_SIZE * sizeof(uint32_t);
  /* read dmi data */
  reg_cfg_cmd[2].cmd_type = dmi_read_info->read_type;
  reg_cfg_cmd[2].u.dmi_info.hi_tbl_offset = cmd_offset + sizeof(uint32_t);
  reg_cfg_cmd[2].u.dmi_info.lo_tbl_offset = cmd_offset;
  reg_cfg_cmd[2].u.dmi_info.len = dmi_read_info->read_length;

  /* reset the start addr = 0 */
  dmi_cfg.reset_channel = 0x100;
  cmd_offset += cmd_len;
  cmd_len = 1 * sizeof(uint32_t);
  /* reset dmi to no bank*/
  reg_cfg_cmd[3].u.rw_info.cmd_data_offset = cmd_offset;
  reg_cfg_cmd[3].cmd_type = VFE_WRITE_MB;
  reg_cfg_cmd[3].u.rw_info.reg_offset = isp_dmi_cfg_offset;
  reg_cfg_cmd[3].u.rw_info.len = 1 * sizeof(uint32_t);

  /* set dmi to proper hist stats bank */
  dmi_cfg.reset_start_addr = 0;
  cmd_offset += cmd_len;
  cmd_len = 1 * sizeof(uint32_t);
  reg_cfg_cmd[4].u.rw_info.cmd_data_offset = cmd_offset;
  reg_cfg_cmd[4].cmd_type = VFE_WRITE_MB;
  reg_cfg_cmd[4].u.rw_info.reg_offset = isp_dmi_addr;
  reg_cfg_cmd[4].u.rw_info.len = 1 * sizeof(uint32_t);

  cur_hw_update_list.cfg_cmd = cfg_cmd;
  cur_hw_update_list.next = NULL;
  cur_hw_update_list.next_size = 0;

  for (hw_id = 0; hw_id < ISP_HW_MAX; hw_id++) {
    if (bet_hw_params_fd[hw_id] > 0) {
      rc = ioctl(bet_hw_params_fd[hw_id], VIDIOC_MSM_VFE_REG_LIST_CFG,
                 &cur_hw_update_list);
      if (rc < 0) {
        printf("%s: isp read register error = %d\n", __func__, rc);
        return rc;
      }
      break;
    }
  }

  memcpy(dump_entry, &dmi_cfg.dmi_tbl[0], dmi_read_info->read_length);
  return rc;
}


/*===========================================================================
 * FUNCTION    - test_vector_malloc_table -
 *
 * DESCRIPTION:
 *==========================================================================*/
 static vfe_status_t test_vector_malloc_table(
   vfe_test_table_t *table, uint32_t size)
{
  table->size = size;
  CDBG_ERROR("%s:malloc size %d", __func__, size);
  table->table = (uint32_t *)malloc((size) * sizeof(uint32_t));
  if (!table->table) { \
    CDBG_ERROR("%s:%d no memory", __func__, __LINE__);
    return VFE_ERROR_NO_MEMORY;
  }
  memset(table->table, 0, size * sizeof(uint32_t));
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - test_vector_malloc_table -
 *
 * DESCRIPTION:
 *==========================================================================*/
static vfe_status_t test_vector_malloc_table_64(
   vfe_test_table_64_t *table, uint32_t size)
{
  table->size = size;
  CDBG_ERROR("%s:malloc size %d", __func__, size);
  table->table = (uint64_t *)malloc((size) * sizeof(uint64_t));
  if (!table->table) { \
    CDBG_ERROR("%s:%d no memory", __func__, __LINE__);
    return VFE_ERROR_NO_MEMORY;
  }
  memset(table->table, 0, (size) * sizeof(uint64_t));
  return VFE_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - test_vector_str_to_lower -
 *
 * DESCRIPTION:
 *==========================================================================*/
static char* test_vector_str_to_lower(char *str)
{
  int i = 0;
  int len = strlen(str);
  for (i=0; i<len; i++)
    str[i] = tolower(str[i]);
  return str;
} /*test_vector_str_to_lower*/

/*===========================================================================
 * FUNCTION    - test_vector_str_to_token -
 *
 * DESCRIPTION:
 *==========================================================================*/
static char* test_vector_str_to_token(char *str)
{
  int i = 0;
  int len = 0;
  /* remove starting space*/
  while(*str == ' ')
    str++;

  if (*str == '\"')
    str++;
  len = strlen(str);
  for (i=(len-1); i>=0; i--) {
    if (str[i] != ' ')
      break;
  }
  if (str[i] == '\"')
    i--;
  str[i+1] = '\0';
  return str;
} /*test_vector_str_to_token*/

/*===========================================================================
 * FUNCTION    - test_vector_strcmp -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int test_vector_strcmp(const char *str1, const char *str2)
{
  int i = 0, j = 0;
  int len2 = strlen(str2);

  /* skip initial white spaces */
  while (str1[i] == ' ')
    i++;

  while((str1[i] != '\0') || (str1[i] != '\n')) {
    if (j >= len2)
      break;
    if (tolower(str1[i]) != tolower(str2[j]))
      return -1;
    i++;
    j++;
  }
  if (j == len2) {
    return 0;
  }
  return -1;
} /*test_vector_strcmp*/

/*===========================================================================
 * FUNCTION    - test_vector_update_param -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t test_vector_update_param(vfe_test_vector_t *mod,
  const char* attribute, const char* value)
{
  vfe_status_t status = VFE_SUCCESS;
  CDBG_ERROR("parsing attribute %s", attribute);
  char input_last_entry[] = "pdaf_pixel_coords";
  chromatix_videoHDR_type  *chromatix_VHDR = NULL;
  chromatix_parms_type     *chromatix_ptr = NULL;
  ASD_VFE_struct_type      *ASD_algo_data = NULL;

  if (attribute == NULL) {
    CDBG_ERROR("%s: attribute NULL, return-1 !\n", __func__);
    return -1;
  }

  /*define last entry to switch parsing mode*/
  if (!strcmp(input_last_entry, attribute)) {
    mod->parse_mode = VFE_TEST_VEC_PARSE_OUTPUT;
    CDBG_ERROR("%s:%d last attribute! change mode to parsing output!\n",
      __func__, __LINE__);
  }

  if (!strcmp(AWB_R_GAIN, attribute)) {
    int data = atoi(value);
    CDBG("%s: AWB_r_gain %d", __func__, data);
    mod->params.awb_gains.r_gain = Q_TO_FLOAT(7, data);
  } else if (!strcmp(AWB_G_GAIN, attribute)) {
    int data = atoi(value);
    CDBG("%s: AWB_g_gain %d", __func__, data);
    mod->params.awb_gains.g_gain = Q_TO_FLOAT(7, data);
  } else if (!strcmp(AWB_B_GAIN, attribute)) {
    int data = atoi(value);
    CDBG("%s: AWB_b_gain %d", __func__, data);
    mod->params.awb_gains.b_gain = Q_TO_FLOAT(7, data);
  } else if (!strcmp(AWB_DECISION, attribute)) {
    CDBG("%s: AWB_decision %s", __func__, value);
  } else if (!strcmp(AWB_CCT, attribute)) {
    int data = atoi(value);
    CDBG("%s: AWB_CCT %d", __func__, data);
    mod->params.color_temp = data;
  } else if (!strcmp(LED_ENABLE, attribute)) {
    int data = VFE_TEST_VEC_IS_TRUE(value);
    CDBG("%s: LED_ENABLE %s %d", __func__, value, data);
    mod->params.led_enable = data;
  } else if (!strcmp(STROBE_ENABLE, attribute)) {
    int data = VFE_TEST_VEC_IS_TRUE(value);
    CDBG("%s: STROBE_ENABLE %s %d", __func__, value, data);
    mod->params.strobe_enable = data;
  } else if (!strcmp(LED_SENSITIVITY_OFF, attribute)) {
    float data = atof(value);
    CDBG("%s: LED_SENSITIVITY_OFF %f", __func__, data);
    mod->params.sensitivity_led.off = data;
  } else if (!strcmp(LED_SENSITIVITY_HIGH, attribute)) {
    float data = atof(value);
    CDBG("%s: LED_SENSITIVITY_HIGH %f", __func__, data);
    mod->params.sensitivity_led.high = data;
  } else if (!strcmp(STROBE_SENSITIVITY_OFF, attribute)) {
    float data = atof(value);
    CDBG("%s: STROBE_SENSITIVITY_OFF %f", __func__, data);
    mod->params.sensitivity_strobe.off = data;
  } else if (!strcmp(STROBE_SENSITIVITY_HIGH, attribute)) {
    float data = atof(value);
    CDBG("%s: STROBE_SENSITIVITY_HIGH %f", __func__, data);
    mod->params.sensitivity_strobe.high = data;
  } else if (!strcmp(LED1_CURRENT, attribute)) {
    float data = atof(value);
    CDBG("%s: LED1_CURRENT %f", __func__, data);
    mod->params.led1_current = data;
  } else if (!strcmp(LED2_CURRENT, attribute)) {
    float data = atof(value);
    CDBG("%s: LED2_CURRENT %f", __func__, data);
    mod->params.led2_current = data;
  } else if (!strcmp(BAYER_PATTERN, attribute)) {
    float data = atoi(value);
    CDBG("%s: BAYER_PATTERN %d", __func__, data);
    mod->bayer_pattern = data;
  } else if (!strcmp(LUX_INDEX, attribute)) {
    int data = atoi(value);
    mod->params.lux_idx = data;
    CDBG("%s: LUX_INDEX %d %f", __func__, data, mod->params.lux_idx);
  } else if (!strcmp(TOTAL_GAIN, attribute)) {
    int data = atoi(value);
    /* convert to float */
    CDBG("%s: TOTAL_GAIN %d", __func__, data);
    mod->params.cur_real_gain = Q_TO_FLOAT(10, data);
  } else if (!strcmp(DIGITAL_GAIN, attribute)) {
    int data = atoi(value);
    /* convert to float */
    mod->params.digital_gain = Q_TO_FLOAT(10, data);
    CDBG("%s: DIGITAL_GAIN %d", __func__, data);
  } else if (!strcmp(CAMIF_WIDTH, attribute)) {
    int data = atoi(value);
    CDBG("%s: CAMIF_WIDTH %d", __func__, data);
    mod->camif_size.width = data;
  } else if (!strcmp(CAMIF_HEIGHT, attribute)) {
    int data = atoi(value);
    CDBG("%s: CAMIF_HEIGHT %d", __func__, data);
    mod->camif_size.height = data;
  } else if (!strcmp(OUTPUT_WIDTH, attribute)) {
    int data = atoi(value);
    CDBG("%s: OUTPUT_WIDTH %d", __func__, data);
    mod->output_size.width = data;
  } else if (!strcmp(OUTPUT_HEIGHT, attribute)) {
    int data = atoi(value);
    CDBG("%s: OUTPUT_HEIGHT %d", __func__, data);
    mod->output_size.height = data;
  } else if (!strcmp(SENSOR_SCALING, attribute)) {
    int data = atoi(value);
    CDBG("%s: SENSOR_SCALING %d", __func__, data);
    mod->sensor_scaling = data;
  } else if (!strcmp(X_OFFSET, attribute) ||
    !strcmp("subgrdh_offset", attribute)) {
    int data = atoi(value);
    CDBG("%s: X_OFFSET %d", __func__, data);
    mod->x_offset = data;
  } else if (!strcmp(Y_OFFSET, attribute) ||
    !strcmp("subgrdv_offset", attribute)) {
    int data = atoi(value);
    CDBG("%s: Y_OFFSET %d", __func__, data);
    mod->y_offset = data;
  } else if (!strcmp(FULL_WIDTH, attribute)) {
    int data = atoi(value);
    CDBG("%s: FULL_WIDTH %d", __func__, data);
    mod->full_size.width = data;
  } else if (!strcmp(FULL_HEIGHT, attribute)) {
    int data = atoi(value);
    CDBG("%s: FULL_HEIGHT %d", __func__, data);
    mod->full_size.height = data;
  } else if (!strcmp(EXP_RATIO, attribute)) {
    float data = atof(value);
    CDBG("%s: EXP_RATIO %f", __func__, data);
    mod->exp_ratio = data;
  } else if (!strcmp(EXP_TIME_RATIO, attribute)) {
    float data = atof(value);
    CDBG("%s: EXP_TIME_RATIO %f", __func__, data);
    mod->exp_time_ratio = data;
  } else if (!strcmp(ADRC_ENABLE, attribute)) {
    int data = VFE_TEST_VEC_IS_TRUE(value);
    CDBG("%s: ADRC_ENABLE %d\n", __func__, data);
    mod->adrc_enable = data;
  } else if (!strcmp(ADRC_COLOR_GAIN, attribute)) {
    float data = atof(value);
    CDBG("%s: ADRC_COLOR_GAIN %f", __func__, data);
    mod->adrc_color_gain = data;
  } else if (!strcmp(ADRC_TOTAL_GAIN, attribute)) {
    float data = atof(value);
    CDBG("%s: ADRC_TOTAL_GAIN %f\n", __func__, data);
    mod->adrc_total_gain = data;
  } else if (!strcmp(ADRC_GAMMA_RATIO, attribute)) {
    float data = atof(value);
    CDBG("%s: ADRC_GAMMA_RATIO %f", __func__, data);
    mod->adrc_gamma_ratio = data;
  } else if (!strcmp(ADRC_LTM_RATIO, attribute)) {
    float data = atof(value);
    CDBG("%s: ADRC_LTM_RATIO %f", __func__, data);
    mod->adrc_ltm_ratio = data;
  } else if (!strcmp(ADRC_LA_RATIO, attribute)) {
    float data = atof(value);
    CDBG("%s: ADRC_LA_RATIO %f", __func__, data);
    mod->adrc_la_ratio = data;
  } else if (!strcmp(ADRC_GTM_RATIO, attribute)) {
    float data = atof(value);
    CDBG("%s: ADRC_GTM_RATIO %f", __func__, data);
    mod->adrc_gtm_ratio = data;
  } else if (!strcmp(AEC_SETTLE, attribute)) {
    CDBG("%s: AEC_SETTLE %s", __func__, value);
  } else if (!strcmp(AEC_CONVERGENCE, attribute)) {
    CDBG("%s: AEC_CONVERGENCE %s", __func__, value);
  } else if (!strcmp(AEC_MAX, attribute)) {
    CDBG("%s: AEC_MAX %s", __func__, value);
  } else if (!strcmp(IHIST_STATS, attribute)) {
    uint32_t cnt = 0;
    char *saveptr = NULL;
    char *tok = strtok_r(strdup(value), ",", &saveptr);
    while (tok != NULL && cnt < 256) {
      mod->params.ihist_stats.histogram[cnt++] = atoi(value);
      tok = strtok_r(NULL, ",", &saveptr);
    }
    CDBG("%s: LA_bins %d bins found", __func__, cnt);
  } else if (!strcmp(BG_RGN_H_NUM, attribute)) {
    int data = atoi(value);
    mod->params.bg_stats.bg_region_h_num = data;
    CDBG_ERROR("%s BG_RGN_H_NUM %d", __func__, data);
  } else if (!strcmp(BG_RGN_V_NUM, attribute)) {
    int data = atoi(value);
    mod->params.bg_stats.bg_region_v_num = data;
    CDBG_ERROR("%s BG_RGN_V_NUM %d", __func__, data);
  } else if (!strcmp(BG_R_SUM    , attribute)) {
    uint32_t cnt = 0;
    char *saveptr = NULL;
    char *tok = strtok_r(strdup(value), ",", &saveptr);
    while (tok != NULL && cnt < MAX_BG_STATS_NUM) {
      mod->params.bg_stats.bg_r_sum[cnt++] = atoi(value);
      tok = strtok_r(NULL, ",", &saveptr);
    }
  } else if (!strcmp(BG_R_CNT    , attribute)) {
    uint32_t cnt = 0;
    char *saveptr = NULL;
    char *tok = strtok_r(strdup(value), ",", &saveptr);
    while (tok != NULL && cnt < MAX_BG_STATS_NUM) {
      mod->params.bg_stats.bg_r_num[cnt++] = atoi(value);
      tok = strtok_r(NULL, ",", &saveptr);
    }
  } else if (!strcmp(BG_GR_SUM   , attribute)) {
    uint32_t cnt = 0;
    char *saveptr = NULL;
    char *tok = strtok_r(strdup(value), ",", &saveptr);
    while (tok != NULL && cnt < MAX_BG_STATS_NUM) {
      mod->params.bg_stats.bg_gr_sum[cnt++] = atoi(value);
      tok = strtok_r(NULL, ",", &saveptr);
    }
  } else if (!strcmp(BG_GR_CNT   , attribute)) {
    uint32_t cnt = 0;
    char *saveptr = NULL;
    char *tok = strtok_r(strdup(value), ",", &saveptr);
    while (tok != NULL && cnt < MAX_BG_STATS_NUM) {
      mod->params.bg_stats.bg_gr_num[cnt++] = atoi(value);
      tok = strtok_r(NULL, ",", &saveptr);
    }
  } else if (!strcmp(BG_GB_SUM   , attribute)) {
    uint32_t cnt = 0;
    char *saveptr = NULL;
    char *tok = strtok_r(strdup(value), ",", &saveptr);
    while (tok != NULL && cnt < MAX_BG_STATS_NUM) {
      mod->params.bg_stats.bg_gb_sum[cnt++] = atoi(value);
      tok = strtok_r(NULL, ",", &saveptr);
    }
  } else if (!strcmp(BG_GB_CNT   , attribute)) {
    uint32_t cnt = 0;
    char *saveptr = NULL;
    char *tok = strtok_r(strdup(value), ",", &saveptr);
    while (tok != NULL && cnt < MAX_BG_STATS_NUM) {
      mod->params.bg_stats.bg_gb_num[cnt++] = atoi(value);
      tok = strtok_r(NULL, ",", &saveptr);
    }
  } else if (!strcmp(BG_B_SUM    , attribute)) {
    uint32_t cnt = 0;
    char *saveptr = NULL;
    char *tok = strtok_r(strdup(value), ",", &saveptr);
    while (tok != NULL && cnt < MAX_BG_STATS_NUM) {
      mod->params.bg_stats.bg_b_sum[cnt++] = atoi(value);
      tok = strtok_r(NULL, ",", &saveptr);
    }
  } else if (!strcmp(BG_B_CNT    , attribute)) {
    uint32_t cnt = 0;
    char *saveptr = NULL;
    char *tok = strtok_r(strdup(value), ",", &saveptr);
    while (tok != NULL && cnt < MAX_BG_STATS_NUM) {
      mod->params.bg_stats.bg_b_num[cnt++] = atoi(value);
      tok = strtok_r(NULL, ",", &saveptr);
    }
  } else if (!strcmp(BHIST_GR    , attribute)) {
    uint32_t cnt = 0;
    char *saveptr = NULL;
    char *tok = strtok_r(strdup(value), ",", &saveptr);
    while (tok != NULL && cnt < MAX_BHIST_STATS) {
      mod->params.raw_bhist_stats[cnt++] = atoi(tok); /* GR */
      tok = strtok_r(NULL, ",", &saveptr);
    }
  } else if (!strcmp(MODE, attribute)) {
    mod->snapshot_mode = strcmp(PREVIEW, value) ? 1 : 0;
    CDBG("%s: MODE %s %d", __func__, value, mod->snapshot_mode);
    /* change parse mode */
  }
  else if (!strcmp(HDR_MODE, attribute)) {
    if ((strcmp(NON_HDR_MODE, value)) == 0){
      mod->hdr_mode = VFE_TEST_NON_HDR;
    } else if ((strcmp(HDR,value)) == 0){
      mod->hdr_mode = VFE_TEST_HDR;
    } else if ((strcmp(SENSOR_HDR,value)) == 0){
      mod->hdr_mode = VFE_TEST_MODE_SENSOR_HDR;
    }
    CDBG_ERROR("bet_dbg HDR_OR NON_HDR_MODE  hdr_mode %d \n", mod->hdr_mode);
    chromatix_ptr = mod->chromatix_data;
    if (chromatix_ptr) {
      chromatix_VHDR =
        &chromatix_ptr->chromatix_post_processing.chromatix_video_HDR;
      CDBG("bet_dbg hdr_mode %d \n", mod->hdr_mode);
      if (chromatix_VHDR) {
        if (mod->hdr_mode == VFE_TEST_NON_HDR) {
          CDBG("bet_dbg Non HDR mode is active");
          chromatix_VHDR->videoHDR_reserve_data.hdr_recon_en = 0;
          chromatix_VHDR->videoHDR_reserve_data.hdr_mac_en = 0;
        } else {
          chromatix_VHDR->videoHDR_reserve_data.hdr_recon_en = 1;
          chromatix_VHDR->videoHDR_reserve_data.hdr_mac_en = 1;
        }
      }
    }
    CDBG("%s: HDR_MODE %s %d", __func__, value, mod->hdr_mode);
    /* change parse mode */
  }
  else if (!strcmp(PDAF_OFFSET_X, attribute)) {
    int data = atoi(value);
    mod->pdaf_xOffset = data;
    CDBG_ERROR("%s pdaf_xOffset %d", __func__, data);
  }
  else if (!strcmp(PDAF_OFFSET_Y, attribute)) {
    int data = atoi(value);
    mod->pdaf_yOffset = data;
    CDBG_ERROR("%s pdaf_yOffset %d", __func__, data);
  }
  else if (!strcmp(PDAF_PIXEL_COUNT, attribute)) {
    int data = atoi(value);
    mod->pdaf_pix_cnt = data;
    CDBG_ERROR("%s pdaf_pix_cnt %d", __func__, data);
  }
  else if (!strcmp(BSD, attribute)) {
    int data = VFE_TEST_VEC_IS_TRUE(value);
    CDBG("%s: backlit_scene_enable %d", __func__, data);
    mod->bsd_enable = data;
    if (!data) {
      chromatix_ptr = mod->chromatix_data;
      if (chromatix_ptr) {
        ASD_algo_data = &(chromatix_ptr->ASD_algo_data);
        if (ASD_algo_data) {
          ASD_algo_data->backlit_scene_detect.backlit_LTM.enable = 0;
        }
      }
    }
  }
  else if (!strcmp(CASE_NAME, attribute)) {
    CDBG("%s: CASE_NAME %s", __func__, value);
  } else if (!strcmp(CASE_NUMBER, attribute)) {
    CDBG("%s: CASE_NUMBER %s", __func__, value);
  } else {
    CDBG("%s: invalid token %s %s", __func__, attribute, value);
  }
  return status;
}/*test_vector_update_param*/

/*===========================================================================
 * FUNCTION    - test_vector_extract_input_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t test_vector_extract_input_params(vfe_test_vector_t *mod,
  char* line)
{
  const int MAX_TOKEN = 3;
  char *val[MAX_TOKEN];
  char *last;
  int index = 0;

  val[index] = strtok_r (line, "=", &last);
  while (val[index] != NULL) {
    if (index > 1) {
      CDBG("%s: line %s parse failed", __func__, val[index]);
      break;
    }
    ++index;
    val[index] = strtok_r (NULL, "=", &last);
  }

  val[1] = test_vector_str_to_token(val[1]);
  CDBG("%s: attr \"%s\" val \"%s\" ", __func__, val[0], val[1]);
  return test_vector_update_param(mod, val[0], val[1]);
}/*test_vector_extract_input_params*/

/*===========================================================================
 * FUNCTION    - test_vector_extract_input_module_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t test_vector_extract_input_module_params(vfe_test_vector_t *mod,
  char* line, uint32_t *module_type)
{
  vfe_status_t status = VFE_SUCCESS;
  const int MAX_TOKEN = 2;
  char *val[MAX_TOKEN];
  char *last;
  int index = 0, i;

  if (!mod || !line || !module_type) {
    CDBG_ERROR("%s: NULL pointer!\n",__func__);
    return -1;
  }

  if (0 == test_vector_strcmp(line, VFE_TEST_VEC_COLOR_CORRECTION)) {
    *module_type = ISP_MOD_COLOR_CORRECT;
    CDBG("%s: Color correction", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_COLOR_CONVERSION)) {
    *module_type = ISP_MOD_CHROMA_ENHANCE;
    CDBG("%s: Color conversion", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_MESH_ROLLOFF)) {
    *module_type = ISP_MOD_ROLLOFF;
    CDBG("%s: Mesh Rolloff", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_DEMOSAIC)) {
    *module_type = ISP_MOD_DEMOSAIC;
    CDBG("%s: Demosaic", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_CAC)) {
    *module_type = ISP_MOD_CAC;
    CDBG("%s: CLF", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_BPC)) {
    *module_type = ISP_MOD_BPC;
    CDBG("%s: BPC", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_BCC)) {
    *module_type = ISP_MOD_BCC;
    CDBG("%s: BCC", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_LINEARIZATION)) {
    *module_type = ISP_MOD_LINEARIZATION;
    CDBG("%s: Linearization", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_HDR)) {
    *module_type = ISP_MOD_HDR;
    CDBG("%s: Video HDR", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_SCE)) {
    *module_type = ISP_MOD_SCE;
    CDBG("%s: SCE", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_GAMMA)) {
    *module_type = ISP_MOD_GAMMA;
    CDBG("%s: Gamma", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_ASF7)) {
    *module_type = ISP_MOD_ASF;
    CDBG("%s: ASF", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_ASF9)) {
    *module_type = ISP_MOD_ASF;
    CDBG("%s: ASF", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_AWB)) {
    *module_type = ISP_MOD_WB;
    CDBG("%s: WB", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_ABF)) {
    *module_type = ISP_MOD_ABF;
    CDBG("%s: ABF", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_MCE)) {
    *module_type = ISP_MOD_MCE;
    CDBG("%s: MCE", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_CS)) {
    *module_type = ISP_MOD_CHROMA_SUPPRESS;
    CDBG("%s: Chroma SS", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_GIC)) {
    *module_type = ISP_MOD_GIC;
    CDBG("%s: Green Imbalance Correction", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_PEDESTAL)) {
    *module_type = ISP_MOD_PEDESTAL;
    CDBG("%s: Pedestal Black Level Correction", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_GTM)) {
    *module_type = ISP_MOD_GTM;
    CDBG("%s:Global Tone Mapping", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_LTM)) {
    *module_type = ISP_MOD_LTM;
    CDBG("%s:Local Tone Mapping", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_SNR)) {
    *module_type = ISP_MOD_SNR;
    CDBG("%s: SNR", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_BLS)) {
    *module_type = ISP_MOD_BLS;
    CDBG("%s: BLS", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_DEMUX)) {
    *module_type = ISP_MOD_DEMUX;
    CDBG_ERROR("%s: Demux", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_PDAF)) {
    *module_type = ISP_MOD_PDAF;
    CDBG_ERROR("%s: PDAF", __func__);
  } else if (0 == test_vector_strcmp(line, VFE_TEST_VEC_CST)) {
    *module_type = ISP_MOD_COLOR_XFORM_VIEWFINDER;
    CDBG("%s: CLF", __func__);
  } else {
    /* not a header */
    *module_type = ISP_MOD_MAX_NUM;
  }
  return status;
}/*test_vector_extract_input_module_params*/

/*===========================================================================
 * FUNCTION    - test_vector_extract_reg_values -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t test_vector_extract_reg_values(vfe_test_vector_t *mod,
  char** p_val, int count)
{
  vfe_status_t status = VFE_SUCCESS;
  if (count < 3)
    return status;
  uint32_t val[3];
  CDBG("%s: %s %s %s", __func__, p_val[0], p_val[1], p_val[2]);
  sscanf(p_val[0],"%x", &val[0]);
  sscanf(p_val[1],"%x", &val[1]);
  sscanf(p_val[2],"%x", &val[2]);
  CDBG("%s: 0x%x 0x%x 0x%x", __func__, val[0], val[1], val[2]);
  if (val[0] > mod->mod_input.reg_size*sizeof(uint32_t)) {
    CDBG_ERROR("%s: invalid address", __func__);
    return VFE_ERROR_GENERAL;
  }
  mod->mod_input.reg_dump[val[0]/4] = val[1];
  mod->mod_input.reg_mask[val[0]/4] = val[2];
  return status;
}/*test_vector_extract_reg_values*/

/*===========================================================================
 * FUNCTION    - test_vector_extract_table_int -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t test_vector_extract_table_int(vfe_test_vector_t *mod,
  char** p_val, int count, vfe_test_table_t *table, char *mod_name)
{
  int i;
  vfe_status_t status = VFE_SUCCESS;
  if (count < 3)
    return status;
  uint32_t j = 0;

  if (!mod || !table || !mod_name) {
    CDBG_ERROR("%s: NULL pointer %p %p %p!\n",__func__, mod, table, mod_name);
    return -1;
  }

  for (i = 0; i < count; i++) {
    if (!strcmp("value", p_val[i])) {
      i++;
      break;
    }
    if (!strcmp("mask", p_val[i])) {
      if (i+1 >= count)
        return VFE_ERROR_GENERAL;
      i++;
      sscanf(p_val[i],"%x", &(table->mask));
      CDBG("%s: %s mask 0x%x", __func__, mod_name, table->mask);
    }
  }
  CDBG("%s: %s i %d", __func__, mod_name, i);
  for(; (i < count) && (j < table->size); i++) {
    sscanf(p_val[i],"%x", &table->table[j]);
    CDBG("%s: %s[%d] 0x%x %s", __func__, mod_name, j, table->table[j], p_val[i]);
    j++;
  }
  return status;
} /*test_vector_extract_table_int */

/*===========================================================================
 * FUNCTION    - test_vector_extract_table64_int -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t test_vector_extract_table64_int(vfe_test_vector_t *mod,
  char** p_val, int count, vfe_test_table_64_t *table, char *mod_name)
{
  int i;
  vfe_status_t status = VFE_SUCCESS;

  if (!mod || !table || !mod_name) {
    CDBG_ERROR("%s: NULL pointer %p %p %p!\n",__func__, mod, table, mod_name);
    return -1;
  }

  if (count < 3)
    return status;
  uint32_t j = 0;
  for (i = 0; i < count; i++) {
    if (!strcmp("value", p_val[i])) {
      i++;
      break;
    }
    if (!strcmp("mask", p_val[i])) {
      if (i+1 >= count)
        return VFE_ERROR_GENERAL;
      i++;
      sscanf(p_val[i],"%llx", &(table->mask));
      CDBG("%s: %s mask 0x%016llx", __func__, mod_name,
        (long long unsigned int)table->mask);
    }
  }
  CDBG("%s: %s i %d", __func__, mod_name, i);
  for(; (i < count) && (j < table->size); i++) {
    uint64_t value;
    sscanf(p_val[i],"%llx", &value);
    table->table[j++] = value;
    CDBG("%s: %s[%d] 0x%016llx %s", __func__, mod_name, j,
      (long long unsigned int)table->table[j], p_val[i]);
  }
  return status;
} /*test_vector_extract_table64_int */

/*===========================================================================
 * FUNCTION    - test_vector_parse_module_data -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t test_vector_extract_module_data(vfe_test_vector_t *mod,
  char** p_val, int count)
{
  vfe_status_t status = VFE_SUCCESS;
  isp_hw_module_id_t module_type = mod->module_type;
  int i = 0;
  mod->modules_to_test |= ((uint64_t)1 << mod->module_type);
  switch(module_type) {
    case ISP_MOD_COLOR_XFORM_ENCODER:
    case ISP_MOD_DEMUX:
    case ISP_MOD_CHROMA_SUPPRESS:
    case ISP_MOD_MCE:
    case ISP_MOD_SCE:
    case ISP_MOD_CHROMA_ENHANCE:
    case ISP_MOD_BPC:
    case ISP_MOD_CAC:
    case ISP_MOD_SNR:
    case ISP_MOD_BLS:
    case ISP_MOD_DEMOSAIC:
    case ISP_MOD_HDR:
    case ISP_MOD_COLOR_CORRECT:{
      CDBG("Extracting Allother reg values");
      status = test_vector_extract_reg_values(mod, p_val, count);
      break;
    }

    case ISP_MOD_LINEARIZATION: {
      if (!test_vector_strcmp("table", test_vector_str_to_lower(p_val[0]))) {
        status = test_vector_extract_table64_int(mod, p_val, count,
          &mod->mod_input.linearization, VFE_TEST_VEC_LINEARIZATION);
      } else {
        CDBG("Extracting linearisation reg values");
        status = test_vector_extract_reg_values(mod, p_val, count);
      }
      break;
    }
    case ISP_MOD_ABF: {
      if (!test_vector_strcmp("table", test_vector_str_to_lower(p_val[0]))) {
          if (!test_vector_strcmp("noise_std2_lut_level0", p_val[1])){
          status = test_vector_extract_table_int(
            mod, p_val, count, &mod->mod_input.abf_std_l0, "ABF noise std L0");
      }
      } else {
        CDBG("Extracting abf reg values");
        status = test_vector_extract_reg_values(mod, p_val, count);
      }
      break;
    }
    case ISP_MOD_GIC: {
      if (!test_vector_strcmp("table", test_vector_str_to_lower(p_val[0]))) {
        if (!test_vector_strcmp("noise_std_lut_level0", p_val[1])) {
          status = test_vector_extract_table_int(
            mod, p_val, count, &mod->mod_input.gic_std2_l0, "GIC noise std2 L0");
        }
      } else {
        CDBG("Extracting gic reg values");
        status = test_vector_extract_reg_values(mod, p_val, count);
      }
      break;
    }
    case ISP_MOD_PEDESTAL: {
      if (!test_vector_strcmp("table", test_vector_str_to_lower(p_val[0]))) {
        if (!test_vector_strcmp("DMIRAM_PEDESTAL_T1_L_GR_R_LUT", p_val[1])) {
          status = test_vector_extract_table_int(
            mod, p_val, count, &mod->mod_input.pedestal_t1_gr_r, "pedestal mesh table T1 GR_R");
        } else if (!test_vector_strcmp("DMIRAM_PEDESTAL_T1_L_GB_B_LUT", p_val[1])) {
          status = test_vector_extract_table_int(
            mod, p_val, count, &mod->mod_input.pedestal_t1_gb_b, "pedestal mesh table T1 GB_B");
        }
      } else {
        CDBG("Extracting pedestal reg values");
        status = test_vector_extract_reg_values(mod, p_val, count);
      }
      break;
    }
    case ISP_MOD_GTM: {
      if (!test_vector_strcmp("table", test_vector_str_to_lower(p_val[0]))) {
        status = test_vector_extract_table64_int(
          mod, p_val, count, &mod->mod_input.gtm, "y ratio");
      } else {
        CDBG("Extracting gtm reg values");
        status = test_vector_extract_reg_values(mod, p_val, count);
      }
      break;
    }
    case ISP_MOD_LTM: {
      CDBG("LTM test vector data extraction");
      if (!test_vector_strcmp("table", test_vector_str_to_lower(p_val[0]))) {
             CDBG("bet_dbg Extracting LTM Table ");
        if (!test_vector_strcmp("weight", p_val[1])) {
          CDBG("bet_dbg weight");
          status = test_vector_extract_table_int(
            mod, p_val, count, &mod->mod_input.ltm_weight, "weight");
        } else if (!test_vector_strcmp("mask_rect_curve", p_val[1])) {
          CDBG("mask_rect_curve");
          status = test_vector_extract_table_int(
            mod, p_val, count, &mod->mod_input.ltm_mask_curve, "mask_rect_curve");
        } else if (!test_vector_strcmp("master_curve", p_val[1])) {
          CDBG("master_curve");
          status = test_vector_extract_table_int(
            mod, p_val, count, &mod->mod_input.ltm_master_curve, "master_curve");
        }else if (!test_vector_strcmp("master_scale", p_val[1])) {
          CDBG("master_scale");
          status = test_vector_extract_table_int(
            mod, p_val, count, &mod->mod_input.ltm_master_scale, "master_scale");
        }else if (!test_vector_strcmp("shift_curve", p_val[1])) {
          CDBG("shift_curve");
          status = test_vector_extract_table_int(
            mod, p_val, count, &mod->mod_input.ltm_shift_curve, "shift_curve");
        }else if (!test_vector_strcmp("shift_scale", p_val[1])) {
          CDBG("shift_scale");
          status = test_vector_extract_table_int(
            mod, p_val, count, &mod->mod_input.ltm_shift_scale, "shift_scale");
        }else if (!test_vector_strcmp("saturation_curve", p_val[1])) {
          CDBG("saturation_curve");
          status = test_vector_extract_table_int(
            mod, p_val, count, &mod->mod_input.ltm_sat_curve, "saturation_curve");
        }
      } else {
        CDBG("Extracting LTM reg values");
        status = test_vector_extract_reg_values(mod, p_val, count);
      }
      break;
    }
    case ISP_MOD_GAMMA: {
      vfe_test_table_t *table = &mod->mod_input.gamma_b;
      if (!test_vector_strcmp("Table", test_vector_str_to_lower(p_val[0]))) {
        if (!test_vector_strcmp("r_gamma_lut", p_val[1]))
          table = &mod->mod_input.gamma_r;
        else if (!test_vector_strcmp("g_gamma_lut", p_val[1]))
          table = &mod->mod_input.gamma_g;
      }
      CDBG("Extracting gamma reg values");
      status = test_vector_extract_table_int(mod, p_val, count,
        table, VFE_TEST_VEC_GAMMA);
      break;
    }
    case ISP_MOD_ROLLOFF: {
      if (!test_vector_strcmp("table", test_vector_str_to_lower(p_val[0]))) {
        if (!test_vector_strcmp("Gr_&_R_Mesh_Table", p_val[1])) {
          CDBG_ERROR("extract gr_r");
          status = test_vector_extract_table_int(
            mod, p_val, count, &mod->mod_input.mesh_rolloff.gr_r, "mesh_rolloff(gr_r)");
        } else if (!test_vector_strcmp("Gb_&_B_Mesh_Table", p_val[1])) {
          CDBG_ERROR("extract gb_b");
          status = test_vector_extract_table_int(
            mod, p_val, count, &mod->mod_input.mesh_rolloff.gb_b, "mesh_rolloff(gb_b)");
        }
      } else {
        CDBG("Extracting rolloff reg values");
        status = test_vector_extract_reg_values(mod, p_val, count);
      }
      break;
    }
    case ISP_MOD_PDAF: {
/*      if (!test_vector_strcmp("table", test_vector_str_to_lower(p_val[0]))) {
        status = test_vector_extract_table_int(mod, p_val, count,
          &mod->mod_input.pdaf_pd_mask, VFE_TEST_VEC_PDAF);
      } else */{
        CDBG("Extracting pdpc reg values");
        status = test_vector_extract_reg_values(mod, p_val, count);
      }
      break;
    }
    default:
      break;
  }
  return status;
}

/*===========================================================================
 * FUNCTION    - test_vector_parse_module_data -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t test_vector_parse_module_data(vfe_test_vector_t *mod, char* line)
{
  vfe_status_t status = VFE_SUCCESS;
  const int max_token = strlen(line);
  char **p_val = NULL;
  char *last;
  int count = 0;

  if (max_token <= 1)
    return VFE_SUCCESS;

  if (mod->module_type == ISP_MOD_MAX_NUM) {
    CDBG("%s: Invalid module type %d", __func__, mod->module_type);
    return VFE_SUCCESS;
  }
  p_val = (char **)malloc(max_token * sizeof(char *));
  if (!p_val) {
    CDBG_ERROR("%s: malloc failed: No Memory\n", __func__);
    return VFE_ERROR_NO_MEMORY;
  }

  p_val[count] = strtok_r (line, " =,[]\"", &last);
  if (!strcmp(VFE_TEST_VEC_ADDRESS, p_val[0])) {
    /* skip this line */
    goto end;
  }
  while (p_val[count] != NULL) {
    ++count;
    p_val[count] = strtok_r (NULL, " =,[]\"", &last);
  }

  test_vector_extract_module_data(mod, p_val, count);

end:
  if (p_val)
    free(p_val);
  return status;
}/*test_vector_parse_module_data*/

/*===========================================================================
 * FUNCTION    - test_vector_parse_input -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t test_vector_parse_input(vfe_test_vector_t *mod)
{
  vfe_status_t status = VFE_SUCCESS;
  char *token, *last;
  int bytes_read;
  /*calculate size*/
  fseek(mod->fp, 0L, SEEK_END);
  mod->input_size = ftell(mod->fp);
  fseek(mod->fp, 0L, SEEK_SET);
  CDBG("%s: input_size %d", __func__, mod->input_size);
  mod->input_data = (char *)malloc(mod->input_size * sizeof(char));
  if (mod->input_data == NULL) {
    CDBG_ERROR("%s: cannot allocate input", __func__);
    return VFE_ERROR_NO_MEMORY;
  }
  bytes_read = fread (mod->input_data, 1, mod->input_size, mod->fp);
  CDBG("%s: bytes_read %d input_size %d", __func__,
     bytes_read, mod->input_size);
  if (bytes_read < mod->input_size) {
    CDBG_ERROR("%s: cannot get the data from file %d %d", __func__,
      bytes_read, mod->input_size);
    return VFE_ERROR_GENERAL;
  }
  fclose(mod->fp); /* close file handle */

  mod->parse_mode = VFE_TEST_VEC_PARSE_INPUT;
  token = strtok_r (mod->input_data, "\n", &last);

  while (token != NULL) {
    int len = strlen(token);
    if (len > 0 && token[len-1] == '\r')
      token[len-1] = '\0';
    CDBG ("%s: token %s\n", __func__, token);
    if((len <=2) && (token[0] == '/') && (token[1] == '/')) {
      CDBG("%s: comment %s", __func__, token);
    } else if (mod->parse_mode == VFE_TEST_VEC_PARSE_INPUT) {
      status = test_vector_extract_input_params(mod, token);
      if (status != VFE_SUCCESS) {
        CDBG_ERROR("%s: parsing input error, status = %d\n", __func__, status);
        return status;
      }
    } else { /* parse output */
      uint32_t module_type;
      char *mod_ptr = NULL;
      status = test_vector_extract_input_module_params(mod, token,
        &module_type);
      //CDBG_ERROR("%s: module_type %d", __func__, module_type);
      if (status != VFE_SUCCESS)
        return status;
      if (module_type != ISP_MOD_MAX_NUM) {
        CDBG("%s: module_type %d", __func__, module_type);
        mod->module_type = module_type;
      } else {
        status = test_vector_parse_module_data(mod, token);

        if (status != VFE_SUCCESS)
          return status;
      }
    }
    token = strtok_r (NULL, "\n", &last);
  }
  return status;
}/*test_vector_parse_input*/

/*===========================================================================
 * FUNCTION    - vfe_test_module_data_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_test_module_data_init(vfe_test_module_data_t *mod_data)
{
  vfe_status_t status = VFE_SUCCESS;

  mod_data->reg_size = ISP48_NUM_REG_DUMP * sizeof(uint32_t);
  mod_data->reg_dump = (uint32_t *)malloc(mod_data->reg_size);
  if (!mod_data->reg_dump) {
    CDBG_ERROR("%s:%d] no memory", __func__, __LINE__);
    return VFE_ERROR_NO_MEMORY;
  }

  mod_data->reg_mask = (uint32_t *)malloc(mod_data->reg_size *
    sizeof(uint32_t));
  if (!mod_data->reg_mask) {
    CDBG_ERROR("%s:%d] no memory", __func__, __LINE__);
    return VFE_ERROR_NO_MEMORY;
  }

  test_vector_malloc_table(
     &mod_data->mesh_rolloff.gr_r, ISP_MESH_ROLLOFF_TABLE_SIZE); //221
  test_vector_malloc_table(
    &mod_data->mesh_rolloff.gb_b, ISP_MESH_ROLLOFF_TABLE_SIZE);
  test_vector_malloc_table_64(
     &mod_data->linearization, ISP32_LINEARIZATON_TABLE_LENGTH); //36
  test_vector_malloc_table(
    &mod_data->gamma_r, ISP_GAMMA_NUM_ENTRIES); //256
  test_vector_malloc_table(
    &mod_data->gamma_g, ISP_GAMMA_NUM_ENTRIES);
  test_vector_malloc_table(
    &mod_data->gamma_b, ISP_GAMMA_NUM_ENTRIES);
  test_vector_malloc_table(
     &mod_data->abf_std_l0, ((sizeof(noise_std2_lut_t)/ sizeof(uint32_t)) - 1));  //32
  test_vector_malloc_table(
     &mod_data->gic_std2_l0, GIC_NOISE_STD2_LUT_SIZE);
  test_vector_malloc_table(
     &mod_data->pedestal_t1_gr_r, PEDESTAL_LUT_SIZE);
  test_vector_malloc_table(
     &mod_data->pedestal_t1_gb_b, PEDESTAL_LUT_SIZE);
  test_vector_malloc_table_64(
     &mod_data->gtm, (GTM_LUT_NUM_BIN-1));

  test_vector_malloc_table(
    &mod_data->ltm_weight, LTM_HW_W_LUT_SIZE);
  test_vector_malloc_table(
    &mod_data->ltm_mask_curve, LTM_HW_LUT_SIZE);
  test_vector_malloc_table(
       &mod_data->ltm_master_curve, LTM_HW_LUT_SIZE);
  test_vector_malloc_table(
       &mod_data->ltm_master_scale, LTM_HW_LUT_SIZE);
  test_vector_malloc_table(
       &mod_data->ltm_shift_curve, LTM_HW_LUT_SIZE);
  test_vector_malloc_table(
       &mod_data->ltm_shift_scale, LTM_HW_LUT_SIZE);
  test_vector_malloc_table(
       &mod_data->ltm_sat_curve, LTM_HW_LUT_SIZE);
  test_vector_malloc_table(
       &mod_data->pdaf_pd_mask, PDAF_LUT_COUNT);
  return status;
}

/*===========================================================================
 * FUNCTION    - vfe_test_module_data_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
void vfe_test_module_data_deinit(vfe_test_module_data_t *mod_data)
{
  //_FREE(mod_data->reg_dump);
  _FREE(mod_data->gamma_r.table);
  _FREE(mod_data->gamma_g.table);
  _FREE(mod_data->gamma_b.table);
  _FREE(mod_data->linearization.table);
  _FREE(mod_data->mesh_rolloff.gr_r.table);
  _FREE(mod_data->mesh_rolloff.gb_b.table);
  _FREE(mod_data->abf_std_l0.table);
  _FREE(mod_data->gic_sig2_l0.table);
  _FREE(mod_data->gic_std2_l0.table);
  _FREE(mod_data->pedestal_t1_gr_r.table);
  _FREE(mod_data->pedestal_t1_gb_b.table);
  _FREE(mod_data->gtm.table);
  _FREE(mod_data->la.table);
  _FREE(mod_data->ltm_mask_curve.table);
  _FREE(mod_data->ltm_master_curve.table);
  _FREE(mod_data->ltm_master_scale.table);
  _FREE(mod_data->ltm_shift_curve.table);
  _FREE(mod_data->ltm_shift_scale.table);
  _FREE(mod_data->ltm_weight.table);
  _FREE(mod_data->ltm_sat_curve.table);
  _FREE(mod_data->pdaf_pd_mask.table);
}

/*===========================================================================
 * FUNCTION    - vfe_test_vector_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_test_vector_init(vfe_test_vector_t *mod, char* datafile)
{
  vfe_status_t status = VFE_SUCCESS;
  int i = 0;
  mod->input_data = NULL;

  mod->fp = fopen(datafile, "r");
  if (NULL == mod->fp) {
    CDBG_HIGH("%s: fp NULL", __func__);
    return VFE_ERROR_GENERAL;
  }

  mod->module_type = ISP_MOD_MAX_NUM;

  vfe_test_module_data_init(&mod->mod_input);
  vfe_test_module_data_init(&mod->mod_output);
  //0 means do something. set to -1 to tell software not to do anything with the parameter.
  mod->adrc_gtm_ratio = -1.0f;
  status = test_vector_parse_input(mod);
  return status;
}/*vfe_test_vector_init*/

/*===========================================================================
 * FUNCTION    - vfe_test_vector_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
vfe_status_t vfe_test_vector_deinit(vfe_test_vector_t *mod)
{
  int i = 0;

  _FREE(mod->input_data);
  vfe_test_module_data_deinit(&mod->mod_input);
  vfe_test_module_data_deinit(&mod->mod_output);
  return VFE_SUCCESS;
}/*vfe_test_vector_deinit*/

/*===========================================================================
 * FUNCTION    - vfe_test_vector_get_output -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY:
 *==========================================================================*/
static vfe_status_t vfe_test_vector_get_output(vfe_test_vector_t *mod)
{
  vfe_status_t status = VFE_SUCCESS;
  uint32_t dmi_bank_sel = 0;

  int rc;
  vfe_test_module_output_t *tv_params = &(mod->mod_output);
  isp2_hw_read_info read_info;

  rc = vfe_test_vector_cgc_on();
  if (rc) {
    CDBG_ERROR("%s failed to turn on CGC: rc = %d\n", __func__, rc);
    return VFE_ERROR_GENERAL;
  }

  if (!tv_params->reg_dump) {
    CDBG_ERROR("%s %d reg_dump is null",__func__,__LINE__);
    return status;
  }

  memset(tv_params->reg_dump, 0, sizeof(tv_params->reg_size));
  rc = isp2_hw_read_reg_dump(
    tv_params->reg_dump, VFE_READ, tv_params->reg_size);
  if (rc) {
    CDBG_ERROR("%s failed to get reg dump: rc = %d\n", __func__, rc);
    return VFE_ERROR_GENERAL;
  }

  read_info.bank_idx = 0;

  /* fetch table changed based on how bank order sit in DMI_CFG
     regular: bank0, bank1
     rolloff: bank0*8, bank1*8
     flexible: directly provide bank */

#define FETCH_TABLE(tbl, bank, bank_sel_reg, bank_sel_bit, type, sz) \
  read_info.read_type = type; \
  read_info.read_bank = bank + ((tv_params->reg_dump[bank_sel_reg/4] >> bank_sel_bit) & 1); \
  read_info.read_length = sz * tbl.size; \
  rc = isp2_hw_read_dmi_dump(tbl.table, &read_info, ISP_DMI_CFG_OFF, ISP_DMI_ADDR); \
  if (rc) { \
    CDBG_ERROR("%s failed to get reg dump: rc = %d\n", __func__, rc); \
    return VFE_ERROR_GENERAL; \
  }

#define FETCH_TABLE_ROLLOFF(tbl, bank, bank_sel_reg, bank_sel_bit, type, sz) \
  read_info.read_type = type; \
  if(((tv_params->reg_dump[bank_sel_reg/4] >> bank_sel_bit) & 1) != 0) \
    read_info.read_bank = bank + 4; \
  else \
    read_info.read_bank = bank;\
  read_info.read_length = sz * tbl.size; \
  rc = isp2_hw_read_dmi_dump(tbl.table, &read_info, ISP_DMI_CFG_OFF, ISP_DMI_ADDR); \
  if (rc) { \
    CDBG_ERROR("%s failed to get reg dump: rc = %d\n", __func__, rc); \
    return VFE_ERROR_GENERAL; \
  }

#define FETCH_TABLE_DIRECT(tbl, bank, type, sz) \
  read_info.read_type = type; \
  read_info.read_bank = bank; \
  read_info.read_length = sz * tbl.size; \
  rc = isp2_hw_read_dmi_dump(tbl.table, &read_info, ISP_DMI_CFG_OFF, ISP_DMI_ADDR); \
  if (rc) { \
    CDBG_ERROR("%s failed to get reg dump: rc = %d\n", __func__, rc); \
    return VFE_ERROR_GENERAL; \
  }

  CDBG_ERROR("%s bet_dbg ISP_DMI_CFG_OFF %x, ISP_DMI_ADDR %x",__func__, ISP_DMI_CFG_OFF, ISP_DMI_ADDR);
  FETCH_TABLE_ROLLOFF(tv_params->mesh_rolloff.gr_r,
    ROLLOFF_RAM_L_GR_R_BANK0, ISP_MESH_ROLLOFF_CFG_OFF, 16, VFE_READ_DMI_32BIT, sizeof(uint32_t));
  FETCH_TABLE_ROLLOFF(tv_params->mesh_rolloff.gb_b,
    ROLLOFF_RAM_L_GB_B_BANK0, ISP_MESH_ROLLOFF_CFG_OFF, 16, VFE_READ_DMI_32BIT, sizeof(uint32_t));
  FETCH_TABLE(tv_params->gamma_r,
    RGBLUT_RAM_CH2_BANK0, ISP_RGB_LUT_OFF, 2, VFE_READ_DMI_32BIT, sizeof(uint32_t));
  FETCH_TABLE(tv_params->gamma_g,
    RGBLUT_RAM_CH0_BANK0, ISP_RGB_LUT_OFF, 0, VFE_READ_DMI_32BIT, sizeof(uint32_t));
  FETCH_TABLE(tv_params->gamma_b,
    RGBLUT_RAM_CH1_BANK0, ISP_RGB_LUT_OFF, 1, VFE_READ_DMI_32BIT, sizeof(uint32_t));
  FETCH_TABLE(tv_params->linearization,
    BLACK_LUT_RAM_BANK0, ISP_LINEARIZATION_OFF, 0, VFE_READ_DMI_64BIT, sizeof(uint64_t));

  FETCH_TABLE(tv_params->abf_std_l0,
    ABF_STD2_L0_BANK0, ISP_ABF34_OFF, 1, VFE_READ_DMI_32BIT, sizeof(uint32_t));

  /*GIC*/
  FETCH_TABLE(tv_params->gic_std2_l0,
    GIC_NOISE_STD2_LUT_BANK0, ISP_GIC_OFF, 0, VFE_READ_DMI_32BIT, sizeof(uint32_t));

  /*GTM*/
  FETCH_TABLE(tv_params->gtm,
    GTM_LUT_RAM_BANK0, ISP_GTM_OFF, 0, VFE_READ_DMI_64BIT, sizeof(uint64_t));

  /*PDAF*/
  FETCH_TABLE(tv_params->pdaf_pd_mask,
    PDAF_LUT_BANK, PDAF_OFFSET, 0, VFE_READ_DMI_32BIT, sizeof(uint32_t));

  FETCH_TABLE(tv_params->ltm_mask_curve,
    LTM_MASK_LUT_BANK0, ISP_LTM_CFG_OFF, 8, VFE_READ_DMI_32BIT, sizeof(uint32_t));
  FETCH_TABLE(tv_params->ltm_master_curve,
    LTM_MC_LUT_BANK0, ISP_LTM_CFG_OFF, 8, VFE_READ_DMI_32BIT, sizeof(uint32_t));
  FETCH_TABLE(tv_params->ltm_master_scale,
    LTM_MS_LUT_BANK0, ISP_LTM_CFG_OFF, 8, VFE_READ_DMI_32BIT, sizeof(uint32_t));
  FETCH_TABLE(tv_params->ltm_shift_curve,
    LTM_SC_LUT_BANK0, ISP_LTM_CFG_OFF, 8, VFE_READ_DMI_32BIT, sizeof(uint32_t));
  FETCH_TABLE(tv_params->ltm_shift_scale,
    LTM_SS_LUT_BANK0, ISP_LTM_CFG_OFF, 8, VFE_READ_DMI_32BIT, sizeof(uint32_t));
  FETCH_TABLE(tv_params->ltm_weight,
    LTM_WEIGHT_LUT_BANK0, ISP_LTM_CFG_OFF, 8, VFE_READ_DMI_32BIT, sizeof(uint32_t));
  FETCH_TABLE(tv_params->ltm_sat_curve,
    LTM_SAT_LUT_BANK0, ISP_LTM_CFG_OFF, 8, VFE_READ_DMI_32BIT, sizeof(uint32_t));

  /*PEDESTAL, FETCH TBL direct, input exact BANK0 or BANK1*/
  /*Tbl0 GR_R*/
  dmi_bank_sel = PEDESTAL_F0_RAM_L_GR_R_BANK0 +
    4 * ((tv_params->reg_dump[ISP_PEDESTAL_OFF/4] >> 8) & 1);
  FETCH_TABLE_DIRECT(tv_params->pedestal_t1_gr_r, dmi_bank_sel,
    VFE_READ_DMI_32BIT, sizeof(uint32_t));

  /*Tbl0 GB_B*/
  dmi_bank_sel = PEDESTAL_F0_RAM_L_GB_B_BANK0 +
    4 * ((tv_params->reg_dump[ISP_PEDESTAL_OFF/4] >> 8) & 1);
  FETCH_TABLE_DIRECT(tv_params->pedestal_t1_gb_b, dmi_bank_sel,
    VFE_READ_DMI_32BIT, sizeof(uint32_t));

  return status;
}/*vfe_test_vector_get_output*/
/*===========================================================================
 * FUNCTION    - vfe_test_vector_execute -
 *
 * DESCRIPTION:
 *
 * DEPENDENCY:
 *==========================================================================*/

vfe_status_t vfe_test_vector_execute(vfe_test_vector_t *mod,
  testcase_report_t* testcase_report, char *mod_name[])
{
  vfe_status_t status;
  int show_mismatch_temp = show_mismatch;
  int show_match_temp = show_match;

  if (testcase_report == NULL) {
    CDBG_ERROR("%s: failed: NULL pointer!! test_report %p\n", __func__,
      testcase_report);
    return VFE_ERROR_INVALID_OPERATION;
  }

  status = vfe_test_vector_get_output(mod);
  if (VFE_SUCCESS != status) {
    CDBG_ERROR("%s failed to get output: rc = %d\n", __func__, status);
    return VFE_ERROR_GENERAL;
  }
  memset(testcase_report ,0,sizeof(testcase_report_t));

  int i = 0;
  for (i = 0; i < ISP_MOD_MAX_NUM; i++) {
    if (bet_test_list.vfe_tv_validate_func[i] != NULL) {
      show_mismatch = show_mismatch_temp | bet_test_list.show_mismatch[i];
      show_match = show_match_temp | bet_test_list.show_match[i];
      mod_name[i] = bet_test_list.vfe_module_name[i];
      if (mod->modules_to_test & ((uint64_t)1 << i)) {
        has_mismatch = 0;
        bet_test_list.vfe_tv_validate_func[i](&mod->mod_input, &mod->mod_output);
        if (has_mismatch) {
          printf("\tX : mismatch    in ==> %s (X) \n", bet_test_list.vfe_module_name[i]);
        } else {
          printf("\tO : no mismatch in ==> %s (O)\n", bet_test_list.vfe_module_name[i]);
        }
      } else {
        printf("\t %s  NOT in test list\n", bet_test_list.vfe_module_name[i]);
      }
      if (has_mismatch) {
        testcase_report->mismatch[i] = TRUE;
      }
    }
  }

  /*reset back to orginal show mismatch setting*/
  show_mismatch = show_mismatch_temp;
  show_match = show_match_temp;
  return VFE_SUCCESS;
}
