/*
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __ASD_EXT_ALGO_WRAPPER_H__
#define __ASD_EXT_ALGO_WRAPPER_H__

#include "asd_module.h"
#include "asd.h"

typedef struct  asd_wrapper_iface_handle_t {
  void            *dafault_lib_handle;
  void            *oem_lib_handle;
  asd_ops_t       default_ops;
  asd_ops_t       oem_ops;
} asd_wrapper_iface_handle_t;

void asd_wrapper_link_algo(asd_ops_t *asd_ops);
void asd_wrapper_unlink_algo(asd_ops_t *asd_ops);

#endif /* __ASD_EXT_ALGO_WRAPPER_H__ */