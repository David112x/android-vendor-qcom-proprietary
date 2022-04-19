/* af_ext.h
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __AF_EXT_H__
#define __AF_EXT_H__

#include "af_port.h"
#include "af.h"
#include "stats_util.h"

/**
 * Custom output mask.
 */
typedef enum {
  AF_OUTPUT_CUSTOM_MAX
} af_ext_output_mask_type;

/**
 * Custom output structure. External core should define
 * the parameters here.
 */
typedef struct {
  uint32_t mask;
  /* Add parameters here */
} af_ext_output_t;

typedef struct _af_ext_param {
  af_ext_output_t custom_output;
} af_ext_param_t;

boolean af_port_ext_is_extension_required(void *af_libptr,
  cam_position_t cam_position, boolean *use_default_algo);
boolean af_port_ext_update_func_table(void *ptr);
void * af_port_ext_load_function(af_ops_t *af_ops, void *af_lib,
  cam_position_t cam_position, boolean use_default_algo);
void af_port_ext_unload_function(af_port_private_t *private);



#endif /*__AF_EXT_H__*/
