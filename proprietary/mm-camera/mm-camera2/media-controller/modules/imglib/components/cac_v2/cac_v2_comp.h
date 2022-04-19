/***************************************************************************
* Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
****************************************************************************/

#ifndef __CAC_COMP_H__
#define __CAC_COMP_H__

#include "cac.h"
#include "img_comp_priv.h"
#include "chromatix.h"
#include "cac2_interface.h"

/** MODULE_MASK:
 *
 * Mask to enable dynamic logging
 **/
#undef MODULE_MASK
#define MODULE_MASK IMGLIB_CAC_SHIFT

/** cac_comp_t
 *   @b: base component
 *   @chromatix_info: chromatix info from the chromatix header
 *   @info_3a: Awb gain info
 *   @chroma_order : CBCR or CRCB
 *   @cds_enabled: CDS Enabled Flag
 *   @chromatix_ptr : Ptr to chromatix
 *   @cac_enable_flag: Enable CAC
 *   @rnr_enable_flag: Enable RNR flag
 *   @rnr_hysterisis_info: RNR hysterisys info
 *   @process_cds: Flag which indicates whether to process CDS
 *   @cds_proc_type: Type of CDS processing
 *
 *   CAC Component structure
 **/
typedef struct {
  /*base component*/
  img_component_t b;
  cac_v2_chromatix_info_t cac_chromatix_info;
  rnr_chromatix_info_t rnr_chromatix_info;
  cac_chroma_order chroma_order;
  cac_3a_info_t info_3a;
  uint8_t cds_enabled;
  void *chromatix_ptr;
  uint8_t cac2_enable_flag;
  uint8_t rnr_enable_flag;
  hysterisis_info_t rnr_hysterisis_info;
  uint8_t process_cds;
  img_ops_core_type cds_proc_type;
} cac2_comp_t;

#endif
