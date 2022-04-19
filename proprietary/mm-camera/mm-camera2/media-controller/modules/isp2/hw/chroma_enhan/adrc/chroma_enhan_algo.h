/* chroma_enhan_algo.h
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __CHROMA_ENHAN_ALGO_H__
#define __CHROMA_ENHAN_ALGO_H__

#include "modules.h"

void chroma_enhan_algo_adjust_cv_by_adrc(Chromatix_ADRC_ACE_adj_type *adrc_adj_data,
  chroma_enhancement_type_t *cv, aec_update_t aec_update);

#endif /* __CHROMA_ENHAN_ALGO_H__ */
