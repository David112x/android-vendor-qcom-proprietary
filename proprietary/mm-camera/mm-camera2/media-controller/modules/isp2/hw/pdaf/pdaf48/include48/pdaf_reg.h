/*
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef __PDAF_REG_H__
#define __PDAF_REG_H__

/*============================================================================
                    Module -- PDAF....starting
============================================================================*/
#define PDAF_OFFSET                   0x00000CB4
#define PDAF_HDR_OFFSET               0x00000CB8
#define PDAF_BP_OFFSET                0x00000CBC
#define PDAF_RG_OFFSET                0x00000CC8
#define PDAF_BG_OFFSET                0x00000CCC
#define PDAF_GR_OFFSET                0x00000CD0
#define PDAF_GB_OFFSET                0x00000CD4
#define PDAF_LOC_OFFSET               0x00000CD8
#define PDAF_TOTAL_NUM_REG            11

#define PDAF_LUT_COUNT                64

#define PDAF_CGC_OVERRIDE             TRUE
#define PDAF_CGC_OVERRIDE_REGISTER    0x2C
#define PDAF_CGC_OVERRIDE_BIT         13

/////////////////////////////////////////////////////////////////////////////
////////////////////////hdr submodule starting///////////////////////////////
typedef struct ISP_pdaf_config {
  uint32_t       pdaf_pdpc_en           :  1;
  uint32_t       pdaf_dsbpc_en          :  1;
  uint32_t      /* reserved */          :  6;
  uint32_t       blk_lvl                :  12;
  uint32_t      /* reserved */          :  12;
}__attribute__((packed, aligned(4))) ISP_pdaf_config;

/////////////////////////////////////////////////////////////////////////////
////////////////////////hdr submodule starting///////////////////////////////
typedef struct ISP_pdaf_hdr_exp_ratio {
  uint32_t       exp_ratio_recip        :  9;
  uint32_t      /* reserved */          :  7;
  uint32_t       exp_ratio              :  15;
  uint32_t      /* reserved */          :  1;
}__attribute__((packed, aligned(4))) ISP_pdaf_hdr_exp_ratio;

typedef struct ISP_pdaf_hdr_config {
  ISP_pdaf_hdr_exp_ratio              hdr_exp_ratio;
}__attribute__((packed, aligned(4))) ISP_pdaf_hdr_config;

/////////////////////////////////////////////////////////////////////////////
////////////////////////hdr submodule ending///////////////////////////////

/////////////////////////////////////////////////////////////////////////////
////////////////////////bp submodule starting///////////////////////////////
typedef struct ISP_pdaf_bp_th {
  uint32_t       fmax                   :  8;
  uint32_t       fmin                   :  8;
  uint32_t      /* reserved */          :  16;
}__attribute__((packed, aligned(4))) ISP_pdaf_bp_th;

typedef struct ISP_pdaf_bp_offset {
  uint32_t       offset_rb_pixel        :  15;
  uint32_t      /* reserved */          :  1;
  uint32_t       offset_g_pixel         :  15;
  uint32_t      /* reserved */          :  1;
}__attribute__((packed, aligned(4))) ISP_pdaf_bp_offset;

typedef struct ISP_pdaf_bp_config {
  ISP_pdaf_bp_offset                  bp_offset_t2;
  ISP_pdaf_bp_th                      bp_th;
  ISP_pdaf_bp_offset                  bp_offset;
}__attribute__((packed, aligned(4))) ISP_pdaf_bp_config;

/////////////////////////////////////////////////////////////////////////////
////////////////////////bp submodule ending///////////////////////////////

/////////////////////////////////////////////////////////////////////////////
////////////////////////rg submodule starting///////////////////////////////
typedef struct ISP_pdaf_rg_wb_gain {
  uint32_t       rg_wb_gain             :  17;
  uint32_t      /* reserved */          :  15;
}__attribute__((packed, aligned(4))) ISP_pdaf_rg_wb_gain;

/////////////////////////////////////////////////////////////////////////////
////////////////////////rg submodule ending///////////////////////////////

/////////////////////////////////////////////////////////////////////////////
////////////////////////bg submodule starting///////////////////////////////
typedef struct ISP_pdaf_bg_wb_gain {
  uint32_t       bg_wb_gain             :  17;
  uint32_t      /* reserved */          :  15;
}__attribute__((packed, aligned(4))) ISP_pdaf_bg_wb_gain;

/////////////////////////////////////////////////////////////////////////////
////////////////////////bg submodule ending///////////////////////////////

/////////////////////////////////////////////////////////////////////////////
////////////////////////gr submodule starting///////////////////////////////
typedef struct ISP_pdaf_gr_wb_gain {
  uint32_t       gr_wb_gain             :  17;
  uint32_t      /* reserved */          :  15;
}__attribute__((packed, aligned(4))) ISP_pdaf_gr_wb_gain;

/////////////////////////////////////////////////////////////////////////////
////////////////////////gr submodule ending///////////////////////////////

/////////////////////////////////////////////////////////////////////////////
////////////////////////gb submodule starting///////////////////////////////
typedef struct ISP_pdaf_gb_wb_gain {
  uint32_t       gb_wb_gain             :  17;
  uint32_t      /* reserved */          :  15;
}__attribute__((packed, aligned(4))) ISP_pdaf_gb_wb_gain;

/////////////////////////////////////////////////////////////////////////////
////////////////////////gb submodule ending///////////////////////////////

/////////////////////////////////////////////////////////////////////////////
////////////////////////wb gain submodule starting///////////////////////////////
typedef struct ISP_pdaf_wb_gain_config {
  ISP_pdaf_rg_wb_gain                 rg_wb_gain;
  ISP_pdaf_bg_wb_gain                 bg_wb_gain;
  ISP_pdaf_gr_wb_gain                 gr_wb_gain;
  ISP_pdaf_gb_wb_gain                 gb_wb_gain;
}__attribute__((packed, aligned(4))) ISP_pdaf_wb_gain_config;

/////////////////////////////////////////////////////////////////////////////
////////////////////////wb gain submodule ending///////////////////////////////

/////////////////////////////////////////////////////////////////////////////
////////////////////////loc submodule starting///////////////////////////////
typedef struct ISP_pdaf_loc_offset_cfg {
  uint32_t       x_offset               :  14;
  uint32_t      /* reserved */          :  2;
  uint32_t       y_offset               :  14;
  uint32_t      /* reserved */          :  2;
}__attribute__((packed, aligned(4))) ISP_pdaf_loc_offset_cfg;

typedef struct ISP_pdaf_loc_end_cfg {
  uint32_t       x_end                  :  14;
  uint32_t      /* reserved */          :  2;
  uint32_t       y_end                  :  14;
  uint32_t      /* reserved */          :  2;
}__attribute__((packed, aligned(4))) ISP_pdaf_loc_end_cfg;

typedef struct ISP_pdaf_loc_config {
  ISP_pdaf_loc_offset_cfg             loc_offset_cfg;
  ISP_pdaf_loc_end_cfg                loc_end_cfg;
}__attribute__((packed, aligned(4))) ISP_pdaf_loc_config;

/////////////////////////////////////////////////////////////////////
////////////////////loc submodule ending///////////////////////////

typedef struct ISP_PDAF_Reg_t {
  ISP_pdaf_config                     pdaf_config;
  ISP_pdaf_hdr_config                 hdr_config;
  ISP_pdaf_bp_config                  bp_config;
  ISP_pdaf_wb_gain_config             wb_gain_config;
  ISP_pdaf_loc_config                 loc_config;
} __attribute__((packed, aligned(4))) ISP_PDAF_Reg_t;

#endif /* __PDAF_REG_H__ */