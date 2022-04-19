/* eztune_vfe_diagnostics.h
 *
 * Copyright (c) 2012-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __EZTUNE_VFE_DIAGNOSTICS_H__
#define __EZTUNE_VFE_DIAGNOSTICS_H__

#include <stdbool.h>

#define ROLLOFF_NUM_ROWS 13
#define ROLLOFF_NUM_COLS 17
#define ROLLOFF_TABLE_SIZE (ROLLOFF_NUM_ROWS * ROLLOFF_NUM_COLS)
#define LA_LUT_SIZE 64

typedef struct {
  int32_t enable;
  int32_t cntrlenable;
} config_cntrl_t;

typedef enum {
  SET_ENABLE,
  SET_CONTROLENABLE,
  SET_STATUS,
} optype_t;

typedef enum {
  PREVIEW,
  SNAPSHOT
} opmode_t;

typedef enum {
  VFE_MODULE_ALL,
  VFE_MODULE_LINEARIZATION,
  VFE_MODULE_COLORCORRECTION,
  VFE_MODULE_COLORCONVERSION,
  VFE_MODULE_GAMMA,
  VFE_MODULE_BLACKLEVEL,
  VFE_MODULE_ASF5X5,
  VFE_MODULE_LUMAADAPTATION,
  VFE_MODULE_ROLLOFF,
  VFE_MODULE_BPC,
  VFE_MODULE_BCC,
  VFE_MODULE_ABCC,
  VFE_MODULE_ABF,
  VFE_MODULE_CHROMASUPPRESSION,
  VFE_MODULE_MCE,
  VFE_MODULE_SCE,
  VFE_MODULE_DEMOSAIC,
  VFE_MODULE_DEMUX,
  VFE_MODULE_CLFILTER,
  VFE_MODULE_GIC,
  VFE_MODULE_GTM,
  VFE_MODULE_PEDESTAL,
} vfemodule_t;

typedef struct {
  vfemodule_t module;
  optype_t type;
} ez_vfecmd_t;

typedef struct {
  int32_t coef_rtor;
  int32_t coef_gtor;
  int32_t coef_btor;
  int32_t coef_rtog;
  int32_t coef_gtog;
  int32_t coef_btog;
  int32_t coef_rtob;
  int32_t coef_gtob;
  int32_t coef_btob;
  int32_t roffset;
  int32_t boffset;
  int32_t goffset;
  uint32_t coef_qfactor;
} colorcorrection_t;

typedef struct {
  int32_t param_ap;
  int32_t param_am;
  int32_t param_bp;
  int32_t param_bm;
  int32_t param_cp;
  int32_t param_cm;
  int32_t param_dp;
  int32_t param_dm;
  int32_t param_kcb;
  int32_t param_kcr;
  int32_t param_rtoy;
  int32_t param_gtoy;
  int32_t param_btoy;
  int32_t param_yoffset;
  int32_t param_boffset;
  int32_t param_roffset;
} chromaenhancement_t;

typedef struct {
  uint32_t greenEvenRow;
  uint32_t greenOddRow;
  uint32_t blue;
  uint32_t red;
} demuxchannelgain_t;

typedef struct {
  uint32_t evenRevenC;
  uint32_t evenRoddC;
  uint32_t oddRevenC;
  uint32_t oddRoddC;
} blacklevelcorrection_t;

typedef struct {
  float coefftable_R[ROLLOFF_TABLE_SIZE];
  float coefftable_Gr[ROLLOFF_TABLE_SIZE];
  float coefftable_Gb[ROLLOFF_TABLE_SIZE];
  float coefftable_B[ROLLOFF_TABLE_SIZE];
} rolloff_t;

typedef struct {
  uint32_t smoothfilterEnabled;
  uint32_t sharpMode;
  uint32_t lpfMode;
  uint32_t smoothcoefCenter;
  uint32_t smoothcoefSurr;
  uint32_t pipeflushCount;
  uint32_t pipeflushOvd;
  uint32_t flushhaltOvd;
  uint32_t cropEnable;
  uint32_t normalizeFactor;
  uint32_t sharpthreshE1;
  int32_t sharpthreshE2;
  int32_t sharpthreshE3;
  int32_t sharpthreshE4;
  int32_t sharpthreshE5;
  int32_t sharpK1;
  int32_t sharpK2;

  int32_t f1coef0;
  int32_t f1coef1;
  int32_t f1coef2;
  int32_t f1coef3;
  int32_t f1coef4;
  int32_t f1coef5;
  int32_t f1coef6;
  int32_t f1coef7;
  int32_t f1coef8;
  int32_t f2coef0;
  int32_t f2coef1;
  int32_t f2coef2;
  int32_t f2coef3;
  int32_t f2coef4;
  int32_t f2coef5;
  int32_t f2coef6;
  int32_t f2coef7;
  int32_t f2coef8;
  int32_t f3coef0;
  int32_t f3coef1;
  int32_t f3coef2;
  int32_t f3coef3;
  int32_t f3coef4;
  int32_t f3coef5;
  int32_t f3coef6;
  int32_t f3coef7;
  int32_t f3coef8;
} asfsharpness5x5_t;

typedef struct {
  int32_t lut_yratio[LA_LUT_SIZE];
} lumaadaptation_t;

typedef struct {
  uint32_t ysup1;
  uint32_t ysup2;
  uint32_t ysup3;
  uint32_t ysup4;
  uint32_t ysupM1;
  uint32_t ysupM3;
  uint32_t ysupS1;
  uint32_t ysupS3;
  uint32_t csup1;
  uint32_t csup2;
  uint32_t csupM1;
  uint32_t csupS1;
} chromasuppression_t;

typedef struct {
  uint32_t y1;
  uint32_t y2;
  uint32_t y3;
  uint32_t y4;
  uint32_t yM1;
  uint32_t yM3;
  uint32_t yS1;
  uint32_t yS3;
  uint32_t transWidth;
  uint32_t transTrunc;
  int32_t crZone;
  int32_t cbZone;
  int32_t translope;
  int32_t k;
} memcolorenhancement_t;

typedef struct {
  uint32_t qk;
  memcolorenhancement_t red;
  memcolorenhancement_t green;
  memcolorenhancement_t blue;
} memorycolorenhancement_t;

typedef struct {
  uint32_t fminThreshold;
  uint32_t fmaxThreshold;
  uint32_t rOffsetLo;
  uint32_t rOffsetHi;
  uint32_t grOffsetLo;
  uint32_t gbOffsetLo;
  uint32_t gbOffsetHi;
  uint32_t grOffsetHi;
  uint32_t bOffsetLo;
  uint32_t bOffsetHi;
} badcorrection_t;

typedef struct {
  uint32_t wk;
  uint32_t bk;
  uint32_t lk;
  uint32_t tk;
} interpclassifier_t;

typedef struct {
  interpclassifier_t lut[18];
  bool disable_dirG;
  bool enable_dynG;
  bool disable_dirRB;
  bool enable_dynRB;
  bool enable_v4;
  bool enable_dynClamp_RBXcl;
  bool enable_dynClamp_GXcl;
  uint32_t aG;
  uint32_t bL;
  uint32_t noise_levelG;
  uint32_t noise_levelRB;
  uint32_t bU;
  uint32_t dbLU;
  uint32_t krg;
  uint32_t kbg;
  uint32_t kgr;
  uint32_t kgb;
  uint32_t lambdaG;
  uint32_t lambdaRB;
  uint32_t wgr1;
  uint32_t wgr2;
} demosaic3_t;

typedef struct {
  int32_t vertex00; /*triangle0.point1.cr / cb*/
  int32_t vertex01; /*triangle0.point2.cr / cb*/
  int32_t vertex02; /*triangle0.point3.cr / cb*/
  int32_t vertex10;
  int32_t vertex11;
  int32_t vertex12;
  int32_t vertex20;
  int32_t vertex21;
  int32_t vertex22;
  int32_t vertex30;
  int32_t vertex31;
  int32_t vertex32;
  int32_t vertex40;
  int32_t vertex41;
  int32_t vertex42;
}skinenhancoordinates_t;

typedef struct {
  int32_t coef00; /*cr-coeffA / cb-coeffD*/
  int32_t coef01; /*cr-coeffB / cb-coeffE*/
  int32_t coef10;
  int32_t coef11;
  int32_t coef20;
  int32_t coef21;
  int32_t coef30;
  int32_t coef31;
  int32_t coef40;
  int32_t coef41;
  int32_t coef50;
  int32_t coef51;
}skinenhancoeff_t;

typedef struct {
  int32_t offset0; /*cr-coeffC / cb-coeffF*/
  int32_t offset1;
  int32_t offset2;
  int32_t offset3;
  int32_t offset4;
  int32_t offset5;
  uint32_t shift0; /* tr0 cr-matrix_shift cb-offset_shift*/
  uint32_t shift1;
  uint32_t shift2;
  uint32_t shift3;
  uint32_t shift4;
  uint32_t shift5;
} skinenhanoffset_t;

typedef struct {
  skinenhancoordinates_t crcoord;
  skinenhancoordinates_t cbcoord;
  skinenhancoeff_t crcoeff;
  skinenhancoeff_t cbcoeff;
  skinenhanoffset_t croffset;
  skinenhanoffset_t cboffset;
}skincolorenhancement_t;

typedef struct {
  float hcoeff[4];
  float vcoeff[2];
} chromafiltercoeff_t;

typedef struct {
  uint16_t threshold[3];
  uint16_t pos[16];
  int16_t neg[8];
} filterdata_t;

typedef struct {
  // version 2 fields
  filterdata_t red;
  filterdata_t green;
  filterdata_t blue;
  // version 3 fields
  bool     enable_v3;
  uint32_t noise_std2_lut_level0[129];  // level 0 noise variance LUT
  uint32_t noise_std2_lut_level1[129];  // level 1 noise variance LUT
  float    edge_softness;
  float    softthld_noise_scale[8];
  float    filter_strength[8];
  int32_t  lut_bank_sel;
  int32_t  filter_options;
  int32_t  signal2_lut_level0[32];
  int32_t  signal2_lut_level1[32];
  int32_t  w_table_adj[8];
  int32_t  noise_scale0[8];
  int32_t  noise_scale1[8];
  int32_t  noise_scale2[8];
  int32_t  noise_offset[8];
  int32_t  softthld_noise_shift[8];
  int32_t  distance_ker0[5];
  int32_t  distance_ker1[5];
  int32_t  min_max_sel0[2];
  int32_t  min_max_sel1[2];
  int32_t  curve_offset[8];
} abffilterdata_t;

typedef struct {
  unsigned short threshold_red[3];
  float scalefactor_red[2];
  unsigned short threshold_green[3];
  float scalefactor_green[2];
  float a[2];
  unsigned short threshold_blue[3];
  float scalefactor_blue[2];
  float tablepos[16];
  float tableneg[8];
} lumafiltercoeff_t;


typedef struct {
  chromafiltercoeff_t chromafilter;
  lumafiltercoeff_t lumafilter;
} chromalumafiltercoeff_t;

typedef struct
{
  uint16_t rlut_pl[8];
  uint16_t grlut_pl[8];
  uint16_t gblut_pl[8];
  uint16_t blut_pl[8];
  uint16_t rlut_base[9];
  uint16_t grlut_base[9];
  uint16_t gblut_base[9];
  uint16_t blut_base[9];
  uint32_t rlut_delta[9];
  uint32_t grlut_delta[9];
  uint32_t gblut_delta[9];
  uint32_t blut_delta[9];
} linearization_t;

typedef struct
{
  uint32_t nzhv;
  uint32_t threshold_edge;
  uint32_t threshold_saturation;
  uint32_t poss_cap_pixels_rg_high_thresh;
  uint32_t poss_cap_pixels_rg_low_thresh;
  uint32_t poss_cap_pixels_bg_high_thresh;
  uint32_t poss_cap_pixels_bg_low_thresh;
  uint32_t rg_diff_high_thresh;
  uint32_t rg_diff_low_thresh;
  uint32_t bg_diff_high_thresh;
  uint32_t bg_diff_low_thresh;
  uint32_t edge_hv;
  uint32_t edge_diag;
  uint32_t nz_diag;
  uint32_t force_off;
  uint32_t inv_slope_rg;
  uint32_t qi_slope_rg;
  uint32_t inv_slope_bg;
  uint32_t qi_slope_bg;
  uint32_t gain_g;
  uint32_t offset_g;
  uint32_t gain_b;
  uint32_t offset_b;
  uint32_t gain_r;
  uint32_t offset_r;
} chromatixaberrationcorr_t;

typedef struct {
  uint32_t NoiseStd2LUTLevel0[128];
  uint32_t Signal2LUTLevel0[32];
  uint32_t SoftThreshNoiseScale;
  uint32_t SoftThreshNoiseShift;
  uint32_t FilterStrength;
  uint32_t NoiseScale0;
  uint32_t NoiseScale1;
  uint32_t NoiseOffset;
} gicdiag_t;;

typedef struct {
  uint32_t Xarr[64];
  float YRatioBase[64];
  int32_t YRatioSlope[64]; //not sure what the datatype is and where to map it to

  float AMiddleTone;
  int32_t MaxValThresh;
  int32_t KeyMinThresh;
  int32_t KeyMaxThresh;
  float KeyHistBinWeight;
  int32_t YoutMaxVal;
  float TemporalW;
  float MiddleToneW;
  uint32_t LUTBankSel;
} gtmdiag_t;

typedef struct {
  uint16_t MeshTableT1_R[130];
  uint16_t MeshTableT1_Gr[130];
  uint16_t MeshTableT1_Gb[130];
  uint16_t MeshTableT1_B[130];
  uint16_t MeshTableT2_R[130];
  uint16_t MeshTableT2_Gr[130];
  uint16_t MeshTableT2_Gb[130];
  uint16_t MeshTableT2_B[130];
  uint32_t HDREnable;
  uint32_t ScaleBypass;
  uint32_t IntpFactor;
  uint32_t BWidth;
  uint32_t BHeight;
  uint32_t XDelta;
  uint32_t YDelta;

  //for the following params, not sure where to map them to
  uint32_t LeftImageWD;
  uint32_t Enable3D;
  uint32_t MeshGridBWidth;
  uint32_t MeshGridBHeight;
  uint32_t LXStart;
  uint32_t LYStart;
  uint32_t BXStart;
  uint32_t BYStart;
  uint32_t BXD1;
  uint32_t BYE1;
  uint32_t BYInitE1;
} pedestaldiag_t;

typedef struct {
  config_cntrl_t control_colorcorr;
  colorcorrection_t prev_colorcorr;
  colorcorrection_t snap_colorcorr;
  config_cntrl_t control_colorconv;
  chromaenhancement_t colorconv;

  config_cntrl_t control_gamma;
  config_cntrl_t control_rolloff;
  rolloff_t prev_rolloff;
  rolloff_t snap_rolloff;

  config_cntrl_t control_demux;
  demuxchannelgain_t prev_demuxchannelgain;
  demuxchannelgain_t snap_demuxchannelgain;

  config_cntrl_t control_blacklevel;
  blacklevelcorrection_t prev_blacklevel;
  blacklevelcorrection_t snap_blacklevel;

  config_cntrl_t control_asf5x5;
  asfsharpness5x5_t prev_asf5x5;
  asfsharpness5x5_t snap_asf5x5;

  config_cntrl_t control_lumaadaptation;
  lumaadaptation_t prev_lumaadaptation;
  lumaadaptation_t snap_lumaadaptation;

  config_cntrl_t control_chromasupp;
  chromasuppression_t prev_chromasupp;
  chromasuppression_t snap_chromasupp;

  config_cntrl_t control_memcolorenhan;
  memorycolorenhancement_t prev_memcolorenhan;
  memorycolorenhancement_t snap_memcolorenhan;

  config_cntrl_t control_bpc;
  badcorrection_t prev_bpc;
  badcorrection_t snap_bpc;
  config_cntrl_t control_bcc;
  badcorrection_t prev_bcc;
  badcorrection_t snap_bcc;

  config_cntrl_t control_abfilter;
  abffilterdata_t prev_abfilter;
  abffilterdata_t snap_abfilter;

  config_cntrl_t control_demosaic;
  demosaic3_t prev_demosaic;
  demosaic3_t snap_demosaic;

  config_cntrl_t control_skincolorenhan;
  skincolorenhancement_t prev_skincolorenhan;
  skincolorenhancement_t snap_skincolorenhan;

  config_cntrl_t control_linear;
  linearization_t prev_linear;
  linearization_t snap_linear;

  config_cntrl_t control_clfilter;
  chromalumafiltercoeff_t prev_chromalumafilter;
  chromalumafiltercoeff_t snap_chromalumafilter;

  config_cntrl_t control_ltm;

  chromatixaberrationcorr_t cac;

  config_cntrl_t control_gicdiag;
  gicdiag_t prev_gicdiag;
  //not sure if snapshot is needed

  config_cntrl_t control_gtmdiag;
  gtmdiag_t prev_gtmdiag;
  //not sure if snapshot is needed

  config_cntrl_t control_pedestaldiag;
  pedestaldiag_t prev_pedestaldiag;
  //not sure if snapshot is required
} vfe_diagnostics_t;

#endif /* __EZTUNE_VFE_DIAGNOSTICS_H__ */
