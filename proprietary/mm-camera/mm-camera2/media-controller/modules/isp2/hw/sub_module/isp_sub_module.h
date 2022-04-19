/* isp_sub_module.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __ISP_SUB_MODULE_H__
#define __ISP_SUB_MODULE_H__

/* isp headers */
#include "isp_sub_module_common.h"


mct_module_t *isp_sub_module_init(const char *name, uint32_t num_sink_ports,
  uint32_t num_source_ports, isp_sub_module_private_func_t *private_func,
  isp_hw_module_id_t hw_module_id,char *mod_name, isp_log_sub_modules_t mod_id);

void isp_sub_module_deinit(mct_module_t *module);

#endif
