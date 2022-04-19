/***************************************************************************
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ***************************************************************************/
#ifndef __EZTUNE_LOOKUP_H__
#define __EZTUNE_LOOKUP_H__

#include "chromatix.h"
#include "chromatix_common.h"
#include "chromatix_cpp.h"
#include "chromatix_swpostproc.h"
#include "sensor_common.h"


typedef struct {
  metadata_buffer_t *metadata;
  chromatix_parms_type *chromatixptr;
  chromatix_parms_type *snap_chromatixptr;
  chromatix_VFE_common_type *common_chromatixptr;
  chromatix_cpp_type *cpp_chromatixptr;
  chromatix_cpp_type *snap_cpp_chromatixptr;
  chromatix_sw_postproc_type *swpp_chromatixptr;
  chromatix_3a_parms_type *aaa_chromatixptr;
  void *af_driver_ptr;          /* actuator_driver_params_t */
  void *af_tuning_ptr;          /* ez_af_tuning_params_t */
  void *tuning_set_vfe;
  void *tuning_set_pp;
  void *tuning_set_3a;
  void *tuning_set_focus;
  void *tuning_post_bus_msg;
} eztune_init_t;


#if defined(__cplusplus)
extern "C" {
#endif

void eztune_lookup_init(
  eztune_init_t *eztune_init);

void eztune_lookup_get_range_tags(
  int32_t *out_id_chromatix_start,
  int32_t *out_id_chromatix_end,
  int32_t *out_id_diags_start,
  int32_t *out_id_diags_end);

int32_t eztune_lookup_item(
  int32_t id,
  char *output,
  int32_t output_size);

#if defined(__cplusplus)
}
#endif

#endif  /* __EZTUNE_LOOKUP_H__ */
