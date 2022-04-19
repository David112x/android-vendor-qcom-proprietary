/* abf44.c
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <unistd.h>
#include <math.h>

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_ABF, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_ABF, fmt, ##args)

/* isp headers */
#include "isp_defs.h"
#include "isp_sub_module_log.h"
#include "isp_sub_module_util.h"
#include "isp_common.h"
#include "isp_pipeline_reg.h"
#include "abf44.h"

#define CLAMP_ABF(x1,v1) ((x1 > v1) ? v1 : x1 )

//#define DUMP_ABF
/* sig2 luts pack delta/2 instead of delta */
#define FILL_SIG2_LUT(dst, src, len) \
  { \
    int32_t base; \
    int32_t delta; \
    for (i = 0; i < (len); i++) { \
      base = (src)[i]; \
      base = CLAMP_ABF(base,65535);\
      delta = (src)[i+1] - base; \
      delta = Clamp(delta,-32768,32767); \
      delta = roundf(delta/2.0f); \
      (dst)[i] = (delta << 16) | base; \
    } \
  }

#define FILL_LUT(dst, src, len) \
  { \
    int32_t base; \
    int32_t delta; \
    int32_t next;\
    for (i = 0; i < (len-1); i++) { \
      base = CLAMP_ABF(((src)[i] ),65535); \
      next = CLAMP_ABF(((src)[i+1]),65535);\
      delta = Clamp((next - base),-32768,32767);\
      (dst)[i] = ((delta ) << 16) |(base ); \
    } \
    base = ((src)[i] ); \
    base = CLAMP_ABF(base, 65535);\
    next = (src)[i+1] ;\
    delta = Clamp((next - base),-32768,32767);\
    (dst)[i] = ((delta ) << 16) |(base ); \
  }

#define PRINT_LUT(dst, len) \
     {\
    for (i = 0; i < (len); i++) { \
      ISP_HIGH("LUT index:[%d]== %x",i,(dst)[i]);\
    } \
  }

#define NORMAL_LIGHT_INDEX 4

static void abf44_debug(ISP_ABF3_CmdType *cmd)
{
    ISP_HIGH("   iirLevel0En     %x", cmd->cfg.iirLevel0En);
    ISP_HIGH("   iirLevel1En     %x", cmd->cfg.iirLevel1En);
    ISP_HIGH("   lutBankSel      %x", cmd->cfg.lutBankSel);
    ISP_HIGH("   crossPlLevel0En   :%x", cmd->cfg.crossPlLevel0En);
    ISP_HIGH("   crossPlLevel1En   :%x", cmd->cfg.crossPlLevel1En);
    ISP_HIGH("   minMaxEn  :%x", cmd->cfg.minMaxEn);
    ISP_HIGH("      distGrGb_0      :%x", cmd->level1.distGrGb_0);
    ISP_HIGH("      distGrGb_1        :%x", cmd->level1.distGrGb_1);
    ISP_HIGH("      distGrGb_2        :%x", cmd->level1.distGrGb_2);
    ISP_HIGH("      distGrGb_3        :%x", cmd->level1.distGrGb_3);
    ISP_HIGH("      distGrGb_4        :%x", cmd->level1.distGrGb_4);
    ISP_HIGH("      minMaxSelGrGb0    :%x", cmd->level1.minMaxSelGrGb0);
    ISP_HIGH("      minMaxSelGrGb1    :%x", cmd->level1.minMaxSelGrGb1);
    ISP_HIGH("      distRb0            :%x", cmd->level1.distRb0);
    ISP_HIGH("      distRb1            :%x", cmd->level1.distRb1);
    ISP_HIGH("      distRb2            :%x", cmd->level1.distRb2);
    ISP_HIGH("      distRb3            :%x", cmd->level1.distRb3);
    ISP_HIGH("      distRb4            :%x", cmd->level1.distRb4);
    ISP_HIGH("      minMaxSelRb0       :%x", cmd->level1.minMaxSelRb0);
    ISP_HIGH("      minMaxSelRb1       :%x", cmd->level1.minMaxSelRb1);
#define CH_TEST(f) ISP_HIGH("%s %x",#f, cmd->ch.f);

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

}

/** abf44_config_reserve
 *
 *  @isp_sub_module: isp sub module handle
 *  @abf: abf handle
 *
 *  abf module reserve configuration
 *
 *  Return None
 **/

static boolean abf44_config_reserve(isp_sub_module_t *isp_sub_module,
  abf44_t *abf)
{
  int i = 0;
  chromatix_ABF3_reserve *abf3_reserve = NULL;
  chromatix_parms_type     *chromatix_ptr = NULL;
  chromatix_ABF3_type *chromatix_ABF3 = NULL;
  chromatix_abf3_core_type *abf3_core = NULL;
  uint8_t                   default_idx = 0;

  if (!isp_sub_module || !abf) {
    ISP_ERR("failed: %p %p", isp_sub_module, abf);
    return FALSE;
  }
  if (!isp_sub_module->chromatix_ptrs.chromatixPtr) {
    ISP_ERR("failed:isp_sub_module->chromatix_ptrs.chromatixPtr %p",
      isp_sub_module->chromatix_ptrs.chromatixPtr);
    return FALSE;
  }
  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  chromatix_ABF3 = &chromatix_ptr->chromatix_VFE.chromatix_ABF3;
  abf3_reserve = &(chromatix_ABF3->abf3_reserved_data);
  abf3_core = &chromatix_ABF3->abf3_config[default_idx];

  abf->v3RegCmd.cfg.abf3En = 1;
  abf->v3RegCmd.cfg.lutBankSel =0;
  abf->v3RegCmd.cfg.iirLevel0En     = (abf3_reserve->filter_options >>
                                                            IIRLEVEL0EN) & 1;
  abf->v3RegCmd.cfg.iirLevel1En     = (abf3_reserve->filter_options >>
                                                            IIRLEVEL1EN) & 1;
  abf->v3RegCmd.cfg.crossPlLevel0En = (abf3_reserve->filter_options >>
                                                          CROSSP1LEVEL0EN) & 1;
  abf->v3RegCmd.cfg.crossPlLevel1En = (abf3_reserve->filter_options >>
                                                          CROSSP1LEVEL1EN) & 1;
  abf->v3RegCmd.cfg.minMaxEn        = (abf3_reserve->filter_options >>
                                                            MINMAXEN) & 1;
  abf->v3RegCmd.cfg.pipeFlushOvd    = (abf3_reserve->filter_options >> 7) & 1;
  abf->v3RegCmd.cfg.flushHaltOvd    = (abf3_reserve->filter_options >> 8) & 1;
  abf->v3RegCmd.cfg.pipeFlushCnt    = (abf3_reserve->filter_options >> 16) &
    0x1FFF;

  abf->v3RegCmd.level1.distGrGb_0 = Clamp(abf3_reserve->distance_ker[0][0],0, 3);
  abf->v3RegCmd.level1.distGrGb_1 = Clamp(abf3_reserve->distance_ker[0][1],0, 3);
  abf->v3RegCmd.level1.distGrGb_2 = Clamp(abf3_reserve->distance_ker[0][2],0, 3);
  abf->v3RegCmd.level1.distGrGb_3 = Clamp(abf3_reserve->distance_ker[0][3],0, 3);
  abf->v3RegCmd.level1.distGrGb_4 = Clamp(abf3_reserve->distance_ker[0][4],1, 6);
  abf->v3RegCmd.level1.distRb0    = Clamp(abf3_reserve->distance_ker[1][0],0, 3);
  abf->v3RegCmd.level1.distRb1    = Clamp(abf3_reserve->distance_ker[1][1],0, 3);
  abf->v3RegCmd.level1.distRb2    = Clamp(abf3_reserve->distance_ker[1][2],0, 3);
  abf->v3RegCmd.level1.distRb3    = Clamp(abf3_reserve->distance_ker[1][3],0, 3);
  abf->v3RegCmd.level1.distRb4    = Clamp(abf3_reserve->distance_ker[1][4],1, 6);
  abf->v3RegCmd.level1.minMaxSelGrGb0 = abf3_reserve->minmax_sel[0][0];
  abf->v3RegCmd.level1.minMaxSelGrGb1 = abf3_reserve->minmax_sel[0][1];
  abf->v3RegCmd.level1.minMaxSelRb0   = abf3_reserve->minmax_sel[1][0];
  abf->v3RegCmd.level1.minMaxSelRb1   = abf3_reserve->minmax_sel[1][1];

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION != 0x304)
   #define CHANNEL_CFG_RESERVE(c, idx) \
    abf->v3RegCmd.ch.c##_noiseScale0Lv0 =\
    CLAMP_ABF(Round(((float)abf3_reserve->shot_noise_scale[0][idx]/SHORT_NOISE_SCALE)),4095); \
    abf->v3RegCmd.ch.c##_noiseScale0Lv1 =\
    CLAMP_ABF(Round(((float)abf3_reserve->shot_noise_scale[1][idx]/SHORT_NOISE_SCALE)),4095); \
    abf->v3RegCmd.ch.c##_noiseScale1Lv0 =\
    CLAMP_ABF(abf3_reserve->fpn_noise_scale[0][idx],DENOISE_STRENGTH_MAX_VAL); \
    abf->v3RegCmd.ch.c##_noiseScale1Lv1 =\
    CLAMP_ABF(abf3_reserve->fpn_noise_scale[1][idx],DENOISE_STRENGTH_MAX_VAL); \
    abf->v3RegCmd.ch.c##_wTableAdjLv0 = WTABLE_ADJ_VAL; \
    abf->v3RegCmd.ch.c##_wTableAdjLv1 = WTABLE_ADJ_VAL; \
    abf->v3RegCmd.ch.c##_noiseOffLv0 =\
    CLAMP_ABF(abf3_reserve->noise_offset[0][idx]*NOISE_OFFSET_SCALE, NOISE_OFFSET_MAX_VAL); \
    abf->v3RegCmd.ch.c##_noiseOffLv1 =\
    CLAMP_ABF(abf3_reserve->noise_offset[1][idx]*NOISE_OFFSET_SCALE, NOISE_OFFSET_MAX_VAL); \
    abf->v3RegCmd.ch.c##_curvOffLv0 = abf3_core->curve_offset[0][idx]; \
    abf->v3RegCmd.ch.c##_curvOffLv1 = abf3_core->curve_offset[1][idx]; \
    abf->v3RegCmd.ch.c##_sftThrdNoiseShiftLv0 = \
      CLAMP_ABF(abf3_reserve->softthld_noise_shift[0][idx],15); \
    abf->v3RegCmd.ch.c##_sftThrdNoiseShiftLv1 = \
      CLAMP_ABF(abf3_reserve->softthld_noise_shift[1][idx],15);
#else
   #define CHANNEL_CFG_RESERVE(c, idx) \
    abf->v3RegCmd.ch.c##_noiseScale0Lv0 =\
    CLAMP_ABF(Round(((float)abf3_reserve->shot_noise_scale[0][idx]/SHORT_NOISE_SCALE)),4095); \
    abf->v3RegCmd.ch.c##_noiseScale0Lv1 =\
    CLAMP_ABF(Round(((float)abf3_reserve->shot_noise_scale[1][idx]/SHORT_NOISE_SCALE)),4095); \
    abf->v3RegCmd.ch.c##_noiseScale1Lv0 =\
    CLAMP_ABF(abf3_reserve->fpn_noise_scale[0][idx], NOISE_OFFSET_MAX_VAL); \
    abf->v3RegCmd.ch.c##_noiseScale1Lv1 =\
    CLAMP_ABF(abf3_reserve->fpn_noise_scale[1][idx], NOISE_OFFSET_MAX_VAL); \
    abf->v3RegCmd.ch.c##_wTableAdjLv0 = WTABLE_ADJ_VAL; \
    abf->v3RegCmd.ch.c##_wTableAdjLv1 = WTABLE_ADJ_VAL; \
    abf->v3RegCmd.ch.c##_noiseOffLv0 =\
    CLAMP_ABF(abf3_reserve->noise_offset[0][idx]*NOISE_OFFSET_SCALE, NOISE_OFFSET_MAX_VAL); \
    abf->v3RegCmd.ch.c##_noiseOffLv1 =\
    CLAMP_ABF(abf3_reserve->noise_offset[1][idx]*NOISE_OFFSET_SCALE, NOISE_OFFSET_MAX_VAL); \
    abf->v3RegCmd.ch.c##_curvOffLv0 = abf3_reserve->curve_offset[0][idx]; \
    abf->v3RegCmd.ch.c##_curvOffLv1 = abf3_reserve->curve_offset[1][idx]; \
    abf->v3RegCmd.ch.c##_sftThrdNoiseShiftLv0 = \
      CLAMP_ABF(abf3_reserve->softthld_noise_shift[0][idx],15); \
    abf->v3RegCmd.ch.c##_sftThrdNoiseShiftLv1 = \
      CLAMP_ABF(abf3_reserve->softthld_noise_shift[1][idx],15);
#endif

  CHANNEL_CFG_RESERVE(r,  0);
  CHANNEL_CFG_RESERVE(gr, 1);
  CHANNEL_CFG_RESERVE(gb, 2);
  CHANNEL_CFG_RESERVE(b,  3);

  FILL_SIG2_LUT(abf->abf3_parms.signal2_lut_lv0,
    abf3_reserve->signal2_lut_level0, 32);
  FILL_SIG2_LUT(abf->abf3_parms.signal2_lut_lv1,
    abf3_reserve->signal2_lut_level1, 32);
  return TRUE;
}



/** abf44_init_default
 *
 *  @isp_sub_module: isp sub module handle
 *  @abf: abf handle
 *
 *  abf module configuration initial settings
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean abf44_init_default(isp_sub_module_t *isp_sub_module,
  abf44_t *abf)
{
  int i = 0;
  if (!isp_sub_module || !abf) {
    ISP_ERR("failed: %p %p", isp_sub_module, abf);
    return FALSE;
  }
  if (!abf44_config_reserve(isp_sub_module, abf)) return FALSE;

  return TRUE;
} /* abg44_config */

static void abf44_config_trigger_interpolate(isp_sub_module_t *isp_sub_module,
  abf44_t *abf)
{
  int i = 0;
  float tmp = 0;
  chromatix_abf3_core_type *abf3_data_start = NULL;
  chromatix_abf3_core_type *abf3_data_end = NULL;
  chromatix_parms_type     *chromatix_ptr = NULL;
  chromatix_ABF3_type *chromatix_ABF3 = NULL;
  float ratio = (1 - abf->aec_ratio.ratio);

  /*if (!isp_sub_module || !abf) {
    ISP_ERR("failed: %p %p", isp_sub_module, abf);
    return FALSE;
  }*/
  ISP_HIGH("%s", __func__);
  if (abf->trigger_index + 1 >= MAX_LIGHT_TYPES_FOR_SPATIAL + NUM_SCENE_MODES)
    return;

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return;
  }

  chromatix_ABF3 = &chromatix_ptr->chromatix_VFE.chromatix_ABF3;

  abf3_data_start = &(chromatix_ABF3->abf3_config[abf->trigger_index]);
  abf3_data_end   = &(chromatix_ABF3->abf3_config[abf->trigger_index + 1]);
  #define CHANNEL_CFG_INTERP(c, idx) \
    tmp = (float)(LINEAR_INTERPOLATION(abf3_data_start->edge_softness[0][idx],\
      abf3_data_end->edge_softness[0][idx],\
      ratio));\
    abf->v3RegCmd.ch.c##_noiseScale2Lv0 = CLAMP_ABF(Round((float)(1 << 8) * (tmp)),4095);\
    tmp = (float)(LINEAR_INTERPOLATION(abf3_data_start->edge_softness[1][idx],\
      abf3_data_end->edge_softness[1][idx],\
      ratio));\
    abf->v3RegCmd.ch.c##_noiseScale2Lv1 = CLAMP_ABF(Round((float)(1 << 8) * (tmp)),4095);\
    tmp = (float)LINEAR_INTERPOLATION(abf3_data_start->filter_strength[0][idx],\
      abf3_data_end->filter_strength[0][idx],\
      ratio);\
    abf->v3RegCmd.ch.c##_filterStrLv0 = Clamp(Round((float)(1 << 8) * \
      (tmp)), 0, DENOISE_STRENGTH_MAX_VAL);\
    abf->v3RegCmd.ch.c##_filterStrLv0 = \
      (DENOISE_STRENGTH_MAX_VAL -abf->v3RegCmd.ch.c##_filterStrLv0); \
    tmp = (float)LINEAR_INTERPOLATION(abf3_data_start->filter_strength[1][idx],\
      abf3_data_end->filter_strength[1][idx],\
      ratio);\
    abf->v3RegCmd.ch.c##_filterStrLv1 = \
      Clamp(Round((float)(1 << 8) * (tmp)), 0, DENOISE_STRENGTH_MAX_VAL);\
    abf->v3RegCmd.ch.c##_filterStrLv1 = \
      DENOISE_STRENGTH_MAX_VAL - abf->v3RegCmd.ch.c##_filterStrLv1; \
    tmp = (float)LINEAR_INTERPOLATION(\
      abf3_data_start->softthld_noise_scale[0][idx],\
      abf3_data_end->softthld_noise_scale[0][idx],\
      ratio);\
    abf->v3RegCmd.ch.c##_sftThrdNoiseScaleLv0 = Round((float)(1 << 8) * (tmp));\
    tmp = (float)LINEAR_INTERPOLATION(\
      abf3_data_start->softthld_noise_scale[1][idx],\
      abf3_data_end->softthld_noise_scale[1][idx],\
      ratio);\
    abf->v3RegCmd.ch.c##_sftThrdNoiseScaleLv1 = Round((float)(1 << 8) * (tmp));

  CHANNEL_CFG_INTERP(r,  0);
  CHANNEL_CFG_INTERP(gr, 1);
  CHANNEL_CFG_INTERP(gb, 2);
  CHANNEL_CFG_INTERP(b,  3);

  #define FILL_LUT_INTERP(dst, src1, src2, len) \
    { \
      int32_t base; \
      int32_t next; \
      int32_t delta; \
      for (i = 0; i < (len-1); i++) { \
        base = roundf(((src1)[i])\
        * (1.0f - abf->aec_ratio.ratio) + ((src2)[i]) * \
          abf->aec_ratio.ratio); \
        next = roundf(((src1)[i+1])\
        * (1.0f - abf->aec_ratio.ratio) + \
        ((src2)[i+1] ) * abf->aec_ratio.ratio); \
        base = CLAMP_ABF(base,65535);\
        next = CLAMP_ABF(next,65535);\
        delta = Clamp((next - base),-32768,32767);\
        (dst)[i] = (delta << 16) | base; \
      } \
      base = roundf(((src1)[i])\
        * (1.0f - abf->aec_ratio.ratio) + ((src2)[i]) * \
          abf->aec_ratio.ratio); \
      base = CLAMP_ABF(base, 65535);\
      next = roundf(((src1)[i+1])\
        * (1.0f - abf->aec_ratio.ratio) + \
        ((src2)[i+1] ) * abf->aec_ratio.ratio);\
      delta = Clamp((next - base),-32768,32767);\
      (dst)[i] = ((delta ) << 16) |(base ); \
    }

  FILL_LUT_INTERP(abf->abf3_parms.noise_std2_lut_lv0,
                  abf3_data_start->noise_std2_lut_level0,
                  abf3_data_end->noise_std2_lut_level0,
                  128);
  FILL_LUT_INTERP(abf->abf3_parms.noise_std2_lut_lv1,
                  abf3_data_start->noise_std2_lut_level1,
                  abf3_data_end->noise_std2_lut_level1,
                  128);
}

static void abf44_config_trigger(isp_sub_module_t *isp_sub_module, abf44_t *abf)
{
  int i = 0;
  chromatix_abf3_core_type *abf3_data = NULL;
  chromatix_parms_type     *chromatix_ptr = NULL;
  chromatix_ABF3_type *chromatix_ABF3 = NULL;
  uint8_t tmp;

  if (!isp_sub_module || !abf) {
    ISP_ERR("failed: %p %p", isp_sub_module, abf);
    return;
  }

  if (abf->trigger_index >= MAX_LIGHT_TYPES_FOR_SPATIAL + NUM_SCENE_MODES)
    return;


  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return;
  }

  chromatix_ABF3 = &chromatix_ptr->chromatix_VFE.chromatix_ABF3;

  abf3_data = &(chromatix_ABF3->abf3_config[abf->trigger_index]);

  #define CHANNEL_CFG_TRIGGER(c, idx) \
    abf->v3RegCmd.ch.c##_noiseScale2Lv0 = \
       CLAMP_ABF((FLOAT_TO_Q(8, abf3_data->edge_softness[0][idx])),((1<<12)-1)); \
    abf->v3RegCmd.ch.c##_noiseScale2Lv1 = \
      CLAMP_ABF((FLOAT_TO_Q(8, abf3_data->edge_softness[1][idx])),((1<<12)-1)); \
    abf->v3RegCmd.ch.c##_filterStrLv0 = \
      Clamp(FLOAT_TO_Q(8, abf3_data->filter_strength[0][idx]),0,DENOISE_STRENGTH_MAX_VAL); \
    abf->v3RegCmd.ch.c##_filterStrLv0 = \
      (DENOISE_STRENGTH_MAX_VAL -abf->v3RegCmd.ch.c##_filterStrLv0); \
    abf->v3RegCmd.ch.c##_filterStrLv1 = \
      Clamp(FLOAT_TO_Q(8, abf3_data->filter_strength[1][idx]),0,DENOISE_STRENGTH_MAX_VAL); \
    abf->v3RegCmd.ch.c##_filterStrLv1 = \
      (DENOISE_STRENGTH_MAX_VAL -abf->v3RegCmd.ch.c##_filterStrLv1); \
    abf->v3RegCmd.ch.c##_sftThrdNoiseScaleLv0 = \
       CLAMP_ABF(FLOAT_TO_Q(8, abf3_data->softthld_noise_scale[0][idx]),((1<<12)-1)); \
    abf->v3RegCmd.ch.c##_sftThrdNoiseScaleLv1 = \
      CLAMP_ABF(FLOAT_TO_Q(8, abf3_data->softthld_noise_scale[1][idx]),((1<<12)-1)); \

  CHANNEL_CFG_TRIGGER(r,  0);
  CHANNEL_CFG_TRIGGER(gr, 1);
  CHANNEL_CFG_TRIGGER(gb, 2);
  CHANNEL_CFG_TRIGGER(b,  3);
  FILL_LUT(abf->abf3_parms.noise_std2_lut_lv0, abf3_data->noise_std2_lut_level0,
    128);
  FILL_LUT(abf->abf3_parms.noise_std2_lut_lv1, abf3_data->noise_std2_lut_level1,
    128);
}


/** abf44_do_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @abf: abf struct data
 *
 *  prepare hw update list and append in isp sub module
 *
 * Returns TRUE on success and FALSE on failure
 **/
static boolean abf44_store_hw_update(
  isp_sub_module_t *isp_sub_module, abf44_t *abf)
{
  boolean                           ret = TRUE;
  struct msm_vfe_cfg_cmd2          *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd       *reg_cfg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list      *hw_update = NULL;
  ISP_ABF3_CmdType                 *v3RegCmd = NULL;
  int i;
  uint32_t *data;
  if (!isp_sub_module || !abf) {
    ISP_ERR("failed: %p %p", isp_sub_module, abf);
    return FALSE;
  }

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));
  cfg_cmd = &hw_update->cfg_cmd;

  /* use V3*/
  uint32_t bank;
  uint32_t channel[2][4] = {
    {
      ABF_STD2_L0_BANK0,
      ABF_STD2_L1_BANK0,
      ABF_SIG2_L0_BANK0,
      ABF_SIG2_L1_BANK0
    },
    {
      ABF_STD2_L0_BANK1,
      ABF_STD2_L1_BANK1,
      ABF_SIG2_L0_BANK1,
      ABF_SIG2_L1_BANK1
    }
  };

  if (ABF_CGC_OVERRIDE == TRUE) {
    ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
      ABF_CGC_OVERRIDE_REGISTER, ABF_CGC_OVERRIDE_BIT, TRUE);
    if (ret == FALSE) {
      ISP_ERR("failed: enable cgc");
    }
  }

  bank = abf->v3RegCmd.cfg.lutBankSel;
  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)CAM_MALLOC(sizeof(*reg_cfg_cmd));
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_CMD;
  }
  memset(reg_cfg_cmd, 0, sizeof(*reg_cfg_cmd));
  v3RegCmd = (ISP_ABF3_CmdType *)CAM_MALLOC(sizeof(*v3RegCmd));
  if (!v3RegCmd) {
    ISP_ERR("failed: reg_cmd %p", v3RegCmd);
    goto ERROR_REG_CMD;
  }
  memset(v3RegCmd, 0, sizeof(*v3RegCmd));
  *v3RegCmd = abf->v3RegCmd;

  ret = isp_sub_module_util_write_dmi(
   (void *)&abf->abf3_parms.noise_std2_lut_lv0, sizeof(noise_std2_lut_t),
    channel[bank][0],
    VFE_WRITE_DMI_32BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed writing table_g");
    return FALSE;
  }
  ret = isp_sub_module_util_write_dmi(
     (void *)&abf->abf3_parms.noise_std2_lut_lv1, sizeof(noise_std2_lut_t),
      channel[bank][1],
    VFE_WRITE_DMI_32BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed writing table_g");
    return FALSE;
  }
  ret = isp_sub_module_util_write_dmi(
    (void *)&abf->abf3_parms.signal2_lut_lv0, sizeof(signal2_lut_t),
    channel[bank][2],
    VFE_WRITE_DMI_32BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed writing table_g");
    return FALSE;
  }
  ret = isp_sub_module_util_write_dmi(
    (void *)&abf->abf3_parms.signal2_lut_lv1, sizeof(signal2_lut_t),
    channel[bank][3],
    VFE_WRITE_DMI_32BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed writing table_g");
    return FALSE;
  }

  /* configurations */
  cfg_cmd->cfg_data = (void *)v3RegCmd;
  cfg_cmd->cmd_len = sizeof(*v3RegCmd);
  cfg_cmd->cfg_cmd = (void *) reg_cfg_cmd;
  cfg_cmd->num_cfg = 1;

  reg_cfg_cmd[0].cmd_type = VFE_WRITE;
  reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_ABF3_OFF;
  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[0].u.rw_info.len = ISP_ABF3_LEN * sizeof(uint32_t);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update");

      free(v3RegCmd);
    goto ERROR_REG_CMD;
  }

  if (ABF_CGC_OVERRIDE == TRUE) {
    ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
      ABF_CGC_OVERRIDE_REGISTER, ABF_CGC_OVERRIDE_BIT, FALSE);
    if (ret == FALSE) {
      ISP_ERR("failed: disable cgc");
    }
  }

  abf->v3RegCmd.cfg.lutBankSel ^= 1;

  isp_sub_module->trigger_update_pending = FALSE;
  return TRUE;

ERROR_REG_CMD:
  free(reg_cfg_cmd);
ERROR_REG_CFG_CMD:
  free(hw_update);
  return FALSE;
} /* abf44_store_hw_update */

/** abf44_trigger_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  Perform trigger update if trigger_update_pending flag is
 *  TRUE and append hw update list in global list
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean abf44_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                                    ret = TRUE;
  abf44_t                                    *abf = NULL;
  isp_private_event_t                        *private_event = NULL;
  isp_sub_module_output_t                    *output = NULL;
  chromatix_parms_type                       *chromatix_ptr = NULL;
  abf3_parms_t                               *abf3_parms = NULL;
  chromatix_ABF3_type                        *chromatix_ABF3 = NULL;
  isp_sub_module_output_t                    *sub_module_output = NULL;
  isp_hw_read_info_t                         *abf_dmi_info = NULL;
  uint32_t                                   *abf_dmi_tbl = NULL;
  uint8_t                                     module_enable;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  sub_module_output = (isp_sub_module_output_t *)private_event->data;
  if (!sub_module_output) {
    ISP_ERR("failed: sub_module_output %p", sub_module_output);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  abf = (abf44_t *)isp_sub_module->private_data;
  if (!abf) {
    ISP_ERR("failed: abf %p", abf);
    goto ERROR;
  }

  if (isp_sub_module->manual_ctrls.manual_update &&
      isp_sub_module->chromatix_module_enable) {
    isp_sub_module->manual_ctrls.manual_update = FALSE;
    module_enable = (isp_sub_module->manual_ctrls.abf_mode ==
      CAM_NOISE_REDUCTION_MODE_OFF) ? FALSE : TRUE;

    if (isp_sub_module->submod_enable != module_enable) {
      isp_sub_module->submod_enable = module_enable;
      sub_module_output->stats_params->
        module_enable_info.reconfig_needed = TRUE;
      sub_module_output->stats_params->module_enable_info.
        submod_enable[isp_sub_module->hw_module_id] = module_enable;
      sub_module_output->stats_params->
        module_enable_info.submod_mask[isp_sub_module->hw_module_id] = 1;

      if (!isp_sub_module->submod_enable) {
        goto FILL_METADATA;
      }
    }
  }

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: stats_update %p", chromatix_ptr);
    goto ERROR;
  }
  chromatix_ABF3 = &chromatix_ptr->chromatix_VFE.chromatix_ABF3;


  if (isp_sub_module->submod_enable == FALSE) {
    /* ABF module is disabled, do not perform hw update */
    goto FILL_METADATA;
  }

  if ((isp_sub_module->submod_trigger_enable == TRUE) &&
    (isp_sub_module->trigger_update_pending == TRUE)) {
    ISP_DBG("aec_ratio.ratio = %f",abf->aec_ratio.ratio);
    if (abf->aec_ratio.ratio > 0) {
    abf44_config_trigger_interpolate(isp_sub_module, abf);
    } else {
      abf44_config_trigger(isp_sub_module, abf);
    }
    abf44_store_hw_update(isp_sub_module, abf);
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module,
          sub_module_output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
    goto ERROR;
  }

  if (sub_module_output->meta_dump_params->metadata_dump_enable == 1) {
    /*fill in DMI info*/
    abf_dmi_info = &sub_module_output->
      meta_dump_params->dmi_info[ISP_METADUMP_ABF];
      /* v3 */
      abf_dmi_info->read_length =
        sizeof(signal2_lut_t) * 2 +
        sizeof(noise_std2_lut_t) * 2;

#ifdef DYNAMIC_DMI_ALLOC
      abf_dmi_tbl =
        (uint32_t *)CAM_MALLOC(abf_dmi_info->read_length);
      if (!abf_dmi_tbl) {
        ISP_ERR("failed: %p", abf_dmi_tbl);
        ret = FALSE;
        goto ERROR;
      }
#endif
      memcpy(abf_dmi_info->hw_dmi_tbl,
        &abf->abf3_parms.noise_std2_lut_lv0,
        sizeof(noise_std2_lut_t));
      memcpy(abf_dmi_info->hw_dmi_tbl +
        sizeof(noise_std2_lut_t),
        &abf->abf3_parms.noise_std2_lut_lv1,
        sizeof(noise_std2_lut_t));
      memcpy(abf_dmi_info->hw_dmi_tbl +
        sizeof(noise_std2_lut_t) * 2,
        &abf->abf3_parms.signal2_lut_lv0,
        sizeof(signal2_lut_t));
      memcpy(abf_dmi_info->hw_dmi_tbl +
        (sizeof(noise_std2_lut_t) * 2) +
        sizeof(signal2_lut_t),
        &abf->abf3_parms.signal2_lut_lv1,
        sizeof(signal2_lut_t));
  }

FILL_METADATA:
   if (sub_module_output->frame_meta)
     sub_module_output->frame_meta->abf_mode =
       isp_sub_module->manual_ctrls.abf_mode;
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;
}

/** abf44_aec_manual_update:
 *
 * @mod: abf module
 * @data: handle to stats_update_t
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean abf44_aec_manual_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  abf44_t              *abf = NULL;
  aec_manual_update_t  *stats_update = NULL;
  trigger_ratio_t   aec_ratio;
  chromatix_parms_type  *chromatix_ptr = NULL;
  chromatix_ABF3_type *chromatix_ABF3 = NULL;
  trigger_point_type    *out_trigger_point = NULL;
  trigger_point_type    *low_trigger_point = NULL;
  uint8_t trigger_index = MAX_LIGHT_TYPES_FOR_SPATIAL + 1;
  float aec_reference;
  float ratio = 0;

  int i = 0;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  abf = (abf44_t *)isp_sub_module->private_data;
  if (!abf) {
    ISP_ERR("failed: abf %p", abf);
    return FALSE;
  }

  stats_update = (aec_manual_update_t *)data;
  abf->real_gain = stats_update->real_gain;
  aec_reference = abf->real_gain;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: stats_update %p", stats_update);
    goto ERROR;
  }

  chromatix_ABF3 =
    &chromatix_ptr->chromatix_VFE.chromatix_ABF3;

  ISP_HIGH("aec_reference :%f", aec_reference);

  for (i = 0; i < MAX_LIGHT_TYPES_FOR_SPATIAL; i++) {
    float start = 0;
    float end = 0;
    trigger_point_type *abf3_trigger =
      &(chromatix_ABF3->abf3_config[i].ABF3_trigger);

    trigger_index = i;
    if (i == MAX_LIGHT_TYPES_FOR_SPATIAL - 1) {
      /* falls within region 6 but we do not use trigger points in the region */
      ratio = 0;
      break;
    }

    if (chromatix_ABF3->control_abf3 == 0) {
      start = abf3_trigger->lux_index_start;
      end   = abf3_trigger->lux_index_end;
    } else {
      start = abf3_trigger->gain_start;
      end   = abf3_trigger->gain_end;
      ISP_HIGH("gain_start :%f", start);
      ISP_HIGH("gain_end :%f", end);
    }

    /* index is within interpolation range, find ratio */
     if (aec_reference >= start && aec_reference < end) {
       ratio = (aec_reference - start)/(end - start);
       ISP_DBG("%s [%f - %f - %f] = %f", __func__, start, aec_reference, end,
         ratio);
     }

     /* already scanned past the lux index */
     if (aec_reference < end) {
           break;
     }
  }

  if (trigger_index >= MAX_LIGHT_TYPES_FOR_SPATIAL) {
    ISP_ERR("no trigger match for ABF trigger value: %f\n", aec_reference);
    goto ERROR;
  }

  if ((trigger_index != abf->trigger_index) ||
    (!F_EQUAL(ratio, abf->aec_ratio.ratio))) {
    abf->trigger_index = trigger_index;
    abf->aec_ratio.ratio = ratio;
    isp_sub_module->trigger_update_pending = TRUE;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;
}

/** abf44_aec_update:
 *
 * @mod: abf module
 * @data: handle to stats_update_t
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean abf44_aec_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  stats_update_t        *stats_update = NULL;
  abf44_t   *abf = NULL;
  trigger_ratio_t   aec_ratio;
  chromatix_parms_type  *chromatix_ptr = NULL;
  chromatix_ABF3_type *chromatix_ABF3 = NULL;
  trigger_point_type    *out_trigger_point = NULL;
  trigger_point_type    *low_trigger_point = NULL;
  uint8_t trigger_index = MAX_LIGHT_TYPES_FOR_SPATIAL + 1;
  float aec_reference;
  float ratio = 0;

  int i = 0;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  abf = (abf44_t *)isp_sub_module->private_data;
  if (!abf) {
    ISP_ERR("failed: abf %p", abf);
    return FALSE;
  }

  stats_update = (stats_update_t *)data;
  if (!stats_update) {
    ISP_ERR("failed: stats_update %p", stats_update);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: stats_update %p", stats_update);
    goto ERROR;
  }

  chromatix_ABF3 =
    &chromatix_ptr->chromatix_VFE.chromatix_ABF3;
  if (chromatix_ABF3->control_abf3 == 0) {
    /* lux index based */
    aec_reference = stats_update->aec_update.lux_idx;
  } else {
    /* Gain based */
    aec_reference = stats_update->aec_update.sensor_gain;
  }

  ISP_HIGH("aec_reference :%f", aec_reference);

  for (i = 0; i < MAX_LIGHT_TYPES_FOR_SPATIAL; i++) {
    float start = 0;
    float end = 0;
    trigger_point_type *abf3_trigger =
      &(chromatix_ABF3->abf3_config[i].ABF3_trigger);

    trigger_index = i;
    if (i == MAX_LIGHT_TYPES_FOR_SPATIAL - 1) {
      /* falls within region 6 but we do not use trigger points in the region */
      ratio = 0;
      break;
    }

    if (chromatix_ABF3->control_abf3 == 0) {
      start = abf3_trigger->lux_index_start;
      end   = abf3_trigger->lux_index_end;
    } else {
      start = abf3_trigger->gain_start;
      end   = abf3_trigger->gain_end;
      ISP_HIGH("gain_start :%f", start);
      ISP_HIGH("gain_end :%f", end);
    }

    /* index is within interpolation range, find ratio */
     if (aec_reference >= start && aec_reference < end) {
       ratio = (aec_reference - start)/(end - start);
       ISP_DBG("%s [%f - %f - %f] = %f", __func__, start, aec_reference, end,
         ratio);
     }

     /* already scanned past the lux index */
     if (aec_reference < end) {
           break;
     }
  }

  if (trigger_index >= MAX_LIGHT_TYPES_FOR_SPATIAL) {
    ISP_ERR("no trigger match for ABF trigger value: %f\n", aec_reference);
    goto ERROR;
  }

  if ((trigger_index != abf->trigger_index) ||
    (!F_EQUAL(ratio, abf->aec_ratio.ratio))) {
    abf->trigger_index = trigger_index;
    abf->aec_ratio.ratio = ratio;
    isp_sub_module->trigger_update_pending = TRUE;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;
}

/** abf44_set_chromatix_ptr:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean abf44_set_chromatix_ptr(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean              ret = TRUE;
  abf44_t *abf = NULL;
  modulesChromatix_t  *chromatix_ptrs = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  abf = (abf44_t *)isp_sub_module->private_data;
  if (!abf) {
    ISP_ERR("failed: mod %p", abf);
    return FALSE;
  }

  chromatix_ptrs = (modulesChromatix_t *)data;
  if (!chromatix_ptrs) {
    ISP_ERR("failed: chromatix_ptrs %p", chromatix_ptrs);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  isp_sub_module->chromatix_ptrs = *chromatix_ptrs;

  ret = isp_sub_module_util_configure_from_chromatix_bit(isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: updating module enable bit for hw %d",
      isp_sub_module->hw_module_id);
  }

  ret = abf44_init_default(isp_sub_module, abf);
  if (ret == FALSE) {
    ISP_ERR("failed: abf44_config ret %d", ret);
    goto ERROR;
  }

  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* abf_set_chromatix_ptr */

/** abf44_streamoff:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean abf44_streamoff(isp_sub_module_t *isp_sub_module,
  void *data)
{
  abf44_t *abf = NULL;
  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  abf = (abf44_t *)isp_sub_module->private_data;
  if (!abf) {
    ISP_ERR("failed: abf %p", abf);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  memset(abf, 0, sizeof(*abf));
  abf->trigger_index = NORMAL_LIGHT_INDEX;
  isp_sub_module->manual_ctrls.abf_mode = CAM_NOISE_REDUCTION_MODE_FAST;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* abf44_streamoff */

/** abf44_init:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the abf module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean abf44_init(isp_sub_module_t *isp_sub_module)
{
  abf44_t *abf = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return FALSE;
  }

  abf = (abf44_t *)CAM_MALLOC(sizeof(abf44_t));
  if (!abf) {
    ISP_ERR("failed: abf %p", abf);
    return FALSE;
  }

  memset(abf, 0, sizeof(*abf));

  isp_sub_module->private_data = (void *)abf;
  isp_sub_module->manual_ctrls.abf_mode = CAM_NOISE_REDUCTION_MODE_FAST;
  abf->trigger_index = NORMAL_LIGHT_INDEX;

  return TRUE;
}/* abf44_init */

/** abf44_destroy:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void abf44_destroy(isp_sub_module_t *isp_sub_module)
{
  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return;
  }

  free(isp_sub_module->private_data);
  return;
} /* abf44_destroy */
